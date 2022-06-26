/**
 **************************************************************************
 * \file VSAudioEchoCancel.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Wraper to real Echo Cancelation. Use EchoCancelation class from Audio
 *
 * \b Project Client
 * \author SMirnovK
 * \date 26.03.2003
 *
 * $Revision: 17 $
 *
 * $History: VSAudioEchoCancel.h $
 *
 * *****************  Version 17  *****************
 * User: Sanufriev    Date: 29.06.12   Time: 14:04
 * Updated in $/VSNA/VSClient
 * - webrtc aec + ns
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 21.04.10   Time: 15:54
 * Updated in $/VSNA/VSClient
 * - MAX_NUM_ECHO_CHANNEL = 64
 *
 * *****************  Version 15  *****************
 * User: Sanufriev    Date: 16.04.10   Time: 14:53
 * Updated in $/VSNA/VSClient
 * - were enhancement aec (increase resampling precision, statistic queue
 * lenght, upper bandwith for skip)
 *
 * *****************  Version 14  *****************
 * User: Sanufriev    Date: 6.04.10    Time: 15:21
 * Updated in $/VSNA/VSClient
 * - update aec statistics
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 24.09.09   Time: 14:07
 * Updated in $/VSNA/VSClient
 * - fix aec crash. increase maximum number of aec chanel
 * (MAX_NUM_ECHO_CHANNEL = 12).
 * - new agc
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 20.07.09   Time: 14:44
 * Updated in $/VSNA/VSClient
 * - debuging bug
 * - fix aec for Vista
 * - change directx version detect
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 18.05.09   Time: 15:07
 * Updated in $/VSNA/VSClient
 * - were fixed Echo Module: always first jump after init arender, not
 * change offset when arender is stoped
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 15.05.09   Time: 19:43
 * Updated in $/VSNA/VSClient
 * - were adapted AEC for Direct Sound
 * - were fixed calculation buffer duration in DS
 * - were fixed calculation write position in VS_AudioBarrel
 * - were fixed some AEC bug: reset statistic for capture, render time
 * calculation
 * - were improved AEC: histogram statistic, adaptive range jump, average
 * replaced median for calculation offset error
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 1.04.09    Time: 19:23
 * Updated in $/VSNA/VSClient
 * - bugfix #5815
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 18.03.09   Time: 16:22
 * Updated in $/VSNA/VSClient
 * - were separated preprocessing amd aec ib Global Echo Module
 * (preprocessing is always work)
 * - were added AGC in speex preprocessing
 * - update speex lib (include float-point agc version)
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 5.03.09    Time: 14:59
 * Updated in $/VSNA/VSClient
 * - were added support multi-cnnel aec in VS_GlobalEcho &
 * VS_SpeexEchoCancel
 * - were removed skip-frames for first calculation audio render frequency
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 9.02.09    Time: 9:54
 * Updated in $/VSNA/VSClient
 * - were improved speex aec
 * - were added audio devices frequency calculate
 * - were added speex resample in echo module
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 23.12.08   Time: 13:07
 * Updated in $/VSNA/VSClient
 * - capability of use aec dll library is added
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 5.12.08    Time: 15:13
 * Updated in $/VSNA/VSClient
 * - aec: were changed manual offset for near end audio signal
 * - aec: were changed algorithm offset correction (for far end audio
 * signal)
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 7.04.08    Time: 16:58
 * Updated in $/VSNA/VSClient
 * - change fixed direct port to random one
 * - zoomchat defines refreshed
 * - file size decreased
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 23.12.07   Time: 18:33
 * Updated in $/VS2005/VSClient
 * - more smart ofset calc
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 20.12.07   Time: 16:14
 * Updated in $/VS2005/VSClient
 * - added software AEC
 * - Speex AEC improved
 * - Audio MediaFormat changed to commit Speex preprocess
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 25  *****************
 * User: Smirnov      Date: 15.11.05   Time: 18:26
 * Updated in $/VS/VSClient
 * - new projects configurations
 * - ZoomChat client dll size decreased
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 4.02.05    Time: 17:38
 * Updated in $/VS/VSClient
 * comented echocancel
 * latency analis window = 30 msec
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 3.02.05    Time: 20:22
 * Updated in $/VS/VSClient
 * EcHo cancel repaired
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 ****************************************************************************/
#ifndef VS_AUDIO_ECHO_CANCEL_H
#define VS_AUDIO_ECHO_CANCEL_H

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "../Audio/EchoCancel/SpeexEchoCancel.h"
#include "../std/cpplib/VS_Lock.h"
#include "../Transcoder/VS_AudioReSampler.h"
#include "AudioProcessing/VSEchoDelayDetector.h"
#include "VSAudioUtil.h"
#include <map>

