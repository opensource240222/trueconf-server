/**
 **************************************************************************
 * \file VSVideoCaptureSlot.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Implement
 * \b Project Client CVSVideoCaptureSlot class
 * \author Melechko Ivan
 * \date 22.12.2004
 *
 * $Revision: 15 $
 *
 * $History: VSVideoCaptureSlot.cpp $
 *
 * *****************  Version 15  *****************
 * User: Sanufriev    Date: 12.05.12   Time: 11:57
 * Updated in $/VSNA/VSClient
 * - were added mirror self view video
 *
 * *****************  Version 14  *****************
 * User: Samoilov     Date: 29.03.12   Time: 15:42
 * Updated in $/VSNA/VSClient
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/VSClient
 * - were added auto stereo mode detect
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 29.03.11   Time: 13:08
 * Updated in $/VSNA/VSClient
 * - update Capture module (STA implementation)
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 14.03.11   Time: 14:27
 * Updated in $/VSNA/VSClient
 * - change VS_MediaFormat - were added dwFps
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 5.03.11    Time: 13:16
 * Updated in $/VSNA/VSClient
 * - improve camera initialization
 * - auto modes for camera init
 * - hardware test
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 2.12.10    Time: 16:14
 * Updated in $/VSNA/VSClient
 * - increase accuracy video capture timestamp
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 4.06.10    Time: 17:36
 * Updated in $/VSNA/VSClient
 * - Direct3D Render implementation
 *
 * *****************  Version 7  *****************
 * User: Dront78      Date: 25.06.09   Time: 9:25
 * Updated in $/VSNA/VSClient
 * - removed non-qt render usage
 * - holder interface refactor
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 16.06.09   Time: 14:02
 * Updated in $/VSNA/VSClient
 * - add lock in device status query
 *
 * *****************  Version 5  *****************
 * User: Dront78      Date: 15.06.09   Time: 16:11
 * Updated in $/VSNA/VSClient
 * - fixed missed sender render
 * - fixed '>' symbol in receiver
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 7.06.08    Time: 19:21
 * Updated in $/VSNA/VSClient
 * - compile errors fix
 *
 * *****************  Version 3  *****************
 * User: Melechko     Date: 6.06.08    Time: 20:16
 * Updated in $/VSNA/VSClient
 * Add transparent render
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 20.03.08   Time: 17:03
 * Updated in $/VSNA/VSClient
 * - Video for Linux I420 support added via memory mapped files.
 * Compilation controls via #define VS_LINUX_DEVICE in VSCapture.h
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 11.05.06   Time: 13:16
 * Updated in $/VS/VSClient
 * - added stream command (alfa version)
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 2.05.06    Time: 18:41
 * Updated in $/VS/VSClient
 * - sender reinitialisation
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 25.04.06   Time: 14:05
 * Updated in $/VS/VSClient
 * - sender and it devices classes documented, code cleared
 *
 * *****************  Version 6  *****************
 * User: Melechko     Date: 28.02.05   Time: 11:45
 * Updated in $/VS/VSClient
 * Add audio event in SlotEx
 *
 * *****************  Version 5  *****************
 * User: Melechko     Date: 25.02.05   Time: 15:00
 * Updated in $/VS/VSClient
 * Add DV support
 *
 * *****************  Version 4  *****************
 * User: Melechko     Date: 24.02.05   Time: 12:39
 * Updated in $/VS/VSClient
 * Add CaptureSlotExt
 *
 * *****************  Version 3  *****************
 * User: Melechko     Date: 25.01.05   Time: 13:38
 * Updated in $/VS/VSClient
 * split input video buffer
 *
 * *****************  Version 2  *****************
 * User: Melechko     Date: 20.01.05   Time: 19:59
 * Updated in $/VS/VSClient
 * Exception fix
 *
 * *****************  Version 1  *****************
 * User: Melechko     Date: 19.01.05   Time: 17:00
 * Created in $/VS/VSClient
 * some changes :)
 *
*/
/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSVideoCaptureSlot.h"
#include "VS_Dmodule.h"
#include "../std/cpplib/VS_Protocol.h"

/****************************************************************************
 * Static
 ****************************************************************************/
const char CVideoCaptureSlotBase::_funcResetWindow[] = "ResetWindow";

const char CVideoCaptureSlotExt::_funcSetFormat[] = "SetFormat";
const char CVideoCaptureSlotExt::_funcPushData[] = "PushData";

