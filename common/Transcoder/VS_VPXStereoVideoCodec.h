#pragma once

#include "VideoCodec.h"
#include "VPXCodec.h"
#include <Windows.h>

#define MAX_NUM_THREADS (64)

struct VS_VPXHDState
{
	VPXCodec			*vpx;
	HANDLE				handle_thread;
	HANDLE				handle_end_coding;
	HANDLE				handle_wait_coding;
	HANDLE				handle_wait_exit;
	unsigned char		*invideo;
	unsigned char		*outvideo;
	unsigned char		*Plane[3];
	int					offset[3];
	int					sizePlane[3];
	int					width;
	int					height;
	int					param;
	int					cmpsize;
};

class VS_VPXHDVideoCodec : public VideoCodec
{
protected:
	VS_VPXHDState	m_VPXcodecSt[MAX_NUM_THREADS];
	int				m_num_threads;
	unsigned char	*m_intmp, *m_outtmp;
	HANDLE			m_handle_end_coding[MAX_NUM_THREADS];
	int				ReInitDecoderHD(int num_threads);
	bool			UpdateBitrate();
	int m_width = 0;
	int m_height = 0;
public:
	VS_VPXHDVideoCodec(int CodecId, bool IsCoder);
	~VS_VPXHDVideoCodec();
	virtual int		Init(int w, int h, uint32_t ColorMode = FOURCC_I420, unsigned char sndLvl = 0, int numThreads = 1, unsigned int framerate = 10);
	void			Release();
	int				Convert(uint8_t *in, uint8_t* out, VS_VideoCodecParam* param);
	bool			SetCoderOption(void *param);
};

class VS_VPXStereoVideoCodec : public VS_VPXHDVideoCodec
{
public:
	VS_VPXStereoVideoCodec(int CodecId, bool IsCoder);
	~VS_VPXStereoVideoCodec();
	int				Init(int w, int h, uint32_t ColorMode = FOURCC_I420, unsigned char sndLvl = 0, int numThreads = 1, unsigned int framerate = 10);
};
