#pragma once

#include "std-generic/libstdcxx_version.h"

#include <unordered_set>

// std::erase_if from C++20 is available since libc++ 8.0, libstdc++ 9 and not yet alaivable in MSVC.
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 8000 && _LIBCPP_STD_VER > 17) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 90000 && __cplusplus > 201703L) \
 || 0

namespace vs {
using std::erase_if;
}

#else

namespace vs {

template <class K, class H, class P, class A, class Predicate>
void erase_if(std::unordered_set<K, H, P, A>& c, Predicate pred)
{
	for (auto it = c.begin(), last = c.end(); it != last; )
		if (pred(*it))
			it = c.erase(it);
		else
			++it;
}

template <class K, class H, class P, class A, class Predicate>
void erase_if(std::unordered_multiset<K, H, P, A>& c, Predicate pred)
{
	for (auto it = c.begin(), last = c.end(); it != last; )
		if (pred(*it))
			it = c.erase(it);
		else
			++it;
}

}

#endif
