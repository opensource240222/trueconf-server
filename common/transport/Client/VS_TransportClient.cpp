#if defined(_WIN32) // Not ported yet

/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: Непосредственная реализация поддержки транспортного протокола на клиенте
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_TransportClient.cpp
/// \brief Непосредственная реализация поддержки транспортного протокола на клиенте
/// \note
///

//#define   _MY_DEBUG_
//#include "../../AddressBookCache/VZOchat7.h"

#include "VS_TransportClient.h"
#include "../VS_TransportDefinitions.h"
#include "../Lib/VS_TransportLib.h"
#include "acs/ConnectionManager/VS_ConnectionManager.h"
#include "acs/connection/VS_ConnectionSock.h"
#include "acs/connection/VS_ConnectionTCP.h"
#include "acs/connection/VS_ConnectionTLS.h"
#include "acs/Lib/VS_AcsLib.h"
#include "acs/Lib/VS_AcsLog.h"
#include "acs/Lib/VS_Acs64BitsMsg.h"
#include "net/EndpointRegistry.h"
#include "SecureLib/VS_SymmetricCrypt.h"
#include "SecureLib/VS_SecureHandshake.h"
#include "SecureLib/VS_CryptoInit.h"
#include "SecureLib/VS_SecureConstants.h"
#include "SecureLib/VS_Certificate.h"
#include "std/cpplib/VS_SimpleStr.h"
#include "std/cpplib/VS_WideStr.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_IntConv.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/ThreadUtils.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/VS_RegServer.h"

#include <process.h>

#include <map>

#if (MAXIMUM_WAIT_OBJECTS < 3)
#error "!!!  MAXIMUM_WAIT_OBJECTS < 3  !!!"
#endif // (MAXIMUM_WAIT_OBJECTS < 3)


#define CLIENT_MESSAGES_LOGGING 0
#ifndef _DEBUG
#define printf( x ) ;
#endif

//////////////////// Internal types,defines,variables and announcements //////////////////
struct Arg_Type
{
	unsigned short   type;
};
struct Arg_TransportConnection : public Arg_Type
{
	char   *endpoint;
};
struct Return_TransportConnection : public Arg_Type
{
	bool   ret;
	VS_ConnectionSock   *sndConn, *rcvConn;
	bool   isSndConnThread, isRcvConnThread;
};
struct Arg_SetTransportConnection : public Arg_Type
{
	char   *server_id;
	char   *cid;
	VS_ConnectionSock   *conn;
	bool   isAccept;
	unsigned long   version;
	unsigned short   maxConnSilenceMs;
	unsigned char   fatalSilenceCoef;
	unsigned long	problem;
	bool			isSSLSupport;
	bool			tcpKeepAliveSupport;
};
struct Arg_SetDefaultGate : public Arg_Type
{
	char   *target_endpoint;
};
struct Arg_RegisterServiceTreadId : public Arg_Type
{
	char	*service_name;
	DWORD	threadId;
	UINT	msgType;
};
struct Arg_RegisterServiceHWnd : public Arg_Type
{
	char   *service_name;
	HWND   hWnd;
	UINT   msgType;
};
struct Arg_Service : public Arg_Type
{
	char	*service_name;
	DWORD	*threadId;
	HWND	*hWnd;
	UINT	*msgType;
};
struct Arg_UnregisterService : public Arg_Type
{
	char   *service_name;
};
struct Arg_EndpointConnect : public Arg_Type
{
	char   *to_name;
	unsigned long timeout;
};
struct Return_Bool : public Arg_Type
{
	bool   ret;
};
union Arg_Type_Union
{
	unsigned short					type;
	Arg_TransportConnection			transportConnection;
	Return_TransportConnection		ret_transportConnection;
	Arg_SetTransportConnection		setTransportConnection;
	Arg_SetDefaultGate				setDefaultGate;
	Arg_RegisterServiceTreadId		registerServiceTreadId;
	Arg_RegisterServiceHWnd			registerServiceHWnd;
	Arg_Service						service;
	Arg_UnregisterService			unregisterService;
	Arg_EndpointConnect				endpointConnect;
	Return_Bool						ret;
};


//SSL Support
#define VS_SSL_SUPPORT
// Below listed defines not change !!!
#define   VS_TRANSPORT_MAX_NUMBER_OF_SERVICES    60
#define   VS_TRANSPORT_MAX_NUMBER_OF_ENDPOINTS   (( MAXIMUM_WAIT_OBJECTS - 1 ) / 4 )

#define   VS_TRANSPORT_MAX_CONN_SILENCE_MS   2500
#define   VS_TRANSPORT_FATAL_SILENCE_COEF    4

#define   VS_TC_TYPE_UNINSTALL							0
#define   VS_TC_TYPE_SET_TRANSPORT_CONNECTION			1
#define   VS_TC_TYPE_CLOSE_TRANSPORT_CONNECTION			2
#define   VS_TC_TYPE_REGISTER_SERVICE_THREAD_ID			3
#define   VS_TC_TYPE_REGISTER_SERVICE_HWND				4
#define   VS_TC_TYPE_IS_SERVICE_EXISTS					5
#define   VS_TC_TYPE_UNREGISTER_SERVICE					6
#define   VS_TC_TYPE_FULL_DISCONNECTION	        		7
#define   VS_TC_TYPE_GET_IP								8

#define   VS_WM_TRANSPORT_CLIENT_OFFSET    WM_APP + 0x1111
#define   VS_WM_TRANSPORT_CLIENT_MESSAGE   0 + VS_WM_TRANSPORT_CLIENT_OFFSET
#define   VS_WM_TRANSPORT_CLIENT_MESSAGE_WITHOUT_GATE 1 + VS_WM_TRANSPORT_CLIENT_OFFSET

#define   VS_TRANSPORT_CLIENT_TICK_MS   1000
#define   VS_TRANSPORT_CLIENT_SILENCE_MS   300000
#define   VS_TRANSPORT_CLIENT_CONNECT_MS   60000
#define   VS_TRANSPORT_CLIENT_MAX_QUEUE_MESSAGES   256
#define   VS_MAX_SSL_RECONNECTS	"Max SSL Reconnects"

static unsigned __stdcall TransportClientThread( void *arg );
static inline void ResetStaticVars( void );
static inline void AcceptHandler(const void *cb_arg, VS_ConnectionTCP *conn, net::HandshakeHeader *hs);
static inline void ProcessingManaging( VS_ClientMessage &mess );
static inline bool TransportClientStartProcess( void );
static bool volatile   flagInstall = TransportClientStartProcess();
static char   *logsEndpoint = 0;
static HANDLE   hThread = 0, hEventThread = 0, hEventTransport = 0;
static DWORD   idThread = 0;
static Arg_Type_Union   argUnion;
static CRITICAL_SECTION   argSect, gateSect, cidSect;
static const unsigned long   clientTickSizeMs = VS_TRANSPORT_CLIENT_TICK_MS;
static const unsigned long   maxConnSilenceMs = VS_TRANSPORT_MAX_CONN_SILENCE_MS;
static const unsigned long   fatalSilenceCoef = VS_TRANSPORT_FATAL_SILENCE_COEF;
std::map<VS_SimpleStr,VS_SimpleStr> globalCIDmap;

static unsigned long   currDiffTm, currTm, sumCurrDiffTm, tickDiffTm, tickTm;
static char   defGateEndpoint[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];

const signed long ip_len = 46;//16;
static char vs_last_source_ip[ip_len] = {0};

VS_AcsLog	*SSLReconnectsLogger(0);

static unsigned max_ssl_reconnects = 0;

VS_EndpointCallback		*epcb	= 0;	// endpoint callback class

static void GetMaxCountSSLReconectsSSL()
{
	VS_RegistryKey	regkey(true,"Current Configuration");
	if(!regkey.GetValue(&max_ssl_reconnects, sizeof(max_ssl_reconnects), VS_REG_INTEGER_VT, VS_MAX_SSL_RECONNECTS))
		max_ssl_reconnects = 2;
	SSLReconnectsLogger->TPrintf("MaxSSLReconnects = %d",max_ssl_reconnects);
}
//////////////////// Library functions (see VS_TransportClient.h) ////////////////////////
bool VS_InstallTransportClient( VS_EndpointCallback *epcb )
{
	char   epNm[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1] = { 0 };

	EnterCriticalSection( &argSect );
#ifdef VS_SSL_SUPPORT
	vs::InitOpenSSL();
#endif
	if (flagInstall)
	{	/* Here it will be necessary to throw off in TRACE */	goto go_ret;	}
	if (!VS_EndpointName( epNm, sizeof(epNm) ))
	{	/* Here it will be necessary to throw off in TRACE */	goto go_ret;	}
	if (VS_AddAcceptHandler( VS_Transport_PrimaryField, AcceptHandler, 0 ) <= 0)
	{	/* Here it will be necessary to throw off in TRACE */	goto go_ret;	}
#ifdef _DEBUG
	SSLReconnectsLogger = new VS_AcsLog("SSLReconnects",5000000,1000000);
#else
	SSLReconnectsLogger = new VS_AcsEmptyLog();
#endif
	::epcb = epcb;

	GetMaxCountSSLReconectsSSL();
	hEventThread = CreateEvent( 0, FALSE, FALSE, 0 );
	if (!hEventThread)
	{	/* Here it will be necessary to throw off in TRACE */	goto go_ret;	}
	hEventTransport = CreateEvent( 0, FALSE, FALSE, 0 );
	if (!hEventTransport)
	{	/* Here it will be necessary to throw off in TRACE */	goto go_ret;	}
	hThread = (HANDLE)_beginthreadex( 0, 0, &TransportClientThread, 0, 0, (unsigned *)&idThread );
	if (!hThread || hThread == (HANDLE)-1L)
	{	/* Here it will be necessary to throw off in TRACE */	hThread = 0;	goto go_ret;	}
	if (WaitForSingleObject( hEventTransport, INFINITE ) != WAIT_OBJECT_0)
	{	/* Here it will be necessary to throw off in TRACE */	goto go_ret;	}
	flagInstall = true;
go_ret:
	if (!flagInstall)	ResetStaticVars();
	LeaveCriticalSection( &argSect );
	return flagInstall;
}


void VS_UninstallTransportClient( void )
{
	EnterCriticalSection( &argSect );
	epcb = 0;
	if (flagInstall)
	{	argUnion.type = VS_TC_TYPE_UNINSTALL;
	if (!SetEvent( hEventThread ))
	{	/* Here it will be necessary to throw off in TRACE */	}
	if (WaitForSingleObject( hThread, INFINITE ) != WAIT_OBJECT_0)
	{	/* Here it will be necessary to throw off in TRACE */	}
	else	CloseHandle( hThread );
	hThread = 0;	ResetStaticVars();
	flagInstall = false;
	}
	LeaveCriticalSection( &argSect );
}


char *VS_TransportClientCID( const char *server_id, char *cid, const unsigned int size_cid, unsigned int *necessary_size)
{
	VS_SimpleStr	ret_cid;
	VS_SimpleStr	srv = server_id;
	char *ret(0);

	EnterCriticalSection( &cidSect );
	if(flagInstall)
	{
		ret_cid = globalCIDmap[srv];
		unsigned sz = ret_cid.Length()? ret_cid.Length()+ 1 : 0;
		if(cid && sz <= size_cid )
		{
			if(!sz)
				cid[0] = 0;
			else
				strcpy( cid, ret_cid);
			ret = cid;
		}
		else if(necessary_size)
			*necessary_size = sz;
	}
	LeaveCriticalSection( &cidSect );
	return ret;
}


bool VS_SetCID(const char *cid, const char *serverID)
{
	bool ret = false;
	EnterCriticalSection( &cidSect );
	if(flagInstall)
	{
		globalCIDmap[serverID] = cid;
		ret = true;
	}
	else
		ret = false;
	LeaveCriticalSection( &cidSect );
	return ret;
}


void VS_ClearCID()
{
	EnterCriticalSection( &cidSect );
	globalCIDmap.clear();
	LeaveCriticalSection( &cidSect );
}


bool VS_SetTransportClientDefaultGate(const char *gate_endpoint)
{
	if (gate_endpoint && strlen( gate_endpoint ) > VS_ACS_MAX_SIZE_ENDPOINT_NAME)
		return false;
	bool   ret = false;
	EnterCriticalSection( &gateSect );
	if (flagInstall) {
		strcpy( defGateEndpoint, gate_endpoint );
		ret = true;
	}
	LeaveCriticalSection( &gateSect );
	return ret;
}


