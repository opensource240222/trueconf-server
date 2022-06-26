#include "Live555Thread.hh"
#include "std/cpplib/ThreadUtils.h"

#include <cassert>
#include <functional>

Live555Thread::Live555Thread(UsageEnvironment* env)
	: m_terminate_requested(false)
	, m_acquire_count(0)
	, m_env(env)
	, m_event_loop_watch(0)
	, m_thread(std::bind(&Live555Thread::thread_func, this))
{
}

Live555Thread::~Live555Thread()
{
	if (m_thread.joinable())
		join();
}

void Live555Thread::acquire(std::unique_lock<std::mutex>& lock)
{
	assert(lock.mutex() == &m_mutex);
	assert(lock.owns_lock());
	if (m_active_thread_id != std::this_thread::get_id())
	{
		while (m_active_thread_id != std::thread::id())
		{
			m_event_loop_watch = 1;
			m_cv.wait(lock);
		}
		m_active_thread_id = std::this_thread::get_id();
	}
	++m_acquire_count;
}

void Live555Thread::release()
{
	assert(m_active_thread_id == std::this_thread::get_id());
	--m_acquire_count;
	if (m_acquire_count == 0)
	{
		m_active_thread_id = std::thread::id();
		m_cv.notify_all();
	}
}

void Live555Thread::pause()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	acquire(lock);
}

void Live555Thread::resume()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	release();
}

void Live555Thread::join()
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_terminate_requested = true;
		m_event_loop_watch = 1;
	}
	m_cv.notify_all();
	m_thread.join();
}

void Live555Thread::thread_func()
{
	vs::SetThreadName("Live555");

	std::unique_lock<std::mutex> lock(m_mutex);
	while (1)
	{
		acquire(lock);
		if (m_terminate_requested)
			break;
		m_event_loop_watch = 0;
		lock.unlock();
		m_env->taskScheduler().doEventLoop(const_cast<char*>(&m_event_loop_watch));
		lock.lock();
		release();
		assert(m_acquire_count == 0);
		if (m_terminate_requested)
			break;
		m_cv.wait(lock);
	}
}
