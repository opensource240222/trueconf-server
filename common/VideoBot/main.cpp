#include "../net/EndpointRegistry.h"
#include "SecureLib/VS_CryptoInit.h"
#include "..\std\cpplib\VS_SimpleStr.h"
#include "std-generic/cpplib/VS_Container.h"
#include "..\std\cpplib\VS_RegistryKey.h"
#include "../std/cpplib/VS_Utils.h"
#include "std/cpplib/ThreadUtils.h"
#include "std-generic/cpplib/ignore.h"

#include "transport/Client/VS_TransportClient.h"
#include "../streams/Protocol.h"
#include "../streams/Client/VS_StreamClient.h"
#include "../streams\Client\VS_StreamClientSender.h"
#include "../streams\Client\VS_StreamClientReceiver.h"

#include "../Audio/WinApi/dsutil.h"
#include "../Audio/WinApi/dsutil.h"
#include "../Video/WinApi/CAviFile.h"

#include "VSClient/VS_Dmodule.h"
#include "../VSClient/VSTrClientProc.h"
#include "../VSClient/VS_StreamPacketManagment.h"
#include "../VSClient/VS_MiscCommandProc.h"
#include "../VSClient/VS_TCPStream.h"
#include "../VSClient/VSCompress.h"
#include "../VSClient/VS_NhpHeaders.h"
#include "../VSClient/VS_AviWrite.h"
#include "VSClient/VSAudioEchoCancel.h"
#include "Transcoder/VS_VideoCodecManager.h"

#include "../Transcoder/VS_IppAudiCodec.h"
#include "Transcoder/RtpPayload.h"
#include "../Transcoder/VS_RTP_Buffers.h"
#include "../Transcoder/VS_AudioReSampler.h"
#include "../Transcoder/VS_SpeexAudioCodec.h"
#include "../Transcoder/VS_IppAudiCodec.h"
#include "../Transcoder/VideoCodec.h"
#include "../Transcoder/VS_RTP_Buffers.h"
#include "Transcoder/VS_BitStreamBuff.h"
#include "../MediaParserLib/VS_H263Parser.h"
#include "../MediaParserLib/VS_H264Parser.h"
#include "../MediaParserLib/VS_XCCParser.h"
#include "../MediaParserLib/VS_VPXParser.h"
#include "..\Audio\VoiceActivity\SpecialDspFunctions.h"
#include "std/cpplib/event.h"

#include <algorithm>

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <math.h>


#include  <io.h>
#include <direct.h>
#include <errno.h>
#include "..\tools\Server\VS_ServerLib.h"

static HINSTANCE ippCodecExtLib = 0;
static bool ippCodecExtLibLoaded = false;

static DWORD ClientStatus = 0;
static DWORD RCV_ThreadId = 0;
static HANDLE hDes = 0;
#define MAX_RECIEVERS_NUM 10
static DWORD streamsCreated = 0;
static HANDLE hConfTread[] = {0, 0};
static DWORD s_dwBitrate = 100;
static DWORD SucsTest1 =0;
#define   GWC_CALCVCBITRATE_PERIOD		4

SERVICE_STATUS g_ss;                    //  
SERVICE_STATUS_HANDLE g_ssHandle;       //  


CVideoDecompressor g_decoder;

long g_RequestKeyFrame = 0;
VS_VideoCodecManager* codecManager = 0;

VS_Lock					g_BandContr_Lock;
VS_ControlBandBase*		g_BandContr;		///< bitrate controller
VS_SendFrameQueueBase*	g_FrameQueue;		///< send queue

tc_VideoLevelCaps		g_lvlCaps;
tc_AutoLevelCaps		g_autolvlCaps;

int g_MaxVsBitrate = 384;
int g_LastNeedBand = 0;
long g_ServerBand = 2048;
long g_ServerBandVideo = 640;

VS_MediaFormat		recv_mf, send_mf;
unsigned long		recv_capsFourcc = (0x00000020 | 0x00000040 | 0x00000080 | 0x00000002 | 0x00000004 | 0x00000100); /// exclude xcc
int first_time_recv=true;

VS_SimpleStr g_AviFileName;
VS_SimpleStr g_Login;
VS_SimpleStr g_Pass;
VS_SimpleStr g_Server;
VS_SimpleStr g_ServiceName;
VS_SimpleStr g_WorkDir;
bool		 g_ServiceInstall = false;
bool		 g_ServiceUnInstall = false;
int			 g_WorkMode = 0;
int		     g_maxWidth = 0;
int		     g_maxHeight = 0;
int			 g_useSvc = 0;

HANDLE g_hMain = 0;
HANDLE g_DieEvent;

enum ConfState
{
	ST_NORMAL,
	ST_CALL,
	ST_CONF,
	ST_CONFEND,
};

static	ConfState confstate = ST_NORMAL;
int  UpdateMediaFormat(unsigned char *pSource, bool bKey, unsigned int size);

void CalculateBitrate(int baseBitrate, int maxBitrate, int maxFrameRate, DWORD svcMode)
{
	if (!g_BandContr)
		return ;

	VS_AutoLock lock(&g_BandContr_Lock);

	DWORD CurrBand = g_BandContr->GetVideoBandwidth(maxBitrate);
	if (CurrBand < 32) CurrBand = 32; // set minimum band
	int abr = g_BandContr->GetCurrBand((VS_ControlBandBase::BandType)1);
	int vbr = g_BandContr->GetCurrBand((VS_ControlBandBase::BandType)2);
	if (CurrBand != g_LastNeedBand) {
		g_LastNeedBand = CurrBand;
		double fps = maxFrameRate;
		CurrBand -= 16; // sub audio from band
		if (CurrBand < 10) CurrBand = 10; // minimum bitrate for video
		int CodecBtr = CurrBand;
		if (svcMode & 0x00030000) {
			CodecBtr = baseBitrate;
		}
		if (codecManager)
			codecManager->SetBitrate(CodecBtr, CurrBand, maxFrameRate * 100);
		DTRACE(VSTM_BTRC, "CalcBR: a = %3d, v = %3d, codec btr = %d, curr band = %u, fps = %2.2f, need band = %d", abr, vbr, CodecBtr, CurrBand, maxFrameRate, g_LastNeedBand);
	}
}


