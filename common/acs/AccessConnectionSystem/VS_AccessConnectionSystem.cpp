/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_AccessConnectionSystem.cpp
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

//#define   _MY_DEBUG_

#include <stdio.h>
#include <process.h>
#include <Windows.h>
#include <WinSock2.h>

#include "../../std/debuglog/VS_Debug.h"
#include "VS_AccessConnectionSystem.h"
#include "VS_AccessConnectionMonitor.h"
#include "VS_AccessConnectionHandlerTypes.h"
#include "VS_IOCPsManager.h"
#include "../VS_AcsDefinitions.h"
#include "acs/connection/VS_ConnectionTCP.h"
#include "acs/connection/VS_ConnectionMsg.h"
#include "acs/connection/VS_ConnectionByte.h"
#include "acs/connection/VS_ConnectionOv.h"
#include "../Lib/VS_AcsLib.h"
#include "../../net/Handshake.h"
#include "../../net/EndpointRegistry.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "../../std/cpplib/VS_RegistryConst.h"
#include "../../std/cpplib/VS_Utils.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/fast_mutex.h"

#include <cassert>

/////////////////////////////////////////////////////////////////////////////////////////

#define   VS_ACS_MIN_HANDLERS   4
#define   VS_ACS_MAX_HANDLERS   32

#define   VS_ACS_MIN_LISTENERS   2
#define   VS_ACS_MAX_LISTENERS   32

#define   VS_ACS_MIN_CONNECIONS   128
#define   VS_ACS_MAX_CONNECIONS   1024

#define   VS_ACS_MIN_LISTENER_CONNECIONS   2
#define   VS_ACS_MAX_LISTENER_CONNECIONS   32

#define   VS_ACS_MIN_CONNECTION_LIFETIME   5000
#define   VS_ACS_MAX_CONNECTION_LIFETIME   60000

#define   VS_ACS_TICK_MS   1000

#define   VS_ACS_MAX_BUFFER_LENGTH   32768
#define   VS_ACS_TIMEOUT_THREAD_SHUTDOWN	30000
#define   VS_ACS_REPEAT_ERROR_NUMBER   10

#define   VS_ACS_CONN_SUMP_DEPTH   64

#define   VS_ACS_LISTEN						1
#define   VS_ACS_LISTEN_DEAD				2
#define   VS_ACS_CONNECTION_WRITE			3
#define   VS_ACS_CONNECTION_READ			4
#define   VS_ACS_CONNECTION_DEAD_WRITE		5
#define   VS_ACS_CONNECTION_DEAD_READ		6
#define   VS_ACS_CONTROL_WRITE				7
#define   VS_ACS_CONTROL_READ				8
#define   VS_ACS_MONITOR_WRITE				9
#define   VS_ACS_MONITOR_READ				10
#define   VS_ACS_MONITOR_CONNECT			11

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

const char   VS_AcsPrefixMonPipe[] = VS_ACS_PREFIX_MON_PIPE;
static const char   acsTestAttemptStr[] = "AcsTestAttemptStr";

//#define   VS_ACS_AUTORESTART 1
#define   VS_ACS_AUTORESTART_PERIOD 25
///5 = 1 минута
#ifdef VS_ACS_AUTORESTART
	static unsigned long counter = 0;
#endif

extern std::string g_tr_endpoint_name;

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AccessConnectionSystem_ImplementationBase : public VS_AccessConnectionSystem_ImplementationCalls
{
	VS_AccessConnectionSystem_ImplementationBase()
		: indexSump(0), currDiffTm(0), currTm(0)
		, sumCurrDiffTm(0), tickDiffTm(0), tickTm(0)
	{	memset( (void *)sumpConns, 0, sizeof(sumpConns) );	}
	// end VS_AccessConnectionSystem_ImplementationBase::VS_AccessConnectionSystem_ImplementationBase

	~VS_AccessConnectionSystem_ImplementationBase( void )
	{
	}
	// end VS_AccessConnectionSystem_ImplementationBase::~VS_AccessConnectionSystem_ImplementationBase

	vs::fast_recursive_mutex mtx;
	vs::fast_mutex mtx_read;
	VS_Connection   *sumpConns[VS_ACS_CONN_SUMP_DEPTH];
	unsigned long   indexSump;
	unsigned long   currDiffTm, currTm, sumCurrDiffTm, tickDiffTm, tickTm;

	inline void DeleteConn( VS_Connection *sock )
	{
		if (!sock)	return;
		sock->SetOvFields();		sock->Close();
		++indexSump;	indexSump &= VS_ACS_CONN_SUMP_DEPTH - 1;
		if (sumpConns[indexSump])	delete sumpConns[indexSump];
		sumpConns[indexSump] = sock;
	}
	// end VS_StreamsRouter_Participant::DeleteConn
};
// end VS_AccessConnectionSystem_ImplementationBase struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AccessConnectionSystem_Handler
{
	VS_AccessConnectionSystem_Handler( VS_AccessConnectionSystem_ImplementationBase *acs,
										const char *name,
										VS_AccessConnectionHandler *handler,
										const bool isFinal )
		: isValid(false), handler(handler), isFinal(isFinal), nProcConns(0), nAccConns(0)
	{
		if (!name || !*name || strlen( name ) > VS_ACS_MAX_SIZE_HANDLER_NAME)	return;
		strcpy( handlerName, name );
		if (!handler)	return;
		strcpy( handler->imp->handlerName, name );
		handler->imp->imp_calls = acs;
		if (!handler->Init( handler->imp->handlerName ))	return;
		isValid = true;
	}
	// end VS_AccessConnectionSystem_Handler::VS_AccessConnectionSystem_Handler

	~VS_AccessConnectionSystem_Handler( void )
	{	if (handler)	handler->Destroy( handler->imp->handlerName );	}
	// end VS_AccessConnectionSystem_Handler~VS_AccessConnectionSystem_Handler

	bool   isValid, isFinal;
	char   handlerName[VS_ACS_MAX_SIZE_HANDLER_NAME + 1];
	VS_AccessConnectionHandler   *handler;
	unsigned long   nProcConns, nAccConns;
};
// end VS_AccessConnectionSystem_Handler struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AccessConnectionSystem_ListenerConnection
{
	VS_AccessConnectionSystem_ListenerConnection( VS_AccessConnectionSystem_ImplementationBase *acs,
		VS_ConnectionTCP *listener, const unsigned long listIndex, const unsigned long connIndex ) :
		isValid(false), acs(acs), conn(new VS_ConnectionTCP)
	{
		if (!conn || !conn->IsValid())	return;
		conn->SetOvReadFields( (const VS_ACS_Field)VS_ACS_LISTEN, (const VS_ACS_Field)listIndex, (const VS_ACS_Field)connIndex );
		if (!conn->Accept( listener ))	return;
		isValid = true;
	}
	// end VS_AccessConnectionSystem_ListenerConnection::VS_AccessConnectionSystem_ListenerConnection

	~VS_AccessConnectionSystem_ListenerConnection( void )
	{
		if (conn)
		{
			if (!conn->IsAccept())	acs->DeleteConn( conn );
			else
			{
				conn->SetOvFields( VS_ACS_LISTEN_DEAD, (const VS_ACS_Field)conn );
				conn->Disconnect();
	}	}	}
	// end VS_AccessConnectionSystem_ListenerConnection::~VS_AccessConnectionSystem_ListenerConnection

	bool   isValid;
	VS_AccessConnectionSystem_ImplementationBase   *acs;
	VS_ConnectionTCP   *conn;
};
// end VS_AccessConnectionSystem_ListenerConnection struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AccessConnectionSystem_Listener
{
	VS_AccessConnectionSystem_Listener( VS_AccessConnectionSystem_ImplementationBase *acs,
			const char *host, const int port, const unsigned long maxConnections,
			const unsigned long listIndex, void *hIOCP, const bool is_hidden ) :
		isValid(false), acs(acs), host(_strdup( host )), port(port), maxConns(maxConnections),
		listIndex(listIndex), nConns(0), nRecvConns(0), nUnsucAttempts(0),
		listener(new VS_ConnectionTCP), conns(0), hidden(is_hidden)
	{
		if (!listener || !listener->IsValid()
				|| !listener->Listen( host, port,true) || !listener->SetIOCP( hIOCP ))
			return;
		size_t   size = maxConns * sizeof(VS_AccessConnectionSystem_ListenerConnection *);
		conns = (VS_AccessConnectionSystem_ListenerConnection **)malloc( size );
		if (!conns)		return;
		memset( (void *)conns, 0, size );
		CreateListenerConnections();
		isValid = true;
	}
	// end VS_AccessConnectionSystem_Listener::VS_AccessConnectionSystem_Listener

	~VS_AccessConnectionSystem_Listener( void )
	{
		if (conns)
		{
			for (unsigned long i = 0; i < maxConns; ++i)	if (conns[i])	delete conns[i];
			free( (void *)conns );
		}
		if (listener)	acs->DeleteConn( listener );
		if (host)	free( (void *)host );
	}
	// end VS_AccessConnectionSystem_Listener::~VS_AccessConnectionSystem_Listener

	bool	isValid;
	VS_AccessConnectionSystem_ImplementationBase   *acs;
	const char   *host;
	const int   port;
	const unsigned long   maxConns, listIndex;
	unsigned long   nConns, nRecvConns, nUnsucAttempts;
	VS_ConnectionTCP   *listener;
	VS_AccessConnectionSystem_ListenerConnection   **conns;
	bool hidden;

	inline void CreateListenerConnections( void )
	{
		for (unsigned long i = 0; i < maxConns; ++i)
			if (!conns[i])
			{
				conns[i] = new VS_AccessConnectionSystem_ListenerConnection( acs, listener, listIndex, i );
				if (!conns[i])
				{	/* Here it will be necessary to throw off in TRACE */	}
				else if (!conns[i]->isValid)
				{	/* Here it will be necessary to throw off in TRACE */
					delete conns[i];	conns[i] = 0;
				}
				++nConns;
	}		}
	// end VS_AccessConnectionSystem_Listener::CreateListenerConnections

	inline void DeleteListenerConnection( const unsigned long index )
	{
		delete conns[index];	conns[index] = 0;	--nConns;
		CreateListenerConnections();
	}
	// end VS_AccessConnectionSystem_Listener::DeleteListenerConnection

	inline void GoListen( unsigned long bTrans, VS_Overlapped &ov, VS_ConnectionTCP *&conn )
	{
		unsigned long   index = (unsigned long)ov.field3;
		if (index >= maxConns)
		{	dprint4( "ACS: \"index: %lu\">=\"maxConns: %lu\" in function GoListen(...)\n", index, maxConns );
			return;		}
		if (!conns[index])
		{	dprint4( "ACS: \"conns[index]\" is equal to NULL in function \"GoListen(...)\".\n" );
			return;		}
		if (!conns[index]->conn->SetAcceptResult( bTrans, &ov, listener ))
		{	const time_t   tm = time( 0 );	const VS_ConnectionTCP   &conn = *conns[index]->conn;
			dprint3( "ACS: Attempt of ports scanning. Listening: %s:%s, Incoming: %s:%s. %s\n", conn.GetBindIp(), conn.GetBindPort(), conn.GetPeerIp(), conn.GetPeerPort(), ctime( &tm ));
			++nUnsucAttempts;
		} else {	conn = conns[index]->conn;	conns[index]->conn = 0;
					++nRecvConns;	}
		DeleteListenerConnection( index );
	}
	// end VS_AccessConnectionSystem_Listener::GoListen
};
// end VS_AccessConnectionSystem_Listener struct

/////////////////////////////////////////////////////////////////////////////////////////

struct VS_AccessConnectionSystem_Connection
{
	VS_AccessConnectionSystem_Connection( VS_AccessConnectionSystem_ImplementationBase *acs,
					VS_ConnectionTCP *conn, const VS_AccessConnectionSystem_Handler **servs,
					const unsigned long maxServs ) :
		isValid(false), acs(acs), servs(servs), createTm(acs->currTm), maxServs(maxServs),
		createDate(time(0)), nIterations(0), readBytes(0), wroteBytes(0),
		conn(conn), owServs((bool *)malloc( (size_t)( maxServs * 2 ))),
		isOwner(false), indOwner(0), in_buffer(0), in_size(0), in_bytes(0),
		out_buffer(0), out_size(0), context(0)
	{
		if (!owServs)	return;
		reServs = &owServs[maxServs];
		bool   flag = false;
		for (unsigned long i = 0; i < maxServs; ++i)
			if (servs[i])	flag = owServs[i] = reServs[i] = true;
		isValid = flag;
	}
	// end VS_AccessConnectionSystem_Connection::VS_AccessConnectionSystem_Connection

	~VS_AccessConnectionSystem_Connection( void )
	{
		if (conn)
		{
			if (!conn->IsRW())		acs->DeleteConn( conn );
			else
			{
				conn->SetOvReadFields( VS_ACS_CONNECTION_DEAD_READ, (VS_ACS_Field)conn );
				conn->SetOvWriteFields( VS_ACS_CONNECTION_DEAD_WRITE, (VS_ACS_Field)conn );
				conn->Disconnect();
		}	}
		if (in_buffer)		free( in_buffer );
		if (out_buffer)		free( out_buffer );
		if (owServs)		free( (void *)owServs );
	}
	// end VS_AccessConnectionSystem_Connection::~VS_AccessConnectionSystem_Connection

	bool   isValid;
	VS_AccessConnectionSystem_ImplementationBase   *acs;
	const VS_AccessConnectionSystem_Handler   **servs;
	const unsigned long   maxServs, createTm;
	const time_t   createDate;
	unsigned long   nIterations, readBytes, wroteBytes;
	VS_ConnectionTCP   *conn;
	bool   *owServs, *reServs;
	bool   isOwner;
	unsigned long   indOwner;
	void   *in_buffer;
	unsigned long   in_size, in_bytes;
	void   *out_buffer;
	unsigned long   out_size;
	void   *context;

