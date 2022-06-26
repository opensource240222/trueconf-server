#include "PrimitiveTimer.h"
#include "std/cpplib/ThreadUtils.h"

#include <chrono>
#include "std-generic/cpplib/ThreadUtils.h"
#include "std-generic/compat/condition_variable.h"

typedef std::lock_guard<std::recursive_mutex> lock_guard_t;

uint64_t inline GetSecondsCountSinceEpoch(uint64_t count)
{
	auto current_time_since_epoch = count * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den;

	return current_time_since_epoch;
}

PrimitiveTimer::PrimitiveTimer(void)
{
	m_stop_flag = false;
}

PrimitiveTimer::~PrimitiveTimer(void)
{
	Stop();
}

void PrimitiveTimer::SetCallback(const Callback &callback)
{
	lock_guard_t lock(m_lock);
	m_cb = callback;
}

void PrimitiveTimer::Start(uint32_t timeout_sec, bool once, uint32_t resolution_millis)
{
	Stop();
	{
		vs::condition_variable cv;
		std::mutex cv_lock;
		bool cv_flag = false;

		{
			lock_guard_t lock(m_lock);
			timeout_sec = (timeout_sec * 1000) > resolution_millis ? timeout_sec : resolution_millis / 1000;
			auto timer_thread_func = [this, &cv, &cv_lock, &cv_flag, timeout_sec, once, resolution_millis](void) -> void
			{
				vs::SetThreadName("Timer");

				Callback cb = m_cb;
				{
					cv_lock.lock();
					cv_flag = true;
					cv_lock.unlock();
					cv.notify_one();
				}

				auto last_tick_time = std::chrono::high_resolution_clock().now();
				while (!m_stop_flag)
				{
					vs::SleepFor(std::chrono::milliseconds(resolution_millis));
					if ((GetSecondsCountSinceEpoch(last_tick_time.time_since_epoch().count()) + timeout_sec) <=
						GetSecondsCountSinceEpoch(std::chrono::high_resolution_clock().now().time_since_epoch().count()))
					{
						if (cb != nullptr)
							cb();

						if (once)
						{
							Stop();
							break;
						}
						last_tick_time = std::chrono::high_resolution_clock().now();
					}
				}
			};

			// wait until timer thread ready
			{
				std::unique_lock<std::mutex> lock(cv_lock);
				m_stop_flag = false;
				m_timer_thread = std::thread(timer_thread_func);
				while (!cv_flag) // avoid spurious wakeups
				{
					cv.wait(lock);
				}
			}
		}
	}
}

void PrimitiveTimer::Stop(void)
{
	lock_guard_t lock(m_lock);
	m_stop_flag = true;

	if ((m_timer_thread.get_id() != std::this_thread::get_id()) && m_timer_thread.joinable())
		m_timer_thread.join();
}

bool PrimitiveTimer::IsRunning(void)
{
	return m_stop_flag;
}

