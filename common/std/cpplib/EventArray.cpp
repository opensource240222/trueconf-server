#include "EventArray.h"

#include <cassert>

namespace
{
const uint32_t EVENTS_MAX = 64;
const uint64_t ROLL_ME = 1;
}

namespace vs
{

EventArray::EventArray(uint32_t eventCount, bool manualReset):
	events_(0),
	eventCount_(eventCount),
	lastEvent_(eventCount - 1),
	wakeMeUp_(manualReset),
	timeToDie_(false),
	manualReset_(manualReset),
	state_(as_idle)
{
	if (eventCount > EVENTS_MAX || eventCount == 0)
		throw std::invalid_argument("EventArray: can't handle more than 64 events!");
	eventsMask_ = 1;
	for (uint32_t i = 1; i < eventCount; ++i)
		eventsMask_ = (eventsMask_ << 1) + 1;
}

void EventArray::set(uint32_t index)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (index > eventCount_)
		return;
	events_ |= ROLL_ME << index;
	if (state_ == as_waitForOne ||
		(state_ == as_waitForAll && everyoneIsTriggered()))
		wakeMeUp_.set();
}

void EventArray::kill_listener()
{
	std::lock_guard<std::mutex> lock(mutex_);
	timeToDie_ = true;
	wakeMeUp_.set();
}

void EventArray::reset(uint32_t index)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (index > eventCount_)
		return;
	events_ &= ~(ROLL_ME << index);
	if (!events_)
		wakeMeUp_.reset();
}

void EventArray::reset()
{
	std::lock_guard<std::mutex> lock(mutex_);
	events_ = 0;
	lastEvent_ = eventCount_ - 1;
	timeToDie_ = false;
	wakeMeUp_.reset();
	state_ = as_idle;
}

uint32_t EventArray::wait(bool waitForAll)
{
	uint32_t preWaitResult;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		preWaitResult = preWait(waitForAll);
	}
	if (preWaitResult != wait_result::no_event ||
		preWaitResult == wait_result::time_to_die)
		return preWaitResult;
	wakeMeUp_.wait();
	return postWait(waitForAll, true);
}

uint32_t EventArray::try_wait(bool waitForAll)
{
	std::lock_guard<std::mutex> lock(mutex_);
	uint32_t result = preWait(waitForAll);
	state_ = as_idle;
	return result;
}

uint32_t EventArray::findFirstTriggeredEvent()
{
	for (uint32_t event = lastEvent_ + 1; event < eventCount_; ++event)
		if (events_ & ROLL_ME << event)
			return lastEvent_ = event;
	for (uint32_t event = 0; event < lastEvent_ + 1; ++event)
		if (events_ & ROLL_ME << event)
			return lastEvent_ = event;
	return wait_result::no_event;
}

bool EventArray::everyoneIsTriggered()
{
	return (events_ & eventsMask_) == eventsMask_;
}

uint32_t EventArray::preWait(bool waitForAll)
{
	if (timeToDie_)
		return wait_result::time_to_die;
	if (events_)
	{
		if (!waitForAll)
		{
			uint32_t index = findFirstTriggeredEvent();
			assert(index != wait_result::no_event);
			if (!manualReset_)
				events_ &= ~(ROLL_ME << index);
			return index;
		}
		else if (everyoneIsTriggered())
		{
			if (!manualReset_)
				events_ = 0;
			return 0;
		}
	}
	if (waitForAll)
		state_ = as_waitForAll;
	else
		state_ = as_waitForOne;
//	We REALLY don't want to wake up from wait() with no events up. But if someone triggers something
//	between preWait() and wait() it's fine.
	wakeMeUp_.reset();
	return wait_result::no_event;
}

uint32_t EventArray::postWait(bool waitForAll, bool result)
{
	std::lock_guard<std::mutex> lock(mutex_);
	state_ = as_idle;
	if (timeToDie_)
		return wait_result::time_to_die;

	if (!result)
		return wait_result::timeout;

	if (waitForAll)
	{
		if (!manualReset_)
			events_ = 0;
		return 0;
	} else
	{
		uint32_t index = findFirstTriggeredEvent();
		if (!manualReset_ && index != wait_result::no_event)
			events_ &= ~(ROLL_ME << index);
		return index;
	}
}

}
