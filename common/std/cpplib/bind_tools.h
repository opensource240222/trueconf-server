#pragma once

#include "std-generic/gcc_version.h"

#include <functional>
#include <memory>
#include <type_traits>

namespace vs {
namespace detail {

// Used to trick std::bind into allowing move from binded value
template <class T>
struct bind_rvalue_helper
{
	T value;
	bind_rvalue_helper(T& x) : value(std::move(x)) {}
	bind_rvalue_helper(const bind_rvalue_helper&);
	bind_rvalue_helper(bind_rvalue_helper&& x) : value(std::move(x.value)) {}
	template <class... U>
	T&& operator()(U&&...) { return std::move(value); }
};
template <class T>
struct bind_rvalue_helper<T&>
{
	T& value;
	bind_rvalue_helper(T& x) : value(x) {}
	bind_rvalue_helper(const bind_rvalue_helper&);
	bind_rvalue_helper(bind_rvalue_helper&& x) : value(x.value) {}
	template <class... U>
	T&& operator()(U&&...) { return std::move(value); }
};

} // namespace detail

// Trait used to decide whether T or std::reference_wrapper<const T> should be stored in bind object
template <class T, typename = void>
struct is_fast_copyable : std::false_type {};

template <class T>
struct is_fast_copyable<std::shared_ptr<T>> : std::true_type {};

// small trivially copyable types
template <class T>
struct is_fast_copyable<T, typename std::enable_if<(sizeof(T) <= 4*sizeof(nullptr)) &&
#if defined(GCC_VERSION) && GCC_VERSION < 50000
	std::is_pod<T>::value
#else
	std::is_trivially_copyable<T>::value
#endif
>> : std::true_type {};

template <class T, typename = void>
struct call_arg_traits {};

// pass by value
template <class T>
struct call_arg_traits<T, typename std::enable_if<!std::is_reference<T>::value && !is_fast_copyable<typename std::remove_cv<T>::type>::value>::type>
{
	// sync - store const reference to argument
	template <class U>
	static std::reference_wrapper<const T> forward_sync(typename std::remove_reference<U>::type& x) { return std::reference_wrapper<const T>(x); }
	// async - store value, moving it from argument
	template <class U>
	static T&& forward_async(typename std::remove_reference<U>::type& x) { return std::move(x); }
};

// pass by value, types with fast copy
template <class T>
struct call_arg_traits<T, typename std::enable_if<!std::is_reference<T>::value && is_fast_copyable<typename std::remove_cv<T>::type>::value>::type>
{
	// sync - store value, moving it from argument
	template <class U>
	static T&& forward_sync(typename std::remove_reference<U>::type& x) { return std::move(x); }
	// async - store value, moving it from argument
	template <class U>
	static T&& forward_async(typename std::remove_reference<U>::type& x) { return std::move(x); }
};

// pass by non-const lvalue reference
template <class T>
struct call_arg_traits<T&, typename std::enable_if<!std::is_const<T>::value>::type>
{
	// sync - store reference to argument
	template <class U>
	static std::reference_wrapper<T> forward_sync(typename std::remove_reference<U>::type& x) { return std::reference_wrapper<T>(x); }
	//template <class U>
	//static T& forward_async(typename std::remove_reference<U>::type& x) { static_assert(false, "passing non-const lvalue references to asynchronous calls is dangerous and disabled"); };
};

// pass by const lvalue reference
template <class T>
struct call_arg_traits<T&, typename std::enable_if<std::is_const<T>::value && !is_fast_copyable<typename std::remove_cv<T>::type>::value>::type>
{
	// sync - store reference to argument
	template <class U>
	static std::reference_wrapper<T> forward_sync(typename std::remove_reference<U>::type& x) { return std::reference_wrapper<T>(x); }
	// async - store value, moveing it from argument if possible
	template <class U>
	static T& forward_async(const typename std::remove_reference<U>::type& x) { return x; }
	template <class U>
	static typename std::remove_cv<T>::type&& forward_async(typename std::remove_const<typename std::remove_reference<U>::type>::type& x) { return std::move(x); }
};

// pass by const lvalue reference, types with fast copy
template <class T>
struct call_arg_traits<T&, typename std::enable_if<std::is_const<T>::value && is_fast_copyable<typename std::remove_cv<T>::type>::value>::type>
{
	// sync - store value, moveing it from argument if possible
	template <class U>
	static T& forward_sync(const typename std::remove_reference<U>::type& x) { return x; }
	template <class U>
	static typename std::remove_cv<T>::type&& forward_sync(typename std::remove_const<typename std::remove_reference<U>::type>::type& x) { return std::move(x); }
	// async - store value, moveing it from argument if possible
	template <class U>
	static T& forward_async(const typename std::remove_reference<U>::type& x) { return x; }
	template <class U>
	static typename std::remove_cv<T>::type&& forward_async(typename std::remove_const<typename std::remove_reference<U>::type>::type& x) { return std::move(x); }
};

// pass by rvalue reference
template <class T>
struct call_arg_traits<T&&, typename std::enable_if<!is_fast_copyable<typename std::remove_cv<T>::type>::value>::type>
{
	// sync - store reference to argument and allow bind to move from it
	template <class U>
	static detail::bind_rvalue_helper<T&> forward_sync(typename std::remove_reference<U>::type& x) { return detail::bind_rvalue_helper<T&>(x); }
	// async - store value moving it from argument and allow bind to move from it
	template <class U>
	static detail::bind_rvalue_helper<T> forward_async(typename std::remove_reference<U>::type& x) { return detail::bind_rvalue_helper<T>(x); }
};

// pass by rvalue reference, types with fast copy
template <class T>
struct call_arg_traits<T&&, typename std::enable_if<is_fast_copyable<typename std::remove_cv<T>::type>::value>::type>
{
	// sync - store value moving it from argument and allow bind to move from it
	template <class U>
	static detail::bind_rvalue_helper<T> forward_sync(typename std::remove_reference<U>::type& x) { return detail::bind_rvalue_helper<T>(x); }
	// async - store value moving it from argument and allow bind to move from it
	template <class U>
	static detail::bind_rvalue_helper<T> forward_async(typename std::remove_reference<U>::type& x) { return detail::bind_rvalue_helper<T>(x); }
};

template <class... CallArgTypes, class Callable, class... ArgTypes>
auto forward_sync_call(Callable&& fn, ArgTypes&&... args)
-> decltype(std::bind(std::forward<Callable>(fn), call_arg_traits<CallArgTypes>::template forward_sync<ArgTypes>(args)...))
{
	return std::bind(std::forward<Callable>(fn), call_arg_traits<CallArgTypes>::template forward_sync<ArgTypes>(args)...);
}

template <class... CallArgTypes, class Class, class Pointer, class... ArgTypes>
auto forward_sync_call(void (Class::*pmf)(CallArgTypes...), Pointer&& ptr, ArgTypes&&... args)
-> decltype(std::bind(pmf, std::forward<Pointer>(ptr), call_arg_traits<CallArgTypes>::template forward_sync<ArgTypes>(args)...))
{
	return std::bind(pmf, std::forward<Pointer>(ptr), call_arg_traits<CallArgTypes>::template forward_sync<ArgTypes>(args)...);
}

template <class... CallArgTypes, class Callable, class... ArgTypes>
auto forward_async_call(Callable&& fn, ArgTypes&&... args)
-> decltype(std::bind(std::forward<Callable>(fn), call_arg_traits<CallArgTypes>::template forward_async<ArgTypes>(args)...))
{
	return std::bind(std::forward<Callable>(fn), call_arg_traits<CallArgTypes>::template forward_async<ArgTypes>(args)...);
}

template <class... CallArgTypes, class Class, class Pointer, class... ArgTypes>
auto forward_async_call(void (Class::*pmf)(CallArgTypes...), Pointer&& ptr, ArgTypes&&... args)
-> decltype(std::bind(pmf, std::forward<Pointer>(ptr), call_arg_traits<CallArgTypes>::template forward_async<ArgTypes>(args)...))
{
	return std::bind(pmf, std::forward<Pointer>(ptr), call_arg_traits<CallArgTypes>::template forward_async<ArgTypes>(args)...);
}

}

namespace std {
	template <class T>
	struct is_bind_expression<vs::detail::bind_rvalue_helper<T>> : std::true_type{};
}
