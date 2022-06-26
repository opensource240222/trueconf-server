#ifndef _LIVE555_THREAD_HH
#define _LIVE555_THREAD_HH

#include <UsageEnvironment.hh>

#include <mutex>
#include <thread>

#include "std-generic/compat/condition_variable.h"

class Live555Thread
{
public:
	Live555Thread(UsageEnvironment* env);
	~Live555Thread();

	void pause();
	void resume();
	void join();

	class PauseGuard
	{
	public:
		explicit PauseGuard(Live555Thread& thread)
			: m_thread(thread)
		{
			m_thread.pause();
		}
		~PauseGuard()
		{
			m_thread.resume();
		}

	private:
		Live555Thread& m_thread;

		PauseGuard(const PauseGuard&) = delete;
		PauseGuard(PauseGuard&&) = delete;
		PauseGuard& operator=(const PauseGuard&) = delete;
		PauseGuard& operator=(PauseGuard&&) = delete;
	};

private:
	void thread_func();
	void acquire(std::unique_lock<std::mutex>& lock);
	void release();

private:
	std::mutex m_mutex;
	vs::condition_variable m_cv;
	std::thread::id m_active_thread_id;
	bool m_terminate_requested;
	unsigned m_acquire_count;

	UsageEnvironment* m_env;
	volatile char m_event_loop_watch;

	std::thread m_thread;
};

#endif
