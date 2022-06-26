#pragma once

// This "wheel reinventing" exists because it has these advantages:
//   1. Avoid inclusion of WinSock2.h (pulls Windows.h, results in about 150k
//      lines (33k non-empty)) just for hton* functions.
//   2. Provide efficient implementation for hton* on Windows. On Windows hton*
//      are actual functions with terrible implementation, even with /O2.
//   3. Provide htonll and ntohll on Linux.

#include <cstdint>

#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
#	include <stdlib.h>

// MSVC can't optimize endianness independent implementation (below) into a
// single bswap instruction, so we check architecture and call intrinsic
// directly.
inline uint16_t vs_htons (uint16_t x) { return _byteswap_ushort(x); }
inline uint32_t vs_htonl (uint32_t x) { return _byteswap_ulong (x); }
inline uint64_t vs_htonll(uint64_t x) { return _byteswap_uint64(x); }
inline uint16_t vs_ntohs (uint16_t x) { return _byteswap_ushort(x); }
inline uint32_t vs_ntohl (uint32_t x) { return _byteswap_ulong (x); }
inline uint64_t vs_ntohll(uint64_t x) { return _byteswap_uint64(x); }

#else

// Endianness independent implementation.
inline uint16_t vs_htons (uint16_t x)
{
	auto p = reinterpret_cast<const uint8_t*>(&x);
	return static_cast<uint16_t>(p[1])
	     | static_cast<uint16_t>(p[0]) << 8;
}
inline uint32_t vs_htonl (uint32_t x)
{
	auto p = reinterpret_cast<const uint8_t*>(&x);
	return static_cast<uint32_t>(p[3])
	     | static_cast<uint32_t>(p[2]) << 8
	     | static_cast<uint32_t>(p[1]) << 16
	     | static_cast<uint32_t>(p[0]) << 24;
}
inline uint64_t vs_htonll(uint64_t x)
{
	auto p = reinterpret_cast<const uint8_t*>(&x);
	return static_cast<uint64_t>(p[7])
	     | static_cast<uint64_t>(p[6]) << 8
	     | static_cast<uint64_t>(p[5]) << 16
	     | static_cast<uint64_t>(p[4]) << 24
	     | static_cast<uint64_t>(p[3]) << 32
	     | static_cast<uint64_t>(p[2]) << 40
	     | static_cast<uint64_t>(p[1]) << 48
	     | static_cast<uint64_t>(p[0]) << 56;
}
inline uint16_t vs_ntohs (uint16_t x)
{
	auto p = reinterpret_cast<const uint8_t*>(&x);
	return static_cast<uint16_t>(p[1])
	     | static_cast<uint16_t>(p[0]) << 8;
}
inline uint32_t vs_ntohl (uint32_t x)
{
	auto p = reinterpret_cast<const uint8_t*>(&x);
	return static_cast<uint32_t>(p[3])
	     | static_cast<uint32_t>(p[2]) << 8
	     | static_cast<uint32_t>(p[1]) << 16
	     | static_cast<uint32_t>(p[0]) << 24;
}
inline uint64_t vs_ntohll(uint64_t x)
{
	auto p = reinterpret_cast<const uint8_t*>(&x);
	return static_cast<uint64_t>(p[7])
	     | static_cast<uint64_t>(p[6]) << 8
	     | static_cast<uint64_t>(p[5]) << 16
	     | static_cast<uint64_t>(p[4]) << 24
	     | static_cast<uint64_t>(p[3]) << 32
	     | static_cast<uint64_t>(p[2]) << 40
	     | static_cast<uint64_t>(p[1]) << 48
	     | static_cast<uint64_t>(p[0]) << 56;
}

#endif
