#include "VideoFile.h"
#include "Transcoder/VS_VfwVideoCodec.h"

#include <algorithm>

unsigned int g_TimeSay = 18000;
unsigned int g_TimeRec = 14000;
unsigned int g_TimePlayrec = 13000;

VideoFile::VideoFile()
{
	m_RecAcodec = 0;
	m_PlayAcodec = 0;
	m_tmp1 = m_tmp2 = 0;
	m_aBuff = (unsigned char *)malloc(100000);
}


VideoFile::~VideoFile()
{
	free(m_aBuff);
}

bool VideoFile::Init(char* filename, char* user, int mode)
{
	VS_AutoLock lock(this);
	m_inavi.Release();
	m_outavi.Release();

	m_srcfile = filename;
	m_inavi.Init(m_srcfile, false, false);

	m_start_time = timeGetTime();
	m_read_time = m_start_time;
	m_state = VF_SAY;
	m_waitKey = true;

	bool init = true;
	if (mode == WORK_MODE_VIDEOBOT) {
		SYSTEMTIME st;
		GetSystemTime(&st);
		VS_SimpleStr name(1024);
		sprintf(name, "%04d-%02d-%02d_%02d-%02d-%02d_%s.avi", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, user);
		m_recfile = name;
		init = m_outavi.Init(name, true);
	} else {
		m_state = VF_READYLOOP;
	}
	m_mode = mode;

	return init;
}

void VideoFile::Reset()
{
	m_start_time = timeGetTime();
	m_read_time = m_start_time;
	m_inavi.Reset();
}

bool VideoFile::SetSndFormat(VS_MediaFormat &fmt)
{
	m_sndfmt = fmt;
	PrepareForRead();
	return true;
}

bool VideoFile::SetRcvFormat(VS_MediaFormat &fmt)
{
	m_rcvfmt = fmt;
	if (m_RecAcodec) delete m_RecAcodec; m_RecAcodec = 0;
	m_RecAcodec = VS_RetriveAudioCodec(fmt.dwAudioCodecTag, false);
	WAVEFORMATEX wf;
	wf.cbSize = 0;
	wf.nSamplesPerSec = fmt.dwAudioSampleRate;
	wf.nChannels = 1;
	m_RecAcodec->Init(&wf);
	return true;
}

bool VideoFile::PrepareForRead()
{
	m_read_time = timeGetTime();
	if (m_PlayAcodec) {
		m_PlayAcodec->Release();
		delete m_PlayAcodec; m_PlayAcodec = 0;
	}
	m_PlayVcodec.DisconnectToVideoDecompressor();
	if (m_tmp1) free(m_tmp1); m_tmp1 = 0;
	if (m_tmp2) free(m_tmp2); m_tmp2 = 0;

	int size = 0;
	BITMAPINFOHEADER* bm = 0;
	WAVEFORMATEX *wf = 0;

	if (m_inavi.IsVideo()) {
		if ((size = m_inavi.GetFormat(bm))>0) {
			bm = (BITMAPINFOHEADER *)malloc(size);
			m_inavi.GetFormat(bm);
		}
	}

	if (m_inavi.IsAudio()) {
		if ((size = m_inavi.GetFormat(wf)) > 0) {
			wf = (WAVEFORMATEX *)malloc(size);
			m_inavi.GetFormat(wf);
		}
	}

	m_readfmt.SetZero();
	if (bm) {
		DWORD fcc = bm->biCompression;
		if (bm->biCompression == '08PV') fcc = VS_VCODEC_VPX;
		else if (bm->biCompression == '462H') fcc = VS_VCODEC_H264;
		m_readfmt.SetVideo(bm->biWidth, bm->biHeight, bm->biCompression=='08PV' ? VS_VCODEC_VPX : fcc, (DWORD)m_inavi.m_fps);
		m_PlayVcodec.ConnectToVideoDecompressor(&m_readfmt);
		size = std::max(m_sndfmt.dwVideoHeight* m_sndfmt.dwVideoWidht*3, m_readfmt.dwVideoHeight* m_readfmt.dwVideoWidht*3);
		m_tmp1 = (unsigned char *)malloc(size + 20);
		m_tmp2 = (unsigned char *)malloc(size + 20);
		free(bm);
	}
	if (wf) {
		m_readfmt.SetAudio(wf->nSamplesPerSec, wf->wFormatTag);
		m_PlayAcodec = VS_RetriveAudioCodec(wf->wFormatTag, false);
		m_PlayAcodec->Init(wf);
		free(wf);
	}
	return m_readfmt.IsAudioValid() || m_readfmt.IsVideoValid();
}

