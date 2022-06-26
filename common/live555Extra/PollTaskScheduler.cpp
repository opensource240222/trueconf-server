#include "PollTaskScheduler.hh"
#include "timeval_helper.hh"
#include "std/cpplib/ThreadUtils.h"

#include <GroupsockHelper.hh>

#include <algorithm>
#include <cassert>
#include "std-generic/cpplib/ThreadUtils.h"

#if defined(__WIN32__) || defined(_WIN32)
#define poll WSAPoll
#endif

template <class T1, class T2>
inline timeval make_timeval(T1 v1, T2 v2)
{
	return { static_cast<decltype(timeval::tv_sec)>(v1), static_cast<decltype(timeval::tv_usec)>(v2) };
}

template <class T1, class T2, class T3>
inline pollfd make_pollfd(T1 v1, T2 v2, T3 v3)
{
	return { static_cast<decltype(pollfd::fd)>(v1), static_cast<decltype(pollfd::events)>(v2), static_cast<decltype(pollfd::revents)>(v3) };
}

inline bool pollfd_fd_less(const pollfd& pfd, int fd)
{
	return pfd.fd < fd;
}

inline bool pollfd_ready(const pollfd& pfd)
{
	return pfd.revents != 0;
}

inline short conditionSet2events(int conditionSet)
{
	short events(0);
	if (conditionSet & SOCKET_READABLE)
		events |= POLLIN;
	if (conditionSet & SOCKET_WRITABLE)
		events |= POLLOUT;
	// Ignoring SOCKET_EXCEPTION because there is no way to stop poll() from
	// notifying about socket errors.
	//if (conditionSet & SOCKET_EXCEPTION)
	//	events |= POLLERR | POLLHUP | POLLNVAL;
	return events;
}

inline int events2conditionSet(short events)
{
	int conditionSet(0);
	if (events & POLLIN)
		conditionSet |= SOCKET_READABLE;
	if (events & POLLOUT)
		conditionSet |= SOCKET_WRITABLE;
	if (events & (POLLERR | POLLHUP | POLLNVAL))
		conditionSet |= SOCKET_EXCEPTION;
	return conditionSet;
}

void PollTaskScheduler::timer_handler::operator()() const
{
	if (handler)
		handler(clientData);
}

bool PollTaskScheduler::delayed_task::priority_less::operator()(const PollTaskScheduler::delayed_task& lhs, const PollTaskScheduler::delayed_task& rhs) const
{
	return lhs.time > rhs.time;
}

void PollTaskScheduler::io_handler::operator()(int conditionSet) const
{
	if (handler)
		handler(clientData, conditionSet);
}

PollTaskScheduler* PollTaskScheduler::createNew(unsigned maxWaitTimeMilliseconds)
{
	return new PollTaskScheduler(maxWaitTimeMilliseconds);
}

PollTaskScheduler::PollTaskScheduler(unsigned maxWaitTimeMilliseconds)
	: fMaxWaitTimeMilliseconds(maxWaitTimeMilliseconds)
	, fNextDelayedTaskToken(reinterpret_cast<TaskToken>(1))
	, fLastHandledSocket(-1)
	, fNumSocketsReady(0)
	, fNextEventId(1)
{
	fPollThreadShouldRun.test_and_set();
	fPollThread.reset(new std::thread(&PollTaskScheduler::pollThreadFunc, this));
}

PollTaskScheduler::~PollTaskScheduler()
{
	fPollThreadShouldRun.clear();
	fPFDsCV.notify_all();
	fPollThread->join();
}

void PollTaskScheduler::doEventLoop(char* watchVariable)
{
	while (watchVariable == NULL || *watchVariable == 0)
		singleStep();
}

void PollTaskScheduler::singleStep()
{
	if (tryHandleIO())
	{
		handleEvents();
		return;
	}

	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval delay;
	delay = getWaitTime(now);
	vs::SleepFor(std::chrono::microseconds(delay.tv_sec * 1000000 + delay.tv_usec));
	now += delay;

	if (tryHandleTimer(now))
	{
		handleEvents();
		return;
	}

	handleEvents();
}

