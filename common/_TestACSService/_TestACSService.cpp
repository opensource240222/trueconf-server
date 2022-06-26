// _TestACSService.cpp : Defines the entry point for the console application.
//

#include "..\transport\message\VS_TransportMessage.h"
#include <tchar.h>
#include <conio.h>
#include "_testacsconst.h"
#include "_testacstypes.h"
#include "VS_TestParticipant.h"
#include "VS_Scope.h"
#include "vs_flooder.h"
#include <map>
#include <fstream>
#include "..\UPnPLib\VS_UPnPInterface.h"
//#include "..\acs\lib\AcsLib.h"

std::map<std::string, std::string> __appSwitches;
std::list<void *> __iocpKeys;
std::map<std::string, void *> __acsPlugins;
CRITICAL_SECTION __reboot;

bool DetectQosIsWorking();

union ip_addr {
	unsigned char _part[4];
	unsigned long _addr;
};

struct cidr {
	union ip_addr addr;
	unsigned char mask;
};

int _tMainSetupWorkingMode() {
	std::map<std::string, std::string>::iterator __appSwitchesIterator;
	__appSwitchesIterator = __appSwitches.find("-a");
	if (__appSwitchesIterator == __appSwitches.end()) {
		printf("\nError : target address not specified");
		return -1;
	} else {
		if (strchr((*__appSwitchesIterator).second.c_str(), ':')) {
			AppMode.address = (*__appSwitchesIterator).second;
		} else {
			printf("\nError : target address specified but invalid");
			return -1;
		}
	}
	__appSwitchesIterator = __appSwitches.find("-e");
	if (__appSwitchesIterator == __appSwitches.end()) {
		printf("\nError : target endpoint not specified");
		return -1;
	} else {
		if (strchr((*__appSwitchesIterator).second.c_str(), '#')) {
			AppMode.endpoint = (*__appSwitchesIterator).second;
		} else {
			printf("\nError : target endpoint specified but invalid");
			return -1;
		}
	}
	__appSwitchesIterator = __appSwitches.find("-u");
	if (__appSwitchesIterator == __appSwitches.end()) {
		printf("\nWarning : users count not specified. using %d", AppMode.users);
	} else {
		AppMode.users = atoi((*__appSwitchesIterator).second.c_str());
	}
	__appSwitchesIterator = __appSwitches.find("-t");
	if (__appSwitchesIterator == __appSwitches.end()) {
		printf("\nWarning : test service type not specified. using %s", _TestServiceNames[AppMode.type_service + 1]);
	} else {
		AppMode.type_service = atoi((*__appSwitchesIterator).second.c_str());
	}
	__appSwitchesIterator = __appSwitches.find("-g");
	if (__appSwitchesIterator == __appSwitches.end()) {
		printf("\nInformation : TCP garbage collection disabled");
	} else {
		printf("\nInformation : TCP garbage collection enabled");
		AppMode.trash = true;
	}
	__appSwitchesIterator = __appSwitches.find("-s");
	if (__appSwitchesIterator == __appSwitches.end()) {
		printf("\nWarning : sleep time not specified. using %d", AppMode.sleep_time);
	} else {
		AppMode.sleep_time = atoi((*__appSwitchesIterator).second.c_str());
	}
	__appSwitchesIterator = __appSwitches.find("-p");
	if (__appSwitchesIterator == __appSwitches.end()) {
		printf("\nWarning : pool thread size not specified. using %d", AppMode.nThreads);
	} else {
		AppMode.nThreads = atoi((*__appSwitchesIterator).second.c_str());
	}
	__appSwitchesIterator = __appSwitches.find("-c");
	if (__appSwitchesIterator == __appSwitches.end()) {
		printf("\nInformation : ssl disabled");
	} else {
		printf("\nInformation : ssl enabled");
		AppMode.ssl = true;
	}
	__appSwitchesIterator = __appSwitches.find("-r");
	if (__appSwitchesIterator == __appSwitches.end()) {
		printf("\nWarning : reconnects count not specified. using %d", AppMode.reconnect);
	} else {
		AppMode.reconnect = atoi((*__appSwitchesIterator).second.c_str());
	}
	__appSwitchesIterator = __appSwitches.find("-n");
	if (__appSwitchesIterator == __appSwitches.end()) {
		printf("\nWarning : connection wait time not specified. using %d", AppMode.sleep_connect);
	} else {
		AppMode.sleep_connect = atoi((*__appSwitchesIterator).second.c_str());
	}
	__appSwitchesIterator = __appSwitches.find("-f");
	if (__appSwitchesIterator == __appSwitches.end()) {
		printf("\nInformation : flood disabled");
	} else {
		if (AppMode._TranslateFlood((*__appSwitchesIterator).second.c_str()) != -1) {
			printf("\nInformation : flood enabled");
		} else {
			printf("\nError : flood mode detected but invalid");
			return -1;
		}
	}
	__appSwitchesIterator = __appSwitches.find("-k");
	if (__appSwitchesIterator == __appSwitches.end()) {
		printf("\nInformation : forced kill disabled");
	} else {
		if (AppMode._TranslateKill((*__appSwitchesIterator).second.c_str()) != -1) {
			printf("\nInformation : forced kill enabled");
		} else {
			printf("\nError : forced kill mode detected but invalid");
			return -1;
		}
	}
	__appSwitchesIterator = __appSwitches.find("-w");
	if (__appSwitchesIterator == __appSwitches.end()) {
		printf("\nInformation : sync wait (\"p - button\") disabled");
	} else {
		printf("\nInformation : sync wait (\"p - button\") enabled");
		AppMode.syncwait_ = true;
	}
	__appSwitchesIterator = __appSwitches.find("-1");
	if (__appSwitchesIterator == __appSwitches.end()) {
		printf("\nInformation : using old good 2-socket mode transport");
	} else {
		printf("\nInformation : using modern 1-socket transport");
		AppMode.onesocket = true;
	}
	__appSwitchesIterator = __appSwitches.find("-b");
	if (__appSwitchesIterator == __appSwitches.end()) {
		printf("\nInformation : checkboot disabled.");
	} else {
		AppMode.reboot_time = atoi((*__appSwitchesIterator).second.c_str());
		printf("\nInformation : checkboot wait time = %d min.", AppMode.reboot_time);
	}
	return 0;
}

