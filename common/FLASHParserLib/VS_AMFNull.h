#pragma once
#include "VS_AMFBase.h"

class VS_AMFNull: public VS_AMFBase
{
public:
	VS_AMFNull();
	virtual ~VS_AMFNull();

	unsigned int Encode(void* out);
	unsigned int Decode(const void* in, const unsigned long in_sz = 0);
};