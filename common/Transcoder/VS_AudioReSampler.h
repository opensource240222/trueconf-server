/****************************************************************************
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 *
 * Project: TransCoder
 *
 * Created by SMirnovK at 14.09.2004
 *
 * $History: VS_AudioReSampler.h $
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 2.06.11    Time: 19:32
 * Updated in $/VSNA/Transcoder
 * - speex resampler in transcoder
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 9.02.09    Time: 9:54
 * Updated in $/VSNA/Transcoder
 * - were improved speex aec
 * - were added audio devices frequency calculate
 * - were added speex resample in echo module
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/Transcoder
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Transcoder
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 15.09.04   Time: 12:53
 * Created in $/VS/Transcoder
 * adeed upResampling
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 14.09.04   Time: 19:40
 * Created in $/VS/Transcoder
 * added audio down sampling
 *
 ****************************************************************************/

/****************************************************************************
 * \file VS_AudioReSampler.h
 * \brief Audio Down sample class
 ****************************************************************************/
#ifndef VS_AUDIO_RESAMPLER_H
#define VS_AUDIO_RESAMPLER_H


/****************************************************************************
 * Classes
 ****************************************************************************/

/****************************************************************************
 * Audio Down sample class
 ****************************************************************************/
class VS_AudioReSampler
{
private:
	static const int	BUFFER_SIZE=512;
	void *		m_cbuffer;
	int			m_bytesPerSample;
	long		m_samp_frac;
	long		m_accum;
	int			m_holdover;
	long *		filter_bank;
	int			filter_width;

	long		m_old_rate;
	long		m_new_rate;

	bool		m_IsValid;

	bool Init(long old_rate, long new_rate);
	void Release();
	long Downsample(void *inbuffer, void *outbuffer, long srcsamples);
	long Upsample(void *inbuffer, void *outbuffer, long srcsamples);
public:
	VS_AudioReSampler();
	~VS_AudioReSampler();
	long Process(void *inbuffer, void *outbuffer, long InSize, int Infreq, int OutFreq);
};

struct SpeexResamplerState_;

class VS_AudioReSamplerSpeex
{
private:
	int					m_in_freq, m_out_freq;
	SpeexResamplerState_ *m_st;
	bool				 m_IsValid;
	int					 m_quality;
	bool				 Init(long old_rate, long new_rate);
	void				 Release();
public:
	VS_AudioReSamplerSpeex(int quality = 3);
	~VS_AudioReSamplerSpeex();
	long Process(void *in_buffer, void *out_buffer, long in_size, int in_freq, int out_freq);
};

#endif

/****************************************************************************
 * END OF FILE
 ****************************************************************************/
