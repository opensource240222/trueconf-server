/**
 **************************************************************************
 * \file VS_AviWrite.cpp
 * \brief VS_AviWrite Implementation
 *
 * \b Project Client
 * \author SMirnovK
 * \date 30.06.2004
 *
*
****************************************************************************/


/****************************************************************************
* Includes
****************************************************************************/
#include "VS_AviWrite.h"
#include "VS_Dmodule.h"
#include "VSCompress.h"
#include "../VSClient/VS_ApplicationInfo.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "../Transcoder/VS_IppAudiCodec.h"
#include "Transcoder/VideoCodec.h"
//#include "../Transcoder/VS_MultiMixerWriter.h"
#include "../std/cpplib/VS_VideoLevelCaps.h"
#include "std-generic/cpplib/utf8.h"
#include "VSThinkClient.h"
#include <math.h>
#include "Transcoder/VS_RetriveVideoCodec.h"

#ifdef _BUILD_CONFERENDO
#include "../_Visicron/resource.h"
#endif


/****************************************************************************
* Defines
****************************************************************************/
#define SIELENCE_VIDEO_TIME	1000
#define SIELENCE_AUDIO_TIME	960
#define VIDE_AUDIO_SHIFT	120
#define AUDIO_CHUNK_TIME	240

#define TEMP_SIZE			64000

/****************************************************************************
* Globals
****************************************************************************/
VS_AviWriteGroup *g_AviWriterGroup = 0;

/****************************************************************************
* Classes
****************************************************************************/

//////////

#define DEFAULT_SAMPLERATE	(16000)

#define DEFAULT_VCODEC		(VS_VCODEC_VPX)
#define DEFAULT_ACODEC		(VS_ACODEC_GSM610)

#define DEFAULT_AVI_FPS		(15)

//							  360  480  640  720  720  864  1280  1920  x  x
static int bitrate_tbl[10] = {300, 400, 512, 640, 700, 800, 1200, 1800};


VS_AviWriteGroup::VS_AviWriteGroup(CVSInterface *pParentInterface): CVSInterface("AviWriter", pParentInterface)
{
	m_wstream_state.clear();
	m_is_valid = false;
	m_is_started = false;
	m_is_pcm = false;
	//m_mixer = 0;
	m_acodec = 0;
	m_vcodec = 0;
	m_imgLogo = 0;
	m_num_streams = 0;
	m_vbitrate = 0;
	m_vframerate = DEFAULT_AVI_FPS;
	m_logoW = 0;
	m_logoH = 0;
	m_mixer_mode = 2;
	TryOpenLogo();
}

VS_AviWriteGroup::~VS_AviWriteGroup()
{
	Close();
	if (m_num_streams != 0) {
		for (write_iter i = m_wstream_state.begin(), e = m_wstream_state.end(); i != e; ) {
			ReleaseStream((char*)((i++)->first).data());
		}
	}
	m_wstream_state.clear();
	m_num_streams = 0;
	delete [] m_imgLogo;
}

void VS_AviWriteGroup::TryOpenLogo()
{

#ifdef _BUILD_CONFERENDO

	HMODULE hModule = GetModuleHandle("conferendo.dll");
	if (hModule) {
		HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(IDB_LOGO), RT_BITMAP);
		if (hResource) {
			HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
			if (hLoadedResource) {
				LPVOID pLockedResource = LockResource(hLoadedResource);
				if (pLockedResource) {
					DWORD dwResourceSize = SizeofResource(hModule, hResource);
					BITMAPINFOHEADER bmi;
					memcpy(&bmi, pLockedResource, sizeof(bmi));
					if (bmi.biBitCount == 32) {
						m_imgLogo = new unsigned char [bmi.biSizeImage];
						memcpy(m_imgLogo, (unsigned char*)pLockedResource + sizeof(bmi), bmi.biSizeImage);
						m_logoW = bmi.biWidth;
						m_logoH = bmi.biHeight;
					}
				}
			}
		}
	}

