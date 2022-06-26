/**
 **************************************************************************
 * \file VS_ConnectionManager.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Static pie of TCP-UDP-HTTP/IP communication funtions.
 *
 * Also there is work with regestry that connected with connections of clients.
 *
 * \b Project ManagerOfConnections
 * \author SlavetskyA
 * \date 08.10.02
 *
 * $Revision: 7 $
 *
 * $History: VS_ConnectionManager.cpp $
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 7.08.12    Time: 21:42
 * Updated in $/VSNA/acs/ConnectionManager
 * - static var isDisableNagle  removed
 *
 * *****************  Version 6  *****************
 * User: Dront78      Date: 6.07.10    Time: 17:22
 * Updated in $/VSNA/acs/ConnectionManager
 * - qos fixed
 *
 * *****************  Version 5  *****************
 * User: Dront78      Date: 9.02.09    Time: 19:43
 * Updated in $/VSNA/acs/connectionmanager
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 24.06.08   Time: 18:40
 * Updated in $/VSNA/acs/connectionmanager
 * - IE7 GetInternetOptions compability added
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 18.06.08   Time: 17:00
 * Updated in $/VSNA/acs/connectionmanager
 * - VS_MemoryLeak included
 * - Logging to smtp service added
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:01
 * Updated in $/VSNA/acs/connectionmanager
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 23.10.07   Time: 16:55
 * Updated in $/VS2005/acs/connectionmanager
 * ConnectTCP is ordered without delete bad conns (CheckSrv)
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 22.10.07   Time: 19:46
 * Updated in $/VS2005/acs/connectionmanager
 *  - long time connection fixed
 *  - clearing handle added to VS_ConnectionTypes.h
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 18.10.07   Time: 18:43
 * Updated in $/VS2005/acs/connectionmanager
 * checksrv big timeout fixed
 * checksrv wait infinite fixed
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 10.10.07   Time: 19:27
 * Updated in $/VS2005/acs/connectionmanager
 * Checking server throu proxy added through proxy
 *
 * *****************  Version 7  *****************
 * User: Dront78      Date: 14.09.07   Time: 17:16
 * Updated in $/VS2005/acs/connectionmanager
 * fixed memory and threads handle leaks int testaccessibility
 *
 * *****************  Version 6  *****************
 * User: Dront78      Date: 12.09.07   Time: 18:20
 * Updated in $/VS2005/acs/connectionmanager
 * fixed 14 second testaccessibility timeout
 *
 * *****************  Version 5  *****************
 * User: Dront78      Date: 11.09.07   Time: 17:43
 * Updated in $/VS2005/acs/connectionmanager
 * Temporary fix crash when deleting VS_Test_Accessibility_Thread_Arg.
 * Memory leak 44 bytes.
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 26.07.07   Time: 14:31
 * Updated in $/VS2005/acs/connectionmanager
 * TestAccessibility support proxy added
 *
 * *****************  Version 3  *****************
 * User: Dront78      Date: 20.03.07   Time: 19:44
 * Updated in $/VS2005/acs/connectionmanager
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 5.02.07    Time: 21:30
 * Updated in $/VS2005/acs/connectionmanager
 * - project configuration
 * - depricated functions warnings suppressed
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/acs/connectionmanager
 *
 * *****************  Version 69  *****************
 * User: Avlaskin     Date: 26.06.06   Time: 13:26
 * Updated in $/VS/acs/ConnectionManager
 * Rolled back for release!
 *
 * *****************  Version 67  *****************
 * User: Mushakov     Date: 19.05.06   Time: 12:42
 * Updated in $/VS/acs/ConnectionManager
 *
 * *****************  Version 66  *****************
 * User: Avlaskin     Date: 24.03.06   Time: 17:19
 * Updated in $/VS/acs/ConnectionManager
 *
 * *****************  Version 65  *****************
 * User: Mushakov     Date: 23.03.06   Time: 20:00
 * Updated in $/VS/acs/ConnectionManager
 *
 * *****************  Version 64  *****************
 * User: Mushakov     Date: 13.01.06   Time: 16:42
 * Updated in $/VS/acs/ConnectionManager
 * Added SecureLib, SecureHandshake
 *
 * *****************  Version 63  *****************
 * User: Avlaskin     Date: 17.12.04   Time: 16:58
 * Updated in $/VS/acs/ConnectionManager
 *
 ****************************************************************************/

//#define   _MY_DEBUG_

#include <future>
#include <limits.h>
#include <process.h>
#include <stdio.h>
#include <Winsock2.h>
#include <Wininet.h>
#include <In6addr.h>

#include "VS_ConnectionManager.h"
#include "acs/VS_AcsDefinitions.h"
#include "acs/connection/VS_ConnectionTCP.h"
#include "acs/connection/VS_ConnectionTLS.h"
#include "acs/Lib/VS_AcsLib.h"
#include "acs/Lib/VS_AcsLog.h"
#include "net/Handshake.h"
#include "net/EndpointRegistry.h"
#include "std-generic/cpplib/move_handler.h"
#include "std/cpplib/event.h"
#include "std/cpplib/ThreadUtils.h"
#include "std-generic/cpplib/scope_exit.h"

#include <boost/algorithm/string/predicate.hpp>

const char VS_Ping_PrimaryField[net::HandshakeHeader::primary_field_size] = { '_','V','S','_','P','I','N','G','_',0 };

static inline VS_ConnectionSock *VS_Connect_TCPLike( const char *, const bool isDisableNagle, unsigned long &, unsigned *, VS_CreationAttempts *);
static inline VS_ConnectionSock* VS_Connect_TCPLike(net::endpoint::ConnectTCP& rc, const bool isDisableNagle, unsigned long& milliseconds);

static inline VS_ConnectionSock *VS_Connect_UDPLike( const char * );
static inline int VS_Exists_UDPLike( const char *endpoint );
static inline int VS_Open_Listens_TCPsLike( void );
static inline void VS_Get_Listens_TCPsLike(std::vector<std::unique_ptr<net::endpoint::ConnectTCP>>& connectTCP);
static inline int VS_Close_Listens_TCPsLike( void );
static inline int VS_Add_Handler_For_Listen_TCPsLike( const char *, const VS_AcceptHandlerCallback, const void *, const bool isDisableNagle );
static inline void VS_Test_Accessibility(
	const std::vector<std::unique_ptr<net::endpoint::ConnectTCP>>& rc,
	std::vector<net::endpoint::ConnectTCP*>& success_rc,
	const char* tested_endpoint,
	unsigned long& milliseconds
);
static inline int VS_Delete_Handler_For_Listen_TCPsLike( const char * );
static inline int VS_Delete_Handlers_For_Listen_TCPsLike( void );
static inline void VS_SetAuthentication( VS_Authentication * );
static inline void VS_AddAuthRequest( const char *, const char *, const unsigned short, const char *, const char *, const bool = false );
static inline void VS_GetAuthRequest( const char *, const char *, const unsigned short, char *, const unsigned, char *, const unsigned, bool * );
static inline void VS_DeleteAuthRequest( const char *, const char *, const unsigned short );
static inline void VS_ResetAuthentication( void );
static inline void VS_DeleteAuthentication( void );
static unsigned long VS_ConnectionManagerStart( void );
static void VS_Ping_Callback(const void*, VS_ConnectionTCP*, net::HandshakeHeader*);
#define   START_FLAG_SEQUENCE   0x5C5A5B5A
static CRITICAL_SECTION   sect, authSect;
static unsigned long   startFlag = VS_ConnectionManagerStart();
static bool   flagInstall = false, flagListensInstall = false, /*isDisableNagle = false,*/ m_qos = false;
static _QualityOfService *m_qos_params = NULL;
static char   globalNameEndpoint[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1] = { 0 };
static char   globalAliasNameEndpoint[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1] = { 0 };
VS_AcsLog   *tAcsLog = 0;
static VS_Authentication   *soleAuth = 0;

char   vs_str_all_current[] = VS_CM_STR_ALL_CURRENT,
		vs_str_all_current1[] = VS_CM_STR_ALL_CURRENT1,
		vs_str_all_current2[] = VS_CM_STR_ALL_CURRENT2,
		vs_str_all_current3[] = VS_CM_STR_ALL_CURRENT3;

bool VS_InstallConnectionManager( const char *our_endpoint )
{
	bool   ret = false;
	if (startFlag != START_FLAG_SEQUENCE )	return ret;
	EnterCriticalSection( &sect );
	if (flagInstall)
	{	if (VS_IsOurEndpointName( our_endpoint ))	ret = true;		}
	else
	{
		if (!tAcsLog)
		{	tAcsLog = new VS_AcsLog( our_endpoint );
			if (tAcsLog && !tAcsLog->IsValid())
			{	delete tAcsLog;		tAcsLog = 0;
		}	}
		if (tAcsLog)
		{	flagInstall = true;
			flagInstall = ret = VS_SetEndpointName( our_endpoint );
	}	}
	LeaveCriticalSection( &sect );
	return ret;
}
// end VS_InstallConnectionManager

void VS_UninstallConnectionManager( void )
{
	EnterCriticalSection( &sect );
	flagInstall = false;
	VS_Close_Listens_TCPsLike();
	VS_Delete_Handlers_For_Listen_TCPsLike();
	memset( (void *)globalNameEndpoint, 0, sizeof(globalNameEndpoint) );
	memset( (void *)globalAliasNameEndpoint, 0, sizeof(globalAliasNameEndpoint) );
	LeaveCriticalSection( &sect );
}
// end VS_UninstallConnectionManager

char *VS_EndpointName( char *our_endpoint, const unsigned size_endpoint, unsigned *necessary_size )
{
	char   *ret = 0;
	EnterCriticalSection( &sect );
	if (flagInstall)
	{
		const unsigned   sz = (unsigned)strlen(globalNameEndpoint) + 1;
		if (our_endpoint && sz <= size_endpoint)
		{	strcpy( our_endpoint, globalNameEndpoint );		ret = our_endpoint;		}
		else if (necessary_size)	*necessary_size = sz;
	}
	LeaveCriticalSection( &sect );
	return ret;
}
// end VS_EndpointName

