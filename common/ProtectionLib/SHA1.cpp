/*
    sha1.cpp - source code of

    ============
    SHA-1 in C++
    ============

    100% Public Domain.

    Original C Code
        -- Steve Reid <steve@edmweb.com>
    Small changes to fit into bglibs
        -- Bruce Guenter <bruce@untroubled.org>
    Translation to simpler C++ Code
        -- Volker Grabsch <vog@notjusthosting.com>
*/

#include "ProtectionLib/SHA1.h"
#include "ProtectionLib/Lib.h"
#include "ProtectionLib/Protection.h"

#include <cassert>

namespace protection {

/* Help macros */
#define SHA1_ROL(value, bits) (((value) << (bits)) | (((value) & 0xffffffff) >> (32 - (bits))))
#define SHA1_BLK(i) (block[i&15] = SHA1_ROL(block[(i+13)&15] ^ block[(i+8)&15] ^ block[(i+2)&15] ^ block[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define SHA1_R0(v,w,x,y,z,i) z += ((w&(x^y))^y)     + block[i]    + 0x5a827999 + SHA1_ROL(v,5); w=SHA1_ROL(w,30);
#define SHA1_R1(v,w,x,y,z,i) z += ((w&(x^y))^y)     + SHA1_BLK(i) + 0x5a827999 + SHA1_ROL(v,5); w=SHA1_ROL(w,30);
#define SHA1_R2(v,w,x,y,z,i) z += (w^x^y)           + SHA1_BLK(i) + 0x6ed9eba1 + SHA1_ROL(v,5); w=SHA1_ROL(w,30);
#define SHA1_R3(v,w,x,y,z,i) z += (((w|x)&y)|(w&x)) + SHA1_BLK(i) + 0x8f1bbcdc + SHA1_ROL(v,5); w=SHA1_ROL(w,30);
#define SHA1_R4(v,w,x,y,z,i) z += (w^x^y)           + SHA1_BLK(i) + 0xca62c1d6 + SHA1_ROL(v,5); w=SHA1_ROL(w,30);

// m_buffer is not initialized intentionally, reading from it is guarded by m_total_bytes.
// cppcheck-suppress uninitMemberVar symbolName=SHA1::m_buffer
SECURE_FUNC_INTERNAL
SHA1::SHA1()
{
	/* SHA1 initialization constants */
	m_digest[0] = 0x67452301;
	m_digest[1] = 0xefcdab89;
	m_digest[2] = 0x98badcfe;
	m_digest[3] = 0x10325476;
	m_digest[4] = 0xc3d2e1f0;

	/* Reset counters */
	m_total_bytes = 0;
}

SECURE_FUNC_INTERNAL
void SHA1::Update(const void* data, size_t size)
{
	assert((m_total_bytes >> (sizeof(m_total_bytes) * 8 - 1) == 0) && "SHA1::Update() is called after SHA1::Final()");

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

		uint32_t block[BLOCK_INTS];
		BufferToBlock(m_buffer, block);
		Transform(block);
	}