#endif

}

bool VS_AviWriteGroup::Init(wchar_t *file_name)
{
	if (IsThreadActiv()) return false;

	int ret = -1;
	WAVEFORMATEX *wf = 0;
	if (!m_is_valid) {
		do {
			Release();

			auto utf8_file_name = vs::UTF16toUTF8Convert(file_name);

			//utf8_file_name.replace(utf8_file_name.size() - 3, 3, "mkv");

			if (!m_avi.Init(utf8_file_name)) return false;

			int mixer_mode = m_mixer_mode.load();
			VS_RegistryKey key(true, REG_CurrentConfiguratuon);
			key.GetValue(&mixer_mode, 4, VS_REG_INTEGER_VT, "Mixer Mode");

			if (mixer_mode < 0) mixer_mode = 0;
			if (mixer_mode > 7) mixer_mode = 7;

#ifndef _BUILD_CONFERENDO
			//m_mixer = new VS_MultiMixerWriter(true, mixer_mode);
#else
			//m_mixer = new VS_MultiMixerWriter(true, mixer_mode, SHIFT_MUXERMODE_CONFERENDO);
#endif

			//if (!m_mixer) break;

			m_vbitrate = bitrate_tbl[mixer_mode];
			//int w = m_mixer->GetMuxerOutWidth();
			//int h = m_mixer->GetMuxerOutHeight();
			//m_mf.SetVideo(w, h, DEFAULT_VCODEC);
			//.SetAudio(DEFAULT_SAMPLERATE, DEFAULT_ACODEC);

			BITMAPINFOHEADER bm;
			memset(&bm, 0, sizeof (BITMAPINFOHEADER));
			bm.biSize = sizeof (BITMAPINFOHEADER);
			bm.biCompression = (DEFAULT_VCODEC == VS_VCODEC_VPX) ? '08PV' : DEFAULT_VCODEC;
			bm.biWidth = m_mf.dwVideoWidht;
			bm.biHeight = m_mf.dwVideoHeight;
			bm.biPlanes = 1;

			VSVideoFile::SAudioInfo audioInfo;
			VSVideoFile::SVideoInfo videoInfo;

			videoInfo.Width = m_mf.dwVideoWidht;
			videoInfo.Height = m_mf.dwVideoHeight;
			videoInfo.FPS = 30;
			videoInfo.PixFormat = VSVideoFile::PF_YUV420;
			videoInfo.CodecID = VSVideoFile::VCODEC_ID_VP8;

			if (!m_avi.SetVideoFormat(videoInfo)) break;

			m_vcodec = VS_RetriveVideoCodec(VS_VCODEC_VPX, true);
			if (!m_vcodec) break;
			if (m_vcodec->Init(m_mf.dwVideoWidht, m_mf.dwVideoHeight, FOURCC_I420) == -1) break;
			m_vcodec->SetBitrate(m_vbitrate * 10 / DEFAULT_AVI_FPS);

			int audio_codec = 0;
			key.GetValue(&audio_codec, 4, VS_REG_INTEGER_VT, "Audio Codec Writer");

			if (audio_codec == 0) {
				// PCM
				wf = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
				wf->nSamplesPerSec = DEFAULT_SAMPLERATE;
				wf->cbSize = 0;
				wf->wBitsPerSample = 16;
				wf->nChannels = 1;
				wf->nBlockAlign = wf->wBitsPerSample * wf->nChannels / 8;
				wf->nAvgBytesPerSec = wf->nSamplesPerSec * wf->nBlockAlign;
				wf->wFormatTag = VS_ACODEC_PCM;

				audioInfo.CodecID = VSVideoFile::ACODEC_ID_PCM;
			}
			else if (audio_codec == 1) {
				// OPUS
				wf = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
				wf->nSamplesPerSec = DEFAULT_SAMPLERATE;
				wf->cbSize = 0;
				wf->wBitsPerSample = 16;
				wf->nChannels = 1;
				wf->nBlockAlign = 0;
				wf->nAvgBytesPerSec = (DEFAULT_SAMPLERATE * wf->nBlockAlign) / 320;
				wf->wFormatTag = VS_ACODEC_OPUS_B0914;

				audioInfo.CodecID = VSVideoFile::ACODEC_ID_OPUS;
			}
			else if (audio_codec == 2) {
				// MP3
				wf = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
				wf->nSamplesPerSec = DEFAULT_SAMPLERATE;
				wf->cbSize = 0;
				wf->wBitsPerSample = 16;
				wf->nChannels = 1;
				wf->nBlockAlign = 0;
				wf->nAvgBytesPerSec = (DEFAULT_SAMPLERATE * wf->nBlockAlign) / 320;
				wf->wFormatTag = VS_ACODEC_MP3;

				audioInfo.CodecID = VSVideoFile::ACODEC_ID_MP3;
			}

			m_acodec = VS_RetriveAudioCodec(wf->wFormatTag, true);

			if (!m_acodec || (m_acodec && (m_acodec->Init(wf) != 0))) {
				wf->nSamplesPerSec = DEFAULT_SAMPLERATE;
				wf->cbSize = 0;
				wf->wBitsPerSample = 16;
				wf->nChannels = 1;
				wf->nBlockAlign = wf->wBitsPerSample * wf->nChannels / 8;
				wf->nAvgBytesPerSec = wf->nSamplesPerSec * wf->nBlockAlign;
				wf->wFormatTag = WAVE_FORMAT_PCM;
				m_is_pcm = true;

				audioInfo.CodecID = VSVideoFile::ACODEC_ID_PCM;
			}

			audioInfo.BitsPerSample = wf->wBitsPerSample;
			audioInfo.NumChannels = wf->nChannels;
			audioInfo.SampleRate = wf->nSamplesPerSec;

			if (!m_avi.SetAudioFormat(audioInfo))
				break;

			if (!m_avi.WriteHeader())
				break;

			ret = 0;
		} while(false);
	}
	if (wf) free(wf); wf = 0;
	if (ret == -1) {
		Release();
		return false;
	}
	if (!IsThreadActiv()) {
		if (!ActivateThread(this)) {
			Release();
			return false;
		}
		m_is_valid = true;
	}

	DTRACE(VSTM_AVIW, "Init AviWriter %S ", file_name);

	return true;
}

