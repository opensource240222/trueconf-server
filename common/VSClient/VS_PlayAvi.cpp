/**
 **************************************************************************
 * \file VS_PlayAvi.cpp
 * (c) 2002-2007 Visicron Systems, Inc. All rights reserved.
 *									http://www.visicron.net/
 * \brief Implement Play avi files module
 * \b Project Client
 * \author SMirnovK
 * \date 22.01.2007
 *
 * $Revision: 12 $
 *
 * $History: VS_PlayAvi.cpp $
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 12.05.12   Time: 11:57
 * Updated in $/VSNA/VSClient
 * - were added mirror self view video
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 15.03.12   Time: 9:00
 * Updated in $/VSNA/VSClient
 * - avi player : video render now system default
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 3.10.11    Time: 18:14
 * Updated in $/VSNA/VSClient
 * - ench AviWriter : new presets, blending display name, h.263 -> vp8
 * - refactoring AviWriter
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 25.08.11   Time: 12:42
 * Updated in $/VSNA/VSClient
 * - fix video render for avi player
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 22.02.11   Time: 13:14
 * Updated in $/VSNA/VSClient
 * - fix 8bit audio
 * - fix audio channel num
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 10.09.10   Time: 18:28
 * Updated in $/VSNA/VSClient
 * - fix AviPlayer for 8bit PCM
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 6.08.09    Time: 18:54
 * Updated in $/VSNA/VSClient
 * - allow On2 VP7 decoder in avi player
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 5.08.09    Time: 19:48
 * Updated in $/VSNA/VSClient
 * - modify avi player: support system videocodecs, support play only
 * audio
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 4.08.09    Time: 20:28
 * Updated in $/VSNA/VSClient
 * - fix DS for avi player
 * - add AudioCodecSystem class, change acmDriverEnumCallback function
 * - avi player support system audio codecs
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 24.11.08   Time: 13:23
 * Updated in $/VSNA/VSClient
 * - were added recoding for group confrrence (not enable)
 * - were added new mixer module
 * - were modified CAviFile (add support synch MS GSM 6.10 in play/read
 * mode)
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 17.03.08   Time: 12:57
 * Updated in $/VSNA/VSClient
 * - bugfix#4041
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 22.08.07   Time: 16:26
 * Updated in $/VS2005/VSClient
 * - open reading without vfw decompress interface (speed up)
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 8.08.07    Time: 17:26
 * Updated in $/VS2005/VSClient
 * - Gui desires
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 25.07.07   Time: 18:21
 * Updated in $/VS2005/VSClient
 * исправил зацикливание при попытке сделать setposition при невалидном
 * файле
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 23.07.07   Time: 16:52
 * Updated in $/VS2005/VSClient
 * - more accurate SetPosition()
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 5.07.07    Time: 14:03
 * Updated in $/VS2005/VSClient
 * - locked avi playing from SetPosition
 * - errors in self avi writing fixed
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 7.06.07    Time: 19:05
 * Updated in $/VS2005/VSClient
 * - set/get position in avi (playing mode)
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 21.02.07   Time: 12:30
 * Updated in $/VS2005/VSClient
 * - Bugfix #1741
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 27.01.07   Time: 16:49
 * Updated in $/VS/VSClient
 * - sinc for video in avi play module increased to 200 mc
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 26.01.07   Time: 14:23
 * Updated in $/VS/VSClient
 * - stop avi playing if audio OR vidieo ends
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 25.01.07   Time: 14:07
 * Updated in $/VS/VSClient
 * - avi play first errors corrected, some impruvements
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 24.01.07   Time: 19:41
 * Created in $/VS/VSClient
 * - play avi module
 *
****************************************************************************/
#include <algorithm>
#include <ctime>

#include "VS_PlayAvi.h"
#include "..\std\cpplib\VS_MediaFormat.h"
#include "std-generic/cpplib/utf8.h"
#include "VS_Dmodule.h"
#include "Transcoder/AudioCodecSystem.h"

/**
 **************************************************************************
 ****************************************************************************/
VS_PlayAviFile::VS_PlayAviFile()
{
	m_vRender = 0;
	m_whwnd = 0;
	m_nhwnd = 0;
	m_vframe = 0;
	m_Paused = -1;
	m_Duration = 0;
	m_acodec = 0;
	m_smpl_read = 0;
	SetTimeShift(0);
	m_is_pcm = 0;
	m_is_video = false;
	m_ppRender = NULL;
}

/**
 **************************************************************************
 ****************************************************************************/
VS_PlayAviFile::~VS_PlayAviFile()
{
	Release();
}

