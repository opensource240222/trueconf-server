#pragma once

#include "ProtectionLib/Protection.h"
#include "std-generic/gcc_version.h"

#include <cstddef>
#include <cstdint>

namespace protection {

SECURE_FUNC_INTERNAL
static inline void* memcpy(void* dest, const void* src, size_t n)
{
	auto d = static_cast<char*>(dest);
	auto s = static_cast<const char*>(src);
	if (sizeof(void*) == 8)
		for (; n >= 8; d += 8, s += 8, n -= 8)
			*reinterpret_cast<long long*>(d) = *reinterpret_cast<const long long*>(s);
	else
		for (; n >= 4; d += 4, s += 4, n -= 4)
			*reinterpret_cast<int*>(d) = *reinterpret_cast<const int*>(s);
	for (; n >= 1; ++d, ++s, --n)
		*d = *s;

	return dest;
}

SECURE_FUNC_INTERNAL
static inline void* memmove(void* dest, const void* src, size_t n)
{
	if (reinterpret_cast<uintptr_t>(dest) < reinterpret_cast<uintptr_t>(src))
		return memcpy(dest, src, n);

	auto d = static_cast<char*>(dest) + n - 1;
	auto s = static_cast<const char*>(src) + n - 1;
	if (sizeof(void*) == 8)
	{
		d -= 7;
		s -= 7;
		for (; n >= 8; d -= 8, s -= 8, n -= 8)
			*reinterpret_cast<long long*>(d) = *reinterpret_cast<const long long*>(s);
	}
	else
	{
		d -= 3;
		s -= 3;
		for (; n >= 4; d -= 4, s -= 4, n -= 4)
			*reinterpret_cast<int*>(d) = *reinterpret_cast<const int*>(s);
	}
	for (; n >= 1; --d, --s, --n)
		*d = *s;

	return dest;
}

SECURE_FUNC_INTERNAL
static inline void* memset(void* p, int c, size_t n)
{
	auto pp = static_cast<unsigned char*>(p);
	if (sizeof(void*) == 8)
	{
		const auto cc =
			static_cast<unsigned long long>(c & 0xff) << 56 |
			static_cast<unsigned long long>(c & 0xff) << 48 |
			static_cast<unsigned long long>(c & 0xff) << 40 |
			static_cast<unsigned long long>(c & 0xff) << 32 |
			static_cast<unsigned long long>(c & 0xff) << 24 |
			static_cast<unsigned long long>(c & 0xff) << 16 |
			static_cast<unsigned long long>(c & 0xff) <<  8 |
			static_cast<unsigned long long>(c & 0xff);
		for (; n >= 8; pp += 8, n -= 8)
			*reinterpret_cast<unsigned long long*>(pp) = cc;
	}
	else
	{
		const auto cc =
			static_cast<unsigned int>(c & 0xff) << 24 |
			static_cast<unsigned int>(c & 0xff) << 16 |
			static_cast<unsigned int>(c & 0xff) <<  8 |
			static_cast<unsigned int>(c & 0xff);
		for (; n >= 4; pp += 4, n -= 4)
			*reinterpret_cast<unsigned int*>(pp) = cc;
	}
	for (; n >= 1; ++pp, --n)
		*pp = c & 0xff;

	return p;
}

SECURE_FUNC_INTERNAL
static inline void explicit_bzero(void* p, size_t n)
{
	auto pp = static_cast<volatile char*>(p);
	if (sizeof(void*) == 8)
		for (; n >= 8; pp += 8, n -= 8)
			*reinterpret_cast<volatile long long*>(pp) = 0;
	else
		for (; n >= 4; pp += 4, n -= 4)
			*reinterpret_cast<volatile int*>(pp) = 0;
	for (; n >= 1; ++pp, --n)
		*pp = 0;
}

SECURE_FUNC_INTERNAL
static inline int memcmp(const void* l, const void* r, size_t n)
{
	auto ll = static_cast<const unsigned char*>(l);
	auto rr = static_cast<const unsigned char*>(r);
	for (; n >= 1; ++ll, ++rr, --n)
	{
		const int d = *rr - *ll;
		if (d != 0)
			return d;
	}
	return 0;
}

SECURE_FUNC_INTERNAL
static inline size_t strlen(const char* s)
{
	auto p = s;
	while (*p)
		++p;
	return p - s;
}

SECURE_FUNC_INTERNAL
static inline int strcmp(const char* l, const char* r)
{
#if defined(GCC_VERSION) && GCC_VERSION < 80000 && defined(__x86_64__)
// When compiling this function for x86_64 GCC 6 and 7 puts 2-byte "jmp" followed by a 6-byte "nop" at the start of this function.
// This confuses VMProtect 3.3.1 (build 1120) and it complains that the function is too small to be protected.
// To work around that we put 3-byte nop at the start, this ensures that there will be 5 bytes before the nop placed by the compiler.
	asm ("nopl (%rax)");
#endif
	while (true)
	{
		const int diff = static_cast<unsigned char>(*l) - static_cast<unsigned char>(*r);
		if (diff != 0)
			return diff;
		if (!*l)
			return *r ? -1 : 0;
		++l;
		++r;
	}
}

}
