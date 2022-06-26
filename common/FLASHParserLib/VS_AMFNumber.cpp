#include "VS_AMFNumber.h"

VS_AMFNumber::VS_AMFNumber()
{

}

VS_AMFNumber::~VS_AMFNumber()
{

}

void VS_AMFNumber::Init(const double d)
{
	m_number.d = d;
}

void VS_AMFNumber::Init(const char* c)
{
	for (unsigned int i=0; i <= 7; i++)
		m_number.c[7-i] = c[i];
}

unsigned int VS_AMFNumber::Encode(void* out)
{
	if ( !out )
		return 0;

	// swap 8 bytes
	for(unsigned int i=0; i <= 7; i++)
		((char*)out)[i+1] = m_number.c[7-i];

	// starts of AMF Number
	((char*)out)[0] = 0x00;

	return 8 + 1;
}

unsigned int VS_AMFNumber::Decode(const void* in, const unsigned long in_sz)
{
	if ( !in )
		return 0;

	char* ptr = (char*)in;
	if ( ptr[0] != 0x00 )
		return 0;

	ptr++;	// skip type byte

	// swap 8 bytes
	for(unsigned int i=0; i <= 7; i++)
		m_number.c[7-i] = ptr[i];

	return 8+1;
}