#pragma once

#include <cassert>

// Extracts field value from bitvector that uses big-endian layout:
//   1. Fields are allocated from most significant bit to least significant bit of a bitvector;
//   2. Bitvector itself is stored using big-endian layout: most significant byte is first byte.
template <class T = unsigned int>
T GetBitFieldBE(const void* p, unsigned int offset, unsigned int size)
{
	if (size == 0)
		return T();
	if (size > sizeof(T) * 8)
		size = sizeof(T) * 8;
	T result = T();
	unsigned int result_offset = 0;
	const int first_byte = offset / 8;
	const int last_byte = (offset + size - 1) / 8;
	for (int bi = last_byte; bi >= first_byte; --bi)
	{
		const unsigned int msb_gap = bi == first_byte ? offset % 8 : 0;
		const unsigned int lsb_gap = bi == last_byte ? 7 - (offset + size - 1) % 8 : 0;
		result |= T((reinterpret_cast<const unsigned char*>(p)[bi] & (0xff >> msb_gap)) >> lsb_gap) << result_offset;
		result_offset += 8 - msb_gap - lsb_gap;
	}
	assert(result_offset == size);
	return result;
}

// Extracts field value from bitvector that uses little-endian layout:
//   1. Fields are allocated from least significant bit to most significant bit of a bitvector;
//   2. Bitvector itself is stored using little-endian layout: least significant byte is first byte.
template <class T = unsigned int>
T GetBitFieldLE(const void* p, unsigned int offset, unsigned int size)
{
	if (size == 0)
		return T();
	if (size > sizeof(T) * 8)
		size = sizeof(T) * 8;
	T result = T();
	unsigned int result_offset = 0;
	const int first_byte = offset / 8;
	const int last_byte = (offset + size - 1) / 8;
	for (int bi = first_byte; bi <= last_byte; ++bi)
	{
		const unsigned int msb_gap = bi == last_byte ? 7 - (offset + size - 1) % 8 : 0;
		const unsigned int lsb_gap = bi == first_byte ? offset % 8 : 0;
		result |= T((reinterpret_cast<const unsigned char*>(p)[bi] & (0xff >> msb_gap)) >> lsb_gap) << result_offset;
		result_offset += 8 - msb_gap - lsb_gap;
	}
	assert(result_offset == size);
	return result;
}

// Set field value in bitvector that uses big-endian layout.
template <class T>
void SetBitFieldBE(void* p, unsigned int offset, unsigned int size, T value)
{
	if (size == 0)
		return;
	if (size > sizeof(T) * 8)
		size = sizeof(T) * 8;
	const int first_byte = offset / 8;
	const int last_byte = (offset + size - 1) / 8;
	for (int bi = last_byte; bi >= first_byte; --bi)
	{
		const unsigned int msb_gap = bi == first_byte ? offset % 8 : 0;
		const unsigned int lsb_gap = bi == last_byte ? 7 - (offset + size - 1) % 8 : 0;
		reinterpret_cast<unsigned char*>(p)[bi] |= (value << lsb_gap) & (0xff >> msb_gap);
		value >>= 8 - msb_gap - lsb_gap;
	}
}

// Set field value in bitvector that uses little-endian layout.
template <class T>
void SetBitFieldLE(void* p, unsigned int offset, unsigned int size, T value)
{
	if (size == 0)
		return;
	if (size > sizeof(T) * 8)
		size = sizeof(T) * 8;
	const int first_byte = offset / 8;
	const int last_byte = (offset + size - 1) / 8;
	for (int bi = first_byte; bi <= last_byte; ++bi)
	{
		const unsigned int msb_gap = bi == last_byte ? 7 - (offset + size - 1) % 8 : 0;
		const unsigned int lsb_gap = bi == first_byte ? offset % 8 : 0;
		reinterpret_cast<unsigned char*>(p)[bi] |= (value << lsb_gap) & (0xff >> msb_gap);
		value >>= 8 - msb_gap - lsb_gap;
	}
}
