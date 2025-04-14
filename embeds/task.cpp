#include "task.h"
#include "../Utility.h"

enum
{
	COLOR_WARN = 0xff0000,
	COLOR_NOTE = 0xffd700,
	COLOR_NORM = 0x00ff7f,
	COLOR_FREE = 0x00ffff,
};

static inline std::string ToUserURLLocal(dpp::snowflake _userId)
{
	return "discord://discord.com/users/" + std::to_string(_userId);
}

static inline std::string ToYen(uint32_t yen)
{
	return std::to_string(yen) + ToString(u8"円");
}

dpp::embed GenerateEmbed::About(
	const std::list<Task::TaskContent*>& _inProgressTasks,
	const std::list<Task::TaskContent*>& _unassignedTasks,
	const std::list<Task::TaskContent*>& _outTasks,
	const int64_t& _income,
	const int64_t& _totalCost)
{
	dpp::embed embed = dpp::embed()
		.set_title(ToString(u8"タスク：")
			+ std::to_string(_inProgressTasks.size() + _unassignedTasks.size() + _outTasks.size())
			+ ToString(u8"件 - 空き：")
			+ std::to_string(_totalCost) + " / " + std::to_string(_income)
			+ ToString(u8" 円"));

#pragma region 説明 (あったら)
	if (_income == _totalCost)  // 総額が上限に達していたら
	{
		embed.set_description(
			ToString(u8"現在、タスクの総額が上限に達しています。")
			+ ToString(u8"そのため、タスクの新規発行はできません。")
			+ ToString(u8"タスクを消化し、総額に空きを作ってください。"));
	}
#pragma endregion

#pragma region 色付け
	uint32_t color{};
	if (_outTasks.size() > 0)
		color = COLOR_WARN;  // 警告
	else if (_unassignedTasks.size() > 0)
		color = COLOR_NOTE;  // 注意
	else if (_inProgressTasks.size() > 0)
		color = COLOR_NORM;  // 普通
	else
		color = COLOR_FREE;  // 安心

	embed.set_color(color);
#pragma endregion

#pragma region タスク数フィールド
	bool hasTask{ false };

	if (_outTasks.size() > 0)
	{
		hasTask = true;
		embed.add_field(
			ToString(u8"❌期限超過：") + std::to_string(_outTasks.size()) + ToString(u8"件"),
			ToString(u8"まずいです。優先して消化して下さい。"),
			true);
	}
	if (_unassignedTasks.size() > 0)
	{
		hasTask = true;
		embed.add_field(
			ToString(u8"❕請負人ナシ：") + std::to_string(_unassignedTasks.size()) + ToString(u8"件"),
			ToString(u8"誰か！タスクを受けてください！"),
			true);
	}
	if (_inProgressTasks.size() > 0)
	{
		hasTask = true;
		embed.add_field(
			ToString(u8"🟢進行中：") + std::to_string(_inProgressTasks.size()) + ToString(u8"件"),
			ToString(u8"順調に進んでいる…はずです。"),
			true);
	}
	if (hasTask == false)
	{
		hasTask = true;
		embed.add_field(
			ToString(u8"🎉タスク無し"),
			ToString(u8"タスクを追加する場合は `/help` を参照"),
			true);
	}
#pragma endregion

	return embed;
}

dpp::embed GenerateEmbed::TaskOut(
	const Task::TaskContent* _pTask,
	const dpp::user* _author)
{
	using dpp::utility::timestamp;
	using dpp::utility::time_format;
	using dpp::utility::user_mention;
	using dpp::utility::user_url;

	dpp::embed embed = dpp::embed()
		.set_color(COLOR_WARN)
		.set_author(_author->global_name, user_url(_pTask->author), _author->get_avatar_url())
		.set_title(ToString(u8"**『") + _pTask->name + ToString(u8"』** ❌期限超過❌"))
		.set_description(_pTask->description);

	embed
		.add_field(
			ToString(u8"発生科料"),
			ToYen(_pTask->price),
			true)
		.add_field(
			ToString(u8"期限"),
			timestamp(_pTask->deadline, time_format::tf_relative_time),
			true);

#pragma region フィールド：請負人たち
	std::string undertakersText{};
	if (_pTask->undertakers.size() > 0)  // 請負人がいるなら
	{
		for (auto& undertaker : _pTask->undertakers)
		{
			undertakersText += user_mention(undertaker) + " ";
		}
	}
	else  // 請負人がいないなら
	{
		undertakersText = ToString(u8"いないよ！？");
	}

	embed.add_field(
		ToString(u8"請負人"),
		undertakersText, true);
#pragma endregion

	return embed;
}

dpp::embed GenerateEmbed::TaskUnassigned(
	const Task::TaskContent* _pTask,
	const dpp::user* _author)
{
	using dpp::utility::timestamp;
	using dpp::utility::time_format;
	using dpp::utility::user_mention;
	using dpp::utility::user_url;

	dpp::embed embed = dpp::embed()
		.set_color(COLOR_NOTE)
		.set_author(_author->global_name, user_url(_pTask->author), _author->get_avatar_url())
		.set_title(ToString(u8"**『") + _pTask->name + ToString(u8"』** ❕請負人ナシ❕"))
		.set_description(_pTask->description)
		.set_footer(ToString(u8"期限時刻"), "")
		.set_timestamp(_pTask->deadline);

	embed
		.add_field(
			ToString(u8"価値"),
			ToYen(_pTask->price),
			true)
		.add_field(
			ToString(u8"期限"),
			timestamp(_pTask->deadline, time_format::tf_relative_time),
			true);

	return embed;
}

dpp::embed GenerateEmbed::TaskInProgress(
	const Task::TaskContent* _pTask,
	const dpp::user* _author)
{
	using dpp::utility::timestamp;
	using dpp::utility::time_format;
	using dpp::utility::user_mention;
	using dpp::utility::user_url;

	dpp::embed embed = dpp::embed()
		.set_color(COLOR_NORM)
		.set_author(_author->global_name, user_url(_pTask->author), _author->get_avatar_url())
		.set_title(ToString(u8"**『") + _pTask->name + ToString(u8"』** 🟢進行中🟢"))
		.set_description(_pTask->description)
		.set_footer(ToString(u8"期限時刻"), "")
		.set_timestamp(_pTask->deadline);

	embed
		.add_field(
			ToString(u8"達成報酬"),
			ToYen(_pTask->price / _pTask->undertakers.size()),
			true)
		.add_field(
			ToString(u8"期限"),
			timestamp(_pTask->deadline, time_format::tf_relative_time),
			true);

#pragma region フィールド：請負人たち
	std::string undertakersText{};
	for (auto& undertaker : _pTask->undertakers)
	{
		undertakersText += user_mention(undertaker) + " ";
	}

	embed.add_field(
		ToString(u8"請負人"),
		undertakersText, true);
#pragma endregion

	return embed;
}
