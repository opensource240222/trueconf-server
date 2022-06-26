
#include "VSCapture.h"
#include "VS_Dmodule.h"
#include "ScreenCapturerFactory.h"
#include "../_Visicron/resource.h"
#include "../std/cpplib/VS_Protocol.h"
#include "std/cpplib/ThreadUtils.h"

const wchar_t VS_CaptureDeviceScreen::_nameScreenCapture[] = L"Screen Capturer:";
const wchar_t VS_CaptureDeviceScreen::_nameApplicationCapture[] = L"Application";

BOOL CALLBACK  DialogProcSC(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND activeHwnd = 0;
	static int activeApp = 0;
	switch (iMsg)
	{
		case WM_INITDIALOG :
			{
				VS_CaptureDeviceScreen *pCapture = (VS_CaptureDeviceScreen*)lParam;
				::SetWindowLong(hDlg, GWL_USERDATA, (long)lParam);
				pCapture->EnumerateApp();
				int numApp = pCapture->GetNumActiveApp();
				activeApp = 0;
				if (SendMessage(GetDlgItem(hDlg, IDC_COMBO_APPLIST), CB_GETCOUNT, 0, 0 ) == 0) {
					for (int i = 0; i < numApp; i++) {
						VS_CaptureDeviceScreen::PropertyApplication prop;
						pCapture->GetAppProperty(i, &prop);
						SendDlgItemMessage(hDlg, IDC_COMBO_APPLIST, CB_ADDSTRING, 0, (LPARAM)prop.nameApp);
						if (prop.hWnd == activeHwnd) activeApp = i;
					}
				}
				SendDlgItemMessage(hDlg, IDC_COMBO_APPLIST, CB_SETCURSEL, activeApp, 0);
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
				VS_CaptureDeviceScreen *pCapture = (VS_CaptureDeviceScreen*)::GetWindowLong(hDlg, GWL_USERDATA);
				switch (LOWORD (wParam))
				{
					case IDOK :
						{
							VS_CaptureDeviceScreen::PropertyApplication prop;
							activeApp = (int)SendMessage(GetDlgItem(hDlg, IDC_COMBO_APPLIST), CB_GETCURSEL, 0, 0);
							pCapture->GetAppProperty(activeApp, &prop);
							activeHwnd = prop.hWnd;
							ShowWindow(activeHwnd, SW_RESTORE);
							pCapture->SetCaptureApp(activeApp);
							EndDialog(hDlg, wParam);
							return TRUE;
						}
					case IDCANCEL:
						{
							EndDialog(hDlg, wParam);
							return TRUE;
						}
					case IDC_COMBO_APPLIST:
						{
							return TRUE;
						}
					break;
				}
			}
		case WM_DESTROY:
			{
				EndDialog(hDlg, wParam);
				return TRUE;
			}
	}
	return DefWindowProc(hDlg, iMsg, wParam, lParam);
}

static unsigned int WINAPI thread_property_proc_sc(void *pSelf)
{
	vs::SetThreadName("PropertySC");
	INT_PTR ret = 0;
	VS_CaptureDeviceScreen *pCapture = (VS_CaptureDeviceScreen*)pSelf;
	if (pCapture) {
		HMODULE hModule = GetModuleHandle("visicron.dll");
		if (hModule) {
			HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(IDD_PROPERTYCAPTURE_SC), RT_DIALOG);
			if (hResource) {
				HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
				if (hLoadedResource) {
					DLGTEMPLATEEX *t = (DLGTEMPLATEEX*)LockResource(hLoadedResource);
					HWND hwnd = pCapture->GetHwndPropertyPage();
					if (hwnd) {
						ret = DialogBoxIndirectParam((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), (LPDLGTEMPLATE)t, hwnd, (DLGPROC)DialogProcSC, (LPARAM)pCapture);
					}
				}
			}
		}
		pCapture->InternalClosePropertyPage();
	}
	return ret;
}

BOOL CALLBACK EnumWindowsScreen(HWND hwnd, LPARAM lParam)
{
	VS_CaptureDeviceScreen *pCapture = (VS_CaptureDeviceScreen*)lParam;
	VS_CaptureDeviceScreen::PropertyApplication prop;
	if (IsWindowVisible(hwnd)) {
		prop.hWnd = hwnd;
		if (GetWindowText(hwnd, prop.nameApp, 256) > 0) {
			pCapture->SetAppProperty(&prop);
		}
	}
	return TRUE;
}