void VS_AviWriteGroup::Release()
{
	DTRACE(VSTM_AVIW, "Release AviWriter");

	if (m_vcodec) delete m_vcodec; m_vcodec = 0;
	if (m_acodec) delete m_acodec; m_acodec = 0;
	//if (m_mixer) delete m_mixer; m_mixer = 0;
	m_avi.Release();
	memset(&m_mf, 0, sizeof(VS_MediaFormat));
	m_is_valid = false;
	m_is_started = false;
	m_is_pcm = false;
}

void VS_AviWriteGroup::CreateStream(char *handle, VS_MediaFormat *mf)
{
	VS_AutoLock lock(this);

	DTRACE(VSTM_AVIW, "Try Create Stream %s", handle);

	if (mf == NULL) return;
	write_iter iter = m_wstream_state.find(handle);
	if (iter != m_wstream_state.end()) return;

	DTRACE(VSTM_AVIW, "Create Stream %s", handle);

	VS_StreamStateW st;
	memcpy(&(st.mf), mf, sizeof(VS_MediaFormat));
	st.is_init = false;
	m_wstream_state.emplace(handle, st);
	m_num_streams++;
}

/*
/// for tests
void VS_AviWriteGroup::CreateStream(char *handle, VS_MediaFormat *mf)
{
	int i = 0;

	VS_AutoLock lock(this);

	DTRACE(VSTM_AVIW, "Try Create Stream %s", handle);

	if (mf == NULL) return;
	write_iter iter = m_wstream_state.find(string(handle));
	if (iter != m_wstream_state.end()) return;

	DTRACE(VSTM_AVIW, "Create Stream %s", handle);

	int nstream = 1;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	key.GetValue(&nstream, 4, VS_REG_INTEGER_VT, "nstream");

	for (i = 0; i < nstream; i++) {
		VS_StreamStateW st;
		memcpy(&(st.mf), mf, sizeof(VS_MediaFormat));
		st.is_init = false;
		if (i == 0) {
			m_wstream_state.emplace(string(handle), st);
		} else {
			char buff[128];
			sprintf(buff, "%d тестовый поток", i);
			m_wstream_state.emplace(string(buff), st);
		}
		m_num_streams++;
	}
}
*/

