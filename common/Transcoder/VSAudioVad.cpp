/**
 **************************************************************************
 * \file VSAudioVad.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief VSAudio Voice Activity Detection (VAD)
 *
 * This is a simple vad implementation.  It isn't tuned and testing
 * has been somewhat limited... but it seems to work ok. This code use
 * three functions not provided as part of this file : log10_32
 * and bqProcess.
 * The log function calculates :  10 * log(x) - 93.33 .
 * A biquad is used by the vad to filter out lower frequency background noise.
 *
 * \b Project Audio
 * \author SMirnovK
 * \date 01.10.02
 *
 * $Revision: 2 $
 *
 * $History: VSAudioVad.cpp $
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 18.08.10   Time: 14:09
 * Updated in $/VSNA/Audio/VoiceActivity
 * - vad corrected
 * - alfa Native freq
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/Audio/VoiceActivity
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/Audio/VoiceActivity
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 22.12.04   Time: 16:58
 * Updated in $/VS/Audio/VoiceActivity
 * vad presielence now 240 ms
 * caupture audio started now in conference only
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 17.12.04   Time: 14:21
 * Updated in $/VS/Audio/VoiceActivity
 * added file header comments
 ****************************************************************************/


/****************************************************************************
 * Includes
 ****************************************************************************/
#include <math.h>

#include "VSAudioVad.h"
#include "NoiseGen.h"

/****************************************************************************
 * Defines
 ****************************************************************************/
/// processing data chunk in mSec
#define VAD_DATA_FRAME_LENGTH    (5)

/// Initial number of mSec before we increment noise Floor
#define VAD_NOISEFLOOR_CNT_INIT      (int)(1200)
/// not used yet
#define VAD_SIGNALMAX_CNT_INIT       (int)(1500)

#define VAD_NOISE_TH_BASE        (float)  8.00		///<  10.00 dB Noise Threshold
#define VAD_NOISE_FLOOR_INIT     (float)-48.00		///< -74.00 dB Initial Noise Floor
#define VAD_SIGNAL_MAX_INIT      (float)-80.00		///< -80.00 dB Initial Noise Max
#define VAD_NOISE_TH_MIN         (float)  1.50		///<   1.00 dB Incrementing Noise Threshold

///  Number of mSec of silence before we declare silence period
#define VAD_HANGOVER_CNT_INIT        (int)(240)


/****************************************************************************
 * Classes
 ****************************************************************************/

VSAudioVAD::VSAudioVAD()
{
	m_Mode = DISABLE_ALL;
	m_pNoiseGen = 0;
}
VSAudioVAD::~VSAudioVAD()
{
	Release();
}


/**
 ******************************************************************************
 * Init initial values in VAD
 * \return none
 *
 * \param freq	        		[IN]  - samples per sec;
 * \param bits_per_sample  		[IN]  - bits per sample;
 *
 *  \date    08-10-2002
 *****************************************************************************/
bool VSAudioVAD::Init(int freq, int bits_per_sample)
{
	if ( (freq < 4000)||(freq > 96000)||
		( (bits_per_sample != 8)&&(bits_per_sample != 16) ) )
		return false;
	m_Frequency        = freq/1000;
	m_Bits             = bits_per_sample;
	m_DataFrameLength  = VAD_DATA_FRAME_LENGTH*8;//*m_Frequency;
	m_Mode	           = ENABLE_NOISEGEN;
	m_Sta              = 10000;
	m_NoiseTH          = VAD_NOISE_TH_BASE;
	m_NoiseTnMin       = VAD_NOISE_TH_MIN;
	m_State            = VadState_Unknown;
	m_NoiseFloorCnt    = VAD_NOISEFLOOR_CNT_INIT*m_Frequency;
	m_NoiseFloorCntInit= VAD_NOISEFLOOR_CNT_INIT*m_Frequency;
	m_NoiseFloor       = VAD_NOISE_FLOOR_INIT;
	m_HangoverCnt      = VAD_HANGOVER_CNT_INIT*m_Frequency;
	m_HangoverCntInit  = VAD_HANGOVER_CNT_INIT*m_Frequency;
	m_STARise          = 1;
	m_StateTxCount     = 0;									// used but unnes
	m_SignalMax        = VAD_SIGNAL_MAX_INIT;				// not used
	m_SignalMaxCnt     = VAD_SIGNALMAX_CNT_INIT*m_Frequency; //not used
	m_MinNoiseLevel    = 256*256;
	m_MaxNoiseLevel    = 0;
	m_pNoiseGen = new NoiseGen;
	if (m_pNoiseGen==nullptr) return false;
	if (!m_pNoiseGen->Init(freq, 3000))							//<! set 1000 mSec noise buffer length
	{
		delete m_pNoiseGen;
		m_pNoiseGen = nullptr;
		return false;
	}
	m_prevBQ = 0;
	return true;
}

