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
	/// 試しに書き込む
	/// </summary>
	/// <returns>書き込みに成功 true / false</returns>
	bool TrySave();

private:
	/// <summary>
	/// 試しに読み込む
	/// </summary>
	/// <returns>読み込みに成功 true / false</returns>
	bool TryLoad();

private:
	std::mutex writeMutex_;  // 書き込みの排他制御用ミューテックス
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
