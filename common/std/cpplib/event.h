#pragma once

#include <atomic>
#include <chrono>

#if !defined(_WIN32) && ! defined(__linux__)
#	error Unknown platform
#endif

namespace vs {

class event
{
public:
	event(bool manual_reset, bool initial_state = false);
	~event();
	event(const event&) = delete;
	event& operator=(const event&) = delete;

	void set();
	void reset();
	void wait() const;
	bool try_wait() const;
	template <class Clock, class Duration>
	bool wait_until(const std::chrono::time_point<Clock, Duration>& abs_time) const;
	template <class Rep, class Period>
	bool wait_for(const std::chrono::duration<Rep, Period>& rel_time) const;

#if defined(_WIN32)
	typedef void* native_handle_type;
	native_handle_type native_handle();
#endif

private:
#if defined(_WIN32)
	bool wait_for_impl(std::chrono::milliseconds rel_time) const;

	void* m_ev;
	const bool m_manual_reset;
#elif defined(__linux__)
	bool wait_for_impl(std::chrono::nanoseconds rel_time) const;

	mutable std::atomic<int> m_state;
	const bool m_manual_reset;
	static_assert(sizeof(m_state) == 4, "futex works only with 32-bit values");
#endif
};

#if defined(_WIN32)

template <class Clock, class Duration>
inline bool event::wait_until(const std::chrono::time_point<Clock, Duration>& abs_time) const
{
	return wait_for(abs_time - Clock::now());
}

template <class Rep, class Period>
inline bool event::wait_for(const std::chrono::duration<Rep, Period>& rel_time) const
{
	if (rel_time.count() <= 0)
		return try_wait();
	return wait_for_impl(rel_time);
}

inline event::native_handle_type event::native_handle()
{
	return m_ev;
}

#elif defined(__linux__)

inline event::event(bool manual_reset, bool initial_state)
	: m_state(initial_state ? 1 : 0)
	, m_manual_reset(manual_reset)
{
}

inline event::~event()
{
}

template <class Clock, class Duration>
inline bool event::wait_until(const std::chrono::time_point<Clock, Duration>& abs_time) const
{
	return wait_for(abs_time - Clock::now());
}

template <class Rep, class Period>
inline bool event::wait_for(const std::chrono::duration<Rep, Period>& rel_time) const
{
	if (rel_time.count() <= 0)
		return try_wait();
	if (try_wait())
		return true;
	return wait_for_impl(rel_time);
}

#endif

}
