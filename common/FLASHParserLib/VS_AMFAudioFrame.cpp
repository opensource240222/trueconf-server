#include "VS_AMFAudioFrame.h"
#include <memory.h>

VS_AMFAudioFrame::VS_AMFAudioFrame(): m_data(0), m_data_sz(0), m_session(0)
{

}

VS_AMFAudioFrame::~VS_AMFAudioFrame()
{

}

void VS_AMFAudioFrame::SetData(const void* ptr, const unsigned int sz)
{
	m_data = (void*) ptr;
	m_data_sz = sz;
}

void VS_AMFAudioFrame::SetSession(const VS_FLASHSessionData* sess)
{
	m_session = (VS_FLASHSessionData*) sess;
}

unsigned int VS_AMFAudioFrame::Encode(void* out)
{
	if ( !out )
		return 0;

	char byte = 0;

	byte = (m_session->m_acodec & 0x0F) << 4;

	byte += (m_session->m_rate & 0x03) << 2;
	byte += (m_session->m_bits & 0x01) << 1;
	byte += (m_session->m_stereo & 0x01);

	char* ptr = (char*) out;
	ptr[0] = byte;

	memcpy((void*) &ptr[1], m_data, m_data_sz);

	return m_data_sz + 1;
}

unsigned int VS_AMFAudioFrame::Decode(const void* in, const unsigned long in_sz)
{

	return 0;
}