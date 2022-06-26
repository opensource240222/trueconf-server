/**
 **************************************************************************
 * \file VSThinkClient.h
 * (c) 2014 TrueConf
 * \brief Main Client module. Contain Receiver and Sender calsses
 * \b Project Client
 * \author Melechko Ivan
 * \author SMirnovK
 * \date 07.10.2002
 *
 ****************************************************************************/
#ifndef _VSTHINK_CLIENT_H_
#define _VSTHINK_CLIENT_H_

/****************************************************************************
 * Includes
 ****************************************************************************/
#include <windows.h>
#include "VSClientBase.h"
#include "VSCompress.h"
#include "VS_AviWrite.h"
#include "VS_PlayAvi.h"
#include "VSAudioUtil.h"
#include "VSRender.h"
#include "VSAudioNew.h"
#include "VS_StreamPacketManagment.h"
#include "SecureLib/VS_StreamCrypter.h"
#include "../std/cpplib/VS_RcvFunc.h"
#include "Transcoder/VS_VideoWindow.h"
#include "VS_SysBenchmarkWindows.h"
#include "VS_MediaFormatManager.h"
#include <queue>
#include "boost/shared_ptr.hpp"
#include "std/VS_TimeDeviation.h"
#include "Transcoder/VS_VoiceAnalyze.h"

/****************************************************************************
 * Declarations
 ****************************************************************************/
class CAudioCaptureList;
class VS_CaptureDevice;
class CCaptureSource;
class CDIBRender;
class CDirectXRender;
class CVideoRenderBase;
class CRenderAudio;
class CVideoCompressor;
class CVideoDecompressor;
class CCaptureAudioSource;
class CRenderAudioDevices;
class VS_StreamClientReceiver;
class VS_StreamClientSender;
class VS_MediaFormat;
class CVSTrClientProc;
class tc_VideoLevelCaps;
class TC_PhnxHid;

/****************************************************************************
 * Defines
 ****************************************************************************/
#define SEND_QUEUE_SIZE 201
#define DATA_TRACK 5

/**
 **************************************************************************
 * \brief received Server buffer statistics
 ****************************************************************************/
struct TServerStatistics
{
	int iBuffers;
	int iBuffersDiff;
	int iAudioBuffers;
	int iAudioBuffersDiff;
	DWORD packStat;
};
/**
 **************************************************************************
 * \brief Video Render parametrs struct
 ****************************************************************************/
struct TVideoWindowParams
{
	HWND hWindowHandle;
	RENDERPROC wndProc;
	LPVOID lpInstance;
};

struct TConferenceStatSnd
{
	int	started;				///< state of conference statistic
	int start_t;				///< conference start time
	int	now_t;					///< last time when bytes procesed to streams
	int last_t;					///< control time for calculate
	int bytes;					///< tortal bytes processed by streams
	int	media_traffic;			///< trafic media stream
	int video_w, video_h;		///< resolution of video
	int	num_frames;				///< number of frames
	int sum_cpu_load;			///< load cpu
	int inc_cpu;
	std::chrono::system_clock::time_point start_part_gmt;
	std::chrono::system_clock::time_point end_part_gmt;
	int GetPeriodTime() {return (now_t - start_t);}
	int GetPeriodCalc() {return (now_t - last_t);}
};

struct TConferenceStatRcv
{
	int	started;				///< state of conference statistic
	int start_t;				///< conference start time
	int	now_t;					///< last time when bytes procesed to streams
	int last_t;					///< control time for calculate
	int bytes;					///< total bytes processed by streams
	int bytes_cur[5];			///< current bytes for any type data
	int	media_traffic;			///< trafic media stream
	int avg_jitter;				///< average jitter
	int loss_rcv_packets;		///< loss packet in receiver
	int num_rcv;
	int GetPeriodTime() {return (now_t - start_t);}
	int GetPeriodCalc() {return (now_t - last_t);}
};

struct TConferenceStatistics;

/**
 **************************************************************************
 * \brief used to send stream data to external interfaces
 ****************************************************************************/

