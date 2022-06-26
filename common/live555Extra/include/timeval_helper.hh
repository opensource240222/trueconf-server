#ifndef _TIMEVAL_HELPER_HH_
#define _TIMEVAL_HELPER_HH_

#include <NetCommon.h>

#include <chrono>
#include <ostream>
#include <iomanip>

inline timeval timeval_normalize(timeval x)
{
	return {x.tv_sec + x.tv_usec/1000000, x.tv_usec%1000000};
}

inline timeval operator+(timeval x)
{
	return x;
}

inline timeval operator-(timeval x)
{
	return timeval_normalize({-(x.tv_sec+1), -(x.tv_usec-1000000)});
}

inline timeval operator+(timeval l, timeval r)
{
	return timeval_normalize({l.tv_sec + r.tv_sec, l.tv_usec + r.tv_usec});
}

template <class Rep, class Period>
inline timeval operator+(timeval l, std::chrono::duration<Rep, Period> r)
{
	return timeval_normalize({
		l.tv_sec  + long(std::chrono::duration_cast<std::chrono::microseconds>(r).count()/1000000),
		l.tv_usec + long(std::chrono::duration_cast<std::chrono::microseconds>(r).count()%1000000)
	});
}

template <class T>
inline timeval operator-(timeval l, T r)
{
	return l+(-r);
}

template <class T>
inline timeval& operator+=(timeval& l, T r)
{
	return l = l+r;
}

template <class T>
inline timeval& operator-=(timeval& l, T r)
{
	return l = l-r;
}

inline bool operator==(timeval l, timeval r)
{
	return l.tv_sec == r.tv_sec && l.tv_usec == r.tv_usec;
}

inline bool operator!=(timeval l, timeval r)
{
	return !(l == r);
}

inline bool operator<(timeval l, timeval r)
{
	return l.tv_sec == r.tv_sec ? l.tv_usec < r.tv_usec : l.tv_sec < r.tv_sec;
}

inline bool operator>(timeval l, timeval r)
{
	return r < l;
}

inline bool operator<=(timeval l, timeval r)
{
	return !(l > r);
}

inline bool operator>=(timeval l, timeval r)
{
	return !(l < r);
}

inline std::chrono::microseconds timeval_as_duration(timeval x)
{
	return std::chrono::seconds(x.tv_sec)+std::chrono::microseconds(x.tv_usec);
}

template <class Rep, class Period>
inline bool operator<(timeval l, std::chrono::duration<Rep, Period> r)
{
	return timeval_as_duration(l) < r;
}

template <class Rep, class Period>
inline bool operator<(std::chrono::duration<Rep, Period> l, timeval r)
{
	return l < timeval_as_duration(r);
}

template <class Rep, class Period>
inline bool operator>(timeval l, std::chrono::duration<Rep, Period> r)
{
	return r < l;
}

template <class Rep, class Period>
inline bool operator>(std::chrono::duration<Rep, Period> l, timeval r)
{
	return r < l;
}

template <class Rep, class Period>
inline bool operator<=(timeval l, std::chrono::duration<Rep, Period> r)
{
	return !(l > r);
}

template <class Rep, class Period>
inline bool operator<=(std::chrono::duration<Rep, Period> l, timeval r)
{
	return !(l > r);
}

template <class Rep, class Period>
inline bool operator>=(timeval l, std::chrono::duration<Rep, Period> r)
{
	return !(l < r);
}

template <class Rep, class Period>
inline bool operator>=(std::chrono::duration<Rep, Period> l, timeval r)
{
	return !(l < r);
}

inline timeval abs(timeval x)
{
	if (x.tv_sec < 0)
		return -x;
	else
		return x;
}

template <class CharT, class Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& s, timeval x)
{
	if (x.tv_sec >= 0)
		s << x.tv_sec << "." << std::setw(6) << std::setfill('0') << x.tv_usec;
	else
		s << (x.tv_sec+1) << "." << std::setw(6) << std::setfill('0') << (1000000-x.tv_usec);
	return s;
}

#endif
