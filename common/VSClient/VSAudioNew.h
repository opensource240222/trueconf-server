/**
 **************************************************************************
 * \file VSAudioNew.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Contain classes working with audio: capturing, rendering, compressing
 *
 * \b Project Client
 * \author SMirnovK
 * \date 19.11.2004
 *
 * $Revision: 20 $
 *
 * $History: VSAudioNew.h $
 *
 * *****************  Version 20  *****************
 * User: Sanufriev    Date: 1.11.11    Time: 16:20
 * Updated in $/VSNA/VSClient
 * - new DS render implementation (for many streams - muxer + 1 DSOut
 * device + 1 aec)
 * - change method of detect max sample rate value (from ds guid)
 * - change hardware test,  TestAudio
 * - some refactoring audio + delete DMO audio
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 18.08.10   Time: 14:09
 * Updated in $/VSNA/VSClient
 * - vad corrected
 * - alfa Native freq
 *
 * *****************  Version 18  *****************
 * User: Sanufriev    Date: 29.06.10   Time: 18:15
 * Updated in $/VSNA/VSClient
 * - ench 7471 (prepare avi writer)
 *
 * *****************  Version 17  *****************
 * User: Sanufriev    Date: 16.04.10   Time: 14:53
 * Updated in $/VSNA/VSClient
 * - were enhancement aec (increase resampling precision, statistic queue
 * lenght, upper bandwith for skip)
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 8.04.10    Time: 16:29
 * Updated in $/VSNA/VSClient
 * - fix hardware test for AEC DMO
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 22.03.10   Time: 15:07
 * Updated in $/VSNA/VSClient
 * - agc with overloads counter
 *
 * *****************  Version 14  *****************
 * User: Sanufriev    Date: 24.02.10   Time: 13:24
 * Updated in $/VSNA/VSClient
 * - were added DMO AEC (temporary disabled)
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 10.12.09   Time: 19:04
 * Updated in $/VSNA/VSClient
 * - unicode capability for hardware lists
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 27.10.09   Time: 16:56
 * Updated in $/VSNA/VSClient
 * - aec, bugfix #6565
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 24.09.09   Time: 14:07
 * Updated in $/VSNA/VSClient
 * - fix aec crash. increase maximum number of aec chanel
 * (MAX_NUM_ECHO_CHANNEL = 12).
 * - new agc
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 4.08.09    Time: 20:28
 * Updated in $/VSNA/VSClient
 * - fix DS for avi player
 * - add AudioCodecSystem class, change acmDriverEnumCallback function
 * - avi player support system audio codecs
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 3.06.09    Time: 11:52
 * Updated in $/VSNA/VSClient
 * - audiorender cleanup
 *
 * *****************  Version 8  *****************
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
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 9.02.09    Time: 9:54
 * Updated in $/VSNA/VSClient
 * - were improved speex aec
 * - were added audio devices frequency calculate
 * - were added speex resample in echo module
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 25.09.08   Time: 15:27
 * Updated in $/VSNA/VSClient
 * - were fixed GetBufferedDurr() (decrease probability of threads
 * conflict on multi-core cpu)
 * - were changed jitter algorithm
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 9.07.08    Time: 13:24
 * Updated in $/VSNA/VSClient
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 9.07.08    Time: 13:08
 * Updated in $/VSNA/VSClient
 * - were modified audio render algorithm
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
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 24.12.07   Time: 17:37
 * Updated in $/VS2005/VSClient
 * - master volume added
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
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 25.01.07   Time: 14:07
 * Updated in $/VS/VSClient
 * - avi play first errors corrected, some impruvements
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 24.01.07   Time: 19:41
 * Updated in $/VS/VSClient
 * - play avi module
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 17.01.07   Time: 12:33
 * Updated in $/VS/VSClient
 * - for DirectSound devices NoiseGen restored to "always"
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 29.09.06   Time: 14:02
 * Updated in $/VS/VSClient
 * - programm audio mute
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 25.07.06   Time: 18:09
 * Updated in $/VS/VSClient
 * - Added system waveout Mute
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 13.07.06   Time: 18:56
 * Updated in $/VS/vsclient
 * - new audio controls (via mixer)
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 20.04.06   Time: 13:46
 * Updated in $/VS/VSClient
 * - new audio hardware test
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 11.04.06   Time: 16:37
 * Updated in $/VS/VSClient
 * - console hrdware test
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 5.04.06    Time: 17:24
 * Updated in $/VS/VSClient
 * - low-level audio devices
 * - Direct Sound devices added
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 9.03.06    Time: 17:38
 * Updated in $/VS/VSClient
 * - AGC implemented
 * - "long latency" audiorender mode
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 10.08.05   Time: 19:23
 * Updated in $/VS/VSClient
 * - strict audiobufferisation parametr
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 10.08.05   Time: 13:33
 * Updated in $/VS/VSClient
 * - new low latensy and noise generation schema
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 1.03.05    Time: 13:25
 * Updated in $/VS/vsclient
 * bug(568) on hiperthreading with audio capture fixed
 *
 * *****************  Version 10  *****************
 * User: Melechko     Date: 24.02.05   Time: 12:39
 * Updated in $/VS/VSClient
 * Add CaptureSlotExt
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 22.02.05   Time: 18:22
 * Updated in $/VS/VSClient
 * low latency switch
 * bitrate degradation more accurate
 * Neigl for streams only for local conf
 * fix with Localalloc for otherId
 *
 * *****************  Version 8  *****************
 * User: Melechko     Date: 20.01.05   Time: 15:40
 * Updated in $/VS/VSClient
 * Microphone level function restore
 *
 * *****************  Version 7  *****************
 * User: Melechko     Date: 19.01.05   Time: 18:01
 * Updated in $/VS/VSClient
 * Add re-init audio format
 *
 * *****************  Version 6  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:02
 * Updated in $/VS/VSClient
 * some changes :)
 *
 * *****************  Version 5  *****************
 * User: Admin        Date: 16.12.04   Time: 20:08
 * Updated in $/VS/VSClient
 * doxigen comments
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 8.12.04    Time: 20:26
 * Updated in $/VS/VSClient
 * new video-audio sinc
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

#ifndef VS_AUDIO_NEW_H
#define VS_AUDIO_NEW_H


/****************************************************************************
 * Includes
 ****************************************************************************/
