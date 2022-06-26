/**
 **************************************************************************
 * \file md5.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief This code declare interface for MD5 message-digest algorithm.
 *
 *	To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 *
 * \b Project Standart Libraries
 * \author The algorithm is due to Ron Rivest.
 * \author This code was written by Colin Plumb in 1993, no copyright is claimed.
 * \author Adopted by SMirnovK
 * \date 10.06.03
 *
 * $Revision: 1 $
 *
 * $History: md5.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/clib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/clib
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 30.12.04   Time: 14:12
 * Updated in $/VS/std/clib
 * files headers
 ****************************************************************************/

#pragma once

#include "std-generic/cpplib/string_view.h"

#include <cstddef>
#include <cstdint>

class MD5
{
public:
	MD5();
	void Update(const void* data, size_t size);
	void Update(string_view s)
	{
		Update(s.data(), s.size());
	}
	void Final();
	void Reset();

	void GetBytes(unsigned char buffer[16]) const;
	void GetString(char buffer[33], bool upper_case = true) const;

private:
	static const unsigned int BLOCK_INTS = 16;  /* number of 32bit integers per MD5 block */
	static const unsigned int BLOCK_BYTES = BLOCK_INTS * 4;
	static const unsigned int DIGEST_INTS = 4;  /* number of 32bit integers per MD5 digest */

	unsigned char m_buffer[BLOCK_BYTES];
	uint64_t m_total_bytes;
	uint32_t m_digest[DIGEST_INTS];

	void Transform(const unsigned char data[BLOCK_BYTES]);
};

inline void VS_ConvertToMD5(string_view in, char out[33])
{
	MD5 md5;
	md5.Update(in);
	md5.Final();
	md5.GetString(out);
}

// Makes string from <raw_md5> 16-bytes array and writes it into <buffer>.
// Size of buffer must be 33 bytes (32-bytes hashstring and \0).
void VS_MD5ToString(const unsigned char raw_md5[16], char buffer[33]);
