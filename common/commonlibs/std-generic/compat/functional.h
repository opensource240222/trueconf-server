#pragma once

#include "std-generic/libstdcxx_version.h"

#include <functional>

// Comparison functors with specializations for transparent comparison from C++14 are available since libc++ 3.4, libstdc++ 4.9 and MSVC 14.0 (VS 2015).
// Note: libc++ versions prior to 3.7 had the same value for _LIBCPP_VERSION, so the check is imprecise.
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3700 && _LIBCPP_STD_VER > 11) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 40900 && __cplusplus > 201103L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1900)

namespace vs {
using std::less;
using std::less_equal;
using std::greater;
using std::greater_equal;
}

#else

namespace vs {

template <class T = void>
struct less : std::binary_function<T, T, bool>
{
	constexpr bool operator()(const T& l, const T& r) const { return l < r; };
};
template <>
struct less<void>
{
	using is_transparent = int;
	template <class L, class R>
	constexpr auto operator()(L&& l, R&& r) const
	noexcept(noexcept(std::forward<L>(l) < std::forward<R>(r)))
	->       decltype(std::forward<L>(l) < std::forward<R>(r))
	{          return(std::forward<L>(l) < std::forward<R>(r)); }
};

template <class T = void>
struct less_equal : std::binary_function<T, T, bool>
{
	constexpr bool operator()(const T& l, const T& r) const { return l <= r; };
};
template <>
struct less_equal<void>
{
	using is_transparent = int;
	template <class L, class R>
	constexpr auto operator()(L&& l, R&& r) const
	noexcept(noexcept(std::forward<L>(l) <= std::forward<R>(r)))
	->       decltype(std::forward<L>(l) <= std::forward<R>(r))
	{          return(std::forward<L>(l) <= std::forward<R>(r)); }
};

template <class T = void>
struct greater : std::binary_function<T, T, bool>
{
	constexpr bool operator()(const T& l, const T& r) const { return l > r; };
};
template <>
struct greater<void>
{
	using is_transparent = int;
	template <class L, class R>
	constexpr auto operator()(L&& l, R&& r) const
	noexcept(noexcept(std::forward<L>(l) > std::forward<R>(r)))
	->       decltype(std::forward<L>(l) > std::forward<R>(r))
	{          return(std::forward<L>(l) > std::forward<R>(r)); }
};

template <class T = void>
struct greater_equal : std::binary_function<T, T, bool>
{
	constexpr bool operator()(const T& l, const T& r) const { return l >= r; };
};
template <>
struct greater_equal<void>
{
	using is_transparent = int;
	template <class L, class R>
	constexpr auto operator()(L&& l, R&& r) const
	noexcept(noexcept(std::forward<L>(l) >= std::forward<R>(r)))
	->       decltype(std::forward<L>(l) >= std::forward<R>(r))
	{          return(std::forward<L>(l) >= std::forward<R>(r)); }
};

}

#endif

namespace vs
{
namespace detail
{
template <class T>
constexpr T &&forward(typename std::remove_reference<T>::type &arg) noexcept
{ // forward an lvalue as either an lvalue or an rvalue
	return (static_cast<T &&>(arg));
}
	
template <class T>
constexpr T &&forward(typename std::remove_reference<T>::type &&arg) noexcept
{ // forward an rvalue as an rvalue
	static_assert(!std::is_lvalue_reference<T>::value, "bad forward call");
	return (static_cast<T &&>(arg));
}
} //namespace detail
} //namespace vs 

// std::invoke from C++17 is available since libc++ 3.7 libstdc++ 6 and MSVC 14.1 (VS 2017).
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3700 && _LIBCPP_STD_VER > 14) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 60000 && __cplusplus > 201402L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1910 && _MSVC_LANG > 201402L)

namespace vs
{
using std::invoke;
} //namespace vs

