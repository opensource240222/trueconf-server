/****************************************************************************
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
*
* Project: Video
*
* $History: CAviFile.cpp $
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 5.08.09    Time: 19:48
 * Updated in $/VSNA/Video/winapi
 * - modify avi player: support system videocodecs, support play only
 * audio
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 4.08.09    Time: 20:28
 * Updated in $/VSNA/Video/winapi
 * - fix DS for avi player
 * - add AudioCodecSystem class, change acmDriverEnumCallback function
 * - avi player support system audio codecs
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 24.11.08   Time: 13:23
 * Updated in $/VSNA/Video/winapi
 * - were added recoding for group confrrence (not enable)
 * - were added new mixer module
 * - were modified CAviFile (add support synch MS GSM 6.10 in play/read
 * mode)
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/Video/winapi
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 22.08.07   Time: 16:06
 * Updated in $/VS2005/Video/winapi
 * - speed up for avi playing in case of no video auto decompress
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 8.08.07    Time: 17:26
 * Updated in $/VS2005/Video/winapi
 * - Gui desires
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 24.07.07   Time: 21:25
 * Updated in $/VS2005/Video/winapi
 * - bugfix #2928
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 18.07.07   Time: 17:42
 * Updated in $/VS2005/Video/winapi
 * - bugfix2806
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 5.07.07    Time: 19:04
 * Updated in $/VS2005/Video/winapi
 * - bugfix #2702
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 7.06.07    Time: 19:05
 * Updated in $/VS2005/Video/winapi
 * - set/get position in avi (playing mode)
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Video/winapi
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 24.01.07   Time: 19:41
 * Updated in $/VS/Video/WinApi
 * - play avi module
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 26.09.05   Time: 13:01
 * Updated in $/VS/Video/WinApi
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 5.04.05    Time: 10:53
 * Updated in $/VS/Video/WinApi
 * video depend strongly from videointerval
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 6.07.04    Time: 20:58
 * Updated in $/VS/video/winapi
 * correct size of format info
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 5.07.04    Time: 14:39
 * Updated in $/VS/Video/WinApi
 * new sinc shema
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 30.06.04   Time: 18:04
 * Updated in $/VS/Video/WinApi
 * added sender and reciever audio support
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 28.06.04   Time: 18:52
 * Updated in $/VS/Video/WinApi
 * fps settings
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 28.06.04   Time: 17:46
 * Updated in $/VS/Video/WinApi
 * addded time support for video
 * added set-get media format support
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 22.06.04   Time: 20:11
 * Updated in $/VS/Video/WinApi
 * class for avi files
 * avi source adapted to CAVIFILe
*
****************************************************************************/

/****************************************************************************
* \file CAviFile.h
* \brief Define Interface to read - write operation for avi file
****************************************************************************/

/****************************************************************************
* Includes
****************************************************************************/

#include "CAviFile.h"
#define UNICODE
#include <vfw.h>

#include <algorithm>

/****************************************************************************
* Structures
****************************************************************************/
struct VS_AviInfo
{
	VS_AviInfo() {
		memset(this, 0, sizeof(VS_AviInfo));
	}
	PAVIFILE			m_pAviFile;
	PAVISTREAM			m_pStreamVideo;
	PAVISTREAM			m_pStreamAudio;
	AVISTREAMINFO		m_VideoStreamInfo;
	AVISTREAMINFO		m_AudioStreamInfo;
	PGETFRAME			m_pGetFrame;
};

/****************************************************************************
* Classes
****************************************************************************/
/****************************************************************************
* Begin CAviFile class
****************************************************************************/
/****************************************************************************
* Constructor. Set variables to zero
****************************************************************************/
CAviFile::CAviFile()
{
	memset(this, 0, sizeof(CAviFile));
}

/****************************************************************************
* Destructor. Release resources
****************************************************************************/
CAviFile::~CAviFile()
{
	Release();
}