int ReceivedCommandProcess()
{
	stream::Command cmd;
	while (g_cmdProc.ReadRcvCommand(cmd)) {
		// info commands
		auto result = stream::Command::UnknownError;
		if (cmd.type == stream::Command::Type::RequestKeyFrame) {
			InterlockedExchange(&g_RequestKeyFrame, true);
		} else if (cmd.type == stream::Command::Type::RestrictBitrate) {
			if (cmd.data_size == 4) {
				InterlockedExchange(&g_ServerBand, *reinterpret_cast<uint32_t*>(cmd.data));
				InterlockedExchange(&g_ServerBandVideo, g_ServerBand);
			}
		} else if (cmd.type == stream::Command::Type::RestrictBitrateSVC) {
			if (cmd.data_size == 8) {
				InterlockedExchange(&g_ServerBand, reinterpret_cast<uint32_t*>(cmd.data)[0]);
				InterlockedExchange(&g_ServerBandVideo, reinterpret_cast<uint32_t*>(cmd.data)[1]);
			}
		}
		g_BandContr_Lock.Lock();
		if (g_BandContr)
			g_BandContr->SetReceivedCommand(cmd);
		g_BandContr_Lock.UnLock();
		if (cmd.sub_type == stream::Command::Request) {
			cmd.MakeReply(result);
			g_cmdProc.AddCommand(cmd);
		}
	}
	return 0;
}


#include "VideoFile.h"

VideoFile g_vf;
VoiceChanger vc;



int  UpdateMediaFormat(unsigned char *pSource, bool bKey, unsigned int size)
{
	int width = 0, height = 0, fps = 30, stereo = 0;
	unsigned int fourcc = 0;
	int ret = -1, num_threads = 0;

	if (recv_capsFourcc & (0x00000020 | 0x00000040 | 0x00000080)) ret = ResolutionFromBitstream_VPX(pSource, size, width, height, num_threads);
	if (ret == 0) {
		if (num_threads == 1) fourcc = VS_VCODEC_VPX;
		else if (num_threads == 2) fourcc = VS_VCODEC_VPXSTEREO;
		else fourcc = VS_VCODEC_VPXHD;
	} else {
		if (recv_capsFourcc & 0x00000002) ret = ResolutionFromBitstream_H264(pSource, size, width, height);
		if (ret == 0) {
			fourcc = VS_VCODEC_H264;
		} else {
			if (recv_capsFourcc & 0x00000100) ret = ResolutionFromBitstream_H264(pSource, size, width, height);
			if (ret == 0) {
				fourcc = VS_VCODEC_H265;
			}
			else {
				if (recv_capsFourcc & 0x00000001) ret = ResolutionFromBitstream_XCC(pSource, size, width, height);
				if (ret == 0) {
					fourcc = VS_VCODEC_XC02;
				}
				else {
					if (recv_capsFourcc & 0x00000004) ret = ResolutionFromBitstream_H263(pSource, size, width, height);
					if (ret == 0) {
						fourcc = VS_VCODEC_H263P;
					}
				}
			}
		}
	}
	if (ret < 0) {
		if (ret == -2) ret = 0;
		return ret;
	}
	recv_mf.SetVideo(width, height, fourcc, fps, stereo);
	g_vf.SetRcvFormat(recv_mf);
	return 0;
}