#else
namespace vs
{
namespace detail
{
template <class T>
struct is_reference_wrapper final : std::false_type
{
};
template <class U>
struct is_reference_wrapper<std::reference_wrapper<U>> final : std::true_type
{
};

template <class Base, class T, class Derived, class... Args>
constexpr auto INVOKE(T Base::*pmf, Derived &&ref, Args &&... args) noexcept(noexcept((vs::detail::forward<Derived>(ref).*pmf)(vs::detail::forward<Args>(args)...)))
	-> typename std::enable_if<std::is_function<T>::value &&
	                           std::is_base_of<Base, typename std::decay<Derived>::type>::value,
	decltype((vs::detail::forward<Derived>(ref).*pmf)(vs::detail::forward<Args>(args)...))>::type
{
	return (vs::detail::forward<Derived>(ref).*pmf)(vs::detail::forward<Args>(args)...);
}

template <class Base, class T, class RefWrap, class... Args>
constexpr auto INVOKE(T Base::*pmf, RefWrap &&ref, Args &&... args) noexcept(noexcept((ref.get().*pmf)(vs::detail::forward<Args>(args)...)))
	-> typename std::enable_if<std::is_function<T>::value &&
	                           is_reference_wrapper<typename std::decay<RefWrap>::type>::value,
	decltype((ref.get().*pmf)(vs::detail::forward<Args>(args)...))>::type

{
	return (ref.get().*pmf)(vs::detail::forward<Args>(args)...);
}

template <class Base, class T, class Pointer, class... Args>
constexpr auto INVOKE(T Base::*pmf, Pointer &&ptr, Args &&... args) noexcept(noexcept(((*vs::detail::forward<Pointer>(ptr)).*pmf)(vs::detail::forward<Args>(args)...)))
	-> typename std::enable_if<std::is_function<T>::value &&
	                           !is_reference_wrapper<typename std::decay<Pointer>::type>::value &&
	                           !std::is_base_of<Base, typename std::decay<Pointer>::type>::value,
	decltype(((*vs::detail::forward<Pointer>(ptr)).*pmf)(vs::detail::forward<Args>(args)...))>::type
{
	return ((*vs::detail::forward<Pointer>(ptr)).*pmf)(vs::detail::forward<Args>(args)...);
}

template <class Base, class T, class Derived>
constexpr auto INVOKE(T Base::*pmd, Derived &&ref) noexcept(noexcept(vs::detail::forward<Derived>(ref).*pmd))
	-> typename std::enable_if<!std::is_function<T>::value &&
	                           std::is_base_of<Base, typename std::decay<Derived>::type>::value,
	decltype(vs::detail::forward<Derived>(ref).*pmd)>::type
{
	return vs::detail::forward<Derived>(ref).*pmd;
}

template <class Base, class T, class RefWrap>
constexpr auto INVOKE(T Base::*pmd, RefWrap &&ref) noexcept(noexcept(ref.get().*pmd))
	-> typename std::enable_if<!std::is_function<T>::value &&
	                           is_reference_wrapper<typename std::decay<RefWrap>::type>::value,
	decltype(ref.get().*pmd)>::type
{
	return ref.get().*pmd;
}

template <class Base, class T, class Pointer>
constexpr auto INVOKE(T Base::*pmd, Pointer &&ptr) noexcept(noexcept((*vs::detail::forward<Pointer>(ptr)).*pmd))
	-> typename std::enable_if<!std::is_function<T>::value &&
	                           !is_reference_wrapper<typename std::decay<Pointer>::type>::value &&
	                           !std::is_base_of<Base, typename std::decay<Pointer>::type>::value,
	decltype((*vs::detail::forward<Pointer>(ptr)).*pmd)>::type
{
	return (*vs::detail::forward<Pointer>(ptr)).*pmd;
}

template <class F, class... Args>
constexpr auto INVOKE(F &&f, Args &&... args) noexcept(noexcept(vs::detail::forward<F>(f)(vs::detail::forward<Args>(args)...)))
	-> typename std::enable_if<!std::is_member_pointer<typename std::decay<F>::type>::value,
							   decltype(vs::detail::forward<F>(f)(vs::detail::forward<Args>(args)...))>::type
{
	return vs::detail::forward<F>(f)(vs::detail::forward<Args>(args)...);
}

} // namespace detail

template <class F, class... ArgTypes>
constexpr auto invoke(F &&f, ArgTypes &&... args) noexcept(noexcept(detail::INVOKE(vs::detail::forward<F>(f), vs::detail::forward<ArgTypes>(args)...)))
	-> decltype(detail::INVOKE(vs::detail::forward<F>(f), vs::detail::forward<ArgTypes>(args)...))
{
	return detail::INVOKE(vs::detail::forward<F>(f), vs::detail::forward<ArgTypes>(args)...);
}

} //namespace vs
#endif