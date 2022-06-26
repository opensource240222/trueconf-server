#pragma once

#if defined(_DEBUG)
	#if defined(_WIN32)
		extern void __cdecl __debugbreak(void);
		#define debugbreak() __debugbreak()
	#else
		#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__APPLE__)
			#include <signal.h>
			#define debugbreak() ::raise(SIGTRAP)
		#elif defined(__GNUC__)
			#define debugbreak() __builtin_trap()
		#else
			#include <assert.h>
			#define debugbreak() assert(false)
		#endif
	#endif
#endif