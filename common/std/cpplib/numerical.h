#pragma once

#include <limits>
#include <type_traits>

// Performs less comparison between two numerical values with respect to their signedness
template <class L, class R>
inline typename std::enable_if<std::is_signed<L>::value && std::is_unsigned<R>::value, bool>::type numeric_less(L l, R r)
{
	return l < L(0) || static_cast<typename std::make_unsigned<L>::type>(l) < r;
}
template <class L, class R>
inline typename std::enable_if<std::is_unsigned<L>::value && std::is_signed<R>::value, bool>::type numeric_less(L l, R r)
{
	return r >= R(0) && l < static_cast<typename std::make_unsigned<R>::type>(r);
}
template <class L, class R>
inline typename std::enable_if<std::is_signed<L>::value && std::is_signed<R>::value, bool>::type numeric_less(L l, R r)
{
	return l < r;
}
template <class L, class R>
inline typename std::enable_if<std::is_unsigned<L>::value && std::is_unsigned<R>::value, bool>::type numeric_less(L l, R r)
{
	return l < r;
}

// Casts numerical value to another type properly bounding the result
template <class T, class U>
inline typename std::enable_if<std::numeric_limits<T>::is_specialized, T>::type clamp_cast(U x)
{
	if (numeric_less(x, std::numeric_limits<T>::min()))
		return std::numeric_limits<T>::min();
	if (numeric_less(std::numeric_limits<T>::max(), x))
		return std::numeric_limits<T>::max();
	return static_cast<T>(x);
}
