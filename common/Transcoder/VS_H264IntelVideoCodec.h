#pragma once

#include "VideoCodec.h"
#include "intel_mdk/mfxvideo++.h"

#include <vector>

class VS_H264IntelVideoCodec : public VideoCodec
{
private:
	MFXVideoSession				m_mfxSession;
	MFXVideoENCODE				*m_pmfxEncoder;
	MFXVideoDECODE				*m_pmfxDecoder;
	MFXVideoVPP					*m_pmfxVPP;
	mfxVideoParam				m_mfxParams;
	mfxVideoParam				m_mfxVppParams;
	mfxFrameSurface1			*m_pSurfaces;
	mfxBitstream				m_mfxBS;
	mfxExtCodingOption			m_CodingExtOption;
	mfxExtCodingOption2			m_CodingExtOption2;
	std::vector<mfxExtBuffer*>	m_EncExtParams;
	mfxVersion					m_mfxVersion;

	unsigned short			m_numFramesQueue;
	bool					m_bInitFromBs;

	mfxStatus			AllocSurfaces(bool bCoder);
	void				DeleteSurfaces();
	void				ResetExtOptions();

#ifdef TEST_MDK_VCODEC
	FILE				*m_fStat;
	unsigned int		m_numPuchFrames;
#endif

protected:
	unsigned int	m_uLastKeyRequest;
	bool			m_bNextKey, m_bWaitKey;
	bool			m_bForceSoftware;
	int				m_maxSliceSize;
	int				m_deviceId;
	bool			UpdateBitrate();
public:
	VS_H264IntelVideoCodec(int CodecId, bool IsCoder);
	virtual ~VS_H264IntelVideoCodec();
	int		Init(int w, int h, uint32_t ColorMode = FOURCC_I420, unsigned char sndLvl = 0, int numThreads = 1, unsigned int framerate = 10);
	void	Release();
	int		Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param);
	bool	SetCoderOption(void *param);
};

// h.264 intel mdk only sw

class VS_H264SwIntelVideoCodec : public VS_H264IntelVideoCodec
{
public:
	VS_H264SwIntelVideoCodec(int CodecId, bool IsCoder) : VS_H264IntelVideoCodec(CodecId, IsCoder) {
		m_bForceSoftware = true;
	}
	virtual ~VS_H264SwIntelVideoCodec() {};
};

// h.264 intel mdk for transcoder

class VS_H264TranscoderIntelVideoCodec : public VS_H264IntelVideoCodec
{
public:
	VS_H264TranscoderIntelVideoCodec(int CodecId, bool IsCoder) : VS_H264IntelVideoCodec(CodecId, IsCoder) {
		m_maxSliceSize = 1400;
	}
	VS_H264TranscoderIntelVideoCodec(int CodecId, bool IsCoder, int32_t deviceid) : VS_H264IntelVideoCodec(CodecId, IsCoder) {
		m_maxSliceSize = 1400; m_deviceId = deviceid;
	}
	virtual ~VS_H264TranscoderIntelVideoCodec() {};
};
