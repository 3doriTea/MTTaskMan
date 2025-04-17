#pragma once
#include <list>
#include <dpp/dpp.h>
#include "../Task.h"
#include <string>
#include "json/JsonReader.h"

namespace GenerateEmbed
{
	/// <summary>
	/// タスクの概要embedを生成する
	/// </summary>
	/// <param name="_inProgressTasks">順調なタスクたちのポインタ</param>
	/// <param name="_unassignedTasks">請負人ナシのタスクたちのポインタ</param>
	/// <param name="_outTasks">期限超過したタスクたちのポインタ</param>
	/// <param name="_income">上限金額</param>
	/// <param name="_totalCost">未消化タスクの総額</param>
	/// <returns>embed</returns>
	dpp::embed About(
		const std::list<Task::TaskContent*>& _inProgressTasks,
		const std::list<Task::TaskContent*>& _unassignedTasks,
		const std::list<Task::TaskContent*>& _outTasks,
		const int64_t& _income,
		const int64_t& _totalCost);

	/// <summary>
	/// タスクを勝手に分類してembedを生成する
	/// </summary>
	/// <param name="_pTask">タスクのポインタ</param>
	/// <returns>embed</returns>
	dpp::embed Task(
		const Task::TaskContent* _pTask,
		JsonReader* _pTasks);

	dpp::embed TaskOut(
		const Task::TaskContent* _pTask,
		const dpp::user* _author,
		JsonReader* _pTasks);

	dpp::embed TaskUnassigned(
		const Task::TaskContent* _pTask,
		const dpp::user* _author,
		JsonReader* _pTasks);
	
	dpp::embed TaskInProgress(
		const Task::TaskContent* _pTask,
		const dpp::user* _author,
		JsonReader* _pTasks);
}
