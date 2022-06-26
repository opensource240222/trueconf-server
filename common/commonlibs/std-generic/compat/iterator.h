#pragma once

#include "std-generic/libstdcxx_version.h"

#include <iterator>

// std::{c,r,cr}{begin,end} from C++14 are available since libc++ 3.4, libstdc++ 5 and MSVC 14.0 (VS 2015).
// Note: libc++ versions prior to 3.7 had the same value for _LIBCPP_VERSION, so the check is imprecise.
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3700 && _LIBCPP_STD_VER > 11) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 50000 && __cplusplus > 201103L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1900)

namespace vs {
using std::rbegin;
using std::rend;
using std::cbegin;
using std::cend;
using std::crbegin;
using std::crend;
}

#else

namespace vs {

template <class C> constexpr auto rbegin(C& c)
-> decltype(c.rbegin())
{ return   (c.rbegin()); }
template <class C> constexpr auto rbegin(const C& c)
-> decltype(c.rbegin())
{ return   (c.rbegin()); }

template <class C> constexpr auto rend(C& c)
-> decltype(c.rend())
{ return   (c.rend()); }
template <class C> constexpr auto rend(const C& c)
-> decltype(c.rend())
{ return   (c.rend()); }

template <class T, size_t N> constexpr std::reverse_iterator<T*> rbegin(T (&array)[N])
{ return std::reverse_iterator<T*>(array + N); }

template <class T, size_t N> constexpr std::reverse_iterator<T*> rend(T (&array)[N])
{ return std::reverse_iterator<T*>(array); }

template <class E> constexpr std::reverse_iterator<const E*> rbegin(std::initializer_list<E> il)
{ return std::reverse_iterator<const E*>(il.end()); }

template <class E> constexpr std::reverse_iterator<const E*> rend(std::initializer_list<E> il)
{ return std::reverse_iterator<const E*>(il.begin()); }

template <class C> constexpr auto cbegin(const C& c)
noexcept(noexcept(std::begin(c)))
-> decltype      (std::begin(c))
{ return         (std::begin(c)); }

template <class C> constexpr auto cend(const C& c)
noexcept(noexcept(std::end(c)))
-> decltype      (std::end(c))
{ return         (std::end(c)); }

template <class C> constexpr auto crbegin(const C& c)
noexcept(noexcept(vs::rbegin(c)))
-> decltype      (vs::rbegin(c))
{ return         (vs::rbegin(c)); }

template <class C> constexpr auto crend(const C& c)
noexcept(noexcept(vs::rend(c)))
-> decltype      (vs::rend(c))
{ return         (vs::rend(c)); }

}

#endif

// std::{size,empty,data} from C++17 are available since libc++ 3.6, libstdc++ 6 and MSVC 14.0 (VS 2015).
// Note: libc++ versions prior to 3.7 had the same value for _LIBCPP_VERSION, so the check is imprecise.
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3700 && _LIBCPP_STD_VER > 14) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 60000 && __cplusplus > 201402L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1900)

namespace vs {
using std::size;
using std::empty;
using std::data;
}

#else

namespace vs {

template <class C> constexpr auto size(const C& c)
-> decltype(c.size())
{ return   (c.size()); }

template <class T, size_t N> constexpr size_t size(const T (&)[N]) noexcept
{ return N; }

template <class C> constexpr auto empty(const C& c)
-> decltype(c.empty())
{ return   (c.empty()); }

template <class T, size_t N> constexpr bool empty(const T (&)[N]) noexcept
{ return N == 0; }

template <class E> constexpr bool empty(std::initializer_list<E> il) noexcept
{ return il.size() == 0; }

template <class C> constexpr auto data(C& c)
-> decltype(c.data())
{ return   (c.data()); }
template <class C> constexpr auto data(const C& c)
-> decltype(c.data())
{ return   (c.data()); }

template <class T, size_t N> constexpr T* data(T (&array)[N]) noexcept
{ return array; }

template <class E> constexpr const E* data(std::initializer_list<E> il) noexcept
{ return il.begin(); }

}

#endif

// std::iter_*_t and std::incrementable_traits from C++20 are available since libstdc++ 10 and and yet alaivable in libstdc++ and MSVC.
#if 0 \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 100000 && __cplusplus > 201703L) \
 || 0

namespace vs {
using std::incrementable_traits;
using std::iter_reference_t;
using std::iter_difference_t;
// Not importing iter_value_t, iter_rvalue_reference_t and iter_common_reference_t because we don't need them yet.
}

#else

#include "std-generic/compat/type_traits.h"

namespace vs {

namespace detail {
template <class T, class = void> struct has_difference_type : std::false_type {};
template <class T> struct has_difference_type<T, void_t<typename T::difference_type>> : std::true_type {};
}

template <class T, class = void> struct incrementable_traits {};
template <class T> struct incrementable_traits<T*, typename std::enable_if<std::is_object<T>::value>::type>
{
	using difference_type = std::ptrdiff_t;
};
template <class T> struct incrementable_traits<const T> : incrementable_traits<T> {};
template <class T> struct incrementable_traits<T, void_t<typename T::difference_type>>
{
	using difference_type = typename T::difference_type;
};
template <class T> struct incrementable_traits<T, typename std::enable_if<
	!detail::has_difference_type<T>::value
	&& std::is_integral<decltype(std::declval<T>() - std::declval<T>())>::value
	// This is require to disambiguate this partial specialization with a partial specialization for pointers.
	// It isn't present in the standard because there concepts are used instead of SFINAE.
	&& !std::is_pointer<T>::value
>::type>
{
	using difference_type = typename std::make_signed<decltype(std::declval<T>() - std::declval<T>())>::type;
};

template <class T>
using iter_reference_t = decltype(*std::declval<T&>());

template <class T>
using iter_difference_t = typename std::conditional<
	// Note: Actual condition is "std::iterator_traits<T> names a specialization generated from the primary template"
	!detail::has_difference_type<std::iterator_traits<T>>::value,
	typename incrementable_traits<T>::difference_type,
	typename std::iterator_traits<T>::difference_type
>::type;

}

#endif
