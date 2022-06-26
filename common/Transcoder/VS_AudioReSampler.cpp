#ifdef _WIN32
/****************************************************************************
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 *
 * Project: TransCoder
 *
 * Created by SMirnovK at 14.09.2004
 *
 * $History: VS_AudioReSampler.cpp $
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 2.06.11    Time: 19:32
 * Updated in $/VSNA/Transcoder
 * - speex resampler in transcoder
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 17.08.10   Time: 14:55
 * Updated in $/VSNA/Transcoder
 * - change speex resample quality (set to 3)
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 15.07.10   Time: 12:39
 * Updated in $/VSNA/Transcoder
 * - fix out of memory for high frequency audio resampling
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 9.02.09    Time: 9:54
 * Updated in $/VSNA/Transcoder
 * - were improved speex aec
 * - were added audio devices frequency calculate
 * - were added speex resample in echo module
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:01
 * Updated in $/VSNA/Transcoder
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 26.12.07   Time: 13:23
 * Updated in $/VS2005/Transcoder
 * - more accurate resampling
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
 * \file VS_AudioReSampler.cpp
 * \brief Audio Down sample class
 ****************************************************************************/



/****************************************************************************
 * Includes
 ****************************************************************************/
#include <math.h>
#include <windows.h>
#include "VS_AudioReSampler.h"

#include <speex/speex_resampler.h>

#include <cstdlib>

/****************************************************************************
 * Globals
 ****************************************************************************/
/****************************************************************************
 * Round
 ****************************************************************************/
int VDRoundToInt(double x) {
	return (int)floor(x + 0.5);
}

/****************************************************************************
 * base downsampling routine
 ****************************************************************************/
static long audio_downsample_mono16(void *dst, void *src, long *filter_bank, int filter_width, long accum, long samp_frac, long cnt) {
	signed short *d = (signed short *)dst;
	signed short *s = (signed short *)src;

	do {
		long sum = 0;
		int w;
		long *fb_ptr;
		signed short *s_ptr;

		w = filter_width;
		fb_ptr = filter_bank + filter_width * ((accum>>11)&0xff);
		s_ptr = s + (accum>>19);
		do {
			sum += *fb_ptr++ * (int)*s_ptr++;
		} while(--w);

		if (sum < -0x20000000)
			*d++ = -0x8000;
		else if (sum > 0x1fffffff)
			*d++ = 0x7fff;
		else
			*d++ = (short)((sum + 0x2000)>>14);

		accum += samp_frac;
	} while(--cnt);

	return accum;
}


/****************************************************************************
 * base upsampling routine
 ****************************************************************************/
static long audio_upsample_mono16(void *dst, void *src, long accum, long samp_frac, long cnt) {
	signed short *d = (signed short *)dst;
	signed short *s = (signed short *)src;

	do {
		signed short *s_ptr = s + (accum>>19);
		long frac = (accum>>3) & 0xffff;

		*d++ = (short)(((int)s_ptr[0] * (0x10000 - frac) + (int)s_ptr[1] * frac) >> 16);
		accum += samp_frac;
	} while(--cnt);

	return accum;
}

/****************************************************************************
 * helper function
 ****************************************************************************/
static int permute_index(int a, int b) {
	return (b-(a>>8)-1) + (a&255)*b;
}

/****************************************************************************
 * make filter coeficients
 ****************************************************************************/
static void make_downsample_filter(long *filter_bank, int filter_width, long samp_frac) {
	int i, j, v;
	double filt_max;
	double filtwidth_frac;

	filtwidth_frac = samp_frac/2048.0;
	filter_bank[filter_width-1] = 0;
	filt_max = (16384.0 * 524288.0) / samp_frac;

	for(i=0; i<128*filter_width; i++) {
		int y = 0;
		double d = i / filtwidth_frac;
		if (d<1.0)
			y = VDRoundToInt(filt_max*(1.0 - d));
		filter_bank[permute_index(128*filter_width + i, filter_width)]
			= filter_bank[permute_index(128*filter_width - i, filter_width)]
			= y;
	}
	// Normalize the filter to correct for integer roundoff errors
	for(i=0; i<256*filter_width; i+=filter_width) {
		v=0;
		for(j=0; j<filter_width; j++)
			v += filter_bank[i+j];

		v = (0x4000 - v)/filter_width;
		for(j=0; j<filter_width; j++)
			filter_bank[i+j] += v;
	}
}

/****************************************************************************
 * Classes
 ****************************************************************************/
/****************************************************************************
 * Audio Down sample class
 ****************************************************************************/
/****************************************************************************
 * Constructor
 ****************************************************************************/
VS_AudioReSampler::VS_AudioReSampler() {
	memset(this, 0, sizeof(VS_AudioReSampler));
}

/****************************************************************************
 * Destructor
 ****************************************************************************/
VS_AudioReSampler::~VS_AudioReSampler() {
	Release();
}

/****************************************************************************
 * Constructor
 ****************************************************************************/