VS_CaptureDeviceScreen::VS_CaptureDeviceScreen(VS_VideoCaptureSlotObserver *observerSlot) : VS_CaptureDevice(observerSlot)
{
	m_eDeviceState = DEVICE_SHUTDOWN;
	m_pDesktopCapture = ScreenCapturerFactory::Create(CapturerType::DEFAULT);
	m_pBuffer = 0;
	m_iSampleSize = 0;
	m_typeDevice = VS_CaptureDevice::SCREEN_CAPTURE;
	m_hTtreadProperty = NULL;
	m_hUpdateApp = CreateEvent(NULL, FALSE, FALSE, NULL);
	ActivateThread(this);
}

VS_CaptureDeviceScreen::~VS_CaptureDeviceScreen()
{
	DesactivateThread();

	m_lockApp.Lock();

	m_listApp.clear();

	m_lockApp.UnLock();

	if (m_hUpdateApp) CloseHandle(m_hUpdateApp);
	delete m_pDesktopCapture;
}

void VS_CaptureDeviceScreen::Sleep(wchar_t *szName, VS_MediaFormat *mf, int deviceMode, bool state)
{
	VS_AutoLock lock(this);
	if (!state) {
		VS_CaptureDeviceSettings set;
		set.hwndProp = m_hwndPropPage;
		Connect(szName, mf, deviceMode, &set);
	} else {
		Disconnect();
	}
}

bool VS_CaptureDeviceScreen::GetPropertyPage()
{
	SetEvent(m_hGetPropertyPage);
	return true;
}

int VS_CaptureDeviceScreen::InternalClosePropertyPage()
{
	m_lockProperty.Lock();
	if (m_hTtreadProperty) {
		CloseHandle(m_hTtreadProperty);
		m_hTtreadProperty = 0;
	}
	m_lockProperty.UnLock();
	return 0;
}

int VS_CaptureDeviceScreen::InternalPropertyPage()
{
	m_lockProperty.Lock();
	if (!m_hTtreadProperty) {
		m_hTtreadProperty = (HANDLE)_beginthreadex(0, 0, thread_property_proc_sc, this, 0, 0);
	}
	m_lockProperty.UnLock();
	return 0;
}

int VS_CaptureDeviceScreen::InternalConnect(wchar_t *deviceName)
{
	int ret = -1;
	InternalDisconnect(deviceName);
	if (m_pDesktopCapture) {
		if (m_pDesktopCapture->EnumerateOutputs()) {
			size_t numOutputs = m_pDesktopCapture->GetNumOutputs();
			for (size_t i = 0; i < numOutputs; i++) {
				ScreenCapturerDesc desc;
				m_pDesktopCapture->GetDescOutput(i, &desc);
				if (wcsstr(deviceName, desc.name) != 0) {
					m_captureFmt.SetVideo(desc.widthCaptureArea, desc.heightCaptureArea);
					m_iSampleSize = m_captureFmt.dwVideoWidht * m_captureFmt.dwVideoHeight * 4;
					m_pBuffer = new unsigned char [m_iSampleSize+3*sizeof(DWORD)];
					memset(m_pBuffer, 0x50, m_iSampleSize+3*sizeof(DWORD));
					if (m_pDesktopCapture->Init(desc.offsetX, desc.offsetY, desc.widthCaptureArea, desc.heightCaptureArea)) {
						UpdateCaptureState();
						ret = 0;
					}
					break;
				}
			}
			if (wcsstr(deviceName, _nameApplicationCapture) != 0) {
				if (m_pDesktopCapture && m_hwndPropPage != 0) {
					if (m_pDesktopCapture->Init(m_hwndPropPage)) {
						int w = m_pDesktopCapture->Width() &~ 1;
						int h = m_pDesktopCapture->Height() &~ 1;
						m_captureFmt.SetVideo(w, h);
						m_iSampleSize = m_captureFmt.dwVideoWidht * m_captureFmt.dwVideoHeight * 4;
						if (m_iSampleSize > 0) {
							m_pBuffer = new unsigned char[m_iSampleSize + 3 * sizeof(DWORD)];
							memset(m_pBuffer, 0x50, m_iSampleSize + 3 * sizeof(DWORD));
							UpdateCaptureState();
							ret = 0;
						}
					}
				}
			}
		}
	}
	g_DevStatus.SetStatus(DVS_SND_NOTWORK, false, ret != 0);
	return ret;
}

int VS_CaptureDeviceScreen::InternalDisconnect(wchar_t *deviceName)
{
	m_eDeviceState = DEVICE_SHUTDOWN;
	m_pDesktopCapture->Reset();
	delete [] m_pBuffer; m_pBuffer = 0;
	m_iSampleSize = 0;
	return 0;
}

