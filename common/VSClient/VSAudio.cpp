/**
**************************************************************************
* \file VSAudio.cpp
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
* \brief Implement Volume Control, TestAudio Render, AudioRenders wraper
*
* \b Project Client
* \author Melecko Ivan
* \author SMirnovK
* \date 01.11.2002
*
* $Revision: 12 $
*
* $History: VSAudio.cpp $
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 1.11.11    Time: 16:20
 * Updated in $/VSNA/VSClient
 * - new DS render implementation (for many streams - muxer + 1 DSOut
 * device + 1 aec)
 * - change method of detect max sample rate value (from ds guid)
 * - change hardware test,  TestAudio
 * - some refactoring audio + delete DMO audio
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 23.08.10   Time: 22:19
 * Updated in $/VSNA/VSClient
 * - long names in devices
 * - corrected Wide names for devices
 * - init devices section rewrited
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 22.04.10   Time: 20:47
 * Updated in $/VSNA/VSClient
 * - set boost option for microphone
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 8.04.10    Time: 16:29
 * Updated in $/VSNA/VSClient
 * - fix hardware test for AEC DMO
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 6.04.10    Time: 18:04
 * Updated in $/VSNA/VSClient
 * - were enabled DMO AEC ("EnableDMO" in registry)
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 10.03.10   Time: 20:45
 * Updated in $/VSNA/VSClient
 * - fix bug 7035
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 27.10.09   Time: 16:12
 * Updated in $/VSNA/VSClient
 * - fix reenumerate DS devices list
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 3.06.09    Time: 11:52
 * Updated in $/VSNA/VSClient
 * - audiorender cleanup
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 12.03.09   Time: 18:21
 * Updated in $/VSNA/VSClient
 * - posible problem with mic
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
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 24.12.07   Time: 17:37
 * Updated in $/VS2005/VSClient
 * - master volume added
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 9.11.07    Time: 18:54
 * Updated in $/VS2005/VSClient
 * - bugfix #3535
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 89  *****************
 * User: Smirnov      Date: 15.12.06   Time: 17:18
 * Updated in $/VS/VSClient
 * - level and volume returned in audiotest
 *
 * *****************  Version 88  *****************
 * User: Smirnov      Date: 29.09.06   Time: 14:02
 * Updated in $/VS/VSClient
 * - programm audio mute
 *
 * *****************  Version 87  *****************
 * User: Smirnov      Date: 25.07.06   Time: 18:09
 * Updated in $/VS/VSClient
 * - Added system waveout Mute
 *
 * *****************  Version 86  *****************
 * User: Smirnov      Date: 13.07.06   Time: 18:56
 * Updated in $/VS/VSClient
 * - new audio controls (via mixer)
 *
 * *****************  Version 85  *****************
 * User: Smirnov      Date: 6.05.06    Time: 16:24
 * Updated in $/VS/VSClient
 * - new audio hardware test interface to gui
 *
 * *****************  Version 84  *****************
 * User: Smirnov      Date: 25.04.06   Time: 18:27
 * Updated in $/VS/VSClient
 * - removed answer-to-die event in Loop() functions
 *
 * *****************  Version 83  *****************
 * User: Smirnov      Date: 20.04.06   Time: 13:46
 * Updated in $/VS/VSClient
 * - new audio hardware test
 *
 * *****************  Version 82  *****************
 * User: Smirnov      Date: 5.04.06    Time: 17:24
 * Updated in $/VS/VSClient
 * - low-level audio devices
 * - Direct Sound devices added
 *
 * *****************  Version 81  *****************
 * User: Smirnov      Date: 16.02.06   Time: 12:15
 * Updated in $/VS/VSClient
 * - new TheadBase class
 * - receiver now can be inited while in conference
 *
 * *****************  Version 80  *****************
 * User: Smirnov      Date: 15.11.05   Time: 12:32
 * Updated in $/VS/VSClient
 * - multi video codecs support
 *
 * *****************  Version 79  *****************
 * User: Smirnov      Date: 12.09.05   Time: 14:41
 * Updated in $/VS/VSClient
 * - added new codecs support in client: g728, g729a, g722.1
 *
 * *****************  Version 78  *****************
 * User: Smirnov      Date: 27.05.05   Time: 16:08
 * Updated in $/VS/VSClient
 * aded new IPP ver 4.1
 * added g711, g728, g729 from IPP
 *
 * *****************  Version 77  *****************
 * User: Melechko     Date: 25.01.05   Time: 16:47
 * Updated in $/VS/VSClient
 * Device list array HighBound fix
 *
 * *****************  Version 76  *****************
 * User: Admin        Date: 16.12.04   Time: 20:08
 * Updated in $/VS/VSClient
 * doxigen comments
*
* *****************  Version 75  *****************
* User: Smirnov      Date: 30.11.04   Time: 20:54
* Updated in $/VS/VSClient
* removed aold audio
****************************************************************************/