VS_Scope **m_scope = NULL;
VS_Flooder **m_flooder = NULL;
VS_CoolTimer timer;

bool checkaddrincidr(cidr c, ip_addr a) {
	unsigned long _bitmask = 0;
	for (int i = 0; i < c.mask; ++i) { _bitmask |= (1 << i); }
	unsigned long min_vl = c.addr._addr & _bitmask;
	unsigned long max_vl = c.addr._addr | ~_bitmask;
	min_vl = ntohl(min_vl);
	max_vl = ntohl(max_vl);
	a._addr= ntohl(a._addr);
	return ((a._addr > min_vl) && (a._addr < max_vl));;
}

class CrackMsg : public VS_TransportMessage
{
public:
	bool _Set(const unsigned char *data)
	{
		return Set(data);
	}

	bool _CheckMessageIntegrity()
	{
		return CheckMessageIntegrity();
	}
};

void TryToDecode(const unsigned char *data, const size_t len)
{
	CrackMsg msg;
	msg._Set(data);
	msg._CheckMessageIntegrity();
	msg.DstServer();

	for (int i = 0; i < len; ++i) {
		VS_Container cnt;
		if (cnt.Deserialize(data + i, len - i) && cnt.IsValid()) {
			cnt.PrintF();
			printf("Deserialized. i = %d\n", i);
		}
	}
	printf("Press any key to continue...\n");
	_getch();
}

