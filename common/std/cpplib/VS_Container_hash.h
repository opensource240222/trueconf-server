#pragma once
#include "std-generic/cpplib/VS_Container.h"
#include "std/cpplib/md5.h"

uint32_t CalcHashOnMemory(VS_Container& cnt)
{
	if (cnt.IsEmpty())
		return 0;		// invalid hash
	MD5 md5;
	md5.Update(cnt.m_value, cnt.m_valueSize);
	md5.Final();
	unsigned char digest[16];
	md5.GetBytes(digest);
	const auto p = reinterpret_cast<const uint32_t*>(digest);
	return p[0] ^ p[1] ^ p[2] ^ p[3];
}