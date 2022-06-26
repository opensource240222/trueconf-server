#pragma once

#include "std-generic/libstdcxx_version.h"

#include <algorithm>

// TODO: is_permutation is missing

// std::equal and std::mismatch from C++14 are available since libc++ 3.4, libstdc++ 4.9 and MSVC 14.0 (VS 2015).
// Note: libc++ versions prior to 3.7 had the same value for _LIBCPP_VERSION, so the check is imprecise.
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3700 && _LIBCPP_STD_VER > 11) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 40900 && __cplusplus > 201103L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1900)

namespace vs {
using std::mismatch;
using std::equal;
}

#else

#include <type_traits>

namespace vs {

using std::mismatch;
template <class InputIterator1, class InputIterator2>
std::pair<InputIterator1, InputIterator2> mismatch(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2)
{
	for ( ; first1 != last1 && first2 != last2; (void)++first1, ++first2)
		if (!(*first1 == *first2))
			break;
	return { std::move(first1), std::move(first2) };
}
template <class InputIterator1, class InputIterator2, class BinaryPredicate>
std::pair<InputIterator1, InputIterator2> mismatch(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2, BinaryPredicate pred)
{
	for ( ; first1 != last1 && first2 != last2; (void)++first1, ++first2)
		if (!pred(*first1, *first2))
			break;
	return { std::move(first1), std::move(first2) };
}

using std::equal;
template <class InputIterator1, class InputIterator2>
bool equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2)
{
	for ( ; first1 != last1 && first2 != last2; (void)++first1, ++first2)
		if (!(*first1 == *first2))
			return false;
	return first1 == last1 && first2 == last2;
}
template <class InputIterator1, class InputIterator2, class BinaryPredicate>
bool equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2, BinaryPredicate pred)
{
	for ( ; first1 != last1 && first2 != last2; (void)++first1, ++first2)
		if (!pred(*first1, *first2))
			return false;
	return first1 == last1 && first2 == last2;
}

}

#endif
