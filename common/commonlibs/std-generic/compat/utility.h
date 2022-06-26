#pragma once

#include "std-generic/gcc_version.h"
#include "std-generic/libstdcxx_version.h"

#include <utility>

// std::integer_sequence and friends from C++14 are available since libc++ 3.5, libstdc++ 4.9 and MSVC ??.? (assuming 15.0 (VS 2017)).
// Note: libc++ versions prior to 3.7 had the same value for _LIBCPP_VERSION, so the check is imprecise.
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3500 && _LIBCPP_STD_VER > 11) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 40900 && __cplusplus > 201103L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1910)

namespace vs {
using std::integer_sequence;
using std::index_sequence;
using std::make_integer_sequence;
using std::make_index_sequence;
using std::index_sequence_for;
}

#else

namespace vs {

#if !defined(__has_builtin)
#	define __has_builtin(...) 0
#endif

template <class T, T... I>
struct integer_sequence
{
	static_assert(std::is_integral<T>::value, "integer_sequence<T, I...> requires T to be an integral type.");

	using value_type = T;
	static constexpr std::size_t size() noexcept { return sizeof...(I); }
};

// Clang 3.8 and GCC 8 have compiler intrinsics that can generate integer_sequence.
// We prefer them over our simple recursive TMP implementation.
#if __has_builtin(__make_integer_seq)
template <class T, T N>
using make_integer_sequence = __make_integer_seq<integer_sequence, T, N>;
#elif defined(GCC_VERSION) && GCC_VERSION >= 80000
template <class T, T N>
using make_integer_sequence = integer_sequence<T, __integer_pack(N)...>;
#else
namespace detail {
template <unsigned N, unsigned... I>
struct iseq
{
	template <class T>
	using type = typename iseq<N-1, N-1, I...>::template type<T>;
};
template <unsigned... I>
struct iseq<0, I...>
{
	template <class T>
	using type = integer_sequence<T, I...>;
};
}
template <class T, T N>
using make_integer_sequence = typename detail::iseq<N>::template type<T>;
#endif

template <std::size_t... I>
using index_sequence = integer_sequence<std::size_t, I...>;

template <std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

template <class... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;

}

#endif
