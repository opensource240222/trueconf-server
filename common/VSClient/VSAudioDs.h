/**
 **************************************************************************
 * \file VSAudioDs.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Contain intrfaces to audio device classes, and low-level
 *        device manager
 *
 * \b Project Client
 * \author SMirnovK
 * \date 15.03.2006
 *
 * $Revision: 18 $
 *
 * $History: VSAudioDs.h $
 *
 * *****************  Version 18  *****************
 * User: Sanufriev    Date: 1.11.11    Time: 16:20
 * Updated in $/VSNA/VSClient
 * - new DS render implementation (for many streams - muxer + 1 DSOut
 * device + 1 aec)
 * - change method of detect max sample rate value (from ds guid)
 * - change hardware test,  TestAudio
 * - some refactoring audio + delete DMO audio
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 24.09.10   Time: 22:11
 * Updated in $/VSNA/VSClient
 * - removed HiLoad++
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 15.09.10   Time: 16:23
 * Updated in $/VSNA/VSClient
 * - fix DS bug (overflow)
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 23.08.10   Time: 22:19
 * Updated in $/VSNA/VSClient
 * - long names in devices
 * - corrected Wide names for devices
 * - init devices section rewrited
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 12.08.10   Time: 20:08
 * Updated in $/VSNA/VSClient
 * - move interfaces into DS
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 16.04.10   Time: 14:53
 * Updated in $/VSNA/VSClient
 * - were enhancement aec (increase resampling precision, statistic queue
 * lenght, upper bandwith for skip)
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 6.04.10    Time: 18:04
 * Updated in $/VSNA/VSClient
 * - were enabled DMO AEC ("EnableDMO" in registry)
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 24.02.10   Time: 13:23
 * Updated in $/VSNA/VSClient
 * - were added DMO AEC (temporary disabled)
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 27.10.09   Time: 16:56
 * Updated in $/VSNA/VSClient
 * - aec, bugfix #6565
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 27.10.09   Time: 16:12
 * Updated in $/VSNA/VSClient
 * - fix reenumerate DS devices list
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 20.07.09   Time: 14:44
 * Updated in $/VSNA/VSClient
 * - debuging bug
 * - fix aec for Vista
 * - change directx version detect
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 11.06.09   Time: 19:31
 * Updated in $/VSNA/VSClient
 * - aec: audio did not send long time
 *
 * *****************  Version 6  *****************
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
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 1.04.09    Time: 19:23
 * Updated in $/VSNA/VSClient
 * - bugfix #5815
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 5.03.09    Time: 14:59
 * Updated in $/VSNA/VSClient
 * - were added support multi-cnnel aec in VS_GlobalEcho &
 * VS_SpeexEchoCancel
 * - were removed skip-frames for first calculation audio render frequency
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 9.02.09    Time: 9:54
 * Updated in $/VSNA/VSClient
 * - were improved speex aec
 * - were added audio devices frequency calculate
 * - were added speex resample in echo module
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
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
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 15.01.07   Time: 11:33
 * Updated in $/VS/VSClient
 * - XP EAC while conference only
 * - intrface to turn it on added
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 5.04.06    Time: 17:24
 * Created in $/VS/VSClient
 * - low-level audio devices
 * - Direct Sound devices added
 *
 ****************************************************************************/
#ifndef VS_AUDIO_DS_H
#define VS_AUDIO_DS_H

/****************************************************************************
 * Includes
 ****************************************************************************/
#include <mmreg.h>

#include "AudioProcessing/VS_AudioProcessing.h"

/****************************************************************************
 * Enums
 ****************************************************************************/
/// Audio device state
enum VS_AudioDevice_State
{
	ADSTATE_INIT,
	ADSTATE_START,
	ADSTATE_STOP
};

/// Audio device type
enum VS_AudioDevice_Type
{
	ADTYPE_COMMON,
	ADTYPE_WAVEIN,
	ADTYPE_WAVEOUT,
	ADTYPE_FDIN,
	ADTYPE_FDOUT,
	ADTYPE_DSIN,
	ADTYPE_DSOUT,
	ADTYPE_FDMUXOUT
};

///
class CStringList;

/****************************************************************************
 * Classes
 ****************************************************************************/

/**
 **************************************************************************
 * \brief Describe audiodevice and it init parametrs
 ****************************************************************************/
class VS_AudioDeviceDesc
{
public:
	int						dev;		///< device number in list of devices
	WAVEFORMATEX			wf;			///< wave format of audiodata
	void *					Event;		///< notification event of incoming/outcoming data
	int						Num;		///< number of notifications per buffer
	int						Len;		///< length of single data chunck, that write/read from device
	int						IsCapt;		///< type of device, capture or not
	bool					IsVista;	///< type OS System
	/// Clean all members
	VS_AudioDeviceDesc() {Clean();}
	/// Clean all members
	void Clean() {
		memset(this, 0, sizeof(VS_AudioDeviceDesc));
	}
	/// Check description is valid
	bool IsValid() {
		bool noValid = dev < 0 || Len == 0 || Num == 0 || Event == 0;
		return !noValid;
	}
	/// compare two descriptions
	bool operator!=(VS_AudioDeviceDesc& src){
		if (this==0 || &src == 0) return false;
		else if (this==&src) return false;
		else return memcmp(this, &src, sizeof(VS_AudioDeviceDesc))!=0;
	}
};

/**
 **************************************************************************
 * \brief Device frequency statistic
 ****************************************************************************/
class VS_FreqDeviation;

