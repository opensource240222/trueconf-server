#pragma once

#ifdef _WIN32

#include "std-generic/compat/chrono.h"
#include <atomic>
#include <mutex>

namespace vs
{
	class condition_variable
	{
	public:
		using native_handle_type = void *;

		condition_variable();
		~condition_variable();

		condition_variable(const condition_variable &) = delete;
		condition_variable &operator=(const condition_variable &) = delete;

		void wait(std::unique_lock<std::mutex> &lock)
		{
			(void)wait_for_impl(lock, INFINITE_TIMEOUT);
		}

		template< class Predicate>
		void wait(std::unique_lock<std::mutex> &lock, Predicate pred)
		{
			while (!pred())
			{
				wait(lock);
			}
		}

		template<class Rep, class Period>
		std::cv_status wait_for(std::unique_lock<std::mutex> &lock, const std::chrono::duration<Rep, Period> &relTime)
		{
			return wait_for_impl(lock, get_timeout(relTime));
		}

		template< class Rep, class Period, class Predicate>
		bool wait_for(std::unique_lock<std::mutex> &lock, const std::chrono::duration<Rep, Period> &relTime, Predicate pred)
		{
			return wait_until(lock, std::chrono::steady_clock::now() + relTime, std::ref(pred));
		}

		template<class Duration>
		std::cv_status wait_until(std::unique_lock<std::mutex> &lock, const std::chrono::time_point<std::chrono::steady_clock, Duration> &timeoutTime)
		{
			return wait_for(lock, timeoutTime - std::chrono::steady_clock::now());
		}

		template<class Duration, class Pred>
		bool wait_until(std::unique_lock<std::mutex> &lock, const std::chrono::time_point<std::chrono::steady_clock, Duration> &timeoutTime, Pred pred)
		{
			while (!pred())
			{
				if (wait_for_impl(lock, get_timeout(timeoutTime - std::chrono::steady_clock::now())) == std::cv_status::timeout)
				{
					return pred();
				}
			}
			return true;
		}

		void notify_all();
		void notify_one();

		native_handle_type native_handle()
		{
			return m_sem;
		}

	private:
		std::cv_status wait_for_impl(std::unique_lock<std::mutex> &lock, unsigned long dwMilliseconds);

		template<class Rep, class Period>
		inline unsigned long get_timeout(const std::chrono::duration<Rep, Period> &relTime)
		{
			if (relTime <= std::chrono::duration<Rep, Period>{ 0 })
				return 0;
			if (relTime >= std::chrono::milliseconds(INFINITE_TIMEOUT))
				return INFINITE_TIMEOUT - 1;
			return static_cast<decltype(INFINITE_TIMEOUT)>(vs::chrono::ceil<std::chrono::milliseconds>(relTime).count());
		}
	private:
		static const unsigned long INFINITE_TIMEOUT = 0xFFFFFFFF; // Infinite timeout
	private:
		native_handle_type m_sem;
		std::atomic<int> m_numWaiters;
	};
} //namespace vs
#else
#include <condition_variable>
namespace vs
{
	using std::condition_variable;
} // namespace vs
#endif //_WIN32