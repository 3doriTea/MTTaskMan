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
		return false;  // ���s
	}

	fs >> data_;

	fs.close();

	return true;
}

bool JsonReader::TrySave()
{
	assert(isReadonly_ == false  // �������݉\���m�F
		&& "�ǂݎ���p���w�肳��Ă����");

	{  // �r������
		std::lock_guard<std::mutex> lock{ writeMutex_ };

		std::ofstream fs{};
		fs.open(fileName_, std::ios::out);

		if (fs.is_open() == false)
		{
			return false;  // ���s
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
		assert(isReadonly_ == false  // �ǂݎ���p�ł͂Ȃ����ߋ���ۍ쐬
			&& "json���ǂݍ��߂Ȃ�������I�ǂݎ���p�Ȃ̂�");

		data_ = "{}"_json;
		TrySave();
	}
}

JsonReader::~JsonReader()
{
	if (isReadonly_ == false)
	{
		TrySave();  // �������߂�Ȃ�Z�[�u���ďI��
	}
}
