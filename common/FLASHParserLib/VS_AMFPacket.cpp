#include "VS_AMFPacket.h"

#include "VS_AMFAudioFrame.h"

VS_AMFPacket::VS_AMFPacket()
{

}

VS_AMFPacket::~VS_AMFPacket()
{
	Clean();
}

VS_AMFPacket::VS_AMFPacket(const VS_FLASHSessionData* sess)
{
	m_body.SetSession(sess);
}

void VS_AMFPacket::InitHeader(rtmp_headersize_e hsz, const int amf_num, content_types_e ct)
{
	m_header.m_header_sz = hsz;
	m_header.m_amf_number = amf_num;
	m_header.m_content_type = ct;
}

void VS_AMFPacket::Clean()
{
	m_header.Clean();
	m_body.Clean();
}

unsigned int VS_AMFPacket::Size()
{
	return m_header.GetHeaderSize() + m_header.m_content_sz + m_body.m_body_delimiters;
}

unsigned int VS_AMFPacket::Encode(void* out)
{
	// skip header
	void* ptr = (void*) (((char*)out)+12);

	m_body.SetContentType(m_header.m_content_type);

	int n_body = m_body.Encode(ptr);
	m_header.m_content_sz = n_body;

	int n_header = m_header.Encode((void*) out);

	return n_header + n_body;
}

unsigned int VS_AMFPacket::Decode(const void* in, const unsigned long in_sz)
{
	if ( !in || !in_sz )
		return 0;

	// decode header
	int n = m_header.Decode(in, in_sz);
	if ( !n )
		return 0;

	int n_total = n;

	m_body.SetNeededBodySize(m_header.m_content_sz);
	m_body.SetContentType(m_header.m_content_type);

	unsigned int h_sz = m_header.GetHeaderSize();

	void* ptr = (char*)in + h_sz;
	unsigned long ptr_sz = in_sz - h_sz;

	// decode body
	n = m_body.Decode(ptr, ptr_sz);
	if ( !n )
		return 0;

	n_total += n;

	return n_total;
}

void VS_AMFPacket::CreateAudioPacket(const void* data, const unsigned long data_sz)
{
	this->m_header.Init(HEADER_12, this->m_body.GetSession()->GetAudioAMFNumber(), AUDIO_DATA,
						data_sz + 1, this->m_body.GetSession()->GetCallerID());

	VS_AMFAudioFrame* amf_frame = new VS_AMFAudioFrame;
	{
		amf_frame->SetData(data, data_sz);
		amf_frame->SetSession( this->m_body.GetSession() );
	}
	this->m_body.Add(amf_frame);
}

void VS_AMFPacket::CreateVideoPacket(const void* data, const unsigned long data_sz, int frame_type)
{
	this->m_header.Init(HEADER_12, this->m_body.GetSession()->GetVideoAMFNumber(), VIDEO_DATA,
						data_sz + 1, this->m_body.GetSession()->GetCallerID());

	VS_AMFVideoFrame* amf_frame = new VS_AMFVideoFrame;
	{
		amf_frame->m_frame_type = (VS_AMFVideoFrame::FRAME_TYPE) frame_type;
		amf_frame->SetData(data, data_sz);
		amf_frame->SetSession( this->m_body.GetSession() );
	}
	this->m_body.Add(amf_frame);
}