const char CVideoCaptureSlot::_RegDeviceName[] = "DeviceName";
const char CVideoCaptureSlot::_RegDeviceChannel[] = "Channel";
const char CVideoCaptureSlot::_RegDeviceMode[] = "VideoMode";
const char CVideoCaptureSlot::_RegDeviceDeinterlace[] = "Deinterlace";
const char CVideoCaptureSlot::_RegDeviceHDSource[] = "HD Video Source";
const char CVideoCaptureSlot::_RegDeviceFixNTSC[] = "Fix NTSC";

/****************************************************************************
 * CVideoCaptureSlotBase
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
CVideoCaptureSlotBase::CVideoCaptureSlotBase(const char *szSlotName, CVSInterface* pParentInterface, TVideoWindowParams *pVParams)
:CVSInterface(szSlotName, pParentInterface, 0, true)
{
	m_cDeviceName[0] = 0;
	m_cDeviceRegName[0] = 0;
	m_bValid = false;
	m_pRenderBuffer = 0;
	m_hwnd = pVParams->hWindowHandle;

	m_pRender = CVideoRenderBase::RetrieveVideoRender(m_hwnd, this);

	m_ppRender = new DWORD;
	*(DWORD*)m_ppRender = *(DWORD*)(&m_pRender);

	pVParams->wndProc = (RENDERPROC)(m_pRender->WindowProc);
	pVParams->lpInstance = m_ppRender;
	m_sndFormat.SetZero();
	m_rcvFormat.SetZero();
	m_inputSize = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
CVideoCaptureSlotBase::~CVideoCaptureSlotBase()
{
	delete m_pRender; m_pRender = 0;
	delete m_ppRender; m_ppRender = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
void CVideoCaptureSlotBase::DrawFrame()
{
	m_pRender->m_bNewFrame = TRUE;
	m_pRender->DrawFrame(m_hwnd);
}

/**
 **************************************************************************
 ****************************************************************************/
int CVideoCaptureSlotBase::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;

	if (strncmp(pSection, _funcResetWindow, sizeof(_funcResetWindow)) == 0) {
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			ResetWindow((HWND)int(*var));
			return VS_INTERFACE_OK;
		}
	}

	return VS_INTERFACE_NO_FUNCTION;
}


