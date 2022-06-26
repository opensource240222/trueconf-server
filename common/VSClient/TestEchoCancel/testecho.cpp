#include "../VSAudio.h"
#include "../VSAudioDs.h"
#include "../VSAudioNew.h"
#include "../VS_ApplicationInfo.h"
#include "../VS_Dmodule.h"
#include "../../std/cpplib/VS_MediaFormat.h"
#include "../../transcoder/AudioCodec.h"
#include "../../transcoder/VS_IppAudiCodec.h"
#include "../../Audio/WinApi/dsutil.h"
#include "../../std/cpplib/VS_RegistryKey.h"

#include "../../Audio/EchoCancel/SpeexEchoCancel.h"
#include "../../Audio/echocancel/EchoCancelDll.h"
#include "../VSAudioEchoCancel.h"

#include <speex/speex_resampler.h>
#include <speex/speex_preprocess.h>

#include <conio.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam)
{
	//CVideoRenderBase::WindowProc(hWnd, msg, wParam, lParam);
    switch (msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return  DefWindowProc(hWnd, msg, wParam, lParam);
}

bool CreateMainWindow(HWND &hWnd)
{
    WNDCLASS wc =
    {
        CS_CLASSDC | CS_HREDRAW | CS_VREDRAW,
        MsgProc,
        0L,
        0L,
        NULL,
        LoadIcon(NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW),
        NULL,
        NULL,
        "ClassWindow",
    };
	if (!RegisterClass(&wc)) return false;
	RECT rect;
	rect.top = 0;
	rect.left = 0;
	rect.right = 10;
	rect.bottom = 10;
	AdjustWindowRect(&rect, WS_POPUP | WS_SIZEBOX, TRUE);
    hWnd = CreateWindow(
				"ClassWindow",
                "NameWindow",
                WS_POPUP | WS_SIZEBOX,
				0, 0, 10, 10,
                NULL,
                NULL,
                NULL,
                NULL);
    if (!hWnd) return false;
	//ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
    return true;
}

int main0(int argc, char* argv[]);
int main1(int argc, char* argv[]);
int main2(int argc, char* argv[]);
int main3(int argc, char* argv[]);
int main4(int argc, char* argv[]);
int main5(int argc, char* argv[]);
int main6(int argc, char* argv[]);
int main7(int argc, char* argv[]);
int main8(int argc, char* argv[]);
int main9(int argc, char* argv[]);

int main(int argc, char* argv[])
{
	return main4(argc, argv);
	//return main5(argc, argv);
}

int main2(int argc, char* argv[])
{
	if (argc < 4) {
		printf("Usage: *.exe 0 frequency near_end.wav far_end.wav\n");
		printf("or *.exe 1 frequency far_end.wav\n");
		return -1;
	}
	int n = atoi(argv[1]);
	if (n == 0) {
		return main6(argc, argv);
		//return main7(argc, argv);
		//return main4(argc, argv);
	} else {
		return main6(argc, argv);
	}
}

int main3(int argc, char* argv[])
{
	HWND hwnd;
	HANDLE handles[2];
	CoInitialize(0);

	FILE *near_end, *far_end, *echo;
	//near_end = fopen("near_end.pcm", "wb");
	//far_end = fopen("far_end.pcm", "rb");
	//echo = fopen("echo.pcm", "wb");

	if (argc < 4) {
		printf("Usage: *.exe 1 frequency far_end.wav\n");
		return -1;
	}

	if (VS_RetriveEchoCancel(0) == 0) {
		printf("Error: can't create aec\n");
		return -1;
	}

	int freq = atoi(argv[2]);

	char temp[256] = {0};
	sprintf(temp, "Visicron\\test_echo\\");
	VS_RegistryKey::SetRoot(temp);
	g_pDtrase = new VS_DebugOut;

	if (!CreateMainWindow(hwnd))
		return -2;

	VS_AudioDeviceManager::Open(hwnd);
	VS_MediaFormat fmt;
	VS_ACaptureDevice *din = new VS_ACaptureDevice;
	VS_ARenderDevice *dout = new VS_ARenderDevice;
	VS_AudioCapture *acapt = new VS_AudioCapture(NULL, NULL, &fmt);

	int tmp = 2;
	_variant_t *pVar = new _variant_t(tmp);
	acapt->ProcessFunction(SET_PARAM, "UseXPAec", pVar);
	delete pVar;

	int DurrTime = 200;
	int len = 0, num = 0, size = 100 * 16000 / 1000 * 2, len_far = 0,
		d_far = DurrTime * 16 * 2, d_out = 0,
		last_sample = -65536,
		last_durration[20], num_dur = 0, avg_durr = 0;
	char *buff_echo = (char*)malloc((DurrTime * 16000 * 2) / 1000 * 25),
		 *buff_near = (char*)malloc((DurrTime * 16000 * 2) / 1000 * 25),
		 *buff_rend = (char*)malloc((DurrTime * 16000 * 2) / 1000 * 25),
		 *buff_far  = (char*)malloc((2000 * 16000 * 2) / 1000 * 2);

	CWaveFile *m_outf = new CWaveFile;
	WAVEFORMATEX wf;
	m_outf->Open(argv[3], &wf, WAVEFILE_READ);
	wf.nAvgBytesPerSec = freq*2;
	wf.nBlockAlign = 2;
	wf.nChannels = 1;
	wf.nSamplesPerSec = freq;
	wf.wBitsPerSample = 16;
	wf.wFormatTag = WAVE_FORMAT_PCM;

	fmt.SetAudio(freq, VS_ACODEC_PCM, 100);
	din->Init(-1, &fmt);
	dout->Init(-1, &fmt);
	handles[0] = din->GetCmpleteEvent();
	handles[1] = dout->GetCmpleteEvent();
	din->SetVolume(0x8000);
	//din->SetVolume(0xffff);
	dout->SetVolume(0xffff);
	din->Start();
	SetEvent(handles[1]);

	int StartTime = timeGetTime();

	while (1) {
		DWORD dwret = WaitForMultipleObjects(2, handles, FALSE, 100);
		StartTime = timeGetTime();
		if (dwret == WAIT_OBJECT_0 + 0) {
			while (din->Capture(buff_near, len, false)>0) {
				//fwrite(buff_near, len, 1, echo); // echo resudial
			}
		} else if (dwret == WAIT_OBJECT_0 + 1) {
			int res = 1;
			DWORD szread = 1;
			while (dout->GetCurrBuffDurr() < DurrTime) {
				//res = fread(buff_rend , size, 1, far_end);
				szread = 0;
				m_outf->Read((BYTE*)buff_rend, size, &szread);
				//if (!res) break;
				if (!szread) break;
				//dout->Play(buff_rend, size);
				dout->Play(buff_rend, szread);
			}
			if (!szread) break;
		}
		if (_kbhit() && _getch()=='q') break;
	}
	//if (near_end) fclose(near_end);
	//if (far_end) fclose(far_end);
	//if (echo) fclose(echo);
	if (m_outf) m_outf->Close();
	delete m_outf;
	free(buff_echo);
	free(buff_near);
	free(buff_far);
	free(buff_rend);
	if (din) delete din; din = 0;
	if (dout) delete dout; dout = 0;
	VS_AudioDeviceManager::Close();
	DestroyWindow(hwnd);
	CoUninitialize();
	delete g_pDtrase;
	return 0;
}

int main4(int argc, char* argv[])
{
	if (argc < 5) {
		printf("Usage: *.exe 0 frequency near_end.wav far_end.wav\n");
		return -1;
	}

	VS_EchoCancelBase *ec = 0;

	int type = atoi(argv[1]);

	ec = VS_RetriveEchoCancel(type);
	if (ec == 0) {
		printf("Error: can't create aec\n");
		return -2;
	}

	int i = 0, res = 0;
	float j = 0.0;

	int freq = atoi(argv[2]), size_block = 0;

	switch (freq)
	{
	case 8000:
	case 11025:
		size_block = 160;
		break;
	case 16000:
		size_block = 320;
		break;
	default:
		printf("Error: incorrect frequency\n");
		if (ec) delete ec; ec = 0;
		return -3;
	}

	CWaveFile *m_inf = new CWaveFile;
	CWaveFile *m_outf = new CWaveFile;
	CWaveFile *m_compf = new CWaveFile;

	WAVEFORMATEX wf;
	wf.nAvgBytesPerSec = freq * 2;
	wf.nBlockAlign = 2;
	wf.nChannels = 1;
	wf.nSamplesPerSec = freq;
	wf.wBitsPerSample = 16;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.cbSize = sizeof(WAVEFORMATEX);

	char str[256] = {0};
	sprintf(str, "e%s", argv[3]);
	m_inf->Open(argv[3], &wf, WAVEFILE_READ);
	m_outf->Open(argv[4], &wf, WAVEFILE_READ);
	m_compf->Open(str, &wf, WAVEFILE_WRITE);

	unsigned char buff_far[1280*4], buff_near[1280*4], buff_echo[1280*4];
	UINT szwrote = 0;
	UINT szwrite = size_block * 2;
	DWORD szread = 0;

	ec->Init(freq);
	ec->Init(freq, 1, 1);

	unsigned int allt = 0;

	while (1) {
		szread = 0;
		m_inf->Read((BYTE*)buff_near, szwrite, &szread);
		if (szread == 0) break;
		szread = 0;
		m_outf->Read((BYTE*)buff_far, szwrite, &szread);
		if (szread == 0) break;

		unsigned int ctime = timeGetTime();
		ec->Cancellate((short*)buff_far, (short*)buff_near, (short*)buff_echo, size_block);
		allt += timeGetTime() - ctime;

		szwrote = 0;
		m_compf->Write(szwrite, (BYTE*)buff_echo, &szwrote);
	}

	printf("time aec = %d", allt);

	if (m_inf) m_inf->Close(); delete m_inf;
	if (m_outf) m_outf->Close(); delete m_outf;
	if (m_compf) m_compf->Close(); delete m_compf;

	if (ec) delete ec; ec = 0;

	return 0;
}

int main0(int argc, char* argv[])
{
	HWND hwnd;
	HANDLE handles[2];
	CoInitialize(0);

	FILE *near_end, *far_end, *echo;
	//near_end = fopen("near_end.pcm", "wb");
	//far_end = fopen("far_end.pcm", "rb");
	//echo = fopen("echo.pcm", "wb");

	char temp[256] = {0};
	sprintf(temp, "Visicron\\test_echo\\");
	VS_RegistryKey::SetRoot(temp);
	g_pDtrase = new VS_DebugOut;

	if (!CreateMainWindow(hwnd))
		return -2;

	VS_AudioDeviceManager::Open(hwnd);
	VS_MediaFormat fmt;
	VS_ACaptureDevice *din = new VS_ACaptureDevice;
	VS_ARenderDevice *dout = new VS_ARenderDevice;
	VS_AudioCapture *acapt = new VS_AudioCapture(NULL, NULL, &fmt);

	int tmp = 2;
	_variant_t *pVar = new _variant_t(tmp);
	acapt->ProcessFunction(SET_PARAM, "UseXPAec", pVar);
	delete pVar;

	int DurrTime = 200;
	int len = 0, num = 0, size = 100 * 16000 / 1000 * 2, len_far = 0,
		d_far = DurrTime * 16 * 2, d_out = 0,
		last_sample = -65536,
		last_durration[20], num_dur = 0, avg_durr = 0;
	char *buff_echo = (char*)malloc((DurrTime * 16000 * 2) / 1000 * 25),
		 *buff_near = (char*)malloc((DurrTime * 16000 * 2) / 1000 * 25),
		 *buff_rend = (char*)malloc((DurrTime * 16000 * 2) / 1000 * 25),
		 *buff_far  = (char*)malloc((2000 * 16000 * 2) / 1000 * 2);

	fmt.SetAudio(11025, VS_ACODEC_PCM, 100);
	din->Init(-1, &fmt);
	dout->Init(-1, &fmt);
	handles[0] = din->GetCmpleteEvent();
	handles[1] = dout->GetCmpleteEvent();
	din->SetVolume(0x2000);
	//din->SetVolume(0xffff);
	dout->SetVolume(0xffff);
	din->Start();
	SetEvent(handles[1]);

	CWaveFile *m_outf = new CWaveFile;
	WAVEFORMATEX wf;
	wf.nAvgBytesPerSec = 11025*2;
	wf.nBlockAlign = 2;
	wf.nChannels = 1;
	wf.nSamplesPerSec = 11025;
	wf.wBitsPerSample = 16;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	m_outf->Open("outf.wav", &wf, WAVEFILE_READ);

	int StartTime = timeGetTime();

	while (1) {
		DWORD dwret = WaitForMultipleObjects(2, handles, FALSE, 100);
		StartTime = timeGetTime();
		if (dwret == WAIT_OBJECT_0 + 0) {
			while (din->Capture(buff_near, len, false)>0) {
				//fwrite(buff_near, len, 1, echo); // echo resudial
			}
		} else if (dwret == WAIT_OBJECT_0 + 1) {
			int res = 1;
			DWORD szread = 1;
			while (dout->GetCurrBuffDurr() < DurrTime) {
				//res = fread(buff_rend , size, 1, far_end);
				szread = 0;
				m_outf->Read((BYTE*)buff_rend, size, &szread);
				//if (!res) break;
				if (!szread) break;
				//dout->Play(buff_rend, size);
				dout->Play(buff_rend, szread);
			}
			if (!szread) break;
		}
		if (_kbhit() && _getch()=='q') break;
	}
	//if (near_end) fclose(near_end);
	//if (far_end) fclose(far_end);
	//if (echo) fclose(echo);
	if (m_outf) m_outf->Close();
	delete m_outf;
	free(buff_echo);
	free(buff_near);
	free(buff_far);
	free(buff_rend);
	if (din) delete din; din = 0;
	if (dout) delete dout; dout = 0;
	VS_AudioDeviceManager::Close();
	DestroyWindow(hwnd);
	CoUninitialize();
	delete g_pDtrase;
	return 0;
}

#define SIZE_BLOCK (160)

int main1(int argc, char* argv[])
{
	int i = 0, res = 0;
	float j = 0.0;

	LARGE_INTEGER st_t, end_t, all_t, fr;
	QueryPerformanceFrequency(&fr);
	all_t.QuadPart = 0;

	if (argc < 4) return -1;

	int freq = atoi(argv[1]);
	int size_block = (freq == 11025) ? 160 : 320;

	CWaveFile *m_inf = new CWaveFile;
	CWaveFile *m_outf = new CWaveFile;
	CWaveFile *m_compf = new CWaveFile;

	WAVEFORMATEX wf;
	wf.nAvgBytesPerSec = freq*2;
	wf.nBlockAlign = 2;
	wf.nChannels = 1;
	wf.nSamplesPerSec = freq;
	wf.wBitsPerSample = 16;
	wf.wFormatTag = WAVE_FORMAT_PCM;

	//for (j = -40.0; j <= 20.0; j += 1.0) {
		char str[256] = {0};
		sprintf(str, "e%s", argv[2]);
		m_inf->Open(argv[2], &wf, WAVEFILE_READ);
		m_outf->Open(argv[3], &wf, WAVEFILE_READ);
		m_compf->Open(str, &wf, WAVEFILE_WRITE);

		unsigned char buff_far[1280*4], buff_near[1280*4], buff_echo[1280*4];

		VS_SpeexEchoCancel	m_ec;
		m_ec.Init(freq);
		m_ec.Init(freq, 1, 1);

		UINT szwrote = 0;
		UINT szwrite = size_block * 2;
		DWORD szread = 0;
		int NN = 0;
		int sh = ((int)(((40 + j) * freq) / 1000.0 + 0.5)) * 2;

		szread = 0;
		//m_inf->Read((BYTE*)buff_near, sh, &szread);
		//if (!(szread == 0 && sh != 0)) {
			//szread = 0;
			//m_outf->Read((BYTE*)buff_far, (int)((40.0 * freq) / 1000.0 + 0.5) * 2, &szread);
			//if (szread) {
				while (1) {
					szread = 0;
					m_inf->Read((BYTE*)buff_near, szwrite, &szread);
					if (szread == 0) break;
					szread = 0;
					m_outf->Read((BYTE*)buff_far, szwrite, &szread);
					if (szread == 0) break;
					QueryPerformanceCounter(&st_t);
					m_ec.Cancellate((short*)buff_far, (short*)buff_near, (short*)buff_echo, size_block);
					QueryPerformanceCounter(&end_t);
					all_t.QuadPart += end_t.QuadPart - st_t.QuadPart;
					szwrote = 0;
					m_compf->Write(szwrite, (BYTE*)buff_echo, &szwrote);
					NN++;
				}
			//}
		//}

		if (m_inf) m_inf->Close();
		if (m_outf) m_outf->Close();
		if (m_compf) m_compf->Close();
	//}

	delete m_inf;
	delete m_outf;
	delete m_compf;

	printf("%8.2f ms\n", (all_t.QuadPart * 1000.0) / fr.QuadPart);

	return 0;
}

int main5(int argc, char **argv)
{
	int mode = atoi(argv[5]), freq = atoi(argv[1]);
	float delt = atof(argv[4]);

	WAVEFORMATEX wf;
	wf.nAvgBytesPerSec = freq*2;
	wf.nBlockAlign = 2;
	wf.nChannels = 1;
	wf.nSamplesPerSec = freq;
	wf.wBitsPerSample = 16;
	wf.wFormatTag = WAVE_FORMAT_PCM;

	CWaveFile *m_inf = new CWaveFile;
	CWaveFile *m_outf = new CWaveFile;

	m_inf->Open(argv[2], &wf, WAVEFILE_READ);
	m_outf->Open(argv[3], &wf, WAVEFILE_WRITE);

	unsigned char buff_far[1280*4], buff_near[1280*4], buff_echo[1280*4],buff_tmp[1280*4];

	UINT szwrote = 0;
	UINT szwrite = SIZE_BLOCK * 2;
	DWORD szread = 0;
	int NN = 0;
	int n=0;
	int ret = 0;
	szread = 0;

	//SpeexResamplerState * pState=speex_resampler_init((__int64)1,(__int64)freq,(__int64)freq+delt, mode,&ret);
	double outfreq = freq + delt;
	int rn = freq * 100,
		rd = (int)(outfreq * 100);
	SpeexResamplerState * pState = speex_resampler_init_frac(1, rn, rd, freq, (int)outfreq, mode,&ret);
	ret=speex_resampler_skip_zeros(pState);

	while (1) {
		m_inf->Read((BYTE*)buff_far, szwrite, &szread);
		if (szread == 0) break;
		szread/=2;
		szwrote=szread*2;
		//
		int err=speex_resampler_process_int(pState,
											0,
											(const spx_int16_t *)buff_far,
											(spx_uint32_t *)&szread,
											(spx_int16_t *)buff_tmp,
											(spx_uint32_t *)&szwrote);
		m_outf->Write(szwrote*2, (BYTE*)buff_tmp, &szwrote);
	}

	szread = 0;
	m_inf->Close();
	m_outf->Close();

	return 0;
}

#define NUM_ECHO_CHANNEL (3)

int main6(int argc, char* argv[])
{
	int st_t, all_t = 0;
	HWND hwnd;
	HANDLE handles[NUM_ECHO_CHANNEL+1];
	CoInitialize(0);

	FILE *near_end, *far_end, *echo;

	if (argc < 4) {
		printf("Usage: *.exe 1 frequency far_end\n");
		return -1;
	}

	if (VS_RetriveEchoCancel(0) == 0) {
		printf("Error: can't create aec\n");
		return -1;
	}

	int freq = atoi(argv[2]), i = 0, dev = atoi(argv[3]);
	int beg_play[NUM_ECHO_CHANNEL] = {0, 20000, 35000};

	char temp[256] = {0};
	sprintf(temp, "Visicron\\test_echo\\");
	VS_RegistryKey::SetRoot(temp);
	g_pDtrase = new VS_DebugOut;

	if (!CreateMainWindow(hwnd))
		return -2;

	VS_AudioDeviceManager::Open(hwnd);
	VS_MediaFormat fmt;
	VS_ACaptureDevice *din = new VS_ACaptureDevice;
	VS_ARenderDevice *dout[NUM_ECHO_CHANNEL];
	for (i = 0; i < NUM_ECHO_CHANNEL; i++) {
		dout[i] = new VS_ARenderDevice;
	}
	VS_AudioCapture *acapt = new VS_AudioCapture(NULL, NULL, &fmt);

	int tmp = 2;
	_variant_t *pVar = new _variant_t(tmp);
	acapt->ProcessFunction(SET_PARAM, "UseXPAec", pVar);
	delete pVar;

	int DurrTime = 300;
	int len = 0, num = 0, size = 100 * freq / 1000 * 2, len_far = 0,
		d_far = DurrTime * 16 * 2, d_out = 0,
		last_sample = -65536,
		last_durration[20], num_dur = 0, avg_durr = 0;
	char *buff_echo = (char*)malloc((DurrTime * 16000 * 2) / 1000 * 25),
		 *buff_near = (char*)malloc((DurrTime * 16000 * 2) / 1000 * 25),
		 *buff_rend = (char*)malloc((DurrTime * 16000 * 2) / 1000 * 25),
		 *buff_far[NUM_ECHO_CHANNEL];
	for (i = 0; i < NUM_ECHO_CHANNEL; i++) {
		buff_far[i] = (char*)malloc((2000 * 16000 * 2) / 1000 * 2);
	}

	WAVEFORMATEX wf;
	wf.nAvgBytesPerSec = freq*2;
	wf.nBlockAlign = 2;
	wf.nChannels = 1;
	wf.nSamplesPerSec = freq;
	wf.wBitsPerSample = 16;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	CWaveFile *m_outf[NUM_ECHO_CHANNEL];
	char fn[128];
	for (i = 0; i < NUM_ECHO_CHANNEL; i++) {
		m_outf[i] = new CWaveFile;
		sprintf(fn, "%s_%d.wav", argv[4], i);
		m_outf[i]->Open(fn, &wf, WAVEFILE_READ);
	}

	fmt.SetAudio(freq, WAVE_FORMAT_PCM, 100);
	din->Init(dev, &fmt);
	handles[0] = din->GetCmpleteEvent();
	for (i = 1; i < NUM_ECHO_CHANNEL; i++) {
		dout[i]->Init(-1, &fmt);
		handles[1+i] = dout[i]->GetCmpleteEvent();
		dout[i]->SetVolume(0xffff);
		dout[i]->Release();
	}
	dout[0]->Init(-1, &fmt);
	handles[1] = dout[0]->GetCmpleteEvent();
	dout[0]->SetVolume(0xffff);
	din->SetVolume(0x4000);
	//din->SetVolume(0xffff);
	din->Start();
	//for (i = 0; i < NUM_ECHO_CHANNEL; i++) {
	//	SetEvent(handles[1+i]);
	//}

	int StartTime = timeGetTime();
	int StartTimeC = timeGetTime();
	int num_rnd = NUM_ECHO_CHANNEL;

	while (1) {
		DWORD dwret = WaitForMultipleObjects(NUM_ECHO_CHANNEL + 1, handles, FALSE, 50);
		StartTime = timeGetTime();
		if (dwret == WAIT_OBJECT_0 + 0) {
			while (din->Capture(buff_near, len, false)>0) {
				//fwrite(buff_near, len, 1, echo); // echo resudial
			}
		}
		{
			int res = 1;
			DWORD szread = 1;
			for (i = 0; i < NUM_ECHO_CHANNEL; i++) {
				if (!dout[i]->IsValid() && (timeGetTime() - StartTimeC) > beg_play[i]) {
					dout[i]->Init(-1, &fmt);
					dout[i]->SetVolume(0xffff);
				}
				if (dout[i]->IsValid()) {
					if (dout[i]->GetCurrBuffDurr() < DurrTime) {
						//res = fread(buff_rend , size, 1, far_end);
						szread = 0;
						m_outf[i]->Read((BYTE*)buff_rend, size, &szread);
						//if (!res) break;
						if (szread != size) {
							num_rnd--;
							beg_play[i] = 0x7fffffff;
							//delete dout[i]; dout[i] = 0;
							dout[i]->Release();
							continue;
						}
						//dout->Play(buff_rend, size);
						st_t = timeGetTime();
						dout[i]->Play(buff_rend, szread);
						all_t += timeGetTime() - st_t;
					}
				}
			}
			if (num_rnd <= 0) break;
		}
		if (_kbhit() && _getch()=='q') break;
	}

	for (i = 0; i < NUM_ECHO_CHANNEL; i++) {
		if (m_outf[i]) m_outf[i]->Close();
		delete m_outf[i];
	}
	free(buff_echo);
	free(buff_near);
	for (i = 0; i < NUM_ECHO_CHANNEL; i++) {
		free(buff_far[i]);
	}
	free(buff_rend);
	if (din) delete din; din = 0;
	for (i = 0; i < NUM_ECHO_CHANNEL; i++) {
		if (dout[i]) delete dout[i]; dout[i] = 0;
	}
	VS_AudioDeviceManager::Close();
	DestroyWindow(hwnd);
	CoUninitialize();
	delete g_pDtrase;

	printf("t_aec = %d ms\n", all_t);

	return 0;
}

int main7(int argc, char **argv)
{
	int freq = atoi(argv[2]), i = 0;

	WAVEFORMATEX wf;
	wf.nAvgBytesPerSec = freq*2;
	wf.nBlockAlign = 2;
	wf.nChannels = 1;
	wf.nSamplesPerSec = freq;
	wf.wBitsPerSample = 16;
	wf.wFormatTag = WAVE_FORMAT_PCM;

	//CWaveFile *m_inf = new CWaveFile;
	CWaveFile *m_echo = new CWaveFile;
	CWaveFile *m_outf[NUM_ECHO_CHANNEL];
	CWaveFile *m_inf[NUM_ECHO_CHANNEL];
	char fn[128];
	for (i = 0; i < NUM_ECHO_CHANNEL; i++) {
		m_inf[i] = new CWaveFile;
		sprintf(fn, "%s.wav", argv[3], i);
		m_inf[i]->Open(fn, &wf, WAVEFILE_READ);
		m_outf[i] = new CWaveFile;
		//sprintf(fn, "%s_%d.wav", argv[4], i);
		sprintf(fn, "%s_%d.wav", argv[4], i);
		m_outf[i]->Open(fn, &wf, WAVEFILE_READ);
	}
	//m_inf->Open(argv[3], &wf, WAVEFILE_READ);
	m_echo->Open(argv[5], &wf, WAVEFILE_WRITE);

	unsigned char buff_far[1280*6], buff_near[1280*6], buff_echo[1280*6],buff_tmp[1280*6];

	int ifreq = (freq/8000)*8000;
	int frame_size = ifreq/50;

	UINT szwrote = 0;
	UINT szwrite = 3 * frame_size * 2;
	DWORD szread = 0;
	int NN = 0;
	int n=0;
	int ret = 0;
	szread = 0;

	VS_EchoCancelBase *ec = 0;
	ec = VS_RetriveEchoCancel(0);
	ec->Init(freq, 1, NUM_ECHO_CHANNEL);

	while (1) {
		m_inf[0]->Read((BYTE*)buff_near, szwrite, &szread);
		//for (i = 1; i < NUM_ECHO_CHANNEL; i++) {
		//	memcpy(buff_near + i * szwrite, buff_near, szwrite);
		//}
		if (szread != szwrite) break;
		for (i = 0; i < NUM_ECHO_CHANNEL; i++) {
			//m_inf[i]->Read((BYTE*)buff_near + i * szwrite, szwrite, &szread);
			//if (szread != szwrite) break;
			m_outf[i]->Read((BYTE*)buff_far + i * szwrite, szwrite, &szread);
			if (szread != szwrite) break;
		}
		short *pbt = (short*)buff_tmp, *pb = (short*)buff_far;
		for (i = 0; i < frame_size; i++) {
			pbt[i*3+0] = pb[i+0*frame_size];
			pbt[i*3+1] = pb[i+1*frame_size];
			pbt[i*3+2] = pb[i+2*frame_size];
		}
		if (szread != szwrite) break;
		//ec->Cancellate((short*)buff_far, (short*)buff_near, (short*)buff_echo, szwrite / 2);
		ec->Cancellate((short*)buff_tmp, (short*)buff_near, (short*)buff_echo, szwrite / 2);
		m_echo->Write(szwrite, (BYTE*)buff_echo, &szwrote);
	}

	szread = 0;
	//m_inf->Close(); delete m_inf;
	m_echo->Close(); delete m_echo;
	for (i = 0; i < NUM_ECHO_CHANNEL; i++) {
		if (m_inf[i]) m_inf[i]->Close();
		delete m_inf[i];
		if (m_outf[i]) m_outf[i]->Close();
		delete m_outf[i];
	}
	delete ec;

	return 0;
}

int main8(int argc, char **argv)
{
	HWND hwnd;
	HANDLE handles[1];
	CoInitialize(0);

	int freq = atoi(argv[1]);
	float lvl = atoi(argv[2]);

	WAVEFORMATEX wf;
	wf.nAvgBytesPerSec = freq*2;
	wf.nBlockAlign = 2;
	wf.nChannels = 1;
	wf.nSamplesPerSec = freq;
	wf.wBitsPerSample = 16;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	CWaveFile *m_outf = new CWaveFile;
	CWaveFile *m_outf_n = new CWaveFile;
	m_outf->Open(argv[3], &wf, WAVEFILE_WRITE);
	m_outf_n->Open(argv[4], &wf, WAVEFILE_WRITE);

	if (!CreateMainWindow(hwnd))
		return -2;

	VS_AudioDeviceManager::Open(hwnd);
	VS_MediaFormat fmt;
	VS_ACaptureDevice *din = new VS_ACaptureDevice;

	int DurrTime = 200;
	int len = 0, i = 0;
	unsigned int swrote = 0;
	char *buff_near = (char*)malloc((DurrTime * 16000 * 2) / 1000 * 25);
	int fs = (freq/8000*8000)/50*sizeof(short);

	SpeexPreprocessState *st = speex_preprocess_state_init(fs / sizeof(short), freq);
	i = 1;
	speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_AGC, &i);
	speex_preprocess_ctl(st, SPEEX_PREPROCESS_SET_AGC_LEVEL, &lvl);

	fmt.SetAudio(freq, VS_ACODEC_PCM, 100);
	din->Init(-1, &fmt);
	handles[0] = din->GetCmpleteEvent();
	din->SetVolume(0x4000);
	din->Start();

	while (1) {
		DWORD dwret = WaitForMultipleObjects(1, handles, FALSE, 100);
		if (dwret == WAIT_OBJECT_0 + 0) {
			while (din->Capture(buff_near, len, false)>0) {
				for (i = 0; i < len; i += fs) {
					m_outf_n->Write(fs, (unsigned char*)(buff_near + i), &swrote);
					speex_preprocess_run(st, (short*)(buff_near + i));
					m_outf->Write(fs, (unsigned char*)(buff_near + i), &swrote);
				}
			}
		}
		if (_kbhit() && _getch()=='q') break;
	}

	speex_preprocess_state_destroy(st);
	if (m_outf) m_outf->Close();
	delete m_outf;
	if (m_outf_n) m_outf_n->Close();
	delete m_outf_n;
	if (din) delete din; din = 0;
	VS_AudioDeviceManager::Close();
	DestroyWindow(hwnd);
	CoUninitialize();

	return 0;
}