#include <windows.h>
#include <mmreg.h>
#include "VSClientBase.h"
#include "VSAudioDs.h"
#include "VSAudioUtil.h"
#include "VS_VolControl.h"
#include "../std/cpplib/VS_Lock.h"
#include "../std/cpplib/VS_MediaFormat.h"
#include "../Audio/VoiceActivity/SpecialDspFunctions.h"


/****************************************************************************
 * Declaration
 ****************************************************************************/
class AudioCodec;
class VSAudioVAD;
class VS_FifoBuffer;
class CAudioCaptureList;
class VS_AgcWebrtc;
class VS_RtcNoiseSuppression;
class VS_Agc;
/****************************************************************************
 * Classes
 ****************************************************************************/

/**
 **************************************************************************
 * \brief Contain common members for both types of audiodevice
 ****************************************************************************/
class VS_AudioDeviceBase
{
	int					m_Volume;		///< Current audio scalefactor [0..ffff]
protected:
	int					m_BuffLen;		///< Length of single audiodata chunk
	WAVEFORMATEX		m_wf;			///< current audio format
	char *				m_AudioTemp;	///< trmp buffer
	AudioCodec*			m_codec;		///< pointer to audio coder/decoder
	int					m_Level;		///< audio level
	HANDLE				m_hComplEv;		///< Device Event
	bool				m_IsValid;		///< true if Init() passed ok
	char				m_CallId[MAX_PATH];
protected:
	/// Normalise volume by m_Volume, calculate audio level
	void CalcLevel(short* in, int samples);
public:
	/// Set to zero members
	VS_AudioDeviceBase();
	/// release resourcec
	~VS_AudioDeviceBase();
	/// Set audio format
	void SetWaveFormat(VS_MediaFormat *mf);
	/// Get audio level
	int GetLevel(){return m_Level;}
	/// Get current volume scalefactor
	int GetVolume(){return m_Volume;}
	/// Set current volume csalefactor
	void SetVolume(int Volume){m_Volume = Volume>0xffff ? 0xffff : Volume;}
	/// set by device when next buffer complete
	HANDLE GetCmpleteEvent() {return m_hComplEv;}
	/// return validation state
	bool IsValid(){return m_IsValid;}
	/// return internal lenth of data chunk
	int GetChunkLen() {return m_BuffLen;}
	/// rerurn pointer to WAVEFORMAT
	WAVEFORMATEX* GetWF(){return &m_wf;}
	/// set call id
	void SetCallId(char* CallId);
};