DWORD WINAPI Sender(LPVOID lppar)
{
	vs::SetThreadName("Sender");
	int DoExit = 0;

	CVSTrClientProc *ContP = reinterpret_cast<CVSTrClientProc *>(lppar);
	ContP->TrInit(VS_FileTransfer::DEFAULT_FLAGS);
	ResetEvent(hDes);

	boost::shared_ptr<VS_StreamClientSender> snd = ContP->GetConferenseSender();
	if (!snd) {
		ContP->Hangup();
		return -1;
	}

	int Wroten= 0;
	int retu = 0;
	stream::Track track = {};
	unsigned char q_track = 0;
	unsigned long SEND_MILLS = 50;
	unsigned long mills = SEND_MILLS;
	snd->SendFrame(0, 0, stream::Track::old_command, &mills);

	//////////////////////////////////////////////////////////////////////////
	// TODO: calc g_MaxVsBitrate = VS_SignleGW: Line 954
	//////////////////////////////////////////////////////////////////////////

	bool bNhp = snd->ConnectType() == vs_stream_client_connect_not_guaranteed;
	long ConfType = ContP->m_Status.CurrConfInfo->confType;
	bool bIntercom = ConfType == 5 || ConfType < 0;
	bool bGroup = ConfType == 5 || ConfType == 4;

	int startBitrate = 0;
	int MaxBitrate = 0;
	int BaseBitrate = 0;
	int MaxFrameRate = 15;

	unsigned long key_frame_interval = 10*1000;
	unsigned long last_key_frame_time = 0;
	unsigned long tick = 0;
	unsigned long LastSendVideoTime = 0;

	//VS_MediaFormat mf;
	char MyId[MAX_PATH];
	ContP->GetMyName(MyId);
	ContP->GetMediaFormat(MyId,send_mf);

	if (g_WorkMode == WORK_MODE_DEMOGROUP || g_WorkMode == WORK_MODE_DEMOGROUPCOMPRESS || g_WorkMode == WORK_MODE_DEMOGROUPCOMPRESS_SVC) {
		int width = g_maxWidth;
		int height = g_maxHeight;
		if (bGroup) {
			InterlockedExchange(&g_ServerBand, 2048);
			InterlockedExchange(&g_ServerBandVideo, 640);
			BaseBitrate = g_ServerBandVideo;
			MaxBitrate = g_ServerBand;
			startBitrate = 512;
			if (width == 0 || height == 0) {
				width = 1280;
				height = 720;
			}
		} else {
			MaxBitrate =  std::min<int>(4096, ContP->m_Status.CurrConfInfo->ClientCaps.GetBandWRcv());
			BaseBitrate = MaxBitrate;
			startBitrate = MaxBitrate * 2 / 3;
			if (width == 0 || height == 0) {
				width = 1920;
				height = 1080;
			}
			int lvl = ContP->m_Status.CurrConfInfo->ClientCaps.GetLevel() & 0x000000ff;
			int rating = ContP->m_Status.CurrConfInfo->ClientCaps.GetRating();
			lvl = g_lvlCaps.MergeRatingVsLevel(rating, (unsigned char)lvl);
			lvl = g_lvlCaps.CheckLevel(lvl);
			lvl = g_autolvlCaps.CheckLevel(lvl);
			tc_AutoModeDesc_t modeDesc;
			g_autolvlCaps.GetLevelDesc(lvl, ENCODER_SOFTWARE, &modeDesc);
			width = std::min<int>(width, modeDesc.defWidth);
			height = std::min<int>(height, modeDesc.defHeight);
		}
		send_mf.SetVideo(width, height, VS_VCODEC_VPX, 30, 0, send_mf.dwSVCMode);
	} else {
		ContP->GetOtherName(MyId);
		if (!ConfType) {
			MaxBitrate = g_MaxVsBitrate;
			startBitrate = 128;
		} else {
			MaxBitrate = std::min<int>(g_MaxVsBitrate, ContP->m_Status.CurrConfInfo->ClientCaps.GetBandWRcv());
			startBitrate = MaxBitrate * 2 / 3;
		}
		BaseBitrate = MaxBitrate;
	}
	// set avi params
	g_vf.Init(g_AviFileName, MyId, g_WorkMode);
	g_vf.SetSndFormat(send_mf);
	if (g_WorkMode == WORK_MODE_DEMOGROUP || g_WorkMode == WORK_MODE_DEMOGROUPCOMPRESS || g_WorkMode == WORK_MODE_DEMOGROUPCOMPRESS_SVC) {
		MaxFrameRate = g_vf.GetSndFrameRate();
	}

	g_FrameQueue = VS_SendFrameQueueBase::Factory(bNhp, send_mf.dwSVCMode != 0);

	int limitSize = 0;
	VS_RegistryKey key(true, REG_CurrentConfiguratuon, false, true);
	if (key.GetValue(&limitSize, sizeof(int), VS_REG_INTEGER_VT, "PacketLimit"))
		g_FrameQueue->SetLimitSize(limitSize);

	g_BandContr_Lock.Lock();
	g_BandContr = VS_ControlBandBase::Factory(bNhp, bIntercom);
	g_BandContr->SetQueuePointers(g_FrameQueue, snd->GetSendEvent());
	g_BandContr->IsDataAllowed(startBitrate);
	g_BandContr->GetVideoBandwidth(startBitrate);
	g_BandContr_Lock.UnLock();

	codecManager = new VS_VideoCodecManager();
	codecManager->Init(&send_mf, g_lvlCaps.MBps2Level(send_mf.GetMBps()));
	codecManager->SetBitrate(150, 150, MaxFrameRate * 100);
	AudioCodec* a_cmp = VS_RetriveAudioCodec(send_mf.dwAudioCodecTag, true);
	WAVEFORMATEX wf;
	wf.cbSize = 0;
	wf.nSamplesPerSec = send_mf.dwAudioSampleRate;
	wf.nChannels = 1;
	a_cmp->Init(&wf);

	DWORD ratio=100;
	key.GetValue(&ratio, 4, VS_REG_INTEGER_VT, "VoiceChanger");
	vc.Init(ratio/(double)100);

	size_t tmp_size = std::max<size_t>(65536, send_mf.dwVideoWidht*send_mf.dwVideoHeight*4);
	unsigned char *pTmp = (unsigned char *)malloc(tmp_size);
	unsigned char *pTmpOut = (unsigned char *)malloc(tmp_size);
	unsigned char *data;
	int size = 0;

	long n_samples = 0;
	unsigned char slayer = 0, vframe_num = 0;
	unsigned long CalcVSBitratePeriod = 0, smpl_read = 0;

	HANDLE handles[3];
	handles[0] = hDes;
	handles[1] = CreateEvent(0, 0, 0, 0);
	handles[2] = g_cmdProc.GetCmdEvent();
	VideoFile::State vf_state = VideoFile::VF_NONE;

	do {
		DWORD waitRes = WaitForMultipleObjects(3, handles, FALSE, 16);
		VideoFile::State State = g_vf.CheckState();
		if (State!=vf_state) {
			switch(State)
			{
			case VideoFile::VF_NONE: DTRACE(VSTM_AVIW, "--------- STATE IS VF_NONE----------"); break;
			case VideoFile::VF_SAY: DTRACE(VSTM_AVIW, "--------- STATE IS VF_SAY----------");break;
			case VideoFile::VF_REC: DTRACE(VSTM_AVIW, "--------- STATE IS VF_REC----------");break;
			case VideoFile::VF_PLAYLOOP: DTRACE(VSTM_AVIW, "--------- STATE IS VF_PLAYLOOP----------");break;
			case VideoFile::VF_PLAYREC:
				{
					send_mf.dwVideoWidht=recv_mf.dwVideoWidht;
					send_mf.dwVideoHeight=recv_mf.dwVideoHeight;
					g_vf.SetSndFormat(send_mf);
					codecManager->Release();
					free(pTmp);
					free(pTmpOut);
					tmp_size = std::max<size_t>(65536, send_mf.dwVideoWidht*send_mf.dwVideoHeight*4);
					pTmp = (unsigned char *)malloc(tmp_size);
					pTmpOut = (unsigned char *)malloc(tmp_size);
					codecManager->Init(&send_mf);
					CalculateBitrate(BaseBitrate, MaxBitrate, MaxFrameRate, send_mf.dwSVCMode);

					DTRACE(VSTM_AVIW, "--------- STATE IS VF_PLAYREC----------");break;

				}
			case VideoFile::VF_READYLOOP:
				{
					DTRACE(VSTM_AVIW, "--------- STATE IS VF_READYLOOP ----------");break;

				}
			case VideoFile::VF_ENDCALL: DTRACE(VSTM_AVIW, "--------- STATE IS F_ENDCALL----------");break;
			}
			vf_state = State;
			if (vf_state==VideoFile::VF_ENDCALL)
				DoExit = 5;
		}
		unsigned long tm = timeGetTime();
		// calc bitrate
		if (tm >= CalcVSBitratePeriod + 4000) {
			CalcVSBitratePeriod = tm;
			if (bGroup) {
				BaseBitrate = g_ServerBandVideo;
				MaxBitrate = g_ServerBand;
			}
			CalculateBitrate(BaseBitrate, MaxBitrate, MaxFrameRate, send_mf.dwSVCMode);
		}

		switch(waitRes)
		{
		case WAIT_FAILED:
			DoExit = 1;
			break;
		case WAIT_OBJECT_0:
			DoExit = 2;
			break;
		case WAIT_OBJECT_0 + 1:
			while (g_FrameQueue->GetFrame(data, size, q_track, slayer) && size>=0) {
				const stream::Track track = static_cast<stream::Track>(q_track);
				mills = SEND_MILLS;
				retu = snd->SendFrame(data, size, track, &mills);
				if (retu==-2) {
					VS_AutoLock lock(&g_BandContr_Lock);
					g_BandContr->Add(timeGetTime(), (USHORT)size, q_track, (USHORT)SEND_MILLS, 1, slayer);
				}
				else if (retu==-3){
					g_FrameQueue->MarkFirstAsSend();
					DoExit = 3;
				}
				else if (retu==-1){
					DoExit = 4;
					break;
				}
				else {
					Wroten+=retu;
					VS_AutoLock lock(&g_BandContr_Lock);
					g_BandContr->Add(timeGetTime(), (USHORT)size, q_track, (USHORT)(SEND_MILLS-mills), 0, slayer);
					g_FrameQueue->MarkFirstAsSend();
				}
			}
		case WAIT_OBJECT_0 + 2:
			{
				ReceivedCommandProcess();
			}
		case WAIT_TIMEOUT:
			{
				//commands
				long csize = 0;
				if (g_cmdProc.GetSndCommand(pTmp, csize) && csize > 0) {
					g_FrameQueue->AddFrame(254, csize, pTmp, FRAME_PRIORITY_COMMAND);
					SetEvent(handles[1]);
				}

				// Audio
				BYTE* ptrIn = pTmp;
				BYTE* ptrOut = pTmpOut;
				size = g_vf.ReadAudio(pTmp, smpl_read);
				if (size > 0) {
					vc.Process((short*) pTmp,(int &)smpl_read);
					size = a_cmp->Convert(ptrIn, ptrOut+4, smpl_read);
					g_FrameQueue->AddFrame(1, size+4, ptrOut, FRAME_PRIORITY_AUDIO);
					SetEvent(handles[1]);
				}
				// Video
				size = g_vf.ReadVideo(pTmp, smpl_read);
				if (size > 0) {
					bool keyframe = false;
					tick = GetTickCount();

					if (g_WorkMode == WORK_MODE_DEMOGROUPCOMPRESS || g_WorkMode == WORK_MODE_DEMOGROUPCOMPRESS_SVC) {
						keyframe = smpl_read > 0;
						memcpy(pTmpOut+6, pTmp, size);
					} else {
						keyframe = !!InterlockedExchange(&g_RequestKeyFrame, 0);
						keyframe = keyframe || tick-last_key_frame_time > key_frame_interval;
						if (keyframe) {
							last_key_frame_time = tick;
							printf("KeyFrame generated\n");
						}
						size = codecManager->Convert(pTmp, pTmpOut+6, &keyframe);
					}

					if (size > 0) {

						BYTE key = keyframe ? 1 : 0;
						if (true /*!m_bNhp*/) {
							pTmpOut[0] = vframe_num;
							pTmpOut[1] = key != 1;
							*(unsigned int*)(pTmpOut+2) = tm;
						} else {
							VS_NhpVideoHeader* hdr = (VS_NhpVideoHeader*)pTmpOut;
							hdr->TimeStamp = tm;
							hdr->FrameId = vframe_num;
							hdr->FrameType = key != 1;
						}
						vframe_num++;

						g_FrameQueue->AddFrame(2, size+6, pTmpOut, FRAME_PRIORITY_VIDEO);
						SetEvent(handles[1]);
					}
				} else if (size < 0 && State == VideoFile::VF_PLAYLOOP) {
					g_vf.Reset();
				}
			}
		}
	} while (!DoExit);

	ContP->Hangup();
	g_LastNeedBand = 0;
	if (snd)
		if (snd->ClientType() != stream::ClientType::transmitter && snd->ClientType() != stream::ClientType::rtp && snd->ClientType() != stream::ClientType::direct_udp)
			snd.reset();
	free(pTmp);
	free(pTmpOut);
	VS_AutoLock lock(&g_BandContr_Lock);
	delete g_BandContr; g_BandContr = 0;
	delete a_cmp;
	delete codecManager;
	delete g_FrameQueue;
	CloseHandle(handles[1]);
	DTRACE(VSTM_AVIW, "Sender thread finished (reason: %d, wrote: %d bytes)\n", DoExit, Wroten);
	return 0;
}