/****************************************************************************
 * CVideoCaptureSlot
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
CVideoCaptureSlot::CVideoCaptureSlot(const char *szSlotName, CVSInterface* pParentInterface, TVideoWindowParams *pVParams,
									 CVideoCaptureList *pVideoCaptureList, VS_MediaFormatManager *pMediaFormatManager) : CVideoCaptureSlotBase(szSlotName,pParentInterface,pVParams)
{
	m_pInputBuffer = 0;
	m_pVideoCaptureList = pVideoCaptureList;
	m_pMediaFormatManager = pMediaFormatManager;
	m_pColorConversion = new CCaptureCConv();
	m_cDeviceName[0] = 0;
	m_cDeviceRegName[0] = 0;
	m_hConnectDevice = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hDisconnectDevice = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hGetFrame = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hProcessFrameEnd = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hCaptureMutex = CreateMutex(NULL, TRUE, NULL);
	ReleaseMutex(m_hCaptureMutex);
	m_eState = DEVICE_UNDEF;
	m_numConnectRequest = 0;
	for (unsigned int i = 0; i < CVideoCaptureList::CAPTURE_MAX; i++) {
		m_pCaptureDeviceList[i] = 0;
	}
	m_pCaptureDeviceList[CVideoCaptureList::CAPTURE_DS] = VS_CaptureDevice::Create(VS_CaptureDevice::DIRECT_SHOW, this, pVideoCaptureList);
	m_pCaptureDeviceList[CVideoCaptureList::CAPTURE_MF] = VS_CaptureDevice::Create(VS_CaptureDevice::MEDIA_FOUNDATION, this, pVideoCaptureList);
	m_pCaptureDeviceList[CVideoCaptureList::CAPTURE_SCREEN] = VS_CaptureDevice::Create(VS_CaptureDevice::SCREEN_CAPTURE, this, pVideoCaptureList);
	m_pCaptureDevice = m_pCaptureDeviceList[CVideoCaptureList::CAPTURE_MF];
	m_bScreenCapture = false;
	m_realFramerate = 3000;
	m_timestamp = 0;
	m_frameSize = 0;
	/// read dor Reg
	memset(&m_devSettings, 0, sizeof(m_devSettings));
	m_devSettings.iCheckHDInput = -1;
	_variant_t var = L"";
	ReadParam((char*)_RegDeviceName, &var);
	wcscpy(m_cDeviceName, (_bstr_t)var);
	wcscpy(m_cDeviceRegName, m_cDeviceName);
	var = 0;
	ReadParam((char*)_RegDeviceChannel, &var);
	m_devSettings.iChannel = var;
	var = 0;
	ReadParam((char*)_RegDeviceMode, &var);
	m_devSettings.iVideoMode = var;
	var = 0;
	ReadParam((char*)_RegDeviceDeinterlace, &var);
	m_devSettings.iDeinterlace = var;
	var = 0;
	if (ReadParam((char*)_RegDeviceHDSource, &var) == 0) {
		m_devSettings.iCheckHDInput = var;
	}
	var = 0;
	if (ReadParam((char*)_RegDeviceFixNTSC, &var) == 0) {
		m_devSettings.iFixNTSC = var;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
CVideoCaptureSlot::~CVideoCaptureSlot()
{
	if (m_hProcessFrameEnd) {
		SetEvent(m_hProcessFrameEnd);
		if (WaitForSingleObject(m_hCaptureMutex, INFINITE) == WAIT_OBJECT_0) {
			m_bValid = false;
			if (m_hCaptureMutex) CloseHandle(m_hCaptureMutex);
		}
	}
	if (m_hConnectDevice) CloseHandle(m_hConnectDevice);
	if (m_hDisconnectDevice) CloseHandle(m_hDisconnectDevice);
	for (unsigned int i = 0; i < CVideoCaptureList::CAPTURE_MAX; i++) {
		delete m_pCaptureDeviceList[i];
	}
	delete m_pColorConversion;
	_Release();
	if (m_hProcessFrameEnd) CloseHandle(m_hProcessFrameEnd);
	if (m_hGetFrame) CloseHandle(m_hGetFrame);
	/// write to Reg
	_variant_t var = m_devSettings.iChannel;
	WriteParam((char*)_RegDeviceChannel, &var);
	var = m_devSettings.iVideoMode;
	WriteParam((char*)_RegDeviceMode, &var);
	var = (_bstr_t)m_cDeviceRegName;
	WriteParam((char*)_RegDeviceName, &var);
}

CVideoCaptureList::eCapturerType CVideoCaptureSlot::GetTypeCapturer()
{
	VS_AutoLock lock(this);
	return m_pVideoCaptureList->GetTypeCapturer(m_cDeviceName);
}

/**
 **************************************************************************
 ****************************************************************************/
