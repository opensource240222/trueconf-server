
#include "VSCapture.h"
#include <Mfapi.h>
#include <Mferror.h>
#include <Codecapi.h>
#include <Ksmedia.h>
#include "VSVideoCaptureList.h"
#include "VS_Dmodule.h"
#include "../std/cpplib/VS_Protocol.h"
#include "std/cpplib/ThreadUtils.h"
#include "../_Visicron/resource.h"
#include <commctrl.h>

DEFINE_GUID(GUID_MFSourceReaderCallback, 0xDEEC8D99, 0xFA1D, 0x4D82, 0x84, 0xC2, 0x2C, 0x89, 0x69, 0x94, 0x48, 0x67);

namespace DeviceProp
{
	enum ControlType {
		SLIDER = 0,
		CHECK = 1,
		COMBO
	};

	struct APIPropDevice_t {
		ControlType type;
		int sliderId;
		int textId;
		int editId;
		int checkId;
		int comboId;
	};

	static APIPropDevice_t APIPropDevice[VS_CaptureDeviceMediaFoundation::MAXTYPES] = {
		{ SLIDER,   IDC_BRIGHTNESS,   IDC_TEXT_BRIGHTNESS,   IDC_EDIT_BRIGHTNESS,   IDC_CHECK_BRIGHTNESS,					0 },
		{ SLIDER,	  IDC_CONTRAST,	    IDC_TEXT_CONTRAST,	   IDC_EDIT_CONTRAST,	  IDC_CHECK_CONTRAST,					0 },
		{ SLIDER,		   IDC_HUE,		     IDC_TEXT_HUE,		    IDC_EDIT_HUE,		   IDC_CHECK_HUE,					0 },
		{ SLIDER,   IDC_SATURATION,   IDC_TEXT_SATURATION,   IDC_EDIT_SATURATION,	IDC_CHECK_SATURATION,					0 },
		{ SLIDER,    IDC_SHARPNESS,    IDC_TEXT_SHARPNESS,    IDC_EDIT_SHARPNESS,	 IDC_CHECK_SHARPNESS,					0 },
		{ SLIDER,		 IDC_GAMMA,		   IDC_TEXT_GAMMA,		  IDC_EDIT_GAMMA,		 IDC_CHECK_GAMMA,					0 },
		{  CHECK,			     0,					    0,					   0,  IDC_CHECK_COLORENABLE,					0 },
		{ SLIDER, IDC_WHITEBALANCE, IDC_TEXT_WHITEBALANCE, IDC_EDIT_WHITEBALANCE, IDC_CHECK_WHITEBALANCE,					0 },
		{ SLIDER,	 IDC_BACKLIGHT,    IDC_TEXT_BACKLIGHT,	  IDC_EDIT_BACKLIGHT,	 IDC_CHECK_BACKLIGHT,					0 },
		{ SLIDER,		  IDC_GAIN,		    IDC_TEXT_GAIN,		   IDC_EDIT_GAIN,		  IDC_CHECK_GAIN,					0 },
		{  COMBO,				 0,	   IDC_TEXT_POWERFREQ,					   0,					   0, IDC_COMBO_POWERFREQ },
		{ SLIDER,		 IDC_FOCUS,		   IDC_TEXT_FOCUS,		  IDC_EDIT_FOCUS,		 IDC_CHECK_FOCUS,					0 },
		{ SLIDER,	  IDC_EXPOSURE,		IDC_TEXT_EXPOSURE,	   IDC_EDIT_EXPOSURE,	  IDC_CHECK_EXPOSURE,					0 },
		{  CHECK,				 0,					    0,					   0,	  IDC_CHECK_LOWLIGHT,					0 },
		{ SLIDER,		   IDC_PAN,			 IDC_TEXT_PAN,			IDC_EDIT_PAN,		   IDC_CHECK_PAN,					0 },
		{ SLIDER,		  IDC_TILT,			IDC_TEXT_TILT,		   IDC_EDIT_TILT,		  IDC_CHECK_TILT,					0 },
		{ SLIDER,		  IDC_ZOOM,			IDC_TEXT_ZOOM,		   IDC_EDIT_ZOOM,		  IDC_CHECK_ZOOM,				    0 }
	};

	static int id2ksproxy[VS_CaptureDeviceMediaFoundation::MAXTYPES] = {
		VideoProcAmp_Brightness,
		VideoProcAmp_Contrast,
		VideoProcAmp_Hue,
		VideoProcAmp_Saturation,
		VideoProcAmp_Sharpness,
		VideoProcAmp_Gamma,
		VideoProcAmp_ColorEnable,
		VideoProcAmp_WhiteBalance,
		VideoProcAmp_BacklightCompensation,
		VideoProcAmp_Gain,
		KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY,
		CameraControl_Focus,
		CameraControl_Exposure,
		KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY,
		CameraControl_Pan,
		CameraControl_Tilt,
		CameraControl_Zoom
	};
}

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT) {
		(*ppT)->Release();
		*ppT = NULL;
	}
}

void InitControl(HWND hwnd, int controlId, VS_CaptureDeviceMediaFoundation::PropertyDevice *pProperty, bool bDefault) {
	int val = (bDefault) ? pProperty->default_val : pProperty->val;
	pProperty->val = val;
	DeviceProp::APIPropDevice_t *pAPIState = &DeviceProp::APIPropDevice[controlId];
	if (pProperty->valid) {
		if (pAPIState->type == DeviceProp::SLIDER) {
			SendMessage(GetDlgItem(hwnd, pAPIState->sliderId), TBM_SETRANGE, TRUE, (LPARAM) MAKELONG( pProperty->min_val, pProperty->max_val ) );
			SendMessage(GetDlgItem(hwnd, pAPIState->sliderId), TBM_SETPOS, TRUE, (LPARAM) val);
			SetDlgItemInt(hwnd, pAPIState->editId, val, TRUE);
			if (pProperty->flag_caps != (VideoProcAmp_Flags_Auto | VideoProcAmp_Flags_Manual)) EnableWindow(GetDlgItem(hwnd, pAPIState->checkId), FALSE);
			if (pProperty->flag & VideoProcAmp_Flags_Auto) {
				CheckDlgButton(hwnd, pAPIState->checkId, 1);
				EnableWindow(GetDlgItem(hwnd, pAPIState->sliderId), FALSE);
			}
		} else if (pAPIState->type == DeviceProp::CHECK) {
			CheckDlgButton(hwnd, pAPIState->checkId, val);
		} else if (pAPIState->type == DeviceProp::COMBO) {
			if (SendMessage( GetDlgItem(hwnd, pAPIState->comboId), CB_GETCOUNT, 0, 0 ) == 0) {
				SendDlgItemMessage(hwnd, pAPIState->comboId, CB_ADDSTRING, 0, (LPARAM)"50 Hz");
				SendDlgItemMessage(hwnd, pAPIState->comboId, CB_ADDSTRING, 0, (LPARAM)"60 Hz");
			}
			SendDlgItemMessage(hwnd, pAPIState->comboId, CB_SETCURSEL, val - 1, 0);
		}
	} else {
		if (pAPIState->type == DeviceProp::SLIDER) {
			EnableWindow(GetDlgItem(hwnd, pAPIState->sliderId), FALSE);
			EnableWindow(GetDlgItem(hwnd, pAPIState->editId), FALSE);
			EnableWindow(GetDlgItem(hwnd, pAPIState->textId), FALSE);
			EnableWindow(GetDlgItem(hwnd, pAPIState->checkId), FALSE);
			CheckDlgButton(hwnd, pAPIState->checkId, 0);
		} else if (pAPIState->type == DeviceProp::CHECK) {
			EnableWindow(GetDlgItem(hwnd, pAPIState->checkId), FALSE);
			CheckDlgButton(hwnd, pAPIState->checkId, 0);
		} else if (pAPIState->type == DeviceProp::COMBO) {
			EnableWindow(GetDlgItem(hwnd, pAPIState->comboId), FALSE);
			EnableWindow(GetDlgItem(hwnd, pAPIState->textId), FALSE);
		}
	}
}

