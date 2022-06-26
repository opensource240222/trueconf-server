#pragma once
#include "VS_AMFBase.h"
class VS_FLASHSessionData;

typedef enum {
	HEADER_12 = 0x0,
	HEADER_8  = 0x40,
	HEADER_4  = 0x80,
	HEADER_1  = 0xc0
} rtmp_headersize_e;

typedef enum {
	NONE = 0x0,
	CHUNK_SIZE = 0x1,
	UNKNOWN = 0x2,
	BYTES_READ = 0x3,
	PING = 0x4,
	SERVER = 0x5,
	CLIENT = 0x6,
	UNKNOWN2 = 0x7,
	AUDIO_DATA = 0x8,
	VIDEO_DATA = 0x9,
	UNKNOWN3 = 0xa,
	NOTIFY = 0x12,
	SHARED_OBJ = 0x13,
	INVOKE = 0x14,
	FLV_DATA = 0x16
} content_types_e;

#define FLV_MASK_HEADER_SIZE 0xC0
#define FLV_MASK_AMF_NUMBER 0x3F

class VS_AMFHeader
{
public:
	rtmp_headersize_e	m_header_sz;
	int					m_amf_number;

	content_types_e		m_content_type;
	int					m_content_sz;

	unsigned long		m_caller;
public:
	VS_AMFHeader();
	~VS_AMFHeader();

	void Clean();
	void CopyFrom(const VS_AMFHeader* in);

	void Init(rtmp_headersize_e hsz, const int amf_num,
		content_types_e ct, const int csz, const unsigned long caller = 0);

	unsigned int Encode(void* out);
	unsigned int Decode(const void* in, const unsigned long in_sz = 0);

	unsigned int GetHeaderSize() const;

	void SetSession(const VS_FLASHSessionData* sess);
	VS_FLASHSessionData* GetSession() const;

private:
	unsigned int GetHeaderSize(rtmp_headersize_e header_sz) const;

	VS_FLASHSessionData*		m_session;
};