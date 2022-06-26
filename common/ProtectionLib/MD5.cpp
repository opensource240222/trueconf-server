/**
 **************************************************************************
 * \file md5.c
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief This code implements the MD5 message-digest algorithm.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * \b Project Standart Libraries
 * \author The algorithm is due to Ron Rivest.
 * \author This code was written by Colin Plumb in 1993, no copyright is claimed.
 * \author Adopted by SMirnovK
 * \date 10.06.03
 *
 * $Revision: 2 $
 *
 * $History: md5.c $
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 26.11.10   Time: 14:37
 * Updated in $/VSNA/std/clib
 * - VS_ConvertToMD5(): max len = 8192 (was 1024)
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/clib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/clib
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 30.12.04   Time: 14:12
 * Updated in $/VS/std/clib
 * files headers
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "ProtectionLib/MD5.h"
#include "ProtectionLib/Lib.h"
#include "ProtectionLib/Protection.h"

#include <cassert>

namespace protection {

/****************************************************************************
 * Defines
 ****************************************************************************/
#define GET_32BIT_LSB_FIRST(cp) ( \
		static_cast<uint32_t>(static_cast<unsigned char>((cp)[0])) | \
		static_cast<uint32_t>(static_cast<unsigned char>((cp)[1])) << 8| \
		static_cast<uint32_t>(static_cast<unsigned char>((cp)[2])) << 16 | \
		static_cast<uint32_t>(static_cast<unsigned char>((cp)[3])) << 24 \
	)

#define PUT_32BIT_LSB_FIRST(cp, value) do { \
		(cp)[0] = (value) & 0xff; \
		(cp)[1] = ((value) >> 8) & 0xff; \
		(cp)[2] = ((value) >> 16) & 0xff; \
		(cp)[3] = ((value) >> 24) & 0xff; \
	} while (0)

// m_buffer is not initialized intentionally, reading from it is guarded by m_total_bytes.
// cppcheck-suppress uninitMemberVar symbolName=MD5::m_buffer
SECURE_FUNC_INTERNAL
MD5::MD5()
{
	/* MD5 initialization constants */
	m_digest[0] = 0x67452301;
	m_digest[1] = 0xefcdab89;
	m_digest[2] = 0x98badcfe;
	m_digest[3] = 0x10325476;

	/* Reset counters */
	m_total_bytes = 0;
}

SECURE_FUNC_INTERNAL
void MD5::Update(const void* data, size_t size)
{
	assert((m_total_bytes >> (sizeof(m_total_bytes) * 8 - 1) == 0) && "MD5::Update() is called after MD5::Final()");

	auto p = static_cast<const unsigned char*>(data);

	/* If there is data in the buffer then append to it first */
	if (const unsigned in_buffer = m_total_bytes % BLOCK_BYTES)
	{
		if (size < BLOCK_BYTES - in_buffer)
		{
			memcpy(m_buffer + in_buffer, p, size);
			m_total_bytes += size;
			return;
		}

		const unsigned to_copy = BLOCK_BYTES - in_buffer;
		memcpy(m_buffer + in_buffer, p, to_copy);
		m_total_bytes += to_copy;
		p += to_copy;
		size -= to_copy;

		Transform(m_buffer);
	}

	/* Process whole blocks */
	while (size >= BLOCK_BYTES)
	{
		Transform(p);

		m_total_bytes += BLOCK_BYTES;
		p += BLOCK_BYTES;
		size -= BLOCK_BYTES;
	}

	if (size > 0)
	{
		memcpy(m_buffer, p, size);
		m_total_bytes += size;
	}
}

/**
****************************************************************************
* Final wrapup - pad to 64-byte boundary with the bit pattern
* 1 0* (64-bit count of bits processed, MSB-first)
******************************************************************************/
SECURE_FUNC_INTERNAL
void MD5::Final()
{
	/* Total number of hashed bits */
	const uint64_t total_bits = m_total_bytes * 8;

	/* Padding */
	unsigned i = m_total_bytes % BLOCK_BYTES;
	m_buffer[i++] = 0x80;
	for (; i < BLOCK_BYTES; ++i)
		m_buffer[i] = 0;

	/* We need a new block if current one doesn't have enough space to store the bit count */
	if ((m_total_bytes % BLOCK_BYTES) + 1 + 8 > BLOCK_BYTES)
	{
		Transform(m_buffer);
		memset(m_buffer, 0, BLOCK_BYTES - 8);
	}

	/* Append total_bits, split this uint64_t into two uint32_t */
	PUT_32BIT_LSB_FIRST(m_buffer + BLOCK_BYTES - 4, total_bits >> 32);
	PUT_32BIT_LSB_FIRST(m_buffer + BLOCK_BYTES - 8, total_bits & 0xffffffff);
	Transform(m_buffer);
#ifndef NDEBUG
	m_total_bytes |= static_cast<decltype(m_total_bytes)>(1) << (sizeof(m_total_bytes) * 8 - 1); // Use MSB as a flag indicating that Final() was called.
#endif
}

