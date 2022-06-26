#pragma once

#define VS_ENUM_BITOPS(name, type) \
inline name  operator~ (name  x)         { return static_cast<name>(~static_cast<type>(x)); } \
inline name  operator| (name  l, name r) { return static_cast<name>(static_cast<type>(l) | static_cast<type>(r)); } \
inline name& operator|=(name& l, name r) { return l = l | r; } \
inline name  operator& (name  l, name r) { return static_cast<name>(static_cast<type>(l) & static_cast<type>(r)); } \
inline name& operator&=(name& l, name r) { return l = l & r; } \
inline name  operator^ (name  l, name r) { return static_cast<name>(static_cast<type>(l) ^ static_cast<type>(r)); } \
inline name& operator^=(name& l, name r) { return l = l ^ r; } \

#define VS_FORWARDING_CTOR1(name, a1) \
	template <class A1, class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	explicit name(A1&& a1 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_))

#define VS_FORWARDING_CTOR2(name, a1, a2) \
	template <class A1, class A2> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \

#define VS_FORWARDING_CTOR3(name, a1, a2, a3) \
	template <class A1, class A2, class A3> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \

#define VS_FORWARDING_CTOR4(name, a1, a2, a3, a4) \
	template <class A1, class A2, class A3, class A4> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_, A4&& a4 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \

#define VS_FORWARDING_CTOR5(name, a1, a2, a3, a4, a5) \
	template <class A1, class A2, class A3, class A4, class A5> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_, A4&& a4 ## _arg_, A5&& a5 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \

#define VS_FORWARDING_CTOR6(name, a1, a2, a3, a4, a5, a6) \
	template <class A1, class A2, class A3, class A4, class A5, class A6> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_, A4&& a4 ## _arg_, A5&& a5 ## _arg_, A6&& a6 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \

#define VS_FORWARDING_CTOR7(name, a1, a2, a3, a4, a5, a6, a7) \
	template <class A1, class A2, class A3, class A4, class A5, class A6, class A7> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_, A4&& a4 ## _arg_, A5&& a5 ## _arg_, A6&& a6 ## _arg_, A7&& a7 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \

#define VS_FORWARDING_CTOR8(name, a1, a2, a3, a4, a5, a6, a7, a8) \
	template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_, A4&& a4 ## _arg_, A5&& a5 ## _arg_, A6&& a6 ## _arg_, A7&& a7 ## _arg_, A8&& a8 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \

#define VS_FORWARDING_CTOR9(name, a1, a2, a3, a4, a5, a6, a7, a8, a9) \
	template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_, A4&& a4 ## _arg_, A5&& a5 ## _arg_, A6&& a6 ## _arg_, A7&& a7 ## _arg_, A8&& a8 ## _arg_, A9&& a9 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \

#define VS_FORWARDING_CTOR10(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) \
	template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_, A4&& a4 ## _arg_, A5&& a5 ## _arg_, A6&& a6 ## _arg_, A7&& a7 ## _arg_, A8&& a8 ## _arg_, A9&& a9 ## _arg_, A10&& a10 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \

