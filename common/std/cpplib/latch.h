#pragma once

#include "event.h"

#include <atomic>

namespace vs {

// n4392
class latch
{
public:
	explicit latch(ptrdiff_t count)
		: m_counter(count)
		, m_event(true)
	{
	}
	~latch()
	{
	}
	latch(const latch&) = delete;
	latch(latch&&) = delete;
	latch& operator=(const latch&) = delete;
	latch& operator=(latch&&) = delete;

	void count_down_and_wait()
	{
		count_down(1);
		wait();
	}
	void count_down(ptrdiff_t n)
	{
		if (m_counter.fetch_sub(n, std::memory_order_acq_rel) == 1)
			m_event.set();
	}
	bool is_ready() const noexcept
	{
		return m_counter.load(std::memory_order_acquire) == 0;
	}
	void wait() const
	{
		if (!is_ready())
			m_event.wait();
	}

private:
	std::atomic<ptrdiff_t> m_counter;
	vs::event m_event;
};

}
