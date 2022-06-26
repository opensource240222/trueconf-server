/**
 **************************************************************************
 * \file VSThinkClient.cpp
 * (c) 2014 TrueConf
 * \brief Implement Receiver and Sender classes
 * \b Project Client
 * \author Melechko Ivan
 * \author SMirnovK
 * \date 07.10.2002
 *
****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/

#include "VSCapture.h"
#include "VSRender.h"
#include "VSCompress.h"
#include "VSAudio.h"
#include "VSThinkClient.h"
#include "VS_ApplicationInfo.h"
#include "VSTrClientProc.h"
#include "VSVideoCaptureList.h"
#include "VSAudioCaptureList.h"
#include "VSAudioCaptureSlot.h"
#include "VSVideoCaptureSlot.h"
#include "VS_Dmodule.h"
#include "VS_MiscCommandProc.h"
#include "VS_StreamPacketManagment.h"
#include "VS_NhpHeaders.h"
#include "../streams/Client/VS_StreamClientReceiver.h"
#include "../streams/Client/VS_StreamClientSender.h"
#include "../std/cpplib/VS_MediaFormat.h"
#include "../std/cpplib/VS_VideoLevelCaps.h"
#include "../std/cpplib/VS_IntConv.h"
#include "std-generic/cpplib/ignore.h"
#include "std-generic/clib/rangecd.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "..\std\cpplib\json\elements.h"
#include "../std/cpplib/json/writer.h"
#include "../std/cpplib/json/reader.h"
#include "../std/cpplib/VS_PerformanceMonitor.h"
#include "../MediaParserLib/VS_VPXParser.h"
#include "../MediaParserLib/VS_XCCParser.h"
#include "../MediaParserLib/VS_H265Parser.h"
#include "../MediaParserLib/VS_H264Parser.h"
#include "../MediaParserLib/VS_H263Parser.h"
#include "../streams/Protocol.h"
#include "Hidapi/HidDevice.h"
#include <math.h>
#include "Transcoder/GetTypeHardwareCodec.h"

#ifdef VZOCHAT7
#include "../AddressBookCache/QGlRenderHolder.h"
#include "../AddressBookCache/QGlRender.h"
#include "../AddressBookCache/QDxRender.h"
#endif

/****************************************************************************
 * Static
 ****************************************************************************/
const char _BandwidthName[]="Bandwidth";
const char _FPSRateName[]="FPSRate";

HANDLE CThinkClient::m_hSendKeyReq=0;

TServerStatistics *CThinkClient::m_stat=NULL;

const char CThinkClientSender::_funcEnabledDevices[]="EnabledDevices";
const char CThinkClientSender::_funcBandwidth[]="Bandwidth";
const char CThinkClientSender::_funcFPSRate[]="FPSRate";
const char CThinkClientSender::_funcQueryKey[]="QueryKey";
const char CThinkClientSender::_funcRemoveVC[]="RemoveVideoCapture";
const char CThinkClientSender::_funcAddVC[]="AddVideoCapture";
const char CThinkClientSender::_funcAddVCExt[]="AddVideoCaptureExt";
const char CThinkClientSender::_funcAddACExt[]="AddAudioCaptureExt";
const char CThinkClientSender::_funcRemoveAC[]="RemoveAudioCapture";
const char CThinkClientSender::_funcAddAC[]="AddAudioCapture";
const char CThinkClientSender::_funcSetStretch[]="SetStretch";
const char CThinkClientReceiver::_funcResetWindow[]="ResetWindow";
const char CThinkClientReceiver::_funcSetRsvFltr[]="SetRsvFltr";


const char CThinkClient::_funcFormat[]="Format";

const char CReceiversPool::_funcConnectReceiver[]="ConnectReceiver";
const char CReceiversPool::_funcDisconnectReceiver[]="DisconnectReceiver";
const char CReceiversPool::_funcGetReceivedData[]="GetReceivedData";
const char CReceiversPool::_funcParticipantStatus[]="ParticipantStatus";

/****************************************************************************
 * Defines
 ****************************************************************************/
#define DEFAULT_DEVICE 0
#define DEFAULT_AUDIO_DEVICE 1
#define DEFAULT_AUDIO_MODE 0
#define DEFAULT_AUDIO_MODE_CAPTURE 1
#define SNDPING_TIME 2000

/// global track callback class
CTrackCallBack TrackCallBack;
/*****************************************************************************
 * CThinkClient
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
CThinkClient::CThinkClient(const char *szName,CVSInterface* pParentInterface):
CVSInterface(szName,pParentInterface)
{
	m_hReportHwnd = 0;
}

/*****************************************************************************
 * CThinkClientReceiver
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
CThinkClientReceiver::CThinkClientReceiver(char *szName, CVSInterface* pParentInterface, CVideoSinc*pVideoSinc):
CThinkClient(szName,pParentInterface)
{
	m_DeviceStatus.bVideoAvailable=false;
	m_DeviceStatus.bAudioAvailable=false;
	m_DeviceStatus.bVideoValid=true;
	m_DeviceStatus.bAudioValid=true;
	m_DeviceStatus.bUseVideo=false;
	m_DeviceStatus.bUseAudio=false;

	m_pSinc = pVideoSinc;
	m_pRender = 0;
	m_ppRender = 0;
	m_pVideoDecompressor=new CVideoDecompressor;
	m_pRenderAudio= new VS_AudioRender(this);
	m_pVRenderBuffer = 0;
	m_pRcvTmp = 0;
	m_NhpBuff = 0;
	m_capsFourcc = 0x0;

	m_iAudioDevice = -1;
	m_iImageSize = 0;
	m_RcvFltr = 0;
    m_hwnd = 0;
	m_bNhp = false;
	m_bIntercom = false;
	m_bWaitKeyChangeMF = true;
	m_mfNotify.SetZero();

	m_IsInited = false;
}

/**
 **************************************************************************
 ****************************************************************************/
