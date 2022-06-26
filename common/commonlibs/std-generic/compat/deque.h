#pragma once

#include "std-generic/libstdcxx_version.h"

#include <deque>

// std::erase{,_if} from C++20 are available since libc++ 8.0, libstdc++ 9 and not yet alaivable in MSVC.
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 8000 && _LIBCPP_STD_VER > 17) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 90000 && __cplusplus > 201703L) \
 || 0

namespace vs {
using std::erase;
using std::erase_if;
}

#else

#include <algorithm>

namespace vs {

template<class T, class Allocator, class U>
void erase(std::deque<T, Allocator>& c, const U& value)
{
	c.erase(std::remove(c.begin(), c.end(), value), c.end());
}

template<class T, class Allocator, class Predicate>
void erase_if(std::deque<T, Allocator>& c, Predicate pred)
{
	c.erase(std::remove_if(c.begin(), c.end(), pred), c.end());
}

}

#endif