	inline bool InRealloc( const unsigned long new_size )
	{
		if (new_size > in_size)
		{
			void   *new_buffer = malloc( (size_t)new_size );
			if (!new_buffer)	return false;
			if (in_buffer)
			{	memcpy( new_buffer, in_buffer, (size_t)in_bytes );	free( in_buffer );	}
			in_buffer = new_buffer;
			in_size = new_size;
		}
		return true;
	}
	// end VS_AccessConnectionSystem_Connection::InRealloc

	inline bool Refuse( const unsigned long indexServ )
	{
		if (indexServ >= maxServs)	return false;
		owServs[indexServ] = reServs[indexServ] = false;
		for (unsigned long i = 0; i < maxServs; ++i)
			if (owServs[i])		return true;
		return false;
	}
	// end VS_AccessConnectionSystem_Connection::Refuse
};
// end VS_AccessConnectionSystem_Connection struct

/////////////////////////////////////////////////////////////////////////////////////////

#define   MAX_CHK_LSRS   6
#define   MAX_BROKEN_LSNR_PER_ONE_LSNR 300
#define   MAX_BROKEN_LSNR_PER_ALL      3

struct VS_AccessConnectionSystem_Implementation : public VS_AccessConnectionSystem_ImplementationBase,
                                                  public VS_AccessConnectionMonitor
{
	VS_AccessConnectionSystem_Implementation( void )
		: isInit(false), maxServs(0), maxLists(0), maxConns(0), nConns(0)
		, maxIndConns(0), connLifetimeMs(0), servs(0), lists(0), conns(0)
		, tickMs(VS_ACS_TICK_MS), iocpm(0), hiocp(0), hthr(0), ocp(0), icp(0)
		, mcp(0), acmRequest(0), acmReply(0), acmReadState(0), statePeriodic(0)
		, flagMcpConnect(false), flagMcpWrite(false), acmReadSize(0)
		, indexListCount(0), indexConnCount(0), indexHandCount(0),allLsnrsBrokenTick(0),
		prepareToDie(0),m_watchdogSuppression(0)
	{	memset( (void *)chkLsrs, 0, sizeof(chkLsrs) );	Shutdown();		}
	// end VS_AccessConnectionSystem_Implementation::VS_AccessConnectionSystem_Implementation

	~VS_AccessConnectionSystem_Implementation( void ) {		Shutdown();		}
	// end VS_AccessConnectionSystem_Implementation::~VS_AccessConnectionSystem_Implementation
	LONG prepareToDie;
	bool   isInit;
	int allLsnrsBrokenTick;
	const unsigned long   tickMs;
	unsigned long   maxServs, maxLists, maxConns, nConns, maxIndConns, connLifetimeMs;
	VS_AccessConnectionSystem_Handler   **servs;
	VS_AccessConnectionSystem_Listener   **lists;
	VS_AccessConnectionSystem_Connection   **conns;
	VS_IOCPsManager   *iocpm;
	HANDLE   hiocpm, hiocp, hthr;
	VS_ConnectionMsg   *ocp, *icp;	VS_ConnectionByte   *mcp;
	AcmRequest   *acmRequest;	AcmReply   *acmReply;
	unsigned   acmReadState, statePeriodic;		bool   flagMcpConnect, flagMcpWrite;
	unsigned int m_watchdogSuppression;
	unsigned long   acmReadSize, indexListCount, indexConnCount, indexHandCount;
	static unsigned char acsTestAttemptBuf[sizeof(net::HandshakeHeader) + 2];
	enum VS_ACS_Cmd { vs_acs_unknown = 0, vs_acs_start, vs_acs_add_handler,
		vs_acs_remove_handler, vs_acs_remove_all_handlers, vs_acs_add_listener,
		vs_acs_add_listeners, vs_acs_remove_listener, vs_acs_remove_listeners_ip,
		vs_acs_remove_listeners_port, vs_acs_remove_all_listeners, vs_acs_get_listeners,
		vs_acs_shake_listener, vs_acs_set_connection };
	union ControlInquiry
	{	struct CntrlInquiry
		{	VS_ACS_Cmd   cmd;
		};	// end ControlInquiry struct
		struct AddHandler : public CntrlInquiry
		{	char   name[VS_ACS_MAX_SIZE_HANDLER_NAME + 1];
			VS_AccessConnectionHandler   *handler;
			bool   isFinal;
		};	// end AddHandler struct
		struct RemoveHandler : public CntrlInquiry
		{	char   name[VS_ACS_MAX_SIZE_HANDLER_NAME + 1];
		};	// end RemoveHandler struct
		struct AddListeners : public CntrlInquiry
		{	char   endpointName[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
			unsigned   maxConnections;
			bool hidden;
		};	// end AddListeners struct
		struct AddListener : public CntrlInquiry
		{	char   host[24];
			int   port;
			unsigned   maxConnections;
			bool hidden;
		};	// end AddListener struct
		struct RemoveListener : public CntrlInquiry
		{	char   host[24];
			int   port;
		};	// end RemoveListener struct
		struct RemoveListenersIP : public CntrlInquiry
		{	char   host[24];
		};	// end RemoveListenersIP struct
		struct RemoveListenersPort : public CntrlInquiry
		{	int   port;
		};	// end RemoveListenersPort struct
		struct ShakeListener : public CntrlInquiry
		{	char   host[24];
			int   port;
		};	// end ShakeListener struct
		struct SetConnection : public CntrlInquiry
		{	VS_ConnectionTCP   *conn;
		};	// end SetConnection struct
		CntrlInquiry			cntrlInquiry;
		AddHandler				addHandler;
		RemoveHandler			removeHandler;
		AddListeners			addListeners;
		AddListener				addListener;
		RemoveListener			removeListener;
		RemoveListenersIP		removeListenersIP;
		RemoveListenersPort		removeListenersPort;
		ShakeListener			shakeListener;
		SetConnection			setConnection;
	};	// end ControlInquiry union
	union ControlResponse
	{	struct CntrlResponse
		{	VS_ACS_Cmd   cmd;
			bool	res;
		};	// end CntrlResponse struct
		struct AddListeners : public CntrlResponse
		{	unsigned	res;
		};	// end AddListeners struct
		struct GetListeners : public CntrlResponse
		{	char   **hosts;
			int   *ports;
			unsigned   n_listeners;
		};	// end GetListeners struct
		CntrlResponse	cntrlResponse;
		AddListeners	addListeners;
		GetListeners	getListeners;
	};	// end ControlResponse union
	union ControlInquiry   ci;
	union ControlResponse   cr;
	struct ChkLsrs {	char   *host;	int   port, n;		} chkLsrs[MAX_CHK_LSRS];

	inline void ResetToConnectMcp( void )
	{
#ifdef _MY_DEBUG_
puts( "VS_AccessConnectionSystem_Implementation::ResetToConnectMcp()" );
#endif
		flagMcpConnect = flagMcpWrite = false;
		if (mcp)
		{	if (!mcp->IsRW())	DeleteConn( mcp );
			else {	mcp->SetOvReadFields( VS_ACS_CONNECTION_DEAD_READ, (VS_ACS_Field)mcp );
					mcp->SetOvWriteFields( VS_ACS_CONNECTION_DEAD_WRITE, (VS_ACS_Field)mcp );
					mcp->Disconnect();	}
			mcp = 0;	}
		if (acmRequest) {	delete acmRequest;	acmRequest = 0;		}
		if (acmReply) {		delete acmReply;	acmReply = 0;		}
		bool   againFlag = false;
		auto   nameServer = g_tr_endpoint_name;
go_again:
		if (nameServer.empty())	return;
		VS_FilterPath(nameServer.begin(), nameServer.end(), nameServer.begin());
		char   *namePipe = (char *)malloc( 512 );
		if (!namePipe)	return;
		mcp = new VS_ConnectionByte;
		if (!mcp) {		free( (void *)namePipe );	return;		}
		sprintf( namePipe, "%s%s%s\\%s", VS_PipeDir, VS_Servers_PipeDir, nameServer.c_str(), VS_AcsPrefixMonPipe );
		if (!mcp->Create( namePipe, vs_pipe_type_duplex ) || !mcp->SetIOCP( hiocp ))
		{	delete mcp;		mcp = 0;	free( (void *)namePipe );	return;		}
		free( (void *)namePipe );
		mcp->SetOvReadFields( (const VS_ACS_Field)VS_ACS_MONITOR_CONNECT );
		if (!mcp->Connect())
		{
			if (mcp->State() != vs_pipe_state_connected || againFlag)
			{	delete mcp;		mcp = 0;	return;		}
			mcp->SetOvWriteFields( (const VS_ACS_Field)VS_ACS_MONITOR_WRITE );
			mcp->SetOvReadFields( (const VS_ACS_Field)VS_ACS_MONITOR_READ );
			acmRequest = new AcmRequest;
			if (!acmRequest) {		delete mcp;		mcp = 0;	return;		}
			acmReply = new AcmReply;
			if (!acmReply)
			{	delete acmRequest;	acmRequest = 0;		delete mcp;		mcp = 0;	return;		}
			if (!mcp->Read( (void *)&acmRequest->type, sizeof(acmRequest->type) ))
			{	delete acmReply;	acmReply = 0;	delete acmRequest;	acmRequest = 0;
				delete mcp;		mcp = 0;	againFlag = true;	goto go_again;	}
			acmReadState = 0;	acmReadSize = sizeof(acmRequest->type);
			flagMcpConnect = true;
	}	}
	// end VS_AccessConnectionSystem_Implementation::ResetToConnectMcp

	inline void WriteMcp( const void *buf, const unsigned long size )
	{
		if (!mcp->Write( buf, size )) {		ResetToConnectMcp();	return;		}
		flagMcpWrite = true;
	}
	// end VS_AccessConnectionSystem_Implementation::WriteMcp

	inline unsigned long GetFreeIndexHandler( void )
	{
		for (unsigned long i = 0; i < maxServs; ++i)	if (!servs[i])	return i;
		return ~0;
	}
	// end VS_AccessConnectionSystem_Implementation::GetFreeIndexHandler

	inline unsigned long GetIndexHandler( const char *handlerName )
	{
		for (unsigned long i = 0; i < maxServs; ++i)
			if (servs[i] && !strcmp( servs[i]->handlerName, handlerName ))	return i;
		return ~0;
	}
	// end VS_AccessConnectionSystem_Implementation::GetIndexHandler

	inline void DeleteHandler( const unsigned long index )
	{
		const unsigned long   nConns = VS_AccessConnectionSystem_Implementation::nConns;
		for (unsigned long i = 0, j = 0; j < nConns && i < maxConns; ++i)
		{
			VS_AccessConnectionSystem_Connection   *conn = conns[i];
			if (conn)
			{
				if (conn->isOwner && conn->indOwner == index)	DeleteConnection( i );
				++j;
		}	}
		delete servs[index];	servs[index] = 0;
	}
	// end VS_AccessConnectionSystem_Implementation::DeleteHandler

	inline void DeleteAllHandler( void )
	{
		for (unsigned long i = 0; i < maxServs; ++i)
			if (servs[i])	DeleteHandler( i );
	}
	// end VS_AccessConnectionSystem_Implementation::DeleteAllHandler

	inline unsigned long GetFreeIndexListener( void )
	{
		for (unsigned long i = 0; i < maxLists; ++i)	if (!lists[i])	return i;
		return ~0;
	}
	// end VS_AccessConnectionSystem_Implementation::GetFreeIndexListener

	inline void DeleteListener( const unsigned long index )
	{	delete lists[index];	lists[index] = 0;	}
	// end VS_AccessConnectionSystem_Implementation::DeleteListener

	inline bool DeleteHandlerCheckConnection( const unsigned long indexServ, const unsigned long indexConn )
	{	DeleteHandler( indexServ );		return conns[indexConn] != 0;	}
	// end VS_AccessConnectionSystem_Implementation::DeleteHandlerCheckConnection

	inline unsigned long GetFreeIndexConnection( void )
	{
		for (unsigned long i = 0; i < maxConns; ++i)	if (!conns[i])	return i;
		return ~0;
	}
	// end VS_AccessConnectionSystem_Implementation::GetFreeIndexConnection

	inline void	DeleteConnection( const unsigned long index )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::DeleteConnection( index = %u )\n", index );
#endif
		VS_AccessConnectionSystem_Connection   *conn = conns[index];
        if (conn)
        {
		    if (conn->isOwner)
		    {
			    VS_AccessConnectionSystem_Handler   *serv = servs[conn->indOwner];
                if (conn->context)
			        serv->handler->Destructor( conn->context );
		    }
		    delete conns[index];	conns[index] = 0;	--nConns;
		    if (index >= maxIndConns)
		    {	maxIndConns = index;	while (maxIndConns && !conns[--maxIndConns]);	}
        }
	}
	// end VS_AccessConnectionSystem_Implementation::DeleteConnection

	inline void	AcceptConnection( const unsigned long index )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::AcceptConnection( index = %u )\n", index );
#endif

		VS_AccessConnectionSystem_Connection   *conn = conns[index];
		VS_AccessConnectionSystem_Handler   *serv = servs[conn->indOwner];
		if (prepareToDie)
			return;
		serv->handler->Accept( conn->conn, conn->in_buffer, conn->in_bytes, conn->context );
		conn->conn = 0;
		conn->in_bytes = conn->in_size = conn->out_size = 0;
		DeleteConnection( index );
	}
	// end VS_AccessConnectionSystem_Implementation::AcceptConnection

	inline void PrUnrecProtLogDeleteCn( const unsigned long indexConn )
	{
		const VS_AccessConnectionSystem_Connection   &cn = *conns[indexConn];
		if (cn.in_size >= ( sizeof(acsTestAttemptStr) - 1 )
				&& strncmp( (char *)cn.in_buffer, acsTestAttemptStr, sizeof(acsTestAttemptStr) - 1 ))
		{
			const VS_ConnectionTCP   &conn = *cn.conn;		const time_t   tm = time( 0 );
			dprint1( "ACS: Connection of Unrecognized Protocol. Listening: %s:%s, Incoming: %s:%s. Used Server Resources of %lu ms.\n", conn.GetBindIp(), conn.GetBindPort(), conn.GetPeerIp(), conn.GetPeerPort(), currTm - cn.createTm );
		}

		DeleteConnection( indexConn );	return;
	}
	// end VS_AccessConnectionSystem_Implementation::PrUnrecProtLogDeleteCn

	inline void StartConnection( const unsigned long indexConn )
	{
		bool   flag = false;
		unsigned long   maxRead = 0, in_bytes;
		for (unsigned long indexServ = 0; indexServ < maxServs; ++indexServ)
		{
			if (servs[indexServ])
			{
				in_bytes = 0;
				switch (servs[indexServ]->handler->Connection( &in_bytes ))
				{
				case vs_acs_next_step :
					if (!in_bytes || in_bytes > VS_ACS_MAX_BUFFER_LENGTH)
					{	if (!DeleteHandlerCheckConnection( indexServ, indexConn ))	return;		}
					else
					{
						if (in_bytes > maxRead)		maxRead = in_bytes;
						flag = true;
					}
					break;
				case vs_acs_connection_is_not_my :
					if (!conns[indexConn]->Refuse( indexServ ))
					{	PrUnrecProtLogDeleteCn( indexConn );	return;		}
					break;
				case vs_acs_my_connections :
				case vs_acs_free_connections :
				case vs_acs_accept_connections :
				default :
					if (!DeleteHandlerCheckConnection( indexServ, indexConn ))	return;
		}	}	}
		if (!flag || !conns[indexConn]->InRealloc( maxRead )
				|| !conns[indexConn]->conn->Read( conns[indexConn]->in_buffer, maxRead ))
		{	PrUnrecProtLogDeleteCn( indexConn );	return;		}
		++conns[indexConn]->nIterations;
	}
	// end VS_AccessConnectionSystem_Implementation::StartConnection

	inline void ReadUnownedConnection( const unsigned long indexConn )
	{
		bool   flag = false;
		unsigned long   maxRead = 0, in_bytes, out_size;
		void   *out_buffer, *context;
		for (unsigned long indexServ = 0; indexServ < maxServs; ++indexServ)
		{
			if (conns[indexConn]->owServs[indexServ])
			{
				if (!servs[indexServ])
				{
					if (!conns[indexConn]->Refuse( indexServ ))
					{	PrUnrecProtLogDeleteCn( indexConn );	return;		}
					continue;
				}
				in_bytes = conns[indexConn]->in_bytes;
				out_buffer = context = 0;	out_size = 0;
				switch (servs[indexServ]->handler->Protocol( conns[indexConn]->in_buffer, &in_bytes, &out_buffer, &out_size, &context ))
				{
				case vs_acs_next_step :
					if (in_bytes <= conns[indexConn]->in_bytes || out_buffer || out_size || context)
					{	if (!DeleteHandlerCheckConnection( indexServ, indexConn ))	return;		}
					if (in_bytes > maxRead)		maxRead = in_bytes;
					flag = true;
					break;
				case vs_acs_connection_is_not_my :
					if (!conns[indexConn]->Refuse( indexServ ))
					{	PrUnrecProtLogDeleteCn( indexConn );	return;		}
					break;
				case vs_acs_my_connections :
					++servs[indexServ]->nProcConns;
					if ((out_buffer && (!out_size || out_size > VS_ACS_MAX_BUFFER_LENGTH))
							|| (!out_buffer && out_size))
					{	if (!DeleteHandlerCheckConnection( indexServ, indexConn ))	return;		}
					else
					{
						if (out_buffer)
						{
							conns[indexConn]->out_buffer = out_buffer;
							conns[indexConn]->out_size = out_size;
							if (!conns[indexConn]->conn->Write( out_buffer, out_size ))
							{	PrUnrecProtLogDeleteCn( indexConn );	return;		}
							conns[indexConn]->isOwner = true;
							conns[indexConn]->indOwner = indexServ;
							return;
						}
						else if (in_bytes <= conns[indexConn]->in_bytes || in_bytes > VS_ACS_MAX_BUFFER_LENGTH)
						{	if (!DeleteHandlerCheckConnection( indexServ, indexConn ))	return;		}
						else
						{
							if (!conns[indexConn]->InRealloc( in_bytes )
									|| !conns[indexConn]->conn->Read( (void *)&((char *)conns[indexConn]->in_buffer)[conns[indexConn]->in_bytes], in_bytes - conns[indexConn]->in_bytes ))
							{	PrUnrecProtLogDeleteCn( indexConn );	return;		}
							conns[indexConn]->isOwner = true;
							conns[indexConn]->indOwner = indexServ;
							return;
					}	}
					break;
				case vs_acs_free_connections :
					if (!DeleteHandlerCheckConnection( indexServ, indexConn ))	return;
					break;
				case vs_acs_accept_connections :
					++servs[indexServ]->nProcConns;
					++servs[indexServ]->nAccConns;
					conns[indexConn]->indOwner = indexServ;
					AcceptConnection( indexConn );
					return;
				default :	if (!DeleteHandlerCheckConnection( indexServ, indexConn ))	return;
		}	}	}
		if (!flag || !conns[indexConn]->InRealloc( maxRead )
			|| !conns[indexConn]->conn->Read( (void *)&((char *)conns[indexConn]->in_buffer)[conns[indexConn]->in_bytes],
				maxRead - conns[indexConn]->in_bytes ))
		{	PrUnrecProtLogDeleteCn( indexConn );	return;		}
		++conns[indexConn]->nIterations;
	}
	// end VS_AccessConnectionSystem_Implementation::ReadUnownedConnection

	inline void ReadWriteOwnerConnection( const unsigned long indexConn )
	{
		const unsigned long   indexServ = conns[indexConn]->indOwner;
		if (indexServ >= maxServs || !servs[indexServ])
		{	DeleteConnection( indexConn );	return;		}
		unsigned long   in_bytes = conns[indexConn]->in_bytes;
		void   *out_buffer = conns[indexConn]->out_buffer;
		unsigned long   out_size = conns[indexConn]->out_size;
		switch (servs[indexServ]->handler->Protocol( conns[indexConn]->in_buffer, &in_bytes, &out_buffer, &out_size, &conns[indexConn]->context ))
		{
		case vs_acs_next_step :
		case vs_acs_my_connections :
			if ((out_buffer && !out_size) || (!out_buffer && out_size))
				DeleteHandlerCheckConnection( indexServ, indexConn );
			else if (out_buffer)
			{
				conns[indexConn]->out_buffer = out_buffer;
				conns[indexConn]->out_size = out_size;
				if (!conns[indexConn]->conn->Write( out_buffer, out_size ))
				{	PrUnrecProtLogDeleteCn( indexConn );	return;		}
				++conns[indexConn]->nIterations;
			}
			else if (in_bytes <= conns[indexConn]->in_bytes || in_bytes > VS_ACS_MAX_BUFFER_LENGTH)
				DeleteHandlerCheckConnection( indexServ, indexConn );
			else
			{
				if (!conns[indexConn]->InRealloc( in_bytes )
						|| !conns[indexConn]->conn->Read( (void *)&((char *)conns[indexConn]->in_buffer)[conns[indexConn]->in_bytes], in_bytes - conns[indexConn]->in_bytes ))
				{	PrUnrecProtLogDeleteCn( indexConn );	return;		}
				++conns[indexConn]->nIterations;
			}
			return;
		case vs_acs_connection_is_not_my :
		case vs_acs_free_connections :		PrUnrecProtLogDeleteCn( indexConn );	return;
		case vs_acs_accept_connections :
			++servs[indexServ]->nAccConns;
			AcceptConnection( indexConn );	return;
		default :	DeleteHandler( indexServ );		return;
	}	}
	// end VS_AccessConnectionSystem_Implementation::ReadWriteOwnerConnection

	inline bool GoAddHandler( const unsigned long size )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoAddHandler( size = %u )\n", size );