/****************************************************************************
* Includes
****************************************************************************/
#include <math.h>
#include <stdio.h>
#include "VSAudio.h"
#include "Transcoder/VSAudioVad.h"
#include "../Audio/VoiceActivity/VS_Mixer.h"
#include "Audio/WinApi/dsutil.h"
#include "../std/cpplib/VS_MediaFormat.h"
#include "../VSClient/VS_ApplicationInfo.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "std-generic/cpplib/utf8.h"
#include "AudioProcessing/VSEchoDelayDetector.h"
#include "VSAudioEchoCancel.h"

extern VS_GlobalEcho* g_Echo;

int CRenderAudioDevices::m_LevelOut=0;

/**
 **************************************************************************
 ****************************************************************************/
CRenderAudioDevices::CRenderAudioDevices(CVSInterface* pParentInterface):
CVSInterface("AudioPlayback",pParentInterface)
{
	m_pModeList = 0;
	m_VolumeControl = VS_VolControlBase::Factory();
}


/**
 **************************************************************************
 ****************************************************************************/
CRenderAudioDevices::~CRenderAudioDevices()
{
	delete m_VolumeControl;
}


/**
 **************************************************************************
 * See CVSInterface::ProcessFunction. Operations:
 *	"AudioRenderList"	"SystemVolume"	"TestPlayback"	"PrepareAudio"
 *
 ****************************************************************************/