typedef int (__stdcall tTrackCallBack)(void*,int,int,void*,char*);
typedef int (__stdcall tExternalTrack)(void*,int*,int*,void*,char*);
class CTrackCallBack
{
private:
	tTrackCallBack *m_TrackCallBack;
	tExternalTrack *m_ExternalTrack;
	void * m_Param0;
	void * m_Param1;
public:
	CTrackCallBack(){m_TrackCallBack=NULL;m_ExternalTrack=NULL;}
	void SetCallBack(void*pCallBack,void*pParam){
		m_Param0=pParam;
		m_TrackCallBack=(tTrackCallBack *)pCallBack;
	};
	void SetExternalCallBack(void*pCallBack,void*pParam){
		m_Param1=pParam;
		m_ExternalTrack=(tExternalTrack *)pCallBack;
	};
	void SendTrack(int track,int iSize,void*pBuff,char* ID){
		if(m_TrackCallBack){
			(*m_TrackCallBack)(m_Param0,track,iSize,pBuff,ID);
		}
	};
	void ExternalTrack(int *pID,int *pSize,void*pBuff,char* ID){
		if(m_ExternalTrack){
			(*m_ExternalTrack)(m_Param1,pID,pSize,pBuff,ID);
		}
	};
};


extern CTrackCallBack TrackCallBack;

/**
 **************************************************************************
 * \brief Video sinc class, draw decompressed video data
 ****************************************************************************/
class CVideoSource
{
	static const int	VIDEO_FRAMES_MAX = 200;
	VS_TimeDeviation<int>	m_predictor;
	VS_TimeDeviation<int>	m_audiodurr;
	VS_BinBuff			m_CycleBuffer[VIDEO_FRAMES_MAX];
	int 				m_CycleTime[VIDEO_FRAMES_MAX];
	int					m_StartCount;
	int					m_EndCount;
	int					m_LastFillTime;
	int					m_LastFrameTime;
	int					m_PrevDurDurr;
	int					m_recordMBs;
public:
	HWND *				m_phwnd;			///< pointer to current drawn hwnd
	CVideoRenderBase *	m_pRender;			///< pointer to video render
	unsigned char *		m_pOut;				///< pointer to current drawn buffer
	CVideoDecompressor*	m_pVideo;			///< pointer to video decompressor
	VS_AudioRender *	m_pAudio;			///< pointer to current audio render (can be NULL)
	int					m_iBufferSize;		///< sizeof decompressed video frame
	char				m_CallId[MAX_PATH];

	CVideoSource();
	~CVideoSource();
	/// add video data buffer, calculate draw time
	bool AddData(unsigned char* data, int size, unsigned long VideoInterval, bool key, int recordMBs);
	/// check for queued video frames that will be displayed
	bool CheckDraw();
	/// get frame MBps from render
	int GetFrameSizeMB();
	/// get participant name
	char* GetPartName();
	/// clear receiver depended members
	void Detach();
};

#define MAX_VIDEOSOURCE 64
/**
 **************************************************************************
 * \brief Contain CVideoSource list. Use own thread to manage list members
 ****************************************************************************/
class CVideoSinc: public CVSThread, private VS_Lock{
private:

	DWORD Loop(LPVOID lpParameter);

	int				m_VideoSourceCounter;
	CVideoSource*	m_pVideoSource[MAX_VIDEOSOURCE];
	CVSTrClientProc *m_pProtocol;
public:
	void Go();
	void Stop();
	int AddVideoSource(CVideoSource* pVideoSource);
	void RemoveVideoSource(CVideoSource* pVideoSource);
	CVideoSinc(CVSTrClientProc *pProtocol);
	~CVideoSinc(){};
};
// enum of data frames priority

#define MAX_EVENT_SENDER 30
typedef int (*tEventSub)(void*,void*);

/**
 **************************************************************************
 * \brief Sender evens manager. contain events and call correspond functions
 ****************************************************************************/

struct EventState
{
	HANDLE		aEvent;
	tEventSub	eventSub;
	void		*eventParam;
	bool		bExclude;
public:
	EventState()
	{
		aEvent = 0;
		eventSub = 0;
		eventParam = 0;
		bExclude = false;
	}
};

class CEventManager
{
private:
	int			m_iCount;
	EventState	m_stEvent[MAX_EVENT_SENDER];
	int			m_idxEcxlude2Real[MAX_EVENT_SENDER];
	HANDLE		m_hMutex;
	void Remove(int iIndex);
public:
	CEventManager() : m_iCount(0) { m_hMutex = CreateMutex(NULL,false,NULL); memset(m_idxEcxlude2Real, 0, sizeof(int) * MAX_EVENT_SENDER); }
	~CEventManager() { CloseHandle(m_hMutex); }
	void Lock() { WaitForSingleObject(m_hMutex,5000); }
	void Unlock() { ReleaseMutex(m_hMutex); }
	void Add(HANDLE hEv,tEventSub pEvProc,void *pParam);
	void Remove(HANDLE hEv);
	void Remove(tEventSub pEvProc);
	int Process(int iIndex,void *pParam);
	int GetEvents(HANDLE *aEvents);
	void Exclude(HANDLE hEv, bool exclude);
	bool Update(tEventSub pEvProc, HANDLE hEv);
	void Clear() { m_iCount = 0; }
};

