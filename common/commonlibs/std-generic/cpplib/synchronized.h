#pragma once

#include "std-generic/gcc_version.h"

#include <cassert>
#include <mutex>
#include <utility>

// Lightweight version of folly::Synchronized without shared mutexes support.

namespace vs {

namespace detail { template <class S> class LockedPtr; }

template <class T, class Mutex = std::mutex>
class Synchronized
{
	friend class detail::LockedPtr<Synchronized>;
	friend class detail::LockedPtr<const Synchronized>;

public:
	template <class... Args>
	explicit Synchronized(Args&&... args)
		: m_data(std::forward<Args>(args)...)
	{
	}

	Synchronized(Synchronized& x)
		: m_data(*x.lock())
		, m_mutex()
	{
	}

	Synchronized(Synchronized&& x)
		: m_data(std::move(*x.lock()))
		, m_mutex()
	{
	}

	Synchronized& operator=(Synchronized& x)
	{
		if (this == &x)
			return *this;

		// Global ordering of locks to avoid deadlock
		if (this < &x)
		{
			std::lock_guard<Mutex> l(m_mutex);
			std::lock_guard<Mutex> l_x(x.m_mutex);
			m_data = x.m_data;
		}
		else
		{
			std::lock_guard<Mutex> l_x(x.m_mutex);
			std::lock_guard<Mutex> l(m_mutex);
			m_data = x.m_data;
		}
		return *this;
	}

	Synchronized& operator=(Synchronized&& x)
	{
		if (this == &x)
			return *this;

		// Global ordering of locks to avoid deadlock
		if (this < &x)
		{
			std::lock_guard<Mutex> l(m_mutex);
			std::lock_guard<Mutex> l_x(x.m_mutex);
			m_data = std::move(x.m_data);
		}
		else
		{
			std::lock_guard<Mutex> l_x(x.m_mutex);
			std::lock_guard<Mutex> l(m_mutex);
			m_data = std::move(x.m_data);
		}
		return *this;
	}

	Synchronized& operator=(const T& x)
	{
		std::lock_guard<Mutex> l(m_mutex);
		m_data = x;
		return *this;
	}

	Synchronized& operator=(T&& x)
	{
		std::lock_guard<Mutex> l(m_mutex);
		m_data = std::move(x);
		return *this;
	}

	void swap(Synchronized& x)
	{
		if (this == &x)
			return;

		using std::swap;
		// Global ordering of locks to avoid deadlock
		if (this < &x)
		{
			std::lock_guard<Mutex> l(m_mutex);
			std::lock_guard<Mutex> l_x(x.m_mutex);
			swap(m_data, x.m_data);
		}
		else
		{
			std::lock_guard<Mutex> l_x(x.m_mutex);
			std::lock_guard<Mutex> l(m_mutex);
			swap(m_data, x.m_data);
		}
	}

	void swap(T& x)
	{
		using std::swap;
		std::lock_guard<Mutex> l(m_mutex);
		swap(m_data, x);
	}

	detail::LockedPtr<Synchronized> lock()
	{
		return detail::LockedPtr<Synchronized>(this);
	}

	detail::LockedPtr<const Synchronized> lock() const
	{
		return detail::LockedPtr<const Synchronized>(this);
	}

	detail::LockedPtr<Synchronized> operator->()
	{
		return detail::LockedPtr<Synchronized>(this);
	}

	detail::LockedPtr<const Synchronized> operator->() const
	{
		return detail::LockedPtr<const Synchronized>(this);
	}

	template <class F>
	auto withLock(F&& f)
#if defined(GCC_VERSION) && GCC_VERSION < 40800
		-> decltype(std::forward<F>(f)(std::declval<T&>()))
#endif
	{
		std::lock_guard<Mutex> l(m_mutex);
		return std::forward<F>(f)(m_data);
	}

	template <class F>
	auto withLock(F&& f) const
#if defined(GCC_VERSION) && GCC_VERSION < 40800
		-> decltype(std::forward<F>(f)(std::declval<const T&>()))
#endif
	{
		std::lock_guard<Mutex> l(m_mutex);
		return std::forward<F>(f)(m_data);
	}

	template <class F>
	auto withLockPtr(F&& f)
#if defined(GCC_VERSION) && GCC_VERSION < 40800
		-> decltype(std::forward<F>(f)(std::declval<detail::LockedPtr<Synchronized>>()))
#endif
	{
		return std::forward<F>(f)(detail::LockedPtr<Synchronized>(this));
	}

	template <class F>
	auto withLockPtr(F&& f) const
#if defined(GCC_VERSION) && GCC_VERSION < 40800
		-> decltype(std::forward<F>(f)(std::declval<detail::LockedPtr<const Synchronized>>()))
#endif
	{
		return std::forward<F>(f)(detail::LockedPtr<const Synchronized>(this));
	}

private:
	T m_data;
	mutable Mutex m_mutex;
};

namespace detail {

template <class S> class LockedPtr
{
	class UnlockGuard
	{
	public:
		explicit UnlockGuard(LockedPtr* ptr)
			: m_ptr(ptr)
			, m_saved_parent(m_ptr->m_parent)
		{
			assert(m_ptr->m_parent);
			m_ptr->m_parent->m_mutex.unlock();
			m_ptr->m_parent = nullptr;
		}
		UnlockGuard(const UnlockGuard&) = delete;
		UnlockGuard(UnlockGuard&& x) noexcept
			: m_ptr(x.m_ptr)
			, m_saved_parent(x.m_saved_parent)
		{
			x.m_ptr = nullptr;
			x.m_saved_parent = nullptr;
		}
		UnlockGuard& operator=(const UnlockGuard&) = delete;
		UnlockGuard& operator=(UnlockGuard&& x) = delete;

		~UnlockGuard()
		{
			if (m_ptr)
			{
				m_ptr->m_parent = m_saved_parent;
				m_ptr->m_parent->m_mutex.lock();
			}
		}

	private:
		LockedPtr* m_ptr;
		S* m_saved_parent;
	};

public:
	explicit LockedPtr(S* parent)
		: m_parent(parent)
	{
		assert(m_parent);
		m_parent->m_mutex.lock();
	}

	LockedPtr(const LockedPtr&) = delete;
	LockedPtr(LockedPtr&& x) noexcept
		: m_parent(x.m_parent)
	{
		x.m_parent = nullptr;
	}

	LockedPtr& operator=(const LockedPtr&) = delete;
	LockedPtr& operator=(LockedPtr&& x) noexcept
	{
		if (this == &x)
			return *this;
		if (m_parent)
			m_parent->m_mutex.unlock();
		m_parent = x.m_parent;
		x.m_parent = nullptr;
	}

	~LockedPtr()
	{
		if (m_parent)
			m_parent->m_mutex.unlock();
	}

	explicit operator bool() const noexcept
	{
		return m_parent != nullptr;
	}

	decltype(&std::declval<S&>().m_data) operator->() const noexcept
	{
		assert(m_parent);
		return &m_parent->m_data;
	}

	// Note: Don't remove extra parenthesis on the next line, they are needed to force decltype to threat its argument as an expression.
	decltype((std::declval<S&>().m_data)) operator*() const noexcept
	{
		assert(m_parent);
		return m_parent->m_data;
	}

	UnlockGuard scopedUnlock()
	{
		return UnlockGuard(this);
	}

private:
	S* m_parent;
};

}

}