/**
 **************************************************************************
 * \brief Windows Multimedia Waveform audio render device.
 ****************************************************************************/
class VS_ARenderDevice:
	public VS_AudioDeviceBase,
	public VS_AudioDeviceManager
{
	enum frame_type{
		FT_NULL,
		FT_PLAY,
		FT_SKIP,
		FT_NOISE
	};
	int					m_PlayedBuffs;	///< number of played buffers
	int					m_NoiseTime;	///< last time when noise passed to devise
	int					m_SkipTime;		///< last time when data can but not passed to devise
	int					m_LastFillTime;	///< last time when data passed to devise
	int					m_LastFillNoise;
	VSAudioVAD*			m_vad;			///< pointer to VAD class
 	VS_ARenderAnalyse*  m_ajitter;	    ///< pointer to time analiser
	VS_FifoBuffer*		m_fifo;			///< pointer to fifo buffer
	int					m_LastSample;	///< last sample passed to device
	frame_type			m_LastFrameType;///< last audio frame type
	int					m_FramesToSkip; ///< number of frames to skip in current time window
	int					m_start_time_ar;
	int					m_time_skip_delta;
	int					m_noise_lenght;
	int					m_StrictBuffLen;///< Strict buffer length
public:
	static int			m_muteAll;
	/// Set to zero members
	VS_ARenderDevice();
	/// call Release()
	virtual ~VS_ARenderDevice();
	/// Init members according to VS_MediaFormat
	bool Init(int dev, VS_MediaFormat *mf);
	/// Play compressed data
	int Play(char* buff, int size);
	/// Get Current Buffer Duration
	int GetCurrBuffDurr();
	/// Call to check if noise is need
	void CheckNoiseInsert();
	/// Release resources
	void Release();
	/// return last fill time
	int GetLastFillTime() {return m_LastFillTime;}
	//
	void CalcFrequency(int buff_durr);
	/// Set mimum buff durration threshold in mc
	void SetStrictDurr(int durr) {m_StrictBuffLen = durr;}
	//
	VS_AudioDevice_Type Type(){return (m_device) ? m_device->Type() : ADTYPE_COMMON;}
	//
	bool GetBuffBounds(int &a, int &b);
private:
	/// Correct first audio samples in case of nonsequential write
	void CorrectSpike(short* data, int lenght, frame_type type);
};

/**
 **************************************************************************
 * \brief Windows Multimedia Waveform audio render device to plaq PCM
 ****************************************************************************/
class VS_PCMAudioRender:
	public VS_AudioDeviceBase,
	public VS_AudioDeviceManager
{
	VS_FifoBuffer*		m_fifo;			///< pointer to fifo buffer
public:
	/// Set to zero members
	VS_PCMAudioRender();
	/// call Release()
	~VS_PCMAudioRender();
	/// Init members according to VS_MediaFormat
	bool Init(int dev, WAVEFORMATEX *wf);
	/// Release resources
	void Release();
	/// Play data
	int Play(char* buff, int size);
	/// Start Playing
	void Start();
	/// Stop Playing
	void Stop();
	/// Get Current Buffer Duration
	int GetCurrBuffDurr();
};


class VS_AudioMixerVolume
{
public:
	enum Mix_Type {
		MTYPE_MIC,
		MTYPE_WOUT,
		MTYPE_MASTER
	};
private:
	HMIXER				m_mixer;
	MIXERCONTROL*		m_foundControl;
	MIXERCONTROLDETAILS m_mcd;
	MIXERCONTROLDETAILS_UNSIGNED *m_control;
	DWORD				m_channels;
	DWORD				m_Volume;
	DWORD				m_maxrange;
	DWORD				m_minvol;
	DWORD				m_maxvol;
	Mix_Type			m_type;
	static HWND			m_hwnd;
	static void*		m_MicMix;
	static void*		m_WOutMix;
public:
	VS_AudioMixerVolume();
	~VS_AudioMixerVolume();
	bool Init(int device, Mix_Type type);
	void Release();
	bool SetVolume(DWORD val);
	DWORD GetVolume();
	DWORD GetVolumeFast() {return m_Volume;}
	bool IsValid() {return m_foundControl!=0;}
	static void SetWnd(HWND hwnd) {m_hwnd = hwnd;}
	static HWND GetWnd() {return m_hwnd;}
};


