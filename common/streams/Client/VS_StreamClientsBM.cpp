#if defined(_WIN32) // Not ported yet

/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: Реализация multi-conference менеджера клиентов медиа-стримов
//
//  Created: 04.03.03     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_StreamClientsBM.cpp
/// \brief Реализация multi-conference менеджера клиентов медиа-стримов
/// \note
///

#include <Windows.h>
#include <process.h>

#include "VS_StreamClientsBM.h"
#include "VS_StreamClientSender.h"
#include "VS_StreamClientReceiver.h"
#include "../../acs/Lib/VS_AcsLib.h"
#include "acs/connection/VS_ConnectionUDP.h"
#include "../../std/cpplib/VS_MemoryLeak.h"
#include "std/cpplib/ThreadUtils.h"

#include <cstdlib>

struct VS_StreamClientsBM_Implementation
{
	VS_StreamClientsBM_Implementation() :
		isInit(false), host(0), ip(0), br_ip(0), port(0), n_ports(0), rai(0), conns(0),
		event(0), events(0), indexes(0), hthr(0), sender(0), n_port(0), result(false)
	{	InitializeCriticalSection( &sect );		}
	// end VS_StreamClientsBM_Implementation::VS_StreamClientsBM_Implementation

	VS_StreamClientsBM_Implementation(const char *host,
										const unsigned short port, const unsigned n_ports,
										VS_StreamClientsBM::ReceiverAppeared *rai ) :
		isInit(false), host(0), ip(0), br_ip(0), port(0), n_ports(0), rai(0), conns(0),
		event(0), events(0), indexes(0), hthr(0), sender(0), n_port(0), result(false)
	{	InitializeCriticalSection( &sect );		Init( host, port, n_ports, rai );	}
	// end VS_StreamClientsBM_Implementation::VS_StreamClientsBM_Implementation

	~VS_StreamClientsBM_Implementation()
	{	Shutdown();		DeleteCriticalSection( &sect );		}
	// end VS_StreamClientsBM_Implementation::~VS_StreamClientsBM_Implementation

	CRITICAL_SECTION   sect;
	bool   isInit;
	char   *host, *ip, *br_ip;
	unsigned short   port;
	unsigned   n_ports;
	VS_StreamClientsBM::ReceiverAppeared   *rai;
	VS_ConnectionUDP   **conns;
	HANDLE   event, *events;
	unsigned   *indexes;
	HANDLE   hthr;
	VS_StreamClientSender   *sender;
	unsigned   n_port;
	bool   result;

	inline bool InitAct( const char *host, const unsigned short port,
						const unsigned n_ports, VS_StreamClientsBM::ReceiverAppeared *rai )
	{
		if (isInit)		return false;
		if (!host || !*host || !port || !n_ports || n_ports > VS_STREAM_CLIENT_BM_MAX_PORTS || !rai)
			return false;
		VS_StreamClientsBM_Implementation::host = _strdup( host );
		if (!VS_StreamClientsBM_Implementation::host) {		ShutdownAct();	return false;	}
		ip = (char *)malloc( 48 );	if (!ip) {		ShutdownAct();	return false;	}
		br_ip = ip + 24;	memset( (void*)ip, 0, 48 );
		if (!VS_GetHostByName( VS_StreamClientsBM_Implementation::host, ip, 24 )
				|| !VS_GetInetBroadcastAddr( ip, br_ip, 24 ))
		{	ShutdownAct();	return false;	}
		VS_StreamClientsBM_Implementation::port = port;
		VS_StreamClientsBM_Implementation::n_ports = n_ports;
		VS_StreamClientsBM_Implementation::rai = rai;
		size_t   sz = VS_StreamClientsBM_Implementation::n_ports * sizeof(VS_ConnectionUDP *);
		conns = (VS_ConnectionUDP **)malloc( sz );
		if (!conns) {	ShutdownAct();	return false;	}
		memset( (void *)conns, 0, sz );
		sz = ( VS_StreamClientsBM_Implementation::n_ports + 2 ) * sizeof(HANDLE);
		events = (HANDLE *)malloc( sz );
		if (!events) {	ShutdownAct();	return false;	}
		memset( (void *)events, 0, sz );
		sz = ( VS_StreamClientsBM_Implementation::n_ports + 2 ) * sizeof(unsigned);
		event = CreateEvent( 0, FALSE, FALSE, 0 );
		events[0] = CreateEvent( 0, FALSE, FALSE, 0 );
		events[1] = CreateEvent( 0, FALSE, FALSE, 0 );
		indexes = (unsigned *)malloc( sz );
		if (!indexes) {		ShutdownAct();	return false;	}
		memset( (void *)indexes, ~0, sz );
		if (!event || !events[0] || !events[1]) {	ShutdownAct();	return false;	}
		hthr = (HANDLE)_beginthreadex( 0, 0, Thread, (void *)this, 0, 0 );
		if (!hthr || hthr == (HANDLE)-1L) {		hthr = 0;	ShutdownAct();	return false;	}
		return isInit = true;
	}
	// end VS_StreamClientsBM_Implementation::InitAct