int main9(int argc, char* argv[])
{
	int i = 0, res = 0;
	float j = 0.0;

	LARGE_INTEGER st_t, end_t, all_t, fr;
	QueryPerformanceFrequency(&fr);
	all_t.QuadPart = 0;

	if (argc < 4) return -1;

	int freq = atoi(argv[1]);
	int size_block = 160;//(freq == 11025) ? 160 : 320;

	CWaveFile *m_inf = new CWaveFile;
	CWaveFile *m_outf = new CWaveFile;
	CWaveFile *m_compf = new CWaveFile;

	WAVEFORMATEX wf;
	wf.nAvgBytesPerSec = freq*2;
	wf.nBlockAlign = 2;
	wf.nChannels = 1;
	wf.nSamplesPerSec = freq;
	wf.wBitsPerSample = 16;
	wf.wFormatTag = WAVE_FORMAT_PCM;

	//for (j = -40.0; j <= 20.0; j += 1.0) {
		char str[256] = {0};
		sprintf(str, "e%s", argv[2]);
		m_inf->Open(argv[2], &wf, WAVEFILE_READ);
		m_outf->Open(argv[3], &wf, WAVEFILE_READ);
		m_compf->Open(str, &wf, WAVEFILE_WRITE);

		unsigned char buff_far[960*2], buff_near[960*2], buff_echo[960*2];

		VS_SpeexEchoCancel	m_ec;
		m_ec.Init(freq);
		m_ec.Init(freq, 1, 1);

		UINT szwrote = 0;
		UINT szwrite = size_block * 2;
		DWORD szread = 0;
		int NN = 0;
		int sh = (int)((14 * freq) / 1000.0 + 0.5) * 2;//((int)(((40 + j) * freq) / 1000.0 + 0.5)) * 2;
		//m_inf->Read((BYTE*)buff_near, sh, &szread);

		szread = 0;
		//m_inf->Read((BYTE*)buff_near, sh, &szread);
		//if (!(szread == 0 && sh != 0)) {
			//szread = 0;
			//m_outf->Read((BYTE*)buff_far, (int)((40.0 * freq) / 1000.0 + 0.5) * 2, &szread);
			//if (szread) {
				while (1) {
					szread = 0;
					m_inf->Read((BYTE*)buff_near, szwrite, &szread);
					//if (szread == 0) break;
					if (szread != szwrite) break;
					szread = 0;
					m_outf->Read((BYTE*)buff_far, szwrite, &szread);
					if (szread == 0) break;
					QueryPerformanceCounter(&st_t);
					m_ec.Cancellate((short*)buff_far, (short*)buff_near, (short*)buff_echo, size_block);
					QueryPerformanceCounter(&end_t);
					all_t.QuadPart += end_t.QuadPart - st_t.QuadPart;
					szwrote = 0;
					m_compf->Write(szwrite, (BYTE*)buff_echo, &szwrote);
					NN++;
				}
			//}
		//}

		if (m_inf) m_inf->Close();
		if (m_outf) m_outf->Close();
		if (m_compf) m_compf->Close();
	//}

	delete m_inf;
	delete m_outf;
	delete m_compf;

	printf("%8.2f ms\n", (all_t.QuadPart * 1000.0) / fr.QuadPart);

	return 0;
}