void GetValueControl(HWND hwnd, int controlId, VS_CaptureDeviceMediaFoundation::PropertyDevice *pProperty)
{
	DeviceProp::APIPropDevice_t *pAPIState = &DeviceProp::APIPropDevice[controlId];
	if (pAPIState->type == DeviceProp::SLIDER) {
		pProperty->val = (int)SendMessage(GetDlgItem(hwnd, pAPIState->sliderId), TBM_GETPOS, 0, 0);
		pProperty->flag = VideoProcAmp_Flags_Manual;
		if ( IsDlgButtonChecked(hwnd, pAPIState->checkId) == BST_CHECKED ) {
			pProperty->flag = VideoProcAmp_Flags_Auto;
		}
	} else if (pAPIState->type == DeviceProp::CHECK) {
		pProperty->val = ( IsDlgButtonChecked(hwnd, pAPIState->checkId) == BST_CHECKED ) ? 1 : 0;
	} else if (pAPIState->type == DeviceProp::COMBO) {
		pProperty->val = (int)SendMessage(GetDlgItem(hwnd, pAPIState->comboId), CB_GETCURSEL, 0, 0) + 1;
	}
}

void UpdateSliderControl(HWND hwnd, int idSlider, int idEnum)
{
	int valueToChange = (int)SendMessage(GetDlgItem(hwnd, idSlider), TBM_GETPOS, 0, 0);
	SetDlgItemInt(hwnd, idEnum, valueToChange, TRUE);
}

void UpdateCheckControl(HWND hwnd, int controlId)
{
	DeviceProp::APIPropDevice_t *pAPIState = &DeviceProp::APIPropDevice[controlId];
	if ( IsDlgButtonChecked(hwnd, pAPIState->checkId) == BST_CHECKED ) {
		EnableWindow(GetDlgItem(hwnd, pAPIState->sliderId), FALSE);
		UpdateSliderControl(hwnd, pAPIState->sliderId, pAPIState->editId);
	} else {
		EnableWindow(GetDlgItem(hwnd, pAPIState->sliderId), TRUE);
	}
	EnableWindow(GetDlgItem(hwnd, IDC_APPLY), TRUE);
}

bool CheckSliderControl(HWND hwnd, HWND param, int controlId)
{
	DeviceProp::APIPropDevice_t *pAPIState = &DeviceProp::APIPropDevice[controlId];
	if (pAPIState->type == DeviceProp::SLIDER && param == GetDlgItem(hwnd, pAPIState->sliderId)) {
		int valueToChange = (int)SendMessage(GetDlgItem(hwnd, pAPIState->sliderId), TBM_GETPOS, 0, 0);
		SetDlgItemInt(hwnd, pAPIState->editId, valueToChange, TRUE);
		return true;
	}
	return false;
}

BOOL CALLBACK DialogProcMF(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static VS_CaptureDeviceMediaFoundation::PropertyDevice propertyDev[VS_CaptureDeviceMediaFoundation::MAXTYPES];
	static VS_CaptureDeviceMediaFoundation::PropertyDevice temp_propertyDev[VS_CaptureDeviceMediaFoundation::MAXTYPES];
	static std::map <int, int> mapCheckId;
	int idCheck = 0, idSlider = 0, idEnum = 0;
	switch (iMsg)
	{
		case WM_INITDIALOG :
			{
				VS_CaptureDeviceMediaFoundation *pCapture = (VS_CaptureDeviceMediaFoundation*)lParam;
				pCapture->GetPropertyState(propertyDev);
				::SetWindowLong(hDlg, GWL_USERDATA, (long)lParam);
				mapCheckId.clear();
				for (int i = 0; i < VS_CaptureDeviceMediaFoundation::MAXTYPES; i++) {
					memcpy(&temp_propertyDev[i], &propertyDev[i], sizeof(VS_CaptureDeviceMediaFoundation::PropertyDevice));
					InitControl(hDlg, i, &propertyDev[i], false);
					if (DeviceProp::APIPropDevice[i].type == DeviceProp::SLIDER || DeviceProp::APIPropDevice[i].type == DeviceProp::CHECK) {
						mapCheckId.insert(std::pair<int, int>(DeviceProp::APIPropDevice[i].checkId, i));
					}
				}
				EnableWindow(GetDlgItem(hDlg, IDC_APPLY), FALSE);
				UpdateWindow(hDlg);

				// hack for topmost modal window from other thread
				HWND hCurrWnd = ::GetForegroundWindow();
				int iMyTID   = GetCurrentThreadId();
				int iCurrTID = GetWindowThreadProcessId(hCurrWnd, 0);
				SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
				SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
				AttachThreadInput(iMyTID, iCurrTID, TRUE);
				SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, 0, 0);
				SetForegroundWindow(hDlg);
				AttachThreadInput(iMyTID, iCurrTID, FALSE);

				return TRUE;
			}
		case WM_COMMAND :
			{
				VS_CaptureDeviceMediaFoundation *pCapture = (VS_CaptureDeviceMediaFoundation*)::GetWindowLong(hDlg, GWL_USERDATA);
				switch (LOWORD (wParam))
				{
					case IDOK :
						{
							for (int i = 0; i < VS_CaptureDeviceMediaFoundation::MAXTYPES; i++) {
								memcpy(&propertyDev[i], &temp_propertyDev[i], sizeof(VS_CaptureDeviceMediaFoundation::PropertyDevice));
							}
							pCapture->SetPropertyState(propertyDev);
							EndDialog(hDlg, wParam);
							return TRUE;
						}
					case IDCANCEL:
						{
							pCapture->SetPropertyState(propertyDev);
							EndDialog(hDlg, wParam);
							return TRUE;
						}
					case IDC_APPLY:
						{
							for (int i = 0; i < VS_CaptureDeviceMediaFoundation::MAXTYPES; i++) {
								memcpy(&propertyDev[i], &temp_propertyDev[i], sizeof(VS_CaptureDeviceMediaFoundation::PropertyDevice));
							}
							EnableWindow(GetDlgItem(hDlg, IDC_APPLY), FALSE);
							return TRUE;
						}
					case IDC_DEFAULT:
						{
							for (int i = 0; i < VS_CaptureDeviceMediaFoundation::MAXTYPES; i++) {
								InitControl(hDlg, i, &temp_propertyDev[i], true);
							}
							pCapture->SetPropertyState(temp_propertyDev);
							EnableWindow(GetDlgItem(hDlg, IDC_APPLY), TRUE);
							return TRUE;
						}
					case IDC_COMBO_POWERFREQ:
						{
							int valueToChange = (int)SendMessage(GetDlgItem(hDlg, IDC_COMBO_POWERFREQ), CB_GETCURSEL, 0, 0) + 1;
							if (valueToChange != temp_propertyDev[VS_CaptureDeviceMediaFoundation::POWERLINEFREQ].val) {
								temp_propertyDev[VS_CaptureDeviceMediaFoundation::POWERLINEFREQ].val = valueToChange;
								pCapture->SetPropertyState(temp_propertyDev);
								EnableWindow(GetDlgItem(hDlg, IDC_APPLY), TRUE);
							}
							return TRUE;
						}
					case IDC_CHECK_COLORENABLE:
					case IDC_CHECK_LOWLIGHT:
					case IDC_CHECK_BRIGHTNESS:
					case IDC_CHECK_CONTRAST:
					case IDC_CHECK_HUE:
					case IDC_CHECK_SATURATION:
					case IDC_CHECK_SHARPNESS:
					case IDC_CHECK_GAMMA:
					case IDC_CHECK_WHITEBALANCE:
					case IDC_CHECK_BACKLIGHT:
					case IDC_CHECK_GAIN:
					case IDC_CHECK_FOCUS:
					case IDC_CHECK_EXPOSURE:
					case IDC_CHECK_PAN:
					case IDC_CHECK_TILT:
					case IDC_CHECK_ZOOM:
						{
							std::map<int,int>::iterator it = mapCheckId.find(LOWORD(wParam));
							if (it != mapCheckId.end()) {
								if (it->second == IDC_CHECK_COLORENABLE || it->second == IDC_CHECK_LOWLIGHT) {
									EnableWindow(GetDlgItem(hDlg, IDC_APPLY), TRUE);
								} else {
									UpdateCheckControl(hDlg, it->second);
								}
								GetValueControl(hDlg, it->second, &temp_propertyDev[it->second]);
								pCapture->SetPropertyState(temp_propertyDev);
							}
							return TRUE;
						}
				}
				break;
			}
		case WM_DESTROY:
			{
				VS_CaptureDeviceMediaFoundation *pCapture = (VS_CaptureDeviceMediaFoundation*)::GetWindowLong(hDlg, GWL_USERDATA);
				pCapture->SetPropertyState(propertyDev);
				EndDialog(hDlg, wParam);
				return TRUE;
			}
		case WM_HSCROLL:
			{
				VS_CaptureDeviceMediaFoundation *pCapture = (VS_CaptureDeviceMediaFoundation*)::GetWindowLong(hDlg, GWL_USERDATA);
				for (int i = 0; i < VS_CaptureDeviceMediaFoundation::MAXTYPES; i++) {
					if ( CheckSliderControl(hDlg, (HWND)lParam, i) ) {
						GetValueControl(hDlg, i, &temp_propertyDev[i]);
						pCapture->SetPropertyState(temp_propertyDev);
						break;
					}
				}
				EnableWindow(GetDlgItem(hDlg, IDC_APPLY), TRUE);
				return TRUE;
			}
	}
	return DefWindowProc(hDlg, iMsg, wParam, lParam);
}

