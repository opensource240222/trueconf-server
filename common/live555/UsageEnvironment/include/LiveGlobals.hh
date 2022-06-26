#ifndef _LIVE_GLOBALS_HH
#define _LIVE_GLOBALS_HH

#if defined(_MSC_VER)
# if defined(LIVE_EXPORTS)
#  define LIVE_API __declspec(dllexport)
# elif defined(LIVE_DLL)
#  define LIVE_API __declspec(dllimport)
# endif
#elif defined(__GNUC__) || defined(__clang__)
# if defined(LIVE_EXPORTS)
#  define LIVE_API __attribute__((visibility("default")))
# endif
#endif
#if !defined(LIVE_API)
# define LIVE_API
#endif

#endif