void VS_AviWriteGroup::ReleaseStream(char *handle)
{
	VS_AutoLock lock(this);

	DTRACE(VSTM_AVIW, "Try Release Stream %s", handle);

	write_iter iter = m_wstream_state.find(handle);
	if (iter == m_wstream_state.end()) return;

	DTRACE(VSTM_AVIW, "Release Stream %s", handle);

	//if (m_mixer) m_mixer->SetDeleteRay(handle);
	m_wstream_state.erase(iter);
	m_num_streams--;
}

int VS_AviWriteGroup::OpenStream(char *handle, char* stream_name)
{
	VS_AutoLock lock(this);

	DTRACE(VSTM_AVIW, "Try Open Stream %s %s", handle, stream_name);

	if (!m_is_valid) return -1;
	write_iter iter = m_wstream_state.find(handle);
	if (iter == m_wstream_state.end()) return 1;
	VS_StreamStateW *st = &(iter->second);
	if (!st->is_init) {
		DTRACE(VSTM_AVIW, "Open Stream %s %s", handle, stream_name);
		//m_mixer->AddRay(handle, stream_name, &(st->mf));
		//if (m_LastPStream == handle)
		//	m_mixer->SetPriority(handle);
	}
	st->is_init = true;

	return 0;
}

int VS_AviWriteGroup::ChangeMediaFormatStream(char *handle, VS_MediaFormat *mf)
{
	VS_AutoLock lock(this);

	DTRACE(VSTM_AVIW, "Try Change Media Format Stream %s", handle);

	write_iter iter = m_wstream_state.find(handle);
	if (iter == m_wstream_state.end()) return 1;
	VS_StreamStateW *st = &(iter->second);
	DTRACE(VSTM_AVIW, "Change Media Format Stream %s", handle);
	memcpy(&(st->mf), mf, sizeof(VS_MediaFormat));
	if (st->is_init) {
		//m_mixer->ChangeFormatStream(handle, *mf);
		DTRACE(VSTM_AVIW, "Change Media Format inner Stream %s", handle);
	}

	return 0;
}

/*
/// test
int VS_AviWriteGroup::OpenStream(char *handle, wchar_t stream_name[LENGHT_NAME_STREAM])
{
	VS_AutoLock lock(this);

	DTRACE(VSTM_AVIW, "Try Open Stream %s %s", handle, stream_name);

	if (!m_is_valid) return -1;

	write_iter iter = m_wstream_state.begin();
	while (true) {
		if (iter == m_wstream_state.end()) return 1;
		VS_StreamStateW *st = &(iter->second);
		if (!st->is_init) {
			DTRACE(VSTM_AVIW, "Open Stream %s %s", (char*)iter->first.c_str(), stream_name);

			m_mixer->AddRay((char*)iter->first.c_str(), stream_name, &(st->mf));
		}
		st->is_init = true;
		iter++;
	}

	return 0;
}
*/

