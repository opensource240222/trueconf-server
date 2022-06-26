/**
 **************************************************************************
 * \file SpeexEchoCancel.h
 * (c) 2002-2007 Visicron Inc.  http://www.visicron.net/
 * \brief Speex Echo Cancellation processing
 *
 * \b Project Audio
 * \date 30.08.2007
 *
 ****************************************************************************/

#ifndef ECHO_CANCEL_SPEEX_H
#define ECHO_CANCEL_SPEEX_H

#include "modules/audio_processing/aec/echo_cancellation.h"
#include "modules/audio_processing/aecm/aecm_core.h"

/****************************************************************************
 * Classes
 ****************************************************************************/

/**
 **************************************************************************
 * \brief Speex echo cancellation main class
 ****************************************************************************/
#ifdef _DEBUG
/// uncomment this define to get aec statistic
//#define TEST_FUNC_RESPONSE
#endif

#ifdef TEST_FUNC_RESPONSE
#include <stdio.h>
#endif

enum eTypeAEC
{
	AEC_SPEEX = 0,
	AEC_WEBRTC = 1,
	AEC_WEBRTCFAST,
};

// echo cancellation base class
class VS_EchoCancelBase
{
public:
	// destructor
	virtual ~VS_EchoCancelBase() {};
	// init echo canselltion module with specified parametrs.
	// If the function succeeds, the return value is zero
	virtual void Init(int frequency) = 0;
	// ec for multi-channel audio
	virtual void Init(int frequency, int num_mic, int num_spk) = 0;
	// release echo cansellation module,
	virtual void Release() = 0;
	// echo cancellation
	// far_end	- far end signal (from other client)
	// near_end - near end signal (from capture device)
	// out		- calcellated near_end signal
	// samples	- number of samples
	// If the function succeeds, the return value is zero
	virtual void Cancellate(short* far_end, short* near_end, short* out, int samples) = 0;
	// get type aec
	virtual eTypeAEC GetType() = 0;
};

VS_EchoCancelBase* VS_RetriveEchoCancel(int Id);

struct SpeexEchoState_;
struct SpeexPreprocessState_;

class VS_SpeexEchoCancel : public VS_EchoCancelBase
{
	int						m_frame_size;
	SpeexEchoState_*		m_st;
    SpeexPreprocessState_*	m_den;
	int						m_num_spk;
	short					*m_far_tmp;
	eTypeAEC				m_type;

#ifdef TEST_FUNC_RESPONSE
	int						m_is_dump;
	unsigned int			m_num_inc, m_num_incf;
	unsigned int			m_dump_time;
	int						m_freq;
	int*					m_filter_response;
	void					DumpResponseFilter();
#endif

public:
	VS_SpeexEchoCancel();
	~VS_SpeexEchoCancel();
	void Init(int frequency);
	void Init(int frequency, int num_mic, int num_spk);
	void Release();
	void Cancellate(short* far_end, short* near_end, short* echo, int samples);
	eTypeAEC GetType() {return m_type; }
};

class VS_BandSplitterCombiner;

class VS_WebRTCEchoCancel : public VS_EchoCancelBase
{
	int				m_frame_size;
	int				m_freq;
	webrtc::Aec		*m_st;
	float			*m_pFarEnd_float;
	short			*m_lowBandNear;
	short			*m_highBandNear;
	short			*m_lowBandFar;
	short			*m_highBandFar;

	eTypeAEC		m_type;
	VS_BandSplitterCombiner* m_splitterCombinerFar;
	VS_BandSplitterCombiner* m_splitterCombinerNear;

	void ReleaseAEC();

public:

	VS_WebRTCEchoCancel();
	~VS_WebRTCEchoCancel();
	void Init(int frequency);
	void Init(int frequency, int num_mic, int num_spk);
	void Release();
	void Cancellate(short* far_end, short* near_end, short* echo, int samples);
	eTypeAEC GetType() {return m_type; }
};



class VS_WebRTCFastEchoCancel : public VS_EchoCancelBase
{
	int				m_frame_size;
	AecmCore		*m_st;
	eTypeAEC		m_type;

	void ReleaseAEC();

public:

	VS_WebRTCFastEchoCancel();
	~VS_WebRTCFastEchoCancel();
	void Init(int frequency);
	void Init(int frequency, int num_mic, int num_spk);
	void Release();
	void Cancellate(short* far_end, short* near_end, short* echo, int samples);
	eTypeAEC GetType() {return m_type; }
};

// callback class for audio device managment
class VS_DeviceManagmentBase
{
	// destructor
	virtual ~VS_DeviceManagmentBase();
	// set level of input device. value e [0..65535]
	virtual int SetInLevel(unsigned long value);
	// any other method
	// .....
};

#endif