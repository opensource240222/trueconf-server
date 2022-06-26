#pragma once

#include <cassert> // assert
#include <cstddef> // size_t
#include <cstdint> // uintptr_t
#include <functional> // reference_wrapper, bad_function_call
#include <type_traits> // aligned_storage, enable_if, ...
#include <typeinfo> // type_info

#if defined(_MSC_VER)
#	pragma warning(push)
#	pragma warning(disable: 4521 4522)
#endif

namespace vs {
namespace detail {

#if _cpp_lib_invoke >= 201411

template <class F, class... ArgTypes>
using invoke = std::invoke<F, ArgTypes...>;

#else

// Taken from http://en.cppreference.com/w/cpp/utility/functional/invoke
template <class T>
struct is_reference_wrapper : std::false_type {};
template <class U>
struct is_reference_wrapper<std::reference_wrapper<U>> : std::true_type {};

template <class Base, class T, class Derived, class... Args>
auto invoke(T Base::*pmf, Derived&& ref, Args&&... args)
//	noexcept(noexcept((std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...)))
	-> typename std::enable_if<
		std::is_function<T>::value
		&& std::is_base_of<Base, typename std::decay<Derived>::type>::value,
		decltype((std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...))
	>::type
{
	return (std::forward<Derived>(ref).*pmf)(std::forward<Args>(args)...);
}

template <class Base, class T, class RefWrap, class... Args>
auto invoke(T Base::*pmf, RefWrap&& ref, Args&&... args)
//	noexcept(noexcept((ref.get().*pmf)(std::forward<Args>(args)...)))
	-> typename std::enable_if<
		std::is_function<T>::value
		&& is_reference_wrapper<typename std::decay<RefWrap>::type>::value,
		decltype((ref.get().*pmf)(std::forward<Args>(args)...))
	>::type
{
	return (ref.get().*pmf)(std::forward<Args>(args)...);
}

template <class Base, class T, class Pointer, class... Args>
auto invoke(T Base::*pmf, Pointer&& ptr, Args&&... args)
//	noexcept(noexcept(((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...)))
	-> typename std::enable_if<
		std::is_function<T>::value
		&& !is_reference_wrapper<typename std::decay<Pointer>::type>::value
		&& !std::is_base_of<Base, typename std::decay<Pointer>::type>::value,
		decltype(((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...))
	>::type
{
	return ((*std::forward<Pointer>(ptr)).*pmf)(std::forward<Args>(args)...);
}

template <class Base, class T, class Derived>
auto invoke(T Base::*pmd, Derived&& ref)
//	noexcept(noexcept(std::forward<Derived>(ref).*pmd))
	-> typename std::enable_if<
		!std::is_function<T>::value
		&& std::is_base_of<Base, typename std::decay<Derived>::type>::value,
		decltype(std::forward<Derived>(ref).*pmd)
	>::type
{
	return std::forward<Derived>(ref).*pmd;
}

template <class Base, class T, class RefWrap>
auto invoke(T Base::*pmd, RefWrap&& ref)
//	noexcept(noexcept(ref.get().*pmd))
	-> typename std::enable_if<
		!std::is_function<T>::value
		&& is_reference_wrapper<typename std::decay<RefWrap>::type>::value,
		decltype(ref.get().*pmd)
	>::type
{
	return ref.get().*pmd;
}

template <class Base, class T, class Pointer>
auto invoke(T Base::*pmd, Pointer&& ptr)
//	noexcept(noexcept((*std::forward<Pointer>(ptr)).*pmd))
	-> typename std::enable_if<
		!std::is_function<T>::value
		&& !is_reference_wrapper<typename std::decay<Pointer>::type>::value
		&& !std::is_base_of<Base, typename std::decay<Pointer>::type>::value,
		decltype((*std::forward<Pointer>(ptr)).*pmd)
	>::type
{
	return (*std::forward<Pointer>(ptr)).*pmd;
}

template <class F, class... Args>
auto invoke(F&& f, Args&&... args)
//	noexcept(noexcept(std::forward<F>(f)(std::forward<Args>(args)...)))
	-> typename std::enable_if<
		!std::is_member_pointer<typename std::decay<F>::type>::value,
		decltype(std::forward<F>(f)(std::forward<Args>(args)...))
	>::type
{
	return std::forward<F>(f)(std::forward<Args>(args)...);
}

#endif

} // namespace detail

template <class>
class function;

template <class R, class... ArgTypes>
class function<R(ArgTypes...)>
{
	class impl_base
	{
	public:
		virtual ~impl_base() {}
		virtual R invoke(ArgTypes&&...) = 0;
		virtual impl_base* copy() const = 0;
		virtual impl_base* copy_to(void* ptr) const = 0;
		virtual impl_base* move_to(void* ptr) = 0;
		virtual const std::type_info& target_type() const noexcept = 0;
		virtual void* get_target(const std::type_info&) noexcept = 0;
	};

