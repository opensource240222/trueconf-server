/****************************************************************************
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
*
* Project: Video
*
* $History: CAviFile.h $
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 5.08.09    Time: 19:48
 * Updated in $/VSNA/video/winapi
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
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 22.08.07   Time: 16:06
 * Updated in $/VS2005/Video/winapi
 * - speed up for avi playing in case of no video auto decompress
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
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 24.01.07   Time: 19:41
 * Updated in $/VS/Video/WinApi
 * - play avi module
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 6.07.04    Time: 20:58
 * Updated in $/VS/Video/WinApi
 * correct size of format info
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 28.06.04   Time: 17:46
 * Updated in $/VS/Video/WinApi
 * addded time support for video
 * added set-get media format support
 *
 * *****************  Version 5  *****************
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

#ifndef CAVI_FILE_H
#define CAVI_FILE_H

/****************************************************************************
* Includes
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include <memory.h>
#include <mmsystem.h>

/****************************************************************************
* Structures
****************************************************************************/
struct VS_AviInfo;

/****************************************************************************
* Classes
****************************************************************************/
/****************************************************************************
* CAviFile class
****************************************************************************/
class CAviFile
{
	HRESULT				hr;
	bool				m_IsWrite;
	bool				m_IsValid;
	bool				m_IsAudio;
	bool				m_IsVideo;
	bool				m_IsDecompVideo;
	bool				m_initDecompress;
	VS_AviInfo*			m_avinfo;
public:
	BITMAPINFOHEADER	*m_bmInfo;
	WAVEFORMATEX		*m_wFormat;
	DWORD				m_VideoFrameNum;
	DWORD				m_AudioFrameNum;
	double				m_VideoTime;
	double				m_AudioTime;
	double				m_fps;

	CAviFile();
	~CAviFile();
	bool Init(const char* Name, bool write = false, bool initDecompress = true);
	bool Init(const wchar_t* Name, bool write = false, bool initDecompress = true);
	void Release();

	bool SetFormat(BITMAPINFOHEADER *bm);
	int GetFormat(BITMAPINFOHEADER *bm);
	bool SetFormat(WAVEFORMATEX *wf);
	int GetFormat(WAVEFORMATEX *wf);
	bool IsVideo() {return m_IsVideo;}
	bool IsDecompressVideo() {return m_IsDecompVideo;}
	bool IsAudio() {return m_IsAudio;}
	bool WriteVideo(LPBYTE data, DWORD size, bool IsKey = true, unsigned long VideoInterval = -1);
	bool WriteAudio(LPBYTE data, DWORD size);
	int  ReadVideo(LPBYTE data, DWORD size, bool* IsKey = 0);
	int  ReadAudio(LPBYTE data, DWORD size, int samples = 1);
	int  ReadDecompressedVideo(LPBYTE &data);
	long GetVideoTime();
	long GetAudioTime();
	long GetDurationTime();

	void Reset();
	long MoveReadToTime(long tm, int mode = 3);
};

#undef UNICODE

#endif