int _tmain(int argc, _TCHAR* argv[])
{

  //////----------------------
  ////// Initialize Winsock.
  ////WSADATA wsaData;
  ////int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  ////if (iResult != NO_ERROR) {
  ////  printf("Error at WSAStartup()\n");
  ////  return 1;
  ////}

  //////----------------------
  ////// Create a SOCKET for listening for
  ////// incoming connection requests.
  ////SOCKET ListenSocket;
  ////ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  ////if (ListenSocket == INVALID_SOCKET) {
  ////  printf("Error at socket(): %ld\n", WSAGetLastError());
  ////  WSACleanup();
  ////  return 1;
  ////}

  //////----------------------
  ////// The sockaddr_in structure specifies the address family,
  ////// IP address, and port for the socket that is being bound.
  ////sockaddr_in service;
  ////service.sin_family = AF_INET;
  ////service.sin_addr.s_addr = inet_addr("192.168.62.159");
  ////service.sin_port = htons(5050);

  ////if (bind( ListenSocket, 
  ////  (SOCKADDR*) &service, 
  ////  sizeof(service)) == SOCKET_ERROR) {
  ////  printf("bind() failed.\n");
  ////  closesocket(ListenSocket);
  ////  WSACleanup();
  ////  return 1;
  ////}

  //////----------------------
  ////// Listen for incoming connection requests.
  ////// on the created socket
  ////if (listen( ListenSocket, 1 ) == SOCKET_ERROR) {
  ////  printf("Error listening on socket.\n");
  ////  closesocket(ListenSocket);
  ////  WSACleanup();
  ////  return 1;
  ////}

  //////----------------------
  ////// Create a SOCKET for accepting incoming requests.
  ////SOCKET AcceptSocket;
  ////printf("Waiting for client to connect...\n");

  //////----------------------
  ////// Accept the connection.
  ////AcceptSocket = accept( ListenSocket, NULL, NULL );
  ////if (AcceptSocket == INVALID_SOCKET) {
  ////  printf("accept failed: %d\n", WSAGetLastError());
  ////  closesocket(ListenSocket);
  ////  WSACleanup();
  ////  return 1;
  ////} else 
  ////  printf("Client connected.\n");
  ////
  ////sockaddr srv = {};
  ////int len = sizeof(srv);
  ////getsockname(AcceptSocket, &srv, &len);
  ////unsigned int port = ntohs(((struct sockaddr_in *)&srv)->sin_port);
  ////// No longer need server socket
  ////closesocket(ListenSocket);

  ////WSACleanup();
  ////return 0;



/*
	// broken message decoder
	const unsigned char vs_msg_crack[] = {
		0xc1, 0x89, 0x39, 0x00, 0x09, 0x54, 0x00, 0x00, 0x0b, 0x05, 0x00, 0x00, 0x20, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x43, 0x4f, 0x4e, 0x46, 0x45, 0x52, 0x45, 0x4e, 0x43, 0x45, 0x00, 0x02, 0x33, 0x34, 0x00,
		0x00, 0x00, 0x0a, 0x43, 0x4f, 0x4e, 0x46, 0x45, 0x52, 0x45, 0x4e, 0x43, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0d, 0x00, 0x57, 0x00, 0x00, 0x00, 0x07, 0x71, 0x46, 0xd4, 0xb2, 0xb3, 0x2d,
		0x5e, 0xbf, 0xd0, 0x74, 0xc1, 0x42, 0x86, 0xc5, 0xa2, 0x1d, 0x61, 0x91, 0x5a, 0x39, 0xb5, 0xb8, 0x50, 0x15, 0xc8, 0x99, 0xf1, 0x40, 0x22, 0xea, 0x79, 0x4e, 0x82, 0x34, 0x34, 0x8a, 0xe6, 0xb2, 0xc4, 0x6a,
		0xc5, 0x9d, 0xe4, 0xd6, 0xf6, 0x89, 0x27, 0xfa, 0xb9, 0x78, 0xf6, 0x63, 0xff, 0xfb, 0xfd, 0x21, 0x0e, 0xae, 0xed, 0xc5, 0x8f, 0x01, 0x61, 0x48, 0xa3, 0xc3, 0x37, 0x38, 0xef, 0x27, 0xac, 0x42, 0x74, 0x98,
		0xaa, 0x9e, 0xb0, 0x9f, 0x2d, 0x00, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	const size_t vs_msg_len = sizeof(vs_msg_crack);

	TryToDecode(vs_msg_crack, vs_msg_len);

	return 0;
*/
#if 0
	/* newPnP test stage */

//	VS_IP4_CIDR _c1 = { {172, 16, 0, 0}, 12 };
//	ip_addr _a = {24, 2, 61, 100};
	printf("\n%d", VS_CheckIsInternetAddress("192.168.255.255"));
//	VS_CheckAddrInCIDR(VS_ConvertStrToAddr("172.31.1.1"), _c1, true);
	_getch();


	VS_UPnPInterface *_upnp = VS_UPnPInterface::Instance();
	unsigned long *ports = new unsigned long [5];
	while (_getch() != 27) {
		for (int i = 0; i < 5; ++i) {
			ports[i] = rand();
		}
		_upnp->FullAsyncPortMapping(GetCurrentThreadId(), ports, 5, "192.168.1.175");
	}
	return 0;
	WSAStartup(MAKEWORD(2,2), &wsaData);
	SOCKET sock = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,0,0,WSA_FLAG_OVERLAPPED);
	printf("\nErr = %d", WSAGetLastError());
	VS_ConnectionTCP theTCP;
	printf("\ntheTCP = %d\n", theTCP.Listen(15050));
	_getch();
