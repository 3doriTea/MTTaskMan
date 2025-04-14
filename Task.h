#pragma once

namespace Task
{
	/// <summary>
	/// �^�X�N�̓��e
	/// </summary>
	struct TaskContent
	{
		TaskContent(
			std::string _name,
			time_t _deadline,
			int32_t _price,
			std::string _description,
			dpp::snowflake _author,
			std::vector<dpp::snowflake> _undertakers) :
			name{ _name },
			deadline{ _deadline },
			price{ _price },
			description{ _description },
			author{ _author },
			undertakers{ _undertakers }
		{}

		TaskContent() :
			TaskContent{ "", 0, 0, "", 0, {} }
		{}

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
}
