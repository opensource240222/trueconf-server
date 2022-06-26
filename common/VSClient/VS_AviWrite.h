/**
 **************************************************************************
 * \file VS_AviWrite.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Class to write visicron stream data (video, audio) in avi
 *
 * \b Project Client
 * \author SMirnovK
 * \date 30.06.2004
 *
 * $Revision: 10 $
 *
 * $History: VS_AviWrite.h $
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 5.07.12    Time: 15:02
 * Updated in $/VSNA/vsclient
 * - fix avi writer : change media format stream
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 4.07.12    Time: 19:47
 * Updated in $/VSNA/VSClient
 * - fix multi mixer for dynamic change format
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 3.10.11    Time: 18:14
 * Updated in $/VSNA/VSClient
 * - ench AviWriter : new presets, blending display name, h.263 -> vp8
 * - refactoring AviWriter
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 29.04.11   Time: 17:09
 * Updated in $/VSNA/VSClient
 * - fix AviWriter : stream sunch
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 27.04.11   Time: 19:14
 * Updated in $/VSNA/VSClient
 * - were added auto change media format
 * - were added info media format command
 * - wait time reduced to 1000 ms in EventManager
 * - were added new capability : dynamic change media format
 * - capture : unblock SetFps and GetFrame
 * - receivers can dynamic change media format
 * - were added auto check media format from bitstream in receivers
 * - change scheme BtrVsFPS for vpx
 * - change AviWriter for auto change media format
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 16.07.10   Time: 16:41
 * Updated in $/VSNA/VSClient
 * - were add rename user for AviWriter
 * - code clean AviWriter
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 9.07.10    Time: 15:07
 * Updated in $/VSNA/VSClient
 * - AviWriter: "Close" were close all active stream
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 8.05.09    Time: 12:08
 * Updated in $/VSNA/VSClient
 * - were modifed new version AviWriter
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
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 22.08.07   Time: 16:42
 * Updated in $/VS2005/VSClient
 * - stored key frame for recording myself
 * - fix bug with very old stored key-frame
 * - sinc voice in both audio channels when recording myself
 * - fix bug with birtrate in recording myself out of conference
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 20.08.07   Time: 19:10
 * Updated in $/VS2005/VSClient
 * - bugfix #2825
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 26.12.06   Time: 20:03
 * Updated in $/VS/VSClient
 * - corrected avi write to work state
 * - added key-frame request while strarting avi write
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 25.12.06   Time: 13:46
 * Updated in $/VS/VSClient
 * - added internal interface for AviWrite module
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 14.03.05   Time: 16:39
 * Updated in $/VS/VSClient
 * new codecs support in avi writing
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 *
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 24.08.04   Time: 19:53
 * Updated in $/VS/VSClient
 * in GateWay Client added audion spliting onto 195 Byte
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 5.07.04    Time: 14:39
 * Updated in $/VS/VSClient
 * new sinc shema
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 1.07.04    Time: 20:10
 * Updated in $/VS/VSClient
 * audi - video sinc
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 30.06.04   Time: 19:49
 * Updated in $/VS/VSClient
 * audio - video synchronization in normal case
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 30.06.04   Time: 18:04
 * Created in $/VS/VSClient
 * added sender and reciever audio support
*
****************************************************************************/
#ifndef VS_AVI_WRITE_H
#define VS_AVI_WRITE_H


/****************************************************************************
* Includes
****************************************************************************/
#include "../std/cpplib/VS_MediaFormat.h"
#include "../Transcoder/VSVideoFileWriter.h"
#include "../std/cpplib/VS_Lock.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../Transcoder/VS_VS_Buffers.h"
#include "../Transcoder/AudioCodec.h"
#include "../Transcoder/VideoCodec.h"
#include "../Transcoder/VS_BitStreamBuff.h"
#include "../Transcoder/VS_AudioReSampler.h"
#include "VSClientBase.h"
#include <map>
#include <string>
//#include "Transcoder/VS_MultiMixerWriter.h"

#define LENGHT_NAME_STREAM (260)

struct VS_StreamStateW
{
	VS_MediaFormat	mf;
	bool			is_init;
};

typedef std::pair<std::string, VS_StreamStateW>					write_pair;
typedef std::map <std::string, VS_StreamStateW>::iterator		write_iter;

class VS_AviWriteGroup:public CVSThread, public VS_Lock, public CVSInterface
{
	VSVideoFileWriter				m_avi;
	std::map<std::string, VS_StreamStateW>	m_wstream_state;
	//VS_MultiMixerWriter				*m_mixer;
	VideoCodec						*m_vcodec;
	AudioCodec						*m_acodec;
	unsigned char					*m_imgLogo;
	VS_MediaFormat					m_mf;
	bool							m_is_valid, m_is_started, m_is_pcm;
	int								m_num_streams;
	int								m_vbitrate;
	int								m_vframerate;
	int								m_logoW, m_logoH;
	std::atomic<int>				m_mixer_mode;
	VS_SimpleStr					m_LastPStream;
	void TryOpenLogo();
	void DrawLogo(unsigned char *video);
public:
	VS_AviWriteGroup(CVSInterface *pParentInterface);
	~VS_AviWriteGroup();
	bool Init(wchar_t *file_name);
	void CreateStream(char *handle, VS_MediaFormat *mf);
	void ReleaseStream(char *handle);
	int  OpenStream(char *handle, char *stream_name = NULL);
	void CloseStream(char *handle);
	int  ChangeStream(char *handle, char *stream_name = NULL);
	int  ChangeMediaFormatStream(char *handle, VS_MediaFormat *mf);
	int	 GetInitBitrateMode() { return m_vbitrate; }
	int	 GetInitFramerateMode() { return m_vframerate; }
	int  GetRecordMB(char *handle);
	void Close();
	void Release();
	bool Start();
	bool Pause();
	DWORD Loop(LPVOID lpParameter);
	void PutAudio(char *callId, unsigned char *data, int size, double frequency, int cur_abuff_durr = -1);
	void PutVideo(char *callId, unsigned char *data, int size, int cur_abuff_durr);
	void SetPriority(const char *callid);
	void SetMixerMode(int sndMBps);
	int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;
};

extern VS_AviWriteGroup *g_AviWriterGroup;

#endif
/****************************************************************************
* End of file
****************************************************************************/