bool VS_AudioReSampler::Init(long old_rate, long new_rate) {
	Release();
	m_old_rate = old_rate;
	m_new_rate = new_rate;
	m_samp_frac = MulDiv(old_rate, 0x80000L, new_rate);

	m_bytesPerSample = 2;
	m_cbuffer = malloc(m_bytesPerSample * BUFFER_SIZE);
	memset(m_cbuffer, 0, m_bytesPerSample*BUFFER_SIZE);
	m_holdover = 0;
	filter_bank = NULL;
	filter_width = 1;
	m_accum=0;

	if (new_rate < old_rate) {
		// HQ downsample: allocate filter bank
		filter_width = ((m_samp_frac + 0x7ffff)>>19)<<1;
		filter_bank = new long[filter_width * 256];
		make_downsample_filter(filter_bank, filter_width, m_samp_frac);
		memset(m_cbuffer, 0, m_bytesPerSample*filter_width);
		m_holdover = filter_width/2;
	}
	m_IsValid = true;
	return m_IsValid;
}

/****************************************************************************
 * release resources
 ****************************************************************************/
void VS_AudioReSampler::Release() {
	if (m_cbuffer) free(m_cbuffer);
	if (filter_bank) delete[] filter_bank;
	memset(this, 0, sizeof(VS_AudioReSampler));
}


/****************************************************************************
 * Downsample method
 ****************************************************************************/
long VS_AudioReSampler::Downsample(void *inbuffer, void *outbuffer, long srcsamples) {
	long samples = (long)(((((__int64)srcsamples+m_holdover-filter_width)<<19) + 0x7ffff - m_accum)/m_samp_frac + 1);
	long lActualSamples=0;

	// Downsampling is even worse because we have overlap to the left and to the
	// right of the interpolated point.
	//
	// We need (n/2) points to the left and (n/2-1) points to the right.
	while(samples>0) {
		long srcSamples, dstSamples;
		int nhold;

		// Figure out how many source samples we need.
		//
		// To do this, compute the highest fixed-point accumulator we'll reach.
		// Truncate that, and add the filter width.  Then subtract however many
		// samples are sitting at the bottom of the buffer.
		srcSamples = (long)(((__int64)m_samp_frac*(samples-1) + m_accum) >> 19) + filter_width - m_holdover;

		// Don't exceed the buffer (BUFFER_SIZE - holdover).
		if (srcSamples > BUFFER_SIZE - m_holdover)
			srcSamples = BUFFER_SIZE - m_holdover;

		// Read into buffer.
		memcpy((char*)m_cbuffer + m_holdover*m_bytesPerSample, inbuffer, srcSamples*m_bytesPerSample);
		inbuffer = (void *)((char *)inbuffer + m_bytesPerSample * srcSamples);

		if (!srcSamples) break;

		// Figure out how many destination samples we'll get out of what we
		// read.  We'll have (srcSamples+holdover) bytes, so the maximum
		// fixed-pt accumulator we can hit is
		// (srcSamples+holdover-filter_width)<<16 + 0xffff.
		dstSamples = (((srcSamples+m_holdover-filter_width)<<19) + 0x7ffff - m_accum) / m_samp_frac + 1;

		if (dstSamples > samples)
			dstSamples = samples;

		if (dstSamples>=1) {
			m_accum = audio_downsample_mono16(outbuffer, m_cbuffer, filter_bank, filter_width, m_accum, m_samp_frac, dstSamples);
			outbuffer = (void *)((char *)outbuffer + m_bytesPerSample * dstSamples);
			lActualSamples += dstSamples;
			samples -= dstSamples;
		}

		// We're "shifting" the new samples down to the bottom by discarding
		// all the samples in the buffer, so adjust the fixed-pt accum
		// accordingly.
		m_accum -= ((srcSamples+m_holdover)<<19);

		// Oops, did we need some of those?
		//
		// If accum=0, we need (n/2) samples back.  accum>=0x10000 is fewer,
		// accum<0 is more.
		nhold = - (m_accum>>19);
		if (nhold>0) {
			memmove(m_cbuffer, (char *)m_cbuffer+m_bytesPerSample*(srcSamples+m_holdover-nhold), m_bytesPerSample*nhold);
			m_holdover = nhold;
			m_accum += nhold<<19;
		} else
			m_holdover = 0;
	}
	return lActualSamples;
}


/****************************************************************************
 * Upsample method
 ****************************************************************************/