bool PollTaskScheduler::tryHandleIO()
{
	std::unique_lock<std::mutex> lock(fPFDsMutex, std::try_to_lock);
	if (!lock.owns_lock())
		return false;
	if (fPFDs.empty())
		return false;

	if (fNumSocketsReady <= 0)
	{
		fPFDsCV.notify_all();
		return false;
	}

	auto toHandle = fPFDs.end();

	// To avoid starving sockets prefer first socket with id greater then last handled socket
	if (fLastHandledSocket != -1)
		toHandle = std::find_if(
			std::lower_bound(fPFDs.begin(), fPFDs.end(), fLastHandledSocket, pollfd_fd_less),
			fPFDs.end(),
			pollfd_ready
		);
	// ...otherwise select first ready socket
	if (toHandle == fPFDs.end())
		toHandle = std::find_if(fPFDs.begin(), fPFDs.end(), pollfd_ready);

	if (toHandle == fPFDs.end())
	{
		fLastHandledSocket = -1;
		// In case something went wrong and there are no sockets to handle events on but fNumSocketsReady>0, reset it so we will poll again next time.
		fNumSocketsReady = 0;
		fPFDsCV.notify_all();
		return false;
	}

	const int fd = toHandle->fd;
	const short revents = toHandle->revents;

	fLastHandledSocket = fd;
	toHandle->revents = 0;
	--fNumSocketsReady;
	if (fNumSocketsReady == 0)
		fPFDsCV.notify_all();

	lock.unlock();
	fIOHandlers[fd](events2conditionSet(revents));
	return true;
}

bool PollTaskScheduler::tryHandleTimer(struct timeval now)
{
	flushCanceledDelayedTasks();
	if (fDelayedTasks.empty())
		return false;
	if (now < fDelayedTasks.top().time)
		return false;

	timer_handler handler(fDelayedTasks.top().handler);
	fDelayedTasks.pop();
	handler();
	return true;
}

void PollTaskScheduler::handleEvents()
{
	events_t eventsCopy;
	{
		std::lock_guard<std::mutex> eventsLock(fEventsMutex);
		eventsCopy.swap(fEvents);
	}

	for (auto& event: eventsCopy)
	{
		auto eventHandlerIt = fEventHandlers.find(event.first);
		if (eventHandlerIt != fEventHandlers.end())
			eventHandlerIt->second(event.second);
	}
}

struct timeval PollTaskScheduler::getWaitTime(struct timeval now) const
{
	if (fDelayedTasks.empty())
		return make_timeval(fMaxWaitTimeMilliseconds/1000, (fMaxWaitTimeMilliseconds%1000)*1000);

	return std::min(
		std::max(fDelayedTasks.top().time-now, timeval {0, 0}),
		make_timeval(fMaxWaitTimeMilliseconds/1000, (fMaxWaitTimeMilliseconds%1000)*1000)
	);
}

void PollTaskScheduler::flushCanceledDelayedTasks()
{
	while (!fDelayedTasks.empty())
	{
		auto it = fCanceledDelayedTasks.find(fDelayedTasks.top().token);
		if (it == fCanceledDelayedTasks.end())
			break;

		fCanceledDelayedTasks.erase(it);
		fDelayedTasks.pop();
	}
}

void PollTaskScheduler::pollThreadFunc()
{
	vs::SetThreadName("Live555_Poll");

	std::unique_lock<std::mutex> lock(fPFDsMutex);
	while (fPollThreadShouldRun.test_and_set(std::memory_order_consume))
	{
		while (fPFDsCV.wait_for(lock, std::chrono::milliseconds(fMaxWaitTimeMilliseconds)) == std::cv_status::timeout)
			if (!fPollThreadShouldRun.test_and_set(std::memory_order_consume))
				return;

		if (fPFDs.empty())
		{
			fNumSocketsReady = 0;
			continue;
		}
		fNumSocketsReady = poll(fPFDs.data(), fPFDs.size(), fMaxWaitTimeMilliseconds);
		if (fNumSocketsReady < 0)
		{
#if defined(__WIN32__) || defined(_WIN32)
			const int err = WSAGetLastError();
			if (err != WSAEINTR && err != WSAEWOULDBLOCK)
#else
			if (errno != EINTR && errno != EAGAIN)
#endif
				internalError();
		}
	}
}

