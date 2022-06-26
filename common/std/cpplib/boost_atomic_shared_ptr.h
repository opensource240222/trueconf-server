#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/memory_order.hpp>

// C++20 std::atomic<std::shared_ptr<T>> as a separate class template, modified for boost::shared_ptr

namespace boost {

template <class T>
struct atomic_shared_ptr
{
	using value_type = boost::shared_ptr<T>;
	static constexpr bool is_always_lock_free = false;

	bool is_lock_free() const noexcept
	{
		return boost::atomic_is_lock_free(&m_sp);
	}
	void store(boost::shared_ptr<T> desired, boost::memory_order order = boost::memory_order_seq_cst) noexcept
	{
		boost::atomic_store_explicit(&m_sp, std::move(desired), order);
	}
	boost::shared_ptr<T> load(boost::memory_order order = boost::memory_order_seq_cst) const noexcept
	{
		return boost::atomic_load_explicit(&m_sp, order);
	}
	operator boost::shared_ptr<T>() const noexcept
	{
		return boost::atomic_load(&m_sp);
	}

	boost::shared_ptr<T> exchange(boost::shared_ptr<T> desired, boost::memory_order order = boost::memory_order_seq_cst) noexcept
	{
		return boost::atomic_exchange_explicit(&m_sp, std::move(desired), order);
	}

	bool compare_exchange_weak(boost::shared_ptr<T>& expected, boost::shared_ptr<T> desired, boost::memory_order success, boost::memory_order failure) noexcept
	{
		return boost::atomic_compare_exchange_explicit(&m_sp, &expected, std::move(desired), success, failure);
	}
	bool compare_exchange_strong(boost::shared_ptr<T>& expected, boost::shared_ptr<T> desired, boost::memory_order success, boost::memory_order failure) noexcept
	{
		return boost::atomic_compare_exchange_explicit(&m_sp, &expected, std::move(desired), success, failure);
	}

	bool compare_exchange_weak(boost::shared_ptr<T>& expected, boost::shared_ptr<T> desired, boost::memory_order order = boost::memory_order_seq_cst) noexcept
	{
		return boost::atomic_compare_exchange_explicit(&m_sp, &expected, std::move(desired), order, order);
	}
	bool compare_exchange_strong(boost::shared_ptr<T>& expected, boost::shared_ptr<T> desired, boost::memory_order order = boost::memory_order_seq_cst) noexcept
	{
		return boost::atomic_compare_exchange_explicit(&m_sp, &expected, std::move(desired), order, order);
	}

	constexpr atomic_shared_ptr() noexcept
		: m_sp()
	{
	}
	atomic_shared_ptr(boost::shared_ptr<T> desired) noexcept
		: m_sp(std::move(desired))
	{
	}
	atomic_shared_ptr(const atomic_shared_ptr&) = delete;
	atomic_shared_ptr& operator=(const atomic_shared_ptr&) = delete;
	void operator=(boost::shared_ptr<T> desired) noexcept
	{
		boost::atomic_store(&m_sp, std::move(desired));
	}

private:
	boost::shared_ptr<T> m_sp;
};

}
