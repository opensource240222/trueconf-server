#pragma once
#include "VS_AMFBase.h"

class VS_AMFBool: public VS_AMFBase
{
	bool	m_bool;
public:
	VS_AMFBool();
	virtual ~VS_AMFBool();

	void SetBool(const bool str);
	bool GetBool() const;

	unsigned int Encode(void* out);
	unsigned int Decode(const void* in, const unsigned long in_sz = 0);
};