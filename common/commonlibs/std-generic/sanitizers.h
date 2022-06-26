#pragma once

#if !defined(__has_attribute)
#	define __has_attribute(x) 0
#endif

#if __has_attribute(no_sanitize_address) || defined(__SANITIZE_ADDRESS__)
// In GCC attribute no_sanitize_address appeared in the same version as the __SANITIZE_ADDRESS__ macro.
#	define VS_NO_SANITIZE_ADDRESS __attribute__ ((no_sanitize_address))
#else
#	define VS_NO_SANITIZE_ADDRESS
#endif

// ASan doesn't work with exceptions on Windows, see https://github.com/google/sanitizers/issues/749
// The workaround is to disable ASan instrumentation in functions that catch exceptions at runtime.
#if defined(_WIN32)
#	define VS_ASAN_EXCEPTIONS_WORKAROUND VS_NO_SANITIZE_ADDRESS
#else
#	define VS_ASAN_EXCEPTIONS_WORKAROUND
#endif
