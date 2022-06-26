#pragma once

#include "std-generic/libstdcxx_version.h"

#include <chrono>

// std::chrono::{floor,ceil,round,abs} from C++17 are available since libc++ 3.8, libstdc++ 7 and MSVC 14.1 (VS 2017).
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3800 && _LIBCPP_STD_VER > 14) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 70000 && __cplusplus > 201402L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1910)

namespace vs {
namespace chrono {
using std::chrono::floor;
using std::chrono::ceil;
using std::chrono::round;
using std::chrono::abs;
}
}

#else

#include <type_traits>

#if __cplusplus >= 201402L || (defined(_MSC_VER) && _MSC_VER >= 1910)
#	define constexpr_14 constexpr
#else
#	define constexpr_14
#endif

namespace vs {

namespace detail {
template <class T>
struct is_duration : std::false_type {};
template <class Rep, class Period>
struct is_duration<std::chrono::duration<Rep, Period>> : std::true_type {};
}

namespace chrono {

template <class ToDuration, class Rep, class Period, class = typename std::enable_if<
	detail::is_duration<ToDuration>::value
>::type>
constexpr_14 ToDuration floor(const std::chrono::duration<Rep, Period>& d)
{
    ToDuration t = std::chrono::duration_cast<ToDuration>(d);
    if (t > d)
        t -= ToDuration(1);
	return t;
}

template <class ToDuration, class Clock, class Duration, class = typename std::enable_if<
	detail::is_duration<ToDuration>::value
>::type>
constexpr_14 std::chrono::time_point<Clock, ToDuration> floor(const std::chrono::time_point<Clock, Duration>& tp)
{
	return std::chrono::time_point<Clock, ToDuration>(vs::chrono::floor<ToDuration>(tp.time_since_epoch()));
}

template <class ToDuration, class Rep, class Period, class = typename std::enable_if<
	detail::is_duration<ToDuration>::value
>::type>
constexpr_14 ToDuration ceil(const std::chrono::duration<Rep, Period>& d)
{
    ToDuration t = std::chrono::duration_cast<ToDuration>(d);
    if (t < d)
        t += ToDuration(1);
	return t;
}

template <class ToDuration, class Clock, class Duration, class = typename std::enable_if<
	detail::is_duration<ToDuration>::value
>::type>
constexpr_14 std::chrono::time_point<Clock, ToDuration> ceil(const std::chrono::time_point<Clock, Duration>& tp)
{
	return std::chrono::time_point<Clock, ToDuration>(vs::chrono::ceil<ToDuration>(tp.time_since_epoch()));
}

template <class ToDuration, class Rep, class Period, class = typename std::enable_if<
	detail::is_duration<ToDuration>::value &&
	!std::chrono::treat_as_floating_point<typename ToDuration::rep>::value
>::type>
constexpr_14 ToDuration round(const std::chrono::duration<Rep, Period>& d)
{
	ToDuration t0 = floor<ToDuration>(d);
	ToDuration t1 = t0 + ToDuration(1);
	const auto diff0 = d - t0;
	const auto diff1 = t1 - d;
	if (diff0 == diff1)
	{
		if (t0.count() & 1)
			return t1;
		else
			return t0;
	}
	else
	{
		if (diff0 < diff1)
			return t0;
		else
			return t1;
	}
}

template <class ToDuration, class Clock, class Duration, class = typename std::enable_if<
	detail::is_duration<ToDuration>::value &&
	!std::chrono::treat_as_floating_point<typename ToDuration::rep>::value
>::type>
constexpr_14 std::chrono::time_point<Clock, ToDuration> round(const std::chrono::time_point<Clock, Duration>& tp)
{
	return std::chrono::time_point<Clock, ToDuration>(vs::chrono::round<ToDuration>(tp.time_since_epoch()));
}

template <class Rep, class Period, class = typename std::enable_if<
	(std::chrono::duration<Rep, Period>::min() < std::chrono::duration<Rep, Period>::zero())
>::type>
constexpr std::chrono::duration<Rep, Period> abs(std::chrono::duration<Rep, Period> d)
{
    return d >= d.zero() ? d : -d;
}

}
}

#undef constexpr_14

#endif
