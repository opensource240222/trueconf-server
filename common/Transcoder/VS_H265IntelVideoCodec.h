#pragma once

#include "VideoCodec.h"
#include "MfxUtils.h"

class VS_H265IntelVideoCodec : public VideoCodec
{
private:
	MFXVideoSession			m_mfxSession;
	MFXVideoENCODE			*m_pmfxEncoder;
	MFXVideoDECODE			*m_pmfxDecoder;
	MFXVideoVPP				*m_pmfxVPP;
	mfxVideoParam			m_mfxParams;
	mfxVideoParam			m_mfxVppParams;
	mfxFrameSurface1		*m_pSurfaces;
	mfxBitstream			m_mfxBS;
	mfxExtBuffer			*m_pCodingExtBuf;
	mfxExtCodingOption      m_CodingExtOption;
	mfxVersion				m_mfxVersion;

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
	bool			UpdateBitrate();
public:
	VS_H265IntelVideoCodec(int CodecId, bool IsCoder);
	virtual ~VS_H265IntelVideoCodec();
	int		Init(int w, int h, uint32_t ColorMode = FOURCC_I420, unsigned char sndLvl = 0, int numThreads = 1, unsigned int framerate = 10);
	void	Release();
	int		Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param);
	bool	SetCoderOption(void *param);
};
