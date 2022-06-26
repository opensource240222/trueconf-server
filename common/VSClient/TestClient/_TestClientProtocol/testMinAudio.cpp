

#include "../../../transport/client/VS_TransportClient.h"
#include "../../../streams/Protocol.h"
#include "../../../streams/Client/VS_StreamClient.h"
#include "../../../streams\Client\VS_StreamClientSender.h"
#include "../../../streams\Client\VS_StreamClientReceiver.h"
#include "..\..\..\std\cpplib\VS_SimpleStr.h"
#include "std-generic/cpplib/VS_Container.h"
#include "..\..\..\std\cpplib\VS_RegistryKey.h"

#include "..\..\..\transcoder\vs_bitstreambuff.h"
#include "testclientprotocol.h"

//#include "../../VSTrClientProc.h"
#include "../../../servers\h323gateway\h323slotclient\vs_h323slotvisiclient.h"
#include "audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <process.h>



/**************************************************************************************************/

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
HANDLE g_hOut = 0;
/**************************************************************************************************/
unsigned int WINAPI RcvThread(void* par)
{
	VS_H323VisicronClient *ContP = reinterpret_cast<CVSTrClientProc *>(par);
	VS_StreamClientReceiver   *rcv = ContP->GetConferenseReceiver();
	HANDLE hAudioDone = CreateEvent(0, 0, 0, 0);
	ARender rend;
	rend.Init(0, hAudioDone);

	unsigned long mils = 100;
	char *buffer = new char[65536];
	HANDLE Events[3];
	Events[0] = g_hOut;
	Events[1] = rcv->GetReceiveEvent();
	Events[2] = hAudioDone;

	int StatTime = timeGetTime();
	int CurrTime, retu;
	int StatBytes = 0;

	DWORD retWait = WAIT_OBJECT_0+1;
	while (1) {
		CurrTime = timeGetTime();
		if (retWait==WAIT_OBJECT_0) {
			break;
		}
		else if (retWait==WAIT_OBJECT_0+1) {
			stream::Track track = {};
			mils =20;
			retu = rcv->ReceiveFrame(buffer, 65536, &track, &mils);
			if (retu>0) StatBytes+=retu;
			if (retu==-1) {
				puts("MAIN RECEIVER DISCONECT\n");
				break;
			}
			else if (retu==-2) {
			}
			else {
				if (track == stream::Track::audio && retu > 0)
					rend.Play(buffer, retu);
			}
		}
		else if (retWait==WAIT_OBJECT_0+2) {
			rend.GenNoise();
		}
		else if (retWait==WAIT_TIMEOUT) {
		}
		else {
			break;
		}
		retWait = WaitForMultipleObjects(3, Events, FALSE, 100);
		if (CurrTime - StatTime > 2000) {
			printf("RCV Stat = %d kbit \n", StatBytes/128/2);
			StatBytes = 0;
			StatTime = CurrTime;
		}
	}

	delete[] buffer;
	rend.Release();
	rcv->CloseConnection();
	delete rcv;
	CloseHandle(hAudioDone);
	SetEvent(g_hOut);
	return 0;
}

unsigned int WINAPI SndThread(void* par)
{
	VS_H323VisicronClient *ContP = reinterpret_cast<CVSTrClientProc *>(par);
	VS_StreamClientSender *snd = ContP->GetConferenseSender();
	ACapture capt;
	HANDLE hAudioCome = CreateEvent(0, 0, 1, 0);
	capt.Init(0, hAudioCome);

	FILE* fin = fopen("send.txt", "w");

	unsigned long mils = 100;
	char *buffer = new char[65536];
	HANDLE Events[2];
	Events[0] = hAudioCome;
	Events[1] = snd->GetSendEvent();

	int CurrTime, buffSize, retu;
	VS_AudioBuff video;

	int StartTime = timeGetTime();
	int StatTime = StartTime;
	int StatBytes = 0;

	snd->SendFrame(buffer, 0, 2, &mils);
	DWORD retWait = WAIT_OBJECT_0+1;
	while (1) {
		CurrTime = timeGetTime();
		if (retWait==WAIT_OBJECT_0+0 || retWait==WAIT_OBJECT_0+1 ) {
			if (capt.Capture(buffer, buffSize)) {
				retu = snd->SendFrame(buffer, buffSize, 1);
				if (retu==-2)
					printf("\n Main snd timeout: ");
				else if (retu==-1){
					puts("MAIN SENDER DISCONECT\n");
					break;
				}
				int count = retu/65*640/BUFFER_LEN;
				for (int i = 0; i<count; i++)
					fprintf(fin, "%3d\n", CurrTime);
			}
			else {
				if (video.Bytes()!=0) {
					if (video.Bytes()>1200)
						buffSize = 1200;
					else
						buffSize = video.Bytes();
					video.TruncLeft(buffSize);
					retu = snd->SendFrame(buffer, buffSize, 2);
					if (retu==-2)
						printf("\n Main snd timeout: ");
					else if (retu==-1){
						puts("MAIN SENDER DISCONECT\n");
						break;
					}
				}
			}
		}
		else if (retWait==WAIT_TIMEOUT) {
		}
		else {
			break;
		}
		if (CurrTime - StartTime > 100 && video.Bytes()==0) {
			buffSize = rand()%4000;
			video.Add((BYTE*)buffer, buffSize);
			StartTime = CurrTime;
		}
		if (video.Bytes())
			SetEvent(hAudioCome);

		if (WaitForSingleObject(g_hOut, 0) == WAIT_OBJECT_0)
			break;
		retWait = WaitForMultipleObjects(2, Events, TRUE, 50);
	}

	fclose(fin);
	delete[] buffer;
	capt.Release();
	snd->CloseConnection();
	delete snd;
	CloseHandle(hAudioCome);
	SetEvent(g_hOut);
	return 0;
}


