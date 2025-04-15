#pragma once
#include <list>
#include <dpp/dpp.h>
#include "../Task.h"
#include <string>

namespace GenerateEmbed
{
	dpp::embed About(
		const std::list<Task::TaskContent*>& _inProgressTasks,
		const std::list<Task::TaskContent*>& _unassignedTasks,
		const std::list<Task::TaskContent*>& _outTasks,
		const int64_t& _income,
		const int64_t& _totalCost);

	dpp::embed TaskOut(
		const Task::TaskContent* _pTask,
		const dpp::user* _author);

	dpp::embed TaskUnassigned(
		const Task::TaskContent* _pTask,
		const dpp::user* _author);
	
	dpp::embed TaskInProgress(
		const Task::TaskContent* _pTask,
		const dpp::user* _author);
}
