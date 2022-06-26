#pragma once

#include "std-generic/libstdcxx_version.h"

#include <iomanip>

// std::put_time from C++11 is available since libc++ 3.3, libstdc++ 5 and MSVC ??.?.
#if  defined(_LIBCPP_VERSION) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 50000) \
 ||  defined(_MSC_VER)

namespace vs {
using std::put_time;
}

#else

namespace vs {
std::string put_time(const std::tm* time, const char* fmt);
}

#endif
