/**
 **************************************************************************
 * \file VSAudioDs.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief low-level audio device implementations
 *
 * \b Project Client
 * \author SMirnovK
 * \date 20.03.2006
 *
 * $Revision: 28 $
 *
 * $History: VSAudioDs.cpp $
 *
 * *****************  Version 28  *****************
 * User: Sanufriev    Date: 1.11.11    Time: 16:20
 * Updated in $/VSNA/VSClient
 * - new DS render implementation (for many streams - muxer + 1 DSOut
 * device + 1 aec)
 * - change method of detect max sample rate value (from ds guid)
 * - change hardware test,  TestAudio
 * - some refactoring audio + delete DMO audio
 *
 * *****************  Version 27  *****************
 * User: Sanufriev    Date: 29.04.11   Time: 17:09
 * Updated in $/VSNA/VSClient
 * - fix AviWriter : stream sunch
 *
 * *****************  Version 26  *****************
 * User: Smirnov      Date: 24.09.10   Time: 22:11
 * Updated in $/VSNA/VSClient
 * - removed HiLoad++
 *
 * *****************  Version 25  *****************
 * User: Sanufriev    Date: 15.09.10   Time: 16:23
 * Updated in $/VSNA/VSClient
 * - fix DS bug (overflow)
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 3.09.10    Time: 16:54
 * Updated in $/VSNA/VSClient
 * -  not native DS for Vista
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 2.09.10    Time: 20:39
 * Updated in $/VSNA/VSClient
 * - native freq for DirectDound iin XP
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 25.08.10   Time: 19:26
 * Updated in $/VSNA/VSClient
 * - some optimization in interfaces
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 23.08.10   Time: 22:19
 * Updated in $/VSNA/VSClient
 * - long names in devices
 * - corrected Wide names for devices
 * - init devices section rewrited
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 18.08.10   Time: 19:42
 * Updated in $/VSNA/VSClient
 * - returned old agc
 * - old devices
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 18.08.10   Time: 14:09
 * Updated in $/VSNA/VSClient
 * - vad corrected
 * - alfa Native freq
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 12.08.10   Time: 20:08
 * Updated in $/VSNA/VSClient
 * - move interfaces into DS
 *
 * *****************  Version 17  *****************
 * User: Sanufriev    Date: 6.04.10    Time: 18:04
 * Updated in $/VSNA/VSClient
 * - were enabled DMO AEC ("EnableDMO" in registry)
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 24.02.10   Time: 13:23
 * Updated in $/VSNA/VSClient
 * - were added DMO AEC (temporary disabled)
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 25.11.09   Time: 21:13
 * Updated in $/VSNA/VSClient
 * - bugfix in dsound with call while playing audio
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 27.10.09   Time: 16:56
 * Updated in $/VSNA/VSClient
 * - aec, bugfix #6565
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 27.10.09   Time: 16:12
 * Updated in $/VSNA/VSClient
 * - fix reenumerate DS devices list
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 20.07.09   Time: 14:44
 * Updated in $/VSNA/VSClient
 * - debuging bug
 * - fix aec for Vista
 * - change directx version detect
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 29.05.09   Time: 20:02
 * Updated in $/VSNA/VSClient
 * - DirectSound works fine now
 * - last modifications for echo cancelling
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
 * User: Sanufriev    Date: 25.09.08   Time: 15:27
 * Updated in $/VSNA/VSClient
 * - were fixed GetBufferedDurr() (decrease probability of threads
 * conflict on multi-core cpu)
 * - were changed jitter algorithm
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 10.06.08   Time: 15:07
 * Updated in $/VSNA/VSClient
 * - Speex echo cancellation by default
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
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 20.12.07   Time: 16:14
 * Updated in $/VS2005/VSClient
 * - added software AEC
 * - Speex AEC improved
 * - Audio MediaFormat changed to commit Speex preprocess
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 6.09.07    Time: 16:53
 * Updated in $/VS2005/VSClient
 * - bugfix #3218
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 30.08.07   Time: 17:55
 * Updated in $/VS2005/VSClient
 * - bugfix #3200
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 22.08.07   Time: 16:04
 * Updated in $/VS2005/VSClient
 * - set correct states for WaveOut
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 7.06.07    Time: 19:05
 * Updated in $/VS2005/VSClient
 * - set/get position in avi (playing mode)
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 24.01.07   Time: 19:41
 * Updated in $/VS/VSClient
 * - play avi module
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 15.01.07   Time: 11:33
 * Updated in $/VS/VSClient
 * - XP EAC while conference only
 * - intrface to turn it on added
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 15.11.06   Time: 18:05
 * Updated in $/VS/VSClient
 * - debug info
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 22.05.06   Time: 10:54
 * Updated in $/VS/VSClient
 * - comand queue as list
 * - sent frame queue as map
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 20.04.06   Time: 13:46
 * Updated in $/VS/VSClient
 * - new audio hardware test
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 5.04.06    Time: 17:24
 * Created in $/VS/VSClient
 * - low-level audio devices
 * - Direct Sound devices added
 *
 ****************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <mmsystem.h>
#include <dmo.h>
#include <objbase.h>
#include <mediaobj.h>
#include <uuids.h>
#include <wmcodecdsp.h>
#include <MMDeviceApi.h>
#include <dsound.h>
#include <math.h>
#include "VSClientBase.h"
#include "VSAudioDs.h"
#include "VSAudioEchoCancel.h"
#include "VSAudioUtil.h"
#include "VS_ApplicationInfo.h"
#include "VS_Dmodule.h"
#include  "VSAudioNew.h"
#include  "VSAudio.h"
#include "VS_PlayAvi.h"
#include "VS_GetAudioDevicesSampleRate.h"
#include "..\std\cpplib\VS_RegistryKey.h"
#include "std-generic/cpplib/VS_Container.h"
#include "..\std\cpplib\VS_Map.h"
#include "..\std\cpplib\VS_Utils.h"
#include "std-generic/cpplib/utf8.h"
#include "../std/cpplib/VS_Protocol.h"
#include "../Transcoder/VS_BitStreamBuff.h"
#include "../Transcoder/VS_AudioReSampler.h"
#include "Transcoder/VS_AudioMixer.h"
#include "VS_EchoDebugger.h"
#include "std/VS_FifoBuffer.h"

#include <functiondiscoverykeys.h> // PKEY_Device_FriendlyName
#include <Audioclient.h>

#include <boost/algorithm/string/predicate.hpp>

#include <map>
#include <vector>
#include <string>

//DirectSound
typedef  HRESULT (WINAPI * DIRECTSOUNDCREATE8)(LPCGUID, LPDIRECTSOUND8*, LPUNKNOWN );
typedef  HRESULT (WINAPI * DIRECTSOUNDCAPTURECREATE8)(LPCGUID, LPDIRECTSOUNDCAPTURE8*, LPUNKNOWN );
typedef	 HRESULT (WINAPI * DIRECTSOUNDENUMERATE)(LPDSENUMCALLBACKA , LPVOID );
typedef	 HRESULT (WINAPI * GETDEVICEID)(LPCGUID pGuidSrc, LPGUID pGuidDest);
/////////////////////////////////////////////////////////////////////////////
// Globals
/////////////////////////////////////////////////////////////////////////////
/// instance of DirectSound Dll
HINSTANCE g_hDsoundDLL = 0;
/// pointer to interface in DirectSound Dll
DIRECTSOUNDCREATE8				g_DirectSoundCreate8 = 0;
DIRECTSOUNDCAPTURECREATE8		g_DirectSoundCaptureCreate8 = 0;
DIRECTSOUNDENUMERATE			g_DirectSoundEnumerate = 0;
DIRECTSOUNDENUMERATE			g_DirectSoundCaptureEnumerate = 0;
GETDEVICEID						g_GetDeviceID = 0;

struct VS_AudioDeviceParams
{
	std::wstring	m_name;
	GUID		m_guid;
	std::wstring	m_drv;
	std::wstring	m_dev_interface;
	int			m_maxfreq;
};

typedef std::vector<VS_AudioDeviceParams> VS_DevD;
VS_DevD g_rnd;
VS_DevD g_cpt;

/**
 **************************************************************************
 * Called by DirectSound to enumerate devices
 * \param		pGUID	[in] Address of the GUID that identifies the
 *				DirectSound driver being enumerated
 * \param		strDesc	[in] Address of a null-terminated string that provides
 *				a textual description of the DirectSound device
 * \param		strDesc	[in] Address of application-defined data.
 * \return TRUE to continue enumerating drivers, or FALSE to stop
 ****************************************************************************/