DWORD WINAPI Receiver(LPVOID lppar)
{
	vs::SetThreadName("Receiver");
	CVSTrClientProc *ContP = reinterpret_cast<CVSTrClientProc *>(lppar);
	ResetEvent(hDes);


	boost::shared_ptr<VS_StreamClientReceiver> rcv = ContP->GetConferenseReceiver();
	if (!rcv) {
		ContP->Hangup();
		return -1;
	}

	// todo: use NHP
	//m_NhpBuff = VS_NhpBuffBase::Factory(m_bNhp, m_bIntercom, iTypeDecoder);


	VS_MediaFormat mf;
	char MyId[MAX_PATH];
	ContP->GetOtherName(MyId);
	ContP->GetMediaFormat(MyId, recv_mf);
	long ConfType = ContP->m_Status.CurrConfInfo->confType;
	if (ConfType == 5 || ConfType == 4) {
		recv_mf.SetVideo(1280, 720, VS_VCODEC_VPX);
		recv_mf.SetAudio(16000, VS_ACODEC_OPUS_B0914);
	}
	g_vf.SetRcvFormat(recv_mf);	/// for only audio case

	first_time_recv=true;
	DWORD LastRcvTime = timeGetTime();
	bool NeedKey = false;
	DWORD TimeKeyPrev = 0;

	int Read = 0;
	int retu = 0;
	unsigned long mils = 100;
	unsigned long size = 65536;
	unsigned char* buff = new unsigned char[size];
	unsigned char* vframe = new unsigned char[1000000];
	stream::Track track = {};
	HANDLE handles[2];
	handles[0] = hDes;
	handles[1] = rcv->GetReceiveEvent();
	VS_VS_InputBuffer	vs_In;

	int DoExit = 0;
	DWORD retWait = WAIT_OBJECT_0+1;
	while (!DoExit) {
		DWORD CurrTime = timeGetTime();
		switch(retWait)
		{
		case WAIT_FAILED:
			DoExit = 1;
			break;
		case WAIT_OBJECT_0:
			DoExit = 2;
			break;
		case WAIT_OBJECT_0+1: // read
			retu = rcv->ReceiveFrame(buff, 65536, &track, &mils);

			if (retu >= 0) {
				size = retu;
				if (NeedKey && CurrTime - TimeKeyPrev > 2000) {
					stream::Command cmd;
					cmd.RequestKeyFrame();
					g_cmdProc.AddCommand(cmd);
					NeedKey = false;
					DTRACE(VSTM_AVIW, "REQUEST KEYFRAME");
					TimeKeyPrev = CurrTime;
				}
				{
					if		(track == stream::Track::audio) {
						g_vf.WriteAudio(buff, size);
					}
					else if (track == stream::Track::video) {
						unsigned long usize = size;
						unsigned long VideoInterval = 0;
						bool key = false;
						if (!vs_In.Add(buff, usize, track) && track == stream::Track::video) {
							DTRACE(VSTM_AVIW, "PARTIAL frame detected, request key", key);
							NeedKey = true;
						}
						if (vs_In.Get(vframe, usize, vs::ignore<stream::Track>{}, VideoInterval, &key)>=0) {
							if(first_time_recv)
							{
								UpdateMediaFormat(vframe,key,usize);
								first_time_recv=false;
							}
							//DTRACE(VSTM_AVIW, "VI = %4d -- RI = %4d", VideoInterval, CurrTime-LastRcvTime);
							LastRcvTime = CurrTime;
							int res = g_vf.WriteVideo(vframe, usize, key, VideoInterval);
							if (res==-1) {
								DTRACE(VSTM_AVIW, "WAIT KEY frame");
								NeedKey = true;
							}
						}
					}
					else if (track == stream::Track::data) {
					}
					else if (track == stream::Track::command) {
						if (retu > 0) {
							stream::Command cmd(buff, size);
							g_cmdProc.AddCommand(cmd, false);
						}
					}
					else if (track == stream::Track::old_command) {
						stream::Command cmd;
						if (retu==1) {
							cmd.RequestKeyFrame();
							g_cmdProc.AddCommand(cmd, false);
						}
						else if (retu>0) {
							cmd.BrokerStat(buff, size);
							g_cmdProc.AddCommand(cmd, false);
						}
					}
				}
				Read+=retu;
			}
			else {// received size < 0
				if (retu==-1)
					DoExit = 3;
				else if (retu==-2) { // reconnect or re-read
				}
			}
			break;
		}
		if (DoExit)
			break;
		retWait = WaitForMultipleObjects(2, handles, FALSE, 16);
	}
	DTRACE(VSTM_AVIW, "Receiver thread finished (reason: %d, read %d bytes)\n", DoExit, Read);
	delete[] buff;
	delete[] vframe;
	if (rcv)
		if (rcv->ClientType() != stream::ClientType::transmitter && rcv->ClientType() != stream::ClientType::rtp && rcv->ClientType() != stream::ClientType::direct_udp)
			rcv.reset();
	return 0;
}