/**
 ******************************************************************************
 * Do biquad processing
 * \return none
 *
 * \note
 *
 * \param datain	        		[IN]  - input subframe samples;
 * \param dataout	        		[OUT] - output subframe biquad samples;
 * \param n			        		[IN]  - number of samples;
 *
 *  \date    08-10-2002
 *****************************************************************************/
void VSAudioVAD::BqProcess(short *datain, short *dataout, int n)
{
    int i;
	for (i = 0; i < n; i++) {
		dataout[i] = datain[i] - m_prevBQ;
		m_prevBQ = datain[i];
	}
}

/**
 ******************************************************************************
 * Calculate log from sta data
 * \return none
 *
 * \note
 *
 * \param length		       		[IN]  - length of ????;
 * \param login		        		[IN]  - input sta's;
 * \param logout	        		[OUT] - output log's;
 *
 *  \date    08-10-2002
 *****************************************************************************/
void VSAudioVAD::CalcPower ( int length, unsigned long *login, float *logout)
{
    int i;
	for (i = 0; i < length; i++)
		logout[i] = (float) (10 * log10((float)login[i]) - 10 * 9.332);
}

/**
 ******************************************************************************
 * Compute statistic
 * \return max STA
 *
 * \note
 *
 * \param pdata	        		[IN]  - samples per sec;
 * \param length  				[IN]  - length of data (in samples);
 * \param minSta	        	[OUT] - adrees to store min STA;
 *
 *  \date    08-10-2002
 *****************************************************************************/
unsigned long VSAudioVAD::ComputeSTA(short *pdata, int length, unsigned long *minSta)
{
	int  i;
	long  acc0,acc1, summ = 0;
	unsigned long  maxSta;

	*minSta = m_Sta;
	maxSta = m_Sta;

	for (i = 0; i < length; i++)
	{
		/* q.15 * q.15 = q.30 */
		acc1  = pdata[i] * pdata[i];
		summ+=(acc1>>9);

		if ( m_STARise )
		{
			acc0  = -1 * (long)(m_Sta >> 6);
			acc1  = acc1 >> 5;
		}
		else
		{
			acc0  = -1 * (long)(m_Sta >> 10);  // original 10 and 9
			acc1  = acc1 >> 9;
		}

		acc0 += acc1;
		m_STARise = ( 0 >= acc0 ) ? 0 : 1;
		m_Sta += acc0;

		if ( m_Sta > maxSta )
		{
			maxSta = m_Sta;	// arijit - i added the cast
		}
		else
			if ( m_Sta < *minSta )
			{
				*minSta = m_Sta;
			}

	} /* for */

	summ = (int)sqrt(((float)summ/length)*(1<<9));
	if (m_MinNoiseLevel>summ) m_MinNoiseLevel = (m_MinNoiseLevel+summ)/2;
	if (m_MaxNoiseLevel<summ) m_MaxNoiseLevel = (m_MaxNoiseLevel+summ)/2;

	return maxSta;
}

/**
 ******************************************************************************
 * Compute immediate amlitude o fsignal
 * \return evalutive value of amplitude
 *
 * \note return value is not neessesuary right
 *
 *  \date    08-10-2002
 *****************************************************************************/
int VSAudioVAD::GetVadLevel()
{
	return (m_MinNoiseLevel + 128 +(m_MaxNoiseLevel>>4))/2;
}

/**
 ****************************************************************************
 * Compute NFE
 * \return none
 *
 * \note
 *
 * \param minpower	        	[IN]  - relative Noise thereshold of cuurent subframe;
 * \param maxpower  			[IN]  - relative Signal thereshold of cuurent subframe;
 * \param length	        	[IN]  - length of subframe (in samples);
 *
 *  \date    08-10-2002
 *****************************************************************************/
void VSAudioVAD::ComputeNFE(float minpower, float maxpower, int length)
{
	if ( minpower <= m_NoiseFloor )
	{
		m_NoiseFloor = minpower;
		m_NoiseFloorCnt = m_NoiseFloorCntInit;
	}
	else
	{
		if ( m_NoiseFloorCnt < length )
		{
			m_NoiseFloor += m_NoiseTnMin;
			m_NoiseFloorCnt = (m_NoiseFloorCntInit + m_NoiseFloorCnt - length);
		}
		else
		{
			m_NoiseFloorCnt -= length;
		}
	}
}


/**
 ******************************************************************************
 * Process subframe
 * \return true if speech
 *
 * \note
 *
 * \param data		        	[IN]  - input samples of subframe;
 * \param length	        	[IN]  - length of subframe (in samples);
 *
 *  \date    08-10-2002
 *****************************************************************************/
