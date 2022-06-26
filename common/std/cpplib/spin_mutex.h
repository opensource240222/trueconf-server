#pragma once

#include <atomic>
#include <thread>

namespace vs {

template <unsigned spin_count = 100>
class spin_mutex
{
public:
	void lock() noexcept
	{
		if (try_lock())
			return;

		while (true)
		{
			for (unsigned i = 0; i < spin_count; ++i)
				if (try_lock())
					return;
			std::this_thread::yield();
		}
	}

	bool try_lock() noexcept
	{
		return !m_locked.test_and_set(std::memory_order_acq_rel);
	}

	void unlock() noexcept
	{
		m_locked.clear(std::memory_order_release);
	}

private:
	std::atomic_flag m_locked = ATOMIC_FLAG_INIT;
};

}