INT_PTR CALLBACK DSoundEnumCallback( GUID* pGUID, LPCWSTR Desc, LPCWSTR DrvName, VOID* pContext)
{
	const GUID* pg = pGUID;
	wchar_t dev_interface[2*MAX_PATH] = {0};
	if (!pGUID)
		//pg = (pContext == 0) ? &DSDEVID_DefaultPlayback : &DSDEVID_DefaultCapture;
		return TRUE;

	VS_AudioDeviceParams adp;
	adp.m_name = Desc;
	adp.m_guid = *pg;
	adp.m_drv = DrvName;
	adp.m_maxfreq = VS_GetCorrectedDeviceGuidSampleRate(*pg, pContext != 0, dev_interface);
	adp.m_dev_interface = dev_interface;

	if (pContext == 0) g_rnd.push_back(adp);
	else g_cpt.push_back(adp);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Classes
/////////////////////////////////////////////////////////////////////////////
/**
 **************************************************************************
 * \brief Contain common members for wave devices
 ****************************************************************************/
class VS_AudioWaveDevice
{
public:
	WAVEHDR*			m_wh;				///< waveform-audio buffers
	void *				m_Memory;			///< common pointer for all audio buffers
	DWORD				m_lastDeviceSamples;///< last number of samples reported by device
	int					m_StartBuff;		///< round-robin pointer to buffers
	VS_AudioDeviceDesc	m_aDD;				///< audio device description
};

/**
 **************************************************************************
 * \brief waveOut device
 ****************************************************************************/
class VS_AudioWaveOut: public VS_AudioDeviceCommon, public VS_AudioWaveDevice
{
	HWAVEOUT			m_hw;				///< waveout device handle
	void Release();
public:
	VS_AudioWaveOut(VS_AudioDeviceDesc &aDD);
	~VS_AudioWaveOut();
	void Start();
	void Stop();
	int GetBufferedDurr(bool is_precise);
	bool OutWrite(short* data, int len);
};


/**
 **************************************************************************
 * \brief waveIn device
 ****************************************************************************/
class VS_AudioWaveIn: public VS_AudioDeviceCommon, public VS_AudioWaveDevice
{
	HWAVEIN				m_hw;		///< wavein device handle
	void Release();
public:
	VS_AudioWaveIn(VS_AudioDeviceDesc &aDD);
	~VS_AudioWaveIn();
	void Start();
	void Stop();
	int GetBufferedDurr(bool is_precise);
	bool InRead(short* data, int &size);
};


/**
 **************************************************************************
 * \brief DirectSoundBuffer device
 ****************************************************************************/
class VS_AudioDSOut: public VS_AudioDeviceCommon, public VS_Lock
{
protected:
    VS_AudioDeviceDesc			m_aDD;				///< audio device description
	int							m_LastBuffPosition; ///< position in buffer to next write in
	LPDIRECTSOUNDBUFFER8		m_hw;				///< DirectSound "device handle"
	LPDIRECTSOUND8				m_pDS;				///< DirectSound
	void Release();
public:
	VS_AudioDSOut();
	VS_AudioDSOut(VS_AudioDeviceDesc &aDD, HWND hwnd);
	virtual ~VS_AudioDSOut();
	virtual void Start();
	virtual void Stop();
	virtual int GetBufferedDurr(bool is_precise);
	virtual int GetBufferedDurr(int& offset);
	virtual bool OutWrite(short* data, int len);
	virtual bool OutWriteNoise(short* data, int PendBytes);

};

/**
 **************************************************************************
 * \brief DirectSoundCapturreBuffer device
 ****************************************************************************/
class VS_AudioDSIn: public VS_AudioDeviceCommon, public VS_Lock
{
protected:
	VS_AudioDeviceDesc			m_aDD;				///< audio device description
	int							m_LastBuffPosition;	///< position in buffer to next read from
	__int64						m_restart_samples;
	LPDIRECTSOUNDCAPTUREBUFFER8	m_hw;				///< DirectSound Capture "device handle"
	LPDIRECTSOUNDCAPTURE8		m_pDSC;				///< DirectSound Capture
	void Release();
public:
	VS_AudioDSIn();
	VS_AudioDSIn(VS_AudioDeviceDesc &aDD);
	virtual ~VS_AudioDSIn();
	virtual void Start();
	virtual void Stop();
	virtual int GetBufferedDurr(bool is_precise);
	virtual bool InRead(short* data, int &size);
	virtual bool IsRestartDevice() {return (m_Samples >= m_restart_samples);}
};

/**
 **************************************************************************
 * \brief DirectSoundBuffer device
 ****************************************************************************/
class VS_AudioDSOutNative: public VS_AudioDSOut
{
	VS_AudioDeviceDesc			m_aDD2;				///< audio device description
	VS_AudioReSamplerSpeex		m_rs;
	VS_AudioReSamplerSpeex		m_rsn;
	VS_BinBuff					m_buff;
public:
	VS_AudioDSOutNative(VS_AudioDeviceDesc &aDD, HWND hwnd);
	int GetBufferedDurr(bool is_precise);
	int GetBufferedDurr(int& offset);
	__int64 GetNumSamples();
	bool OutWrite(short* data, int len);
	bool OutWriteNoise(short* data, int PendBytes);
};


/**
 **************************************************************************
 * \brief DirectSoundCapturreBuffer device
 ****************************************************************************/
class VS_AudioDSInNative: public VS_AudioDSIn
{
	VS_AudioDeviceDesc			m_aDD2;				///< audio device description
	VS_AudioReSamplerSpeex		m_rs;
	VS_BinBuff					m_buff;
	VS_FifoBuffer				m_fifo;
public:
	VS_AudioDSInNative(VS_AudioDeviceDesc &aDD);
	int GetBufferedDurr(bool is_precise);
	__int64 GetNumSamples();
	bool InRead(short* data, int &size);
};

/**
 **************************************************************************
 * \brief Software Muxer Device (without WSAPI muxer)
 ****************************************************************************/

/**
 **************************************************************************
 * \brief State Audio Muxer
 ****************************************************************************/
struct VS_AudioMuxerState
{
	VS_AudioReSamplerSpeex	*pResampler;
	VS_AudioBuff			*pBuffer;
	short					*pBufferNoise;
	int						iStreamSampleRate;
	int						iBlockAllign;
	int						iLastPosition;
	int						iNoiseLen;
	int						iLastNoisePosition;
	__int64					iLastFillAudio;
};

/**
 **************************************************************************
 * \brief Audio Muxer Implementation
 ****************************************************************************/
typedef std::pair<void*, VS_AudioMuxerState>				muxer_pair;
typedef std::map <void*, VS_AudioMuxerState>::iterator		muxer_iter;

class VS_AudioStreamMuxer: public VS_Lock
{
private:
	std::map<void*, VS_AudioMuxerState>	m_mapMuxer;
	int								m_iMuxerSampleRate;
	unsigned char					*m_pRsmplBuff;
	int								*m_pMuxerBuff;
	int								m_iBuffLen;
	VS_AudioMixer                   mixer;

public:
	VS_AudioStreamMuxer(int iMuxerSampleRate, int BuffLen);
	~VS_AudioStreamMuxer();
	bool AddAudioStream(void *handle, VS_AudioDeviceDesc &aDD);
	bool RemoveAudioStream(void *handle);
	int GetBufferedDurr(void *handle, bool is_precise);
	bool PutAudio(void *handle, short* data, int len);
	bool PutNoise(void *handle, short* data, int len);
	int GetMuxStream(short* data);
	int GetBufferLenght() { return m_iBuffLen; }
	int GetSampleRate() { return m_iMuxerSampleRate; }
};

/**
 **************************************************************************
 * \brief Audio Device Emulation
 ****************************************************************************/
class VS_AudioMuxerDevice: public CVSThread
{
/// static members
private:
	static VS_AudioMuxerDevice *m_MuxerInstance;
	static VS_Lock m_Lock;
public:
	static VS_AudioMuxerDevice* GetDeviceMuxer(VS_AudioDeviceDesc &aDD, HWND hwnd, void *hStream);
	static void DestroyDeviceMuxer(void *hStream);
/// non static members
private:
	bool					m_bValid;
	bool					m_IsVista;
	HANDLE					m_hComplEv;
	VS_FrequencyStat		*m_pFrequencyStat;
	VS_AudioDeviceCommon	*m_pDevice;
	VS_AudioStreamMuxer		*m_pAudioMuxer;


	VS_AudioMuxerDevice(VS_AudioDeviceDesc &aDD, HWND hwnd);
	~VS_AudioMuxerDevice();
	bool OpenAudioStream(void *hStream, VS_AudioDeviceDesc &aDD);
	bool CloseAudioStream(void *hStream);
protected:
	void CalcFrequency(int iBuffDurr);
public:
	bool IsValid() { return m_bValid; }
	bool PutAudio(void *hStream, short* data, int len);
	bool PutNoise(void *hStream, short* data, int len);
	VS_AudioDevice_State Start();
	VS_AudioDevice_State Stop();
	int GetBufferedDurr(void *hStream, bool is_precise);
	double GetFrequency(bool bEqualDevice) { return bEqualDevice ? m_pFrequencyStat->fFrequencyInit : m_pFrequencyStat->fFrequency; }
	DWORD Loop(LPVOID lpParameter);
};

VS_AudioMuxerDevice* VS_AudioMuxerDevice::m_MuxerInstance = NULL;
VS_Lock VS_AudioMuxerDevice::m_Lock;

/**
 **************************************************************************
 * \brief Audio Muxer Device Interface
 ****************************************************************************/
class VS_AudioMuxOut: public VS_AudioDeviceCommon
{
private:
	VS_AudioMuxerDevice *m_pDevice;
public:
	VS_AudioMuxOut(VS_AudioDeviceDesc &aDD, HWND hwnd);
	~VS_AudioMuxOut();
	void Start();
	void Stop();
	int GetBufferedDurr(bool is_precise);
	bool OutWrite(short* data, int len);
	bool OutWriteNoise(short* data, int PendBytes);
};

/***************************************************************************
* VS_AudioDeviceManager
****************************************************************************/
HWND					VS_AudioDeviceManager::m_hwnd = 0;
VS_AudioDeviceDesc		VS_AudioDeviceManager::m_RendDesc;
VS_AudioDeviceDesc 		VS_AudioDeviceManager::m_CaptDesc;
VS_AudioDeviceDesc 		VS_AudioDeviceManager::m_CurrCaptDesc;
VS_AudioDeviceManager*	VS_AudioDeviceManager::m_RendDevice = 0;
VS_AudioDeviceManager*	VS_AudioDeviceManager::m_CaptDevice = 0;

VS_AudioProcessing		VS_AudioDeviceManager::m_AudioProcessing;

long					VS_AudioDeviceManager::m_UseXPAec = 2;
long					VS_AudioDeviceManager::m_DevicesMode = 0;
bool					VS_AudioDeviceManager::m_bVista = false;
bool					VS_AudioDeviceManager::m_EnableAgc = true;	///< enable auto gain control (agc)
bool					VS_AudioDeviceManager::m_bEqualDevice = false;

#define DSNAME(name) #name

bool VSIsDXVersion10();

/**
 **************************************************************************
 * \param		hwnd	[in] window handle for DirectSound
 ****************************************************************************/
void VS_AudioDeviceManager::Open(HWND hwnd)
{
	m_hwnd = hwnd;
	// check existance and DirectSound version
	// First see if DSOUND.DLL even exists.

	g_hDsoundDLL = LoadLibrary( "dsound.dll" );

	if (g_hDsoundDLL) {
		// See if we can create the DirectSoundFullDuplexCreate object.
		g_DirectSoundCreate8 = (DIRECTSOUNDCREATE8)GetProcAddress(g_hDsoundDLL, DSNAME(DirectSoundCreate8));
		g_DirectSoundCaptureCreate8 = (DIRECTSOUNDCAPTURECREATE8)GetProcAddress(g_hDsoundDLL, DSNAME(DirectSoundCaptureCreate8));
		g_DirectSoundEnumerate = (DIRECTSOUNDENUMERATE)GetProcAddress(g_hDsoundDLL, DSNAME(DirectSoundEnumerateW));
		g_DirectSoundCaptureEnumerate = (DIRECTSOUNDENUMERATE)GetProcAddress(g_hDsoundDLL, DSNAME(DirectSoundCaptureEnumerateW));
		g_GetDeviceID = (GETDEVICEID)GetProcAddress(g_hDsoundDLL, DSNAME(GetDeviceID));
		m_bVista = VSIsDXVersion10();
	}

	int muxer =1;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	key.GetValue(&muxer, sizeof(int), VS_REG_INTEGER_VT, "Replace AM");

	if (g_hDsoundDLL)
		m_DevicesMode = (muxer > 0) ? 2 : 1;
	else
		m_DevicesMode = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioDeviceManager::Close()
{
	if (g_hDsoundDLL) FreeLibrary(g_hDsoundDLL); g_hDsoundDLL = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_AudioDeviceManager::GetDeviceList(bool is_capture, CStringList *list)
{
	if (!list) return -1;
	list->ResetList();
	int i;
	UINT dwCount;
	if (m_DevicesMode==1 || m_DevicesMode==2) {
		VS_DevD& dl = is_capture ? g_cpt : g_rnd;
		dl.clear();
		if (is_capture)
			g_DirectSoundCaptureEnumerate( (LPDSENUMCALLBACKA)DSoundEnumCallback, (LPVOID)1);
		else
			g_DirectSoundEnumerate( (LPDSENUMCALLBACKA)DSoundEnumCallback, 0);
		dwCount = dl.size();
		for (int i = 0; i < (int)dwCount; i++)
		{
			int n;
			n=list->iFindString((wchar_t*)dl[i].m_name.c_str());
			if(n<0)
				list->iAddString((wchar_t*)dl[i].m_name.c_str());
			else
			{
				wchar_t* pName=(wchar_t*) dl[i].m_name.c_str();
				char buff[256];
				_bstr_t str(pName);
				_itoa(list->iGetMaxString(),buff,10);
				str+=" #";
				str+=buff;
				list->iAddString((wchar_t*)str);

			}
		}
	}
	else {
		if (is_capture) {
			dwCount = waveInGetNumDevs();
			if (!dwCount) return -1;
			for (i = -1; i < (int)dwCount; i++) {
				WAVEINCAPS wc;
				if (waveInGetDevCaps(i, &wc, sizeof(WAVEINCAPS)) == MMSYSERR_NOERROR) {
					int n;
					n=list->iFindString(wc.szPname);
					if(n<0)
						list->iAddString(wc.szPname);
					else
					{
						char* pName=(char*) wc.szPname;
						char buff[256];
						_bstr_t str(pName);
						_itoa(list->iGetMaxString(),buff,10);
						str+=" #";
						str+=buff;
						list->iAddString((char*)str);
					}
				}
			}
		}
		else {
			dwCount = waveOutGetNumDevs();
			if (!dwCount) return -1;
			for (i = -1; i < (int)dwCount; i++) {
				WAVEOUTCAPS wc;
				if (waveOutGetDevCaps(i, &wc, sizeof(WAVEOUTCAPS)) == MMSYSERR_NOERROR) {
					int n;
					n=list->iFindString(wc.szPname);
					if(n<0)
						list->iAddString(wc.szPname);
					else
					{
						char* pName=(char*) wc.szPname;
						char buff[256];
						_bstr_t str(pName);
						_itoa(list->iGetMaxString(),buff,10);
						str+=" #";
						str+=buff;
						list->iAddString((char*)str);
					}
				}
			}
		}
	}
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_AudioDeviceManager::GetMaxSampleRate(int dev, bool is_capture)
{
	int samplerate = 0;
	if (dev >= 0) {
		if (is_capture && dev < g_cpt.size()) samplerate = g_cpt[dev].m_maxfreq;
		else if (!is_capture && dev < g_rnd.size()) samplerate = g_rnd[dev].m_maxfreq;
	}
	return samplerate;
}

/**
 **************************************************************************
 * \param		aDD	[in] requsted device description
 ****************************************************************************/
void VS_AudioDeviceManager::QueryDevice(VS_AudioDeviceDesc &aDD)
{
	// check input parametr
	if (!aDD.IsValid())
		return;

	// for non-DS or not initialised at all
	if (!m_CaptDevice && aDD.IsCapt)
		m_CaptDesc = aDD;
	if (!m_RendDevice && !aDD.IsCapt)
		m_RendDesc = aDD;

	if (aDD.IsCapt)
		m_CurrCaptDesc = aDD;

	// check DS
	if (m_DevicesMode==1) {
		if (aDD.IsCapt) {
			m_device = m_bVista ? new VS_AudioDSIn(aDD) : new VS_AudioDSInNative(aDD);
			if (!m_CaptDevice)
				m_CaptDevice = this;
		}
		else {
			m_device = m_bVista ? new VS_AudioDSOut(aDD, m_hwnd) : new VS_AudioDSOutNative(aDD, m_hwnd);
			if (!m_RendDevice)
				m_RendDevice = this;
		}
	}
	else if (m_DevicesMode==2) {
		if (aDD.IsCapt) {
			m_device = m_bVista ? new VS_AudioDSIn(aDD) : new VS_AudioDSInNative(aDD);
			if (!m_CaptDevice)
				m_CaptDevice = this;
		}
		else {
				if (RegularDevFlag==1)
					m_device = m_bVista ? new VS_AudioDSOut(aDD, m_hwnd) : new VS_AudioDSOutNative(aDD, m_hwnd);
				else
					m_device = new VS_AudioMuxOut(aDD, m_hwnd);
			if (!m_RendDevice)
				m_RendDevice = this;
		}
	}
	else {
		// set Wave devices only
		if (aDD.IsCapt) {
			m_device = new VS_AudioWaveIn(aDD);
			if (!m_CaptDevice)
				m_CaptDevice = this;
		}
		else {
			m_device = new VS_AudioWaveOut(aDD);
			if (!m_RendDevice)
				m_RendDevice = this;
		}
	}

	if (aDD.IsCapt)
	{
		uint32_t filters = filterType::All_Filter;

		if (!VS_AudioDeviceManager::m_EnableAgc)
			filters &= ~filterType::GainControl_Filter;

		m_AudioProcessing.Init(aDD.wf.nSamplesPerSec, filters);
	}

	//VS_EchoDebugger::GetInstance().Init(std::to_string(timeGetTime()));

	DTRACE(VSTM_AUDI0, "QueryDevice returns %x type %d", m_device, m_device ? m_device->Type() : 0);
	if (m_bVista && m_RendDevice && m_CaptDevice && m_CaptDesc.dev < g_cpt.size() && m_RendDesc.dev < g_rnd.size()) {
		m_bEqualDevice = VS_CompareDeviceInteface(g_cpt[m_CaptDesc.dev].m_dev_interface.c_str(), g_rnd[m_RendDesc.dev].m_dev_interface.c_str());
		DTRACE(VSTM_AUDI0, "QueryDevice Capture Device %s Render Device", m_bEqualDevice ? "==" : "!=");
	}
	memset(&m_FrequencyStat, 0, sizeof(VS_FrequencyStat));
	if (aDD.IsCapt) {
		/// set frequency stat
		m_FrequencyStat.pFreqDeviation = new VS_FreqDeviation();
		int dr, dt;
		double dfr;
		if (m_bVista) {
			dr = 3000;
			dfr = 0.001;
			dt = 10000;
		} else {
			dr = 500;
			dfr = 0.04;
			dt = 5000;
		}
		m_FrequencyStat.Init(aDD.wf.nSamplesPerSec, dfr, dt);
		m_FrequencyStat.pFreqDeviation->Init(dr);
		m_FrequencyStat.pFreqDeviation->Clear();
	} else
	{
					/// set frequency stat
			m_FrequencyStat.pFreqDeviation = new VS_FreqDeviation();
			m_FrequencyStat.Init(aDD.wf.nSamplesPerSec, (m_bVista) ? 0.001 : 0.04, 10000);
			m_FrequencyStat.pFreqDeviation->Init(500);
			m_FrequencyStat.pFreqDeviation->Clear();
	}

	if (m_CaptDevice && m_RendDevice &&
		g_cpt.size() > m_CurrCaptDesc.dev && g_rnd.size() > m_RendDesc.dev)
	{
		VS_RegistryKey key(true, "EchoDelay");

		std::string value;
		auto devPair = vs::UTF16toUTF8Convert(g_cpt[m_CurrCaptDesc.dev].m_name + L" && " + g_rnd[m_RendDesc.dev].m_name);
		if (!devPair.empty() && key.GetString(value, devPair.c_str()))
		{
			if (boost::starts_with(value, "delay"))
			{
				int32_t echoDelay = std::stol(value.substr(5));

				if (echoDelay > 80)
				{
					m_AudioProcessing.SetEchoDelay(echoDelay);
				}
			}
		}
	}

	m_FrequencyStat.bValid = (m_FrequencyStat.pFreqDeviation != 0);
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioDeviceManager::ReleaseDevice()
{
	DTRACE(VSTM_AUDI0, "ReleaseDevice %x type %d", m_device, m_device ? m_device->Type() : 0);
	if (m_device) {
		delete m_device; m_device = 0;
		delete m_FrequencyStat.pFreqDeviation;
		memset(&m_FrequencyStat, 0, sizeof(VS_FrequencyStat));

		m_AudioProcessing.StopDelayDetectTest();

	}
	if (m_CaptDevice == this) {
		m_CaptDevice = 0;
		m_CaptDesc.Clean();
		m_bEqualDevice = false;
	}
	else if (m_RendDevice == this) {
		m_RendDevice = 0;
		m_RendDesc.Clean();
		m_bEqualDevice = false;
	}
}

/**
 **************************************************************************
 * \param		data	[in, out] audio samples put in render device
 * \param		samples	[in] number of samples
 ****************************************************************************/
void VS_AudioDeviceManager::PreRenderCallBack(short* data, unsigned long samples)
{
	if (m_device->Type() != ADTYPE_FDMUXOUT)
	{
		int offset = 0;
		int buff_durr = m_device->GetBufferedDurr(offset);
		m_AudioProcessing.ProcessRenderAudio(data, samples, m_RendDesc.wf.nSamplesPerSec, buff_durr);
	}
}

void VS_AudioDeviceManager::PostRenderCallBack(short* data, unsigned long samples)
{
}

/**
 **************************************************************************
 * \param		data	[in, out] audio samples obtained from capture device
 * \param		samples	[in] number of samples
 ****************************************************************************/
void VS_AudioDeviceManager::CaptureCallBack(short* data, unsigned long samples)
{
	int t1 = m_device->GetBufferedDurr(true);
	CalcFrequency(t1);

	bool isMuted = g_DevStatus.GetStatus() & DVS_SND_PAUSED; // setted from g_DevStatus.SetStatus(DVS_SND_PAUSED, true, vr.boolVal == 0);

	static VS_FifoBuffer fifoBuff(16000 * sizeof(int16_t));
	std::vector<int16_t> tmp(samples);

	fifoBuff.AddData(data, samples * sizeof(int16_t));

	if (fifoBuff.GetDataLen() / 2 >= 16000 / 1000 * 0)
	{
		fifoBuff.GetData(tmp.data(), samples * sizeof(int16_t));
	}
	else
	{
		memset(tmp.data(), 0, samples * sizeof(int16_t));
	}

	m_AudioProcessing.ProcessCaptureAudio(tmp.data(), data, samples, GetFrequency(m_bEqualDevice), t1, isMuted);
}

/****************************************************************************
 * VS_AudioWaveOut
 ****************************************************************************/
/**
 **************************************************************************
 * Open and init WaveOut device
 * \param		aDD	[in] requsted device description
 ****************************************************************************/
VS_AudioWaveOut::VS_AudioWaveOut(VS_AudioDeviceDesc &aDD)
{
	m_dtype = ADTYPE_WAVEOUT;
	m_aDD = aDD;
	int msize = (aDD.Num+2)*aDD.Len;
	m_wh = (WAVEHDR*)malloc(aDD.Num*sizeof(WAVEHDR));
	m_Memory = malloc(msize);
	if (!m_wh || !m_Memory) {
		Release();
		return;
	}
	memset(m_wh, 0, aDD.Num*sizeof(WAVEHDR));
	if (aDD.wf.wBitsPerSample==8) memset(m_Memory, 128, msize);
	else memset(m_Memory, 0, msize);

	if (waveOutOpen(&m_hw, aDD.dev, &aDD.wf, (DWORD_PTR)aDD.Event, 0, CALLBACK_EVENT)!=MMSYSERR_NOERROR) {
		Release();
		return;
	}
	for (int i = 0; i<aDD.Num; i++) {
		m_wh[i].lpData = (LPSTR)((char*)m_Memory + aDD.Len*(i+1));
		m_wh[i].dwBufferLength = aDD.Len;
		if (waveOutPrepareHeader(m_hw, &m_wh[i], sizeof(WAVEHDR))!=MMSYSERR_NOERROR) {
			Release();
			return ;
		}
		m_wh[i].dwFlags|=WHDR_DONE;
	}
	m_Samples = 0;
	m_lastDeviceSamples = 0;
	m_StartBuff = 0;
	m_IsValid = true;
}

/**
 **************************************************************************
 * Close WaveOut device
 ****************************************************************************/
void VS_AudioWaveOut::Release()
{
	m_IsValid = false;
	if (m_hw) {
		waveOutPause(m_hw);
		Sleep(0);
		waveOutReset(m_hw);
		Sleep(0);
		for (int i = 0; i<m_aDD.Num; i++)
			if (m_wh[i].dwFlags&WHDR_PREPARED)
				waveOutUnprepareHeader(m_hw, &m_wh[i], sizeof(WAVEHDR));
		waveOutClose(m_hw);
		m_hw = 0;
		Sleep(0);
		ResetEvent(m_aDD.Event);
	}
	if (m_Memory) free(m_Memory); m_Memory = 0;
	if (m_wh) free(m_wh); m_wh = 0;
	m_Samples = 0;
	m_lastDeviceSamples = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioWaveOut::~VS_AudioWaveOut()
{
	Release();
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioWaveOut::Start()
{
	if (m_IsValid) {
		waveOutRestart(m_hw);
		m_state = ADSTATE_START;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioWaveOut::Stop()
{
	if (m_IsValid) {
		waveOutReset(m_hw);
		m_Samples = 0;
		m_lastDeviceSamples = 0;
		m_state = ADSTATE_STOP;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_AudioWaveOut::GetBufferedDurr(bool is_precise)
{
	if (!m_IsValid) return 0;
	MMTIME mmt;
	mmt.wType = TIME_SAMPLES;
	if (waveOutGetPosition(m_hw, &mmt, sizeof(MMTIME))==MMSYSERR_NOERROR) {
		if (m_lastDeviceSamples > (mmt.u.sample + 320) && m_state==ADSTATE_START) {
			// +320 - для уменьшения вероятности возникновения ситуации конфликта двух потоков (capture и render)
			// на многоядерных процессорах
			Stop(); Start();
			mmt.u.sample = 0;
		}
		m_lastDeviceSamples = mmt.u.sample;
		if (!is_precise) {
			int PendTime = (int)((m_Samples - mmt.u.sample)*1000/m_aDD.wf.nSamplesPerSec);
			return PendTime>0 ? PendTime : 0;
		} else {
			int num_smpl = (int)(m_Samples - mmt.u.sample);
			return num_smpl > 0 ? num_smpl : 0;
		}
	}
	else
		return 0;

}

/**
 **************************************************************************
 * \param		data		[in] audio data for playing.
 * \param		PendTime	[in] duration of pended data in device
 * \return true if write was successful
 ****************************************************************************/
bool VS_AudioWaveOut::OutWrite(short* data, int len)
{
	int i = m_StartBuff%m_aDD.Num;
	if (m_wh[i].dwFlags&WHDR_DONE) {
		m_wh[i].dwFlags&=~WHDR_DONE;
		memcpy(m_wh[i].lpData, data, m_aDD.Len);
		m_Samples+= m_aDD.Len/m_aDD.wf.nBlockAlign;
		MMRESULT mmres = waveOutWrite(m_hw, &m_wh[i], sizeof(WAVEHDR));
		m_StartBuff++;
		return true;
	}
	else
		return false;
}

/****************************************************************************
 * VS_AudioWaveIn
 ****************************************************************************/
/**
 **************************************************************************
 * Open and init WaveIn device
 * \param		aDD	[in] requsted device description
 ****************************************************************************/
VS_AudioWaveIn::VS_AudioWaveIn(VS_AudioDeviceDesc &aDD)
{
	m_dtype = ADTYPE_WAVEIN;
	m_aDD = aDD;
	int msize = (aDD.Num+2)*aDD.Len;
	m_wh = (WAVEHDR*)malloc(aDD.Num*sizeof(WAVEHDR));
	m_Memory = malloc(msize);
	if (!m_wh || !m_Memory) {
		Release();
		return;
	}
	memset(m_wh, 0, aDD.Num*sizeof(WAVEHDR));
	if (aDD.wf.wBitsPerSample==8) memset(m_Memory, 128, msize);
	else memset(m_Memory, 0, msize);

	if (waveInOpen(&m_hw, aDD.dev, &aDD.wf, (DWORD_PTR)aDD.Event, 0, CALLBACK_EVENT)!=MMSYSERR_NOERROR) {
		Release();
		return;
	}
	for (int i = 0; i<aDD.Num; i++) {
		m_wh[i].lpData = (LPSTR)((char*)m_Memory + aDD.Len*(i+1));
		m_wh[i].dwBufferLength = aDD.Len;
		if (waveInPrepareHeader(m_hw, &m_wh[i], sizeof(WAVEHDR))!=MMSYSERR_NOERROR ||
			waveInAddBuffer(m_hw, &m_wh[i], sizeof(WAVEHDR))!=MMSYSERR_NOERROR)
		{
			Release();
			return;
		}
	}
	m_Samples = 0;
	m_lastDeviceSamples = 0;
	m_StartBuff = 0;
	m_IsValid = true;
	Stop();
}

/**
 **************************************************************************
 * Close WaveIn device
 ****************************************************************************/
void VS_AudioWaveIn::Release()
{
	m_IsValid = false;
	if (m_hw) {
		waveInReset(m_hw);
		Sleep(0);
		for (int i = 0; i<m_aDD.Num; i++)
			if (m_wh[i].dwFlags&WHDR_PREPARED)
				waveInUnprepareHeader(m_hw, &m_wh[i], sizeof(WAVEHDR));
		waveInClose(m_hw);
		m_hw =0;
		Sleep(0);
		ResetEvent(m_aDD.Event);
	}
	if (m_Memory) free(m_Memory); m_Memory = 0;
	if (m_wh) free(m_wh); m_wh = 0;
	m_Samples = 0;
	m_lastDeviceSamples = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioWaveIn::~VS_AudioWaveIn()
{
	Release();
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioWaveIn::Start()
{
	if (m_IsValid) {
		waveInStart(m_hw);
		m_state = ADSTATE_START;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioWaveIn::Stop()
{
	if (m_IsValid) {
		waveInStop(m_hw);
		m_Samples = 0;
		m_lastDeviceSamples = 0;
		m_state = ADSTATE_STOP;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_AudioWaveIn::GetBufferedDurr(bool is_precise)
{
	if (!m_IsValid) return 0;
	MMTIME mmt;
	mmt.wType = TIME_SAMPLES;
	if (waveInGetPosition(m_hw, &mmt, sizeof(MMTIME))==MMSYSERR_NOERROR && mmt.wType == TIME_SAMPLES) {
		if (m_lastDeviceSamples > (mmt.u.sample + 320) && m_state==ADSTATE_START) {
			// +320 - для уменьшения вероятности возникновения ситуации конфликта двух потоков (capture и render)
			// на многоядерных процессорах
			Stop(); Start();
			mmt.u.sample = 0;
		}
		m_lastDeviceSamples = mmt.u.sample;
		if (!is_precise) {
			int PendTime = -(int)((m_Samples - mmt.u.sample)*1000/m_aDD.wf.nSamplesPerSec);
			return PendTime>0 ? PendTime : 0;
		} else {
			int num_smpl = -(int)(m_Samples - mmt.u.sample);
			return num_smpl > 0 ? num_smpl : 0;
		}
	}
	else
		return 0;
}

/**
 **************************************************************************
 * \param		data	[out] pointer to captured data.
 * \return true if read was successful
 ****************************************************************************/
bool VS_AudioWaveIn::InRead(short* data, int &size)
{
	int i = m_StartBuff%m_aDD.Num;
	size = 0;
	if (m_wh[i].dwFlags&WHDR_DONE) {
		m_wh[i].dwFlags&=~WHDR_DONE;
		memcpy(data, m_wh[i].lpData, m_wh[i].dwBytesRecorded);
		m_Samples+= m_wh[i].dwBytesRecorded/2;
		MMRESULT mmres = waveInAddBuffer(m_hw, &m_wh[i], sizeof(WAVEHDR));
		size = m_wh[i].dwBufferLength;
		m_StartBuff++;
		return true;
	}
	else
		return false;
}

/****************************************************************************
 * VS_AudioDSOut
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
VS_AudioDSOut::VS_AudioDSOut()
{
	m_dtype = ADTYPE_FDOUT;
	m_LastBuffPosition = 0;
	m_pDS = 0;
	m_hw = 0;
	m_IsValid = false;
}

/**
 **************************************************************************
 * \param		device	[in] LPDIRECTSOUNDBUFFER8 interface.
 * \param		aDD		[in] requsted device description
 ****************************************************************************/
VS_AudioDSOut::VS_AudioDSOut(VS_AudioDeviceDesc &aDD, HWND hwnd)
{
	m_dtype = ADTYPE_FDOUT;
	m_aDD = aDD;
	m_LastBuffPosition = 0;
	m_pDS = 0;
	m_hw = 0;

	DSBUFFERDESC dsd;
	ZeroMemory(&dsd, sizeof(dsd));
	dsd.dwSize = sizeof(dsd);
	dsd.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY|DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_GLOBALFOCUS|DSBCAPS_CTRLVOLUME;
	if (m_aDD.IsVista) dsd.dwFlags |= 0x00080000; // true play position
	dsd.lpwfxFormat = &aDD.wf;
	dsd.dwBufferBytes = aDD.Num*aDD.Len;
	dsd.guid3DAlgorithm = GUID_NULL;

	LPDIRECTSOUNDBUFFER ppdsb = 0;
	g_DirectSoundCreate8(&g_rnd[aDD.dev].m_guid, &m_pDS, 0);
	if (m_pDS) {
		m_pDS->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
		m_pDS->CreateSoundBuffer(&dsd, &ppdsb, 0);
	}
	if (ppdsb) {
		ppdsb->QueryInterface(IID_IDirectSoundBuffer8, (VOID**)&m_hw);
		ppdsb->Release();
		ppdsb = 0;
	}
	if (!m_hw) {
		Release();
		return;
	}

    HRESULT hr;
	DSBPOSITIONNOTIFY*  aPosNotify     = NULL;
	LPDIRECTSOUNDNOTIFY8 pDSNotify      = NULL;
    // the buffer as the sound plays.
	if( FAILED( hr = m_hw->QueryInterface( IID_IDirectSoundNotify8, (VOID**)&pDSNotify ) ) ) {
		Release();
		return;
    }

	aPosNotify = new DSBPOSITIONNOTIFY[ m_aDD.Num ];

    for( int i = 0; i < m_aDD.Num; i++ ){
		aPosNotify[i].dwOffset     = (m_aDD.Len * i);
		aPosNotify[i].hEventNotify = m_aDD.Event;
    }

    // Tell DirectSound when to notify us.
    m_IsValid = SUCCEEDED( hr = pDSNotify->SetNotificationPositions( m_aDD.Num, aPosNotify ) );
	pDSNotify->Release();
	delete[] aPosNotify;
	m_Samples = 0;
	m_hw->SetCurrentPosition(0);
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioDSOut::Release()
{
	m_IsValid = false;
	if (m_hw) {
		m_hw->Stop();
		m_hw->Release();
		m_hw = 0;
	}
	if (m_pDS) {
		m_pDS->Release();
		m_pDS = 0;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioDSOut::~VS_AudioDSOut()
{
	Release();
}

/**
 **************************************************************************
 * Start DirectSound Device. Fill internal Buffer with silence
 ****************************************************************************/
void VS_AudioDSOut::Start()
{
	if (m_IsValid) {
		// write silence
		LPVOID  lpvPtr1;
		DWORD dwBytes1;
		LPVOID  lpvPtr2;
		DWORD dwBytes2;
		HRESULT hr;

		hr = m_hw->Lock(0, 0, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, DSBLOCK_ENTIREBUFFER);
		if (DSERR_BUFFERLOST == hr) {
			m_hw->Restore();
			hr = m_hw->Lock(0, 0, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, DSBLOCK_ENTIREBUFFER);
		}
		if (SUCCEEDED(hr)) {
			memset(lpvPtr1, 0, dwBytes1);
			hr = m_hw->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
		}
		m_hw->Play(0, 0, DSBPLAY_LOOPING);
		m_state = ADSTATE_START;
	}
}

/**
 **************************************************************************
 * Stop DirectSound device. Call it to avoid circullar playing of internal buffer
 ****************************************************************************/
void VS_AudioDSOut::Stop()
{
	if (m_IsValid) {
		m_hw->Stop();
		m_Samples = 0;
		m_state = ADSTATE_STOP;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_AudioDSOut::GetBufferedDurr(bool is_precise)
{
	DWORD PlayCursor;
	if (SUCCEEDED(m_hw->GetCurrentPosition(&PlayCursor, 0))) {
		int durr = m_LastBuffPosition - (int)PlayCursor;
		if (durr< -(m_aDD.Num-3)*m_aDD.Len/3) durr+= m_aDD.Num*m_aDD.Len;
		if (durr> 2*(m_aDD.Num-3)*m_aDD.Len/3) durr-= m_aDD.Num*m_aDD.Len;
		if (!is_precise) {
			durr = durr/m_aDD.wf.nBlockAlign*1000/(int)m_aDD.wf.nSamplesPerSec;
		} else {
			durr = durr/m_aDD.wf.nBlockAlign;

		}
		return durr;
	}
	return 0;
}

int VS_AudioDSOut::GetBufferedDurr(int& offset)
{
	DWORD pc = 0;
	DWORD wc = 0;
	if (SUCCEEDED(m_hw->GetCurrentPosition(&pc, &wc))) {
		int durr = m_LastBuffPosition - (int)pc;
		if (durr< -(m_aDD.Num-3)*m_aDD.Len/3)
			durr+= m_aDD.Num*m_aDD.Len;
		if (durr> 2*(m_aDD.Num-3)*m_aDD.Len/3)
			durr-= m_aDD.Num*m_aDD.Len;
		durr = durr/2;
		offset = int(wc - pc);
		if (offset < 0)
			offset+= m_aDD.Num*m_aDD.Len;
		offset/=2;
		return durr;
	}
	return 0;
}


/**
 **************************************************************************
 * \param		data		[in] audio data for playing.
 * \param		PendTime	[in] duration of pended data in device
 * \return true if write was successful
 ****************************************************************************/
bool VS_AudioDSOut::OutWrite(short* data, int len)
{
	LPVOID  lpvPtr1;
	DWORD dwBytes1;
	LPVOID  lpvPtr2;
	DWORD dwBytes2;
	HRESULT hr;

	// Obtain memory address of write block. This will be in two parts
	// if the block wraps around.

	hr = m_hw->Lock(m_LastBuffPosition, len, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);

	// If the buffer was lost, restore and retry lock.
	if (DSERR_BUFFERLOST == hr) {
		m_hw->Restore();
		hr = m_hw->Lock(m_LastBuffPosition, len, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
	}

	if (SUCCEEDED(hr)) {
		// Write to pointers.
		CopyMemory(lpvPtr1, data, dwBytes1);
		if (NULL != lpvPtr2) {
				CopyMemory(lpvPtr2, data+dwBytes1/2, dwBytes2);
		}

		// Release the data back to DirectSound.
		hr = m_hw->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
		if (SUCCEEDED(hr)) {
			// Success.
			m_LastBuffPosition+= len;
			m_LastBuffPosition%= m_aDD.Len*m_aDD.Num;
			m_Samples += len / m_aDD.wf.nBlockAlign;
			return true;
		}
	}
	// Lock, Unlock, or Restore failed.
	return false;
}

bool VS_AudioDSOut::OutWriteNoise(short* data, int PendBytes)
{
	LPVOID  lpvPtr1;
	DWORD dwBytes1;
	LPVOID  lpvPtr2;
	DWORD dwBytes2;
	HRESULT hr;

	// Obtain memory address of write block. This will be in two parts
	// if the block wraps around.
	int Noff = m_LastBuffPosition + m_aDD.wf.nSamplesPerSec*2*2 - m_aDD.Len; // 2 sec - 1 frame
	Noff%= m_aDD.Len*m_aDD.Num;
	hr = m_hw->Lock(Noff, PendBytes, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);

	// If the buffer was lost, restore and retry lock.
	if (DSERR_BUFFERLOST == hr) {
		m_hw->Restore();
		hr = m_hw->Lock(Noff, PendBytes, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
	}
	if (SUCCEEDED(hr)) {
		// Write to pointers.
		CopyMemory(lpvPtr1, data, dwBytes1);

		if (NULL != lpvPtr2) {
			CopyMemory(lpvPtr2, data+dwBytes1/2, dwBytes2);
		}
		// Release the data back to ПDirectSound.
		hr = m_hw->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
		if (SUCCEEDED(hr)) {
			return true;
		}
	}
	// Lock, Unlock, or Restore failed.
	return false;
}

/****************************************************************************
 * VS_AudioDSIn
 ****************************************************************************/

/**
 **************************************************************************
 * \param		device	[in] pointer to LPDIRECTSOUNDCAPTUREBUFFER8 interface
 * \param		aDD		[in] requsted device description
 ****************************************************************************/
VS_AudioDSIn::VS_AudioDSIn()
{
	m_dtype = ADTYPE_FDIN;
	m_hw = 0;
	m_pDSC = 0;
	m_LastBuffPosition = 0;
	m_IsValid = false;
}

/**
 **************************************************************************
 * \param		device	[in] pointer to LPDIRECTSOUNDCAPTUREBUFFER8 interface
 * \param		aDD		[in] requsted device description
 ****************************************************************************/
VS_AudioDSIn::VS_AudioDSIn(VS_AudioDeviceDesc &aDD)
{
	m_dtype = ADTYPE_FDIN;
	m_aDD = aDD;
	m_hw = 0;
	m_pDSC = 0;
	m_LastBuffPosition = 0;

	DSCBUFFERDESC dscd;
	ZeroMemory(&dscd, sizeof(dscd));
	dscd.dwSize = sizeof(dscd);
	dscd.dwBufferBytes = aDD.Num*aDD.Len;
	dscd.lpwfxFormat = &aDD.wf;

	LPDIRECTSOUNDCAPTUREBUFFER ppdscb = 0;
	g_DirectSoundCaptureCreate8(&g_cpt[aDD.dev].m_guid, &m_pDSC, 0);
	if (m_pDSC)
		m_pDSC->CreateCaptureBuffer(&dscd, &ppdscb, 0);
	if (ppdscb) {
		ppdscb->QueryInterface(IID_IDirectSoundCaptureBuffer8, (VOID**)&m_hw);
		ppdscb->Release();
		ppdscb = 0;
	}
	if (!m_hw) {
		Release();
		return;
	}

    HRESULT hr;
	DSBPOSITIONNOTIFY*  aPosNotify     = NULL;
	LPDIRECTSOUNDNOTIFY8 pDSNotify      = NULL;
    // the buffer as the sound plays.
	if( FAILED( hr = m_hw->QueryInterface( IID_IDirectSoundNotify8, (VOID**)&pDSNotify ) ) ) {
		Release();
		return;
    }

	aPosNotify = new DSBPOSITIONNOTIFY[ m_aDD.Num + 1];
	int i = 0;
    for( ; i < m_aDD.Num; i++ ){
		aPosNotify[i].dwOffset     = (m_aDD.Len * i) + m_aDD.Len - 1;
		aPosNotify[i].hEventNotify = m_aDD.Event;
    }
	aPosNotify[i].dwOffset = 0;
	aPosNotify[i].hEventNotify = 0;

    // Tell DirectSound when to notify us.
	m_IsValid = SUCCEEDED( hr = pDSNotify->SetNotificationPositions( m_aDD.Num, aPosNotify ) );
	pDSNotify->Release();
	delete[] aPosNotify;
	m_Samples = 0;
	__int64 rnd = VS_GenKeyByMD5();
	__int64 dt = (rnd * 600 * m_aDD.wf.nSamplesPerSec) >> 32;
	m_restart_samples = (6 * 3600 - 300) * m_aDD.wf.nSamplesPerSec; // 6 hour for reset DS
	m_restart_samples += dt;
	Stop();
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioDSIn::Release()
{
	VS_AutoLock lock(this);
	m_IsValid = false;
	if (m_hw) {
		m_hw->Stop();
		m_hw->Release();
		m_hw = 0;
	}
	if (m_pDSC) {
		m_pDSC->Release();
		m_pDSC = 0;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioDSIn::~VS_AudioDSIn()
{
	Release();
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioDSIn::Start()
{
	if (m_IsValid) {
		m_hw->Start(DSCBSTART_LOOPING);
		m_state = ADSTATE_START;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioDSIn::Stop()
{
	if (m_IsValid) {
		m_hw->Stop();
		m_Samples = 0;
		m_state = ADSTATE_STOP;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_AudioDSIn::GetBufferedDurr(bool is_precise)
{
	DWORD CapturePosition;
	DWORD ReadPosition;
	if (SUCCEEDED(m_hw->GetCurrentPosition(&CapturePosition, &ReadPosition))) {
		int durr = (int)CapturePosition - m_LastBuffPosition;
		if (durr< 0) durr+= m_aDD.Num*m_aDD.Len;
		if (!is_precise) {
			durr = durr/m_aDD.wf.nBlockAlign*1000/m_aDD.wf.nSamplesPerSec;
		} else {
			durr = durr/m_aDD.wf.nBlockAlign;
		}
		return durr;
	}
	return 0;
}

/**
 **************************************************************************
 * \param		data	[out] pointer to captured data.
 * \return true if read was successful
 ****************************************************************************/
bool VS_AudioDSIn::InRead(short* data, int &size)
{
	VS_AutoLock lock(this);
	HRESULT hr;
	VOID* pbCaptureData  = NULL;
	DWORD dwCaptureLength;
	VOID* pbCaptureData2 = NULL;
	DWORD dwCaptureLength2;
	DWORD dwReadPos;
	LONG lLockSize;
	DWORD OneBufferSize = m_aDD.Len;
	size = 0;

	if (FAILED (hr = m_hw->GetCurrentPosition( NULL, &dwReadPos)))
		return false;

	// Lock everything between our private cursor
	// and the read cursor, allowing for wraparound.
	lLockSize = dwReadPos - m_LastBuffPosition;
	if( lLockSize < 0 ) lLockSize += m_aDD.Num*m_aDD.Len;

	if( lLockSize < m_aDD.Len ) // not enouth data
		return false;

	if (FAILED(hr = m_hw->Lock(m_LastBuffPosition, lLockSize, &pbCaptureData, &dwCaptureLength, &pbCaptureData2, &dwCaptureLength2, 0L)))
		return false;

	if (pbCaptureData2==0) {
		if (dwCaptureLength < OneBufferSize)
			return false;
		else
			memcpy(data, pbCaptureData, OneBufferSize);
	}
	else {
		if (dwCaptureLength < OneBufferSize) {
			if (dwCaptureLength + dwCaptureLength2 < OneBufferSize)
				return false;
			else {
				memcpy(data, pbCaptureData, dwCaptureLength);
				memcpy(data + dwCaptureLength/2, pbCaptureData2, OneBufferSize - dwCaptureLength);
			}
		}
		else {
			memcpy(data, pbCaptureData, OneBufferSize);
		}
	}

	// Unlock the capture buffer.
	m_hw->Unlock( pbCaptureData, dwCaptureLength, pbCaptureData2, dwCaptureLength2);

	// Move the capture offset along.
	m_LastBuffPosition+= m_aDD.Len;
	m_LastBuffPosition%= m_aDD.Len*m_aDD.Num;
	m_Samples += m_aDD.Len / m_aDD.wf.nBlockAlign;
	size = OneBufferSize;

	return true;
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

/**
 **************************************************************************
 * \param		aDD		[in] requsted device description
 * \param		vista	[in] is Vista ?
 * \param		hwnd	[in] windo handler
 ****************************************************************************/
VS_AudioDSOutNative::VS_AudioDSOutNative(VS_AudioDeviceDesc &aDD, HWND hwnd)
{
	GUID guid;
	int freq = 0;
	//if (aDD.dev==-1) { // default device
	//	g_GetDeviceID(&g_rnd[0].m_guid, &guid);
	//	int size = g_rnd.size();
	//	for (int i = 1; i< size; i++)
	//		if (g_rnd[i].m_guid==guid)
	//			freq = g_rnd[i].m_maxfreq;
	//}
	//else
	{
		guid = g_rnd[aDD.dev].m_guid;
		freq = g_rnd[aDD.dev].m_maxfreq;
	}
	if (freq==0)
		freq = aDD.wf.nSamplesPerSec;

	m_aDD2 = aDD;
	m_aDD = aDD;
	m_aDD.Len = freq*2/50; //20 ms
	m_aDD.Num = (__int64)aDD.Num*aDD.Len*freq/aDD.wf.nSamplesPerSec/m_aDD.Len;
	m_aDD.wf.nSamplesPerSec = freq;
	m_aDD.wf.nAvgBytesPerSec = m_aDD.wf.nSamplesPerSec*m_aDD.wf.nBlockAlign;

    HRESULT hr = S_OK;
	DSBUFFERDESC dsd;
	ZeroMemory(&dsd, sizeof(dsd));
	dsd.dwSize = sizeof(dsd);
	dsd.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY|DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_GLOBALFOCUS|DSBCAPS_CTRLVOLUME;
	if (m_aDD.IsVista) dsd.dwFlags |= 0x00080000; // true play position
	dsd.lpwfxFormat = &m_aDD.wf;
	dsd.dwBufferBytes = m_aDD.Num*m_aDD.Len;
	dsd.guid3DAlgorithm = GUID_NULL;

	m_buff.SetSize(dsd.dwBufferBytes);

	LPDIRECTSOUNDBUFFER ppdsb = 0;
	hr = g_DirectSoundCreate8(&guid, &m_pDS, 0);
	if (m_pDS) {
		hr = m_pDS->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
		hr = m_pDS->CreateSoundBuffer(&dsd, &ppdsb, 0);
	}
	if (ppdsb) {
		hr = ppdsb->QueryInterface(IID_IDirectSoundBuffer8, (VOID**)&m_hw);
		ppdsb->Release();
		ppdsb = 0;
	}
	if (!m_hw) {
		Release();
		return;
	}

	DSBPOSITIONNOTIFY*  aPosNotify     = NULL;
	LPDIRECTSOUNDNOTIFY8 pDSNotify      = NULL;
    // the buffer as the sound plays.
	if( FAILED( hr = m_hw->QueryInterface( IID_IDirectSoundNotify8, (VOID**)&pDSNotify ) ) ) {
		Release();
		return;
    }

	aPosNotify = new DSBPOSITIONNOTIFY[ m_aDD.Num ];

    for( int i = 0; i < m_aDD.Num; i++ ){
		aPosNotify[i].dwOffset     = m_aDD.Len * i;
		aPosNotify[i].hEventNotify = m_aDD.Event;
    }

    // Tell DirectSound when to notify us.
    m_IsValid = SUCCEEDED( hr = pDSNotify->SetNotificationPositions( m_aDD.Num, aPosNotify ) );
	pDSNotify->Release();
	delete[] aPosNotify;
	m_Samples = 0;
	m_hw->SetCurrentPosition(0);
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_AudioDSOutNative::GetBufferedDurr(bool is_precise)
{
	int durr =  VS_AudioDSOut::GetBufferedDurr(is_precise);
	return is_precise ? (__int64)durr*m_aDD2.wf.nSamplesPerSec/m_aDD.wf.nSamplesPerSec : durr;
}


int VS_AudioDSOutNative::GetBufferedDurr(int& offset)
{
	int durr = VS_AudioDSOut::GetBufferedDurr(offset);
	offset = (__int64)offset*m_aDD2.wf.nSamplesPerSec/m_aDD.wf.nSamplesPerSec;
	return (__int64)durr*m_aDD2.wf.nSamplesPerSec/m_aDD.wf.nSamplesPerSec;
}

/**
 **************************************************************************
 ****************************************************************************/
__int64 VS_AudioDSOutNative::GetNumSamples()
{
	return m_Samples*m_aDD2.wf.nSamplesPerSec/m_aDD.wf.nSamplesPerSec;
}

/**
 **************************************************************************
 * \param		data		[in] audio data for playing.
 * \param		PendTime	[in] duration of pended data in device
 * \return true if write was successful
 ****************************************************************************/
bool VS_AudioDSOutNative::OutWrite(short* data, int len)
{
	LPVOID  lpvPtr1;
	DWORD dwBytes1;
	LPVOID  lpvPtr2;
	DWORD dwBytes2;
	HRESULT hr;

	int rssize = m_rs.Process(data, (void*)m_buff.Buffer(), len, m_aDD2.wf.nSamplesPerSec, m_aDD.wf.nSamplesPerSec);

	// Obtain memory address of write block. This will be in two parts
	// if the block wraps around.
	hr = m_hw->Lock(m_LastBuffPosition, rssize, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);

	// If the buffer was lost, restore and retry lock.
	if (DSERR_BUFFERLOST == hr) {
		m_hw->Restore();
		hr = m_hw->Lock(m_LastBuffPosition, rssize, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
	}
	if (SUCCEEDED(hr)) {
		// Write to pointers.
		CopyMemory(lpvPtr1, m_buff.Buffer(), dwBytes1);
		if (NULL != lpvPtr2) {
			CopyMemory(lpvPtr2, (short*)m_buff.Buffer()+dwBytes1/2, dwBytes2);
		}

		// Release the data back to DirectSound.
		hr = m_hw->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
		if (SUCCEEDED(hr)) {
			// Success.
			m_LastBuffPosition+= rssize;
			m_LastBuffPosition%= m_aDD.Len*m_aDD.Num;
			m_Samples += rssize / m_aDD.wf.nBlockAlign;
			return true;
		}
	}
	// Lock, Unlock, or Restore failed.
	return false;
}

/**
 **************************************************************************
 * \param		data		[in] noise data.
 * \param		PendTime	[in] length of data
 * \param		Pitch		[in] position marker
 * \return true if write was successful
 ****************************************************************************/
bool VS_AudioDSOutNative::OutWriteNoise(short* data, int PendBytes)
{
	LPVOID  lpvPtr1;
	DWORD dwBytes1;
	LPVOID  lpvPtr2;
	DWORD dwBytes2;
	HRESULT hr;

	int rssize = m_rsn.Process(data, (void*)m_buff.Buffer(), PendBytes, m_aDD2.wf.nSamplesPerSec, m_aDD.wf.nSamplesPerSec);

	// Obtain memory address of write block. This will be in two parts
	// if the block wraps around.
	int Noff = m_LastBuffPosition + m_aDD.wf.nSamplesPerSec*2*2 - m_aDD.Len; // 2 sec - 1 frame
	Noff%= m_aDD.Len*m_aDD.Num;
	hr = m_hw->Lock(Noff, rssize, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);

	// If the buffer was lost, restore and retry lock.
	if (DSERR_BUFFERLOST == hr) {
		m_hw->Restore();
		hr = m_hw->Lock(Noff, rssize, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
	}
	if (SUCCEEDED(hr)) {
		// Write to pointers.
		CopyMemory(lpvPtr1, m_buff.Buffer(), dwBytes1);
		if (NULL != lpvPtr2) {
			CopyMemory(lpvPtr2, (short*)m_buff.Buffer()+dwBytes1/2, dwBytes2);
		}

		// Release the data back to DirectSound.
		hr = m_hw->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
		if (SUCCEEDED(hr)) {
			return true;
		}
	}
	// Lock, Unlock, or Restore failed.
	return false;
}

/****************************************************************************
 * VS_AudioDSIn
 ****************************************************************************/
/**
 **************************************************************************
 * \param		device	[in] pointer to LPDIRECTSOUNDCAPTUREBUFFER8 interface
 * \param		aDD		[in] requsted device description
 ****************************************************************************/
VS_AudioDSInNative::VS_AudioDSInNative(VS_AudioDeviceDesc &aDD) : m_fifo(aDD.wf.nSamplesPerSec*2) // 1 sec
{
	GUID guid;
	int freq = 0;
	//if (aDD.dev==-1) { // default device
	//	g_GetDeviceID(&g_cpt[0].m_guid, &guid);
	//	int size = g_cpt.size();
	//	for (int i = 1; i< size; i++)
	//		if (g_cpt[i].m_guid==guid)
	//			freq = g_cpt[i].m_maxfreq;
	//}
	//else
	{
		guid = g_cpt[aDD.dev].m_guid;
		freq = g_cpt[aDD.dev].m_maxfreq;
	}
	if (freq==0)
		freq = aDD.wf.nSamplesPerSec;

	m_aDD2 = aDD;
	m_aDD = aDD;
	m_aDD.Len = freq*2/25; // 40 ms
	m_aDD.Num = (__int64)aDD.Num*aDD.Len*freq/aDD.wf.nSamplesPerSec/m_aDD.Len;
	m_aDD.wf.nSamplesPerSec = freq;
	m_aDD.wf.nAvgBytesPerSec = m_aDD.wf.nSamplesPerSec*m_aDD.wf.nBlockAlign;

	DSCBUFFERDESC dscd;
	ZeroMemory(&dscd, sizeof(dscd));
	dscd.dwSize = sizeof(dscd);
	dscd.dwBufferBytes = m_aDD.Num*m_aDD.Len;
	dscd.lpwfxFormat = &m_aDD.wf;

	m_buff.SetSize(dscd.dwBufferBytes);

	LPDIRECTSOUNDCAPTUREBUFFER ppdscb = 0;
	g_DirectSoundCaptureCreate8(&guid, &m_pDSC, 0);
	if (m_pDSC)
		m_pDSC->CreateCaptureBuffer(&dscd, &ppdscb, 0);
	if (ppdscb) {
		ppdscb->QueryInterface(IID_IDirectSoundCaptureBuffer8, (VOID**)&m_hw);
		ppdscb->Release();
		ppdscb = 0;
	}
	if (!m_hw) {
		Release();
		return;
	}

    HRESULT hr;
	DSBPOSITIONNOTIFY*  aPosNotify     = NULL;
	LPDIRECTSOUNDNOTIFY8 pDSNotify      = NULL;
    // the buffer as the sound plays.
	if( FAILED( hr = m_hw->QueryInterface( IID_IDirectSoundNotify8, (VOID**)&pDSNotify ) ) ) {
		Release();
		return;
    }

	aPosNotify = new DSBPOSITIONNOTIFY[ m_aDD.Num + 1];
	int i = 0;
    for( ; i < m_aDD.Num; i++ ){
		aPosNotify[i].dwOffset     = (m_aDD.Len * i) + m_aDD.Len - 1;
		aPosNotify[i].hEventNotify = m_aDD.Event;
    }
	aPosNotify[i].dwOffset = 0;
	aPosNotify[i].hEventNotify = 0;

    // Tell DirectSound when to notify us.
	m_IsValid = SUCCEEDED( hr = pDSNotify->SetNotificationPositions( m_aDD.Num, aPosNotify ) );
	pDSNotify->Release();
	delete[] aPosNotify;
	m_Samples = 0;
	__int64 rnd = VS_GenKeyByMD5();
	__int64 dt = (rnd * 600 * m_aDD.wf.nSamplesPerSec) >> 32;
	m_restart_samples = (6 * 3600 - 300) * m_aDD.wf.nSamplesPerSec; // 6 hour for reset DS
	m_restart_samples += dt;
	Stop();
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_AudioDSInNative::GetBufferedDurr(bool is_precise)
{
	int durr =  VS_AudioDSIn::GetBufferedDurr(is_precise);
	return is_precise ? (__int64)durr*m_aDD2.wf.nSamplesPerSec/m_aDD.wf.nSamplesPerSec : durr;
}

/**
 **************************************************************************
 ****************************************************************************/
__int64 VS_AudioDSInNative::GetNumSamples()
{
	return m_Samples*m_aDD2.wf.nSamplesPerSec/m_aDD.wf.nSamplesPerSec;
}

/**
 **************************************************************************
 * \param		data	[out] pointer to captured data.
 * \return true if read was successful
 ****************************************************************************/
bool VS_AudioDSInNative::InRead(short* data, int &size)
{

	VS_AutoLock lock(this);
	HRESULT hr;
	VOID* pbCaptureData  = NULL;
	DWORD dwCaptureLength;
	VOID* pbCaptureData2 = NULL;
	DWORD dwCaptureLength2;
	DWORD dwReadPos;
	LONG lLockSize;
	DWORD OneBufferSize = m_aDD.Len;
	size = 0;

	if (FAILED (hr = m_hw->GetCurrentPosition( NULL, &dwReadPos)))
		return false;

	// Lock everything between our private cursor
	// and the read cursor, allowing for wraparound.
	lLockSize = dwReadPos - m_LastBuffPosition;
	if( lLockSize < 0 ) lLockSize += m_aDD.Num*m_aDD.Len;

	if( lLockSize < m_aDD.Len ) // not enouth data
		return false;

	if (FAILED(hr = m_hw->Lock(m_LastBuffPosition, lLockSize, &pbCaptureData, &dwCaptureLength, &pbCaptureData2, &dwCaptureLength2, 0L)))
		return false;

	if (pbCaptureData2==0) {
		if (dwCaptureLength < OneBufferSize)
			return false;
		else
			memcpy((short*)m_buff.Buffer(), pbCaptureData, OneBufferSize);
	}
	else {
		if (dwCaptureLength < OneBufferSize) {
			if (dwCaptureLength + dwCaptureLength2 < OneBufferSize)
				return false;
			else {
				memcpy((short*)m_buff.Buffer(), pbCaptureData, dwCaptureLength);
				memcpy((short*)m_buff.Buffer() + dwCaptureLength/2, pbCaptureData2, OneBufferSize - dwCaptureLength);
			}
		}
		else {
			memcpy((short*)m_buff.Buffer(), pbCaptureData, OneBufferSize);
		}
	}

	// Unlock the capture buffer.
	m_hw->Unlock( pbCaptureData, dwCaptureLength, pbCaptureData2, dwCaptureLength2);

	// Move the capture offset along.
	m_LastBuffPosition+= m_aDD.Len;
	m_LastBuffPosition%= m_aDD.Len*m_aDD.Num;
	m_Samples += m_aDD.Len / m_aDD.wf.nBlockAlign;
	size = OneBufferSize;

	size = m_rs.Process((short*)m_buff.Buffer(), data, OneBufferSize, m_aDD.wf.nSamplesPerSec, m_aDD2.wf.nSamplesPerSec);
	if (size > 0) {
		m_fifo.AddData(data, size);
	}

	if (m_fifo.GetDataLen() < m_aDD2.Len)
		size = 0;
	else {
		size = m_aDD2.Len;
		m_fifo.GetData(data, size);
	}

	return size > 0;
}

/****************************************************************************
 * VS_AudioMuxOut
 ****************************************************************************/

/**
 **************************************************************************
 * \param		aDD		[in] requsted device description
 ****************************************************************************/
VS_AudioMuxOut::VS_AudioMuxOut(VS_AudioDeviceDesc &aDD, HWND hwnd)
{
	m_dtype = ADTYPE_FDMUXOUT;
	m_pDevice = VS_AudioMuxerDevice::GetDeviceMuxer(aDD, hwnd, this);
	m_IsValid = m_pDevice->IsValid();
};

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioMuxOut::~VS_AudioMuxOut()
{
	VS_AudioMuxerDevice::DestroyDeviceMuxer(this);
};

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioMuxOut::Start()
{
	m_state = m_pDevice->Start();
};

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioMuxOut::Stop()
{
	m_state = m_pDevice->Stop();
};

/**
 **************************************************************************
 ****************************************************************************/
int VS_AudioMuxOut::GetBufferedDurr(bool is_precise)
{
	return m_pDevice->GetBufferedDurr(this, is_precise);
};

/**
 **************************************************************************
 * \param		data		[in] audio data for playing.
 * \param		PendTime	[in] duration of pended data in device
 * \return true if write was successful
 ****************************************************************************/
bool VS_AudioMuxOut::OutWrite(short* data, int len)
{
	return m_pDevice->PutAudio(this, data, len);
};

/**
 **************************************************************************
 * \param		data		[in] noise data.
 * \param		PendTime	[in] length of data
 * \return true if write was successful
 ****************************************************************************/
bool VS_AudioMuxOut::OutWriteNoise(short* data, int PendBytes)
{
	return m_pDevice->PutNoise(this, data, PendBytes);
};

/****************************************************************************
 * VS_AudioStreamMuxer
 ****************************************************************************/

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioStreamMuxer::VS_AudioStreamMuxer(int iMuxerSampleRate, int BuffLen)
{
	m_iMuxerSampleRate = iMuxerSampleRate;
	m_iBuffLen = BuffLen;
	m_mapMuxer.clear();
	m_pRsmplBuff = (unsigned char*)malloc(m_iMuxerSampleRate*2*sizeof(short)); // 2 sec
	m_pMuxerBuff = (int*)malloc(m_iBuffLen*sizeof(int));
	srand(timeGetTime());
	int ammode=AMM_FRAMENORM;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	key.GetValue(&ammode, sizeof(int), VS_REG_INTEGER_VT, "mix mode");
	if((ammode<=4)&&(ammode>=1))
		mixer.Init(static_cast <VS_AudioMixerMode> (ammode));
};

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioStreamMuxer::~VS_AudioStreamMuxer()
{
	if (m_pRsmplBuff) free(m_pRsmplBuff);
	if (m_pMuxerBuff) free(m_pMuxerBuff);
};

/**
 **************************************************************************
 ****************************************************************************/
bool VS_AudioStreamMuxer::AddAudioStream(void *handle, VS_AudioDeviceDesc &aDD)
{
	VS_AutoLock lock(this);
	muxer_iter iter = m_mapMuxer.find(handle);
	if (iter != m_mapMuxer.end()) return false;
	VS_AudioMuxerState st;
	memset(&st, 0, sizeof(VS_AudioMuxerState));
	st.pResampler = new VS_AudioReSamplerSpeex();
	st.iStreamSampleRate = aDD.wf.nSamplesPerSec;
	st.iBlockAllign = aDD.wf.nBlockAlign;
	st.pBuffer = new VS_AudioBuff();
	st.pBuffer->AddConstBytes(0, 2*st.iStreamSampleRate*st.iBlockAllign); // 2 sec init
	st.pBuffer->Reset();
	st.iNoiseLen = aDD.wf.nSamplesPerSec*sizeof(short);
	st.pBufferNoise = (short*)malloc(st.iNoiseLen); // 1 sec
	memset(st.pBufferNoise, 0, st.iNoiseLen);
	st.iLastFillAudio = -1;
	m_mapMuxer.emplace(handle, st);

	return true;
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_AudioStreamMuxer::RemoveAudioStream(void *handle)
{
	VS_AutoLock lock(this);

	muxer_iter iter = m_mapMuxer.find(handle);
	if (iter != m_mapMuxer.end()) {
		if (iter->second.pBuffer) delete iter->second.pBuffer; iter->second.pBuffer = 0;
		if (iter->second.pResampler) delete iter->second.pResampler; iter->second.pResampler = 0;
		if (iter->second.pBufferNoise) free(iter->second.pBufferNoise); iter->second.pBufferNoise = 0;
		m_mapMuxer.erase(iter);
	}

	return (m_mapMuxer.size() > 0);
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_AudioStreamMuxer::PutAudio(void *handle, short* data, int len)
{
	VS_AutoLock lock(this);
	muxer_iter iter = m_mapMuxer.find(handle);
	if (iter == m_mapMuxer.end()) return false;
	VS_AudioMuxerState *st = &(iter->second);
	int samples = len / 2;
	if (st->iStreamSampleRate != m_iMuxerSampleRate) {
		long rbytes = st->pResampler->Process(data, m_pRsmplBuff, samples*2, st->iStreamSampleRate, m_iMuxerSampleRate);
		st->pBuffer->Add(m_pRsmplBuff, rbytes);
	} else {
		st->pBuffer->Add((unsigned char*)data, samples * 2);
	}
	st->iLastPosition = st->pBuffer->Bytes();
	return true;
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_AudioStreamMuxer::PutNoise(void *handle, short* data, int len)
{
	VS_AutoLock lock(this);
	muxer_iter iter = m_mapMuxer.find(handle);
	if (iter == m_mapMuxer.end()) return false;
	VS_AudioMuxerState *st = &(iter->second);
	st->iLastNoisePosition = 0;
	memcpy(st->pBufferNoise, data, std::min(len, st->iNoiseLen));
	return true;
}

/**
 **************************************************************************
 ****************************************************************************/
#define CLIP(val) ( ( (val) > 32767 ) ? 32767 : ( ( (val) < -32768 ) ? -32768 : (val) ) )
int VS_AudioStreamMuxer::GetMuxStream(short* data)
{
	VS_AutoLock lock(this);

	int k = 0;
	unsigned int ct = timeGetTime();
	int numStreams = m_mapMuxer.size();
	if (numStreams < 1) return -1;
	if (numStreams > 1) {
		short *pD[1024] = {0};
		int actualSize[1024] = {0};
		for (muxer_iter i = m_mapMuxer.begin(), e = m_mapMuxer.end(); i != e;) {
			VS_AudioMuxerState *st = &(i->second);
			short *pAudio = (short*)(st->pBuffer->Buff());
			actualSize[k] = std::min(m_iBuffLen, st->iLastPosition);
			pD[k] = pAudio;
			int dl = m_iBuffLen - actualSize[k];
			if (dl > 0) {
				if (ct - (unsigned int)st->iLastFillAudio < 2000) {
					int pos = (st->iLastNoisePosition % (st->iNoiseLen - m_iBuffLen)) / 2;
					memcpy(pD[k] + actualSize[k]/2, st->pBufferNoise + pos,dl);
					st->iLastNoisePosition += dl;
				} else {
					memset(pD[k] + actualSize[k]/2, 0, dl);
				}
			}
			i++;
			k++;
		}

		mixer.Mix(pD, numStreams, data, m_iBuffLen/2);     ///// mixing

		k = 0;
		for (muxer_iter i = m_mapMuxer.begin(), e = m_mapMuxer.end(); i != e;) {
			VS_AudioMuxerState *st = &(i->second);
			st->pBuffer->TruncLeft(actualSize[k]);
			st->iLastPosition = st->pBuffer->Bytes();
			if (st->iLastPosition != 0) st->iLastFillAudio = (__int64)ct;
			i++;
			k++;
		}

	} else {
		int actualSize = 0;
		muxer_iter i = m_mapMuxer.begin();
		VS_AudioMuxerState *st = &(i->second);
		actualSize = std::min(m_iBuffLen, st->iLastPosition);
		memcpy(data, st->pBuffer->Buff(), actualSize);
		int dl = m_iBuffLen - actualSize;
		if (dl > 0) {
			if (ct - (unsigned int)st->iLastFillAudio < 2000) {
				int pos = (st->iLastNoisePosition % (st->iNoiseLen - m_iBuffLen)) / 2;
				memcpy(data + actualSize/2, st->pBufferNoise + pos, dl);
				st->iLastNoisePosition += dl;
			} else {
				memset(data + actualSize/2, 0, dl);
			}
		}
		st->pBuffer->TruncLeft(actualSize);
		st->iLastPosition = st->pBuffer->Bytes();
		if (st->iLastPosition != 0) st->iLastFillAudio = (__int64)ct;
	}

	if(VS_ARenderDevice::m_muteAll==1)
		memset(data,0,m_iBuffLen);
	__int64 s = 0;
	for(int i=0; i<m_iBuffLen/2; i++)
		s+= data[i]*data[i];
	if(m_iBuffLen!=0)
		CRenderAudioDevices::m_LevelOut = (int)sqrt((double)2*s/m_iBuffLen);

	return m_iBuffLen;
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_AudioStreamMuxer::GetBufferedDurr(void *handle, bool is_precise)
{
	VS_AutoLock lock(this);
	muxer_iter iter = m_mapMuxer.find(handle);
	if (iter == m_mapMuxer.end()) return false;
	VS_AudioMuxerState *st = &(iter->second);
	int streamDurr = iter->second.iLastPosition;
	if (!is_precise) {
		streamDurr = streamDurr / st->iBlockAllign * 1000 / st->iStreamSampleRate;
	} else {
		streamDurr = streamDurr / st->iBlockAllign;
	}
	return streamDurr;
}

/****************************************************************************
 * VS_AudioMuxerDevice
 ****************************************************************************/

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioMuxerDevice* VS_AudioMuxerDevice::GetDeviceMuxer(VS_AudioDeviceDesc &aDD, HWND hwnd, void *hStream)
{
	VS_AutoLock lock(&m_Lock);
	if (!m_MuxerInstance) m_MuxerInstance = new VS_AudioMuxerDevice(aDD, hwnd);
	m_MuxerInstance->OpenAudioStream(hStream, aDD);
	return m_MuxerInstance;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioMuxerDevice::DestroyDeviceMuxer(void *hStream)
{
	VS_AutoLock lock(&m_Lock);
	if (m_MuxerInstance->CloseAudioStream(hStream) == false) {
		delete m_MuxerInstance; m_MuxerInstance = NULL;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioMuxerDevice::VS_AudioMuxerDevice(VS_AudioDeviceDesc &aDD, HWND hwnd)
{
	int iBuffMs = 20;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	key.GetValue(&iBuffMs, sizeof(int), VS_REG_INTEGER_VT, "BuffDurrMs");

	m_bValid = false;
	m_hComplEv = CreateEvent(0, 0, 0, 0);
	VS_AudioDeviceDesc desc;
	desc = aDD;
	//desc.wf.nSamplesPerSec = 16000;  //
	desc.Len = (iBuffMs * desc.wf.nSamplesPerSec * desc.wf.nBlockAlign) / 1000;
	desc.Num = (aDD.Num * aDD.Len) / desc.Len;
	desc.wf.nAvgBytesPerSec = desc.wf.nSamplesPerSec * desc.wf.nBlockAlign;
	desc.Event = m_hComplEv;
	//m_pDevice = new VS_AudioDSOutNative(desc, hwnd);
	m_pDevice = !aDD.IsVista ? new VS_AudioDSOutNative(desc, hwnd) : new VS_AudioDSOut(desc, hwnd);
	m_IsVista=aDD.IsVista;
	m_pFrequencyStat = NULL;
	m_pAudioMuxer = NULL;
	if (m_pDevice->IsValid()) {
		m_pFrequencyStat = new VS_FrequencyStat;
		m_pFrequencyStat->Init(desc.wf.nSamplesPerSec, (desc.IsVista) ? 0.001 : 0.04, 10000);
		m_pFrequencyStat->pFreqDeviation = new VS_FreqDeviation();
		m_pFrequencyStat->pFreqDeviation->Init(500);
		m_pFrequencyStat->pFreqDeviation->Clear();
		m_pAudioMuxer = new VS_AudioStreamMuxer(desc.wf.nSamplesPerSec, desc.Len);
		m_bValid = (m_pAudioMuxer && m_pFrequencyStat);
		if (m_bValid && !IsThreadActiv()) {
			ActivateThread(this);
			SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);
		}
	}
}

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioMuxerDevice::~VS_AudioMuxerDevice()
{
	m_bValid = false;
	DesactivateThread();
	if (m_pFrequencyStat) {
		delete m_pFrequencyStat->pFreqDeviation;
		m_pFrequencyStat->pFreqDeviation = NULL;
	}
	delete m_pFrequencyStat; m_pFrequencyStat = NULL;
	delete m_pAudioMuxer; m_pAudioMuxer = NULL;
	delete m_pDevice; m_pDevice = NULL;
	CloseHandle(m_hComplEv);
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_AudioMuxerDevice::OpenAudioStream(void *hStream, VS_AudioDeviceDesc &aDD)
{
	if (!m_bValid) return false;
	return m_pAudioMuxer->AddAudioStream(hStream, aDD);
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_AudioMuxerDevice::CloseAudioStream(void *hStream)
{
	if (!m_bValid) return false;
	return m_pAudioMuxer->RemoveAudioStream(hStream);
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_AudioMuxerDevice::PutAudio(void *hStream, short* data, int len)
{
	VS_AutoLock lock(&m_Lock);
	if (!m_bValid) return false;
	return m_pAudioMuxer->PutAudio(hStream, data, len);
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_AudioMuxerDevice::PutNoise(void *hStream, short* data, int len)
{
	VS_AutoLock lock(&m_Lock);
	if (!m_bValid) return false;
	return m_pAudioMuxer->PutNoise(hStream, data, len);
}

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioDevice_State VS_AudioMuxerDevice::Start()
{
	VS_AutoLock lock(&m_Lock);
	if (!m_bValid) return ADSTATE_STOP;
	if (m_pDevice->State() != ADSTATE_START) m_pDevice->Start();
	return m_pDevice->State();
}

/**
 **************************************************************************
 ****************************************************************************/
VS_AudioDevice_State VS_AudioMuxerDevice::Stop()
{
	VS_AutoLock lock(&m_Lock);
	if (!m_bValid) return ADSTATE_STOP;
	if (m_pDevice->State() == ADSTATE_START) m_pDevice->Stop();
	return m_pDevice->State();
}

/**
 **************************************************************************
 ****************************************************************************/
int VS_AudioMuxerDevice::GetBufferedDurr(void *hStream, bool is_precise)
{
	VS_AutoLock lock(&m_Lock);
	if (!m_bValid) return 0;
	int stream_durr = m_pAudioMuxer->GetBufferedDurr(hStream, is_precise);
	int dev_durr = m_pDevice->GetBufferedDurr(is_precise);
	dev_durr = std::max(0, dev_durr + stream_durr);
	return dev_durr;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_AudioMuxerDevice::CalcFrequency(int iBuffDurr)
{
	LARGE_INTEGER curt;
	QueryPerformanceCounter(&curt);
	int dbuff = 0;
	if (iBuffDurr <= dbuff && m_pFrequencyStat->start_pos_buffer != 0) {
		DTRACE(VSTM_ECHO, "render zero buffer, %d samples", iBuffDurr);
		return;
	}
	if (m_pFrequencyStat->in_time.QuadPart == 0) {
		m_pFrequencyStat->in_time.QuadPart = curt.QuadPart + 3 * m_pFrequencyStat->freq.QuadPart;
	}
	double dt = 1000.0 * (curt.QuadPart - m_pFrequencyStat->in_time.QuadPart) / (double)(m_pFrequencyStat->freq.QuadPart);
	if (dt >= 0) {
		if (m_pFrequencyStat->start_pos_buffer == 0) {
			m_pFrequencyStat->start_pos_buffer = iBuffDurr;
			m_pFrequencyStat->prev_buff_durr = iBuffDurr;
			m_pFrequencyStat->start_samples = m_pDevice->GetNumSamples();
			m_pFrequencyStat->in_time = curt;
			m_pFrequencyStat->dt_calc = dt;
		} else if (m_pFrequencyStat->prev_buff_durr != iBuffDurr) {
			__int64 num_smpls = m_pDevice->GetNumSamples() - m_pFrequencyStat->start_samples -
								(iBuffDurr - m_pFrequencyStat->start_pos_buffer);
			m_pFrequencyStat->pFreqDeviation->Snap(dt, (double)num_smpls);
			m_pFrequencyStat->prev_buff_durr = iBuffDurr;
		}
		if ((dt - m_pFrequencyStat->dt_calc) >= m_pFrequencyStat->iCalcRange) {
			double freq = m_pFrequencyStat->pFreqDeviation->GetA1() * 1000.0;
			if (freq > 0.0) {
				if (fabs((freq - m_pFrequencyStat->fFrequencyInit) / m_pFrequencyStat->fFrequencyInit) <= m_pFrequencyStat->fDeltaFrequency)
					m_pFrequencyStat->fFrequency = freq;
			}
			DTRACE(VSTM_ECHO, "rnd_freq = %8.2f, dt = %8.2f", m_pFrequencyStat->fFrequency, dt);
			m_pFrequencyStat->is_first	 = false;
			m_pFrequencyStat->dt_calc = dt;
		}
	}
}

/**
 **************************************************************************
 ****************************************************************************/
DWORD VS_AudioMuxerDevice::Loop(LPVOID hEvDie)
{
	bool exit = false;
	HANDLE Handles[2] = { hEvDie, m_hComplEv };

	int i = 0;
	int freq = m_pAudioMuxer->GetSampleRate();
	int buffLen = m_pAudioMuxer->GetBufferLenght();

	int BuffConst = 5;
//	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
//	key.GetValue(&BuffConst, 4, VS_REG_INTEGER_VT, "SafeAudioBuff");

	int BoudnBuffLen = BuffConst * freq*2/1000 + buffLen; // bytes
	int averBC = 40 * freq/1000; // 40 ms at init

	short *pAudio = (short*)malloc(10 * buffLen * sizeof(short)); // 10 sec
	if(!m_IsVista)
	{
		m_pDevice->Stop();    //hack for bug 17846
		m_pDevice->Start();
	}
	do {
		DWORD waitRes = WaitForMultipleObjects(2, Handles, FALSE, 100);
		switch(waitRes) {
			case WAIT_FAILED:
			case WAIT_OBJECT_0 + 0: /// End Of Thread
				{
					exit = true;
					break;
				}
			case WAIT_OBJECT_0 + 1:
				{
					int offset = 0;
					int buff_durr = m_pDevice->GetBufferedDurr(offset);
					if (offset > averBC)
						averBC = (int)(averBC * 0.75f + offset*0.25f + 0.5f);
					else
						averBC = (int)(averBC * 0.94f + offset*0.06f + 0.5f);

					int dbuff = BoudnBuffLen - buff_durr * 2 + averBC * 2;

					int num = 0;
					if (dbuff > 0) {
						num = dbuff / buffLen;
						if (dbuff % buffLen != 0) num++;
					}
					for (i = 0; i < num; i++) {
						int sizeMux = m_pAudioMuxer->GetMuxStream(pAudio);
						if (sizeMux < 0) break; /// num streams = 0

						//VS_EchoDebugger::GetInstance().WriteData(VS_EchoDebugger::DT_FAREND, pAudio, sizeMux);

						VS_AudioDeviceManager::m_AudioProcessing.ProcessRenderAudio(pAudio, sizeMux / 2, freq, buff_durr);

						m_pDevice->OutWrite(pAudio, sizeMux);
					}
					//DTRACE(VSTM_AUDI0, "LOOP: Durr = %4d, off = %3d, aver = %3d, num = %d", buff_durr, offset, averBC, num);
					CalcFrequency(buff_durr);
					break;
				}
		}

	} while (!exit);

	free(pAudio);

	return 0;
}