bool VS_SetEndpointName( const char *new_endpoint )
{
	bool   ret = false;
	if (!new_endpoint || !*new_endpoint
			|| strlen( new_endpoint ) > VS_ACS_MAX_SIZE_ENDPOINT_NAME)
		return ret;
	EnterCriticalSection( &sect );
	if (flagInstall)
	{
		strcpy( globalAliasNameEndpoint, globalNameEndpoint );
		strcpy( globalNameEndpoint, new_endpoint );
		ret = true;
	}
	LeaveCriticalSection( &sect );
	return ret;
}
// end VS_SetEndpointName

bool VS_IsOurEndpointName( const char *endpoint )
{
	bool   ret = false;
	if (!endpoint || !*endpoint || strlen( endpoint ) > VS_ACS_MAX_SIZE_ENDPOINT_NAME)
		return ret;
	EnterCriticalSection( &sect );
	ret = flagInstall && (!strcmp( globalNameEndpoint, endpoint )
							|| !strcmp( globalAliasNameEndpoint, endpoint ));
	LeaveCriticalSection( &sect );
	return ret;
}
// end VS_SetEndpointName

bool VS_SetAuthenticationInterface( VS_Authentication *auth )
{
	if (!auth || startFlag != START_FLAG_SEQUENCE)	return false;
	EnterCriticalSection( &authSect );
	VS_SetAuthentication( auth );
	LeaveCriticalSection( &authSect );
	return true;
}
// end VS_SetAuthenticationInterface

void VS_ResetAuthenticationInformation( void )
{
	EnterCriticalSection( &authSect );
	VS_ResetAuthentication();
	LeaveCriticalSection( &authSect );
}
// end VS_ResetAuthenticationInformation

void VS_ResetAuthenticationInterface( void )
{
	EnterCriticalSection( &authSect );
	VS_DeleteAuthentication();
	LeaveCriticalSection( &authSect );
}
// end VS_ResetAuthenticationInterface

int VS_EstablishAcceptPorts( void )
{
	int   ret = -1;
	EnterCriticalSection( &sect );
	if (flagInstall)
	{	ret = VS_Open_Listens_TCPsLike();
		if (ret)	VS_Add_Handler_For_Listen_TCPsLike( VS_Ping_PrimaryField, VS_Ping_Callback, 0, false );
	}
	LeaveCriticalSection( &sect );
	return ret;
}
// end VS_EstablishAcceptPorts

void VS_GetEstablishedAcceptPorts(std::vector<std::unique_ptr<net::endpoint::ConnectTCP>>& connectTCP)
{
	EnterCriticalSection( &sect );
	if (flagInstall)
		VS_Get_Listens_TCPsLike(connectTCP);
	LeaveCriticalSection( &sect );
}
// end VS_GetEstablishedAcceptPorts

int VS_RemoveAcceptPorts( void )
{
	int   ret = -1;
	EnterCriticalSection( &sect );
	if (flagInstall)	ret = VS_Close_Listens_TCPsLike();
	LeaveCriticalSection( &sect );
	return ret;
}
// end VS_RemoveAcceptPorts

int VS_AddAcceptHandler( const char *startHandshakeString,
							const VS_AcceptHandlerCallback cb, const void *cb_arg ,
							const bool is_disable_nagle, const bool qos, _QualityOfService *qos_params)
{
	//isDisableNagle = is_disable_nagle;
	m_qos = qos;
	m_qos_params = qos_params;
	int   ret = -1;
	if (!cb || !startHandshakeString || !*startHandshakeString
			|| strlen(startHandshakeString) + 1 > net::HandshakeHeader::primary_field_size)
		return ret;
	EnterCriticalSection( &sect );
	if (flagInstall)	ret = VS_Add_Handler_For_Listen_TCPsLike( startHandshakeString, cb, cb_arg,is_disable_nagle );
	LeaveCriticalSection( &sect );
	return ret;
}
// end VS_AddAcceptHandler

int VS_DeleteAcceptHandler( const char *startHandshakeString )
{
	int   ret = -1;
	if (!startHandshakeString || !*startHandshakeString
			|| strlen(startHandshakeString) + 1 > net::HandshakeHeader::primary_field_size)
		return ret;
	EnterCriticalSection( &sect );
	if (flagInstall)	ret = VS_Delete_Handler_For_Listen_TCPsLike( startHandshakeString );
	LeaveCriticalSection( &sect );
	return ret;
}
// end VS_DeleteAcceptHandler

int VS_DeleteAllAcceptHandlers( void )
{
	int   ret = -1;
	EnterCriticalSection( &sect );
	if (flagInstall)	ret = VS_Delete_Handlers_For_Listen_TCPsLike();
	LeaveCriticalSection( &sect );
	return ret;
}
// end VS_DeleteAllAcceptHandlers

void VS_TestAccessibility(
	const std::vector<std::unique_ptr<net::endpoint::ConnectTCP>>& rc,
	std::vector<net::endpoint::ConnectTCP*>& success_rc,
	const char* tested_endpoint,
	unsigned long& milliseconds
)
{
	VS_Test_Accessibility(rc, success_rc, tested_endpoint, milliseconds);
}
// end VS_TestAccessibility

VS_ConnectionSock *VS_CreateNotGuaranteedConnection( const char *endpoint )
{
	VS_ConnectionSock   *ret = 0;
	if (!endpoint || !*endpoint)	return ret;
	EnterCriticalSection( &sect );
	if (flagInstall)	ret = VS_Connect_UDPLike( endpoint );
	LeaveCriticalSection( &sect );
	return ret;
}
// end VS_ExistsNotGuaranteed

VS_ConnectionSock *VS_CreateConnection( const char *endpoint, unsigned long &milliseconds,
											unsigned *nTCP, const bool canBeNotGuaranteed,
											VS_CreationAttempts *attempts ,
											const bool is_disable_nagle, const bool qos, _QualityOfService * qos_params )
{
	VS_ConnectionSock   *ret = 0;
	///isDisableNagle = is_disable_nagle;
	m_qos = qos;
	m_qos_params = qos_params;
	if (!endpoint || !*endpoint)	return ret;
	EnterCriticalSection( &sect );
	if (flagInstall)
	{
		LeaveCriticalSection( &sect );
		if (canBeNotGuaranteed)		ret = VS_Connect_UDPLike( endpoint );
		if (!ret)	ret = VS_Connect_TCPLike( endpoint, is_disable_nagle, milliseconds, nTCP, attempts );
	}
	else	LeaveCriticalSection( &sect );
	return ret;
}


// end VS_CreateConnection