#endif
		if (size != sizeof(ControlInquiry::AddHandler))
		{	dprint4( "ACS: \"size: %lu\" != \"sizeof(ControlInquiry::AddHandler): %u\" in function \"GoAddHandler(...)\".\n", size, sizeof(ControlInquiry::AddHandler) );
			return false;	}
		cr.cntrlResponse.cmd = vs_acs_add_handler;
		cr.cntrlResponse.res = false;
		const char   *name = ci.addHandler.name;
		VS_AccessConnectionHandler   *handler = ci.addHandler.handler;
		const bool   isFinal = ci.addHandler.isFinal;
		const unsigned long   index = GetFreeIndexHandler();
		if (index < maxServs)
		{
			VS_AccessConnectionSystem_Handler   *serv = new VS_AccessConnectionSystem_Handler( this, name, handler, isFinal );
			if (serv)
			{
				if (!serv->isValid)		delete serv;
				else {	servs[index] = serv;	cr.cntrlResponse.res = true;	}
		}	}
		return icp->Write( (const void *)&cr.cntrlResponse, sizeof(cr.cntrlResponse) );
	}
	// end VS_AccessConnectionSystem_Implementation::GoAddHandler

	inline bool GoRemoveHandler( const unsigned long size )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoRemoveHandler( size = %u )\n", size );
#endif
		if (size != sizeof(ControlInquiry::RemoveHandler))
		{	dprint4( "ACS: \"size: %lu\" != \"sizeof(ControlInquiry::RemoveHandler): %u\" in function \"GoRemoveHandler(...)\".\n", size, sizeof(ControlInquiry::RemoveHandler) );
			return false;	}
		cr.cntrlResponse.cmd = vs_acs_remove_handler;
		const unsigned long   index = GetIndexHandler( ci.removeHandler.name );
		if (index < maxServs)	DeleteHandler( index );
		return icp->Write( (const void *)&cr.cntrlResponse.cmd, sizeof(cr.cntrlResponse.cmd) );
	}
	// end VS_AccessConnectionSystem_Implementation::GoRemoveHandler
	inline void GetRegWatchdogSuppression()
	{
            VS_RegistryKey key(false, CONFIGURATION_KEY);
            unsigned int   L = 0;
			if (key.GetValue(&L, sizeof(L), VS_REG_INTEGER_VT, SUPPRESS_ACS_WATCHDOG_TAG_NAME) == sizeof(L))
			{
				m_watchdogSuppression = L;
			}

	}
	inline bool GoRemoveAllHandlers( const unsigned long size )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoRemoveAllHandlers( size = %u )\n", size );
#endif
		if (size != sizeof(VS_ACS_Cmd))
		{	dprint4( "ACS: \"size: %lu\" != \"sizeof(VS_ACS_Cmd): %u\" in function \"GoRemoveAllHandlers(...)\".\n", size, sizeof(VS_ACS_Cmd) );
			return false;	}
		cr.cntrlResponse.cmd = vs_acs_remove_handler;
		DeleteAllHandler();
		return icp->Write( (const void *)&cr.cntrlResponse.cmd, sizeof(cr.cntrlResponse.cmd) );
	}
	// end VS_AccessConnectionSystem_Implementation::GoRemoveAllHandlers

	inline bool GoAddListeners( const unsigned long size )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoAddListeners( size = %u )\n", size );
#endif
		if (size != sizeof(ControlInquiry::AddListeners))
		{	dprint4( "ACS: \"size: %lu\" != \"sizeof(ControlInquiry::AddListeners): %u\" in function \"GoAddListeners(...)\".\n", size, sizeof(ControlInquiry::AddListeners) );
			return false;	}
		const char   *endpointName = ci.addListeners.endpointName;
		const unsigned   maxConnections = ci.addListeners.maxConnections;
		const bool hidden = ci.addListeners.hidden;
		cr.addListeners.cmd = vs_acs_add_listeners;
		cr.addListeners.res = 0;
		const unsigned number = net::endpoint::GetCountAcceptTCP(endpointName, false);
		for (unsigned i = 1; i <= number; ++i)
		{
			auto rat = net::endpoint::ReadAcceptTCP(i, endpointName, false);
			if (!rat)	continue;
			const unsigned long   index = GetFreeIndexListener();
			if (index >= maxLists)	break;
			VS_AccessConnectionSystem_Listener   *list = new VS_AccessConnectionSystem_Listener(this, rat->host.c_str(), rat->port, maxConnections, index, hiocp, hidden);
			if (list) {		if (!list->isValid)		delete list;
							else {	lists[index] = list;	++cr.addListeners.res;	}
		}	}
		return icp->Write( (const void *)&cr.addListeners, sizeof(cr.addListeners) );
	}
	// end VS_AccessConnectionSystem_Implementation::GoAddListeners

	inline bool GoAddListener( const unsigned long size )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoAddListener( size = %u )\n", size );
