#include "VS_AMFBool.h"

VS_AMFBool::VS_AMFBool(): m_bool(false)
{

}

VS_AMFBool::~VS_AMFBool()
{

}

void VS_AMFBool::SetBool(const bool b)
{
	m_bool = b;
}

bool VS_AMFBool::GetBool() const
{
	return m_bool;
}

unsigned int VS_AMFBool::Encode(void* out)
{
	if ( !out )
		return 0;

	char* ptr = (char*) out;
	ptr[0] = 0x01;
	ptr[1] = m_bool;
	return 2;
}

unsigned int VS_AMFBool::Decode(const void* in, const unsigned long in_sz)
{
	if ( !in )
		return 0;

	char* ptr = (char*) in;
	if (ptr[0] != 0x01)
		return 0;

	m_bool = (ptr[1] == 0x01)? true: false;
	return 2;
}