long VS_AudioReSampler::Upsample(void *inbuffer, void *outbuffer, long srcsamples) {
	long samples = (long)((((__int64)srcsamples<<19) - m_accum - 0x40001)/m_samp_frac + 1);
	long lActualSamples=0;

	// Upsampling: producing more output samples than input
	//
	// There are two issues we need to watch here:
	//
	//	o  An input sample can be read more than once.  In particular, even
	//	   when point sampling, we may need the last input sample again.
	//
	//	o  When interpolating (HQ), we need one additional sample.
	while(samples>0) {
		long srcSamples, dstSamples;
		int m_holdover = 0;

		// A negative accum value indicates that we need to reprocess a sample.
		// The last iteration should have left it at the bottom of the buffer
		// for us.  In interpolation mode, we'll always have at least a 1
		// sample overlap.
		if (m_accum<0) {
			m_holdover = 1;
			m_accum += 0x80000;
		}
		++m_holdover;

		// figure out how many source samples we need
		srcSamples = (long)(((__int64)m_samp_frac*(samples-1) + m_accum) >> 19) + 1 - m_holdover;
		++srcSamples;

		if (srcSamples > BUFFER_SIZE-m_holdover)
			srcSamples = BUFFER_SIZE-m_holdover;
		memcpy((char*)m_cbuffer + m_holdover*m_bytesPerSample, inbuffer, srcSamples*m_bytesPerSample);
		inbuffer = (void *)((char *)inbuffer + m_bytesPerSample * srcSamples);
		if (!srcSamples) break;

		srcSamples += m_holdover;

		// figure out how many destination samples we'll get out of what we read
		dstSamples = ((srcSamples<<19) - m_accum - 0x40001)/m_samp_frac + 1;

		if (dstSamples > samples)
			dstSamples = samples;

		if (dstSamples>=1) {
			m_accum = audio_upsample_mono16(outbuffer, m_cbuffer, m_accum, m_samp_frac, dstSamples);
			outbuffer = (void *)((char *)outbuffer + m_bytesPerSample * dstSamples);
			lActualSamples += dstSamples;
			samples -= dstSamples;
		}
		m_accum -= ((srcSamples-1)<<19);

		// do we need to hold a sample over?
		if (m_accum<0)
			memcpy(m_cbuffer, (char *)m_cbuffer + (srcSamples-2)*m_bytesPerSample, m_bytesPerSample*2);
		else
			memcpy(m_cbuffer, (char *)m_cbuffer + (srcSamples-1)*m_bytesPerSample, m_bytesPerSample);
	}
	return lActualSamples;
}

/****************************************************************************
 * Upsample method
 ****************************************************************************/
long VS_AudioReSampler::Process(void *inbuffer, void *outbuffer, long InSize, int Infreq, int OutFreq)
{
	if (!m_IsValid || Infreq!= m_old_rate || OutFreq!= m_new_rate)
		Init(Infreq, OutFreq);
	if (m_old_rate> m_new_rate) {
		return Downsample(inbuffer, outbuffer, InSize/m_bytesPerSample)*m_bytesPerSample;
	}
	else if (m_old_rate< m_new_rate) {
		return Upsample(inbuffer, outbuffer, InSize/m_bytesPerSample)*m_bytesPerSample;
	}
	else {
		memcpy(outbuffer, inbuffer, InSize);
		return InSize;
	}
}

/****************************************************************************
 * Speex Audio Resampler
 ****************************************************************************/

VS_AudioReSamplerSpeex::VS_AudioReSamplerSpeex(int quality)
{
	memset(this, 0, sizeof(VS_AudioReSamplerSpeex));
	m_quality = quality;
}

VS_AudioReSamplerSpeex::~VS_AudioReSamplerSpeex()
{
	Release();
}

bool VS_AudioReSamplerSpeex::Init(long old_rate, long new_rate)
{
	Release();
	int err = 0;
	m_in_freq = old_rate;
	m_out_freq = new_rate;
	m_st = speex_resampler_init(1, old_rate, new_rate, m_quality, &err);
	if (m_st == 0 || err != RESAMPLER_ERR_SUCCESS) {
		return false;
	}
	m_IsValid = true;
	return m_IsValid;
}

void VS_AudioReSamplerSpeex::Release()
{
	if (m_st) speex_resampler_destroy(m_st); m_st = 0;
	m_in_freq = 0;
	m_out_freq = 0;
	m_IsValid = false;
}

long VS_AudioReSamplerSpeex::Process(void *in_buffer, void *out_buffer, long in_size, int in_freq, int out_freq)
{
	unsigned int out_size = 1048576;
	in_size /= 2;
	if (in_buffer == 0 || out_buffer == 0) return 0;
	if (!m_IsValid) {
		if (!Init(in_freq, out_freq)) return 0;
	}
	if (in_freq != m_in_freq || out_freq != m_out_freq) {
		m_in_freq = in_freq;
		m_out_freq = out_freq;
		if (speex_resampler_set_rate(m_st, m_in_freq, m_out_freq) != RESAMPLER_ERR_SUCCESS) return 0;
	}
	int i=0;
	short  *pBuf= (short*) in_buffer;
	for(i=0;i<in_size;i++)
		pBuf[i]=(pBuf[i]*7)>>3;         // bugfix with resampler
	speex_resampler_process_int(m_st, 0, (short*)in_buffer, (unsigned int*)(&in_size), (short*)out_buffer, &out_size);
	return out_size * 2;
}

/****************************************************************************
 * END OF FILE
 ****************************************************************************/
#endif