	template <class F>
	class impl : public impl_base
	{
	public:
		impl(const impl& x)
			: m_f(x.m_f)
		{
		}
		impl(impl& x)
			: m_f(x.m_f)
		{
		}
		impl(impl&& x)
			: m_f(std::move(x.m_f))
		{
		}
		explicit impl(const F& f)
			: m_f(f)
		{
		}
		explicit impl(F&& f)
			: m_f(std::move(f))
		{
		}
		impl& operator=(const impl&) = delete;
		impl& operator=(impl&) = delete;
		impl& operator=(impl&&) = delete;

	private:
		R invoke(ArgTypes&&... args) override
		{
			return detail::invoke(m_f, std::forward<ArgTypes>(args)...);
		}
		impl_base* copy() const override
		{
			return new impl(*this);
		}
		impl_base* copy_to(void* ptr) const override
		{
			return ::new (ptr) impl(*this);
		}
		impl_base* move_to(void* ptr) override
		{
			return ::new (ptr) impl(std::move(*this));
		}
		const std::type_info& target_type() const noexcept override
		{
			return typeid(F);
		}
		void* get_target(const std::type_info& ti) noexcept override
		{
			if (ti == typeid(F))
				return &m_f;
			else
				return nullptr;
		}
		F m_f;
	};

	static const size_t storage_size = 64;
	typedef	typename std::aligned_storage<storage_size>::type storage_type;

	template <class F>
	using impl_type = impl<typename std::remove_reference<F>::type>;

public:
	typedef R result_type;