static unsigned int WINAPI thread_property_proc_mf(void *pSelf)
{
	vs::SetThreadName("PropertyMF");
	INT_PTR ret = 0;
	VS_CaptureDeviceMediaFoundation *pCapture = (VS_CaptureDeviceMediaFoundation*)pSelf;
	if (pCapture) {
		HMODULE hModule = GetModuleHandle("visicron.dll");
		if (hModule) {
			HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(IDD_PROPERTYCAPTURE), RT_DIALOG);
			if (hResource) {
				HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
				if (hLoadedResource) {
					DLGTEMPLATEEX *t = (DLGTEMPLATEEX*)LockResource(hLoadedResource);
					HWND hwnd = pCapture->GetHwndPropertyPage();
					if (hwnd) {
						ret = DialogBoxIndirectParam((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), (LPDLGTEMPLATE)t, hwnd, (DLGPROC)DialogProcMF, (LPARAM)pCapture);
					}
				}
			}
		}
		pCapture->InternalClosePropertyPage();
	}
	return ret;
}

class VS_AsyncVaptureOp : public IUnknown
{
	LONG    m_cRef;

public :

	VS_CaptureDeviceState m_devState;
	VS_AsyncVaptureOp(VS_CaptureDeviceState devState) : m_cRef(1), m_devState(devState) {};
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(VS_AsyncVaptureOp, IUnknown),
            { 0 }
        };
        return QISearch(this, qit, riid, ppv);
    }
    STDMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }
    STDMETHODIMP_(ULONG) Release()
    {
        LONG cRef = InterlockedDecrement(&m_cRef);
        if (cRef == 0)
        {
            delete this;
        }
        return cRef;
    }
};

VS_CaptureDeviceMediaFoundation::VS_CaptureDeviceMediaFoundation(VS_VideoCaptureSlotObserver *observerSlot, CVideoCaptureList *pCaptureList) : VS_CaptureDevice(observerSlot, pCaptureList)
{
	HRESULT hr;
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	hr = g_MFStartup(MF_VERSION, MFSTARTUP_FULL);
	m_dwWorkQueue = 0;
	hr = g_MFAllocateSerialWorkQueue(MFASYNC_CALLBACK_QUEUE_MULTITHREADED, &m_dwWorkQueue);
	m_cRef = 1;
	m_bFirstSample = false;
	m_llBaseTime = 0;
	m_pSource = NULL;
	m_pReader = NULL;
	m_pPresentation = NULL;
	m_pCodecApi = NULL;
	m_pModeList = NULL;
	m_nOptimalVideoMode = -1;
	m_idActiveStream = 0;
	m_hardwareRequest = HARDWARE_UNDEF;
	memset(&m_stHardwareEncoder, 0, sizeof(m_stHardwareEncoder));
	m_eTypeHardware = ENCODER_SOFTWARE;
	m_pHardwareEncoder = 0;
	m_pMediaBuffer = NULL;
	m_pBuffer = NULL;
	m_eDeviceState = DEVICE_SHUTDOWN;
	m_hDestroy = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hTtreadProperty = NULL;

	m_pHardwareObserver = static_cast <VS_HardwareEncoderObserver*> (this);

	m_typeDevice = VS_CaptureDevice::MEDIA_FOUNDATION;
}

VS_CaptureDeviceMediaFoundation::~VS_CaptureDeviceMediaFoundation()
{
	Lock();
	m_devState.FillState(NULL, DEVICE_DESTROY, NULL, 0);
	CreateAsyncResult(m_devState);
	UnLock();

	WaitForSingleObject(m_hDestroy, INFINITE);

	if (m_hDestroy) CloseHandle(m_hDestroy); m_hDestroy = NULL;
	if (m_hTtreadProperty) CloseHandle(m_hTtreadProperty); m_hTtreadProperty = NULL;
	g_MFUnlockWorkQueue(m_dwWorkQueue);
	g_MFShutdown();
	CoUninitialize();
}

