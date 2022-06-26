#include "std-generic/compat/iomanip.h"

#include <cstring>
#include <ctime>

#if  defined(_LIBCPP_VERSION) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 50000) \
 ||  defined(_MSC_VER)
#else

namespace vs {

std::string put_time(const std::tm* time, const char* fmt)
{
	std::string result;
	result.resize(std::strlen(fmt) + 48);
	const auto length = std::strftime(&result[0], result.size(), fmt, time);
	result.resize(length);
	return result;
}

}

#endif
