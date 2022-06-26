#pragma once

#include "VideoCodec.h"
#include "VPXCodec.h"

class ScalableVideoCodec
{
public:
	ScalableVideoCodec() {};
	virtual ~ScalableVideoCodec() {};
	virtual void SetLayerBitrate(int id, int bitrate) = 0;
	virtual void DisableSLayer(int id) = 0;
	virtual void EnableSLayer(int id) = 0;
};

class VS_VPXVideoCodec : public VideoCodec, public ScalableVideoCodec
{
protected:
	VPXCodec		*m_VPXcodec;
	bool			UpdateBitrate();
public:
	VS_VPXVideoCodec(int CodecId, bool IsCoder);
	~VS_VPXVideoCodec();
	virtual int		Init(int w, int h, uint32_t ColorMode = FOURCC_I420, unsigned char sndLvl = 0, int numThreads = 1, unsigned int framerate = 10);
	void			Release();
	int				Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param);
	bool			SetSVCMode(uint32_t &param);
	bool			SetCoderOption(void *param);
	/// svc controls
	void SetLayerBitrate(int id, int bitrate) {};
	void DisableSLayer(int id) {};
	void EnableSLayer(int id) {};
};