/**
 **************************************************************************
 * \param	filename	 - file name to open
 * \param	videoWindow	 - window to draw
 * \param	notifyWindow - window to notify if playing thread ends
 * \return	 0 = все ОК,
 *			-1 = неизвестная ошибка,
 *			-2 = ошибка инициализации видео рендера, (removed)
 *			-3 = ошибка инициализации видео декодера, (removed)
 *			-4 = неподдерживаемый видеокодек, (removed)
 *			-5 = нет видео в файле (нет формата), (removed)
 *			-6 = ошибка инициализации аудиорендера, (removed)
 *			-7 = аудио сжато (не PCM формат), (removed)
 *			-8 = нет аудио (нет формата), (removed)
 *			-9 = ошибка открытия ави файла или файл пустой.
 * \date    24-01-2007		Created
 ****************************************************************************/
int VS_PlayAviFile::Init(wchar_t * filename, HWND videoWindow, HWND notifyWindow, int dev)
{
	m_whwnd = videoWindow;
	m_nhwnd = notifyWindow;
	int result = 0;
	Release();

	auto utf8_file_name = vs::UTF16toUTF8Convert(filename);

	if (m_avi.Init(utf8_file_name)) {
		VSVideoFile::SAudioInfo audioInfo;
		VSVideoFile::SVideoInfo videoInfo;
		int size = 0;
		WAVEFORMATEX *wf = new WAVEFORMATEX;
		BITMAPINFOHEADER *bm = new BITMAPINFOHEADER;
		if (m_avi.GetAudioFormat(audioInfo))
		{
			wf->nChannels = audioInfo.NumChannels;
			wf->wBitsPerSample = audioInfo.BitsPerSample;
			wf->nSamplesPerSec = audioInfo.SampleRate;
			wf->nAvgBytesPerSec = audioInfo.SampleRate * audioInfo.BitsPerSample / 8;
			wf->cbSize = sizeof(WAVEFORMATEX);
			wf->nBlockAlign = wf->wBitsPerSample / 8;

			switch (audioInfo.CodecID)
			{
			case VSVideoFile::ACODEC_ID_PCM:
				wf->wFormatTag = VS_ACODEC_PCM;
				break;

			case VSVideoFile::ACODEC_ID_MP3:
				wf->wFormatTag = VS_ACODEC_MP3;
				break;
			}

			if (wf->nChannels <= 2) {
				if (wf->wFormatTag != WAVE_FORMAT_PCM/* || (wf->wFormatTag == WAVE_FORMAT_PCM && wf->wBitsPerSample == 8)*/) {
					m_smpl_read = wf->nAvgBytesPerSec / (wf->nBlockAlign * 10) + 1;
					m_acodec = VS_RetriveAudioCodec(wf->wFormatTag, false);
					if (!m_acodec) m_acodec = new AudioCodecSystem(wf->wFormatTag, false, wf->nBlockAlign);
					if (m_acodec && (m_acodec->Init(wf) == 0)) {
						wf->cbSize = 0;
						wf->wBitsPerSample = 16;
						wf->nBlockAlign = wf->wBitsPerSample * wf->nChannels / 8;
						wf->nAvgBytesPerSec = wf->nSamplesPerSec * wf->nBlockAlign;
						wf->wFormatTag = WAVE_FORMAT_PCM;
					} else {
						result = -7;
					}
				} else {
					m_is_pcm = 1;
					if (wf->wBitsPerSample == 8) {
						m_is_pcm = 2;
						wf->wBitsPerSample = 16;
						wf->nBlockAlign = wf->nChannels * wf->wBitsPerSample / 8;
						wf->nAvgBytesPerSec = wf->nBlockAlign * wf->nSamplesPerSec;
					}
					m_smpl_read = wf->nAvgBytesPerSec / (wf->nBlockAlign * 8);
				}
				if (result == 0) {
					m_aRender.RegularDevFlag=1;
					m_aRender.Init(dev, wf);
				}
			}
			delete wf;
		}
		if (m_avi.GetVideoFormat(videoInfo))
		{
			VS_MediaFormat mf;
			CColorMode cm;
			int mode;

			bm->biWidth = videoInfo.Width;
			bm->biHeight = videoInfo.Height;

			switch (videoInfo.CodecID)
			{
			case VSVideoFile::VCODEC_ID_VP8:
				bm->biCompression = VS_VCODEC_VPX;
				break;

			case VSVideoFile::VCODEC_ID_H264:
				bm->biCompression = VS_VCODEC_H264;
				break;
			}

			mf.SetVideo(bm->biWidth, bm->biHeight, bm->biCompression);

			if (m_decoder.ConnectToVideoDecompressor(&mf) == 0) {
				m_vframe = (BYTE*)malloc(bm->biWidth * bm->biHeight * 3 / 2);
				mode = cm.I420;
				m_is_video = true;
			}

			if (m_is_video) {
				m_vRender = CVideoRenderBase::RetrieveVideoRender(videoWindow, 0);
				m_ppRender = new DWORD;
				*(DWORD*)m_ppRender = *(DWORD*)(&m_vRender);
				cm.SetColorMode(NULL, mode, mf.dwVideoHeight, mf.dwVideoWidht);
				if (m_vRender->iInitRender(videoWindow, m_vframe, &cm, false) != 0) m_is_video = false;
			}
			if (!m_is_video) {
				if (m_vRender) delete m_vRender; m_vRender = 0;
				m_decoder.DisconnectToVideoDecompressor();
				free(m_vframe); m_vframe = 0;
			}
			delete bm;
		}
		m_Duration = std::max(m_avi.GetVideoDuration(), m_avi.GetAudioDuration());
		if (!m_aRender.IsValid() && !m_is_video)
			result = -9;
	}
	else
	{
		result = -9;
	}

	m_LastSeekTime = -1;

	// Clone of VS_PlayAviFile::Start(), but "m_Paused = 0" for stop playing
	m_NewSeekTime = 0;
	m_Paused = 0;
	if (!IsThreadActiv())
		if (!ActivateThread(this))
			result = -1;

	if (result != 0)
		Release();

	return result;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_PlayAviFile::Release()
{
	DesactivateThread();
	if (m_vRender) delete m_vRender; m_vRender = 0;
	if (m_ppRender) delete m_ppRender; m_ppRender = 0;
	m_decoder.DisconnectToVideoDecompressor();
	if (m_vframe) delete m_vframe; m_vframe = 0;
	if (m_acodec) delete m_acodec; m_acodec = 0;
	m_aRender.Release();
	m_avi.Release();
	m_Paused = -1;
	m_Duration = 0;
	m_smpl_read = 0;
	m_is_pcm = 0;
	m_is_video = false;
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_PlayAviFile::Start()
{
	bool ret = false;
	if (!IsThreadActiv())
		ret = ActivateThread(this);
	m_Paused = -1;
	m_aRender.Start();
	return ret;
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_PlayAviFile::Stop()
{
	DesactivateThread();
	return true;
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_PlayAviFile::Pause()
{
	VS_AutoLock lock(this);
	m_Paused = (m_is_video) ? m_avi.GetCurrentVideoTime() : m_avi.GetCurrentAudioTime();
	m_aRender.Stop();
	return true;
}

/**
 **************************************************************************
 ****************************************************************************/
DWORD VS_PlayAviFile::Loop(LPVOID hEvDie)
{
	int sizeTmp = 0x40000;
	if (m_is_video) {
		VSVideoFile::SVideoInfo vi;
		m_avi.GetVideoFormat(vi);
		sizeTmp = vi.Height*vi.Width / 2;
	}

	unsigned char *pTmp = (unsigned char *)malloc(sizeTmp),
				  *pTmpOut = (unsigned char *)malloc(65536 * 4),
				  *pDecFrame = 0;
	short *pTmpRender = (m_is_pcm == 1) ? (short*)pTmp : (short*)pTmpOut;

	int i = 0;
	int ExitReason = 0;
	int audiotime, videotime, duetime;
	bool AudioIsOut = false, VideoIsOut = false,
		 VideoWasPaused = false,
		 NeedKey = false;
	DWORD StartVideoTime = timeGetTime() - m_avi.GetCurrentVideoTime();
	m_LastSeekTime = -1;

	HANDLE handles[2];
	handles[0] = hEvDie;
	handles[1] = m_aRender.GetCmpleteEvent();

	do {
		DWORD waitRes = WaitForMultipleObjects(2, handles, FALSE, 16);
		switch(waitRes)
		{
		case WAIT_FAILED:
			ExitReason = 1;
			break;
		case WAIT_OBJECT_0 + 0:
			ExitReason = 2;
			break;
		case WAIT_OBJECT_0 + 1:
		case WAIT_TIMEOUT:
			break;
		}

		UpdateSeekPosition(pTmp);

		if (m_Paused !=- 1) {
			VideoWasPaused = true;
			continue;
		}
		if (VideoWasPaused && !m_aRender.IsValid()) {
			StartVideoTime = timeGetTime() - m_avi.GetCurrentVideoTime();
			VideoWasPaused = false;
		}

		Lock();

		long len = 0;
		int t1, t2;
		if (m_aRender.IsValid()) { // if audio exist
			int curr_durr = m_aRender.GetCurrBuffDurr();
			if (curr_durr < 500 && !AudioIsOut) {
				len = m_avi.ReadAudio((char*)pTmp);
				if (m_is_pcm == 0) { // decoding audio
					if (len > 0) len = m_acodec->Convert(pTmp, pTmpOut, len);
				}
				if (len > 0) {
					if (m_is_pcm == 2) {
						for (i = 0; i < len; i++) {
							pTmpRender[i] = (pTmp[i] - 128) << 8;
						}
						len *= 2;
					}
					m_aRender.Play((char*)pTmpRender, len);
				} else if (len < 0) {
					AudioIsOut = true;
				}
			}
			t1 = m_avi.GetCurrentAudioTime();
			t2 = m_aRender.GetCurrBuffDurr();
			audiotime = m_avi.GetCurrentAudioTime() - m_aRender.GetCurrBuffDurr();
		}
		if (m_is_video) { // if video exist
			videotime = m_avi.GetCurrentVideoTime() + m_timeshift;
			duetime = (m_aRender.IsValid()) ? audiotime : timeGetTime() - StartVideoTime;
			len = 0;
			if (duetime > videotime) {
				bool IsKey = 0;
				if (!VideoIsOut) {
					len = m_avi.ReadVideo((char*)pTmp, &IsKey);
				}
				if (len > 0) {
					if (!NeedKey || IsKey) {
						if (m_decoder.DecompressFrame(pTmp, len, IsKey, m_vframe) < 0)
							NeedKey = true;
						else {
							m_vRender->m_bNewFrame = 1;
							DTRACE(VSTM_AVIW, "draw video : %d", timeGetTime());
							m_vRender->DrawFrame(m_whwnd);
							NeedKey = false;
						}
					}
				}
				else if (len<0)
					VideoIsOut = true;
			}
		}
		if ((AudioIsOut || VideoIsOut) && m_aRender.GetCurrBuffDurr() <= 120)
			ExitReason = 3;

		UnLock();
	} while (ExitReason == 0);

	free(pTmp); pTmpOut = 0;
	free(pTmpOut); pTmp = 0;
	PostMessage(m_nhwnd , WM_USER + 19, 0, ExitReason);
	m_aRender.Stop();
	m_Paused = -1;
	return NOERROR;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_PlayAviFile::SetPosition(long tm)
{
	DTRACE(VSTM_AVIW, "_SetPosition(%d) : %d", tm, timeGetTime());

	m_NewSeekTime = tm;
}

/**
**************************************************************************
****************************************************************************/
void VS_PlayAviFile::UpdateSeekPosition(unsigned char *tmp)
{
	size_t m_SeekTimeToSet = m_NewSeekTime;
	if (m_LastSeekTime != m_SeekTimeToSet && m_Duration > 0)
	{
		DTRACE(VSTM_AVIW, "UpdateSeekPosition(%d) : %d", m_SeekTimeToSet, timeGetTime());
		size_t start = 0;
		size_t total = 0;
		size_t frameNum = 0;
		size_t totalReadVideo = 0;
		size_t totalDecompressFrame = 0;

		int mode = (int)m_is_video * 2 + (int)m_aRender.IsValid();

		start = timeGetTime();
		m_avi.SeekToTime(m_SeekTimeToSet);
		total = timeGetTime() - start;

		DTRACE(VSTM_AVIW, "SeekToTime() : %d", total);

		if (m_is_video) {
			int len = 0;

			while (m_SeekTimeToSet >= m_avi.GetCurrentVideoTime() && len >= 0) {
				bool IsKey = 0;
				unsigned char *pFrame = 0;

				start = timeGetTime();
				len = m_avi.ReadVideo((char*)tmp, &IsKey);
				totalReadVideo += timeGetTime() - start;

				frameNum++;

				start = timeGetTime();
				if (len > 0)
					m_vRender->m_bNewFrame = m_decoder.DecompressFrame(tmp, len, IsKey, m_vframe) >= 0;
				totalDecompressFrame += timeGetTime() - start;
			}

			DTRACE(VSTM_AVIW, "ReadVideo() : %d", totalReadVideo);
			DTRACE(VSTM_AVIW, "DecompressFrame() : %d", totalDecompressFrame);
			DTRACE(VSTM_AVIW, "frameNum : %d", frameNum);

			start = timeGetTime();
			while (m_avi.GetCurrentAudioTime() < m_avi.GetCurrentVideoTime())
			{
				if (m_avi.ReadAudio((char*)tmp) < 0)
					break;
			}
			total = timeGetTime() - start;

			DTRACE(VSTM_AVIW, "ReadAudio() : %d", total);

			if (m_vRender->m_bNewFrame)
				m_vRender->DrawFrame(m_whwnd);
		}

		m_LastSeekTime = m_SeekTimeToSet;
	}
}


/**
 **************************************************************************
 ****************************************************************************/
long VS_PlayAviFile::GetPosition(int type)
{
	if (type == 0)
	{
		if (m_NewSeekTime != m_LastSeekTime)
		{
			return m_NewSeekTime;
		}

		return (m_is_video) ? m_avi.GetCurrentVideoTime() : m_avi.GetCurrentAudioTime();
	}
	else
	{
		return m_Duration;
	}
}
