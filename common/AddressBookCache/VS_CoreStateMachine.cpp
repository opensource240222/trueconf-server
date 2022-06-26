//#include "VZOchat7.h"
#include "VS_CoreStateMachine.h"
#include "VS_AddressBookManager.h"
#include "VS_LoginManager.h"
#include "VS_ChatManager.h"
#include "VS_ServerStateManager.h"
#include "VS_ConferenceManager.h"

#include <windows.h>

#ifndef VZO_DEBUG_CONTAINER
// #define VZO_DEBUG_CONTAINER
#endif
//TODO: FIX THIS!
#define WM_NOTIFY_GUI				WM_USER+16
#define WM_NOTIFY_GUI_CPY			WM_USER+18
#define WM_USERLOGIN				WM_APP+2	//< User Login Service Message
#define WM_ENDPOINTREG				WM_APP+3	//< Endpoint Registration Service Message
#define WM_CONFERENCE				WM_APP+4	//< Conference Service Message
#define WM_USERPESENCE				WM_APP+5	///< User Presence Service Message
#define WM_NETWORKCONF				WM_APP+6	///< Network Configuration Service Message
#define WM_PING						WM_APP+7	///< Ping Service Message
#define WM_CHAT						WM_APP+8	///< Chat Service Message
#define WM_CLIENT					WM_APP+9	///< Client Service Message
#include "../ClientInterface/ClientInterface.h"
#include "../ClientInterface/VSClient.h"

volatile LONG hhelper(0);

VS_CoreStateMachine::VS_CoreStateMachine(VS_AddressBookManager *abm, VS_LoginManager *lgm, VS_ChatManager *chm, VS_ServerStateManager *ssm, VS_ConferenceManager *conf)
	: VS_CoreFlag(), CVSThread(), m_ContP(0), m_abm(abm), m_login(lgm), m_chat(chm), m_ssm(ssm), m_conf(conf), m_helper(0)
{
	superFlags = new DWORD_PTR[64];
	memset(superFlags, 0, sizeof(DWORD_PTR)*64);
	superReg1 = _strdup("TrueConf\\Terminal");
	//superReg2 = _strdup("trueconf.ru");
	superReg2 = NULL;
	DWORD threadId(0);
	m_threadSync = CreateEvent(0, true, false, 0); // lock cvsthread
	m_helperSync = CreateEvent(0, true, false, 0); // lock early helper
	ActivateThread(0, &threadId);
	confHandle = Initialize((DWORD_PTR)superFlags, threadId, superReg1, superReg2);
	SetEvent(m_threadSync); // release cvsthread
}

VS_CoreStateMachine::~VS_CoreStateMachine()
{
	Shutdown();
}

void VS_CoreStateMachine::OnRaise(core::pin pin, unsigned long option, unsigned long ext)
{
#ifdef _CSM_DEBUG_
	printf("\nflags - pin.raise %d", pin);
#endif
	switch (pin)
	{
	case core::loggedin:
		{
#ifdef _CSM_DEBUG_
			printf("loggedin\n");
#endif
			char myname[256] = {0};
			m_ContP->GetMyName(myname);
			m_abm->Init(myname, m_ContP->m_CurrBroker);
			m_login->SetHomeServer(m_ContP->m_CurrBroker);
//			m_login->ParseIncomimgMessage(m_ContP->m_LoginContainer);
		}
		break;

	case core::servavail:
		{
#ifdef _CSM_DEBUG_
			printf("servavail\n");
#endif
			VS_Container cnt;
			cnt.AddValue("server", m_ContP->m_CurrBroker.m_str);
			m_ssm->ParseIncomimgMessage(cnt);
		}
		break;

	case core::incall:
		{
#ifdef _CSM_DEBUG_
			printf("incall\n");
#endif
		}
		break;

	case core::conference:
		{
#ifdef _CSM_DEBUG_
			printf("conference\n");
#endif
			char name[256], fname[256];
			::GetOtherName(confHandle, (DWORD_PTR)name, (DWORD_PTR)fname, (DWORD_PTR)fname);
		}
		break;

	case core::userinfo:
		{
#ifdef _CSM_DEBUG_
			printf("userinfo\n");
#endif
		}
		break;

	case core::message:
		{
#ifdef _CSM_DEBUG_
			printf("message\n");
#endif
		}
		break;

	case core::command:
		{
#ifdef _CSM_DEBUG_
			printf("command\n");
#endif
		}
		break;

	case core::reqinvite:
		{
#ifdef _CSM_DEBUG_
			printf("reqinvite\n");
#endif
		}
		break;

	default:{}
	}
}

