#include "VS_AMFVideoFrame.h"
#include <memory.h>

VS_AMFVideoFrame::VS_AMFVideoFrame(): m_data(0), m_data_sz(0), m_session(0)
{

}

VS_AMFVideoFrame::~VS_AMFVideoFrame()
{

}
/*
VS_AMFVideoFrame::VS_AMFVideoFrame(const unsigned int content_sz)
{
	m_data_sz = content_sz;
}
*/
void VS_AMFVideoFrame::SetData(const void* ptr, const unsigned int sz)
{
	m_data = (void*) ptr;
	m_data_sz = sz;
}

void VS_AMFVideoFrame::SetSession(const VS_FLASHSessionData* sess)
{
	m_session = (VS_FLASHSessionData*) sess;
}

const void* VS_AMFVideoFrame::GetDataPtr() const
{
	return m_data;
}

unsigned int VS_AMFVideoFrame::GetDataSize() const
{
	return m_data_sz;
}

void VS_AMFVideoFrame::CopyFrom(const VS_AMFVideoFrame* frame)
{
	if ( !frame )
		return ;

	unsigned int sz = frame->GetDataSize();
	const void* data = frame->GetDataPtr();

	if ( !sz || !data )
		return ;

	m_data = (void*) data;
	m_data_sz = sz;
	m_frame_type = frame->m_frame_type;
}

unsigned int VS_AMFVideoFrame::Encode(void* out)
{
	if ( !out )
		return 0;

	char* ptr = (char*) out;

	char byte = 0;

	byte = m_session->m_vcodec & 0x0F;
	byte += (this->m_frame_type & 0x0F) << 4;

	ptr[0] = byte;

	memcpy((void*) &ptr[1], m_data, m_data_sz);

	return m_data_sz + 1;
}

unsigned int VS_AMFVideoFrame::Decode(const void* in, const unsigned long in_sz)
{
	// first byte
	if ( in_sz < m_data_sz )
		return 0;

	char* ptr = (char*) in;

	char byte = ptr[0];

	if ( m_session )
		m_session->m_vcodec = (VS_FLASHSessionData::V_CODEC) (byte & 0x0F);

	this->m_frame_type = (FRAME_TYPE) ((byte & 0xF0) >> 4);

	ptr++;		// skip start byte

	if ( m_data ) { delete m_data; m_data = 0; }

	m_data = new char[in_sz - 1];
	memcpy(m_data, (void*) ptr, in_sz-1);

	m_data_sz = in_sz;

	return in_sz;
}