int VS_CaptureDeviceScreen::ReinitApplication(int width, int height)
{
	if (width != m_captureFmt.dwVideoWidht || height != m_captureFmt.dwVideoHeight) {
		delete [] m_pBuffer;
		m_captureFmt.SetVideo(width, height);
		m_iSampleSize = m_captureFmt.dwVideoWidht * m_captureFmt.dwVideoHeight * 4;
		m_pBuffer = new unsigned char [m_iSampleSize+3*sizeof(DWORD)];
		memset(m_pBuffer, 0x50, m_iSampleSize+3*sizeof(DWORD));
		return UpdateCaptureState();
	}
	return 0;
}

int VS_CaptureDeviceScreen::UpdateCaptureState()
{
	BITMAPINFOHEADER bm;
	CleanFramerate();

	m_lockFramerate.Lock();
	m_realFramerate = (int)m_renderFmt.dwFps * 100;
	m_startFramerate = (int)m_renderFmt.dwFps;
	m_setFramerate = m_realFramerate;
	m_lockFramerate.UnLock();

	m_eDeviceState = DEVICE_BUILD;

	return 0;
}

bool VS_CaptureDeviceScreen::EnumerateApp()
{
	m_lockApp.Lock();

	m_listApp.clear();

	m_lockApp.UnLock();

	if (EnumWindows(EnumWindowsScreen, (LPARAM)this) == TRUE) {
		return true;
	}
	return false;
}

int VS_CaptureDeviceScreen::GetNumActiveApp()
{
	return m_listApp.size();
}

bool VS_CaptureDeviceScreen::SetAppProperty(PropertyApplication *prop)
{
	m_lockApp.Lock();

	m_listApp.push_back(*prop);

	m_lockApp.UnLock();

	return false;
}

bool VS_CaptureDeviceScreen::GetAppProperty(int index, PropertyApplication *prop)
{
	m_lockApp.Lock();

	if (index > m_listApp.size()) {
		m_lockApp.UnLock();
		return false;
	}
	*prop = m_listApp[index];

	m_lockApp.UnLock();

	return true;
}

void VS_CaptureDeviceScreen::SetCaptureApp(int index)
{
	m_lockApp.Lock();

	m_hwndPropPage = m_listApp[index].hWnd;
	SetEvent(m_hUpdateApp);

	m_lockApp.UnLock();
}

