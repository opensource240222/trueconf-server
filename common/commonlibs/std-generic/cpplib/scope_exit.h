#pragma once

#include <utility>

namespace vs { namespace detail {

template <class F>
class scope_exit
{
public:
	explicit scope_exit(F&& f) : f_(std::forward<F>(f)) {}
	~scope_exit() { f_(); }
private:
	F f_;
};

struct scope_exit_maker
{
	template <class F>
	scope_exit<F> operator<<(F&& f) { return scope_exit<F>(std::forward<F>(f)); }
};

}}

#define VS_CONCAT_IMPL(x, y) x ## y
#define VS_CONCAT(x, y) VS_CONCAT_IMPL(x, y)
#define VS_SCOPE_EXIT auto VS_CONCAT(scope_exit_, __LINE__) = vs::detail::scope_exit_maker() << [&]()