/**
 **************************************************************************
 * \brief Windows Multimedia Waveform audio capture device.
 ****************************************************************************/
class VS_ACaptureDevice:
	public VS_AudioDeviceBase,
	private VS_Lock,
	public VS_AudioDeviceManager

{
	VSAudioVAD*				m_vad;			///< pointer to VAD class
	VS_VolControlBase		*m_VolContr;
public:

	/// Set to zero members
	VS_ACaptureDevice();
	/// call Release()
	~VS_ACaptureDevice();
	/// Init members according to VS_MediaFormat
	bool Init(int dev, VS_MediaFormat *mf);
	/// Return compressed audio data
	int Capture(char* buff, int &size, bool use_audio);
	/// Release resources
	void Release();
	/// Save captyre volume level to register
	void SaveVolumeToRegister();
	/// Load captyre volume level from register
	void LoadVolumeFromRegister();
	/// Pause untill Start() will be called
	void Pause();
	/// Continue normal work after pause
	void Start();
	/// Restart device
	void ReStart(int dev);
	/// Get current volume scalefactor
	int GetVolume();
	/// Set current volume csalefactor
	void SetVolume(int Volume);
	//
	void CalcFrequency(int buff_durr);
	//

	VS_AudioDevice_Type Type(){return (m_device) ? m_device->Type() : ADTYPE_COMMON;}

protected:
	friend class VSTestAudio;
	wchar_t m_CurrenDeviceName[MAX_PATH];	///< Current Device name
	VoiceChanger			m_VoiceChanger;

};



////////////////////////////////////////////////////////////////////////////////
// Interfaces
////////////////////////////////////////////////////////////////////////////////
/**
 **************************************************************************
 * \brief Class - interface to audio capture device
 ****************************************************************************/
class VS_AudioCapture:
	public CVSInterface,
	public VS_ACaptureDevice
{
public:
	const static char _funcMicLevel[];		///< Microphone Level
	const static char _funcMicVolume[];		///< Microphone Volume
	const static char _funcCaptureCurrent[];///< Current Capture name
	const static char _funcACConnect[];
	const static char _funcACDisconnect[];
	const static char _funcStart[];         ///< Start capture
	const static char _funcStop[];          ///< Stop capture
	const static char _funcSetCallId[];     ///< Set CallId
	const static char _RegDevName[];		///< Current Device name registry value name
    const static char _RegVolume[];		    ///< Current volume registry value name
	const static char _funcPitchFiltr[];


    VS_MediaFormat		*m_pmf;
	VS_MediaFormat		m_currentFmt;
	CAudioCaptureList	*m_pCaptureList;
	VS_AudioDevice_State m_eState;
	/// set/get operations, see static members
	int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;
	/// check if device can operate
	int iGetDeviceModeList(int device_number);
public:
	/// set name in parent CVSInterface, read registry
	VS_AudioCapture(CVSInterface* pParentInterface,CAudioCaptureList *pCaptureList,VS_MediaFormat *pvmf);
	/// write to  registry
	~VS_AudioCapture();
	int iConnectDevice(wchar_t *szName);
	void ReInit(VS_MediaFormat *pmf);
	void ReInitDevice();
	virtual bool IsInterfaceSupport(VS_INTERFACE_TYPE TypeChecked){return TypeChecked==IT_AUDIOCAPTURE;};
};


/**
 **************************************************************************
 * \brief Class - interface to audio render device
 ****************************************************************************/
class VS_AudioRender:
	public CVSInterface,
	public VS_ARenderDevice
{
	const static char _funcVolume[];		///< Volume
	const static char _funcAudioQuality[];	///< Audio Quality
	const static char _funcSetCallId[];     ///< Set CallId

	/// set/get operations, see static members
	int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;
public:
	/// set name in parent CVSInterface
	VS_AudioRender(CVSInterface* pParentInterface);
	virtual ~VS_AudioRender() {};
};

#endif