long VideoFile::GetSndFrameRate()
{
	return m_readfmt.dwFps;
}

long VideoFile::WriteAudio(unsigned char *buff, unsigned long size)
{
	VS_AutoLock lock(this);
	if (m_state!=VF_REC)
		return 0;
	if (!m_RecAcodec)
		return -1;

	unsigned char tb[65536];
	int ret = m_RecAcodec->Convert(buff, tb, size);
	if (ret > 0) {
		return m_outavi.WriteAudio(tb, ret) ? 1 : -3;
	}
	else
		return -2;

}

long VideoFile::WriteVideo(unsigned char *buff, unsigned long size, bool key, unsigned long VideoInterval)
{
	VS_AutoLock lock(this);
	if (m_state!=VF_REC)
		return 0;
	if (m_waitKey && !key)
		return -1;
	if (key)
		m_waitKey = false;

	return m_outavi.WriteVideo(buff, size, key, VideoInterval) ? 1 : -3;
}

long VideoFile::ReadAudio(unsigned char *buff, unsigned long &size)
{
	VS_AutoLock lock(this);
	if (m_state==VF_NONE)
		return 0;

	DWORD ct = timeGetTime();
	if (ct - m_read_time < m_inavi.GetAudioTime())
		return 0;

	int s = m_readfmt.dwAudioCodecTag==WAVE_FORMAT_PCM ? m_readfmt.dwAudioSampleRate*2/25 : AVISTREAMREAD_CONVENIENT;
	int ret = m_inavi.ReadAudio(m_aBuff, 65536, s);
	if (ret > 0) {
		if (m_readfmt.dwAudioCodecTag!=WAVE_FORMAT_PCM) {
			size = m_PlayAcodec->Convert(m_aBuff, buff, ret);
		}
		else {
			memcpy(buff, m_aBuff, ret);
			size = ret;
		}
	}
	return ret;
}

long VideoFile::ReadVideo(unsigned char *buff, unsigned long &size)
{
	VS_AutoLock lock(this);
	if (m_state==VF_NONE)
		return 0;

	DWORD ct = timeGetTime();
	if (ct - m_read_time < m_inavi.GetVideoTime())
		return 0;

	int ret = 0;

	if (m_inavi.IsDecompressVideo()){
		unsigned char * data = 0;
		ret = m_inavi.ReadDecompressedVideo(data);
		if (ret > 0) {
			BYTE *pY = m_tmp2;
			BYTE *pU = pY + m_readfmt.dwVideoWidht*m_readfmt.dwVideoHeight;
			BYTE *pV = pU + m_readfmt.dwVideoWidht*m_readfmt.dwVideoHeight/4;
			m_proc.ConvertBMF24ToI420(data, pY, pU, pV, m_readfmt.dwVideoWidht*3, m_readfmt.dwVideoHeight, m_readfmt.dwVideoWidht);
		}
	}
	else {
		bool key = false;
		ret = m_inavi.ReadVideo(m_tmp1, m_readfmt.dwVideoHeight* m_readfmt.dwVideoWidht*3, &key);
		if (ret > 0) {
			if (m_mode != WORK_MODE_DEMOGROUPCOMPRESS && m_mode != WORK_MODE_DEMOGROUPCOMPRESS_SVC) {
				m_PlayVcodec.DecompressFrame(m_tmp1, ret, key, m_tmp2);
			} else {
				memcpy(buff, m_tmp1, ret);
				size = (key) ? 1 : 0;
				return ret;
			}
		}
	}

	if (ret > 0) {
		unsigned char* decomp = m_tmp2;
		unsigned char* comp = m_tmp1;
		// resize
		if (m_sndfmt.dwVideoWidht*m_sndfmt.dwVideoHeight!=m_readfmt.dwVideoWidht*m_readfmt.dwVideoHeight) {
			double xk = (double)m_readfmt.dwVideoWidht / (double)m_sndfmt.dwVideoWidht;
			double yk = (double)m_readfmt.dwVideoHeight / (double)m_sndfmt.dwVideoHeight;
			double m_fFactorW = 0., m_fFactorH = 0.;

			int m_outWidthAR = m_readfmt.dwVideoWidht;
			int m_outHeightAR = m_readfmt.dwVideoHeight;
			int m_iOffsetW = 0;
			int m_iOffsetH = 0;
			if (yk < xk) {
				m_outWidthAR = ((int)((int)m_sndfmt.dwVideoWidht * yk + 1.5)) &~ 1;
				m_iOffsetW = ((int)m_readfmt.dwVideoWidht - m_outWidthAR) / 2;
				m_fFactorH = 1.0 / yk;
				m_fFactorW = (double)m_sndfmt.dwVideoWidht / (double)m_outWidthAR;
			} else if (xk <= yk) {
				m_outHeightAR = ((int)((int)m_sndfmt.dwVideoHeight * xk + 1.5)) &~ 1;
				m_iOffsetH = ((int)m_readfmt.dwVideoHeight - m_outHeightAR) / 2;
				m_fFactorW = 1.0 / xk;
				m_fFactorH = (double)m_sndfmt.dwVideoHeight / (double)m_outHeightAR;
			}

			unsigned char* pIn[3] = {decomp, decomp + m_readfmt.dwVideoWidht*m_readfmt.dwVideoHeight, decomp + m_readfmt.dwVideoWidht*m_readfmt.dwVideoHeight*5/4};
			unsigned char* pOut[3] = {comp, comp + m_sndfmt.dwVideoWidht*m_sndfmt.dwVideoHeight, comp + m_sndfmt.dwVideoWidht*m_sndfmt.dwVideoHeight*5/4};
			bool ret = m_proc.ResampleCropI420(pIn, pOut,
				m_readfmt.dwVideoWidht, m_readfmt.dwVideoHeight, m_readfmt.dwVideoWidht, m_sndfmt.dwVideoWidht, m_sndfmt.dwVideoHeight, m_sndfmt.dwVideoWidht,
				m_outWidthAR, m_outHeightAR, m_iOffsetW, m_iOffsetH, m_fFactorW, m_fFactorH);

			size = m_sndfmt.dwVideoWidht*m_sndfmt.dwVideoHeight*3/2;
			memcpy(buff, m_tmp1, size);
		} else {
			size = m_sndfmt.dwVideoWidht*m_sndfmt.dwVideoHeight*3/2;
			memcpy(buff, m_tmp2, size);
		}
	}
	return ret > 0 ? size : ret;
}

