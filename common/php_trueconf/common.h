#ifndef PHP_TRUECONF_COMMON_H
#define PHP_TRUECONF_COMMON_H

#include "version.h"

/*
// Macro debug kit:
//   #pragma message(TC_PM(FOO))
#define TC_STRINGIZE_(x) #x
#define TC_STRINGIZE(x) TC_STRINGIZE_(x)
#define TC_PM(x) #x "=" TC_STRINGIZE(x)
*/

#define TC_EVAL_(macro, ...) macro __VA_ARGS__
#define TC_EVAL(macro, ...) TC_EVAL_(macro, (__VA_ARGS__))
#define TC_MAKE_VERSION_STRING4(a,b,c,d) #a "." #b "." #c "." #d

#define PHP_TRUECONF_VERSION TC_EVAL(TC_MAKE_VERSION_STRING4, PRODUCTVER)

// PHP uses register keyword in its headers, this doesn't work in C++17.
#if defined(__cplusplus) && __cplusplus >= 201703L
#	define register
#endif

#if defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

// Stupid PHP defines mocks with inline keyword.
// So we first include all that stupid headers that are doing that
// and then undef inline.
// (By the way, not fixing this will result in bizarre dllimport related errors inside c++ library.)
#include <zend.h>
#undef inline

#if defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif

#define _char(x) char *x = 0; size_t x##_len = 0

#endif
