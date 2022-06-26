/**
 **************************************************************************
 * \file VSAudioUtil.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Time deviation, Audio Buffer, FIFO buffer classes
 *
 * \b Project Client
 * \author SMirnovK
 * \date 19.11.2004
 *
 * $Revision: 15 $
 *
 * $History: VSAudioUtil.h $
 *
 * *****************  Version 15  *****************
 * User: Sanufriev    Date: 2.07.12    Time: 10:39
 * Updated in $/VSNA/VSClient
 * - fix audio device bug : fifo buffer
 *
 * *****************  Version 14  *****************
 * User: Sanufriev    Date: 28.09.11   Time: 14:44
 * Updated in $/VSNA/vsclient
 * - beta nhp revision
 * - fix fps on low bitrates
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 18.10.10   Time: 19:47
 * Updated in $/VSNA/VSClient
 * - agc is gooood
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 17.09.10   Time: 21:26
 * Updated in $/VSNA/VSClient
 * - agc corrected by Disp
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 18.08.10   Time: 14:09
 * Updated in $/VSNA/VSClient
 * - vad corrected
 * - alfa Native freq
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 28.04.10   Time: 14:36
 * Updated in $/VSNA/VSClient
 * - preamp in agc
 * - use variance in agc
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 5.04.10    Time: 18:56
 * Updated in $/VSNA/VSClient
 * - overload behavior changed
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 22.03.10   Time: 15:07
 * Updated in $/VSNA/VSClient
 * - agc with overloads counter
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 19.02.10   Time: 20:39
 * Updated in $/VSNA/VSClient
 * - new AGC
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 24.09.09   Time: 14:07
 * Updated in $/VSNA/VSClient
 * - fix aec crash. increase maximum number of aec chanel
 * (MAX_NUM_ECHO_CHANNEL = 12).
 * - new agc
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 9.02.09    Time: 9:54
 * Updated in $/VSNA/VSClient
 * - were improved speex aec
 * - were added audio devices frequency calculate
 * - were added speex resample in echo module
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 25.09.08   Time: 15:27
 * Updated in $/VSNA/VSClient
 * - were fixed GetBufferedDurr() (decrease probability of threads
 * conflict on multi-core cpu)
 * - were changed jitter algorithm
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 9.07.08    Time: 13:08
 * Updated in $/VSNA/VSClient
 * - were modified audio render algorithm
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 15.11.07   Time: 21:10
 * Updated in $/VS2005/VSClient
 * - new AG Control
 * - fixed bug with audio capture devices having no control
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 5.04.06    Time: 17:24
 * Updated in $/VS/VSClient
 * - low-level audio devices
 * - Direct Sound devices added
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 9.03.06    Time: 17:38
 * Updated in $/VS/VSClient
 * - AGC implemented
 * - "long latency" audiorender mode
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 10.08.05   Time: 13:33
 * Updated in $/VS/VSClient
 * - new low latensy and noise generation schema
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 10.03.05   Time: 12:48
 * Updated in $/VS/VSClient
 * new sinc for audio-video
 *
 * *****************  Version 4  *****************
 * User: Admin        Date: 16.12.04   Time: 20:08
 * Updated in $/VS/VSClient
 * doxigen comments
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 9.12.04    Time: 19:48
 * Updated in $/VS/VSClient
 * last good sinc
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 8.12.04    Time: 20:26
 * Updated in $/VS/VSClient
 * new video-audio sinc
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 24.11.04   Time: 20:13
 * Created in $/VS/VSClient
 * added new audio files and classes
 *
 ****************************************************************************/

#ifndef VS_AUDIO_UTIL_H
#define VS_AUDIO_UTIL_H

/****************************************************************************
 * Includes
 ****************************************************************************/
//#include <windows.h>
//#include <mmreg.h>
#include "std-generic/cpplib/VS_Container.h"
#include <deque>

class AudioCodec;
class VSAudioVAD;

/****************************************************************************
 * Classes
 ****************************************************************************/

class VS_ARenderAnalyse
{
	unsigned int	*m_values, *m_hvalues;
	unsigned int	*m_pend_min, *m_skip, *m_empty_buff;
	unsigned int	m_MaxCount, m_hMaxCount, m_chunk_size, m_MaxAvgCount;
	unsigned int	m_CurCount;
	int				m_Deviation_A;
	int				m_Deviation_B;
	int				m_MaxBuffDur, m_MinBuffDur;
	bool			m_IsValid;
	bool			m_NewSample;
	bool			Calculate();
public:
	VS_ARenderAnalyse();
	~VS_ARenderAnalyse();
	void	Snap(int val, int pend_min);
	void	SnapSkip();
	void	SnapEmptyBuffer();
	bool	GetBuffDur(int &BuffDurA, int &BuffDurB, int &MaxBuffDur, int &MinBuffDur,
					   int &GMinBuffDur, int &NumSkip, int &NumEmptyBuff);
	void	Init(int MaxCount, int MaxAvgCount);
	void	Clear();
};

class VS_FreqDeviation
{
	double	*m_values_x,
			*m_values_y;		///< pointer to analized values
	int		m_MaxCount;			///< max count of analized values
	int		m_CurrCount;		///< current number of analized values
	bool	m_IsValid;			///< true if Init() called
	bool	m_NewSample;		///< true if coef became old

	/// Predict 1-st order polinom for data by LSE method
	bool Predict(double &a0, double &a1);
public:
	/// Constructor
	VS_FreqDeviation();
	/// Destructor
	~VS_FreqDeviation();
	/// Add (time) value to analised buffer
	void Snap(double valx, double valy);
	/// Get average time value
	double GetA1();
	void GetA(double &a0, double &a1);
	/// Initialization
	void Init(int MaxCount);
	/// Reset counter for new accumulation of data
	void Clear();
	//
	void DumpData(double dt);
};

class VS_AutoGain
{
#define AG_WIN_LEN 128						// must be N^2
private:
	unsigned long		m_freq;				// freq
	unsigned long		m_LastTime;			// counter for voice frames
	unsigned long		m_NoLog;			// counter for consequnse sielent frames
	unsigned long		m_MixerCount;		// couner for change mixer volume
	unsigned long		m_wasOverload;		// couner for change mixer volume
	long				m_Counter;			// cyclic counter for buffer
	long				m_preamp;
	float				m_oldGain;
	float				m_maxDisp;
	float				m_minFloor;
	float				m_log[AG_WIN_LEN];
	int					m_gist[100];
	VSAudioVAD			*m_vad;
	VS_BinBuff			m_buff;
	short				m_lastx;
	short				m_lasty;
public:
	VS_AutoGain();
	~VS_AutoGain();
	bool SetMode(unsigned long freq);
	void BqProcess(short *datain, int n);
	long AnaliseGain(short *in, int len);
	void AdjustVolume(short *in, int len);
	void Preamp(short *in, int len);
};

/// Calculate audio level without normalisation

int CalcLevelSimple(short* in, int samples);

#endif
