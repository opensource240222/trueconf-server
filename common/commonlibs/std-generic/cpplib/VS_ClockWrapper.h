#pragma once

#include <chrono>

#if __cplusplus >= 201402L || (defined(_MSC_VER) && _MSC_VER >= 1910)
#	define constexpr_14 constexpr
#else
#	define constexpr_14
#endif

template<class clock>
class clock_wrapper final
{
public:
	constexpr clock_wrapper() noexcept : m_diff(0)
	{}
	constexpr_14 void set_diff(const typename clock::duration &diff) noexcept
	{
		m_diff = diff;
	}
	constexpr_14 void add_diff(const typename clock::duration &diff) noexcept
	{
		m_diff += diff;
	}
	constexpr typename clock::time_point now() const noexcept
	{
		return clock::now() + m_diff;
	}
private:
	typename clock::duration m_diff;
};

#undef constexpr_14

typedef clock_wrapper<std::chrono::steady_clock>			steady_clock_wrapper;
typedef clock_wrapper<std::chrono::system_clock>			system_clock_wrapper;
typedef clock_wrapper<std::chrono::high_resolution_clock>	high_resolution_clock_wrapper;
