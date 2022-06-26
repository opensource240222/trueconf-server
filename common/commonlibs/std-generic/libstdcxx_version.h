#pragma once

// libstdc++ already has a version marco: __GLIBCXX__, but it is completely useless because it contains a release date.
// So a bugfix release for an older major GCC version will have a greater __GLIBCXX__ value that in a newer major GCC version.
// Some examples: 20140612 (GCC 4.7.4) > 20130531 (GCC 4.8.1); 20171010 (GCC 5.5) > 20160427 (GCC 6.1).

// To get definition of __GLIBCXX__ we need to include any C++ header, ciso646 is the smallest one.
#include <ciso646>

#if defined(__GLIBCXX__)
#	if defined(__clang__)
		// This sequence of checks is taken from Boost.Config
#		if __has_include(<charconv>)
#			define LIBSTDCXX_VERSION 80100
#		elif __has_include(<any>)
#			define LIBSTDCXX_VERSION 70100
#		elif __has_include(<experimental/memory_resource>)
#			define LIBSTDCXX_VERSION 60100
#		elif __has_include(<experimental/any>)
#			define LIBSTDCXX_VERSION 50100
#		elif __has_include(<shared_mutex>)
#			define LIBSTDCXX_VERSION 40900
#		elif __has_include(<ext/cmath>)
#			define LIBSTDCXX_VERSION 40800
#		elif __has_include(<scoped_allocator>)
#			define LIBSTDCXX_VERSION 40700
#		elif __has_include(<typeindex>)
#			define LIBSTDCXX_VERSION 40600
#		elif __has_include(<future>)
#			define LIBSTDCXX_VERSION 40500
#		elif  __has_include(<ratio>)
#			define LIBSTDCXX_VERSION 40400
#		elif __has_include(<array>)
#			define LIBSTDCXX_VERSION 40300
#		endif
#	elif defined(__GNUC__)
#		define LIBSTDCXX_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#	endif
#endif
