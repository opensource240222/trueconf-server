#pragma once

#include <tuple>

// std::apply from C++17 is available since libc++ 3.9 libstdc++ 7 and MSVC 14.1 (VS 2017).
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3900 && _LIBCPP_STD_VER > 14) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 70000 && __cplusplus > 201402L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1910 && _MSVC_LANG > 201402L)

namespace vs
{
using std::apply;
} //namespace vs

#else

#include "utility.h"
#include "functional.h"

namespace vs
{
namespace detail
{
template <class F, class Tuple, std::size_t... I>
constexpr auto apply_impl(F &&f, Tuple &&t, std::index_sequence<I...>)
	-> decltype(vs::invoke(vs::detail::forward<F>(f), std::get<I>(vs::detail::forward<Tuple>(t))...))
{
	return vs::invoke(vs::detail::forward<F>(f), std::get<I>(vs::detail::forward<Tuple>(t))...);
}
} // namespace detail

template <class F, class Tuple>
constexpr auto apply(F &&f, Tuple &&t)
	-> decltype(detail::apply_impl(
		vs::detail::forward<F>(f), vs::detail::forward<Tuple>(t),
		vs::make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>{}))
{
	return detail::apply_impl(
		vs::detail::forward<F>(f), vs::detail::forward<Tuple>(t),
		vs::make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>{});
}
} //namespace vs

#endif