#define VS_FORWARDING_CTOR11(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) \
	template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_, A4&& a4 ## _arg_, A5&& a5 ## _arg_, A6&& a6 ## _arg_, A7&& a7 ## _arg_, A8&& a8 ## _arg_, A9&& a9 ## _arg_, A10&& a10 ## _arg_, A11&& a11 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \
	, a11(std::forward<A11>(a11 ## _arg_)) \

#define VS_FORWARDING_CTOR12(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) \
	template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_, A4&& a4 ## _arg_, A5&& a5 ## _arg_, A6&& a6 ## _arg_, A7&& a7 ## _arg_, A8&& a8 ## _arg_, A9&& a9 ## _arg_, A10&& a10 ## _arg_, A11&& a11 ## _arg_, A12&& a12 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \
	, a11(std::forward<A11>(a11 ## _arg_)) \
	, a12(std::forward<A12>(a12 ## _arg_)) \

#define VS_FORWARDING_CTOR13(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) \
	template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_, A4&& a4 ## _arg_, A5&& a5 ## _arg_, A6&& a6 ## _arg_, A7&& a7 ## _arg_, A8&& a8 ## _arg_, A9&& a9 ## _arg_, A10&& a10 ## _arg_, A11&& a11 ## _arg_, A12&& a12 ## _arg_, A13&& a13 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \
	, a11(std::forward<A11>(a11 ## _arg_)) \
	, a12(std::forward<A12>(a12 ## _arg_)) \
	, a13(std::forward<A13>(a13 ## _arg_)) \

#define VS_FORWARDING_CTOR14(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) \
	template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_, A4&& a4 ## _arg_, A5&& a5 ## _arg_, A6&& a6 ## _arg_, A7&& a7 ## _arg_, A8&& a8 ## _arg_, A9&& a9 ## _arg_, A10&& a10 ## _arg_, A11&& a11 ## _arg_, A12&& a12 ## _arg_, A13&& a13 ## _arg_, A14&& a14 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \
	, a11(std::forward<A11>(a11 ## _arg_)) \
	, a12(std::forward<A12>(a12 ## _arg_)) \
	, a13(std::forward<A13>(a13 ## _arg_)) \
	, a14(std::forward<A14>(a14 ## _arg_)) \

#define VS_FORWARDING_CTOR15(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) \
	template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_, A4&& a4 ## _arg_, A5&& a5 ## _arg_, A6&& a6 ## _arg_, A7&& a7 ## _arg_, A8&& a8 ## _arg_, A9&& a9 ## _arg_, A10&& a10 ## _arg_, A11&& a11 ## _arg_, A12&& a12 ## _arg_, A13&& a13 ## _arg_, A14&& a14 ## _arg_, A15&& a15 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \
	, a11(std::forward<A11>(a11 ## _arg_)) \
	, a12(std::forward<A12>(a12 ## _arg_)) \
	, a13(std::forward<A13>(a13 ## _arg_)) \
	, a14(std::forward<A14>(a14 ## _arg_)) \
	, a15(std::forward<A15>(a15 ## _arg_)) \

#define VS_FORWARDING_CTOR16(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) \
	template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_, A4&& a4 ## _arg_, A5&& a5 ## _arg_, A6&& a6 ## _arg_, A7&& a7 ## _arg_, A8&& a8 ## _arg_, A9&& a9 ## _arg_, A10&& a10 ## _arg_, A11&& a11 ## _arg_, A12&& a12 ## _arg_, A13&& a13 ## _arg_, A14&& a14 ## _arg_, A15&& a15 ## _arg_, A16&& a16 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \
	, a11(std::forward<A11>(a11 ## _arg_)) \
	, a12(std::forward<A12>(a12 ## _arg_)) \
	, a13(std::forward<A13>(a13 ## _arg_)) \
	, a14(std::forward<A14>(a14 ## _arg_)) \
	, a15(std::forward<A15>(a15 ## _arg_)) \
	, a16(std::forward<A16>(a16 ## _arg_)) \

#define VS_FORWARDING_CTOR17(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17) \
	template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_, A4&& a4 ## _arg_, A5&& a5 ## _arg_, A6&& a6 ## _arg_, A7&& a7 ## _arg_, A8&& a8 ## _arg_, A9&& a9 ## _arg_, A10&& a10 ## _arg_, A11&& a11 ## _arg_, A12&& a12 ## _arg_, A13&& a13 ## _arg_, A14&& a14 ## _arg_, A15&& a15 ## _arg_, A16&& a16 ## _arg_, A17&& a17 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \
	, a11(std::forward<A11>(a11 ## _arg_)) \
	, a12(std::forward<A12>(a12 ## _arg_)) \
	, a13(std::forward<A13>(a13 ## _arg_)) \
	, a14(std::forward<A14>(a14 ## _arg_)) \
	, a15(std::forward<A15>(a15 ## _arg_)) \
	, a16(std::forward<A16>(a16 ## _arg_)) \
	, a17(std::forward<A17>(a17 ## _arg_)) \

#define VS_FORWARDING_CTOR18(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18) \
	template <class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13, class A14, class A15, class A16, class A17, class A18> \
	name(A1&& a1 ## _arg_, A2&& a2 ## _arg_, A3&& a3 ## _arg_, A4&& a4 ## _arg_, A5&& a5 ## _arg_, A6&& a6 ## _arg_, A7&& a7 ## _arg_, A8&& a8 ## _arg_, A9&& a9 ## _arg_, A10&& a10 ## _arg_, A11&& a11 ## _arg_, A12&& a12 ## _arg_, A13&& a13 ## _arg_, A14&& a14 ## _arg_, A15&& a15 ## _arg_, A16&& a16 ## _arg_, A17&& a17 ## _arg_, A18&& a18 ## _arg_) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \
	, a11(std::forward<A11>(a11 ## _arg_)) \
	, a12(std::forward<A12>(a12 ## _arg_)) \
	, a13(std::forward<A13>(a13 ## _arg_)) \
	, a14(std::forward<A14>(a14 ## _arg_)) \
	, a15(std::forward<A15>(a15 ## _arg_)) \
	, a16(std::forward<A16>(a16 ## _arg_)) \
	, a17(std::forward<A17>(a17 ## _arg_)) \
	, a18(std::forward<A17>(a18 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR1(name, a1) \
	template <class A1 = decltype(a1), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	explicit name(A1&& a1 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR2(name, a1, a2) \
	template <class A1 = decltype(a1), class A2 = decltype(a2), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	name(A1&& a1 ## _arg_ = {}, A2&& a2 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR3(name, a1, a2, a3) \
	template <class A1 = decltype(a1), class A2 = decltype(a2), class A3 = decltype(a3), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	name(A1&& a1 ## _arg_ = {}, A2&& a2 ## _arg_ = {}, A3&& a3 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR4(name, a1, a2, a3, a4) \
	template <class A1 = decltype(a1), class A2 = decltype(a2), class A3 = decltype(a3), class A4 = decltype(a4), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	name(A1&& a1 ## _arg_ = {}, A2&& a2 ## _arg_ = {}, A3&& a3 ## _arg_ = {}, A4&& a4 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR5(name, a1, a2, a3, a4, a5) \
	template <class A1 = decltype(a1), class A2 = decltype(a2), class A3 = decltype(a3), class A4 = decltype(a4), class A5 = decltype(a5), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	name(A1&& a1 ## _arg_ = {}, A2&& a2 ## _arg_ = {}, A3&& a3 ## _arg_ = {}, A4&& a4 ## _arg_ = {}, A5&& a5 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR6(name, a1, a2, a3, a4, a5, a6) \
	template <class A1 = decltype(a1), class A2 = decltype(a2), class A3 = decltype(a3), class A4 = decltype(a4), class A5 = decltype(a5), class A6 = decltype(a6), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	name(A1&& a1 ## _arg_ = {}, A2&& a2 ## _arg_ = {}, A3&& a3 ## _arg_ = {}, A4&& a4 ## _arg_ = {}, A5&& a5 ## _arg_ = {}, A6&& a6 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR7(name, a1, a2, a3, a4, a5, a6, a7) \
	template <class A1 = decltype(a1), class A2 = decltype(a2), class A3 = decltype(a3), class A4 = decltype(a4), class A5 = decltype(a5), class A6 = decltype(a6), class A7 = decltype(a7), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	name(A1&& a1 ## _arg_ = {}, A2&& a2 ## _arg_ = {}, A3&& a3 ## _arg_ = {}, A4&& a4 ## _arg_ = {}, A5&& a5 ## _arg_ = {}, A6&& a6 ## _arg_ = {}, A7&& a7 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR8(name, a1, a2, a3, a4, a5, a6, a7, a8) \
	template <class A1 = decltype(a1), class A2 = decltype(a2), class A3 = decltype(a3), class A4 = decltype(a4), class A5 = decltype(a5), class A6 = decltype(a6), class A7 = decltype(a7), class A8 = decltype(a8), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	name(A1&& a1 ## _arg_ = {}, A2&& a2 ## _arg_ = {}, A3&& a3 ## _arg_ = {}, A4&& a4 ## _arg_ = {}, A5&& a5 ## _arg_ = {}, A6&& a6 ## _arg_ = {}, A7&& a7 ## _arg_ = {}, A8&& a8 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR9(name, a1, a2, a3, a4, a5, a6, a7, a8, a9) \
	template <class A1 = decltype(a1), class A2 = decltype(a2), class A3 = decltype(a3), class A4 = decltype(a4), class A5 = decltype(a5), class A6 = decltype(a6), class A7 = decltype(a7), class A8 = decltype(a8), class A9 = decltype(a9), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	name(A1&& a1 ## _arg_ = {}, A2&& a2 ## _arg_ = {}, A3&& a3 ## _arg_ = {}, A4&& a4 ## _arg_ = {}, A5&& a5 ## _arg_ = {}, A6&& a6 ## _arg_ = {}, A7&& a7 ## _arg_ = {}, A8&& a8 ## _arg_ = {}, A9&& a9 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR10(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) \
	template <class A1 = decltype(a1), class A2 = decltype(a2), class A3 = decltype(a3), class A4 = decltype(a4), class A5 = decltype(a5), class A6 = decltype(a6), class A7 = decltype(a7), class A8 = decltype(a8), class A9 = decltype(a9), class A10 = decltype(a10), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	name(A1&& a1 ## _arg_ = {}, A2&& a2 ## _arg_ = {}, A3&& a3 ## _arg_ = {}, A4&& a4 ## _arg_ = {}, A5&& a5 ## _arg_ = {}, A6&& a6 ## _arg_ = {}, A7&& a7 ## _arg_ = {}, A8&& a8 ## _arg_ = {}, A9&& a9 ## _arg_ = {}, A10&& a10 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR11(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) \
	template <class A1 = decltype(a1), class A2 = decltype(a2), class A3 = decltype(a3), class A4 = decltype(a4), class A5 = decltype(a5), class A6 = decltype(a6), class A7 = decltype(a7), class A8 = decltype(a8), class A9 = decltype(a9), class A10 = decltype(a10), class A11 = decltype(a11), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	name(A1&& a1 ## _arg_ = {}, A2&& a2 ## _arg_ = {}, A3&& a3 ## _arg_ = {}, A4&& a4 ## _arg_ = {}, A5&& a5 ## _arg_ = {}, A6&& a6 ## _arg_ = {}, A7&& a7 ## _arg_ = {}, A8&& a8 ## _arg_ = {}, A9&& a9 ## _arg_ = {}, A10&& a10 ## _arg_ = {}, A11&& a11 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \
	, a11(std::forward<A11>(a11 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR12(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) \
	template <class A1 = decltype(a1), class A2 = decltype(a2), class A3 = decltype(a3), class A4 = decltype(a4), class A5 = decltype(a5), class A6 = decltype(a6), class A7 = decltype(a7), class A8 = decltype(a8), class A9 = decltype(a9), class A10 = decltype(a10), class A11 = decltype(a11), class A12 = decltype(a12), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	name(A1&& a1 ## _arg_ = {}, A2&& a2 ## _arg_ = {}, A3&& a3 ## _arg_ = {}, A4&& a4 ## _arg_ = {}, A5&& a5 ## _arg_ = {}, A6&& a6 ## _arg_ = {}, A7&& a7 ## _arg_ = {}, A8&& a8 ## _arg_ = {}, A9&& a9 ## _arg_ = {}, A10&& a10 ## _arg_ = {}, A11&& a11 ## _arg_ = {}, A12&& a12 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \
	, a11(std::forward<A11>(a11 ## _arg_)) \
	, a12(std::forward<A12>(a12 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR13(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) \
	template <class A1 = decltype(a1), class A2 = decltype(a2), class A3 = decltype(a3), class A4 = decltype(a4), class A5 = decltype(a5), class A6 = decltype(a6), class A7 = decltype(a7), class A8 = decltype(a8), class A9 = decltype(a9), class A10 = decltype(a10), class A11 = decltype(a11), class A12 = decltype(a12), class A13 = decltype(a13), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	name(A1&& a1 ## _arg_ = {}, A2&& a2 ## _arg_ = {}, A3&& a3 ## _arg_ = {}, A4&& a4 ## _arg_ = {}, A5&& a5 ## _arg_ = {}, A6&& a6 ## _arg_ = {}, A7&& a7 ## _arg_ = {}, A8&& a8 ## _arg_ = {}, A9&& a9 ## _arg_ = {}, A10&& a10 ## _arg_ = {}, A11&& a11 ## _arg_ = {}, A12&& a12 ## _arg_ = {}, A13&& a13 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \
	, a11(std::forward<A11>(a11 ## _arg_)) \
	, a12(std::forward<A12>(a12 ## _arg_)) \
	, a13(std::forward<A13>(a13 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR14(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) \
	template <class A1 = decltype(a1), class A2 = decltype(a2), class A3 = decltype(a3), class A4 = decltype(a4), class A5 = decltype(a5), class A6 = decltype(a6), class A7 = decltype(a7), class A8 = decltype(a8), class A9 = decltype(a9), class A10 = decltype(a10), class A11 = decltype(a11), class A12 = decltype(a12), class A13 = decltype(a13), class A14 = decltype(a14), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	name(A1&& a1 ## _arg_ = {}, A2&& a2 ## _arg_ = {}, A3&& a3 ## _arg_ = {}, A4&& a4 ## _arg_ = {}, A5&& a5 ## _arg_ = {}, A6&& a6 ## _arg_ = {}, A7&& a7 ## _arg_ = {}, A8&& a8 ## _arg_ = {}, A9&& a9 ## _arg_ = {}, A10&& a10 ## _arg_ = {}, A11&& a11 ## _arg_ = {}, A12&& a12 ## _arg_ = {}, A13&& a13 ## _arg_ = {}, A14&& a14 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \
	, a11(std::forward<A11>(a11 ## _arg_)) \
	, a12(std::forward<A12>(a12 ## _arg_)) \
	, a13(std::forward<A13>(a13 ## _arg_)) \
	, a14(std::forward<A14>(a14 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR15(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) \
	template <class A1 = decltype(a1), class A2 = decltype(a2), class A3 = decltype(a3), class A4 = decltype(a4), class A5 = decltype(a5), class A6 = decltype(a6), class A7 = decltype(a7), class A8 = decltype(a8), class A9 = decltype(a9), class A10 = decltype(a10), class A11 = decltype(a11), class A12 = decltype(a12), class A13 = decltype(a13), class A14 = decltype(a14), class A15 = decltype(a15), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	name(A1&& a1 ## _arg_ = {}, A2&& a2 ## _arg_ = {}, A3&& a3 ## _arg_ = {}, A4&& a4 ## _arg_ = {}, A5&& a5 ## _arg_ = {}, A6&& a6 ## _arg_ = {}, A7&& a7 ## _arg_ = {}, A8&& a8 ## _arg_ = {}, A9&& a9 ## _arg_ = {}, A10&& a10 ## _arg_ = {}, A11&& a11 ## _arg_ = {}, A12&& a12 ## _arg_ = {}, A13&& a13 ## _arg_ = {}, A14&& a14 ## _arg_ = {}, A15&& a15 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \
	, a11(std::forward<A11>(a11 ## _arg_)) \
	, a12(std::forward<A12>(a12 ## _arg_)) \
	, a13(std::forward<A13>(a13 ## _arg_)) \
	, a14(std::forward<A14>(a14 ## _arg_)) \
	, a15(std::forward<A15>(a15 ## _arg_)) \

#define VS_FORWARDING_DEF_CTOR16(name, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) \
	template <class A1 = decltype(a1), class A2 = decltype(a2), class A3 = decltype(a3), class A4 = decltype(a4), class A5 = decltype(a5), class A6 = decltype(a6), class A7 = decltype(a7), class A8 = decltype(a8), class A9 = decltype(a9), class A10 = decltype(a10), class A11 = decltype(a11), class A12 = decltype(a12), class A13 = decltype(a13), class A14 = decltype(a14), class A15 = decltype(a15), class A16 = decltype(a16), class = typename std::enable_if<!std::is_convertible<A1, name>::value>::type> \
	name(A1&& a1 ## _arg_ = {}, A2&& a2 ## _arg_ = {}, A3&& a3 ## _arg_ = {}, A4&& a4 ## _arg_ = {}, A5&& a5 ## _arg_ = {}, A6&& a6 ## _arg_ = {}, A7&& a7 ## _arg_ = {}, A8&& a8 ## _arg_ = {}, A9&& a9 ## _arg_ = {}, A10&& a10 ## _arg_ = {}, A11&& a11 ## _arg_ = {}, A12&& a12 ## _arg_ = {}, A13&& a13 ## _arg_ = {}, A14&& a14 ## _arg_ = {}, A15&& a15 ## _arg_ = {}, A16&& a16 ## _arg_ = {}) \
	: a1(std::forward<A1>(a1 ## _arg_)) \
	, a2(std::forward<A2>(a2 ## _arg_)) \
	, a3(std::forward<A3>(a3 ## _arg_)) \
	, a4(std::forward<A4>(a4 ## _arg_)) \
	, a5(std::forward<A5>(a5 ## _arg_)) \
	, a6(std::forward<A6>(a6 ## _arg_)) \
	, a7(std::forward<A7>(a7 ## _arg_)) \
	, a8(std::forward<A8>(a8 ## _arg_)) \
	, a9(std::forward<A9>(a9 ## _arg_)) \
	, a10(std::forward<A10>(a10 ## _arg_)) \
	, a11(std::forward<A11>(a11 ## _arg_)) \
	, a12(std::forward<A12>(a12 ## _arg_)) \
	, a13(std::forward<A13>(a13 ## _arg_)) \
	, a14(std::forward<A14>(a14 ## _arg_)) \
	, a15(std::forward<A15>(a15 ## _arg_)) \
	, a16(std::forward<A16>(a16 ## _arg_)) \