/****************************************************************************
* Open AVI file to read/write operations
****************************************************************************/
bool CAviFile::Init(const wchar_t* Name, bool write, bool initDecompress)
{
	if (!Name || !*Name) return false;

	m_avinfo = new VS_AviInfo;

	AVIFileInit();
	m_IsWrite = write;
	if (m_IsWrite) {
		DeleteFileW(Name);
		m_IsValid = AVIFileOpen(&m_avinfo->m_pAviFile, Name, OF_WRITE|OF_CREATE, NULL) == AVIERR_OK;
	}
	else {

		m_IsValid = AVIFileOpen(&m_avinfo->m_pAviFile, Name, OF_READ, NULL)  == AVIERR_OK;
		m_initDecompress = initDecompress;
		if (m_IsValid) {
			AVIFILEINFO	avifileinfo;
			AVIFileInfo(m_avinfo->m_pAviFile, &avifileinfo, sizeof(AVIFILEINFO));
			if (avifileinfo.dwStreams<1) {
				m_IsValid = false;
			}
			else {
				GetFormat((WAVEFORMATEX*)0);
				GetFormat((BITMAPINFOHEADER*)0);
			}
		}
	}
	return m_IsValid;
}

/****************************************************************************
* Open AVI file to read/write operations
****************************************************************************/
bool CAviFile::Init(const char* Name, bool write, bool initDecompress)
{
	if (!Name || !*Name) return false;
	wchar_t wname[260] = {0};

    int res = MultiByteToWideChar(CP_ACP, 0L, Name, -1, wname, 260);
	if (res > 0)
		return Init(wname, write, initDecompress);
	else
		return false;
}


/****************************************************************************
* Close all resources
****************************************************************************/
void CAviFile::Release()
{
	if (m_avinfo) {
		if (m_avinfo->m_pGetFrame)
			AVIStreamGetFrameClose(m_avinfo->m_pGetFrame);
		if (m_avinfo->m_pStreamAudio)
			AVIStreamClose(m_avinfo->m_pStreamAudio);
		if (m_avinfo->m_pStreamVideo)
			AVIStreamClose(m_avinfo->m_pStreamVideo);
		if (m_avinfo->m_pAviFile)
			AVIFileClose(m_avinfo->m_pAviFile);
		delete m_avinfo;
	}
	AVIFileExit();
	if (m_bmInfo) free(m_bmInfo);
	if (m_wFormat) free(m_wFormat);
	memset(this, 0, sizeof(CAviFile));
}

/****************************************************************************
* Open video stream and set its format
****************************************************************************/
bool CAviFile::SetFormat(BITMAPINFOHEADER *bm)
{
	if (!m_IsValid || !m_IsWrite || !bm) return false;
	// check stream
	if (m_avinfo->m_pStreamVideo)
		AVIStreamClose(m_avinfo->m_pStreamVideo);
	m_avinfo->m_pStreamVideo = 0;
	m_IsVideo = false;

	// Fill in the header for the video stream....
	memset(&m_avinfo->m_VideoStreamInfo, 0, sizeof(AVISTREAMINFO));
	m_avinfo->m_VideoStreamInfo.fccType                = streamtypeVIDEO;// stream type
	m_avinfo->m_VideoStreamInfo.fccHandler             = bm->biCompression;
	m_avinfo->m_VideoStreamInfo.dwScale                = 100;
	if (m_fps==0.0) m_fps = 10.0;
	m_avinfo->m_VideoStreamInfo.dwRate                 = (int)(m_fps*100+0.5);
	m_avinfo->m_VideoStreamInfo.dwQuality				 = -1;
	SetRect(&m_avinfo->m_VideoStreamInfo.rcFrame, 0, 0, bm->biWidth, bm->biHeight);	// rectangle for stream

	// And create the stream;
	if (AVIFileCreateStream(m_avinfo->m_pAviFile, &m_avinfo->m_pStreamVideo, &m_avinfo->m_VideoStreamInfo) == AVIERR_OK) {
		int size = bm->biSize+bm->biClrUsed*sizeof(RGBQUAD);
		if (AVIStreamSetFormat(m_avinfo->m_pStreamVideo, 0, bm, size)==AVIERR_OK) {
			if (m_bmInfo) free(m_bmInfo);
			m_bmInfo = (BITMAPINFOHEADER*)malloc(size);
			memcpy(m_bmInfo, bm, size);
			m_IsVideo = true;
		}
	}

	return m_IsVideo;
}