#endif
		if (size != sizeof(ControlInquiry::AddListener))
		{	dprint4( "ACS: \"size: %lu\" != \"sizeof(ControlInquiry::AddListener): %u\" in function \"GoAddListener(...)\".\n", size, sizeof(ControlInquiry::AddListener) );
			return false;	}
		const char   *host = ci.addListener.host;
		const int   port = ci.addListener.port;
		const unsigned   maxConnections = ci.addListener.maxConnections;
		const bool hidden = ci.addListener.hidden;
		cr.cntrlResponse.cmd = vs_acs_add_listener;
		cr.cntrlResponse.res = false;
		const unsigned long   index = GetFreeIndexListener();
		if (index < maxLists)
		{
			VS_AccessConnectionSystem_Listener   *list = new VS_AccessConnectionSystem_Listener( this, host, port, maxConnections, index, hiocp,hidden );
			if (list)
			{
				if (!list->isValid)		delete list;
				else {	lists[index] = list;	cr.cntrlResponse.res = true;	}
		}	}
		return icp->Write( (const void *)&cr.cntrlResponse, sizeof(cr.cntrlResponse) );
	}
	// end VS_AccessConnectionSystem_Implementation::GoAddListener

	inline bool GoRemoveListener( const unsigned long size )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoRemoveListener( size = %u )\n", size );
#endif
		if (size != sizeof(ControlInquiry::RemoveListener))
		{	dprint4( "ACS: \"size: %lu\" != \"sizeof(ControlInquiry::RemoveListener): %u\" in function \"GoRemoveListener(...)\".\n", size, sizeof(ControlInquiry::RemoveListener) );
			return false;	}
		const char   *host = ci.removeListener.host;
		const int   port = ci.removeListener.port;
		for (unsigned long i = 0; i < maxLists; ++i)
			if (lists[i])
			{
				VS_AccessConnectionSystem_Listener   *list = lists[i];
				if (!strcmp( host, list->host ) && port == list->port )
					DeleteListener( i );
			}
		return true;
	}
	// end VS_AccessConnectionSystem_Implementation::GoRemoveListener

	inline bool GoRemoveListenersIp( const unsigned long size )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoRemoveListenersIp( size = %u )\n", size );
#endif
		if (size != sizeof(ControlInquiry::RemoveListenersIP))
		{	dprint4( "ACS: \"size: %lu\" != \"sizeof(ControlInquiry::RemoveListenersIP): %u\" in function \"GoRemoveListenersIp(...)\".\n", size, sizeof(ControlInquiry::RemoveListenersIP) );
			return false;	}
		const char   *host = ci.removeListenersIP.host;
		for (unsigned long i = 0; i < maxLists; ++i)
			if (lists[i])
			{
				VS_AccessConnectionSystem_Listener   *list = lists[i];
				if (!strcmp( host, list->host ))	DeleteListener( i );
			}
		return true;
	}
	// end VS_AccessConnectionSystem_Implementation::GoRemoveListenerIp

	inline bool GoRemoveListenersPort( const unsigned long size )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoRemoveListenersPort( size = %u )\n", size );
#endif
		if (size != sizeof(ControlInquiry::RemoveListenersPort))
		{	dprint4( "ACS: \"size: %lu\" != \"sizeof(ControlInquiry::RemoveListenersPort): %u\" in function \"GoRemoveListenersPort(...)\".\n", size, sizeof(ControlInquiry::RemoveListenersPort) );
			return false;	}
		const int   port = ci.removeListenersPort.port;
		for (unsigned long i = 0; i < maxLists; ++i)
			if (lists[i])
			{
				VS_AccessConnectionSystem_Listener   *list = lists[i];
				if (port == list->port)		DeleteListener( i );
			}
		return true;
	}
	// end VS_AccessConnectionSystem_Implementation::GoRemoveListenerPort

	inline bool GoRemoveAllListeners( const unsigned long size )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoRemoveAllListeners( size = %u )\n", size );
#endif
		if (size != sizeof(VS_ACS_Cmd))
		{	dprint4( "ACS: \"size: %lu\" != \"sizeof(VS_ACS_Cmd): %u\" in function \"GoRemoveAllListeners(...)\".\n", size, sizeof(VS_ACS_Cmd) );
			return false;	}
		for (unsigned long i = 0; i < maxLists; ++i)	if (lists[i])	DeleteListener( i );
		return true;
	}
	// end VS_AccessConnectionSystem_Implementation::GoRemoveListener
	inline char * GetIPS()
	{
		char * host = new char[256];
		int i = 0;
		int net_interface = 5;
		char *ips_my[5] = {0,0,0,0,0};
		if (VS_GetDefaultHostName(host,256))
		{
			unsigned ips_count = 0;


			for(;i<net_interface;i++)
			{
				ips_my[i] = new char[64];
				memset(ips_my[i],0,64);
			}
			if(!(ips_count = VS_GetHostsByName(host,ips_my,net_interface,64)))
			{
				for(i =0;i<net_interface;i++)
				{
					if (ips_my[i])
					{
						delete [] ips_my[i];
						ips_my[i] = 0;
					}
				}
				if (host)
				{
					delete [] host;
					host = 0;
				}
				return _strdup("127.0.0.1");
			}else
			{
				unsigned long   ip = 0;
				for(i =0;i<net_interface;i++)
				{
					if (ips_my[i] && ips_my[i][0])
					if(VS_GetIpByHost(ips_my[i],&ip))
					{
						if(ip >0x01000000)
						{
							char * ptr = _strdup(ips_my[i]);
							for(i =0;i<net_interface;i++)
							{
								if (ips_my[i])
								{
									delete [] ips_my[i];
									ips_my[i] = 0;
								}
							}
							if (host)
							{
								delete [] host;
								host = 0;
							}
							return ptr;
						}
					}
				}
			}
		}
		for(i =0;i<net_interface;i++)
		{
			delete [] ips_my[i];
		}
		if (host)
		{
			delete [] host;
			host = 0;
		}
		return _strdup("127.0.0.1");
	}
	inline bool GoGetListeners( const unsigned long size )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoGetListeners( size = %u )\n", size );
#endif
		if (size != sizeof(VS_ACS_Cmd))
		{	dprint4( "ACS: \"size: %lu\" != \"sizeof(VS_ACS_Cmd): %u\" in function \"GoGetListeners(...)\".\n", size, sizeof(VS_ACS_Cmd) );
			return false;	}
		memset( (void *)&cr.getListeners, 0, sizeof(cr.getListeners) );
		cr.getListeners.cmd = vs_acs_get_listeners;
		cr.getListeners.res = false;
		size_t   sz = sizeof(char *) * maxLists;
		cr.getListeners.hosts = (char **)malloc( sz );
		if (cr.getListeners.hosts)
		{	memset( (void *)cr.getListeners.hosts, 0, sz );
			sz = sizeof(int) * maxLists;
			cr.getListeners.ports = (int *)malloc( sz );
			if (cr.getListeners.ports)
			{
				memset( cr.getListeners.ports, 0, sz );
				unsigned n = 0;
				for (unsigned i = 0; i < maxLists; ++i)
				{	VS_AccessConnectionSystem_Listener   *list = lists[i];
					if (list&&list->hidden == false)
					{	if (list->host)
						{
							if (_stricmp(list->host,"0.0.0.0")==0)
							{
								cr.getListeners.hosts[n] = GetIPS();
							}
							else
							{
								cr.getListeners.hosts[n] = _strdup( list->host );
							}

						}
						cr.getListeners.ports[n] = list->port;		++n;
					}
				}
				cr.getListeners.n_listeners = n;	cr.getListeners.res = true;
		}	}
		return icp->Write( (const void *)&cr.getListeners, sizeof(cr.getListeners) );
	}
	// end VS_AccessConnectionSystem_Implementation::GoGetListeners

	inline bool GoShakeListener( const unsigned long size )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoShakeListener( size = %u )\n", size );
#endif
		if (size != sizeof(ControlInquiry::ShakeListener))
		{	dprint4( "ACS: \"size: %lu\" != \"sizeof(ControlInquiry::ShakeListener): %u\" in function \"GoShakeListener(...)\".\n", size, sizeof(ControlInquiry::ShakeListener) );
			return false;	}
		const char   *host = ci.shakeListener.host;
		const int   port = ci.shakeListener.port;
		cr.cntrlResponse.cmd = vs_acs_shake_listener;
		cr.cntrlResponse.res = false;
		for (unsigned long i = 0; i < maxLists; ++i)
		{	if (lists[i])
			{	VS_AccessConnectionSystem_Listener   *list = lists[i];
				if (!strcmp( host, list->host ) && port == list->port )
				{	const unsigned long   maxConns = list->maxConns;	DeleteListener( i );
					VS_AccessConnectionSystem_Listener   *list = new VS_AccessConnectionSystem_Listener( this, host, port, maxConns, i, hiocp,false);
					if (list)
					{	if (!list->isValid)		delete list;
						else {	lists[i] = list;	cr.cntrlResponse.res = true;	break;	}
		}	}	}	}
		return icp->Write( (const void *)&cr.cntrlResponse, sizeof(cr.cntrlResponse) );
	}
	// end VS_AccessConnectionSystem_Implementation::GoShakeListener

	inline bool GoSetConnection( const unsigned long size )
	{
		if (size != sizeof(ControlInquiry::SetConnection))
		{	dprint4( "ACS: \"size: %lu\" != \"sizeof(ControlInquiry::SetConnection): %u\" in function \"GoShakeListener(...)\".\n", size, sizeof(ControlInquiry::SetConnection) );
			return false;	}
		VS_ConnectionTCP   *connTCP = ci.setConnection.conn;
		if (!connTCP)
		{	dprint4( "ACS: For Set Connection connTCP equal ZERO.\n" );
			return false;	}




		if (nConns >= maxConns) {	dprint4( "ACS: \"nConns: %lu\" >= \"maxConns: %lu\" in function \"GoSetConnection(...)\".\n", nConns, maxConns );
									DeleteConn( connTCP );	return true;	}
		VS_AccessConnectionSystem_Connection   *conn = new VS_AccessConnectionSystem_Connection(
			this, connTCP, (const VS_AccessConnectionSystem_Handler **)servs, maxServs );
		if (!conn) {	dprint4( "ACS: \"conn\" is equal to NULL in function \"GoSetConnection(...)\".\n" );
						DeleteConn( connTCP );	return true;	}
		if (!conn->isValid) {	dprint4( "ACS: \"conn\" is not initialized in function \"GoSetConnection(...)\".\n" );
								delete conn;	return true;	}
		const unsigned long   indexConn = GetFreeIndexConnection();
		conns[indexConn] = conn;	++nConns;
		if (indexConn > maxIndConns)	maxIndConns = indexConn;
		connTCP->SetOvWriteFields( VS_ACS_CONNECTION_WRITE, indexConn );
		connTCP->SetOvReadFields( VS_ACS_CONNECTION_READ, indexConn );
		connTCP->SetOvFildIOCP( hiocp );
		StartConnection( indexConn );


		return true;
	}
	// end VS_AccessConnectionSystem_Implementation::GoSetConnection

	inline bool GoControlInquiry( const unsigned long size )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoControlInquiry( size = %u )\n", size );
