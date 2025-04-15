#pragma once
#include <dpp/json.h>
#include <cstdint>

namespace UserData
{
	struct UserContent
	{
		UserContent(
			bool _white,
			int64_t _money,
			int64_t _completedTotalCount,
			int64_t _completedTotalPrice) :
			white{ _white },
			money{ _money },
			completedTotalCount{ _completedTotalCount },
			completedTotalPrice{ _completedTotalPrice }
		{}

		UserContent() :
			UserContent{ false, 0LL, 0LL, 0LL }
		{}

		bool white;                   // �z���C�g���X�g�ɓo�^����Ă��邩
		int64_t money;                // ������
		int64_t completedTotalCount;  // �^�X�N�B����
		int64_t completedTotalPrice;  // �^�X�N�B�����z
	};

	static inline void to_json(nlohmann::json& j, const UserContent& content)
	{
		j = nlohmann::json
		{
			{ "white", content.white },
			{ "money", content.money },
			{ "completedTotalCount", content.completedTotalCount },
			{ "completedTotalPrice", content.completedTotalPrice }
		};
	}

	static inline void from_json(const nlohmann::json& j, UserContent& content)
	{
		j.at("white").get_to(content.white);

		if (j.find("money").value().is_null())
		{
			content.money = 0;  // json�Œl��null�Ȃ�0�����Ă���
		}
		else
		{
			j.at("money").get_to(content.money);
		}

		if (j.find("completedTotalCount").value().is_null())
		{
			content.completedTotalCount = 0;  // json�Œl��null�Ȃ�0�����Ă���
		}
		else
		{
			j.at("completedTotalCount").get_to(content.completedTotalCount);
		}

		if (j.find("completedTotalPrice").value().is_null())
		{
			content.completedTotalPrice = 0;  // json�Œl��null�Ȃ�0�����Ă���
		}
		else
		{
			j.at("completedTotalPrice").get_to(content.completedTotalPrice);
		}

	}
}
