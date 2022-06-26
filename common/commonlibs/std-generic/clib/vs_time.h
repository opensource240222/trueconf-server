#pragma once

#if defined(__cplusplus)
#	include <ctime>
#else
#	include <time.h>
#endif

#if defined(_WIN32)

inline struct tm *gmtime_r(const time_t *time, struct tm *result)
{
	struct tm *p = gmtime(time);
	if (!p)
		return p;
	*result = *p;
	return result;
}

inline struct tm *localtime_r(const time_t *time, struct tm *result)
{
	struct tm *p = localtime(time);
	if (!p)
		return p;
	*result = *p;
	return result;
}

#endif