/****************************************************************************
* Read format from video stream
****************************************************************************/
int CAviFile::GetFormat(BITMAPINFOHEADER *bm)
{
	if (!m_IsValid) return -1;

	if (!m_IsWrite) {
		if (!m_IsVideo) {
			if (AVIFileGetStream(m_avinfo->m_pAviFile, &m_avinfo->m_pStreamVideo, streamtypeVIDEO, NULL)!=0)
				return -2;

			if (AVIStreamInfo(m_avinfo->m_pStreamVideo, &m_avinfo->m_VideoStreamInfo, sizeof(AVISTREAMINFO))!=0)
				return -3;
			m_fps = (float)m_avinfo->m_VideoStreamInfo.dwRate/m_avinfo->m_VideoStreamInfo.dwScale;

			long size = 0;
			if (AVIStreamReadFormat(m_avinfo->m_pStreamVideo, 0, 0, &size)!=0)
				return -4;

			if (m_bmInfo) free(m_bmInfo);
			m_bmInfo = (BITMAPINFOHEADER*)malloc(size);
			if (AVIStreamReadFormat(m_avinfo->m_pStreamVideo, 0, m_bmInfo, &size)!=0)
				return -5;

			m_IsVideo = true;

			if (m_initDecompress) {
				// try to open decompressed video
				BITMAPINFOHEADER bmDecomp;
				memset(&bmDecomp, 0, sizeof(BITMAPINFOHEADER));
				bmDecomp.biSize = sizeof(BITMAPINFOHEADER);
				bmDecomp.biBitCount = 24;
				bmDecomp.biCompression = BI_RGB;
				bmDecomp.biHeight = m_avinfo->m_VideoStreamInfo.rcFrame.bottom;
				bmDecomp.biPlanes = 1;
				bmDecomp.biWidth = m_avinfo->m_VideoStreamInfo.rcFrame.right;
				bmDecomp.biSizeImage= bmDecomp.biWidth*bmDecomp.biHeight*3;
				m_avinfo->m_pGetFrame = AVIStreamGetFrameOpen(m_avinfo->m_pStreamVideo, &bmDecomp);
				m_IsDecompVideo = m_avinfo->m_pGetFrame != NULL;
			}
		}
	}
	else {
		if (!m_IsVideo)
			return -6;
	}
	size_t size = m_bmInfo->biSize + m_bmInfo->biClrUsed*sizeof(RGBQUAD);
	if (bm)
		memcpy(bm, m_bmInfo, std::min(sizeof(BITMAPINFOHEADER), size));
	return size;
}

/****************************************************************************
* Open audio stream and set its format
****************************************************************************/
bool CAviFile::SetFormat(WAVEFORMATEX *wf)
{
	if (!m_IsValid || !m_IsWrite) return false;
	// check stream
	if (m_avinfo->m_pStreamAudio)
		AVIStreamClose(m_avinfo->m_pStreamAudio);
	m_avinfo->m_pStreamAudio = 0;
	m_IsAudio = false;

	// Fill in the header for the video stream....
	memset(&m_avinfo->m_AudioStreamInfo, 0, sizeof(AVISTREAMINFO));
	m_avinfo->m_AudioStreamInfo.fccType                = streamtypeAUDIO;
	m_avinfo->m_AudioStreamInfo.dwScale                = wf->nBlockAlign;
	m_avinfo->m_AudioStreamInfo.dwRate                 = wf->nAvgBytesPerSec;
	m_avinfo->m_AudioStreamInfo.dwQuality              = -1;
	m_avinfo->m_AudioStreamInfo.dwSampleSize           = wf->nBlockAlign;

	// And create the stream;
	if (AVIFileCreateStream(m_avinfo->m_pAviFile, &m_avinfo->m_pStreamAudio, &m_avinfo->m_AudioStreamInfo) == AVIERR_OK) {
		int size = sizeof(WAVEFORMATEX) + (wf->wFormatTag==WAVE_FORMAT_PCM ? 0 :wf->cbSize);
		if (AVIStreamSetFormat(m_avinfo->m_pStreamAudio, 0, wf, size)==AVIERR_OK) {
			m_IsAudio = true;
			if (m_wFormat) free(m_wFormat);
			m_wFormat = (WAVEFORMATEX*)malloc(size);
			memcpy(m_wFormat, wf, size);
		}
	}
	return m_IsAudio;
}