int VS_CaptureDeviceMediaFoundation::CreateAsyncResult(VS_CaptureDeviceState devState)
{
	VS_AsyncVaptureOp *pOp = new (std::nothrow) VS_AsyncVaptureOp(devState);
	if (!pOp) return -1;
	IMFAsyncResult *pResult = NULL;
	HRESULT hr = g_MFCreateAsyncResult(pOp, this, NULL, &pResult);
	if ( SUCCEEDED(hr) ) {
		hr = g_MFPutWorkItem(m_dwWorkQueue, this, pResult);
		pResult->Release();
	}
	return 0;
}

int VS_CaptureDeviceMediaFoundation::Connect(wchar_t *szName, VS_MediaFormat *mf, int deviceMode, VS_CaptureDeviceSettings *devSettings)
{
	VS_AutoLock lock(this);
	if (m_devState.eAction == DEVICE_DESTROY) return 0;
	m_devState.FillState(szName, DEVICE_CONNECT, mf, deviceMode);
	return CreateAsyncResult(m_devState);
}

int VS_CaptureDeviceMediaFoundation::Disconnect()
{
	VS_AutoLock lock(this);
	if (m_devState.eAction == DEVICE_DESTROY) return 0;
	m_devState.FillState(NULL, DEVICE_DISCONNECT, &m_renderFmt, -1);
	return CreateAsyncResult(m_devState);
}

void VS_CaptureDeviceMediaFoundation::Sleep(wchar_t *szName, VS_MediaFormat *mf, int deviceMode, bool state)
{
	VS_AutoLock lock(this);
	if (!state) {
		Connect(szName, mf, deviceMode, NULL);
	} else {
		Disconnect();
	}
}

bool VS_CaptureDeviceMediaFoundation::GetPropertyPage()
{
	VS_AutoLock lock(this);
	VS_CaptureDeviceState devState;
	devState.eAction = DEVICE_PROPERTY;
	CreateAsyncResult(devState);
	return true;
}

STDMETHODIMP VS_CaptureDeviceMediaFoundation::QueryInterface(REFIID riid, void** ppv)
{
	static const QITAB qit[] =
	{
		QITABENT(VS_CaptureDeviceMediaFoundation, IMFSourceReaderCallback),
		QITABENT(VS_CaptureDeviceMediaFoundation, IMFAsyncCallback),
		{ 0 }
	};
	return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) VS_CaptureDeviceMediaFoundation::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) VS_CaptureDeviceMediaFoundation::Release()
{
    long cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) {
        delete this;
    }
    return cRef;
}

STDMETHODIMP VS_CaptureDeviceMediaFoundation::Invoke(IMFAsyncResult* pResult)
{
	HRESULT hr = E_FAIL;

	IUnknown *pState = NULL;
	IUnknown *pUnk = NULL;
	IMFAsyncResult *pCallerResult = NULL;

	if (!pResult) return E_UNEXPECTED;
	hr = pResult->GetState(&pState);
	if ( SUCCEEDED(hr) ) {
		hr = pState->QueryInterface(IID_PPV_ARGS(&pCallerResult));
		if ( SUCCEEDED(hr) ) {
			hr = pCallerResult->GetObject(&pUnk);
			if ( SUCCEEDED(hr) ) {
				VS_AsyncVaptureOp *pOp = reinterpret_cast <VS_AsyncVaptureOp*> (pUnk);

				switch (pOp->m_devState.eAction) {
					case DEVICE_CONNECT:
						{
							int ret = InternalConnect(pOp->m_devState.cDeviceName, &(pOp->m_devState.mf), pOp->m_devState.iOptimalMode);
							g_DevStatus.SetStatus(DVS_SND_NOTWORK, false, ret != 0);
							break;
						}
					case DEVICE_DISCONNECT:
						{
							InternalDisconnect(pOp->m_devState.cDeviceName);
							break;
						}
					case DEVICE_PROPERTY:
						{
							InternalPropertyPage();
							break;
						}
					case DEVICE_DESTROY:
						{
							InternalDestroy();
							break;
						}
				}

			}
		}
	}

	if (pCallerResult) {
		pCallerResult->SetStatus(hr);
		g_MFInvokeCallback(pCallerResult);
	}

    SafeRelease(&pState);
    SafeRelease(&pUnk);
    SafeRelease(&pCallerResult);

	return S_OK;
}

HRESULT VS_CaptureDeviceMediaFoundation::TrySourceProperty(GUID guid, int val)
{
	VARIANT var, var_min, var_max;
	VARIANT *pVar = NULL;
	ULONG count = 0;
	VariantInit(&var);
	VariantInit(&var_min);
	VariantInit(&var_max);

	HRESULT hr = m_pCodecApi->IsSupported(&guid);
	if ( SUCCEEDED(hr) ) {
		hr = m_pCodecApi->IsModifiable(&guid);
		if ( SUCCEEDED(hr) ) {
			hr = m_pCodecApi->GetValue(&guid, &var);
			if ( SUCCEEDED(hr) ) {
				if (var.vt == VT_BOOL) {
					var.boolVal = val;
				} else {
					var.uintVal = val;
				}
				hr = m_pCodecApi->SetValue(&guid, &var);
			}
		}
	}

	VariantClear(&var_max);
	VariantClear(&var_min);
	VariantClear(&var);
	for (unsigned int i = 0; i < count; i++) {
		VariantClear(&pVar[i]);
	}
	CoTaskMemFree(pVar);

	return hr;
}

HRESULT VS_CaptureDeviceMediaFoundation::FindExtensionNode(IKsTopologyInfo* pKsTopologyInfo, GUID extensionGuid)
{
	DWORD numberOfNodes;
	HRESULT hr = S_OK;

	hr = pKsTopologyInfo->get_NumNodes(&numberOfNodes);
	if (SUCCEEDED(hr)) {
		DWORD i;
		for (i = 0; i < numberOfNodes; i++) {
			GUID nodeGuid = {0};
			if (SUCCEEDED(pKsTopologyInfo->get_NodeType(i, &nodeGuid))) {
				if (IsEqualGUID(KSNODETYPE_DEV_SPECIFIC, nodeGuid)) {
					IKsControl *pKsControl = NULL;
					hr = pKsTopologyInfo->CreateNodeInstance(i, IID_IKsControl, (VOID**)&pKsControl);
					if (SUCCEEDED(hr)) {
					    KSP_NODE s;
                        ULONG ulBytesReturned = 0;
                        s.Property.Set = extensionGuid;
                        s.Property.Id = UVCX_VIDEO_CONFIG_PROBE;
                        s.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
                        s.NodeId = i;
						_uvcx_video_config_probe_commit_t h264State;
                        hr = pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), &(h264State), sizeof(h264State), &ulBytesReturned);
						if (SUCCEEDED(hr)) {
							m_pHardwareEncoder = new VS_HardwareEncoderDS();
							m_pHardwareEncoder->SetControl(pKsControl, i);
							return hr;
						}
						pKsControl->Release();
					}
				}
			}
		}
		if (i == numberOfNodes) hr = (HRESULT)(-1);
	}

	return hr;
}

