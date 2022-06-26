#pragma once

#include <cstdlib>
#include <cstring>
#include <cwchar>

#include "std-generic/cpplib/string_view.h"

// nullptr-aware wrappers for variaous integer conversion functions

// strto*
inline          long      strtol_s  (const char* str, char** str_end, int base)
{
	if (str) return ::strtol(str, str_end, base);
	if (str_end) *str_end = nullptr;
	return 0l;
}
inline          long long strtoll_s (const char* str, char** str_end, int base)
{
	if (str) return ::strtoll(str, str_end, base);
	if (str_end) *str_end = nullptr;
	return 0ll;
}
inline unsigned long      strtoul_s (const char* str, char** str_end, int base)
{
	if (str) return ::strtoul(str, str_end, base);
	if (str_end) *str_end = nullptr;
	return 0ul;
}
inline unsigned long long strtoull_s(const char* str, char** str_end, int base)
{
	if (str) return ::strtoull(str, str_end, base);
	if (str_end) *str_end = nullptr;
	return 0ull;
}
inline float       strtof_s(const char* str, char** str_end)
{
	if (str) return ::strtof(str, str_end);
	if (str_end) *str_end = nullptr;
	return 0.;
}
inline double      strtod_s(const char* str, char** str_end)
{
	if (str) return ::strtod(str, str_end);
	if (str_end) *str_end = nullptr;
	return 0.;
}
inline long double strtold_s(const char* str, char** str_end)
{
	if (str) return ::strtold(str, str_end);
	if (str_end) *str_end = nullptr;
	return 0.;
}

// Non-standard versions of ato* for unsigned types
inline unsigned int       atou  (const char* str) { return static_cast<unsigned int>       (::atoi (str)); }
inline unsigned long      atoul (const char* str) { return static_cast<unsigned long>      (::atol (str)); }
inline unsigned long long atoull(const char* str) { return static_cast<unsigned long long >(::atoll(str)); }

// ato*
inline int       atoi_s (const char* str) { return str ? ::atoi (str) : 0;   }
inline long      atol_s (const char* str) { return str ? ::atol (str) : 0l;  }
inline long long atoll_s(const char* str) { return str ? ::atoll(str) : 0ll; }
inline double    atof_s (const char* str) { return str ? ::atof (str) : 0.;  }

inline unsigned int       atou_s  (const char* str) { return str ? ::atou  (str) : 0u;   }
inline unsigned long      atoul_s (const char* str) { return str ? ::atoul (str) : 0ul;  }
inline unsigned long long atoull_s(const char* str) { return str ? ::atoull(str) : 0ull; }

// wcsto*
inline          long      wcstol_s  (const wchar_t* str, wchar_t** str_end, int base)
{
	if (str) return ::wcstol(str, str_end, base);
	if (str_end) *str_end = nullptr;
	return 0l;
}
inline          long long wcstoll_s (const wchar_t* str, wchar_t** str_end, int base)
{
	if (str) return ::wcstoll(str, str_end, base);
	if (str_end) *str_end = nullptr;
	return 0ll;
}
inline unsigned long      wcstoul_s (const wchar_t* str, wchar_t** str_end, int base)
{
	if (str) return ::wcstoul(str, str_end, base);
	if (str_end) *str_end = nullptr;
	return 0ul;
}
inline unsigned long long wcstoull_s(const wchar_t* str, wchar_t** str_end, int base)
{
	if (str) return ::wcstoull(str, str_end, base);
	if (str_end) *str_end = nullptr;
	return 0ull;
}
inline float       wcstof_s(const wchar_t* str, wchar_t** str_end)
{
	if (str) return ::wcstof(str, str_end);
	if (str_end) *str_end = nullptr;
	return 0.;
}
inline double      wcstod_s(const wchar_t* str, wchar_t** str_end)
{
	if (str) return ::wcstod(str, str_end);
	if (str_end) *str_end = nullptr;
	return 0.;
}
inline long double wcstold_s(const wchar_t* str, wchar_t** str_end)
{
	if (str) return ::wcstold(str, str_end);
	if (str_end) *str_end = nullptr;
	return 0.;
}

// Non-standard wtoi, wtol, wtoll, wtof
inline int       wtoi (const wchar_t* str) { return static_cast<int>(::wcstol(str, nullptr, 10)); }
inline long      wtol (const wchar_t* str) { return ::wcstol(str, nullptr, 10); }
inline long long wtoll(const wchar_t* str) { return ::wcstoll(str, nullptr, 10); }
inline double    wtof (const wchar_t* str) { return ::wcstod(str, nullptr); }

// Non-standard versions of wto* for unsigned types
inline unsigned int       wtou  (const wchar_t* str) { return static_cast<unsigned int>       (::wtoi (str)); }
inline unsigned long      wtoul (const wchar_t* str) { return static_cast<unsigned long>      (::wtol (str)); }
inline unsigned long long wtoull(const wchar_t* str) { return static_cast<unsigned long long >(::wtoll(str)); }

// wto*
inline int       wtoi_s (const wchar_t* str) { return str ? ::wtoi (str) : 0;   }
inline long      wtol_s (const wchar_t* str) { return str ? ::wtol (str) : 0l;  }
inline long long wtoll_s(const wchar_t* str) { return str ? ::wtoll(str) : 0ll; }
inline double    wtof_s (const wchar_t* str) { return str ? ::wtof (str) : 0.;  }

inline unsigned int       wtou_s  (const wchar_t* str) { return str ? ::wtou  (str) : 0u;   }
inline unsigned long      wtoul_s (const wchar_t* str) { return str ? ::wtoul (str) : 0ul;  }
inline unsigned long long wtoull_s(const wchar_t* str) { return str ? ::wtoull(str) : 0ull; }