bool VS_GetTransportClientDefaultGate(char *gate_endpoint, unsigned long len)
{
	if (!gate_endpoint || !len)
		return false;
	bool   ret = false;
	EnterCriticalSection( &gateSect );
	if (flagInstall) {
		if (len > strlen(defGateEndpoint)) {
			strcpy(gate_endpoint, defGateEndpoint);
			ret = true;
		}
	}
	LeaveCriticalSection( &gateSect );
	return ret;
}


void VS_DisconnectEndpoint( const char *endpoint )
{
	if (!endpoint || (strlen( endpoint ) > VS_ACS_MAX_SIZE_ENDPOINT_NAME))
		return;
	EnterCriticalSection( &argSect );
	if (!flagInstall) {		LeaveCriticalSection( &argSect );	return;		}
	Arg_TransportConnection   &arg = argUnion.transportConnection;
	argUnion.type = VS_TC_TYPE_CLOSE_TRANSPORT_CONNECTION;
	arg.endpoint = (char *)endpoint;
	if (!SetEvent( hEventThread ))
	{	/* Here it will be necessary to throw off in TRACE */	}
	if (WaitForSingleObject( hEventTransport, INFINITE ) != WAIT_OBJECT_0)
	{	/* Here it will be necessary to throw off in TRACE */	}
	LeaveCriticalSection( &argSect );
}


void VS_FullDisconnectEndpoint( const char *endpoint )
{
	if (strlen( endpoint ) > VS_ACS_MAX_SIZE_ENDPOINT_NAME)		return;
	EnterCriticalSection( &argSect );
	if (!flagInstall) {		LeaveCriticalSection( &argSect );	return;		}
	Arg_TransportConnection   &arg = argUnion.transportConnection;
	argUnion.type = VS_TC_TYPE_FULL_DISCONNECTION;
	arg.endpoint = (char *)endpoint;
	if (!SetEvent( hEventThread ))
	{	/* Here it will be necessary to throw off in TRACE */	}
	if (WaitForSingleObject( hEventTransport, INFINITE ) != WAIT_OBJECT_0)
	{	/* Here it will be necessary to throw off in TRACE */	}
	LeaveCriticalSection( &argSect );
}


void VS_GetEndpointSourceIP( const char *endpoint, char *dest, signed long &length )
{
	if (!dest) { length = -1; return; }
	dest[0] = 0;
	if (length < ip_len) { length = ip_len; return; }
	if (strlen( endpoint ) > VS_ACS_MAX_SIZE_ENDPOINT_NAME)		return;
	EnterCriticalSection( &argSect );
	if (!flagInstall) {		LeaveCriticalSection( &argSect );	return;		}
	Arg_TransportConnection   &arg = argUnion.transportConnection;
	argUnion.type = VS_TC_TYPE_GET_IP;
	arg.endpoint = (char *)endpoint;
	if (!SetEvent( hEventThread ))
	{	/* Here it will be necessary to throw off in TRACE */	}
	if (WaitForSingleObject( hEventTransport, INFINITE ) != WAIT_OBJECT_0)
	{	/* Here it will be necessary to throw off in TRACE */	}
	strncpy(dest, (char *)&vs_last_source_ip, ip_len - 1);
	dest[strlen((char *)&vs_last_source_ip)] = 0;
	LeaveCriticalSection( &argSect );
}


bool VS_RegisterService( const char *service_name, const DWORD threadId, const UINT msgType )
{
	bool   ret = false;
	if (strlen( service_name ) > VS_TRANSPORT_MAX_SIZE_SERVICE_NAME || !threadId)
		return ret;
	EnterCriticalSection( &argSect );
	if (!flagInstall) {		LeaveCriticalSection( &argSect );	return ret;		}
	Arg_RegisterServiceTreadId   &arg = argUnion.registerServiceTreadId;
	argUnion.type = VS_TC_TYPE_REGISTER_SERVICE_THREAD_ID;
	arg.service_name = (char *)service_name;
	arg.threadId = threadId;
	arg.msgType = msgType;
	if (!SetEvent( hEventThread ))
	{	/* Here it will be necessary to throw off in TRACE */	}
	if (WaitForSingleObject( hEventTransport, INFINITE ) != WAIT_OBJECT_0)
	{	/* Here it will be necessary to throw off in TRACE */	}
	else	ret = argUnion.ret.ret;
	LeaveCriticalSection( &argSect );	return ret;
}


bool VS_RegisterService( const char *service_name, HWND hWnd, const UINT msgType )
{
	bool   ret = false;
	if (strlen( service_name ) > VS_TRANSPORT_MAX_SIZE_SERVICE_NAME || !hWnd)
		return ret;
	EnterCriticalSection( &argSect );
	if (!flagInstall) {		LeaveCriticalSection( &argSect );	return ret;		}
	Arg_RegisterServiceHWnd   &arg = argUnion.registerServiceHWnd;
	argUnion.type = VS_TC_TYPE_REGISTER_SERVICE_HWND;
	arg.service_name = (char *)service_name;
	arg.hWnd = hWnd;
	arg.msgType = msgType;
	if (!SetEvent( hEventThread ))
	{	/* Here it will be necessary to throw off in TRACE */	}
	if (WaitForSingleObject( hEventTransport, INFINITE ) != WAIT_OBJECT_0)
	{	/* Here it will be necessary to throw off in TRACE */	}
	else	ret = argUnion.ret.ret;
	LeaveCriticalSection( &argSect );
	return ret;
}


bool VS_IsServiceExists( const char *service_name, DWORD *threadId, HWND *hWnd, UINT *msgType )
{
	bool   ret = false;
	if (strlen( service_name ) > VS_TRANSPORT_MAX_SIZE_SERVICE_NAME)	return ret;
	EnterCriticalSection( &argSect );
	if (!flagInstall) {		LeaveCriticalSection( &argSect );	return ret;		}
	Arg_Service   &arg = argUnion.service;
	argUnion.type = VS_TC_TYPE_IS_SERVICE_EXISTS;
	arg.service_name = (char *)service_name;
	arg.threadId = threadId;
	arg.hWnd = hWnd;
	arg.msgType = msgType;
	if (!SetEvent( hEventThread ))
	{	/* Here it will be necessary to throw off in TRACE */	}
	if (WaitForSingleObject( hEventTransport, INFINITE ) != WAIT_OBJECT_0)
	{	/* Here it will be necessary to throw off in TRACE */	}
	else	ret = argUnion.ret.ret;
	LeaveCriticalSection( &argSect );
	return ret;
}


void VS_UnregisterService( const char *service_name )
{
	if (strlen( service_name ) > VS_TRANSPORT_MAX_SIZE_SERVICE_NAME)	return;
	EnterCriticalSection( &argSect );
	if (!flagInstall) {		LeaveCriticalSection( &argSect );	return;		}
	Arg_UnregisterService   &arg = argUnion.unregisterService;
	argUnion.type = VS_TC_TYPE_UNREGISTER_SERVICE;
	arg.service_name = (char *)service_name;
	if (!SetEvent( hEventThread ))
	{	/* Here it will be necessary to throw off in TRACE */	}
	if (WaitForSingleObject( hEventTransport, INFINITE ) != WAIT_OBJECT_0)
	{	/* Here it will be necessary to throw off in TRACE */	}
	LeaveCriticalSection( &argSect );
}




//////////////////////////////////////////////////////////////////////////////////////////

inline bool TransportClientStartProcess( void )
{
	InitializeCriticalSection( &argSect );
	InitializeCriticalSection( &gateSect);
	InitializeCriticalSection( &cidSect );

	return false;
}


//////////////////// Realization of class VS_ClientMessage ///////////////////////////////

VS_ClientMessage::VS_ClientMessage( void ):isOurMessage(0), dropGate(false) {}

VS_ClientMessage::VS_ClientMessage( MSG *msg ):isOurMessage(0),dropGate(false){	Set( msg );		}

VS_ClientMessage::VS_ClientMessage( const char *our_service, const char *add_string,
								   const char *endpoint, const char *service,
								   const unsigned long ms_timelimit,
								   const void *body, const unsigned long size,
								   const char * to_user, const char * from_user,
								   const char * to_server, const char *from_server)
								   :isOurMessage(0),dropGate(false)
{

	VS_TransportMessage::Set(1, VS_GetNewSequense(), 0, our_service, add_string, 0, service,
		ms_timelimit, body, size, to_user, from_user, to_server, from_server);
}


VS_ClientMessage::~VS_ClientMessage( void ) {}

bool VS_ClientMessage::Set( MSG *msg )
{	return VS_TransportMessage::Set( (const unsigned char *)VS_Get64Bits( *msg ) );	}

unsigned long VS_ClientMessage::SendAct( bool dropGate )
{
	unsigned long   seq_number = ((transport::MessageFixedPart*)mess)->seq_number;
	MSG   msg;
	msg.message = dropGate ? VS_WM_TRANSPORT_CLIENT_MESSAGE_WITHOUT_GATE: VS_WM_TRANSPORT_CLIENT_MESSAGE;
	VS_Set64Bits( (unsigned __int64)mess, msg );
	if (!PostThreadMessage( idThread, dropGate ? VS_WM_TRANSPORT_CLIENT_MESSAGE_WITHOUT_GATE: VS_WM_TRANSPORT_CLIENT_MESSAGE, msg.wParam, msg.lParam ))
		return 0;
	mess = 0;
	Reset();
	return seq_number;
}

unsigned long VS_ClientMessage::Send( bool dropGate)
{
	if (!flagInstall || !isValid || Type() != transport::MessageType::Request)
		return false;
	return SendAct(dropGate);
}
unsigned long VS_ClientMessage::Reply( const unsigned long ms_timelimit,
									  const void *body, const unsigned long size )
{
	if (!FormReply( 0, ms_timelimit, body, size ))	return 0;
	return SendAct();
}