HRESULT VS_CaptureDeviceMediaFoundation::ConfigureHardwareSinkSource(GUID guid_subtype)
{
	HRESULT hr = S_OK;

	if ( IsEqualGUID(guid_subtype, MFVideoFormat_H264) ) {
		m_eTypeHardware = ENCODER_H264_LOGITECH;
		m_hardwareRequest = HARDWARE_NEEDKEY;
		m_lastKeyRequest = 0;
		hr = m_pSource->QueryInterface(IID_ICodecAPI, (void**)&m_pCodecApi);
		if ( SUCCEEDED(hr) ) {
			m_pHardwareEncoder = new VS_HardwareEncoderMF();
			m_pHardwareEncoder->SetControl(m_pCodecApi, 0);

			HRESULT _hr = S_OK;

			GUID guid[] = {
				CODECAPI_AVEncCommonRateControlMode, /// need eAVEncCommonRateControlMode_CBR
				CODECAPI_AVEncCommonQualityVsSpeed, /// False for c930
				CODECAPI_AVEncCommonMeanBitRate, /// start bitrate 1MBps
				CODECAPI_AVEncMPVGOPSize, /// in frames (15 sec)
				CODECAPI_AVEncSliceControlMode, /// need  = 1, bits per slise
				CODECAPI_AVEncSliceControlSize, /// 1280 * 8
				CODECAPI_AVEncVideoMaxNumRefFrame, /// need = 1
				CODECAPI_AVEncH264CABACEnable, /// need VARIANT_FALSE
				CODECAPI_AVLowLatencyMode, /// need  = 1
				CODECAPI_AVEncAdaptiveMode, /// need eAVEncAdaptiveMode_FrameRate
				CODECAPI_AVEncVideoOutputFrameRateConversion, /// need eAVEncVideoOutputFrameRateConversion_Enable
			};

			int val_guid[] = {
				eAVEncCommonRateControlMode_CBR,
				50,
				1000000,
				m_renderFmt.dwFps * 15,
				1,
				1200 * 8,
				1,
				VARIANT_FALSE,
				TRUE,
				eAVEncAdaptiveMode_FrameRate,
				eAVEncVideoOutputFrameRateConversion_Enable,
			};

			int cnt = sizeof(guid) / sizeof(GUID);
			for (int i = 0; i < cnt; i++) {
				_hr = TrySourceProperty(guid[i], val_guid[i]);
			}
		} else {
			IKsTopologyInfo *pKsTopologyInfo = 0;
			hr = m_pSource->QueryInterface(__uuidof(IKsTopologyInfo), (void**)&pKsTopologyInfo);
			if ( SUCCEEDED(hr) ) {
				hr = FindExtensionNode(pKsTopologyInfo, GUID_UVCX_H264_XU);
				pKsTopologyInfo->Release();
			}
		}
	}

	return hr;
}

HRESULT VS_CaptureDeviceMediaFoundation::OpenMediaSource(IMFMediaSource *pSource, GUID guid_subtype)
{
	IMFAttributes *pAttributes = NULL;
	HRESULT hr = S_OK;
	hr = ConfigureHardwareSinkSource(guid_subtype);
	if ( SUCCEEDED(hr) ) {
		hr = g_MFCreateAttributes(&pAttributes, 3);
		if ( SUCCEEDED(hr) ) {
			hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, static_cast <IMFSourceReaderCallback*> (this));
			if ( SUCCEEDED(hr) ) {
				hr = ConfigurePropertySource();
				hr = g_MFCreateSourceReaderFromMediaSource(m_pSource,
														 pAttributes,
														 &m_pReader);
			}
		}
	}
	SafeRelease(&pAttributes);
	return hr;
}

void VS_CaptureDeviceMediaFoundation::GetPropertyState(PropertyDevice devProperty[])
{
	m_lockProperty.Lock();
	for (int i = 0; i < VS_CaptureDeviceMediaFoundation::MAXTYPES; i++) {
		memcpy(&devProperty[i], &m_propertyDev[i], sizeof(PropertyDevice));
	}
	m_lockProperty.UnLock();
}

HRESULT VS_CaptureDeviceMediaFoundation::SetPropertySource(IUnknown *pUnknown, long devProperty)
{
	HRESULT hr = E_FAIL;
	if (m_propertyDev[devProperty].valid) {
		if (devProperty < VS_CaptureDeviceMediaFoundation::FOCUS) {
			IAMVideoProcAmp *pProcAmp = (IAMVideoProcAmp*)pUnknown;
			hr = pProcAmp->Set(DeviceProp::id2ksproxy[devProperty], m_propertyDev[devProperty].val, m_propertyDev[devProperty].flag);
		} else {
			IAMCameraControl *pCtrlCamera = (IAMCameraControl*)pUnknown;
			hr = pCtrlCamera->Set(DeviceProp::id2ksproxy[devProperty], m_propertyDev[devProperty].val, m_propertyDev[devProperty].flag);
		}
	}
	return hr;
}

void VS_CaptureDeviceMediaFoundation::SetPropertyState(PropertyDevice devProperty[])
{
	IAMVideoProcAmp *pProcAmp = NULL;
	IAMCameraControl *pCtrlCamera = NULL;

	m_lockProperty.Lock();
	if (m_eDeviceState == DEVICE_START) {
		HRESULT hr_proc = m_pSource->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
		HRESULT hr_ctrl = m_pSource->QueryInterface(IID_IAMCameraControl, (void**)&pCtrlCamera);
		if (SUCCEEDED(hr_proc)) {
			HRESULT res = S_OK;
			for (int i = 0; i < VS_CaptureDeviceMediaFoundation::FOCUS; i++) {
				memcpy(&m_propertyDev[i], &devProperty[i], sizeof(PropertyDevice));
				res = SetPropertySource(pProcAmp, i);
			}
		}
		if (SUCCEEDED(hr_ctrl)) {
			HRESULT res = S_OK;
			for (int i = VS_CaptureDeviceMediaFoundation::FOCUS; i < VS_CaptureDeviceMediaFoundation::MAXTYPES; i++) {
				memcpy(&m_propertyDev[i], &devProperty[i], sizeof(PropertyDevice));
				res = SetPropertySource(pCtrlCamera, i);
			}
		}
	}
	m_lockProperty.UnLock();

	SafeRelease(&pProcAmp);
	SafeRelease(&pCtrlCamera);
}

