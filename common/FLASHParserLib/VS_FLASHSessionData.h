#pragma once

#include "VS_AMFHeader.h"
#include <map>

class VS_FLASHSessionData
{
public:						// make it private???
	typedef enum {
		RATE_5_5 = 0,
		RATE_11,
		RATE_22,
		RATE_44,
	} RATE;

	typedef enum {
		CODEC_RAW = 0,
		CODEC_ADPCM,
		CODEC_MP3,

		CODEC_NELLYMOSER_8_kHz = 5,
		CODEC_NELLYMOSER = 6
	} A_CODEC;

	typedef enum {
		CODEC_H263 = 2,
		CODEC_SCREEN,
		CODEC_VP6
	} V_CODEC;

public:
	RATE		m_rate;
	bool		m_bits;
	bool		m_stereo;
	A_CODEC		m_acodec;

	V_CODEC		m_vcodec;

	long		m_chunk_size;

private:
	long m_caller_id;

	typedef std::map<int, VS_AMFHeader*> AMF_HEADERS_MAP;
	typedef std::pair<int, VS_AMFHeader*> AMF_HEADERS_MAP_PAIR;
	AMF_HEADERS_MAP m_headers;

public:
	VS_FLASHSessionData();
	~VS_FLASHSessionData();

	void InitMedia();

	void SetCallerID(long id)
	{
		m_caller_id = id;
	}

	long GetCallerID() const
	{
		return m_caller_id;
	}

	const int GetAudioAMFNumber() const
	{
		return 3;
	}

	const int GetVideoAMFNumber() const
	{
		return 3;
	}

	VS_AMFHeader* GetLastHeader(const unsigned int amf_num);
	void AddHeader(const VS_AMFHeader* header);
};