#define CMD_PARAM_AVI_FILE_NAME "--avi="
#define CMD_PARAM_LOGIN "--login="
#define CMD_PARAM_PASS "--pass="
#define CMD_PARAM_SERVER "--server="
#define CMD_PARAM_SERVICE "--service="
#define CMD_PARAM_WDIR "--wdir="
#define CMD_PARAM_INSTALL "--install"
#define CMD_PARAM_UNINSTALL "--uninstall"
#define CMD_PARAM_WORK_MODE "--mode="
#define CMD_PARAM_MAX_WIDTH "--maxw="
#define CMD_PARAM_MAX_HEIGHT "--maxh="
#define CMD_PARAM_USE_SVC "--svc="

bool InitFromCmdArgs(int argc, char* argv[])
{
	bool ret = true;
	for(int i=0; i < argc; i++)
	{
		const char* p = 0;
		if ((p=strstr(argv[i], CMD_PARAM_AVI_FILE_NAME))!=0)
		{
			p+=sizeof(CMD_PARAM_AVI_FILE_NAME)-1;
			g_AviFileName = p;
			if (!g_AviFileName.Length())
			{
				printf("ERROR: Invalid AVI file name param\n");
				ret = false;
			}
		}

		if ((p=strstr(argv[i], CMD_PARAM_LOGIN))!=0)
		{
			p+=sizeof(CMD_PARAM_LOGIN)-1;
			g_Login = p;
			if (!g_Login.Length())
			{
				printf("ERROR: Invalid login param\n");
				ret = false;
			}
		}

		if ((p=strstr(argv[i], CMD_PARAM_PASS))!=0)
		{
			p+=sizeof(CMD_PARAM_PASS)-1;
			g_Pass = p;
			if (!g_Pass.Length())
			{
				printf("ERROR: Invalid password param\n");
				ret = false;
			}
		}

		if ((p=strstr(argv[i], CMD_PARAM_SERVER))!=0)
		{
			p+=sizeof(CMD_PARAM_SERVER)-1;
			g_Server = p;
			if (!g_Server.Length())
			{
				printf("ERROR: Invalid server param\n");
				ret = false;
			}
		}

		if ((p=strstr(argv[i], CMD_PARAM_SERVICE))!=0)
		{
			p+=sizeof(CMD_PARAM_SERVICE)-1;
			g_ServiceName = p;
			if (!g_ServiceName.Length())
			{
				printf("ERROR: Invalid service name param\n");
				ret = false;
			}
		}

		if ((p=strstr(argv[i], CMD_PARAM_WDIR))!=0)
		{
			p+=sizeof(CMD_PARAM_WDIR)-1;
			g_WorkDir = p;
			if (!g_WorkDir) {
				printf("ERROR: Invalid working dir name param\n");
				ret = false;
			}
		}

		if ((p=strstr(argv[i], CMD_PARAM_INSTALL))!=0)
			g_ServiceInstall = true;

		if ((p=strstr(argv[i], CMD_PARAM_UNINSTALL))!=0)
			g_ServiceUnInstall = true;

		if ((p=strstr(argv[i], CMD_PARAM_WORK_MODE))!=0)
		{
			p+=sizeof(CMD_PARAM_WORK_MODE)-1;
			g_WorkMode = atoi(p);
		}

		if ((p=strstr(argv[i], CMD_PARAM_MAX_WIDTH))!=0)
		{
			p+=sizeof(CMD_PARAM_MAX_WIDTH)-1;
			g_maxWidth = atoi(p);
		}

		if ((p=strstr(argv[i], CMD_PARAM_MAX_HEIGHT))!=0)
		{
			p+=sizeof(CMD_PARAM_MAX_HEIGHT)-1;
			g_maxHeight = atoi(p);
		}

		if ((p=strstr(argv[i], CMD_PARAM_USE_SVC))!=0)
		{
			p+=sizeof(CMD_PARAM_USE_SVC)-1;
			g_useSvc = atoi(p);
		}
	}

	if (g_ServiceUnInstall && !!g_ServiceName)
		return true;

	if (!g_AviFileName)
	{
		printf("ERROR: Missing %s param\n", CMD_PARAM_AVI_FILE_NAME);
		ret = false;
	}

	if (!g_Login)
	{
		printf("ERROR: Missing %s param\n", CMD_PARAM_LOGIN);
		ret = false;
	}

	if (!g_Pass)
	{
		printf("ERROR: Missing %s param\n", CMD_PARAM_PASS);
		ret = false;
	}

	return ret;
}

void Usage()
{
	printf("Command line params:\n");
	printf("\t%sC:\\video\\file.avi\n", CMD_PARAM_AVI_FILE_NAME);
	printf("\t%slogin\n", CMD_PARAM_LOGIN);
	printf("\t%spassword\n", CMD_PARAM_PASS);
	printf("\t%sserver [optional param]\n", CMD_PARAM_SERVER);
	printf("\t%sworkig directory [optional param]\n", CMD_PARAM_WDIR);
	printf("\t%swork mode [optional param]\n", CMD_PARAM_WORK_MODE);
	printf("\t%smax width [optional param, for work mode = 1]\n", CMD_PARAM_MAX_WIDTH);
	printf("\t%smax height [optional param, for work mode = 1]\n", CMD_PARAM_MAX_HEIGHT);
	printf("\t%suse svc [optional param, for only! work mode = 1 & 3]\n", CMD_PARAM_USE_SVC);
}

void __stdcall ServiceMain( DWORD argc, LPTSTR *argv );
int VideoBotMain();
int StartAsConsole();
int StartAsService();
bool InstallAsService(const char* exe);
bool UnInstallAsService();

int main(int argc, char* argv[])
{
	if (!InitFromCmdArgs(argc, argv))
	{
		Usage();
		return 0;
	}

	if (g_ServiceInstall) {
		if (InstallAsService(argv[0]))
			printf("Successfully installed as %s service (%ld)\n", g_ServiceName.m_str, GetLastError());
		else
			printf("Can't install as %s service (%ld)\n", g_ServiceName.m_str, GetLastError());
		return 0;
	} else if (g_ServiceUnInstall) {
		if (UnInstallAsService())
			printf("Successfully uninstalled %s service (%ld)\n", g_ServiceName.m_str, GetLastError());
		else
			printf("Can't uninstall %s service (%ld)\n", g_ServiceName.m_str, GetLastError());
		return 0;
	}

	g_DieEvent = CreateEvent(0, true, false, 0);

	return !g_ServiceName ? StartAsConsole() : StartAsService();
}