#define MAX_NUM_ECHO_CHANNEL (64)

class CWaveFile;

/****************************************************************************
 * Classes
 ****************************************************************************/
/**
 **************************************************************************
 * \brief Virtual empty class  interface
 ****************************************************************************/
class VS_GlobalEchoVirtual
{
protected:
	bool				m_IsValid;			///< true if valid mode has been set
public:
	VS_GlobalEchoVirtual() {m_IsValid = false;}
	virtual ~VS_GlobalEchoVirtual() {}
	/// init for capture device
	virtual void Init(int CuptFreq, bool isDX10) {}
	/// release capture depended resources
	virtual void Release(){}
	/// add new render channel
	virtual void OpenEchoChannel(void* handle_channel, int RendFreq) {}
	/// remove render channel
	virtual void CloseEchoChannel(void* handle_channel = 0) {}
	/// must be called before adding audio to plaing device
	virtual void AddOutBufer(short* out, int samples, double RendFreq, void* handle_channe){}
	/// called to cancelate captured audio frame
	virtual void CanselInBuffer(short* in, int samples, int CaptTime, double CaptFreq){}
	/// return validation state
	bool IsValid() {return m_IsValid;}
};


class VS_AudioBarrel
{
	short*			m_buff;
	unsigned long	m_samples;
	unsigned long	m_WritePos;
	unsigned long	m_ReadPos;
public:
	VS_AudioBarrel(unsigned long samples = 16000*8);
	~VS_AudioBarrel();
	void Add(short* data, unsigned long samples);
	bool Read(short* data, unsigned long samples, unsigned long OfsetFromEnd);
};
/**
 **************************************************************************
 * Echo Cancelation class
 ****************************************************************************/

#define FILTER_LENGHT	(11)

struct VS_EchoChannelState
{
	int						RendFreq;
	int						Offset;
	VS_AudioBarrel			*out_abuffer;
	int						window_derr[FILTER_LENGHT];
	int						lsize_wnd;
	int						prev_derr;
	int						id_channel;
	bool					is_first;
	VS_AudioReSamplerSpeex	*rs;
	unsigned int			m_samples;
	/// histogram settings
	int						size_hist;
	int						*hist_err;
	int						last_hist_time;
	int						int_bound;
	/// debug wav files
	CWaveFile		*outf;	///< output far signal
	CWaveFile		*outf_no_rsmpl;	///< output far signal (no resampling)
};

typedef std::pair<void*, VS_EchoChannelState>				ec_pair;
typedef std::map <void*, VS_EchoChannelState>::iterator		ec_iter;

class VS_GlobalEcho : public VS_GlobalEchoVirtual, VS_Lock
{
	int						m_manualOfset;
	int						m_num_channel;
	int						m_CuptFreq;
	int						m_RealCuptFreq;
	double					m_RealCuptFreqFloat;
	short*					m_temp1;
	short*					m_temp2;
	short*					m_far_tmp;
	int						m_last_size_far;
	int						m_min_bound;
	eTypeAEC				m_type_aec;

	VS_EchoCancelBase				*m_ec; ///< Echo main object
	std::map<void*, VS_EchoChannelState>	m_ec_state;

	VSEchoDelayDetector m_DelayDetector;

public:
	VS_GlobalEcho();
	~VS_GlobalEcho();
	void Init(int CuptFreq, bool isDX10);
	//void Release(void* handle_channel = 0);
	void Release();
	void OpenEchoChannel(void* handle_channel, int RendFreq);
	void CloseEchoChannel(void* handle_channel = 0);
	void AddOutBufer(short* out, int samples, double RendFreq, void* handle_channe);
	void CanselInBuffer(short* in, int samples, int CaptTime, double CaptFreq);

	void AddPattern(short* out, int samples);
	void SetEchoDelay(int32_t delay);
	void ResetEchoDelay();
	VSEchoDelayDetector::EGetDelayResult GetEchoDelay(int32_t& delay);
	void StartDelayDetectTest();
	void StopDelayDetectTest();

private:
	CWaveFile			*m_inf;				///< debug wav file with input near signal
	CWaveFile			*m_compf;			///< debug wav file with canceled input near signal
	void DebugNew(void* handle_channel = 0);
	void DebugWrite(short *in, short* out, short* out_nr, short *comp, int samples, void* handle_channel = 0);
	void DebugDelete(void* handle_channel = 0);
	void ChangeTypeAEC(int num_channels, int frequency);
};

#endif