bool VSAudioVAD::VadSubProcess(short *data, int length)
{
	bool  SpeechDetected;
	bool  FrameSpeechFlag;
	short   tmpData[VAD_DATA_FRAME_LENGTH*96]; // max 96 kHz
	unsigned long   sta[2];
	float    power[2];

	SpeechDetected  = true;
	FrameSpeechFlag = false;

	BqProcess(data, tmpData, length);
	sta[1] = ComputeSTA(tmpData, length, &sta[0]);
	CalcPower(2, sta, power);
	ComputeNFE(power[0], power[1], length);

	if (power[1] > (m_NoiseFloor + m_NoiseTH))
	{
		FrameSpeechFlag = true;
	}

	if ( FrameSpeechFlag == false)
	{
		if ( m_HangoverCnt < length )
		{
			SpeechDetected  = false;
			m_HangoverCnt = 0;
			if ( m_State != VadState_Silence )
			{
				m_StateTxCount++;
			}
			m_State = VadState_Silence;
		}
		else
		{
			m_HangoverCnt -= length;
		}
	}
	else
	{
		m_HangoverCnt = m_HangoverCntInit;
		if ( m_State == VadState_Silence )
		{
			m_StateTxCount++;
		}
		m_State = VadState_Speech;
	}
	return SpeechDetected;
}

/**
 ******************************************************************************
 * Process frame
 * \return true if speech
 *
 * \note
 *
 * \param data		        	[IN]  - input samples of frame;
 * \param length	        	[IN]  - length of frame (in bytes);
 *
 *  \date    08-10-2002
 *****************************************************************************/
bool VSAudioVAD::IsVad(void *data, int length)
{
	int idx;
	int step;
	bool ret;

	ret = false;

	if ( m_Mode!=DISABLE_ALL )
	{
		if (m_Bits == 16)
		{
			short *Data = (short*)data;
			length/=2;
			int totalLen = length;
			//<! Cut up the frame into 5ms chunks for processing purposes
			for (idx = 0; length > 0; length -= step)
			{
				step = (length < m_DataFrameLength) ? length : m_DataFrameLength;
				ret |= VadSubProcess(&Data[idx], step);
				idx += step;
			}
			//<! call for noise generation
			if (m_Mode==ENABLE_NOISEGEN)
			{
				if (!ret) m_pNoiseGen->FillBuffer(Data, totalLen);
			}
		}
		else
		{
			unsigned char *Data = (unsigned char *)data;
			short *temp = new short[length];
			Convert8BitTo16Bit(temp, Data, length);
			int totalLen = length;
			//<! Cut up the frame into 5ms chunks for processing purposes
			for (idx = 0; length > 0; length -= step)
			{
				step = (length < m_DataFrameLength) ? length : m_DataFrameLength;
				ret |= VadSubProcess(&temp[idx], step);
				idx += step;
			}
			//<! call for noise generation
			if (m_Mode==ENABLE_NOISEGEN)
			{
				if (!ret) m_pNoiseGen->FillBuffer(temp, totalLen);
			}
			delete[] temp;
		}
		m_pNoiseGen->VadNoiseLevel = GetVadLevel();
	}
	else
	{
		ret = true;
	}
	return ret;
}

/**
 ******************************************************************************
 * Convert 8 bit audio to 16 bit audio
 * \return none
 *
 * \note convert mono audio
 *
 * \param dest	        		[OUT] - destination buffer;
 * \param src	        		[IN]  - source buffer;
 * \param count	        		[IN]  - number of samples to converting;
 *
 *  \date    07-10-2002
 ******************************************************************************
 */
void VSAudioVAD::Convert8BitTo16Bit(void *dest, void *src, long count)
{
	unsigned char *s = (unsigned char *)src;
	signed short  *d = (signed short  *)dest;
	int i;

	for (i = 0; i<count; i++)
		d[i] = (signed short)((unsigned long)(s[i]-0x80)<<8);
}

/**
 ******************************************************************************
 * Convert 16 bit audio to 8 bit audio
 * \return none
 *
 * \note convert mono audio
 *
 * \param dest	        		[OUT] - destination buffer;
 * \param src	        		[IN]  - source buffer;
 * \param count	        		[IN]  - number of samples to converting;
 *
 *  \date    07-10-2002
 *****************************************************************************/
void VSAudioVAD::Convert16BitTo8Bit(void *dest, void *src, long count)
{
	signed short  *s = (signed short  *)src;
	unsigned char *d = (unsigned char *)dest;
	int i;

	for (i = 0; i<count; i++)
		d[i] = (unsigned char)((((unsigned long)s[i])+0x8080)>>8);
}

/***************************** END OF FILE ************************************/
