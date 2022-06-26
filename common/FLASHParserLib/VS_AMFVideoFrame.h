#pragma once
#include "VS_AMFBase.h"
#include "VS_FLASHSessionData.h"

class VS_AMFVideoFrame: public VS_AMFBase
{
	VS_FLASHSessionData*	m_session;

	void*					m_data;
	unsigned long			m_data_sz;

public:
	typedef enum {
		FRAME_KEY = 1,
		FRAME_INTERFRAME,
		FRAME_DISPOSABLE
	} FRAME_TYPE;


	FRAME_TYPE		m_frame_type;

public:
	VS_AMFVideoFrame();
//	VS_AMFVideoFrame(const unsigned int content_sz);
	virtual ~VS_AMFVideoFrame();

	void CopyFrom(const VS_AMFVideoFrame* frame);

	void SetData(const void* ptr, const unsigned int sz);
	void SetSession(const VS_FLASHSessionData* sess);

	const void* GetDataPtr() const;
	unsigned int GetDataSize() const;

	unsigned int Encode(void* out);
	unsigned int Decode(const void* in, const unsigned long in_sz = 0);
};