/**************************************************************************************************/

int main(int argc, char* argv[])
{
	VS_SetAppVer("testMinAudio", "1.0.1");
	DWORD dwInstallUser = 0;
	char temp[256];
	char server[256];
	int supprDir = 0;

	if (argc<3) {
		goto error;
	}
	else {
		strcpy(server, argv[1]);
		dwInstallUser = atoi(argv[2]);
		sprintf(temp, "Visicron\\Test\\%s%d", GUI_version, dwInstallUser);
		VS_RegistryKey::SetRoot(temp);
		VS_RegistryKey key(1, REG_CurrentConfiguratuon);
		key.GetValue(&supprDir, 4, VS_REG_INTEGER_VT, "Suppress Direct");
	}

	auto ContP = std::make_shared<CVSTrClientProc>(0);

	ContP->Init(GetCurrentThreadId());
	ContP->m_dwSuppressDirectCon = supprDir;
	ContP->m_DirectPort = 5000 + (unsigned short)dwInstallUser;
	ContP->ConnectServer(server);

	Sleep(rand()%500);
	char Un[256], Ps[256];
	sprintf(Un, "crash_un%ld", dwInstallUser);
	sprintf(Ps, "crash_pass");
	g_All.me.Us = Un;
	g_All.me.Ps = Ps;


	DWORD ClientStatus = 0;
	ConfState confstate = ST_NORMAL;
	int streamsCreated = 0;
	int CurrTime = timeGetTime();
	int TimeLogin = CurrTime, TimeConf = CurrTime, TimeCall = CurrTime, TimeEndConf = CurrTime;
	MSG msg;

	g_hOut = CreateEvent(0, 1, 0, 0);

	HANDLE handles[3] = {g_hOut, 0, 0};

	while (true) {
		DWORD obj = MsgWaitForMultipleObjects(1, handles, 0, 1000, QS_POSTMESSAGE);
		CurrTime = timeGetTime();
		if (obj== WAIT_OBJECT_0+1 || obj== WAIT_TIMEOUT) {
			if (obj== WAIT_TIMEOUT) {
			}
			// visicron protocol messages
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_USER+16) {
					ClientStatus = msg.wParam;
					if (ClientStatus&0x80)
						if (ClientStatus&0xf) {// reject
							confstate = ST_CONFEND; TimeEndConf = CurrTime;
							ContP->Hangup();
						}
				}
				else if (msg.message == WM_USER+17) {
					if (msg.wParam==6)
						streamsCreated = 1;
					else // if 5
						streamsCreated = -1;
				}
			}

			if (!(ClientStatus&STATUS_SERVAVAIL))
				continue;

			if (!(ClientStatus&STATUS_LOGGEDIN)) {
				if (CurrTime - TimeLogin > 5000) {
					printf("~~~~~~ LOGIN~~~~~~~~~\n");
					ContP->LoginUser(g_All.me.Us, g_All.me.Ps, false);
					TimeLogin = CurrTime;
				}
				continue;
			}


			switch(confstate)
			{
			case ST_NORMAL:
				if (CurrTime - TimeEndConf < 10000)
					continue;
				if (dwInstallUser==1) {
					if (ClientStatus&STATUS_INCALL) {
						ContP->Accept();
						confstate = ST_CALL; TimeCall = CurrTime;
					}
				}
				else {
					if (!ContP->PlaseCall("crash_em1")) {
						confstate = ST_CALL; TimeCall = CurrTime;
					}
					else {
						confstate = ST_CONFEND; TimeEndConf = CurrTime;
					}
				}
				break;
			case ST_CALL:
				if (ClientStatus&STATUS_CONFERENCE) {
					if (streamsCreated==1) {
						ResetEvent(g_hOut);
						handles[1] = (HANDLE)_beginthreadex( 0, 0, RcvThread, (void *)ContP, 0, 0 );
						handles[2] = (HANDLE)_beginthreadex( 0, 0, SndThread, (void *)ContP, 0, 0 );
						confstate = ST_CONF; TimeConf = CurrTime;
					}
					else if (streamsCreated==-1) {
						ContP->Hangup();
						confstate = ST_CONFEND; TimeEndConf = CurrTime;
					}
				}
				else {
					if ( (CurrTime - TimeCall)> 15000) { // wait responce from transport
						ContP->Hangup();
						confstate = ST_CONFEND; TimeEndConf = CurrTime;
					}
				}
				break;
			case ST_CONF:
				if (ClientStatus&STATUS_CONFERENCE){
					if ((CurrTime - TimeConf)>120000){ // conf live time
						ContP->Hangup();
						confstate = ST_CONFEND; TimeEndConf = CurrTime;
					}
				}
				else {
					confstate = ST_CONFEND; TimeEndConf = CurrTime;
				}
				break;
			case ST_CONFEND:
				if (ClientStatus&STATUS_CONFERENCE){
					continue;
				}
				else {
					printf("~~~~~~ STOP ~~~~~~~~~\n");
					SetEvent(g_hOut);
				}
				break;
			}
		}
		else if (obj==WAIT_OBJECT_0) {
			int num = (handles[0]!=0) + (handles[1]!=0) + (handles[2]!=0);
			WaitForMultipleObjects(num, handles, true, 10000);
			if (handles[1]) CloseHandle(handles[1]); handles[1] = 0;
			if (handles[2]) CloseHandle(handles[2]); handles[2] = 0;
			confstate = ST_NORMAL;
			streamsCreated = 0;
//			TimeConf = 0, TimeCall = 0, TimeEndConf = 0;
			ResetEvent(g_hOut);
		}
		else {
			break;
		}
	}
error:
	return 0;
}

