

#include "../Video/WinApi/CAviFile.h"
#include "../std/cpplib/VS_MediaFormat.h"
#include "../std/cpplib/VS_SimpleStr.h"
#include "../std/cpplib/VS_Lock.h"
#include "../Transcoder/AudioCodec.h"
#include "../Transcoder/VideoCodec.h"
#include "../Transcoder/VS_AudioReSampler.h"
#include "../Transcoder/AudioCodec.h"
#include "../VSClient/VSClientBase.h"
#include "../VSClient/VSCompress.h"
#include "../Video/VSVideoProc.h"

#define WORK_MODE_VIDEOBOT				(0)
#define WORK_MODE_DEMOGROUP				(1)
#define WORK_MODE_DEMOGROUPCOMPRESS		(2)
#define WORK_MODE_DEMOGROUPCOMPRESS_SVC	(3)

extern unsigned int g_TimeSay;
extern unsigned int g_TimeRec;
extern unsigned int g_TimePlayrec;

class VideoFile: public VS_Lock
{
public:
	enum State
	{
		VF_NONE,
		VF_SAY,
		VF_REC,
		VF_PLAYREC,
		VF_READYLOOP,
		VF_PLAYLOOP,
		VF_ENDCALL
	};
private:
	State				m_state;
	CAviFile			m_inavi, m_outavi;
	VS_MediaFormat		m_sndfmt, m_rcvfmt, m_readfmt;
	VS_SimpleStr		m_srcfile, m_recfile;
	VS_AudioReSampler	m_srcrsmp;
	AudioCodec*			m_RecAcodec;
	AudioCodec*			m_PlayAcodec;
	CVideoDecompressor	m_PlayVcodec;
	unsigned char		*m_vBuff, *m_aBuff, *m_tmp1, *m_tmp2;
	unsigned long		m_read_time, m_start_time;
	VS_VideoProc		m_proc;
	bool				m_waitKey;
	int					m_mode;
public:
	VideoFile();
	~VideoFile();
	bool Init(char* filename, char *user, int mode);
	bool SetSndFormat(VS_MediaFormat &fmt);
	bool SetRcvFormat(VS_MediaFormat &fmt);
	long WriteAudio(unsigned char *buff, unsigned long size);
	long WriteVideo(unsigned char *buff, unsigned long size, bool key, unsigned long VideoInterval);
	long ReadAudio(unsigned char *buff, unsigned long &size);
	long ReadVideo(unsigned char *buff, unsigned long &size);
	State CheckState();
	long GetSndFrameRate();
	void Reset();
private:
	bool PrepareForRead();
};