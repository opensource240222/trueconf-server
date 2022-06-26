#pragma once

class VS_AMFBase
{
public:
	VS_AMFBase();
	virtual ~VS_AMFBase();

	virtual unsigned int Encode(void* out) = 0;
	virtual unsigned int Decode(const void* in, const unsigned long in_sz = 0) = 0;
};