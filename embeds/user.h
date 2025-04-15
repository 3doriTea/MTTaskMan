#pragma once
#include <dpp/dpp.h>
#include "../Task.h"
#include "UserData.h"

namespace GenerateEmbed
{
	dpp::embed UserInfo(
		const std::list<Task::TaskContent*>& _inProgressTasks,
		const std::list<Task::TaskContent*>& _outTasks,
		const UserData::UserContent& _userContent,
		const dpp::user* _pAuthor);
}