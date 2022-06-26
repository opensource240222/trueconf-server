#pragma once
#include "VS_AMFHeader.h"
#include "VS_AMFBody.h"

#include "VS_AMFVideoFrame.h"

class VS_AMFPacket
{
	VS_AMFPacket();

public:
	VS_AMFHeader	m_header;
	VS_AMFBody		m_body;

public:
	VS_AMFPacket(const VS_FLASHSessionData* sess);
	~VS_AMFPacket();

	void Clean();

	void InitHeader(rtmp_headersize_e hsz, const int amf_num, content_types_e ct);

	unsigned int Size();

	unsigned int Encode(void* out);
	unsigned int Decode(const void* in, const unsigned long in_sz = 0);

	void CreateAudioPacket(const void* data, const unsigned long data_sz);
	void CreateVideoPacket(const void* data, const unsigned long data_sz, int frame_type = VS_AMFVideoFrame::FRAME_KEY);
};