/****************************************************************************
* Open stream and read format from audio stream if avi is readable
****************************************************************************/
int CAviFile::GetFormat(WAVEFORMATEX *wf)
{
	if (!m_IsValid) return -1;

	if (!m_IsWrite) {
		if (!m_IsAudio) {
			if (AVIFileGetStream(m_avinfo->m_pAviFile, &m_avinfo->m_pStreamAudio, streamtypeAUDIO, NULL)!=0)
				return -2;

			if (AVIStreamInfo(m_avinfo->m_pStreamAudio, &m_avinfo->m_AudioStreamInfo, sizeof(AVISTREAMINFO))!=0)
				return -3;

			long size = 0;
			if (AVIStreamReadFormat(m_avinfo->m_pStreamAudio, 0, 0, &size)!=0)
				return -4;

			if (m_wFormat) free(m_wFormat);
			m_wFormat = (WAVEFORMATEX*)malloc(size);
			if (AVIStreamReadFormat(m_avinfo->m_pStreamAudio, 0, m_wFormat, &size)!=0)
				return -5;

			m_IsAudio = true;
		}
	}
	else {
		if (!m_IsAudio)
			return -6;
	}
	int size = sizeof(WAVEFORMATEX) + (m_wFormat->wFormatTag==WAVE_FORMAT_PCM ? 0 : m_wFormat->cbSize);
	if (wf)
		memcpy(wf, m_wFormat, size);
	return size;
}

/****************************************************************************
* Write image
****************************************************************************/
bool CAviFile::WriteVideo(LPBYTE data, DWORD size, bool IsKey, unsigned long VideoInterval)
{
	if (!m_IsValid || !m_IsWrite || !m_IsVideo ) return false;

	if (m_VideoTime==0) {
		// first frame
		m_VideoFrameNum = 0;
		m_VideoTime+= 1000./m_fps;
	}
	else if (VideoInterval==-1) {
		// consecutive frames
		m_VideoFrameNum++;
		m_VideoTime+= 1000./m_fps;
	}
	else {
		// anisochronous consecutive frames
		//if (VideoInterval<(int)1000./m_fps)
		//	m_VideoTime += 1000./m_fps;
		//else
			m_VideoTime += VideoInterval;
		DWORD VideoFrameNum = (DWORD)(m_VideoTime*m_fps/1000.);
		if (VideoFrameNum<=m_VideoFrameNum)
			m_VideoFrameNum++;
		else
			m_VideoFrameNum = VideoFrameNum;
	}

	HRESULT res;
	if ( (res = AVIStreamWrite(m_avinfo->m_pStreamVideo, m_VideoFrameNum, 1, data, size,
		IsKey ? AVIIF_KEYFRAME : 0,	NULL, NULL))== 0)
		return true;
	else
		return false;
}

/****************************************************************************
* Write audio
****************************************************************************/
bool CAviFile::WriteAudio(LPBYTE data, DWORD size)
{
	if (!m_IsValid || !m_IsWrite || !m_IsAudio ) return false;

	m_AudioTime += 1000.*(double)size/m_avinfo->m_AudioStreamInfo.dwRate;
	long samples = size/m_avinfo->m_AudioStreamInfo.dwSampleSize;
	if (AVIStreamWrite(m_avinfo->m_pStreamAudio, m_AudioFrameNum, samples, data, size, 0, NULL, NULL)== 0) {
		m_AudioFrameNum+=samples;
		return true;
	}
	return false;

}