void VS_CoreStateMachine::OnFall(core::pin pin, unsigned long option, unsigned long ext)
{
#ifdef _CSM_DEBUG_
	printf("\nflags - pin.fall %d", pin);
#endif
	switch (pin)
	{
	case core::loggedin:
		{
//			m_login->ParseIncomimgMessage(m_ContP->m_LoginContainer);
		}
		break;

	case core::servavail:
		{
#ifdef _CSM_DEBUG_
			printf("servavail\n");
#endif
			VS_Container cnt;
			cnt.AddValue("server", "");
			m_ssm->ParseIncomimgMessage(cnt);
		}
		break;

	case core::incall:
		{
#ifdef _CSM_DEBUG_
			printf("incall\n");
#endif
		}
		break;

	case core::conference:
		{
#ifdef _CSM_DEBUG_
			printf("conference\n");
#endif
		}
		break;

	case core::userinfo:
		{
#ifdef _CSM_DEBUG_
			printf("userinfo\n");
#endif
		}
		break;

	case core::message:
		{
#ifdef _CSM_DEBUG_
			printf("message\n");
#endif
			/*
			VS_Container *cnt = static_cast<VS_Container*>(m_ContP->m_MessContainers.GetList(ext));
			if (cnt) m_chat->ParseIncomimgMessage(*cnt);
			*/
		}
		break;

	case core::command:
		{
#ifdef _CSM_DEBUG_
			printf("command\n");
#endif
		}
		break;

	case core::reqinvite:
		{
#ifdef _CSM_DEBUG_
			printf("reqinvite\n");
#endif
		}
		break;

	default:{}
	}
}

void VS_CoreStateMachine::OnNotify(unsigned long option, unsigned long ext)
{
#ifdef _CSM_DEBUG_
	printf("Warning : VS_CoreStateMachine::OnNotify is empty\n");
#endif
}

void VS_CoreStateMachine::Shutdown()
{
	m_abm = 0;
	m_login = 0;
	m_chat = 0;
	m_ssm = 0;
	m_conf = 0;
}

