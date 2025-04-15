#include "user.h"
#include "../Utility.h"
#include "Color.h"

dpp::embed GenerateEmbed::UserInfo(
	const std::list<Task::TaskContent*>& _inProgressTasks,
	const std::list<Task::TaskContent*>& _outTasks,
	const UserData::UserContent& _userContent,
	const dpp::user* _pAuthor)
{
	using dpp::utility::user_url;

	dpp::embed embed = dpp::embed()
		.set_author(_pAuthor->global_name, user_url(_pAuthor->id), _pAuthor->get_avatar_url());

	if ((_inProgressTasks.size() + _outTasks.size()) == 0ULL)
	{
		// 請負中のタスクNothingなら
		embed.set_title(ToString(u8"請負中のタスク：なし"));
	}
	else
	{
		// 請負中のタスクがあるなら
		embed.set_title(ToString(u8"請負中のタスク：")
			+ std::to_string(_inProgressTasks.size() + _outTasks.size())
			+ ToString(u8"件 - 期限遵守率")
			+ std::to_string(
				static_cast<int>(  // (期限内) / (期限内 + 期限外) * 100.0f
					static_cast<float>(_inProgressTasks.size())
					/ (_inProgressTasks.size() + _outTasks.size())
					* 100.0f))
			+ ToString(u8" %"));
	}

#pragma region 色付け
	uint32_t color{};
	if (_outTasks.size() > 0)
		color = COLOR_WARN;  // 警告
	else if (_inProgressTasks.size() > 0)
		color = COLOR_NORM;  // 普通
	else
		color = COLOR_FREE;  // 安心

	embed.set_color(color);
#pragma endregion

#pragma region ユーザデータ表示フィールド
	if (_userContent.money < 0)  // 科料を負っているなら
	{
		embed.add_field(
			ToString(u8"🥴科料：") + std::to_string(_userContent.money) + ToString(u8"円 ↓"),
			ToString(u8"期限内にタスクを消化してプラスにしましょう。"),
			true);
	}
	else if (_userContent.money > 0) // 0よりプラスなら
	{
		embed.add_field(
			ToString(u8"🙂取得金額：") + std::to_string(_userContent.money) + ToString(u8"円 ↑"),
			ToString(u8"もっとお金を得ましょう。"),
			true);
	}

	embed.add_field(
		ToString(u8"🔥消化タスク数：") + std::to_string(_userContent.completedTotalCount) + ToString(u8"個"),
		ToString(u8"もっとタスクを消化しましょう。"),
		true);

	embed.add_field(
		ToString(u8"💸消化タスク総額：") + std::to_string(_userContent.completedTotalPrice) + ToString(u8"円相当"),
		ToString(u8"経済効果ありますね。"),
		true);
#pragma endregion

#pragma region 各タスク数フィールド
	bool hasTask{ false };

	if (_outTasks.size() > 0)
	{
		hasTask = true;
		embed.add_field(
			ToString(u8"❌期限超過：") + std::to_string(_outTasks.size()) + ToString(u8"件"),
			ToString(u8"まずいです。優先して消化して下さい。"),
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