	/* Process whole blocks */
	while (size >= BLOCK_BYTES)
	{
		uint32_t block[BLOCK_INTS];
		BufferToBlock(p, block);
		Transform(block);

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


/*
 * Add padding and calculate the final digest value.
 */

SECURE_FUNC_INTERNAL
void SHA1::Final()
{
	/* Total number of hashed bits */
	const uint64_t total_bits = m_total_bytes * 8;

	/* Padding */
	unsigned i = m_total_bytes % BLOCK_BYTES;
	m_buffer[i++] = 0x80;
	for (; i < BLOCK_BYTES; ++i)
		m_buffer[i] = 0;

	uint32_t block[BLOCK_INTS];
	BufferToBlock(m_buffer, block);

	/* We need a new block if current one doesn't have enough space to store the bit count */
	if ((m_total_bytes % BLOCK_BYTES) + 1 + 8 > BLOCK_BYTES)
	{
		Transform(block);
		for (unsigned j = 0; j < BLOCK_INTS; ++j)
			block[j] = 0;
	}

	/* Append total_bits, split this uint64_t into two uint32_t */
	block[BLOCK_INTS - 1] = total_bits & 0xffffffff;
	block[BLOCK_INTS - 2] = total_bits >> 32;
	Transform(block);
#ifndef NDEBUG
	m_total_bytes |= static_cast<decltype(m_total_bytes)>(1) << (sizeof(m_total_bytes) * 8 - 1); // Use MSB as a flag indicating that Final() was called.
#endif
}

SECURE_FUNC_INTERNAL
void SHA1::Reset()
{
	/* SHA1 initialization constants */
	m_digest[0] = 0x67452301;
	m_digest[1] = 0xefcdab89;
	m_digest[2] = 0x98badcfe;
	m_digest[3] = 0x10325476;
	m_digest[4] = 0xc3d2e1f0;

	/* Reset counters */
	m_total_bytes = 0;
}

SECURE_FUNC_INTERNAL
void SHA1::GetBytes(unsigned char buffer[20]) const
{
	assert((m_total_bytes >> (sizeof(m_total_bytes) * 8 - 1) != 0) && "SHA1::Final() wasn't called");
	for (unsigned i = 0; i < DIGEST_INTS; ++i)
	{
		const auto x = m_digest[i];
		buffer[i*4+0] = (x >> 24) & 0xff;
		buffer[i*4+1] = (x >> 16) & 0xff;
		buffer[i*4+2] = (x >>  8) & 0xff;
		buffer[i*4+3] = (x      ) & 0xff;
	}
}

SECURE_FUNC_INTERNAL
void SHA1::GetString(char buffer[41]) const
{
	assert((m_total_bytes >> (sizeof(m_total_bytes) * 8 - 1) != 0) && "SHA1::Final() wasn't called");
	// We use non-const non-static array to force its initialization in the code.
	// This is less efficient but we need to avoid global static data because it is not protected unlike the code.
	char hex_char[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	for (unsigned i = 0; i < DIGEST_INTS; ++i)
	{
		const auto x = m_digest[i];
		buffer[i*8+0] = hex_char[(x >> 28) & 0x0f];
		buffer[i*8+1] = hex_char[(x >> 24) & 0x0f];
		buffer[i*8+2] = hex_char[(x >> 20) & 0x0f];
		buffer[i*8+3] = hex_char[(x >> 16) & 0x0f];
		buffer[i*8+4] = hex_char[(x >> 12) & 0x0f];
		buffer[i*8+5] = hex_char[(x >>  8) & 0x0f];
		buffer[i*8+6] = hex_char[(x >>  4) & 0x0f];
		buffer[i*8+7] = hex_char[(x      ) & 0x0f];
	}
	buffer[40] = '\0';
}

/*
 * Hash a single 512-bit block. This is the core of the algorithm.
 */

SECURE_FUNC_INTERNAL
void SHA1::Transform(uint32_t block[BLOCK_INTS])
{
	/* Copy m_digest[] to working vars */
	uint32_t a = m_digest[0];
	uint32_t b = m_digest[1];
	uint32_t c = m_digest[2];
	uint32_t d = m_digest[3];
	uint32_t e = m_digest[4];


	/* 4 rounds of 20 operations each. Loop unrolled. */
	SHA1_R0(a,b,c,d,e, 0);
	SHA1_R0(e,a,b,c,d, 1);
	SHA1_R0(d,e,a,b,c, 2);
	SHA1_R0(c,d,e,a,b, 3);
	SHA1_R0(b,c,d,e,a, 4);
	SHA1_R0(a,b,c,d,e, 5);
	SHA1_R0(e,a,b,c,d, 6);
	SHA1_R0(d,e,a,b,c, 7);
	SHA1_R0(c,d,e,a,b, 8);
	SHA1_R0(b,c,d,e,a, 9);
	SHA1_R0(a,b,c,d,e,10);
	SHA1_R0(e,a,b,c,d,11);
	SHA1_R0(d,e,a,b,c,12);
	SHA1_R0(c,d,e,a,b,13);
	SHA1_R0(b,c,d,e,a,14);
	SHA1_R0(a,b,c,d,e,15);
	SHA1_R1(e,a,b,c,d,16);
	SHA1_R1(d,e,a,b,c,17);
	SHA1_R1(c,d,e,a,b,18);
	SHA1_R1(b,c,d,e,a,19);
	SHA1_R2(a,b,c,d,e,20);
	SHA1_R2(e,a,b,c,d,21);
	SHA1_R2(d,e,a,b,c,22);
	SHA1_R2(c,d,e,a,b,23);
	SHA1_R2(b,c,d,e,a,24);
	SHA1_R2(a,b,c,d,e,25);
	SHA1_R2(e,a,b,c,d,26);
	SHA1_R2(d,e,a,b,c,27);
	SHA1_R2(c,d,e,a,b,28);
	SHA1_R2(b,c,d,e,a,29);
	SHA1_R2(a,b,c,d,e,30);
	SHA1_R2(e,a,b,c,d,31);
	SHA1_R2(d,e,a,b,c,32);
	SHA1_R2(c,d,e,a,b,33);
	SHA1_R2(b,c,d,e,a,34);
	SHA1_R2(a,b,c,d,e,35);
	SHA1_R2(e,a,b,c,d,36);
	SHA1_R2(d,e,a,b,c,37);
	SHA1_R2(c,d,e,a,b,38);
	SHA1_R2(b,c,d,e,a,39);
	SHA1_R3(a,b,c,d,e,40);
	SHA1_R3(e,a,b,c,d,41);
	SHA1_R3(d,e,a,b,c,42);
	SHA1_R3(c,d,e,a,b,43);
	SHA1_R3(b,c,d,e,a,44);
	SHA1_R3(a,b,c,d,e,45);
	SHA1_R3(e,a,b,c,d,46);
	SHA1_R3(d,e,a,b,c,47);
	SHA1_R3(c,d,e,a,b,48);
	SHA1_R3(b,c,d,e,a,49);
	SHA1_R3(a,b,c,d,e,50);
	SHA1_R3(e,a,b,c,d,51);
	SHA1_R3(d,e,a,b,c,52);
	SHA1_R3(c,d,e,a,b,53);
	SHA1_R3(b,c,d,e,a,54);
	SHA1_R3(a,b,c,d,e,55);
	SHA1_R3(e,a,b,c,d,56);
	SHA1_R3(d,e,a,b,c,57);
	SHA1_R3(c,d,e,a,b,58);
	SHA1_R3(b,c,d,e,a,59);
	SHA1_R4(a,b,c,d,e,60);
	SHA1_R4(e,a,b,c,d,61);
	SHA1_R4(d,e,a,b,c,62);
	SHA1_R4(c,d,e,a,b,63);
	SHA1_R4(b,c,d,e,a,64);
	SHA1_R4(a,b,c,d,e,65);
	SHA1_R4(e,a,b,c,d,66);
	SHA1_R4(d,e,a,b,c,67);
	SHA1_R4(c,d,e,a,b,68);
	SHA1_R4(b,c,d,e,a,69);
	SHA1_R4(a,b,c,d,e,70);
	SHA1_R4(e,a,b,c,d,71);
	SHA1_R4(d,e,a,b,c,72);
	SHA1_R4(c,d,e,a,b,73);
	SHA1_R4(b,c,d,e,a,74);
	SHA1_R4(a,b,c,d,e,75);
	SHA1_R4(e,a,b,c,d,76);
	SHA1_R4(d,e,a,b,c,77);
	SHA1_R4(c,d,e,a,b,78);
	SHA1_R4(b,c,d,e,a,79);

	/* Add the working vars back into m_digest[] */
	m_digest[0] += a;
	m_digest[1] += b;
	m_digest[2] += c;
	m_digest[3] += d;
	m_digest[4] += e;
}

SECURE_FUNC_INTERNAL
void SHA1::BufferToBlock(const void* data, uint32_t block[BLOCK_INTS])
{
	auto p = static_cast<const uint8_t*>(data);
	for (unsigned i = 0; i < BLOCK_INTS; ++i)
	{
		block[i] = static_cast<uint32_t>(p[3])
			| static_cast<uint32_t>(p[2]) << 8
			| static_cast<uint32_t>(p[1]) << 16
			| static_cast<uint32_t>(p[0]) << 24;
		p += 4;
	}
}

}

#undef SHA1_ROL
#undef SHA1_BLK
#undef SHA1_R0
#undef SHA1_R1
#undef SHA1_R2
#undef SHA1_R3
#undef SHA1_R4
