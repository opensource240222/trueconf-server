/**
 **************************************************************************
 * \file VSAudioNew.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Contain classes working with audio: capturing, rendering, compressing
 *
 * \b Project Client
 * \author SMirnovK
 * \date 19.11.2004
 *
 * $Revision: 47 $
 *
 * $History: VSAudioNew.cpp $
 *
 * *****************  Version 47  *****************
 * User: Sanufriev    Date: 23.07.12   Time: 19:29
 * Updated in $/VSNA/VSClient
 * - ench audio render (buffe durration min bound)
 *
 * *****************  Version 46  *****************
 * User: Sanufriev    Date: 1.11.11    Time: 16:20
 * Updated in $/VSNA/VSClient
 * - new DS render implementation (for many streams - muxer + 1 DSOut
 * device + 1 aec)
 * - change method of detect max sample rate value (from ds guid)
 * - change hardware test,  TestAudio
 * - some refactoring audio + delete DMO audio
 *
 * *****************  Version 45  *****************
 * User: Sanufriev    Date: 29.04.11   Time: 17:09
 * Updated in $/VSNA/VSClient
 * - fix AviWriter : stream sunch
 *
 * *****************  Version 44  *****************
 * User: Smirnov      Date: 1.12.10    Time: 20:47
 * Updated in $/VSNA/VSClient
 * - possible fix for NULL variant error
 *
 * *****************  Version 43  *****************
 * User: Sanufriev    Date: 27.09.10   Time: 16:41
 * Updated in $/VSNA/VSClient
 * - bugfix 7855 (module PlayAvi)
 *
 * *****************  Version 42  *****************
 * User: Smirnov      Date: 24.09.10   Time: 22:11
 * Updated in $/VSNA/VSClient
 * - removed HiLoad++
 *
 * *****************  Version 41  *****************
 * User: Sanufriev    Date: 15.09.10   Time: 16:23
 * Updated in $/VSNA/VSClient
 * - fix DS bug (overflow)
 *
 * *****************  Version 40  *****************
 * User: Smirnov      Date: 18.08.10   Time: 14:09
 * Updated in $/VSNA/VSClient
 * - vad corrected
 * - alfa Native freq
 *
 * *****************  Version 39  *****************
 * User: Smirnov      Date: 12.08.10   Time: 20:08
 * Updated in $/VSNA/VSClient
 * - move interfaces into DS
 *
 * *****************  Version 38  *****************
 * User: Sanufriev    Date: 29.06.10   Time: 18:15
 * Updated in $/VSNA/VSClient
 * - ench 7471 (prepare avi writer)
 *
 * *****************  Version 37  *****************
 * User: Smirnov      Date: 28.04.10   Time: 14:35
 * Updated in $/VSNA/VSClient
 * - preamp in agc
 * - use variance in agc
 *
 * *****************  Version 36  *****************
 * User: Sanufriev    Date: 21.04.10   Time: 15:44
 * Updated in $/VSNA/VSClient
 * - fix aec
 *
 * *****************  Version 35  *****************
 * User: Sanufriev    Date: 16.04.10   Time: 14:53
 * Updated in $/VSNA/VSClient
 * - were enhancement aec (increase resampling precision, statistic queue
 * lenght, upper bandwith for skip)
 *
 * *****************  Version 34  *****************
 * User: Sanufriev    Date: 24.03.10   Time: 20:33
 * Updated in $/VSNA/VSClient
 * - change regulate audio quality
 *
 * *****************  Version 33  *****************
 * User: Smirnov      Date: 22.03.10   Time: 15:07
 * Updated in $/VSNA/VSClient
 * - agc with overloads counter
 *
 * *****************  Version 32  *****************
 * User: Sanufriev    Date: 10.03.10   Time: 21:09
 * Updated in $/VSNA/VSClient
 * - fix bug 7014 (state aec on start)
 *
 * *****************  Version 31  *****************
 * User: Sanufriev    Date: 24.02.10   Time: 13:24
 * Updated in $/VSNA/VSClient
 * - were added DMO AEC (temporary disabled)
 *
 * *****************  Version 30  *****************
 * User: Smirnov      Date: 19.02.10   Time: 20:39
 * Updated in $/VSNA/VSClient
 * - new AGC
 *
 * *****************  Version 29  *****************
 * User: Sanufriev    Date: 21.12.09   Time: 13:44
 * Updated in $/VSNA/VSClient
 * - unicode capability ReadParam
 *
 * *****************  Version 28  *****************
 * User: Sanufriev    Date: 10.12.09   Time: 19:04
 * Updated in $/VSNA/VSClient
 * - unicode capability for hardware lists
 *
 * *****************  Version 27  *****************
 * User: Smirnov      Date: 27.10.09   Time: 16:56
 * Updated in $/VSNA/VSClient
 * - aec, bugfix #6565
 *
 * *****************  Version 26  *****************
 * User: Sanufriev    Date: 24.09.09   Time: 14:07
 * Updated in $/VSNA/VSClient
 * - fix aec crash. increase maximum number of aec chanel
 * (MAX_NUM_ECHO_CHANNEL = 12).
 * - new agc
 *
 * *****************  Version 25  *****************
 * User: Sanufriev    Date: 4.08.09    Time: 20:28
 * Updated in $/VSNA/VSClient
 * - fix DS for avi player
 * - add AudioCodecSystem class, change acmDriverEnumCallback function
 * - avi player support system audio codecs
 *
 * *****************  Version 24  *****************
 * User: Sanufriev    Date: 20.07.09   Time: 14:44
 * Updated in $/VSNA/VSClient
 * - debuging bug
 * - fix aec for Vista
 * - change directx version detect
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 25.06.09   Time: 13:46
 * Updated in $/VSNA/VSClient
 * - audio buffer enh
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 17.06.09   Time: 18:57
 * Updated in $/VSNA/VSClient
 * - echocancel small impr
 *
 * *****************  Version 21  *****************
 * User: Sanufriev    Date: 16.06.09   Time: 17:18
 * Updated in $/VSNA/VSClient
 * - fix aec (change position restart render, reinit aec stat after jump)
 * - update h.264 libs (link icc libs)
 *
 * *****************  Version 20  *****************
 * User: Sanufriev    Date: 11.06.09   Time: 19:31
 * Updated in $/VSNA/VSClient
 * - aec: audio did not send long time
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 3.06.09    Time: 11:52
 * Updated in $/VSNA/VSClient
 * - audiorender cleanup
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 2.06.09    Time: 15:35
 * Updated in $/VSNA/VSClient
 * - min autogain decreased
 *
 * *****************  Version 17  *****************
 * User: Sanufriev    Date: 18.05.09   Time: 15:56
 * Updated in $/VSNA/VSClient
 * - were removed noise insert in CheckNoiseInsert(), only checked
 * ARENDER_NOISE_MAXTIME for all devices
 * - were added first jump after zero buffer for WaveOut device in Echo
 * Module
 *
 * *****************  Version 16  *****************
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
 * *****************  Version 15  *****************
 * User: Dront78      Date: 13.05.09   Time: 14:15
 * Updated in $/VSNA/VSClient
 * - delete [] fixed
 *
 * *****************  Version 14  *****************
 * User: Sanufriev    Date: 8.05.09    Time: 12:08
 * Updated in $/VSNA/VSClient
 * - were modifed new version AviWriter
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 5.03.09    Time: 14:59
 * Updated in $/VSNA/VSClient
 * - were added support multi-cnnel aec in VS_GlobalEcho &
 * VS_SpeexEchoCancel
 * - were removed skip-frames for first calculation audio render frequency
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 10.02.09   Time: 13:50
 * Updated in $/VSNA/VSClient
 * - were changed start time for aec statistic calc
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 9.02.09    Time: 9:54
 * Updated in $/VSNA/VSClient
 * - were improved speex aec
 * - were added audio devices frequency calculate
 * - were added speex resample in echo module
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 24.11.08   Time: 13:24
 * Updated in $/VSNA/VSClient
 * - were added recoding for group confrrence (not enable)
 * - were added new mixer module
 * - were modified CAviFile (add support synch MS GSM 6.10 in play/read
 * mode)
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 10.10.08   Time: 14:15
 * Updated in $/VSNA/VSClient
 * - upper bound for skiping no vad frame is up
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 25.09.08   Time: 15:27
 * Updated in $/VSNA/VSClient
 * - were fixed GetBufferedDurr() (decrease probability of threads
 * conflict on multi-core cpu)
 * - were changed jitter algorithm
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 9.07.08    Time: 13:24
 * Updated in $/VSNA/VSClient
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 9.07.08    Time: 13:08
 * Updated in $/VSNA/VSClient
 * - were modified audio render algorithm
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 17.06.08   Time: 17:51
 * Updated in $/VSNA/VSClient
 * - bugfix #4465
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 10.06.08   Time: 15:07
 * Updated in $/VSNA/VSClient
 * - Speex echo cancellation by default
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 23.05.08   Time: 17:24
 * Updated in $/VSNA/VSClient
 * - bugfix #4355
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 24.12.07   Time: 17:37
 * Updated in $/VS2005/VSClient
 * - master volume added
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 20.12.07   Time: 16:14
 * Updated in $/VS2005/VSClient
 * - added software AEC
 * - Speex AEC improved
 * - Audio MediaFormat changed to commit Speex preprocess
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 15.11.07   Time: 21:10
 * Updated in $/VS2005/VSClient
 * - new AG Control
 * - fixed bug with audio capture devices having no control
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 4.07.07    Time: 19:25
 * Updated in $/VS2005/VSClient
 * - device statuses added
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 4.06.07    Time: 11:34
 * Updated in $/VS2005/VSClient
 * - bugfix #2162
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 44  *****************
 * User: Smirnov      Date: 25.01.07   Time: 14:07
 * Updated in $/VS/VSClient
 * - avi play first errors corrected, some impruvements
 *
 * *****************  Version 43  *****************
 * User: Smirnov      Date: 24.01.07   Time: 19:41
 * Updated in $/VS/VSClient
 * - play avi module
 *
 * *****************  Version 42  *****************
 * User: Smirnov      Date: 22.01.07   Time: 15:30
 * Updated in $/VS/VSClient
 * - changed default value for XP EAC
 *
 * *****************  Version 41  *****************
 * User: Smirnov      Date: 17.01.07   Time: 12:33
 * Updated in $/VS/VSClient
 * - for DirectSound devices NoiseGen restored to "always"
 *
 * *****************  Version 40  *****************
 * User: Smirnov      Date: 15.01.07   Time: 11:33
 * Updated in $/VS/VSClient
 * - XP EAC while conference only
 * - intrface to turn it on added
 *
 * *****************  Version 39  *****************
 * User: Smirnov      Date: 16.11.06   Time: 19:26
 * Updated in $/VS/VSClient
 * - more aggressive voice skipping
 *
 * *****************  Version 38  *****************
 * User: Smirnov      Date: 15.11.06   Time: 18:05
 * Updated in $/VS/VSClient
 * - debug info
 *
 * *****************  Version 37  *****************
 * User: Smirnov      Date: 4.10.06    Time: 16:17
 * Updated in $/VS/VSClient
 * - set minimal capture volume as 1/256-th of maximum
 *
 * *****************  Version 36  *****************
 * User: Smirnov      Date: 29.09.06   Time: 14:02
 * Updated in $/VS/VSClient
 * - programm audio mute
 *
 * *****************  Version 35  *****************
 * User: Smirnov      Date: 25.07.06   Time: 18:09
 * Updated in $/VS/VSClient
 * - Added system waveout Mute
 *
 * *****************  Version 34  *****************
 * User: Smirnov      Date: 13.07.06   Time: 18:56
 * Updated in $/VS/VSClient
 * - new audio controls (via mixer)
 *
 * *****************  Version 33  *****************
 * User: Smirnov      Date: 12.04.06   Time: 16:47
 * Updated in $/VS/VSClient
 * - hardware endpoint properties repaired
 *
 * *****************  Version 32  *****************
 * User: Smirnov      Date: 5.04.06    Time: 17:24
 * Updated in $/VS/VSClient
 * - low-level audio devices
 * - Direct Sound devices added
 *
 * *****************  Version 31  *****************
 * User: Smirnov      Date: 9.03.06    Time: 17:38
 * Updated in $/VS/VSClient
 * - AGC implemented
 * - "long latency" audiorender mode
 *
 * *****************  Version 30  *****************
 * User: Smirnov      Date: 16.02.06   Time: 12:15
 * Updated in $/VS/VSClient
 * - new TheadBase class
 * - receiver now can be inited while in conference
 *
 * *****************  Version 29  *****************
 * User: Smirnov      Date: 6.10.05    Time: 14:18
 * Updated in $/VS/VSClient
 * - ipp audiocodecs now application depended
 *
 * *****************  Version 28  *****************
 * User: Smirnov      Date: 28.09.05   Time: 18:54
 * Updated in $/VS/VSClient
 * - g722 removed from CLient library
 *
 * *****************  Version 27  *****************
 * User: Smirnov      Date: 12.09.05   Time: 14:41
 * Updated in $/VS/VSClient
 * - added new codecs support in client: g728, g729a, g722.1
 *
 * *****************  Version 26  *****************
 * User: Smirnov      Date: 11.08.05   Time: 18:21
 * Updated in $/VS/VSClient
 * - added debug module
 *
 * *****************  Version 25  *****************
 * User: Smirnov      Date: 10.08.05   Time: 19:23
 * Updated in $/VS/VSClient
 * - strict audiobufferisation parametr
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 10.08.05   Time: 13:33
 * Updated in $/VS/VSClient
 * - new low latensy and noise generation schema
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 2.08.05    Time: 19:27
 * Updated in $/VS/VSClient
 * audio latensy in loss mode is decreased
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 27.05.05   Time: 16:08
 * Updated in $/VS/VSClient
 * aded new IPP ver 4.1
 * added g711, g728, g729 from IPP
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 10.03.05   Time: 20:42
 * Updated in $/VS/VSClient
 * last sinc fix
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 10.03.05   Time: 12:48
 * Updated in $/VS/VSClient
 * new sinc for audio-video
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 1.03.05    Time: 13:25
 * Updated in $/VS/VSClient
 * bug(568) on hiperthreading with audio capture fixed
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 24.02.05   Time: 19:29
 * Updated in $/VS/VSClient
 * volume level corected
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 22.02.05   Time: 18:22
 * Updated in $/VS/VSClient
 * low latency switch
 * bitrate degradation more accurate
 * Neigl for streams only for local conf
 * fix with Localalloc for otherId
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 4.02.05    Time: 17:38
 * Updated in $/VS/VSClient
 * comented echocancel
 * latency analis window = 30 msec
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 3.02.05    Time: 20:22
 * Updated in $/VS/VSClient
 * EcHo cancel repaired
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 21.01.05   Time: 15:56
 * Updated in $/VS/VSClient
 * renders volume
 *
 * *****************  Version 13  *****************
 * User: Melechko     Date: 20.01.05   Time: 15:40
 * Updated in $/VS/VSClient
 * Microphone level function restore
 *
 * *****************  Version 12  *****************
 * User: Melechko     Date: 19.01.05   Time: 18:01
 * Updated in $/VS/VSClient
 * Add re-init audio format
 *
 * *****************  Version 11  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:02
 * Updated in $/VS/VSClient
 * some changes :)
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 18.01.05   Time: 14:32
 * Updated in $/VS/VSClient
 * added new caps - Any Audio Buffer Len
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 22.12.04   Time: 16:58
 * Updated in $/VS/VSClient
 * vad presielence now 240 ms
 * caupture audio started now in conference only
 *
 * *****************  Version 8  *****************
 * User: Admin        Date: 16.12.04   Time: 20:08
 * Updated in $/VS/VSClient
 * doxigen comments
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 9.12.04    Time: 19:48
 * Updated in $/VS/VSClient
 * last good sinc
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 8.12.04    Time: 20:26
 * Updated in $/VS/VSClient
 * new video-audio sinc
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 30.11.04   Time: 20:54
 * Updated in $/VS/VSClient
 * removed aold audio
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 29.11.04   Time: 20:04
 * Updated in $/VS/VSClient
 * new audio render and capture classes
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 27.11.04   Time: 13:27
 * Updated in $/VS/VSClient
 * intefaca changed
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 25.11.04   Time: 18:38
 * Updated in $/VS/VSClient
 * added some kinds of improvements
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 24.11.04   Time: 20:13
 * Created in $/VS/VSClient
 * added new audio files and classes
*
****************************************************************************/


