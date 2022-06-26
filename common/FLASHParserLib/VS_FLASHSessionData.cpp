#include "VS_FLASHSessionData.h"

VS_FLASHSessionData::VS_FLASHSessionData(): m_chunk_size(128)
{
	InitMedia();
}

VS_FLASHSessionData::~VS_FLASHSessionData()
{
}

void VS_FLASHSessionData::InitMedia()
{
	m_rate = RATE_11;
	m_bits = true;		// 16 bits
	m_stereo = false;	// mono
	m_acodec = CODEC_RAW;

	m_vcodec = CODEC_H263;
}


VS_AMFHeader* VS_FLASHSessionData::GetLastHeader(const unsigned int amf_num)
{
	AMF_HEADERS_MAP::iterator it = m_headers.find( amf_num );

	if ( it != m_headers.end() )
		return it->second;
	else
		return 0;
}

void VS_FLASHSessionData::AddHeader(const VS_AMFHeader* header)
{
	AMF_HEADERS_MAP::iterator it = m_headers.find( header->m_amf_number );

	bool exist = false;
	if ( it != m_headers.end() )
		exist = true;

	VS_AMFHeader* h = 0;

	if ( exist )
		h = it->second;
	else
		h = new VS_AMFHeader;

	h->CopyFrom(header);

	AMF_HEADERS_MAP_PAIR p(h->m_amf_number, (VS_AMFHeader*) h);
	if ( !exist )
		m_headers.insert(p);
}