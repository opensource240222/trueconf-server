#pragma once

#include "std-generic/compat/utility.h"

namespace vs {

// A compile time string.
// Read it through the 'data' member, pass it as a type template parameter.
template <char... x>
struct cts { static constexpr char data[] = { x..., 0 }; };

// Older compilers (e.g. GCC 6) require a definition for static constexpr objects.
template <char... x>
constexpr char cts<x...>::data[];

// Macro that creates a vs::cts value from a string literal.
#define VS_CTS(s) \
	[]() noexcept { \
		struct str { static constexpr char get(unsigned i) { return (s)[i]; } }; \
		return vs::detail::build_cts<str>(vs::make_integer_sequence<unsigned, sizeof(s)-1>()); \
	}()

// Macro that creates vs::cts type from a string literal.
// Unlike VS_CTS, can be used in an unevaluated context (eg. as a template argument).
// Also, unlike VS_CTS, has a limit on maximum string length.
// Generally VS_CTS should be preferred.
// Argument 'max_len' must be a power of 2.
#define VS_CTS_TYPE(s, max_len) \
	typename vs::detail::cts_extract< \
		vs::cts<TO_CHARS_##max_len(s, 0), sizeof(s) <= max_len ? 0 : throw "String is too long">, \
		0, vs::make_integer_sequence<unsigned, sizeof(s)-1> \
	>::type

namespace detail {

// Helper function for the VS_CTS macro that provides a context for parameter pack expansion.
template <class S, unsigned... I>
constexpr cts<S::get(I)...> build_cts(vs::integer_sequence<unsigned, I...>) { return {}; }

// Metafunction that extacts a substring from a cts.
// It should be called in this way: cts_substr<str, pos, vs::make_integer_sequence<unsigned, count>>::type
//
// We use it in the VS_CTS_TYPE macro to strip extra null bytes at the end.
// This makes the resulting type shorter and thus more readable.
template <class CTS, unsigned Pos, class>
struct cts_extract;
template <class CTS, unsigned Pos, unsigned... I>
struct cts_extract<CTS, Pos, vs::integer_sequence<unsigned, I...>> { using type = cts<CTS::data[Pos+I]...>; };

#define TO_CHARS_1(s,i) (sizeof(s) > (i) ? s[i] : 0)
#define TO_CHARS_2(s,i)   TO_CHARS_1(s,i),  TO_CHARS_1(s,i+1)
#define TO_CHARS_4(s,i)   TO_CHARS_2(s,i),  TO_CHARS_2(s,i+2)
#define TO_CHARS_8(s,i)   TO_CHARS_4(s,i),  TO_CHARS_4(s,i+4)
#define TO_CHARS_16(s,i)  TO_CHARS_8(s,i),  TO_CHARS_8(s,i+8)
#define TO_CHARS_32(s,i)  TO_CHARS_16(s,i), TO_CHARS_16(s,i+16)
#define TO_CHARS_64(s,i)  TO_CHARS_32(s,i), TO_CHARS_32(s,i+32)
#define TO_CHARS_128(s,i) TO_CHARS_64(s,i), TO_CHARS_64(s,i+64)

}

}