DWORD VS_CaptureDeviceScreen::Loop(LPVOID hEvDie)
{
	bool exit = false;
	HANDLE Handles[4] = { hEvDie,
						 m_hUpdateState,
						 m_hGetPropertyPage,
						 m_hUpdateApp};

	unsigned int ct = 0;
	unsigned int start_time = timeGetTime();
	unsigned int dt_framerate = 0;
	unsigned int next_time = start_time;
	unsigned int dt_wait = INFINITE;
	unsigned int cnt_skip_frame = 0;

	do {
		DWORD waitRes = WaitForMultipleObjects(4, Handles, FALSE, dt_wait);

		if (waitRes == WAIT_TIMEOUT) {
			Lock();
			if (!m_qDevState.empty()) {
				waitRes = WAIT_OBJECT_0 + 1;
			}
			UnLock();
		}

		switch(waitRes) {
			case WAIT_FAILED:
			case WAIT_OBJECT_0 + 0: /// End Of Thread
				{
					dt_wait = INFINITE;
					m_eDeviceState = DEVICE_SHUTDOWN;
					exit = true;
					break;
				}
			case WAIT_OBJECT_0 + 1: /// Change Device State
				{
					VS_CaptureDeviceState devState;

					Lock();
					if (!m_qDevState.empty()) {
						memcpy(&devState, &m_qDevState.front(), sizeof(VS_CaptureDeviceState));
						m_qDevState.pop();
					}
					UnLock();

					switch (devState.eAction) {
						case DEVICE_CONNECT:
							{
								m_eDeviceState = DEVICE_SHUTDOWN;
								if (wcsstr(devState.cDeviceName, _nameScreenCapture) != 0) {
									wcscpy(m_CurrentDeviceName, devState.cDeviceName);
									m_nOptimalVideoMode = devState.iOptimalMode;
									m_renderFmt = devState.mf;
									dt_framerate = (int)((1000.0 / (double)m_renderFmt.dwFps) + 0.5);
									DTRACE(VSTM_VCAPTURE, "Connect capture: Screen Capture, [%d x %d @ %d]", m_renderFmt.dwVideoWidht, m_renderFmt.dwVideoHeight, m_renderFmt.dwFps);
									InternalConnect(m_CurrentDeviceName);
								}
								break;
							}
						case DEVICE_DISCONNECT:
							{
								InternalDisconnect(m_CurrentDeviceName);
								dt_wait = 250;
								DTRACE(VSTM_VCAPTURE, "Disconnect Screen Capture");
								break;
							}
					}

					break;
				}
			case WAIT_OBJECT_0 + 2: /// Property page
				{
					if (wcsstr(m_CurrentDeviceName, _nameApplicationCapture) != 0) {
						InternalConnect(m_CurrentDeviceName);
					}
					break;
				}
			case WAIT_OBJECT_0 + 3: /// Change Device State
				{
					InternalConnect(m_CurrentDeviceName);
					break;
				}
		}

		if (m_eDeviceState < DEVICE_BUILD) continue;

		//m_lockFramerate.Lock();
		//if (m_last_setFramerate != m_setFramerate) {
		//	m_last_setFramerate = m_setFramerate;
		//	dt_framerate = (int)((1000.0 / (double)m_setFramerate * 100.0) + 0.5);
		//	DTRACE(VSTM_VCAPTURE, "Set Framerate capture: Screen Capture, fps = %d", m_last_setFramerate);
		//}
		//m_lockFramerate.UnLock();

		unsigned int ct = timeGetTime();
		int blt = -1;
		if (ct >= next_time) {
			bool ret = false;
			if (m_pDesktopCapture->Type() == ScreenCapturer::SCREEN_APPLICATION) {
				ret = m_pDesktopCapture->UpdateApplication();
			} else {
				ret = m_pDesktopCapture->UpdateScreen();
			}
			if (ret) {
				__int64 ms100ns = (__int64)ct * 10000;
				bool bSkipFrame = SnapFramerate(ct, ms100ns, m_setFramerate);
				int realFramerate = m_realFramerate;
				if ( GetFramerate(ct, realFramerate) ) {
					m_lockFramerate.Lock();
					m_realFramerate = realFramerate;
					m_lockFramerate.UnLock();
				}
				if (!bSkipFrame) {
					unsigned char *pFrame = 0;
					int w = 0, h = 0;
					int sampleSize = 0;
					if (m_pDesktopCapture->Type() == ScreenCapturer::SCREEN_APPLICATION) {
						w = m_pDesktopCapture->Width() &~ 1;
						h = m_pDesktopCapture->Height() &~ 1;
						ReinitApplication(w, h);
						unsigned char *pSrc = (unsigned char*)m_pDesktopCapture->GetScreen();
						unsigned char *pDst = (unsigned char*)m_pBuffer;
						for (int ic = 0; ic < m_captureFmt.dwVideoHeight; ic++) {
							memcpy(pDst, pSrc, m_captureFmt.dwVideoWidht * 4);
							pDst += m_captureFmt.dwVideoWidht * 4;
							pSrc += m_pDesktopCapture->Width() * 4;
						}
						pFrame = m_pBuffer;
						sampleSize = m_iSampleSize;
					} else {
						w = m_captureFmt.dwVideoWidht &~ 1;
						h = m_captureFmt.dwVideoHeight &~ 1;
						if (w != m_captureFmt.dwVideoWidht || h != m_captureFmt.dwVideoHeight) {
							unsigned char *pSrc = (unsigned char*)m_pDesktopCapture->GetScreen();
							unsigned char *pDst = (unsigned char*)m_pBuffer;
							for (int ic = 0; ic < h; ic++) {
								memcpy(pDst, pSrc, w * 4);
								pDst += w * 4;
								pSrc += m_captureFmt.dwVideoWidht * 4;
							}
							pFrame = m_pBuffer;
						} else {
							pFrame = (unsigned char*)m_pDesktopCapture->GetScreen();
						}
						sampleSize = w * h * 4;
					}
					if (m_pVideoSlotObserver) {
						m_pVideoSlotObserver->PushFrame(pFrame, sampleSize, m_realFramerate, ct,
														w, h, BI_RGB, false);
					}
					m_eDeviceState = DEVICE_START;
				}
			}
		}

		if (ct >= next_time) {
			next_time += dt_framerate;
		}
		ct = timeGetTime();
		dt_wait = 0;
		if (ct < next_time) {
			dt_wait = next_time - ct;
			cnt_skip_frame = 0;
		} else {
			if (cnt_skip_frame >= 3) {
				cnt_skip_frame = 0;
				while (next_time <= ct) next_time += dt_framerate;
			} else {
				cnt_skip_frame++;
			}
		}

	} while (!exit);

	return 0;
}