VideoFile::State VideoFile::CheckState()
{
	VS_AutoLock lock(this);
	unsigned long curr_time = timeGetTime();
	if (m_state==VF_NONE) {
	}
	else if (m_state==VF_SAY) {
		if (curr_time - m_start_time > g_TimeSay) {
			m_state = VF_REC;

			BITMAPINFOHEADER bm;
			memset(&bm, 0, sizeof (BITMAPINFOHEADER));
			bm.biSize = sizeof (BITMAPINFOHEADER);
			bm.biCompression = m_rcvfmt.dwVideoCodecFCC == VS_VCODEC_VPX ? '08PV' : m_rcvfmt.dwVideoCodecFCC;
			bm.biWidth = m_rcvfmt.dwVideoWidht;
			bm.biHeight = m_rcvfmt.dwVideoHeight;
			bm.biPlanes = 1;
			m_outavi.m_fps = 30.;
			m_outavi.SetFormat(&bm);

			WAVEFORMATEX wf;
			memset(&wf, 0, sizeof (WAVEFORMATEX));
			wf.wFormatTag = VS_ACODEC_PCM;
			wf.nSamplesPerSec = m_rcvfmt.dwAudioSampleRate;
			wf.nChannels = 1;
			wf.wBitsPerSample = 16;
			wf.nBlockAlign = wf.wBitsPerSample * wf.nChannels / 8;
			wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
			m_outavi.SetFormat(&wf);
		}
	}
	else if (m_state==VF_REC) {
		if (curr_time - m_start_time > (g_TimeSay + g_TimeRec)) {
			// rebase avi
			m_state = VF_PLAYREC;
			m_outavi.Release();
			m_inavi.Release();
			m_inavi.Init(m_recfile, false, false);
			PrepareForRead();
		}
	}
	else if (m_state==VF_PLAYREC) {
		if (curr_time - m_start_time > (g_TimeSay + g_TimeRec + g_TimePlayrec)) {
			m_state = VF_ENDCALL;
		}
	}
	else if (m_state==VF_READYLOOP) {
		m_outavi.Release();
		m_inavi.Release();
		if (m_mode == WORK_MODE_DEMOGROUP) m_inavi.Init(m_srcfile, false);
		else m_inavi.Init(m_srcfile, false, false);
		PrepareForRead();
		m_state = VF_PLAYLOOP;
	}
	return m_state;
}