unsigned long VS_ClientMessage::Recv( const DWORD milliseconds, const UINT msgType )
{
	Reset();
	DWORD   mills = milliseconds, oldTick = GetTickCount(), tmpTick;
	MSG   msg = { 0 };
	while (1)
	{
		while (PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
			if (msg.message == msgType && Set( &msg ))	return Sequence();
		switch (MsgWaitForMultipleObjects( 0, 0, FALSE, mills, QS_ALLPOSTMESSAGE ))
		{
		case WAIT_OBJECT_0 :	break;
		case WAIT_TIMEOUT :		return 0;
		default :	return 0; // Here it will be necessary to throw off in TRACE
		}
		tmpTick = GetTickCount();
		oldTick = tmpTick - oldTick;
		mills = oldTick < mills ? mills - oldTick : 0;
		oldTick = tmpTick;
	}	}

unsigned long VS_ClientMessage::Recv( const DWORD milliseconds, const UINT msgTypeMin, const UINT msgTypeMax )
{
	Reset();
	DWORD   mills = milliseconds, oldTick = GetTickCount(), tmpTick;
	MSG   msg = { 0 };
	while (1)
	{	while (PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
	if (msg.message >= msgTypeMin && msg.message <= msgTypeMax && Set( &msg ))
		return Sequence();
	switch (MsgWaitForMultipleObjects( 0, 0, FALSE, mills, QS_ALLPOSTMESSAGE ))
	{
	case WAIT_OBJECT_0 :	break;
	case WAIT_TIMEOUT :		return 0;
	default :	return 0; // Here it will be necessary to throw off in TRACE
	}
	tmpTick = GetTickCount();
	oldTick = tmpTick - oldTick;
	mills = oldTick < mills ? mills - oldTick : 0;
	oldTick = tmpTick;
	}	}



//////////////////// Internal realizing defines,types,variables and functions ////////////
struct VS_TransportClient_Service
{
	VS_TransportClient_Service( Arg_RegisterServiceTreadId &arg ) : isValid(false)
	{
		strcpy( name, arg.service_name );	msgType = arg.msgType;
		typeThread = true;	threadId = arg.threadId;	hWnd = 0;
		isValid = true;
	}

	VS_TransportClient_Service( Arg_RegisterServiceHWnd &arg ) : isValid(false)
	{
		strcpy( name, arg.service_name );	msgType = arg.msgType;
		typeThread = false;		threadId = 0;	hWnd = arg.hWnd;
		isValid = true;
	}

	~VS_TransportClient_Service( void ) {}

	bool	isValid, typeThread;
	char	name[VS_TRANSPORT_MAX_SIZE_SERVICE_NAME + 1];
	DWORD	threadId;
	HWND	hWnd;
	UINT	msgType;


	//////////////////////////////////////////////////////////////////////////////////////
	static inline VS_TransportClient_Service *GetService( const char *name )
	{
		VS_TransportClient_Service   *serv = headServ;
		for (unsigned i = 0; i < nSers && serv; ++i, serv = serv->nextServ)
			if (!strncmp( name, serv->name, sizeof(serv->name) ))	return serv;
		return 0;
	}

	static inline void AddService( VS_TransportClient_Service *serv )
	{
		if (!nSers)
		{	headServ = endServ = serv;	serv->previousServ = serv->nextServ = 0;	}
		else
		{	endServ->nextServ = serv;	serv->previousServ = endServ;
		endServ = serv;		serv->nextServ = 0;		}
		++nSers;
	}

	static inline void DeleteService( VS_TransportClient_Service *serv )
	{
		if (nSers == 1)		;
		else if (serv == headServ)
		{	headServ = serv->nextServ;	headServ->previousServ = 0;		}
		else if (serv == endServ)
		{	endServ = serv->previousServ;	endServ->nextServ = 0;		}
		else
		{	serv->nextServ->previousServ = serv->previousServ;
		serv->previousServ->nextServ = serv->nextServ;		}
		if (!--nSers)		headServ = endServ = 0;
		delete serv;
	}

	static inline void DeleteAllServices( void )
	{
		VS_TransportClient_Service   *serv = headServ, *serv_next;
		for (unsigned i = 0; i < VS_TRANSPORT_MAX_NUMBER_OF_SERVICES && serv; ++i, serv = serv_next)
		{	serv_next = serv->nextServ;		DeleteService( serv );	}
		headServ = endServ = 0;		nSers = 0;
	}


	//////////////////////////////////////////////////////////////////////////////////////
	static inline void RegisterTreadId( void )
	{
		argUnion.ret.ret = false;
		if (nSers >= VS_TRANSPORT_MAX_NUMBER_OF_SERVICES)	return;
		VS_TransportClient_Service   *serv = GetService( argUnion.registerServiceTreadId.service_name );
		if (serv)	return;
		serv = new VS_TransportClient_Service( argUnion.registerServiceTreadId );
		if (!serv)		return;
		if (!serv->isValid) {	delete serv;	return;		}
		AddService( serv );		argUnion.ret.ret = true;
	}

	static inline void RegisterHWnd( void )
	{
		argUnion.ret.ret = false;
		if (nSers >= VS_TRANSPORT_MAX_NUMBER_OF_SERVICES)	return;
		VS_TransportClient_Service   *serv = GetService( argUnion.registerServiceHWnd.service_name );
		if (serv)	return;
		serv = new VS_TransportClient_Service( argUnion.registerServiceHWnd );
		if (!serv)		return;
		if (!serv->isValid) {	delete serv;	return;		}
		AddService( serv );		argUnion.ret.ret = true;
	}

	static inline void IsServiceExists( void )
	{
		Arg_Service   &arg = argUnion.service;
		VS_TransportClient_Service   *serv = GetService( arg.service_name );
		if (serv)
		{
			*arg.threadId = serv->threadId;
			*arg.hWnd = serv->hWnd;
			*arg.msgType = serv->msgType;
			argUnion.ret.ret = true;
		}
		argUnion.ret.ret = false;
	}

	static inline void UnregisterService( void )
	{
		Arg_UnregisterService   &arg = argUnion.unregisterService;
		VS_TransportClient_Service   *serv = GetService( arg.service_name );
		if (serv)	DeleteService( serv );
	}

	static inline void ProcessingMessages( VS_ClientMessage &mess )
	{
		const char   *service = !mess.IsNotify() ? mess.DstService() : mess.SrcService();
		if (!service || !*service) {
			ProcessingManaging( mess );
		}
		else {
			MSG   msg;		VS_Set64Bits( (unsigned __int64)mess.mess, msg );
			VS_TransportClient_Service   *serv = GetService( service );
			if (!serv) {
				/* In these places it will be necessary to throw off in TRACE */	}
			else if (serv->threadId) {
				if (!PostThreadMessage( serv->threadId, serv->msgType, msg.wParam, msg.lParam )) {
					/* In these places it will be necessary to throw off in TRACE */
				}
				else mess.mess = 0;
			}
			else if (serv->hWnd) {
				if (!PostMessage( serv->hWnd, serv->msgType, msg.wParam, msg.lParam )) {
					/* In these places it will be necessary to throw off in TRACE */
				}
				else	mess.mess = 0;
			}
			else
			{
				/* In these places it will be necessary to throw off in TRACE */
			}
		}
		mess.Reset();
	}

	VS_TransportClient_Service			*previousServ, *nextServ;
	static VS_TransportClient_Service	*headServ, *endServ;
	static unsigned						nSers;
};




VS_TransportClient_Service   *VS_TransportClient_Service::headServ = 0, *VS_TransportClient_Service::endServ = 0;
unsigned   VS_TransportClient_Service::nSers = 0;
//////////////////////////////////////////////////////////////////////////////////////////

#define   VS_TC_SND_WRITE   0
#define   VS_TC_SND_READ    1
#define   VS_TC_RCV_WRITE   2
#define   VS_TC_RCV_READ    3
#define   VS_TC_BOTH_WRITE	4
#define	  VS_TC_BOTH_READ	5

static HANDLE   hs[MAXIMUM_WAIT_OBJECTS];
static struct VS_TransportClient_Endpoint   *tes[MAXIMUM_WAIT_OBJECTS];
static unsigned   tts[MAXIMUM_WAIT_OBJECTS];
static DWORD   nhs;

static std::map<VS_SimpleStr,bool>	SSLIsTurnOn;
static std::map<VS_SimpleStr,int>	EndpointsRcvSSLReconnects;
static std::map<VS_SimpleStr,int>	EndpointsSndSSLReconnects;
static std::map<VS_TransportClient_Endpoint*,int>	EndpointsWaitingFree;


struct VS_TransportClient_Endpoint
{
	VS_TransportClient_Endpoint( const char *arg_endpoint ) : isValid(false),
		m_connectThreadHandle(0), bothConn(0),
		maxLackMs(VS_TRANSPORT_CLIENT_SILENCE_MS), maxConnectMs(VS_TRANSPORT_CLIENT_CONNECT_MS),
		maxMsgs(VS_TRANSPORT_CLIENT_MAX_QUEUE_MESSAGES),
		maxConnSilenceMs(::maxConnSilenceMs), fatalSilenceCoef(::fatalSilenceCoef),
		lackTm(currTm), lastTickTm(0), nMsgs(0), writeSize(0), readSize(0), rcvWriteSize(0),
		maxSndSilenceMs(0), maxSndSilenceCoefMs(0), maxRcvSilenceMs(0), maxRcvSilenceCoefMs(0),
		lastReadTm(0), lastWriteTm(0), writeBuffer(0), readBuffer(0), stateRcv(0), endMsg(0), headMsg(0),
		newSecureAbility(false), oldSecureAbility(false), m_Logger(0),pReadCrypt(0),pWriteCrypt(0),
		rcvEncrBuf(0),sndEncrBuf(0),rcvEncrBufSize(0),sndEncrBufSize(0),
		stopStartedConns(false),pauseConns(false),//isEpDeleted(false),isAllEpDeletedFromMap(false),
		isNoNotify(false),recurcyTest(0),isFirstConnect(true), m_IsTCPPingSupported(false)
	{
		memset((void *)&readHead, 0, sizeof(readHead));
		memset(server_id,0,sizeof(server_id));
		strcpy( cid, arg_endpoint );
		isValid = true;
#ifdef _DEBUG
		m_Logger = new VS_AcsLog("VS_TransportClientEndPoint.log",10000000,4999999,"./");
#else
		m_Logger = new VS_AcsEmptyLog();
#endif
	}

	~VS_TransportClient_Endpoint( void )
	{
		WaitConnectThread();
		if(bothConn) {
			delete bothConn;
			if (epcb) epcb->OnConnect(server_id, err_disconnect);
		}
#ifdef VS_SSL_SUPPORT
		if(pReadCrypt)
			VS_SecureHandshake::ReleaseSymmetricCrypt(&pReadCrypt);
		if(pWriteCrypt)
			VS_SecureHandshake::ReleaseSymmetricCrypt(&pWriteCrypt);
#endif
		if(rcvEncrBufSize)
		{
			rcvEncrBufSize = 0;
			delete [] rcvEncrBuf;
			rcvEncrBuf = 0;
		}
		if(sndEncrBufSize)
		{
			sndEncrBufSize = 0;
			delete [] sndEncrBuf;
			sndEncrBuf = 0;
		}
	}

	bool   isValid;
	bool   stopStartedConns;
	bool   pauseConns;

	bool   isFirstConnect;

	char	cid[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
	char	server_id[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];

	HANDLE m_connectThreadHandle;
	VS_ConnectionSock	*bothConn;
	bool	isNoNotify;

	const unsigned long   maxLackMs, maxMsgs, maxConnSilenceMs, fatalSilenceCoef;
	unsigned long maxConnectMs;
	unsigned long   lackTm, lastTickTm, nMsgs, writeSize, readSize, rcvWriteSize,
		maxSndSilenceMs, maxSndSilenceCoefMs, maxRcvSilenceMs, maxRcvSilenceCoefMs,
		lastReadTm, lastWriteTm,recurcyTest;
	transport::MessageFixedPart readHead;
	unsigned char		*writeBuffer, *readBuffer;
	unsigned			stateRcv;
	bool                newSecureAbility;
	bool				oldSecureAbility;
	VS_SymmetricCrypt	*pReadCrypt,*pWriteCrypt;
	unsigned char		*rcvEncrBuf,*sndEncrBuf;
	uint32_t		rcvEncrBufSize, sndEncrBufSize;
#ifdef VS_SSL_SUPPORT
	VS_Certificate		serverCertificate;
#endif
	VS_AcsLog* m_Logger;
	bool m_IsTCPPingSupported;

	struct Tr_Msg
	{
		~Tr_Msg( void ) {	if (msg)	free( (void *)msg );	}
		transport::MessageFixedPart* msg;
		Tr_Msg   *previousMsg, *nextMsg;
	} *headMsg, *endMsg;

	static inline void SetTransportConnection( const char *server_id,
		VS_ConnectionSock *conn, unsigned long version, const bool isAccept,
		unsigned short maxConnSilenceMs, unsigned char fatalSilenceCoef,
		const char * cid = 0,
		const bool isSSLSupport = false,
		unsigned long problem = 0,
		bool tcpKeepAliveSupport = false)
	{
		EnterCriticalSection( &argSect );
		if (!flagInstall)
		{
			if (conn)
				delete conn;
			LeaveCriticalSection( &argSect );
			return;
		}
		Arg_SetTransportConnection   &arg = argUnion.setTransportConnection;
		argUnion.type = VS_TC_TYPE_SET_TRANSPORT_CONNECTION;
		arg.server_id = (char *)server_id;
		arg.cid = (char*)cid;
		arg.conn = conn;
		arg.version = version;
		arg.isAccept = isAccept;
		arg.maxConnSilenceMs = maxConnSilenceMs;
		arg.fatalSilenceCoef = fatalSilenceCoef;
		arg.isSSLSupport = isSSLSupport;
		arg.problem		 = problem;
		arg.tcpKeepAliveSupport = tcpKeepAliveSupport;

		if (!::SetEvent( hEventThread ))
		{	/* Here it will be necessary to throw off in TRACE */	}
		if (WaitForSingleObject( hEventTransport, INFINITE ) != WAIT_OBJECT_0)
		{	/* Here it will be necessary to throw off in TRACE */	}
		LeaveCriticalSection( &argSect );
	}

	static inline void AcceptHandler( const void *cb_arg, VS_ConnectionTCP *conn,
		net::HandshakeHeader *hs)
	{
		VS_SCOPE_EXIT {
			delete conn;
			free(hs);
		};
		if (!conn || !hs)
			return;
		char *endpoint(0), *o_e(0);
		unsigned char	hops = 0;
		unsigned long rnd_data_ln(0);
		const unsigned char *rnd_data(0);
		unsigned long sign_sz(0);
		const unsigned char *sign(0);
		bool tcpKeepAliveSupport(false);

		if (!VS_TransformTransportHandshake( hs, endpoint, o_e, hops,rnd_data_ln,rnd_data,sign_sz,sign, tcpKeepAliveSupport ))
			return;

		SetTransportConnection( endpoint, conn, hs->version, true, ::maxConnSilenceMs, ::fatalSilenceCoef, 0, false, 0, tcpKeepAliveSupport );
		conn = 0;
	}

	static inline bool CreateRefConnection( VS_ConnectionSock *&conn, long &milliseconds, unsigned &nTCP,
		const char* endpoint, net::HandshakeHeader* hs, VS_TransportClient_Endpoint* p)
	{
		unsigned long milliseconds1 = milliseconds/2;
		if(p && (p->stopStartedConns || p->pauseConns)) {
			printf("-");
			p->pauseConns = false;
			return false;
		}

		if (!conn) {
			printf("-");
			conn = 0;
			milliseconds = !milliseconds1 ? 0 : milliseconds/2 + milliseconds1;
			return false;
		}
		if(p && p->stopStartedConns) {
			printf("-");
			delete conn;
			conn = 0;
			return false;
		}

		unsigned long milliseconds2 = milliseconds/2 + milliseconds1;

		if (!conn->CreateOvReadEvent() || !conn->CreateOvWriteEvent() || !VS_WriteZeroHandshake( conn, hs, milliseconds2 )) {
			delete conn;
			conn = 0;
			printf("-");
			return false;
		}
		milliseconds = milliseconds2;
		return true;
	}

	static inline void DestroyPtr(net::HandshakeHeader*& hs)
	{
		if (hs) free( (void *)hs ); hs = 0;
	}

	static inline void ConnectHandler( unsigned long milliseconds, VS_TransportClient_Endpoint *p )
	{
		const unsigned long	pauseTimeout(5000);
		VS_ConnectionSock   *conn = 0;
		char   *cid(0);
		char	trCID[VS_ACS_MAX_SIZE_CID + 1] = {0};
		long	mills(0);
		unsigned		nTCP(0);

		//////////пока//////////////
		bool	bSSLSupport(true);
		/////////////////////////////

		if(!p)
			return;

		bool isConnectNeeded(true);
		net::HandshakeHeader* hs = nullptr;
		/***************************************************************
		Если присоединяемся впервые, то добавить реквест (то есть CID'а нет), если CID есть, то значит реконнект (может редирект),
		если в процессе реконнекта CID меняется, то добавляем реквест, если редирект, то надо сделать реквест
		***************************************************************/
		///////////////////////////////////////////////////////////////////////////////
		unsigned cid_sz(0);
		mills = milliseconds;
		unsigned long total_timeout(0), start_tick(GetTickCount());
		bool tryNextConnectTCP = false;

		while(isConnectNeeded)
		{
			unsigned char	resultCode(0);
			unsigned short	maxConnSilenceMs(0);
			unsigned char	fatalSilenceCoef(0);
			unsigned char	hops(0);
			unsigned long	version(0);
			unsigned long	problem(1);
			bool tcpKeepAliveSupport(false);
			char *serverIdReplay(0);

			conn = VS_CreateConnection( p->cid, milliseconds, &nTCP );
			tcpKeepAliveSupport = conn && conn->SetKeepAliveMode(true, 20000, 2000);

			if((!(hs = VS_FormTransportHandshake(VS_TransportClientCID(p->cid,trCID, sizeof(trCID)), p->cid, 0, bSSLSupport, true ))))
			{
				/** err_noresources
				выйти
				*/
				if (epcb&&!p->stopStartedConns)
					epcb->OnConnect(p->cid, err_noresources);
				if(conn) {
					delete conn;
					conn = 0;
				}
				break;
			}
			tryNextConnectTCP = false;
			if(!CreateRefConnection(conn,mills,nTCP,p->cid,hs,p))
			{
				/** либо таймаут, либо ошибка
				таймаут заново
				ошибка слиип и заново
				*/
				if(conn) {
					delete conn;
					conn = 0;
				}
				if(mills) {
					Sleep(pauseTimeout);
					mills = mills > pauseTimeout ? mills - pauseTimeout : 0;
				}
				DestroyPtr(hs);
			}
			else {
				DestroyPtr(hs);
				unsigned long readMills = mills > 2 * VS_CM_MIN_TCP_LIKE_HANDSHAKE_TIMEOUT ? mills / 2 : std::min<long>(mills, VS_CM_MIN_TCP_LIKE_HANDSHAKE_TIMEOUT);
				mills -= readMills;
				bool readRes = VS_ReadZeroHandshake( conn, &hs, readMills );
				mills += readMills;
				if(!readRes) {
					/** может быть таймаут или ошибка
					таймаут заново
					ошибка callmack error
					выход
					*/
					DestroyPtr(hs);
					tryNextConnectTCP = !readMills;
					if(conn) {
						delete conn;
						conn = 0;
					}
					if(readMills)
					{
						if (epcb&&!p->stopStartedConns)
							epcb->OnConnect(p->cid, err_inaccessible);
						break;
						/** Callback выход
						*/
					}
				}
				else if(!VS_TransformTransportReplyHandshake( hs, resultCode, maxConnSilenceMs, fatalSilenceCoef, hops, serverIdReplay, cid, tcpKeepAliveSupport))
				{
					/** Неправильное сообщение
					ошибка хендшейка и не таймаут, значит выходим
					*/
					if (epcb&&!p->stopStartedConns)
						epcb->OnConnect(p->cid,err_alienserver);
					if(conn) {
						delete conn;
						conn = 0;
					}
					DestroyPtr(hs);
					break;
				}
				else {
					// проверить resultCode, по resultCode можем понять что делать
					version = hs->version;
					// HS прошел успешно, дергаем SetTransportConnection и выходим, callback должен вызваться в SetTransportHandshake;
					switch(resultCode)
					{
					case 3:
						if (epcb&&!p->stopStartedConns)
							epcb->OnConnect(p->cid,err_antikserver);
						isConnectNeeded = false;
						if(conn) {
							delete conn;
							conn = 0;
						}
						DestroyPtr(hs);
						continue;
					case 4:
						// Чуждый брокер, я вас не знаю.
						if (epcb&&!p->stopStartedConns)
							epcb->OnConnect(p->cid,err_alienserver);
						isConnectNeeded = false;
						if(conn) {
							delete conn;
							conn = 0;
						}
						DestroyPtr(hs);
						continue;
					case 5:
						// Сервер перегружен, делать паузу или нет? специальная ошибка
						if (epcb&&!p->stopStartedConns)
							epcb->OnConnect(p->cid,err_serverbusy);
						isConnectNeeded = false;
						if(conn) {
							delete conn;
							conn = 0;
						}
						DestroyPtr(hs);
						continue;
					case 6:
						/////////BREAK/////////////////////////////////////////////////////////////////
						/// Брокер не поддерживает не SSL. Реинстала нет - дождемся свичинга брокеров.
						/// делать ли паузу
						///////////////////////////////////////////////////////////////////////////////
						//DestroyPtr(hs);
						break;
					case 0:
						//DestroyPtr(hs);
						if (nTCP > 1)
							net::endpoint::MakeFirstConnectTCP(nTCP, p->cid);
						break;
					default:
						// старый клиент or unknown error
						if (epcb&&!p->stopStartedConns)
							epcb->OnConnect(p->cid,err_antikclient);
						isConnectNeeded = false;
						if(conn) {
							delete conn;
							conn = 0;
						}
						DestroyPtr(hs);
						continue;
					}
					//  serverIdReplay == 0 check missed
					if (_stricmp(p->cid, serverIdReplay) != 0) {
						if (epcb&&!p->stopStartedConns)
							epcb->OnSidChange(p->cid, serverIdReplay);
					}
					SetTransportConnection(serverIdReplay, conn, version, false, maxConnSilenceMs, fatalSilenceCoef, cid, bSSLSupport, problem, tcpKeepAliveSupport);
					DestroyPtr(hs);
					break;
				}
			}
			/**
			Вычислить isConnectNeeded
			если первый коннект, то одна логика, если реконнект, то другая

			если соединяемся впервые,
			то наличие сообщения для еп это и есть критерий
			если реконнект
			то засекаем тймаут
			*/
			total_timeout = GetTickCount() - start_tick;
			if(p->isFirstConnect) {
				if(!(isConnectNeeded = (p->IsMessageExist() && !p->stopStartedConns || tryNextConnectTCP) && (total_timeout < milliseconds))) {
					if (epcb&&!p->stopStartedConns) epcb->OnConnect(p->cid,err_inaccessible);
				}
			}
			else {
				if(total_timeout >= milliseconds) {
					isConnectNeeded = false;
					if (epcb&&!p->stopStartedConns) epcb->OnConnect(p->cid,err_disconnect);
				}
				else
					isConnectNeeded = !p->stopStartedConns;
			}
		}
	}

	bool IsMessageExist()
	{
		return !!nMsgs;
	}

	bool ClientSecureHandshake()
	{
		return BothClientSecureHandshake();
	}

	bool BothClientSecureHandshake()
	{
		serverCertificate = {};
		SSLReconnectsLogger->TPrintf("RcvClientSecureHandshake start. Endpoint = %s",cid);
		VS_SecureHandshake SecureHandshakeMgr;
		if(!bothConn)
			return false;
		if(!SecureHandshakeMgr.Init(1,handshake_type_Client))
			return false;
		VS_SecureHandshakeState state = SecureHandshakeMgr.Next();
		char *buf(0);
		uint32_t buf_size(0);
		bool	bRes(false);
		unsigned long mills(5000);
		while((secure_st_Error!=state)&&(secure_st_Finish!=state))
		{
			mills = 5000;
			switch(state)
			{
			case secure_st_GetPacket:
				if(!SecureHandshakeMgr.PreparePacket((void**)&buf,&buf_size))
					bRes = false;
				else
				{
					if((!bothConn->Read(buf,buf_size))||
					   (bothConn->GetReadResult(mills)!= buf_size))
					{
						SecureHandshakeMgr.FreePacket((void**)&buf);
						buf_size = 0;
						bRes = false;
					}
					else
					{
						bRes = SecureHandshakeMgr.ProcessPacket(buf,buf_size);
						SecureHandshakeMgr.FreePacket((void**)&buf);
						buf_size = 0;
					}
				}
				break;
			case secure_st_SendPacket:
				if(!SecureHandshakeMgr.PreparePacket((void**)&buf,&buf_size))
					bRes = false;
				else
				{
					if((!bothConn->Write(buf,buf_size))||
						(bothConn->GetWriteResult(mills)!= buf_size))
					{
						SecureHandshakeMgr.FreePacket((void**)&buf);
						buf_size = 0;
						bRes = false;
					}
					else
					{
						SecureHandshakeMgr.FreePacket((void**)&buf);
						buf_size = 0;
						bRes = true;
					}
				}
				break;
			case secure_st_SendCert:
				bRes = false;
				break;
			case secure_st_Error:
				bRes = false;
				break;
			case secure_st_Finish:
				bRes = true;
				break;
			default:
				bRes = false;
			}
			if(!bRes)
			{
				if((e_err_VerificationFailed == SecureHandshakeMgr.GetHandshakeErrorCode())||
					(e_err_DataAreNotCertificate == SecureHandshakeMgr.GetHandshakeErrorCode()))
				{
					SSLReconnectsLogger->TPrintf("Certificate Verification failed. Endpoint = %s",cid);
					uint32_t sz(0);
					char *pem_buf(0);
					if(e_err_VerificationFailed == SecureHandshakeMgr.GetHandshakeErrorCode())
					{
						SecureHandshakeMgr.GetCertificate(pem_buf,sz);
						VS_Certificate	cert;
						if(sz)
						{
							pem_buf = new char[sz];
							if(SecureHandshakeMgr.GetCertificate(pem_buf,sz))
							{
								if(cert.SetCert(pem_buf,sz,store_PEM_BUF))
								{
									switch(cert.CheckExpirationTime())
									{
									case 1:

										/**
	Server certificate expired
	Сертификат сервера истек
										*/
										if(epcb) epcb->OnConnect(server_id,err_cert_expired);
										break;
									case -1:
										if(epcb) epcb->OnConnect(server_id,err_cert_not_yet_valid);
										/**
									Server certificate is not yet valid
	Сертификат сервера не начал действовать

										*/
										break;
									default:
										if(epcb) epcb->OnConnect(server_id,err_cert_is_invalid);
										/**
	Certificate is invalid
	Сертификат недействителен
										*/
										break;
									};
									/**

									*/
								}
							}
							else
							{
								if(epcb) epcb->OnConnect(server_id,err_cert_is_invalid);
							}
							delete [] pem_buf;
							sz = 0;
						}
					}
					else
						if(epcb) epcb->OnConnect(server_id,err_cert_is_invalid);
				}
				else
				{
					if(max_ssl_reconnects)
					{
						unsigned long counter(0);
						if(EndpointsRcvSSLReconnects.end() == EndpointsRcvSSLReconnects.find(cid))
							EndpointsRcvSSLReconnects[cid] = 1;
						else
							EndpointsRcvSSLReconnects[cid]++;
						counter = EndpointsRcvSSLReconnects[cid];
						SSLReconnectsLogger->TPrintf("Endpoint = %s, failed RCV handshake N %d",cid,counter);
					}
				}
				SSLReconnectsLogger->TPrintf("RcvClientSecureHandshake failed");
				SecureHandshakeMgr.StopHandshake();
				return false;
			}
			if(secure_st_Finish != state)
				state = SecureHandshakeMgr.Next();
		}
		if(secure_st_Finish==state)
		{
			uint32_t sz(0);
			char *pem_buf(0);
			SecureHandshakeMgr.GetCertificate(pem_buf,sz);
			if(sz)
			{
				pem_buf = new char[sz];
				if(SecureHandshakeMgr.GetCertificate(pem_buf,sz))
					serverCertificate.SetCert(pem_buf,sz,store_PEM_BUF);
				delete [] pem_buf;
				sz = 0;
			}
			if(_stricmp(RegServerName,server_id))
			{
				VS_WideStr serverName, country, organization, contactPerson,email;
				VS_SimpleStr notBefore, notAfter;
				if(GetCertificateData(serverName,country,organization,contactPerson,email,notBefore,notAfter))
				{
					VS_WideStr tmpstr;
					tmpstr.AssignUTF8(server_id);
					if(serverName!=tmpstr)
					{
						if(epcb) epcb->OnConnect(server_id,err_srv_name_is_invalid);
						SecureHandshakeMgr.StopHandshake();
						return false;
					}
				}
				else
				{
					if(epcb) epcb->OnConnect(server_id,err_cert_is_invalid);
					SecureHandshakeMgr.StopHandshake();
					return false;
				}
				std::string srv_ver_buf;
				if(!serverCertificate.GetExtension(SERVER_VERSION_EXTENSIONS,srv_ver_buf))
				{
					if(epcb) epcb->OnConnect(server_id,err_version_do_not_match);
					SecureHandshakeMgr.StopHandshake();
					return false;
				}
				else
				{
					if(atou_s(VCS_CLIENT_VERSION) > atou_s(srv_ver_buf.c_str()))
					{
						if(epcb) epcb->OnConnect(server_id,err_version_do_not_match);
						SecureHandshakeMgr.StopHandshake();
						return false;
					}
				}
			}

			pWriteCrypt = SecureHandshakeMgr.GetWriteSymmetricCrypt();
			pReadCrypt = SecureHandshakeMgr.GetReadSymmetricCrypt();
			SecureHandshakeMgr.GetCertificate(pem_buf,sz);
			if(sz)
			{
				pem_buf = new char[sz];
				if(SecureHandshakeMgr.GetCertificate(pem_buf,sz))
					serverCertificate.SetCert(pem_buf,sz,store_PEM_BUF);
				delete [] pem_buf;
				sz = 0;
			}
			SecureHandshakeMgr.StopHandshake();
			if(!pReadCrypt || !pWriteCrypt)
				return false;
			else
				return true;
		}
		else
		{
			if((e_err_VerificationFailed == SecureHandshakeMgr.GetHandshakeErrorCode())||
				(e_err_DataAreNotCertificate == SecureHandshakeMgr.GetHandshakeErrorCode()))
			{
				uint32_t sz(0);
				char *pem_buf(0);
				if(e_err_VerificationFailed == SecureHandshakeMgr.GetHandshakeErrorCode())
				{
					SecureHandshakeMgr.GetCertificate(pem_buf,sz);
					VS_Certificate	cert;
					if(sz)
					{
						pem_buf = new char[sz];
						if(SecureHandshakeMgr.GetCertificate(pem_buf,sz))
						{
							if(cert.SetCert(pem_buf,sz,store_PEM_BUF))
							{
								switch(cert.CheckExpirationTime())
								{
								case 1:

									/**
Server certificate expired
Сертификат сервера истек
									*/
									if(epcb) epcb->OnConnect(server_id,err_cert_expired);
									break;
								case -1:
									if(epcb) epcb->OnConnect(server_id,err_cert_not_yet_valid);
									/**
								Server certificate is not yet valid
Сертификат сервера не начал действовать

									*/
									break;
								default:
									if(epcb) epcb->OnConnect(server_id,err_cert_is_invalid);
									/**
Certificate is invalid
Сертификат недействителен
									*/
									break;

								};
								/**

								*/
							}
						}
						else
						{
							if(epcb) epcb->OnConnect(server_id,err_cert_is_invalid);
						}
						delete [] pem_buf;
						sz = 0;
					}
				}
				else
					if(epcb) epcb->OnConnect(server_id,err_cert_is_invalid);
			}
			else
			{
				if(max_ssl_reconnects)
				{
					unsigned long counter(0);
					if(EndpointsRcvSSLReconnects.end() == EndpointsRcvSSLReconnects.find(cid))
						EndpointsRcvSSLReconnects[cid] = 0;
					else
						EndpointsRcvSSLReconnects[cid]++;
					SSLReconnectsLogger->TPrintf("Endpoint = %s, failed RCV handshake N %d",cid,counter);
					counter = EndpointsRcvSSLReconnects[cid];
				}
			}
			SSLReconnectsLogger->TPrintf("RcvClientSecureHandhake failed!");
			SecureHandshakeMgr.StopHandshake();
			return false;
		}
	}

	bool GetCertificateData(VS_WideStr &serverEP,
		VS_WideStr &country,
		VS_WideStr &organization,
		VS_WideStr &contact_person,
		VS_WideStr &email,
		VS_SimpleStr &notBefore,
		VS_SimpleStr &notAfter)
	{
		std::string buf;
		std::string notAfterBuf, notBeforeBuf;
		if(!oldSecureAbility && !newSecureAbility)
			return false;
		if(serverCertificate.GetSubjectEntry("commonName",buf))
			serverEP.AssignUTF8(buf.c_str());
		else
			serverEP.Empty();
		if(serverCertificate.GetSubjectEntry("countryName",buf))
			country.AssignUTF8(buf.c_str());
		else
			country.Empty();
		if(serverCertificate.GetSubjectEntry("organizationName",buf))
			organization.AssignUTF8(buf.c_str());
		else
			organization.Empty();
		if(serverCertificate.GetSubjectEntry("surname",buf))
			contact_person.AssignUTF8(buf.c_str());
		else
			contact_person.Empty();
		if(serverCertificate.GetSubjectEntry("emailAddress",buf))
			email.AssignUTF8(buf.c_str());
		else
			email.Empty();
		if(serverCertificate.GetExpirationTime(notBeforeBuf, notAfterBuf))
		{
			notBefore = notBeforeBuf.c_str();
			notAfter = notAfterBuf.c_str();
		}
		else
		{
			notBefore.Empty();
			notAfter.Empty();
		}
		return true;
	}


	struct ArgConnectTransport
	{
		ArgConnectTransport( const unsigned long milliseconds, VS_TransportClient_Endpoint *ep): ep_(ep)
		{
			ArgConnectTransport::milliseconds = milliseconds;
		}
		unsigned long   milliseconds;
		VS_TransportClient_Endpoint *ep_;

	};

	static unsigned __stdcall ConnectThread( void *arg )
	{
		vs::SetThreadName("TransportClient");
		if (arg)
		{
			ArgConnectTransport   *c = (ArgConnectTransport *)arg;
			ConnectHandler( c->milliseconds, c->ep_);
			delete c;
		}
		return 0;
	}

	static inline HANDLE StartConnectThread( const unsigned long milliseconds, VS_TransportClient_Endpoint *ep )
	{
		ArgConnectTransport   *arg = new ArgConnectTransport( milliseconds, ep );
		if (arg)
		{
			uintptr_t   thr = _beginthreadex(0,0,ConnectThread,arg,0,0);
			if (thr != -1L && thr)
				return (HANDLE) thr;
			else
				delete arg;
		}
		return 0;
	}


	inline void CloseBothConnStartThread()
	{
		if (bothConn)
		{
			delete bothConn;
			bothConn = 0;
		}
#ifdef VS_SSL_SUPPORT
		if(pReadCrypt)
			VS_SecureHandshake::ReleaseSymmetricCrypt(&pReadCrypt);
		if(pWriteCrypt)
			VS_SecureHandshake::ReleaseSymmetricCrypt(&pWriteCrypt);
#endif
		if(rcvEncrBufSize)
		{
			rcvEncrBufSize = 0;
			delete [] rcvEncrBuf;
			rcvEncrBuf = 0;
		}
		if(sndEncrBufSize)
		{
			sndEncrBufSize = 0;
			delete [] sndEncrBuf;
			sndEncrBuf = 0;
		}
		StartConnectThreads();
	}

	inline void StartConnectThreads( void )
	{
		if(!bothConn && !IsConnectProcessing())
			m_connectThreadHandle = StartConnectThread( maxConnectMs, this);
	}
	inline bool IsConnectProcessing()
	{
		if(m_connectThreadHandle)
		{
			if(WAIT_TIMEOUT==WaitForSingleObject(m_connectThreadHandle,0))
				return true;
			else
			{
				CloseHandle(m_connectThreadHandle);
				m_connectThreadHandle = 0;
				return false;
			}
		}
		else
			return false;
	}
	inline void WaitConnectThread()
	{
		if(m_connectThreadHandle)
		{
			WaitForSingleObject(m_connectThreadHandle,INFINITE);
			CloseHandle(m_connectThreadHandle);
			m_connectThreadHandle = 0;
		}
	}
	inline void StopStartedConnections()
	{
		stopStartedConns = true;
	}
	inline void PauseConnections()
	{
		pauseConns = true;
	}
	inline void DeleteMsg( Tr_Msg *deleteMsg , bool isMarked = false)
	{
		if (!deleteMsg) return;

		bool res = false;
		VS_ClientMessage   *mess = new VS_ClientMessage;
		if (mess && deleteMsg->msg)
		{
			deleteMsg->msg->notify = 1;
			if (mess->VS_TransportMessage::Set( (const unsigned char *)deleteMsg->msg ))
			{
				res = true;
			}
		}

		if (nMsgs == 1)		;
		else if (deleteMsg == headMsg)
		{	headMsg = deleteMsg->nextMsg;	headMsg->previousMsg = 0;	}
		else if (deleteMsg == endMsg)
		{	endMsg = deleteMsg->previousMsg;	endMsg->nextMsg = 0;	}
		else
		{	deleteMsg->nextMsg->previousMsg = deleteMsg->previousMsg;
		deleteMsg->previousMsg->nextMsg = deleteMsg->nextMsg;
		}

		if (res && mess)
		{
			deleteMsg->msg = 0;
			delete deleteMsg;
			deleteMsg = 0;
			if (!--nMsgs)	headMsg = endMsg = 0;
			++recurcyTest;
			if (recurcyTest < 2 )
				VS_TransportClient_Service::ProcessingMessages( *mess );
			else
			{
				if (mess)
				{
					delete mess;
					mess = 0;
				}
				recurcyTest = 0;
			}
			if (recurcyTest!=0)
				--recurcyTest;
		}else
		{
			delete deleteMsg;
			deleteMsg = 0;
			if (!--nMsgs)	headMsg = endMsg = 0;
			if (mess)
			{
				delete mess;
				mess = 0;
			}
		}
	}

	inline void AddMsg( const unsigned char *addMsg )
	{
		if (!addMsg)	return;
		Tr_Msg   *msg = new Tr_Msg;
		msg->msg = (transport::MessageFixedPart*)addMsg;
		if (!nMsgs)
		{	headMsg = endMsg = msg;		msg->previousMsg = msg->nextMsg = 0;	}
		else
		{	endMsg->nextMsg = msg;	msg->previousMsg = endMsg;	endMsg = msg;
		msg->nextMsg = 0;	}
		if (++nMsgs > maxMsgs)		DeleteMsg( headMsg , true );
		StartConnectThreads();
	}

	inline void AddMsg( VS_ClientMessage &mess )
	{	AddMsg( mess.mess );	mess.mess = 0;	}

	inline void AddMsgToHead( const unsigned char *addMsg )
	{
		if (!addMsg)	return;
		if (nMsgs >= maxMsgs) {		free( (void *)addMsg );		return;		}
		Tr_Msg   *msg = new Tr_Msg;
		msg->msg = (transport::MessageFixedPart*)addMsg;
		if (!nMsgs)
		{	headMsg = endMsg = msg;		msg->previousMsg = msg->nextMsg = 0;	}
		else
		{	endMsg->previousMsg = msg;	msg->nextMsg = endMsg;	headMsg = msg;
		msg->previousMsg = 0;	}
		++nMsgs;
		StartConnectThreads();
	}

	inline void AddMsgToHead( VS_ClientMessage &mess )
	{	AddMsgToHead( mess.mess );	mess.mess = 0;	}

	inline void DeleteAllMessages( void )
	{
		Tr_Msg   *current = endMsg, *next = 0;
		/// Bug #925 fix
		for (; nMsgs && current;  current = next)
		{
			next = current->previousMsg;
			DeleteMsg( current );
		}
		/// Bug #925 fix end
	}

	inline void LogMessageToFile( VS_ClientMessage &mess )
	{
		const char * srce = mess.SrcCID();
		const char * dste = mess.DstCID();
		const char * srcs = mess.SrcService();
		const char * dsts = mess.DstService();
		const auto   type = mess.Type();
		const auto length = mess.BodySize();
		bool noSrc = (srcs==0) || (srcs && (*srcs==0));
		bool noDsc = (dsts==0) || (dsts && (*dsts==0));
		if (noSrc && noDsc)
		{
			this->m_Logger->TPrintf("%6.6d %10.10s|00000   ---MANAGE> %10.10s|00000",
				length,srce,dste);
		}else
		{
			if (noSrc)
			{
				this->m_Logger->TPrintf("%6.6d %10.10s|0000000000 -(%d)--> %10.10s|%10.10s",
					length,srce,type,dste,dsts);

			} else if (noDsc)
			{
				this->m_Logger->TPrintf("%6.6d %10.10s|%10.10s -(%d)--> %10.10s|00000",
					length,srce,srcs,type,dste);
			}else
			{
				this->m_Logger->TPrintf("%6.6d %10.10s|%10.10s -(%d)--> %10.10s|%10.10s",
					length,srce,srcs,type,dste,dsts);
			}
		}
	}
	static inline void LogMessage( VS_ClientMessage &mess )
	{
		if (!CLIENT_MESSAGES_LOGGING) return;
		VS_TransportClient_Endpoint   *p = headEndpoint;
		if (!p) return;
		p->LogMessageToFile( mess );
	}

	static inline void ProcessingMessages( VS_ClientMessage &mess )
	{
		LogMessage(mess);
		if (mess.isOurMessage) {
			VS_TransportClient_Endpoint *p(0);
			char gate[256] = {0};
			if(!mess.dropGate)
				VS_GetTransportClientDefaultGate(gate, 256);

			const char *to_sid = mess.DstServer();
			const char *to_uid = mess.DstUser();
			const char *dst = *gate ? gate : (to_sid && *to_sid ? to_sid : (to_uid && *to_uid ? to_uid : 0));
			if (dst && ((p = GetEndpoint(dst)) || (p = AddEndpoint(dst, mess.TimeLimit())))) {
				p->AddMsg(mess); return;
			}
			((transport::MessageFixedPart*)mess.mess)->notify = 1;
		}
		VS_TransportClient_Service::ProcessingMessages( mess );
	}

	inline bool SetEvent( const HANDLE event, const unsigned type )
	{
		if (nhs >= MAXIMUM_WAIT_OBJECTS)	return false;
		hs[nhs] = event;	tes[nhs] = this;	tts[nhs++] = type;		return true;
	}

	inline bool SetBothWriteEvent()
	{
		return SetEvent( (const HANDLE)bothConn->OvWriteEvent(), VS_TC_BOTH_WRITE);
	}

	inline bool SetBothReadEvent()
	{
		return SetEvent( (const HANDLE)bothConn->OvReadEvent(), VS_TC_BOTH_READ);
	}

	inline bool SetEvents( void )
	{
		if(bothConn)
		{
			if(!bothConn->IsWrite())
			{
				if(nMsgs)
				{
					if(isFirstConnect)
						isFirstConnect = false;

					unsigned char *msg = (unsigned char *)headMsg->msg;
					uint32_t sz = headMsg->msg->head_length + headMsg->msg->body_length + 1;
					if(pWriteCrypt)
					{
						if(sndEncrBufSize)
						{
							sndEncrBufSize = 0;
							delete [] sndEncrBuf;
							sndEncrBuf = 0;
						}
						pWriteCrypt->Encrypt((const unsigned char *)msg,sz,0,&sndEncrBufSize);
						sndEncrBuf = new unsigned char[sndEncrBufSize];
						if(!pWriteCrypt->Encrypt((const unsigned char *)msg,sz,sndEncrBuf,&sndEncrBufSize))
						{
							if(sndEncrBufSize)
							{
								sndEncrBufSize = 0;
								delete [] sndEncrBuf;
								sndEncrBuf = 0;
							}
							--nhs;
							CloseBothConnStartThread();
							return true;
						}
						else if(!bothConn->Write( (const void *)sndEncrBuf,sndEncrBufSize))
						{
							if(sndEncrBufSize)
							{
								sndEncrBufSize = 0;
								delete [] sndEncrBuf;
								sndEncrBuf = 0;
							}
						}
						else
						{
							headMsg->msg = 0;
							writeBuffer = msg;
							writeSize = sz;
							DeleteMsg(headMsg);
						}
					}
					else if(!oldSecureAbility)
					{
						if(!bothConn->Write( (const void *)msg,sz))
						{
							--nhs;
							CloseBothConnStartThread();
							return true;
						}
						else
						{
							headMsg->msg = 0;
							writeBuffer = msg;
							writeSize = sz;
							DeleteMsg( headMsg );
						}
					}
					if(!SetBothWriteEvent())
						return false;
				}
			}
			else if(!SetBothWriteEvent())
				return false;
			if(!bothConn->IsRead())
			{
				stateRcv = 0;
				if(readBuffer)
				{
					free( (void*) readBuffer );
					readBuffer = 0;
					readSize = 0;
				}
				if(pReadCrypt)
				{
					if(rcvEncrBufSize)
					{
						rcvEncrBufSize = 0;
						delete [] rcvEncrBuf;
						rcvEncrBuf = 0;
					}
					pReadCrypt->Encrypt((const unsigned char*)&readHead, sizeof(transport::MessageFixedPart), 0, &rcvEncrBufSize);
					if (rcvEncrBufSize != sizeof(transport::MessageFixedPart)) // cipher in block mode
						rcvEncrBufSize--;
					rcvEncrBuf = new unsigned char[rcvEncrBufSize];
					if(!bothConn->Read((void*) rcvEncrBuf, rcvEncrBufSize))
					{
						if(rcvEncrBufSize)
						{
							rcvEncrBufSize = 0;
							delete [] rcvEncrBuf;
							rcvEncrBuf = 0;
						}
						CloseBothConnStartThread();
						return true;
					}
				}
				else
				{
					if(!bothConn->Read((void *)&readHead, sizeof(transport::MessageFixedPart)))
					{
						CloseBothConnStartThread();
						return true;
					}
				}
			}
			if(!SetBothReadEvent())
				return false;
		}
		return true;
	}

	inline void ProcessingBothWriteEvent()
	{
		unsigned long	mills = 0;
		lastWriteTm = currTm;
		int	retWrite = bothConn->GetWriteResult(mills);

		if(retWrite == -2)
			return;
		else if(((unsigned long)retWrite != writeSize) && ((unsigned long) retWrite != sndEncrBufSize))
		{
			AddMsgToHead(writeBuffer);
			writeBuffer = 0;
			m_Logger->TPrintf("retWrite != writeSize; Run CloseSndConnStartThread()");
			CloseBothConnStartThread();
			return;
		}
		if(writeBuffer)
		{
			free( (void *) writeBuffer );
			writeBuffer = 0;
		}
		if(sndEncrBuf)
		{
			delete [] sndEncrBuf;
			sndEncrBuf = 0;
		}
		sndEncrBufSize = 0;
		writeSize = 0;
	}

	inline void	ProcessingBothReadEvent()
	{
		lastReadTm = currTm;
		while (1)
		{
			unsigned long	mills = 0;
			int ret = bothConn->GetReadResult( mills );
			if(ret == -2)
				return;
			if(ret == -1)
			{
				printf("\n\tr");
				CloseBothConnStartThread();
				return;
			}
			switch (stateRcv)
			{
			case 0:
				if(pReadCrypt)
				{
					unsigned char *decrdata = new unsigned char[rcvEncrBufSize];
					uint32_t headsize = rcvEncrBufSize;
					if((ret != rcvEncrBufSize) ||
						(!pReadCrypt->Decrypt(rcvEncrBuf,rcvEncrBufSize,decrdata,&headsize))||
						(headsize != sizeof(transport::MessageFixedPart)))
					{
						delete [] rcvEncrBuf;
						rcvEncrBuf = 0;
						rcvEncrBufSize = 0;
						delete [] decrdata;
						CloseBothConnStartThread();
						return;
					}
					memcpy(&readHead, decrdata,headsize);
					delete [] decrdata;
					delete [] rcvEncrBuf;
					rcvEncrBuf = 0;
					rcvEncrBufSize = 0;
					ret = headsize;
				}
				if(ret != sizeof(transport::MessageFixedPart) || !readHead.ms_life_count
					|| readHead.version < 1
					|| readHead.head_length < sizeof(transport::MessageFixedPart) + 6)
				{
					CloseBothConnStartThread();
					return;
				}
				readSize = readHead.head_length + readHead.body_length + 1;
				readBuffer = (unsigned char *)malloc( (size_t)readSize );
				if (!readBuffer)
				{
					CloseBothConnStartThread();
					return;
				}
				*(transport::MessageFixedPart*)readBuffer = readHead;
				readSize -= sizeof(transport::MessageFixedPart);
				stateRcv = 1;
				if(pReadCrypt)
				{
					rcvEncrBufSize = 0;
					pReadCrypt->Encrypt(readBuffer, readSize,0,&rcvEncrBufSize);
					rcvEncrBufSize -= pReadCrypt->GetBlockSize() - 1;
					rcvEncrBuf = new unsigned char[rcvEncrBufSize];
					if(!bothConn->Read((void*)rcvEncrBuf,rcvEncrBufSize))
					{
						delete [] rcvEncrBuf;
						rcvEncrBuf = 0;
						rcvEncrBufSize = 0;
						CloseBothConnStartThread();
						return;
					}
				}
				else
				{
					if(!bothConn->Read((void*)&readBuffer[sizeof(transport::MessageFixedPart)], readSize))
					{
						CloseBothConnStartThread();
						return;
					}
				}
				break;
			case 1:
				{
					if(pReadCrypt)
					{
						uint32_t decrsize = readSize + pReadCrypt->GetBlockSize();
						unsigned char * decrdata = new unsigned char[decrsize];
						if((rcvEncrBufSize != ret) ||
							(!pReadCrypt->Decrypt(rcvEncrBuf, rcvEncrBufSize,decrdata,&decrsize)) ||
							(decrsize != readSize))
						{
							delete [] rcvEncrBuf;
							rcvEncrBuf = 0;
							rcvEncrBufSize = 0;
							delete [] decrdata;
							CloseBothConnStartThread();
							return;
						}
						memcpy(&readBuffer[sizeof(transport::MessageFixedPart)], decrdata, decrsize);
						delete [] decrdata;
						delete [] rcvEncrBuf;
						rcvEncrBuf = 0;
						rcvEncrBufSize = 0;
						ret = readSize;
					}
					if((unsigned long)ret != readSize)
					{
						CloseBothConnStartThread();
						return;
					}
					VS_ClientMessage	mess;
					mess.VS_TransportMessage::Set( readBuffer );
					readBuffer = 0;
					readSize = 0;
					stateRcv = 0;
					if(!mess.IsValid())
					{
						CloseBothConnStartThread();
						return;
					}
					ProcessingMessages( mess );
				}
				if(pReadCrypt)
				{
					if(rcvEncrBufSize)
					{
						rcvEncrBufSize = 0;
						delete [] rcvEncrBuf;
						rcvEncrBuf = 0;
					}
					pReadCrypt->Encrypt((const unsigned char *)&readHead, sizeof(transport::MessageFixedPart), 0, &rcvEncrBufSize);
					if (rcvEncrBufSize != sizeof(transport::MessageFixedPart)) // cipher in block mode
						rcvEncrBufSize--;
					rcvEncrBuf = new unsigned char[rcvEncrBufSize];
					if(!bothConn->Read((void*)rcvEncrBuf,rcvEncrBufSize))
					{
						if(rcvEncrBufSize)
						{
							rcvEncrBufSize = 0;
							delete [] rcvEncrBuf;
							rcvEncrBuf = 0;
						}
						CloseBothConnStartThread();
						return;
					}
				}
				else
				{
					if(!bothConn->Read((void*)&readHead, sizeof(transport::MessageFixedPart)))
					{
						CloseBothConnStartThread();
						return;
					}
				}
				break;
			default:
				CloseBothConnStartThread();
				return;
			}
		}
	}

	inline void ProcessingEvents( unsigned type )
	{
		lastTickTm = currTm;
		switch (type)
		{
		case VS_TC_BOTH_READ:	ProcessingBothReadEvent();	return;
		case VS_TC_BOTH_WRITE:	ProcessingBothWriteEvent();	return;
		}
	}


	//////////////////////////////////////////////////////////////////////////////////////
	static inline VS_TransportClient_Endpoint *GetEndpoint( const char *endpoint )
	{
		VS_TransportClient_Endpoint   *p = headEndpoint;
		for (unsigned i = 0; i < nEndpoint && p; ++i, p = p->nextEndpoint)
			if (!strcmp( endpoint, p->cid))	return p;
		return 0;
	}

	static inline VS_TransportClient_Endpoint *AddEndpoint( const char *endpoint, const unsigned long connectTimeout )
	{
		if (!((endpoint) && (*endpoint))) return 0;
		VS_TransportClient_Endpoint   *p = GetEndpoint( endpoint );
		if (p)	return p;
		{
			if (net::endpoint::GetCountConnectTCP(endpoint, true) == 0)
				return 0;
		}
		if (nEndpoint < VS_TRANSPORT_MAX_NUMBER_OF_ENDPOINTS)
		{
go_new:		p = new VS_TransportClient_Endpoint( endpoint );
			p->maxConnectMs = (connectTimeout > VS_TRANSPORT_CLIENT_CONNECT_MS) || !connectTimeout ? VS_TRANSPORT_CLIENT_CONNECT_MS : connectTimeout;
			if (p)
			{
				if (!p->isValid) {		delete p;	p = 0;		}
				else
				{
					if (!nEndpoint)
					{	headEndpoint = endEndpoint = p;
					p->previousEndpoint = p->nextEndpoint = 0;	}
					else
					{	endEndpoint->nextEndpoint = p;	p->previousEndpoint = endEndpoint;
					endEndpoint = p;	p->nextEndpoint = 0;	}
					++nEndpoint;
				}	}	}
		else
		{
			VS_TransportClient_Endpoint   *p = headEndpoint, *pret = 0;
			unsigned long   diffTickMs, maxTickMs = 0;
			for (unsigned i = 0; i < nEndpoint && p; ++i, p = p->nextEndpoint)
			{
				diffTickMs = currTm - p->lastTickTm;
				if (diffTickMs > maxTickMs) {	maxTickMs = diffTickMs;		pret = p;	}
			}
			if (pret)
			{
				DeleteEndpoint( pret );
				if (nEndpoint >= VS_TRANSPORT_MAX_NUMBER_OF_ENDPOINTS)		return 0;
				goto go_new;
			}	}
		return p;
	}

	static inline void DeleteEndpoint( VS_TransportClient_Endpoint *p , bool aIsNoNotify=false )
	{
		if (aIsNoNotify)
			p->isNoNotify = aIsNoNotify;
		p->DeleteAllMessages();
		p->StopStartedConnections();

		if (nEndpoint == 1)		;
		else if (p == headEndpoint)
		{
			headEndpoint = p->nextEndpoint;		headEndpoint->previousEndpoint = 0;		}
		else if (p == endEndpoint)
		{	endEndpoint = p->previousEndpoint;	endEndpoint->nextEndpoint = 0;	}
		else
		{	p->nextEndpoint->previousEndpoint = p->previousEndpoint;
		p->previousEndpoint->nextEndpoint = p->nextEndpoint;	}
		if (!--nEndpoint)	headEndpoint = endEndpoint = 0;
		if(p->IsConnectProcessing())
			EndpointsWaitingFree[p] = 1;
		else
		{
			delete p;
			EndpointsWaitingFree.erase(p);
		}
	}

	static inline void DeleteEndpoint( const char *endpoint , bool aIsNoNotify=false)
	{
		VS_TransportClient_Endpoint   *p = GetEndpoint( endpoint );
		if (p)
			DeleteEndpoint( p , aIsNoNotify );
	}

	static inline void DeleteAllEndpoints( void )
	{
		VS_TransportClient_Endpoint   *p = headEndpoint, *p_next;
		for (unsigned i = 0; i < VS_TRANSPORT_MAX_NUMBER_OF_ENDPOINTS && p; ++i, p = p_next)
		{
			p_next = p->nextEndpoint;
			DeleteEndpoint( p , true );
		}
		headEndpoint = endEndpoint = 0;		nEndpoint = 0;
	}


	//////////////////////////////////////////////////////////////////////////////////////
	static inline void SetTransportConnection( void )
	{
		VS_ConnectionSock   *conn = argUnion.setTransportConnection.conn;
		const char   *server_id = argUnion.setTransportConnection.server_id;
		const char   *cid = argUnion.setTransportConnection.cid;
		const unsigned long   version = argUnion.setTransportConnection.version;
		const bool   isAccept = argUnion.setTransportConnection.isAccept;
		const unsigned short   maxConnSilenceMs = argUnion.setTransportConnection.maxConnSilenceMs;
		const unsigned char   fatalSilenceCoef = argUnion.setTransportConnection.fatalSilenceCoef;
		const bool	bIsSSLSupport = argUnion.setTransportConnection.isSSLSupport;
		const unsigned long	problem = argUnion.setTransportConnection.problem;
		bool tcpKeepAliveSupport = argUnion.setTransportConnection.tcpKeepAliveSupport;

		if (!server_id || !*server_id ) {
			/* In these places it will be necessary to throw off in TRACE */
			return;
		}
		if (cid) {
			VS_SetCID( cid, server_id );
		}
		VS_TransportClient_Endpoint   *p = GetEndpoint( server_id );
		if (!p) {
			if (!conn) {
				// сюда по идее попасть нельзя
				if (epcb) epcb->OnConnect(server_id,err_inaccessible);
				return;
			}
			p = AddEndpoint( server_id, 0 );
			if (!p) {
				delete conn;
				if (epcb) epcb->OnConnect(server_id,err_inaccessible);
				return;
			}
		}
		else {
			if(p->stopStartedConns) {
				if(conn)
					delete conn;
				DeleteEndpoint(p);
				if (epcb) epcb->OnConnect(server_id,p->isFirstConnect?err_inaccessible:err_disconnect);
				return;
			}


#ifndef VS_SSL_SUPPORT
			p->oldSecureAbility = false;
#endif
			if (!conn) {
				if (epcb) epcb->OnConnect(server_id,p->isFirstConnect?err_inaccessible:err_disconnect);
				if (problem == 1) {
					p->StartConnectThreads();
				}
				else {
					DeleteEndpoint( p );
				}
				return;
			}
		}
		const VS_TlsContext* context = conn ? conn->GetTlsContext() : nullptr;
		if (/* is tls connection */context)
		{
			p->newSecureAbility = true;
			std::string commonName;
//			the first condition should always be false, since if cert check fails,
//			client won't establish TLS connection
			if (/* cert check failed */context->certCheckStatus != VS_TlsContext::ccs_success ||
			    /* server name doesn't exist */!context->cert.GetSubjectEntry("commonName", commonName) ||
			    /* server name is wrong */commonName != server_id
			   )
			{
				delete conn;
				conn = 0;
				DeleteEndpoint(p);
				return;
			}
			p->serverCertificate = context->cert;
		} else
			p->newSecureAbility = false;
		if((VS_SSL_SUPPORT_BITMASK&version)&&(bIsSSLSupport))
			p->oldSecureAbility = true;
		else
		{
			p->oldSecureAbility = false;
			/**
			SSL не поддерживается, задать ошибку, отключиться, если не TLS-подключение
			*/
// TODO: remove #ifndef VZOCHAT7 after VZOchat SSL support finished
#ifndef VZOCHAT7
			if (!p->newSecureAbility)
			{
				if(epcb) epcb->OnConnect(server_id,err_ssl_could_not_established);
				if(conn)
				{
					delete conn;
					conn = 0;
				}
				DeleteEndpoint( p );
				return;
			}
#endif
		}
		if (server_id)
			strcpy(p->server_id,server_id);
		p->lastTickTm = currTm;
		if(p->bothConn) delete p->bothConn;
		p->bothConn = conn;
		p->m_IsTCPPingSupported = tcpKeepAliveSupport;
		bool local_tcpKeepAliveSupport = p->bothConn->SetKeepAliveMode(true, 20000, 2000);
		p->lastWriteTm = currTm;
		p->lastReadTm = currTm;
		p->maxSndSilenceMs = maxConnSilenceMs;
		p->maxSndSilenceCoefMs = maxConnSilenceMs * fatalSilenceCoef;
		p->maxRcvSilenceMs = maxConnSilenceMs;
		p->maxRcvSilenceCoefMs = maxConnSilenceMs * fatalSilenceCoef;

		if (isAccept) {
			/*
			char *user_id  = "user";

			net::HandshakeHeader *hs = VS_FormTransportReplyHandshake( 0, 0,
			(const unsigned short)p->maxConnSilenceMs,
			(const unsigned char)p->fatalSilenceCoef,
			0, server_id, user_id);
			*/
			net::HandshakeHeader *hs = VS_FormTransportReplyHandshake( 0, 0,
				(const unsigned short)p->maxConnSilenceMs,
				(const unsigned char)p->fatalSilenceCoef,
				0, server_id, false, local_tcpKeepAliveSupport );
			if (!hs) {
				p->bothConn = 0;
			}
			else {
				unsigned long size = sizeof(net::HandshakeHeader) + hs->body_length + 1;
				VS_Buffer   buffer = { size, (void *)hs };
				if (!conn->RWrite( &buffer, 1 )) {
					p->bothConn = 0;
				}
				else {
					p->writeSize = size;
					conn = 0;
				}
			}
			if (conn)
				delete conn;
			if (hs)
				free( (void *)hs );
		}
		else {
			if (p->oldSecureAbility) {
				SSLReconnectsLogger->TPrintf("Both SSL Connect. Endpoint = %s", server_id);
				if(!p->BothClientSecureHandshake())
					DeleteEndpoint(p);
				else
					if (epcb) epcb->OnConnect(server_id,err_ok);
			}
			else {
				if (epcb) epcb->OnConnect(server_id,err_ok);
				SSLReconnectsLogger->TPrintf("EndPoint = %s; Connect without SSL",server_id);
			}
		}
	}

	static inline void FullDisconnection( void )
	{
		const char * endpoint = argUnion.transportConnection.endpoint;

		VS_TransportClient_Endpoint   *p = GetEndpoint( endpoint );
		if (p)
		{
			p->DisconnectMe();
			return;
		}
	}
	static inline void CloseTransportConnection( void )
	{	DeleteEndpoint( argUnion.transportConnection.endpoint , true );	}


	static inline void GetEndpointIP( void )
	{
		const char * endpoint = argUnion.transportConnection.endpoint;
		VS_TransportClient_Endpoint   *p = GetEndpoint( endpoint );
		memset(vs_last_source_ip, 0, ip_len);
		if ((p) && (p->bothConn))
		{
			const char* ch = p->bothConn->GetBindIp();
			if (!ch || !*ch || !strcmp(ch, "127.0.0.1")) {
				char host[MAX_PATH+1] = {0};
				if (VS_GetDefaultHostName(host, MAX_PATH) && VS_GetHostByName(host, vs_last_source_ip, ip_len - 1)) {
					return;
				} else {
					strncpy(vs_last_source_ip, "127.0.0.1", ip_len - 1);
				}
			} else {
				strncpy(vs_last_source_ip, ch, ip_len - 1);
			}
		}
		return;
	}

	static inline bool EndpointConnect(const char *to_name, int timeout)
	{
		VS_ClientMessage mess;
		const char opcode[] = { VS_TRANSPORT_MANAGING_CONNECT, '\0' };
		mess.VS_TransportMessage::Set(1, ~0, 0, 0, (const char *)&opcode, 0, 0, timeout, "", 1, 0, 0, to_name, 0);
		mess.Send();
		return true;
	}

	inline void DisconnectMe()
	{
		VS_ClientMessage   mess;
		const char opcode[] = { VS_TRANSPORT_MANAGING_DISCONNECT, '\0' };
		mess.VS_TransportMessage::Set(1, ~0, 0, 0, (const char *)&opcode, 0, 0, VS_TRANSPORT_TIMELIFE_DISCONNECT, "", 1, 0, 0, server_id, 0);
		AddMsg( mess );
	}

	inline bool ProcessingEndpointTick( void )
	{
		if (!bothConn)
		{
			if (( tickTm - lackTm ) > maxLackMs)
				return false;
		}
		lackTm = tickTm;
		Tr_Msg   *current = headMsg, *next = 0;
		for (unsigned long i = 0; i < nMsgs && current; ++i, current = next)
		{
			next = current->nextMsg;
			if (current->msg->ms_life_count < tickDiffTm)
				DeleteMsg( current );
			else
				current->msg->ms_life_count -= tickDiffTm;
		}
		if (bothConn && !m_IsTCPPingSupported)
		{
			const unsigned long   diffWriteTm = currTm - lastWriteTm;
			if (diffWriteTm > maxSndSilenceCoefMs)
			{
				printf("x");
				CloseBothConnStartThread();
				return true;
			}
			else if (!nMsgs && !bothConn->IsWrite() && diffWriteTm > maxSndSilenceMs)
			{
				printf("*");
				VS_ClientMessage   mess;
				const char opcode[] = { VS_TRANSPORT_MANAGING_PING, '\0' };
				mess.VS_TransportMessage::Set( 1, ~0, 0, 0, (const char *)&opcode, 0, 0, VS_TRANSPORT_TIMELIFE_PING, "", 1, 0, 0, server_id ,0 );
				if (mess.IsRequest())
					AddMsg( mess );
			}
			const unsigned long   diffReadTm = currTm - lastReadTm;
			if (diffReadTm > maxRcvSilenceCoefMs )
				CloseBothConnStartThread();
		}
		return true;
	}


	static inline void ProcessingTick( void )
	{
		VS_TransportClient_Endpoint   *p = headEndpoint, *pNext = 0;
		for (unsigned i = 0; i < nEndpoint && p; ++i, p = pNext)
		{
			pNext = p->nextEndpoint;
			if (!p->ProcessingEndpointTick())
				DeleteEndpoint( p );
		}
		std::map<VS_TransportClient_Endpoint*,int>::iterator	iter = EndpointsWaitingFree.begin();
		while(iter!= EndpointsWaitingFree.end())
		{
			VS_TransportClient_Endpoint	*ep_for_del = iter->first;
			if(!ep_for_del->IsConnectProcessing())
			{
				delete ep_for_del;
				iter = EndpointsWaitingFree.erase(iter);
			}
			else
				iter++;

		}
	}

	static inline void ProcessingMessages( void )
	{
		MSG   msg;
		memset( (void *)&msg, 0, sizeof(msg) );
		VS_ClientMessage   mess;
		++ mess.isOurMessage;
		while (PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
		{
			if (msg.message == VS_WM_TRANSPORT_CLIENT_MESSAGE || msg.message == VS_WM_TRANSPORT_CLIENT_MESSAGE_WITHOUT_GATE)
			{
				if(msg.message == VS_WM_TRANSPORT_CLIENT_MESSAGE_WITHOUT_GATE)
					mess.dropGate = true;
				else
					mess.dropGate = false;
				if (!mess.Set( &msg ))
				{	/* In these places it will be necessary to throw off in TRACE */
					mess.mess = 0;
				}
				else
					ProcessingMessages( mess );
				mess.Reset();
			}
			memset( (void *)&msg, 0, sizeof(msg) );
		}
	}

	static inline void SetForMultipleEvents( void )
	{
		nhs = 1;
		VS_TransportClient_Endpoint   *p = headEndpoint;
		for (unsigned i = 0; i < nEndpoint && p; ++i, p = p->nextEndpoint)
			if (!p->SetEvents())	break;
	}

	static inline void ProcessingEvents( DWORD nEvent )
	{
		tes[nEvent]->ProcessingEvents( tts[nEvent] );
	}

	VS_TransportClient_Endpoint			*previousEndpoint, *nextEndpoint;
	static VS_TransportClient_Endpoint	*headEndpoint, *endEndpoint;
	static unsigned						nEndpoint;
};



//bool VS_TransportClient_Endpoint::SSLIsTurnOn = true;
VS_TransportClient_Endpoint   *VS_TransportClient_Endpoint::headEndpoint = 0,
*VS_TransportClient_Endpoint::endEndpoint = 0;
unsigned   VS_TransportClient_Endpoint::nEndpoint = 0;


bool VS_CreateConnect(const char *to_name, int timeout)
{
	return VS_TransportClient_Endpoint::EndpointConnect(to_name, timeout);
}


static inline void AcceptHandler(const void *cb_arg, VS_ConnectionTCP *conn, net::HandshakeHeader *hs)
{	VS_TransportClient_Endpoint::AcceptHandler( cb_arg, conn, hs );		}


//////////////////////////////////////////////////////////////////////////////////////////
static inline void ProcessingManaging( VS_ClientMessage &mess )
{
	if (!mess.IsValid())
		return;
	if (mess.IsNotify() || !mess.IsRequest())
		return;
	const char   *endpoint = mess.SrcCID();
	if (!endpoint || !*endpoint)
		return;
	VS_TransportClient_Endpoint   *ep = VS_TransportClient_Endpoint::GetEndpoint( endpoint );
	if (!ep || !ep->bothConn)
		return;
	const char *type = mess.AddString();
	if (!type || !*type)
		return;

	switch (type[0])
	{
	case VS_TRANSPORT_MANAGING_PING :
		break;

	case VS_TRANSPORT_MANAGING_CONNECT : break;
	case VS_TRANSPORT_MANAGING_DISCONNECT :
		{
			VS_TransportClient_Service   *serv = VS_TransportClient_Service::headServ;
			for (unsigned i = 0; i < VS_TransportClient_Service::nSers && serv; ++i, serv = serv->nextServ)
			{
				if (serv->threadId)
				{
					PostThreadMessage(serv->threadId,WM_APP+100,0,0);
					printf("\n Recv disconnect!");
					return;
				}
			}
		} break;
	}
}



//////////////////////////////////////////////////////////////////////////////////////////
static bool   continueTransport;

static inline void ProcessingCommands( void )
{
	bool   ret = false;
	switch (argUnion.type)
	{
	case VS_TC_TYPE_UNINSTALL :	// Here it will be necessary to throw off in TRACE
		continueTransport = false;										break;
	case VS_TC_TYPE_SET_TRANSPORT_CONNECTION :
		VS_TransportClient_Endpoint::SetTransportConnection();			break;
	case VS_TC_TYPE_FULL_DISCONNECTION :
		///Так надо, Сначало посыл сообщения, потом отключение транспорта.
		VS_TransportClient_Endpoint::FullDisconnection();				break;
	case VS_TC_TYPE_CLOSE_TRANSPORT_CONNECTION:
		VS_TransportClient_Endpoint::CloseTransportConnection();		break;
	case VS_TC_TYPE_REGISTER_SERVICE_THREAD_ID :
		VS_TransportClient_Service::RegisterTreadId();					break;
	case VS_TC_TYPE_REGISTER_SERVICE_HWND :
		VS_TransportClient_Service::RegisterHWnd();						break;
	case VS_TC_TYPE_IS_SERVICE_EXISTS :
		VS_TransportClient_Service::IsServiceExists();					break;
	case VS_TC_TYPE_UNREGISTER_SERVICE :
		VS_TransportClient_Service::UnregisterService();				break;
	case VS_TC_TYPE_GET_IP :
		VS_TransportClient_Endpoint::GetEndpointIP();					break;
	}
	SetEvent( hEventTransport );
}

unsigned __stdcall TransportClientThread( void *arg )
{
	vs::SetThreadName("TransportClient");
	MSG   msg = { 0 };
	PeekMessage( &msg, NULL, WM_USER, WM_USER, PM_NOREMOVE );
	SetEvent( hEventTransport );
	hs[0] = hEventThread;
	nhs = 1;
	DWORD   ret, mills = clientTickSizeMs, old_tick = GetTickCount(), tmp_tick;
	continueTransport = true;
	while (continueTransport)
	{
		ret = MsgWaitForMultipleObjects( nhs, hs, FALSE, mills, QS_ALLPOSTMESSAGE );
		if (ret == WAIT_OBJECT_0)
			ProcessingCommands();
		else if (ret < ( WAIT_OBJECT_0 + nhs ))
			VS_TransportClient_Endpoint::ProcessingEvents( ret - WAIT_OBJECT_0 );
		else if (ret == ( WAIT_OBJECT_0 + nhs ))
			VS_TransportClient_Endpoint::ProcessingMessages();
		else if (ret != WAIT_TIMEOUT)
		{	/* Here it will be necessary to throw off in TRACE */	}
		tmp_tick = GetTickCount();
		currDiffTm = tmp_tick - old_tick;
		currTm += currDiffTm;
		sumCurrDiffTm += currDiffTm;
		if (currDiffTm >= mills)
		{
			tickDiffTm = sumCurrDiffTm;
			sumCurrDiffTm = 0;
			tickTm += tickDiffTm;
			mills = clientTickSizeMs;
			VS_TransportClient_Endpoint::ProcessingTick();
		}
		else
			mills -= currDiffTm;
		old_tick = tmp_tick;
		VS_TransportClient_Endpoint::SetForMultipleEvents();
	}
	_endthreadex( 0 );		return 0;
}


inline void ResetStaticVars( void )
{
	VS_TransportClient_Endpoint::DeleteAllEndpoints();
	VS_TransportClient_Service::DeleteAllServices();
	if (hEventThread) {		CloseHandle( hEventThread );	hEventThread = 0;	}
	if (hEventTransport) {		CloseHandle( hEventTransport );	hEventTransport = 0;	}
	if (tAcsLog) {		delete tAcsLog;	tAcsLog = 0;	}
	hThread = 0;	idThread = 0;
	defGateEndpoint[0] = 0;
	VS_ClearCID();
}



//////////////////////////////////////////////////////////////////////////////////////////




bool VS_GetCertificateData(VS_WideStr &serverEP,
						   VS_WideStr &country,
						   VS_WideStr &organization,
						   VS_WideStr &contact_person,
						   VS_WideStr &email,
						   VS_SimpleStr &notBefore,
						   VS_SimpleStr &notAfter)
{
	VS_TransportClient_Endpoint	*p(0);
	char defaultGate[256] = {0};
	VS_GetTransportClientDefaultGate(defaultGate, 256);
	if (*defaultGate) {
		p = VS_TransportClient_Endpoint::GetEndpoint(defaultGate);
		if (p)
			return p->GetCertificateData(serverEP,country,organization,contact_person, email, notBefore, notAfter);
	}
	return false;
}

#endif