/****************************************************************************
* Try to read decompressed video farme
****************************************************************************/
int CAviFile::ReadDecompressedVideo(LPBYTE &data)
{
	data = 0;
	if (!m_IsValid || !m_IsDecompVideo )
		return -1;
	data = (BYTE*)AVIStreamGetFrame(m_avinfo->m_pGetFrame, m_VideoFrameNum);
	if (data) {
		m_VideoFrameNum++;
		BITMAPINFOHEADER *bm = (BITMAPINFOHEADER *)data;
		data+= bm->biSize+bm->biClrUsed*sizeof(RGBQUAD);
		return bm->biSizeImage;
	}
	else
		return -1;
}

/****************************************************************************
* read video frame
****************************************************************************/
int CAviFile::ReadVideo(LPBYTE data, DWORD size, bool* IsKey)
{
	if (!m_IsValid || !m_IsVideo) return -1;
	long bytes = 0;
	if (AVIStreamRead(m_avinfo->m_pStreamVideo, m_VideoFrameNum, 1, data, size, &bytes, 0)==0) {
		if (IsKey) {
			*IsKey = AVIStreamIsKeyFrame(m_avinfo->m_pStreamVideo, m_VideoFrameNum);
		}
		m_VideoFrameNum++;
		return bytes;
	}
	else return -1;
}

/****************************************************************************
* read audio frame(s)
****************************************************************************/
int CAviFile::ReadAudio(LPBYTE data, DWORD size, int samples)
{
	if (!m_IsValid || !m_IsAudio) return -1;
	long bytes = 0;
	if (AVIStreamRead(m_avinfo->m_pStreamAudio, m_AudioFrameNum, samples, data, size, &bytes, 0)==0) {
		m_AudioFrameNum+=samples;
		return bytes;
	}
	else return -1;
}

/****************************************************************************
* read current video frame time
****************************************************************************/
long CAviFile::GetVideoTime()
{
	if (m_IsValid && m_IsVideo)
		return AVIStreamSampleToTime(m_avinfo->m_pStreamVideo, m_VideoFrameNum);
	else return -1;
}

/****************************************************************************
* read current audio frame time
****************************************************************************/
long CAviFile::GetAudioTime()
{
	if (m_IsValid && m_IsAudio) {
		//long t1 = ((m_AudioFrameNum * m_avinfo->m_AudioStreamInfo.dwScale) * 1000 + (m_avinfo->m_AudioStreamInfo.dwRate >> 1)) /
		//		  m_avinfo->m_AudioStreamInfo.dwRate;
		long t1 = AVIStreamSampleToTime(m_avinfo->m_pStreamAudio, m_AudioFrameNum);
		return t1;
	}
	else return -1;
}

/****************************************************************************
* return total video time
****************************************************************************/
long CAviFile::GetDurationTime()
{
	if (m_IsValid)
		return (m_IsVideo) ? AVIStreamLengthTime(m_avinfo->m_pStreamVideo) : AVIStreamLengthTime(m_avinfo->m_pStreamAudio);
	else return -1;
}

/****************************************************************************
* Reset all frame positions
****************************************************************************/
void CAviFile::Reset()
{
	m_AudioFrameNum = 0;
	m_VideoFrameNum = 0;
}

/****************************************************************************
* Move to point position
****************************************************************************/
long CAviFile::MoveReadToTime(long tm, int mode)
{
	if (!m_IsValid)
		return -1;
	if (mode&1) {
		if (!m_IsAudio)
			return -1;
		//int denom = 1000 * m_avinfo->m_AudioStreamInfo.dwScale;
		//m_AudioFrameNum = (tm * m_avinfo->m_AudioStreamInfo.dwRate + (denom >> 1)) / denom;
		m_AudioFrameNum = AVIStreamTimeToSample(m_avinfo->m_pStreamAudio, tm);
	}
	if (mode&2) {
		if (!m_IsVideo)
			return -1;
		DWORD frame = AVIStreamTimeToSample(m_avinfo->m_pStreamVideo, tm);
		DWORD KeyFrame = AVIStreamNearestKeyFrame(m_avinfo->m_pStreamVideo, frame);
		if (KeyFrame >= m_VideoFrameNum || frame < m_VideoFrameNum)
			m_VideoFrameNum = KeyFrame;
	}
	return 0;
}

/****************************************************************************
* End CAviFile class
****************************************************************************/
