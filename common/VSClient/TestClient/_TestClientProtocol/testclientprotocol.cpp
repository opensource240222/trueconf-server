#pragma comment (lib,"glew32s.lib")

#include "Audio/EchoCancel/SpeexEchoCancel.h"
#include "net/EndpointRegistry.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_SimpleStr.h"
#include "streams/Client/VS_StreamClient.h"
#include "streams/Client/VS_StreamClientReceiver.h"
#include "streams/Client/VS_StreamClientSender.h"
#include "Transcoder/AudioCodec.h"
#include "Transcoder/VideoCodec.h"
#include "Transcoder/VS_BitStreamBuff.h"
#include "Transcoder/VS_SpeexAudioCodec.h"
#include "transport/Client/VS_TransportClient.h"
#include "VSClient/VS_Dmodule.h"
#include "VSClient/VS_UserList.h"
#include "VSClient/VSTrClientProc.h"

#include "audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <fstream>

enum ConfState
{
	ST_NORMAL,
	ST_CALL,
	ST_CONF,
	ST_CONFEND,
};
/*
AudioCodec* VS_RetriveAudioCodec(int Id, bool isCodec)
{
	return 0;
}
VideoCodec* VS_RetriveVideoCodec(int Id, bool isCodec, unsigned int typeHardware)
{
	return 0;
}
*/
/*void VS_GetAppPorts(unsigned long* &ports, unsigned long &num) {
	num = 0;
}*/

typedef CVSTrClientProc test_client_protocol;
/**************************************************************************************************/

static bool bStop = false;
static DWORD ClientStatus = 0;
static HANDLE hConfTread[] = {0, 0};
static DWORD dwConfTreadId = 0;
static HANDLE hDes = 0;
static HANDLE hList = 0;
static DWORD NUMBER_OF_PART = 0;
static DWORD dwInstallUser = 0;
static DWORD s_dwBitrate = 100;
static DWORD SucsTest1 =0;
static DWORD streamsCreated = 0;

struct User
{
	VS_SimpleStr Us;
	VS_SimpleStr Fs;
	VS_SimpleStr Ls;
	VS_SimpleStr Em;
	VS_SimpleStr Ps;
};

struct All
{
	User me;
	User he;
	VS_SimpleStr conf;
	VS_SimpleStr endp;
};

All g_All;
DWORD WINAPI Conference(LPVOID lp);
static DWORD RCV_ThreadId = 0;
void RecalculateLiveTime();

#define MAX_RECIEVERS_NUM 36

static VS_SimpleStr g_named_conf = "";

/**************************************************************************************************/

