#pragma once
#include "VS_AMFBase.h"
#include <vector>

class VS_AMFObjectsContainer: public VS_AMFBase
{
private:
	typedef std::pair<std::string, VS_AMFBase*> TPair;
	typedef std::vector<TPair> TContainer;
	TContainer	m_objs;

public:
	VS_AMFObjectsContainer();
	virtual ~VS_AMFObjectsContainer();

	void Add(const char* key, VS_AMFBase* value);
	void Add(const char* key, const char* value);
	void Add(const char* key, const double value);

	unsigned int Encode(void* out);
	unsigned int Decode(const void* in, const unsigned long in_sz = 0);

private:
	bool IsEnd(const char* in);
};