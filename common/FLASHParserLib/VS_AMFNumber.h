#pragma once
#include "VS_AMFBase.h"

class VS_AMFNumber: public VS_AMFBase
{
public:
	union LittleEndianDouble
	{
		double d;
		char c[8];
	};

	LittleEndianDouble	m_number;

public:
	VS_AMFNumber();
	virtual ~VS_AMFNumber();

	void Init(const double d);
	void Init(const char* c);

	unsigned int Encode(void* out);
	unsigned int Decode(const void* in, const unsigned long in_sz = 0);
};