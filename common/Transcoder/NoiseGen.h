/**
 **************************************************************************
 * \file NoiseGen.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief noise signal generation class
 *
 * \b Project Audio
 * \author SMirnovK
 * \date 07.10.02
 *
 * $Revision: 1 $
 *
 * $History: NoiseGen.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:48
 * Created in $/VSNA/Audio/VoiceActivity
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/Audio/VoiceActivity
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 17.12.04   Time: 14:21
 * Updated in $/VS/Audio/VoiceActivity
 * added file header comments
 ****************************************************************************/
#ifndef NOISE_GEN_H
#define NOISE_GEN_H


/****************************************************************************
 * Defines
 ****************************************************************************/
#define START_NOISE_LEVEL 127


/****************************************************************************
 * Classes
 ****************************************************************************/
/**
 **************************************************************************
 * \brief generate noise signal from "sielence" recieved from VAD
 ****************************************************************************/
class NoiseGen
{
public:
		 NoiseGen	();
		 ~NoiseGen	();
	bool Init		(int freq, int mSec);
	void FillBuffer	(short * Sample, int Len);
	void GetNoise	(short * Noise, int Len);
	int  VadNoiseLevel;
private:
	int  GetSummSq	(short * Sample, int Len);

	short *m_psBuffer;
	int	  m_BufferPos;
	int   m_BufferSize;

	short *m_psPinkNoise;
	int   m_NoiseBufferSize;

	short *m_psBufferPonter;
	int m_NoiseLevel;
	bool  m_bBufReady;
};

#endif /*NOISE_GEN_H*/

/***************************** END OF FILE ************************************/