struct VS_FrequencyStat
{
	bool				bValid;
	__int64				start_samples;
	int					start_pos_buffer;
	LARGE_INTEGER		freq, in_time;
	int					prev_buff_durr;
	double				dt_prev, dt_calc;
	bool				is_first;
	double				fFrequency;
	double				fFrequencyInit;
	double				fDeltaFrequency;
	int					iCalcRange;
	VS_FreqDeviation	*pFreqDeviation;
	void Init(int frequency, double dfreq, int dt)
	{
		bValid = false;
		start_samples = 0;
		start_pos_buffer = 0;
		QueryPerformanceFrequency(&freq);
		in_time.HighPart = 0;
		in_time.LowPart = 0;
		prev_buff_durr = 0;
		dt_prev = 0.0;
		dt_calc = 0.0;
		is_first = false;
		iCalcRange = dt;
		fFrequency = (double)frequency;
		fFrequencyInit = fFrequency;
		fDeltaFrequency = dfreq;
	};
	void Reset()
	{
		start_samples = 0;
		start_pos_buffer = 0;
		QueryPerformanceFrequency(&freq);
		in_time.HighPart = 0;
		in_time.LowPart = 0;
		prev_buff_durr = 0;
		dt_prev = 0.0;
		dt_calc = 0.0;
		is_first = false;
		fFrequency = fFrequencyInit;
	};
};

/**
 **************************************************************************
 * \brief Base interface to audiodevice
 ****************************************************************************/

class VS_AudioDeviceCommon
{
protected:
	bool					m_IsValid;	///< true if device ready to work
	VS_AudioDevice_State	m_state;	///< state of device
	VS_AudioDevice_Type		m_dtype;	///< type of derived class
	__int64					m_Samples;
public:
	/// set members
	VS_AudioDeviceCommon() : m_IsValid(false), m_state(ADSTATE_INIT), m_dtype(ADTYPE_COMMON) {}
	/// used to right delete of derivrd classes
	virtual ~VS_AudioDeviceCommon(){}
	/// Start device
	virtual void Start() {m_state = ADSTATE_START;}
	/// Stop device
	virtual void Stop() {m_state = ADSTATE_STOP;}
	/// Get Durration of queued data in device at this time
	virtual int GetBufferedDurr(bool is_precise = false) {return 0;}
	virtual int GetBufferedDurr(int& offset) {return 0;}
	/// Write to Out device
	virtual bool OutWrite(short* data, int len){return false;}
	/// Write noise to Out device
	virtual bool OutWriteNoise(short* data, int PendBytes){return false;}
	/// Read from In device
	virtual bool InRead(short* data, int &size){return false;}
	/// Get samples number for device
	virtual __int64 GetNumSamples() {return m_Samples;}
	/// Restart audio device
	virtual bool IsRestartDevice() {return false;}
	/// return device validity state
	bool IsValid(){return m_IsValid;}
	/// return device state
	VS_AudioDevice_State State() { return m_state;}
	/// return device type
	VS_AudioDevice_Type Type() {return m_dtype;}
};

/**
 **************************************************************************
 * \brief Manage VS_AudioDeviceCommon instances, must be base for classes
 * wanted request audio device
 ****************************************************************************/
class VS_AudioDeviceManager
{
	static HWND						m_hwnd;			///< used for DirectSound
	static VS_AudioDeviceDesc		m_RendDesc;		///< render device description
	static VS_AudioDeviceDesc 		m_CaptDesc;		///< cupture device description
	static VS_AudioDeviceDesc 		m_CurrCaptDesc; ///< current capture device
	static VS_AudioDeviceManager*	m_RendDevice;	///< pointer to first requested render device
	static VS_AudioDeviceManager*	m_CaptDevice;	///< pointer to first requested capture device

protected:
	static long						m_UseXPAec;		///< turn On AEC
	static long						m_DevicesMode;	///< device mode (Wave, DS, Replace Audio Muxer...)
	static bool						m_bVista;
	static bool						m_EnableAgc;	///< enable auto gain control (agc)
	static bool						m_bEqualDevice;
	VS_AudioDeviceCommon*			m_device;		///< pointer to requsted audiodevice used by derived class
	VS_FrequencyStat				m_FrequencyStat;

public:
	static VS_AudioProcessing		m_AudioProcessing;

	int RegularDevFlag;
	/// Init audio device manager, handle must exist all process living time
	static void Open(HWND hwnd);
	/// Close audio device manager
	static void Close();
	/// Renew audio devices list
	static int GetDeviceList(bool is_capture, CStringList *list);
	/// Get Maximum value sample rate supported
	static int GetMaxSampleRate(int dev, bool is_capture);
	///
	static bool IsEqualDevice() { return m_bEqualDevice; }

protected:
	/// set to zero m_device
	VS_AudioDeviceManager() : m_device(0) {RegularDevFlag=0; memset(&m_FrequencyStat, 0, sizeof(VS_FrequencyStat)); }
	/// Call it to obtain m_device
	void QueryDevice(VS_AudioDeviceDesc &desc);
	/// Release m_device
	void ReleaseDevice();
	/// Render CallBack
	void PreRenderCallBack(short* data, unsigned long samples);
	/// Render CallBack
	void PostRenderCallBack(short* data, unsigned long samples);
	/// Capture CallBack
	void CaptureCallBack(short* data, unsigned long samples);
	/// Get real device frequency
	double GetFrequency(bool bEqualDevice) { return bEqualDevice ? m_FrequencyStat.fFrequencyInit : m_FrequencyStat.fFrequency; }
	/// Calculate real device frequency
	virtual void CalcFrequency(int buff_durr) {};

};

#endif /*VS_AUDIO_DS_H*/