int VS_AviWriteGroup::ChangeStream(char *handle, char* stream_name)
{
	VS_AutoLock lock(this);

	DTRACE(VSTM_AVIW, "Try Rename Stream %s %S", handle, stream_name);

	if (!m_is_valid) return -1;
	write_iter iter = m_wstream_state.find(handle);
	if (iter == m_wstream_state.end()) return 1;
	VS_StreamStateW *st = &(iter->second);
	if (st->is_init) {
		DTRACE(VSTM_AVIW, "Rename Stream %s %S", handle, stream_name);

		//bool res = m_mixer->RenameRay(handle, stream_name);
		//return (res) ? 0 : 1;
	}

	return 2;
}

void VS_AviWriteGroup::CloseStream(char *handle)
{
	VS_AutoLock lock(this);

	DTRACE(VSTM_AVIW, "Try Close Stream %s", handle);

	write_iter iter = m_wstream_state.find(handle);
	if (iter == m_wstream_state.end()) return;
	if (iter->second.is_init) {
		DTRACE(VSTM_AVIW, "Close Stream %s", handle);

		//if (m_mixer) m_mixer->SetDeleteRay(handle);
		VS_StreamStateW *st = &(iter->second);
		st->is_init = false;
	}
}

int VS_AviWriteGroup::GetRecordMB(char *handle)
{
	VS_AutoLock lock(this);

	DTRACE(VSTM_AVIW, "Try Get Record MBs Stream %s", handle);

	write_iter iter = m_wstream_state.find(handle);
	return (iter == m_wstream_state.end() || !iter->second.is_init) ? 0 : 0;// m_mixer->GetMuxerRayFrameMB(handle);
}

/*
/// for tests
void VS_AviWriteGroup::CloseStream(char *handle)
{
	VS_AutoLock lock(this);

	DTRACE(VSTM_AVIW, "Try Close Stream %s", handle);

	write_iter iter = m_wstream_state.begin();
	while (true) {
		if (iter == m_wstream_state.end()) return;
		if (iter->second.is_init) {
			DTRACE(VSTM_AVIW, "Close Stream %s", (char*)iter->first.c_str());

			if (m_mixer) m_mixer->SetDeleteRay((char*)iter->first.c_str());
			VS_StreamStateW *st = &(iter->second);
			st->is_init = false;
		}
		iter++;
	}
}
*/

void VS_AviWriteGroup::Close()
{
	Lock();

	DTRACE(VSTM_AVIW, "Close All Streams");

	if (m_num_streams != 0) {
		for (write_iter i = m_wstream_state.begin(), e = m_wstream_state.end(); i != e; ) {
			char *handle = (char*)(i->first).data();

			DTRACE(VSTM_AVIW, "Try Close Stream %s", handle);

			if (i->second.is_init) {

				DTRACE(VSTM_AVIW, "Close Stream %s", handle);

				//if (m_mixer) m_mixer->SetDeleteRay(handle);
				VS_StreamStateW *st = &(i->second);
				st->is_init = false;
			}
			i++;
		}
	}

	UnLock();

	DesactivateThread();
}

void VS_AviWriteGroup::PutAudio(char *callId, unsigned char *data, int size, double frequency, int cur_abuff_durr)
{
	VS_AutoLock lock(this);

	if (m_is_valid) {
		write_iter iter = m_wstream_state.find(callId);
		if (iter == m_wstream_state.end() || !iter->second.is_init) return;
		//m_mixer->Add(data, size, TMT_AUDIO, callId, 0, frequency);
	}
}

void VS_AviWriteGroup::PutVideo(char *callId, unsigned char *data, int size, int cur_abuff_durr)
{
	VS_AutoLock lock(this);

	if (m_is_valid) {
		write_iter iter = m_wstream_state.find(callId);
		if (iter == m_wstream_state.end() || !iter->second.is_init) return;
		//m_mixer->Add(data, size, TMT_VIDEO, callId, cur_abuff_durr);
	}
}

void VS_AviWriteGroup::SetPriority(const char *callid)
{
	VS_AutoLock lock(this);
	if (m_is_valid) {
		//m_mixer->SetPriority(callid);
	}
	m_LastPStream = callid;
}

