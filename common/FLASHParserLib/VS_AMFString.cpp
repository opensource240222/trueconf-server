#include "VS_AMFString.h"
#include <winsock2.h>

VS_AMFString::VS_AMFString():m_withFirst(true)
{

}

VS_AMFString::~VS_AMFString()
{

}

void VS_AMFString::WithFirst(bool withFirst)
{
	m_withFirst = withFirst;
}

void VS_AMFString::SetStr(const char* str)
{
	m_str = str;
}

const char* VS_AMFString::GetStr() const
{
	if ( !m_str.length() )
		return 0;

	return m_str.c_str();
}

unsigned int VS_AMFString::Encode(void* out)
{
	if ( !out )
		return 0;

	unsigned int len = (unsigned int) m_str.length();
	if ( !len )
		return false;

	// type(1)? + size(2) + string(len)
	char* ptr = ((char*)out);
	if (m_withFirst)
	{
		ptr[0] = 0x02;
		ptr++;
	}

	ptr[0] = len & 0x0000FF00;
	ptr[1] = len & 0x000000FF;

	memcpy((void*) (ptr+2),  (void*) m_str.c_str(), len);

	int sz = 2 + len;
	if (m_withFirst)
		sz++;

	return sz;
}

unsigned int VS_AMFString::Decode(const void* in, const unsigned long in_sz)
{
	if (!in)
		return 0;

	char* ptr = (char*) in;
	unsigned int n_total = 0;	

	// decode first byte (TYPE_STRING)
	if ( m_withFirst )
	{
		if ( ptr[0] != 0x02 )
			return 0;

		ptr++;
		n_total++;
	}

	// decode string size
	unsigned int sz = (ptr[0] << 8) + ptr[1];

	ptr += 2;
	n_total += 2;	// size	

	m_str.assign(ptr, sz);
	n_total += (unsigned int) m_str.length();

	return n_total;
}