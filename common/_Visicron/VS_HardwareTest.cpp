#include "../VSClient/VSClientBase.h"
#include "../VSClient/VSAudio.h"
#include "../VSClient/VSCapture.h"
#include "../VSClient/VSVideoCaptureList.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "../std/cpplib/VS_MediaFormat.h"
#include "../std/cpplib/VS_Utils.h"
#include "../std/cpplib/VS_VideoLevelCaps.h"
#include "../Transcoder/VideoCodec.h"
#include "Transcoder/GetTypeHardwareCodec.h"
#include "version.h"

void VSGetDirectDrawInfo(VS_SimpleStr &prop);
int VSGetDirect3DInfo(VS_SimpleStr &prop, HWND hwnd);

void my_puts(FILE *f, const char *s) {
	fprintf(f, "%s\n", s);
}

#define puts(x) my_puts(f, (x))

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam)
{
    return  DefWindowProc(hWnd, msg, wParam, lParam);
}

WNDCLASS camwndclass =
{
	CS_PARENTDC,
	&MsgProc,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	"$$d3dtest"
};

int VS_HardwareTest()
{
	if (!VS_RegistryKey::InitDefaultBackend("registry:force_lm=false"))
		throw std::runtime_error("Can't initialize registry backend!");

	HWND hwnd = NULL;
#ifdef _SVKS_M_BUILD_
	VS_RegistryKey::SetDefaultRoot("SVKS-M\\Client");
#else
	VS_RegistryKey::SetDefaultRoot("TrueConf\\Online");
#endif

	VS_SysBenchmarkWindows *pSysBench = new VS_SysBenchmarkWindows();
	if (!pSysBench) return -1;
	pSysBench->Run();
	DWORD ret = WaitForSingleObject(pSysBench->GetBenchEvent(), INFINITE);
	if (ret != WAIT_OBJECT_0) {
		delete pSysBench;
		return -1;
	}

	_variant_t vr;
	FILE *f = fopen("vp_configuration.txt", "wt");
	if (f==NULL) {
		return -1;
		delete pSysBench;
	}

	puts("*****************************************************************");
	puts("Main system section");
	puts("*****************************************************************");
	VS_SimpleStr tt(1000);
	tu::TimeToGStr(std::chrono::system_clock::now(), tt.m_str, 1000);
	puts(tt);
	VSGetSystemInfo_OS(tt);
	puts(tt);
	VSGetSystemInfo_Processor(tt);
	puts(tt);
	fprintf(f, "DLL version %s\n", STRFILEVER);
	HRESULT res = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if(!((res==S_OK)||(res==S_FALSE)) )
		puts("COM Fail !");
	puts("*****************************************************************");
	puts("Direct3D caps");
	puts("*****************************************************************");
	tt.Empty();
	ATOM myWndClass = RegisterClass(&camwndclass);
	hwnd = CreateWindow((LPCSTR)myWndClass, "d3dtestapp", 0, 0, 0, 320, 240, 0, 0, 0, 0);
	CVideoRenderBase::Open();
	int d3d = VSGetDirect3DInfo(tt, hwnd);
	CVideoRenderBase::Close();
	puts(tt);
	if (d3d == 0) {
		puts("*****************************************************************");
		puts("DirectX caps");
		puts("*****************************************************************");
		tt.Empty();
		VSGetDirectDrawInfo(tt);
		puts(tt);
	}

	puts("\n*****************************************************************");
	puts("Hardware caps");
	puts("*****************************************************************");

	char th2name[MAX_NUM_HW_ENCODERS][128] =
	{
		"Software",
		"Logitech H.264",
		"Intel H.264",
		"Intel H.264 [mss = 1400]",
		"Nvidia H.264"
	};

	eHardwareEncoder typeEnc = GetTypeHardwareCodec();
	fprintf(f, "encoder type: %s \n\n", th2name[(int)typeEnc]);

	///< get Audio Device Propet
	CStringList captList, rndList;
	VS_AudioDeviceManager::Open(hwnd);
	VS_AudioDeviceManager::GetDeviceList(true, &captList);
	VS_AudioDeviceManager::GetDeviceList(false, &rndList);

	puts("*****************************************************************");
	puts("Audio render section");
	puts("*****************************************************************");

	setlocale(LC_CTYPE, "Russian_Russia.1251");
	int NumOfDevDS = rndList.iGetMaxString();
	int NumOfDev = waveOutGetNumDevs();
	if (NumOfDev && NumOfDevDS) {
		fprintf(f, "Found : %d device(s)\n", NumOfDev);
		for (int j = 0; j < NumOfDevDS; j++) {
			puts("-------------------------------");
			fprintf(f, "Device N %d\n %S\n", j, rndList.szGetStringByNumber(j));
			if (j > 0) {
				for (int i = 0; i < NumOfDev; i++) {
					WAVEOUTCAPS caps;
					MMRESULT res = waveOutGetDevCaps(i, &caps, sizeof(WAVEOUTCAPS));
					if (res==MMSYSERR_NOERROR){
						_bstr_t bs = caps.szPname;
						wchar_t *pch = wcsstr(rndList.szGetStringByNumber(j), (const wchar_t*)bs);
						if (pch) {
							fprintf(f, "---- Mid %d, Pid %d, Driver Version %x, Formats %lx, Channels %d \n",
								caps.wMid, caps.wPid, caps.vDriverVersion, caps.dwFormats, caps.wChannels);
							if (caps.dwSupport&WAVECAPS_LRVOLUME)
								puts("---- Supports separate left and right volume control");
							if (caps.dwSupport&WAVECAPS_PITCH)
								puts("---- Supports pitch control");
							if (caps.dwSupport&WAVECAPS_PLAYBACKRATE)
								puts("---- Supports playback rate control");
							if (caps.dwSupport&WAVECAPS_SYNC)
								puts("---- The driver is synchronous and will block while playing a buffer");
							if (caps.dwSupport&WAVECAPS_VOLUME)
								puts("---- Supports volume control");
							if (caps.dwSupport&WAVECAPS_SAMPLEACCURATE)
								puts("---- Returns sample-accurate position information");
							break;
						}
					}
					else if (res==MMSYSERR_BADDEVICEID)
						puts("Specified device identifier is out of range.");
					else if (res==MMSYSERR_NODRIVER)
						puts("No device driver is present.");
					else if (res==MMSYSERR_NOMEM)
						puts("Unable to allocate or lock memory.");
					else
						puts("Unknown error");
				}
			}
			fprintf(f, "---- Maximum SampleRate %d Hz\n", VS_AudioDeviceManager::GetMaxSampleRate(j, false));
		}
	}

	puts("*****************************************************************");
	puts("Audio capture section");
	puts("*****************************************************************");

	NumOfDevDS = captList.iGetMaxString();
	NumOfDev = waveInGetNumDevs();
	if (NumOfDev && NumOfDevDS) {
		fprintf(f, "Found : %d device(s)\n", NumOfDev);
		for (int j = 0; j < NumOfDevDS; j++) {
			puts("-------------------------------");
			fprintf(f, "Device N %d\n %S\n", j, captList.szGetStringByNumber(j));
			if (j > 0) {
				for (int i = -1; i<NumOfDev; i++) {
					WAVEINCAPS caps;
					MMRESULT res = waveInGetDevCaps(i, &caps, sizeof(WAVEINCAPS));
					if (res==MMSYSERR_NOERROR){
						_bstr_t bs = caps.szPname;
						wchar_t *pch = wcsstr(captList.szGetStringByNumber(j), (const wchar_t*)bs);
						if (pch) {
							fprintf(f, "---- Mid %d, Pid %d, Driver Version %x, Formats %lx, Channels %d \n",
								caps.wMid, caps.wPid, caps.vDriverVersion, caps.dwFormats, caps.wChannels);
						}
					}
					else if (res==MMSYSERR_BADDEVICEID)
						puts("Specified device identifier is out of range.");
					else if (res==MMSYSERR_NODRIVER)
						puts("No device driver is present.");
					else if (res==MMSYSERR_NOMEM)
						puts("Unable to allocate or lock memory.");
					else
						puts("Unknown error");
				}
			}
			fprintf(f, "---- Maximum SampleRate %d Hz\n", VS_AudioDeviceManager::GetMaxSampleRate(j, true));
		}
	}

	VS_AudioDeviceManager::Close();
	DestroyWindow(hwnd);
	UnregisterClass("$$d3dtest", GetModuleHandle(0));

	puts("*****************************************************************");
	puts("Video capture section");
	puts("*****************************************************************");

	VS_CaptureDevice::Open();
	HANDLE hGetFrame = CreateEvent(NULL, FALSE, FALSE, NULL);
	CVideoCaptureList *pClist = CVideoCaptureList::Create(CVideoCaptureList::CAPTURE_MF, 0, VIDEO_LEVEL_4K_MAX);
	VS_CaptureDevice *m_pCaptureVideo[CVideoCaptureList::CAPTURE_MAX];
	m_pCaptureVideo[CVideoCaptureList::CAPTURE_DS] = VS_CaptureDevice::Create(VS_CaptureDevice::DIRECT_SHOW, 0, pClist);
	m_pCaptureVideo[CVideoCaptureList::CAPTURE_MF] = VS_CaptureDevice::Create(VS_CaptureDevice::MEDIA_FOUNDATION, 0, pClist);
	m_pCaptureVideo[CVideoCaptureList::CAPTURE_SCREEN] = VS_CaptureDevice::Create(VS_CaptureDevice::SCREEN_CAPTURE, 0, pClist);

	if (pClist->Process(RUN_COMMAND, "VideoCaptureList", 0) != 0) {

		puts("Video capture devices fail !!!");

	} else {

		char type2name[CVideoCaptureList::CAPTURE_MAX][256] =
		{
			"DirectShow",
			"Media Foundation",
			"Screen Capture"
		};

		int i, k;
		int n = pClist->iGetDeviceCounter();
		tc_VideoLevelCaps *pLevelCaps = new tc_VideoLevelCaps();
		tc_AutoLevelCaps *pAutoLevelCaps = new tc_AutoLevelCaps();
		unsigned char sndLvl, rcvLvl, rtnLvl;
		fprintf(f, "Found : %d device(s)\n", n);

		for (i = 0; i < n; i++) {

			std::wstring devName(pClist->m_pSourceList->szGetStringByNumber(i));
			char nbuff[256] = {0};
			wcstombs(nbuff, devName.c_str(), 255);
			CVideoCaptureList::eCapturerType type = pClist->GetTypeCapturer(devName.c_str());
			puts("===========================================");
			fprintf(f, "Device N %d\n %s [%s]\n", i + 1, nbuff, type2name[type]);
			CModeList *pModeList = pClist->GetModeListByName(devName.c_str());

			if (pModeList) {

				puts("---------- Supported Video Modes ----------");
				int j;
				int nm = pModeList->iGetMaxMode();

				for (j = 0; j < nm; j++) {

					CColorModeDescription cmd;
					pModeList->iGetModeDescription(j, &cmd);
					fprintf(f, "---- CAMERA MODE %3d ----\n", j);
					fprintf(f, "     Resolution : %4d x %4d -> %4d x %4d, dw = %2d dh = %2d; WxH orig: %4d x %4d (use: %4d x %4d) \n",
							cmd.WidthMin, cmd.HeightMin, cmd.WidthMax, cmd.HeightMax,
							cmd.dW, cmd.dH, cmd.WidthBase, cmd.HeightBase, cmd.Width, cmd.Height);
					fprintf(f, "     FrameRate  : (%4.2f, %4.2f); FrameRate List :",
							10000000.0 / cmd.FrameIntMax, 10000000.0 / cmd.FrameIntMin);

					if (cmd.FrameIntList.empty()) {
						puts(" Unsupported");
					} else {
						for (k = 0; k < (int)cmd.FrameIntList.size(); k++) {
							if (k == 0) fprintf(f, " (%4.2f", 10000000.0 / cmd.FrameIntList[k]);
							else fprintf(f, " %4.2f", 10000000.0 / cmd.FrameIntList[k]);
							if (k == cmd.FrameIntList.size() - 1) fprintf(f, ")\n");
							else fprintf(f, ",");
						}
					}

					fprintf(f, "     Interlaced : %d \n", (cmd.bInterlace) ? 1 : 0);
					fprintf(f, "     Color      : ");

					switch(cmd.Color)
					{
					case CColorSpace::YUYV:puts("YUYV");break;
					case CColorSpace::CLPL:puts("CLPL");break;
					case CColorSpace::IYUV:puts("IYUV");break;
					case CColorSpace::I420:puts("I420");break;
					case CColorSpace::YVU9:puts("YVU9");break;
					case CColorSpace::Y411:puts("Y411");break;
					case CColorSpace::Y41P:puts("Y41P");break;
					case CColorSpace::YUY2:puts("YUY2");break;
					case CColorSpace::YVYU:puts("YVYU");break;
					case CColorSpace::UYVY:puts("UYVY");break;
					case CColorSpace::HDYC:puts("HDYC");break;
					case CColorSpace::H264:puts("H264");break;
					case CColorSpace::H264_ES:puts("H264_ES");break;
					case CColorSpace::Y211:puts("Y211");break;
					case CColorSpace::YV12:puts("YV12");break;
					case CColorSpace::CLJR:puts("CLJR");break;
					case CColorSpace::IF09:puts("IF09");break;
					case CColorSpace::CPLA:puts("CPLA");break;
					case CColorSpace::RGB24:puts("RGB24");break;
					case CColorSpace::RGB32:puts("RGB32");break;
					case CColorSpace::MJPEG:puts("MJPEG");break;
					case CColorSpace::DVSD:puts("DVSD");break;
					case CColorSpace::DVHD:puts("DVHD");break;
					case CColorSpace::DVSL:puts("DVSL");break;
					case CColorSpace::RGB1:puts("RGB1");break;
					case CColorSpace::RGB4:puts("RGB4");break;
					case CColorSpace::RGB8:puts("RGB8");break;
					case CColorSpace::RGB565:puts("RGB565");break;
					case CColorSpace::RGB555:puts("RGB555");break;
					case CColorSpace::NV12:puts("NV12");break;
					default: {
								puts("UNKNOWN");break;
							 }
					}

					fprintf(f, "     Stream     : %d \n", pModeList->iGetPinNumber(j));
				}

				puts("-------- Supported Auto Video Modes -------");

				for (j = pAutoLevelCaps->GetMinLevel(); j <= pAutoLevelCaps->GetMaxLevel(); j++) {
					tc_levelVideo_t lvlDesc;
					tc_AutoModeDesc_t modeDesc;
					if (pLevelCaps->GetLevelDesc(j, &lvlDesc)) {
						if (pAutoLevelCaps->GetLevelDesc(j, ENCODER_SOFTWARE, &modeDesc)) {
							pClist->SetAutoLevel(j, modeDesc.ar);
							pClist->UpdateSenderLevel(j, false);
							tc_LevelModeState lvlState = pClist->GetLevelState(pClist->m_pSourceList->szGetStringByNumber(i));
							int swMode = lvlState.nVideoMode;
							pClist->UpdateSenderLevel(j, true);
							lvlState = pClist->GetLevelState(pClist->m_pSourceList->szGetStringByNumber(i));
							int hwMode = lvlState.nVideoMode;
							int hdMode = pClist->GetAutoModeHD(pClist->m_pSourceList->szGetStringByNumber(i));
							if (lvlState.nVideoMode >= 0) {
								fprintf(f, "LEVEL %5s [MBps = %9d, MBframe = %6d, ar = %d] : mode [sw = %3d, hw = %3d, hd = %3d]\n",
											lvlDesc.name, lvlDesc.maxMBps, lvlDesc.maxFrameSizeMB, modeDesc.ar, swMode, hwMode, hdMode);
							}
						}
					}
				}

				puts("------------ Optimal Video Level -----------");

				fprintf(f, "     BENCHMARK: %d\n", pSysBench->GetBenchMBps(ENCODER_SOFTWARE));
				fprintf(f, " SND BENCHMARK: %d\n", pSysBench->GetSndMBps(ENCODER_SOFTWARE));
				fprintf(f, " RCV BENCHMARK: %d\n", pSysBench->GetRcvMBps(ENCODER_SOFTWARE));
				rtnLvl = pSysBench->GetBenchLevel(ENCODER_SOFTWARE);
				sndLvl = pSysBench->GetSndLevel(ENCODER_SOFTWARE);
				rcvLvl = pSysBench->GetRcvLevel();
				tc_levelVideo_t lvlDesc;
				tc_AutoModeDesc_t modeDesc;

				if (pLevelCaps->GetLevelDesc(rtnLvl, &lvlDesc)) {
					fprintf(f, "  RATING LEVEL: %5s [MBframe = %6d, MBps = %9d]\n", lvlDesc.name, lvlDesc.maxFrameSizeMB, lvlDesc.maxMBps);
					sndLvl = pAutoLevelCaps->CheckLevel(sndLvl);
					pAutoLevelCaps->GetLevelDesc(sndLvl, ENCODER_SOFTWARE, &modeDesc);
					pClist->SetAutoLevel(sndLvl, modeDesc.ar);
					if (pLevelCaps->GetLevelDesc(sndLvl, &lvlDesc)) {
						fprintf(f, "  SENDER LEVEL: %5s [MBframe = %6d, MBps = %9d]\n", lvlDesc.name, lvlDesc.maxFrameSizeMB, lvlDesc.maxMBps);
					}
				}

				if (pLevelCaps->GetLevelDesc(rcvLvl, &lvlDesc)) {
					fprintf(f, "RECEIVER LEVEL: %5s [MBframe = %6d, MBps = %9d]\n", lvlDesc.name, lvlDesc.maxFrameSizeMB, lvlDesc.maxMBps);
				}
			}
		}

		puts("===========================================");

		for (i = 0; i < n; i++) {

			std::wstring devName(pClist->m_pSourceList->szGetStringByNumber(i));
			char nbuff[256] = {0};
			wcstombs(nbuff, devName.c_str(), 255);
			fprintf(f, "Trying to connect to %s ... ", nbuff);
			CModeList *pModeList = pClist->GetModeListByName(devName.c_str());
			if (pModeList) {

				VS_CaptureDeviceSettings devSettings;
				memset(&devSettings, 0, sizeof(devSettings));
				CColorModeDescription cmd;
				pModeList->iGetModeDescription(0, &cmd);
				VS_MediaFormat mf;
				mf.SetVideo(320, 240, VS_VCODEC_VPX);
				CVideoCaptureList::eCapturerType type = pClist->GetTypeCapturer(devName.c_str());
				VS_CaptureDevice *pCaptureDeviceCurr = m_pCaptureVideo[type];

				if (pModeList && pCaptureDeviceCurr->Connect((wchar_t*)devName.c_str(), &mf, 0, &devSettings) == 0) {

					tc_AutoModeDesc_t modeDesc;
					sndLvl = pSysBench->GetSndLevel(ENCODER_SOFTWARE);
					sndLvl = pAutoLevelCaps->CheckLevel(sndLvl);
					pAutoLevelCaps->GetLevelDesc(sndLvl, ENCODER_SOFTWARE, &modeDesc);
					pClist->SetAutoLevel(sndLvl, modeDesc.ar);
					tc_LevelModeState lvlState = pClist->GetLevelState(pClist->m_pSourceList->szGetStringByNumber(i));
					int mode = lvlState.nVideoMode;
					mf.SetVideo(lvlState.nWidth, lvlState.nHeight, modeDesc.fourcc, modeDesc.fps);

					fprintf(f, "Success !\n");
					__int64 li = 0;

					if ((li = pCaptureDeviceCurr->GetVideoModes()) >= 0) {
						int hi = (int)(li >> 32);
						int lw = (int)(li & 0xffffffff);
						fprintf(f, " Found video mode : %x %x\n", hi, lw);
					}

					if (pCaptureDeviceCurr->GetPins(&vr) >= 0) {
						long l, u;
						long m;
						if (vr.vt & (VT_ARRAY|VT_VARIANT) ) {
							SAFEARRAY *psa = vr.parray;
							SafeArrayGetLBound(psa, 1, &l);
							SafeArrayGetUBound(psa, 1, &u);
							fprintf(f, "Found %ld input pin(s)\n", u - l + 1);
							for (m = l; m <= u; m++) {
								VARIANT vr;
								SafeArrayGetElement(psa, &m, &vr);
								fprintf(f, "Pin number %ld Type: %S\n", m + 1, (wchar_t *)(_bstr_t)vr);
							}
						}
					}

				} else {
					fprintf(f, "Failed : Connect!\n");
				}

				pCaptureDeviceCurr->Disconnect();

			} else {

				fprintf(f, "Failed : Invalid video modes! \n");

			}
		}

		delete pLevelCaps;
	}

	if (hGetFrame) CloseHandle(hGetFrame);
	for (int i = 0; i < CVideoCaptureList::CAPTURE_MAX; i++) {
		delete m_pCaptureVideo[i];
	}
	delete pClist;

	VS_CaptureDevice::Close();

	CoUninitialize();

	delete pSysBench;

	fclose(f);
	return 0;
}