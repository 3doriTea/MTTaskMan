#pragma once
#include <dpp/json.h>
#include <cstdint>

namespace UserData
{
	struct UserContent
	{
		UserContent(
			bool _write,
			int64_t _money,
			int64_t _completedTotalCount,
			int64_t _completedTotalPrice) :
			write{ _write },
			money{ _money },
			completedTotalCount{ _completedTotalCount },
			completedTotalPrice{ _completedTotalPrice }
		{}

		UserContent() :
			UserContent{ false, 0LL, 0LL, 0LL }
		{}

		bool write;                   // �z���C�g���X�g�ɓo�^����Ă��邩
		int64_t money;                // ������
		int64_t completedTotalCount;  // �^�X�N�B����
		int64_t completedTotalPrice;  // �^�X�N�B�����z
	};

	static inline void to_json(nlohmann::json& j, const UserContent& content)
	{
		j = nlohmann::json
		{
			{ "write", content.write },
			{ "money", content.money },
			{ "completedTotalCount", content.completedTotalCount },
			{ "completedTotalPrice", content.completedTotalPrice }
		};
	}

	static inline void from_json(const nlohmann::json& j, UserContent& content)
	{
		j.at("write").get_to(content.write);
		j.at("money").get_to(content.money);
		j.at("completedTotalCount").get_to(content.completedTotalCount);
		j.at("completedTotalPrice").get_to(content.completedTotalPrice);
	}
}