#endif
	/*
	int offset = 0;
	std::ifstream _file("c:\\msg.txt");
	unsigned char *buf = new unsigned char[8192];
	memset(buf, 0xfe, 8192);
	while (!_file.eof()) {
		std::string _str;
		std::getline(_file, _str);
		int j = 0;
		for (size_t i = 0; i < _str.length(); i += 3) {
			buf[offset + j] = (char)strtol(_str.c_str() + i, 0, 16);
			++j;
		}
		offset += j;
	}

	VS_TransportMessage *_msg = new VS_TransportMessage();
	_msg->Set(buf);
	void* body;	unsigned long	bodySize;
	VS_Container cnt;
	bodySize = _msg->Body(&body);
	if (cnt.Deserialize(body, bodySize)) {
		printf("\npreved ok");
	}
	_getch();
	/* iterator erase sample
	std::map<int, int> list;
	std::map<int, int>::iterator ilist;
	for (int i =0; i<10; i++) {
		list[i] = i;
	}
	for (ilist = list.begin(); ilist != list.end(); ) {
		printf("\n%d", (*ilist).second);
		ilist = list.erase(ilist);
	}
	_getch();
	*/

	/*
	if (!DetectQosIsWorking()) {
		printf("\nQoS is not functional.");
		return -1;
	} else {
		printf("\nQoS is OK.");
		return 0;
	}
	*/

	printf("\n%s version %s", GUI_version, APP_version);
	GetModuleFileName(NULL, (char *)&myFileName, MAX_PATH + 1);
	__appSwitches.clear();
	{
		for (int i = 1; i < argc; i++) {
			char __prefix[3] = {0};
			strncpy_s((char *)&__prefix, 3, argv[i], 2);
			strcat_s((char *)&myFileParm, MAX_PATH + 1, argv[i]);
			strcat_s((char *)&myFileParm, MAX_PATH + 1, " ");
			std::string __index = ((char *)&__prefix);
			std::string __param = ((char *)argv[i] + 2);
			__appSwitches[__index] = __param;
		}
	}
	/* setup working mode */
	if (_tMainSetupWorkingMode() == -1) {
		printf("\nError :: bad parameters. please read _TestACSService.txt");
		return -1;
	}
	InitializeCriticalSection(&gcs);
	InitializeCriticalSection(&__reboot);
	int xResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (xResult != NO_ERROR) {
		printf("\nError :: WSAStartup() :: %d", GetLastError());
		return -1;
	};

	TIMECAPS ptc = {};
	timeGetDevCaps(&ptc, sizeof(TIMECAPS));
	timeBeginPeriod(ptc.wPeriodMin);
	printf("\nInformation :: timer resolution %d :: %d", ptc.wPeriodMin, GetLastError());
	VS_AcsLibInitial();
	VS_CryptoInit::Init(prov_OPENSSL); serverCertificate.Init(prov_OPENSSL);

	m_scope = new VS_Scope *[AppMode.nThreads];
	for (int i = 0; i < AppMode.nThreads; i++) {
		m_scope[i] = new VS_Scope(AppMode.address.c_str(), AppMode.users, AppMode.sleep_time);
	}
	m_flooder = new VS_Flooder *[1];
	m_flooder[0] = new VS_Flooder(AppMode.address.c_str(), AppMode.users, AppMode.flood_sleep, 1000/*AppMode.flood_mult*/, 1000, 2*60*60 * 1000);