DWORD WINAPI ReadRcv(LPVOID lpParameter)
{
	RCV_ThreadId = GetCurrentThreadId();
	test_client_protocol *ContP = (test_client_protocol*)lpParameter;
	ResetEvent(hDes);

	const int BuffLen = 65536;
	DWORD	ret = WAIT_TIMEOUT, i = 0;
	char	*ReadBuf = new char[BuffLen];
	HANDLE	handles[1 + MAX_RECIEVERS_NUM];/// event change list, read events
	boost::shared_ptr<VS_StreamClientReceiver> RCV[MAX_RECIEVERS_NUM];
	handles[0] = hDes;
	VS_SimpleStr Names[MAX_RECIEVERS_NUM];

	DWORD	Num = 0;
	ret = WAIT_TIMEOUT;
	stream::Track track = {};
	unsigned long mils = 0, timeout = 0;
	int rcvRetVal = 0;
    MSG msg;
	int retval[3] = { 0 };
	/// force to create message queue
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	while(TRUE)
	{
		DWORD j;
		if (PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_REMOVE)) {
			if (msg.message==WM_USER) {
				boost::shared_ptr<VS_StreamClientReceiver> rcv;
				long fltr = 0;
				char name[256]; *name = 0;
				int action = ContP->GetRcvAction(msg.lParam, name, rcv, fltr);
				switch(action)
				{
				case 1: // connect
					Names[Num] = name;
					RCV[Num] = rcv;
					RCV[Num]->ReceiveFrame(ReadBuf, BuffLen, &track, &mils);
					Num++;
					printf("\n^^^^^^^^new rcv with fltr = %ld : total = %ld ^^^^^^^ \n",fltr, Num);
					break;
				case 2:
					break;
				case 3:
					for (j = 0; j<Num; j++) {
						if (Names[j]==(VS_SimpleStr)name)
							break;
					}
					if (j==Num) break;// not found
					RCV[j]->CloseConnection();
					RCV[j].reset();
					printf("\n &&&&&&&&&&&&&& RCV %s CLOSED BY PROTOCOL &&&&&&&&&&&\n", Names[j].m_str);
					Num--;
					for (; j<Num; j++) {
						RCV[j] = RCV[j+1];
						Names[j] = Names[j+1];
					}
					RCV[j].reset(); Names[j].Empty();
					break;
				default:
					printf("\n &&&&&&&&&&&&&& PROTOCOL ERROR &&&&&&&&&&&\n");
					break;
				}
			}
		}

		DWORD ReadNum =Num;
		for (i = 0; i<ReadNum;) {
			if (RCV[i]->ConnectType()!=vs_stream_client_connect_type_unconnected) {
				handles[i+1] = RCV[i]->GetReceiveEvent(); i++;
			}
			else {
				VS_SimpleStr tmpN = Names[i];
				boost::shared_ptr<VS_StreamClientReceiver> tmpR = RCV[i];
				DWORD j = i;
				for (; j< Num-1; j++) {
					RCV[j] = RCV[j+1];
					Names[j] = Names[j+1];
				}
				RCV[j] = tmpR;
				Names[j] = tmpN;
				ReadNum--;
			}
		}
		ret = MsgWaitForMultipleObjects(ReadNum+1, handles, FALSE, 500, QS_ALLPOSTMESSAGE);

		if		(ret==WAIT_OBJECT_0) {/// die
			break;
		}
		else if (ret>=WAIT_OBJECT_0+1 && ret<WAIT_OBJECT_0+Num+1) { // events from rcv for read it
			i = ret-(WAIT_OBJECT_0+1); /// read rcv number
			rcvRetVal = RCV[i]->ReceiveFrame(ReadBuf, BuffLen, &track, &mils);
			if (rcvRetVal==-1) {
				printf("\n &&&&&&&&&&&&&& RCV %s CLOSED BY RETVAL &&&&&&&&&&&\n", Names[i].m_str);
				Num--;
				RCV[i].reset();
				for (j = i; j<Num; j++) {
					RCV[j] = RCV[j+1];
					Names[j] = Names[j+1];
				}
				RCV[j].reset(); Names[j].Empty();
			}
			else {
				unsigned long ticks = GetTickCount();
				if (rcvRetVal > 0) {
					if (track == stream::Track::audio)
						retval[0] += rcvRetVal;
					else if (track == stream::Track::video)
						retval[1] += rcvRetVal;
					else
						retval[2] += rcvRetVal;
				}
				if (ticks - timeout > 2000) {
					printf("\n RCV=%2ld read A=%3d | V=%5d | O=%2d", ReadNum, retval[0] / 256, retval[1] / 256, retval[2] / 256);
					retval[0] = retval[1] = retval[2] = 0;
					timeout = ticks;
				}
			}
		}
		else if (ret<WAIT_OBJECT_0+Num+1) { // any post message
		}
		else if (ret == WAIT_FAILED) {
			puts("ERORRERORRERORRERORRERORRERORRERORRERORRERORRERORR");
			Sleep(10);
		}
	}

	if (ReadBuf) delete[] ReadBuf;
	puts("Recievers thread destroyed");
	RCV_ThreadId = 0;
	return 0;
}


# include "..\..\VS_StreamPacketManagment.h"