SECURE_FUNC_INTERNAL
void MD5::Reset()
{
	/* MD5 initialization constants */
	m_digest[0] = 0x67452301;
	m_digest[1] = 0xefcdab89;
	m_digest[2] = 0x98badcfe;
	m_digest[3] = 0x10325476;

	/* Reset counters */
	m_total_bytes = 0;
}

SECURE_FUNC_INTERNAL
void MD5::GetBytes(unsigned char buffer[16]) const
{
	assert((m_total_bytes >> (sizeof(m_total_bytes) * 8 - 1) != 0) && "MD5::Final() wasn't called");
	for (unsigned i = 0; i < DIGEST_INTS; ++i)
	{
		const auto x = m_digest[i];
		buffer[i*4+0] = (x      ) & 0xff;
		buffer[i*4+1] = (x >>  8) & 0xff;
		buffer[i*4+2] = (x >> 16) & 0xff;
		buffer[i*4+3] = (x >> 24) & 0xff;
	}
}

SECURE_FUNC_INTERNAL
void MD5::GetString(char buffer[33], bool upper_case) const
{
	assert((m_total_bytes >> (sizeof(m_total_bytes) * 8 - 1) != 0) && "MD5::Final() wasn't called");
	// We use non-const non-static array to force its initialization in the code.
	// This is less efficient but we need to avoid global static data because it is not protected unlike the code.
	char hex_char[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	if (upper_case)
	{
		hex_char[10] = 'A';
		hex_char[11] = 'B';
		hex_char[12] = 'C';
		hex_char[13] = 'D';
		hex_char[14] = 'E';
		hex_char[15] = 'F';
	}
	for (unsigned i = 0; i < DIGEST_INTS; ++i)
	{
		const auto x = m_digest[i];
		buffer[i*8+0] = hex_char[(x >>  4) & 0x0f];
		buffer[i*8+1] = hex_char[(x      ) & 0x0f];
		buffer[i*8+2] = hex_char[(x >> 12) & 0x0f];
		buffer[i*8+3] = hex_char[(x >>  8) & 0x0f];
		buffer[i*8+4] = hex_char[(x >> 20) & 0x0f];
		buffer[i*8+5] = hex_char[(x >> 16) & 0x0f];
		buffer[i*8+6] = hex_char[(x >> 28) & 0x0f];
		buffer[i*8+7] = hex_char[(x >> 24) & 0x0f];
	}
	buffer[32] = '\0';
}

/** The four core functions - F1 is optimized somewhat **/
/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/** This is the central step in the MD5 algorithm. **/
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data, w = w<<s | w>>(32-s), w += x )

/**
****************************************************************************
* The core of the MD5 algorithm, this alters an existing MD5 hash to
* reflect the addition of 16 longwords of new data.  MD5Update blocks
* the data and converts bytes into longwords for this routine.
******************************************************************************/
SECURE_FUNC_INTERNAL
void MD5::Transform(const unsigned char data[BLOCK_BYTES])
{
	uint32_t in[BLOCK_INTS];
	for (unsigned i = 0; i < BLOCK_INTS; ++i)
		in[i] = GET_32BIT_LSB_FIRST(data + 4 * i);

	uint32_t a = m_digest[0];
	uint32_t b = m_digest[1];
	uint32_t c = m_digest[2];
	uint32_t d = m_digest[3];

	MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
	MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
	MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
	MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
	MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
	MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
	MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
	MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
	MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
	MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
	MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
	MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
	MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
	MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
	MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
	MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

	MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
	MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
	MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
	MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
	MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
	MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
	MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
	MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
	MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
	MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
	MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
	MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
	MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
	MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
	MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
	MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

	MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
	MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
	MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
	MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
	MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
	MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
	MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
	MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
	MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
	MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
	MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
	MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
	MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
	MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
	MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
	MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

	MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
	MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
	MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
	MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
	MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
	MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
	MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
	MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
	MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
	MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
	MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
	MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
	MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
	MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
	MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
	MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

	m_digest[0] += a;
	m_digest[1] += b;
	m_digest[2] += c;
	m_digest[3] += d;
}

}

#undef GET_32BIT_LSB_FIRST
#undef PUT_32BIT_LSB_FIRST
#undef F1
#undef F2
#undef F3
#undef F4
#undef MD5STEP
