#pragma once

#if defined(_WIN32)

#ifndef VS_FAST_MUTEX_RECURSION_CHECKS
#	define VS_FAST_MUTEX_RECURSION_CHECKS 1
#endif

#include "std/cpplib/numerical.h"

#include <Windows.h> // TODO: Replace with #include <synchapi.h> after migrating to Win 8 SDK

#include <chrono>
#include <stdexcept>
#include <system_error>

namespace vs {

class fast_timed_mutex
{
public:
	fast_timed_mutex()
		: m_handle(::CreateMutex(NULL, FALSE, NULL))
#if VS_FAST_MUTEX_RECURSION_CHECKS
		, m_lock_count(0)
#endif
	{
		if (m_handle == NULL)
			throw std::system_error(::GetLastError(), std::system_category(), "CreateMutex");
	}
	~fast_timed_mutex()
	{
		::CloseHandle(m_handle);
	}
	fast_timed_mutex(const fast_timed_mutex&) = delete;
	fast_timed_mutex& operator=(const fast_timed_mutex&) = delete;
	void lock()
	{
		while (!lock_impl(INFINITE)) {}
	}
	bool try_lock()
	{
		return lock_impl(0);
	}
	template <class Rep, class Period>
	bool try_lock_for(const std::chrono::duration<Rep, Period>& rel_time)
	{
		return lock_impl(clamp_cast<::DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(rel_time).count()));
	}
	template <class Clock, class Duration>
	bool try_lock_until(const std::chrono::time_point<Clock, Duration>& abs_time)
	{
		return try_lock_for(abs_time - Clock::now());
	}
	void unlock()
	{
#if VS_FAST_MUTEX_RECURSION_CHECKS
		::InterlockedDecrement(&m_lock_count);
#endif
		if (::ReleaseMutex(m_handle) == 0)
			throw std::system_error(::GetLastError(), std::system_category(), "ReleaseMutex");
	}
	typedef ::HANDLE native_handle_type;
	native_handle_type native_handle()
	{
		return m_handle;
	}

private:
	bool lock_impl(::DWORD timeout_ms)
	{
		switch (::WaitForSingleObject(m_handle, timeout_ms))
		{
		case WAIT_OBJECT_0:
#if VS_FAST_MUTEX_RECURSION_CHECKS
			if (::InterlockedIncrement(&m_lock_count) != 1)
			{
				// We are trying to lock the mutex twice in the same thread, this is UB because we are breaking pre-condition for used lock function.
				::InterlockedDecrement(&m_lock_count);
				::ReleaseMutex(&m_handle);
				if (timeout_ms > 0)
				{
					// Standard advises to throw std::system_error if implementation can detect this case (for lock()).
					throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
				}
				else
				{
#ifdef _DEBUG
					__debugbreak();
#endif
					return false;
				}
			}
#endif
			return true;
		case WAIT_TIMEOUT:
			return false;
		case WAIT_FAILED:
			throw std::system_error(::GetLastError(), std::system_category(), "WaitForSingleObject");
		case WAIT_ABANDONED:
			::ReleaseMutex(m_handle);
			throw std::runtime_error("Mutex was abandonned");
		default:
			return false;
		}
	}

	::HANDLE m_handle;

#if VS_FAST_MUTEX_RECURSION_CHECKS
	::LONG m_lock_count;
#endif
};

class fast_recursive_timed_mutex
{
public:
	fast_recursive_timed_mutex()
		: m_handle(::CreateMutex(NULL, FALSE, NULL))
	{
		if (m_handle == NULL)
			throw std::system_error(::GetLastError(), std::system_category(), "CreateMutex");
	}
	~fast_recursive_timed_mutex()
	{
		::CloseHandle(m_handle);
	}
	fast_recursive_timed_mutex(const fast_recursive_timed_mutex&) = delete;
	fast_recursive_timed_mutex& operator=(const fast_recursive_timed_mutex&) = delete;
	void lock()
	{
		while (!lock_impl(INFINITE)) {}
	}
	bool try_lock()
	{
		return lock_impl(0);
	}
	template <class Rep, class Period>
	bool try_lock_for(const std::chrono::duration<Rep, Period>& rel_time)
	{
		return lock_impl(clamp_cast<::DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(rel_time).count()));
	}
	template <class Clock, class Duration>
	bool try_lock_until(const std::chrono::time_point<Clock, Duration>& abs_time)
	{
		return try_lock_for(abs_time - Clock::now());
	}
	void unlock()
	{
		if (::ReleaseMutex(m_handle) == 0)
			throw std::system_error(::GetLastError(), std::system_category(), "ReleaseMutex");
	}
	typedef ::HANDLE native_handle_type;
	native_handle_type native_handle()
	{
		return m_handle;
	}

private:
	bool lock_impl(::DWORD timeout_ms)
	{
		switch (::WaitForSingleObject(m_handle, timeout_ms))
		{
		case WAIT_OBJECT_0:
			return true;
		case WAIT_TIMEOUT:
			return false;
		case WAIT_FAILED:
			throw std::system_error(::GetLastError(), std::system_category(), "WaitForSingleObject");
		case WAIT_ABANDONED:
			::ReleaseMutex(m_handle);
			throw std::runtime_error("Mutex was abandonned");
		default:
			return false;
		}
	}

	::HANDLE m_handle;
};

} // namespace vs

#else

namespace vs {

using fast_timed_mutex = std::timed_mutex;
using fast_recursive_timed_mutex = std::recursive_timed_mutex;

}

#endif