DWORD WINAPI Conference(LPVOID lppar)
{
	test_client_protocol *ContP = reinterpret_cast<test_client_protocol *>(lppar);
	ResetEvent(hDes);

	boost::shared_ptr<VS_StreamClientSender> snd = ContP->GetConferenseSender();
	boost::shared_ptr<VS_StreamClientReceiver> rcv = ContP->GetConferenseReceiver();
	if (!snd) snd.reset(new VS_StreamClientSender());
	if (!rcv) rcv.reset(new VS_StreamClientReceiver());
	VS_SendFrameQueueBase *frameque = VS_SendFrameQueueBase::Factory(false);
	VS_ControlBandBase *contrBand = VS_ControlBandBase::Factory(false, false);
	contrBand->SetQueuePointers(frameque, snd->GetSendEvent());

	int Wroten= 0, Read = 0;
	int retu = 0;
	unsigned char traks[3] = {1, 2, 255};
	stream::Track track = stream::Track::audio;
	unsigned long mils = 100;
	char *buffer = new char[65536];
	HANDLE Events[2];
	static int incremeter = 0;
	Events[0] = hDes;
	Events[1] = rcv->GetReceiveEvent();
	bool DoNotRise = false;
	DWORD dwBitrate = s_dwBitrate/2;
	contrBand->GetVideoBandwidth(dwBitrate);

	int StartTime = timeGetTime();
	int CurrTime, buffSize ;
	int sleep_time = 0;
	DWORD retWait = WAIT_OBJECT_0+1;
	while (1){
		bool Break = false;
		switch(retWait)
		{
		case WAIT_OBJECT_0: // return
			printf("\n Main thread RETURN");
			Break = true;
			break;
		case WAIT_OBJECT_0+1: // read
			retu = rcv->ReceiveFrame(buffer, 65536, &track, &mils);
			if (retu==-2)  {
				puts("MAIN RECEIVER TimeOUT\n");
			}
			else if (retu==-1)	{
			}
			else {
				Read+=retu;
				if (track == stream::Track::old_command && retu > 1) {
					stream::Command cmd;
					cmd.BrokerStat(buffer, retu);
					contrBand->SetReceivedCommand(cmd);
				}
			}
			break;
		case WAIT_TIMEOUT:
			buffSize = rand()%2400;
			if (buffSize > 1200)
				buffSize = 1200;
			if (buffSize < 200)
				buffSize = 96;

			for (int i = 0; i< buffSize; i++)
				buffer[i] = (char)incremeter++;

			mils = 50;
			track = buffSize == 96 ? stream::Track::audio : stream::Track::video;
			retu = snd->SendFrame(buffer, buffSize, track, &mils);
			if (retu==-2) {
				printf("\n Main snd timeout: ");
				contrBand->Add(timeGetTime(), (USHORT)buffSize, (USHORT)track, 50, 1, 0);
			}
			else if (retu==-3){
			}
			else if (retu==-1){
				puts("MAIN SENDER DISCONECT\n");
				Break = true;
				break;
			}
			else {
				Wroten+=retu;
				contrBand->Add(timeGetTime(), (USHORT)buffSize, (USHORT)track, (USHORT)(50-mils), 0, 0);
			}
			break;
		default:
			Sleep(10);
			break;
		}
		if (Break) break;
		int wait_num = 1;
		if (rcv->ConnectType()==vs_stream_client_connect_type_connect) {
			Events[1] = rcv->GetReceiveEvent();
			wait_num = 2;
		}
		CurrTime = timeGetTime() - StartTime;
		sleep_time = 8*Wroten/dwBitrate - CurrTime;
		if (CurrTime>500) {
			DWORD newdwBitrate = contrBand->GetVideoBandwidth(s_dwBitrate);
			if (newdwBitrate!=dwBitrate) {
				printf("\n!!! new Bitrate = %ld, prev write bytes = %d", dwBitrate, Wroten);
				dwBitrate = newdwBitrate;
			}
			StartTime = timeGetTime();
			Wroten = 0;
		}

		if (sleep_time<0) sleep_time = 0;

		retWait = WaitForMultipleObjects(wait_num, Events, FALSE, sleep_time);
	}
	printf("\n---read %d bytes\n", Read);
	printf("\n---write %d bytes\n", Wroten);
	if ( Read>0 && Wroten>0 ) SucsTest1 |= (4|8);
	else SucsTest1 |= 8;

	delete[] buffer;
	snd.reset();
	rcv.reset();
	if (contrBand) delete contrBand; contrBand = 0;
	if (frameque) delete frameque; frameque = 0;


	WaitForSingleObject(hConfTread[1], 5000);
	puts("Conference thread destroyed");
	PostMessage(ContP->m_ExternalWindow, WM_USER, 0, 0);
	RecalculateLiveTime();
	return 0;
}


