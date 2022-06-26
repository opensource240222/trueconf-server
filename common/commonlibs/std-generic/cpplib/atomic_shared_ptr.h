#pragma once

#include <atomic>
#include <memory>

// C++20 std::atomic<std::shared_ptr<T>> as a separate class template

namespace vs {

template <class T>
struct atomic_shared_ptr
{
	using value_type = std::shared_ptr<T>;
	static constexpr bool is_always_lock_free = false;

	bool is_lock_free() const noexcept
	{
		return false;
	}
	void store(std::shared_ptr<T> desired, std::memory_order /*order*/ = std::memory_order_seq_cst) noexcept
	{
		lock();
		m_sp.swap(desired); // Use swap instead of move assignment to avoid potential call to T::~T() under lock.
		unlock();
	}
	std::shared_ptr<T> load(std::memory_order /*order*/ = std::memory_order_seq_cst) const noexcept
	{
		lock();
		auto result = m_sp;
		unlock();
		return result;
	}
	operator std::shared_ptr<T>() const noexcept
	{
		lock();
		auto result = m_sp;
		unlock();
		return result;
	}

	std::shared_ptr<T> exchange(std::shared_ptr<T> desired, std::memory_order /*order*/ = std::memory_order_seq_cst) noexcept
	{
		lock();
		auto result = std::move(m_sp);
		m_sp = std::move(desired);
		unlock();
		return result;
	}

	bool compare_exchange_weak(std::shared_ptr<T>& expected, std::shared_ptr<T> desired, std::memory_order success, std::memory_order failure) noexcept
	{
		return compare_exchange_strong(expected, std::move(desired), success, failure);
	}
	bool compare_exchange_strong(std::shared_ptr<T>& expected, std::shared_ptr<T> desired, std::memory_order /*success*/, std::memory_order /*failure*/) noexcept
	{
		lock();
		const bool result = (m_sp == expected);
		if (result)
			m_sp.swap(desired); // Use swap instead of move assignment to avoid potential call to T::~T() under lock.
		else
			expected = m_sp;
		unlock();
		return result;
	}

	bool compare_exchange_weak(std::shared_ptr<T>& expected, std::shared_ptr<T> desired, std::memory_order order = std::memory_order_seq_cst) noexcept
	{
		return compare_exchange_weak(expected, std::move(desired), order, order);
	}
	bool compare_exchange_strong(std::shared_ptr<T>& expected, std::shared_ptr<T> desired, std::memory_order order = std::memory_order_seq_cst) noexcept
	{
		return compare_exchange_strong(expected, std::move(desired), order, order);
	}

	constexpr atomic_shared_ptr() noexcept = default;
	atomic_shared_ptr(std::shared_ptr<T> desired) noexcept
		: m_sp(std::move(desired))
	{
	}
	atomic_shared_ptr(const atomic_shared_ptr&) = delete;
	atomic_shared_ptr& operator=(const atomic_shared_ptr&) = delete;
	void operator=(std::shared_ptr<T> desired) noexcept
	{
		lock();
		m_sp.swap(desired); // Use swap instead of move assignment to avoid potential call to T::~T() under lock.
		unlock();
	}

private:
	std::shared_ptr<T> m_sp;
	mutable std::atomic<bool> m_spin { false };

	void lock() const noexcept
	{
		while (m_spin.exchange(true, std::memory_order_acq_rel) == true) {}
	}
	void unlock() const noexcept
	{
		m_spin.store(false, std::memory_order_release);
	}
};

}