/**
 **************************************************************************
 * \brief Base class for Receiver and Sender. Contain common simple methods
 ****************************************************************************/
class CThinkClient: public CVSThread, public CVSInterface
{
protected:
	class CDeviceStatus
	{
	public:
		bool bUseVideo;
		bool bUseAudio;
		bool bVideoValid;
		bool bAudioValid;
		bool bVideoAvailable;
		bool bAudioAvailable;
		bool IsVideo(){return bUseVideo&&bVideoValid;}
		bool IsAudio(){return bUseAudio&&bAudioValid;}
	};
	const static char _funcFormat[];
	HWND						m_hReportHwnd;		///< window to send events(messages)
	VS_BinBuff					m_StoredKey;		///< store last key frame
	VS_StreamCrypter			m_StreamCrypter;	///< crypt sreams by conference key
public:
	CDeviceStatus				m_DeviceStatus;		///< audio and video device states

	static HANDLE				m_hSendKeyReq;		///< event for SendKeyReq()
	static TServerStatistics *	m_stat;				///< stat from server (updated throw receiver)

protected:
	/// Resest Conference stat. Call before conference begin
	virtual void RstConfStat() {};
	/// Set processed bytes and time of it
	virtual void SetConfStat(int now_t, int bytes = 0, int type = 0) {};
	/// Ends conf stat collection
	virtual void EndConfStat() {};
	/// Request opposite side to sent key frame
	void SendKeyReq(){SetEvent(m_hSendKeyReq);};
public:
	void SetNotifyWnd(HWND wnd) {m_hReportHwnd = wnd;}
	/// Reset vars
	CThinkClient(const char *szName,CVSInterface* pParentInterface);
	/// Get conferense sent bytes and durration
	virtual bool GetConfStat(int &bytes, int &time) {bytes = 0; time = 0; return false;}
	/// Get Current Traffic
	virtual int GetTraffic(){return 0;}
	/// return pointer to key-frame buffer
	VS_BinBuff * GetStoredKey() {return &m_StoredKey;}
};


/**
 **************************************************************************
 * \brief Sender class. Manage video-audio capturing, and send confernce
 * data in conference mode.
 ****************************************************************************/
class CThinkClientSender: public CThinkClient, VS_Lock
{
	class CConnectStatus{
	public:
		bool			bNeedKeyFrame;
		bool			bWaitKeyFrame;
		bool			bWaitCompress;
		bool			UseNhp;
		bool			HalfVideo;
		bool			bUpdateBitrate;
		bool			bSkipNextFrame;
		bool			bGrabFrame;
		int				iBitrate;
		int				iBaseBitrate;
		int				iSwitchBitrate;
		int				iFPS;
		int				iRealFPS;
		int				iSkipAudio;
		int				iNumDropFrames;
		unsigned int	LastSendVideo;
		unsigned int	iVideoFrameNum;
		unsigned int	HalVideoTime;
		unsigned int	SwitchVModeTime;
		std::queue <unsigned int> QueueVideoTimestamps;
		CConnectStatus() {Clear();}
		void Clear();
		void DropVideoFrame();
	};
private:
	const static char _funcEnabledDevices[];
	const static char _funcVideoRender[];
	const static char _funcBandwidth[];
	const static char _funcFPSRate[];
	const static char _funcQueryKey[];
	const static char _funcAddVC[];
	const static char _funcAddVCExt[];
	const static char _funcAddACExt[];
	const static char _funcRemoveVC[];
	const static char _funcAddAC[];
	const static char _funcRemoveAC[];
	const static char _funcSetStretch[];
	/**********************/
	int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;
	DWORD Loop(LPVOID lpParameter);

	HANDLE					m_hEventCmd;		///< event for InfoMediaFormatProcess()
	HANDLE					m_hEventFirstSnd;	///< event for first send
	MMRESULT				m_timerCmd;			/// id timer for InfoMediaFormatProcess(...)
	CEventManager			m_EventManager;		///< meneger, call appropriate functions for input events
	CConnectStatus			m_ConnectStatus;	///< connect status varibles class