bool InstallAsService(const char* exe)
{
	SC_HANDLE h = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE );
	if (!h)
		return false;

	char cmd[4096] = {0};
	GetCurrentDirectory(2048, cmd);
	size_t len = strlen(cmd);
	cmd[len] = '\\';
	sprintf_s(cmd+len+1, 2047, "%s %s%s %s%s %s%s %s%s %s%s %s%s", exe,
							CMD_PARAM_AVI_FILE_NAME, g_AviFileName.m_str,
							CMD_PARAM_LOGIN, g_Login.m_str,
							CMD_PARAM_PASS, g_Pass.m_str,
							!!g_Server ? CMD_PARAM_SERVER : "", !!g_Server ? g_Server.m_str : "",
							!!g_WorkDir ? CMD_PARAM_WDIR : "", !!g_WorkDir ? g_WorkDir.m_str : "",
							CMD_PARAM_SERVICE, g_ServiceName.m_str);
	if (g_WorkMode!=0)
		sprintf_s(cmd + strlen(cmd), 2000, " %s%d", CMD_PARAM_WORK_MODE, g_WorkMode);
	if (g_useSvc!=0)
		sprintf_s(cmd + strlen(cmd), 2000, " %s%d", CMD_PARAM_USE_SVC, g_useSvc);
	if (g_maxHeight!=0)
		sprintf_s(cmd + strlen(cmd), 2000, " %s%d", CMD_PARAM_MAX_HEIGHT, g_maxHeight);
	if (g_maxWidth!=0)
		sprintf_s(cmd + strlen(cmd), 2000, " %s%d", CMD_PARAM_MAX_WIDTH, g_maxWidth);

	HANDLE srv = CreateService(h, g_ServiceName, g_ServiceName, SC_MANAGER_ALL_ACCESS , SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START,
							SERVICE_ERROR_NORMAL, cmd, 0, 0, 0, 0, 0);
	if (!srv)
		return false;

	CloseHandle(h);
	CloseHandle(srv);

	return true;
}

bool UnInstallAsService()
{
	SC_HANDLE h = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE );
	if (!h)
		return false;

	SC_HANDLE srv = OpenService(h, g_ServiceName, SERVICE_STOP | DELETE);
	if(!srv)
	{
		CloseHandle(h);
		return false;
	}

	if(g_ss.dwCurrentState != SERVICE_STOPPED)
		ControlService(srv, SERVICE_CONTROL_STOP, &g_ss);

	//   
	DeleteService(srv);

	if (!srv)
		return false;

	CloseHandle(h);
	CloseHandle(srv);
	return true;
}

int StartAsConsole()
{
	int res = VideoBotMain();
	if (res!=0) printf("\nerror = %d\n", res);
	return res;
}

int StartAsService()
{
	//SC_HANDLE h = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE );
	//HANDLE service = CreateService(h, g_ServiceName, g_ServiceName, SC_MANAGER_ALL_ACCESS , SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START,
	//						SERVICE_ERROR_NORMAL, "d:\\work\\visi\\VSNA\\^VideoBot\\Debug\\VideoBot.exe --avi=D:\\films\\parkour\\2.avi --login=sip@video-port.com --pass=sipka --server=ru4.v-port.net#as --service=VideoBotTest3", 0, 0, 0, 0, 0);

	fclose( stdin );
	if (!g_WorkDir)
		g_WorkDir = "C:\\Videobot";
	wchar_t dirpath[MAX_PATH];
	mbstowcs(dirpath, g_WorkDir.m_str, MAX_PATH);
	MakeChangeDir(dirpath);
	const char   stdout_file_name[] = "Stdout.log";
	const char   stdout_backup_file_name[] = "Stdout.old.log";
	VS_AcsLog_HandleFileSize(stdout_file_name, stdout_backup_file_name, 1000000000);
	VS_RedirectOutput(stdout_file_name);
	SERVICE_TABLE_ENTRY   dispatchTable[] = {{ g_ServiceName, ServiceMain }, { 0, 0 }};
	if (!StartServiceCtrlDispatcher( dispatchTable ))
	{
//		VS_Server::log->Printf( "%s: StartServiceCtrlDispatcher( ... ) error: %u", s_name, GetLastError() );
		return -1;
	}
	return 0;
}

void WINAPI ServiceControl(DWORD dwControlCode);
void SetSomeServiceStatus(DWORD dwCurrentState,
						  DWORD dwWin32ExitCode,
						  DWORD dwWaitHint);
void StopSomeService();

void __stdcall ServiceMain( DWORD argc, LPTSTR *argv )
{
	g_ssHandle = RegisterServiceCtrlHandler(g_ServiceName, ServiceControl);
	if(!g_ssHandle) {
		printf("Error registering ServiceControl\n");
		return;
	}

	g_ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

	//    
	SetSomeServiceStatus(SERVICE_START_PENDING, NO_ERROR, 4000);

	//   
	SetSomeServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);

	VideoBotMain();
}

void WINAPI ServiceControl(DWORD dwControlCode) {
	//     
	switch(dwControlCode) {
		//  
	  case SERVICE_CONTROL_STOP: {
		  //   
		  SetSomeServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

		  // 
		  StopSomeService();

		  //   
		  SetSomeServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
		  break;
								 }
								 //   
	  case SERVICE_CONTROL_INTERROGATE: {
		  SetSomeServiceStatus(g_ss.dwCurrentState, NO_ERROR, 0);
		  break;
										}
	  default: {
		  SetSomeServiceStatus(g_ss.dwCurrentState, NO_ERROR, 0);
		  break;
			   }
	}
}

void SetSomeServiceStatus(DWORD dwCurrentState,
						  DWORD dwWin32ExitCode,
						  DWORD dwWaitHint)
{
	//   
	static DWORD dwCheckPoint = 1;

	//      ,   
	if(dwCurrentState == SERVICE_START_PENDING)
		g_ss.dwControlsAccepted = 0;
	else
		g_ss.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	g_ss.dwCurrentState = dwCurrentState;
	g_ss.dwWin32ExitCode = dwWin32ExitCode;
	//   
	g_ss.dwWaitHint = dwWaitHint;

	//      ,   
	//  
	if(dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED)
		g_ss.dwCheckPoint = 0;
	else
		g_ss.dwCheckPoint = dwCheckPoint++;

	//   
	SetServiceStatus(g_ssHandle, &g_ss);
}

void StopSomeService()
{
	SetEvent(g_DieEvent);

	if (WaitForSingleObject(g_hMain, 5000)==WAIT_OBJECT_0) {
		CloseHandle(g_DieEvent);
	}
}

