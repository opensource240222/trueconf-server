#pragma once

#include "std-generic/libstdcxx_version.h"

#include "std-generic/compat/type_traits.h"
#include <memory>

namespace vs {
namespace detail {

template <class T>
struct make_unique_result
{
	using object = std::unique_ptr<T>;
};
template <class T>
struct make_unique_result<T[]>
{
	using array = std::unique_ptr<T[]>;
};
template <class T, std::size_t size>
struct make_unique_result<T[size]>
{
	using invalid = void;
};

}
}

// std::make_unique from C++14 is available since libc++ 3.4, libstdc++ 4.9 and MSVC ??.?.
// Note: libc++ versions prior to 3.7 had the same value for _LIBCPP_VERSION, so the check is imprecise.
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3700 && _LIBCPP_STD_VER > 11) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 40900 && __cplusplus > 201103L) \
 ||  defined(_MSC_VER)

namespace vs {
using std::make_unique;
}

#else

namespace vs {

template <class T, class... Args>
typename detail::make_unique_result<T>::object make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T>
typename detail::make_unique_result<T>::array make_unique(std::size_t size)
{
	return std::unique_ptr<T>(new typename std::remove_extent<T>::type[size]());
}

template <class T, class... Args>
typename detail::make_unique_result<T>::invalid make_unique(Args&&...) = delete;

}

#endif

// std::make_unique_default_init from C++20 is not available in any standard library yet.

namespace vs {

template <class T>
typename detail::make_unique_result<T>::object make_unique_default_init()
{
    return std::unique_ptr<T>(new T);
}

template <class T>
typename detail::make_unique_result<T>::array make_unique_default_init(std::size_t size)
{
	return std::unique_ptr<T>(new typename std::remove_extent<T>::type[size]);
}

template <class T, class... Args>
typename detail::make_unique_result<T>::invalid make_unique_default_init(Args&&...) = delete;

}

// std::enable_shared_from_this::weak_from_this() from C++17 is available since libc++ 3.9, libstdc++ 7 and MSVC 14.?.
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 3900 && _LIBCPP_STD_VER > 14) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 70000 && __cplusplus > 201402L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1914)

namespace vs {
using std::enable_shared_from_this;
}

#else

namespace vs {

template <class T>
class enable_shared_from_this : public std::enable_shared_from_this<T>
{
public:
	std::weak_ptr<T>       weak_from_this()       { return this->shared_from_this(); }
	std::weak_ptr<const T> weak_from_this() const { return this->shared_from_this(); }
};

}

#endif

// std::to_address from C++20 is available since libc++ 6, libstdc++ 8 and MSVC 14.22.
#if (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 6000 && _LIBCPP_STD_VER > 17) \
 || (defined(LIBSTDCXX_VERSION) && LIBSTDCXX_VERSION >= 80000 && __cplusplus > 201703L) \
 || (defined(_MSC_VER) && _MSC_VER >= 1922 && _MSVC_LANG > 201703L)

namespace vs {
using std::to_address;
}

#else

namespace vs {

namespace detail {
template <class T, class = void> struct has_pointer_traits_to_address : std::false_type {};
template <class T> struct has_pointer_traits_to_address<T,
	void_t<decltype(std::pointer_traits<T>::to_address(std::declval<const T&>()))>
> : std::true_type {};
}

template <class T>
constexpr T* to_address(T* p) noexcept
{
	static_assert(!std::is_function<T>::value, "to_address is not defined for function pointers");
	return p;
}

template <class Ptr>
auto to_address(const Ptr& p) noexcept -> decltype(std::pointer_traits<Ptr>::to_address(p))
{
	return std::pointer_traits<Ptr>::to_address(p);
}

template <class Ptr, typename std::enable_if<!detail::has_pointer_traits_to_address<Ptr>::value, int>::type = 0>
auto to_address(const Ptr& p) noexcept -> decltype(vs::to_address(p.operator->()))
{
	return vs::to_address(p.operator->());
}

}

#endif