	VS_ControlPacketQueue*	m_pPacketQueueContr;///<
	VS_ControlBandBase*		m_BandContr;		///< bitrate controller
	VS_SendFrameQueueBase*	m_FrameQueue;		///< send queue
	CVideoCompressor *		m_pVideoCompressor;	///< videocompressor

	boost::shared_ptr<VS_StreamClientSender>	m_pSender;			///< Stream sender

	CVSTrClientProc*		m_pProtocol;		///< pointer to main protocol clas
	CVideoCaptureList *		m_pVCaptureList;	///< list of video devices and it modes
	CAudioCaptureList *		m_pACaptureList;	///< list of audio devices and it modes
	unsigned char*			m_ExtFrameMem;
	VS_SysBenchmarkWindows*	m_pSysBench;
	VS_MediaFormatManager * m_pMediaFormatManager;
	tc_VideoLevelCaps *		m_pLevelCaps;
	TC_PhnxHid*				m_PhoenixDev;

	bool					m_Connected;		///< true if Connect() called
	volatile bool			m_MediaConnected;	///< true if SetCurrentMedia() called
	bool					m_isIntercom;
	DWORD					m_LastSendTime;		///< last time when data was send
	DWORD					m_KeyRepeatTime;	///< last time when key frame was generated
	DWORD					m_KeyRepeatPeriod;	///< maximum time between keyframes
	DWORD					m_PrevVideoTime;	///< last time of videoframe
	int						m_FPSvsBr;			///< preferable "Bitrate vs FPS" koef
	int 					m_CurrFPSvsBr;		///< current "Bitrate vs FPS" koef (may change in conf by opposite side)
	int						m_PrevBandwith;		///< previous Bandwith value
	int 					m_BrLoadFactor;		///< distortion factor of broker buffer load value
	int 					m_FPSFixMax;		///< distortion factor of broker buffer load value
	int 					m_BrAutoDisabled;	///< disable bitrate optimisation
	int 					m_iMaxFPS;			///< maximum captured FPS from vodeo device in conference
	int 					m_iMinBitrate;		///< minimum bitrate for codec relative to 10 codec fps
	int 					m_iBandwidth;		///< preferable outcoming bitrate
	int						m_iOtherSideBand;	///< restriction to outcoming bitrate from opposite side (if exist)
	int						m_iServerBand;		///< restriction to outcoming bitrate from server
	int						m_iServerBandVideo;	///< restriction to outcoming bitrate for video from server
	int						m_MaxGConfBand;		///< restriction to outcoming bitrate for group conferences
	VS_MediaFormat			mf_current;			///< current sent mediaformt
	VS_MediaFormat			m_vmf;				///< initail (preferable) mediaformat
	VS_MediaFormat			m_mf_notify;
	TConferenceStatSnd		m_conf_stat;
	int						m_scheme_bitrate;
	int						m_last_quality;
	char					m_CallId[MAX_PATH];
	bool					m_bAllowDynChange;
	int						m_TypeOfAutoPodium;

	// send frame processing
	/// add frame to queue and split it in case of video frame, set m_hEvent event
	int AddFrameToQueue(int track,int Size,unsigned char *pBuffer,bool bLimitedFrame,int iPriority);
	/// send first packet in queue, in case of sucsess call external callback
	int ProcessSendQueue();

	// called from sender thread
	/// return -1 to exit from Loop()
	static int DieProcess(void*pParam,void*pParam1);
	/// read data from audio device, sent it to queue if connected
	static int AudioProcess(void*pParam,void*pParam1);
	/// add to queue 1 byte of data to produce key frame on opposite side
	static int QueryKeyProcess(void*pParam,void*pParam1);
	/// call ProcessSendQueue(), if video is sent do dynamic bitrate optimazation
	static int QueueProcess(void*pParam,void*pParam1);
	/// call ProcessSendQueue() first time
	static int QueueFirstProcess(void*pParam,void*pParam1);
	/// read compressed video to response on compressor event and add it to queue
	static int VideoCompressProcess(void*pParam,void*pParam1);
	/// read uncompressed video to response on videocapture event and send it to compressor
	static int VideoFrameProcess(void*pParam,void*pParam1);
	/// process obtained by receiver commands
	static int ReceivedCommandProcess(void*pParam, void*pParam1);
	/// reset video if need
	static int CheckVideoModeProcess(void*pParam, void*pParam1);
	/// connect device if need
	static int CheckConnectDeviceProcess(void*pParam, void*pParam1);
	/// disconnect device if need
	static int CheckDisconnectDeviceProcess(void*pParam, void*pParam1);
	/// send info media format command
	static int InfoMediaFormatProcess(void*pParam, void*pParam1);
	/// restrict media format from server
	static int CheckRestrictMediaFormat(void*pParam, void*pParam1);