/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSAudioNew.h"
#include "VSAudioCaptureList.h"
#include "VS_ApplicationInfo.h"
#include "../std/cpplib/VS_MediaFormat.h"
#include "../std/cpplib/VS_Protocol.h"
#include "std-generic/cpplib/utf8.h"
#include "Transcoder/AudioCodec.h"
#include "Transcoder/VS_IppAudiCodec.h"
#include "Transcoder/VSAudioVad.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "VS_Dmodule.h"
#include "../Audio/NoiseSuppression/VS_RtcNoiseSuppression.h"
#include "../Audio/GainControl/VS_RtcDigitalAgc.h"
#include "../Audio/GainControl/VS_Agc.h"
#include "../Audio/GainControl/VS_SysVolSettings.h"
#include "std/VS_FifoBuffer.h"
#include "VS_Dmodule.h"
#include "VS_EchoDebugger.h"


/****************************************************************************
 * Defines
 ****************************************************************************/
#define ARENDER_BUFFER_MAXTIME		6000	///< max time of queued data, msec
#define ARENDER_INIT_LATENCYBUFFS	3		///< initial recomended buffer duration, buffers
#define ARENDER_NOISE_MAXTIME		2000	///< maximum time of continuous noise generation, msec
#define ARENDER_ANALISETIME			40		///< time of analising window, sec
#define ARENDERJ_ANALISETIME		30		///< time of analising window, sec
#define ARENDER_PENDANALISETIME		1		///< time of analising window, sec
#define ARENDER_SKIPVSNOISE_TIME	5000	///< time sinse noise gen, before that we can't skip audio, msec
#define ARENDER_NOISEONETIME		500		///< max durration of noise generated at one time, msec
#define ARENDER_RECOMENDEDMINDURR	240		///< max durration of noise generated at one time, msec

