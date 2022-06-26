/**
 **************************************************************************
 * \file NoiseGen.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief noise signal generation class
 *
 * \b Project Audio
 * \author SMirnovK
 * \date 07.10.02
 *
 * $Revision: 1 $
 *
 * $History: NoiseGen.cpp $
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
 * User: Smirnov      Date: 4.04.06    Time: 14:07
 * Updated in $/VS/Audio/VoiceActivity
 * - noise more intensive
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 17.12.04   Time: 14:21
 * Updated in $/VS/Audio/VoiceActivity
 * added file header comments
 ****************************************************************************/


/****************************************************************************
 * Includes
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cinttypes>
#include <math.h>
#include "NoiseGen.h"


/**
 ******************************************************************************
 * NoiseGen constructor
 * \return none
 *
 *  \date    07-10-2002
 *****************************************************************************/
NoiseGen::NoiseGen()
{
	m_BufferPos  = 0;
	m_BufferSize = 0;
	m_NoiseLevel = 0;
	VadNoiseLevel = START_NOISE_LEVEL;
	m_bBufReady = false;
	m_NoiseBufferSize = 10000;
	m_psPinkNoise = new short[m_NoiseBufferSize];
	m_psBuffer = nullptr;
	m_psBufferPonter = nullptr;
}

/**
 ******************************************************************************
 * NoiseGen destructor
 * \return none
 *
 *  \date    07-10-2002
 *****************************************************************************/
NoiseGen::~NoiseGen()
{
	if (m_psBuffer)	delete[] m_psBuffer;
	if (m_psPinkNoise) delete[] m_psPinkNoise;
}

/**
 ******************************************************************************
 * NoiseGen init Buffer
 * \return none
 *
 * \param freq	        		[IN]  - number of saples per seq;
 * \param mSec	        		[IN]  - prefeared length of Noise Buffer;
 *
 *  \date    07-10-2002
 *****************************************************************************/
bool NoiseGen::Init(int freq, int mSec)
{
	if (mSec<500) mSec=500;
	m_BufferSize = freq/1000*mSec;

	for (int i = 0; i<m_NoiseBufferSize; i++)
		m_psPinkNoise[i] = rand()&0x7ff; // normalized to 2^11

	if (m_psBuffer)
		delete[] m_psBuffer;
	m_psBuffer = nullptr;

	if (m_BufferSize>0)
		m_psBuffer = new short[m_BufferSize];
	if (m_psBuffer==nullptr) return false;

	std::memset(m_psBuffer, 0, m_BufferSize*2);
	m_bBufReady = false;
	m_psBufferPonter = m_psBuffer;
	m_BufferPos = 0;
	return true;
}


/**
 ******************************************************************************
 * Initialise Noise Buffer to far using
 * \return none
 *
 * \note must be used after VAD detected sielence
 *
 * \param Sample        		[IN] - pointer to detected sielence;
 * \param Len	        		[IN] - number of input samples;
 *
 *  \date    07-10-2002
 *****************************************************************************/
void NoiseGen::FillBuffer(short *Sample, int Len)
{
	int i;

	if (m_BufferPos >=m_BufferSize) m_BufferPos = 0;
	m_NoiseLevel = (m_NoiseLevel+GetSummSq(Sample, Len))/2;				// Summ(x^2)/N

	if (!m_bBufReady)									// first fill
	{
		short *pNoise = m_psPinkNoise;
		for (i = 0; i<m_BufferSize; i++)
		{
			m_psBuffer[i] = (*(pNoise++)*m_NoiseLevel)>>12;
			if ((pNoise-m_psPinkNoise) >= m_NoiseBufferSize) pNoise = m_psPinkNoise;
		}
		m_bBufReady = true;
	}

	if ( (m_BufferPos+Len)> m_BufferSize) Len = m_BufferSize-m_BufferPos; // >=

	short *pNoise = m_psPinkNoise;
	short *pBuf = m_psBuffer+m_BufferPos;
	for (i = 0; i<Len; i++)
	{
		pBuf[i] = (pBuf[i]*5 + Sample[i]*11 + ((*pNoise*m_NoiseLevel)>>13))>>4;	// 5*buf+11*sample+0.5*pinkNoise
		pNoise++;
		if ((pNoise-m_psPinkNoise) >= m_NoiseBufferSize) pNoise = m_psPinkNoise;
	}
	if (m_BufferPos==0) {
		pBuf[0]/=2;
	}
	else {
		int d = (pBuf[-1]-pBuf[0])/3;
		pBuf[-1]-=d;
		pBuf[0]+=d;
	}
	m_BufferPos+=i;
}


/**
 ******************************************************************************
 * Get mean sq. error
 * \return mean sq. error
 *
 * \note
 *
 * \param Sample        		[IN] - input samples;
 * \param Len	        		[IN] - number of input samples;
 *
 *  \date    07-10-2002
 *****************************************************************************/
int  NoiseGen::GetSummSq(short * Sample, int Len)
{
	int i;
	int64_t sqe = 0;

	for (i = 0; i<Len; i++)
	{
		sqe += (Sample[i]*Sample[i]);
	}
	return (int)sqrt((float)(sqe)/Len);
}


/**
 ******************************************************************************
 * Get Noise
 * \return none
 *
 * \param Noise        			[OUT] - output samples of Noise;
 * \param Len	        		[IN]  - number of samples to prepare;
 *
 *  \date    07-10-2002
 *****************************************************************************/
void NoiseGen::GetNoise(short * Noise, int Len)
{
	const int offset = 2000;
	if (m_bBufReady)				//<! we have complete Noise Buffer
	{
		if (Len <= (m_BufferSize-offset)) // min buffer size = 0.5 sec = min 4000 samples
		{
			if ( (m_psBufferPonter-m_psBuffer)>=(m_BufferSize-Len) )
				m_psBufferPonter = m_psBuffer + rand()%offset;

			memcpy(Noise, m_psBufferPonter, Len*2);
			m_psBufferPonter+=Len;
		}
		else
		{
			int size = m_BufferSize-offset;
			m_psBufferPonter = m_psBuffer + rand()%offset;
			memcpy(Noise, m_psBufferPonter, size*2);
			Noise = Noise + size;
			GetNoise(Noise, Len-size);
		}
	}
	else
	{
		short *pNoise = m_psPinkNoise+rand()%m_NoiseBufferSize;
		for (int i = 0; i<Len; i++)
		{
			Noise[i] = (*(pNoise++)*VadNoiseLevel)>>12;
			if ((pNoise-m_psPinkNoise) >= m_NoiseBufferSize) pNoise = m_psPinkNoise;
		}
	}
	Noise[0] /=2;
	Noise[Len-1] /=2;
}

/***************************** END OF FILE ************************************/
