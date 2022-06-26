#include <cstddef>

#include "std/cpplib/cts.h"

#include <Zend/zend_API.h>
#include <Zend/zend_types.h>

#define vs_zend_parse_parameters(num_args, format, ...) \
	zpp::impl(0, num_args, VS_CTS(format), __VA_ARGS__)

#define vs_zend_parse_parameters_ex(flags, num_args, format, ...) \
	zpp::impl(flags, num_args, VS_CTS(format), __VA_ARGS__)

namespace zpp { // zend_parse_parameters

// Metafunction that converts a format string (Format) to the list of corresponding parameter types (T...).
// The result is a regular function that can be called to perform matching of the passed types with expected types.
// This allows us get nice and readable compiler errors when an argument of a wrong type is passed.
// The format string in parsed recursively by (ab)using partial template specialization selection rules.
template <class Format, bool bang = false, class... T>
struct param_checker
{
	// Fallback case, it is used when there is no match for the beginning of the format string.
	// Condition in the static_assert is meaningless, it is needed to workaround language rule that condition can't be always false.
	static_assert(sizeof(Format::data) < 1, "Invalid type specifier. See the first character in vs::cts from the latest instantiation zpp::param_checker.");
};
template <bool bang, class... T>
struct param_checker<vs::cts<>, bang, T...> { static inline void check(T...) noexcept {}; };

template <char... x, bool bang, class... T>
struct param_checker<vs::cts<'!', x...>, bang, T...> : param_checker<vs::cts<x...>, true, T...> {};

#define DECL_PARAM(c, ...) \
	template <char... x, bool bang, class... T> \
	struct param_checker<vs::cts<c, x...>, bang, T...>  : param_checker<vs::cts<x...>, false, T..., ##__VA_ARGS__> {}
#define DECL_PARAM_BANG(c, ...) \
	template <char... x, class... T> \
	struct param_checker<vs::cts<c, x...>, false, T...> : param_checker<vs::cts<x...>, false, T..., ##__VA_ARGS__> {}; \
	template <char... x, class... T> \
	struct param_checker<vs::cts<c, x...>, true, T...>  : param_checker<vs::cts<x...>, false, T..., ##__VA_ARGS__, zend_bool*> {}
DECL_PARAM('|');
DECL_PARAM('/');
DECL_PARAM('a', zval**);
DECL_PARAM('A', zval**);
DECL_PARAM_BANG('b', zend_bool*);
DECL_PARAM('C', zend_class_entry**);
DECL_PARAM_BANG('d', double*);
DECL_PARAM('f', zend_fcall_info*, zend_fcall_info_cache*);
DECL_PARAM('h', HashTable**);
DECL_PARAM('H', HashTable**);
DECL_PARAM_BANG('l', zend_long*);
DECL_PARAM('o', zval**);
DECL_PARAM('O', zval**, zend_class_entry*);
DECL_PARAM('p', char**, size_t*);
DECL_PARAM('P', zend_string**);
DECL_PARAM('r', zval**);
DECL_PARAM('s', char**, size_t*);
DECL_PARAM('S', zend_string**);
DECL_PARAM('z', zval**);
DECL_PARAM('*', zval**, int*);
DECL_PARAM('+', zval**, int*);
#undef DECL_PARAM
#undef DECL_PARAM_BANG

template <class Format, class... Args>
int impl(int flags, int num_args, Format, Args... args)
{
	param_checker<Format>::check(args...);
	return zend_parse_parameters_ex(flags, num_args, Format::data, args...);
}

}
