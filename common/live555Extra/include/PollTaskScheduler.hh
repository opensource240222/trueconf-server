#ifndef _POLL_TASK_SCHEDULER_HH
#define _POLL_TASK_SCHEDULER_HH

#include "timeval_helper.hh"

#include <UsageEnvironment.hh>

#if defined(__WIN32__) || defined(_WIN32)
#include <WinSock2.h>
#else
#include <poll.h>
#endif

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <thread>
#include <utility>
#include <vector>

#include "std-generic/compat/condition_variable.h"

class PollTaskScheduler : public TaskScheduler
{
public:
	static PollTaskScheduler* createNew(unsigned maxWaitTimeMilliseconds = 10);
	virtual ~PollTaskScheduler();

protected:
	PollTaskScheduler(unsigned maxWaitTimeMilliseconds);

	virtual void doEventLoop(char* watchVariable = NULL);
	virtual void singleStep();

	virtual TaskToken scheduleDelayedTask(int64_t microseconds, TaskFunc* proc, void* clientData);
	virtual void unscheduleDelayedTask(TaskToken& prevTask);

	virtual void setBackgroundHandling(int socketNum, int conditionSet, BackgroundHandlerProc* handlerProc, void* clientData);
	virtual void moveSocketHandling(int oldSocketNum, int newSocketNum);

	virtual EventTriggerId createEventTrigger(TaskFunc* eventHandlerProc);
	virtual void deleteEventTrigger(EventTriggerId eventTriggerId);

	virtual void triggerEvent(EventTriggerId eventTriggerId, void* clientData = NULL);


	bool tryHandleIO();
	bool tryHandleTimer(struct timeval now);
	void handleEvents();
	struct timeval getWaitTime(struct timeval now) const;
	void flushCanceledDelayedTasks();

private:
	void pollThreadFunc();

protected:
	struct timer_handler
	{
		TaskFunc* handler;
		void* clientData;

		timer_handler(
			TaskFunc* handler_ = 0,
			void* clientData_ = 0
		)
			: handler(handler_)
			, clientData(clientData_)
		{}

		void operator()() const;
	};

	struct delayed_task
	{
		TaskToken token;
		struct timeval time;
		timer_handler handler;

		delayed_task(TaskToken token_,
			struct timeval time_,
			TaskFunc* handler_,
			void* clientData_
		)
			: token(token_)
			, time(time_)
			, handler(handler_, clientData_)
		{}

		struct priority_less
		{
			bool operator()(const delayed_task& lhs, const delayed_task& rhs) const;
		};
	};

	struct io_handler
	{
		BackgroundHandlerProc* handler;
		void* clientData;

		io_handler(
			BackgroundHandlerProc* handler_ = 0,
			void* clientData_ = 0
		)
			: handler(handler_)
			, clientData(clientData_)
		{}

		void operator()(int conditionSet) const;
	};

	unsigned fMaxWaitTimeMilliseconds;

	TaskToken fNextDelayedTaskToken;
	std::priority_queue<
		delayed_task,
		std::vector<delayed_task>,
		delayed_task::priority_less
	> fDelayedTasks;
	std::set<TaskToken> fCanceledDelayedTasks;

	int fLastHandledSocket;
	std::mutex fPFDsMutex;
	vs::condition_variable fPFDsCV;
	int fNumSocketsReady;
	std::vector<pollfd> fPFDs;
	std::atomic_flag fPollThreadShouldRun;
	std::unique_ptr<std::thread> fPollThread;
	std::map<int, io_handler> fIOHandlers;

	EventTriggerId fNextEventId;
	std::map<EventTriggerId, TaskFunc*> fEventHandlers;
	std::mutex fEventsMutex;
	typedef std::vector<std::pair<EventTriggerId, void*>> events_t;
	events_t fEvents;
};

#endif
