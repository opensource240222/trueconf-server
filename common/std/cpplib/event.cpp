#include "event.h"

#include <system_error>

#if defined(_WIN32)
#	include "std-generic/cpplib/AtomicCache.h"

#	include <Windows.h>
#elif defined(__linux__)
#	include <limits.h>
#	include <time.h>
#	include <unistd.h>
#	include <sys/syscall.h>
#	include <linux/futex.h>
#else
#	error Unknown platform
#endif

#if !defined(__has_feature)
#	define __has_feature(x) 0
#endif

#if __has_feature(thread_sanitizer) || defined(__SANITIZE_THREAD__)
extern "C" void __tsan_acquire(void*);
extern "C" void __tsan_release(void*);
#endif

namespace vs {

#if defined(_WIN32)

static_assert(std::is_same< ::HANDLE, void*>::value, "");
struct HANDLE_deleter { void operator()(::HANDLE x) const {
	if (::CloseHandle(x) == 0)
	{
#if _DEBUG
		volatile auto err = ::GetLastError();
		(void)err;
		__debugbreak();
#endif
	}
}};

static vs::AtomicCache< ::HANDLE, HANDLE_deleter, 16> s_event_cache_manual_reset(16, NULL);
static vs::AtomicCache< ::HANDLE, HANDLE_deleter, 16> s_event_cache_auto_reset(16, NULL);

event::event(bool manual_reset, bool initial_state)
	: m_ev(manual_reset ? s_event_cache_manual_reset.Get() : s_event_cache_auto_reset.Get())
	, m_manual_reset(manual_reset)
{
	if (m_ev != NULL)
	{
		if (initial_state)
			set();
		else
			reset();
		return;
	}

	m_ev = ::CreateEvent(NULL, m_manual_reset ? TRUE : FALSE, initial_state ? TRUE : FALSE, NULL);
	if (m_ev == NULL)
		throw std::system_error(::GetLastError(), std::system_category(), "CreateEvent");
}

event::~event()
{
	if (m_manual_reset)
		s_event_cache_manual_reset.Put(m_ev);
	else
		s_event_cache_auto_reset.Put(m_ev);
}

void event::set()
{
	if (::SetEvent(m_ev) == 0)
		throw std::system_error(::GetLastError(), std::system_category(), "SetEvent");
}

void event::reset()
{
	if (::ResetEvent(m_ev) == 0)
		throw std::system_error(::GetLastError(), std::system_category(), "ResetEvent");
}

void event::wait() const
{
	if (::WaitForSingleObject(m_ev, INFINITE) != WAIT_OBJECT_0)
		throw std::system_error(::GetLastError(), std::system_category(), "WaitForSingleObject");
}

bool event::try_wait() const
{
	auto ret = ::WaitForSingleObject(m_ev, 0);
	if (ret == WAIT_OBJECT_0)
		return true;
	else if (ret == WAIT_TIMEOUT)
		return false;
	else
		throw std::system_error(::GetLastError(), std::system_category(), "WaitForSingleObject");
}

bool event::wait_for_impl(std::chrono::milliseconds rel_time) const
{
	auto ret = ::WaitForSingleObject(m_ev, rel_time.count());
	if (ret == WAIT_OBJECT_0)
		return true;
	else if (ret == WAIT_TIMEOUT)
		return false;
	else
		throw std::system_error(::GetLastError(), std::system_category(), "WaitForSingleObject");
}

#elif defined(__linux__)

void event::set()
{
	int old_state = m_state.exchange(1, std::memory_order_acq_rel);
	if (old_state == 0)
	{
		const auto manual_reset = m_manual_reset;
#if __has_feature(thread_sanitizer) || defined(__SANITIZE_THREAD__)
		__tsan_release(&m_state);
#endif
		if (::syscall(SYS_futex, static_cast<const void*>(&m_state), FUTEX_WAKE_PRIVATE, manual_reset ? INT_MAX : 1, NULL, NULL, 0) == -1)
			throw std::system_error(errno, std::system_category(), "syscall(SYS_futex)");
	}
}

void event::reset()
{
	m_state.store(0, std::memory_order_release);
}

void event::wait() const
{
	while (true)
	{
		if (try_wait())
			return;
		if (::syscall(SYS_futex, static_cast<const void*>(&m_state), FUTEX_WAIT_PRIVATE, 0, NULL, NULL, 0) == -1)
			if (errno != EAGAIN && errno != EINTR)
				throw std::system_error(errno, std::system_category(), "syscall(SYS_futex)");
#if __has_feature(thread_sanitizer) || defined(__SANITIZE_THREAD__)
		__tsan_acquire(&m_state);
#endif
	}
}

bool event::try_wait() const
{
	if (m_manual_reset)
		return m_state.load(std::memory_order_acquire) == 1;
	else
	{
		int expected_value = 1;
		return m_state.compare_exchange_strong(expected_value, 0, std::memory_order_acq_rel);
	}
}

bool event::wait_for_impl(std::chrono::nanoseconds rel_time) const
{
	const auto rel_time_s  = std::chrono::duration_cast<std::chrono::seconds>(rel_time);
	const auto rel_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(rel_time - rel_time_s);
	timespec timeout;
	timeout.tv_sec  = rel_time_s.count();
	timeout.tv_nsec = rel_time_ns.count();

	timespec start;
	::clock_gettime(CLOCK_REALTIME, &start);
	while (true)
	{
		if (::syscall(SYS_futex, static_cast<const void*>(&m_state), FUTEX_WAIT_PRIVATE, 0, &timeout, NULL, 0) == -1)
		{
			if (errno == ETIMEDOUT)
				return false;
			else if (errno != EAGAIN && errno != EINTR)
				throw std::system_error(errno, std::system_category(), "syscall(SYS_futex)");
		}
#if __has_feature(thread_sanitizer) || defined(__SANITIZE_THREAD__)
		__tsan_acquire(&m_state);
#endif

		if (try_wait())
			return true;

		timespec now;
		::clock_gettime(CLOCK_REALTIME, &now);
		timeout.tv_sec  = rel_time_s.count()  - (now.tv_sec  - start.tv_sec);
		timeout.tv_nsec = rel_time_ns.count() - (now.tv_nsec - start.tv_nsec);
		if (timeout.tv_nsec > 1000000000)
		{
			timeout.tv_nsec -= 1000000000;
			timeout.tv_sec += 1;
		}
		else if (timeout.tv_nsec < 0)
		{
			timeout.tv_nsec += 1000000000;
			timeout.tv_sec -= 1;
		}
		if (timeout.tv_sec < 0)
			return false;
	}
}

#endif

}
