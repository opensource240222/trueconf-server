#pragma once

#if !defined(__has_cpp_attribute)
#	define __has_cpp_attribute(x) 0
#endif

#if defined(__cplusplus) && __has_cpp_attribute(deprecated)
#	define VS_DEPRECATED [[deprecated]]
#elif defined(_MSC_VER)
#	if defined(__cplusplus) && _MSC_VER >= 1900
#		define VS_DEPRECATED [[deprecated]]
#	else
#		define VS_DEPRECATED __declspec(deprecated)
#	endif
#elif defined(__clang__)
#	if __has_attribute(deprecated)
#		define VS_DEPRECATED __attribute__ ((deprecated))
#	endif
#elif defined(__GNUC__)
#	define VS_DEPRECATED __attribute__ ((deprecated))
#endif
#ifndef VS_DEPRECATED
#	define VS_DEPRECATED
#endif

#if defined(__cplusplus) && __has_cpp_attribute(nodiscard)
#	define VS_NODISCARD [[nodiscard]]
#elif defined(_MSC_VER)
#	if defined(_MSVC_LANG) && _MSVC_LANG >= 201703L
#		define VS_NODISCARD [[nodiscard]]
#	elif _MSC_VER >= 1800
#		include <sal.h>
#		define VS_NODISCARD _Check_return_
#	endif
#elif defined(__clang__)
#	if __has_attribute(warn_unused_result)
#		define VS_NODISCARD __attribute__ ((warn_unused_result))
#	endif
#elif defined(__GNUC__)
#	define VS_NODISCARD __attribute__ ((warn_unused_result))
#endif
#ifndef VS_NODISCARD
#	define VS_NODISCARD
#endif

#if defined(__cplusplus) && __has_cpp_attribute(maybe_unused)
#	define VS_UNUSED [[maybe_unused]]
#elif defined(_MSC_VER)
#	if defined(_MSVC_LANG) && _MSVC_LANG >= 201703L
#		define VS_UNUSED [[maybe_unused]]
#	elif _MSC_VER >= 1900
#		define VS_UNUSED __pragma(warning(suppress:4100))
#	endif
#elif defined(__clang__)
#	if __has_attribute(unused)
#		define VS_UNUSED __attribute__ ((unused))
#	endif
#elif defined(__GNUC__)
#	define VS_UNUSED __attribute__ ((unused))
#endif
#ifndef VS_UNUSED
#	define VS_UNUSED
#endif

#if defined(__cplusplus) && __has_cpp_attribute(fallthrough)
#	define VS_FALLTHROUGH [[fallthrough]]
#elif defined(_MSC_VER)
#	if defined(_MSVC_LANG) && _MSVC_LANG >= 201703L
#		define VS_FALLTHROUGH [[fallthrough]]
#	endif
#elif defined(__cplusplus) && defined(__clang__)
#	define VS_FALLTHROUGH [[clang::fallthrough]]
#endif
#ifndef VS_FALLTHROUGH
#	define VS_FALLTHROUGH
#endif