void VS_AviWriteGroup::SetMixerMode(int sndMBps)
{
	int mixer_mode(0);
	int mbps(0);
	for (int i = 0; i < 8; i++) {
		//if (mixer_grid[i].arW == 16 && mixer_grid[i].arH == 9) {
			//int mbps = mixer_grid[i].W * mixer_grid[i].H * DEFAULT_AVI_FPS / 256;
			if (mbps <= sndMBps / 2) {
				mixer_mode = i;
			}
		//}
	}
	m_mixer_mode = mixer_mode;
}


/*
/// for tests
void VS_AviWriteGroup::PutVideo(char *callId, unsigned char *data, int size, int cur_abuff_durr)
{
	VS_AutoLock lock(this);

	if (m_is_valid) {
		write_iter iter = m_wstream_state.begin();
		while (true) {
			if (iter == m_wstream_state.end() || !iter->second.is_init) return;
			m_mixer->Add(data, size, TMT_VIDEO, (char*)iter->first.c_str(), cur_abuff_durr);
			iter++;
		}
	}
}
*/

bool VS_AviWriteGroup::Start()
{
	VS_AutoLock lock(this);

	if (!m_is_valid)
		return false;
	DTRACE(VSTM_AVIW, "Start Record AVI ");
	m_is_started = true;
	return true;
}

bool VS_AviWriteGroup::Pause()
{
	VS_AutoLock lock(this);

	if (!m_is_valid)
		return false;
	DTRACE(VSTM_AVIW, "Pause Record AVI ");
	m_is_started = false;
	return true;
}

void VS_AviWriteGroup::DrawLogo(unsigned char *video)
{
	if (m_imgLogo) {
		unsigned char *pVideo = video + m_mf.dwVideoWidht * (m_mf.dwVideoHeight - m_logoH - 16) + (m_mf.dwVideoWidht - m_logoW - 16);
		unsigned char *pLogo = m_imgLogo + 4 * m_logoW * (m_logoH - 1);
		for (int i = 0; i < m_logoH; i++) {
			for (int j = 0; j < m_logoW; j++) {
				int src = pLogo[4*j+3];
				int dst = (int)pVideo[j];
				dst = dst * (255 - src) / 255 + src;
				if (dst > 255) dst = 255;
				pVideo[j] = dst;
			}
			pVideo += m_mf.dwVideoWidht;
			pLogo -= m_logoW * 4;
		}
	}
}