#endif
		switch (ci.cntrlInquiry.cmd)
		{
		case vs_acs_add_handler :			if (!GoAddHandler( size ))			return false;
											break;
		case vs_acs_remove_handler :		if (!GoRemoveHandler( size ))		return false;
											break;
		case vs_acs_remove_all_handlers :	if (!GoRemoveAllHandlers( size ))	return false;
											break;
		case vs_acs_add_listeners :			if (!GoAddListeners( size ))		return false;
											break;
		case vs_acs_add_listener :			if (!GoAddListener( size ))			return false;
											break;
		case vs_acs_remove_listener :		if (!GoRemoveListener( size ))		return false;
											break;
		case vs_acs_remove_listeners_ip :	if (!GoRemoveListenersIp( size ))	return false;
											break;
		case vs_acs_remove_listeners_port :	if (!GoRemoveListenersPort( size ))	return false;
											break;
		case vs_acs_remove_all_listeners :	if (!GoRemoveAllListeners( size ))	return false;
											break;
		case vs_acs_get_listeners :			if (!GoGetListeners( size ))		return false;
											break;
		case vs_acs_shake_listener :		if (!GoShakeListener( size ))		return false;
											break;
		case vs_acs_set_connection :		if (!GoSetConnection( size ))		return false;
											break;
		default :	return false;
		}
		memset( (void *)&ci, 0, sizeof(ci) );
		return !icp->IsWrite() ? icp->Read( (void *)&ci, sizeof(ControlInquiry) ) : true;
	}
	// end VS_AccessConnectionSystem_Implementation::GoControlInquiry

	inline void GoMonitorInquiry( const unsigned long size )
	{
		if (size != acmReadSize)
		{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoMonitorInquiry: if (size != acmReadSize)\n" );
#endif
			ResetToConnectMcp();	return;
		}
		switch (acmRequest->type)
		{
		case ACM_TYPE_UNKNOWN :	break;
		default:	ResetToConnectMcp();
	}	}
	// end VS_AccessConnectionSystem_Implementation::GoMonitorInquiry

	inline void GoListen( unsigned long bTrans, VS_Overlapped &ov )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoListen( bTrans: %u, &ov: %08X\n", bTrans, _MD_POINT_(&ov) );
#endif
		const unsigned long   indexList = (unsigned long)ov.field2;
		if (indexList > maxLists) {		dprint4( "ACS: \"indexList: %lu\" >= \"maxLists: %lu\" in function \"GoListen(...)\".\n", indexList, maxLists );
										return;		}
		VS_AccessConnectionSystem_Listener   *list = lists[indexList];
		if (!list) {	dprint4( "ACS: \"list\" is equal to NULL in function \"GoListen(...)\".\n" );
						return;		}
		VS_ConnectionTCP   *connTCP = 0;
		list->GoListen( bTrans, ov, connTCP );
		if (!connTCP)	return;
		if (nConns >= maxConns) {	dprint4( "ACS: \"nConns: %lu\" >= \"maxConns: %lu\" in function \"GoListen(...)\".\n", nConns, maxConns );
									DeleteConn( connTCP );	return;		}
		VS_AccessConnectionSystem_Connection   *conn = new VS_AccessConnectionSystem_Connection(
			this, connTCP, (const VS_AccessConnectionSystem_Handler **)servs, maxServs );
		if (!conn) {	dprint4( "ACS: \"conn\" is equal to NULL in function \"GoListen(...)\".\n" );
						DeleteConn( connTCP );	return;		}
		if (!conn->isValid) {	dprint4( "ACS: \"conn\" is not initialized in function \"GoListen(...)\".\n" );
								delete conn;	return;		}
		const unsigned long   indexConn = GetFreeIndexConnection();
		conns[indexConn] = conn;	++nConns;
		if (indexConn > maxIndConns)	maxIndConns = indexConn;
		connTCP->SetOvWriteFields( VS_ACS_CONNECTION_WRITE, indexConn );
		connTCP->SetOvReadFields( VS_ACS_CONNECTION_READ, indexConn );
		if (!connTCP->SetIOCP( hiocpm )) {	dprint4( "ACS: Could not adhere to \"connTCP\" the \"hiocpm\" in function \"GoListen(...)\".\n" );
											delete conn;	return;		}
		connTCP->SetOvFildIOCP( hiocp );
		StartConnection( indexConn );
	}
	// end VS_AccessConnectionSystem_Implementation::GoListen

	inline void GoListenDead( unsigned long bTrans, VS_Overlapped &ov )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoListenDead( bTrans: %u, &ov: %08X\n", bTrans, _MD_POINT_(&ov) );
#endif
		VS_ConnectionSock   *conn = (VS_ConnectionSock *)ov.field2;
		if (!conn) {	dprint4( "ACS: \"conn\" is equal to NULL in function \"GoListenDead(...)\".\n" );
						return;		}
		DeleteConn( conn );
	}
	// end VS_AccessConnectionSystem_Implementation::GoListenDead

	inline void GoConnectionWrite( unsigned long bTrans, VS_Overlapped &ov )
	{
		unsigned long   indexConn = (unsigned long)ov.field2;
		if (indexConn >= maxConns) {	dprint4( "ACS: \"indexConn: %lu >= maxConns: %lu\" in function \"GoConnectionWrite(...)\".\n", indexConn, maxConns );
										return;		}
		VS_AccessConnectionSystem_Connection   *conn = conns[indexConn];
		if (!conn) {	dprint4( "ACS: \"conn\" is equal to NULL in function \"GoConnectionWrite(...)\".\n" );
						return;		}
		if (!conn->isOwner) {	dprint4( "ACS: \"conn\" does not belong to anybody, it is inadmissible for \"GoConnectionWrite(...)\".\n" );
								DeleteConnection( indexConn );	return;		}
		int   ret = conn->conn->SetWriteResult( bTrans, &ov );
		if (ret <= 0) {		DeleteConnection( indexConn );	return;		}
		conn->wroteBytes += ret;
		ReadWriteOwnerConnection( indexConn );
	}
	// end VS_AccessConnectionSystem_Implementation::GoConnectionWrite

	inline void GoConnectionRead( unsigned long bTrans, VS_Overlapped &ov )
	{
		unsigned long   indexConn = (unsigned long)ov.field2;
		if (indexConn >= maxConns) {	dprint4( "ACS: \"indexConn: %lu >= maxConns: %lu\" in function \"GoConnectionRead(...)\".\n", indexConn, maxConns );
										return;		}
		VS_AccessConnectionSystem_Connection   *conn = conns[indexConn];
		if (!conn) {	dprint4( "ACS: \"conn\" is equal to NULL in function \"GoConnectionRead(...)\".\n" );
						return;		}
		int   ret = conn->conn->SetReadResult( bTrans, &ov, 0, true );
		if (ret <= 0) {		DeleteConnection( indexConn );	return;		}
		conn->in_bytes += ret;
		conn->readBytes += ret;
		if (!conn->isOwner)		ReadUnownedConnection( indexConn );
		else					ReadWriteOwnerConnection( indexConn );
	}
	// end VS_AccessConnectionSystem_Implementation::GoConnectionRead

	inline void GoConnectionDeadWrite( unsigned long bTrans, VS_Overlapped &ov )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoConnectionDeadWrite( bTrans: %u, &ov: %08X\n", bTrans, _MD_POINT_(&ov) );
#endif
		VS_ConnectionSock   *conn = (VS_ConnectionSock *)ov.field2;
		if (!conn) {	dprint4( "ACS: \"conn\" is equal to NULL in function \"GoConnectionDeadWrite(...)\".\n" );
						return;		}
		conn->SetWriteResult( bTrans, &ov );
		if (!conn->IsRead())	DeleteConn( conn );
		else
		{
			dprint4( "ACS: GoConnectionDeadWrite !IsRead. Connection cannot be deleted. It is a memory leak.\n" );
		}
	}
	// end VS_TransportRouter_Implementation::GoConnectionWrite

	inline void GoConnectionDeadRead( unsigned long bTrans, VS_Overlapped &ov )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoConnectionDeadRead( bTrans: %u, &ov: %08X\n", bTrans, _MD_POINT_(&ov) );
#endif
		VS_ConnectionSock   *conn = (VS_ConnectionSock *)ov.field2;
		if (!conn) {	dprint4( "ACS: \"conn\" is equal to NULL in function \"GoConnectionDeadRead(...)\".\n" );
						return;		}
		conn->SetReadResult( bTrans, &ov );
		if (!conn->IsWrite())	DeleteConn( conn );
		else {
			dprint4( "ACS: GoConnectionDeadRead !IsWrite. Connection cannot be deleted. It is a memory leak.\n" );
		}
	}
	// end VS_TransportRouter_Implementation::GoConnectionRead

	inline bool GoControlWrite( unsigned long bTrans, VS_Overlapped &ov )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoControlWrite( bTrans: %u, &ov: %08X\n", bTrans, _MD_POINT_(&ov) );
#endif
		if (ov.error)
		{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoControlWrite: ov.error = %u\n", ov.error );
#endif
			return false;
		}
		if (icp->SetWriteResult( bTrans, &ov ) < 0)
		{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoControlWrite: if (icp->SetWriteResult( bTrans, &ov ) < 0)\n" );
#endif
			return false;
		}
		return icp->Read( (void *)&ci, sizeof(ControlInquiry) );
	}
	// end VS_AccessConnectionSystem_Implementation::GoControlWrite

	inline bool GoControlRead( unsigned long bTrans, VS_Overlapped &ov )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoControlRead( bTrans: %u, &ov: %08X\n", bTrans, _MD_POINT_(&ov) );
#endif
		if (ov.error)
		{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoControlRead: ov.error = %u\n", ov.error );
#endif
			return false;
		}
		int   ret = icp->SetReadResult( bTrans, &ov, 0, true );
		if (ret < (int)sizeof(ci.cntrlInquiry.cmd))
		{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::GoControlRead: if (ret < (int)sizeof(ci.cntrlInquiry.cmd))\n" );
#endif
			return false;
		}
		return GoControlInquiry( (const unsigned long)ret );
	}
	// end VS_AccessConnectionSystem_Implementation::GoControlRead

	inline void GoMonitorWrite( unsigned long bTrans, VS_Overlapped &ov )
	{
		if (mcp->SetWriteResult( bTrans, &ov ) < 0)		ResetToConnectMcp();
		flagMcpWrite = false;
	}
	// end VS_AccessConnectionSystem_Implementation::GoMonitorWrite

	inline void GoMonitorRead( unsigned long bTrans, VS_Overlapped &ov )
	{
		GoMonitorInquiry( (const unsigned long)mcp->SetReadResult( bTrans, &ov ) );
	}
	// end VS_AccessConnectionSystem_Implementation::GoMonitorRead

	inline void GoMonitorConnect( unsigned long bTrans, VS_Overlapped &ov )
	{
		if (mcp->SetConnectResult( bTrans, &ov ))
		{
			mcp->SetOvWriteFields( (const VS_ACS_Field)VS_ACS_MONITOR_WRITE );
			mcp->SetOvReadFields( (const VS_ACS_Field)VS_ACS_MONITOR_READ );
			acmRequest = new AcmRequest;
			if (!acmRequest) {	ResetToConnectMcp();	return;		}
			acmReply = new AcmReply;
			if (!acmReply) {	ResetToConnectMcp();	return;		}
			if (!mcp->Read( (void *)&acmRequest->type, sizeof(acmRequest->type) ))
			{	ResetToConnectMcp();	return;		}
			acmReadState = 0;	acmReadSize = sizeof(acmRequest->type);
			flagMcpConnect = true;
	}	}
	// end VS_AccessConnectionSystem_Implementation::GoMonitorConnect

	inline bool RunHandle( unsigned long bTrans, VS_Overlapped &ov )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::RunHandle( bTrans: %u, &ov: %08X\n", bTrans, _MD_POINT_(&ov) );
#endif
		switch ((unsigned long)ov.field1)
		{
		case VS_ACS_LISTEN                : GoListen( bTrans, ov );              return true;
		case VS_ACS_LISTEN_DEAD           : GoListenDead( bTrans, ov );          return true;
		case VS_ACS_CONNECTION_WRITE      : GoConnectionWrite( bTrans, ov );     return true;
		case VS_ACS_CONNECTION_READ       : GoConnectionRead( bTrans, ov );      return true;
		case VS_ACS_CONNECTION_DEAD_WRITE : GoConnectionDeadWrite( bTrans, ov ); return true;
		case VS_ACS_CONNECTION_DEAD_READ  : GoConnectionDeadRead( bTrans, ov );  return true;
		case VS_ACS_CONTROL_WRITE         : return GoControlWrite( bTrans, ov );
		case VS_ACS_CONTROL_READ          : return GoControlRead( bTrans, ov );
		case VS_ACS_MONITOR_WRITE         : GoMonitorWrite( bTrans, ov );        return true;
		case VS_ACS_MONITOR_READ          : GoMonitorRead( bTrans, ov );         return true;
		case VS_ACS_MONITOR_CONNECT       : GoMonitorConnect( bTrans, ov );      return true;
		default :	/* Here it will be necessary to throw off in TRACE */        return true;
	}	}
	// end VS_AccessConnectionSystem_Implementation::RunHandle

	inline void ProcessingPeriodic( void )
	{
		if (!mcp)	ResetToConnectMcp();
		if (flagMcpWrite)	return;
		switch (statePeriodic)
		{
		case 0 :
			if (flagMcpConnect)
			{
#ifdef _MY_DEBUG_
puts( "VS_AccessConnectionSystem_Implementation::ProcessingPeriodic: case 0: WriteMcp( ... )" );
#endif
				acmReply->type = ACM_TYPE_PERIODIC_START;
				WriteMcp( (const void *)acmReply, sizeof(acmReply->type) );
			}
			statePeriodic = 1;		return;
		case 1 :
			if (flagMcpConnect)
			{
#ifdef _MY_DEBUG_
puts( "VS_AccessConnectionSystem_Implementation::ProcessingPeriodic: case 1: WriteMcp( ... )" );
#endif
				AcmReply::StartListeners   &startListeners = acmReply->startListeners;
				startListeners.type = ACM_TYPE_PERIODIC_START_LISTENERS;
				startListeners.maxListeners = maxLists;
				WriteMcp( (const void *)acmReply, sizeof(startListeners) );
			}
			statePeriodic = 2;	indexListCount = ~0;	return;
		case 2 :
			while (++indexListCount < maxLists)
			{
				VS_AccessConnectionSystem_Listener   *list = lists[indexListCount];
				if (!list)		continue;
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::ProcessingPeriodic: indexListCount: %u\n", indexListCount );
#endif
				if (flagMcpConnect)
				{
#ifdef _MY_DEBUG_
puts( "VS_AccessConnectionSystem_Implementation::ProcessingPeriodic: case 2: WriteMcp( ... )" );
#endif
					AcmReply::Listener   &listener = acmReply->listener;
					listener.type = ACM_TYPE_PERIODIC_LISTENER;
					listener.maxConns = list->maxConns;
					listener.nConns = list->nConns;
					listener.nRecvConns = list->nRecvConns;
					listener.nUnsucAttempts = list->nUnsucAttempts;
					*listener.host = 0;
					if (list->host)
					{	strncpy( listener.host, list->host, sizeof(listener.host) - 1 );
						listener.host[sizeof(listener.host) - 1] = 0;	}
					listener.port = (unsigned short)list->port;
					WriteMcp( (const void *)acmReply, sizeof(acmReply->listener) );
				}
				return;
			}
			statePeriodic = 3;		return;
		case 3 :
			if (flagMcpConnect)
			{
#ifdef _MY_DEBUG_
puts( "VS_AccessConnectionSystem_Implementation::ProcessingPeriodic: case 3: WriteMcp( ... )" );
#endif
				acmReply->type = ACM_TYPE_PERIODIC_STOP_LISTENERS;
				WriteMcp( (const void *)acmReply, sizeof(acmReply->type) );
			}
			statePeriodic = 4;		return;
		case 4 :
			if (flagMcpConnect)
			{
#ifdef _MY_DEBUG_
puts( "VS_AccessConnectionSystem_Implementation::ProcessingPeriodic: case 4: WriteMcp( ... )" );
#endif
				AcmReply::StartConnections   &startConnections = acmReply->startConnections;
				startConnections.type = ACM_TYPE_PERIODIC_START_CONNECTIONS;
				startConnections.maxConnections = maxConns;
				WriteMcp( (const void *)acmReply, sizeof(startConnections) );
			}
			statePeriodic = 5;	indexConnCount = ~0;	return;
		case 5 :
			while (++indexConnCount <= maxIndConns)
			{
				VS_AccessConnectionSystem_Connection   *conn = conns[indexConnCount];
				if (!conn)		continue;
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::ProcessingPeriodic: indexConnCount: %u\n", indexConnCount );
#endif
				if (flagMcpConnect)
				{
#ifdef _MY_DEBUG_
puts( "VS_AccessConnectionSystem_Implementation::ProcessingPeriodic: case 5: WriteMcp( ... )" );
#endif
					AcmReply::Connection   &connection = acmReply->connection;
					connection.type = ACM_TYPE_PERIODIC_CONNECTION;
					connection.creationDateTime = conn->createDate;
					*connection.localHost = *connection.remoteHost = 0;
					connection.localPort = connection.remotePort = 0;
					const char* pCh = conn->conn->GetBindIp();
					if (pCh) {	strncpy( connection.localHost, pCh, sizeof(connection.localHost) );
								connection.localHost[sizeof(connection.localHost) - 1] = 0;		}
					pCh = conn->conn->GetBindPort();
					if (pCh) {	connection.localPort = (unsigned short)strtoul(pCh, 0, 0); }
					pCh = conn->conn->GetPeerIp();
					if (pCh) {	strncpy( connection.remoteHost, pCh, sizeof(connection.remoteHost) );
								connection.remoteHost[sizeof(connection.remoteHost) - 1] = 0;	}
					pCh = conn->conn->GetPeerPort();
					if (pCh) {	connection.remotePort = (unsigned short)strtoul(pCh, 0, 0); }
					connection.nIterations = conn->nIterations;
					connection.readedBytes = conn->readBytes;
					connection.writtenBytes = conn->wroteBytes;
					WriteMcp( (const void *)acmReply, sizeof(acmReply->connection) );
				}
				const unsigned long   lt = currTm - conn->createTm;
				if (lt > connLifetimeMs )
				{
					const time_t   tm = time( 0 );	const VS_ConnectionTCP   &conn = *conns[indexConnCount]->conn;
					dprint1( "ACS: Zombied Connection. Listening: %s:%s, Incoming: %s:%s.\tUsed Server Resources of %lu ms.\n", conn.GetBindIp(), conn.GetBindPort(), conn.GetPeerIp(), conn.GetPeerPort(), lt );
					DeleteConnection( indexConnCount );
				}
				return;
			}
			statePeriodic = 6;		return;
		case 6 :
			if (flagMcpConnect)
			{
#ifdef _MY_DEBUG_
puts( "VS_AccessConnectionSystem_Implementation::ProcessingPeriodic: case 6: WriteMcp( ... )" );
#endif
				acmReply->type = ACM_TYPE_PERIODIC_STOP_CONNECTIONS;
				WriteMcp( (const void *)acmReply, sizeof(acmReply->type) );
			}
			statePeriodic = 7;		return;
		case 7 :
			if (flagMcpConnect)
			{
#ifdef _MY_DEBUG_
puts( "VS_AccessConnectionSystem_Implementation::ProcessingPeriodic: case 7: WriteMcp( ... )" );
#endif
				AcmReply::StartHandlers   &startHandlers = acmReply->startHandlers;
				startHandlers.type = ACM_TYPE_PERIODIC_START_HANDLERS;
				startHandlers.maxHandlers = maxServs;
				WriteMcp( (const void *)acmReply, sizeof(acmReply->startHandlers) );
			}
			statePeriodic = 8;	indexHandCount = ~0;	return;
		case 8 :
			while (++indexHandCount < maxServs)
			{
				VS_AccessConnectionSystem_Handler   *serv = servs[indexHandCount];
				if (!serv || !serv->isFinal)	continue;
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::ProcessingPeriodic: indexHandCount: %u\n", indexHandCount );
#endif
				if (flagMcpConnect)
				{
#ifdef _MY_DEBUG_
puts( "VS_AccessConnectionSystem_Implementation::ProcessingPeriodic: case 8: WriteMcp( ... )" );
#endif
					AcmReply::Handler   &handler = acmReply->handler;
					handler.type = ACM_TYPE_PERIODIC_HANDLER;
					handler.nProcConns = serv->nProcConns;
					handler.nAccConns = serv->nAccConns;
					*handler.handlerName = 0;
					if (*serv->handlerName)
					{	strncpy( handler.handlerName, serv->handlerName, sizeof(handler.handlerName));
						handler.handlerName[sizeof(handler.handlerName) - 1] = 0;	}
					WriteMcp( (const void *)acmReply, sizeof(acmReply->handler) );
				}
				return;
			}
			statePeriodic = 9;		return;
		case 9 :
			if (flagMcpConnect)
			{
#ifdef _MY_DEBUG_
puts( "VS_AccessConnectionSystem_Implementation::ProcessingPeriodic: case 9: WriteMcp( ... )" );
#endif
				acmReply->type = ACM_TYPE_PERIODIC_STOP_HANDLERS;
				WriteMcp( (const void *)acmReply, sizeof(acmReply->type) );
			}
			statePeriodic = 10;		return;
		case 10 :
			if (flagMcpConnect)
			{
#ifdef _MY_DEBUG_
puts( "VS_AccessConnectionSystem_Implementation::ProcessingPeriodic: case 10: WriteMcp( ... )" );
#endif
				acmReply->type = ACM_TYPE_PERIODIC_STOP;
				WriteMcp( (const void *)acmReply, sizeof(acmReply->type) );
			}
			statePeriodic = 11;		return;
		case 11 :
			statePeriodic = 0;		return;
		default :
			dprint4( "ACS: Unreal condition of \"statePeriodic\" in \"ProcessingPeriodic()\".\n" );
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::ProcessingPeriodic: default, statePeriodic: %u\n", statePeriodic );
#endif
			statePeriodic = 0;		return;
	}	}
	// end VS_AccessConnectionSystem_Implementation::ProcessingPeriodic

	inline void ProcessingTick( void ) {}
	// end VS_AccessConnectionSystem_Implementation::ProcessingTick

	inline void Thread( void )
	{
		cr.cntrlResponse.cmd = vs_acs_start;	cr.cntrlResponse.res = true;
		if (icp->Write( (const void *)&cr.cntrlResponse, sizeof(cr.cntrlResponse) ))
		{
			DWORD   trans, error, n_repeat_error = 0, repeat_error = 0,
						mills = tickMs, old_tick = GetTickCount(), tmp_tick;
			ULONG_PTR   key;	VS_Overlapped   *pov;
			while (1)
			{
				trans = 0;	key = 0;	pov = 0;	error = 0;
				if (!GetQueuedCompletionStatus( hiocp, &trans, &key, (LPOVERLAPPED *)&pov, !statePeriodic || flagMcpWrite ? mills : 0 ))
				{
					error = GetLastError();
					if (pov == 0)
					switch (error)
					{
					case WAIT_TIMEOUT :
					case ERROR_SEM_TIMEOUT :	n_repeat_error = 0;		goto go_continue;
					}
					if (!pov)
					{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::Thread: if (!pov), error: %u\n", error );
#endif
						dprint4( "ACS: Error of GetQueuedCompletionStatus(...) and \"pov\" is equal to NULL. System Error: %lu.\n", error );
						if (!n_repeat_error)
						{
							repeat_error = error;	++n_repeat_error;
						}
						else if (repeat_error == error)
						{
							if (++n_repeat_error >= VS_ACS_REPEAT_ERROR_NUMBER)
							{
								dprint4( "ACS: The amount of recurrence of this mistake has exceeded %lu.\n", n_repeat_error );
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::Thread: if (n_repeat_error >= %u), error: %u\n", VS_ACS_REPEAT_ERROR_NUMBER, error );
#endif
								goto go_return;
						}	}
						else	n_repeat_error = 0;
						goto go_continue;
					}
					else	pov->error = error;
					n_repeat_error = 0;
				}
				if (!RunHandle( (unsigned long)trans, *pov ))
				{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::Thread: if (!RunHandle( (unsigned long)bTrans, *pov )), error: %u\n", error );
#endif
					goto go_return;
				}
go_continue:	tmp_tick = GetTickCount();	currDiffTm = tmp_tick - old_tick;
				currTm += currDiffTm;		sumCurrDiffTm += currDiffTm;
				if (statePeriodic)		ProcessingPeriodic();
				if (sumCurrDiffTm >= tickMs)
				{
					tickDiffTm = sumCurrDiffTm;		sumCurrDiffTm = 0;
					tickTm += tickDiffTm;			mills = tickMs;
					if (!statePeriodic)		ProcessingPeriodic();
					ProcessingTick();
				}
				else	mills -= currDiffTm;
				old_tick = tmp_tick;
		}	}
		else	dprint1( "ACS: Error of writing in \"icp\" at start of thread ACS. System Error: %lu.\n", GetLastError() );
go_return:
		icp->Close();
#ifdef _MY_DEBUG_
puts( "VS_AccessConnectionSystem_Implementation::Thread: return;" );
#endif
	}
	// end VS_AccessConnectionSystem_Implementation::Thread

	static unsigned __stdcall Thread( void *arg )
	{
		vs::SetThreadName("ACS");
		((VS_AccessConnectionSystem_Implementation *)arg)->Thread();
		_endthreadex( 0 );	return 0;
	}
	// end VS_AccessConnectionSystem_Implementation::Thread

	inline bool CntrlSR( const ControlInquiry *cI, const unsigned long sI,
							ControlResponse *cR = 0, const unsigned long sR = 0,
							unsigned long mills = 600000 )
	{
#ifdef _MY_DEBUG_
printf( "VS_AccessConnectionSystem_Implementation::CntrlSR( cI: %08X, sI: %u, cR: %08X, sR: %u )\n", _MD_POINT_(cI), sI, _MD_POINT_(cR), sR );
#endif
		std::unique_lock<decltype(mtx)> l(mtx);
		if (!isInit)
			return false;

		assert(cI != nullptr);
		assert(sI != 0);
		if (!ocp->Write(cI, sI) || sI != ocp->GetWriteResult(mills))
		{
			dprint1("ACS: Error writing to \"ocp\". System Error: %lu.", GetLastError());
			return false;
		}

		if (!sR) // No response is expected, we are done.
			return true;

		std::lock_guard<decltype(mtx_read)> l_read(mtx_read);
		// Release the main mutex to allow other threads to send async requests while we are waiting for our response.
		l.unlock();
		assert(cR != nullptr);
		if (!ocp->Read(cR, sR) || sR != ocp->GetReadResult(mills) || cI->cntrlInquiry.cmd != cR->cntrlResponse.cmd)
		{
			dprint1("ACS: Error reading from \"ocp\". System Error: %lu.", GetLastError());
			return false;
		}
		return true;
	}
	// end VS_AccessConnectionSystem_Implementation::CntrlSR

	/////////////////////////////////////////////////////////////////////////////////////

	inline bool InitAct( const unsigned long maxHandlers,
				const unsigned long maxListeners, const unsigned long maxConnections,
				const unsigned long connectionLifetime )
	{
		if (isInit)		return false;
		GetRegWatchdogSuppression();
		statePeriodic = 0;
		size_t   size = maxHandlers * sizeof(VS_AccessConnectionSystem_Handler *);
		servs = (VS_AccessConnectionSystem_Handler **)malloc( size );
		if (!servs) {	dprint0( "ACS: Error of initialization of a array of \"servs\".\n" );
						ShutdownAct();	return false;	}
		memset( (void *)servs, 0, size );		maxServs = maxHandlers;
		size = maxListeners * sizeof(VS_AccessConnectionSystem_Listener *);
		lists = (VS_AccessConnectionSystem_Listener **)malloc( size );
		if (!lists) {	dprint0( "ACS: Error of initialization of a array of \"lists\".\n" );
						ShutdownAct();	return false;	}
		memset( (void *)lists, 0, size );		maxLists = maxListeners;
		size = maxConnections * sizeof(VS_AccessConnectionSystem_Connection *);
		conns = (VS_AccessConnectionSystem_Connection **)malloc( size );
		if (!conns) {	dprint0( "ACS: Error of initialization of a array of \"conns\".\n" );
						ShutdownAct();	return false;	}
		memset( (void *)conns, 0, size );		maxConns = maxConnections;
		if (!(iocpm = new VS_IOCPsManager) || !iocpm->IsValid() || !iocpm->Init()
				|| !(hiocpm = (HANDLE)iocpm->HandleIocp()) || hiocpm == INVALID_HANDLE_VALUE)
		{	dprint0( "ACS: Error of initialization of VS_IOCPsManager.\n" );
			ShutdownAct();	return false;	}
		if (!(hiocp = CreateIoCompletionPort( INVALID_HANDLE_VALUE, 0, 0, 10 )))
		{	dprint0( "ACS: Error of creation of own IOCPort. System Error: %lu.\n", GetLastError() );
			ShutdownAct();	return false;	}
		if (!(ocp = new VS_ConnectionMsg) || !ocp->Create( vs_pipe_type_duplex )
			|| !ocp->CreateOvReadEvent() || !ocp->CreateOvWriteEvent()
			|| !(icp = new VS_ConnectionMsg) || !icp->Open( ocp, vs_pipe_type_duplex )
			|| !icp->SetIOCP( hiocp ))
		{	dprint0( "ACS: Error of creation or initialization \"ocp\" or \"icp\". System Error: %lu.\n", GetLastError() );
			ShutdownAct();	return false;	}
		icp->SetOvWriteFields( (const VS_ACS_Field)VS_ACS_CONTROL_WRITE );
		icp->SetOvReadFields( (const VS_ACS_Field)VS_ACS_CONTROL_READ );
		ResetToConnectMcp();
		uintptr_t   th = _beginthreadex( 0, 0, Thread, (void *)this, 0, 0 );
		if ( !th || th == -1L)
		{	dprint0( "ACS: Error of creation of own thread. System Error: %lu.\n", GetLastError() );
			ShutdownAct();	return false;	}
		hthr = (HANDLE)th;
		ControlResponse::CntrlResponse	 cr;	memset( (void *)&cr, 0, sizeof(cr) );
		unsigned long   mills = 120000;
		if (!ocp->Read( (void *)&cr, sizeof(cr) )
			|| ocp->GetReadResult( mills ) != sizeof(cr)
			|| cr.cmd != vs_acs_start || !cr.res)
		{	dprint0( "ACS: Error of reading or read from \"ocp\". System Error: %lu.\n", GetLastError() );
			ShutdownAct();	return false;	}
		connLifetimeMs = connectionLifetime;	return isInit = true;
	}
	// end VS_AccessConnectionSystem_Implementation::InitAct

	inline bool Init( const unsigned long maxHandlers,
				const unsigned long maxListeners, const unsigned long maxConnections,
				const unsigned long connectionLifetime )
	{
		if (maxHandlers < VS_ACS_MIN_HANDLERS || maxHandlers > VS_ACS_MAX_HANDLERS
				|| maxListeners < VS_ACS_MIN_LISTENERS || maxListeners > VS_ACS_MAX_LISTENERS
				|| maxConnections < VS_ACS_MIN_CONNECIONS || maxConnections > VS_ACS_MAX_CONNECIONS
				|| connectionLifetime < VS_ACS_MIN_CONNECTION_LIFETIME || connectionLifetime > VS_ACS_MAX_CONNECTION_LIFETIME)
		{	dprint4( "ACS: Arguments the handed functions are erroneous.\n" );
			return false;	}
		std::lock_guard<decltype(mtx)> l(mtx);
		return InitAct(maxHandlers, maxListeners, maxConnections, connectionLifetime);
	}
	// end VS_AccessConnectionSystem_Implementation::Init

	inline bool IsInit( void )
	{
		std::lock_guard<decltype(mtx)> l(mtx);
		return isInit;
	}
	// end VS_AccessConnectionSystem_Implementation::IsInit

	inline void ShutdownAct( void )
	{
		if (hthr)
		{
			if (ocp) {		delete ocp;		ocp = 0;	}
			if (WaitForSingleObject( hthr, VS_ACS_TIMEOUT_THREAD_SHUTDOWN ) == WAIT_OBJECT_0)
				CloseHandle( hthr );
			hthr = 0;
		}
		if (icp) {		delete icp;		icp = 0;	}
		if (ocp) {		delete ocp;		ocp = 0;	}
		if (mcp) {		delete mcp;		mcp = 0;	}
		if (acmRequest) {	delete acmRequest;	acmRequest = 0;		}
		if (acmReply) {		delete acmReply;	acmReply = 0;		}
		acmReadState = 0;
		if (iocpm) {	delete iocpm;	iocpm = 0;	}
		if (hiocp) {	CloseHandle( hiocp );	hiocp = 0;	}
		if (conns)
		{
			for (unsigned long i = 0; i < maxConns; ++i)
			{
				VS_AccessConnectionSystem_Connection   *conn = conns[i];
				if (conn)
				{
					if (conn->conn) {	delete conn->conn;	conn->conn = 0;		}
					delete conn;
			}	}
			free( (void *)conns );	conns = 0;
		}
		if (lists)
		{
			for (unsigned long i = 0; i < maxLists; ++i)
			{
				VS_AccessConnectionSystem_Listener   *list = lists[i];
				if (list)
				{
					if (list->conns)
					{
						for (unsigned long i = 0; i < list->maxConns; ++i)
						{
							VS_AccessConnectionSystem_ListenerConnection   *conn = list->conns[i];
							if (conn && conn->conn) {	delete conn->conn;	conn->conn = 0;		}
					}	}
					delete list;
			}	}
			free( (void *)lists );	lists = 0;
		}
		if (servs)
		{
			for (unsigned long i = 0; i < maxServs; ++i)
				if (servs[i])	delete servs[i];
			free( (void *)servs );	servs = 0;
		}
		for (unsigned long i = 0; i < VS_ACS_CONN_SUMP_DEPTH; ++i)
			if (sumpConns[i])	delete sumpConns[i];
		memset( (void *)sumpConns, 0, sizeof(sumpConns) );	indexSump = 0;
		for (unsigned i = 0; i < MAX_CHK_LSRS; ++i)
			if (chkLsrs[i].host)	free( (void *)chkLsrs[i].host );
		memset( (void *)chkLsrs, 0, sizeof(chkLsrs) );
		allLsnrsBrokenTick = 0;
		isInit = false;
		strncpy( (char *)acsTestAttemptBuf, acsTestAttemptStr, sizeof(acsTestAttemptBuf) );
	}
	// end VS_AccessConnectionSystem_Implementation::ShutdownAct

	inline void Shutdown( void )
	{
		std::lock_guard<decltype(mtx)> l(mtx);
		ShutdownAct();
	}
	// end VS_AccessConnectionSystem_Implementation::Shutdown

	inline bool AddHandler( const char *name, VS_AccessConnectionHandler *handler, const bool isFinal )
	{
		bool   ret = false;
		ControlInquiry::AddHandler   cmd;	memset( (void *)&cmd, 0, sizeof(cmd) );
		cmd.cmd = vs_acs_add_handler;
		if (!name || !*name || strlen( name ) > VS_ACS_MAX_SIZE_HANDLER_NAME)	return ret;
		strncpy( cmd.name, name, sizeof(cmd.name) );	cmd.name[sizeof(cmd.name) - 1] = 0;
		if (!handler)	return ret;
		cmd.handler = handler;	cmd.isFinal = isFinal;
		ControlResponse::CntrlResponse   res;	memset( (void *)&res, 0, sizeof(res) );
		if (CntrlSR( (const ControlInquiry *)&cmd, sizeof(cmd), (ControlResponse *)&res, sizeof(res) ))
			ret = res.res;
		return ret;
	}
	// end VS_AccessConnectionSystem_Implementation::AddHandler

	inline void RemoveHandler( const char *name )
	{
		ControlInquiry::RemoveHandler   cmd;	memset( (void *)&cmd, 0, sizeof(cmd) );
		cmd.cmd = vs_acs_remove_handler;
		if (!name || !*name || strlen( name ) > VS_ACS_MAX_SIZE_HANDLER_NAME)	return;
		strncpy( cmd.name, name, sizeof(cmd.name) );	cmd.name[sizeof(cmd.name) - 1] = 0;
		VS_ACS_Cmd   res = vs_acs_unknown;
		CntrlSR( (const ControlInquiry *)&cmd, sizeof(cmd), (ControlResponse *)&cmd, sizeof(res) );
	}
	// end VS_AccessConnectionSystem_Implementation::RemoveHandler

	inline void RemoveAllHandlers( void )
	{
		VS_ACS_Cmd   cmd = vs_acs_remove_all_handlers;
		VS_ACS_Cmd   res = vs_acs_unknown;
		CntrlSR( (const ControlInquiry *)&cmd, sizeof(cmd), (ControlResponse *)&cmd, sizeof(res) );
	}
	// end VS_AccessConnectionSystem_Implementation::RemoveAllHandlers

	unsigned AddListeners( const char *endpointName, const bool hidden,const unsigned maxConnections )
	{
		unsigned   ret = 0;
		ControlInquiry::AddListeners   cmd;		memset( (void *)&cmd, 0, sizeof(cmd) );
		cmd.cmd = vs_acs_add_listeners;
		if (!endpointName || !*endpointName || strlen( endpointName ) > VS_ACS_MAX_SIZE_ENDPOINT_NAME)
			return ret;
		strncpy( cmd.endpointName, endpointName, sizeof(cmd.endpointName) );
		if (maxConnections < VS_ACS_MIN_LISTENER_CONNECIONS
			|| maxConnections > VS_ACS_MAX_LISTENER_CONNECIONS)		return ret;
		cmd.maxConnections = maxConnections;
		cmd.hidden = hidden;
		ControlResponse::AddListeners   res;	memset( (void *)&res, 0, sizeof(res) );
		if (CntrlSR( (const ControlInquiry *)&cmd, sizeof(cmd), (ControlResponse *)&res, sizeof(res) ))
			ret = res.res;
		return ret;
	}
	// end VS_AccessConnectionSystem_Implementation::AddListeners

	inline bool AddListener( const char *host, const int port, const bool hidden,const unsigned maxConnections )
	{
		bool   ret = false;
		ControlInquiry::AddListener   cmd;		memset( (void *)&cmd, 0, sizeof(cmd) );
		cmd.cmd = vs_acs_add_listener;
		if (!VS_GetHostByName( host, cmd.host, sizeof(cmd.host) ) || port <= 0 || port > 65535)
			return ret;
		cmd.port = port;
		if (maxConnections < VS_ACS_MIN_LISTENER_CONNECIONS
			|| maxConnections > VS_ACS_MAX_LISTENER_CONNECIONS)		return ret;
		cmd.maxConnections = maxConnections;
		cmd.hidden = hidden;
		ControlResponse::CntrlResponse   res;	memset( (void *)&res, 0, sizeof(res) );
		if (CntrlSR( (const ControlInquiry *)&cmd, sizeof(cmd), (ControlResponse *)&res, sizeof(res) ))
			ret = res.res;
		return ret;
	}
	// end VS_AccessConnectionSystem_Implementation::AddListener

	inline bool AddListenerByIP( const char *ip, const int port, const bool hidden, const unsigned maxConnections )
	{
		if (!ip || (port <= 0 || port > 65535))
			return false;
		bool   ret = false;
		ControlInquiry::AddListener   cmd;		memset( (void *)&cmd, 0, sizeof(cmd) );
		cmd.cmd = vs_acs_add_listener;
		memcpy_s(&cmd.host[0], 24, ip, strlen(ip));
		cmd.port = port;
		if (maxConnections < VS_ACS_MIN_LISTENER_CONNECIONS
			|| maxConnections > VS_ACS_MAX_LISTENER_CONNECIONS)		return ret;
		cmd.maxConnections = maxConnections;
		cmd.hidden = hidden;
		ControlResponse::CntrlResponse   res;	memset( (void *)&res, 0, sizeof(res) );
		if (CntrlSR( (const ControlInquiry *)&cmd, sizeof(cmd), (ControlResponse *)&res, sizeof(res) ))
			ret = res.res;
		return ret;
	}
	// end VS_AccessConnectionSystem_Implementation::AddListener


	inline int GetListeners(std::string& str)
	{
		int   ret = false;
		ControlInquiry::CntrlInquiry   cmd;		memset( (void *)&cmd, 0, sizeof(cmd) );
		cmd.cmd = vs_acs_get_listeners;
		ControlResponse::GetListeners   res;	memset( (void *)&res, 0, sizeof(res) );
		if (CntrlSR( (const ControlInquiry *)&cmd, sizeof(cmd), (ControlResponse *)&res, sizeof(res) ))
		{
			char tmp[32] = {0};
			for (unsigned int i = 0; i < res.n_listeners; i++)
			{
				bool is_ipv6 = strchr(res.hosts[i], ':') != 0;
				if (is_ipv6) str +="[";
				str += res.hosts[i];
				if(is_ipv6) str += "]";
				str += ":";
				str += _itoa(res.ports[i], tmp, 10);
				if (i < (res.n_listeners-1))
					str += ",";
			}
			ret = res.n_listeners;
			for(unsigned int i=0; i<res.n_listeners; ++i)
			{
				free((char*)res.hosts[i]);
			}
			free(res.hosts);
			free(res.ports);
		}
		return ret;
	}
	// end VS_AccessConnectionSystem_Implementation::GetListeners

	inline void RemoveListener( const char *host, const int port )
	{
		ControlInquiry::RemoveListener   cmd;	memset( (void *)&cmd, 0, sizeof(cmd) );
		cmd.cmd = vs_acs_remove_listener;
		if (!VS_GetHostByName( host, cmd.host, sizeof(cmd.host) ))	return;
		if (port <= 0 || port > 65535)	return;
		cmd.port = port;
		CntrlSR( (const ControlInquiry *)&cmd, sizeof(cmd) );
	}
	// end VS_AccessConnectionSystem_Implementation::RemoveListener

	inline void RemoveListenersIp( const char *host )
	{
		ControlInquiry::RemoveListenersIP   cmd;	memset( (void *)&cmd, 0, sizeof(cmd) );
		cmd.cmd = vs_acs_remove_listeners_ip;
		if (!VS_GetHostByName( host, cmd.host, sizeof(cmd.host) ))	return;
		CntrlSR( (const ControlInquiry *)&cmd, sizeof(cmd) );
	}
	// end VS_AccessConnectionSystem_Implementation::RemoveListenerIp

	inline void RemoveListenersPort( const int port )
	{
		if (port <= 0 || port > 65535)	return;
		ControlInquiry::RemoveListenersPort   cmd;		memset( (void *)&cmd, 0, sizeof(cmd) );
		cmd.cmd = vs_acs_remove_listeners_port;
		cmd.port = port;
		CntrlSR( (const ControlInquiry *)&cmd, sizeof(cmd) );
	}
	// end VS_AccessConnectionSystem_Implementation::RemoveListenerPort

	inline void RemoveAllListeners( void )
	{
		VS_ACS_Cmd   cmd = vs_acs_remove_all_listeners;
		CntrlSR( (const ControlInquiry *)&cmd, sizeof(cmd) );
	}
	// end VS_AccessConnectionSystem_Implementation::RemoveAllListeners

	inline void *HandleIOCP( void )
	{
		std::lock_guard<decltype(mtx)> l(mtx);
		return !isInit ? 0 : (void *)hiocp;
	}
	// end VS_AccessConnectionSystem_Implementation::HandleIOCP

	inline bool ShakeListener( const char *host, const int port )
	{
		bool   ret = false;
		ControlInquiry::ShakeListener   cmd;	memset( (void *)&cmd, 0, sizeof(cmd) );
		cmd.cmd = vs_acs_shake_listener;
		if (!VS_GetHostByName( host, cmd.host, sizeof(cmd.host) ) || port <= 0 || port > 65535)
			return ret;
		cmd.port = port;
		ControlResponse::CntrlResponse   res;	memset( (void *)&res, 0, sizeof(res) );
		if (CntrlSR( (const ControlInquiry *)&cmd, sizeof(cmd), (ControlResponse *)&res, sizeof(res) ))
			ret = res.res;
		return ret;
	}
	// end VS_AccessConnectionSystem_Implementation::ShakeListener

	void SetConnection( VS_ConnectionTCP *conn )
	{
		if (!conn)	return;
		ControlInquiry::SetConnection   cmd;		memset( (void *)&cmd, 0, sizeof(cmd) );
		cmd.cmd = vs_acs_set_connection;
		cmd.conn = conn;
		CntrlSR( (const ControlInquiry *)&cmd, sizeof(cmd) );
	}
	// end VS_AccessConnectionSystem_Implementation::SetConnection
	inline bool TestReturnFalse()
	{
		if (m_watchdogSuppression==1)
		{
			///Suppress watchdog tests
			return true;
		}
		return false;
	}
	inline bool Test( void )
	{
#ifdef VS_ACS_AUTORESTART
		if (counter>VS_ACS_AUTORESTART_PERIOD)
			return false;
		else
		{
			counter++;
		}
#endif
		VS_ACS_Cmd   cmd = vs_acs_get_listeners;
		ControlResponse::GetListeners   res;	memset( (void *)&res, 0, sizeof(res) );
		bool clrRes = false;
		if (!(clrRes=CntrlSR( (const ControlInquiry *)&cmd, sizeof(cmd), (ControlResponse *)&res, sizeof(res) ))
			|| !res.res || !res.n_listeners || !res.ports || *res.ports < 1 || *res.ports > 65535
			|| !res.hosts || !*res.hosts || !**res.hosts)
		{
			if (!clrRes)
			{
				dprint0("ACS::Test error #0 Operation error.  ");
				dprint0(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
				return TestReturnFalse();
			}
			if (!res.res)
			{
				dprint0("ACS::Test error #1 !res.res.  ");
				dprint0(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
			}
			if (!res.n_listeners)
			{
				dprint0("ACS::Test error #2 There are no listen ports. Check up your server's network configuration.  ");
				dprint0(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
				return false; //TestReturnFalse();
			}
			if (!res.ports)
			{
				dprint0("ACS::Test error #3 !res.ports. ");
				dprint0(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
				return TestReturnFalse();
			}
			if (*res.ports < 1) 	dprint0("ACS::Test error #4 *res.ports < 1. Check ports configuration.\n");
			if (*res.ports > 65535) 	dprint0("ACS::Test error #5 *res.ports > 65535.  Check ports configuration.\n");
			if (!res.hosts)
			{
				dprint0("ACS::Test error #6 !res.hosts. Check hosts configuration.  ");
				dprint0(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
				return TestReturnFalse();
			}
			if (!*res.hosts)
			{
				dprint0("ACS::Test error #7 !*res.hosts.  Check hosts configuration.  ");
				dprint0(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
				return TestReturnFalse();
			}
			if (!**res.hosts)
			{
				dprint0("ACS::Test error #8 !**res.hosts.  Check hosts configuration.  ");
				dprint0(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
				return TestReturnFalse();
			}
			return TestReturnFalse();
		}
		bool   ret = false;		VS_ConnectionTCP   *tcp = 0;
		bool isOneConnect = false;
		bool isAllLsnrsBroken = true;
		for (unsigned i = 0; i < res.n_listeners; ++i)
		{	tcp = new VS_ConnectionTCP;
			if (!tcp)
			{
				dprint0("ACS::Test error #10 cannot make new VS_ConnectionTCP. Check your virtual memory limit.  ");
				dprint0(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
				goto go_return;
			}
			unsigned long   ms = 20000;
			if (!tcp->IsValid())
			{
				dprint0("ACS::Test error #11 VS_ConnectionTCP not valid. Ask developer.  ");
				dprint0(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
				goto go_return;
			}
			if (!tcp->Connect( res.hosts[i], res.ports[i], ms ))
			{
				unsigned conn_err = GetLastError();
				dprint0("ACS::Test error #12. Cannot listen host:%s port: %u. Conn Err = %d Check your firewall settings.  ",res.hosts[i], res.ports[i],conn_err);
				dprint0(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
				delete tcp;
				tcp = 0;
				Sleep(16); /// for Kost; maybe some resources will be released by OS
				continue;
			} else isOneConnect = true;

			if ( !tcp->CreateOvWriteEvent())
			{
				dprint0("ACS::Test error #13 cannot CreateOvWriteEvent in VS_ConnectionTCP.  ");
				dprint0(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
				goto go_return;
			}

			if ( !tcp->CreateOvReadEvent())
			{
				dprint0("ACS::Test error #14 cannot CreateOvReadEvent in VS_ConnectionTCP.  ");
				dprint0(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
				goto go_return;
			}
			bool localTest = false;
			if (tcp->Write( (const void *)acsTestAttemptBuf, sizeof(acsTestAttemptBuf) )
					&& tcp->GetWriteResult( ms ) == sizeof(acsTestAttemptBuf)
					&& tcp->Read( 0, 0 )) localTest = true;
			int localRes = 0;
			if (localTest) localRes = tcp->GetReadResult( ms );
			if (localTest && localRes != -1 )
			{	unsigned   already = ~0;
				for (unsigned j = 0; j < MAX_CHK_LSRS; ++j)
				{	if (chkLsrs[j].host && res.ports[i] == chkLsrs[j].port && !strcmp( res.hosts[i], chkLsrs[j].host ))
					{	chkLsrs[j].n += 1;		already = j;
						const time_t   tm1 = time( 0 );
						dprint4( "ACS: checked listener ticker++: %d Host: %s, Port: %u. %s\n",chkLsrs[j].n, res.hosts[i], res.ports[i], ctime( &tm1 ) );
						if (chkLsrs[j].n > MAX_BROKEN_LSNR_PER_ONE_LSNR)
						{	const time_t   tm = time( 0 );
							dprint0( "ACS: Have Broken Listener, Host: %s, Port: %u. %s\n", res.hosts[i], res.ports[i], ctime( &tm ) );
							goto go_return;
				}	}	}

				if (!ShakeListener( res.hosts[i], res.ports[i] ))
				{	const time_t   tm = time( 0 );
					dprint0( "ACS: Have Broken ACS, Host: %s, Port: %u. %s  ", res.hosts[i], res.ports[i], ctime( &tm ) );
					dprint0(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
					goto go_return;
				}
				else
				{	const time_t   tm = time( 0 );
					dprint4( "ACS: Result of read: %d Shake \
					well on Host: %s, Port: %u. %s  ",localRes, res.hosts[i], res.ports[i], ctime( &tm ) );
					dprint4(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
				}

				if (already >= MAX_CHK_LSRS)
				{	const time_t   tm = time( 0 );
					dprint4( "ACS: Adding checked Listener, Host: %s, Port: %u. %s\n", res.hosts[i], res.ports[i], ctime( &tm ) );
					for (unsigned j = 0; j < MAX_CHK_LSRS; ++j)
					{	if (!chkLsrs[j].host)
						{	chkLsrs[j].host = res.hosts[i];		res.hosts[i] = 0;
							chkLsrs[j].port = res.ports[i];
			}	}	}	}
			else
			{
				isAllLsnrsBroken = false;
				for (unsigned j = 0; j < MAX_CHK_LSRS; ++j)
				{	if (chkLsrs[j].host && res.ports[i] == chkLsrs[j].port && !strcmp( res.hosts[i], chkLsrs[j].host ))
					{	free( (void *)chkLsrs[j].host );	memset( (void *)&chkLsrs[j], 0, sizeof(ChkLsrs) );
			}	}	}
			delete tcp;		tcp = 0;
		}
		if (isAllLsnrsBroken)
		{
			if(allLsnrsBrokenTick > MAX_BROKEN_LSNR_PER_ALL)
			{
				dprint0("ACS::Test error #16 all listen port was broken enought times.  ");
				dprint0(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
				allLsnrsBrokenTick = 0;
				goto go_return;
			}
			allLsnrsBrokenTick++;
		} else allLsnrsBrokenTick = 0;
		if (!isOneConnect)
		{
			dprint0("ACS::Test error #15 cannot listen all TCP conections.  Check your firewall settings.  ");
			dprint0(":: gle = %ld, wgle = %d\n", GetLastError(), WSAGetLastError());
			goto go_return;
		}

		ret = true;
go_return:
		if (tcp)	delete tcp;
		for (unsigned i = 0; i < res.n_listeners; ++i)
			if (res.hosts[i])	free( (void *)res.hosts[i] );
		free( (void *)res.hosts );		free( (void *)res.ports );
		return (ret==false)?(TestReturnFalse()):ret;
	}
	// end VS_AccessConnectionSystem_Implementation::Test
};
// end VS_AccessConnectionSystem_Implementation struct

unsigned char   VS_AccessConnectionSystem_Implementation::acsTestAttemptBuf[];

/////////////////////////////////////////////////////////////////////////////////////////
void VS_AccessConnectionSystem::PrepareToDie( void )
{
	if (imp)
	{	InterlockedIncrement(&imp->prepareToDie);	}
}

VS_AccessConnectionSystem::VS_AccessConnectionSystem( void )
{	imp = new VS_AccessConnectionSystem_Implementation;		}
// end VS_AccessConnectionSystem::VS_AccessConnectionSystem

VS_AccessConnectionSystem::~VS_AccessConnectionSystem( void )
{	if (imp)	delete imp;		}
// end VS_AccessConnectionSystem::~VS_AccessConnectionSystem

bool VS_AccessConnectionSystem::Init( const unsigned long maxHandlers, const unsigned long maxListeners, const unsigned long maxConnections, const unsigned long connectionLifetime )
{	return !imp ? false : imp->Init( maxHandlers, maxListeners, maxConnections, connectionLifetime );	}
// end VS_AccessConnectionSystem::Init

bool VS_AccessConnectionSystem::IsInit( void ) const
{	return !imp ? false : imp->IsInit();	}
// end VS_AccessConnectionSystem::IsInit

void VS_AccessConnectionSystem::Shutdown( void )
{	if (imp)	imp->Shutdown();	}
// end VS_AccessConnectionSystem::Shutdown

bool VS_AccessConnectionSystem::AddHandler( const char *name, VS_AccessConnectionHandler *handler, const bool isFinal )
{	return !imp ? false : imp->AddHandler( name, handler, isFinal );	}
// end VS_AccessConnectionSystem::AddHandler

void VS_AccessConnectionSystem::RemoveHandler( const char *name )
{	if (imp)	imp->RemoveHandler( name );		}
// end VS_AccessConnectionSystem::RemoveHandler

void VS_AccessConnectionSystem::RemoveAllHandlers( void )
{	if (imp)	imp->RemoveAllHandlers();	}
// end VS_AccessConnectionSystem::RemoveAllHandlers

unsigned VS_AccessConnectionSystem::AddListeners( const char *endpointName, const bool hidden, const unsigned maxConnections )
{	return !imp ? 0 : imp->AddListeners( endpointName, hidden, maxConnections );	}
// end VS_AccessConnectionSystem::AddListeners

bool VS_AccessConnectionSystem::AddListener( const char *host, const int port, const bool hidden, const unsigned maxConnections )
{	return !imp ? false : imp->AddListener( host, port, hidden,maxConnections );	}
// end VS_AccessConnectionSystem::AddListener

bool VS_AccessConnectionSystem::AddListenerByIP( const char *host, const int port, const bool hidden, const unsigned maxConnections )
{	return !imp ? false : imp->AddListenerByIP( host, port,hidden,maxConnections );	}
// end VS_AccessConnectionSystem::AddListenerByIP

int VS_AccessConnectionSystem::GetListeners(std::string& str)
{	return !imp ? false : imp->GetListeners(str);	}
// end VS_AccessConnectionSystem::AddListenerByIP

void VS_AccessConnectionSystem::RemoveListener( const char *host, const int port )
{	if (imp)	imp->RemoveListener( host, port );	}
// end VS_AccessConnectionSystem::RemoveListener

void VS_AccessConnectionSystem::RemoveListenersIp( const char *host )
{	if (imp)	imp->RemoveListenersIp( host );		}
// end VS_AccessConnectionSystem::RemoveListenersIp

void VS_AccessConnectionSystem::RemoveListenersPort( const int port )
{	if (imp)	imp->RemoveListenersPort( port );	}
// end VS_AccessConnectionSystem::RemoveListenersPort

void VS_AccessConnectionSystem::RemoveAllListeners( void )
{	if (imp)	imp->RemoveAllListeners();	}
// end VS_AccessConnectionSystem::RemoveAllListeners

void *VS_AccessConnectionSystem::HandleIOCP( void )
{	return !imp ? 0 : imp->HandleIOCP();	}
// end VS_AccessConnectionSystem::HandleIOCP

bool VS_AccessConnectionSystem::Test( void )
{	return !imp ? false : imp->Test();	}
// end VS_AccessConnectionSystem::Test

/////////////////////////////////////////////////////////////////////////////////////////
