#pragma once

#include <cstring>

namespace vs
{
	template<typename To>
	inline To mem_cast(const void *var) noexcept
	{
		To to;
		std::memcpy(&to, var, sizeof(To));
		return to;
	}
	template<typename T>
	inline void mem_write(void *dst, const T &src) noexcept
	{
		std::memcpy(dst, &src, sizeof(src));
	}
} //namespace vs