DWORD VS_AviWriteGroup::Loop(LPVOID hEvDie)
{
	int exit = 0;
	int asize = 0, vsize = 0;
	unsigned char *abuff=(unsigned char *)malloc(m_mf.dwAudioSampleRate * 2 * 10),
				  *vbuff=(unsigned char *)malloc(m_mf.dwVideoWidht * m_mf.dwVideoHeight * 3 / 2),
				  *buffcmp=(unsigned char *)malloc(m_mf.dwVideoWidht * m_mf.dwVideoHeight),
				  *abuffcmp=(unsigned char *)malloc(m_mf.dwAudioSampleRate * 2 * 10);
	VS_VideoCodecParam prm;
	memset(&prm, 0, sizeof(VS_VideoCodecParam));
	bool aret, vret, close_writer;
	int startTime = 0;
	int currTime = 0;
	int lastTime = 0;
	int deltaTime = 0;
	int writeDeltaTime = 0;
	int videoTimestamp = 0;
	int constAudioVideoInterval = 0;

	if (m_is_valid) {
		aret = false;
		vret = false;
		close_writer = false;
		do {
			DWORD waitRes = WaitForSingleObject(hEvDie, 20);
			switch(waitRes) {
				case WAIT_FAILED:
					Close();
					exit = 1;
					break;
				case WAIT_OBJECT_0 + 0:
					exit = 2;
					break;
				case WAIT_TIMEOUT:
					break;
			}

			Lock();

			if (!startTime)
				lastTime = startTime = timeGetTime();

			lastTime = currTime;
			currTime = timeGetTime() - startTime;
			deltaTime = currTime - lastTime;

			writeDeltaTime += deltaTime;

			if (videoTimestamp)
				videoTimestamp += deltaTime;

			DTRACE(VSTM_AVIW, "deltaTime : %d", deltaTime);

			//aret = m_mixer->Get(abuff, asize, TMT_AUDIO, (unsigned int)currTime, deltaTime);
			//vret = m_mixer->Get(vbuff, vsize, TMT_VIDEO, (unsigned int)currTime, deltaTime);

			DTRACE(VSTM_AVIW, "aret : %d", aret);

			UnLock();

			if (vret || aret) {
				if (aret) { // аудио пишем постоянно
					if (!m_is_pcm) {
						asize = m_acodec->Convert(abuff, abuffcmp, asize);

						DTRACE(VSTM_AVIW, "asize : %d", asize);

						if (asize > 0) {
							m_avi.WriteAudioTimeAbs((char*)abuffcmp, asize, currTime);
						}
					} else {
						if (asize > 0) {
							m_avi.WriteAudioTimeAbs((char*)abuff, asize, currTime);
						}
					}
				}

				if (vret) {
					DTRACE(VSTM_AVIW, "vsize : %d", vsize);

					if (writeDeltaTime > 1000.0f / float(DEFAULT_AVI_FPS))
					{
						prm.cmp.KeyFrame = 0;
						DrawLogo(vbuff);
						vsize = m_vcodec->Convert(vbuff, buffcmp, &prm);

						if (!videoTimestamp)
						{
							videoTimestamp = 1;
							constAudioVideoInterval = currTime;
						}

						m_avi.WriteVideoTimeAbs((char*)buffcmp, vsize, prm.cmp.IsKeyFrame == 1, videoTimestamp);

						int diffTime = int(m_avi.GetCurrentAudioTime()) - (videoTimestamp + constAudioVideoInterval);

						if (diffTime < -120) // video is ahead
						{
							videoTimestamp -= 100;
						}
						else if (diffTime > 120)
						{
							videoTimestamp += 100;
						}

						writeDeltaTime = 0;
					}
				}
			}

			Lock();

			if (exit != 0 && !aret && !vret) {
				Release();
				close_writer = true;
			}

			UnLock();

		} while (!close_writer);
	}

	if (abuff) free(abuff); abuff = 0;
	if (vbuff) free(vbuff); vbuff = 0;
	if (buffcmp) free(buffcmp); buffcmp = 0;
	if (abuffcmp) free(abuffcmp); abuffcmp = 0;

	return NOERROR;
}

int VS_AviWriteGroup::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;
	bool res = false;

	if (strcmp(pSection, "Create") == 0) {
		if (Init((wchar_t*)(_bstr_t)*var)) {
			res = Start();
		}
	}
	else if (strcmp(pSection, "Close")==0) {
		if (Pause()) {
			Close();
			res = true;
		}
	}
	else if (strcmp(pSection, "AddUser")==0) {
		VARIANT *vars = 0;
		int num = ExtractVars(vars, var);
		if (num > 1) {
			if (OpenStream((char*)(_bstr_t)vars[0], (char*)(_bstr_t)vars[1]) == 0)
				res = true;
		}
		if (num > 0)
			delete[] vars;
	}
	else if (strcmp(pSection, "RemoveUser")==0) {
		CloseStream((char*)(_bstr_t)*var);
		res = true;
	}
	else if (strcmp(pSection, "ChangeUser")==0) {
		VARIANT *vars = 0;
		int num = ExtractVars(vars, var);
		if (num > 1) {
			if (ChangeStream((char*)(_bstr_t)vars[0], (char*)(_bstr_t)vars[1]) == 0)
				res = true;
		}
		if (num > 0)
			delete[] vars;
	}
	else
		return VS_INTERFACE_NO_FUNCTION;
	return res ? VS_INTERFACE_OK : VS_INTERFACE_INTERNAL_ERROR;
}