//	VS_Scope *__scope1 = new VS_Scope(AppMode.address.c_str(), AppMode.users, AppMode.sleep_time);

	while (true) {
		bool _quit = false;
		bool _spec = false;
		char _nextkey = _getch();
		if (!_nextkey) {
			_spec = true;
			_nextkey = _getch();
		}
		switch (_nextkey) {
/*
			case 'c' : {
				SetEvent(__scope1->m_heCEVENT);
				break;
					   }
*/
			case 'q' : {
				InterlockedIncrement(&VS_Scope::m_terminate);
				SetEvent(VS_Scope::m_hTIMER);
				for (int i = 0; i < AppMode.nThreads; i++) {
					PostQueuedCompletionStatus(m_scope[i]->m_hIOCP, 0, NULL, NULL);
					PostQueuedCompletionStatus(m_scope[i]->m_hCCCP, 0, NULL, NULL);
					WaitForSingleObject(m_scope[i]->m_hCThread, INFINITE);
					WaitForSingleObject(m_scope[i]->m_hDThread, INFINITE);
					WaitForSingleObject(m_scope[i]->m_hZThread, INFINITE);
				}
				_quit = true;
				break;
					   }
/*
			case 'p' : {
				if (AppMode.syncwait_) {
					AppMode.syncwait_ = false;
					SetEvent(__scope1->m_hePAUSE);
				} else {
					AppMode.syncwait_ = true;
					ResetEvent(__scope1->m_hePAUSE);
				}
				break;
					   }
*/
			case ' ' : {
				signed long m_connections = 0;
				m_connections = InterlockedExchange(&m_scope[0]->m_clients, m_scope[0]->m_clients);
				printf("\nInformation : live endpoinds %d, cnt = %.2f, hst = %.2f", m_connections, timer.cRez(tm_connect), timer.cRez(tm_handshake));
				break;
					   }
		};
		if (_quit) {
			break; break;
		}
	}
    timeEndPeriod(ptc.wPeriodMin); delete[] m_strSysConfig; DeleteCriticalSection(&gcs); return 0;
}

void CreateNewInstance() {
	EnterCriticalSection(&__reboot);
	InterlockedIncrement(&VS_Scope::m_terminate);
	SetEvent(VS_Scope::m_hTIMER);
	for (int i = 0; i < AppMode.nThreads; i++) {
		PostQueuedCompletionStatus(m_scope[i]->m_hIOCP, 0, NULL, NULL);
		PostQueuedCompletionStatus(m_scope[i]->m_hCCCP, 0, NULL, NULL);
		WaitForSingleObject(m_scope[i]->m_hCThread, INFINITE);
		WaitForSingleObject(m_scope[i]->m_hDThread, INFINITE);
		WaitForSingleObject(m_scope[i]->m_hZThread, INFINITE);
	}
	ShellExecute(0, "open", (char *)&myFileName, (char *)&myFileParm, NULL, SW_SHOWDEFAULT);
	TerminateProcess(GetCurrentProcess(), 0);
};

void MyClNotifyHandler( HANDLE ClRegCtx, HANDLE ClIfcCtx, ULONG Event, HANDLE SubCode, ULONG BufSize, PVOID Buffer) {
};