/**************************************************************************************************/
static	int TimeConf = 0, TimeCall = 0, TimeNorm = 0, TimeEndConf = 0;
static	ConfState confstate = ST_NORMAL;
static int LivingConfTime = 300000;
static int LivingConfTimeNow = 0;

void RecalculateLiveTime()
{
	double pok = log((LivingConfTime+1)/40000.)/log(4.);
	LivingConfTimeNow = (int)(40.*pow(4., rand()*pok/RAND_MAX)*1000);
}

void ConfCreate(test_client_protocol *ContP, DWORD dwMode)
{
	int CurrTime = timeGetTime();
	if (confstate==ST_NORMAL) {
		if (dwInstallUser&1) { // nechetniy
			if (ClientStatus&STATUS_INCALL) {
				printf("\n Accept a call...");
				ContP->Accept();
				confstate = ST_CALL;
				TimeCall = CurrTime;
			}
		}
		else {
			if ((CurrTime - TimeNorm)>10000 + LivingConfTimeNow/2) { // standby time
				if (dwMode!=1) {
					char Em[256]; DWORD dwPart = 0;
					dwPart = ((rand()%NUMBER_OF_PART))|1;
					sprintf(Em, "crash_un%ld", dwPart);
					g_All.he.Em = Em;
				}
				else {
					g_All.he.Em = "q@q.com";
				}
				printf("\n Try to call to %s", g_All.he.Em.m_str);
				if (!ContP->PlaseCall(g_All.he.Em)) {
					TimeCall = CurrTime;
					confstate = ST_CALL;
				}
			}
		}
	}
	else if (confstate==ST_CALL) {
		if (ClientStatus&STATUS_CONFERENCE) {
			if (streamsCreated==1) {
				if (dwInstallUser&1) { // nechetniy
					RecalculateLiveTime();
				}
				else
					LivingConfTimeNow = LivingConfTime;
				hConfTread[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Conference, ContP, 0, &dwConfTreadId);
				confstate = ST_CONF;
				TimeConf = CurrTime;
			}
			else if (streamsCreated==-1) {
				printf("\n HANGUP Stream not created!!!");
				ContP->Hangup();
				TimeNorm = CurrTime;
				confstate = ST_NORMAL;
			}
		}
		else {
			if ( (CurrTime - TimeCall)> 15000) { // wait responce from transport
				printf("\n HANGUP in CALL state!!!");
				ContP->Hangup();
				TimeNorm = CurrTime;
				confstate = ST_NORMAL;
			}
		}
	}
	else if (confstate == ST_CONF){
		if (ClientStatus&STATUS_CONFERENCE){
			if ((CurrTime - TimeConf)>(dwMode!=1? LivingConfTimeNow : LivingConfTime)){ // conf live time
				printf("\n HANGUP by LivingConfTime = %d\n", (CurrTime - TimeConf)>>10);
				ContP->Hangup();
			}
		}
		else {
			printf("\n GOTO Normal from Conf, LivingConfTime = %d\n", (CurrTime - TimeConf)>>10);
			if (hConfTread[0]) {
				SetEvent(hDes);
				if (WaitForSingleObject(hConfTread[0], 5000)==WAIT_OBJECT_0) {
					CloseHandle(hConfTread[0]); hConfTread[0] = NULL;
					streamsCreated = 0;
				}
				else {
				}
				confstate =ST_NORMAL;
				TimeNorm = CurrTime;
			}
		}
	}
}

