#pragma once
#include <chrono>

enum class REFRESHER : unsigned int;

struct TimerExtention
{
	std::chrono::steady_clock::time_point lastUpdate;
	bool IsUpdating;
	std::chrono::steady_clock::duration refreshPeriod;
	REFRESHER refresher;
};