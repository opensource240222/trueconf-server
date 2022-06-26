#pragma once

// std::bit_cast from C++20 is not available in any standard library yet.

#include <cstring>
#include <type_traits>

namespace vs
{
	template <class To, class From>
	To bit_cast(const From &src) noexcept
	{
		static_assert(sizeof(To) == sizeof(From), "size of destination and source objects must be equal");
		static_assert(std::is_trivially_copyable<To>::value, "To type must be trivially copyable.");
		static_assert(std::is_trivially_copyable<From>::value, "From type must be trivially copyable");
		
		To dst;
		std::memcpy(&dst, &src, sizeof(To));
		return dst;
	}
} //namespace vs