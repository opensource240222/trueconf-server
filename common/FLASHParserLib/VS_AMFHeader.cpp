#include "VS_AMFHeader.h"
#include <memory.h>
#include "VS_FLASHSessionData.h"

VS_AMFHeader::VS_AMFHeader()
{
	Clean();
}

VS_AMFHeader::~VS_AMFHeader()
{

}

void VS_AMFHeader::Init(rtmp_headersize_e hsz, const int amf_num, content_types_e ct, const int csz, const unsigned long caller)
{
	m_header_sz = hsz;
	m_amf_number = amf_num;
	m_content_type = ct;
	m_content_sz = csz;
	m_caller = caller;
}

void VS_AMFHeader::Clean()
{
	m_session = 0;

	m_amf_number = 0;	
	m_header_sz = HEADER_12;
	m_content_type = NONE;
	m_content_sz = 0;
	m_caller = 0;
}

void VS_AMFHeader::CopyFrom(const VS_AMFHeader* in)
{
	if ( !in )
		return ;

	this->m_amf_number = in->m_amf_number;
	this->m_caller = in->m_caller;
	this->m_content_sz = in->m_content_sz;
	this->m_content_type = in->m_content_type;
	this->m_header_sz = in->m_header_sz;
}

void VS_AMFHeader::SetSession(const VS_FLASHSessionData* sess)
{
	m_session = (VS_FLASHSessionData*) sess;
}

VS_FLASHSessionData* VS_AMFHeader::GetSession() const
{
	return m_session;
}

unsigned int VS_AMFHeader::Encode(void* out)
{
	if ( !out || !m_content_sz )
		return 0;

	unsigned int n = 0;
	char first_byte = 0;

	first_byte = m_header_sz;
	first_byte += (m_amf_number & 0x3F);		// skip first 2 bits

	if ( m_header_sz <= HEADER_1 )
	{
		((char*)out)[0] = first_byte;
		n++;
	}

	if ( m_header_sz <= HEADER_4 )
	{
		// don't use: never received - not tested
		memset((void*)(((char*)out)+1), 0, 3);
		n += 3;
	}

	if ( m_header_sz <= HEADER_8 )
	{
		// size
		memset((void*)(((char*)out)+4), (m_content_sz & 0xFF0000) >> 16, 1);
		memset((void*)(((char*)out)+5), (m_content_sz & 0x00FF00) >> 8, 1);
		memset((void*)(((char*)out)+6), m_content_sz & 0x0000FF, 1);
		n += 3;
	}

	if ( (m_header_sz != HEADER_1) && (m_header_sz != HEADER_4) )	// size of header >= 8 bytes
	{
		((char*)out)[7] = m_content_type;
		n++;
	}

	if ( m_header_sz == HEADER_12 )
	{
		((int*)out)[2] = m_caller;
		n += 4;
	}

	return n;
}

unsigned int VS_AMFHeader::Decode(const void* in, const unsigned long in_sz)
{
	if ( !in || !in_sz )
		return 0;

	int n_to_read = 1;
	int n_received = 0;

	char* tmp = (char*) in;
	m_amf_number = tmp[0] & FLV_MASK_AMF_NUMBER;
	m_header_sz = (rtmp_headersize_e)(tmp[0] & FLV_MASK_HEADER_SIZE);				// получить старшие два бита

	n_to_read = GetHeaderSize(m_header_sz);

	if ( in_sz < n_to_read )
		return 0;

	n_to_read -= 1;				// skip first byte

	if (n_to_read == 0)			// 1 byte header
		return 1;
	else if (n_to_read == -1)	// invalid header
		return 0;

	if (m_header_sz == HEADER_4)		// currently, not supported
	{
		if ( !m_session )
			return 0;

		// try get from session
		VS_AMFHeader* last_header = m_session->GetLastHeader( this->m_amf_number );
		if ( !last_header )
			return 0;

		rtmp_headersize_e tmp_hsz = m_header_sz;

		this->CopyFrom(last_header);
		this->m_header_sz = tmp_hsz;

		return GetHeaderSize(m_header_sz);
	}

// header_8
	tmp = ((char*)in) + 3 + 1;									// skip 3 bytes and first byte
//	this->m_content_sz = (ntohl(*((int*)tmp)) & 0xFFFFFF00) >> 8;		// next 3 bytes is size

	this->m_content_sz |= (tmp[0] << 16)	& 0x00FF0000;
	this->m_content_sz |= (tmp[1] << 8)		& 0x0000FF00;
	this->m_content_sz |= (tmp[2] << 0)		& 0x000000FF;

	this->m_content_type = (content_types_e) (((char*)in)[7]);

	if (m_header_sz == HEADER_12)		// currently, not supported
	{
		tmp = ((char*)in) + 1 + 3 + 3 + 1;
		//m_caller = (unsigned long) (ntohl(*((int*)tmp)));
		m_caller |= (tmp[3] << 24)	& 0xFF000000;
		m_caller |= (tmp[2] << 16)	& 0x00FF0000;
		m_caller |= (tmp[1] << 8)	& 0x0000FF00;
		m_caller |= (tmp[0] << 0)	& 0x000000FF;
	}

	return n_to_read + 1;
}

unsigned int VS_AMFHeader::GetHeaderSize() const
{
	return this->GetHeaderSize(this->m_header_sz);
}

unsigned int VS_AMFHeader::GetHeaderSize(rtmp_headersize_e header_sz) const
{
	if (header_sz == HEADER_12)
		return 12;
	else if (header_sz == HEADER_8)
		return 8;
	else if (header_sz == HEADER_4)
		return 4;
	else if (header_sz == HEADER_1)
		return 1;
	else
		return 0;
}