#define ACAPTURE_BUFFER_MAXTIME		4000	///< max time of captured data without reading it, msec

#define AGC_CONTROL_TIME			4		///< time of averaging, sec
#define AGC_MAX_GAIN				2.		///< maximum gain for agc, multiplier

#define SYSVOL_ANALISE_TIME			6		///< time of averaging, secz

/****************************************************************************
 * VS_AudioDeviceBase
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
VS_AudioDeviceBase::VS_AudioDeviceBase()
{
	memset(this, 0, sizeof(VS_AudioDeviceBase));
	m_hComplEv = CreateEvent(0, 0, 0, 0);
}

VS_AudioDeviceBase::~VS_AudioDeviceBase()
{
	if (m_hComplEv) CloseHandle(m_hComplEv);
}

/**
 **************************************************************************
 * \param		mf	[in] pointer to mediaformet
 ****************************************************************************/
void VS_AudioDeviceBase::SetWaveFormat(VS_MediaFormat *mf)
{
	m_wf.wFormatTag = WAVE_FORMAT_PCM;
	m_wf.nChannels = 1;
	m_wf.nSamplesPerSec = mf->dwAudioSampleRate;
	m_wf.wBitsPerSample = 16;
	m_wf.nBlockAlign = m_wf.nChannels*m_wf.wBitsPerSample / 8;
	m_wf.nAvgBytesPerSec = m_wf.nSamplesPerSec*m_wf.nBlockAlign;
}

/**
****************************************************************************
* \param	in	[in/out] pointer to procesing data buffer
* \param	samples	[in] number of samplea in buffer
******************************************************************************/
void VS_AudioDeviceBase::CalcLevel(short* in, int samples)
{
	if (!m_IsValid) return;

	// adjust volume
	int Volume = m_Volume>>4;					// normalize to 12 bit
	if (Volume < 0x1) {							// set zero
		memset(in, 0, samples*sizeof(short));
		m_Level = 0;
		return;
	}
	else if (Volume < 0x1000) {
		for(int i=0; i<samples; i++)
			in[i] = (in[i]*Volume)>>12;
	}
	else if (Volume > 0x1000) {
		for(int i=0; i<samples; i++) {
			int v = (in[i]*Volume)>>12;
			if		(v>0x7fff) v = 0x7fff;
			else if (v<-0x8000) v = -0x8000;
			in[i] = v;
		}
	}
	// calculate level
	__int64 s = 0;
	for(int i=0; i<samples; i++)
		s+= in[i]*in[i];
	m_Level = (int)sqrt((double)s/samples);
}

void VS_AudioDeviceBase::SetCallId(char* CallId)
{
	strncpy(m_CallId, CallId, MAX_PATH);
}

/****************************************************************************
 * VS_ARenderDevice
 ****************************************************************************/
int VS_ARenderDevice::m_muteAll = 0;
/**
 **************************************************************************
 ****************************************************************************/