void ConfCreate4(test_client_protocol *ContP, bool isBroadcast)
{
	int CurrTime = timeGetTime();
	switch(confstate)
	{
	case ST_NORMAL:
		if (CurrTime - TimeNorm > 10000 + rand()%5000) { // idle
			if (!isBroadcast) {
				ContP->Join("@c_test", "test");
			}
			else {
				ContP->Join(g_named_conf, 0, CT_MULTISTREAM);
			}
			TimeCall = CurrTime;
			confstate = ST_CALL;
		}
		break;
	case ST_CALL:
		if (ClientStatus&STATUS_CONFERENCE) {
 			if (streamsCreated==1) {
				hConfTread[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadRcv, ContP, 0, &dwConfTreadId);
				hConfTread[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Conference, ContP, 0, &dwConfTreadId);
				confstate = ST_CONF;
				TimeConf = CurrTime;
			}
			else if (streamsCreated==-1) {
				ContP->Hangup();
				confstate = ST_CONFEND; TimeEndConf = CurrTime;
			}
		}
		else {
			if (CurrTime - TimeCall > 25000) { // wait responce from transport
				ContP->Hangup();
				TimeNorm = CurrTime;
				confstate = ST_NORMAL;
			}
		}
		break;
	case ST_CONF:
		if (ClientStatus&STATUS_CONFERENCE){
			if (CurrTime - TimeConf > LivingConfTime){ // conf live time
				ContP->Hangup();
				TimeEndConf = CurrTime;
				confstate = ST_CONFEND;
			}
		}
		else {
			TimeEndConf = CurrTime;
			confstate = ST_CONFEND;
		}
		break;
	case ST_CONFEND:
		if (ClientStatus&STATUS_CONFERENCE){
			if (CurrTime - TimeEndConf > 5000){ // max time to end...
				confstate =ST_NORMAL;
				TimeNorm = CurrTime;
			}
		}
		else {
			int TrNum = 0;
			TrNum+=hConfTread[0]!=NULL;
			TrNum+=hConfTread[1]!=NULL;
			if (TrNum) {
				SetEvent(hDes);
				WaitForMultipleObjects(TrNum, hConfTread, TRUE, 5000);
				if (hConfTread[0]) CloseHandle(hConfTread[0]); hConfTread[0] = NULL;
				if (hConfTread[1]) CloseHandle(hConfTread[1]); hConfTread[1] = NULL;
				confstate =ST_NORMAL;
				TimeNorm = CurrTime;
			}
		}
		break;
	}
}


#if defined(_VPX_INCLUDED_) || defined(_H323GATEWAYCLIENT_)
#	pragma comment (lib,"libvpx.lib")
#else

VPXCodec* VS_RetriveVPXCodec(int tag, bool isCoder)
{
	return 0;
}

#endif

#if defined(_H264_INCLUDED_) || defined(_H323GATEWAYCLIENT_)
//#	pragma comment (lib,"h264codec.lib")
#else

H264Codec* VS_RetriveH264Codec(int Id, bool isCodec)
{
	return 0;
}

#endif

class NVVideoCodec;
NVVideoCodec* VS_RetriveNVCodec(int Id, bool isCoder)
{
	return 0;
}

static HINSTANCE ippVCodecExtLib = 0;
static bool ippVCodecExtLibLoaded = false;


//HINSTANCE VS_LoadCodecsLib()
//{
//	static HINSTANCE ippCodecExtLib = 0;
//	static bool ippCodecExtLibLoaded = false;
//	if (ippCodecExtLib == 0 && !ippCodecExtLibLoaded) {
//		ippCodecExtLib = LoadLibrary("CodecsDll");
//		ippCodecExtLibLoaded = true;
//	}
//	return ippCodecExtLib;
//}

// torrents
bool isTorrentFileSent = false;
CVSInterface *m_client;

void Torrent_CallProcessF(_variant_t *pVar, int count )
{
	VARIANT var;

	SAFEARRAYBOUND rgsabound[1];
	SAFEARRAY * psa;
	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = count;
	psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
	if (psa==0)
		return ;

	var.parray=psa;
	var.vt= VT_ARRAY | VT_VARIANT;
	for (long i = 0; i<count; i++)
		SafeArrayPutElement(psa, &i, &pVar[i]);

	m_client->Process(RUN_COMMAND, "TorrentFileTransfer", &var);
}



long ExtractVars(VARIANT *&vars, VARIANT *pVar)
{
	vars = 0;
	int num = 0;
	if (pVar->vt==(VT_ARRAY|VT_VARIANT)) {
		long l, u;
		SAFEARRAY *psa = pVar->parray;
		SafeArrayGetLBound(psa, 1, &l);
		SafeArrayGetUBound(psa, 1, &u);
		num = u - l + 1;
		if (num > 0) {
			vars = new VARIANT[num];
			for(long i = 0; i < num; ++i) {
				VariantInit(&vars[i]);
				SafeArrayGetElement(psa, &i, &vars[i]);
			}
		}
	}
	return num;
}

