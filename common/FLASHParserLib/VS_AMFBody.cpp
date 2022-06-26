#include "VS_AMFBody.h"

#include "VS_AMFNull.h"
#include "VS_AMFString.h"
#include "VS_AMFNumber.h"
#include "VS_AMFFactory.h"

VS_AMFBody::VS_AMFBody()
{
	Clean();
}

VS_AMFBody::~VS_AMFBody()
{
	Clean();
}

void VS_AMFBody::Clean()
{
//	m_flex_prefix = false;
	m_content_type = 0;
	m_body_sz_need = -1;
	m_body_delimiters = 0;
	m_session = 0;

	for(unsigned int i=0; i < m_cnt.size(); i++)
		if (m_cnt[i]) { delete m_cnt[i]; m_cnt[i] = 0; }

	m_cnt.clear();
}
/*
void VS_AMFBody::SetFLEXMessage(const bool IsFLEXPrefix)
{
	m_flex_prefix = IsFLEXPrefix;		
}
*/
void VS_AMFBody::SetContentType(const unsigned int t)
{
	m_content_type = t;
}

void VS_AMFBody::SetNeededBodySize(const unsigned int body_sz_need)
{
	m_body_sz_need = body_sz_need;
}

void VS_AMFBody::SetSession(const VS_FLASHSessionData* sess)
{
	m_session = (VS_FLASHSessionData*) sess;
}

VS_FLASHSessionData* VS_AMFBody::GetSession() const
{
	return m_session;
}

void VS_AMFBody::Add(VS_AMFBase* value)
{
	m_cnt.push_back(value);
}

void VS_AMFBody::AddNull(void)
{
	this->Add(new VS_AMFNull);
}

void VS_AMFBody::AddString(const char* str)
{
	if ( !str )
		return ;

	VS_AMFString* amf_str = new VS_AMFString;
	amf_str->SetStr(str);
	
	m_cnt.push_back((VS_AMFBase*) amf_str);
}

void VS_AMFBody::AddNumber(const double d)
{
	VS_AMFNumber* amf_num = new VS_AMFNumber;
	amf_num->Init(d);
	
	m_cnt.push_back((VS_AMFBase*) amf_num);
}
unsigned int VS_AMFBody::Encode(void* out)
{
	if ( !out )
		return 0;

	void* ptr = out;
	unsigned int n = 0;
	unsigned int n_total = 0;

	if ( m_content_type == 0x11 )		// HeaderType is FLEX message, so first byte is null
	{
		((char*)ptr)[0] = 0x00;
		ptr = (void*)((char*)ptr + 1);
		n_total++;
	}

	for(unsigned int i=0; i < m_cnt.size(); i++)
	{
		n = m_cnt[i]->Encode(ptr);
		if (n)
		{
			ptr = (void*) (((char*)ptr)+n);
			n_total += n;
		}
	}

	CalcReplaces(out, n_total);

	return n_total;
}

unsigned int VS_AMFBody::Decode(const void* in, const unsigned long in_sz)
{
	if ( !in || (in_sz < m_body_sz_need) || !m_session )
		return 0;

	void* ptr = (void*) in;
	int n_total = 0;
	VS_AMFFactory* factory = VS_AMFFactory::Instance();

	if ( m_content_type == VIDEO_DATA || m_content_type == AUDIO_DATA )
	{
		unsigned int chunk_size = (m_content_type == VIDEO_DATA)? 128: 64;

		m_body_delimiters = CalcReplaces(in, in_sz, m_session->m_chunk_size);

		if (in_sz < (m_body_sz_need + m_body_delimiters))
			return 0;

		Replace(in, in_sz, m_session->m_chunk_size);

		VS_AMFBase* base = factory->CreateDecoder(m_content_type);
		if ( !base )
			return 0;

		int n = base->Decode(ptr, in_sz);
		if ( !n )
			return 0;

		ptr = ((char*)ptr) + n;

//		m_cnt.push_back(base);		// todo: DELETE!!!

		this->Add(base);
		n_total += n;
	
	} else{

		m_body_delimiters = CalcReplaces(in, in_sz, m_session->m_chunk_size);

		if (in_sz < (m_body_sz_need + m_body_delimiters))
			return 0;

		Replace(in, in_sz, m_session->m_chunk_size);

		if ( m_content_type == 0x11 )		// skip first byte: FLEX message
		{									// start: 0x00
			ptr = ((char*)ptr) + 1;			// stop:  0x05 (included in header.content_size)
			n_total++;
		}

		while (( n_total + m_body_delimiters) < in_sz )
		{
			VS_AMFBase* base = factory->CreateDecoder(ptr);
			if ( !base )
				return 0;

			int n = base->Decode(ptr, in_sz - n_total);
			if ( !n )
				return 0;

			ptr = ((char*)ptr) + n;

			this->Add(base);
			n_total += n;
		}

		n_total += m_body_delimiters;	// add replaces
	}

	return n_total;
}

unsigned int VS_AMFBody::CalcReplaces(const void* in, const unsigned long in_sz, const unsigned int chunk_sz) const
{
	if ( !in || !in_sz )
		return 0;

	unsigned int replaces = 0;

	char* ptr = (char*) in;
	unsigned long n = 0;

	while( (in_sz - n) > chunk_sz )
	{
		ptr += chunk_sz;
		n += chunk_sz;
		replaces++;

		ptr++;				// when calc - skip delimiter char
		n++;
	}
	return replaces;
}

void VS_AMFBody::Replace(const void* in, const unsigned long in_sz, const unsigned int chunk_sz)
{
	if ( !in || !in_sz )
		return ;

	char* ptr = (char*) in;
	unsigned long n = 0;

	while( (in_sz - n) > chunk_sz )
	{
		ptr += chunk_sz;
		n += chunk_sz;
		memcpy(ptr, ptr+1, in_sz-n);
	}
}