	function() noexcept
		: m_impl(nullptr)
	{
	}
	// cppcheck-suppress noExplicitConstructor
	function(std::nullptr_t) noexcept
		: m_impl(nullptr)
	{
	}
	function(const function& x)
	{
		copy_from(x);
	}
	function(function&& x)
	{
		move_from(std::move(x));
	}
	template <class F, typename = typename std::enable_if<!std::is_same<function, typename std::decay<F>::type>::value>::type>
	// cppcheck-suppress noExplicitConstructor
	function(F&& f)
		: function(std::forward<F>(f), std::integral_constant<bool, (sizeof(impl_type<F>) <= storage_size)>())
	{
	}
	function& operator=(const function& x)
	{
		if (&x == this)
			return *this;
		destroy();
		copy_from(x);
		return *this;
	}
	function& operator=(function&& x)
	{
		if (&x == this)
			return *this;
		destroy();
		move_from(std::move(x));
		return *this;
	}
	function& operator=(std::nullptr_t)
	{
		destroy();
		return *this;
	}
	template <class F, typename = typename std::enable_if<!std::is_same<function, typename std::decay<F>::type>::value>::type>
	function& operator=(F&& f)
	{
		return operator_assign_impl(std::forward<F>(f), std::integral_constant<bool, (sizeof(impl_type<F>) <= storage_size)>());
	}
	// All seems to work correctly without this overload:
	// template <class F>
	// function& operator=(std::reference_wrapper<F> f);
	~function() noexcept
	{
		destroy();
	}
	void swap(function& x) // TODO: noexcept
	{
		function tmp(std::move(*this));
		*this = std::move(x);
		x = std::move(tmp);
	}
	explicit operator bool() const noexcept
	{
		return impl_ptr() != nullptr;
	}
	R operator()(ArgTypes... args) const
	{
		if (!*this)
			throw std::bad_function_call();
		return impl_ptr()->invoke(std::forward<ArgTypes>(args)...);
	}
	const std::type_info& target_type() const noexcept
	{
		if (impl_ptr() == nullptr)
			return typeid(void);
		else
			return impl_ptr()->target_type();
	}
	template <class T>
	T* target() noexcept
	{
		if (impl_ptr() == nullptr)
			return nullptr;
		else
			return static_cast<T*>(impl_ptr()->get_target(typeid(T)));
	}
	template <class T>
	const T* target() const noexcept
	{
		if (impl_ptr() == nullptr)
			return nullptr;
		else
			return static_cast<const T*>(impl_ptr()->get_target(typeid(T)));
	}

private:
	template <class F>
	function(F&& f, std::true_type)
		: m_impl(::new (&m_storage) impl_type<F>(std::forward<F>(f)))
	{
		assert(storage_flag() == 0);
		set_storage_flag();
	}
	template <class F>
	function(F&& f, std::false_type)
		: m_impl(new impl_type<F>(std::forward<F>(f)))
	{
		assert(storage_flag() == 0);
	}
	template <class F>
	function& operator_assign_impl(F&& f, std::true_type)
	{
		destroy();
		m_impl = ::new (&m_storage) impl_type<F>(std::forward<F>(f));
		assert(storage_flag() == 0);
		set_storage_flag();
		return *this;
	}
	template <class F>
	function& operator_assign_impl(F&& f, std::false_type)
	{
		destroy();
		m_impl = new impl_type<F>(std::forward<F>(f));
		assert(storage_flag() == 0);
		return *this;
	}
	uintptr_t storage_flag() const noexcept
	{
		return reinterpret_cast<uintptr_t>(m_impl) & 0x1;
	}
	void set_storage_flag() noexcept
	{
		m_impl = reinterpret_cast<impl_base*>(reinterpret_cast<uintptr_t>(m_impl) | 0x1);
	}
	impl_base* impl_ptr() const noexcept
	{
		return reinterpret_cast<impl_base*>(reinterpret_cast<uintptr_t>(m_impl) ^ storage_flag());
	}
	void destroy() noexcept
	{
		if (impl_ptr() == nullptr)
			return;
		if (storage_flag() != 0)
			impl_ptr()->~impl_base();
		else
			delete impl_ptr();
		m_impl = nullptr;
	}
	void copy_from(const function& x)
	{
		if (x.impl_ptr() == nullptr)
			m_impl = nullptr;
		else if (x.storage_flag() != 0)
		{
			m_impl = x.impl_ptr()->copy_to(&m_storage);
			assert(storage_flag() == 0);
			set_storage_flag();
		}
		else
		{
			m_impl = x.impl_ptr()->copy();
			assert(storage_flag() == 0);
		}
	}
	void move_from(function&& x)
	{
		if (x.impl_ptr() == nullptr)
			m_impl = nullptr;
		if (x.storage_flag() != 0)
		{
			m_impl = x.impl_ptr()->move_to(&m_storage);
			assert(storage_flag() == 0);
			set_storage_flag();
			x.destroy();
		}
		else
		{
			m_impl = x.m_impl;
			assert(storage_flag() == 0);
			x.m_impl = nullptr;
		}
	}

	impl_base* m_impl; // least significant bit means: 0 == impl is allocated with new, 1 == impl is stored in m_storage
	storage_type m_storage;
};

template <class R, class... ArgTypes>
bool operator==(const function<R(ArgTypes...)>& f, std::nullptr_t) noexcept
{
	return !f;
}
template <class R, class... ArgTypes>
bool operator==(std::nullptr_t, const function<R(ArgTypes...)>& f) noexcept
{
	return !f;
}
template <class R, class... ArgTypes>
bool operator!=(const function<R(ArgTypes...)>& f, std::nullptr_t) noexcept
{
	return f;
}
template <class R, class... ArgTypes>
bool operator!=(std::nullptr_t, const function<R(ArgTypes...)>& f) noexcept
{
	return f;
}

} // namespace vs

#if defined(_MSC_VER)
#	pragma warning(pop)
#endif