bool DetectQosIsWorking() {
	bool QosOK = true;
	HANDLE ClientHandle; HANDLE IfcHandle; HANDLE flowHandle;
	// Initialize call back functions. We dont' need these, but is required for TcRegisterClient API  
	TCI_CLIENT_FUNC_LIST QoSFunctions = {};  
	QoSFunctions.ClAddFlowCompleteHandler = NULL;  
	QoSFunctions.ClDeleteFlowCompleteHandler = NULL;  
	QoSFunctions.ClModifyFlowCompleteHandler = NULL;  
	QoSFunctions.ClNotifyHandler = (TCI_NOTIFY_HANDLER)MyClNotifyHandler;  
	// Register the client with Traffic control interface.
	QosOK = (TcRegisterClient(CURRENT_TCI_VERSION, NULL, &QoSFunctions,&ClientHandle) == NO_ERROR);  
	if (QosOK) {  
		TC_IFC_DESCRIPTOR InterfaceBuffer[32] = {};  
		PTC_IFC_DESCRIPTOR pInterfaceBuffer = &InterfaceBuffer[0];  
		ULONG BufferSize = 32 * sizeof(TC_IFC_DESCRIPTOR);  
		// Find traffic control enabled interfaces on the cmachine  
		QosOK = ((TcEnumerateInterfaces(ClientHandle, &BufferSize, pInterfaceBuffer) == NO_ERROR) && (BufferSize != 0));
		if (QosOK)  {
			TCHAR interfaceName[500] = {};  
			// function maps a wide-character string to a new character string  
			WideCharToMultiByte(CP_ACP, 0, InterfaceBuffer[0].pInterfaceName, -1,interfaceName, sizeof(interfaceName), 0, 0 );  
			// The TcOpenInterface function identifies and opens an interface based on its text  
			// string, which is available from a call to TcEnumerateInterfaces  
			QosOK = (TcOpenInterface(interfaceName, ClientHandle, NULL, &IfcHandle) == NO_ERROR);  
			if (QosOK) {
				// Creating of Traffic flow headers  
				int curSize = sizeof (TC_GEN_FLOW )+ sizeof (QOS_DS_CLASS) + sizeof(QOS_OBJECT_HDR);  
				char *bufFlow = new char[curSize];  
				memset((void *)bufFlow, 0, curSize);
				PTC_GEN_FLOW newFlow = (PTC_GEN_FLOW) bufFlow;  
				// Create the new temp flow.  
				LPQOS_OBJECT_HDR objHdr = NULL;  
				//Set the Flow Spec  
/*
				newFlow->ReceivingFlowspec = default_g711;
				newFlow->SendingFlowspec = default_g711;
				newFlow->TcObjectsLength = sizeof(QOS_DS_CLASS) + sizeof(QOS_OBJECT_HDR);
*/

//Set the Flow Spec
newFlow->ReceivingFlowspec.DelayVariation = QOS_NOT_SPECIFIED;
newFlow->ReceivingFlowspec.Latency = QOS_NOT_SPECIFIED;
newFlow->ReceivingFlowspec.MaxSduSize = QOS_NOT_SPECIFIED;
newFlow->ReceivingFlowspec.MinimumPolicedSize = QOS_NOT_SPECIFIED;
newFlow->ReceivingFlowspec.PeakBandwidth = QOS_NOT_SPECIFIED;
newFlow->ReceivingFlowspec.ServiceType = SERVICETYPE_BESTEFFORT;//SERVICETYPE_CONTROLLEDLOAD;
newFlow->ReceivingFlowspec.TokenBucketSize = QOS_NOT_SPECIFIED;
newFlow->ReceivingFlowspec.TokenRate =QOS_NOT_SPECIFIED;

newFlow->SendingFlowspec.DelayVariation = QOS_NOT_SPECIFIED;
newFlow->SendingFlowspec.Latency = QOS_NOT_SPECIFIED;
newFlow->SendingFlowspec.MaxSduSize = QOS_NOT_SPECIFIED;
newFlow->SendingFlowspec.MinimumPolicedSize = QOS_NOT_SPECIFIED;
newFlow->SendingFlowspec.PeakBandwidth = QOS_NOT_SPECIFIED;//speed;//
newFlow->SendingFlowspec.ServiceType=SERVICETYPE_BESTEFFORT;//SERVICETYPE_CONTROLLEDLOAD;
newFlow->SendingFlowspec.TokenBucketSize = QOS_NOT_SPECIFIED;//speed;//
newFlow->SendingFlowspec.TokenRate = QOS_NOT_SPECIFIED;//10000;//speed;// if this value is greater than 1 mb than qos works as if it is 1 mb
newFlow->TcObjectsLength =sizeof QOS_DS_CLASS + sizeof QOS_OBJECT_HDR; //sizeof QOS_SD_MODE + sizeof QOS_SHAPING_RATE + sizeof QOS_OBJECT_HDR;//0;//sizeof QOS_DS_CLASS + sizeof QOS_OBJECT_HDR;


				// Set  DSCP.  
				LPQOS_DS_CLASS pQOSClass = (LPQOS_DS_CLASS)(&(newFlow->TcObjects[0]));  
				pQOSClass->ObjectHdr.ObjectType = QOS_OBJECT_DS_CLASS;  
				pQOSClass->ObjectHdr.ObjectLength = sizeof(QOS_DS_CLASS);
				pQOSClass->DSField = 0x00;  
				objHdr = (LPQOS_OBJECT_HDR)((char *)&(newFlow->TcObjects[0]) + sizeof(QOS_DS_CLASS));  
				// Set the end of the list  
				objHdr->ObjectType = QOS_OBJECT_END_OF_LIST;  
				objHdr->ObjectLength = sizeof QOS_OBJECT_HDR;  
				// adds a new flow on the specified interface. The flow is like a channel,  
				// that determines how to shape the traffic if it tunnelled through it.  
				int res = TcAddFlow(IfcHandle, NULL, 0, newFlow, &flowHandle); 
				QosOK = (res == NO_ERROR);
				delete [] bufFlow;
				TcCloseInterface(IfcHandle);
			}
		}
	}
	return QosOK;
};