CThinkClientReceiver::~CThinkClientReceiver()
{
	Disconnect();
	Release();
	delete m_pRenderAudio;
	delete m_pVideoDecompressor;
	if (m_pRender) delete m_pRender; m_pRender = 0;
	if (m_ppRender) delete m_ppRender; m_ppRender = 0;
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientReceiver::RemoveAudio(int iDevice)
{
	VS_AutoLock lock(this);
	m_pRenderAudio->Release();
	m_DeviceStatus.bAudioAvailable = false;
	m_DeviceStatus.bUseAudio = m_DeviceStatus.bAudioAvailable;
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientReceiver::SetChangeAudioDevice(int iDevice)
{
	VS_AutoLock lock(this);
	PrepareAudio(iDevice);
	m_DeviceStatus.bAudioAvailable = m_pRenderAudio->Init(m_iAudioDevice, &m_mf);
	m_DeviceStatus.bUseAudio = m_DeviceStatus.bAudioAvailable;

}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientReceiver::PrepareAudio(int iDevice)
{
	m_iAudioDevice = iDevice;
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientReceiver::PrepareVideo(HWND hRenderView,RENDERPROC *pwndpSelfView,LPVOID *lpSelfView)
{
	m_pRender = CVideoRenderBase::RetrieveVideoRender(hRenderView, this);

	m_ppRender = new DWORD;
	*(DWORD*)m_ppRender = *(DWORD*)(&m_pRender);

	*pwndpSelfView = (RENDERPROC)(m_pRender->WindowProc);
	*lpSelfView = m_ppRender;
	m_hwnd = hRenderView;
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientReceiver::Init(VS_MediaFormat &mf, unsigned long capsFourcc)
{
	VS_AutoLock lock(this);
	Release();
	bool changeStereo(m_mf.dwStereo != mf.dwStereo);
	m_mf = mf;
	strncpy(m_CallId, GetInterfaceName(), MAX_PATH);
	m_CallId[strlen(m_CallId)-1] = '\0';
	// init audio
	m_DeviceStatus.bAudioAvailable = m_pRenderAudio->Init(m_iAudioDevice, &m_mf);
	m_pRenderAudio->SetCallId(m_CallId);
	m_DeviceStatus.bUseAudio = m_DeviceStatus.bAudioAvailable;
	m_DeviceStatus.bUseVideo = m_DeviceStatus.bVideoAvailable;
	// init video
	if (m_mf.IsVideoValid()) {
		CColorMode cm;
		BITMAPINFOHEADER bih;
		int height(m_mf.dwVideoHeight);
		int color(cm.I420);
		if (m_mf.dwStereo > 0) {
			if (m_mf.dwVideoCodecFCC != VS_VCODEC_VPXSTEREO) {
				height /= 2;
				color = cm.I420_STR1;
			}
			else {
				color = cm.I420_STR0;
			}
		}
		cm.SetColorMode(NULL, color, height, m_mf.dwVideoWidht);
		cm.ColorModeToBitmapInfoHeader(&bih);
		m_pVRenderBuffer = (unsigned char *)malloc(bih.biSizeImage);
		memset(m_pVRenderBuffer, 0, bih.biSizeImage);
		if (changeStereo) {
			delete m_pRender; m_pRender = 0;
			m_pRender = CVideoRenderBase::RetrieveVideoRender(m_hwnd, this, m_mf.dwStereo);
			*(DWORD*)m_ppRender = *(DWORD*)(&m_pRender);
		}
		m_pRender->iInitRender(m_hwnd, m_pVRenderBuffer, &cm, false);
		// init video decompressor
		m_pVideoDecompressor->ConnectToVideoDecompressor(&m_mf);
		m_iImageSize = bih.biSizeImage;
		m_pRcvTmp = (unsigned char*)malloc(0x200000);
		// register in video sinc thread
		m_VideoSource.m_phwnd = &m_hwnd;
		m_VideoSource.m_pRender = m_pRender;
		m_VideoSource.m_pOut = m_pVRenderBuffer;
		m_VideoSource.m_pVideo = m_pVideoDecompressor;
		m_VideoSource.m_iBufferSize = m_iImageSize;
		m_VideoSource.m_pAudio = m_DeviceStatus.IsAudio() ? m_pRenderAudio : 0;
		strncpy(m_VideoSource.m_CallId, m_CallId, MAX_PATH);
		m_pSinc->AddVideoSource(&m_VideoSource);
	} else {
		m_pRcvTmp = (unsigned char*)malloc(0x400000);
	}
	m_capsFourcc = capsFourcc;
	m_IsInited = true;
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientReceiver::ChangeCurrentMedia(VS_MediaFormat &mf, bool resetQueue)
{
	VS_AutoLock lock(this);

	if (!m_IsInited) return -1;

	int videoInit = 1;
	bool eqAudio = m_mf.AudioEq(mf);
	bool eqVideo = m_mf.VideoEq(mf);
	bool eqMedia = eqAudio && eqVideo;
	bool eqStereo = m_mf.dwStereo == mf.dwStereo;
	wchar_t stream_name[MAX_PATH] = {0};

	if (eqMedia) return 0;

	m_IsInited = false;
	mf.dwHWCodec = m_mf.dwHWCodec;
	m_mf = mf;

	if (!m_mf.IsVideoValid()) {
		m_mf.dwVideoWidht = 0;
		m_mf.dwVideoHeight = 0;
		videoInit = -2;
	}

	strncpy(m_CallId, GetInterfaceName(), MAX_PATH);
	m_CallId[strlen(m_CallId)-1] = '\0';

	if (!eqAudio) {
		m_pRenderAudio->Release();
		// init audio
		m_DeviceStatus.bAudioAvailable = m_pRenderAudio->Init(m_iAudioDevice, &m_mf);
		m_pRenderAudio->SetCallId(m_CallId);
		m_DeviceStatus.bUseAudio = m_DeviceStatus.bAudioAvailable;
	}

	if (m_mf.IsVideoValid() && !eqVideo) {
		m_pSinc->RemoveVideoSource(&m_VideoSource);
		m_pRender->Release();
		m_pVideoDecompressor->DisconnectToVideoDecompressor();
		if (m_pVRenderBuffer) free(m_pVRenderBuffer); m_pVRenderBuffer = 0;
		m_StoredKey.Empty();
		// init video
		CColorMode cm;
		BITMAPINFOHEADER bih;
		int height(m_mf.dwVideoHeight);
		int color(cm.I420);
		if (m_mf.dwStereo > 0) {
			if (m_mf.dwVideoCodecFCC != VS_VCODEC_VPXSTEREO) {
				height /= 2;
				color = cm.I420_STR1;
			}
			else {
				color = cm.I420_STR0;
			}
		}
		cm.SetColorMode(NULL, color, height, m_mf.dwVideoWidht);
		cm.ColorModeToBitmapInfoHeader(&bih);
		m_pVRenderBuffer = (unsigned char *)malloc(bih.biSizeImage);
		memset(m_pVRenderBuffer, 0, bih.biSizeImage);
		if (!eqStereo) {
			delete m_pRender; m_pRender = 0;
			m_pRender = CVideoRenderBase::RetrieveVideoRender(m_hwnd, this, m_mf.dwStereo);
			*(DWORD*)m_ppRender = *(DWORD*)(&m_pRender);
		}
		m_pRender->iInitRender(m_hwnd, m_pVRenderBuffer, &cm, false);
		// init video decompressor
		m_pVideoDecompressor->ConnectToVideoDecompressor(&m_mf);
		m_iImageSize = bih.biSizeImage;
		m_pRcvTmp = (unsigned char*)realloc(m_pRcvTmp, 0x200000);
		m_DeviceStatus.bUseVideo = m_DeviceStatus.bVideoAvailable;
		// register in video sinc thread
		m_VideoSource.m_phwnd = &m_hwnd;
		m_VideoSource.m_pRender = m_pRender;
		m_VideoSource.m_pOut = m_pVRenderBuffer;
		m_VideoSource.m_pVideo = m_pVideoDecompressor;
		m_VideoSource.m_iBufferSize = m_iImageSize;
		m_VideoSource.m_pAudio = m_DeviceStatus.IsAudio() ? m_pRenderAudio : 0;
		strncpy(m_VideoSource.m_CallId, m_CallId, MAX_PATH);
		m_pSinc->AddVideoSource(&m_VideoSource);
		if (resetQueue) {
			m_NhpBuff->ResetBuffer();
		}
	} else if (!eqAudio) {
		m_VideoSource.m_pAudio = m_DeviceStatus.IsAudio() ? m_pRenderAudio : 0;
	}

	if (!eqMedia) g_AviWriterGroup->ChangeMediaFormatStream(m_CallId, &m_mf);

	m_IsInited = true;
	return videoInit;
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientReceiver::Release()
{
	VS_AutoLock lock(this);
	m_IsInited = false;
	m_pSinc->RemoveVideoSource(&m_VideoSource);
	m_pRenderAudio->Release();
	m_pRender->Release();
	m_pVideoDecompressor->DisconnectToVideoDecompressor();
	if (m_pVRenderBuffer) free(m_pVRenderBuffer); m_pVRenderBuffer = 0;
	if (m_pRcvTmp) free(m_pRcvTmp); m_pRcvTmp = 0;
	m_StoredKey.Empty();
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientReceiver::Connect(const boost::shared_ptr<VS_StreamClientReceiver> &pTransport, HWND hReportHwnd, long fltr, VS_BinBuff &simkey, int ConfType)
{
	Disconnect();

	SetNotifyWnd(hReportHwnd);
	m_pReceiver = pTransport;
	m_bNhp = m_pReceiver->ConnectType() == vs_stream_client_connect_not_guaranteed;
	m_bIntercom = ConfType == 5 || ConfType < 0;
	int iTypeDecoder = 0;
	if (m_bIntercom) {
		if (m_mf.dwVideoCodecFCC == VS_VCODEC_H264 ||
			m_mf.dwVideoCodecFCC == VS_VCODEC_VPX ||
			m_mf.dwVideoCodecFCC == VS_VCODEC_VPXHD ||
			m_mf.dwVideoCodecFCC == VS_VCODEC_VPXSTEREO)
			iTypeDecoder = 2;
	}
	m_NhpBuff = VS_NhpBuffBase::Factory(m_bNhp, m_bIntercom, iTypeDecoder);
	m_RcvFltr = fltr;
	if (simkey.IsValid())
		m_StreamCrypter.Init((const unsigned char *)simkey.Buffer(), simkey.Size());

	RstConfStat();
	g_AviWriterGroup->CreateStream(m_CallId, &m_mf);
	ActivateThread(this);
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientReceiver::Disconnect()
{
	DesactivateThread();
	g_AviWriterGroup->ReleaseStream(m_CallId);
	if (m_pReceiver) {
		m_pReceiver->CloseConnection();
		m_pReceiver.reset();
	}
	if (m_NhpBuff) delete m_NhpBuff; m_NhpBuff = 0;
	m_StreamCrypter.Free();
}

/**
 **************************************************************************
 ****************************************************************************/
bool CThinkClientReceiver::SetReceivedCommand(stream::Command& cmd)
{
	if (cmd.type == stream::Command::Type::ChangeSndMFormat && cmd.sub_type == stream::Command::Reply && cmd.result == stream::Command::OK) {
		VS_MediaFormat mf;
		mf = *reinterpret_cast<VS_MediaFormat*>(cmd.data);
		mf.dwHWCodec = m_mf.dwHWCodec;
		if (!(mf==m_mf))
			Init(mf, m_capsFourcc);
		return true;
	} else if (cmd.type == stream::Command::Type::ChangeRcvMFormat && cmd.sub_type == stream::Command::Info) {
		VS_MediaFormat mf;
		mf = *reinterpret_cast<VS_MediaFormat*>(cmd.data);
		ChangeCurrentMedia(mf, true);
		if (m_mfNotify.IsVideoValid()) {
			NotifyResolution(m_mf);
		} else {
			NotifyResolution(m_mfNotify);
		}
		return true;
	}
	return false;
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientReceiver::SetRcvFltr(long fltr)
{
	m_RcvFltr = fltr;
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientReceiver::ResetWindow(HWND hwnd)
{
	m_hwnd = hwnd;
	if( m_pRender->m_IsValid)
		m_pRender->SetMode(VRM_DEFAULT, hwnd);
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientReceiver::RstConfStat()
{
	memset(&m_conf_stat, 0, sizeof(TConferenceStatRcv));
	m_conf_stat.start_t = m_conf_stat.last_t = timeGetTime();
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientReceiver::EndConfStat()
{
	if (m_conf_stat.started == 1) m_conf_stat.started = 2;
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientReceiver::SetConfStat(int now_t, int bytes, int type)
{
	if (m_conf_stat.started == 0 || m_conf_stat.started == 1) {
		m_conf_stat.now_t = now_t;
		m_conf_stat.bytes += bytes;
		m_NhpBuff->GetStatistics(m_conf_stat.avg_jitter, m_conf_stat.loss_rcv_packets);
		m_conf_stat.bytes_cur[type] += bytes;
		int dt = m_conf_stat.GetPeriodCalc();
		if (dt > 4000) {
			m_conf_stat.media_traffic =
				(int)(((double)m_conf_stat.bytes_cur[0] + (double)m_conf_stat.bytes_cur[1] + (double)m_conf_stat.bytes_cur[2] + (double)m_conf_stat.bytes_cur[3]) * 1000.0 / (double)dt);
			m_conf_stat.last_t = now_t;
			for (int i = 0; i < 5; i++) m_conf_stat.bytes_cur[i] = 0;
		}
		m_conf_stat.started = 1;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
bool CThinkClientReceiver::GetConfStat(TConferenceStatistics *cst)
{
	if (m_conf_stat.started == 0) return false;
	cst->participant_time = m_conf_stat.GetPeriodTime();
	cst->avg_jitter = m_conf_stat.avg_jitter;
	cst->loss_rcv_packets = m_conf_stat.loss_rcv_packets;
	cst->avg_rcv_bitrate = m_conf_stat.bytes;
	return m_conf_stat.started == 2;
}


int CThinkClientReceiver::GetQuality()
{
	int a = 0, b = 0;
	if (!m_pRenderAudio->GetBuffBounds(a, b) || (a+b)==0)
		return 0;
	a = (a+b)/2;
	if (a < 200)
		return 6;
	else if (a < 300)
		return 5;
	else if (a < 500)
		return 4;
	else if (a < 800)
		return 3;
	else if (a < 1200)
		return 2;
	else
		return 1;
}

void CThinkClientReceiver::EnableBorders(bool enable)
{
	m_pRender->EnableBorders(enable);
}

void CThinkClientReceiver::SetBordersAlpha(uint8_t alpha)
{
	m_pRender->SetBordersAlpha(alpha);
}

/**
 **************************************************************************
 ****************************************************************************/
DWORD CThinkClientReceiver::Loop(LPVOID hEvDie)
{

	unsigned char *pTmp = (unsigned char *)malloc(65536);
	unsigned char *decbuff = (unsigned char *)malloc(65536);
	stream::Track track;
	int32_t iSize;
	bool NeedKey = false;
	DWORD TimeKeyPrev = 0;
	boost::shared_ptr<VS_StreamClientReceiver> localpReceiver = m_pReceiver;
	HANDLE handles[4];
	handles[0] = hEvDie;
	handles[1] = m_pRenderAudio->GetCmpleteEvent();
	handles[2] = localpReceiver->GetReceiveEvent();
	DWORD waitRes = WAIT_OBJECT_0 + 2;
	DWORD exitReason = 0;
	DWORD LastRcvTime = timeGetTime();
	bool udpStream = localpReceiver->ConnectType()==vs_stream_client_connect_not_guaranteed;

	while(true) {
		DWORD CurrTime = timeGetTime();
		switch(waitRes)
		{
		case WAIT_OBJECT_0 + 0:
			exitReason = 1;
			break;
		case WAIT_OBJECT_0 + 1:
			Lock();
			m_pRenderAudio->CheckNoiseInsert();
			UnLock();
			break;
		case WAIT_OBJECT_0 + 2:
			iSize = localpReceiver->ReceiveFrame(pTmp, 65536, &track);
			if (iSize >= 0) {
				if (iSize > 0 && m_StreamCrypter.IsValid()) {
					uint32_t decSize = 0x10000;
					if (m_StreamCrypter.Decrypt(pTmp, iSize, decbuff, &decSize) && decSize > 0) {
						memcpy(pTmp, decbuff, decSize);
						iSize = decSize;
					}
				}
				LastRcvTime = CurrTime;
				if (NeedKey && CurrTime - TimeKeyPrev > 2000) {
					SendKeyReq();
					NeedKey = false;
					TimeKeyPrev = CurrTime;
				}
				int DataType;
				bool trackReceived = true;
				if (m_bNhp) {
					VS_NhpFirstHeader* hdr = (VS_NhpFirstHeader*)pTmp;
					DataType = hdr->dataType;
					ReceiveNHPData(pTmp, iSize);
				}
				else {
					if		(track == stream::Track::audio) {
						DataType = 1;
						trackReceived = (m_RcvFltr&VS_RcvFunc::FLTR_RCV_AUDIO)!=0;
					}
					else if (track == stream::Track::video) {
						DataType = 2;
						trackReceived = (m_RcvFltr&VS_RcvFunc::FLTR_RCV_VIDEO)!=0;
					}
					else if (track == stream::Track::data) {
						DataType = 3;
						trackReceived = (m_RcvFltr&VS_RcvFunc::FLTR_RCV_DATA)!=0;
					}
					else
						DataType = 0;
					if (trackReceived) {
						if (ReceiveTCPData(pTmp, iSize, track)==-1) {
							NeedKey = true;
						}
					}
					else
						iSize = 0;
				}
				SetConfStat(CurrTime, iSize, DataType);
			}
			else {// received size < 0
				if (iSize == -1)
				{
					DTRACE(VSTM_THCL, "localpReceiver->ReceiveFrame return -1. LastError = %d",GetLastError());
					exitReason = 2;
				}
				else if (iSize==-2) { // reconnect or re-read
					DTRACE(VSTM_THCL, "localpReceiver->ReceiveFrame return -2. Repead reading...", m_InterfaceName);
				}
			}
			break;
		case WAIT_TIMEOUT:
			break;
		case WAIT_FAILED:
			exitReason = 3;
			break;
		}
		// if no data more then 10 sec disconect
		if (udpStream && CurrTime-LastRcvTime > 10000 && !m_bIntercom)
			exitReason = 4;
		if (exitReason)
			break; // while
		if (!udpStream && handles[2]!= localpReceiver->GetReceiveEvent()) {
			DTRACE(VSTM_THCL, "Receiver %s Reconnect", m_InterfaceName);
			handles[2] = localpReceiver->GetReceiveEvent();
			waitRes = WAIT_OBJECT_0 + 2;
//			continue;
		}
		waitRes = WaitForMultipleObjects(3, handles, FALSE, 100);
	}
	if (localpReceiver)
		localpReceiver->CloseConnection();
	free(pTmp); pTmp = 0;
	free(decbuff); decbuff = 0;
	if (exitReason!=1){
		char *p=(char*)LocalAlloc(0,strlen(m_InterfaceName)+1); *p = 0;
		strcpy(p,m_InterfaceName);
		PostMessage(m_hReportHwnd , WM_USER+17, exitReason==4 ? 10 : 3, (LPARAM)p);
	}
	EndConfStat();
	return NOERROR;
}

int CThinkClientReceiver::CheckMediaFormat(unsigned char *pSource, bool bKey, unsigned int size)
{
	if (!bKey) {
		if (m_bWaitKeyChangeMF) return -1;
		else return 0;
	}
	m_bWaitKeyChangeMF = true;

	VS_MediaFormat mf = m_mf;
	int width = m_mf.dwVideoWidht, height = m_mf.dwVideoHeight, fps = m_mf.dwFps, stereo = m_mf.dwStereo;
	unsigned int fourcc = m_mf.dwVideoCodecFCC;
	int ret = -1, num_threads = 0;

	if (m_capsFourcc & (0x00000020 | 0x00000040 | 0x00000080)) ret = ResolutionFromBitstream_VPX(pSource, size, width, height, num_threads);
	if (ret == 0) {
		if (num_threads == 1) fourcc = VS_VCODEC_VPX;
		else if (num_threads == 2) fourcc = VS_VCODEC_VPXSTEREO;
		else fourcc = VS_VCODEC_VPXHD;
	} else {
		if (m_capsFourcc & 0x00000002) ret = ResolutionFromBitstream_H264(pSource, size, width, height);
		if (ret == 0 || ret == -2) {
			fourcc = VS_VCODEC_H264;
		} else {
			if (m_capsFourcc & 0x00000100) ret = ResolutionFromBitstream_H265(pSource, size, width, height);
			if (ret == 0 || ret == -2) {
				fourcc = VS_VCODEC_H265;
			}
			else {
				if (m_capsFourcc & 0x00000001) ret = ResolutionFromBitstream_XCC(pSource, size, width, height);
				if (ret == 0) {
					fourcc = VS_VCODEC_XC02;
				}
				else {
					if (m_capsFourcc & 0x00000004) ret = ResolutionFromBitstream_H263(pSource, size, width, height);
					if (ret == 0) {
						fourcc = VS_VCODEC_H263P;
					}
				}
			}
		}
	}
	if (ret < 0) {
		if (ret == -2) ret = 0;
		m_bWaitKeyChangeMF = (ret < 0);
		return ret;
	}
	mf.SetVideo(width, height, fourcc, fps, stereo);
	ret = ChangeCurrentMedia(mf, false);
	m_mfNotify = m_mf;
	NotifyResolution(m_mfNotify);
	m_bWaitKeyChangeMF = (ret < 0);

	return ret;
}

void CThinkClientReceiver::NotifyResolution(VS_MediaFormat &mf)
{
	int mode = mf.GetModeResolution();
	PostMessage(m_hReportHwnd, WM_USER+23, MAKELONG(mf.dwVideoWidht, mf.dwVideoHeight), MAKELONG(1, mode));
}

int CThinkClientReceiver::ReceiveTCPData(unsigned char* buff, int size, stream::Track track)
{
	int ret = 0;
	if		(track == stream::Track::audio) {
		TrackCallBack.SendTrack(static_cast<uint8_t>(track), size, buff, m_InterfaceName);
		if (m_DeviceStatus.IsAudio()) {
			Lock();
			m_pRenderAudio->Play((char*)buff, size);
			unsigned long level = m_pRenderAudio->GetLevel();

			m_Van.Set(GetTickCount(), level);

			m_pRender->SetBordersAlpha(std::min(255, int(m_Van.Get() * 255 / 1500)));

			UnLock();
		}
	}
	else if (track == stream::Track::video) {
		TrackCallBack.SendTrack(static_cast<uint8_t>(track), size, buff, m_InterfaceName);
		unsigned long usize = size;
		unsigned long VideoInterval = 0;
		bool key = false;
		if (!m_vs_In.Add(buff, usize, track) && track == stream::Track::video)
			ret = -1;
		if (m_vs_In.Get(m_pRcvTmp, usize, vs::ignore<stream::Track>{}, VideoInterval, &key)>=0) {
			Lock();
			if (CheckMediaFormat(m_pRcvTmp, key, usize) >= 0) {
				bool res = m_VideoSource.AddData(m_pRcvTmp, usize, VideoInterval, key, g_AviWriterGroup->GetRecordMB(m_CallId));
				if (res) {
					if (key) {
						m_StoredKey.Set(m_pRcvTmp, usize);
					}
				}
				else {
					ret = -1;
				}
			}
			UnLock();
		}
	}
	else if (track == stream::Track::data) {
		int ID = *(unsigned short *)(buff);
		if ((ID>>8)==0x02) { // rcdv flag
			int iSize_ = *(unsigned short *)(buff+size-2);
			RCDV_Decode(buff+2, m_pRcvTmp, iSize_);
			TrackCallBack.ExternalTrack(&ID, &iSize_, m_pRcvTmp, m_InterfaceName);
		}
		else if ((ID>>8)==0) { // not compressed
			int iSize_ = size-2;
			TrackCallBack.ExternalTrack(&ID, &iSize_, buff+2, m_InterfaceName);
		}
	}
	else if (track == stream::Track::command) {
		if (size > 0) {
			stream::Command cmd(buff, size);
			if (!SetReceivedCommand(cmd))
				g_cmdProc.AddCommand(cmd, false);
		}
	}
	else if (track == stream::Track::old_command) {
		if (size==1) {
			stream::Command cmd;
			cmd.RequestKeyFrame();
			g_cmdProc.AddCommand(cmd, false);
		}
		else if (size>0) {
			stream::Command cmd;
			cmd.BrokerStat(buff, size);
			g_cmdProc.AddCommand(cmd, false);
		}
	}
	return ret;
}


int CThinkClientReceiver::ReceiveNHPData(unsigned char* buff, int size)
{
	m_NhpBuff->Add(buff, size);
	int type = -1;
	while (m_NhpBuff->Get(m_pRcvTmp, size, type)) {
		if		(type==NHPH_DT_AUDIO) {
			TrackCallBack.SendTrack(1, size, m_pRcvTmp, m_InterfaceName);
			if (m_DeviceStatus.IsAudio()) {
				Lock();
				m_pRenderAudio->Play((char*)m_pRcvTmp+4, size-4);
				UnLock();
			}
		}
		else if (type==NHPH_DT_VIDEO) {
			TrackCallBack.SendTrack(2, size, m_pRcvTmp, m_InterfaceName);
			Lock();
			VS_NhpVideoHeader* hdr = (VS_NhpVideoHeader*)m_pRcvTmp;
			if (CheckMediaFormat(m_pRcvTmp+6, hdr->FrameType==0, size) >= 0){
				VS_NhpVideoHeader* hdr = (VS_NhpVideoHeader*)m_pRcvTmp;
				DWORD dTime = hdr->TimeStamp - m_PrevVideoTime;
				m_PrevVideoTime = hdr->TimeStamp;
				if (dTime > 1000)
					dTime = 1000;
				bool res = m_VideoSource.AddData(m_pRcvTmp+6, size-6, dTime, hdr->FrameType == 0, g_AviWriterGroup->GetRecordMB(m_CallId));
				if (res && hdr->FrameType == 0)
					m_StoredKey.Set(m_pRcvTmp+6, size-6);
			}
			UnLock();
		}
		else if (type==NHPH_DT_DATA) {
			int ID = *(unsigned short *)(m_pRcvTmp);
			if ((ID>>8)==0x02) { // rcdv flag
				int iSize_ = *(unsigned short *)(m_pRcvTmp+size-2);
				RCDV_Decode(m_pRcvTmp+2, buff, iSize_);
				TrackCallBack.ExternalTrack(&ID, &iSize_, buff, m_InterfaceName);
			}
			else if ((ID>>8)==0) { // not compressed
				int iSize_ = size-2;
				TrackCallBack.ExternalTrack(&ID, &iSize_, m_pRcvTmp+2, m_InterfaceName);
			}
		}
		else if (type==NHPH_DT_CMND) {
			stream::Command cmd(m_pRcvTmp, size);
			if (!SetReceivedCommand(cmd) && !m_NhpBuff->SetReceivedCommand(cmd))
				g_cmdProc.AddCommand(cmd, false);
		}
	}
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientReceiver::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;
	if(strncmp(pSection,_funcResetWindow,sizeof(_funcResetWindow))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			ResetWindow((HWND)int(*var));
			return VS_INTERFACE_OK;
		}
	}
	else if(strncmp(pSection,_funcFormat,sizeof(_funcFormat))==0){
		switch(VS_OPERATION)
		{
		case GET_PARAM:{
			long i;
			SAFEARRAYBOUND rgsabound[1];
			SAFEARRAY * psa;
			rgsabound[0].lLbound = 0;
			rgsabound[0].cElements = 6;
			psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
			if(psa == NULL)
				return VS_INTERFACE_INTERNAL_ERROR;
			var->parray=psa;
			var->vt= VT_ARRAY | VT_VARIANT;
			_variant_t var_;
			i=0;
			var_=m_mf.dwVideoWidht;
			SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
			i++;
			var_=m_mf.dwVideoHeight;
			SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
			i++;
			var_=m_mf.dwAudioSampleRate;
			SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
			i++;
			var_=m_mf.dwAudioCodecTag;
            SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
			i++;
			var_=m_mf.dwAudioBufferLen;
			SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
			i++;
			var_=m_mf.dwVideoCodecFCC;
			SafeArrayPutElement(psa, &i, &var_.GetVARIANT());

			return VS_INTERFACE_OK;
					   }
		}
	}
	else if (strcmp(pSection, _funcSetRsvFltr)==0) {
		if (VS_OPERATION == SET_PARAM) {
			SetRcvFltr(long(*var));
			return VS_INTERFACE_OK;
		}
	}
	return VS_INTERFACE_NO_FUNCTION;
}

/*****************************************************************************
 * CThinkClientSender::CConnectStatus
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::CConnectStatus::Clear()
{
	bNeedKeyFrame = true;
	bWaitKeyFrame = true;
	bWaitCompress = false;
	UseNhp = false;
	HalfVideo = false;
	bUpdateBitrate = false;
	bSkipNextFrame = false;
	bGrabFrame = false;
	iBitrate = 100;
	iBaseBitrate = 100;
	iFPS = 3000;
	iRealFPS = 3000;
	iSkipAudio = 10;
	iVideoFrameNum = 0;
	HalVideoTime = 0;
	iNumDropFrames = 0;
	iSwitchBitrate = 0;
	SwitchVModeTime = 0;
	QueueVideoTimestamps = {};
};

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::CConnectStatus::DropVideoFrame(){
	bNeedKeyFrame = true;
	bWaitKeyFrame = true;
	bWaitCompress = false;
	bSkipNextFrame = false;
	bGrabFrame = false;
}

/*****************************************************************************
 * CThinkClientSender
 ****************************************************************************/
#define READ_DWORD_PARAM(x, name, defx, minx, maxx) \
	if (ReadParam((name), &vr.GetVARIANT())) (x) = (defx); \
	else { (x) = vr; if ((x) < (minx)) (x) = (minx); if ((x) > (maxx)) (x) = (maxx); }
/**
 **************************************************************************
 ****************************************************************************/
CThinkClientSender::CThinkClientSender(CVSInterface *pParentInterface, CVSTrClientProc* protocol):
CThinkClient("Sender",pParentInterface)
{
	// CThinkClient members
	m_DeviceStatus.bVideoAvailable = false;
	m_DeviceStatus.bAudioAvailable = false;
	m_DeviceStatus.bUseAudio = true;
	m_DeviceStatus.bUseVideo = true;
	m_DeviceStatus.bAudioValid = true;
	m_DeviceStatus.bVideoValid = true;
	m_stat = (TServerStatistics*)malloc(sizeof(TServerStatistics));
	m_hSendKeyReq = CreateEvent(NULL,FALSE,FALSE,NULL);
	// self members
	m_timerCmd = 0;
	m_hEventCmd = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hEventFirstSnd = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_BandContr = 0;
	m_FrameQueue = 0;
	m_pPacketQueueContr = 0;
	m_pVideoCompressor = new CVideoCompressor;
	m_pProtocol = protocol;
	m_pSysBench = new VS_SysBenchmarkWindows;
	m_pVCaptureList = CVideoCaptureList::Create(CVideoCaptureList::CAPTURE_MF, this, m_pSysBench->GetSndLevel());
	m_pACaptureList = new CAudioCaptureList(this);

	m_Connected = false;
	m_MediaConnected = false;
	m_isIntercom = false;
	_variant_t vr(0);

	m_LastSendTime = 0;
	m_KeyRepeatTime = 0;
	m_KeyRepeatPeriod = 10000; // 10 sec
	m_PrevVideoTime = 0;
	READ_DWORD_PARAM(m_FPSvsBr, (char*)_FPSRateName, 32, 8, 80);
	SetCurrentFpsVsQ(m_FPSvsBr); //m_CurrFPSvsBr
	READ_DWORD_PARAM(m_BrLoadFactor, "BrLoadFactor", 100, 50, 250);
	READ_DWORD_PARAM(m_BrAutoDisabled, "BrAutoDisabled", 0, 0, 1);
#ifdef _BUILD_CONFERENDO
	READ_DWORD_PARAM(m_MaxGConfBand, "MaxGConfBand", 384, 64, 512); // the same
#else
	READ_DWORD_PARAM(m_MaxGConfBand, "MaxGConfBand", 2048, 64, 2048);
#endif
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	int hq_auto = 1;
	int snd_stereo_mode = 0;
	m_FPSFixMax = 0;
	key.GetValue(&m_FPSFixMax, 4, VS_REG_INTEGER_VT, "FPSFixMax");
	key.GetValue(&hq_auto, 4, VS_REG_INTEGER_VT, "HQ Auto");
	key.GetValue(&snd_stereo_mode, 4, VS_REG_INTEGER_VT, "SndStereo");
	m_PrevBandwith = 0;
	m_iMaxFPS = 2500;
	m_iMinBitrate = 0;
#ifdef _BUILD_CONFERENDO
	READ_DWORD_PARAM(m_iBandwidth, (char*)_BandwidthName, 512, 32, 512);
	m_iOtherSideBand = 512;
	m_iServerBand = 512;
#else
	READ_DWORD_PARAM(m_iBandwidth, (char*)_BandwidthName, 1024, 32, 10240);
	m_iOtherSideBand = 10240;
	m_iServerBand = 10240;
#endif
	m_iServerBandVideo = m_iServerBand;
	m_ExtFrameMem = 0;
	m_last_quality = 8;
	m_pLevelCaps = new tc_VideoLevelCaps;
	RstConfStat();
	m_scheme_bitrate = 0;
	m_pMediaFormatManager = new VS_MediaFormatManager;
	m_pMediaFormatManager->SetControlExternal(CTRL_EXT_HQAUTO, hq_auto);
	m_pMediaFormatManager->SetControlExternal(CTRL_EXT_STEREO, snd_stereo_mode);
	m_pMediaFormatManager->SetControlExternal(CTRL_EXT_SYSBENCH, m_pSysBench);
	m_pMediaFormatManager->SetControlExternal(CTRL_EXT_VIDEOCAPTURELIST, m_pVCaptureList);
	m_pMediaFormatManager->SetControlExternal(CTRL_EXT_PROTOCOL, m_pProtocol);
	m_pMediaFormatManager->SetControlExternal(CTRL_EXT_LEVELCAPS, m_pLevelCaps);
	m_bAllowDynChange = false;
	m_TypeOfAutoPodium = 0;

	m_PhoenixDev = 0;

}

/**
 **************************************************************************
 ****************************************************************************/
CThinkClientSender::~CThinkClientSender()
{
	_variant_t var(m_iBandwidth);
	WriteParam((char*)_BandwidthName, &var.GetVARIANT());
	var = m_FPSvsBr;
	WriteParam((char*)_FPSRateName, &var.GetVARIANT());
	CloseHandle(m_hEventCmd);
	CloseHandle(m_hEventFirstSnd);
	CloseHandle(m_hSendKeyReq);
	m_timerCmd = 0;
	free(m_stat);
	delete m_pMediaFormatManager;
	delete m_pLevelCaps;
	delete m_pSysBench;
	delete m_pVideoCompressor;
	delete m_pVCaptureList;
	delete m_pACaptureList;
	VS_Map::Iterator pIterator;
	GetEnumInterface(&pIterator);
	CVSInterface *pInt;
	while (pInt=GetNextInterface(&pIterator))
		delete pInt;
	return;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::Init(VS_MediaFormat &mf)
{
	Release();
	m_pMediaFormatManager->GetMediaFormat(&mf, &m_vmf);
	mf_current = m_vmf;
	// Common events init
	m_EventManager.Clear();
	m_EventManager.Add(m_hSendKeyReq, QueryKeyProcess, NULL);
	m_EventManager.Add(g_cmdProc.GetCmdEvent(), ReceivedCommandProcess, &g_cmdProc);
	m_EventManager.Add(m_pSysBench->GetBenchEvent(), CheckVideoModeProcess, 0);
	m_EventManager.Add(m_pProtocol->GetRestrictHandle(), CheckRestrictMediaFormat, 0);
	m_EventManager.Add(m_hEventCmd, InfoMediaFormatProcess, 0);
	m_pSysBench->Run();
	int level = m_pSysBench->GetRcvLevel();
	int level_group = m_pSysBench->GetRcvGroupLevel();
	int rating = m_pLevelCaps->LevelSnd2Rating(level);
	m_pProtocol->SetMediaFormat(m_vmf, rating, level, level_group);
	ActivateThread(this);
	return rating;
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::Release()
{
	Disconnect();
	DesactivateThread();
	m_EventManager.Clear();
	return;
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::Connect(const boost::shared_ptr<VS_StreamClientSender> &pTransport, HWND hReportHwnd, int Type, int Bandwidth, bool isIntercom)
{
	Disconnect();
	m_pSender = pTransport;
	VS_BinBuff simkey;
	if (m_pProtocol->GetSimKey(0, simkey))
		m_StreamCrypter.Init((unsigned char*)simkey.Buffer(), simkey.Size());

	int server_bw(0);
	int server_fps(0);
	{
		char server_bw_str[256] = {0};
		if (m_pProtocol->GetProperties(ALLOWED_BY_SERVER_MAX_BW,server_bw_str)==ERR_OK && server_bw_str && *server_bw_str)
			server_bw = atoi(server_bw_str);
		char server_fps_str[256] = {0};
		if (m_pProtocol->GetProperties(ALLOWED_BY_SERVER_MAX_FPS,server_fps_str)==ERR_OK && server_fps_str && *server_fps_str)
			server_fps = atoi(server_fps_str);
	}

	SetNotifyWnd(hReportHwnd);
	srand(timeGetTime());
	if (Type) {	// host or groupconf
		m_iServerBand = 2048;
		m_iOtherSideBand = (server_bw) ? std::min(server_bw, m_MaxGConfBand) : m_MaxGConfBand;
		m_iServerBandVideo = 640;
		m_iMaxFPS = (server_fps) ? std::min(server_fps, 1500) : 1500;
		m_KeyRepeatPeriod = (isIntercom) ? 6000 + (rand()&0x7ff) : 10000 + (rand()&0xfff);
	}
	else {
		m_iServerBand = 10240;
		m_iOtherSideBand = (server_bw) ? std::min(server_bw, Bandwidth) : Bandwidth;
		m_iMaxFPS = (server_fps) ? std::min(server_fps, 3000) : 3000;
		m_KeyRepeatPeriod = 30000 + (rand()&0x1fff);
	}
	m_iMinBitrate = 20;

	memset(m_stat, 0, sizeof(TServerStatistics));
	RstConfStat();

	VS_MediaFormat mf = m_pMediaFormatManager->SetConnection();


	m_ConnectStatus.Clear();
	m_ConnectStatus.UseNhp = m_pSender->ConnectType() == vs_stream_client_connect_not_guaranteed;
	m_FrameQueue = VS_SendFrameQueueBase::Factory(m_ConnectStatus.UseNhp, mf.dwSVCMode != 0);

	int limitSize = 0;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	if (key.GetValue(&limitSize, sizeof(int), VS_REG_INTEGER_VT, "PacketLimit"))
		m_FrameQueue->SetLimitSize(limitSize);

	m_BandContr = VS_ControlBandBase::Factory(m_ConnectStatus.UseNhp, isIntercom);
	m_BandContr->SetQueuePointers(m_FrameQueue, m_pSender->GetSendEvent());
	m_pPacketQueueContr = new VS_ControlPacketQueue();

	m_pMediaFormatManager->SetControlExternal(CTRL_EXT_CTRLBANDWIDTH, m_BandContr);

	int T = 100;
	key.GetValue(&T, sizeof(int), VS_REG_INTEGER_VT, "T bound");
	m_pPacketQueueContr->Init(m_FrameQueue, m_pSender, &m_StreamCrypter, std::min(m_iBandwidth, m_iOtherSideBand), T);

	m_scheme_bitrate = 0;
	key.GetValue(&m_scheme_bitrate, 4, VS_REG_INTEGER_VT, "Scheme Bitrate");

	int MaxBitrate = std::min(m_iBandwidth, m_iOtherSideBand);
	int StartBitrate = MaxBitrate * 2 / 3;

#ifdef _BUILD_CONFERENDO
	if (StartBitrate > 256)
		StartBitrate = 256;
	if (StartBitrate < 32)
		StartBitrate = 32;
#else
	if (StartBitrate > 384)
		StartBitrate = 384;
	if (StartBitrate < 48)
		StartBitrate = 48;
#endif

	key.GetValue(&StartBitrate, 4, VS_REG_INTEGER_VT, "StartBitrate");
	if (StartBitrate > MaxBitrate)
		StartBitrate = MaxBitrate;

	m_BandContr->IsDataAllowed(StartBitrate);
	m_ConnectStatus.iBitrate = m_BandContr->GetVideoBandwidth(StartBitrate);
	m_ConnectStatus.iBaseBitrate = m_iServerBandVideo;
	m_ConnectStatus.iFPS = m_iMaxFPS;
	m_ConnectStatus.iRealFPS = m_iMaxFPS;
	SetCurrentFpsVsQ(m_FPSvsBr);
	m_ExtFrameMem = (unsigned char*)malloc(0x20000);
	m_isIntercom = isIntercom;
	m_last_quality = 8;
	VS_MediaFormat mfn;
	m_pMediaFormatManager->SetControlExternal(CTRL_EXT_BITRATE, StartBitrate);
	m_bAllowDynChange = m_pMediaFormatManager->GetMediaFormat(&mf, &mfn);
	m_timerCmd = timeSetEvent(2000, 10, (LPTIMECALLBACK)m_hEventCmd, NULL, TIME_CALLBACK_EVENT_SET | TIME_PERIODIC);
	m_EventManager.Add(m_hEventFirstSnd, QueueFirstProcess, NULL);
	m_EventManager.Add(m_pSender->GetSendEvent(), QueueProcess, NULL);
	m_EventManager.Exclude(m_pSender->GetSendEvent(), true);
	m_Connected = true;
	SetEvent(m_hEventFirstSnd);
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::Disconnect()
{
	VS_AutoLock lock(this);
	if (m_timerCmd != 0) timeKillEvent(m_timerCmd); m_timerCmd = 0;
	m_Connected = false;
	m_scheme_bitrate = 0;
	m_pMediaFormatManager->ResetConnection();
	ReleaseCurrentMedia();
	if (m_pSender) {
		m_EventManager.Remove(QueueFirstProcess);
		m_EventManager.Remove(QueueProcess);
		m_pSender->CloseConnection();
		m_pSender.reset();
	}
	m_StreamCrypter.Free();
	EndConfStat();
	if (m_FrameQueue) delete m_FrameQueue; m_FrameQueue = 0;
	if (m_BandContr) delete m_BandContr; m_BandContr = 0;
	if (m_pPacketQueueContr) delete m_pPacketQueueContr; m_pPacketQueueContr = 0;
	if (m_ExtFrameMem) free(m_ExtFrameMem); m_ExtFrameMem = 0;
	int level = m_pSysBench->GetRcvLevel();
	int level_group = m_pSysBench->GetRcvGroupLevel();
	int rating = m_pLevelCaps->LevelSnd2Rating(level);
	m_pProtocol->SetMediaFormat(m_vmf, rating, level, level_group);
	SetEvent(m_pProtocol->GetRestrictHandle());
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::SetCurrentMedia(VS_MediaFormat &mf)
{
	VS_AutoLock lock(this);
	ReleaseCurrentMedia();
	mf_current = mf;
	// reinit video capture slot in case of format mismatch
	CVideoCaptureSlotBase *cs=(CVideoCaptureSlotBase *)FindChildrenByName("VideoCaptureSlot");
	if (cs) {
		if (m_DeviceStatus.bVideoValid) {
			m_pMediaFormatManager->GetMediaFormat(&mf, &mf_current);
			cs->_Init(mf_current, DEVICE_CHANGEFORMAT);
		}
		NotifyResolution(&mf_current);
	}
	// clear compression status
	m_ConnectStatus.DropVideoFrame();
	// create and start new compressor
	m_pVideoCompressor->ConnectToVideoCompressor(&mf_current);
	m_EventManager.Add(m_pVideoCompressor->GetEventEx(), VideoCompressProcess, m_pVideoCompressor);
	m_PrevVideoTime = timeGetTime();
	// create and start (compressed) audio capture
	m_pProtocol->GetMyName(m_CallId);
	_variant_t var;
	var = (bstr_t)m_CallId;
	Apply2Children(IT_AUDIOCAPTURE, SET_PARAM, "SetCallId", &var);
	var.Clear();
	var = (int)&mf_current;
	Apply2Children(IT_AUDIOCAPTURE, RUN_COMMAND, "Start", &var);
	g_AviWriterGroup->CreateStream(m_CallId, &mf_current);
	// set initaial bitrate for new compressor and videocapture fps
	m_last_quality = 1; //reset audio q
	CalcBitrate(m_ConnectStatus.iBitrate, m_ConnectStatus.iBaseBitrate, &m_ConnectStatus.iFPS);
	m_MediaConnected = true;
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::ReleaseCurrentMedia()
{
	VS_AutoLock lock(this);
	if (!m_MediaConnected) return; else m_MediaConnected = false;
	g_AviWriterGroup->ReleaseStream(m_CallId);
	// disconnect compressor
	m_EventManager.Remove(m_pVideoCompressor->GetEventEx());
	m_pVideoCompressor->DisconnectToVideoCompressor();
	// stop audio
	_variant_t var = 0;
	Apply2Children(IT_AUDIOCAPTURE, RUN_COMMAND, "Stop", &var);
	// restore format if not connected
	if (!m_Connected) {
		CVideoCaptureSlotBase *cs=(CVideoCaptureSlotBase *)FindChildrenByName("VideoCaptureSlot");
		if (cs) {
			VS_MediaFormat mf;
			m_pMediaFormatManager->GetMediaFormat(&m_vmf, &mf);
			var = mf.dwFps;
			Apply2Children(IT_VIDEOCAPTURE, RUN_COMMAND, (char*)VS_CaptureDevice::_funcRealFramerate, &var);
			m_vmf = mf;
			cs->_Init(m_vmf, DEVICE_CHANGEFORMAT);
			NotifyResolution(&m_vmf);
		}
	}
	// stop QueueProcess()
	if (m_FrameQueue && m_pSender) {
		m_EventManager.Exclude(m_pSender->GetSendEvent(), true);
		m_FrameQueue->EraseMedia();
		m_ConnectStatus.QueueVideoTimestamps = {};
		m_ConnectStatus.LastSendVideo = 0;
		m_EventManager.Exclude(m_pSender->GetSendEvent(), false);
	}
	// restore preferable mediaformat
	m_StoredKey.Empty();
	mf_current = m_vmf;
}

/**
**************************************************************************
****************************************************************************/
void CThinkClientSender::ChangeCurrentMediaCommand(VS_MediaFormat &mf, eDeviceAction act)
{
	VS_AutoLock lock(this);
	mf.dwHWCodec = mf_current.dwHWCodec;
	ChangeCurrentMedia(mf, act);
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::ChangeCurrentMedia(VS_MediaFormat &mf, eDeviceAction act)
{
	VS_AutoLock lock(this);

	bool eqaudio = mf_current.AudioEq(mf);
	bool eqvideo = mf_current.VideoEq(mf) && (mf_current.dwHWCodec == mf.dwHWCodec);
	bool eqmedia = eqaudio && eqvideo;
	bool ismconnect = m_MediaConnected;
	wchar_t name_stream[MAX_PATH] = {0};

	m_MediaConnected = false;

	if (ismconnect) {
		// stop QueueProcess()
		if (!eqvideo || !eqaudio) {
			if (m_FrameQueue) {
				if (mf.dwHWCodec != ENCODER_H264_LOGITECH) {
					if (!eqvideo) {
						m_FrameQueue->EraseVideo();
						m_ConnectStatus.QueueVideoTimestamps = {};
						m_ConnectStatus.LastSendVideo = 0;
					}
				}
				if (!eqaudio) m_FrameQueue->EraseAudio();
			}
		}
	}

	if (!eqvideo || mf_current.dwFps != mf.dwFps || act == DEVICE_STARTUP) { /// change video
		if (!eqvideo && ismconnect) { /// resolution or fourcc
			// disconnect compressor
			m_EventManager.Remove(m_pVideoCompressor->GetEventEx());
			m_pVideoCompressor->DisconnectToVideoCompressor();
			// restore preferable mediaformat
			m_StoredKey.Empty();
		}
		// reinit video capture slot in case of format mismatch
		CVideoCaptureSlotBase *cs=(CVideoCaptureSlotBase *)FindChildrenByName("VideoCaptureSlot");
		if (cs) {
			cs->_Init(mf, act);
			NotifyResolution(&mf);
		}
		if (!eqvideo) {
			// clear compression status
			m_ConnectStatus.DropVideoFrame();
			if (mf.dwHWCodec == ENCODER_H264_LOGITECH) m_ConnectStatus.bNeedKeyFrame = false;
			// create and start new compressor
			m_pVideoCompressor->ConnectToVideoCompressor(&mf);
			m_EventManager.Add(m_pVideoCompressor->GetEventEx(), VideoCompressProcess, m_pVideoCompressor);
		}
	}
	if (!eqaudio) { /// change audio
		// stop audio
		_variant_t var = 0;
		Apply2Children(IT_AUDIOCAPTURE, RUN_COMMAND, "Stop", &var);
		var.Clear();
		var = (int)&mf;
		Apply2Children(IT_AUDIOCAPTURE, RUN_COMMAND, "Start", &var);
	}
	mf_current = mf;
	if (ismconnect) m_ConnectStatus.bUpdateBitrate = true;
	if (!eqmedia) g_AviWriterGroup->ChangeMediaFormatStream(m_CallId, &mf_current);
	m_MediaConnected = ismconnect;
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::SendInfoMediaFormat(VS_MediaFormat *mf)
{
	VS_AutoLock lock(this);

	bool bVideo = m_pProtocol->IsTrackSent(2) && m_DeviceStatus.IsVideo();
	bool bAudio = m_pProtocol->IsTrackSent(1) && m_DeviceStatus.IsAudio();

	if (bVideo || bAudio) {
		stream::Command cmd;
		if (mf == NULL) mf = &mf_current;
		cmd.InfoRcvMFormat(*mf);
		g_cmdProc.AddCommand(cmd);
	}
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::ChangeVideoMode()
{
	VS_AutoLock lock(this);
	if (m_MediaConnected) return;
	bool updateCapture(false);
	CVideoCaptureSlotBase *cs = (CVideoCaptureSlotBase *)FindChildrenByName("VideoCaptureSlot");
	if (cs) {
		updateCapture = m_pVCaptureList->UpdateSenderLevel(m_pSysBench->GetSndLevel(ENCODER_SOFTWARE), GetTypeHardwareCodec() != ENCODER_SOFTWARE);
	}
	if (g_AviWriterGroup) {
		g_AviWriterGroup->SetMixerMode(m_pSysBench->GetSndMBps(ENCODER_SOFTWARE));
	}
	VS_MediaFormat mf;
	if (!m_pMediaFormatManager->GetMediaFormat(&m_vmf, &mf)) return;
	//g_AviWriterGroup->ReleaseStream(m_CallId);
	// set camera format if not connected
	if (mf.IsVideoValid()) {
		if (cs) {
			cs->_Init(mf, updateCapture ? DEVICE_STARTUP : DEVICE_CHANGEFORMAT);
		}
		m_vmf = mf;
		NotifyResolution(&m_vmf);
		mf_current = m_vmf;
		int level = m_pSysBench->GetRcvLevel();
		int level_group = m_pSysBench->GetRcvGroupLevel();
		int rating = m_pLevelCaps->LevelSnd2Rating(level);
		m_pProtocol->SetMediaFormat(m_vmf, rating, level, level_group);
	}
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::RestrictMediaFormat()
{
	VS_AutoLock lock(this);
	if (m_MediaConnected) return;
	CVideoCaptureSlotBase *cs = (CVideoCaptureSlotBase *)FindChildrenByName("VideoCaptureSlot");
	if (!cs) return;
	VS_MediaFormat mf;
	char wxh_str[256] = {0};
	bool restrict = false;
	if (m_pProtocol->GetProperties(ALLOWED_BY_SERVER_MAX_WXH, wxh_str) == ERR_OK && wxh_str && *wxh_str) {
		char* ptr = strchr(wxh_str, 'x');
		if (ptr && *ptr)
			++ptr;
		if (ptr&&*ptr) {
			int w = atoi(wxh_str);
			int h = atoi(ptr);
			if ( w && w >= 64 && w <= 2048 &&
				 h && h >= 64 && h <= 2048 &&
				 ((w | h) & 0x7) == 0 )
			{
				m_pMediaFormatManager->SetControlExternal(CTRL_EXT_HQAUTO, false);
				m_pProtocol->SetAutoMode(0);
				m_vmf.dwVideoWidht = w;
				m_vmf.dwVideoHeight = h;
				restrict = true;
			}
		}
	}
	if (!restrict) {
		int hq_auto = 1;
		VS_RegistryKey key(true, REG_CurrentConfiguratuon);
		key.GetValue(&hq_auto, 4, VS_REG_INTEGER_VT, "HQ Auto");
		m_pMediaFormatManager->SetControlExternal(CTRL_EXT_HQAUTO, hq_auto);
		m_pProtocol->SetAutoMode(hq_auto);
	}
	m_pMediaFormatManager->GetMediaFormat(&m_vmf, &mf);
	if (mf.IsVideoValid()) {
		m_vmf = mf;
		mf_current = mf;
		cs->_Init(m_vmf, DEVICE_CHANGEFORMAT);
		NotifyResolution(&m_vmf);
		int level = m_pSysBench->GetRcvLevel();
		int level_group = m_pSysBench->GetRcvGroupLevel();
		int rating = m_pLevelCaps->LevelSnd2Rating(level);
		m_pProtocol->SetMediaFormat(m_vmf, rating, level, level_group);
	}
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::ConnectVideoDevice()
{
	VS_AutoLock lock(this);
	CVideoCaptureSlotBase *cs = (CVideoCaptureSlotBase *)FindChildrenByName("VideoCaptureSlot");
	if (!cs) return;
	VS_MediaFormat mf;
	m_pMediaFormatManager->GetMediaFormat(&mf_current, &mf);
	if (mf.IsVideoValid()) {
		NotifyResolution(&mf);
		if (m_MediaConnected) {
			ChangeCurrentMedia(mf, DEVICE_STARTUP);
		} else {
			cs->_Init(mf, DEVICE_STARTUP);
			m_vmf = mf;
			mf_current = mf;
			m_pProtocol->m_Status.MyInfo.ClientCaps.GetMediaFormat(mf);
			m_vmf.SetAudio(mf.dwAudioSampleRate, mf.dwAudioCodecTag);
			m_vmf.dwAudioBufferLen = mf.dwAudioBufferLen;
			mf_current = m_vmf;
			int level = m_pSysBench->GetRcvLevel();
			int level_group = m_pSysBench->GetRcvGroupLevel();
			int rating = m_pLevelCaps->LevelSnd2Rating(level);
			m_pProtocol->SetMediaFormat(m_vmf, rating, level, level_group);
		}
	} else {
		cs->_Init(mf, DEVICE_SHOOTDOWN);
	}
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::DisconnectVideoDevice()
{
	VS_AutoLock lock(this);
	CVideoCaptureSlotBase *cs = (CVideoCaptureSlotBase *)FindChildrenByName("VideoCaptureSlot");
	if (!cs) return;
	VS_MediaFormat mf;
	cs->_Init(mf, DEVICE_SHOOTDOWN);
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::AddFrameToQueue(int track,int Size,unsigned char *pBuffer,bool bLimitedFrame,int iPriority)
{
	uint32_t tm(0);
	if (track==2) {
		VS_NhpVideoHeader* hdr = (VS_NhpVideoHeader*)pBuffer;
		DWORD dTime = hdr->TimeStamp - m_PrevVideoTime;
		m_PrevVideoTime = hdr->TimeStamp;
		if (dTime > 1000)
			dTime = 1000;
		if (hdr->FrameType==0)
			m_StoredKey.Set(pBuffer+6, Size-6);
		if (!m_FrameQueue)
			m_ConnectStatus.bWaitCompress = false;
		if (!m_pProtocol->IsTrackSent(2)) {
			m_ConnectStatus.bWaitCompress = false;
			return 0;
		}
		tm = hdr->TimeStamp;
	}
	else if (track==1) {
		if (!m_pProtocol->IsTrackSent(1))
			return 0;
	}
	else if (track == 5) {
		m_ConnectStatus.HalfVideo = true;
		if (!m_pProtocol->IsTrackSent(5))
			return 0;
	}

	if (!m_Connected)
		return 0;
	if (!m_FrameQueue || !m_FrameQueue->AddFrame(track, Size, pBuffer, iPriority)) {
		return -1;
	} else {
		if (track == 2) {
			m_ConnectStatus.bWaitCompress = false;
			m_ConnectStatus.QueueVideoTimestamps.push(tm);
		}
	}

	DWORD CurrBand = 0;
	DWORD CurrTime = timeGetTime();
	if (m_BrAutoDisabled)
		CurrBand = CurrentBandwidth();
	else if (m_BandContr)
		CurrBand = m_BandContr->GetVideoBandwidth(CurrentBandwidth());
	if (CurrBand != m_ConnectStatus.iSwitchBitrate && CurrTime - m_ConnectStatus.SwitchVModeTime > 15000) {
		CheckSwitchVideoMode(CurrBand, CurrTime);
	}
	if (m_ConnectStatus.bUpdateBitrate ||
		CurrBand != m_ConnectStatus.iBitrate || m_iServerBandVideo != m_ConnectStatus.iBaseBitrate || CurrTime - m_ConnectStatus.HalVideoTime > 4000) {
		m_ConnectStatus.iBitrate = CurrBand;
		m_ConnectStatus.iBaseBitrate = m_iServerBandVideo;
		CalcBitrate(m_ConnectStatus.iBitrate, m_ConnectStatus.iBaseBitrate, &m_ConnectStatus.iFPS);
		m_ConnectStatus.HalVideoTime = CurrTime;
		m_ConnectStatus.bUpdateBitrate = false;
	}
	boost::shared_ptr<VS_StreamClientSender> localpSender = m_pSender;
	if(!localpSender)
		return 0;
	m_EventManager.Exclude(localpSender->GetSendEvent(), false);
	return 0;
}

unsigned char ConvertTrack(unsigned char track)
{
	if		(track==NHPH_DT_AUDIO)
		return 1;
	else if (track==NHPH_DT_VIDEO)
		return 2;
	else if (track==NHPH_DT_DATA)
		return 5;
	else
		return 254;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::ProcessSendQueue()
{
	boost::shared_ptr<VS_StreamClientSender> localpSender = m_pSender;
	if (!m_Connected || !localpSender)
		return 0;

	unsigned long iWaitTime = 0;//20;
	unsigned long iTimeOut = iWaitTime;
	unsigned char *pBuffer;
	int size;
	unsigned char q_track;
	unsigned char slayer;

	if (m_scheme_bitrate == 0) {
		if (!m_FrameQueue || m_FrameQueue->GetFrame(pBuffer, size, q_track, slayer)<=0) {
			m_EventManager.Exclude(localpSender->GetSendEvent(), true);
			return 0;
		}
		int res = -1;
		const auto track = static_cast<stream::Track>(m_ConnectStatus.UseNhp ? ConvertTrack(q_track) : q_track);

		if (localpSender) {
			if (size > 0 && m_StreamCrypter.IsValid()) {
				uint32_t encsize = 0x10000;
				if (m_StreamCrypter.Encrypt(pBuffer, size, m_ExtFrameMem, &encsize))
					res = localpSender->SendFrame(m_ExtFrameMem, encsize, track, &iTimeOut);
				else
					res = localpSender->SendFrame(pBuffer, size, track, &iTimeOut);
			}
			else
				res = localpSender->SendFrame(pBuffer, size, track, &iTimeOut);
		}
		unsigned int iTimeTCur = timeGetTime();

		if (res == -1) {
			return -1;
		}
		else if (res==-3) {
			if ((track == stream::Track::video) && (slayer == 0) && m_FrameQueue->MarkFirstAsSend()) {
				m_ConnectStatus.QueueVideoTimestamps.pop();
			}
		}
		else if (res==-2) {
			m_BandContr->Add(iTimeTCur, size, q_track, USHORT(iWaitTime), 1, slayer);
				DTRACE(VSTM_THCL, "localpSender->SendFrame return -2. Resend frame...", m_InterfaceName);
		}
		else {
			m_LastSendTime = iTimeTCur;
			m_BandContr->Add(iTimeTCur, size, q_track, USHORT(iWaitTime - iTimeOut), 0, slayer);
			TrackCallBack.SendTrack(static_cast<uint8_t>(track), size, pBuffer, m_InterfaceName);
			bool isLast = m_FrameQueue->MarkFirstAsSend();
			if (track == stream::Track::video) {
				if (isLast && (slayer == 0)) {
					m_ConnectStatus.QueueVideoTimestamps.pop();
				}
				m_ConnectStatus.LastSendVideo = iTimeTCur;
			}
			return isLast ? q_track : 0;
		}
	} else {
		int iTimeTCur = timeGetTime();
		int res = m_pPacketQueueContr->ProcessSendQueue(iTimeTCur, pBuffer, q_track, slayer, size, m_ConnectStatus.UseNhp);
		const auto track = static_cast<stream::Track>(q_track);

		if (res < -4) {
			res += 3;
		}

		if (res==-4) {
			return 0;
		}
		else if (res==-1) {
			return -1;
		}
		else if (res==-3) {
			if ((track == stream::Track::video) && (slayer == 0) && m_FrameQueue->MarkFirstAsSend()) {
				m_ConnectStatus.QueueVideoTimestamps.pop();
			}
		}
		else if (res==-2) {
			m_BandContr->Add(iTimeTCur, size, q_track, USHORT(iWaitTime), 1, slayer);
		}
		else {
			m_LastSendTime = iTimeTCur;
			m_BandContr->Add(iTimeTCur, size, q_track, USHORT(iWaitTime - iTimeOut), 0, slayer);
			TrackCallBack.SendTrack(static_cast<uint8_t>(track), size, pBuffer, m_InterfaceName);
			bool isLast = m_FrameQueue->MarkFirstAsSend();
			if (track == stream::Track::video) {
				if (isLast && (slayer == 0)) {
					m_ConnectStatus.QueueVideoTimestamps.pop();
				}
				m_ConnectStatus.LastSendVideo = iTimeTCur;
			}
			return isLast ? q_track : 0;
		}
	}
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::CheckSwitchVideoMode(int bitrate, unsigned int ctime)
{
	CVideoCaptureSlotBase *cs = (CVideoCaptureSlotBase *)FindChildrenByName("VideoCaptureSlot");
	if (!cs || !m_bAllowDynChange || !m_DeviceStatus.IsVideo()) return;
	m_pMediaFormatManager->SetControlExternal(CTRL_EXT_BITRATE, bitrate);
	VS_MediaFormat mf;
	m_pMediaFormatManager->GetMediaFormat(&mf_current, &mf);
	if (!(mf_current == mf)) m_ConnectStatus.SwitchVModeTime = ctime;
	SendInfoMediaFormat(&mf);
	ChangeCurrentMedia(mf, DEVICE_CHANGEFORMAT);
	m_ConnectStatus.iSwitchBitrate = bitrate;
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::CalcBitrate(int NeedBand, int BaseBand, int *pFPS)
{
	if (NeedBand < 16) NeedBand = 16;		// set minimum band
	int CurrBand = NeedBand;
	int bDynamicChange = 0;

	if (m_Connected) {
		int abr = m_BandContr->GetCurrBand(VS_ControlBandBase::BT_AUDIO);
		int vbr = m_BandContr->GetCurrBand(VS_ControlBandBase::BT_VIDEO);
		int dbr = m_BandContr->GetCurrBand(VS_ControlBandBase::BT_DATA);

		int quality;
		if (NeedBand >= 128) {
			quality = 8;
		} else if (NeedBand >= 106) {
			quality = 7;
		} else if (NeedBand >= 85) {
			quality = 6;
		} else if (NeedBand >= 64) {
			quality = 5;
		} else {
			quality = 4;
		}
		if (m_last_quality != quality) {
			_variant_t var = quality;
			Apply2Children(IT_AUDIOCAPTURE, SET_PARAM, "CodecQuality", &var);
		}
		m_last_quality = quality;

		if (mf_current.dwSVCMode == 0x0) {
			CurrBand-=abr;						// sub audio from band
			if (dbr > 5*CurrBand/100) { // if data channel is in use, decrease video bitrate && not SVC
				CurrBand = int(CurrBand/(1. + sqrt(CurrBand/128.)) + 0.5);
				m_ConnectStatus.HalfVideo = true;
			}
			else {
				m_ConnectStatus.HalfVideo = false;
			}
		} else {
			m_ConnectStatus.HalfVideo = false;
		}

		DTRACE(VSTM_THCL, "stat: a = %3d, v = %3d, d = %3d, q = %d", abr, vbr, dbr, m_last_quality);
		if (CurrBand < 10)						// minimum bitrate for video
			CurrBand = 10;
		bDynamicChange = (m_pProtocol->m_Status.CurrConfInfo->ClientCaps.GetVideoRcv() & VSCC_VIDEO_DYNCHANGE);
	}

	int FPSLimit = std::min(std::min(m_ConnectStatus.iRealFPS, m_iMaxFPS), (int)(mf_current.dwFps * 100));
	FPSLimit = ((FPSLimit+50)/100)*100;		// rounding to nearest integral fps
	if (FPSLimit < 500) FPSLimit = 500;		// stupid check if m_ConnectStatus.iRealFPS==0
	double MinBitrate = m_iMinBitrate*sqrt(mf_current.dwVideoHeight*mf_current.dwVideoWidht/320./240.);
	if (MinBitrate < 10) MinBitrate = 10;	// total restrict for MinBitrate
	double factor;
	int CodecBitrate = CurrBand;
	int FPS;
	bool bScreenCapturer = false;

	CVideoCaptureSlotBase *cs = (CVideoCaptureSlotBase *)FindChildrenByName("VideoCaptureSlot");
	if (cs) {
		CVideoCaptureList::eCapturerType typeCapturer = cs->GetTypeCapturer();
		bScreenCapturer = typeCapturer == CVideoCaptureList::CAPTURE_SCREEN;
	}

	if (mf_current.dwSVCMode & 0x00030000) {
		FPS = FPSLimit;
		CodecBitrate = BaseBand;
		m_PrevBandwith = NeedBand;
	} else if (bScreenCapturer) {
		double kMB = 0.0084279835; /// kbps on 1 macroblock
		int frameMB = mf_current.dwVideoWidht * mf_current.dwVideoHeight / 256;
		FPS = (int)((double)CurrBand / kMB / (double)frameMB * 100.0);
		FPS = std::max(500, std::min(FPSLimit, FPS));
	} else if (m_bAllowDynChange && (mf_current.dwVideoWidht > 640 && mf_current.dwVideoHeight > 480)) {
		FPS = FPSLimit;
		m_PrevBandwith = NeedBand;
	} else if (mf_current.dwVideoCodecFCC == VS_VCODEC_XC02 ||
		mf_current.dwVideoCodecFCC == VS_VCODEC_H261 || mf_current.dwVideoCodecFCC == VS_VCODEC_H263 || mf_current.dwVideoCodecFCC == VS_VCODEC_H263P) {
		CurrBand*=1000;							// scale
		factor = pow(0.6*m_CurrFPSvsBr, 1.5);
		CodecBitrate = (int)((sqrt(1440000. + 4.*factor*16.8*CurrBand) - 1200.)/(2.*factor+1.) + 1.);
		if (CodecBitrate < MinBitrate)
			CodecBitrate = MinBitrate;
		FPS = (int)(CurrBand /CodecBitrate) + 10;
		if (FPS > FPSLimit) {
			FPS = FPSLimit;
		}
		CurrBand /= 1000;
		CodecBitrate = CurrBand;
		m_PrevBandwith = NeedBand;
	} else if (mf_current.dwVideoCodecFCC == VS_VCODEC_H264) {
		double base_square = 320.0 * 240.0,
			   cur_square = mf_current.dwVideoHeight * mf_current.dwVideoWidht;
		factor = log(base_square / cur_square) * 30.0 / 6.0 + 1.0;
		int FPSf = (int)(100 * (4.0 * factor - 1.0) / 3.0);
		if (FPSf < 100) FPSf = 100;
		if (FPSf > 500) FPSf = 500;
		if (FPSLimit > 3000) FPSLimit = 3000;
		FPS = (int)(100 * pow(CurrBand, sqrt(2.0) * 3.1416 * (37.0 - factor) / 222.0) / 3.0);
		if (FPS < FPSf) FPS = FPSf;
		if (CurrBand > m_PrevBandwith) {
			if (FPS > 2200) FPS = FPSLimit;
		} else {
			if (FPS > 1700 && *pFPS == FPSLimit) FPS = FPSLimit;
		}
		if (FPS > FPSLimit) FPS = FPSLimit;
		m_PrevBandwith = CurrBand;
	} else if (mf_current.dwVideoCodecFCC == VS_VCODEC_VPX ||
			   mf_current.dwVideoCodecFCC == VS_VCODEC_VPXHD ||
			   mf_current.dwVideoCodecFCC == VS_VCODEC_VPXSTEREO) {

		double base_square = 320.0 * 240.0,
			   cur_square = mf_current.dwVideoHeight * mf_current.dwVideoWidht;
		factor = cur_square / base_square;

		/// fps(f, btr) = k(f) * btr + (b(f) - 10)

		double k = 0.204 * pow(factor, -0.77);
		double b = 20.99 * pow(factor, -0.32) - 10.0;
		if (factor <= 2) b = 3;
		b = std::max(b, 0.0);
		//double k = 0.213 * pow(factor, -0.87);
		//double b = 2.96;
		int FPSf = int(25 * factor + 300);
		if (FPSf < 100) FPSf = 100;
		if (FPSLimit > 3000) FPSLimit = 3000;
		FPS = (int)((k * CurrBand + b) * 100 + 0.5);
		FPS = std::max(FPSf, std::min(FPS, FPSLimit));
	} else {
		FPS = FPSLimit;
	}
	if (m_FPSFixMax) {
		unsigned int fixfps = m_FPSFixMax;
		if (fixfps > 60)
			fixfps = 60;
		FPS = fixfps*100;
	}

	*pFPS = FPS;
    _variant_t var = FPS;
	Apply2Children(IT_VIDEOCAPTURE, SET_PARAM, (char*)VS_CaptureDevice::_funcRealFramerate, &var);
	m_pVideoCompressor->SetBitrate(CodecBitrate, CurrBand, FPS);
	DTRACE(VSTM_THCL, "NeedBand = %d, br = %d, fps = %d", NeedBand, CodecBitrate, FPS);
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::QueryExternalFrame()
{
	if (m_Connected) {
		if (!m_BandContr || !m_BandContr->IsDataAllowed(m_iServerBand))
			return;
		if (!m_FrameQueue || (m_FrameQueue->GetSize() > SEND_QUEUE_SIZE / 2))
			return;
		int iSize, ID=-1;
		TrackCallBack.ExternalTrack(&ID, &iSize, m_ExtFrameMem, m_InterfaceName);
		if (ID > 0) {
			ID&=0xff;
			unsigned char *pCodeBuff = m_ExtFrameMem+0x10000;
			int iSize_ = iSize;
			iSize = RCDV_Encode(m_ExtFrameMem, iSize, pCodeBuff+2);
			if (iSize + 4 > iSize_ + 2) {
				// do not compress
				*(unsigned short *)pCodeBuff = ID;
				memcpy(pCodeBuff+2, m_ExtFrameMem, iSize_);
				AddFrameToQueue(DATA_TRACK, iSize_+2, pCodeBuff, false, FRAME_PRIORITY_DATA);
			}
			else {
				*(unsigned short *)pCodeBuff = ID|0x0200; // set rcdv flag
				*(unsigned short *)(pCodeBuff+iSize+2) = iSize_;
				AddFrameToQueue(DATA_TRACK, iSize+4, pCodeBuff, false, FRAME_PRIORITY_DATA);
			}
		}
	}
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::AnalyseQueueVideoFrame()
{
	if (m_Connected) {
		CVideoCaptureSlotBase *cs = (CVideoCaptureSlotBase *)FindChildrenByName("VideoCaptureSlot");
		if (!cs) return;
		if ((eHardwareEncoder)mf_current.dwHWCodec == ENCODER_H264_LOGITECH) {
			if (m_ConnectStatus.bSkipNextFrame && m_BandContr->GetLoadQueue() < 10) {
				m_ConnectStatus.bSkipNextFrame = false;
				if (m_ConnectStatus.bGrabFrame) SetEvent(cs->GetVideoEvent());
				m_ConnectStatus.bGrabFrame = false;
				m_ConnectStatus.bNeedKeyFrame = true;
			}
		}
	}
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::Ping(DWORD CurrTime) {
	if (CurrTime-m_LastSendTime > SNDPING_TIME) {
		if (m_ConnectStatus.UseNhp) {
			stream::Command cmd;
			cmd.Ping();
			g_cmdProc.AddCommand(cmd);
		}
		else {
			AddFrameToQueue(255, 0, 0, false, FRAME_PRIORITY_COMMAND);
		}
		m_LastSendTime = CurrTime;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
unsigned long CThinkClientSender::CurrentBandwidth()
{
	if (m_Connected) {
		int br = std::min(m_iBandwidth, m_iOtherSideBand);
		br = std::min(br, m_iServerBand);
		return br;
	}
	else
		return m_iOtherSideBand;
}

/**
 **************************************************************************
 ****************************************************************************/
unsigned long CThinkClientSender::SetCurrentFpsVsQ(unsigned long val)
{
	if (val<8) val = 8;
	else if (val>80) val = 80;
	m_CurrFPSvsBr = val;
	return val;
}

/**
**************************************************************************
****************************************************************************/
bool CThinkClientSender::GetBlockVideoCapture(uint32_t timestamp)
{
	bool skip(false);
	uint32_t ftm(0), dct(0);
	size_t numframes(m_ConnectStatus.QueueVideoTimestamps.size());
	if (numframes > 0) {
		ftm = m_ConnectStatus.QueueVideoTimestamps.front();
		if (m_ConnectStatus.LastSendVideo > 0) {
			dct = timeGetTime() - m_ConnectStatus.LastSendVideo;
		}
	}
	if (dct >= 2000) {
		skip = true;
		DTRACE(VSTM_BTRC, " Force block video frame : dt = %u", dct);
	}
	else if (m_ConnectStatus.iNumDropFrames == 0) {
		uint32_t dt(0);
		int32_t calcDecimate(0);
		if (ftm > 0) {
			dt = timestamp - ftm;
			calcDecimate = static_cast<int32_t>(2.0f * (static_cast<float>(dt) / 1000.0f + 0.5f)) - 1;
		}
		m_ConnectStatus.iNumDropFrames = std::max(calcDecimate, static_cast<int32_t>(numframes) / 10);
		DTRACE(VSTM_BTRC, " Not block video frame : f = %2d max[calc = %2d, num = %2d], dt = %6u, dct = %6u", m_ConnectStatus.iNumDropFrames, calcDecimate, static_cast<int32_t>(numframes) / 10, dt, dct);
	}
	else {
		DTRACE(VSTM_BTRC, " Block video frame : f = %2d", m_ConnectStatus.iNumDropFrames);
		m_ConnectStatus.iNumDropFrames--;
		skip = true;
	}
	return skip;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::QueueProcess(void*pParam,void*pParam1)
{
	CThinkClientSender *pSelf=(CThinkClientSender *)pParam;
	int res=pSelf->ProcessSendQueue();
	if (res==-1) {// Стрим порвался :(
		if (pSelf->m_Connected) {
			pSelf->Disconnect();
			PostMessage(pSelf->m_hReportHwnd , WM_USER+17, 3, NULL);
		}
		return 0;
	}
	if (!pSelf->m_Connected)
		return 0;
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::QueueFirstProcess(void*pParam,void*pParam1)
{
	CThinkClientSender *pSelf=(CThinkClientSender *)pParam;
	pSelf->AddFrameToQueue(255, 0, 0, false, FRAME_PRIORITY_COMMAND);
	pSelf->ProcessSendQueue();
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::QueryKeyProcess(void*pParam,void*pParam1){
	CThinkClientSender *pSelf=(CThinkClientSender *)pParam;
	unsigned char c;
	pSelf->AddFrameToQueue(255,1,&c,false,FRAME_PRIORITY_COMMAND);
	return 0;
}


/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::DieProcess(void*pParam,void*pParam1){
	CThinkClientSender *pSelf=(CThinkClientSender *)pParam;
	return -1;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::AudioProcess(void*pParam,void*pParam1)
{
	CThinkClientSender *pSelf=(CThinkClientSender *)pParam;
	CAudioCaptureSlot *pAC=(CAudioCaptureSlot *)pParam1;
	char *buff = (char *)malloc(0x1000);
	int size = 0;
	*(DWORD*)buff = timeGetTime();
	while(1) {
		pAC->Capture(buff+4, size, pSelf->m_DeviceStatus.IsAudio());
		if (!size)
			break;
		if (!pSelf->m_Connected && pSelf->m_ConnectStatus.iSkipAudio>0 && pSelf->m_ConnectStatus.iVideoFrameNum==0 && pSelf->m_DeviceStatus.IsVideo()) {
			pSelf->m_ConnectStatus.iSkipAudio--;
			continue;
		}
		pSelf->m_ConnectStatus.iSkipAudio=0;
		if (pSelf->m_DeviceStatus.IsAudio())
			pSelf->AddFrameToQueue(1, size+4, (unsigned char*)buff, false, FRAME_PRIORITY_AUDIO);
	}
	free(buff);
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::VideoFrameProcess(void*pParam,void*pParam1){
	CThinkClientSender *pSelf=(CThinkClientSender *)pParam;

	unsigned int timestamp = 0;
	unsigned char *pRenderFrame = 0;
	CVideoCaptureSlotBase *pVC = (CVideoCaptureSlotBase*)pParam1;
	unsigned char *pBuff = NULL;
	eHardwareEncoder typeEnc = (eHardwareEncoder)pSelf->mf_current.dwHWCodec;

	if (typeEnc != ENCODER_H264_LOGITECH) {
		pBuff = pVC->GetVideoFrame(&pSelf->m_ConnectStatus.iRealFPS, &timestamp, pRenderFrame);
		if (pBuff) {
			bool bSkip = false;
			DWORD CurrTime = timeGetTime();
			int decimateFactor = 0;
			if (pSelf->m_MediaConnected && !pSelf->m_ConnectStatus.bWaitCompress && pSelf->m_DeviceStatus.IsVideo()) {
				bSkip = pSelf->GetBlockVideoCapture(timestamp);
				if (!bSkip) {
					bool bKey = false;
					if ((pSelf->m_ConnectStatus.bNeedKeyFrame && CurrTime - pSelf->m_KeyRepeatTime > 2000) ||
						CurrTime - pSelf->m_KeyRepeatTime > pSelf->m_KeyRepeatPeriod) {
						pSelf->m_ConnectStatus.bNeedKeyFrame = false;
						pSelf->m_KeyRepeatTime = CurrTime;
						bKey = true;
					}
					pSelf->m_pVideoCompressor->CompressFrame(pBuff, bKey, timestamp);
					pSelf->m_ConnectStatus.bWaitCompress = true;
					g_AviWriterGroup->PutVideo(pSelf->m_CallId, pBuff, (pSelf->mf_current.dwVideoHeight * pSelf->mf_current.dwVideoWidht * 3) / 2, 0);
				}
			}
			if (!bSkip) {
				pVC->DrawFrame();
			}
		}
	} else {
		pBuff = pVC->GetVideoFrame(&pSelf->m_ConnectStatus.iRealFPS, &timestamp, pRenderFrame);
		if (pBuff) {
			bool bDraw = true;
			if (pSelf->m_MediaConnected) {
				if (pSelf->m_DeviceStatus.IsVideo()) {
					bDraw = false;
					if (!pSelf->m_ConnectStatus.bSkipNextFrame) {
						DWORD CurrTime = timeGetTime();
						bool bKey = false;
						if (pSelf->m_ConnectStatus.bNeedKeyFrame) {
							pSelf->m_ConnectStatus.bNeedKeyFrame = false;
							pSelf->m_KeyRepeatTime = CurrTime;
							bKey = true;
						}
						pSelf->m_pVideoCompressor->CompressFrame(pBuff, bKey, timestamp);
						if (!pSelf->m_ConnectStatus.bWaitKeyFrame || bKey) {
							if (bKey) {
								pSelf->m_ConnectStatus.bNeedKeyFrame = false;
							}
							pSelf->m_ConnectStatus.bWaitKeyFrame = false;
							VideoCompressProcess(pSelf, pSelf->m_pVideoCompressor);
							if (pRenderFrame) {
								g_AviWriterGroup->PutVideo(pSelf->m_CallId, pRenderFrame, (pSelf->mf_current.dwVideoHeight * pSelf->mf_current.dwVideoWidht * 3) / 2, 0);
							}
							bDraw = true;
						}
						if (pSelf->m_BandContr->GetLoadQueue() >= 30 && !pSelf->m_ConnectStatus.bSkipNextFrame) {
							pSelf->m_ConnectStatus.bSkipNextFrame = true;
						}
					}
				} else {
					pSelf->m_ConnectStatus.bSkipNextFrame = true;
					pSelf->m_ConnectStatus.bWaitKeyFrame = true;
				}
			}
			if (bDraw) pVC->DrawFrame();
		}
	}

	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::VideoCompressProcess(void*pParam,void*pParam1){
	CThinkClientSender *pSelf=(CThinkClientSender *)pParam;
	int s;
	unsigned char *c;
	bool bKey;
	if (!pSelf->m_MediaConnected)
		return 0;
	if (!pSelf->m_pVideoCompressor->GetResult()){
		pSelf->m_ConnectStatus.DropVideoFrame();
		return 0;
	}
	int w = 0, h = 0;
	c = pSelf->m_pVideoCompressor->GetCompressedData(&s,&bKey,&w,&h);
	if (w != -1 && h != -1) {
		VS_MediaFormat mf;
		mf.dwVideoWidht = w;
		mf.dwVideoHeight = h;
		pSelf->NotifyResolution(&mf);
	}
	if (s == 6) { /// only header
		pSelf->m_ConnectStatus.bWaitCompress = false;
		return 0;
	}
	if (pSelf->AddFrameToQueue(2, s, c, true, FRAME_PRIORITY_VIDEO) < 0) {
		pSelf->m_ConnectStatus.DropVideoFrame();
	}
	else {
		pSelf->m_ConnectStatus.iVideoFrameNum++;
	}
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::ReceivedCommandProcess(void*pParam, void*pParam1)
{
	CThinkClientSender *pSelf = (CThinkClientSender *)pParam;
	VS_CmndProc *prc =  (VS_CmndProc*)pParam1;
	if (pSelf->m_Connected){
		stream::Command cmd;
		while (prc->ReadRcvCommand(cmd)) {
			// info commands
			auto result = stream::Command::UnknownError;
			if (cmd.type == stream::Command::Type::RequestKeyFrame) {
				pSelf->m_ConnectStatus.bNeedKeyFrame = true;
			}
			else if (cmd.type == stream::Command::Type::RestrictBitrate) {
				if (cmd.data_size == 4) {
					pSelf->m_iServerBand = *reinterpret_cast<uint32_t*>(cmd.data);
					pSelf->m_iServerBandVideo = pSelf->m_iServerBand;
					result = stream::Command::OK;
				}
			}
			else if (cmd.type == stream::Command::Type::RestrictBitrateSVC) {
				if (cmd.data_size == 8) {
					pSelf->m_iServerBand = reinterpret_cast<uint32_t*>(cmd.data)[0];
					pSelf->m_iServerBandVideo = reinterpret_cast<uint32_t*>(cmd.data)[1];
					result = stream::Command::OK;
				}
			}
			else if (cmd.type == stream::Command::Type::SetFPSvsQ) {
				if (cmd.data_size == 4) {
					long CurrKoef = *reinterpret_cast<uint32_t*>(cmd.data);
					pSelf->SetCurrentFpsVsQ(CurrKoef);
					result = stream::Command::OK;
				}
			}
			else if (cmd.type == stream::Command::Type::ChangeSndMFormat) {
				if (cmd.data_size > 0 && cmd.data_size <= sizeof(stream::Command) - stream::Command::header_size) {
					VS_MediaFormat fmt;
					fmt = *reinterpret_cast<VS_MediaFormat *>(cmd.data); // any future mediaformat fields are not copied
					pSelf->ChangeCurrentMediaCommand(fmt, DEVICE_CHANGEFORMAT);
					result = stream::Command::OK;
				}
			}
			if (pSelf->m_BandContr)
				pSelf->m_BandContr->SetReceivedCommand(cmd);
			if (cmd.sub_type == stream::Command::Request) {
				cmd.MakeReply(result);
				prc->AddCommand(cmd);
			}
		}
		unsigned char buff[sizeof(stream::Command)];
		long size = sizeof (buff);
		while (prc->GetSndCommand(buff, size) && size>0) {
			pSelf->AddFrameToQueue(254, size, buff, false, FRAME_PRIORITY_COMMAND);
		}
	}
	return 0;
}

/**************************************************************************
****************************************************************************/
int CThinkClientSender::CheckVideoModeProcess(void*pParam, void*pParam1)
{
	CThinkClientSender *pSelf=(CThinkClientSender *)pParam;
	pSelf->ChangeVideoMode();
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::CheckConnectDeviceProcess(void*pParam, void*pParam1)
{
	CThinkClientSender *pSelf = (CThinkClientSender *)pParam;
	pSelf->ConnectVideoDevice();
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::CheckDisconnectDeviceProcess(void*pParam, void*pParam1)
{
	CThinkClientSender *pSelf = (CThinkClientSender *)pParam;
	pSelf->DisconnectVideoDevice();
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::CheckRestrictMediaFormat(void*pParam, void*pParam1)
{
	CThinkClientSender *pSelf = (CThinkClientSender *)pParam;
	pSelf->RestrictMediaFormat();
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::InfoMediaFormatProcess(void*pParam, void*pParam1)
{
	CThinkClientSender *pSelf = (CThinkClientSender *)pParam;
	pSelf->SendInfoMediaFormat();
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;
    if (strncmp(pSection,_funcSetStretch,sizeof(_funcSetStretch))==0){
		switch(VS_OPERATION)
		{
		case SET_PARAM:
			//AddAudioCapture((char*)(_bstr_t)var);
			CTransDIBRender::m_StretchFunc=(tTransparentStretch *)(int)(*var);
			return VS_INTERFACE_OK;
		}
	}
	if (strncmp(pSection,_funcAddAC,sizeof(_funcAddAC))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			AddAudioCapture((char*)(_bstr_t)var);
			return VS_INTERFACE_OK;
		}
	}
	else if (strncmp(pSection,_funcAddACExt,sizeof(_funcAddACExt))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			AddAudioCapture((char*)(_bstr_t)var, true);
			return VS_INTERFACE_OK;
		}
	}
	else if (strncmp(pSection,_funcRemoveAC,sizeof(_funcRemoveAC))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			RemoveAudioCapture((char*)(_bstr_t)*var);
			return VS_INTERFACE_OK;
		}
	}
	else if (strncmp(pSection,_funcAddVCExt,sizeof(_funcAddVCExt))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			if(var->vt==(VT_ARRAY|VT_VARIANT)){
				long l,u;
				SAFEARRAY *psa=var->parray;
				SafeArrayGetLBound(psa,1,&l);
				SafeArrayGetUBound(psa,1,&u);
				if(u-l==1){
					VARIANT vr,vr_h;
					TVideoWindowParams VParams;
					SafeArrayGetElement(psa,&l,&vr);
					l++;
					SafeArrayGetElement(psa,&l,&vr_h);
					VParams.hWindowHandle=(HWND)vr_h.intVal;
					AddVideoCapture((char*)(_bstr_t)vr,&VParams,true);
					_variant_t var_,var2;
					l=0;
					var_=(int)VParams.wndProc;
					SafeArrayPutElement(psa, &l, &var_.GetVARIANT());
					l++;
					var2=(int)VParams.lpInstance;
					SafeArrayPutElement(psa, &l, &var2.GetVARIANT());
					return VS_INTERFACE_OK;
				}
				return VS_INTERFACE_INTERNAL_ERROR;
			}
		}
	}
	else if (strncmp(pSection,_funcAddVC,sizeof(_funcAddVC))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			if(var->vt==(VT_ARRAY|VT_VARIANT)){
				long l,u;
				SAFEARRAY *psa=var->parray;
				SafeArrayGetLBound(psa,1,&l);
				SafeArrayGetUBound(psa,1,&u);
				if(u-l==1){
					VARIANT vr,vr_h;
					TVideoWindowParams VParams;
					SafeArrayGetElement(psa,&l,&vr);
					l++;
					SafeArrayGetElement(psa,&l,&vr_h);
					VParams.hWindowHandle=(HWND)vr_h.intVal;
					AddVideoCapture((char*)(_bstr_t)vr,&VParams);
					_variant_t var_,var2;
					l=0;
					var_=(int)VParams.wndProc;
					SafeArrayPutElement(psa, &l, &var_.GetVARIANT());
					l++;
					var2=(int)VParams.lpInstance;
					SafeArrayPutElement(psa, &l, &var2.GetVARIANT());
					return VS_INTERFACE_OK;
				}
				return VS_INTERFACE_INTERNAL_ERROR;
			}
		}
	}
	else if (strncmp(pSection,_funcRemoveVC,sizeof(_funcRemoveVC))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			RemoveVideoCapture((char*)(_bstr_t)*var);
			return VS_INTERFACE_OK;
		}
	}
	else if (strncmp(pSection,_funcQueryKey,sizeof(_funcQueryKey))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			if(m_Connected) {
				SetEvent(m_hSendKeyReq);
				return VS_INTERFACE_OK;
			}
		}
	}
	else if(strncmp(pSection,_funcFPSRate,sizeof(_funcFPSRate))==0){
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			*var=m_FPSvsBr;
			return VS_INTERFACE_OK;
		case SET_PARAM:
			m_FPSvsBr=*var;
			return VS_INTERFACE_OK;
		}
	}
	else if(strncmp(pSection,_funcEnabledDevices,sizeof(_funcEnabledDevices))==0){
		switch(VS_OPERATION)
		{
		case GET_PARAM:{
			long i;
			SAFEARRAYBOUND rgsabound[1];
			SAFEARRAY * psa;
			rgsabound[0].lLbound = 0;
			rgsabound[0].cElements = 2;
			psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
			if(psa == NULL)
				return VS_INTERFACE_INTERNAL_ERROR;
			var->parray=psa;
			var->vt= VT_ARRAY | VT_VARIANT;
			_variant_t var_;
			i=0;
			var_=m_DeviceStatus.bAudioValid;
			SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
			i++;
			var_=m_DeviceStatus.bVideoValid;
			SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
			return VS_INTERFACE_OK;
					   }
		case SET_PARAM:
			if(var->vt==(VT_ARRAY|VT_VARIANT)){
				long l,u;
				SAFEARRAY *psa=var->parray;
				SafeArrayGetLBound(psa,1,&l);
				SafeArrayGetUBound(psa,1,&u);
				if(u-l==1){
					VARIANT vr;
					SafeArrayGetElement(psa,&l,&vr);
					m_DeviceStatus.bUseAudio = vr.boolVal!=0;
					g_DevStatus.SetStatus(DVS_SND_PAUSED, true, vr.boolVal==0);
					l++;
					SafeArrayGetElement(psa,&l,&vr);
					m_DeviceStatus.bUseVideo = vr.boolVal!=0;
					g_DevStatus.SetStatus(DVS_SND_PAUSED, false, vr.boolVal==0);
					NotifyResolution(NULL);
					return VS_INTERFACE_OK;
				}
				return VS_INTERFACE_INTERNAL_ERROR;
			}
		}
	}
	else if(strncmp(pSection,_funcBandwidth,sizeof(_funcBandwidth))==0){
		switch(VS_OPERATION)
		{
		case SET_PARAM:
			m_iBandwidth = int(*var);
			if (m_iBandwidth < 32)
				m_iBandwidth = 32;
			return VS_INTERFACE_OK;
		case GET_PARAM:
			*var = m_iBandwidth;
			return VS_INTERFACE_OK;
		case RUN_COMMAND:
			{
				CVSInterface* face = GetParent();
				if (face) {
					return face->Process(RUN_COMMAND, pSection, pVar);
				}
				else {
					*var = 0;
					return VS_INTERFACE_OK;
				}
			}
		}
	}
	else if(strncmp(pSection,_funcFormat,sizeof(_funcFormat))==0){
		switch(VS_OPERATION)
		{
		case GET_PARAM:{
			long i;
			SAFEARRAYBOUND rgsabound[1];
			SAFEARRAY * psa;
			rgsabound[0].lLbound = 0;
			rgsabound[0].cElements = 6;
			psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
			if(psa == NULL)
				return VS_INTERFACE_INTERNAL_ERROR;
			var->parray=psa;
			var->vt= VT_ARRAY | VT_VARIANT;
			_variant_t var_;
			i=0;
			var_=mf_current.dwVideoWidht;
			SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
			i++;
			var_=mf_current.dwVideoHeight;
			SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
			i++;
			var_=mf_current.dwAudioSampleRate;
			SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
			i++;
			var_=mf_current.dwAudioCodecTag;
			SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
			i++;
			var_=mf_current.dwAudioBufferLen;
			SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
			i++;
			var_=mf_current.dwVideoCodecFCC;
			SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
			return VS_INTERFACE_OK;
					   }
		}
	}
	else if (strcmp(pSection, "PacketLimit")==0) {
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			int packet_l = VIDEODATASIZE_MAX;
			boost::shared_ptr<VS_StreamClientSender> local = m_pSender ? m_pSender : m_pProtocol->m_Status.CurrConfInfo->Snd;
			bool bNhp = (local) ? local->ConnectType() == vs_stream_client_connect_not_guaranteed : false;
			if (bNhp) packet_l = VIDEODATASIZE_DEFAULT;
			VS_RegistryKey key(true, REG_CurrentConfiguratuon);
			key.GetValue(&packet_l, sizeof(int), VS_REG_INTEGER_VT, "PacketLimit");
			if (packet_l < VIDEODATASIZE_MIN) packet_l = VIDEODATASIZE_MIN;
			if (packet_l > VIDEODATASIZE_MAX) packet_l = VIDEODATASIZE_MAX;
			*var = packet_l;
			return VS_INTERFACE_OK;
		}
	}
	else if (strcmp(pSection, "AviWriter")==0) {
		if (m_Connected) return VS_INTERFACE_INTERNAL_ERROR;
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			{
				bool res = false;
				VARIANT *vars = 0;
				if (var->vt == VT_I4) {
					if	(var->lVal == 0) {
						if (g_AviWriterGroup->Pause()) {
							g_AviWriterGroup->Close();
							ReleaseCurrentMedia();
							_variant_t var = m_vmf.dwFps * 100;
							Apply2Children(IT_VIDEOCAPTURE, SET_PARAM, (char*)VS_CaptureDevice::_funcRealFramerate, &var);
							res = true;
						}
					}
				} else {
					int num = ExtractVars(vars, var);
					if (num == 3) {
						if (vars[0].lVal == 1) {
							if (g_AviWriterGroup->Init((wchar_t*)(_bstr_t)vars[1])) {
								if (g_AviWriterGroup->Start()) {

									srand(timeGetTime());
									m_iOtherSideBand = g_AviWriterGroup->GetInitBitrateMode();
									m_iMaxFPS = 3000;
									m_iMinBitrate = 30;
									m_KeyRepeatPeriod = 30000 + (rand()&0xfff);
									memset(m_stat, 0, sizeof(TServerStatistics));
									RstConfStat();

									m_ConnectStatus.Clear();
									m_ConnectStatus.iBitrate = g_AviWriterGroup->GetInitBitrateMode();
									m_ConnectStatus.iFPS = m_iMaxFPS;
									m_ConnectStatus.iRealFPS = m_iMaxFPS;
									SetCurrentFpsVsQ(m_FPSvsBr);

									if (!m_MediaConnected) {
										SetCurrentMedia(m_vmf);
										_variant_t var = g_AviWriterGroup->GetInitFramerateMode() * 100;
										Apply2Children(IT_VIDEOCAPTURE, SET_PARAM, (char*)VS_CaptureDevice::_funcRealFramerate, &var);
									}
									if (g_AviWriterGroup->OpenStream(m_CallId, (char*)(_bstr_t)vars[2]) == 0) {
										m_ConnectStatus.bWaitCompress = false;
										res = true;
									}
								}
							}
						}
					}
					if (num > 0) delete[] vars;
				}
				return res ? VS_INTERFACE_OK : VS_INTERFACE_INTERNAL_ERROR;
			}
		}
	}
	else if (strcmp(pSection, "IsPodiumOn")==0) {
		if (VS_OPERATION == SET_PARAM) {
			m_TypeOfAutoPodium = *var;
			return VS_INTERFACE_OK;
		}
		else if (VS_OPERATION == GET_PARAM) {
			int valid = 0;
			if (m_TypeOfAutoPodium == 1) {
				VS_RegistryKey key(true, REG_CurrentConfiguratuon);
				long type = 0;
				valid = key.GetValue(&type, 4, VS_REG_INTEGER_VT, "IsPodiumOn") > 0;
			}
			else if (m_TypeOfAutoPodium == 2) {
				if (!m_PhoenixDev || !m_PhoenixDev->IsValid()) {
					if (m_PhoenixDev) delete m_PhoenixDev;

					VS_RegistryKey key(true, REG_CurrentConfiguratuon);
					long hdev = TC_PhnxHid::DT_MT202_PCS_Duet_PCS;
					key.GetValue(&hdev, 4, VS_REG_INTEGER_VT, "HidDevice");
					m_PhoenixDev = new TC_PhnxHid((TC_PhnxHid::DeviceType)hdev);
				}
				valid = m_PhoenixDev->IsValid();
			}
			*var = valid;
			return VS_INTERFACE_OK;
		}
		else if (VS_OPERATION == RUN_COMMAND) {
			int goPodium = 0;
			if (m_TypeOfAutoPodium == 1) {
				VS_RegistryKey key(true, REG_CurrentConfiguratuon);
				key.GetValue(&goPodium, 4, VS_REG_INTEGER_VT, "IsPodiumOn");
			}
			else if (m_TypeOfAutoPodium == 2) {
				if (m_PhoenixDev)
					goPodium = !m_PhoenixDev->GetMute();
			}
			*var = goPodium;
			return VS_INTERFACE_OK;
		}
	}
	return VS_INTERFACE_NO_FUNCTION;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::AddVideoCapture(char *szSlotName,TVideoWindowParams *pVParams,bool bExt)
{
	VS_AutoLock lock(this);

	CVideoCaptureSlotBase *pVC;
#ifdef VS_LINUX_DEVICE
	pVC = new CVideocaptureV4L(szSlotName, this,pVParams, m_pVCaptureList);
#else
	if (bExt)
		pVC = new CVideoCaptureSlotExt(szSlotName,this,pVParams);
	else
		pVC  =new CVideoCaptureSlot(szSlotName, this, pVParams, m_pVCaptureList, m_pMediaFormatManager);
#endif //VS_LINUX_DEVICE
	pVC->_Init(m_vmf, DEVICE_CHANGEFORMAT);
	NotifyResolution(&m_vmf);
	if(pVC->IsValid()){
		m_EventManager.Add(pVC->GetVideoEvent(), VideoFrameProcess, pVC);
		m_EventManager.Add(pVC->GetVideoEventConnect(),CheckConnectDeviceProcess,pVC);
		m_EventManager.Add(pVC->GetVideoEventDisconnect(),CheckDisconnectDeviceProcess,pVC);
		return 0;
	}
	else{
		delete pVC; pVC = 0;
		return -1;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::RemoveVideoCapture(char *szName)
{
#ifdef VZOCHAT7
	VS_AutoLock lock(this);
#endif
    CVideoCaptureSlotBase *pVC=(CVideoCaptureSlotBase *)FindChildrenByName(szName);
	if(pVC!=NULL){
		m_EventManager.Remove(pVC->GetVideoEventConnect());
		m_EventManager.Remove(pVC->GetVideoEventDisconnect());
		m_EventManager.Remove(pVC->GetVideoEvent());
		delete pVC; pVC = 0;
	}
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::AddAudioCapture(char *szSlotName,bool bExt)
{
#ifdef VZOCHAT7
	VS_AutoLock lock(this);
#endif
	CAudioCaptureSlot *pAC;
	if(bExt)
	  pAC=new CAudioCaptureSlotExt(szSlotName,this,m_vmf,m_pACaptureList);
	else
	  pAC=new CAudioCaptureSlot(szSlotName,this,m_vmf,m_pACaptureList);
	pAC->_Init(m_vmf,m_pACaptureList);
	if(pAC->IsValid()){
		m_EventManager.Add(pAC->GetAudioEvent(),AudioProcess, pAC);
		return 0;
	}
	else{
	  pAC->_Release();
      delete(pAC); pAC = 0;
	}
  return -1;
}

/**
 **************************************************************************
 ****************************************************************************/
int CThinkClientSender::RemoveAudioCapture(char *szName)
{
#ifdef VZOCHAT7
	VS_AutoLock lock(this);
#endif
    CAudioCaptureSlot *pAC=(CAudioCaptureSlot *)FindChildrenByName(szName);
	if(pAC!=NULL){
		m_EventManager.Remove(pAC->GetAudioEvent());
		pAC->_Release();
		delete pAC; pAC = 0;
	}
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::RstConfStat()
{
	m_conf_stat.inc_cpu = m_conf_stat.sum_cpu_load =
	m_conf_stat.bytes = m_conf_stat.now_t = m_conf_stat.media_traffic =
	m_conf_stat.num_frames = m_conf_stat.video_h = m_conf_stat.video_w =
	m_conf_stat.started = 0;
	m_conf_stat.start_t = m_conf_stat.last_t = timeGetTime();
	m_conf_stat.start_part_gmt = std::chrono::system_clock::now();
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::EndConfStat()
{
	if (m_conf_stat.started == 1) m_conf_stat.started = 2;
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::NotifyResolution(VS_MediaFormat *mf)
{
	int mode = 0, w = 320, h = 176;
	if (mf) {
		m_mf_notify = *mf;
	}
	if (m_DeviceStatus.bUseVideo) {
		mode = m_mf_notify.GetModeResolution();
		w = m_mf_notify.dwVideoWidht;
		h = m_mf_notify.dwVideoHeight;
	}
	PostMessage(m_hReportHwnd, WM_USER+23, MAKELONG(w, h), MAKELONG(0, mode));
}

/**
 **************************************************************************
 ****************************************************************************/
void CThinkClientSender::SetConfStat(int now_t, int bytes, int type)
{
	VS_ConferenceStatSnd stat;
	int dt = m_conf_stat.GetPeriodCalc();
	if (m_BandContr && dt > 4000) {
		m_BandContr->GetStatistics(&stat, true);
		m_conf_stat.media_traffic = (int)(((double)stat.bytes_cur[0] + (double)stat.bytes_cur[1] + (double)stat.bytes_cur[2] + (double)stat.bytes_cur[3]) * 1000.0 / (double)dt);
		m_conf_stat.last_t = now_t;
	}
	if (m_Connected && (m_conf_stat.started == 0 || m_conf_stat.started == 1)) {
		if (m_BandContr) m_BandContr->GetStatistics(&stat, false);
		m_conf_stat.bytes = (stat.bytes_all[0] + stat.bytes_all[1] + stat.bytes_all[2] + stat.bytes_all[3] + stat.bytes_all[4]);
		m_conf_stat.now_t = now_t;
		m_conf_stat.num_frames = m_ConnectStatus.iVideoFrameNum;
		m_conf_stat.video_w = mf_current.dwVideoWidht;
		m_conf_stat.video_h = mf_current.dwVideoHeight;
		m_conf_stat.end_part_gmt = std::chrono::system_clock::now();
		if (dt > 4000) {
			int cpu_load = static_cast<int>(VS_PerformanceMonitor::Instance().GetTotalProcessorTime());
			m_conf_stat.sum_cpu_load += cpu_load;
			m_conf_stat.inc_cpu++;
			m_conf_stat.last_t = now_t;
			m_pProtocol->SetSystemLoad(cpu_load);
		}
		m_conf_stat.started = 1;
	}
}

/**
 **************************************************************************
 ****************************************************************************/
bool CThinkClientSender::GetConfStat(TConferenceStatistics *cst)
{
	if (m_conf_stat.started == 0) return false;
	cst->participant_time = m_conf_stat.GetPeriodTime();
	cst->avg_send_bitrate = m_conf_stat.bytes;
	cst->avg_send_fps = (double)m_conf_stat.num_frames / ((double)cst->participant_time / 1000.0);
	cst->avg_cpu_load = (m_conf_stat.sum_cpu_load + m_conf_stat.inc_cpu / 2) / (m_conf_stat.inc_cpu + 1);
	cst->video_w = m_conf_stat.video_w;
	cst->video_h = m_conf_stat.video_h;
	cst->start_part_gmt = m_conf_stat.start_part_gmt;
	cst->end_part_gmt = m_conf_stat.end_part_gmt;
	return m_conf_stat.started == 2;
}

/**
 **************************************************************************
 ****************************************************************************/
DWORD CThinkClientSender::Loop(LPVOID hEvDie)
{
	m_EventManager.Add(hEvDie, DieProcess, NULL);

#ifdef VZOCHAT7
	CoInitializeEx(0, COINIT_MULTITHREADED);
#else
	CoInitializeEx(0, COINIT_MULTITHREADED);
#endif

	while(1){
		if (m_scheme_bitrate == 0) {
			m_EventManager.Lock();

			HANDLE aEvents[MAX_EVENT_SENDER] = {0};
			DWORD numOfEv = m_EventManager.GetEvents(aEvents);
			DWORD res = WaitForMultipleObjects(numOfEv, aEvents, FALSE, 200);
			if (res>=WAIT_OBJECT_0 && res<WAIT_OBJECT_0+numOfEv) {
				if (m_EventManager.Process(res-WAIT_OBJECT_0, this)){
					m_EventManager.Unlock();
					break;
				}
			} else if (res == WAIT_TIMEOUT) {
				QueueProcess(this, NULL);
			}
			QueryExternalFrame();
			AnalyseQueueVideoFrame();
			DWORD iTimeTCur = timeGetTime();
			SetConfStat(iTimeTCur);
			if (m_Connected) {
				boost::shared_ptr<VS_StreamClientSender> localpSender = m_pSender;
				if(localpSender) {
					if ( m_EventManager.Update(QueueProcess, localpSender->GetSendEvent()) ) {
						SetEvent(m_hEventFirstSnd);
					}
				}
				if (!m_isIntercom) {
					Ping(iTimeTCur);
				}
			}
			m_EventManager.Unlock();
		} else {
			m_EventManager.Lock();
			HANDLE aEvents[MAX_EVENT_SENDER];
			DWORD numOfEv = m_EventManager.GetEvents(aEvents);
			DWORD res = WaitForMultipleObjects(numOfEv, aEvents, FALSE, 20);
			if (res>=WAIT_OBJECT_0 && res<WAIT_OBJECT_0+numOfEv) {
				if (m_EventManager.Process(res-WAIT_OBJECT_0, this)){
					m_EventManager.Unlock();
					break;
				}
			} else if (res == WAIT_TIMEOUT) {
				QueueProcess(this, NULL);
			}
			QueryExternalFrame();
			DWORD iTimeTCur = timeGetTime();
			SetConfStat(iTimeTCur);
			if (m_Connected) {
				boost::shared_ptr<VS_StreamClientSender> localpSender = m_pSender;
				if(localpSender) {
					if ( m_EventManager.Update(QueueProcess, localpSender->GetSendEvent()) ) {
						SetEvent(m_hEventFirstSnd);
					}
				}
				if (!m_isIntercom) {
					Ping(iTimeTCur);
				}
			}
			m_EventManager.Unlock();
		}
	}

	CoUninitialize();
	return NOERROR;
}


/*****************************************************************************
 * CEventManager
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
int CEventManager::GetEvents(HANDLE *aEvents)
{
	int idx = 0;
	for (int i = 0; i < m_iCount; i++) {
		if (!m_stEvent[i].bExclude) {
			m_idxEcxlude2Real[idx] = i;
			aEvents[idx] = m_stEvent[i].aEvent;
			idx++;
		}
	}
	return idx;
}

/**
 **************************************************************************
 ****************************************************************************/
void CEventManager::Add(HANDLE hEv, tEventSub pEvProc, void* pParam)
{
	int res = WaitForSingleObject(m_hMutex, 1000);
	if (m_iCount < MAX_EVENT_SENDER) {
		m_stEvent[m_iCount].aEvent = hEv;
		m_stEvent[m_iCount].eventSub = pEvProc;
		m_stEvent[m_iCount].eventParam = pParam;
		m_stEvent[m_iCount].bExclude = false;
		m_iCount++;
	}
    if (res == WAIT_OBJECT_0)
		ReleaseMutex(m_hMutex);
}

/**
 **************************************************************************
 ****************************************************************************/
void CEventManager::Remove(HANDLE hEv)
{
	int i;
	int res = WaitForSingleObject(m_hMutex, 1000);
	for (i = 0; i < m_iCount; i++) {
		if (m_stEvent[i].aEvent == hEv) {
			Remove(i);
			break;
		}
	}
    if (res == WAIT_OBJECT_0)
		ReleaseMutex(m_hMutex);
}

/**
 **************************************************************************
 ****************************************************************************/
void CEventManager::Remove(tEventSub pEvProc)
{
	int i;
	int res = WaitForSingleObject(m_hMutex, 1000);
	for (i = 0; i < m_iCount; i++) {
		if (m_stEvent[i].eventSub == pEvProc) {
			Remove(i);
			break;
		}
	}
    if (res == WAIT_OBJECT_0)
		ReleaseMutex(m_hMutex);
}

/**
 **************************************************************************
 ****************************************************************************/
void CEventManager::Remove(int iIndex)
{
	if (iIndex < m_iCount) {
		m_iCount--;
		m_stEvent[iIndex] = m_stEvent[m_iCount];
		memset(&m_stEvent[m_iCount], 0, sizeof(EventState));
	}
}

/**
 **************************************************************************
 ****************************************************************************/
void CEventManager::Exclude(HANDLE hEv, bool exclude)
{
	int res = WaitForSingleObject(m_hMutex, 1000);
	for (int i = 0; i < m_iCount; i++) {
		if (m_stEvent[i].aEvent == hEv) {
			m_stEvent[i].bExclude = exclude;
			break;
		}
	}
    if (res == WAIT_OBJECT_0)
		ReleaseMutex(m_hMutex);
}

/**
 **************************************************************************
 ****************************************************************************/
bool CEventManager::Update(tEventSub pEvProc, HANDLE hEv)
{
	bool ret = false;
	int idx = 0;
	for (int i = 0; i < m_iCount; i++) {
		if (m_stEvent[i].eventSub == pEvProc) {
			if (m_stEvent[i].aEvent != hEv) {
				m_stEvent[i].aEvent = hEv;
				ret = true;
			}
		}
	}
	return ret;
}

/**
 **************************************************************************
 ****************************************************************************/
int CEventManager::Process(int iIndex, void* pParam)
{
	if (iIndex < m_iCount) {
		int realIdx = m_idxEcxlude2Real[iIndex];
		if (m_stEvent[realIdx].eventSub != 0) {
			return m_stEvent[realIdx].eventSub(pParam, m_stEvent[realIdx].eventParam);
		}
	}
	return 0;
}

/*****************************************************************************
 * CVideoSource
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
CVideoSource::CVideoSource()
{
	m_predictor.Init(20);
	m_audiodurr.Init(8);
	Detach();
}

/**
 **************************************************************************
 ****************************************************************************/
CVideoSource::~CVideoSource() {
}

/**
 **************************************************************************
 ****************************************************************************/
bool CVideoSource::AddData(unsigned char* data, int size, unsigned long VideoInterval, bool key, int recordMBs)
{
	if (!m_pVideo)		// not valid or not in VideoSinc
		return true;
	m_recordMBs = recordMBs;
	VS_BinBuff *currBuff = &m_CycleBuffer[m_EndCount%VIDEO_FRAMES_MAX];
	currBuff->SetSize(m_iBufferSize);
	int CurrTime = (int)timeGetTime();
	DTRACE(VSTM_THCL, "%.10s | ct = %9d", m_CallId, CurrTime);
	unsigned char *pBuffer = (unsigned char*)currBuff->Buffer();
	if (!pBuffer)
		return false;
	if (m_pVideo->DecompressFrame(data, size, key, pBuffer))
		return false;
	else {
		int vInt = VideoInterval;
		int CurrBuffDurr = 0;
		if (m_pAudio)
			CurrBuffDurr = m_pAudio->GetCurrBuffDurr();
		if (CurrBuffDurr < 60)
			CurrBuffDurr = 60;
		if (abs(CurrTime - m_LastFillTime) > 2000 || CurrBuffDurr - m_PrevDurDurr > 2000 || vInt >= 1000) // clear stat
		{
			//m_StartCount = 1;
			//m_EndCount = 1;
			m_predictor.Clear();
			m_audiodurr.Clear();
			//if (currBuff != &m_CycleBuffer[1])
				//m_CycleBuffer[1] = *currBuff;
			m_LastFrameTime = CurrTime + CurrBuffDurr;
			DTRACE(VSTM_THCL, "%.10s | Clear Stat vint = %4d, lft=%d \n", m_CallId, vInt, m_LastFrameTime);
			vInt = 0;
		}
		m_PrevDurDurr = CurrBuffDurr;
		int dt = CurrTime - m_LastFillTime;
		m_LastFillTime = CurrTime;
		m_audiodurr.Snap(CurrBuffDurr);
		m_audiodurr.GetAverage(CurrBuffDurr);

		m_predictor.Snap(CurrTime + CurrBuffDurr);

		int DueTime = CurrTime;
		double predictedTime = 0.;
		if (!m_predictor.GetPredictedFromLast(-0.5, predictedTime)) { 	// not enough data
			if (m_EndCount==1) 		// first reference value
				DueTime = m_LastFrameTime;
			else
				DueTime = m_LastFrameTime + vInt;
		}
		else {
			DueTime = int(predictedTime*0.5 - 5.0) + (m_LastFrameTime + vInt)/2;
		}
		if (DueTime > m_LastFrameTime + vInt*2)
			DueTime = m_LastFrameTime + vInt*2;
		if (DueTime < m_LastFrameTime + 10)
			DueTime = m_LastFrameTime + 10;
		if (DueTime < m_LastFrameTime + vInt/2)
			DueTime = m_LastFrameTime + vInt/2;
		m_CycleTime[m_EndCount%VIDEO_FRAMES_MAX] = DueTime;
		m_EndCount+=(m_EndCount-m_StartCount < VIDEO_FRAMES_MAX-1);
		DTRACE(VSTM_THCL, "%.10s | cd= %4d, pred= %4d, dif= %4d, vi= %4d, f=%3d, dt=%3d",
			m_CallId, CurrBuffDurr, DueTime - CurrTime, m_CycleTime[(m_EndCount - 1) % VIDEO_FRAMES_MAX] - m_LastFrameTime, VideoInterval, m_EndCount - m_StartCount, dt);
		m_LastFrameTime = DueTime;
	}

	return true;
}

/**
 **************************************************************************
 ****************************************************************************/

bool CVideoSource::CheckDraw()
{
	int CurrTime = (int)timeGetTime(); // refresh time
	int idx = m_StartCount % VIDEO_FRAMES_MAX;
	while (m_EndCount > (m_StartCount + 1) && CurrTime - m_CycleTime[idx] > 100) { // more than one frame
		DTRACE(VSTM_THCL, "NODRAW, t=%3d", CurrTime - m_CycleTime[idx]);
		m_CycleBuffer[idx].Empty();
		m_StartCount++;
		idx = m_StartCount % VIDEO_FRAMES_MAX;
	}
	if (m_EndCount > m_StartCount && m_CycleTime[idx] < CurrTime) { // there are frame in buffer
		VS_BinBuff *currBuff = &m_CycleBuffer[idx];
		memcpy(m_pOut, currBuff->Buffer(), m_iBufferSize);
		g_AviWriterGroup->PutVideo(m_CallId, (unsigned char*)currBuff->Buffer(), currBuff->Size(), (m_pAudio) ? m_pAudio->GetCurrBuffDurr() : 0);
		currBuff->Empty();
		m_StartCount++;
		static int prev = 0;
		//DTRACE(VSTM_THCL, "OK, t=%3d", CurrTime - prev);
		prev = CurrTime;
		m_pRender->m_bNewFrame = TRUE;
		m_pRender->DrawFrame(*m_phwnd);
	}
	return true;
}

int CVideoSource::GetFrameSizeMB()
{
	return std::max(m_pRender->GetFrameSizeMB(), m_recordMBs);
}

char* CVideoSource::GetPartName()
{
	return m_CallId;
}

/**
 **************************************************************************
 ****************************************************************************/
void CVideoSource::Detach() {
	m_predictor.Clear();
	m_audiodurr.Clear();
	m_StartCount = 1;
	m_EndCount = 1;
	m_PrevDurDurr = 0;
	m_phwnd = 0;
	m_pRender = 0;
	m_pOut = 0;
	m_pVideo = 0;
	m_pAudio = 0;
	m_iBufferSize = 0;
	m_recordMBs = 0;
}

/*****************************************************************************
 * CVideoSinc
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
CVideoSinc::CVideoSinc(CVSTrClientProc *pProtocol){
	int i;
	for(i=0; i < MAX_VIDEOSOURCE; i++)
		m_pVideoSource[i] = NULL;
	m_VideoSourceCounter = 0;
	m_pProtocol = pProtocol;
}


/**
 **************************************************************************
 ****************************************************************************/
void CVideoSinc::Go(){
	ActivateThread((LPVOID)this);
}

/**
 **************************************************************************
 ****************************************************************************/
void CVideoSinc::Stop(){
	DesactivateThread();
}

/**
 **************************************************************************
 ****************************************************************************/
DWORD CVideoSinc::Loop(LPVOID hEvDie)
{
	while(1) {
		switch(WaitForSingleObject(hEvDie, 10))
		{
		case WAIT_OBJECT_0:
			return NOERROR;
		case WAIT_TIMEOUT:
			Lock();
			for (int i = 0; i<MAX_VIDEOSOURCE; i++)
				if (m_pVideoSource[i]) {
					m_pVideoSource[i]->CheckDraw();
					m_pProtocol->SetFrameSizeMBUser(m_pVideoSource[i]->GetPartName(), m_pVideoSource[i]->GetFrameSizeMB());
				}
			UnLock();
			break;
		default:
			Sleep(10);
			break;
		}
	}
	return NOERROR;
}

/**
 **************************************************************************
 ****************************************************************************/
int CVideoSinc::AddVideoSource(CVideoSource*pVideoSource)
{
	VS_AutoLock lock(this);
	if (m_VideoSourceCounter < MAX_VIDEOSOURCE) {
		for (int i = 0; i < MAX_VIDEOSOURCE; i++) {
			if(m_pVideoSource[i]==NULL) {
				m_pVideoSource[i] = pVideoSource;
				m_VideoSourceCounter++;
				return i;
			}
		}
	}
	return -1;
}

/**
 **************************************************************************
 ****************************************************************************/
void CVideoSinc::RemoveVideoSource(CVideoSource*pVideoSource)
{
	VS_AutoLock lock(this);
	for (int i = 0; i < MAX_VIDEOSOURCE; i++) {
		if(m_pVideoSource[i]==pVideoSource) {
			pVideoSource->Detach();
			m_pVideoSource[i] = 0;
			m_VideoSourceCounter--;
			break;
		}
	}
}

/*****************************************************************************
 * CReceiversPool
 ****************************************************************************/
/**
 **************************************************************************
 ****************************************************************************/
CReceiversPool::CReceiversPool(CVSInterface *pParentInterface,CVSTrClientProc *pProtocol):
CVSInterface("Receivers",pParentInterface)
{
	m_pProtocol=pProtocol;
	szPlaybackDevice[0]=0;
	m_pVideoSinc=new CVideoSinc(pProtocol);
	m_pVideoSinc->Go();
	m_pRenderDevices=new CRenderAudioDevices(this);
	int i;
	for(i=0;i<MAX_VIDEOSOURCE;i++)
		m_pReceivers[i]=NULL;
	RstConfStat();

	m_UseVAD = 0;
	m_LayoutPriorityType = VS_WindowGrid::PRIORITY_LAYOUT_OVERLAY;
	m_pKey->GetValue(&m_UseVAD, 4, VS_REG_INTEGER_VT, "LayoutUsingVad");
	m_pKey->GetValue(&m_LayoutPriorityType, 4, VS_REG_INTEGER_VT, "LayoutPriorityType");
	m_lgrid.SetPriorityLayoutType((VS_WindowGrid::ePriorityLayoutType)m_LayoutPriorityType);

	VS_RegistryKey vadKey(true, "Client");
	vadKey.GetValue(&m_BordersMode, sizeof(m_BordersMode), VS_REG_INTEGER_VT, "VadFrame");

	/*
	// init
	m_lgrid.Init(200, 100, 1.0f);
	_variant_t var;
	// set parts
	{
		json::Object ans;
		json::Array part_list;
		for (unsigned long i = 'a'; i < 'd'; i++) {
			char id[2] = { i, 0 };
			json::Object part;
			part[id] = json::Boolean(i == 'a');

			part_list.Insert(part);
		}
		ans["list"] = part_list;

		std::stringstream ss;
		json::Writer::Write(ans, ss);
		if (ss.str().length() > 0) {
			var = ss.str().c_str();
			ProcessFunction(SET_PARAM, "CalcLayout", &var);
			DTRACE(255, "\n%s", (char*)(_bstr_t)var);
		}
	}
	// get part coords
	var.Clear();
	ProcessFunction(GET_PARAM, "CalcLayout", &var);
	DTRACE(255, "\n%s", (char*)(_bstr_t)var);
	*/
}

/**
 **************************************************************************
 ****************************************************************************/
CReceiversPool::~CReceiversPool()
{
	int i;
	for(i=0;i<MAX_VIDEOSOURCE;i++)
		if(m_pReceivers[i]!=NULL){
			delete m_pReceivers[i];
			m_pReceivers[i]=NULL;
		}
		m_pVideoSinc->Stop();
		delete m_pVideoSinc;
		delete m_pRenderDevices;
}

/**
 **************************************************************************
 ****************************************************************************/
void CReceiversPool::RstConfStat()
{
	memset(&m_stat_rcv, 0, sizeof(TConferenceStatRcv));
}

/**
 **************************************************************************
 ****************************************************************************/
void CReceiversPool::GetConfStat(TConferenceStatistics *cst)
{
	int i, n = 0;
	TConferenceStatistics c_cst;
	*cst = {};
	for (i = 0; i < MAX_VIDEOSOURCE; i++){
		if (m_pReceivers[i] != NULL) {
			m_pReceivers[i]->GetConfStat(&c_cst);
			cst->avg_jitter += c_cst.avg_jitter;
			cst->loss_rcv_packets += c_cst.loss_rcv_packets;
			cst->avg_rcv_bitrate += c_cst.avg_rcv_bitrate;
			n++;
		}
	}
	cst->avg_jitter += m_stat_rcv.avg_jitter;
	cst->loss_rcv_packets += m_stat_rcv.loss_rcv_packets;
	cst->avg_rcv_bitrate += m_stat_rcv.bytes;
	m_stat_rcv.num_rcv += n;
	if (m_stat_rcv.num_rcv != 0) {
		cst->avg_jitter /= m_stat_rcv.num_rcv;
	}
	m_stat_rcv.num_rcv -= n;
}

int CReceiversPool::GetQualityBitrate()
{
	int res = 0;
	int num = 0;
	for (int i = 0; i<MAX_VIDEOSOURCE; i++) {
		if (m_pReceivers[i]) {
			int br = m_pReceivers[i]->GetTraffic();
			if (br > 32) {
				res+= br;
				num++;
			}
		}
	}
	if (num) res/=num;
	return res;
}


int CReceiversPool::GetQuality()
{
	int res = 0;
	int num = 0;
	unsigned int maxvol = 0;
	CThinkClientReceiver* loudestReceiver = nullptr;
	std::string user;
	unsigned long currTime = GetTickCount();
	for (int i = 0; i<MAX_VIDEOSOURCE; i++) {
		if (m_pReceivers[i]) {
			int br = m_pReceivers[i]->GetTraffic();
			int q = m_pReceivers[i]->GetQuality();
			unsigned int vol = m_pReceivers[i]->GetVol(currTime);
			if (vol > maxvol) {
				maxvol = vol;
				user = m_pReceivers[i]->GetCallID();
				loudestReceiver = m_pReceivers[i];
			}
			if (br > 0 && q > 0) {
				res+= q;
				num++;
			}

			if (m_BordersMode == BM_ALL)
			{
				if (vol > 300)
					m_pReceivers[i]->EnableBorders(true);
				else
					m_pReceivers[i]->EnableBorders(false);
			}
			else if (m_BordersMode == BM_LOUDEST)
			{
				m_pReceivers[i]->EnableBorders(false);
			}
		}
	}
	if (num > 1)
		res = (res + num/2)/ num;

	if (m_BordersMode == BM_LOUDEST && loudestReceiver)
	{
		if (maxvol > 300)
			loudestReceiver->EnableBorders(true);
		else
			loudestReceiver->EnableBorders(false);
	}

	if (m_UseVAD && !user.empty()) {
		VS_WindowGrid::sOneWindow wn = m_lgrid.GetWindow(user);
		int type = -1;
		if (maxvol > 50) { // some vol detected
			if (wn.userId.empty()) { // not found in main window
				m_lgrid.SetP0();
				type = 0;
			}
			else {
				if (wn.priority != VS_WindowGrid::PRIORITY_HIGH) {
					m_lgrid.SetP1(user);
					type = 1;
				}
			}
		}
		if (type >= 0)
			PostMessage(m_pProtocol->m_ExternalWindow, WM_USER + 24, 0, 0);
		DTRACE(VSTM_THCL, "The loudest RCV = %s, vol = %d, type = %d", user.c_str(), maxvol, type);
	}
	return res;
}

/**
 **************************************************************************
 ****************************************************************************/
int CReceiversPool::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;
	if (strcmp(pSection, "PlaybackDevice")==0) {
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			*var=(_bstr_t)szPlaybackDevice;
			return VS_INTERFACE_OK;
		case SET_PARAM:
			if (IsBSTR(pVar)) {
				wcsncpy(szPlaybackDevice,(_bstr_t)(*var),MAX_PATH);
				int nAudio=m_pRenderDevices->iGetDeviceNumberByName(szPlaybackDevice);
				for (int i = 0; i < MAX_VIDEOSOURCE; i++) {
					if (m_pReceivers[i] != 0) {
						m_pReceivers[i]->RemoveAudio(nAudio);
					}
				}
				if (nAudio>=0) {
					m_pRenderDevices->iGetDeviceModeList(nAudio);
					*var = nAudio;
					Process(RUN_COMMAND, "AudioPlayback\\SystemVolume", var);
					Process(RUN_COMMAND, "AudioPlayback\\MasterVolume", var);
					for (int i = 0; i < MAX_VIDEOSOURCE; i++) {
						if (m_pReceivers[i] != 0) {
							m_pReceivers[i]->SetChangeAudioDevice(nAudio);
						}
					}
				} else {
					for (int i = 0; i < MAX_VIDEOSOURCE; i++) {
						if (m_pReceivers[i] != 0) {
							m_pReceivers[i]->PrepareAudio(nAudio);
						}
					}
				}
				m_pKey->SetValue(szPlaybackDevice, 0, VS_REG_WSTRING_VT, "RenderAudioName");
				return VS_INTERFACE_OK;
			}
			return VS_INTERFACE_INTERNAL_ERROR;
		case RUN_COMMAND:
			m_pKey->GetValue(szPlaybackDevice, 260, VS_REG_WSTRING_VT, "RenderAudioName");
			return VS_INTERFACE_OK;
		}
	}
	else if(strncmp(pSection,_funcParticipantStatus,sizeof(_funcParticipantStatus))==0){
		switch(VS_OPERATION)
		{
		case GET_PARAM:
			if (IsBSTR(pVar)) {
				long dvs;
				if (m_pProtocol->GetRcvDevStatus((char*)(_bstr_t)*var, &dvs)) {
					*var = dvs;
					return VS_INTERFACE_OK;
				}
			}
			return VS_INTERFACE_INTERNAL_ERROR;
		case SET_PARAM:
			if(var->vt==(VT_ARRAY|VT_VARIANT)){
				long l,u;
				RENDERPROC lParam1=NULL;
				LPVOID lParam2=NULL;
				SAFEARRAY *psa=var->parray;
				SafeArrayGetLBound(psa,1,&l);
				SafeArrayGetUBound(psa,1,&u);
				if(u-l==1){
					_variant_t varID,varFlag;
					SafeArrayGetElement(psa,&l, &varID.GetVARIANT());
					l++;
					SafeArrayGetElement(psa,&l, &varFlag.GetVARIANT());
					m_pProtocol->ConnectReceiver((char*)(_bstr_t)varID,varFlag);
					return VS_INTERFACE_OK;
				}
			}
			return VS_INTERFACE_INTERNAL_ERROR;
		}
	}
	else if(strncmp(pSection,_funcDisconnectReceiver,sizeof(_funcDisconnectReceiver))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			if (IsBSTR(pVar)) {
				int i;
				for (i=0;i<MAX_VIDEOSOURCE;i++) {
					if (m_pReceivers[i]!=0 && strncmp(m_pReceivers[i]->GetInterfaceName(), (char*)(_bstr_t)*var, MAX_PATH)==0) {
						m_pReceivers[i]->Disconnect();
						TConferenceStatistics cst;
						m_pReceivers[i]->GetConfStat(&cst);
						m_stat_rcv.avg_jitter += cst.avg_jitter;
						m_stat_rcv.bytes += cst.avg_rcv_bitrate;
						m_stat_rcv.loss_rcv_packets += cst.loss_rcv_packets;
						m_stat_rcv.num_rcv++;
						delete m_pReceivers[i];
						m_pReceivers[i]=NULL;
						return VS_INTERFACE_OK;
					}
				}
			}
			return VS_INTERFACE_INTERNAL_ERROR;
		}
	}
	else if(strncmp(pSection,_funcConnectReceiver,sizeof(_funcConnectReceiver))==0){
		switch(VS_OPERATION)
		{
		case RUN_COMMAND:
			if(var->vt==(VT_ARRAY|VT_VARIANT)){
				long l,u;
				RENDERPROC lParam1=NULL;
				LPVOID lParam2=NULL;
				SAFEARRAY *psa=var->parray;
				SafeArrayGetLBound(psa,1,&l);
				SafeArrayGetUBound(psa,1,&u);
				if(u-l==2){
					int i;
					for (i=0;i<MAX_VIDEOSOURCE;i++){
						if(m_pReceivers[i]==NULL)
							break;
					}
					if(i==MAX_VIDEOSOURCE)
						return VS_INTERFACE_INTERNAL_ERROR;
					HWND hRenderView,hReportHwnd;
					int ID;
					VARIANT vr;
					_variant_t tmpvar;
					SafeArrayGetElement(psa,&l,&vr);
					l++;
					tmpvar.Attach(vr);
					hRenderView=(HWND)int(tmpvar);
					SafeArrayGetElement(psa,&l,&vr);
					l++;
					tmpvar.Attach(vr);
					hReportHwnd=(HWND)int(tmpvar);
					SafeArrayGetElement(psa,&l,&vr);
					tmpvar.Attach(vr);
					ID=int(tmpvar);

					VS_MediaFormat mf;
					VS_BinBuff simkey;
					char OtherName[MAX_PATH], tmpName[MAX_PATH];
					boost::shared_ptr<VS_StreamClientReceiver> pReceiver;
					long fltr = VS_RcvFunc::FLTR_ALL_MEDIA;
					if (ID) {
						int n = m_pProtocol->GetRcvAction(ID, OtherName, pReceiver, fltr);
						if (n!=1)
							return VS_INTERFACE_INTERNAL_ERROR;
					}
					else {
						m_pProtocol->GetOtherName(OtherName);
						pReceiver = m_pProtocol->GetConferenseReceiver();
					}
					if (pReceiver==NULL)
						return VS_INTERFACE_INTERNAL_ERROR;
					VS_ClientCaps caps;
					int confType = m_pProtocol->GetMediaFormat(OtherName,mf,&caps);
					VS_ClientType clt = static_cast<VS_ClientType>(caps.GetClientType());
					mf.dwHWCodec = (clt == CT_TRANSCODER || clt == CT_TRANSCODER_CLIENT) ? 0 : 1;
					m_pProtocol->GetSimKey(OtherName, simkey);
					strncpy(tmpName,OtherName,MAX_PATH);
					strcat(tmpName,">");
					unsigned long capsFourcc = 0x0;
					unsigned long nmy = 9;
					size_t nfrom = 0;
					caps.GetVideoCodecs(0, nfrom);
					if (nfrom > 0) {
						auto pFourcc = std::make_unique<uint32_t[]>(nfrom);
						caps.GetVideoCodecs(pFourcc.get(), nfrom);
						unsigned long imy, ifrom;
						for (imy = 0; imy < nmy; imy++) {
							for (ifrom = 0; ifrom < nfrom; ifrom++) {
								if (pFourcc[ifrom] == VS_EnumVCodecs[imy]) {
									capsFourcc |= (1 << imy);
									break;
								}
							}
						}
					}
					m_pReceivers[i]=new CThinkClientReceiver(tmpName,this,m_pVideoSinc);
					m_pReceivers[i]->PrepareVideo(hRenderView,&lParam1,&lParam2);
					int iDevice=m_pRenderDevices->iGetDeviceNumberByName(szPlaybackDevice);
					m_pReceivers[i]->PrepareAudio(iDevice);
					m_pReceivers[i]->Init(mf, capsFourcc);
					m_pReceivers[i]->Connect(pReceiver, hReportHwnd, fltr, simkey, confType);

					_variant_t var_;
					l=0;
					var_=int(lParam1);
					SafeArrayPutElement(psa, &l, &var_.GetVARIANT());
					l++;
					var_=int(lParam2);
					SafeArrayPutElement(psa, &l, &var_.GetVARIANT());
					l++;
					var_=_bstr_t(OtherName);
					SafeArrayPutElement(psa, &l, &var_.GetVARIANT());

					return VS_INTERFACE_OK;
				}
			}
			return VS_INTERFACE_INTERNAL_ERROR;
		}
	}
	else if(strncmp(pSection,_funcGetReceivedData,sizeof(_funcGetReceivedData))==0){
		switch(VS_OPERATION)
		{
		case GET_PARAM:{
			int res = 0;
			for (int i = 0; i<MAX_VIDEOSOURCE; i++) {
				if (m_pReceivers[i])
					res+= m_pReceivers[i]->GetTraffic();
			}
			*var=res;
			return VS_INTERFACE_OK;
					   }
		}
	}
	else if (strcmp(pSection, "AviPlay") == 0) {
		switch (VS_OPERATION)
		{
		case SET_PARAM:
			if (var->vt == (VT_ARRAY | VT_VARIANT)) {
				HWND vWnd, nWnd;
				long l, u;
				SAFEARRAY *psa = var->parray;
				SafeArrayGetLBound(psa, 1, &l);
				SafeArrayGetUBound(psa, 1, &u);
				if ((u - l) >= 2){
					int num = u - l + 1; long i;
					VARIANT *vars = new VARIANT[num];
					for (i = 0; i < num; ++i)
						VariantInit(&vars[i]);
					for (i = 0; i < num; ++i)
						SafeArrayGetElement(psa, &i, &vars[i]);
					VS_WideStr fileName = (wchar_t*)(_bstr_t)vars[0];
					vWnd = (HWND)vars[1].lVal;
					nWnd = (HWND)vars[2].lVal;
					delete[] vars;

					int dev = m_pRenderDevices->iGetDeviceNumberByName(szPlaybackDevice);
					_variant_t var_;

					i = 0;
					var_ = m_PlayAvi.Init(fileName, vWnd, nWnd, dev);
					SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
					i++;
					var_ = (long)CVideoRenderBase::WindowProc;
					SafeArrayPutElement(psa, &i, &var_.GetVARIANT());
					i++;
					var_ = m_PlayAvi.GetVRender();
					SafeArrayPutElement(psa, &i, &var_.GetVARIANT());

					return VS_INTERFACE_OK;
				}
			}
			m_PlayAvi.Release();
			return VS_INTERFACE_OK;
		case GET_PARAM:
			*var = m_PlayAvi.GetPosition(*var);
			return VS_INTERFACE_OK;
		case RUN_COMMAND:
			if (IsBSTR(pVar)) {
				m_PlayAvi.SetPosition(atou_s((char*)(_bstr_t)*var));
			}
			else {
				if (pVar->lVal == 0)
					m_PlayAvi.Stop();
				else if (pVar->lVal == 1)
					m_PlayAvi.Start();
				else if (pVar->lVal == 2)
					m_PlayAvi.Pause();
				else
					return VS_INTERFACE_NO_FUNCTION;
			}
			return VS_INTERFACE_OK;
		}
	}
	// from receiver ProcessFunc
	else if (strcmp(pSection, "ReceiverFunc") == 0) {
		VARIANT* vars = 0;
		int n = ExtractVars(vars, pVar);
		if (n >= 3) { // func name, receiver name, params
			VS_SimpleStr fname = (char*)(_bstr_t)vars[0].bstrVal;
			VS_SimpleStr rname = (char*)(_bstr_t)vars[1].bstrVal;
			CThinkClientReceiver* r = 0;

			for (int i = 0; i < MAX_VIDEOSOURCE; i++) {
				if (m_pReceivers[i] != 0 && strncmp(m_pReceivers[i]->GetInterfaceName(), rname, MAX_PATH) == 0)
					r = m_pReceivers[i];
			}

			if (r) {
				if (fname == "ResetWindow") {
					r->ResetWindow((HWND)vars[2].lVal);
					return VS_INTERFACE_OK;
				}
				else if (fname == "Format") {
					VS_MediaFormat mf;
					r->GetCurrentMediaFormat(mf);
					_variant_t vars[6];

					vars[0] = mf.dwVideoWidht;
					vars[1] = mf.dwVideoHeight;
					vars[2] = mf.dwAudioSampleRate;
					vars[3] = mf.dwAudioCodecTag;
					vars[4] = mf.dwAudioBufferLen;
					vars[5] = mf.dwVideoCodecFCC;

					if (CombineVars(pVar, vars, 6))
						return VS_INTERFACE_OK;
					else
						return VS_INTERFACE_INTERNAL_ERROR;

				}
				else if (fname == "SetRsvFltr") {
					r->SetRcvFltr(vars[2].lVal);
					return VS_INTERFACE_OK;
				}
			}
		}
		if (n > 0)
			delete[] vars;
	}
#ifdef _BUILD_CONFERENDO
#define WIN_AR 4.0f / 3.0f;
#else
#define WIN_AR 16.0f / 9.0f;
#endif

	else if (strcmp(pSection, "CalcLayout") == 0) {
		if (VS_OPERATION == RUN_COMMAND) {
			VARIANT *vars = 0;
			int num = ExtractVars(vars, var);
			if (num > 0) {
				VS_SimpleStr fname = (char*)(_bstr_t)vars[0].bstrVal;
				int res = 1;
				if (fname == "Init") {
					float ar = WIN_AR;
					res = m_lgrid.Init(vars[1].lVal, vars[2].lVal, ar);
				}
				else if (fname == "AddWindow") {
					std::string user = (const char*)(_bstr_t)vars[1].bstrVal;
					res = m_lgrid.AddWindow(user);
				}
				else if (fname == "RemoveWindow") {
					std::string user = (const char*)(_bstr_t)vars[1].bstrVal;
					res = m_lgrid.RemoveWindow(user);
				}
				else if (fname == "SetP0") {
					res = m_lgrid.SetP0();
				}
				else if (fname == "SetP1") {
					std::string user = (const char*)(_bstr_t)vars[1].bstrVal;
					res = m_lgrid.SetP1(user);
				}
				else if (fname == "SetP2") {
					std::string user1 = (const char*)(_bstr_t)vars[1].bstrVal;
					std::string user2 = (const char*)(_bstr_t)vars[2].bstrVal;
					res = m_lgrid.SetP2(user1, user2);
				}
				else if (fname == "Swap") {
					std::string user1 = (const char*)(_bstr_t)vars[1].bstrVal;
					std::string user2 = (const char*)(_bstr_t)vars[2].bstrVal;
					res = m_lgrid.Swap(user1, user2);
				}
				else if (fname == "LayoutPriorityType") {
					m_LayoutPriorityType = vars[1].lVal;
					m_lgrid.SetPriorityLayoutType((VS_WindowGrid::ePriorityLayoutType)m_LayoutPriorityType);
					m_pKey->SetValue(&m_LayoutPriorityType, 4, VS_REG_INTEGER_VT, "LayoutPriorityType");
					res = 0;
				}
				delete[] vars;

				if (res == 0)
					return VS_INTERFACE_OK;
			}
			return VS_INTERFACE_INTERNAL_ERROR;
		}
		else if (VS_OPERATION == SET_PARAM) {
			long res = VS_INTERFACE_OK;
			std::string p;
			try {
				std::string svar = (char*)(_bstr_t)*var;
				std::stringstream ss(svar);
				json::Object req;
				json::Reader::Read(req, ss);
				json::Object::iterator it = req.Find("list");
				if (it != req.End()) {
					json::Array list = it->element;
					m_lgrid.RemoveAll();
					for (json::Array::iterator ii = list.Begin(); ii != list.End(); ii++) {
						json::Object part = *ii;
						std::string name = part.Begin()->name;
						bool priority = (const json::Boolean)part.Begin()->element;
						m_lgrid.AddWindow(name);
						if (p.length() == 0 && priority)
							p = name;
					}
				}
			}
			catch (json::Exception & err)
			{
				DTRACE(VSTM_THCL, "CalcLayout ERR: %s", err.what());
				res = VS_INTERFACE_INTERNAL_ERROR;
			}
			if (p.length() != 0)
				m_lgrid.SetP1(p);
			g_AviWriterGroup->SetPriority(p.c_str());

			return res;
		}
		else if (VS_OPERATION == GET_PARAM) {
			json::Object ans;
			int lwin = m_lgrid.GetAllWindows(ans);
			if (lwin > 0) {
				std::stringstream ss;
				json::Writer::Write(ans, ss);
				if (ss.str().length() > 0) {
					*var = ss.str().c_str();
					return VS_INTERFACE_OK;
				}
			}
			return VS_INTERFACE_INTERNAL_ERROR;
		}
	}
	else if (strcmp(pSection, "LayoutUsingVad") == 0) {
		if (VS_OPERATION == SET_PARAM) {
			m_UseVAD = *var;
			m_pKey->SetValue(&m_UseVAD, 4, VS_REG_INTEGER_VT, "LayoutUsingVad");
			return VS_INTERFACE_OK;
		}
		else if (VS_OPERATION == GET_PARAM) {
			m_pKey->GetValue(&m_UseVAD, 4, VS_REG_INTEGER_VT, "LayoutUsingVad");
			*var = m_UseVAD;
			return VS_INTERFACE_OK;
		}
	}
	else if (strcmp(pSection, "VadFrame") == 0)
	{
		VS_RegistryKey vadKey(true, "Client", false, true);

		if (VS_OPERATION == SET_PARAM)
		{
			if (DWORD(*var) == 0)
			{
				m_BordersMode = BM_NONE;

				for (int i = 0; i < MAX_VIDEOSOURCE; i++)
				{
					if (m_pReceivers[i])
						m_pReceivers[i]->EnableBorders(false);
				}
			}
			else
				m_BordersMode = BM_ALL;

			vadKey.SetValue(&m_BordersMode, sizeof(m_BordersMode), VS_REG_INTEGER_VT, "VadFrame");
			return VS_INTERFACE_OK;
		}
		else if (VS_OPERATION == GET_PARAM)
		{
			vadKey.GetValue(&m_BordersMode, sizeof(m_BordersMode), VS_REG_INTEGER_VT, "VadFrame");

			if (m_BordersMode == BM_NONE)
				*var = 0;
			else
				*var = -1;

			return VS_INTERFACE_OK;
		}
	}
	return VS_INTERFACE_NO_FUNCTION;
}
