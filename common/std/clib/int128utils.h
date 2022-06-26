#ifndef _INT128_H
#define _INT128_H

#include <stdint.h>
#include <emmintrin.h>

/*
Artem Boldarev (13.10.2015):

Simple utilities to help manipulate 128 bit integers using SSE2 SIMD CPU extension.
It is initially added for IPv6 support in VS_NetworkConnectionACL.
*/

#ifdef __cplusplus
namespace int128_utils {
#endif

inline __m128i int128_load(const uint32_t u1, const uint32_t u2, const uint32_t u3, const uint32_t u4)
{
	__m128i res;
	uint32_t data[4] = {u1, u2, u3, u4};
	res = _mm_loadu_si128((__m128i *)data);

	return res;
}

inline __m128i int128_load_array(const uint32_t data[])
{
	__m128i res;
	res = _mm_loadu_si128((__m128i *)data);

	return res;
}

inline void int128_store(const __m128i val, uint32_t *u1, uint32_t *u2, uint32_t *u3, uint32_t *u4)
{
	uint32_t res[4];
	_mm_storeu_si128((__m128i *)res, val);

	if (u1)
		*u1 = res[0];

	if (u2)
		*u2 = res[1];

	if (u3)
		*u3 = res[2];

	if (u4)
		*u4 = res[3];
}

inline void int128_store_array(const __m128i val, uint32_t data[])
{
	_mm_storeu_si128((__m128i *)data, val);
}

inline __m128i int128_set_highest_bits(size_t bits_number)
{
	size_t nbits = bits_number;
	size_t count;
	__m128i res;
	uint8_t mask_value[16] = {0};

	nbits = nbits > 128 ? 128 : nbits; // normalize
	count = ((nbits % 8) > 0 ? (nbits / 8) + 1 : (nbits / 8));
	for (size_t i = 0; i < count; i++)
	{
		size_t bits_left = nbits - ((i) * 8);
		if (bits_left >= 8)
		{
			mask_value[i] = ~((uint8_t) 0);
		}
		else
		{
			mask_value[i] = (bits_left > 1 ? ((1 << bits_left) - 1) : 1) << (8 - bits_left);
		}
	}

	res = _mm_loadu_si128((__m128i *)mask_value);

	return res;
}

inline __m128i int128_set_lowest_bits(size_t bits_number)
{
	size_t nbits = bits_number;
	size_t count;
	__m128i res;
	uint8_t mask_value[16] = {0};

	nbits = nbits > 128 ? 128 : nbits; // normalize
	count = ((nbits % 8) > 0 ? (nbits / 8) + 1 : (nbits / 8));
	for (size_t i = 0; i < count; i++)
	{
		size_t bits_left = nbits - ((i) * 8);
		mask_value[15 - i] = (bits_left >= 8 ? ~((uint8_t)0) : ((1 << bits_left) - 1));
	}

	res = _mm_loadu_si128((__m128i *)mask_value);

	return res;
}

inline bool int128_cmpeq(const __m128i a, const __m128i b)
{
	return _mm_movemask_epi8(_mm_cmpeq_epi32(a, b)) == 0xFFFF;
}

#ifdef __cplusplus
}
#endif

#endif /*_INT128_H*/
