#pragma once

#include <stddef.h>

#if defined(_WIN32)
#	define strcasecmp vs_strcasecmp
#	define strncasecmp vs_strncasecmp
#elif defined(__unix__)
#	include <strings.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Custom strcasecmp implementation to use on Windows.
// Windows has _stricmp and _strnicmp, but they are slower than this code at least by 50%.
// Please don't call these functions directly, use POSIX names instead: strcasecmp and strncasecmp.

inline int vs_strcasecmp(const char* l, const char* r)
{
	while (1)
	{
		if (*l != *r)
		{
			const unsigned char cl = (*l >= 'A' && *l <= 'Z') ? *l - 'A' + 'a' : *l;
			const unsigned char cr = (*r >= 'A' && *r <= 'Z') ? *r - 'A' + 'a' : *r;
			if (cl != cr)
				return cl - cr;
		}
		else if (*l == '\0')
			return 0;
		++l;
		++r;
	}
}

inline int vs_strncasecmp(const char* l, const char* r, size_t count)
{
	while (count--)
	{
		if (*l != *r)
		{
			const unsigned char cl = (*l >= 'A' && *l <= 'Z') ? *l - 'A' + 'a' : *l;
			const unsigned char cr = (*r >= 'A' && *r <= 'Z') ? *r - 'A' + 'a' : *r;
			if (cl != cr)
				return cl - cr;
		}
		else if (*l == '\0')
			return 0;
		++l;
		++r;
	}
	return 0;
}

#ifdef __cplusplus
}
#endif
