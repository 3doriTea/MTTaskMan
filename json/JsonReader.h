#pragma once
#include <string>
#include <dpp/json.h>
#include <mutex>

class JsonReader
{
public:
	JsonReader(const std::string& _fileName, const bool& _readonly = false);
	~JsonReader();

	inline nlohmann::json& Json() { return data_; }

	/// <summary>
	/// �����ɏ�������
	/// </summary>
	/// <returns>�������݂ɐ��� true / false</returns>
	bool TrySave();

private:
	/// <summary>
	/// �����ɓǂݍ���
	/// </summary>
	/// <returns>�ǂݍ��݂ɐ��� true / false</returns>
	bool TryLoad();

private:
	std::mutex writeMutex_;  // �������݂̔r������p�~���[�e�b�N�X
	bool isReadonly_;
	nlohmann::json data_;
	std::string fileName_;
};

//namespace JsonReader
//{
//	bool TryLoad(const std::string& _fileName, nlohmann::json& _data);
//
//	bool TrySave(const std::string& _fileName, nlohmann::json& _data);
//}
