#pragma once
#include <ctime>
#include <functional>
#include <thread>

class Alarm
{
public:
	enum struct CallType
	{
		Getup,  // ãNè∞
	};

	using OnAlarmFunc = std::function<void(Alarm::CallType)>;

public:
	Alarm(const std::time_t& _lastAlarmTime);
	~Alarm();

	void Run();
	void Join();

	void OnAlarm(OnAlarmFunc _callback);

private:
	std::thread timeThread_;
	OnAlarmFunc onAlarmFunc_;
	std::time_t lastAlarmTime_;

	std::time_t nextOnGetup_;  // ãNè∞éûçè
};
