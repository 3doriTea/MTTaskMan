#include <dpp/dpp.h>
#include <iostream>
#include "json/JsonReader.h"
#include "Alarm.h"
#include <string>
#include <vector>
#include <mutex>
#include <list>
#include <cstdint>
#include <map>
#include <utility>
#include "Utility.h"
#include "Task.h"
#include "UserData.h"
#include "embeds/help.h"
#include "embeds/task.h"
#include "embeds/user.h"
#include "ReleaseUtility.h"

/*
* あくまでも完成がメイン -> きれいにするのは後回しになってしまった…
*/

int main()
{
	using std::cout;
	using std::endl;
	using Task::TaskContent;
	using UserData::UserContent;
	using json = nlohmann::json;

	std::promise<int> onAram{};
	std::future<int> onAramFuture{ onAram.get_future() };

	JsonReader* config = new JsonReader{ "private/config.json", true };
	JsonReader* users = new JsonReader{ "private/users.json" };
	JsonReader* last = new JsonReader{ "private/last.json" };
	JsonReader* tasks = new JsonReader{ "private/task.json" };

	std::mutex jsonWriteMutex{};  // jsonに書き込む際の排他制御用
	
	dpp::cluster bot(config->Json()["TOKEN"].get<std::string>());

	bot.on_log(dpp::utility::cout_logger());

	// コマンドを受信
	bot.on_slashcommand([&bot, &tasks, &config, &users, &jsonWriteMutex](const dpp::slashcommand_t& event)
		{
			using dpp::utility::timestamp;
			using dpp::utility::time_format;
			using dpp::utility::user_mention;
			using dpp::utility::user_url;

			if (users->Json()[std::to_string(event.command.usr.id)]["white"].get<bool>() == false)
			{
				event.reply(dpp::message(ToString(u8"-# 権限がありません。")).set_flags(dpp::m_ephemeral));
				return;  // ホワイトリストに乗っていないユーザからのコマンドは回帰
			}

			// 各コマンドに反応
			std::string commandName{ event.command.get_command_name() };
			if (commandName == "ping")
			{
				event.reply("Pong!");
			}
			else if (commandName == "help")
			{
				// ヘルプのembed返す
				event.reply(dpp::message(
					event.command.channel_id,
					GenerateEmbed::Help(tasks))
					.set_flags(dpp::m_ephemeral));
			}
			else if (commandName == "newtask")
			{
				TaskContent content{};

				content.id = tasks->Json()["list"].size() + tasks->Json()["archive"].size();
				content.name = std::get<std::string>(event.get_parameter("name"));

				// タスク名の重複 + 総額 を探索
				int64_t totalCost{ 0 };
				for (auto& taskJson : tasks->Json()["list"])
				{
					if (taskJson["name"].get<std::string>() == content.name)
					{
						event.reply(dpp::message(ToString(u8"既存のタスクと名前が重複しています。")).set_flags(dpp::m_ephemeral));
						return;
					}

					totalCost += taskJson["price"].get<int32_t>();
				}

				const int64_t INCOME{ config->Json()["INCOME"].get<int64_t>() };

				if (totalCost == INCOME)
				{
					event.reply(dpp::message(ToString(u8"現在タスクの総額が上限に達しています。\nタスクを消化してください。")).set_flags(dpp::m_ephemeral));
					return;  // 総額に達している場合は回帰
				}

				dpp::message message{ ToString(u8"新しいタスクを発行しました。") };

				content.price = static_cast<int32_t>(std::get<int64_t>(event.get_parameter("price")));

				if ((content.price + totalCost) > INCOME)
				{
					content.price = INCOME - totalCost;  // 上限に収まる価格に変更
					message.content += ToString(u8"\nなお、リクエストされた価格が上限を超えるため価格下げを行いました。");
				}

				// (1日 = 60 * 60 * 24 秒) * (期限時間(日にち単位)) + 現在時刻 = 期限時刻
				content.deadline = static_cast<time_t>(
						(60.0 * 60.0 * 24.0)
						* (std::get<double_t>(event.get_parameter("deadline")))
					) + std::time(nullptr);

				content.description = std::get<std::string>(event.get_parameter("description"));
				content.author = event.command.usr.id;

				{  // 排他制御
					std::lock_guard<std::mutex> lock(jsonWriteMutex);
					tasks->Json()["list"][std::to_string(content.id)] = content;
					tasks->TrySave();
				}

				message.add_embed(GenerateEmbed::TaskUnassigned(&content, &event.command.usr, tasks));

				event.reply(message);  // 返信！
			}
			else if (commandName == "tasks")
			{
				size_t size{ tasks->Json()["list"].size() };
				if (size <= 0)
				{
					event.reply(ToString(u8"今のところタスクはありません！ 🎉"));
					return;  // タスク無いならreturn
				}

				// タスクがあるなら状況分けして表示

				std::list<TaskContent*> inProgressTasks{};  // 順調に進行中のタスクたち
				std::list<TaskContent*> unassignedTasks{};  // 請負人がいないタスクたち
				std::list<TaskContent*> outTasks{};         // 期限超過してるタスクたち

				time_t nowTime{ std::time(nullptr) };  // 今の時刻

				// 全タスク周回 + 総額計算
				int64_t totalCost{ 0 };
				for (auto& taskJson : tasks->Json()["list"])
				{
					TaskContent* taskContent{ new TaskContent{ taskJson.get<TaskContent>() } };

					totalCost += taskContent->price;  // 総額加算

					// タスクの振り分け

					if (taskContent->deadline > nowTime)  // 期限内か
					{
						if (taskContent->undertakers.size() > 0)  // 請負人がいるか
						{
							// 問題なく進行中のタスクとして追加
							Task::InsertListDesc(inProgressTasks, taskContent);
						}
						else
						{
							// 請負人がいないタスクとして追加
							Task::InsertListDesc(unassignedTasks, taskContent);
						}
					}
					else
					{
						// 期限超過のタスクとして追加 (まずい…)
						Task::InsertListDesc(outTasks, taskContent);
					}
				}

				// 返信するメッセージに追加していく
				dpp::message message{};

				for (auto& content : outTasks)
				{
					message.add_embed(GenerateEmbed::TaskOut(content, dpp::find_user(content->author), tasks));
				}

				for (auto& content : unassignedTasks)
				{
					message.add_embed(GenerateEmbed::TaskUnassigned(content, dpp::find_user(content->author), tasks));
				}

				for (auto& content : inProgressTasks)
				{
					message.add_embed(GenerateEmbed::TaskInProgress(content, dpp::find_user(content->author), tasks));
				}

				// 概要のembed追加
				message.add_embed(GenerateEmbed::About(
					inProgressTasks,
					unassignedTasks,
					outTasks,
					config->Json()["INCOME"].get<int64_t>(),
					totalCost));

				event.reply(message);  // 返信！

				SAFE_DELETE_LIST(inProgressTasks);
				SAFE_DELETE_LIST(unassignedTasks);
				SAFE_DELETE_LIST(outTasks);
			}
			else if (commandName == "dotask")
			{
				if (tasks->Json()["list"].size() <= 0)
				{
					event.reply(ToString(u8"今のところタスクはありませよ！ 🎉"));
					return;  // タスク無いなら回帰
				}

				std::string requestTaskIdStr{ std::get<std::string>(event.get_parameter("donthavetaskname")) };

				if (tasks->Json()["list"].count(requestTaskIdStr) != 1)
				{
					event.reply(
						dpp::message(ToString(u8"タスクの名前が一致しません。"))
						.set_flags(dpp::m_ephemeral));
					return;  // 名前不一致で回帰
				}

				auto& taskJson = tasks->Json()["list"][requestTaskIdStr];
				TaskContent content{ taskJson.get<TaskContent>() };

				for (auto& undertaker : content.undertakers)
				{
					if (undertaker == event.command.usr.id)
					{
						event.reply(
							dpp::message(ToString(u8"既に請負人になっています！"))
								.set_flags(dpp::m_ephemeral));
						return;  // 既に請負人なら回帰
					}
				}

				{  // 排他制御
					std::lock_guard<std::mutex> lock(jsonWriteMutex);
					content.undertakers.push_back(event.command.usr.id);
					taskJson = content;
					tasks->TrySave();
				}

				dpp::message message
				{
					ToString(u8"タスクを受諾しました。")
				};

				// 期限内、外で変化
				if (content.deadline >= std::time(nullptr))
				{
					message.add_embed(GenerateEmbed::TaskInProgress(&content, &event.command.usr, tasks));
				}
				else
				{
					message.add_embed(GenerateEmbed::TaskOut(&content, &event.command.usr, tasks));
				}

				event.reply(message);
				return;  // 受諾できたら回帰
			}
			else if (commandName == "comptask")
			{
				if (tasks->Json()["list"].size() <= 0)
				{
					event.reply(ToString(u8"今のところタスクはありませよ！ 🎉"));
					return;  // タスク無いなら回帰
				}

				std::string requestTaskIdStr{ std::get<std::string>(event.get_parameter("havetaskname")) };

				if (tasks->Json()["list"].count(requestTaskIdStr) != 1)
				{
					event.reply(dpp::message(ToString(u8"タスクの名前が一致しません。")).set_flags(dpp::m_ephemeral));
					return;  // 名前不一致で回帰
				}

				auto& taskJson = tasks->Json()["list"][requestTaskIdStr];
				TaskContent content{ taskJson.get<TaskContent>() };  // タスク内容取得

				std::list<TaskContent*> childContents{};

				// 全子タスクの探索
				for (auto& pickTaskJson : tasks->Json()["list"])
				{
					if (pickTaskJson["parentTaskId"].get<int64_t>() == content.id)
					{
						childContents.push_back(new TaskContent{ pickTaskJson.get<TaskContent>() });
					}
				}

				// 子タスクがあるなら
				if (childContents.size() > 0)
				{
					dpp::message message{ ToString(u8"未完了のサブタスクがあるため、完了できません。") };

					for (auto& childContent : childContents)
					{
						message.add_embed(GenerateEmbed::Task(childContent, tasks));
					}

					SAFE_DELETE_LIST(childContents);  // 使い終わったから解放

					event.reply(message);
					return;  // 未消化のサブタスクがあるため回帰
				}

				// 請負人か探索
				bool isUndertaker{ false };
				for (auto& undertaker : content.undertakers)
				{
					if (undertaker == event.command.usr.id)
					{
						isUndertaker = true;
						break;
					}
				}

				if (isUndertaker == false)
				{
					event.reply(dpp::message(ToString(u8"あなたはこのタスクの請負人ではありません！")).set_flags(dpp::m_ephemeral));
					return;  // 請負人ではないなら回帰
				}

				// 期限を超過しているか
				bool isOutDeadline{ false };
				if (content.deadline < std::time(nullptr))
				{
					isOutDeadline = true;
				}

				// タスクの請負人全員のデータ更新
				for (auto& undertakerId : content.undertakers)
				{
					UserContent userContent{ users->Json()[std::to_string(undertakerId)].get<UserContent>() };

					// userContent.write = true;
					userContent.completedTotalCount++;  // 完了タスク消化数加算
					userContent.completedTotalPrice += content.price;  // 完了タスク総額加算

					if (isOutDeadline)
					{
						// 期限超過なら科料分を引く
						userContent.money -= content.price;
					}
					else
					{
						// 期限内なら分けて加算
						userContent.money += content.price / content.undertakers.size();
					}

					{  // 排他制御
						std::lock_guard<std::mutex> lock(jsonWriteMutex);
						users->Json()[std::to_string(undertakerId)] = userContent;
					}
				}

				{  // 排他制御
					std::lock_guard<std::mutex> lock(jsonWriteMutex);

					tasks->Json()["archive"].push_back(content);
					tasks->Json()["list"].erase(requestTaskIdStr);

					tasks->TrySave();
					users->TrySave();
				}

				event.reply(ToString(u8"タスクを完了しました。🎉\n-# お疲れ様！おめでとう！"));
				return;  // 完了できたら回帰
			}
			else if (commandName == "myinfo")
			{
				// 返信するメッセージに追加していく
				dpp::message message{};

				// タスクがあるなら状況分けして表示

				std::list<TaskContent*> inProgressTasks{};  // 順調に進行中のタスクたち
				std::list<TaskContent*> outTasks{};         // 期限超過してるタスクたち

				time_t nowTime{ std::time(nullptr) };  // 今の時刻

				// 全タスク周回 + 総額計算
				int64_t totalCost{ 0 };
				for (auto& taskContentJson : tasks->Json()["list"])
				{
					TaskContent* taskContent{ new TaskContent{ taskContentJson.get<TaskContent>() } };

					// 請負人かどうか探索
					bool isUndertaker{ false };
					for (auto& undertaker : taskContent->undertakers)
					{
						if (undertaker == event.command.usr.id)
						{
							isUndertaker = true;
							break;
						}
					}

					if (isUndertaker == false)
					{
						continue;  // 請負人でないなら回帰
					}

					totalCost += taskContent->price;  // 総額加算

					// タスクの振り分け

					if (taskContent->deadline > nowTime)  // 期限内か
					{
						// 問題なく進行中のタスクとして追加
						Task::InsertListDesc(inProgressTasks, taskContent);
					}
					else
					{
						// 期限超過のタスクとして追加 (まずい…)
						Task::InsertListDesc(outTasks, taskContent);
					}
				}

				for (auto& content : outTasks)
				{
					message.add_embed(GenerateEmbed::TaskOut(content, dpp::find_user(content->author), tasks));
				}

				for (auto& content : inProgressTasks)
				{
					message.add_embed(GenerateEmbed::TaskInProgress(content, dpp::find_user(content->author), tasks));
				}

				// 概要のembed追加
				message.add_embed(GenerateEmbed::UserInfo(
					inProgressTasks,
					outTasks,
					users->Json()[std::to_string(event.command.usr.id)].get<UserContent>(),
					&event.command.usr));

				event.reply(message);  // 返信！

				SAFE_DELETE_LIST(outTasks);
				SAFE_DELETE_LIST(inProgressTasks);
			}
			else if (commandName == "splitask")
			{
				std::string requestParentIdStr{ std::get<std::string>(event.get_parameter("taskname")) };

				if (tasks->Json()["list"].count(requestParentIdStr) != 1)
				{
					event.reply(
						dpp::message(ToString(u8"分割元のタスク名が一致しません。"))
						.set_flags(dpp::m_ephemeral));
					return;  // 分割元タスク名不一致で回帰
				}

				TaskContent parentContent{ tasks->Json()["list"][requestParentIdStr].get<TaskContent>() };

				TaskContent content{};

				content.parentTaskId = parentContent.id;
				content.id = tasks->Json()["list"].size() + tasks->Json()["archive"].size();
				content.name = std::get<std::string>(event.get_parameter("name"));

				// タスク名の重複を探索
				for (auto& taskJson : tasks->Json()["list"])
				{
					if (taskJson["name"].get<std::string>() == content.name)
					{
						event.reply(dpp::message(ToString(u8"既存のタスクと名前が重複しています。")).set_flags(dpp::m_ephemeral));
						return;
					}
				}

				const int64_t PRICE_MAX{ parentContent.price };

				dpp::message message{ ToString(u8"タスクを分割し、サブタスクを発行しました。") };

				content.price = static_cast<int32_t>(std::get<int64_t>(event.get_parameter("price")));

				bool changedRequestPrice{ false };
				if (content.price > PRICE_MAX)
				{
					changedRequestPrice = true;
					content.price = PRICE_MAX;  // 上限に収まる価格に変更
					message.content += ToString(u8"\nなお、リクエストされた価格が上限を超えるため価格下げを行いました。");
				}

				parentContent.price -= content.price;  // 親の額から引く

				// (1日 = 60 * 60 * 24 秒) * (期限時間(日にち単位)) + 現在時刻 = 期限時刻
				content.deadline = static_cast<time_t>(
					(60.0 * 60.0 * 24.0)
					* (std::get<double_t>(event.get_parameter("deadline")))
					) + std::time(nullptr);

				if (content.deadline > parentContent.deadline)
				{
					content.deadline = parentContent.deadline;  // 親の期限を超えないように変更
					message.content += changedRequestPrice  // 価格変更のメッセージがあるなら始め方を変える
						? ToString(u8"\nまた、")
						: ToString(u8"\nなお、");
					message.content += ToString(u8"リクエストされた期日が分割元の期日を超えるため、調整を行いました。");
				}

				content.description = std::get<std::string>(event.get_parameter("description"));
				content.author = event.command.usr.id;

				{  // 排他制御
					std::lock_guard<std::mutex> lock(jsonWriteMutex);
					tasks->Json()["list"][std::to_string(parentContent.id)] = parentContent;
					tasks->Json()["list"][std::to_string(content.id)] = content;
					tasks->TrySave();
				}

				message.add_embed(GenerateEmbed::TaskUnassigned(&content, &event.command.usr, tasks));

				event.reply(message);  // 返信！
			}

		});

	// 準備中...　コマンドの登録とか
	bot.on_ready([&bot, &config, &tasks](const dpp::ready_t& event)
		{
			if (dpp::run_once<struct register_bot_commands>())
			{
#if false  // もしコマンドを綺麗さっぱり削除するならture
				bot.global_bulk_command_delete();
				return;
#endif

				std::u8string description{};
				std::vector<dpp::slashcommand> slashcommands{};

#pragma region コマンドの登録
				description = u8"疎通確認をする";
				slashcommands.push_back(
					dpp::slashcommand("ping", { description.begin(), description.end() }, bot.me.id));


				description = u8"ヘルプを確認する";
				slashcommands.push_back(
					dpp::slashcommand("help", { description.begin(), description.end() }, bot.me.id));
				

				description = u8"自分の情報を確認する";
				slashcommands.push_back(
					dpp::slashcommand("myinfo", { description.begin(), description.end() }, bot.me.id));


				description = u8"新しいタスクを発行する";
				slashcommands.push_back(
					dpp::slashcommand("newtask", { description.begin(), description.end() }, bot.me.id)
					.add_option(
						dpp::command_option(
							dpp::co_string,
							"name",
							ToString(u8"タスクの名前(検索時に打ちやすい文字列を推奨)"),
							true))
					.add_option(
						dpp::command_option(
							dpp::co_integer,
							"price",
							ToString(u8"タスクの値段"),
							true)
							.set_min_value(0LL)
							.set_max_value(config->Json()["INCOME"].get<int64_t>()))
					.add_option(
						dpp::command_option(
							dpp::co_number,
							"deadline",
							ToString(u8"タスクの締め切り(%f日後)"),
							true)
							.set_min_value(0.0)
							.set_max_value(config->Json()["DAYS_AHEAD_MAX"].get<double>()))
					.add_option(
						dpp::command_option(
							dpp::co_string,
							"description",
							ToString(u8"タスクの詳細な説明"),
							true)));
				

				description = u8"発行されているタスクを確認する";
				slashcommands.push_back(
					dpp::slashcommand("tasks", { description.begin(), description.end() }, bot.me.id));
				

				description = u8"タスクを引き受ける";
				slashcommands.push_back(
					dpp::slashcommand("dotask", { description.begin(), description.end() }, bot.me.id)
					.add_option(
						dpp::command_option(
							dpp::co_string,
							"donthavetaskname",
							ToString(u8"引き受けるタスクの名前"),
							true)
						.set_auto_complete(true)));


				description = u8"タスクを完了する";
				slashcommands.push_back(
					dpp::slashcommand("comptask", { description.begin(), description.end() }, bot.me.id)
					.add_option(
						dpp::command_option(
							dpp::co_string,
							"havetaskname",
							ToString(u8"完了するタスクの名前"),
							true)
						.set_auto_complete(true)));

				description = u8"サブタスクに分割する";
				slashcommands.push_back(
					dpp::slashcommand("splitask", { description.begin(), description.end() }, bot.me.id)
					.add_option(
						dpp::command_option(
							dpp::co_string,
							"taskname",
							ToString(u8"分割したいタスクの名前"),
							true)
						.set_auto_complete(true))
					.add_option(
						dpp::command_option(
							dpp::co_string,
							"name",
							ToString(u8"サブタスクの名前(検索時に打ちやすい文字列を推奨)"),
							true))
					.add_option(
						dpp::command_option(
							dpp::co_integer,
							"price",
							ToString(u8"タスクの値段"),
							true)
						.set_min_value(0LL)
						.set_max_value(config->Json()["INCOME"].get<int64_t>()))
					.add_option(
						dpp::command_option(
							dpp::co_number,
							"deadline",
							ToString(u8"サブタスクの締め切り(%f日後)"),
							true)
						.set_min_value(0.0)
						.set_max_value(config->Json()["DAYS_AHEAD_MAX"].get<double>()))
					.add_option(
						dpp::command_option(
							dpp::co_string,
							"description",
							ToString(u8"サブタスクの詳細な説明"),
							true)));
#pragma endregion

				// 一気にまとめて登録 && jsonにもカキカキ
				bot.global_bulk_command_create(slashcommands, [&tasks](const dpp::confirmation_callback_t& confirmation)
					{
						dpp::slashcommand_map commands{ confirmation.get<dpp::slashcommand_map>() };
						for (auto& command : commands)
						{
							// コマンドidをjsonに保存
							tasks->Json()["command-ids"][command.second.name] = command.first;
							tasks->TrySave();
						}
					});
			}
		});

	// 自動補完の応答
	bot.on_autocomplete([&bot, &tasks](const dpp::autocomplete_t& autocompolete)
		{
			for (auto& option : autocompolete.options)
			{
				// 全タスク名
				if (option.name == "taskname")
				{
					std::string inputValue{ std::get<std::string>(option.value) };

					dpp::interaction_response response = dpp::interaction_response(dpp::ir_autocomplete_reply);

					for (auto& taskJson : tasks->Json()["list"])
					{
						response.add_autocomplete_choice(dpp::command_option_choice(
							taskJson["name"].get<std::string>(),
							std::to_string(taskJson["id"].get<int64_t>())));
					}

					bot.interaction_response_create(autocompolete.command.id, autocompolete.command.token, response);
					break;
				}
				// 持っているタスク名
				else if (option.name == "havetaskname")
				{
					dpp::snowflake userId{ autocompolete.command.usr.id };

					std::string inputValue{ std::get<std::string>(option.value) };
					dpp::interaction_response response = dpp::interaction_response(dpp::ir_autocomplete_reply);

					// TODO: タスクの探索は非効率すぎるため、ユーザごとに持っているタスクを保持する
					// 全タスクを探索
					for (auto& taskJson : tasks->Json()["list"])
					{
						// 請負人の探索
						for (auto& undertakerJson : taskJson["undertakers"])
						{
							// 請負人に含まれているか
							if (static_cast<dpp::snowflake>(undertakerJson.get<std::string>()) == userId)
							{
								response.add_autocomplete_choice(dpp::command_option_choice(
									taskJson["name"].get<std::string>(),
									std::to_string(taskJson["id"].get<int64_t>())));
								break;  // 含まれているなら探索終了
							}
						}
					}

					bot.interaction_response_create(autocompolete.command.id, autocompolete.command.token, response);
					break;
				}
				// 持っていないタスク名
				else if (option.name == "donthavetaskname")
				{
					dpp::snowflake userId{ autocompolete.command.usr.id };

					std::string inputValue{ std::get<std::string>(option.value) };
					dpp::interaction_response response = dpp::interaction_response(dpp::ir_autocomplete_reply);

					// TODO: タスクの探索は非効率すぎるため、ユーザごとに持っているタスクを保持する
					// 全タスクを探索
					for (auto& taskJson : tasks->Json()["list"])
					{
						// 請負人の探索
						bool isUndertaker{ false };
						for (auto& undertakerJson : taskJson["undertakers"])
						{
							// 請負人に含まれているか
							if (static_cast<dpp::snowflake>(undertakerJson.get<std::string>()) == userId)
							{
								isUndertaker = true;
								break;  // 含まれているなら探索終了
							}
						}

						// 請負人にいないなら候補に追加
						if (isUndertaker == false)
						{
							response.add_autocomplete_choice(dpp::command_option_choice(
								taskJson["name"].get<std::string>(),
								std::to_string(taskJson["id"].get<int64_t>())));
						}
					}

					bot.interaction_response_create(autocompolete.command.id, autocompolete.command.token, response);
					break;
				}
			}
		});

	// メッセージ受信
	bot.on_message_create([&bot, &users](const dpp::message_create_t& event)
		{
			if (users->Json()[std::to_string(event.msg.author.id)]["white"].get<bool>() == false)
			{
				return;  // ホワイトリストに乗っていないユーザからのメッセージは回帰
			}

			return;

			dpp::snowflake id = event.msg.channel_id;
			std::u8string message{ u8"検知" };
			event.reply(std::string { message.begin(), message.end() });
			//
		});

	Alarm alarm{ last->Json()["lastTime"].get<long long>() };

	alarm.Run();

	alarm.OnAlarm([&bot, &last](Alarm::CallType callType)
		{
			last->Json()["lastTime"] = std::time(nullptr);
			last->TrySave();
		});

	bot.start(dpp::st_wait);

	alarm.Join();  // 非同期を待つ

	delete config;
	delete users;
	delete last;
	delete tasks;
}