TaskToken PollTaskScheduler::scheduleDelayedTask(int64_t microseconds, TaskFunc* proc, void* clientData)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	const TaskToken token(fNextDelayedTaskToken);
	fNextDelayedTaskToken = reinterpret_cast<TaskToken>(reinterpret_cast<intptr_t>(fNextDelayedTaskToken)+1);
	if (fNextDelayedTaskToken == 0)
		fNextDelayedTaskToken = reinterpret_cast<TaskToken>(1);
	fDelayedTasks.emplace(token, now+std::chrono::microseconds(microseconds), proc, clientData);
	return token;
}

void PollTaskScheduler::unscheduleDelayedTask(TaskToken& prevTask)
{
	fCanceledDelayedTasks.insert(prevTask);
	prevTask = 0;
}

void PollTaskScheduler::setBackgroundHandling(int socketNum, int conditionSet, BackgroundHandlerProc* handlerProc, void* clientData)
{
	if (socketNum < 0)
		return;

	std::lock_guard<std::mutex> lock(fPFDsMutex);

	auto it = std::lower_bound(fPFDs.begin(), fPFDs.end(), socketNum, pollfd_fd_less);

	if (conditionSet == 0)
	{
		if (it != fPFDs.end() && it->fd == socketNum)
		{
			if (pollfd_ready(*it))
				--fNumSocketsReady;
			fPFDs.erase(it);
		}
		fIOHandlers.erase(socketNum);
	}
	else
	{
		if (it != fPFDs.end() && it->fd == socketNum)
			it->events = conditionSet2events(conditionSet);
		else
			fPFDs.insert(it, make_pollfd(socketNum, conditionSet2events(conditionSet), 0));
		fIOHandlers[socketNum] = io_handler(handlerProc, clientData);
	}
}

void PollTaskScheduler::moveSocketHandling(int oldSocketNum, int newSocketNum)
{
	if (oldSocketNum < 0 || newSocketNum < 0 || oldSocketNum == newSocketNum)
		return;

	std::lock_guard<std::mutex> lock(fPFDsMutex);

	auto oldit = std::lower_bound(fPFDs.begin(), fPFDs.end(), oldSocketNum, pollfd_fd_less);
	if (oldit != fPFDs.end() && oldit->fd == oldSocketNum)
	{
		auto newit = std::lower_bound(fPFDs.begin(), fPFDs.end(), newSocketNum, pollfd_fd_less);
		if (newit != fPFDs.end() && newit->fd == newSocketNum)
		{
			newit->events = oldit->events;
			newit->revents = 0;
		}
		else
			newit = fPFDs.insert(newit, make_pollfd(newSocketNum, oldit->events, 0));

		if (pollfd_ready(*oldit))
			--fNumSocketsReady;
		fPFDs.erase(oldit);
	}
	fIOHandlers[newSocketNum] = fIOHandlers[oldSocketNum];
	fIOHandlers.erase(oldSocketNum);
}

EventTriggerId PollTaskScheduler::createEventTrigger(TaskFunc* eventHandlerProc)
{
	bool inserted = fEventHandlers.emplace(fNextEventId, eventHandlerProc).second;
	assert(inserted);
	++fNextEventId;
	return fNextEventId-1;
}

void PollTaskScheduler::deleteEventTrigger(EventTriggerId eventTriggerId)
{
	fEventHandlers.erase(eventTriggerId);
}

void PollTaskScheduler::triggerEvent(EventTriggerId eventTriggerId, void* clientData)
{
	std::lock_guard<std::mutex> eventsLock(fEventsMutex);
	fEvents.emplace_back(eventTriggerId, clientData);
}