HRESULT VS_CaptureDeviceMediaFoundation::GetPropertySource(IUnknown *pUnknown, long devProperty)
{
	HRESULT hr = E_FAIL;
	m_propertyDev[devProperty].valid = false;
	if (devProperty < VS_CaptureDeviceMediaFoundation::FOCUS) {
		IAMVideoProcAmp *pProcAmp = (IAMVideoProcAmp*)pUnknown;
		hr = pProcAmp->Get(DeviceProp::id2ksproxy[devProperty], &m_propertyDev[devProperty].val, &m_propertyDev[devProperty].flag);
		if ( SUCCEEDED(hr) ) {
			hr = pProcAmp->GetRange(DeviceProp::id2ksproxy[devProperty],
									&m_propertyDev[devProperty].min_val,
									&m_propertyDev[devProperty].max_val,
									&m_propertyDev[devProperty].step,
									&m_propertyDev[devProperty].default_val,
									&m_propertyDev[devProperty].flag_caps);
		}
	} else {
		IAMCameraControl *pCtrlCamera = (IAMCameraControl*)pUnknown;
		hr = pCtrlCamera->Get(DeviceProp::id2ksproxy[devProperty], &m_propertyDev[devProperty].val, &m_propertyDev[devProperty].flag);
		if ( SUCCEEDED(hr) ) {
			hr = pCtrlCamera->GetRange(DeviceProp::id2ksproxy[devProperty],
									   &m_propertyDev[devProperty].min_val,
									   &m_propertyDev[devProperty].max_val,
									   &m_propertyDev[devProperty].step,
									   &m_propertyDev[devProperty].default_val,
									   &m_propertyDev[devProperty].flag_caps);
		}
	}
	if ( SUCCEEDED(hr) ) {
		m_propertyDev[devProperty].valid = true;
	}
	return hr;
}

HRESULT VS_CaptureDeviceMediaFoundation::ConfigurePropertySource()
{
	IAMVideoProcAmp *pProcAmp = NULL;
	IAMCameraControl *pCtrlCamera = NULL;
	HRESULT hr_proc = m_pSource->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
	HRESULT hr_ctrl = m_pSource->QueryInterface(IID_IAMCameraControl, (void**)&pCtrlCamera);

	m_lockProperty.Lock();
	if ( SUCCEEDED(hr_proc) ) {
		HRESULT res = S_OK;
		for (int i = 0; i < VS_CaptureDeviceMediaFoundation::FOCUS; i++) {
			res = GetPropertySource(pProcAmp, i);
		}
	}
	if ( SUCCEEDED(hr_ctrl) ) {
		HRESULT res = S_OK;
		for (int i = VS_CaptureDeviceMediaFoundation::FOCUS; i < VS_CaptureDeviceMediaFoundation::MAXTYPES; i++) {
			res = GetPropertySource(pCtrlCamera, i);
		}
	}
	m_lockProperty.UnLock();

	SafeRelease(&pProcAmp);
	SafeRelease(&pCtrlCamera);
	return ( SUCCEEDED(hr_proc) || SUCCEEDED(hr_ctrl) ) ? S_OK : E_FAIL;
}

HRESULT VS_CaptureDeviceMediaFoundation::StartSinkSource()
{
	m_bFirstSample = true;
	m_llBaseTime = 0;
    HRESULT hr = m_pReader->ReadSample(m_idActiveStream, 0, NULL, NULL, NULL, NULL);
	if ( SUCCEEDED(hr) ) {
		m_eDeviceState = DEVICE_START;
	}
	return hr;
}

HRESULT VS_CaptureDeviceMediaFoundation::ConfigureCurrentType(IMFMediaType *pMediaType, CColorModeDescription *cmd, GUID guid_subtype)
{
	IMFMediaType *pType = NULL;
	HRESULT hr = g_MFCreateMediaType(&pType);
	if ( pMediaType && SUCCEEDED(hr) ) {
		hr = pMediaType->CopyAllItems(pType);
		if ( SUCCEEDED(hr) ) {
			if ( IsEqualGUID(guid_subtype, MFVideoFormat_MJPG) ) {
			    GUID subtypes[] = {
					MFVideoFormat_YUY2,
					MFVideoFormat_I420,
					MFVideoFormat_YV12,
					MFVideoFormat_UYVY,
					MFVideoFormat_RGB32,
					MFVideoFormat_RGB24
				};
				int type[] = {
					CColorSpace::YUY2,
					CColorSpace::I420,
					CColorSpace::YV12,
					CColorSpace::UYVY,
					CColorSpace::RGB32,
					CColorSpace::RGB24
				};
				for (UINT32 i = 0; i < ARRAYSIZE(subtypes); i++) {
					hr = pType->SetGUID(MF_MT_SUBTYPE, subtypes[i]);
					if ( SUCCEEDED(hr) ) {
						hr = m_pReader->SetCurrentMediaType(m_idActiveStream, NULL, pType);
						if ( SUCCEEDED(hr) ) {
							cmd->Color = type[i];
							break;
						}
					}
				}
			}
			if ( SUCCEEDED(hr) ) {
				UINT32 fixedSampleSize = FALSE;
				hr = pType->GetUINT32(MF_MT_FIXED_SIZE_SAMPLES, &fixedSampleSize);
				if ( SUCCEEDED(hr) ) {
					UINT32 sampleSize = 0;
					if (fixedSampleSize == TRUE) {
						hr = pType->GetUINT32(MF_MT_SAMPLE_SIZE, &sampleSize);
					} else {
						UINT32 width = 0, height = 0;
						hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
						if ( SUCCEEDED(hr) ) {
							sampleSize = width * height * 3 / 2;
						}
					}
					if ( SUCCEEDED(hr) ) {
						hr = g_MFCreateMemoryBuffer(sampleSize, &m_pMediaBuffer);
						if ( SUCCEEDED(hr) ) {
							m_pBuffer = new unsigned char [sampleSize+sizeof(DWORD)*3];
							memset(m_pBuffer, 0, sampleSize+sizeof(DWORD)*3);
							hr = m_pReader->SetCurrentMediaType(m_idActiveStream, NULL, pType);
							if ( SUCCEEDED(hr) ) {
								m_captureFmt.dwVideoWidht = cmd->Width;
								m_captureFmt.dwVideoHeight = cmd->Height;
								m_captureFmt.dwVideoCodecFCC = cmd->Color;
							}

						}
					}
				}
			}
		}
	}
	SafeRelease(&pType);
	return hr;
}

