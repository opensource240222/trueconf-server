#pragma once

#include <mutex>
#include <vector>

#include "std/cpplib/event.h"

namespace vs
{

namespace wait_result
{
const uint32_t no_event =      0xffffffff;
const uint32_t timeout =       0xfffffffe;
const uint32_t time_to_die =   0xfffffffd;
}

class EventArray
{
public:
	EventArray(uint32_t eventCount, bool manualReset);
	void set(uint32_t index);
	void kill_listener();
	void reset(uint32_t index);
	void reset();
	uint32_t wait(bool waitForAll);
	uint32_t try_wait(bool waitForAll);
	template <class Rep, class Period>
	uint32_t wait_for(const std::chrono::duration<Rep, Period>& rel_time, bool waitForAll);
	template <class Clock, class Duration>
	uint32_t wait_until(const std::chrono::time_point<Clock, Duration>& abs_time, bool waitForAll);
private:
	enum state
	{
		as_idle,
		as_waitForOne,
		as_waitForAll,
	};

	bool everyoneIsTriggered();
	uint32_t findFirstTriggeredEvent();
	uint32_t preWait(bool waitForAll);
	uint32_t postWait(bool waitForAll, bool waitResult);

	uint64_t events_;
	uint64_t eventsMask_;
	uint32_t eventCount_;
	uint32_t lastEvent_;
	vs::event wakeMeUp_;// inside
	bool timeToDie_;
	bool manualReset_;
	state state_;

	std::mutex mutex_;
};

template <class Rep, class Period>
uint32_t EventArray::wait_for(const std::chrono::duration<Rep, Period>& rel_time, bool waitForAll)
{
	uint32_t preWaitResult;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		preWaitResult = preWait(waitForAll);
	}
	if (preWaitResult != wait_result::no_event ||// I dont like this 'if' too
		preWaitResult == wait_result::time_to_die)
		return preWaitResult;
	bool result = wakeMeUp_.wait_for(rel_time);
	return postWait(waitForAll, result);
}

template <class Clock, class Duration>
uint32_t EventArray::wait_until(const std::chrono::time_point<Clock, Duration>& abs_time, bool waitForAll)
{
	uint32_t preWaitResult;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		preWaitResult = preWait(waitForAll);
	}
	if (preWaitResult != wait_result::no_event ||
		preWaitResult == wait_result::time_to_die)
		return preWaitResult;
	bool result = wakeMeUp_.wait_until(abs_time);
	return postWait(waitForAll, result);
}


}