	// other internal functions
	/// call external callback function to compressd and add external data to queue
	void QueryExternalFrame();
	///return current max out bitrate
	unsigned long CurrentBandwidth();
	/// recalculate bitrate of viceocodec and fps of video device
	void CalcBitrate(int OldBand, int BaseBand, int *pFPS);
	/// send zero-lehgth buffer to keep connection alive
	void Ping(DWORD CurrTime);
	///
	void SetConfStat(int now_t, int bytes = 0, int type = 0);
	///
	void RstConfStat();
	///
	void EndConfStat();
	///
	void NotifyResolution(VS_MediaFormat *mf);
	///
	void CheckSwitchVideoMode(int bitrate, unsigned int ctime);
	///
	void SendInfoMediaFormat(VS_MediaFormat *mf = NULL);
	///
	void AnalyseQueueVideoFrame();
	///
	bool GetBlockVideoCapture(uint32_t timestamp);
public:
	/// craete events, classes, read params from registry
	CThinkClientSender(CVSInterface *pParentInterface, CVSTrClientProc* protocol);
	/// save params to registry, delete resoureces
	~CThinkClientSender();
	/// store preferable mediaformat, fill event manager, start Loop() in sender thread
	int Init(VS_MediaFormat &mf);
	/// stop sender thread
	void Release();
	/// set inital conf params, free queue, set initial bitrate and fps
	void Connect(const boost::shared_ptr<VS_StreamClientSender> &pTransport, HWND hReportHwnd, int Type, int Bandwidth, bool isIntercom);
	/// set current sending Media Format. Must be called after Connect().
	/// connect videocompressor and start audio capture devices whith mf_current.
	void SetCurrentMedia(VS_MediaFormat &mf);
	/// disconnect videocompressor, stop audio capture, Reset video capture FPS to default,
	/// restore current Media Format to preferable
	void ReleaseCurrentMedia();
	/// change media format, video or/and audio if need
	void ChangeCurrentMedia(VS_MediaFormat &mf, eDeviceAction act);
	/// change media format, video or/and audio if need
	void ChangeCurrentMediaCommand(VS_MediaFormat &mf, eDeviceAction act);
	/// reset video
	void ChangeVideoMode();
	/// restrict mediqa format from server
	void RestrictMediaFormat();
	/// connect device state
	void ConnectVideoDevice();
	/// disconnect device state
	void DisconnectVideoDevice();
	/// ReleaseCurrentMedia, close stream
	void Disconnect();
	/// create video capture device and init it with preferable mediaformat
	int AddVideoCapture(char *szSlotName, TVideoWindowParams *pVParams, bool bExt=false);
	/// delte video capture device
	int RemoveVideoCapture(char *szName);
	/// create audio capture device and init it with preferable mediaformat
	int AddAudioCapture(char *szSlotName, bool bExt=false);
	/// delte audio capture device
	int RemoveAudioCapture(char *szName);
	/// change current fps-quality factor, return new value
	unsigned long SetCurrentFpsVsQ(unsigned long val);
	///
	bool GetConfStat(TConferenceStatistics *cst);
	///
	int GetTraffic() {
		return m_conf_stat.media_traffic;
	}
};

/**
 **************************************************************************
 * \brief Receiver class. Created in conferense. Used to receive data manage it
 * conference data from one participant.
 ****************************************************************************/