int CRenderAudioDevices::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;
	if (strcmp(pSection, "PrepareAudio")==0) {
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			OSVERSIONINFOEX osvi;
			ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
			if (GetVersionEx((OSVERSIONINFO *)&osvi)) {
				if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion == 6 )
					return VS_INTERFACE_OK;
			}
			//VS_MixerSetFeature(VS_MIXER_VOLUME_ALL|VS_MIXER_VOLUME_MIC|VS_MIXER_BOOST_MIC);
			return VS_INTERFACE_OK;
		}
		return VS_INTERFACE_INTERNAL_ERROR;
	}
	else  if(strcmp(pSection, "AudioRenderList")==0){
		switch(VS_OPERATION)
		{
		case GET_PARAM:{
			long i;
			SAFEARRAYBOUND rgsabound[1];
			SAFEARRAY * psa;
			int n=0;
			rgsabound[0].lLbound = 0;
			n= m_pSourceList->iGetMaxString();
			if(n<0)
				n=0;
			rgsabound[0].cElements = n;//m_pSourceList->iGetMaxString();
			psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
			if(psa == NULL)
				return VS_INTERFACE_INTERNAL_ERROR;
			var->parray=psa;
			var->vt= VT_ARRAY | VT_VARIANT;
			for(i=0;i<m_pSourceList->iGetMaxString();i++){
				_variant_t var_;
				var_=m_pSourceList->szGetStringByNumber(i);
				SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
			}
			return VS_INTERFACE_OK;
					   }
		case RUN_COMMAND:{
			if(iGetDeviceList()!=0)
				return VS_INTERFACE_INTERNAL_ERROR;
			return VS_INTERFACE_OK;
					  }
		}
	}
	else if (strcmp(pSection, "SystemVolume")==0){
		switch(VS_OPERATION)
		{
			case GET_PARAM:
			{
				float vol;
				m_VolumeControl->GetMicVolume(&vol);
				*var = int(vol * 65535.0f);
				return VS_INTERFACE_OK;
			}
			case SET_PARAM:
			{
				m_VolumeControl->SetMicVolume(float(*var) / 65535.0f);
				return VS_INTERFACE_OK;
			}
			case RUN_COMMAND:
			{
				m_VolumeControl->Init(m_pSourceList->szGetStringByNumber(*var), (int)(*var), true);
				return VS_INTERFACE_OK;
			}
		}
	}
	else if (strcmp(pSection, "MasterVolume")==0){
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			*var = m_MasterVolume.GetVolume();
			return VS_INTERFACE_OK;
		case SET_PARAM:
			m_MasterVolume.SetVolume(*var);
			return VS_INTERFACE_OK;
		case RUN_COMMAND:
			m_MasterVolume.Init(int(*var), VS_AudioMixerVolume::MTYPE_MASTER);
			return VS_INTERFACE_OK;
		}
	}
	else if (strcmp(pSection, "SystemMute")==0){
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			*var = VS_ARenderDevice::m_muteAll;
			return VS_INTERFACE_OK;
		case SET_PARAM:
			VS_ARenderDevice::m_muteAll = *var;
			return VS_INTERFACE_OK;
		}
	}
	else if (strcmp(pSection, "TestPlayback") == 0) {
		if (VS_OPERATION == RUN_COMMAND) {
			if (var->vt == (VT_ARRAY | VT_VARIANT)) {
				long l, u;
				SAFEARRAY *psa = var->parray;
				SafeArrayGetLBound(psa, 1, &l);
				SafeArrayGetUBound(psa, 1, &u);
				int num = u - l + 1;
				if (num == 3) {
					long i;
					VARIANT *vars = new VARIANT[num];
					for (i = 0; i < num; ++i) {
						VariantInit(&vars[i]);
						SafeArrayGetElement(psa, &i, &vars[i]);
					}
					VS_MediaFormat fmt;
					fmt.SetAudio(vars[0].lVal, VS_ACODEC_PCM);

					int renderID = vars[1].lVal;
					int captureID = vars[2].lVal;
					VSTestAudio::ETestMode mode = VSTestAudio::TM_NONE;

					if (captureID >= 0 && renderID >= 0)
						mode = VSTestAudio::TM_CAPTURE_RENDER;
					else if (captureID >= 0)
						mode = VSTestAudio::TM_CAPTURE;
					else if (renderID >= 0)
						mode = VSTestAudio::TM_RENDER;

					DWORD res = m_TestAudio.Start(&fmt, renderID, captureID, mode);
					delete[] vars;
					if (res == 0)
						return VS_INTERFACE_OK;
				}
			}
			else {
				m_TestAudio.Stop();
				return VS_INTERFACE_OK;
			}
		}
		else if (VS_OPERATION == SET_PARAM) {
			m_TestAudio.SetVolume(int(*var));
			return VS_INTERFACE_OK;
		}
		else if (VS_OPERATION == GET_PARAM) {
			*var = (int)((m_TestAudio.GetLevel() & 0xffff) | (m_TestAudio.GetVolume() << 16));
			return VS_INTERFACE_OK;
		}
		return VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (strcmp(pSection, "TestEchoDelay") == 0) {
		if (VS_OPERATION == RUN_COMMAND) {
			if (var->vt == (VT_ARRAY | VT_VARIANT)) {
				long l, u;
				SAFEARRAY *psa = var->parray;
				SafeArrayGetLBound(psa, 1, &l);
				SafeArrayGetUBound(psa, 1, &u);
				int num = u - l + 1;
				if (num == 3) {
					long i;
					VARIANT *vars = new VARIANT[num];
					for (i = 0; i < num; ++i) {
						VariantInit(&vars[i]);
						SafeArrayGetElement(psa, &i, &vars[i]);
					}
					VS_MediaFormat fmt;
					fmt.SetAudio(vars[0].lVal, VS_ACODEC_PCM);

					int renderID = vars[1].lVal;
					int captureID = vars[2].lVal;
					VSTestAudio::ETestMode mode = VSTestAudio::TM_ECHO_DELAY;

					if (captureID >= 0 && renderID >= 0)
					{
						DWORD res = m_TestAudio.Start(&fmt, renderID, captureID, mode);
						delete[] vars;
						if (res == 0)
							return VS_INTERFACE_OK;
					}
				}
			}
			else {
				m_TestAudio.Stop();
				return VS_INTERFACE_OK;
			}
		}
		else if (VS_OPERATION == SET_PARAM) {
			return VS_INTERFACE_OK;
		}
		else if (VS_OPERATION == GET_PARAM) {
			*var = (int)(m_TestAudio.GetDelayTestStatus());
			return VS_INTERFACE_OK;
		}
		return VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (strcmp(pSection, "Level")==0) {
		if	(VS_OPERATION == GET_PARAM) {
			*var = m_LevelOut;
         	return VS_INTERFACE_OK;
		}
	}
	return VS_INTERFACE_NO_FUNCTION;
}


/**
 **************************************************************************
* Получает список доступных режимов воспроизведения аудио
* \param device_number  [IN] Number of audio render device
* \return  0 в случае успеха \n
*         -1 устройства с таким номером нет в списке \n
*         -2 нет совместимых режимов
 ****************************************************************************/
int CRenderAudioDevices::iGetDeviceModeList(int device_number){
	if (device_number>=0 && device_number<m_pSourceList->iGetMaxString()) {
		WAVEOUTCAPS wc;
		if(waveOutGetDevCaps(device_number, &wc, sizeof(WAVEOUTCAPS))!=MMSYSERR_NOERROR
			|| !(wc.dwFormats & (WAVE_FORMAT_1M16 | WAVE_FORMAT_2M16) ))
			return -2;
		else
			return 0;
	}
	return -1;
}

/**
 **************************************************************************
 *  Получает список доступных устройств воспроизведения аудио
 * \return 0 в случае успеха или -1 если устройств нет.
 ****************************************************************************/
int CRenderAudioDevices::iGetDeviceList(void){
	return VS_AudioDeviceManager::GetDeviceList(false, m_pSourceList);
}

/****************************************************************************
 * VSTestAudio
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
VSTestAudio::VSTestAudio()
{
	m_Mode = TM_NONE;
	m_capture = new VS_ACaptureDevice;
	m_render = new VS_ARenderDevice;
}

/**
 **************************************************************************
 ****************************************************************************/
VSTestAudio::~VSTestAudio()
{
	Stop();
	delete m_render;
	delete m_capture;
}

/**
 **************************************************************************
 * \param	RenderId	[in] audio render device Id
 * \param	CaptureId	[in] audio capture device Id
 * \return 0 if all OK.
 ****************************************************************************/
static const int StrictDur = 1200;

DWORD VSTestAudio::Start(VS_MediaFormat *fmt, int RenderId, int CaptureId, ETestMode mode)
{
	Stop();
	if (!fmt->IsAudioValid())
		return -1;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	DWORD format = 0;
	int freq = 16000;
	if (key.GetValue(&format, 4, VS_REG_INTEGER_VT, "Format")) {
		format = (format>>16)&0xff;
		if		(format==0) freq = 8000;
		else if (format==1) freq = 11025;
		else if (format==2) freq = 16000;
		else if (format==3) freq = 22050;
		else if (format==4) freq = 32000;
		else if (format==5) freq = 44100;
		else if (format==6) freq = 48000;
	}
	fmt->SetAudio(freq, fmt->dwAudioCodecTag);

	m_Mode = mode;

	if (CaptureId >= 0) {
		CStringList captList;
		VS_AudioDeviceManager::GetDeviceList(true, &captList);
		wchar_t *cd = captList.szGetStringByNumber(CaptureId);
		wcscpy(m_capture->m_CurrenDeviceName, cd);

		m_CaptureName = captList.szGetStringByNumber(CaptureId);
	}

	if (RenderId >= 0) {
		CStringList rndList;
		VS_AudioDeviceManager::GetDeviceList(false, &rndList);

		m_RenderName = rndList.szGetStringByNumber(RenderId);
	}

	if (m_Mode == TM_NONE) { // none
		return -2;
	}
	else if	(m_Mode == TM_CAPTURE) { //only capture
		if (!m_capture->Init(CaptureId, fmt))
			return -3;
	}
	else if	(m_Mode == TM_RENDER) { // only render
		VS_MediaFormat mf;
		mf = *fmt;
		mf.dwAudioCodecTag = VS_ACODEC_PCM;
		if (!m_render->Init(RenderId, &mf))
			return -4;
		m_render->SetStrictDurr(300);
	}
	else if (m_Mode == TM_CAPTURE_RENDER) { // render + capture

		if (!m_capture->Init(CaptureId, fmt) || !m_render->Init(RenderId, fmt))
			return -5;
		m_render->SetStrictDurr(StrictDur);
	}
	else if (m_Mode == TM_ECHO_DELAY)
	{
		fmt->SetAudio(16000, fmt->dwAudioCodecTag);

		if (!m_capture->Init(CaptureId, fmt) || !m_render->Init(RenderId, fmt))
			return -5;

		VS_AudioDeviceManager::m_AudioProcessing.SetEchoDelay(0);
	}

	if (!ActivateThread(this))
		return -6;
	return 0;
}


/**
 **************************************************************************
 ****************************************************************************/
DWORD VSTestAudio::Stop()
{
	DesactivateThread();
	m_render->Release();
	m_capture->Release();
	m_Mode = TM_NONE;
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int VSTestAudio::GetVolume() {
	return m_capture->GetVolume();
}

/**
 **************************************************************************
 ****************************************************************************/
void VSTestAudio::SetVolume(DWORD vol) {
	m_capture->SetVolume(vol);
}

/**
 **************************************************************************
 ****************************************************************************/
DWORD VSTestAudio::GetLevel() {
	return m_capture->GetLevel();
}

/**
**************************************************************************
****************************************************************************/
int VSTestAudio::GetDelayTestStatus()
{
	return m_DelayTestStatus;
}

/**
 **************************************************************************
 ****************************************************************************/
DWORD VSTestAudio::Loop(LPVOID hEvDie)
{
	HANDLE handles[2];
	handles[0] = hEvDie;
	char *bufffer = new char[65000];
	int len = 0;

	if (m_Mode == TM_CAPTURE) {
		handles[1] = m_capture->GetCmpleteEvent();
		m_capture->Start();

		while(true) {
			DWORD dwret = WaitForMultipleObjects(2, handles, FALSE, 500);
			if		(dwret == WAIT_OBJECT_0+0)
				break;
			else if (dwret == WAIT_OBJECT_0+1)
				while (m_capture->Capture(bufffer, len, true)>0);
		}
	}
	else if	(m_Mode == TM_RENDER) {
		handles[1] = m_render->GetCmpleteEvent();
		int pos = 0, bufflen = m_render->GetChunkLen();
		int buffmux = (m_render->GetWF()->nSamplesPerSec * 20 * m_render->GetWF()->nBlockAlign) / 1000;

		int size = m_render->GetWF()->nSamplesPerSec*5;// 2.5 sec
		char* file = new char[size];
		{
			double val, koef = 2*3.14*200;
			double k1 = 1.00, k2 = 1.74, k3 = 2.31, k4 = 3.20, k5 = 4.81;
			double a1 = 0.40, a2 = 0.72, a3 = 0.62, a4 = 0.39, a5 = 0.21;
			double ampl = 10000.;
			int i, _buffsize = size/2;
			short *_buff = (short*)file;
			for (i = 0; i<_buffsize; i++) {
				double time = (double)i/(double)(m_render->GetWF()->nSamplesPerSec+1);
				val = a1*sin(koef*k1*time) + a2*sin(koef*k2*time) + a3*sin(koef*k3*time) + a4*sin(koef*k4*time) + a5*sin(koef*k5*time);
				val = val*ampl*(exp(-pow(1.8*(time+0.2),1.5))*(4*(time+0.1)+0.3)+0.1);
				_buff[i] = (short)val;
			}
		}

		if (m_render->Type() != ADTYPE_FDMUXOUT) {
			while (m_render->GetCurrBuffDurr() < 500) {
				if (pos+bufflen > size)
					pos = 0;
				m_render->Play(file+pos, bufflen);
				pos+=bufflen;
			}
		}

		int wt = (m_render->Type() != ADTYPE_FDMUXOUT) ? 500 : 20;

		while(true) {
			DWORD dwret = WaitForMultipleObjects(2, handles, FALSE, wt);
			if		(dwret == WAIT_OBJECT_0+0)
				break;
			else if (dwret == WAIT_OBJECT_0+1) {
				while (m_render->GetCurrBuffDurr() < 300) {
					if (pos+bufflen > size)
						pos = 0;
					m_render->Play(file+pos, bufflen);
					pos+=bufflen;
				}
			} else if (m_render->Type() == ADTYPE_FDMUXOUT) {
				while (m_render->GetCurrBuffDurr() < 200) {
					if (pos+buffmux > size)
						pos = 0;
					m_render->Play(file+pos, buffmux);
					pos+=buffmux;
				}
			}
		}
	}
	else if (m_Mode == TM_CAPTURE_RENDER) {
		int chunk_len = m_capture->GetChunkLen();

		handles[1] = m_capture->GetCmpleteEvent();
		m_capture->Start();
		int StartTime = timeGetTime();

		while (true) {
			DWORD dwret = WaitForMultipleObjects(2, handles, FALSE, 500);
			if (dwret == WAIT_OBJECT_0 + 0)
				break;
			else if (dwret == WAIT_OBJECT_0 + 1)
			{
				if ((int)timeGetTime() < StartTime + StrictDur)
					continue;
				else
					Sleep(rand() % 40);

				while (m_capture->Capture(bufffer, len, true) > 0) {
					m_render->Play(bufffer, len);
				}
			}
		}
	}
	else if (m_Mode == TM_ECHO_DELAY)
	{
		std::vector<char> zeroBuffer(65000, 0);
		int chunk_len = m_capture->GetChunkLen();

		handles[1] = m_capture->GetCmpleteEvent();
		m_capture->Start();
		int StartTime = timeGetTime();

		VS_AudioDeviceManager::m_AudioProcessing.StartDelayDetectTest(0);

		m_DelayTestStatus = DTS_ON_PROCESS;

		while (true) {
			DWORD dwret = WaitForMultipleObjects(2, handles, FALSE, 500);
			if (dwret == WAIT_OBJECT_0 + 0)
				break;
			else if (dwret == WAIT_OBJECT_0 + 1)
			{
				Sleep(rand() % 40);

				int32_t echoDelay;
				VSEchoDelayDetector::EGetDelayResult getDelayRes;

				getDelayRes = VS_AudioDeviceManager::m_AudioProcessing.GetEchoDelay(echoDelay);

				if (getDelayRes == VSEchoDelayDetector::GDR_OK ||
					getDelayRes == VSEchoDelayDetector::GDR_NO_ECHO ||
					getDelayRes == VSEchoDelayDetector::GDR_MULTIPLE_ECHO)
				{
					// save value to registry
					VS_RegistryKey key(true, "EchoDelay", false, true);

					std::string result;

					if (getDelayRes == VSEchoDelayDetector::GDR_OK)
						result = "delay " + std::to_string(echoDelay - 16);
					else if (getDelayRes == VSEchoDelayDetector::GDR_NO_ECHO)
						result = "no";
					else if (getDelayRes == VSEchoDelayDetector::GDR_MULTIPLE_ECHO)
						result = "retest";

					auto devPair = vs::UTF16toUTF8Convert(m_CaptureName + L" && " + m_RenderName);
					key.SetString(result.c_str(), devPair.c_str());

					// adapt to delay
					if (getDelayRes == VSEchoDelayDetector::GDR_OK && echoDelay > 80)
					{
						VS_AudioDeviceManager::m_AudioProcessing.SetEchoDelay(echoDelay);
					}

					VS_AudioDeviceManager::m_AudioProcessing.StopDelayDetectTest();
				}

				if (getDelayRes == VSEchoDelayDetector::GDR_OK)
				{
					m_DelayTestStatus = (int16_t(echoDelay) << 16) | DTS_DELAY_FOUNDED;
				}
				else if (getDelayRes == VSEchoDelayDetector::GDR_NO_ECHO)
				{
					m_DelayTestStatus = DTS_NO_ECHO;
				}
				else if (getDelayRes == VSEchoDelayDetector::GDR_MULTIPLE_ECHO)
				{
					m_DelayTestStatus = DTS_MULTIPLE_ECHO;
				}

				while (m_capture->Capture(bufffer, len, true)>0) {
					m_render->Play(zeroBuffer.data(), len);
				}
			}
		}
	}

	delete[] bufffer;

	return 0;
}

