#pragma once

#include "../std/cpplib/VS_MediaFormat.h"

#ifdef _WIN32
#include <Windows.h>
#include <mmsystem.h> // Msacm.h fails without this
#include <mmreg.h>
#include <MSAcm.h>
#else
struct ACMSTREAMHEADER
{
	uint32_t           cbStruct;               // sizeof(ACMSTREAMHEADER)
	uint32_t           fdwStatus;              // ACMSTREAMHEADER_STATUSF_*
	uint32_t*       dwUser;                 // user instance data for hdr
	uint8_t*          pbSrc;
	uint32_t           cbSrcLength;
	uint32_t           cbSrcLengthUsed;
	uint32_t*       dwSrcUser;              // user instance data for src
	uint8_t*          pbDst;
	uint32_t           cbDstLength;
	uint32_t           cbDstLengthUsed;
	uint32_t*       dwDstUser;              // user instance data for dst
	//uint32_t           dwReservedDriver[_DRVRESERVED];   // driver reserved work space
};

struct WAVEFORMATEX
{
	uint16_t        wFormatTag;         /* format type */
	uint16_t        nChannels;          /* number of channels (i.e. mono, stereo...) */
	uint32_t       nSamplesPerSec;     /* sample rate */
	uint32_t       nAvgBytesPerSec;    /* for buffer estimation */
	uint16_t        nBlockAlign;        /* block size of data */
	uint16_t        wBitsPerSample;     /* number of bits per sample of mono data */
	uint16_t        cbSize;             /* the count in bytes of the size of */
									/* extra information (after cbSize) */
};

#define WAVE_FORMAT_PCM     1

#endif

class AudioCodec
{
	bool			m_coder;
	uint32_t			m_Tag;
protected:
    bool			m_valid;
    ACMSTREAMHEADER m_ash;
	WAVEFORMATEX*	m_pcmfmt;
	WAVEFORMATEX*	m_cdcfmt;
	uint32_t			m_GRAN;
	uint8_t*			m_inBuff;
	uint8_t*			m_outBuff;
	uint8_t*			m_DataPointer;
	static const int m_SIZE; // 1sec of mono data 44.1 kHz
public:
	AudioCodec(uint32_t outTag, bool coder, uint32_t gran);
	virtual ~AudioCodec();
	// set tag, channels and freq in *in;
	virtual int  Init(WAVEFORMATEX* /*in*/) {return 0;}
	virtual void Release() {};
	virtual int  Convert(uint8_t *in, uint8_t *out, uint32_t insize);
	virtual int  ConvertFunction() {return 0;}
	virtual void SetQuality(int /*quality*/) {};
	virtual void SetComplexity(int /*complexity*/) {};

	bool IsValid(){return m_valid;}
	bool IsCoder(){return m_coder;}
	uint32_t GetTag(){return m_Tag;}

	WAVEFORMATEX* GetPCMFormat(){
		if (m_valid)
			return m_pcmfmt;
		else
			return 0;
	}
	WAVEFORMATEX* GetCDCFormat(){
		if (m_valid)
			return m_cdcfmt;
		else
			return 0;
	}
};

AudioCodec* VS_RetriveAudioCodec(int Id, bool isCodec);