typedef void ( *GUICallback)( _variant_t *var  );

void TorrentGUICallback( _variant_t *pVar  )
 {
	_variant_t* var=(_variant_t*)pVar;

	VARIANT *vars = 0;
	int count = ExtractVars(vars, var);
	if (count < 1) return;
	_variant_t data[10];

	WCHAR * name = vars[0].bstrVal;

	if ( wcscmp(name, L"OnAcceptFile") == 0 )
	{
		const wchar_t *file = vars[1].bstrVal;
		const wchar_t *from = vars[2].bstrVal;
		const wchar_t *info_hash = vars[4].bstrVal;

		wprintf(L"accept file \"%s\" from %s", file, from);

		data[0] = L"ConfirmDownload";
		data[1] = info_hash;
		data[2] = true;
		data[3] = (std::wstring(L"torret_from_") + from + L"_to_crash_un" + std::to_wstring(dwInstallUser) +  L"\\").c_str();
		Torrent_CallProcessF(data, 4);

	} else
	{
	}


 }

void SendTorrentFiles() {
	if (isTorrentFileSent == true) return;
	isTorrentFileSent = true;

	_variant_t data[10];
	data[0] = L"Init";
	data[1] = (unsigned long) &TorrentGUICallback;

	Torrent_CallProcessF(data, 2);

	std::wifstream f("torrent_config.txt");
	if (!f) return;

	std::wstring line;
	while (std::getline(f, line)) {
		std::wistringstream iss(line);
		std::wstring file_path, send_to;
		int index;

		iss >> index >> file_path;
		if (index == dwInstallUser) {
			while (iss >> send_to) {
				data[0] = L"SendFile";
				data[1] = file_path.c_str();
				data[2] = send_to.c_str();
				wprintf(L"send file \"%s\" to %s\n", file_path.c_str(), send_to.c_str());
				Torrent_CallProcessF(data, 3);
			}
			break;
		}
	}
	wprintf(L"Waiting for torrent files..\n");
}

void SendTorrentFiles1()
{
	if (isTorrentFileSent == true) return ;
	isTorrentFileSent = true;

	_variant_t data[10];
	data[0] = L"Init";
//	GUICallback call = (GUICallback)0x00c7a070;
	GUICallback call = TorrentGUICallback;

	unsigned long address = (unsigned long)call;
	data[1] = address ;

	Torrent_CallProcessF(data, 2);

	FILE *f = fopen("torrent_config.txt", "rt");
	if( !f ) return;

	std::wstring filename;

	while ( true )
	{
		wchar_t buffer[1024];
		buffer[0] = 0;
		fgetws(buffer, 1024, f);
		if (buffer[0] == 0) break;

		buffer[wcslen(buffer) - 1] = 0;

		if (buffer[0] == 0) filename = L"";
		else
		if ( filename.empty() ) filename = buffer;
		else
		{
			if (dwInstallUser == 0) {
				data[0] = L"SendFile";
				data[1] = filename.c_str();
				data[2] = buffer;
				wprintf(L"send file \"%s\" to %s\n", filename.c_str(), buffer);
				Torrent_CallProcessF(data, 3);
			}
		}
	}
	wprintf(L"Waiting for torrent files..\n");
	fclose(f);
}

