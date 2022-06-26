#pragma once

#include <type_traits>

namespace vs {

namespace detail {
template <class T, class U> typename std::enable_if< std::is_base_of<typename std::remove_pointer<T>::type, U>::value, T>::type down_cast_impl(U* x) { return static_cast<T>(x); }
template <class T, class U> typename std::enable_if<!std::is_base_of<typename std::remove_pointer<T>::type, U>::value, T>::type down_cast_impl(U*)   { return nullptr; }
}

template <class T, class U>
T down_cast(U* x)
{
	static_assert(std::is_pointer<T>::value && std::is_class<typename std::remove_pointer<T>::type>::value, "target type must be a pointer to a class");
	static_assert(std::is_class<U>::value, "operand must be a pointer to a class");
	/* C++17 version:
	if constexpr (std::is_base_of<std::remove_pointer_t<T>, U>::value)
		return static_cast<T>(x);
	else
		return nullptr;
	*/
	return detail::down_cast_impl<T>(x);
}

}

using vs::down_cast;
