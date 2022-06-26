#pragma once
#include "VS_AMFBase.h"
#include "VS_FLASHSessionData.h"

class VS_AMFAudioFrame: public VS_AMFBase
{
public:
	VS_FLASHSessionData* m_session;

	void*			m_data;
	unsigned long	m_data_sz;
	

public:
	VS_AMFAudioFrame();
	virtual ~VS_AMFAudioFrame();

	void SetData(const void* ptr, const unsigned int sz);
	void SetSession(const VS_FLASHSessionData* sess);

	unsigned int Encode(void* out);
	unsigned int Decode(const void* in, const unsigned long in_sz = 0);
};