int main(int argc, char* argv[])
{
	VS_RegistryKey::InitDefaultBackend("registry:force_lm=false");
	VS_SetAppVer("_TestClientProtocol", "4.0.0");
	DWORD dwMode = 0;
	hDes = NULL;
	MSG msg;
	test_client_protocol *ContP = NULL;
	char server[256];
	char temp[256];
	unsigned int times_number = 0;
	hList = CreateEvent(NULL, FALSE, FALSE, NULL);
	hDes = CreateEvent(NULL, TRUE, FALSE, NULL);
	srand( (unsigned)time( NULL ) );
	DWORD SleepTime = 100;
	g_pDtrase = new VS_DebugOut;

	if (argc<5) {
		puts("ServerEndpoint, mode, crash user number, total crash users, [send bitrate] [ConfLivingTime]");
		return 0;
	}
	else {
		strcpy(server, argv[1]);
		dwMode = atoi(argv[2]);
		dwInstallUser = atoi(argv[3]);
		if (dwMode == 7 || dwMode == 8) {
			g_All.me.Us = argv[3];
			g_All.me.Ps = argv[4];
			if (!g_All.me.Us || !g_All.me.Ps)
			{
				puts("Invalid args: login or password\n");
				return 0;
			}
			sprintf(temp, "Visicron\\Test\\%s%ld", GUI_version, dwInstallUser);
			//VS_RegistryKey::SetRoot(temp);
			CVSInterface *gaga = new CVSInterface("Client", 0, temp, true);
			ContP = new test_client_protocol(gaga);
		} else {
			NUMBER_OF_PART = atoi(argv[4]);
			if (!dwInstallUser || !NUMBER_OF_PART) {
				puts("Invalid args: crash user number or total crash users");
				return 0;
			}
			if (argc > 5)
				s_dwBitrate = atoi(argv[5]);
			if (argc > 6)
				LivingConfTime = atoi(argv[6])*1000;
			TimeNorm = timeGetTime();
			RecalculateLiveTime();

			sprintf(temp, "Visicron\\Test\\%s%ld", GUI_version, dwInstallUser);
			//VS_RegistryKey::SetRoot(temp);
			CVSInterface *gaga = new CVSInterface("Client", 0, temp, true);
			ContP = new test_client_protocol(gaga);
			ContP->SetDiscoveryServise("pca.ru");
			VS_MediaFormat fmt;
			fmt.SetVideo(640, 480, VS_VCODEC_VPX, 15, 0, 1);
			fmt.SetAudio(16000, VS_ACODEC_SPEEX);
			ContP->SetMediaFormat(fmt, 800, 55, 55);
		}

		if(dwMode == 5)
			g_named_conf = argv[7];
	}

	m_client = (CVSInterface *)ContP;

	static bool case7 = false;	// try login

	if (ContP->Init(GetCurrentThreadId())) bStop = true;
	ContP->ReadProps();

	Sleep(rand()%500);
	if (dwMode != 7)
	{
		char Un[256], Ps[256];
		sprintf(Un, "crash_un%ld", dwInstallUser);
		sprintf(Ps, "crash_pass");
		g_All.me.Us = Un;
		g_All.me.Ps = Ps;

	}

	ContP->m_dwSuppressDirectCon = 1;
	ContP->m_DirectPort = 0; // disable accept ports
	FILE * f = fopen("config.srv", "rt");
	if (f) {
		char txt[3][200];
		fscanf(f, "%s\n", txt[0]);
		fscanf(f, "%s\n", txt[1]);
		fscanf(f, "%s\n", txt[2]);
		net::endpoint::ClearAllConnectTCP(txt[0]);
		if (net::endpoint::AddConnectTCP({ txt[1], atoi(txt[2]), net::endpoint::protocol_tcp }, txt[0]) == 1)
			strcpy(server, txt[0]);
		fclose(f); f = 0;
	}
	else
	if (dwMode != 8){
		VS_SimpleStr srv;
		ContP->m_ServerList.GetTheBest(srv);
		strcpy(server, srv);
	}
	if (ContP->ConnectServer(server)) bStop = true;

	DWORD TimeLogin = timeGetTime();
	DWORD TimeLogout = timeGetTime();
	DWORD avTime = timeGetTime();
	HANDLE handles[1];
	unsigned int my_int = 0;
	bool ab_req = false;
	while (!bStop) {
		DWORD obj = MsgWaitForMultipleObjects(0, handles, 0, SleepTime, QS_POSTMESSAGE);

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

		// case 7
		if (dwMode == 7)
		{
			if (!(ClientStatus&STATUS_SERVAVAIL))
			{	// wait 30 sec for connect to server
				if (timeGetTime() - TimeLogin > 30*1000)
					return -1;
				else
					continue;
			}

			if (!case7) {
				ContP->LoginUser(g_All.me.Us, g_All.me.Ps, false);
				case7 = true;
			}

			if (!(ClientStatus&STATUS_LOGGEDIN)) {
				if (timeGetTime() - TimeLogin > 60*1000)
					return -2;
				else
					continue;
			} else {
				ContP->LogoutUser(true);
				return 0;
			}

			continue;
		}

		if (!(ClientStatus&STATUS_SERVAVAIL))
			continue;
		DWORD CurrTime = timeGetTime();

		if (dwMode==2) {
			if (!(ClientStatus&STATUS_LOGGEDIN)) {
				TimeLogout = 0;
				if (CurrTime - TimeLogin > 10000) {
					ContP->LoginUser(g_All.me.Us, g_All.me.Ps, false);
					TimeLogin = CurrTime;
				}
			}
			else {
				TimeLogin = 0;
				if (CurrTime - TimeLogout > 10000) {
					times_number++;
					ContP->LogoutUser(false);
					TimeLogout = CurrTime;
				}
				if (CurrTime - avTime > 10000) {
					float f = (float)times_number/(CurrTime - avTime)*1000;
					printf("LOGGED %6.2f times per sec\n", f);
					times_number/=2;
					avTime+=5000;
				}
			}
		}
		else {
			if (!(ClientStatus&STATUS_LOGGEDIN)) {
				if (CurrTime - TimeLogin > 5000) {
					printf("Login ++++++ \n");
					ContP->LoginUser(g_All.me.Us, g_All.me.Ps, true);
					TimeLogin = CurrTime;
				}
				ab_req = false;
			}
			else {
				char name[256] = {0};
				char from[256];
				char mess[256];
				switch (dwMode)
				{
				case 1:
				case 3:
					/// get ab and subscribe
					if (!ab_req) {
						ContP->SearchAddressBook("", AB_COMMON, 0);
						ab_req = true;
					}
					if ((msg.wParam&0xffff)==VSTRCL_UPCONT_OK) {
						VS_Container *cnt = ContP->m_PresenceContainers.GetList(msg.lParam);

						if(cnt)
						{
							VS_SimpleStr method = cnt->GetStrValueRef(METHOD_PARAM);
							if (method==SEARCHADDRESSBOOK_METHOD) {
								long ab = 0;
								cnt->GetValueI32(ADDRESSBOOK_PARAM, ab);
								if (ab==AB_COMMON) {
									long result = 0;
									long cause = 0;
									cnt->GetValueI32(RESULT_PARAM, result);
									cnt->GetValueI32(CAUSE_PARAM, cause);
									printf("AB: result = %ld, cause = %ld\n", result, cause);
									cnt->Reset();
									char* names[1000];
									int n_num = 0;
									while (cnt->Next() && n_num <1000) {
										if (_stricmp(cnt->GetName(), USERNAME_PARAM)==0) {
											names[n_num] = (char*)cnt->GetStrValueRef();
											n_num++;
										}
									}
									ContP->Subscribe(names, n_num, true);
								}
							}
						}
					}
					ConfCreate(ContP, dwMode);
					break;
				case 4:
					ConfCreate4(ContP, false);
					break;
				case 5:
					ConfCreate4(ContP, true);
					break;
				case 6:
					if (ClientStatus&STATUS_MESSAGE) {
						ContP->GetMessageA(msg.lParam, from, mess);
						times_number++;
					}
					ContP->GetMyName(name);
					sprintf(mess, "Test text for _TestClientProtocol mess #%6d", times_number);
					ContP->ChatSend(mess, name);
					if (CurrTime - avTime > 10000) {
						float f = (float)times_number/(CurrTime - avTime)*1000;
						printf("CHAT %6.2f times per sec\n", f);
						times_number/=2;
						avTime+=5000;
					}
					break;
				case 8:
					SendTorrentFiles();
				}
			}
		}
	}

	if (case7)
		return -3;
	SetEvent(hDes);
	if (hConfTread[0]) {
		WaitForSingleObject(hConfTread[0], 1000);
		CloseHandle(hConfTread[0]); hConfTread[0] = NULL;
	}
	if (hConfTread[1]) {
		WaitForSingleObject(hConfTread[1], 1000);
		CloseHandle(hConfTread[1]); hConfTread[1] = NULL;
	}
	if (ContP) delete ContP;
	delete 	g_pDtrase;
	CloseHandle(hDes);
	CloseHandle(hList);
	return SucsTest1;
}

