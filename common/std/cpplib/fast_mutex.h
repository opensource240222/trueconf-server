#pragma once

#if defined(_WIN32)

#ifndef VS_FAST_MUTEX_RECURSION_CHECKS
#	define VS_FAST_MUTEX_RECURSION_CHECKS 1
#endif

#include <Windows.h> // TODO: Replace with #include <synchapi.h> after migrating to Win 8 SDK

#if VS_FAST_MUTEX_RECURSION_CHECKS
#	include <system_error>
#endif

namespace vs {

class fast_mutex
{
public:
	fast_mutex() noexcept
#if VS_FAST_MUTEX_RECURSION_CHECKS
		: m_lock_count(0)
#endif
	{
		if (0 == ::InitializeCriticalSectionAndSpinCount(&m_cs, 1000))
			::InitializeCriticalSection(&m_cs);
	}
	~fast_mutex()
	{
		::DeleteCriticalSection(&m_cs);
	}
	fast_mutex(const fast_mutex&) = delete;
	fast_mutex& operator=(const fast_mutex&) = delete;
	void lock()
	{
		::EnterCriticalSection(&m_cs);
#if VS_FAST_MUTEX_RECURSION_CHECKS
		if (::InterlockedIncrement(&m_lock_count) != 1)
		{
			// We are trying to lock the mutex twice in the same thread, this is UB because we are breaking pre-condition for lock().
			// Standard advises to throw std::system_error if implementation can detect this case.
			::InterlockedDecrement(&m_lock_count);
			::LeaveCriticalSection(&m_cs);
			throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
		}
#endif
	}
	bool try_lock()
	{
		if (::TryEnterCriticalSection(&m_cs))
		{
#if VS_FAST_MUTEX_RECURSION_CHECKS
			if (::InterlockedIncrement(&m_lock_count) != 1)
			{
				// We are trying to lock the mutex twice in the same thread, this is UB because we are breaking pre-condition for try_lock().
				// Standard doesn't have a recomended way of handling this issue, but requires try_lock() to not throw exceptions.
				// Note: Different standard library implementations have different behaviour in this case: MSVC and libc++ return false, libstdc++ return true.
				::InterlockedDecrement(&m_lock_count);
				::LeaveCriticalSection(&m_cs);
#ifdef _DEBUG
				__debugbreak();
#endif
				return false;
			}
#endif
			return true;
		}
		else
			return false;
	}
	void unlock()
	{
#if VS_FAST_MUTEX_RECURSION_CHECKS
		::InterlockedDecrement(&m_lock_count);
#endif
		::LeaveCriticalSection(&m_cs);
	}
	typedef ::CRITICAL_SECTION* native_handle_type;
	native_handle_type native_handle()
	{
		return &m_cs;
	}

private:
	::CRITICAL_SECTION m_cs;

#if VS_FAST_MUTEX_RECURSION_CHECKS
	::LONG m_lock_count;
#endif
};

class fast_recursive_mutex
{
public:
	fast_recursive_mutex()
	{
		if (0 == ::InitializeCriticalSectionAndSpinCount(&m_cs, 1000))
			::InitializeCriticalSection(&m_cs);
	}
	~fast_recursive_mutex()
	{
		::DeleteCriticalSection(&m_cs);
	}
	fast_recursive_mutex(const fast_recursive_mutex&) = delete;
	fast_recursive_mutex& operator=(const fast_recursive_mutex&) = delete;
	void lock()
	{
		::EnterCriticalSection(&m_cs);
	}
	bool try_lock() noexcept
	{
		return ::TryEnterCriticalSection(&m_cs) != FALSE;
	}
	void unlock()
	{
		::LeaveCriticalSection(&m_cs);
	}
	typedef ::CRITICAL_SECTION* native_handle_type;
	native_handle_type native_handle()
	{
		return &m_cs;
	}

private:
	::CRITICAL_SECTION m_cs;
};

} // namespace vs

#else

#include <mutex>

namespace vs {

using fast_mutex = std::mutex;
using fast_recursive_mutex = std::recursive_mutex;

}

#endif