DWORD VS_CoreStateMachine::Loop(LPVOID hEvDie)
{
	WaitForSingleObject(m_threadSync, INFINITE); // wait vsclient initialize
	CloseHandle(m_threadSync);

	DWORD ClientStatus = 0;
	//VS_SetAppVer("TrueConf Terminal", "1.2.5");
	VS_RegistryKey::SetRoot("TrueConf\\Terminal");
	MSG msg;
	srand(static_cast<unsigned int>(time(0)));
	g_pDtrase = new VS_DebugOut;
	DWORD TimeLogin = timeGetTime();
	VSClient* m_pVsc = reinterpret_cast<VSClient*>(confHandle);

	//TODO: fix race another way
	InterlockedExchange(&hhelper, 0);
	m_helper = new VS_OleProtoHelper(reinterpret_cast<VSClient*>(confHandle));
	SetEvent(m_helperSync); // now we can play))
	InterlockedExchange(&hhelper, 1);

	m_ContP = m_pVsc->GetProtocol();

	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	DWORD hq_auto = 1;
	DWORD snd_stereo=0;
	key.GetValue(&snd_stereo, 4, VS_REG_INTEGER_VT, "SndStereo");
	if(snd_stereo)
	  hq_auto=0;
	else
	  key.GetValue(&hq_auto, 4, VS_REG_INTEGER_VT, "HQ Auto");

	int h = 240;
	int w = 320;
	if (hq_auto == 0)
	{
		DWORD format = 0;
		if (key.GetValue(&format, 4, VS_REG_INTEGER_VT, "Format") > 0)
		{
			h = (BYTE)(format>>8)*8;
			w = (BYTE)(format)*8;
		}
	}

	m_pVsc->InitSender(w/*320*/, h/*240*/, 11025);

	VS_SimpleStr srv;
	srv.Resize(100);
	VS_ReadAS(srv);
	m_ContP->ConnectServer(srv);
	HANDLE handles[1] = { hEvDie };
	DWORD CurrTime(0);


	WORD low = 0;
	WORD high = 0;

	while (true) {
		DWORD obj = MsgWaitForMultipleObjects(1, handles, 0, 100, /*QS_POSTMESSAGE*/QS_ALLINPUT);
		if (obj==WAIT_OBJECT_0) break;

		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			switch (msg.message)
			{
			case WM_USER+23:
				{
					low = (WORD)(msg.wParam & 0x0000FFFF);
					high = (WORD)(msg.wParam >> 16);
				}
				break;
			case WM_NOTIFY_GUI:
				{
					DWORD ret = msg.wParam & 0xffff;
					ClientStatus = msg.wParam;
					if (VSTRCL_UPCONT_OK == ret){
						VS_Container *cnt = static_cast<VS_Container*>(m_ContP->m_PresenceContainers.GetList(msg.lParam));
						if (cnt) {
#ifdef VZO_DEBUG_CONTAINER
							SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
							cnt->PrintF();
							SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);
#endif
							m_abm->ParseIncomimgMessage(*cnt);
						}
					}
					DWORD rr = VSTRCL_SSL_ERR & ret;
					//if ((VSTRCL_SSL_ERR & ret) == VSTRCL_SSL_ERR)
					if ((ret & 0xFFF0) == VSTRCL_SSL_ERR)
					{
						m_ssm->DisconnectWithError(ret);
						//printf("VSTRCL_SSL_ERR: %H", ret);
					}
					SetFlag(ClientStatus, ret, msg.lParam);
					OnNotify(msg.wParam /*& 0x0FFF*/, msg.lParam);
				}
				break;

			case WM_NOTIFY_GUI_CPY:
				{
					switch (msg.wParam)
					{
					case WM_USERLOGIN:
						{
							VS_Container *cnt = static_cast<VS_Container*>(m_ContP->m_ForkContainers.GetList(msg.lParam));
							if (cnt) {
#ifdef VZO_DEBUG_CONTAINER
								SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
								cnt->PrintF();
								SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);
#endif
								if (m_login) m_login->ParseIncomimgMessage(*cnt);
							}
						}
						break;

					case WM_USERPESENCE:
					case WM_CONFERENCE:
						{
							VS_Container *cnt = static_cast<VS_Container*>(m_ContP->m_ForkContainers.GetList(msg.lParam));
							if (cnt) {
#ifdef VZO_DEBUG_CONTAINER
								SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
								cnt->PrintF();
								SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);
#endif
								if (m_conf) m_conf->ParseIncomimgMessage(*cnt);
							}
						}
						break;

					case WM_CHAT:
						{
							VS_Container *cnt = static_cast<VS_Container*>(m_ContP->m_ForkContainers.GetList(msg.lParam));
							if (cnt) {
#ifdef VZO_DEBUG_CONTAINER
								SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
								cnt->PrintF();
								SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);
#endif
								if (m_chat) m_chat->ParseIncomimgMessage(*cnt);
							}
						}
						break;

					case WM_NETWORKCONF:
					case WM_CLIENT:
					default:
						{
						}
					}
				}
				break;

			default:
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					/*
					printf("unknown message %d\n", msg.message);
					*/
				}
			}
		}
	}
	return 0;
}

VS_AddressBookManager& VS_CoreStateMachine::AddressBookManager()
{
	return *m_abm;
}

VS_LoginManager& VS_CoreStateMachine::LoginManager()
{
	return *m_login;
}

VS_ChatManager& VS_CoreStateMachine::ChatManager()
{
	return *m_chat;
}

VS_ServerStateManager& VS_CoreStateMachine::ServerStateManager()
{
	return *m_ssm;
}

VS_ConferenceManager& VS_CoreStateMachine::ConferenceManager()
{
	return *m_conf;
}

VS_OleProtoHelper& VS_CoreStateMachine::ProtocolHelper()
{
	//TODO: fix race anoter way
	if (1 == InterlockedExchange(&hhelper, hhelper)) return *m_helper;

	MSG msg;
	while (true) {
		switch (MsgWaitForMultipleObjects(1, &m_helperSync, FALSE, INFINITE, QS_ALLINPUT)) {
		case WAIT_OBJECT_0:
			return *m_helper;
		case WAIT_OBJECT_0+1:
		  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			//TODO: WM_TIMER flow can here... why?
			if (1 == InterlockedExchange(&hhelper, hhelper)) return *m_helper;
		  }
		}
	}
	return *m_helper;
}