int VideoBotMain()
{
	vs::InitOpenSSL();

	g_hMain = GetCurrentThread();

	VS_SetAppVer("VideoBot", "1.4.8");

	hDes = CreateEvent(NULL, TRUE, FALSE, NULL);

	std::string key_name = "TrueConf\\VideoBot";
	if (!!g_ServiceName) {
		key_name+="\\";
		key_name+=g_ServiceName;
	}
	CVSInterface* gaga = new CVSInterface("Client", 0, (char*) key_name.c_str(), true);
	g_pDtrase = new VS_DebugOut;
	auto ContP = std::make_shared<CVSTrClientProc>(gaga);
	ContP->TrInit(VS_FileTransfer::DEFAULT_FLAGS);

	VS_RegistryKey botRegistry(true, "", true, false);

	botRegistry.GetValue(&g_TimeSay, sizeof(g_TimeSay), VS_REG_INTEGER_VT, "TimeSay");
	botRegistry.GetValue(&g_TimeRec, sizeof(g_TimeRec), VS_REG_INTEGER_VT, "TimeRec");
	botRegistry.GetValue(&g_TimePlayrec, sizeof(g_TimePlayrec), VS_REG_INTEGER_VT, "TimePlayrec");

	VS_MediaFormat mf;
	mf.SetVideo(1280, 720, VS_VCODEC_VPX, 30);
	mf.SetAudio(16000, VS_ACODEC_OPUS_B0914);
	unsigned char lvl = g_lvlCaps.MBps2Level(mf.GetMBps());
	int rating = g_lvlCaps.LevelSnd2Rating(lvl);
	ContP->SetMediaFormat(mf, rating, lvl);

	if (g_useSvc == 0) {
		ContP->m_Status.MyInfo.ClientCaps.SetStreamsDC(ContP->m_Status.MyInfo.ClientCaps.GetStreamsDC() & ~VSCC_STREAM_CAN_USE_SVC);
	}
	ContP->m_Status.MyInfo.ClientCaps.SetVideoRcv(ContP->m_Status.MyInfo.ClientCaps.GetVideoRcv() & ~VSCC_VIDEO_DYNCHANGE);
	ContP->m_Status.MyInfo.ClientCaps.SetVideoSnd(ContP->m_Status.MyInfo.ClientCaps.GetVideoSnd());


	VS_SimpleStr server;

	if (!g_Server) {
		ContP->SetDiscoveryServise("trueconf.com");
		ContP->m_ServerList.GetTheBest(server);
	}
	else {
		ContP->SetDiscoveryServise("");
		server = g_Server;
		net::endpoint::ClearAllConnectTCP(server.m_str);
		net::endpoint::AddConnectTCP({ server, 4307, net::endpoint::protocol_tcp }, server.m_str);
	}

	if (ContP->Init(GetCurrentThreadId()))
		return -3;
	ContP->ReadProps();

	Sleep(rand()%500);
	// disable direct
	//ContP->m_dwSuppressDirectCon = 1;
	//ContP->m_DirectPort = 0; // disable accept ports
	CVSTrClientProc::m_dwUseNhp = 0; // disable NHP

	printf("Connect to %s server...\n", server.m_str);
	ContP->ConnectServer(server);

	struct RCV_Garbage{
		boost::shared_ptr<VS_StreamClientReceiver> ptr;
		VS_SimpleStr name;
		std::thread thread;
		vs::event die {true};

		~RCV_Garbage() { die.set(); thread.join(); }
	};
	std::vector<std::unique_ptr<RCV_Garbage>> receivers;

	DWORD TimeCall = 0;
	int CalcVSBitratePeriod = 0;
	DWORD TimeLogin = 0;
	bool bStop = false;
	MSG msg;
	DWORD SleepTime = 100;
	HANDLE handles[1];
	handles[0] = g_DieEvent;

	VS_RegistryKey confkey(true, REG_CurrentConfiguratuon);

	while (!bStop) {
		DWORD obj = MsgWaitForMultipleObjects(1, handles, 0, SleepTime, QS_POSTMESSAGE);

		if (obj == WAIT_OBJECT_0)
			bStop = true;

		while (PeekMessage(&msg,NULL,0,0, PM_REMOVE)) {
			if (msg.message == WM_USER+16) {
				DWORD ret = msg.wParam&0xffff;
				if (ret == VSTRCL_RCV_NEW || ret == VSTRCL_RCV_UPD || ret == VSTRCL_RCV_REM)
					PostThreadMessage(RCV_ThreadId, WM_USER, 0, msg.lParam);
				ClientStatus = msg.wParam;
				break;
			}
			else if (msg.message == WM_USER+17) {
				if (msg.wParam==6)
					streamsCreated = 1;
				else // if 5
					streamsCreated = -1;
			}
		}

		if (!(ClientStatus&STATUS_SERVAVAIL))
		{
			// TODO: krushnikov: check time of connect to server; reconnect
			continue;
		}
		DWORD CurrTime = timeGetTime();

		if (!(ClientStatus&STATUS_LOGGEDIN)) {
			if (!TimeLogin || CurrTime-TimeLogin > 10000) {
				printf("LoginUser(%s)\n", g_Login.m_str);
				ContP->LoginUser(g_Login, g_Pass, false);
				TimeLogin = CurrTime;
			}
		} else {	// Logged In
			if (confstate==ST_NORMAL)
			{
				if (ClientStatus&STATUS_INCALL) {
					DWORD ret = ClientStatus&0xffff;
					if (ret==VSTRCL_CONF_CALL || ret==VSTRCL_ACCEPT_OK) {
						printf("Accept(from=%s, conf=%s, type=%ld)\n",
								ContP->m_Status.ConfInfo[1].UserName.m_str,
								ContP->m_Status.ConfInfo[1].Conference.m_str,
								ContP->m_Status.ConfInfo[1].confType);
						Sleep(200);
						streamsCreated = 0;
						ContP->Accept();
						confstate = ST_CALL;
						TimeCall = CurrTime;
					}
					else if (ret==VSTRCL_CONF_INVMULTI) {
						if (g_WorkMode == WORK_MODE_DEMOGROUP || g_WorkMode == WORK_MODE_DEMOGROUPCOMPRESS || g_WorkMode == WORK_MODE_DEMOGROUPCOMPRESS_SVC) {
							printf("Accept Multi Incall from=%s\n", ContP->m_Status.ConfInfo[1].UserName.m_str);
							streamsCreated = 0;
							ContP->Accept();
							confstate = ST_CALL;
							TimeCall = CurrTime;
						}
						else {
							printf("Reject Multi Incall from=%s\n", ContP->m_Status.ConfInfo[1].UserName.m_str);
							ContP->Reject();
						}
					}
				}
				else {
					if (g_WorkMode == WORK_MODE_DEMOGROUP) {
						char cname[256] = {0};
						confkey.GetValue(cname, 256, VS_REG_STRING_VT, "confname");
						if (cname[0]) {
							streamsCreated = 0;
							ContP->Join(cname);
							confstate = ST_CALL;
							TimeCall = CurrTime;
						}
					}
				}
			}
			else if (confstate==ST_CALL) {
				if (ClientStatus&STATUS_CONFERENCE) {
					if (streamsCreated==1) {
						InterlockedExchange(&g_RequestKeyFrame, true);
						DWORD tid = 0;
						hConfTread[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Sender, ContP.get(), 0, &tid);
						hConfTread[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Receiver, ContP.get(), 0, &tid);
						confstate = ST_CONF;
						//TimeConf = CurrTime;
					}
					else if (streamsCreated==-1) {
						printf("\n HANGUP Stream not created!!!");
						ContP->Hangup();
						//TimeNorm = CurrTime;
						confstate = ST_NORMAL;
					}
				}
				else {
					if ( (CurrTime - TimeCall)> 15000) { // wait responce from transport
						printf("\n HANGUP in CALL state!!!");
						ContP->Hangup();
						//TimeNorm = CurrTime;
						confstate = ST_NORMAL;
					}
				}
			}
			else if (confstate == ST_CONF){
				if (ClientStatus&STATUS_CONFERENCE){
					//if ((CurrTime - TimeConf)>(dwMode!=1? LivingConfTimeNow : LivingConfTime)){ // conf live time
					//	printf("\n HANGUP by LivingConfTime = %d\n", (CurrTime - TimeConf)>>10);
					//	ContP->Hangup();
					//}
					if (g_WorkMode == WORK_MODE_DEMOGROUP) {
						if (ClientStatus&STATUS_REQINVITE) {
							char uname[256] = {0};
							ContP->GetOtherName(uname);
							ContP->InviteToMulti(uname);
						}
					}
				}
				else {
					printf("\n GOTO Normal from Conf\n");
					SetEvent(hDes);
					if (hConfTread[0]) {
						if (WaitForSingleObject(hConfTread[0], 5000)==WAIT_OBJECT_0) {
							CloseHandle(hConfTread[0]); hConfTread[0] = NULL;
							streamsCreated = 0;
						}
						confstate =ST_NORMAL;
					}
					if (hConfTread[1]) {
						if (WaitForSingleObject(hConfTread[1], 5000)==WAIT_OBJECT_0) {
							CloseHandle(hConfTread[1]); hConfTread[1] = NULL;
							streamsCreated = 0;
						}
						confstate =ST_NORMAL;
					}
					confstate = ST_NORMAL;
				}
			}

//			if (ContP->m_Status.dwStatus&STATUS_REQINVITE)
//			{
////				ContP->m_Status.ConfInfo[1].UserName =  ContP->m_Status.MyInfo.UserName;
//				ContP->InviteReply(0);
//				ContP->SetReqInviteStatus(false);
//			}


			int action = LOWORD(ClientStatus);
			if (action == VSTRCL_RCV_ROLE) {
				VS_Container* cnt = (VS_Container*)ContP->m_RoleEventContainers.GetList((int)msg.lParam);
				if (cnt) {
					int32_t role = PR_EQUAL; cnt->GetValue(ROLE_PARAM, role);
					int32_t q = RET_INQUIRY; cnt->GetValue(TYPE_PARAM, q);
					const char* user = cnt->GetStrValueRef(USERNAME_PARAM);
					if (!user)
						user = ContP->m_Status.MyInfo.UserName;
					if (q == RET_INQUIRY)
						ContP->AnswerRole((char*) user, role, RIA_POSITIVE);
					InterlockedExchange(&g_RequestKeyFrame, true);
				}
			}else if (action==VSTRCL_RCV_NEW) {
				boost::shared_ptr<VS_StreamClientReceiver> rcv;
				long fltr = 0;
				char name[256] = {0};
				action = ContP->GetRcvAction(msg.lParam, name, rcv, fltr);
				if (action==1)
				{
					auto g = std::make_unique<RCV_Garbage>();
					g->name = name;
					g->ptr = rcv;
					g->thread = std::thread([rcv, die_ev = g->die.native_handle()]() {
						stream::Track track = {};
						HANDLE handles[2];
						handles[0] = die_ev;
						handles[1] = rcv->GetReceiveEvent();
						unsigned long mils = 100;
						unsigned long size = 65536;
						auto buff = std::make_unique<unsigned char[]>(size);
						int DoExit = 0;
						DWORD retWait = WAIT_OBJECT_0 + 1;
						while (!DoExit)
						{
							switch (retWait)
							{
							case WAIT_FAILED:
								DoExit = 1;
								break;
							case WAIT_OBJECT_0:
								DoExit = 2;
								break;
							case WAIT_OBJECT_0 + 1: // read
							{
								auto retu = rcv->ReceiveFrame(buff.get(), size, &track, &mils);
								if (retu == -1)
									DoExit = 3;
							}
							break;
							};
							if (DoExit)
								break;
							retWait = WaitForMultipleObjects(2, handles, FALSE, 16);
						}
					});
					receivers.push_back(std::move(g));
					InterlockedExchange(&g_RequestKeyFrame, true);
				}
			} else if (action==VSTRCL_RCV_REM) {
				boost::shared_ptr<VS_StreamClientReceiver> rcv;
				long fltr = 0;
				char name[256]; *name = 0;
				action = ContP->GetRcvAction(msg.lParam, name, rcv, fltr);
				if (action==3) {
					receivers.erase(std::remove_if(receivers.begin(), receivers.end(), [&name](auto&& p) {
						return p->name == name;
					}), receivers.end());
				}
			}
			// action

			if (ClientStatus&STATUS_COMMAND) {
				VS_SimpleStr mess(10000);
				char from[256] = {0};
				ContP->GetCommand(msg.lParam, from, mess);
				if (mess== CCMD_RECORD_QUERY)
					ContP->SendCommand(CCMD_RECORD_ACCEPT, from);
			}
		} // Logged In
	} // while (!bStop)

	SetEvent(hDes);
	ContP->Hangup();
	ContP->LogoutUser(false);
	//if (hConfTread[0]) {
	//	if (WaitForSingleObject(hConfTread[0], 5000)==WAIT_OBJECT_0) {
	//		CloseHandle(hConfTread[0]); hConfTread[0] = NULL;
	//		streamsCreated = 0;
	//	}
	//	confstate =ST_NORMAL;
	//}
	//if (hConfTread[1]) {
	//	if (WaitForSingleObject(hConfTread[1], 5000)==WAIT_OBJECT_0) {
	//		CloseHandle(hConfTread[1]); hConfTread[1] = NULL;
	//		streamsCreated = 0;
	//	}
	//	confstate =ST_NORMAL;
	//}
	delete g_pDtrase; g_pDtrase = 0;
	return 0;
}