HRESULT VS_CaptureDeviceMediaFoundation::BuildSinkSource()
{
	HRESULT hr = E_FAIL;
	IMFAttributes *pAttributes = NULL;

	GUID guid_subtype = { 0 };
	IMFMediaType *pMediaType = NULL;
	CColorModeDescription cmd;
	int nVideoMode = -1;
	m_pModeList = m_pCaptureList->GetModeListByName(m_CurrentDeviceName);
	if (m_pModeList) {
		nVideoMode = FindOptimalVideoMode(m_renderFmt, 0);
		m_pModeList->iGetModeDescription(nVideoMode, &cmd);
		m_idActiveStream = m_pModeList->iGetPinNumber(nVideoMode);
		pMediaType = reinterpret_cast <IMFMediaType*> (m_pModeList->GetModeID(nVideoMode));
		if (pMediaType) {
			hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &guid_subtype);
		}
	}

	if ( SUCCEEDED(hr) ) {
		hr = g_MFCreateAttributes(&pAttributes, 2);
		if ( SUCCEEDED(hr) ) {
			hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
									  MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
			if ( SUCCEEDED(hr) ) {
				hr = E_FAIL;
				wchar_t *pSymbolicLink = (wchar_t*)m_pCaptureList->GetSymbolicLinkByName(m_CurrentDeviceName);
				if (pSymbolicLink) {
					hr = pAttributes->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, pSymbolicLink);
					if ( SUCCEEDED(hr) ) {
						hr = g_MFCreateDeviceSource(pAttributes, &m_pSource);
						if ( SUCCEEDED(hr) ) {
							hr = m_pSource->CreatePresentationDescriptor(&m_pPresentation);
							if ( SUCCEEDED(hr) ) {
								hr = OpenMediaSource(m_pSource, guid_subtype);
								if ( SUCCEEDED(hr) ) {
									BOOL bSelected = FALSE;
									hr = m_pReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
									hr = m_pReader->SetStreamSelection(m_idActiveStream, TRUE);
									hr = m_pReader->GetStreamSelection(m_idActiveStream, &bSelected);
									if ( SUCCEEDED(hr) && bSelected == TRUE ) {
										hr = ConfigureCurrentType(pMediaType, &cmd, guid_subtype);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	SafeRelease(&pAttributes);

	return hr;
}

int VS_CaptureDeviceMediaFoundation::InternalDisconnect(wchar_t *deviceName)
{
	m_eDeviceState = DEVICE_SHUTDOWN;

	m_lockCapture.Lock();

	delete m_pHardwareEncoder; m_pHardwareEncoder = 0;
	HRESULT hr = S_OK;
	SafeRelease(&m_pReader);
	SafeRelease(&m_pPresentation);
	SafeRelease(&m_pCodecApi);
	SafeRelease(&m_pSource);
	SafeRelease(&m_pMediaBuffer);
	delete [] m_pBuffer; m_pBuffer = 0;
	m_nOptimalVideoMode = -1;
	m_idActiveStream = 0;
	m_eTypeHardware = ENCODER_SOFTWARE;
	m_hardwareRequest = HARDWARE_UNDEF;
	memset(&m_stHardwareEncoder, 0, sizeof(m_stHardwareEncoder));
	CleanFramerate();

	m_lockFramerate.Lock();
	m_startFramerate = 3;
	m_setFramerate = m_last_setFramerate = 300;
	m_lockFramerate.UnLock();

	DTRACE(VSTM_VCAPTURE, "Set DEVICE_DISCONNECT: %S", deviceName);

	m_lockCapture.UnLock();

	return 0;
}

int VS_CaptureDeviceMediaFoundation::InternalConnect(wchar_t *deviceName, VS_MediaFormat *mf, int deviceMode)
{
	int ret = -1;

	InternalDisconnect(deviceName);

	m_lockCapture.Lock();

	m_renderFmt = *mf;
	m_nOptimalVideoMode = deviceMode;
	if (!deviceName || !*deviceName) {
		*m_CurrentDeviceName = 0;

		DTRACE(VSTM_VCAPTURE, "Connect capture: INCORRECT device name");

	} else {
		wcscpy(m_CurrentDeviceName, deviceName);

		DTRACE(VSTM_VCAPTURE, "Set DEVICE_CONNECT: %S, [%d x %d @ %d], %d",
								m_CurrentDeviceName, m_renderFmt.dwVideoWidht, m_renderFmt.dwVideoHeight, m_renderFmt.dwFps, m_nOptimalVideoMode);

		HRESULT hr = BuildSinkSource();
		if ( SUCCEEDED(hr) ) {
			hr = StartSinkSource();
			if ( SUCCEEDED(hr) ) {
				ret = 0;
			}
		}

		m_lockFramerate.Lock();
		m_startFramerate = (int)m_renderFmt.dwFps;
		m_setFramerate = m_startFramerate * 100;
		m_realFramerate = m_setFramerate;
		m_lockFramerate.UnLock();

		DTRACE(VSTM_VCAPTURE, "Connect capture: %S, ret = %d", m_CurrentDeviceName, ret);
	}

	m_lockCapture.UnLock();

	return ret;
}

int VS_CaptureDeviceMediaFoundation::InternalClosePropertyPage()
{
	m_lockProperty.Lock();
	if (m_hTtreadProperty) {
		CloseHandle(m_hTtreadProperty);
		m_hTtreadProperty = 0;
	}
	m_lockProperty.UnLock();
	return 0;
}

int VS_CaptureDeviceMediaFoundation::InternalPropertyPage()
{
	m_lockProperty.Lock();
	if (!m_hTtreadProperty) {
		m_hTtreadProperty = (HANDLE)_beginthreadex(0, 0, thread_property_proc_mf, this, 0, 0);
	}
	m_lockProperty.UnLock();
	return 0;
}

int VS_CaptureDeviceMediaFoundation::InternalDestroy()
{
	InternalDisconnect(m_CurrentDeviceName);
	SetEvent(m_hDestroy);
	return 0;
}

STDMETHODIMP VS_CaptureDeviceMediaFoundation::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample *pSample)
{
	m_lockCapture.Lock();

	HRESULT hr = S_OK;

	if (m_eDeviceState != DEVICE_START || FAILED(hrStatus)) {
		hr = hrStatus;
		m_lockCapture.UnLock();
		return hr;
	}

	m_hardwareLock.Lock();

	DWORD hwRequest = m_hardwareRequest;
	VS_HardwareMFEncoderState state;
	memcpy(&state, &m_stHardwareEncoder, sizeof(VS_HardwareMFEncoderState));
	m_hardwareRequest = HARDWARE_UNDEF;

	m_hardwareLock.UnLock();

	m_lockFramerate.Lock();

	if (m_last_setFramerate != m_setFramerate) {
		m_last_setFramerate = m_setFramerate;
		m_hardwareRequest |= HARDWARE_FRAMERATE;
		state.framerate = m_last_setFramerate;
		DTRACE(VSTM_VCAPTURE, "Set Framerate capture: %S, fps = %d", m_CurrentDeviceName, m_last_setFramerate);
	}

	m_lockFramerate.UnLock();

	if (m_eTypeHardware == ENCODER_H264_LOGITECH && m_pHardwareEncoder) {
		unsigned int ctime = timeGetTime();
		if (m_lastKeyRequest == 0) m_lastKeyRequest = ctime;
		if (ctime - m_lastKeyRequest > 15000) {
			hwRequest |= HARDWARE_NEEDKEY;
			m_lastKeyRequest = ctime;
		}
		if (hwRequest != HARDWARE_UNDEF) {
			if (hwRequest & HARDWARE_SETBITRATE) {
				m_pHardwareEncoder->SetBitrate(state.bitrate);
			}
			if (hwRequest & HARDWARE_NEEDKEY) {
				m_pHardwareEncoder->SetKeyFrame();
			}
			if (hwRequest & HARDWARE_FRAMERATE) {
				m_pHardwareEncoder->SetBitrate(state.framerate);
			}
		}
	}

	if (pSample) {
		if (m_bFirstSample) {
			m_llBaseTime = llTimestamp;
			m_bFirstSample = false;
		}
		llTimestamp -= m_llBaseTime;

		unsigned int ctime = timeGetTime();
		bool bSkipFrame = SnapFramerate(ctime, llTimestamp, m_last_setFramerate);
		int realFramerate = m_realFramerate;
		if ( GetFramerate(ctime, realFramerate) ) {
			m_lockFramerate.Lock();
			m_realFramerate = realFramerate;
			m_lockFramerate.UnLock();
		}
		if (m_eTypeHardware == ENCODER_H264_LOGITECH) bSkipFrame = false;

		if (!bSkipFrame) {
			hr = pSample->CopyToBuffer(m_pMediaBuffer);
			if ( SUCCEEDED(hr) ) {
				unsigned char *pBuffer = 0;
				DWORD pcbMaxLength, pcbCurrentLength;
				hr = m_pMediaBuffer->Lock(&pBuffer, &pcbMaxLength, &pcbCurrentLength);
				if ( SUCCEEDED(hr) ) {
					if (m_pVideoSlotObserver) {
						BITMAPINFOHEADER bih;
						CColorMode cm;
						cm.SetColorMode(NULL, m_captureFmt.dwVideoCodecFCC, m_captureFmt.dwVideoHeight, m_captureFmt.dwVideoWidht);
						cm.ColorModeToBitmapInfoHeader(&bih);
						m_pVideoSlotObserver->PushFrame(pBuffer, pcbCurrentLength,
														realFramerate, (DWORD)((double)llTimestamp / 10000.0),
														m_captureFmt.dwVideoWidht, m_captureFmt.dwVideoHeight, bih.biCompression,
														m_eTypeHardware != 0);
					}
				}
				m_pMediaBuffer->Unlock();
			}
		}
	}
	if ( SUCCEEDED(hr) ) {
		hr = m_pReader->ReadSample(dwStreamIndex, 0, NULL, NULL, NULL, NULL);
	}

	m_lockCapture.UnLock();

	return hr;
}

void VS_CaptureDeviceMediaFoundation::SetHWEncoderRequest(eHardwareRequest request, unsigned int iVal)
{
	m_hardwareLock.Lock();

	m_hardwareRequest |= request;
	if (request == HARDWARE_SETBITRATE) {
		m_stHardwareEncoder.bitrate = iVal;
	}
	if (request == HARDWARE_FRAMERATE) {
		m_stHardwareEncoder.framerate = iVal;
	}

	m_hardwareLock.UnLock();
}

void VS_HardwareEncoderDS::SetControl(void *pControl, int set_val)
{
	m_pKsControl = (IKsControl*)pControl;
	m_iNode = set_val;
	m_gExtensionGuid = GUID_UVCX_H264_XU;
	/// set params
	_uvcx_video_config_probe_commit_t h264State;
	h264State.wProfile = 0x4200;
	//h264State.wWidth = width;
	//h264State.wHeight = height;
	h264State.wSliceMode = SLICEMODE_BITSPERSLICE;
	h264State.wSliceUnits = 1200 * 8;
	//h264State.dwFrameInterval = (int)(10000000.0 / (double)framerate);
	h264State.wIFramePeriod = 15 * 1000;
	h264State.bRateControlMode = RATECONTROL_CBR;
	h264State.bEntropyCABAC = ENTROPY_CAVLC;
	h264State.bUsageType = 1;
	h264State.dwBitRate = 1000000;

	HRESULT hr;
    KSP_NODE s;
    ULONG ulBytesReturned = 0;
    s.Property.Set = m_gExtensionGuid;
    s.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
    s.NodeId = m_iNode;
	s.Property.Id = UVCX_VIDEO_CONFIG_PROBE;
	hr = m_pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), &h264State, sizeof(h264State), &ulBytesReturned);
	s.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
	hr = m_pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), &h264State, sizeof(h264State), &ulBytesReturned);
	s.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
	s.Property.Id = UVCX_VIDEO_CONFIG_COMMIT;
    hr = m_pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), &h264State, sizeof(h264State), &ulBytesReturned);
}