	inline bool Init( const char *host, const unsigned short port,
					const unsigned n_ports, VS_StreamClientsBM::ReceiverAppeared *rai )
	{
		EnterCriticalSection( &sect );
		bool   ret = InitAct( host, port, n_ports, rai );
		LeaveCriticalSection( &sect );
		return ret;
	}
	// end VS_StreamClientsBM_Implementation::Init

	inline void ShutdownAct( void )
	{
		if (hthr) {	if (events && SetEvent( events[1] )
							&& WaitForSingleObject( hthr, 60000 ) == WAIT_OBJECT_0)
						CloseHandle( hthr );
					hthr = 0;	}
		if (indexes) {		free( (void *)indexes );	indexes = 0;	}
		if (events) {	if (events[1])	CloseHandle( events[1] );
						if (events[0])	CloseHandle( events[0] );
						free( (void *)events );		events = 0;		}
		if (event) {	CloseHandle( event );	event = 0;	}
		if (conns) {	for (unsigned i = 0; i < n_ports; ++i)
						if (conns[i])	delete conns[i];
						free( (void *)conns );		conns = 0;		}
		rai = 0;	n_ports = 0;	port = 0;
		if (ip) {	free( (void *)ip );		ip = br_ip = 0;		}
		if (host) {		free( (void *)host );	host = 0;	}
		isInit = false;
	}
	// end VS_StreamClientsBM_Implementation::ShutdownAct

	inline void Shutdown( void )
	{
		EnterCriticalSection( &sect );
		ShutdownAct();
		LeaveCriticalSection( &sect );
	}
	// end VS_StreamClientsBM_Implementation::Shutdown

	inline bool SetNewSenderAct( VS_StreamClientSender *sender, unsigned *d_port, const unsigned n_port )
	{
		bool   ret = result = false;
		if (!isInit || !sender || (n_port >= n_ports && n_port != ~0))	return ret;
		VS_StreamClientsBM_Implementation::sender = sender;
		VS_StreamClientsBM_Implementation::n_port = n_port;
		if (SetEvent( events[0] ) && WaitForSingleObject( event, 600000 ) == WAIT_OBJECT_0)
		{	if (result) {	ret = result;
				if (d_port)		*d_port = VS_StreamClientsBM_Implementation::n_port;	}}
		return ret;
	}
	// end VS_StreamClientsBM_Implementation::SetNewSenderAct

	inline bool SetNewSender( VS_StreamClientSender *sender, unsigned *d_port, const unsigned n_port )
	{
		EnterCriticalSection( &sect );
		bool   ret = SetNewSenderAct( sender, d_port, n_port );
		LeaveCriticalSection( &sect );
		return ret;
	}
	// end VS_StreamClientsBM_Implementation::SetNewSender

