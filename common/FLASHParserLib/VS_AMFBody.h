#pragma once
#include "VS_AMFBase.h"
#include "VS_FLASHSessionData.h"
#include <vector>

class VS_AMFBody: public VS_AMFBase
{
	unsigned int				m_content_type;		// same as in header (set from header)

	unsigned int				m_body_sz_need;	

	VS_FLASHSessionData*		m_session;

public:
	std::vector<VS_AMFBase*>	m_cnt;
	unsigned int				m_body_delimiters;	

public:
	VS_AMFBody();
	~VS_AMFBody();

	void Clean();

	void SetNeededBodySize(const unsigned int body_sz_need);
	void SetContentType(const unsigned int t);

	void SetSession(const VS_FLASHSessionData* sess);
	VS_FLASHSessionData* GetSession() const;

	void Add(VS_AMFBase* value);
	void AddNull(void);
	void AddString(const char* str);
	void AddNumber(const double d);

	unsigned int CalcReplaces(const void* in, const unsigned long in_sz, const unsigned int chunk_sz = 0xffffffff) const;
	void Replace(const void* in, const unsigned long in_sz, const unsigned int chunk_sz = 0xffffffff);

	unsigned int Encode(void* out);
	unsigned int Decode(const void* in, const unsigned long in_sz = 0);
};