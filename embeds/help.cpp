#include "help.h"
#include <string>
#include "json/JsonReader.h"
#include "Utility.h"

dpp::embed GenerateEmbed::Help(JsonReader* tasks)
{
	using dpp::utility::slashcommand_mention;
	return dpp::embed()
		.set_color(0x00ff7f)
		.set_title(ToString(u8"🔖 ヘルプ 🔖"))
		.set_description(ToString(u8"タスクを管理するために使う最低限のコマンドを説明します。"))
		.set_thumbnail("https://raw.githubusercontent.com/free-icons/free-icons/5a6047cf7efd6a99ad6db6ad4d33ec9d34af429e/svgs/thin-cloud-question.svg")
		.add_field(
			slashcommand_mention(tasks->Json()["command-ids"]["tasks"].get<std::string>(), "tasks"),
			ToString(u8"発行されているタスクを確認できます。")
		)
		.add_field(
			slashcommand_mention(tasks->Json()["command-ids"]["dotask"].get<std::string>(), "dotask"),
			ToString(u8"タスクを選択して引き受けることができます。")
		)
		.add_field(
			slashcommand_mention(tasks->Json()["command-ids"]["comptask"].get<std::string>(), "comptask"),
			ToString(u8"引き受けたタスクを選択して、完了することができます。")
		)
		.add_field(
			slashcommand_mention(tasks->Json()["command-ids"]["newtask"].get<std::string>(), "newtask"),
			ToString(u8"タスクを発行することができます。")
		)
		.set_footer(
			dpp::embed_footer()
				.set_text(ToString(u8"他困ったことは制作者まで！"))
		);
}
