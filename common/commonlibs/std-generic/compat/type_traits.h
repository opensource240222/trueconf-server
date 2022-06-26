#pragma once

#include "std-generic/libstdcxx_version.h"

#include <type_traits>

// std::void_t from C++17 is available since libc++ 3.6, libstdc++ 6 and MSVC 14.0 (VS 2015).
// Note: libc++ versions prior to 3.7 had the same value for _LIBCPP_VERSION, so the check is imprecise.
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3700 && _LIBCPP_STD_VER > 14) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 60000 && __cplusplus > 201402L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1900)

namespace vs {
using std::void_t;
}

#else

namespace vs {
template <class...>
using void_t = void;
}

#endif

// std::type_identity_t from C++20 is available since libc++ 8, libstdc++ 9 and MSVC 14.21 (VS 2019).
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 8000 && _LIBCPP_STD_VER > 17) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 90000 && __cplusplus > 201703L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1921 && _MSVC_LANG > 201703L)

namespace vs {
using std::type_identity;
using std::type_identity_t;
}

#else

namespace vs {
template <class T> struct type_identity { using type = T; };
template <class T> using type_identity_t = typename type_identity<T>::type;
}

#endif

namespace vs {
namespace detail {

template <class Compare, class, class = void_t<>>
struct is_transparent : std::false_type {};
template <class Compare, class X>
struct is_transparent<Compare, X, void_t<typename Compare::is_transparent>> : std::true_type {};

}
}

// std::conjunction/std::disjunction/std::negation from C++17 are available since libc++ 3.8, libstdc++ 6 and MSVC 140 (VS 14.?).
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3800 && _LIBCPP_STD_VER > 14) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 60000 && __cplusplus > 201402L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1900)

namespace vs {
using std::conjunction;
using std::disjunction;
using std::negation;
}

#else

namespace vs {
//logical AND metafunction
template <class...> 
struct conjunction : std::true_type { };
template <class B1> 
struct conjunction<B1> : B1 { };
template <class B1, class... Bn>
struct conjunction<B1, Bn...> : std::conditional<bool(B1::value), conjunction<Bn...>, B1>::type {};
//logical OR metafunction
template <class...> 
struct disjunction : std::false_type { };
template <class B1> 
struct disjunction<B1> : B1 { };
template <class B1, class... Bn>
struct disjunction<B1, Bn...> : std::conditional<bool(B1::value), B1, disjunction<Bn...>>::type { };
//logical NOT metafunction
template <class B>
struct negation : std::integral_constant<bool, !bool(B::value)> { };
}

#endif