VS_ConnectionSock* VS_CreateConnection(net::endpoint::ConnectTCP& rc, unsigned long& milliseconds, const bool is_disable_nagle)
{
	VS_ConnectionSock	*ret = 0;
	EnterCriticalSection( &sect );
	if (flagInstall)
	{
		LeaveCriticalSection( &sect );
		//if (canBeNotGuaranteed)		ret = VS_Connect_UDPLike( endpoint );
		ret = VS_Connect_TCPLike( rc, is_disable_nagle, milliseconds);
	}
	else
		LeaveCriticalSection( &sect );
	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////

class VS_AcceptHandler
{
public:
	VS_AcceptHandler( const char *startHandshakeString,
						const VS_AcceptHandlerCallback cb, const void *cb_arg, const bool isDisableNagle ) :
		isValid(false), startHandshakeString(_strdup( startHandshakeString )),
		cb(cb), cb_arg(cb_arg), m_isDisableNagle(isDisableNagle)
	{
		if (!VS_AcceptHandler::startHandshakeString)	return;
		isValid = true;
	}
	// end VS_AcceptHandler::VS_AcceptHandler

	~VS_AcceptHandler( void )
	{	if (startHandshakeString)	free( (void *)startHandshakeString );	}
	// end VS_AcceptHandler::~VS_AcceptHandler

	bool   isValid;
	const char   *startHandshakeString;
	const VS_AcceptHandlerCallback   cb;
	const void   *cb_arg;
	bool m_isDisableNagle;

	static inline VS_AcceptHandler *GetHandler( const char *startHandshakeString )
	{
		VS_AcceptHandler   *h = headHandler;
		for (unsigned i = 0; i < nHandlers && h; ++i, h = h->nextHandler)
			if (!strcmp( startHandshakeString, h->startHandshakeString ))	return h;
		return 0;
	}
	// end VS_AcceptHandler::GetHandler

	static inline void AddHandler( VS_AcceptHandler *h )
	{
		if (!nHandlers)
		{	headHandler = endHandler = h;	h->previousHandler = h->nextHandler = 0;	}
		else
		{	endHandler->nextHandler = h;	h->previousHandler = endHandler;
			endHandler = h;		h->nextHandler = 0;		}
		++nHandlers;
	}
	// end VS_AcceptHandler::AddHandler

	static inline void DeleteHandler( VS_AcceptHandler *h )
	{
		if (nHandlers == 1)		;
		else if (h == headHandler)
		{	headHandler = h->nextHandler;	headHandler->previousHandler = 0;	}
		else if (h == endHandler)
		{	endHandler = h->previousHandler;	endHandler->nextHandler = 0;	}
		else
		{	h->nextHandler->previousHandler = h->previousHandler;
			h->previousHandler->nextHandler = h->nextHandler;	}
		if (!--nHandlers)	headHandler = endHandler = 0;
		delete h;
	}
	// end VS_AcceptHandler::DeleteHandler

	static inline void DeleteAllHandlers( void )
	{
		VS_AcceptHandler   *h = headHandler, *h_next;
		for (unsigned i = 0; i < VS_CM_MAX_TCP_LIKE_ACCEPT_HANDLERS && h; ++i, h = h_next)
		{	h_next = h->nextHandler;	DeleteHandler( h );		}
		headHandler = endHandler = 0;	nHandlers = 0;
	}
	// end VS_AcceptHandler::DeleteAllHandlers

	static inline VS_AcceptHandler *AddAcceptHandler( const char *startHandshakeString,
								const VS_AcceptHandlerCallback cb, const void *cb_arg, const bool isDisableNagle )
	{
		VS_AcceptHandler   *h = GetHandler( startHandshakeString );
		if (h)
		{
			if (h->cb != cb || h->cb_arg != cb_arg)		return 0;
			return h;
		}
		h = new VS_AcceptHandler( startHandshakeString, cb, cb_arg, isDisableNagle );
		if (!h)		return 0;
		if (!h->isValid) {		delete h;	return 0;	}
		AddHandler( h );	return h;
	}
	// end VS_AcceptHandler::AddAcceptHandler

	static inline int DeleteAcceptHandler( const char *startHandshakeString )
	{
		VS_AcceptHandler   *h = GetHandler( startHandshakeString );
		if (h)	DeleteHandler( h );
		return (int)nHandlers;
	}
	// end VS_AcceptHandler::DeleteAcceptHandler

	static inline int DeleteAllAcceptHandlers( void )
	{
		int   ret = (int)nHandlers;
		DeleteAllHandlers();	return ret;
	}
	// end VS_AcceptHandler::DeleteAllAcceptHandlers

	static inline void Thread( VS_ConnectionTCP *tcp )
	{
#ifdef _MY_DEBUG_
printf( "\nVS_ConnectionManager: VS_AcceptHandler::Thread( tcp: %08X )\n", _MD_POINT_(tcp) );
#endif
		net::HandshakeHeader* hs = nullptr;
		VS_SCOPE_EXIT {
			free(hs);
			delete tcp;
		};
		unsigned long   mills = VS_CM_MAX_TCP_LIKE_HANDSHAKE_TIMEOUT;
		if (!VS_ReadZeroHandshake( tcp, &hs, mills ))
			return;
		EnterCriticalSection( &sect );
		VS_AcceptHandler   *handler = headHandler;
		for (unsigned i = 0; handler && i < nHandlers; handler = handler->nextHandler, ++i)
			if (!strcmp(hs->primary_field, handler->startHandshakeString))
			{
				const VS_AcceptHandlerCallback   cb = handler->cb;
				const void   *cb_arg = handler->cb_arg;
				if(handler->m_isDisableNagle)
					tcp->SetFastSocket(true);
				LeaveCriticalSection( &sect );
				(cb)( cb_arg, tcp, hs );
				hs = nullptr;
				tcp = nullptr;
				return;
			}
		LeaveCriticalSection( &sect );
	}
	// end VS_AcceptHandler::Thread

	static void __cdecl Thread( void *arg )
	{
		vs::SetThreadName("ConnMgr");
		Thread(static_cast<VS_ConnectionTCP*>(arg));
		_endthread();
	}
	// end VS_AcceptHandler::Thread

	static inline void SetTCPLikeSocket( VS_ConnectionTCP *tcp )
	{
#ifdef _MY_DEBUG_
printf( "\nVS_ConnectionManager: VS_AcceptHandler::SetTCPLikeSocket( tcp: %08X )\n", _MD_POINT_(tcp) );
#endif
		if (!nHandlers) {	delete tcp;		return;		}
		_beginthread( &Thread, 0, (void *)tcp );
	}
	// end VS_AcceptHandler::SetTCPLikeSocket

	VS_AcceptHandler			*previousHandler, *nextHandler;
	static VS_AcceptHandler		*headHandler, *endHandler;
	static unsigned				nHandlers;
};
// end VS_AcceptHandler class
VS_AcceptHandler   *VS_AcceptHandler::headHandler = 0, *VS_AcceptHandler::endHandler = 0;
unsigned   VS_AcceptHandler::nHandlers = 0;

//////////////////////////////////////////////////////////////////////////////////////////

static inline int VS_Add_Handler_For_Listen_TCPsLike( const char *startHandshakeString,
									const VS_AcceptHandlerCallback cb, const void *cb_arg, const bool isDisableNagle )
{	return !VS_AcceptHandler::AddAcceptHandler( startHandshakeString, cb, cb_arg, isDisableNagle ) ? -1 : VS_AcceptHandler::nHandlers;	}
// end VS_Add_Handler_For_Listen_TCPsLike

static inline int VS_Delete_Handler_For_Listen_TCPsLike( const char *startHandshakeString )
{	return VS_AcceptHandler::DeleteAcceptHandler( startHandshakeString );	}
// end VS_Delete_Handler_For_Listen_TCPsLike

static inline int VS_Delete_Handlers_For_Listen_TCPsLike( void )
{	return VS_AcceptHandler::DeleteAllAcceptHandlers();		}
// end VS_Delete_Handlers_For_Listen_TCPsLike

//////////////////////////////////////////////////////////////////////////////////////////

class VS_AcceptPoint
{
public:
	VS_AcceptPoint( const char *host, const unsigned short port, const char *protocol ) :
		isValid(false), host(_strdup( host )), protocol(_strdup( protocol )), port(port),
		listTCP(new VS_ConnectionTCP)
	{
		if (!VS_AcceptPoint::host || !VS_AcceptPoint::protocol || !VS_AcceptPoint::port
			|| !listTCP || !listTCP->IsValid() || !listTCP->Listen( host, port, false))	return;
		isValid = true;
	}
	// end VS_AcceptPoint::VS_AcceptPoint

	~VS_AcceptPoint( void )
	{
		if (listTCP)	delete listTCP;
		if (protocol)	free( (void *)protocol );
	}
	// end VS_AcceptPoint::~VS_AcceptPoint

	bool   isValid;
	const char    *host, *protocol;
	const unsigned short   port;
	VS_ConnectionTCP   *listTCP;

	static inline unsigned long GetFreeIndex( void )
	{
		for (unsigned i = 0; i < VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS; ++i)
			if (!point[i])		return i;
		return ~0;
	}
	// end VS_AcceptPoint::GetFreeIndex

	static inline void DeletePoint( const unsigned long index )
	{
		if (index >= VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS)	return;
		if (!point[index])	return;
		delete point[index];	point[index] = 0;	--nPoints;
	}
	// end VS_AcceptPoint::DeletePoint

	static inline void DeleteAllPoints( void )
	{
		for (unsigned i = 0; i < VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS; ++i)
			if (point[i]) 	delete point[i];
		memset( (void *)point, 0, sizeof(point) );	nPoints = 0;
	}
	// end VS_AcceptPoint::DeleteAllPoints

	static inline void ThreadAccept( const unsigned index )
	{
		VS_AcceptPoint   *p = point[index];
		if (!p)		return;
		VS_ConnectionTCP   *c = p->listTCP;
		if (!c || c->State() != vs_sock_state_listen)	return;
		VS_ConnectionTCP   *conn = 0;
		if (!_stricmp( p->protocol, net::endpoint::protocol_tcp ))
			conn = new VS_ConnectionTCP;
//		else if (!_stricmp(p->protocol, net::endpoint::protocol_http))
//			conn = (VS_ConnectionTCP *)(new VS_ConnectionHTTP);
		else {		DeletePoint( index );	return;		}
		if (!conn) {	DeletePoint( index );	return;		}
		if (!conn->IsValid()) {		delete conn;	DeletePoint( index );	return;		}
		unsigned long   mills = 0;
		if (conn->Accept( c, mills, /*isDisableNagle*/true, m_qos, m_qos_params ) != 1)
		{	delete conn;	DeletePoint( index );	return;		}
		if (!conn->CreateOvWriteEvent() || !conn->CreateOvReadEvent())
		{	delete conn;	return;		}
		VS_AcceptHandler::SetTCPLikeSocket( conn );
	}
	// end VS_AcceptPoint::ThreadAccept

	static inline void Thread( void )
	{
		void   *handles[VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS];
		unsigned   n_handles;
		while (threadContinue)
		{
			memset( (void *)handles, 0, sizeof(handles) );	n_handles = 0;
			EnterCriticalSection( &sect );
			for (unsigned i = 0, j = 0; j < nPoints && i < VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS; ++i)
			{	VS_AcceptPoint   *p = point[i];
				if (p)
				{	++j;
					VS_ConnectionTCP   *c = p->listTCP;
					if (c && c->State() == vs_sock_state_listen)
						handles[n_handles++] = c->GetHandle();
			}	}
			LeaveCriticalSection( &sect );
			if (!n_handles)		threadContinue = false;
			else
			{
				unsigned long   mills = INFINITE;
				int   ret = VS_ConnectionSock::SelectIn( (const void **)handles, n_handles, mills );
				switch (ret)
				{
				case -2 :	break;
				case -1 :	threadContinue = false;		break;
				default :	EnterCriticalSection( &sect );
							ThreadAccept( (const unsigned)ret );
							LeaveCriticalSection( &sect );
	}	}	}	}
	// end VS_AcceptPoint::Thread

	static unsigned __stdcall Thread(void* arg)
	{
		vs::SetThreadName("ConnMgr_Accept");
		Thread();
		_endthreadex(0);
		return 0;
	}
	// end VS_AcceptPoint::Thread

	static inline bool CreateAcceptThread( void )
	{
		threadContinue = true;
		hThread = (HANDLE)_beginthreadex( 0, 0, Thread, 0, 0, 0 );
		if (!hThread || hThread == (HANDLE)-1L) {	hThread = 0;	return false;	}
		return true;
	}
	// end VS_AcceptPoint::CreateAcceptThread

	static inline int DeleteAllPointsCloseAcceptThread( void )
	{
		threadContinue = false;
		EnterCriticalSection( &sect );
		int   ret = nPoints;	DeleteAllPoints();
		LeaveCriticalSection( &sect );
		if (hThread)
		{
			if (WaitForSingleObject( hThread, 60000 ) == WAIT_OBJECT_0)
				CloseHandle( hThread );
			hThread = 0;
		}
		return ret;
	}
	// end VS_AcceptPoint::DeleteAllPointsCloseAcceptThread

	static inline void CreateAcceptPointAct( const char *host,
									const unsigned short port, const char *protocol )
	{
		const unsigned long   index = GetFreeIndex();
		if (index >= VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS)	return;
		for(int i = 0; i<VS_CM_RANGE_FOR_ACCEPT_PORTS; i++)
		{
			VS_AcceptPoint   *p = new VS_AcceptPoint( host, port + i, protocol );
			//if (!p)		return;
			//if (!p->isValid) {		delete p;	return;		}
			if(!p)
				continue;
			if(!p->isValid)
				delete p;
			else
			{
				point[index] = p;	++nPoints;
				break;
			}
		}
	}
	// end VS_AcceptPoint::CreateAcceptPointAct

	static inline void CreateAcceptPoint( const char *host,
								const unsigned short port, const char *protocol )
	{
		if (nPoints >= VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS || !port || !protocol
			|| _stricmp(protocol, net::endpoint::protocol_tcp))	return;
//				&& _stricmp(protocol, net::endpoint::protocol_http)))		return;
		if (!host || !*host
			|| !_stricmp( host, vs_str_all_current ) || !_stricmp( host, vs_str_all_current1 )
			|| !_stricmp( host, vs_str_all_current2 ) || !_stricmp( host, vs_str_all_current3 ))
		{
			char   *my_host = (char *)malloc( 512 ), *hosts[VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS] = { 0 };
			unsigned   nHosts = 0;
			if (!my_host)	return;		memset( (void *)my_host, 0, 512 );
			if (!VS_GetDefaultHostName( my_host, 512 ))		goto go_return;
			for (unsigned long i = 0; i < VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS; ++i)
			{	if (!(hosts[i] = (char *)malloc( 32 )))		goto go_return;
				memset( (void *)hosts[i], 0, 32 );		}
			nHosts = VS_GetHostsByName( my_host, hosts, VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS, 32 );
			if (!nHosts)	goto go_return;
			for (unsigned i = 0; i < nHosts; ++i)
				if (hosts[i] && *hosts[i])	CreateAcceptPointAct( hosts[i], port, protocol );
go_return:
			for (unsigned long i = 0; i < VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS; ++i)
				if (hosts[i])	free( (void *)hosts[i] );
			if (my_host)	free( (void *)my_host );
		}
		else	CreateAcceptPointAct( host, port, protocol );
	}
	// end VS_AcceptPoint::CreateAcceptPoint

	static inline void GetAllPointsRegistryAcceptTCP(std::vector<std::unique_ptr<net::endpoint::ConnectTCP>>& connectTCP)
	{
		EnterCriticalSection( &sect );
		for (unsigned i = 0, j = 0; j < nPoints && i < VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS; ++i)
		{
			VS_AcceptPoint* p = point[i];
			if (p)
			{
				++j;
				if (!p->host || !p->port || !p->protocol)
					continue;
				std::unique_ptr<net::endpoint::ConnectTCP> c(new net::endpoint::ConnectTCP{ p->host, p->port, p->protocol });
				c->is_ipv6 = p->listTCP->IsIPv6();
				connectTCP.emplace_back(std::move(c));
			}
		}
		LeaveCriticalSection( &sect );
	}
	// end VS_AcceptPoint::GetAllPointsRegistryAcceptTCP

	static VS_AcceptPoint		*point[VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS];
	static unsigned				nPoints;
	static HANDLE				hThread;
	static bool					threadContinue;
	static CRITICAL_SECTION		sect;
};
// end VS_AcceptPoint class
VS_AcceptPoint   *VS_AcceptPoint::point[] = { 0 };
unsigned   VS_AcceptPoint::nPoints = 0;		HANDLE   VS_AcceptPoint::hThread = 0;
bool   VS_AcceptPoint::threadContinue = false;
CRITICAL_SECTION   VS_AcceptPoint::sect = { 0 };

//////////////////// VS_Open,Get,Close_Listens_TCPLike ///////////////////////////////////

static inline void VS_Get_Listens_TCPsLike(std::vector<std::unique_ptr<net::endpoint::ConnectTCP>>& connectTCP)
{
	if (!flagListensInstall)
		return;
	VS_AcceptPoint::GetAllPointsRegistryAcceptTCP(connectTCP);
}
// end VS_Get_Listens_TCPsLike

static inline int VS_Close_Listens_TCPsLike( void )
{
	if (!flagListensInstall)	return 0;
	int   ret = VS_AcceptPoint::DeleteAllPointsCloseAcceptThread();
	flagListensInstall = false;
	return ret;
}
// end VS_Close_Listens_TCPsLike

static inline int VS_Open_Listens_TCPsLike( void )
{
	if (flagListensInstall)		return VS_AcceptPoint::nPoints;
	char   nmEp[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
	if (!VS_EndpointName(nmEp, sizeof(nmEp)))
		return VS_AcceptPoint::nPoints;
	bool   continueFlag = true;
	const unsigned max = net::endpoint::GetCountAcceptTCP(nmEp);
	for (unsigned i = 1; continueFlag && i <= max; ++i)
	{
		auto rc = net::endpoint::ReadAcceptTCP(i, nmEp);
		if (!rc)	continue;
		VS_AcceptPoint::CreateAcceptPoint(rc->host.c_str(), rc->port, rc->protocol_name.c_str());
	}
	if (VS_AcceptPoint::nPoints)
	{
		InitializeCriticalSection( &VS_AcceptPoint::sect );
		if (!VS_AcceptPoint::CreateAcceptThread())
		{	VS_AcceptPoint::DeleteAllPoints();
			DeleteCriticalSection( &VS_AcceptPoint::sect );		}
		else	flagListensInstall = true;
	}
	return VS_AcceptPoint::nPoints;
}
// end VS_Open_Listens_TCPsLike

//////////////////// Internet Options Functions //////////////////////////////////////////

inline bool VS_Get_InternetOptions( char *&lpszProxy, char *&lpszProxyBypass )
{
	const unsigned long   len = sizeof(INTERNET_PROXY_INFO) + (sizeof(INTERNET_PROXY_INFO) > 2048 ? 0 : 2048 - sizeof(INTERNET_PROXY_INFO));
	const INTERNET_PROXY_INFO   *info = (INTERNET_PROXY_INFO *)malloc( (size_t)len );
	if (!info)		return false;			unsigned long   sz = len;
	if (!InternetQueryOption( 0, INTERNET_OPTION_PROXY, (void *)info, &sz ))	return false;
	if (info->lpszProxy && *(info->lpszProxy))
		lpszProxy = _strdup( info->lpszProxy );
	if (info->lpszProxyBypass && *(info->lpszProxyBypass))
		lpszProxyBypass = _strdup( info->lpszProxyBypass );
	return true;
}
// end VS_Get_InternetOptions

inline unsigned VS_Get_InternetOptions( char *&proxy_host, unsigned short &proxy_port, const char * templet = "socks=")
{
	char   *lpszProxy = 0, *lpszProxyBypass = 0;
	if (!VS_Get_InternetOptions( lpszProxy, lpszProxyBypass ) || !lpszProxy)	return 0;
	unsigned   ret = 1;

	char   *pc, *host = 0; int port = 0;

	host = strstr(lpszProxy, templet);
	if (host) {
		host += strlen(templet);
		char * _space = strchr(host, 0x20); // disable spaces
		if (_space) { *_space = 0; }
	} else {
		if (strchr(lpszProxy, ':')) {
			host = lpszProxy;
			char * _space = strchr(host, 0x20); // disable spaces
			if (_space) { *_space = 0; }
		} else {
			host = 0; ret = 0; goto go_ret;
		}
	}

	pc = strchr( host, ':' );		if (!pc) {		ret = 0;	goto go_ret;	}
	*pc = 0;	++pc;		if (!pc) {		ret = 0;	goto go_ret;	}
	port = atoi( pc );		if (port < 1 || port > 65535) {		ret = 0;	goto go_ret;	}
	proxy_host = _strdup( host ); if (!proxy_host) {	ret = 0;	goto go_ret;	}
	proxy_port = port;
go_ret:
	if (lpszProxyBypass)	free( (void *)lpszProxyBypass );
	if (lpszProxy)		free( (void *)lpszProxy );			return ret;
}
// end VS_Get_InternetOptions

inline bool VS_Get_InternetOptions_Socks(net::endpoint::ConnectTCP& rc)
{
	char   *proxy_host = 0;		unsigned short   proxy_port = 0;
	const bool ret = VS_Get_InternetOptions(proxy_host, proxy_port, "socks=");
	if (proxy_host) {
		rc.socks_host = proxy_host;
		free((void *)proxy_host);
	}
	rc.socks_port = proxy_port;
	rc.socks_version = 4;
	return ret;
}
// end VS_Get_InternetOptions_Socks

static inline bool VS_Get_InternetOptions_HttpTnl(net::endpoint::ConnectTCP& rc)
{
	char   *proxy_host = 0;		unsigned short   proxy_port = 0;
	const bool ret = VS_Get_InternetOptions( proxy_host, proxy_port, "http=");
	if (proxy_host) {
		rc.http_host = proxy_host;
		free((void *)proxy_host);
	}
	rc.http_port = proxy_port;
	return ret;
}
// end VS_Get_InternetOptions_HttpTnl

//////////////////// TCP-Like Connections Functions //////////////////////////////////////

static inline VS_ConnectionTCP *VS_Connect_TCP( const char *host, unsigned short port, const bool isDisableNagle, unsigned long &mills )
{

	VS_ConnectionTCP   *ret = new VS_ConnectionTCP;
	if (!ret)		return 0;
	if (!ret->IsValid()) {		delete ret;		return 0;		}
	unsigned long ip(0);
	void *hevent(0);
	unsigned long tick(GetTickCount());
	unsigned long diff(0);

	if (ret->ConnectAsynch(host, port, hevent) && ret->GetConnectResult(mills, false, isDisableNagle, m_qos, m_qos_params))
	{
		diff = GetTickCount() - tick;
		mills = diff > mills ? 0 : mills - diff;

		return ret;
	}

	/*if(VS_GetHostByName(host,ip_host,0xff) && VS_GetIpByHost(ip_host,&ip)&&ret->ConnectAsynch(ip,port,hevent))
	{
		if(ret->GetConnectResult(mills, false, isDisableNagle, m_qos, m_qos_params))
		{
			diff = GetTickCount() - tick;
			mills = diff>mills?0:mills - diff;
			return ret;
		}
	}
	*/
	delete ret;
	diff = GetTickCount() - tick;
	mills = diff>mills?0:mills - diff;
	return 0;

	/*VS_ConnectionTCP   *ret = new VS_ConnectionTCP;
	if (!ret)		return 0;
	if (!ret->IsValid()) {		delete ret;		return 0;		}
	if (ret->Connect( host, port, mills , isDisableNagle, m_qos, m_qos_params ))		return ret;
	delete ret;		return 0;*/
}
// end VS_Connect_TCP

namespace
{
inline VS_ConnectionTLS* VS_ConvertToTLS(VS_ConnectionTCP* conn, unsigned long& mills)
{
	if (!conn)
		return nullptr;
	VS_ConnectionTLS* conTls = new VS_ConnectionTLS(conn);
	delete conn;
	if (!conTls->IsValid())
	{
		delete conTls;
		return nullptr;
	}
	vs::event done(false);
	bool handshakeSuccess(false);

	if (conTls->Handshake(nullptr, 0,
			[&done, &handshakeSuccess](bool success, VS_ConnectionTLS*, const void*, const unsigned long)
			{
				handshakeSuccess = success;
				done.set();
			},
			mills,
			false
	    )
	   )
	{
		done.wait();
		if (handshakeSuccess)
			return conTls;
	}
	delete conTls;
	return nullptr;
}
}

inline VS_ConnectionTCP* VS_Pass_Socks(net::endpoint::ConnectTCP& rc, const bool isDisableNagle, unsigned long& mills, bool& wasChange, bool tryTls)
{
	wasChange = false;
	const char* host = rc.socks_host.c_str();
	const char* hst = rc.host.c_str();
	const unsigned short port = rc.socks_port;
	const unsigned short prt = rc.port;
	const unsigned char version = rc.socks_version;
	VS_ConnectionTCP *tcp = VS_Connect_TCP(host, port, isDisableNagle, mills);
	if (!tcp)
		return 0;
	if (!tcp->IsValid()) {
		delete tcp;
		return 0;
	}
	if (version == 4)
	{
		if (tcp->PassSocks4( hst, prt, mills ))		return tryTls ? VS_ConvertToTLS(tcp, mills) : tcp;
		delete tcp;		return 0;
	}
	else if (version == 5)
	{
go_s5:
		const char* usr = rc.socks_user.c_str();
		const char* psw = rc.socks_password.c_str();
		unsigned   st = tcp->PassSocks5( hst, prt, usr, psw, mills );
		if (st == 1)
		{
			VS_DeleteAuthRequest(net::endpoint::protocol_socks, host, port);
			return tryTls ? VS_ConvertToTLS(tcp, mills) : tcp;
		}
		else if (st == 2)
		{	delete tcp;		tcp = VS_Connect_TCP( host, port, isDisableNagle, mills );
			if (!tcp)	return 0;
			if (!tcp->IsValid()) {		delete tcp;		return 0;	}
			char   usr_buf[256] = { 0 }, psw_buf[256] = { 0 };		bool   escFlag = false;
			if (usr) {	strncpy( usr_buf, usr, sizeof(usr_buf) );	usr_buf[sizeof(usr_buf) - 1] = 0;	}
			if (psw) {	strncpy( psw_buf, psw, sizeof(psw_buf) );	psw_buf[sizeof(psw_buf) - 1] = 0;	}
			VS_GetAuthRequest(net::endpoint::protocol_socks, host, port, usr_buf, sizeof(usr_buf), psw_buf, sizeof(psw_buf), &escFlag);
			if (escFlag) {		delete tcp;		return 0;	}
			st = tcp->PassSocks5( hst, prt, usr_buf, psw_buf, mills );
			if (st == 1)	return tryTls ? VS_ConvertToTLS(tcp, mills) : tcp;
			else if (st == 2)
			{
				EnterCriticalSection( &authSect );
				if (!soleAuth) {	delete tcp;		tcp = 0;	}
				else
				{	{	char   usr_buf_new[64] = { 0 }, psw_buf_new[64] = { 0 };	escFlag = false;
						strncpy( usr_buf_new, usr_buf, sizeof(usr_buf_new) );	usr_buf_new[sizeof(usr_buf_new) - 1] = 0;
						strncpy( psw_buf_new, psw_buf, sizeof(psw_buf_new) );	psw_buf_new[sizeof(psw_buf_new) - 1] = 0;
						VS_GetAuthRequest(net::endpoint::protocol_socks, host, port, usr_buf_new, sizeof(usr_buf_new), psw_buf_new, sizeof(psw_buf_new), &escFlag);
						if (escFlag) {		delete tcp;		tcp = 0;	}
						else if (strcmp( psw_buf, psw_buf_new ) || strcmp( usr_buf, usr_buf_new ))
						{	strncpy( psw_buf, psw_buf_new, sizeof(psw_buf) );	psw_buf[sizeof(psw_buf) - 1] = 0;
							strncpy( usr_buf, usr_buf_new, sizeof(usr_buf) );	usr_buf[sizeof(usr_buf) - 1] = 0;
							delete tcp;		goto go_connect;
					}	}
					while (tcp)
					{	delete tcp;		tcp = 0;		if (st != 2)	break;
						st = soleAuth->Request(net::endpoint::protocol_socks, host, port, usr_buf, sizeof(usr_buf), psw_buf, sizeof(psw_buf));
						if (!st)
						{
							VS_AddAuthRequest(net::endpoint::protocol_socks, host, port, usr_buf, psw_buf, true);
							break;
						}
						else if (st == 1)
							VS_DeleteAuthRequest(net::endpoint::protocol_socks, host, port);
						else if (st == 2)
							VS_AddAuthRequest(net::endpoint::protocol_socks, host, port, usr_buf, psw_buf);
						else if (st == 3)
						{
							rc.socks_user = usr_buf;
							rc.socks_password = psw_buf;
							VS_DeleteAuthRequest(net::endpoint::protocol_socks, host, port);
							wasChange = true;
						}
						else	break;
go_connect:				tcp = VS_Connect_TCP( host, port, isDisableNagle, mills );
						if (!tcp)		break;
						if (!tcp->IsValid()) {		st = 0;		continue;	}
						st = tcp->PassSocks5( hst, prt, usr_buf, psw_buf, mills );
						if (st == 1)	break;
				}	}
				LeaveCriticalSection( &authSect );
				return tryTls ? VS_ConvertToTLS(tcp, mills) : tcp;
			}
			else
			{	delete tcp;		return 0;
		}	}
		else
		{	delete tcp;		return 0;
	}	}
	else
	{
		if (tcp->PassSocks4( hst, prt, mills ))		return tryTls ? VS_ConvertToTLS(tcp, mills) : tcp;
		delete tcp;		tcp = VS_Connect_TCP( host, port, isDisableNagle, mills );
		if (!tcp)		return 0;
		if (!tcp->IsValid()) {		delete tcp;		return 0;	}
		goto go_s5;
}	}
// end VS_Pass_Socks


inline VS_ConnectionTCP* VS_Pass_HttpTnl(net::endpoint::ConnectTCP& rc, const bool isDisableNagle, unsigned long& mills, bool& wasChange, bool tryTls)
{
	wasChange = false;
	const char* host = rc.http_host.c_str();
	const char* hst = rc.host.c_str();
	const char* usr = rc.http_user.c_str();
	const char* psw = rc.http_password.c_str();
	const unsigned short port = rc.http_port;
	const unsigned short prt = rc.port;
	VS_ConnectionTCP   *tcp = VS_Connect_TCP( host, port, isDisableNagle, mills );
	if (!tcp)	return 0;
	if (!tcp->IsValid()) {		delete tcp;		return 0;	}
	unsigned   st = tcp->PassHttpTnl( hst, prt, usr, psw, mills );
	if (st == 1)
	{
		VS_DeleteAuthRequest(net::endpoint::protocol_http_tnl, host, port);
		return tryTls ? VS_ConvertToTLS(tcp, mills) : tcp;
	}
	else if (st == 2)
	{	delete tcp;		tcp = VS_Connect_TCP( host, port, isDisableNagle, mills );
		if (!tcp)	return 0;
		if (!tcp->IsValid()) {		delete tcp;		return 0;	}
		char   usr_buf[256] = { 0 }, psw_buf[256] = { 0 };		bool   escFlag = false;
		if (usr) {	strncpy( usr_buf, usr, sizeof(usr_buf) );	usr_buf[sizeof(usr_buf) - 1] = 0;	}
		if (psw) {	strncpy( psw_buf, psw, sizeof(psw_buf) );	psw_buf[sizeof(psw_buf) - 1] = 0;	}
		VS_GetAuthRequest(net::endpoint::protocol_http_tnl, host, port, usr_buf, sizeof(usr_buf), psw_buf, sizeof(psw_buf), &escFlag);
		if (escFlag) {		delete tcp;		return 0;	}
		st = tcp->PassHttpTnl( hst, prt, usr_buf, psw_buf, mills );
		if (st == 1)	return tryTls ? VS_ConvertToTLS(tcp, mills) : tcp;
		else if (st == 2)
		{
			EnterCriticalSection( &authSect );
			if (!soleAuth) {	delete tcp;		tcp = 0;	}
			else
			{	{	char   usr_buf_new[256] = { 0 }, psw_buf_new[256] = { 0 };	escFlag = false;
					strncpy( usr_buf_new, usr_buf, sizeof(usr_buf_new) );	usr_buf_new[sizeof(usr_buf_new) - 1] = 0;
					strncpy( psw_buf_new, psw_buf, sizeof(psw_buf_new) );	psw_buf_new[sizeof(psw_buf_new) - 1] = 0;
					VS_GetAuthRequest(net::endpoint::protocol_http_tnl, host, port, usr_buf_new, sizeof(usr_buf_new), psw_buf_new, sizeof(psw_buf_new), &escFlag);
					if (escFlag) {		delete tcp;		tcp = 0;	}
					else if (strcmp( psw_buf, psw_buf_new ) || strcmp( usr_buf, usr_buf_new ))
					{	strncpy( psw_buf, psw_buf_new, sizeof(psw_buf) );	psw_buf[sizeof(psw_buf) - 1] = 0;
						strncpy( usr_buf, usr_buf_new, sizeof(usr_buf) );	usr_buf[sizeof(usr_buf) - 1] = 0;
						delete tcp;		goto go_connect;
				}	}
				while (tcp)
				{	delete tcp;		tcp = 0;		if (st != 2)	break;
					st = soleAuth->Request(net::endpoint::protocol_http_tnl, host, port, usr_buf, sizeof(usr_buf), psw_buf, sizeof(psw_buf));
					if (!st)
					{
						VS_AddAuthRequest(net::endpoint::protocol_http_tnl, host, port, usr_buf, psw_buf, true);
						break;
					}
					else if (st == 1)
						VS_DeleteAuthRequest(net::endpoint::protocol_http_tnl, host, port);
					else if (st == 2)
						VS_AddAuthRequest(net::endpoint::protocol_http_tnl, host, port, usr_buf, psw_buf);
					else if (st == 3)
					{
						rc.http_user = usr_buf;
						rc.http_password = psw_buf;
						VS_DeleteAuthRequest(net::endpoint::protocol_http_tnl, host, port);
						wasChange = true;
					}
					else	break;
go_connect:			tcp = VS_Connect_TCP( host, port, isDisableNagle, mills );
					if (!tcp)		break;
					if (!tcp->IsValid()) {		st = 0;		continue;	}
					st = tcp->PassHttpTnl( hst, prt, usr_buf, psw_buf, mills );
					if (st == 1)	break;
			}	}
			LeaveCriticalSection( &authSect );
			return tryTls ? VS_ConvertToTLS(tcp, mills) : tcp;
		}
		else
		{	delete tcp;		return 0;
	}	}
	else
	{	delete tcp;		return 0;
}	}
// end VS_Pass_HttpTnl

inline VS_ConnectionTCP* VS_Pass_HttpTnlMsq(net::endpoint::ConnectTCP& rc, const bool isDisableNagle, unsigned long& mills, bool& wasChange, bool tryTls)
{
	if (rc.host.empty() || rc.port == 0)
		return nullptr;
	rc.http_host = rc.host;
	rc.http_port = rc.port;
	return VS_Pass_HttpTnl(rc, isDisableNagle, mills, wasChange, tryTls);
}
// end VS_Pass_HttpTnlMsq

inline VS_ConnectionTLS* VS_Connect_TLS(const net::endpoint::ConnectTCP& rc, const bool isDisableNagle, unsigned long& mills, bool& wasChanged)
{
	VS_ConnectionTLS* ret = new VS_ConnectionTLS();
	vs::event done(false);
	bool connectSuccess(false);
	ret->SetCallback(
		[&connectSuccess, &done](bool success, VS_ConnectionTLS*, const void*, const unsigned long)
		{
			connectSuccess = success;
			done.set();
		}
	);
	if (ret->Connect(rc.host.c_str(), rc.port, mills, isDisableNagle))
	{
		done.wait();
		if (connectSuccess)
			return ret;
	}
	delete ret;
	return nullptr;
}

inline VS_ConnectionTCP* VS_Connect_TCP(const net::endpoint::ConnectTCP& rc, const bool isDisableNagle, unsigned long& mills, bool& wasChange)
{

	VS_ConnectionTCP   *ret = new VS_ConnectionTCP;
	if (!ret)		return 0;
	if (!ret->IsValid()) {		delete ret;		return 0;		}
	//unsigned long ip(0);
	void *hevent(0);
	unsigned long tick(GetTickCount());
	unsigned long diff(0);
	//char host[0xff];

	if (ret->ConnectAsynch(rc.host.c_str(), rc.port, hevent) && ret->GetConnectResult(mills, false, isDisableNagle, m_qos, m_qos_params))
	{
		diff = GetTickCount() - tick;
		mills = diff > mills ? 0 : mills - diff;

		return ret;
	}

	/*if(VS_GetHostByName(rc->Host(),host,0xff) && VS_GetIpByHost(host,&ip)&&ret->ConnectAsynch(ip,rc->Port(),hevent))
	{
		if(ret->GetConnectResult(mills, false, isDisableNagle, m_qos, m_qos_params))
		{
			diff = GetTickCount() - tick;
			mills = diff>mills?0:mills - diff;

			return ret;
		}
	}*/
	delete ret;
	diff = GetTickCount() - tick;
	mills = diff>mills?0:mills - diff;
	return 0;
	/*VS_ConnectionTCP   *ret = new VS_ConnectionTCP;
	if (!ret)		return 0;
	if (!ret->IsValid()) {		delete ret;		return 0;		}
	if (ret->Connect( rc->Host(), rc->Port(), mills, isDisableNagle, m_qos, m_qos_params ))		return ret;
	delete ret;		return 0;*/
}
// end VS_Connect_TCP



inline VS_ConnectionTCP* VS_Pass_InternetOptions(net::endpoint::ConnectTCP& rc, const bool isDisableNagle, unsigned long& mills, bool& wasChange, bool tryTls)
{
	VS_ConnectionTCP   *ret = 0;
	signed long _millz = mills;
	if (VS_Get_InternetOptions_HttpTnl( rc ))	ret = VS_Pass_HttpTnl( rc, isDisableNagle, mills, wasChange, tryTls );
	if (ret)	return ret;
	rc.http_host.clear();
	rc.http_port = 0;
	rc.http_user.clear();
	rc.http_password.clear();
	mills = _millz;
	if (VS_Get_InternetOptions_Socks( rc ))		ret = VS_Pass_Socks( rc, isDisableNagle, mills, wasChange, tryTls );
	if (ret)	return ret;
	rc.socks_host.clear();
	rc.socks_port = 0;
	rc.socks_user.clear();
	rc.socks_password.clear();
	rc.socks_version = 0;
	mills = _millz;
	if (tryTls)
		return VS_Connect_TLS(rc, isDisableNagle, mills, wasChange);
	else
		return VS_Connect_TCP(rc, isDisableNagle, mills, wasChange);
}
// end VS_Pass_InternetOptions

static inline VS_ConnectionSock* VS_Connect_TCPLike(net::endpoint::ConnectTCP& rc, const bool isDisableNagle, unsigned long& milliseconds)
{
	VS_ConnectionSock   *ret = 0;
	bool wasChange = false;
	unsigned long mills = milliseconds;

	if      (boost::iequals(rc.protocol_name, string_view(net::endpoint::protocol_tcp)))
	{
		mills = milliseconds / 2;
		ret = VS_Connect_TLS(rc, isDisableNagle, mills, wasChange);
		mills += milliseconds / 2;
		if (!ret)
			ret = VS_Connect_TCP(rc, isDisableNagle, mills, wasChange);
	}
	else if (boost::iequals(rc.protocol_name, string_view(net::endpoint::protocol_socks)))
		if (!(ret = VS_Pass_Socks(rc, isDisableNagle, mills, wasChange, true)))
			ret = VS_Pass_Socks(rc, isDisableNagle, mills, wasChange, false);
	else if (boost::iequals(rc.protocol_name, string_view(net::endpoint::protocol_http_tnl)))
		if (!(ret = VS_Pass_HttpTnl(rc, isDisableNagle, mills, wasChange, true)))
			ret = VS_Pass_HttpTnl(rc, isDisableNagle, mills, wasChange, false);
	else if (boost::iequals(rc.protocol_name, string_view(net::endpoint::protocol_http_tnl_msq)))
		if (!(ret = VS_Pass_HttpTnlMsq(rc, isDisableNagle, mills, wasChange, true)))
			ret = VS_Pass_HttpTnlMsq(rc, isDisableNagle, mills, wasChange, false);
	else if (boost::iequals(rc.protocol_name, string_view(net::endpoint::protocol_ssl)))
		;
	else if (boost::iequals(rc.protocol_name, string_view(net::endpoint::protocol_socks_ssl)))
		;
	else if (boost::iequals(rc.protocol_name, string_view(net::endpoint::protocol_https)))
		;
	else if (boost::iequals(rc.protocol_name, string_view(net::endpoint::protocol_http)))
		;
	else if (boost::iequals(rc.protocol_name, string_view(net::endpoint::protocol_internet_options)))
		if (!(ret = VS_Pass_InternetOptions(rc, isDisableNagle, mills, wasChange, true)))
			ret = VS_Pass_InternetOptions(rc, isDisableNagle, mills, wasChange, false);
	milliseconds = mills;
	return ret;
}

static inline VS_ConnectionSock *VS_Connect_TCPLike( const char *endpoint, bool isDisableNagle, unsigned long &milliseconds, unsigned *nTCP, VS_CreationAttempts *attempts )
{
	VS_ConnectionSock   *ret = 0;
	unsigned i, j=0;
	const unsigned int nTCPInitial = !nTCP ? 0 : *nTCP;
	bool   continueFlag = true, wasChange = false;
	const unsigned max = net::endpoint::GetCountConnectTCP(endpoint);
	if (max && nTCPInitial < max)
	{
		unsigned long   limitMills = milliseconds / ( max * 2 ), mills;
		j = nTCPInitial;
		if (limitMills < VS_CM_MIN_TCP_LIKE_HANDSHAKE_TIMEOUT)
			limitMills = VS_CM_MIN_TCP_LIKE_HANDSHAKE_TIMEOUT;
		for (i = 1; continueFlag && i <= max * 2; ++i)
		{
			bool tryTls = i <= max;
			j = ( j < max ? j + 1 : nTCPInitial + 1 );
			mills = limitMills;
			auto rc = net::endpoint::ReadConnectTCP(j, endpoint);
			if (!rc)
				continue;
			if (attempts)
			{
				continueFlag = attempts->Attempt(j, rc.get());
			}
			if (!continueFlag)
			{}
			else if (boost::iequals(rc->protocol_name, string_view(net::endpoint::protocol_tcp)))
			{
				if (tryTls)
					ret = VS_Connect_TLS(*rc, isDisableNagle, mills, wasChange);
				else
					ret = VS_Connect_TCP(*rc, isDisableNagle, mills, wasChange);
			}
			else if (boost::iequals(rc->protocol_name, string_view(net::endpoint::protocol_socks)))
				ret = VS_Pass_Socks(*rc, isDisableNagle, mills, wasChange, tryTls);
			else if (boost::iequals(rc->protocol_name, string_view(net::endpoint::protocol_http_tnl)))
				ret = VS_Pass_HttpTnl(*rc, isDisableNagle, mills, wasChange, tryTls);
			else if (boost::iequals(rc->protocol_name, string_view(net::endpoint::protocol_http_tnl_msq)))
				ret = VS_Pass_HttpTnlMsq(*rc, isDisableNagle, mills, wasChange, tryTls);
			else if (boost::iequals(rc->protocol_name, string_view(net::endpoint::protocol_ssl)))
				;
			else if (boost::iequals(rc->protocol_name, string_view(net::endpoint::protocol_socks_ssl)))
				;
			else if (boost::iequals(rc->protocol_name, string_view(net::endpoint::protocol_https)))
				;
			else if (boost::iequals(rc->protocol_name, string_view(net::endpoint::protocol_http)))
				;
			else if (boost::iequals(rc->protocol_name, string_view(net::endpoint::protocol_internet_options)))
				ret = VS_Pass_InternetOptions(*rc, isDisableNagle, mills, wasChange, tryTls);
			if (wasChange)
			{
				wasChange = false;
				net::endpoint::UpdateConnectTCP(j, *rc, endpoint);
			}
			continueFlag = !ret && continueFlag;
			mills = limitMills - mills;
			milliseconds = milliseconds < mills ? 0 : milliseconds - mills;
		}
	}
go_return:
	if (ret && nTCP)
		*nTCP = j;
	return ret;
}
// end VS_Connect_TCPLike

//////////////////// VS_Connect_UDPLike( endpoint ) //////////////////////////////////////

static inline VS_ConnectionSock *VS_Connect_UDPLike( const char *endpoint )
{
	return 0;
}
// end VS_Connect_UDPLike

//////////////////////////////////////////////////////////////////////////////////////////

void VS_Ping_Callback(const void *cb_arg, VS_ConnectionTCP *tcp, net::HandshakeHeader *hs)
{
	unsigned long   mills = 5000;	VS_WriteZeroHandshake( tcp, hs, mills );
	Sleep( mills );		delete tcp;		free( (void *)hs );
}
// end VS_Ping_Callback

inline bool VS_Test_Accessibility_Thread( VS_ConnectionTCP **tcp,
					const net::HandshakeHeader *hs, const net::endpoint::ConnectTCP& rc,
					unsigned long &milliseconds )
{
	if (!tcp || !hs)
		return false;
	bool wasChange = false;

	net::endpoint::ConnectTCP temprc(rc);
	if      (boost::iequals(temprc.protocol_name, string_view(net::endpoint::protocol_tcp)))
		*tcp = VS_Connect_TCP(temprc, false, milliseconds, wasChange);
	else if (boost::iequals(temprc.protocol_name, string_view(net::endpoint::protocol_socks)))
		*tcp = VS_Pass_Socks(temprc, false, milliseconds, wasChange, false);
	else if (boost::iequals(temprc.protocol_name, string_view(net::endpoint::protocol_http_tnl)))
		*tcp = VS_Pass_HttpTnl(temprc, false, milliseconds, wasChange, false);
	else if (boost::iequals(temprc.protocol_name, string_view(net::endpoint::protocol_http_tnl_msq)))
		*tcp = VS_Pass_HttpTnlMsq(temprc, false, milliseconds, wasChange, false);
	else if (boost::iequals(temprc.protocol_name, string_view(net::endpoint::protocol_ssl)))
		;
	else if (boost::iequals(temprc.protocol_name, string_view(net::endpoint::protocol_socks_ssl)))
		;
	else if (boost::iequals(temprc.protocol_name, string_view(net::endpoint::protocol_https)))
		;
	else if (boost::iequals(temprc.protocol_name, string_view(net::endpoint::protocol_http)))
		;
	else if (boost::iequals(temprc.protocol_name, string_view(net::endpoint::protocol_internet_options)))
		*tcp = VS_Pass_InternetOptions(temprc, false, milliseconds, wasChange, false);

	if (!(*tcp)) { return false; }
	if (!(*tcp)->IsValid() || !(*tcp)->Socket()) { delete (*tcp); (*tcp) = 0; return false; }

	net::HandshakeHeader* rhs = nullptr;
	if (!VS_WriteZeroHandshake( *tcp, hs, milliseconds )
		 || !VS_ReadZeroHandshake( *tcp, &rhs, milliseconds ))
		  return false;
	free( (void *)rhs ); return true;
}
// end VS_Test_Accessibility_Thread

struct VS_Test_Accessibility_Thread_Arg
{
	VS_Test_Accessibility_Thread_Arg( const net::HandshakeHeader *hs,
							const net::endpoint::ConnectTCP *rc, unsigned long milliseconds ) :
		isValid(false), tcp(0), hs(hs), rc(rc), milliseconds(milliseconds), result(false)
	{ }
	// end VS_Test_Accessibility_Thread_Arg::VS_Test_Accessibility_Thread_Arg

	~VS_Test_Accessibility_Thread_Arg( void ) {		if (tcp)	delete tcp;		}
	// end VS_Test_Accessibility_Thread_Arg::~VS_Test_Accessibility_Thread_Arg

	bool								isValid;
	VS_ConnectionTCP					*tcp;
	const net::HandshakeHeader			*hs;
	const net::endpoint::ConnectTCP		*rc;
	unsigned long						milliseconds;
	bool								result;
};
// end VS_Test_Accessibility_Thread_Arg struct

struct vs_local_garbage {
private:
	signed long m_count;
	VS_Test_Accessibility_Thread_Arg **m_pa;
	HANDLE *m_ht;
	net::HandshakeHeader *m_hs;
	vs_local_garbage(const vs_local_garbage &);
public:
	vs_local_garbage(const signed long _n, VS_Test_Accessibility_Thread_Arg **_pa, HANDLE *_ht, net::HandshakeHeader *hs);
	~vs_local_garbage();
	bool free_trash();
};
/* dumbs */
vs_local_garbage::vs_local_garbage(const vs_local_garbage &) {};
vs_local_garbage::~vs_local_garbage() {};
/* end of dumbs */
vs_local_garbage::vs_local_garbage(const long _n, VS_Test_Accessibility_Thread_Arg **_pa, HANDLE *_ht, net::HandshakeHeader *hs)
: m_count(_n), m_pa(_pa), m_ht(_ht), m_hs(hs) { };
bool vs_local_garbage::free_trash() {
	for (int i = 0; i < m_count; ++i) {
		if (m_pa[i] && !m_pa[i]->isValid && m_pa[i]->tcp) {
			m_pa[i]->tcp->Terminate();
		}
	}
	WaitForMultipleObjects( (DWORD)m_count, m_ht, TRUE, INFINITE );
	for (int i = 0; i < m_count; ++i) {
		if (m_ht[i])	{ CloseHandle( m_ht[i] ); m_ht[i] = 0; }
		if (m_pa[i])	{ delete m_pa[i]; m_pa[i] = 0; }
	}
	if (m_ht) delete m_ht;
	if (m_pa) delete m_pa;
	if (m_hs) free(m_hs);
	return true;
};

void __cdecl VS_Garbage_Thread( void *arg )
{
	if (arg) {
		vs_local_garbage *lg = static_cast<vs_local_garbage*>(arg);
		lg->free_trash();
		delete lg;
	}
	_endthread();
	return;
};

unsigned __stdcall VS_Test_Accessibility_Thread( void *arg )
{
	vs::SetThreadName("TestAccess");
	VS_Test_Accessibility_Thread_Arg   *p = (VS_Test_Accessibility_Thread_Arg *)arg;
	p->result = VS_Test_Accessibility_Thread( &p->tcp, p->hs, *p->rc, p->milliseconds );
	p->isValid = (p->tcp) && (p->result);
	_endthreadex( 0 );    return 0;
}
// end VS_Check_Accessibility

inline void VS_Test_Accessibility(
	const std::vector<std::unique_ptr<net::endpoint::ConnectTCP>>& rc,
	std::vector<net::endpoint::ConnectTCP*>& success_rc,
	const char* tested_endpoint,
	unsigned long& milliseconds
)
{
	success_rc.clear();
	if (milliseconds < VS_CM_MIN_TEST_ACCESSIBILITY_TIMEOUT || milliseconds > VS_CM_MAX_TEST_ACCESSIBILITY_TIMEOUT)
		return;
	unsigned n = (rc.size() <= VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS) ? rc.size() : VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS;
	for (unsigned i = 0; i < n; ++i)
		if (!rc[i])
			return;
	/*change to dynamic new from local!!!*/
/*
	VS_Test_Accessibility_Thread_Arg   *pa[VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS] = { 0 };
	HANDLE   ht[VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS] = { 0 };
*/
	VS_Test_Accessibility_Thread_Arg ** pa = new VS_Test_Accessibility_Thread_Arg*[VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS];
	HANDLE *ht = new HANDLE[VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS];
	if (!pa || !ht)
		return; // if no memory there is nothing to do :)
	memset((void *)pa, 0, sizeof(VS_Test_Accessibility_Thread_Arg *) * VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS);
	memset((void *)ht, 0, sizeof(HANDLE) * VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS);

	DWORD st;
	unsigned   nht = 0;		unsigned long   tc;
	const unsigned long   el = (unsigned long)strlen( tested_endpoint ), ln = el + 2;
	const size_t sz = sizeof(net::HandshakeHeader) + ln;
	net::HandshakeHeader* hs = (net::HandshakeHeader *)malloc( sz );
	if (!hs)
		return;
	memset((void *)hs, 0, sz);
	strcpy(hs->primary_field, VS_Ping_PrimaryField);
	hs->body_length = ln - 1;	hs->version = 1;
	unsigned char* body = &((unsigned char *)hs)[sizeof(net::HandshakeHeader)];
	*body = (unsigned char)el;	strcpy( (char *)&body[1], tested_endpoint );
	net::UpdateHandshakeChecksums(*hs);
	for (unsigned i = 0; i < n; ++i)
	{
		pa[i] = new VS_Test_Accessibility_Thread_Arg( hs, rc[i].get(), milliseconds );
		if (!pa[i]) goto go_error_return;
		pa[i]->rc = rc[i].get();
		ht[i] = (HANDLE)_beginthreadex( 0, 0, VS_Test_Accessibility_Thread, (void *)pa[i], 0, 0 );
		if (!ht[i] || ht[i] == (HANDLE)-1L) {	ht[i] = 0;	nht = i;	goto go_error_return;	}}
	tc = GetTickCount();
	st = WaitForMultipleObjects( (DWORD)n, ht, TRUE, milliseconds );
	tc = GetTickCount() - tc;	milliseconds = milliseconds < tc ? 0 : milliseconds - tc;
	if ((st < WAIT_OBJECT_0 || st >= (WAIT_OBJECT_0 + n)) && st != WAIT_TIMEOUT)
		goto go_error_return;
	for (unsigned i = 0; i < n; ++i)
	{ if (pa[i] && pa[i]->isValid && pa[i]->tcp)  pa[i]->tcp->Close(); }
//	st = WaitForMultipleObjects( (DWORD)n, ht, TRUE, 10000 );
//	if (st >= WAIT_OBJECT_0 && st < (WAIT_OBJECT_0 + n))
//		for (i = 0; i < n; ++i)		if (ht[i])	{ CloseHandle( ht[i] ); ht[i] = 0; }
	for (unsigned i = 0; i < n; ++i)
	{
		if (pa[i]->result)
			success_rc.emplace_back(rc[i].get());
		/*delete pa[i];*/
	}
//	free( (void *)hs ); kill handshake in thread
	//CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&VS_Garbage_Thread, (LPVOID)_lg, 0, 0);
	_beginthread(&VS_Garbage_Thread, 0, new vs_local_garbage(n, pa, ht, hs));
	return;
go_error_return:
	/*
	for (i = 0; i < n; ++i)
		if (pa[i] && pa[i]->isValid)	VS_ConnectionSock::Break( pa[i]->tcp->GetHandle() );
	if (nht)
	{	st = WaitForMultipleObjects( (DWORD)nht, ht, TRUE, 10000 );
		if (st >= WAIT_OBJECT_0 && st < (WAIT_OBJECT_0 + nht))
		{	for (i = 0; i < nht; ++i)	if (ht[i])	CloseHandle( ht[i] );
			/*for (i = 0; i < n; ++i)		if (pa[i])	delete pa[i];*/	/*}}
	*/
//	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&VS_Garbage_Thread, (LPVOID)_lg, 0, 0);
	_beginthread(&VS_Garbage_Thread, 0, new vs_local_garbage(nht, pa, ht, hs));
//	if (hs)		free( (void *)hs ); kill handshake in thread
}
// end VS_Test_Accessibility

//////////////////////////////////////////////////////////////////////////////////////////

struct AuthRequest
{	AuthRequest( const char *protocol, const char *proxy, const unsigned short port,
					const char *user, const char *passwd, const bool escFlag )
		: isValid(false), escFlag(escFlag), protocol(0), proxy(0), user(0), passwd(0), port(0)
	{
		AuthRequest::protocol = _strdup( protocol );	if (!AuthRequest::protocol)	return;
		AuthRequest::proxy = _strdup( proxy );		if (!AuthRequest::proxy)	return;
		if (user) {		AuthRequest::user = _strdup( user );
						if (!AuthRequest::user)		return;		}
		if (passwd) {	AuthRequest::passwd = _strdup( passwd );
						if (!AuthRequest::passwd)	return;		}
		AuthRequest::port = port;		isValid = true;
	}
	// end AuthRequest::AuthRequest
	~AuthRequest( void )
	{
		if (protocol)	free( (void *)protocol );
		if (proxy)		free( (void *)proxy );
		if (user)		free( (void *)user );
		if (passwd)		free( (void *)passwd );
	}
	// end AuthRequest::~AuthRequest
	inline bool Cmp( const char *prot, const char *prx, const unsigned short prt )
	{
		if (prt != port)
			return false;

		if (prot && protocol)
		{
			if (_stricmp(prot, protocol) != 0)
				return false;
		}
		else if (prot || protocol)
			return false;

		if (prx && proxy)
		{
			if (_stricmp(prx, proxy) != 0)
				return false;
		}
		else if (prx || proxy)
			return false;

		return true;
	}
	// end AuthRequest::Cmp
	inline void Reset( const char *usr, const char *psw, const bool escFlag )
	{
		if (!usr) {		if (user) {		free( (void *)user );	user = 0;	}}
		else {		if (user)
					{	if (strcmp( user, usr ))
						{	free( (void *)user );	user = _strdup( usr );	}}
					else	user = _strdup( usr );		}
		if (!psw) {		if (passwd) {	free( (void *)passwd );	passwd = 0;	}}
		else {		if (passwd)
					{	if (strcmp( passwd, psw ))
						{	free( (void *)passwd );	passwd = _strdup( psw );	}}
					else	passwd = _strdup( psw );		}
		AuthRequest::escFlag = escFlag;
	}
	bool   isValid, escFlag;
	char   *protocol, *proxy, *user, *passwd;
	unsigned short   port;
};	// end AuthRequest struct

static const unsigned   maxAuthRequests = 16;
static AuthRequest   *authRequests[maxAuthRequests] = { 0 };

inline void VS_SetAuthentication( VS_Authentication *auth )
{	EnterCriticalSection( &authSect );	soleAuth = auth;	LeaveCriticalSection( &authSect );	}
// end VS_SetAuthentication

inline void VS_AddAuthRequest( const char *protocol, const char *proxy,
						const unsigned short port, const char *user, const char *passwd,
						const bool escFlag )
{
	EnterCriticalSection( &authSect );
	AuthRequest   **zAr = 0;	bool   existsFlag = false;
	for (unsigned i = 0; i < maxAuthRequests; ++i)
	{	AuthRequest   *&ar = authRequests[i];
		if (!ar) {	if (!ar)	zAr = &ar;	}
		else {	if (ar->Cmp( protocol, proxy, port ))
				{	ar->Reset( user, passwd, escFlag );	existsFlag = true;	break;	}}}
	if (!existsFlag && zAr)		*zAr = new AuthRequest( protocol, proxy, port, user, passwd, escFlag );
	LeaveCriticalSection( &authSect );
}
// end VS_AddAuthRequest

inline void VS_GetAuthRequest( const char *protocol, const char *proxy, const unsigned short port,
						char *user, const unsigned user_size, char *passwd, const unsigned passwd_size,
						bool *escFlag )
{
	EnterCriticalSection( &authSect );
	for (unsigned i = 0; i < maxAuthRequests; ++i)
	{	AuthRequest   *ar = authRequests[i];
		if (ar)
		{	if (ar->Cmp( protocol, proxy, port ))
			{	strncpy( user, ar->user, user_size );	user[user_size - 1] = 0;
				strncpy( passwd, ar->passwd, passwd_size );		passwd[passwd_size - 1] = 0;
				*escFlag = ar->escFlag;		break;
	}	}	}
	LeaveCriticalSection( &authSect );
}
// end VS_GetAuthRequest

inline void VS_DeleteAuthRequest( const char *protocol, const char *proxy, const unsigned short port )
{
	EnterCriticalSection( &authSect );
	for (unsigned i = 0; i < maxAuthRequests; ++i)
	{	AuthRequest   *&ar = authRequests[i];
		if (ar && ar->Cmp( protocol, proxy, port ))
		{		delete ar;	ar = 0;		}}
	LeaveCriticalSection( &authSect );
}
// end VS_DeleteAuthRequest

inline void VS_ResetAuthentication( void )
{
	EnterCriticalSection( &authSect );
	for (unsigned i = 0; i < maxAuthRequests; ++i)
	{	AuthRequest   *ar = authRequests[i];	if (ar)		delete ar;	}
	memset( (void *)authRequests, 0, sizeof(authRequests) );
	LeaveCriticalSection( &authSect );
}
// end VS_ResetAuthentication

inline void VS_DeleteAuthentication( void )
{	EnterCriticalSection( &authSect );	soleAuth = 0;	LeaveCriticalSection( &authSect );	}
// end VS_DeleteAuthentication

//////////////////////////////////////////////////////////////////////////////////////////

static unsigned long VS_ConnectionManagerStart( void )
{
	InitializeCriticalSection( &sect );
	InitializeCriticalSection( &authSect );
	return START_FLAG_SEQUENCE;
}
// end VS_ConnectionManagerStart

//////////////////////////////////////////////////////////////////////////////////////////