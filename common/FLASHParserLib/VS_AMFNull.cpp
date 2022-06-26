#include "VS_AMFNull.h"

VS_AMFNull::VS_AMFNull()
{

}

VS_AMFNull::~VS_AMFNull()
{

}

unsigned int VS_AMFNull::Encode(void* out)
{
	if ( !out )
		return 0;

	((char*)out)[0] = 0x05;
	return 1;
}

unsigned int VS_AMFNull::Decode(const void* in, const unsigned long in_sz)
{
	if ( !in /*|| (((char*)in)[0] != 0x05)*/ )	// tmp hack (for 0x06 support)
		return 0;

	return 1;
}