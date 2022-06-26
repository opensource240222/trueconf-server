#pragma once

#include "std-generic/cpplib/string_view.h"

namespace vs
{
namespace detail
{
	template<typename Type, typename Func>
	inline Type a2number_sv(string_view str, Func func) noexcept
	{
		static_assert(std::is_integral<Type>::value, "!");

		constexpr auto max_len = std::numeric_limits<Type>::digits10 + 1;
		if (str.empty() || str.length() > max_len)
		{
			return 0;
		}
		char buffer_str[max_len + 1 /*0-terminator*/];
		::memcpy(buffer_str, str.data(), str.length());
		buffer_str[str.length()] = '\0';
		return (*func)(buffer_str);
	}
} // namespace detail

inline int       atoi_sv (string_view str) noexcept { return detail::a2number_sv<int>      (str, ::atoi);  }
inline long      atol_sv (string_view str) noexcept { return detail::a2number_sv<long>     (str, ::atol);  }
inline long long atoll_sv(string_view str) noexcept { return detail::a2number_sv<long long>(str, ::atoll); }

inline unsigned int       atou_sv  (string_view str) noexcept { return static_cast<unsigned int>      (atoi_sv(str));  }
inline unsigned long      atoul_sv (string_view str) noexcept { return static_cast<unsigned long>     (atol_sv(str));  }
inline unsigned long long atoull_sv(string_view str) noexcept { return static_cast<unsigned long long>(atoll_sv(str)); }

} //namespace vs