	inline void Thread( void )
	{
		DWORD   nCounts, res;	unsigned char   rb;
		while (1)
		{
			nCounts = 2;
			for (unsigned i = 0; i < n_ports; ++i)
			{
				VS_ConnectionUDP   *&conn = conns[i];
				if (!conn && VS_BindPortIsFree( ip, port + i ))
				{
					conn = new VS_ConnectionUDP;
					if (!conn)	break;
					if (!conn->IsValid() || !conn->Bind( ip, port + i, false)
						|| !conn->CreateOvReadEvent()
						|| !conn->Read( (void *)&rb, sizeof(rb) ))
					{	delete conn;	conn = 0;	continue;	}
				}
				if (conn)
				{
					events[nCounts] = (HANDLE)conn->OvReadEvent();
					indexes[nCounts] = i;	++nCounts;
			}	}
			res = WaitForMultipleObjects( nCounts, events, FALSE, 2000 );
			if (res == WAIT_TIMEOUT)	;
			else if (res > ( WAIT_OBJECT_0 + 1 ) && res < ( WAIT_OBJECT_0 + nCounts ))
			{
				const unsigned   index = indexes[res - WAIT_OBJECT_0];
				if (index >= n_ports)	break;
				VS_ConnectionUDP   *conn = conns[index];	conns[index] = 0;
				if (!conn)		break;
				delete conn;
				conn = new VS_ConnectionUDP;
				if (!conn)	break;
				if (!conn->IsValid() || !conn->Bind( ip, port + index,false ))
				{	delete conn;	continue;	}
				void   *buffer = malloc( 65536 );
				char   from_host[64] = { 0 };	unsigned short   from_port = 0;
				unsigned long   mills = 2000;
				int   ret = conn->ReceiveFrom( buffer, 65536, from_host, sizeof(from_host), &from_port, mills );
				if (ret < 0) {		free( buffer );		delete conn;	continue;	}
				VS_StreamClientReceiver   *rcv = new VS_StreamClientReceiver;
				if (!rcv)	break;
				if (!rcv->IsValid() || !rcv->SetConnection( conn ))
				{	delete rcv;		free( buffer );		delete conn;	continue;	}
				stream::Track track = {};
				mills = 2000;
				ret = rcv->ReceiveFrame( buffer, 65536, &track, &mills );
				free( buffer );
				if (ret < 0)	delete rcv;
				else			rai->NewReceiver( rcv, from_host, index );
			}
			else if (res == ( WAIT_OBJECT_0 + 0 ))
			{
				if (n_port >= n_ports)
				{
					unsigned i = 0;
					for (; i < n_ports && !conns[i]; ++i);
					if (i < n_ports)	n_port = i;
				}
				if (n_port < n_ports)
				{
					VS_ConnectionUDP   *conn = conns[n_port];	conns[n_port] = 0;
					if (conn)	delete conn;
					conn = new VS_ConnectionUDP;
					if (conn && (!conn->IsValid()
						|| !conn->Connect( ip, port + n_port, br_ip, port + n_port,false )))
					{	delete conn;	conn = 0;	}
					if (conn)
					{
						if (!sender->IsValid() || !sender->SetConnection( conn ))
						{	delete conn;	conn = 0;	}
						else	result = true;
				}	}
				if (!SetEvent( event ))		break;
			}
			else	break;
	}	}
	// end VS_StreamClientsBM_Implementation::Thread

	static unsigned __stdcall Thread( void *arg )
	{
		vs::SetThreadName("StreamClientBM");
		if (arg)	((VS_StreamClientsBM_Implementation *)arg)->Thread();
		_endthreadex( 0 );		return 0;
	}
	// end VS_StreamClientsBM_Implementation::Thread
};
// end VS_StreamClientsBM_Implementation struct

VS_StreamClientsBM::VS_StreamClientsBM( void )
{	imp = new VS_StreamClientsBM_Implementation();	}
// end VS_StreamClientsBM::VS_StreamClientsBM

VS_StreamClientsBM::VS_StreamClientsBM( const char *host, const unsigned short port,
										   const unsigned n_ports, ReceiverAppeared *rai )
{	imp = new VS_StreamClientsBM_Implementation( host, port, n_ports, rai );	}
// end VS_StreamClientsBM::VS_StreamClientsBM

VS_StreamClientsBM::~VS_StreamClientsBM( void ) {	if (imp)	delete imp;		}
// end VS_StreamClientsBM::~VS_StreamClientsBM

bool VS_StreamClientsBM::IsInit( void ) const {		return !imp ? false : imp->isInit;	}
// end VS_StreamClientsBM::IsInit

bool VS_StreamClientsBM::Init( const char *host, const unsigned short port,
									const unsigned n_ports, ReceiverAppeared *rai )
{	return !imp ? false : imp->Init( host, port, n_ports, rai );	}
// end VS_StreamClientsBM::Init

void VS_StreamClientsBM::Shutdown( void ) {		if (imp)	imp->Shutdown();	}
// end VS_StreamClientsBM::Shutdown

bool VS_StreamClientsBM::SetNewSender( VS_StreamClientSender *sender,
										unsigned *d_port, const unsigned n_port )
{	return !imp ? 0 : imp->SetNewSender( sender, d_port, n_port );	}
// end VS_StreamClientsBM::SetNewSender

#endif