int CVideoCaptureSlot::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;

	if (strncmp(pSection, _funcResetWindow, sizeof(_funcResetWindow)) == 0) {
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			ResetWindow((HWND)int(*var));
			return VS_INTERFACE_OK;
		}
	}
	else if (strncmp(pSection, VS_CaptureDevice::_funcConnect, strlen(VS_CaptureDevice::_funcConnect)) == 0) {
		_variant_t* var = (_variant_t*)pVar;
		switch (VS_OPERATION)
		{
		case RUN_COMMAND:
			{
				VS_AutoLock lock(this);
				if (var->vt == VT_NULL) *var = L"";
				if (var->bstrVal == NULL) *var = L"none";
				bool bNone = _wcsicmp((_bstr_t)*var, L"none") == 0;
				g_DevStatus.SetStatus(DVS_SND_NOTCHOSEN, false, bNone);
				WriteParam((char*)_RegDeviceName, var);
				wcscpy(m_cDeviceRegName, (_bstr_t)*var);
				wcscpy(m_cDeviceName, m_cDeviceRegName);
				m_devSettings.hwndProp = 0;
				*var = -1;
				if (!bNone) {
					CVideoCaptureList::eDeviceType deviceType = m_pVideoCaptureList->ParseDeviceName(m_cDeviceName);
					m_pMediaFormatManager->SetControlExternal(CTRL_EXT_VIDEODEVICENAME, m_cDeviceName);
					*var = 0;
					m_eState = DEVICE_CONNECT;
					m_numConnectRequest++;
					CVideoCaptureList::eCapturerType type = m_pVideoCaptureList->GetTypeCapturer(m_cDeviceName);
					if (m_pCaptureDevice != m_pCaptureDeviceList[type]) {
						m_pCaptureDevice->Disconnect();
					}
					m_pCaptureDevice = m_pCaptureDeviceList[type];
					SetEvent(m_hConnectDevice);
				}
				return VS_INTERFACE_OK;
			}
		}
	}
	else if (strncmp(pSection, VS_CaptureDevice::_funcDisconnect, strlen(VS_CaptureDevice::_funcDisconnect)) == 0) {
		_variant_t* var = (_variant_t*)pVar;
		switch (VS_OPERATION)
		{
		case RUN_COMMAND:
			{
				VS_AutoLock lock(this);
				m_eState = DEVICE_DISCONNECT;
				SetEvent(m_hDisconnectDevice);
				return VS_INTERFACE_OK;
			}
		}
	}
	else if (strncmp(pSection, VS_CaptureDevice::_funcControl, strlen(VS_CaptureDevice::_funcControl)) == 0) {
		_variant_t* var = (_variant_t*)pVar;
		switch (VS_OPERATION)
		{
		case RUN_COMMAND:
			{
				VS_AutoLock lock(this);
				bool state = *var;
				tc_LevelModeState lvlState = m_pVideoCaptureList->GetLevelState(m_cDeviceName);
				m_pCaptureDevice->Sleep(m_cDeviceName, &m_sndFormat, lvlState.nVideoMode, !state);
				m_eState = (state) ? DEVICE_STARTUP : DEVICE_SLEEP;
				return VS_INTERFACE_OK;
			}
		}
		return VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (strncmp(pSection, VS_CaptureDevice::_funcCurrentName, strlen(VS_CaptureDevice::_funcCurrentName)) == 0) {
		_variant_t* var = (_variant_t*)pVar;
		switch (VS_OPERATION)
		{
		case GET_PARAM:
			{
				VS_AutoLock lock(this);
				*var = (_bstr_t)m_cDeviceRegName;
				return VS_INTERFACE_OK;
			}
		}
	}
	else if (strncmp(pSection, VS_CaptureDevice::_funcRealFramerate, strlen(VS_CaptureDevice::_funcRealFramerate)) == 0) {
		_variant_t* var = (_variant_t*)pVar;
		int framerate = 0;
		switch (VS_OPERATION)
		{
		case RUN_COMMAND:
			framerate = *var;
			m_pCaptureDevice->StartCaptureFramerate(framerate);
			return VS_INTERFACE_OK;
		case SET_PARAM:
			framerate = *var;
			m_pCaptureDevice->SetCaptureFramerate(framerate);
			return VS_INTERFACE_OK;
		case GET_PARAM:
			*var = m_pCaptureDevice->GetRealFramerate();
			return VS_INTERFACE_OK;
		}
	}
	else if (strncmp(pSection, VS_CaptureDevice::_funcCaptureFramerate, strlen(VS_CaptureDevice::_funcCaptureFramerate)) == 0) {
		_variant_t* var = (_variant_t*)pVar;
		switch (VS_OPERATION)
		{
		case GET_PARAM:
			*var = m_pCaptureDevice->GetCaptureFramerate();
			return VS_INTERFACE_OK;
		};
	}
	else if (strncmp(pSection, VS_CaptureDevice::_funcPropertyPage, strlen(VS_CaptureDevice::_funcPropertyPage)) == 0) {
		long ret = VS_INTERFACE_INTERNAL_ERROR;
		_variant_t* var = (_variant_t*)pVar;
		switch (VS_OPERATION)
		{
		case GET_PARAM:
			*var = m_pCaptureDevice->IsPropertyPage();
			ret = VS_INTERFACE_OK;
			break;
		case RUN_COMMAND:
			{
				VS_AutoLock lock(this);
				m_devSettings.hwndProp = HWND(var->byref);
				m_pCaptureDevice->RunPropertyPage(m_devSettings.hwndProp);
			}
			ret = VS_INTERFACE_OK;
			break;
		default : ret = VS_INTERFACE_NO_FUNCTION;
		}
		return ret;
	}
	else if (strncmp(pSection, VS_CaptureDevice::_funcPins, strlen(VS_CaptureDevice::_funcPins)) == 0) {
		long ret = VS_INTERFACE_INTERNAL_ERROR;
		_variant_t* var = (_variant_t*)pVar;
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			{
				{
					long res = m_pCaptureDevice->GetPins(var);
					if (res >= 0) {
						ret = res;
					}
					break;
				}
			}
		case RUN_COMMAND:
			{
				if (m_pCaptureDevice->SetPin(var->intVal)) {
					Lock();
					m_devSettings.iChannel = var->intVal;
					UnLock();

					/// write to Reg
					_variant_t var = m_devSettings.iChannel;
					WriteParam((char*)_RegDeviceChannel, &var);

					ret = VS_INTERFACE_OK;
				}
				break;
			}
		default : ret = VS_INTERFACE_NO_FUNCTION;
		}
		return ret;
	}
	else if (strncmp(pSection, VS_CaptureDevice::_funcVideoMode, strlen(VS_CaptureDevice::_funcVideoMode)) == 0) {
		long ret = VS_INTERFACE_INTERNAL_ERROR;
		_variant_t* var = (_variant_t*)pVar;
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			{
				__int64 res = m_pCaptureDevice->GetVideoModes();
				if (res >= 0) {
					var->Clear();
					V_VT(var) = VT_I8;
					V_I8(var) = res;
					ret = VS_INTERFACE_OK;
				}
			}
			break;
		case SET_PARAM:
			if (m_pCaptureDevice->SetVideoMode(*var)) {
				Lock();
				m_devSettings.iVideoMode = *var;
				UnLock();
				ret = VS_INTERFACE_OK;
			}
			break;
		default : ret = VS_INTERFACE_NO_FUNCTION;
		}
		return ret;
	}

	return VS_INTERFACE_NO_FUNCTION;
}

