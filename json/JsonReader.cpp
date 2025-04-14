#include "JsonReader.h"
#include <fstream>
#include <cassert>

using json = nlohmann::json;

bool JsonReader::TryLoad()
{
	std::ifstream fs{};
	fs.open(fileName_, std::ios::in);

	if (!fs)
	{
		return false;
	}

	if (fs.is_open() == false)
	{
		return false;  // 失敗
	}

	fs >> data_;

	fs.close();

	return true;
}

bool JsonReader::TrySave()
{
	assert(isReadonly_ == false  // 書き込み可能を確認
		&& "読み取り専用が指定されているよ");

	{  // 排他制御
		std::lock_guard<std::mutex> lock{ writeMutex_ };

		std::ofstream fs{};
		fs.open(fileName_, std::ios::out);

		if (fs.is_open() == false)
		{
			return false;  // 失敗
		}

		fs << data_.dump() << std::endl;

		fs.close();
	}

	return true;
}

JsonReader::JsonReader(const std::string& _fileName, const bool& _readonly) :
	fileName_{ _fileName },
	isReadonly_{ _readonly }
{
	if (TryLoad() == false)
	{
		assert(isReadonly_ == false  // 読み取り専用ではないため空っぽ作成
			&& "jsonが読み込めなかったよ！読み取り専用なのに");

		data_ = "{}"_json;
		TrySave();
	}
}

JsonReader::~JsonReader()
{
	if (isReadonly_ == false)
	{
		TrySave();  // 書き込めるならセーブして終了
	}
}