VS_ARenderDevice::VS_ARenderDevice()
{
	m_NoiseTime = 0;
	m_SkipTime = 0;
	m_LastFillTime = 0;
	m_LastFillNoise = 0;
	m_time_skip_delta = 0;
	m_vad = 0;
	m_ajitter = 0;
	m_fifo = 0;
	m_Level = 0;
	SetVolume(0xffff);
	m_LastSample = 0;
	m_LastFrameType = FT_NULL;
	m_FramesToSkip = 0;
	m_StrictBuffLen = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
VS_ARenderDevice::~VS_ARenderDevice()
{
	Release();
}

/**
 **************************************************************************
 * \param		dev		[in] device identifier
 * \param		mf		[in] pointer to mediaformet
 * \param		hVoice	[in] handle to event, signalled when next buffer played
 * \return false if initialization failed
 ****************************************************************************/

bool VS_ARenderDevice::Init(int dev, VS_MediaFormat *mf)
{
	if (!mf || !mf->IsAudioValid()) return false;

	Release();

	m_NoiseTime = timeGetTime();
	m_start_time_ar = m_NoiseTime;
	m_SkipTime = m_NoiseTime;
	m_time_skip_delta = m_NoiseTime;
	m_LastFillTime = 0;
	m_LastFillNoise = 0;

	SetWaveFormat(mf);

	m_codec = VS_RetriveAudioCodec(mf->dwAudioCodecTag, false);
	if (!m_codec)
		return false;

	m_BuffLen = mf->dwAudioBufferLen;
	int NumOfBuff = ARENDER_BUFFER_MAXTIME*m_wf.nAvgBytesPerSec/1000/m_BuffLen;
	m_AudioTemp = (char*)malloc(150000);

	m_vad = new VSAudioVAD;
	m_vad->Init(m_wf.nSamplesPerSec, m_wf.wBitsPerSample);
	m_vad->SetMode(ENABLE_NOISEGEN);
	if (m_codec->Init(&m_wf) != 0)
		return false;

	m_ajitter = new VS_ARenderAnalyse;
	m_ajitter->Init(ARENDERJ_ANALISETIME*m_wf.nAvgBytesPerSec/m_BuffLen, ARENDERJ_ANALISETIME*m_wf.nAvgBytesPerSec/m_BuffLen);

	m_fifo = new VS_FifoBuffer(m_wf.nAvgBytesPerSec/2);

	VS_AudioDeviceDesc desc;
	desc.dev = dev;
	desc.Event = m_hComplEv;
	desc.IsCapt = false;
	desc.Len = m_BuffLen;
	desc.Num = NumOfBuff;
	desc.wf = m_wf;
	desc.IsVista = m_bVista;

	QueryDevice(desc);
	m_IsValid = m_device && m_device->IsValid();
	if (!m_IsValid)
		Release();

	m_noise_lenght = (((2000 * m_wf.nSamplesPerSec * 2) / 1000) / m_BuffLen + 1) * m_BuffLen;

	return m_IsValid;
}


/**
 **************************************************************************
 ****************************************************************************/
void VS_ARenderDevice::Release()
{
	m_IsValid = false;
	ReleaseDevice();
	if (m_fifo) delete m_fifo; m_fifo = 0;
	if (m_ajitter) delete m_ajitter; m_ajitter = 0;
	if (m_codec) delete m_codec; m_codec = 0;
	if (m_vad) delete m_vad; m_vad = 0;
	if (m_AudioTemp) free(m_AudioTemp); m_AudioTemp = 0;
	m_LastSample = 0;
	m_LastFrameType = FT_NULL;
	m_FramesToSkip = 0;
	m_PlayedBuffs = 0;
}


/**
 **************************************************************************
 * \return cuurent pending buffer duration
 ****************************************************************************/
int VS_ARenderDevice::GetCurrBuffDurr()
{
	if (!m_IsValid) return 0;
	return m_device->GetBufferedDurr();
}


/**
 **************************************************************************
 * \param		buff	[in] pointer to input compressed audio data
 * \param		size	[in] size of input data
 * \return 1 if buffer queued or 0 else
 ****************************************************************************/

int VS_ARenderDevice::Play(char* buff, int size)
{
	int BuffDurrA = 0, BuffDurrB = 0, MaxBuffDur = 0, MinBuffDur = 0;
	int GMinBuffDur = 0, NumSkip = 0, NumEmptyBuff = 0, PendTime = 0;

	if (!m_IsValid) return 0;

	size = m_codec->Convert((BYTE*)buff, (BYTE*)m_AudioTemp, size);
	buff = m_AudioTemp;

	int CurrTime = timeGetTime();
	int dt = CurrTime - m_LastFillTime;
	if (dt > ARENDER_BUFFER_MAXTIME) {
		m_ajitter->Clear();
		dt = 0;
		m_time_skip_delta = CurrTime;
		m_FramesToSkip = 0;
	}
	m_LastFillTime = CurrTime;
	if (m_device->State()!= ADSTATE_START)
		m_device->Start();

	int num_smpls = m_device->GetBufferedDurr(true);
	m_fifo->AddData(buff, size);
	if (m_device->Type() == ADTYPE_FDOUT && num_smpls <= 0) {
		const int PreSamples = (m_wf.nSamplesPerSec * 50) / 1000; // 20 ms
		num_smpls = -num_smpls + PreSamples;
		int len = num_smpls * 2;
		m_vad->GetNoise((short*)buff, len);
		CorrectSpike((short*)buff, len, FT_NOISE);

		PreRenderCallBack((short*)buff, num_smpls);

		bool res = m_device->OutWrite((short*)buff, len);
		if (!res) {
			DTRACE(VSTM_AUDI0, "Write to Out Adio Failed");
		} else {
			PostRenderCallBack((short*)buff, num_smpls);
		}
		DTRACE(VSTM_AUDI0, "Add noise: %d samples", num_smpls);
		num_smpls = PreSamples;
	}
	else {
		PendTime = (num_smpls * 1000) / m_wf.nSamplesPerSec;
	}
	CalcFrequency(num_smpls);
	int OneBuffDurr = 1000 * m_BuffLen / m_wf.nAvgBytesPerSec;
	int dB = 3840 - (3 * OneBuffDurr) / 2;
	int DurrBound = 70;

	m_ajitter->Snap(dt, PendTime);
	m_PlayedBuffs++;

	if (m_PlayedBuffs < ARENDER_INIT_LATENCYBUFFS) {	/// then add buffers to fifo
		return 1;
	}

	if (PendTime == 0) { // если буфер обнулился
		m_ajitter->SnapEmptyBuffer();
	}
	m_ajitter->GetBuffDur(BuffDurrA, BuffDurrB, MaxBuffDur, MinBuffDur, GMinBuffDur, NumSkip, NumEmptyBuff);

	if (PendTime >= dB) {
		m_FrequencyStat.is_first = false;
	}

	while (m_fifo->GetData(buff, m_BuffLen)) {			/// flush all data from fifo
		bool IsVad = m_vad->IsVad(buff, m_BuffLen);

		bool SkipFrame = false;

		if (!m_FrequencyStat.is_first) {

			if (m_FramesToSkip > 0 && ((PendTime - m_FramesToSkip * OneBuffDurr) < (DurrBound + OneBuffDurr))) { // защита от опустошения буфера
				m_FramesToSkip = 0;
			}

			if (m_FramesToSkip > 0 || PendTime >= dB) {

				SkipFrame = true;

			} else if (IsVad) {

				if (CurrTime - m_start_time_ar > 5000) {
					if (PendTime > 2 * BuffDurrB) { // если поступило много данных
						if (PendTime > (DurrBound + OneBuffDurr)) {
							double dmin = std::min(0.9 * PendTime, 1.2 * BuffDurrB + OneBuffDurr);
							m_FramesToSkip = (int)((double)(PendTime - dmin) / (double)OneBuffDurr);
							if (m_FramesToSkip > 0) {
								m_ajitter->SnapSkip();
								m_time_skip_delta = CurrTime;
							}
						}
					} else if (CurrTime - m_start_time_ar > 14000) { // только после 14 секунд от начала приема звука
						if (CurrTime - m_time_skip_delta > 1000) { // 1 раз в секунду
							if (PendTime > (DurrBound + OneBuffDurr)) {
								if (NumSkip == 0 && NumEmptyBuff == 0) { // если в течении предыдущих 30 секунд не было скипов и опустошений
									if (GMinBuffDur > OneBuffDurr) { // ограничение на минимальное значение глобального минимума буфера
										double delta = std::min<double>(GMinBuffDur, 0.5 * BuffDurrA);
										if (PendTime - delta < 0.9 * BuffDurrB) { // ограничение на нижнюю границу буфера
											delta = PendTime - 0.9 * BuffDurrB;
										}
										m_FramesToSkip = (int)(delta / (double)OneBuffDurr);
									}
								}
							}
							m_time_skip_delta = CurrTime;
						}
					} else {
						m_time_skip_delta = CurrTime;
					}
				}

				if (m_FramesToSkip > 0) {
					SkipFrame = true;
				}
			}
			else {
				if (PendTime > (DurrBound + OneBuffDurr)) {
					SkipFrame = GMinBuffDur > 0 ? PendTime > MinBuffDur + OneBuffDurr/2 :  PendTime > MaxBuffDur*5/4 + OneBuffDurr/2;
				}
			}
		}

		/*DTRACE(VSTM_AUDI0, "ct=%9d, b=%4d, minb=%4d, a=%4d, b=%4d, fskip=%3d, svad=%d, snvad=%d",
							CurrTime - m_start_time_ar, PendTime, GMinBuffDur, BuffDurrA, BuffDurrB,
							(m_FramesToSkip > 0) ? m_FramesToSkip : 0, (m_FramesToSkip > 0 && SkipFrame) ? 1 : 0,
							(m_FramesToSkip <= 0 && SkipFrame) ? 1 : 0);*/
		if (SkipFrame && PendTime > m_StrictBuffLen) {
			//DTRACE(VSTM_AUDI0, "ct=%9d p=%4d, bda=%4d, bdb=%4d --SKIP=%2d", CurrTime, PendTime, BuffDurrA, BuffDurrB, m_FramesToSkip);
			if (m_FramesToSkip>0) m_FramesToSkip--;
			m_SkipTime = CurrTime;
			m_LastFrameType = FT_SKIP;
		} else {
			m_FramesToSkip = 0;
			CalcLevel((short*)buff, m_BuffLen/2);
			//DTRACE(VSTM_AUDI0, "ct=%9d p=%4d, bda=%4d, bdb=%4d LOUD=%5d", CurrTime, PendTime, BuffDurrA, BuffDurrB, m_Level);
			CorrectSpike((short*)buff, m_BuffLen, FT_PLAY);

			PreRenderCallBack((short*)buff, m_BuffLen / 2);

			bool res = m_device->OutWrite((short*)buff, m_BuffLen);

			if (!res) {
				DTRACE(VSTM_AUDI0, "Write to Out Adio Failed");
			} else {
				PostRenderCallBack((short*)buff, m_BuffLen/2);
			}
			m_PlayedBuffs++;
			PendTime+=OneBuffDurr;
			if (res && g_AviWriterGroup) {
				g_AviWriterGroup->PutAudio(m_CallId, (unsigned char*)buff, m_BuffLen, GetFrequency(m_bEqualDevice), PendTime);
			}
		}

	}

	if (m_device->Type() == ADTYPE_FDOUT || m_device->Type() == ADTYPE_FDMUXOUT) {
		if (m_device->Type() == ADTYPE_FDMUXOUT) {
			if ((CurrTime - m_LastFillNoise) > 1000) {
				m_vad->GetNoise((short*)buff, m_wf.nSamplesPerSec * m_wf.nBlockAlign);
				m_device->OutWriteNoise((short*)buff, m_wf.nSamplesPerSec * m_wf.nBlockAlign);
				m_LastFillNoise = CurrTime;
			}
		} else {

			m_vad->GetNoise((short*)buff, m_BuffLen);
			m_device->OutWriteNoise((short*)buff, m_BuffLen);
		}
	}

	return 1;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_ARenderDevice::CheckNoiseInsert()
{
	if (!m_IsValid) return;
	if (m_PlayedBuffs < 1) return; // first data yet not arrived

	if ((int)timeGetTime()-m_LastFillTime > ARENDER_NOISE_MAXTIME) { // no new audio more then ARENDER_NOISE_MAXTIME
		if (m_device->State() != ADSTATE_STOP) {
			m_device->Stop();
			m_FrequencyStat.Reset();
			m_FrequencyStat.pFreqDeviation->Clear();
		}
	}
}



/**
 **************************************************************************
 * \param		data	[in] samples to correct
 * \param		lenght	[in] lenght buffer to correct
 * \param		type	[in] audio data type
 ****************************************************************************/
void VS_ARenderDevice::CorrectSpike(short* data, int lenght, frame_type type)
{
	if (m_muteAll)
		memset(data, 0, lenght);
	if (type != m_LastFrameType) {
		int d = (m_LastSample - data[0])/3;
		data[0]+=2*d;
		data[1]+=d;
	}
	if (type != FT_NULL) {
		m_LastFrameType = type;
		m_LastSample = data[lenght/2-1];
	}
}

void VS_ARenderDevice::CalcFrequency(int buff_durr)
{
	if (!m_FrequencyStat.bValid) return;
	LARGE_INTEGER curt;
	QueryPerformanceCounter(&curt);
	int dbuff = 0;
	if (m_device->Type() == ADTYPE_WAVEOUT) dbuff = m_wf.nSamplesPerSec / 50;
	if (buff_durr <= dbuff && m_FrequencyStat.start_pos_buffer != 0) {
		DTRACE(VSTM_ECHO, "%x : zero buffer, %d samples", this, buff_durr);
		if (m_device->Type() == ADTYPE_FDOUT) return;
		m_FrequencyStat.start_pos_buffer = 0;
		m_FrequencyStat.in_time.QuadPart = 0;
		m_FrequencyStat.dt_calc = 0.0;
		m_FrequencyStat.pFreqDeviation->Clear();
	}
	if (m_FrequencyStat.in_time.QuadPart == 0) {
		m_FrequencyStat.in_time.QuadPart = curt.QuadPart + 3 * m_FrequencyStat.freq.QuadPart;
	}
	double dt = 1000.0 * (curt.QuadPart - m_FrequencyStat.in_time.QuadPart) / (double)(m_FrequencyStat.freq.QuadPart);
	if (dt >= 0) {
		if (m_FrequencyStat.start_pos_buffer == 0) {
			m_FrequencyStat.start_pos_buffer = buff_durr;
			m_FrequencyStat.prev_buff_durr = buff_durr;
			m_FrequencyStat.start_samples = m_device->GetNumSamples();
			m_FrequencyStat.in_time = curt;
			m_FrequencyStat.dt_calc = dt;
		} else if (m_FrequencyStat.prev_buff_durr != buff_durr) {
			__int64 num_smpls = m_device->GetNumSamples() - m_FrequencyStat.start_samples -
								(buff_durr - m_FrequencyStat.start_pos_buffer);
			m_FrequencyStat.pFreqDeviation->Snap(dt, (double)num_smpls);
			m_FrequencyStat.prev_buff_durr = buff_durr;
		}
		if ((dt - m_FrequencyStat.dt_calc) >= m_FrequencyStat.iCalcRange) {
			double freq = m_FrequencyStat.pFreqDeviation->GetA1() * 1000.0;
			if (freq > 0.0) {
				if (fabs((freq - m_FrequencyStat.fFrequencyInit) / m_FrequencyStat.fFrequencyInit) <= m_FrequencyStat.fDeltaFrequency)
					m_FrequencyStat.fFrequency = freq;
			}
			DTRACE(VSTM_ECHO, "%x : calc_freq = %8.2f, rnd_freq = %8.2f, dt = %8.2f", this, freq, m_FrequencyStat.fFrequency, dt);
			m_FrequencyStat.is_first = false;
			m_FrequencyStat.dt_calc = dt;
		}
	}
}

bool VS_ARenderDevice::GetBuffBounds(int &a, int &b)
{
	if (!IsValid())
		return false;
	int BuffDurrA = 0, BuffDurrB = 0, MaxBuffDur = 0, MinBuffDur = 0;
	int GMinBuffDur = 0, NumSkip = 0, NumEmptyBuff = 0, PendTime = 0;
	bool res = m_ajitter->GetBuffDur(BuffDurrA, BuffDurrB, MaxBuffDur, MinBuffDur, GMinBuffDur, NumSkip, NumEmptyBuff);
	a = BuffDurrA;
	b = BuffDurrB;
	return res;
}


/****************************************************************************
 * VS_PCMAudioRender
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
VS_PCMAudioRender::VS_PCMAudioRender()
{
	m_Level = 0;
	SetVolume(0xffff);
	m_fifo = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
VS_PCMAudioRender::~VS_PCMAudioRender()
{
	Release();
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_PCMAudioRender::Init(int dev, WAVEFORMATEX* wf)
{
	Release();
	m_wf = *wf;

	m_BuffLen = wf->nAvgBytesPerSec/8;
	int NumOfBuff = 4000*m_wf.nAvgBytesPerSec/1000/m_BuffLen;

	m_fifo = new VS_FifoBuffer(m_wf.nAvgBytesPerSec/2);

	VS_AudioDeviceDesc desc;
	desc.dev = dev;
	desc.Event = m_hComplEv;
	desc.IsCapt = false;
	desc.Len = m_BuffLen;
	desc.Num = NumOfBuff;
	desc.wf = m_wf;
	desc.IsVista = m_bVista;

	QueryDevice(desc);
	m_IsValid = m_device && m_device->IsValid();
	if (!m_IsValid)
		Release();
	return m_IsValid;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_PCMAudioRender::Release()
{
	m_IsValid = false;
	ReleaseDevice();
	if (m_fifo) delete m_fifo; m_fifo = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_PCMAudioRender::Play(char* buff, int size)
{
	if (!m_IsValid) return 0;

	int CurrTime = timeGetTime();

	int PendTime = GetCurrBuffDurr();

	m_fifo->AddData(buff, size);

	while (m_fifo->GetData(buff, m_BuffLen)) {			/// flush all data from fifo
		if (m_device->State()!= ADSTATE_START)
			m_device->Start();
		CalcLevel((short*)buff, m_BuffLen/2);
		if (VS_ARenderDevice::m_muteAll)
			memset(buff, 0, m_BuffLen);
		bool res = m_device->OutWrite((short*)buff, m_BuffLen);
	}
	return 1;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_PCMAudioRender::Start()
{
	if (m_IsValid)
		m_device->Start();
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_PCMAudioRender::Stop()
{
	if (m_IsValid)
		m_device->Stop();
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_PCMAudioRender::GetCurrBuffDurr()
{
	return m_IsValid ? m_device->GetBufferedDurr() : 0;
}


/****************************************************************************
 * VS_ACaptureDevice
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
VS_ACaptureDevice::VS_ACaptureDevice()
{
	m_Level = 0;
	m_vad = 0;
	*m_CurrenDeviceName = 0;
	VS_AudioDeviceBase::SetVolume(0xffff);
	m_VolContr=0;
}

/**
 **************************************************************************
 ****************************************************************************/
VS_ACaptureDevice::~VS_ACaptureDevice()
{
	Release();
}

/**
 **************************************************************************
 * \param		dev		[in] device identifier
 * \param		mf		[in] pointer to mediaformet
 * \param		hVoice	[in] handle to event, signalled when next buffer arrived
 * \return false if initialization failed
 ****************************************************************************/
bool VS_ACaptureDevice::Init(int dev, VS_MediaFormat *mf)
{
	if (!mf || !mf->IsAudioValid()) return false;
	VS_AutoLock alock(this);

	Release();

	SetWaveFormat(mf);

	m_codec = VS_RetriveAudioCodec(mf->dwAudioCodecTag, true);
	if (!m_codec)
		return false;

	m_BuffLen = mf->dwAudioBufferLen;
	int NumOfBuff = ACAPTURE_BUFFER_MAXTIME*m_wf.nAvgBytesPerSec/1000/m_BuffLen;

	m_AudioTemp = (char*)malloc(100000);

	m_vad = new VSAudioVAD;
	m_vad->Init(m_wf.nSamplesPerSec, m_wf.wBitsPerSample);
	m_vad->SetMode(ENABLE_VAD);

	if (m_codec->Init(&m_wf) != 0)
		return false;

	DWORD vol = GetVolume();
	VS_AudioDeviceDesc desc;
	desc.dev = dev;
	desc.Event = m_hComplEv;
	desc.IsCapt = true;
	desc.Len = m_BuffLen;
	desc.Num = NumOfBuff;
	desc.wf = m_wf;
	desc.IsVista = m_bVista;

	m_VolContr = VS_VolControlBase::Factory();
	m_VolContr->Init(m_CurrenDeviceName, dev, false);
	LoadVolumeFromRegister();

	PostMessage(VS_AudioMixerVolume::GetWnd(), MM_MIXM_CONTROL_CHANGE, 42, 0);

	m_VoiceChanger.Init(1.0);

	QueryDevice(desc);
	m_IsValid = m_device && m_device->IsValid();
	if (!m_IsValid)
		Release();

	return m_IsValid;
}


/**
 **************************************************************************
 ****************************************************************************/
void VS_ACaptureDevice::Release()
{
	VS_AutoLock alock(this);

	m_IsValid = false;
	ReleaseDevice();
	if (m_vad) delete m_vad; m_vad = 0;
	if (m_codec) delete m_codec; m_codec = 0;
	if (m_AudioTemp) free(m_AudioTemp); m_AudioTemp = 0;
	SaveVolumeToRegister();
	delete m_VolContr; m_VolContr=0;

	m_Level = 0;
}

void VS_ACaptureDevice::SaveVolumeToRegister()
{
	if (m_VolContr)
	{
		std::string dev_name = "Client\\AudioCaptureSlot\\";
		dev_name += vs::WideCharToUTF8Convert(m_CurrenDeviceName);
		VS_RegistryKey vol_key(true, dev_name, false, true);

		float currVol = 0.5f;
		float currBoost = 10.f;

		m_VolContr->GetMicVolume(&currVol);
		m_VolContr->GetMicBoost(&currBoost);

		int onlineVol = (int)(currVol * 100.0f);
		int onlineBoost = (int)currBoost + 1000;

		vol_key.SetValue(&onlineVol, 4, VS_REG_INTEGER_VT, "SysVolOnline");
		vol_key.SetValue(&onlineBoost, 4, VS_REG_INTEGER_VT, "SysBoostOnline");
	}
}

void VS_ACaptureDevice::LoadVolumeFromRegister()
{
	if (m_VolContr)
	{
		std::string dev_name = "Client\\AudioCaptureSlot\\";
		dev_name += vs::WideCharToUTF8Convert(m_CurrenDeviceName);
		VS_RegistryKey vol_key(true, dev_name, false, true);

		int onlineVol = 50;
		int onlineBoost = 1010;

		vol_key.GetValue(&onlineVol, 4, VS_REG_INTEGER_VT, "SysVolOnline");
		vol_key.GetValue(&onlineBoost, 4, VS_REG_INTEGER_VT, "SysBoostOnline");

		m_VolContr->SetMicVolume((float)onlineVol / 100.0f);
		m_AudioProcessing.SetAnalogLevel(onlineVol * 10);
		m_VolContr->SetMicBoost((float)onlineBoost - 1000.0f);
	}
}

/**
 **************************************************************************
 * \param		buff	[out] pointer to data buffer
 * \param		size	[out] size of copied data
 * \return size of copied data
 ****************************************************************************/
int VS_ACaptureDevice::Capture(char* buff, int &size, bool use_audio)
{

	VS_AutoLock alock(this);
	size = 0;
	if (m_IsValid && m_device->IsRestartDevice()) {
		DTRACE(VSTM_AUDI0, "ReInit Audio");
		return -1;
	}
	if (m_IsValid && m_device->InRead((short*)m_AudioTemp, size))
	{

		if (m_VolContr && m_VolContr->IsValid())
		{
			HWND hwnd = VS_AudioMixerVolume::GetWnd();
			if (hwnd)
				PostMessage(hwnd, 0x03d1, 0, 0);
		}

		//VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_NEAREND, m_AudioTemp, size);

		CaptureCallBack((short*)m_AudioTemp, size/2);

		if (VS_AudioDeviceManager::m_EnableAgc)
		{
			m_VolContr->SetAgcToDeviceVolume(m_AudioProcessing.GetAnalogLevel());

			float micVolume = 0.0f;

			if (m_bVista)
				m_VolContr->GetMicVolumedB(&micVolume);
			else
				m_VolContr->GetMicVolume(&micVolume);

			m_AudioProcessing.SetAnalogLevel(micVolume * 1000.0f);
		}

		//m_Level = m_AudioProcessing.GetRmsLevel() * 32768.0f;
		m_Level = CalcLevelSimple((short*) m_AudioTemp, size / 2);

		VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_ECHO, m_AudioTemp, size);

		if (use_audio && g_AviWriterGroup) g_AviWriterGroup->PutAudio(m_CallId, (unsigned char*)m_AudioTemp, size, GetFrequency(m_bEqualDevice));
		size = m_codec->Convert((BYTE*)m_AudioTemp, (BYTE*)buff, size);
	}
	return size;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_ACaptureDevice::Pause()
{
	if (m_IsValid) {
		m_device->Stop();
		m_FrequencyStat.Reset();
		m_FrequencyStat.pFreqDeviation->Clear();
	}
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_ACaptureDevice::Start()
{
	if (m_IsValid) {
		m_device->Start();
		m_FrequencyStat.Reset();
		m_FrequencyStat.pFreqDeviation->Clear();
	}
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_ACaptureDevice::ReStart(int dev)
{
	VS_AutoLock alock(this);

	ReleaseDevice();
	int NumOfBuff = ACAPTURE_BUFFER_MAXTIME*m_wf.nAvgBytesPerSec/1000/m_BuffLen;

	VS_AudioDeviceDesc desc;
	desc.dev = dev;
	desc.Event = m_hComplEv;
	desc.IsCapt = true;
	desc.Len = m_BuffLen;
	desc.Num = NumOfBuff;
	desc.wf = m_wf;
	desc.IsVista = m_bVista;

	QueryDevice(desc);
	m_IsValid = m_device && m_device->IsValid();
	Start();

	if (!m_IsValid) Release();
}

int VS_ACaptureDevice::GetVolume()
{
	DTRACE(VSTM_AGCIN, "%s [%d]", __FUNCTION__, std::this_thread::get_id());
	if (m_VolContr && m_VolContr->IsValid())
	{
		float vol;

		m_VolContr->GetMicVolume(&vol);

		return int(vol * 65535.0f);
	}
	else
		return VS_AudioDeviceBase::GetVolume();
}

void VS_ACaptureDevice::SetVolume(int Volume)
{
	if (m_VolContr && m_VolContr->IsValid())
	{
		float volume = float(Volume) / 65535.0f;

		if (!VS_AudioDeviceManager::m_EnableAgc)
			m_VolContr->SetMicVolume(volume);
	}
	else
		VS_AudioDeviceBase::SetVolume(Volume);
}

void VS_ACaptureDevice::CalcFrequency(int buff_durr)
{
	if (!m_FrequencyStat.bValid) return;
	LARGE_INTEGER curt;
	QueryPerformanceCounter(&curt);
	if (m_FrequencyStat.in_time.QuadPart == 0) {
		m_FrequencyStat.in_time.QuadPart = curt.QuadPart + 3 * m_FrequencyStat.freq.QuadPart;
	}
	double dt = 1000.0 * (curt.QuadPart - m_FrequencyStat.in_time.QuadPart) / (double)(m_FrequencyStat.freq.QuadPart);
	if (dt >= 0) {
		if (m_FrequencyStat.dt_prev > 0.0 && (dt - m_FrequencyStat.dt_prev) > 4000.0) {
			m_FrequencyStat.Reset();
			m_FrequencyStat.pFreqDeviation->Clear();
			dt = 0.0;
		}
		if (m_FrequencyStat.start_pos_buffer == 0) {
			m_FrequencyStat.start_pos_buffer = buff_durr;
			m_FrequencyStat.start_samples = m_device->GetNumSamples();
			m_FrequencyStat.in_time = curt;
			m_FrequencyStat.dt_calc = dt;
		} else {
			__int64 num_smpls = (m_device->GetNumSamples() - m_FrequencyStat.start_samples) -
								(m_FrequencyStat.start_pos_buffer - buff_durr);
			m_FrequencyStat.pFreqDeviation->Snap(dt, (double)num_smpls);
		}
		if ((dt - m_FrequencyStat.dt_calc) >= m_FrequencyStat.iCalcRange) {
			double freq = m_FrequencyStat.pFreqDeviation->GetA1() * 1000.0;
			if (freq > 0.0) {
				if (fabs((freq - m_FrequencyStat.fFrequencyInit) / m_FrequencyStat.fFrequencyInit) <= m_FrequencyStat.fDeltaFrequency)
					m_FrequencyStat.fFrequency = freq;
			}
			DTRACE(VSTM_ECHO, "calc_freq = %8.2f, cpt_freq = %8.2f, dt = %8.2f", freq, m_FrequencyStat.fFrequency, dt);
			m_FrequencyStat.dt_calc = dt;
		}
		m_FrequencyStat.dt_prev = dt;
	}
}

/****************************************************************************
 * VS_AudioMixerVolume
 ****************************************************************************/
HWND VS_AudioMixerVolume::m_hwnd = 0;
void* VS_AudioMixerVolume::m_MicMix = 0;
void* VS_AudioMixerVolume::m_WOutMix = 0;

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioMixerVolume::VS_AudioMixerVolume()
{
	m_mixer = 0;
	m_foundControl = 0;
	m_control = 0;
	m_Volume = 0x8000;
	Release();
};

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioMixerVolume::~VS_AudioMixerVolume()
{
	Release();
};

/**
 **************************************************************************
 ****************************************************************************/
bool VS_AudioMixerVolume::Init(int device, Mix_Type type)
{
	Release();
	MMRESULT mmres;
	MIXERCAPS mcaps;
	m_type = type;
	if (device==-1)
		device = 0;
	else if (device<-1)
		return false;
	if		(m_type==MTYPE_MIC) {
		bool flag = m_hwnd!=0 && m_WOutMix==0;
		if (MMSYSERR_NOERROR != mixerOpen(&m_mixer, device, (UINT)m_hwnd, 0, MIXER_OBJECTF_WAVEIN | (flag ? CALLBACK_WINDOW : 0)))
			return false;
		if (flag)
			m_MicMix = m_mixer;
	}
	else if (m_type==MTYPE_WOUT) {
		bool flag = m_hwnd!=0 && m_MicMix==0;
		if (MMSYSERR_NOERROR != mixerOpen(&m_mixer, device, (UINT)m_hwnd, 0, MIXER_OBJECTF_WAVEOUT |(flag ? CALLBACK_WINDOW : 0)))
			return false;
		if (flag)
			m_WOutMix = m_mixer;
	}
	else if (m_type==MTYPE_MASTER) {
		if (MMSYSERR_NOERROR != mixerOpen(&m_mixer, device, 0, 0, MIXER_OBJECTF_WAVEOUT))
			return false;
	}
	mmres = mixerGetDevCaps((UINT)m_mixer, &mcaps, sizeof(MIXERCAPS));
	if (mmres!= MMSYSERR_NOERROR)
		return false;
	for (DWORD j = 0; j<mcaps.cDestinations; j++) {
		MIXERLINE mline;
		mline.cbStruct = sizeof(MIXERLINE);
		mline.dwDestination = j;
		mmres = mixerGetLineInfo((HMIXEROBJ)m_mixer, &mline, MIXER_GETLINEINFOF_DESTINATION);
		if (mmres!=MMSYSERR_NOERROR)
			return false;
		if		(m_type==MTYPE_MIC && mline.dwComponentType!=MIXERLINE_COMPONENTTYPE_DST_WAVEIN)
			continue;
		else if (m_type==MTYPE_WOUT && mline.dwComponentType!=MIXERLINE_COMPONENTTYPE_DST_SPEAKERS)
			continue;
		else if (m_type==MTYPE_MASTER && mline.dwComponentType!=MIXERLINE_COMPONENTTYPE_DST_SPEAKERS)
			continue;
		DWORD ConnNum = mline.cConnections;
		for (DWORD m = 0; m<(ConnNum+1); m++) {
			mline.dwSource = m-1;
			mmres = mixerGetLineInfo((HMIXEROBJ)m_mixer, &mline, m==0? MIXER_GETLINEINFOF_DESTINATION : MIXER_GETLINEINFOF_SOURCE);
			if (mmres!=MMSYSERR_NOERROR)
				return false;
			if (mline.cControls!=0) {
				MIXERLINECONTROLS mlc;
				MIXERCONTROL * pmcs = new MIXERCONTROL[mline.cControls];
				mlc.cbStruct = sizeof(MIXERLINECONTROLS);
				mlc.dwLineID = mline.dwLineID;
				mlc.cControls = mline.cControls;
				mlc.cbmxctrl = sizeof(MIXERCONTROL);
				mlc.pamxctrl = pmcs;
				mmres = mixerGetLineControls((HMIXEROBJ)m_mixer, &mlc, MIXER_GETLINECONTROLSF_ALL);
				if (mmres== MMSYSERR_NOERROR) {
					for (DWORD k = 0; k<mline.cControls; k++) {
						MIXERCONTROL * pp = &pmcs[k];
						bool fnd = false;
						if		(m_type==MTYPE_MIC)
							fnd = (mline.dwComponentType==MIXERLINE_COMPONENTTYPE_DST_WAVEIN || mline.dwComponentType==MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)
							&& pp->dwControlType == MIXERCONTROL_CONTROLTYPE_VOLUME;
						else if (m_type==MTYPE_WOUT)
							fnd = mline.dwComponentType==MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT
							&& pp->dwControlType == MIXERCONTROL_CONTROLTYPE_VOLUME;
						else if (m_type==MTYPE_MASTER)
							fnd = mline.dwComponentType==MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
							&& pp->dwControlType == MIXERCONTROL_CONTROLTYPE_VOLUME;
						if (fnd) {
							m_channels = (pp->fdwControl&MIXERCONTROL_CONTROLF_UNIFORM) ? 1 : mline.cChannels;
							m_foundControl = new MIXERCONTROL;
							*m_foundControl = *pp;
							m_mcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
							m_mcd.dwControlID = m_foundControl->dwControlID;
							m_mcd.cChannels = m_channels;
							m_mcd.cMultipleItems = 0;
							m_control = new MIXERCONTROLDETAILS_UNSIGNED[m_channels];
							m_mcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
							m_mcd.paDetails = m_control;
							delete[] pmcs;
							m_maxrange = m_foundControl->Bounds.dwMaximum - m_foundControl->Bounds.dwMinimum;
							m_minvol = m_foundControl->Bounds.dwMinimum + m_maxrange/4096;
							m_maxvol = m_foundControl->Bounds.dwMaximum;
							m_Volume = GetVolume();

							return true;
						}
					}
				}
				delete[] pmcs;
			}
		}
	}
	return false;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioMixerVolume::Release()
{
	if (m_control) delete [] m_control; m_control = 0;
	if (m_foundControl) delete m_foundControl; m_foundControl = 0;
	if (m_mixer) mixerClose(m_mixer); m_mixer = 0;
	if		(m_type==MTYPE_MIC)
		m_MicMix = 0;
	else if (m_type==MTYPE_WOUT)
		m_WOutMix = 0;
	m_channels = 0;
	m_maxrange = 0xffff;
	m_minvol = 0;
	m_maxvol = 0xffff;
	m_type = MTYPE_MIC;
}


/**
 **************************************************************************
 ****************************************************************************/
bool VS_AudioMixerVolume::SetVolume(DWORD val)
{
	if (val > m_maxvol)
		val = m_maxvol;
	else if (val < m_minvol)
		val = m_minvol;
	m_Volume = val;
	if (IsValid()) {
		for (DWORD i =0; i< m_channels; i++)
			m_control[i].dwValue = m_Volume;
		return mixerSetControlDetails((HMIXEROBJ)m_mixer, &m_mcd, MIXER_SETCONTROLDETAILSF_VALUE)==MMSYSERR_NOERROR;
	}
	else
		return false;
}

/**
 **************************************************************************
 ****************************************************************************/
DWORD VS_AudioMixerVolume::GetVolume()
{
	if (IsValid()) {
		if (mixerGetControlDetails((HMIXEROBJ)m_mixer, &m_mcd, MIXER_SETCONTROLDETAILSF_VALUE)==MMSYSERR_NOERROR) {
			DWORD val = 0;
			for (DWORD i =0; i< m_channels; i++)
				val += m_control[i].dwValue;
			if (m_channels)
				val/=m_channels;
			m_Volume = val;
		}
	}
	return m_Volume;
}


/****************************************************************************
 * VS_AudioCapture
 ****************************************************************************/
const char VS_AudioCapture::_funcMicLevel[]="MicrophoneLevel";
const char VS_AudioCapture::_funcMicVolume[]="MicrophoneVolume";
const char VS_AudioCapture::_funcCaptureCurrent[]="CurrentCapture";
const char VS_AudioCapture::_funcStart[]="Start";
const char VS_AudioCapture::_funcStop[]="Stop";
const char VS_AudioCapture::_funcACConnect[]="Connect";
const char VS_AudioCapture::_funcPitchFiltr[]="PitchFiltr";
const char VS_AudioCapture::_funcACDisconnect[]="Disconnect";
const char VS_AudioCapture::_funcSetCallId[]="SetCallId";

const char VS_AudioCapture::_RegDevName[]="DeviceName";
const char VS_AudioCapture::_RegVolume[]="Volume";
#define func_UseXPAec "UseXPAec"
#define func_EnableAGC "EnableAGC"
#define func_CodecQuality "CodecQuality"
/**
 **************************************************************************
 ****************************************************************************/
VS_AudioCapture::VS_AudioCapture(CVSInterface* pParentInterface,CAudioCaptureList *pCaptureList,VS_MediaFormat *pvmf) :
CVSInterface("AudioCapture", pParentInterface/*, NULL, true*/)
{
	m_pCaptureList=pCaptureList;
	_variant_t var = L"";
	ReadParam((char*)_RegDevName, &var);
	wcscpy(m_CurrenDeviceName, (_bstr_t)var);
	var=0xffff;
	ReadParam((char*)_RegVolume, &var);
	SetVolume(var);
	m_pmf = pvmf;
	var = 1; // enable by default
	ReadParam(func_EnableAGC, &var);
	m_EnableAgc = var.operator long()!=0;
	var = 2; // by default use Speex (=2)
	ReadParam(func_UseXPAec, &var);
	m_UseXPAec = var;
	m_eState = ADSTATE_STOP;
}

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioCapture::~VS_AudioCapture()
{
	_variant_t var;
	var = m_UseXPAec;
    WriteParam(func_UseXPAec, &var);
	var = m_EnableAgc;
    WriteParam(func_EnableAGC, &var);
	var = (_bstr_t)m_CurrenDeviceName;
    WriteParam((char*)_RegDevName, &var);
	var=GetVolume();
	WriteParam((char*)_RegVolume, &var);
}

/**
 **************************************************************************
 * See CVSInterface::ProcessFunction
 ****************************************************************************/
int VS_AudioCapture::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;
	if(strncmp(pSection,_funcACConnect,sizeof(_funcACConnect))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			if (var->vt==VT_NULL)
				*var = L"";
			g_DevStatus.SetStatus(DVS_SND_NOTCHOSEN, true, _wcsicmp((_bstr_t)*var, L"none")==0);
			WriteParam((char*)_RegDevName, var);
			if (m_eState != ADSTATE_START) {
				*var = iConnectDevice((_bstr_t)*var);
				g_DevStatus.SetStatus(DVS_SND_NOTWORK, true, var->lVal != 0);
			} else {
				wcscpy(m_CurrenDeviceName, (_bstr_t)*var);
				ReInit(&m_currentFmt);
				*var = 0;
				Start();
				g_DevStatus.SetStatus(DVS_SND_NOTWORK, true, var->lVal != 0);
			}
			return VS_INTERFACE_OK;
		}
	}
	else if(strncmp(pSection,_funcACDisconnect,sizeof(_funcACDisconnect))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			Release();
			return VS_INTERFACE_OK;
		}
	}
	else if (strncmp(pSection,_funcCaptureCurrent,sizeof(_funcCaptureCurrent))==0){
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			*var=(_bstr_t)m_CurrenDeviceName;
			return VS_INTERFACE_OK;
		}
	}
	else if(strncmp(pSection,_funcStop,sizeof(_funcStop))==0) {
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			Pause();
			m_eState = ADSTATE_STOP;
			return VS_INTERFACE_OK;
		}
	}
	else if(strncmp(pSection,_funcStart,sizeof(_funcStart))==0) {
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			if((int)*var==0)
				ReInit(m_pmf);
			else
				ReInit((VS_MediaFormat *)(int)*var);
			Start();
			m_eState = ADSTATE_START;

			return VS_INTERFACE_OK;
		}
	}
	else if(strncmp(pSection,_funcMicLevel,sizeof(_funcMicLevel))==0) {
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			*var=m_Level;
			DTRACE(VSTM_AUDI0, "m_Level : %d\n", m_Level);
			return VS_INTERFACE_OK;
		case RUN_COMMAND:
			return VS_INTERFACE_OK;
		}
	}
	else if(strncmp(pSection,_funcMicVolume,sizeof(_funcMicVolume))==0){
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			*var=GetVolume();
			return VS_INTERFACE_OK;
		case SET_PARAM:
			SetVolume(*var);
			return VS_INTERFACE_OK;
		}
	}
	else if(strcmp(pSection, func_UseXPAec)==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			*var = 1;
			return VS_INTERFACE_OK;
		case GET_PARAM:
			*var = VS_AudioDeviceManager::m_UseXPAec;
			return VS_INTERFACE_OK;
		case SET_PARAM:
			VS_AudioDeviceManager::m_UseXPAec = *var;
			return VS_INTERFACE_OK;
		}
	}
	else if(strcmp(pSection, func_EnableAGC)==0) {
		if (VS_OPERATION == GET_PARAM)
			*var = VS_AudioDeviceManager::m_EnableAgc;
		else if (VS_OPERATION == SET_PARAM)
		{
			VS_AudioDeviceManager::m_EnableAgc = *var;

			m_AudioProcessing.EnableFilter(filterType::GainControl_Filter, VS_AudioDeviceManager::m_EnableAgc);
		}
		return VS_INTERFACE_OK;
	}
	else if(strcmp(pSection, func_CodecQuality)==0) {
		if (VS_OPERATION == SET_PARAM)
			if (m_codec) m_codec->SetQuality((int)*var);
		return VS_INTERFACE_OK;
	}
	else if(strcmp(pSection, _funcSetCallId)==0) {
		if (VS_OPERATION == SET_PARAM)
			strncpy(m_CallId, (char*)(_bstr_t)*var, MAX_PATH);
		return VS_INTERFACE_OK;
	}
	else if(strcmp(pSection, _funcPitchFiltr)==0) {
		if (VS_OPERATION == SET_PARAM)
		{
			int value =(int)*var;
			if (value==0) m_VoiceChanger.Init(1.0);
			if (value==1) m_VoiceChanger.Init(1.5);
			if (value==2) m_VoiceChanger.Init(0.85);
		}
		if (VS_OPERATION == GET_PARAM)
		{
			double value=m_VoiceChanger.GetRatio();
			if(value==1)
				*var=0;
			if(value>1)
				*var=1;
			if(value<1)
				*var=2;
		}
		return VS_INTERFACE_OK;
	}
	return VS_INTERFACE_NO_FUNCTION;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioCapture::ReInit(VS_MediaFormat *pmf)
{
	m_currentFmt = *pmf;
	int device_number = m_pCaptureList->iGetDeviceNumberByName(m_CurrenDeviceName);
	Init(device_number, pmf);
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioCapture::ReInitDevice()
{
	int device_number = m_pCaptureList->iGetDeviceNumberByName(m_CurrenDeviceName);
	ReStart(device_number);
}

/**
 **************************************************************************
 * \param	device_number  [in] device number (identifier)
 * \return  0 if all ok, \n
 *         -1 no device with this number \n
 *         -2 incompatible device
 ****************************************************************************/
int VS_AudioCapture::iGetDeviceModeList(int device_number)
{
	WAVEINCAPS wc;
	if (waveInGetDevCaps(device_number, &wc, sizeof(WAVEINCAPS))!=MMSYSERR_NOERROR
		|| !(wc.dwFormats & (WAVE_FORMAT_1M16 | WAVE_FORMAT_2M16) ))
		return -2;
	else
		return 0;
}

int VS_AudioCapture::iConnectDevice(wchar_t *szName)
{
	wcscpy(m_CurrenDeviceName, szName);
	int device_number = m_pCaptureList->iGetDeviceNumberByName(m_CurrenDeviceName);
	if (Init(device_number, m_pmf))
		return 0;
	else
		return -1;
}


/****************************************************************************
 * VS_AudioRender
 ****************************************************************************/
const char VS_AudioRender::_funcVolume[]="Volume";
const char VS_AudioRender::_funcAudioQuality[]="AudioQuality";

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioRender::VS_AudioRender(CVSInterface* pParentInterface):
CVSInterface("RenderAudio",pParentInterface)
{
}


/**
 **************************************************************************
 * See CVSInterface::ProcessFunction
 ****************************************************************************/
int VS_AudioRender::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;
	if(strncmp(pSection,_funcVolume,sizeof(_funcVolume))==0){
		switch(VS_OPERATION)
		{
		case SET_PARAM:
			SetVolume(int(*var));
			return VS_INTERFACE_OK;
		}
	}
	else if(strncmp(pSection,_funcAudioQuality,sizeof(_funcAudioQuality))==0) {
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			*var=2;
			return VS_INTERFACE_OK;
		}
	}
	return VS_INTERFACE_NO_FUNCTION;
}