/**
 **************************************************************************
 ****************************************************************************/
void CVideoCaptureSlot::ReleaseRcvBuffers()
{
	delete [] m_pInputBuffer; m_pInputBuffer = 0;
	m_rcvFormat.SetZero();
	m_inputSize = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
void CVideoCaptureSlot::ReleaseSndBuffers()
{
	m_bValid = false;
	m_pRender->Release();
	delete [] m_pRenderBuffer; m_pRenderBuffer = 0;
	m_sndFormat.SetZero();
}

/**
 **************************************************************************
 ****************************************************************************/
bool CVideoCaptureSlot::UpdateSndFormat(VS_MediaFormat *mf, bool bScreenCapture)
{
	bool ret = false;
	if (!m_sndFormat.VideoSpatialEq(*mf) || (bScreenCapture != m_bScreenCapture)) {
		bool bChangeRender = (m_sndFormat.dwStereo != mf->dwStereo);
		ReleaseSndBuffers();
		CColorMode cm;
		/// renderer
		int k = (mf->dwStereo > 0) ? 2 : 1;
		int iSampleSize = (mf->dwVideoHeight * mf->dwVideoWidht * k * 12) >> 3;
		m_pRenderBuffer = new unsigned char [iSampleSize];
		if (m_pRenderBuffer) {
			cm.SetColorMode(NULL, (k == 2) ? cm.I420_STR0 : cm.I420, mf->dwVideoHeight, mf->dwVideoWidht);
			if (bChangeRender) {
				delete m_pRender; m_pRender = 0;
				m_pRender = CVideoRenderBase::RetrieveVideoRender(m_hwnd, this, mf->dwStereo);
				*(DWORD*)m_ppRender = *(DWORD*)(&m_pRender);
			}
			m_pRender->iInitRender(m_hwnd, m_pRenderBuffer, &cm, !bScreenCapture);
			m_bValid = true;
			ret = true;
		}
		/// csc
		if (m_bValid) {
			m_pColorConversion->ResetOutFormat(cm, (bScreenCapture) ? 1 : 6);
		}
		m_sndFormat = *mf;
	}
	m_bScreenCapture = bScreenCapture;
	return ret;
}

/**
 **************************************************************************
 ****************************************************************************/
bool CVideoCaptureSlot::UpdateRcvFormat(VS_MediaFormat *mf, int size)
{
	bool ret = false;
	if (!m_rcvFormat.VideoEq(*mf) || (m_inputSize != size && mf->dwVideoCodecFCC != CColorSpace::FCC_MJPG && mf->dwVideoCodecFCC != CColorSpace::FCC_H264)) {
		ReleaseRcvBuffers();
		BITMAPINFOHEADER bih;
		bih.biCompression = mf->dwVideoCodecFCC;
		bih.biWidth = mf->dwVideoWidht;
		bih.biHeight = mf->dwVideoHeight;
		bih.biSizeImage = 4 * bih.biWidth * bih.biHeight;
		if (mf->dwVideoCodecFCC == CColorSpace::FCC_MJPG || mf->dwVideoCodecFCC == CColorSpace::FCC_H264) {
			m_inputSize = bih.biSizeImage;
		} else {
			m_inputSize = size;
		}
		int bits = 0;
		if (mf->dwVideoCodecFCC == BI_RGB) {
			bits = size / (bih.biWidth * bih.biHeight) * 8;
		}
		bih.biBitCount = bits;
		m_pInputBuffer = new unsigned char[bih.biSizeImage];
		m_pColorConversion->ResetInFormat(&bih, m_pInputBuffer, m_devSettings.iDeinterlace);
		m_rcvFormat = *mf;
		ret = true;
	}
	return ret;
}

/**
 **************************************************************************
 ****************************************************************************/
void CVideoCaptureSlot::_Init(VS_MediaFormat &mf, eDeviceAction action)
{
	VS_AutoLock lock(this);

	bool bChangeMode = false;
	bool bScreenCapture = false;

	switch (m_eState)
	{
		case DEVICE_CONNECT:
			{
				if (action == DEVICE_STARTUP) {
					m_numConnectRequest--;
					if (m_numConnectRequest == 0) {
						tc_LevelModeState lvlState = m_pVideoCaptureList->GetLevelState(m_cDeviceName);
						m_pCaptureDevice->Connect(m_cDeviceName, &mf, lvlState.nVideoMode, &m_devSettings);
						m_eState = DEVICE_STARTUP;
					}
					bScreenCapture = (m_pCaptureDeviceList[CVideoCaptureList::CAPTURE_SCREEN] == m_pCaptureDevice);
					UpdateSndFormat(&mf, bScreenCapture);
				}
				break;
			}
		case DEVICE_DISCONNECT:
			{
				m_numConnectRequest = 0;
				if (action == DEVICE_SHOOTDOWN) {
					m_pCaptureDevice->Disconnect();
				}
				break;
			}
		default :
			{
				if (action != DEVICE_SHOOTDOWN) {
					if (action == DEVICE_STARTUP) {
						tc_LevelModeState lvlState = m_pVideoCaptureList->GetLevelState(m_cDeviceName);
						m_pCaptureDevice->Connect(m_cDeviceName, &mf, lvlState.nVideoMode, &m_devSettings);
						if (m_eState == DEVICE_SLEEP) {
							m_pCaptureDevice->Disconnect();
						}
						else {
							m_eState = DEVICE_STARTUP;
						}
					}
					bScreenCapture = (m_pCaptureDeviceList[CVideoCaptureList::CAPTURE_SCREEN] == m_pCaptureDevice);
					UpdateSndFormat(&mf, bScreenCapture);
				}
				break;
			}
	}

}

/**
 **************************************************************************
 ****************************************************************************/
void CVideoCaptureSlot::_Release()
{
	ReleaseSndBuffers();
	ReleaseRcvBuffers();
}

/**
 **************************************************************************
 ****************************************************************************/
HANDLE CVideoCaptureSlot::GetVideoEvent()
{
	return m_hGetFrame;
}

/**
 **************************************************************************
 ****************************************************************************/
HANDLE CVideoCaptureSlot::GetVideoEventConnect()
{
	return m_hConnectDevice;
}

/**
 **************************************************************************
 ****************************************************************************/
HANDLE CVideoCaptureSlot::GetVideoEventDisconnect()
{
	return m_hDisconnectDevice;
}

/**
 **************************************************************************
 ****************************************************************************/
unsigned char *CVideoCaptureSlot::GetVideoFrame(int *pFPS, unsigned int *pTimestamp, unsigned char *&pRenderFrame)
{
	VS_AutoLock lock(this);
	unsigned char *pImage = 0;
	pRenderFrame = 0;
	if (m_bValid) {
		unsigned int size = 0;
		if (WaitForSingleObject(m_hCaptureMutex, 20) == WAIT_OBJECT_0) {
			if (m_pColorConversion->ConvertInput(NULL, m_frameSize)) {
				size = m_pColorConversion->GetOutput(pRenderFrame, pImage);
				*pFPS = m_realFramerate;
				*pTimestamp = m_timestamp;
			}
			if (size) {
				memcpy(m_pRenderBuffer, pRenderFrame, size);
				if (pImage == 0) pImage = m_pRenderBuffer;
			} else {
				pImage = 0;
			}
			ReleaseMutex(m_hCaptureMutex);
		}
	}
	pRenderFrame = m_pRenderBuffer;
	return pImage;
}

/**
 **************************************************************************
 ****************************************************************************/
void CVideoCaptureSlot::PushFrame(unsigned char *pBuffer, int size, int realFramerate, unsigned int timestamp, int width, int height, unsigned int color, bool hardware)
{
	int dtWait = 10;
	if (hardware) {
		WaitForSingleObject(m_hProcessFrameEnd, INFINITE);
		dtWait = INFINITE;
	}
	if (WaitForSingleObject(m_hCaptureMutex, dtWait) == WAIT_OBJECT_0) {
		if (m_bValid) {
			m_frameSize = size;
			m_realFramerate = realFramerate;
			m_timestamp = timestamp;
			VS_MediaFormat mf;
			mf.SetVideo(width, height, color);
			UpdateRcvFormat(&mf, size);
			memcpy(m_pInputBuffer, pBuffer, size);
			SetEvent(m_hGetFrame);
		}
		ReleaseMutex(m_hCaptureMutex);
	}
}

/****************************************************************************
 * CVideoCaptureSlotExt
 ****************************************************************************/
CVideoCaptureSlotExt::CVideoCaptureSlotExt(const char *szSlotName, CVSInterface* pParentInterface, TVideoWindowParams *pVParams)
:CVideoCaptureSlotBase(szSlotName,pParentInterface,pVParams)
{
	m_pTmpBuffer = m_pInputBuffer = 0;
	m_hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_iSampleSize = 0;
	iRealFps = iFps = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
CVideoCaptureSlotExt::~CVideoCaptureSlotExt()
{
	_Release();
	CloseHandle(m_hEvent); m_hEvent = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
void CVideoCaptureSlotExt::_Init(VS_MediaFormat &mf, eDeviceAction action) {
	if (m_sndFormat.dwVideoHeight!=mf.dwVideoHeight || m_sndFormat.dwVideoWidht!=mf.dwVideoWidht) {
		_Release();
		m_sndFormat = mf;
		/// Вычисление размера буффера для формата i420
		m_iSampleSize = (mf.dwVideoHeight*mf.dwVideoWidht*12)>>3;
		unsigned char* pBuff =(unsigned char* )malloc(m_iSampleSize*3);
		if (pBuff){
			m_pRenderBuffer = pBuff;
			m_pInputBuffer = pBuff+m_iSampleSize;
			m_pTmpBuffer = pBuff+m_iSampleSize*2;
			CColorMode cm;
			cm.SetColorMode(NULL,cm.I420,mf.dwVideoHeight,mf.dwVideoWidht);
			m_ColConv.SetOutFormat(cm);
			m_bValid = m_pRender->iInitRender(m_hwnd, m_pRenderBuffer, &cm, true)==0;
		}
	}
}

/**
 **************************************************************************
 ****************************************************************************/
void CVideoCaptureSlotExt::_Release(){
	m_bValid = false;
	m_pRender->Release();
	if (m_pRenderBuffer) {
		delete [] m_pRenderBuffer;
		m_pRenderBuffer = m_pInputBuffer = m_pTmpBuffer = 0;
	}
	m_iSampleSize = 0;
	iRealFps = iFps = 0;
	m_sndFormat.SetZero();
}

/**
 **************************************************************************
 ****************************************************************************/
HANDLE CVideoCaptureSlotExt::GetVideoEvent(){
	return m_bValid ? m_hEvent : 0;
}

/**
 **************************************************************************
 ****************************************************************************/
HANDLE CVideoCaptureSlotExt::GetVideoEventConnect(){
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
HANDLE CVideoCaptureSlotExt::GetVideoEventDisconnect(){
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
CVideoCaptureList::eCapturerType CVideoCaptureSlotExt::GetTypeCapturer()
{
	return CVideoCaptureList::CAPTURE_DS;
}

/**
 **************************************************************************
 ****************************************************************************/
unsigned char *CVideoCaptureSlotExt::GetVideoFrame(int *pFPS, unsigned int *pTimestamp, unsigned char *&pRenderFrame){
	unsigned char *pBuff=NULL;
	pRenderFrame = 0;
	if (m_bValid) {
		memcpy(m_pInputBuffer, m_pTmpBuffer, m_iSampleSize);
		memcpy(m_pRenderBuffer,m_pTmpBuffer, m_iSampleSize);
		*pFPS=iRealFps;
		pRenderFrame = m_pRenderBuffer;
		return m_pInputBuffer;
	}
	else
		return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CVideoCaptureSlotExt::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;
	if(strncmp(pSection,_funcResetWindow,sizeof(_funcResetWindow))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			ResetWindow((HWND)int(*var));
			return VS_INTERFACE_OK;
		}
	}
	if(strncmp(pSection,_funcSetFormat,sizeof(_funcSetFormat))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			if(var->vt==(VT_ARRAY|VT_VARIANT)){
				long l,u;
				SAFEARRAY *psa=var->parray;
				SafeArrayGetLBound(psa,1,&l);
				SafeArrayGetUBound(psa,1,&u);
				if(u-l==1){
					VARIANT vr,vr_h;
					SafeArrayGetElement(psa,&l,&vr);
					l++;
					SafeArrayGetElement(psa,&l,&vr_h);

					LPBITMAPINFOHEADER lbp = (LPBITMAPINFOHEADER)(void*)(int)vr.intVal;
					iRealFps = vr_h.intVal;
					iFps = iRealFps;
					m_ColConv.SetInFormat(lbp);
					return VS_INTERFACE_OK;
				}
			}
		}
	}
	else if(strncmp(pSection,_funcPushData,sizeof(_funcPushData))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			if (m_bValid) {
				unsigned char *pBuff=(unsigned char *)int(*var);
				m_ColConv.ConvertInput(pBuff, 0);
				unsigned char *pFrameI420, *pCompressFrame;
				unsigned int SizeI420 = m_ColConv.GetOutput(pFrameI420, pCompressFrame);
				if(SizeI420){
					memcpy(m_pTmpBuffer,pFrameI420,SizeI420);
					*var=iFps;
					SetEvent(m_hEvent);
				}
			}
			return VS_INTERFACE_OK;
		}
	}
	else if(strncmp(pSection,VS_CaptureDevice::_funcRealFramerate,strlen(VS_CaptureDevice::_funcRealFramerate))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
		case SET_PARAM:
			iFps=*var;
			return VS_INTERFACE_OK;
		}
	}
	return VS_INTERFACE_NO_FUNCTION;
}

#ifdef VS_LINUX_DEVICE

CVideocaptureV4L::CVideocaptureV4L(const char *szSlotName, CVSInterface* pParentInterface, TVideoWindowParams *pVParams, CVideoCaptureList *pVideoCaptureList)
:CVideoCaptureSlotBase(szSlotName, pParentInterface, pVParams)
{
	m_size = 115200; m_sleep = 115; //96; hmm... 96 makes video freezing a little
	new CCaptureVideoSourceV4L(this, "VideoCapture", pVideoCaptureList);
	m_bValid	= true;
	m_Event		= CreateEvent(0, FALSE, FALSE, 0);
	m_EventTimer= CreateEvent(0, FALSE, FALSE, 0);
	destroy		= false;
	m_ofile		= CreateFileA("c:\\v4l.obj", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	m_file		= CreateFileMappingA(m_ofile, 0, PAGE_READWRITE, 0, m_size, "v4l");
	m_map		= MapViewOfFile(m_file, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, m_size);
	m_copy		= malloc(m_size);
	m_Thread	= CreateThread(0, 0, &CVideocaptureV4L::ThreadProc, this, 0, 0);
}

CVideocaptureV4L::~CVideocaptureV4L()
{
	_Release();
}

HANDLE CVideocaptureV4L::GetVideoEvent()
{
	return m_Event;
}

unsigned char *CVideocaptureV4L::GetVideoFrame(int *pFPS)
{
	memcpy(m_copy, m_map, m_size);
	return (unsigned char *)m_copy;
}

void CVideocaptureV4L::_Init(VS_MediaFormat &mf)
{
	if (m_mf.dwVideoHeight!=mf.dwVideoHeight || m_mf.dwVideoWidht!=mf.dwVideoWidht) {
		_Release();
		m_mf = mf;
		// Вычисление размера буффера для формата i420
		int iSampleSize = (mf.dwVideoHeight*mf.dwVideoWidht*12)>>3;
		unsigned char* pBuff = (unsigned char* )malloc(iSampleSize*2);
		if (pBuff) {
			CColorMode cm;
			cm.SetColorMode(NULL, cm.I420, mf.dwVideoHeight, mf.dwVideoWidht);
			m_pRenderBuffer = pBuff;
			m_pInputBuffer = pBuff + iSampleSize;
			m_pRender->iInitRender(m_hwnd, (unsigned char *)m_copy, &cm);
			m_bValid = true;
		}
		if (!m_bValid)
			_Release();
	}
}

void CVideocaptureV4L::_Release()
{
}

DWORD WINAPI CVideocaptureV4L::ThreadProc(LPVOID lpParameter)
{
	while (true) {
		WaitForSingleObject(((CVideocaptureV4L *)lpParameter)->m_EventTimer, ((CVideocaptureV4L *)lpParameter)->m_sleep);
		SetEvent(((CVideocaptureV4L *)lpParameter)->m_Event);
	}
}
#endif