class CThinkClientReceiver: public CThinkClient, VS_Lock, VS_MiscCommandProcess
{
private:
	CVideoDecompressor *		m_pVideoDecompressor;
	VS_AudioRender *			m_pRenderAudio;
	CVideoRenderBase *			m_pRender;
	DWORD *						m_ppRender;
	boost::shared_ptr<VS_StreamClientReceiver>	m_pReceiver;
	VS_NhpBuffBase *			m_NhpBuff;
	CVideoSinc *				m_pSinc;
	CVideoSource				m_VideoSource;
	VS_VS_InputBuffer			m_vs_In;
	VS_MediaFormat				m_mf;
	VS_MediaFormat				m_mfNotify;
	VS_VoiceAnalyze				m_Van;
    HWND						m_hwnd;
	bool						m_bNhp;
	bool						m_bIntercom;
	bool						m_IsInited;
	unsigned char *				m_pRcvTmp;
	unsigned char *				m_pVRenderBuffer;
	DWORD						m_PrevVideoTime;
	int							m_iAudioDevice;
	int							m_iImageSize;
	long						m_RcvFltr;
	TConferenceStatRcv			m_conf_stat;
	char						m_CallId[MAX_PATH];
	unsigned long				m_capsFourcc;
	bool						m_bWaitKeyChangeMF;

	void SetConfStat(int now_t, int bytes = 0, int type = 0);
	void RstConfStat();
	void EndConfStat();
	DWORD Loop(LPVOID lpParameter);
	int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;
	int CheckMediaFormat(unsigned char *pSource, bool bKey, unsigned int size);
	void NotifyResolution(VS_MediaFormat &mf);
public:
	const static char			_funcResetWindow[];
	const static char			_funcSetRsvFltr[];

	CThinkClientReceiver(char *szName, CVSInterface *pParentInterface, CVideoSinc*pVideoSinc);
	~CThinkClientReceiver();
	int  PrepareAudio(int iDevice);
	int  PrepareVideo(HWND hRenderView,RENDERPROC *pwndpSelfView,LPVOID *lpSelfView);
	int  Init(VS_MediaFormat &mf, unsigned long capsFourcc);
	int  ChangeCurrentMedia(VS_MediaFormat &mf, bool resetQueue);
	void Release();
	void Connect(const boost::shared_ptr<VS_StreamClientReceiver> &pTransport, HWND hReportHwnd, long fltr, VS_BinBuff &simkey, int ConfType);
	void Disconnect();
	bool SetReceivedCommand(stream::Command& cmd);
	void SetRcvFltr(long fltr);
	void ResetWindow(HWND hwnd);
	int  ReceiveTCPData(unsigned char* buff, int size, stream::Track track);
	int  ReceiveNHPData(unsigned char* buff, int size);
	bool GetConfStat(TConferenceStatistics *cst);
	int GetTraffic() {
		return m_conf_stat.media_traffic;
	}
	void GetCurrentMediaFormat(VS_MediaFormat& mf) {
		mf = m_mf;
	}
	unsigned long GetVol(unsigned long currTime){ return m_Van.Get(currTime); }
	const char * GetCallID(){ return m_CallId; }
	void RemoveAudio(int iDevice);
	void SetChangeAudioDevice(int iDevice);
	// return quality of rcv channel, 6 - excellent, 1 - poor
	int GetQuality();
	void EnableBorders(bool enable);
	void SetBordersAlpha(uint8_t alpha);
};

/**
 **************************************************************************
 * \brief Receiver pool. Usaed to manage many reseivers, for example in
 * multi-party conferences
 ****************************************************************************/
class CReceiversPool: public CVSInterface{
public:
	/*-----------------------------------*/
	CVideoSinc *			m_pVideoSinc;
	CRenderAudioDevices *	m_pRenderDevices;
	VS_PlayAviFile			m_PlayAvi;
	VS_WindowGrid			m_lgrid;
	/*-----------------------------------*/
private:
	enum EBordersMode : DWORD
	{
		BM_NONE,
		BM_ALL,
		BM_LOUDEST
	} m_BordersMode = BM_NONE;

	/************************/
	const static char _funcConnectReceiver[];
	const static char _funcDisconnectReceiver[];
	const static char _funcGetReceivedData[];
	const static char _funcParticipantStatus[];
	unsigned long			m_UseVAD;
	unsigned long			m_LayoutPriorityType;
	/************************/
	TConferenceStatRcv m_stat_rcv;
	CVSTrClientProc *m_pProtocol;
	wchar_t szPlaybackDevice[MAX_PATH];
	CThinkClientReceiver *m_pReceivers[MAX_VIDEOSOURCE];
	int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;

public:
	CReceiversPool(CVSInterface *pParentInterface,CVSTrClientProc *pProtocol);
	~CReceiversPool();
	void GetConfStat(TConferenceStatistics *cst);
	void RstConfStat();
	int GetQualityBitrate();
	int GetQuality();
};

#endif
