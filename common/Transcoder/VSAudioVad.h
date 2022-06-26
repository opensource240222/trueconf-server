/**
 **************************************************************************
 * \file VSAudioVad.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief VSAudio Voice Activity Detection (VAD)
 *
 * \b Project Audio
 * \author SMirnovK
 * \date 01.10.02
 *
 * $Revision: 2 $
 *
 * $History: VSAudioVad.h $
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
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 17.12.04   Time: 14:21
 * Updated in $/VS/Audio/VoiceActivity
 * added file header comments
 *
 *			01.10.02 Source code poken
 *			08.10.02 Rewrited for difefrent frequencies
 *			12.11.02 Interface canged, moved to C++
 *			05.02.03 added CwaveFile for debug purposes
 *			15.04.04 removed CwaveFile
 *
 ****************************************************************************/
#ifndef VS_AUDIO_VAD_H
#define VS_AUDIO_VAD_H


/****************************************************************************
 * Includes
 ****************************************************************************/
#include "NoiseGen.h"

/// Internal VAD State
typedef enum _VSAudioVad_eState
{
  VadState_Silence = 0,
  VadState_Speech,
  VadState_Unknown
} VSAudioVad_eState;

/// External VAD working mode
typedef enum _VSAudioVad_eMode
{
  DISABLE_ALL = 0,
  ENABLE_VAD,
  ENABLE_NOISEGEN
} VSAudioVad_eMode;

/**
 ******************************************************************************
 * \brief Simple Implementation of Voice Activity Detection
 *****************************************************************************/
class VSAudioVAD
{
public:
	VSAudioVAD();
	~VSAudioVAD();

	bool Init				(int freq, int bits_per_sample);
	void Release			(){if (m_pNoiseGen) delete m_pNoiseGen; m_pNoiseGen = nullptr;};
	void SetMode			(VSAudioVad_eMode mode){m_Mode = mode;};
	VSAudioVad_eMode GetMode(){return m_Mode;};
	bool IsVad				(void *data, int length);
	void GetNoise			(short * Noise, int Len){return m_pNoiseGen->GetNoise(Noise, Len/2);};
	inline int  GetVadLevel	();

	int					m_HangoverCnt;		//!< Countdown of consecutive samples before declare silence

private:
	inline void BqProcess			(short *datain, short *dataout, int n);
	inline void CalcPower			(int length, unsigned long *login, float *logout);
	unsigned long ComputeSTA		(short *pdata, int length, unsigned long *minSta);
	void ComputeNFE					(float minpower, float maxpower, int length);
	bool VadSubProcess				(short *data, int length);
	inline void Convert8BitTo16Bit	(void *dest, void *src, long count);
	inline void Convert16BitTo8Bit	(void *dest, void *src, long count);

	NoiseGen*			m_pNoiseGen;		//<! Noise Generatian class pointer;

	///
	VSAudioVad_eMode	m_Mode;				//!< Working mode: 0 - no vad; 1 - vad; 2 - vad+NoiseGen;
	VSAudioVad_eState	m_State;			//!< state == 1 if VOICE	state == 0 if SILENCE
  	unsigned long		m_Sta;				//!< Saved STA between input frames
	float				m_NoiseTH;			//!< Threshold above which a signal is considered to be speech
	int					m_NoiseFloorCnt;	//!< Countdown after which the noise floor is incremented
	float				m_NoiseFloor;		//!< Noise floor in dB
	int					m_SignalMaxCnt;		//!< Countdown after which the signal max is  decremented
	float				m_SignalMax;		//!< Signal max in dB
	int					m_STARise;			//!< STARise == 1 if sta is rising STARise == 0 else
	int					m_StateTxCount;

	int					m_Frequency;		//!< input sgnal frequency
	int					m_Bits;				//!< bit per sample
	int					m_NoiseFloorCntInit;//!< Init value depend on freq
	int					m_HangoverCntInit;	//!< Init value depend on freq
	int					m_DataFrameLength;	//!< data Frame Length depend on freq
	float				m_NoiseTnMin;		//!< value for Increment noise floor
	int					m_MinNoiseLevel;	//!< Value to prymary level
	int					m_MaxNoiseLevel;	//!< Value to prymary level
	short				m_prevBQ;			// store last bq processed value
};

#endif /*VS_AUDIO_VAD_H*/

/***************************** END OF FILE ************************************/
