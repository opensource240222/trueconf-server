/*
    sha1.h - header of

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

#pragma once

#include "std-generic/cpplib/string_view.h"

#include <cstdint>
#include <cstddef>

class SHA1
{
public:
	SHA1();
	void Update(const void* data, size_t size);
	void Update(string_view s)
	{
		Update(s.data(), s.size());
	}
	void Final();
	void Reset();

	void GetBytes(unsigned char buffer[20]) const;
	void GetString(char buffer[41]) const;

private:
	static const unsigned int BLOCK_INTS = 16;  /* number of 32bit integers per SHA1 block */
	static const unsigned int BLOCK_BYTES = BLOCK_INTS * 4;
	static const unsigned int DIGEST_INTS = 5;  /* number of 32bit integers per SHA1 digest */

	unsigned char m_buffer[BLOCK_BYTES];
	uint64_t m_total_bytes;
	uint32_t m_digest[DIGEST_INTS];

	void Transform(uint32_t block[BLOCK_INTS]);
	static void BufferToBlock(const void* data, uint32_t block[BLOCK_INTS]);
};
