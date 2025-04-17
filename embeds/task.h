#pragma once
#include <list>
#include <dpp/dpp.h>
#include "../Task.h"
#include <string>
#include "json/JsonReader.h"

namespace GenerateEmbed
{
	/// <summary>
	/// �^�X�N�̊T�vembed�𐶐�����
	/// </summary>
	/// <param name="_inProgressTasks">�����ȃ^�X�N�����̃|�C���^</param>
	/// <param name="_unassignedTasks">�����l�i�V�̃^�X�N�����̃|�C���^</param>
	/// <param name="_outTasks">�������߂����^�X�N�����̃|�C���^</param>
	/// <param name="_income">������z</param>
	/// <param name="_totalCost">�������^�X�N�̑��z</param>
	/// <returns>embed</returns>
	dpp::embed About(
		const std::list<Task::TaskContent*>& _inProgressTasks,
		const std::list<Task::TaskContent*>& _unassignedTasks,
		const std::list<Task::TaskContent*>& _outTasks,
		const int64_t& _income,
		const int64_t& _totalCost);

	/// <summary>
	/// �^�X�N������ɕ��ނ���embed�𐶐�����
	/// </summary>
	/// <param name="_pTask">�^�X�N�̃|�C���^</param>
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