void VS_HardwareEncoderDS::SetBitrate(int val)
{
	KSP_NODE s;
    ULONG ulBytesReturned = 0;

    s.Property.Set = m_gExtensionGuid;
    s.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
    s.NodeId = m_iNode;
	s.Property.Id = UVCX_BITRATE_LAYERS;

	LPVOID pPropertyData = 0;
	ULONG dataLen = 0;
	_uvcx_bitrate_layers_t stBtr;

	stBtr.wLayerID = 0x0;
	stBtr.dwAverageBitrate = (val >> 16);
	stBtr.dwPeakBitrate = (val << 16) | (val >> 16);
	pPropertyData = &stBtr;
	dataLen = sizeof(_uvcx_bitrate_layers_t);

	m_pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), pPropertyData, dataLen, &ulBytesReturned);
}

void VS_HardwareEncoderDS::SetFramerate(int val)
{
	KSP_NODE s;
    ULONG ulBytesReturned = 0;

    s.Property.Set = m_gExtensionGuid;
    s.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
    s.NodeId = m_iNode;
	s.Property.Id = UVCX_FRAMERATE_CONFIG;

	LPVOID pPropertyData = 0;
	ULONG dataLen = 0;
	_uvcx_framerate_config_t stFPS;

	stFPS.wLayerID = 0x0;
	stFPS.dwFrameInterval = (val >> 16);
	pPropertyData = &stFPS;
	dataLen = sizeof(_uvcx_framerate_config_t);

	m_pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), pPropertyData, dataLen, &ulBytesReturned);
}

void VS_HardwareEncoderDS::SetKeyFrame()
{
	KSP_NODE s;
    ULONG ulBytesReturned = 0;

    s.Property.Set = m_gExtensionGuid;
    s.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
    s.NodeId = m_iNode;
	s.Property.Id = UVCX_PICTURE_TYPE_CONTROL;

	LPVOID pPropertyData = 0;
	ULONG dataLen = 0;
	_uvcx_picture_type_control_t stKey;

	stKey.wLayerID = 0x0;
	stKey.wPicType = 0x0002;
	pPropertyData = &stKey;
	dataLen = sizeof(_uvcx_picture_type_control_t);

	m_pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), pPropertyData, dataLen, &ulBytesReturned);
}

void VS_HardwareEncoderMF::SetControl(void *pControl, int set_val)
{
	m_pCodecApi = (ICodecAPI*)pControl;
}

void VS_HardwareEncoderMF::SetBitrate(int val)
{
	VARIANT var;
	VariantInit(&var);
	var.vt = VT_UI4;
	var.uintVal = val;
	HRESULT hr = m_pCodecApi->SetValue(&CODECAPI_AVEncCommonMeanBitRate, &var);
	VariantClear(&var);
}

void VS_HardwareEncoderMF::SetFramerate(int val)
{
	VARIANT var;
	VariantInit(&var);
	var.vt = VT_UI8;
	var.ullVal = (UINT64)(( ((UINT64)val) << 32 ) | ( 1 ));
	HRESULT hr = m_pCodecApi->SetValue(&CODECAPI_AVEncVideoOutputFrameRate, &var);
	VariantClear(&var);
}

void VS_HardwareEncoderMF::SetKeyFrame()
{
	VARIANT var;
	VariantInit(&var);
	var.vt = VT_UI4;
	var.uintVal = 1;
	HRESULT hr = m_pCodecApi->SetValue(&CODECAPI_AVEncVideoForceKeyFrame, &var);
	VariantClear(&var);
}

