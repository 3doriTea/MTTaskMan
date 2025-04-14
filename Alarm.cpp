#include "Alarm.h"
#include <chrono>
#include <iostream>

Alarm::Alarm(const std::time_t& _lastAlarmTime) :
	onAlarmFunc_{},
	lastAlarmTime_{ _lastAlarmTime }
{
	//nextOnGetup_ = 
}

Alarm::~Alarm()
{
}

void Alarm::Run()
{
	using std::cout;
	using std::endl;

	timeThread_ = std::thread
	{
		[&]()
		{
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				cout << "." << endl;

				std::time_t now{ std::time(nullptr) };
				if (now >= nextOnGetup_)
				{
					onAlarmFunc_(CallType::Getup);

					std::time_t local{ now };

					//local += 60ULL * 24ULL;  // 1ì˙â¡éZ
					local += 60ULL;  // 1ï™â¡éZ
					local -= local % 60;  // ïbêîÇÕñ≥éã

					nextOnGetup_ = local;
				}
			}
		}
	};
}

void Alarm::Join()
{
	timeThread_.join();
}

void Alarm::OnAlarm(OnAlarmFunc _callback)
{
	onAlarmFunc_ = _callback;
}
