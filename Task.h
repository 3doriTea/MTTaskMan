#pragma once
#include <list>

namespace Task
{
	/// <summary>
	/// �^�X�N�̓��e
	/// </summary>
	struct TaskContent
	{
		TaskContent(
			int64_t _id,
			std::string _name,
			time_t _deadline,
			int32_t _price,
			std::string _description,
			dpp::snowflake _author,
			std::vector<dpp::snowflake> _undertakers) :
			id{ _id },
			name{ _name },
			deadline{ _deadline },
			price{ _price },
			description{ _description },
			author{ _author },
			undertakers{ _undertakers }
		{}

		TaskContent() :
			TaskContent{ -1LL, "", 0, 0, "", 0, {} }
		{}

		int64_t id;                               // �ŗL���ʎq
		std::string name;                         // ���O
		time_t deadline;                          // ����
		int32_t price;                            // �l�i
		std::string description;                  // ����
		dpp::snowflake author;                    // ���s��
		std::vector<dpp::snowflake> undertakers;  // �����l����
	};

	static inline void to_json(nlohmann::json& j, const TaskContent& taskContent)
	{
		j = nlohmann::json
		{
			{ "id", taskContent.id },
			{ "name", taskContent.name },
			{ "deadline", taskContent.deadline },
			{ "price", taskContent.price },
			{ "description", taskContent.description },
			{ "author", taskContent.author },
			{ "undertakers", taskContent.undertakers }
		};
	}

	static inline void from_json(const nlohmann::json& j, TaskContent& taskContent)
	{
		j.at("id").get_to(taskContent.id);
		j.at("name").get_to(taskContent.name);
		j.at("deadline").get_to(taskContent.deadline);
		j.at("price").get_to(taskContent.price);
		j.at("description").get_to(taskContent.description);
		taskContent.author = j.at("author").get<std::string>();

		taskContent.undertakers.clear();
		for (auto& undertaker : j.at("undertakers"))
		{
			taskContent.undertakers.push_back(undertaker.get<std::string>());
		}
	}

	// �~���œK�؂ȏꏊ�ɑ}�����郉���_��
	static inline void InsertListDesc(std::list<TaskContent*>& contentList, TaskContent* content)
	{
		for (auto&& itr = contentList.begin(); itr != contentList.end(); itr++)
		{
			if ((*itr)->deadline > content->deadline)
			{
				contentList.insert(itr, content);
				return;
			}
		}

		contentList.push_back(content);  // �q�b�g���Ȃ���΍Ō���ɒǉ�
	}
}
