/**
 **************************************************************************
 * \file VS_PlayAvi.h
 * (c) 2002-2007 Visicron Systems, Inc. All rights reserved.
 *									http://www.visicron.net/
 * \brief Play avi files module
 * \b Project Client
 * \author SMirnovK
 * \date 22.01.2007
 *
 * $Revision: 6 $
 *
 * $History: VS_PlayAvi.h $
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 25.08.11   Time: 12:42
 * Updated in $/VSNA/VSClient
 * - fix video render for avi player
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 10.09.10   Time: 18:28
 * Updated in $/VSNA/VSClient
 * - fix AviPlayer for 8bit PCM
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 5.08.09    Time: 19:48
 * Updated in $/VSNA/VSClient
 * - modify avi player: support system videocodecs, support play only
 * audio
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 4.08.09    Time: 20:28
 * Updated in $/VSNA/VSClient
 * - fix DS for avi player
 * - add AudioCodecSystem class, change acmDriverEnumCallback function
 * - avi player support system audio codecs
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 24.11.08   Time: 13:23
 * Updated in $/VSNA/VSClient
 * - were added recoding for group confrrence (not enable)
 * - were added new mixer module
 * - were modified CAviFile (add support synch MS GSM 6.10 in play/read
 * mode)
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 5.07.07    Time: 14:03
 * Updated in $/VS2005/VSClient
 * - locked avi playing from SetPosition
 * - errors in self avi writing fixed
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 9.06.07    Time: 20:32
 * Updated in $/VS2005/VSClient
 * - chanded return parameters from aviPlayInit
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 7.06.07    Time: 19:05
 * Updated in $/VS2005/VSClient
 * - set/get position in avi (playing mode)
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
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
#include "../Transcoder/VSVideoFileReader.h"
#include "VSRender.h"
#include "VSAudioNew.h"
#include "VSCompress.h"
#include "../Transcoder/AudioCodec.h"

/**
 **************************************************************************
 * \brief Play AVI whith PCM audio and Cyclone video
 ****************************************************************************/
class VS_PlayAviFile: public CVSThread, VS_Lock
{
	VSVideoFileReader	m_avi;
	VS_PCMAudioRender	m_aRender;
	CVideoDecompressor	m_decoder;
	CVideoRenderBase*	m_vRender;
	DWORD				*m_ppRender;
	AudioCodec			*m_acodec;
	HWND				m_whwnd;
	HWND				m_nhwnd;
	BYTE*				m_vframe;
	int					m_timeshift;
	int					m_smpl_read;
	long				m_Paused;
	long				m_Duration;
	int					m_is_pcm;
	bool				m_is_video;
	long				m_LastSeekTime;
	long				m_NewSeekTime;
public:
	/// variables ser to zero
	VS_PlayAviFile();
	/// call Release()
	~VS_PlayAviFile();
	/// Open avi-file, init internal classes
	int Init(wchar_t * filename, HWND videoWindow, HWND notifyWindow, int dev);
	/// Release internal classes, close avi
	void Release();
	/// start playing thread from avi begining
	bool Start();
	/// stop playing thread
	bool Stop();
	/// pause playing thread
	bool Pause();
	/// Playing thread
	DWORD Loop(LPVOID lpParameter);
	/// set position to start playing (in msec)
	void SetPosition(long tm);
	/// Get current position info
	long GetPosition(int type);
	/// for testing
	void SetTimeShift(int timeshift) {m_timeshift = timeshift;}
	/// return video render adress
	long GetVRender() {return (long )m_ppRender;}
private:
	/// Update position after seeking
	void UpdateSeekPosition(unsigned char *tmp);
};