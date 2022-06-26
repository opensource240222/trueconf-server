#pragma once
#include "VS_AMFBase.h"
#include <string>

class VS_AMFString: public VS_AMFBase
{
	bool			m_withFirst;

public:
	std::string		m_str;

public:
	VS_AMFString();
	virtual ~VS_AMFString();

	void WithFirst(bool withFirst = true);

	void SetStr(const char* str);
	const char* GetStr() const;

	unsigned int Encode(void* out);
	unsigned int Decode(const void* in, const unsigned long in_sz = 0);
};