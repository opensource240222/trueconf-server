/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: Непосредственная реализация клиента протокола медиа потоков
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_StreamClientTypes.h
/// \brief Непосредственная реализация клиента протокола медиа потоков
/// \note
///

#ifndef VS_STREAM_CLIENT_TYPES_H
#define VS_STREAM_CLIENT_TYPES_H

#define VS_COUNT_OF_FIRST_SYN_MESS		10
#define VS_COUNT_OF_SYN_ACK_MESS		5
#define VS_COUNT_OF_ACK_MESS			5
#define VS_RECONNECT_DEFAULT_TIMEOUT	10000

#ifdef _DEBUG
	#define dbgprintf printf
#else
	#define dbgprintf (void)
#endif
//#define   _MY_DEBUG_

#include <stdio.h>
#include <string>
#include <process.h>
#include <windows.h>

#include "../Handshake.h"
#include "../Protocol.h"
#include "../VS_StreamsDefinitions.h"
#include "../../acs/VS_AcsDefinitions.h"
#include "../../acs/ConnectionManager/VS_ConnectionManager.h"
#include "acs/connection/VS_ConnectionTCP.h"
#include "acs/connection/VS_ConnectionUDP.h"
#include "../../acs/Lib/VS_AcsLib.h"
#include "../../acs/Lib/VS_AcsLibDefinitions.h"
#include "../../std/cpplib/VS_SimpleStr.h"
#include "../../transport/Client/VS_TransportClient.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/ThreadUtils.h"
#include "std-generic/cpplib/hton.h"
#include "VS_ConnectNHPExecutor.h"
#include "../NHP/VS_NHP_Types.h"
#include "../NHP/VS_NHP_Handshake_Impl.h"
#include "acs/Lib/VS_IPPortAddress.h"

#include "VSClient/VS_Dmodule.h"

#include "time.h"

#include "in6addr.h"

#define   VS_MAX_NUMBER_STREAM_CLIENTS   2048
#define   VS_MIN_CONNECT_TIMEOUT   2000
#define   VS_MAX_CONNECT_TIMEOUT   60000


struct VS_StreamClient_Implementation
{
	inline bool AddThis( void )
	{
		EnterCriticalSection( &sect );
		if (nClients >= VS_MAX_NUMBER_STREAM_CLIENTS)
		{	LeaveCriticalSection( &sect );	return false;	}
		if (!nClients)
		{	headClient = endClient = this;	previousClient = nextClient = 0;	}
		else
		{	endClient->nextClient = this;	previousClient = endClient;
			endClient = this;	nextClient = 0;		}
		++nClients;
		LeaveCriticalSection( &sect );
		return true;
	}
	// end AddThis

	VS_StreamClient_Implementation(stream::ClientType type) :
		clientId(++clientIdSequence), type(stream::ClientType::uninstalled),
		sendMutex(CreateMutex( NULL, FALSE, NULL )),
		recvMutex(CreateMutex( NULL, FALSE, NULL )),
		connEvent(CreateEvent( 0, TRUE, TRUE, 0 )), argConnEvent(INVALID_HANDLE_VALUE),connNHPEvent(0),connectDirectUDPEvent(0),
		conn(0), sendBytes(0), recvBytes(sizeof(stream::FrameHeader)), stateRecv(0),
		oldSysTC(0), diffClnTC(0), currClnTC(0),
		connectTimeout(VS_CONNECT_TIMEOUT), sendConnectTimeout(0), recvConnectTimeout(0),connectDirectUdpTimeout(0),
		connectType(vs_stream_client_connect_type_unconnected), host(0), port(0),
		handle(0), isConnectThreadStarted(false),isNagleDisable(true),UIDR(0),UIDS(0),rtpDataLen(0),m_qos(false),
		m_qos_params(NULL),
		pLastIPV4From(0),pLastIPV6From(0),pLastPortFrom(0),lastAddressFamily(VS_IPPortAddress::ADDR_UNDEF)
	{
		memset( (void *)conferenceName, 0, sizeof(conferenceName) );
		memset( (void *)participantName, 0, sizeof(participantName) );
		memset( (void *)connectedParticipantName, 0, sizeof(connectedParticipantName) );
		memset( (void *)connectedEndpointName, 0, sizeof(connectedEndpointName) );
		memset( (void *)ftracks, 0, sizeof(ftracks) );
		memset( rbufrtp,0,65535);
		memset(lastAddrFrom,0,sizeof(lastAddrFrom));
		if (!sendMutex || !recvMutex || !connEvent || !AddThis())	return;
		VS_StreamClient_Implementation::type = type;
	}
	// end VS_StreamClient_Implementation::VS_StreamClient_Implementation
	virtual void FakeMethod(){}
	inline void DeleteThis( void )
	{
		if (nClients == 1)		;
		else if (this == headClient)
		{	headClient = nextClient;	headClient->previousClient = 0;	}
		else if (this == endClient)
		{	endClient = previousClient;		endClient->nextClient = 0;	}
		else
		{	nextClient->previousClient = previousClient;	previousClient->nextClient = nextClient;	}
		--nClients;
		if (!nClients)		headClient = endClient = 0;
	}
	// end DeleteThis
	void DeleteConn()
	{
		if(!conn)
			return;
		if (type != stream::ClientType::direct_udp)
			delete conn;
		conn = 0;
	}

	~VS_StreamClient_Implementation( void )
	{
		boost::shared_ptr<VS_ConnectNHPExecutor> nhp = m_nhp.lock();
		if(nhp)
			nhp->Stop();
		EnterCriticalSection( &sect );
		EnterMutexes();
		if (conn)
			DeleteConn();
		if (host)	free( (void *)host );
		CloseHandle( connEvent );
		CloseHandle( recvMutex );
		CloseHandle( sendMutex );
		DeleteThis();
		LeaveCriticalSection( &sect );
	}
	// end VS_StreamClient_Implementation::~VS_StreamClient_Implementation
	boost::weak_ptr<VS_ConnectNHPExecutor> m_nhp;

	static bool directUdpConnEstablished;

	// In static init methods we create two connections:
	// - for ipv4 address space
	// - for ipv6 address space
	// When we set connection to another client, we choose one of this
	// connections and copy it to udpConn variable.
	// So, this class in one moment of time us either udpConnV4 or udpConnV6.
	static VS_ConnectionUDP	*udpConnV4;
	static VS_ConnectionUDP	*udpConnV6;
	static VS_ConnectionUDP	*udpConn;

	static VS_AcsLog		*m_udpConnLogger;

	// we have two endpoints: for ipv4 and for ipv6 address spaces
	static VS_IPPortAddress bindUDPv4;
	static VS_IPPortAddress bindUDPv6;

	static HANDLE	hThreadHandshakeDirectUDP;


	VS_IPPortAddress directUDP;

	char			lastAddrFrom[512];
	unsigned long	*pLastIPV4From;
	in6_addr		*pLastIPV6From;
	// one of VS_IPPortAddress::ADDR_* constants
	unsigned		lastAddressFamily;
	unsigned short	*pLastPortFrom;



	const unsigned long   clientId;
	stream::ClientType type;
	HANDLE   sendMutex, recvMutex, connEvent, argConnEvent, /*reconnEvent,*/connNHPEvent, connectDirectUDPEvent;
	VS_ConnectionSock   *conn;
	stream::FrameHeader sendFrame, recvFrame;
	///fix ambiguous
	stream::UDPFrameHeader uSendFrame, uRecvFrame;
	unsigned long UIDR,UIDS;
	///fix ambiguous end
	int   sendBytes, recvBytes, stateRecv;
	unsigned long   oldSysTC, diffClnTC, currClnTC;
	unsigned long   connectTimeout, sendConnectTimeout, recvConnectTimeout, connectDirectUdpTimeout;
	VS_StreamClient_ConnectType   connectType;
	char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1],
			participantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1],
			connectedParticipantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1],
			connectedEndpointName[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
#define ip_addr_len_vs_sct 16
	char source_ip_addr[ip_addr_len_vs_sct];
	unsigned char   ftracks[256];
	char			rbufrtp[65535];
	int				rtpDataLen;

	char			udp_hs_header_buff[0xffff];
	char			udp_hs_ok_buff[0xffff];
	char   *host;	unsigned short   port;
	void   *handle;
	bool   isConnectThreadStarted;
	bool isNagleDisable;
	bool				m_qos;
	_QualityOfService					*m_qos_params;



	static inline bool InitUdpListener(unsigned long ip, const unsigned short port)
	{
		EnterCriticalSection(&sect);
		if(udpConnV4)
		{
			LeaveCriticalSection(&sect);
			return false;
		}
		udpConnV4 = new VS_ConnectionUDP;
		if(!udpConnV4->Bind(ip,port,false) || !udpConnV4->CreateOvReadEvent() || !udpConnV4->CreateOvWriteEvent() )
		{
			//m_udpConnLogger->TPrintf("bind udpConn failed: host = %s; port = %d",host,port);
			delete udpConnV4;
			udpConnV4 = 0;
			LeaveCriticalSection(&sect);
			return false;
		}
		else
		{
			bindUDPv4.ipv4(ip);
			bindUDPv4.port(port);
			//m_udpConnLogger->TPrintf("udpConn is binded: host = %s; port = %d",bindUdpHost,bindUdpPort);
			LeaveCriticalSection(&sect);
			return true;
		}
	}

	static inline bool InitUdpListener(in6_addr ip, const unsigned short port)
	{
		EnterCriticalSection(&sect);
		if(udpConnV6)
		{
			LeaveCriticalSection(&sect);
			return false;
		}
		udpConnV6 = new VS_ConnectionUDP;
		if(!udpConnV6->Bind(ip,port,false) || !udpConnV6->CreateOvReadEvent() || !udpConnV6->CreateOvWriteEvent() )
		{
			//m_udpConnLogger->TPrintf("bind udpConn failed: host = %s; port = %d",host,port);
			delete udpConnV6;
			udpConnV6 = 0;
			LeaveCriticalSection(&sect);
			return false;
		}
		else
		{
			bindUDPv6.ipv6(ip);
			bindUDPv6.port(port);
			//m_udpConnLogger->TPrintf("udpConn is binded: host = %s; port = %d",bindUdpHost,bindUdpPort);
			LeaveCriticalSection(&sect);
			return true;
		}
	}

	static inline bool InitUdpListener()
	{
		if(!m_udpConnLogger)
		{
#ifndef _DEBUG
			m_udpConnLogger = new VS_AcsEmptyLog();
#else
			m_udpConnLogger = new VS_AcsLog("udpDirect.log",10000000,4999999,"./");
#endif
		}
		// if we have at least one connection.
		if(bindUDPv4.port() || bindUDPv6.port()) return true;
		char my_host_name[256] = {0};

		if(!VS_GetDefaultHostName(my_host_name,256))
			return false;

		unsigned long	ip4;
		in6_addr		ip6;
		bool			has_ip4, has_ip6;
		VS_GetIPAddressesByHostName(my_host_name, &ip4, &ip6, has_ip4, has_ip6);

		if(!has_ip4 && !has_ip6) return false;

		unsigned long port(0), portv4(0), portv6(0);
		VS_RegistryKey key(true, "Current configuration");
		if(!key.GetValue(&port,sizeof(port),VS_REG_INTEGER_VT,"UDPPort") || !port || (port>0xffff) )
			return false;

		bool v4_binded = false;
		bool v6_binded = false;

		portv4 = port;
		while(portv4 < 0xffff && !InitUdpListener(vs_ntohl(ip4), (unsigned short)portv4))
			portv4+=37;
		if(portv4<0xffff) v4_binded = true;

		portv6 = port;
		while(portv6 < 0xffff && !InitUdpListener(ip6,(unsigned short)portv6))
			portv6+=37;
		if(portv6<0xffff) v6_binded = true;

		// if we have at least one connection.
		return v4_binded || v6_binded;
	}

	static inline void FreeUdpListener()
	{
		EnterCriticalSection(&sect);
		if(directUdpConnEstablished)
		{
			LeaveCriticalSection(&sect);
			return;
		}
		if(hThreadHandshakeDirectUDP)
		{
			WaitForSingleObject(hThreadHandshakeDirectUDP,INFINITE);
			CloseHandle(hThreadHandshakeDirectUDP);
			hThreadHandshakeDirectUDP = 0;
		}
		if(udpConnV4)
			delete udpConnV4;
		if(udpConnV6)
			delete udpConnV6;
		bindUDPv4 = VS_IPPortAddress();
		bindUDPv6 = VS_IPPortAddress();
		LeaveCriticalSection(&sect);
	}
	static inline bool GetBindedUdpAddressV4(char *host, unsigned long host_sz, unsigned short &port)
	{
		bool bRes(false);
		EnterCriticalSection(&sect);

		if(bindUDPv4.port() && !bindUDPv4.isZero())
		{
			if(bindUDPv4.GetHostByIp(host, host_sz))
			{
				port = bindUDPv4.port();
				bRes = true;
			}
			else
				bRes = false;
		}
		else
			bRes = false;

		LeaveCriticalSection(&sect);
		return bRes;
	}

	static inline bool GetBindedUdpAddressV6(char *host, unsigned long host_sz, unsigned short &port)
	{
		bool bRes(false);
		EnterCriticalSection(&sect);

		if(bindUDPv6.port() && !bindUDPv6.isZero())
		{
			if(bindUDPv6.GetHostByIp(host, host_sz))
			{
				port = bindUDPv6.port();
				bRes = true;
			}
			else
				bRes = false;
		}
		else
			bRes = false;

		LeaveCriticalSection(&sect);
		return bRes;
	}

	static inline bool UdpIsBinded()
	{
		// at least one connection
		return bindUDPv4.port() || bindUDPv6.port();
	}

	static inline bool AdhereToConnectionManager( void )
	{	return VS_AddAcceptHandler(stream::PrimaryField, AcceptHandler, 0) > 0;	}
	// end VS_StreamClient_Implementation::AdhereToConnectionManager

	inline bool CmpMTracks( const unsigned char *mtracks )
	{
		uint8_t ftracks[256];
		stream::MTracksToFTracks(ftracks, mtracks);
		return !memcmp( ftracks, VS_StreamClient_Implementation::ftracks, 256 );
	}
	// end VS_StreamClient_Implementation::CmpMTracks

	inline void ConnectionAcceptHandlerAct( VS_ConnectionSock *conn,
							const unsigned char *read_mtracks, unsigned long &milliseconds )
	{
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::ConnectionAcceptHandlerAct( conn: %08X, read_mtracks: %08X, milliseconds: %u )\n", _MD_POINT_(conn), _MD_POINT_(read_mtracks), milliseconds );
#endif
		if (!conn) {	CloseConnectionAct();	return;		}
		uint8_t mtracks[32];
		stream::FTracksToMTracks(mtracks, ftracks);
		auto hs = stream::CreateHandshakeResponse(mtracks);
		if (!hs || !VS_WriteZeroHandshake(conn, hs.get(), milliseconds))
		{
			CloseConnectionAct();
			return;
		}
		stream::MTracksIntersection(mtracks, read_mtracks);
		stream::MTracksToFTracks(ftracks, mtracks);
		if (!SetConnectionAct( conn, milliseconds ))
		{
			CloseConnectionAct();
			return;
		}
	}
	// end VS_StreamClient_Implementation::ConnectionAcceptHandlerAct

	inline void ConnectionConnectHandlerAct( VS_ConnectionSock *conn,
							const unsigned char *read_mtracks, unsigned long &milliseconds )
	{
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::ConnectionConnectHandlerAct( conn: %08X, read_mtracks: %08X, milliseconds: %u )\n", _MD_POINT_(conn), _MD_POINT_(read_mtracks), milliseconds );
#endif
		if (!conn)
		{	CloseConnectionAct();	return;		}
		if (read_mtracks)
		{
			uint8_t mtracks[32];
			stream::FTracksToMTracks(mtracks, ftracks);
			stream::MTracksIntersection(mtracks, read_mtracks);
			stream::MTracksToFTracks(ftracks, mtracks);
		}
		if (!SetConnectionAct( conn, milliseconds ))
		{
			delete conn;
			CloseConnectionAct();
			return;
		}
	}
	// end VS_StreamClient_Implementation::ConnectionConnectHandlerAct

	inline void ConnectionConnectHostHandlerAct( VS_ConnectionSock *conn,
										HANDLE connectEvent, unsigned long &milliseconds )
	{
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::ConnectionConnectHostHandlerAct( conn: %08X, connectEvent: %08X, milliseconds: %u )\n", _MD_POINT_(conn), _MD_POINT_(connectEvent), milliseconds );
#endif
		if (!conn)	CloseConnectionAct();
		else if (!SetConnectionAct( conn, milliseconds ))
		{
			delete conn;
			CloseConnectionAct();
		}
		if (connectEvent)	SetEvent( connectEvent );
	}
	// end VS_StreamClient_Implementation::ConnectionConnectHostHandlerAct

	static void AcceptHandler( const void *cb_arg, VS_ConnectionTCP *conn,
											net::HandshakeHeader *hs)
	{
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::AcceptHandler( cb_arg: %08X, conn: %08X, hs: %08X )\n", _MD_POINT_(cb_arg), _MD_POINT_(conn), _MD_POINT_(hs) );
#endif
		const uint8_t* read_mtracks;
		const char* conferenceName;
		const char* participantName;
		const char* connectedParticipantName;
		const char* connectedEndpointName;
		stream::ClientType type;
		if (conn && stream::GetHandshakeFields(hs, type, conferenceName, participantName,
				connectedParticipantName, connectedEndpointName, read_mtracks))
		{	unsigned long   mills;
			do
			{	EnterCriticalSection( &sect );
				VS_StreamClient_Implementation   *client = headClient;
				for (unsigned i = 0; i < nClients && client; ++i, client = client->nextClient )
					if (client->type == type)
					{	client->EnterMutexes();
						if (client->connectType == vs_stream_client_connect_type_accept
								&& !strcmp( conferenceName, client->conferenceName )
								&& !strcmp( participantName, client->connectedParticipantName )
								&& !strcmp( connectedParticipantName, client->participantName )
								&& !strcmp( connectedEndpointName, client->connectedEndpointName ))
						{	LeaveCriticalSection( &sect );
							unsigned long   mills = client->connectTimeout;
							client->ConnectionAcceptHandlerAct( conn, read_mtracks, mills );
							client->LeaveMutexes();			free( (void *)hs );		return;
						}
						client->LeaveMutexes();
					}
				LeaveCriticalSection( &sect );		mills = 250;
			} while (!conn->SelectIn( mills ));
		}
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::AcceptHandler: delete conn: %08X\n", _MD_POINT_(conn) );
#endif
		if (conn)
			delete conn;
		if (hs)		free( (void *)hs );
	}
	// end VS_StreamClient_Implementation::AcceptHandler

	static inline void ConnectHandler( const unsigned long clientId,
						stream::ClientType type, const char *conferenceName,
						const char *participantName, const char *connectedParticipantName,
						const char *connectedEndpointName,
						const unsigned char *mtracks, unsigned long &milliseconds,
						bool isNagleDisable = true, bool m_qos = false, _QualityOfService *m_qos_params = NULL )
	{
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::ConnectHandler( clientId: %u, type: %s, conferenceName: %s, participantName: %s, connectedParticipantName: %s, connectedEndpointName: %s, mtracks: %08X, milliseconds: %u )\n", clientId, _MD_TYPE_(type), _MD_STR_(conferenceName), _MD_STR_(participantName), _MD_STR_(connectedParticipantName), _MD_STR_(connectedEndpointName), _MD_POINT_(mtracks), milliseconds );
#endif
		VS_ConnectionSock   *conn = 0;
		const uint8_t* read_mtracks = 0;
		char   epNm[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
		const auto ep_name = VS_EndpointName(epNm, sizeof(epNm));
		auto hs = stream::CreateHandshake(type, conferenceName ? conferenceName : "", participantName ? participantName : "", connectedParticipantName ? connectedParticipantName : "", ep_name ? ep_name : "", mtracks);
		if (hs)
		{
			conn = VS_CreateConnection( connectedEndpointName, milliseconds, 0, 0, 0, isNagleDisable, m_qos, m_qos_params );
			if (conn && !VS_WriteZeroHandshake(conn, hs.get(), milliseconds))
			{
				delete conn;
				conn = 0;
			}
			net::HandshakeHeader* response_hs = nullptr;
			if (conn && (!VS_ReadZeroHandshake(conn, &response_hs, milliseconds) || !stream::GetHandshakeResponseFields(response_hs, read_mtracks)))
			{
				delete conn;
				conn = 0;
			}
			free(response_hs);
		}
		EnterCriticalSection( &sect );
		VS_StreamClient_Implementation   *client = headClient;
		for (unsigned i = 0; i < nClients && client; ++i, client = client->nextClient )
			if (client->clientId == clientId && client->type == type)
			{
				client->EnterMutexes();
				if (client->connectType == vs_stream_client_connect_type_connect
					&& !strcmp( conferenceName, client->conferenceName )
					&& !strcmp( participantName, client->participantName )
					&& !strcmp( connectedParticipantName, client->connectedParticipantName )
					&& !strcmp( connectedEndpointName, client->connectedEndpointName )
					&& client->CmpMTracks( mtracks ))
				{
					LeaveCriticalSection( &sect );
					client->ConnectionConnectHandlerAct( conn, read_mtracks, milliseconds );
					client->LeaveMutexes();
					return;
				}
				client->LeaveMutexes();
			}
		LeaveCriticalSection( &sect );
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::ConnectHandler: delete conn: %08X\n", _MD_POINT_(conn) );
#endif
		delete conn;
	}
	// end VS_StreamClient_Implementation::ConnectHandler

	struct ArgConnectStream
	{
		ArgConnectStream( VS_StreamClient_Implementation &imp )
		{
			clientId = imp.clientId;
			type = imp.type;
			strcpy( conferenceName, imp.conferenceName );
			strcpy( participantName, imp.participantName );
			strcpy( connectedParticipantName, imp.connectedParticipantName );
			strcpy( connectedEndpointName, imp.connectedEndpointName );
			stream::FTracksToMTracks(mtracks, imp.ftracks);
			connectTimeout = imp.connectTimeout;
			isNagleDisable = imp.isNagleDisable;
			m_qos = imp.m_qos;
			m_qos_params = imp.m_qos_params;
		}
		// end ArgConnectStream::ArgConnectStream
		unsigned long   clientId;
		stream::ClientType type;
		char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1],
				participantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1],
				connectedParticipantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1],
				connectedEndpointName[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
		unsigned char   mtracks[32];
		unsigned long   connectTimeout;
		bool isNagleDisable;
		bool m_qos;
		_QualityOfService *m_qos_params;
	};
	// end ArgConnectStream struct

	static void __cdecl ConnectThread( void *arg )
	{
		vs::SetThreadName("SC_Connect");
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::ConnectThread( arg: %08X )\n", _MD_POINT_(arg) );
#endif
		if (arg)
		{
			ArgConnectStream   *c = (ArgConnectStream *)arg;
			ConnectHandler( c->clientId, c->type, c->conferenceName, c->participantName, c->connectedParticipantName, c->connectedEndpointName, c->mtracks, c->connectTimeout , c->isNagleDisable, c->m_qos, c->m_qos_params );
			delete c;
		}
		_endthread();
	}
	// end VS_StreamClient_Implementation::ConnectThread

	inline bool StartConnectThread( void )
	{
		if (isConnectThreadStarted)
			return true;
#ifdef _MY_DEBUG_
puts( "StartConnectThread()" );
#endif
		bool   ret = false;
		ArgConnectStream   *arg = new ArgConnectStream( *this );
		if (arg)
		{
			uintptr_t   thr = _beginthread( ConnectThread, 0, (void *)arg );
			if (thr != -1L && thr)
				ret = true;
			else
				delete arg;
		}
		return isConnectThreadStarted = ret;
	}
	// end VS_StreamClient_Implementation::StartConnectThread

	static inline void ConnectHostHandler( const unsigned long clientId,
							stream::ClientType type,
							const VS_StreamClient_ConnectType connectType, const char *host,
							const unsigned short port, const unsigned long connectTimeout,
							HANDLE connectEvent, VS_ConnectionTCP *conn )
	{
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::ConnectHostHandler( clientId: %u, type: %s, connectType: %s, host: %s, port: %u, connectTimeout: %u, connectEvent: %08X )\n", clientId, _MD_TYPE_(type), _MD_CONNECT_TYPE_(connectType), _MD_STR_(host), port, connectTimeout, _MD_POINT_(connectEvent) );
#endif
		unsigned long   mills = connectTimeout;
		if (conn)
		{
			if (conn->State() != vs_sock_state_created)
			{
				delete conn;
				conn = 0;
			}
			else
			{
				switch (connectType)
				{
				case vs_stream_client_connect_type_host_accept :
					if (!conn->Accept( host, port, mills,false))
					{
						delete conn;
						conn = 0;
					}
					break;
				case vs_stream_client_connect_type_host_connect :
					if (!conn->Connect( host, port, mills ))
					{
						delete conn;
						conn = 0;
					}
					break;
				default :
					delete conn;
					conn = 0;
		}	}	}
		EnterCriticalSection( &sect );
		VS_StreamClient_Implementation   *client = headClient;
		for (unsigned i = 0; i < nClients && client; ++i, client = client->nextClient )
			if (client->clientId == clientId && client->type == type)
			{
				client->EnterMutexes();
				if (client->connectType == connectType && client->port == port
					&& client->host && !strcmp( client->host, host ))
				{
					LeaveCriticalSection( &sect );
					client->ConnectionConnectHostHandlerAct( conn, connectEvent, mills );
					client->LeaveMutexes();		return;
				}
				client->LeaveMutexes();
			}
		LeaveCriticalSection( &sect );
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::ConnectHostHandler: delete conn: %08X\n", _MD_POINT_(conn) );
#endif
		if (conn)
			delete conn;
	}
	// end VS_StreamClient_Implementation::ConnectHostHandler

	struct ArgConnectHostStream
	{
		ArgConnectHostStream( VS_StreamClient_Implementation &imp,
								const unsigned long milliseconds, HANDLE connectEvent,
								VS_ConnectionTCP *conn )
		{
			clientId = imp.clientId;
			type = imp.type;
			connectType = imp.connectType;
			connectTimeout = milliseconds;
			host = _strdup( imp.host );	port = imp.port;
			ArgConnectHostStream::connectEvent = connectEvent;
			ArgConnectHostStream::conn = conn;
		}
		// end ArgConnectHostStream::ArgConnectHostStream
		~ArgConnectHostStream( void ) {		if (host)	delete host;	}
		// end ArgConnectHostStream::~ArgConnectHostStream
		unsigned long   clientId;
		stream::ClientType type;
		VS_StreamClient_ConnectType   connectType;
		char   *host;	unsigned short   port;
		unsigned long   connectTimeout;
		HANDLE   connectEvent;
		VS_ConnectionTCP   *conn;
	};
	// end ArgConnectHostStream struct

	static void __cdecl ConnectHostThread( void *arg )
	{
		vs::SetThreadName("SC_ConnectHost");
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::ConnectHostThread( arg: %08X )\n", _MD_POINT_(arg) );
#endif
		if (arg)
		{
			ArgConnectHostStream   *c = (ArgConnectHostStream *)arg;
			ConnectHostHandler( c->clientId, c->type, c->connectType, c->host, c->port, c->connectTimeout, c->connectEvent, c->conn );
			delete c;
		}
		_endthread();
	}
	// end VS_StreamClient_Implementation::ConnectHostThread

	inline bool StartConnectHostThread( const unsigned long milliseconds, HANDLE connectEvent, VS_ConnectionTCP *conn )
	{
#ifdef _MY_DEBUG_
printf( "StartConnectHostThread( connectEvent: %08X )\n", _MD_POINT_(connectEvent) );
#endif
		bool   ret = false;
		ArgConnectHostStream   *arg = new ArgConnectHostStream( *this, milliseconds, connectEvent, conn );
		if (arg)
		{
			uintptr_t   thr = _beginthread( ConnectHostThread, 0, (void *)arg );
			if (thr != -1L && thr)		ret = true;
			else	delete arg;
		}
		return ret;
	}
	// end VS_StreamClient_Implementation::StartConnectHostThread

	static unsigned InitStreamClients( void )
	{
		InitializeCriticalSection( &sect );
		return 0;
	}
	// end VS_StreamClient_Implementation::InitStreamClients

	VS_StreamClient_Implementation			*previousClient, *nextClient;
	static VS_StreamClient_Implementation	*headClient, *endClient;
	static unsigned							nClients;
	static CRITICAL_SECTION					sect;
	static unsigned long					acceptTimeout;
	static const unsigned char				allTracks[];
	static unsigned long					clientIdSequence;

	//////////////////////////////////////////////////////////////////////////////////

	inline unsigned long NextTickCount( void )
	{
		unsigned long   tcNew = GetTickCount();
		diffClnTC = tcNew - oldSysTC;
		oldSysTC = tcNew;
		return currClnTC += diffClnTC;
	}
	// end VS_StreamClient_Implementation::NextTickCount

	inline int WaitConnEventEnterMutex( HANDLE hMutex, unsigned long &milliseconds )
	{
		HANDLE   hs[] = { connEvent, hMutex };
		DWORD   tc = GetTickCount();
		switch (WaitForMultipleObjects( 2, hs, TRUE, milliseconds ))
		{
		case WAIT_OBJECT_0 :	case WAIT_OBJECT_0 + 1 :	case WAIT_ABANDONED_0 :
		case WAIT_ABANDONED_0 + 1 :		tc = GetTickCount() - tc;
								milliseconds = milliseconds > tc ? milliseconds - tc : 0;
								return 0;
		case WAIT_TIMEOUT :		milliseconds = 0;	return -2;
		default :				return -1;
	}	}
	// end VS_StreamClient_Implementation::WaitConnEventEnterSendMutex

	inline int WaitConnEventEnterSendMutex( unsigned long &milliseconds )
	{	return WaitConnEventEnterMutex( sendMutex, milliseconds );	}
	// end VS_StreamClient_Implementation::WaitConnEventEnterSendMutex

	inline int WaitConnEventEnterRecvMutex( unsigned long &milliseconds )
	{	return WaitConnEventEnterMutex( recvMutex, milliseconds );	}
	// end VS_StreamClient_Implementation::WaitConnEventEnterRecvMutex

	inline int WaitConnEvent( unsigned long &milliseconds )
	{
		DWORD   tc = GetTickCount();
		switch (WaitForSingleObject( connEvent, milliseconds ))
		{
		case WAIT_OBJECT_0 :	tc = GetTickCount() - tc;
								milliseconds = milliseconds > tc ? milliseconds - tc : 0;
								return 0;
		case WAIT_TIMEOUT :		milliseconds = 0;	return -2;
		default :				return -1;
	}	}
	// end VS_StreamClient_Implementation::WaitConnEvent

	inline int WaitEnterMutex( HANDLE hMutex, unsigned long &milliseconds )
	{
		DWORD   tc = GetTickCount();
		switch (WaitForSingleObject( hMutex, milliseconds ))
		{
		case WAIT_OBJECT_0 :
		case WAIT_ABANDONED :	tc = GetTickCount() - tc;
								milliseconds = milliseconds > tc ? milliseconds - tc : 0;
								return 0;
		case WAIT_TIMEOUT :		milliseconds = 0;	return -2;
		default :				return -1;
	}	}
	// end VS_StreamClient_Implementation::WaitEnterMutex

	inline int WaitEnterSendMutex( unsigned long &milliseconds )
	{	return WaitEnterMutex( sendMutex, milliseconds );	}
	// end VS_StreamClient_Implementation::WaitEnterSendMutex

	inline int WaitEnterRecvMutex( unsigned long &milliseconds )
	{	return WaitEnterMutex( recvMutex, milliseconds );	}
	// end VS_StreamClient_Implementation::WaitEnterRecvMutex

	inline int EnterMutexes( void )
	{
		HANDLE   hs[] = { sendMutex, recvMutex };
		switch (WaitForMultipleObjects( 2, hs, TRUE, INFINITE ))
		{
		case WAIT_OBJECT_0 :
		case WAIT_ABANDONED_0 :		return 0;
		default :					return -1;
	}	}
	// end VS_StreamClient_Implementation::EnterMutexes

	inline void LeaveSendMutex( void )
	{	ReleaseMutex( sendMutex );	}
	// end VS_StreamClient_Implementation::LeaveSendMutex

	inline void LeaveRecvMutex( void )
	{	ReleaseMutex( recvMutex );	}
	// end VS_StreamClient_Implementation::LeaveRecvMutex

	inline void LeaveMutexes( void )
	{
		ReleaseMutex( recvMutex );
		ReleaseMutex( sendMutex );
	}
	// end VS_StreamClient_Implementation::LeaveMutexes

	inline bool ConnectUseHost( const VS_StreamClient_ConnectType connectType,
									const char *host, const unsigned short port,
									const unsigned long milliseconds,
									void *connectEvent, const bool waitForConnection )
	{
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::ConnectUseHost( connectType: %s, host: %s, port: %u, milliseconds: %u, waitForConnection: %s, connectEvent: %08X )\n", _MD_CONNECT_TYPE_(connectType), _MD_STR_(host), port, milliseconds, _MD_BOOL_(waitForConnection), _MD_POINT_(connectEvent) );
#endif
		bool ret;
		VS_ConnectionTCP   *conn = 0;
		EnterMutexes();
		if (VS_StreamClient_Implementation::connectType != vs_stream_client_connect_type_unconnected)
		{	LeaveMutexes();		return false;	}
		CloseConnectionAct();	ResetEvent( connEvent );
		memset( (void *)ftracks, true, sizeof(ftracks) );
		unsigned long   mills = milliseconds ? milliseconds : VS_CONNECT_TIMEOUT;
		if (!host || !*host)
		{
			VS_StreamClient_Implementation::host = (char *)malloc( MAX_PATH );
			if (!VS_StreamClient_Implementation::host
					|| !memset( (void *)VS_StreamClient_Implementation::host, 0, MAX_PATH )
					|| !VS_GetDefaultHostName( VS_StreamClient_Implementation::host, MAX_PATH ))
				goto go_return_false;
		}
		else if (!(VS_StreamClient_Implementation::host = _strdup( host )))
			goto go_return_false;
		VS_StreamClient_Implementation::port = port;
		VS_StreamClient_Implementation::connectType = connectType;
		conn = new VS_ConnectionTCP;
		if (!conn || !conn->Socket())	goto go_return_false;
		handle = conn->GetHandle();
		if (!StartConnectHostThread( mills, (HANDLE)connectEvent, conn ))	goto go_return_false;
		conn = 0;
		LeaveMutexes();
		if (!waitForConnection)		return true;
		mills = INFINITE;
		ret = !WaitConnEvent( mills );
		EnterMutexes();
		if (ret)
		{
			ret = VS_StreamClient_Implementation::connectType == connectType;
			LeaveMutexes();
			return ret;
		}
go_return_false:
		handle = 0;
		if (conn)
			delete conn;
		CloseConnectionAct();
		LeaveMutexes();
		return false;
	}
	// end ConnectUseHost

	inline bool Accept( const char *host, const unsigned short port, const unsigned long milliseconds,
											void *connectEvent, const bool waitForConnection )
	{
		return ConnectUseHost(vs_stream_client_connect_type_host_accept, host, port ? port : (type == stream::ClientType::sender ? 4444 : 4445), milliseconds, connectEvent, waitForConnection);
	}
	// end VS_StreamClient_Implementation::Accept

	inline bool Connect( const char *host, const unsigned short port, const unsigned long milliseconds,
											void *connectEvent, const bool waitForConnection )
	{
		return ConnectUseHost(vs_stream_client_connect_type_host_connect, host, port ? port : (type == stream::ClientType::sender ? 4445 : 4444), milliseconds, connectEvent, waitForConnection);
	}
	// end VS_StreamClient_Implementation::Connect


	inline bool ConnectNHPNotGuaranteed( const char *host, const unsigned short port,VS_ConnectionSock	*&out_conn,
										unsigned long *ip_out = 0, unsigned short * port_out = 0, const char *source_ip = NULL)
	{
		if (VS_StreamClient_Implementation::connectType != vs_stream_client_connect_type_unconnected)
			return false;

		VS_ConnectionUDP   *conn = 0;
		SetEvent( connEvent );
		if (argConnEvent != INVALID_HANDLE_VALUE)
		{
			SetEvent( argConnEvent );
			argConnEvent = INVALID_HANDLE_VALUE;
		}
		ResetEvent( connEvent );
		memset( (void *)ftracks, 1, sizeof(ftracks) );
		conn = new VS_ConnectionUDP;
		if (!conn)		return false;
		if (!conn->IsValid())
		{
			delete conn;
			return false;
		}
		///////////////////////////////////////////////////
		char   brIp[32] = { 0 }, my_host[32] = { 0 };
		if ((source_ip) && strlen(source_ip)) { strncpy(my_host, source_ip, 31); }
		else if(!VS_GetDefaultHostName( my_host , 32 )) return false;
		if (ip_out)
		{

			char ip_my[256]={0};
			VS_GetHostByName( my_host , ip_my , 255 );
			VS_GetIpByHost( ip_my, ip_out );
		}
		unsigned short fport = (port >1024? 100 : 1024) + port;
		while(!VS_BindPortIsFree( my_host, fport,VS_TEST_BIND_UDP))
		{
			fport++;
		}
		///////////////////////////////////////////////////
		if (port_out)
		{
			*port_out = fport;
		}
		switch (type)
		{
		case stream::ClientType::transmitter:
		case stream::ClientType::rtp:
		case stream::ClientType::sender:
			if (conn->Connect( my_host, fport, host, port,false))		goto go_ok;
			break;
		case stream::ClientType::receiver:
			if (conn->Connect( my_host, fport, host, port, false))		goto go_ok;
		}
		delete conn;
		return false;
go_ok:	//VS_StreamClient_Implementation::connectType = vs_stream_client_connect_not_guaranteed;
		unsigned long   mills = 10000;
		/*if (!SetConnectionAct( conn, mills ))
		{
			delete conn;
			return false;
		}*/
		if (!conn || !conn->CreateOvReadEvent() || !conn->CreateOvWriteEvent())
			return false;
		switch (type)
		{
		case stream::ClientType::sender:
			if (!conn->SetAsWriter( mills ))		return false;
			break;
		case stream::ClientType::receiver:
			if (!conn->SetAsReader( mills ))		return false;
			break;
		case stream::ClientType::transmitter:
		case stream::ClientType::rtp:
			if (!conn->SetSizeBuffers(-1,-1))			return false;
			break;
		default :	return false;
		}
		out_conn = conn;
		/*memset( ftracks, 1, sizeof(ftracks) );		*ftracks = 0;*/	return true;
	}
	inline bool ConnectNotGuaranteed( const char *host, const unsigned short port,
									    unsigned long * ip_out=0,unsigned short * port_out=0)
	{
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::ConnectNotGuaranteed( local_host: %s, target_host: %s, port: %u )\n", _MD_STR_(local_host), _MD_STR_(target_host), port );
#endif
		if (VS_StreamClient_Implementation::connectType != vs_stream_client_connect_type_unconnected)
			return false;

		VS_ConnectionUDP   *conn = 0;
		CloseConnectionAct();	ResetEvent( connEvent );
		memset( (void *)ftracks, 1, sizeof(ftracks) );
		conn = new VS_ConnectionUDP;
		if (!conn)		return false;
		if (!conn->IsValid())
		{
			delete conn;
			return false;
		}
		///////////////////////////////////////////////////
		char   brIp[32] = { 0 }, my_host[32] = { 0 };
		if(!VS_GetDefaultHostName( my_host , 32 )) return false;
		if (ip_out)
		{

			char ip_my[256]={0};
			VS_GetHostByName( my_host , ip_my , 255 );
			VS_GetIpByHost( ip_my, ip_out );
		}
		unsigned short fport = port+100;
		while(!VS_BindPortIsFree( my_host, fport,VS_TEST_BIND_UDP))
		{
			fport++;
		}
		///////////////////////////////////////////////////
		if (port_out)
		{
			*port_out = fport;
		}
		switch (type)
		{
		case stream::ClientType::transmitter:
		case stream::ClientType::rtp:
		case stream::ClientType::sender:
			if (conn->Connect( my_host, fport, host, port,false))		goto go_ok;
			break;
		case stream::ClientType::receiver:
			if (conn->Connect( my_host, fport, host, port,false))		goto go_ok;
		}
		delete conn;
		return false;
go_ok:	VS_StreamClient_Implementation::connectType = vs_stream_client_connect_not_guaranteed;
		unsigned long   mills = 10000;
		if (!SetConnectionAct( conn, mills ))
		{
			delete conn;
			return false;
		}
		memset( ftracks, 1, sizeof(ftracks) );		*ftracks = 0;	return true;
	}
	// end VS_StreamClient_Implementation::ConnectNotGuaranteed

	inline bool ConnectNotGuaranteedBroadcast( const char *host, const unsigned short port )
	{
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::ConnectNotGuaranteed( local_host: %s, target_host: %s, port: %u )\n", _MD_STR_(local_host), _MD_STR_(target_host), port );
#endif
		if (VS_StreamClient_Implementation::connectType != vs_stream_client_connect_type_unconnected)
			return false;
		char   brIp[32] = { 0 }, my_host[32] = { 0 };
		if (!VS_GetInetBroadcastAddr( host, brIp, sizeof(brIp) )
			|| !VS_GetHostByBroadcast( brIp, my_host, sizeof(my_host) ))	return false;
		VS_ConnectionUDP   *conn = 0;
		CloseConnectionAct();	ResetEvent( connEvent );
		memset( (void *)ftracks, 1, sizeof(ftracks) );
		conn = new VS_ConnectionUDP;
		if (!conn)		return false;
		if (!conn->IsValid())
		{
			delete conn;
			return false;
		}
		switch (type)
		{
		case stream::ClientType::sender:
			if (conn->Connect( my_host, port, brIp, port,false ))		goto go_ok;
			break;
		case stream::ClientType::receiver:
			if (conn->Connect( my_host, port, host, port,false))		goto go_ok;
		}
		delete conn;
		return false;
go_ok:	VS_StreamClient_Implementation::connectType = vs_stream_client_connect_not_guaranteed;
		unsigned long   mills = 10000;
		if (!SetConnectionAct( conn, mills ))
		{
			delete conn;
			return false;
		}
		memset( ftracks, 1, sizeof(ftracks) );		*ftracks = 0;	return true;
	}
	// end VS_StreamClient_Implementation::ConnectNotGuaranteed
	inline bool ConnectNotGuaranteedMulticast(	const char *mcast_host,
												const unsigned short port,
												const char *source_ip = NULL)
	{
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::ConnectNotGuaranteedMulticast( local_host: %s, target_host: %s, port: %u )\n", _MD_STR_(local_host), _MD_STR_(target_host), port );
#endif
		if (VS_StreamClient_Implementation::connectType != vs_stream_client_connect_type_unconnected)
			return false;
		VS_ConnectionUDP   *conn = 0;
		CloseConnectionAct();	ResetEvent( connEvent );
		memset( (void *)ftracks, 1, sizeof(ftracks) );
		conn = new VS_ConnectionUDP;
		if (!conn)		return false;
		if (!conn->IsValid())
		{
			delete conn;
			return false;
		}
		///////////////////////////////////////////////////
		char   brIp[32] = { 0 }, my_host[46] = { 0 };
		if ((source_ip) && strlen(source_ip)) { strncpy(my_host, source_ip, 45); }
		else if (!VS_GetDefaultHostName( my_host , 46 )) return false;
		unsigned short fport = port+100;
		while(!VS_BindPortIsFree( my_host, fport,VS_TEST_BIND_UDP))
		{
			fport++;
		}
		///////////////////////////////////////////////////
		switch (type)
		{
		case stream::ClientType::sender:
			if (conn->ConnectMulticast( my_host, fport, mcast_host, port, true , 0, false ))	break;

		case stream::ClientType::receiver:
			if (conn->ConnectMulticast( my_host, fport, mcast_host , port, false , 0, false))	break;

		default:
			delete conn;
			return false;
		}

		///////////////////////////////////////////////////
		VS_StreamClient_Implementation::connectType = vs_stream_client_connect_not_guaranteed;
		unsigned long   mills = 10000;
		if (!SetConnectionAct( conn, mills ))
		{
			delete conn;
			return false;
		}
		memset( ftracks, 1, sizeof(ftracks) );		*ftracks = 0;	return true;
	}
	// end VS_StreamClient_Implementation::ConnectNotGuaranteed

	// addressFamily - one of VS_IPPortAddress::ADDR_* constants.
	inline bool ConnectDirectUDP(unsigned addressFamily, const char *ip, const unsigned short port, const char *conf_name, const char *our_username, const char *peer_username, void * connectEvent, const unsigned long timeout)
	{

		if(directUdpConnEstablished || !port || !directUDP.SetIPFromAddressString(ip) ||
			!conf_name || (VS_STREAMS_MAX_SIZE_CONFERENCE_NAME<strlen(conf_name))||
			!our_username ||(VS_ACS_MAX_SIZE_ENDPOINT_NAME < strlen(our_username)) ||
			!peer_username || (VS_ACS_MAX_SIZE_ENDPOINT_NAME < strlen(peer_username)) ||
			!connectEvent  || hThreadHandshakeDirectUDP || !UdpIsBinded())
			return false;

//conferenceName
//participantName
//connectedParticipantName

		strcpy(conferenceName,conf_name);
		strcpy(participantName,our_username);
		strcpy(connectedParticipantName,peer_username);

		directUDP.port(port);
		//directUdpConnEstablished = true;
		//type = stream::ClientType::direct_udp;

		connectType = vs_stream_client_connect_not_guaranteed;
		connectDirectUDPEvent = connectEvent;
		connectDirectUdpTimeout = timeout;

		// Here we coohse, what kind of udp-connection will be used (ipv4 or ipv6).
		if(addressFamily == VS_IPPortAddress::ADDR_IPV4)
			udpConn = udpConnV4;
		else
			udpConn = udpConnV6;

		hThreadHandshakeDirectUDP = (HANDLE)_beginthreadex(0,0,ThreadConnectDirectUDP,this,0,0);
		if(!hThreadHandshakeDirectUDP)
			return false;

		type = stream::ClientType::direct_udp;
		m_udpConnLogger->TPrintf("ConnectDirectUdp start: ip_to = %s, port_to = %d, conf name = %s, part name = %s, peer name =%s",
			ip, port,conferenceName,participantName,connectedParticipantName);
		return true;
	}
	inline void SetNHPConn(VS_ConnectionSock *NhpConn, const unsigned long uidr,const unsigned long uids)
	{
		assert(!conn);
		UIDR = uidr;
		UIDS = uids;
		conn = NhpConn;
		SetEvent(connNHPEvent);
	}
	// end VS_StreamClient_Implementation::
	inline bool ConnectNHP(const char* Cl_Endpoint,const char *Serv_Endpoint,const char *ConferenceName,
							void * connectEvent, const unsigned long timeout, const char *source_ip = NULL )
	{
		if(!ConferenceName ||
			(VS_STREAMS_MAX_SIZE_CONFERENCE_NAME<strlen(ConferenceName))	||
			(VS_ACS_MAX_SIZE_ENDPOINT_NAME<strlen(Cl_Endpoint))			||
			(VS_ACS_MAX_SIZE_ENDPOINT_NAME<strlen(Serv_Endpoint))		||
			(!connectEvent))
				return false;

		char   epNm[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
		::VS_EndpointName(epNm,sizeof(epNm));
		EnterMutexes();
		VS_StreamClient_Implementation::connectType = vs_stream_client_connect_not_guaranteed;
		memset( ftracks, 1, sizeof(ftracks) );		*ftracks = 0;
		LeaveMutexes();
		boost::shared_ptr<VS_ConnectNHPExecutor> nhp(new VS_ConnectNHPExecutor(epNm,Cl_Endpoint,Serv_Endpoint,ConferenceName,timeout,source_ip));
		m_nhp = nhp;

		boost::signals2::connection conn = nhp->ConnectToSetNHPConn(boost::bind(&VS_StreamClient_Implementation::SetNHPConn,this,_1,_2,_3));
		if(!conn.connected())
			return false;
		nhp->MakeNHP();
		connNHPEvent = connectEvent;
		return true;
	}

	inline bool ConnectNHPServer(const char *host, const unsigned short port ,
			const long uid, VS_ConnectionSock	*&out_conn, unsigned long& timeout, bool isSimpleUdpStream = false,
						 unsigned long * ip_out = 0, unsigned short * port_out =0, const char *source_ip = NULL)
	{
		if (!host || !*host || !port)	return false;

		if (type == stream::ClientType::sender ||
			type == stream::ClientType::transmitter ||
			type == stream::ClientType::rtp)
		{
			UIDS = uid;
		}

		if (type == stream::ClientType::receiver)
		{
			UIDR = uid;
		}

		bool ret = false;
		EnterMutexes();
		if (!isSimpleUdpStream)
			ret = false;
		else
			ret = ConnectNHPNotGuaranteed( host, port, out_conn, ip_out, port_out, source_ip);
		LeaveMutexes();
		return ret;
	}

	bool is_multicast(const char *host)
	{
		VS_IPPortAddress addr(host);

		if (addr.isZero()) return false;

		if (addr.getAddressType() == VS_IPPortAddress::ADDR_IPV4) {
			return atoi(host) >= 224 && atoi(host) <= 239;
		} else if (addr.getAddressType() == VS_IPPortAddress::ADDR_IPV6){
			return tolower(host[0]) == 'f' && tolower(host[1]) == 'f';
		}

		return false;
	}

	inline bool Connect( const char *host, const unsigned short port ,
						 const long uid, bool isSimpleUdpStream = false,
						 unsigned long * ip_out = 0, unsigned short * port_out =0, const char *source_ip = NULL )
	{
		if (!host || !*host || !port)	return false;

		if (type == stream::ClientType::sender ||
			type == stream::ClientType::transmitter ||
			type == stream::ClientType::rtp)
		{
			UIDS = uid;
		}

		if (type == stream::ClientType::receiver)
		{
			UIDR = uid;
		}

		bool ret = false;
		EnterMutexes();
		if (!isSimpleUdpStream)
		{
			if (!is_multicast(host))
				ret = ConnectNotGuaranteedBroadcast( host, port );
			else
				ret = ConnectNotGuaranteedMulticast( host , port , source_ip );
		}
		else
		{
			ret = ConnectNotGuaranteed( host, port, ip_out, port_out );
		}
		LeaveMutexes();
		return ret;
	}
	// end VS_StreamClient_Implementation::Connect

	inline bool ConnectUseManager( const VS_StreamClient_ConnectType connectType,
					const char *conferenceName, const char *participantName,
					const char *connectedParticipantName, const char *connectedEndpointName,
					const stream::Track* tracks, const unsigned n_tracks,
					const unsigned long connectTimeout,
					void *connectEvent, const bool waitForConnection )
	{
#ifdef _MY_DEBUG_
printf( "VS_StreamClient_Implementation::ConnectUseManager( connectType: %s, conferenceName: %s, participantName: %s, connectedParticipantName: %s, connectedEndpointName: %s, tracks: %08X, n_tracks: %u, connectTimeout: %u, waitForConnection: %s, connectEvent: %08X )\n", _MD_CONNECT_TYPE_(connectType), _MD_STR_(conferenceName), _MD_STR_(participantName), _MD_STR_(connectedParticipantName), _MD_STR_(connectedEndpointName), _MD_POINT_(tracks), n_tracks, connectTimeout, _MD_BOOL_(waitForConnection), _MD_POINT_(connectEvent) );
#endif
		bool ret;
		EnterMutexes();
		if (VS_StreamClient_Implementation::connectType != vs_stream_client_connect_type_unconnected )
		{
			LeaveMutexes();
			return false;
		}
		CloseConnectionAct();
		ResetEvent( connEvent );
		if (conferenceName)
		{
			if ( strlen( conferenceName ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME )
				goto go_return_false;
			strcpy( VS_StreamClient_Implementation::conferenceName, conferenceName );
		}
		if (participantName)
		{
			if ( strlen( participantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME )
				goto go_return_false;
			strcpy( VS_StreamClient_Implementation::participantName, participantName );
		}
		if (connectedParticipantName)
		{
			if ( strlen( connectedParticipantName ) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME )
				goto go_return_false;
			strcpy( VS_StreamClient_Implementation::connectedParticipantName, connectedParticipantName );
		}
		if (!connectedEndpointName || !*connectedEndpointName
				|| strlen( connectedEndpointName ) > VS_ACS_MAX_SIZE_ENDPOINT_NAME)
			goto go_return_false;
		strcpy( VS_StreamClient_Implementation::connectedEndpointName, connectedEndpointName );
		if (!stream::TracksToFTracks(ftracks, tracks, n_tracks))
			goto go_return_false;
		if (connectTimeout < VS_MIN_CONNECT_TIMEOUT || connectTimeout > VS_MAX_CONNECT_TIMEOUT)
			goto go_return_false;
		VS_StreamClient_Implementation::connectTimeout = connectTimeout;
		switch (VS_StreamClient_Implementation::connectType = connectType)
		{
		case vs_stream_client_connect_type_accept :
			acceptTimeout = connectTimeout;
			if (VS_AddAcceptHandler(stream::PrimaryField, AcceptHandler, 0, isNagleDisable, m_qos, m_qos_params) <= 0)
				goto go_return_false;
			break;
		case vs_stream_client_connect_type_connect :
			if (!StartConnectThread())
				goto go_return_false;
			break;
		default :
			goto go_return_false;
		}
		if (connectEvent && connectEvent != INVALID_HANDLE_VALUE)
			argConnEvent = connectEvent;
		LeaveMutexes();
		if (!waitForConnection)
			return true;
		ret = !WaitConnEvent( (unsigned long &)connectTimeout );
		EnterMutexes();
		if (ret)
		{
			ret = VS_StreamClient_Implementation::connectType == connectType;
			LeaveMutexes();
			return ret;
		}
go_return_false:
		CloseConnectionAct();
		LeaveMutexes();
		return false;
	}
	// end VS_StreamClient_Implementation::ConnectUseManager

protected:

	static unsigned __stdcall ThreadConnectDirectUDP(void *arg)
	{
		vs::SetThreadName("SC_ConnectUDP");
		VS_StreamClient_Implementation* pThis = (VS_StreamClient_Implementation*)arg;
		pThis->ThreadConnectDirectUDP();
		return 0;
	}

	void ThreadConnectDirectUDP()
	{
		if(!udpConn)
			return;

		unsigned long curr_tick(GetTickCount());
		unsigned long elapsed_tick(0);

		srand( (unsigned)time( NULL ) );
		const long our_ctrl_num(rand());
		int32_t peer_ctrl_num(0);

		VS_Container cnt;
		cnt.AddValue("ConferenceName",conferenceName);
		cnt.AddValue("participantName",participantName);
		cnt.AddValue("connectedParticipantName",connectedParticipantName);
		cnt.AddValueI32("CtrlNum", our_ctrl_num);

		unsigned long trytimeout = (connectDirectUdpTimeout/*-100*/)/10;
		unsigned long mills(trytimeout);

		void *buf(0);
		VS_Buffer	sndbuff,rSndBuff;

		rSndBuff.buffer = 0;
		rSndBuff.length = 0;
		sndbuff.buffer = 0;
		sndbuff.length = 0;

		char rcv_buf[0xffff]={0};
		const unsigned long rcv_buf_sz = 0xffff;
		unsigned long sz(0), send_sz(0),r_send_sz(0);
		size_t sz_tmp;
		cnt.SerializeAlloc(buf, sz_tmp);
		sz = sz_tmp;
		const char	udp_conn_header[] = "UDP_DIRECT_CONNECT";
		const unsigned long header_sz = sizeof(udp_conn_header);
		const char udp_conn_ok[] = "UDP_CONN_OK";


		send_sz = sz + sizeof(unsigned long) + strlen(udp_conn_header) + 1;
		sndbuff.buffer = udp_hs_header_buff;
		sndbuff.length = send_sz;

		memcpy(sndbuff.buffer,udp_conn_header,strlen(udp_conn_header)+1);
		memcpy((char*)sndbuff.buffer + strlen(udp_conn_header)+1,&sz,sizeof(sz));
		memcpy((char*)sndbuff.buffer+sizeof(sz)+strlen(udp_conn_header)+1,buf,sz);
		unsigned state(0);

		free(buf);
		buf =0;
		sz = 0;
		m_udpConnLogger->TPrintf("Start handshake...");
		m_udpConnLogger->TPrintf("Our cntrl_num = %d",our_ctrl_num);

		HANDLE	h[] = {udpConn->OvWriteEvent(),udpConn->OvReadEvent()};

		unsigned long ip(0);
		unsigned short port(0);
		bool bStop(false);

		/**
		надо посчитать время, которое прошшло
		*/
		unsigned long last_write(0);
		while(!bStop)
		{
			mills = trytimeout;
			int res(0);

			if(!udpConn->IsWrite())
			{
				/**
				отправить 2 пакета
				*/
				if(GetTickCount() - last_write>=trytimeout)
				{
					if(state&0x1)
					{
						if(directUDP.getAddressType() == VS_IPPortAddress::ADDR_IPV4)
							res = udpConn->RWriteTo(&rSndBuff,1,directUDP.ipv4(),directUDP.port());
						else
							res = udpConn->RWriteToV6(&rSndBuff,1,directUDP.ipv6(),directUDP.port());
						sendBytes = rSndBuff.length;
						m_udpConnLogger->TPrintf("Send %s",udp_conn_ok);
					}
					if(!(state&0x2))
					{
						unsigned long tmp_mills(100);
						if(!udpConn->IsWrite()||(udpConn->GetWriteResult(tmp_mills)>0))
						{
							if(directUDP.getAddressType() == VS_IPPortAddress::ADDR_IPV4)
								udpConn->RWriteTo(&sndbuff,1,directUDP.ipv4(),directUDP.port());
							else
								udpConn->RWriteToV6(&sndbuff,1,directUDP.ipv6(),directUDP.port());
							sendBytes = sndbuff.length;
							m_udpConnLogger->TPrintf("Send %s",udp_conn_header);
						}
					}
					last_write = GetTickCount();
				}
			}
			if(!udpConn->IsRead())
			{
				if(!udpConn->IsIPv6())
				{
					lastAddressFamily = VS_IPPortAddress::ADDR_IPV4;
					udpConn->AsynchReceiveFrom(rcv_buf,rcv_buf_sz,lastAddrFrom,pLastIPV4From,pLastPortFrom);
				}
				else
				{
					lastAddressFamily = VS_IPPortAddress::ADDR_IPV6;
					udpConn->AsynchReceiveFrom(rcv_buf,rcv_buf_sz,lastAddrFrom,pLastIPV6From,pLastPortFrom);
				}
			}

			unsigned long tmp_mills(100);
			bool isTimeout(false);
			do
			{
				switch(WaitForMultipleObjects(2,h,FALSE,mills))
				{
				case WAIT_OBJECT_0:
					udpConn->GetWriteResult(tmp_mills);
					break;
				case WAIT_OBJECT_0+1:
					res = udpConn->GetReadResult(mills, &buf,true);

					if(res>0)
					{
						m_udpConnLogger->TPrintf("Data received..");

						VS_IPPortAddress lastAddress;
						lastAddress.port_netorder(pLastPortFrom ? *pLastPortFrom : 0);
						if(lastAddressFamily == VS_IPPortAddress::ADDR_IPV4)
						{
							if(pLastIPV4From) lastAddress.ipv4_netorder(*pLastIPV4From);
						}
						else
						{
							if(pLastIPV6From) lastAddress.ipv6(*pLastIPV6From);
						}

						if(directUDP == lastAddress)
						{
							/**
							адрес верный, расшифорвать пакет
							*/
							m_udpConnLogger->TPrintf("Data from peer");
							if(/*!(state&0x1)&&*/!strncmp(rcv_buf,udp_conn_header,strlen(udp_conn_header)))
							{
								char *pos = rcv_buf + strlen(udp_conn_header) + 1;
								unsigned long rcv_sz = *((unsigned long *)pos);
								pos+=sizeof(rcv_sz);
								m_udpConnLogger->TPrintf("Packet is %s", udp_conn_header);

								if(rcv_sz>0xffff - (strlen(udp_conn_header) + 1) -sizeof(rcv_sz))
								{
									//большой пакет, отбрасываем
								}
								else
								{
									VS_Container rcv_cnt;
									if(rcv_cnt.Deserialize(pos,rcv_sz))
									{
										const char * rcv_conf_name = rcv_cnt.GetStrValueRef("ConferenceName");
										const char *rcv_part_name = rcv_cnt.GetStrValueRef("participantName");
										const char *rcv_conn_part_name = rcv_cnt.GetStrValueRef("connectedParticipantName");
										if(rcv_conf_name && rcv_part_name && rcv_conn_part_name &&
											!_stricmp(rcv_conf_name,conferenceName) && !_stricmp(rcv_part_name,connectedParticipantName)&&
											!_stricmp(rcv_conn_part_name,participantName))
										{
											//
											//отослать OK и ждать OK в ответ
											//
											if(rSndBuff.buffer)
											{
												rSndBuff.buffer =0;
												rSndBuff.length = 0;
											}


											rcv_cnt.GetValue("CtrlNum",peer_ctrl_num);//Для того, чтобы не обрабатывать подтверждение о полученом пакете от предыдущих ХШ
											m_udpConnLogger->TPrintf("Packet for us, peer_ctrl_num = %d. Send %s",peer_ctrl_num,udp_conn_ok);

											state |= 0x1;
											VS_Container rCnt;
											rCnt.AddValue("CtrlNum",peer_ctrl_num);
											size_t sz_tmp;
											rCnt.SerializeAlloc(buf, sz_tmp);
											sz = sz_tmp;

											r_send_sz = sz + sizeof(unsigned long) + strlen(udp_conn_ok) +1;
											rSndBuff.buffer = udp_hs_ok_buff;
											rSndBuff.length = r_send_sz;
											memcpy(rSndBuff.buffer,udp_conn_ok,strlen(udp_conn_header)+1);
											memcpy((char*)rSndBuff.buffer + strlen(udp_conn_ok)+1,&sz,sizeof(sz));
											memcpy((char*)rSndBuff.buffer+sizeof(sz)+strlen(udp_conn_ok)+1,buf,sz);

											free(buf);
											buf = 0;
											sz = 0;
											m_udpConnLogger->TPrintf("Send %s",udp_conn_ok);
											tmp_mills = 100;
											if(!udpConn->IsWrite()||(udpConn->GetWriteResult(tmp_mills)>0))
											{
												if(directUDP.getAddressType() == VS_IPPortAddress::ADDR_IPV4)
													udpConn->RWriteTo(&rSndBuff,1,directUDP.ipv4(),directUDP.port());
												else
													udpConn->RWriteToV6(&rSndBuff,1,directUDP.ipv6(),directUDP.port());
												sendBytes = rSndBuff.length;
											}
											if(state & 0x2)
												bStop = true;
										}
										else
										{
										}
									}
									else
									{
									}
								}
							}
							else if(!strncmp(rcv_buf,udp_conn_ok,strlen(udp_conn_ok)))
							{
								char *pos = rcv_buf + strlen(udp_conn_ok) + 1;
								unsigned long rcv_sz = *((unsigned long *)pos);
								pos+=sizeof(rcv_sz);
								m_udpConnLogger->TPrintf("Packet is %s", udp_conn_ok);
								if(rcv_sz>0xffff - (strlen(udp_conn_ok) + 1) -sizeof(rcv_sz))
								{
								}
								else
								{
									VS_Container rcv_cnt;
									if(rcv_cnt.Deserialize(pos,rcv_sz))
									{
										int32_t ctrl_num(0);
										if(rcv_cnt.GetValue("CtrlNum",ctrl_num) && our_ctrl_num ==ctrl_num)
										{
											m_udpConnLogger->TPrintf("%s for us", udp_conn_ok,ctrl_num);
											state |=0x2;
											if(state & 0x1)
												bStop = true;
										}
										m_udpConnLogger->TPrintf("ctrl_num = %d",ctrl_num);
									}
								}
							}
							else
							{
								m_udpConnLogger->TPrintf("Packet dropped. res = %d",res);
							}
						}
					}
					break;
				case WAIT_TIMEOUT:
					isTimeout = true;
					break;
				default:
					break;
				}
			}while(udpConn->IsRead()&&!isTimeout);
			elapsed_tick += GetTickCount() - curr_tick;
			curr_tick = GetTickCount();
			if(elapsed_tick>=connectDirectUdpTimeout)
				bStop = true;

		}

		if( (state&0x1) && (state&0x2))
		{
			bStop = true;
			for(int j =0;j<5;j++)
			{
				unsigned long tmp_mills(500);
				if(!udpConn->IsWrite()||(udpConn->GetWriteResult(tmp_mills)>0))
				{
					if(directUDP.getAddressType() == VS_IPPortAddress::ADDR_IPV4)
						udpConn->RWriteTo(&rSndBuff,1,directUDP.ipv4(),directUDP.port());
					else
						udpConn->RWriteToV6(&rSndBuff,1,directUDP.ipv6(),directUDP.port());
					sendBytes = rSndBuff.length;
				}
			}
			directUdpConnEstablished = true;
			type = stream::ClientType::direct_udp;
			conn = udpConn;
			if(connectDirectUDPEvent)
				SetEvent(connectDirectUDPEvent);
		}
		if( (state&0x1) && (state&0x2))
		{
			m_udpConnLogger->TPrintf("Udp  handshake OK");
		}
		else
			m_udpConnLogger->TPrintf("Udp  handshake failed");
	}

public:

	inline bool Accept( const char *conferenceName, const char *participantName,
						const char *connectedParticipantName, const char *connectedEndpointName,
						const stream::Track* tracks, const unsigned n_tracks,
						const unsigned long connectTimeout,
						void *connectEvent, const bool waitForConnection )
	{
		return ConnectUseManager( vs_stream_client_connect_type_accept, conferenceName, participantName, connectedParticipantName, connectedEndpointName, tracks, n_tracks, connectTimeout, connectEvent, waitForConnection );
	}
	// end VS_StreamClient_Implementation::Accept

	inline bool Connect( const char *conferenceName, const char *participantName,
						const char *connectedParticipantName, const char *connectedEndpointName,
						const stream::Track* tracks, const unsigned n_tracks,
						const unsigned long connectTimeout,
						void *connectEvent, const bool waitForConnection )
	{
		return ConnectUseManager( vs_stream_client_connect_type_connect, conferenceName, participantName, connectedParticipantName, connectedEndpointName, tracks, n_tracks, connectTimeout, connectEvent, waitForConnection );
	}
	// end VS_StreamClient_Implementation::Connect

	inline bool SetConnectionAct( VS_ConnectionSock *conn, unsigned long &milliseconds )
	{
		isConnectThreadStarted = false;
		if (!conn || !conn->CreateOvReadEvent() || !conn->CreateOvWriteEvent())
			return false;
		switch (type)
		{
		case stream::ClientType::sender:
			if (!conn->SetAsWriter( milliseconds ))		return false;
			break;
		case stream::ClientType::receiver:
			if (!conn->SetAsReader( milliseconds ))		return false;
			break;
		case stream::ClientType::transmitter:
		case stream::ClientType::rtp:
			if (!conn->SetSizeBuffers(-1,-1))			return false;
			break;
		default :	return false;
		}
		if (VS_StreamClient_Implementation::conn)	delete VS_StreamClient_Implementation::conn;
		VS_StreamClient_Implementation::conn = conn;
		SetEvent( connEvent );
		if (argConnEvent != INVALID_HANDLE_VALUE)
		{	SetEvent( argConnEvent );	argConnEvent = INVALID_HANDLE_VALUE;	}
		return true;
	}
	// end VS_StreamClient_Implementation::SetConnectionAct

	inline bool SetConnection( VS_ConnectionSock *conn, const unsigned long milliseconds = 0,
			const VS_StreamClient_ConnectType connectType = vs_stream_client_connect_type_set )
	{
		bool   ret = false;
		if (connectType != vs_stream_client_connect_type_accept
			&& connectType != vs_stream_client_connect_type_connect
			&& connectType != vs_stream_client_connect_type_host_accept
			&& connectType != vs_stream_client_connect_type_host_connect
			&& connectType != vs_stream_client_connect_type_set )			return ret;
		EnterMutexes();
		if (VS_StreamClient_Implementation::connectType == vs_stream_client_connect_type_unconnected)
		{
			VS_StreamClient_Implementation::connectType = connectType;
			ret = SetConnectionAct( conn, (unsigned long &)milliseconds );
			if (ret)	memset( &ftracks[1], 1, sizeof(ftracks) - sizeof(*ftracks) );
		}
		LeaveMutexes();
		return ret;
	}
	// end VS_StreamClient_Implementation::SetConnection

	inline void CloseConnectionAct( void )
	{
#ifdef _MY_DEBUG_
printf( "CloseConnectionAct( terminateConnect: %s )", _MD_BOOL_(terminateConnect) );
#endif
		EnterMutexes();
		isConnectThreadStarted = false;
		VS_ConnectionSock::Break( handle );		handle = 0;
		connectType = vs_stream_client_connect_type_unconnected;
		if (conn)
			DeleteConn();//delete conn;
		if (host) {		free( (void *)host );		host = 0;	}	port = 0;
		sendBytes = 0;	recvBytes = sizeof(stream::FrameHeader);	stateRecv = 0;
		oldSysTC = diffClnTC = currClnTC = 0;
		sendConnectTimeout = recvConnectTimeout = 0;
		memset( (void *)conferenceName, 0, sizeof(conferenceName) );
		memset( (void *)participantName, 0, sizeof(participantName) );
		memset( (void *)connectedParticipantName, 0, sizeof(connectedParticipantName) );
		memset( (void *)connectedEndpointName, 0, sizeof(connectedEndpointName) );
		memset( (void *)ftracks, 0, sizeof(ftracks) );
		SetEvent( connEvent );
		if (argConnEvent != INVALID_HANDLE_VALUE)
		{	SetEvent( argConnEvent );	argConnEvent = INVALID_HANDLE_VALUE;	}
		LeaveMutexes();

	}
	// end VS_StreamClient_Implementation::CloseConnectionAct

	inline void CloseConnection( void )
	{
		CloseUDPConnection();
		CloseConnectionAct();
	}
	// end VS_StreamClient_Implementation::CloseConnection

	inline void CloseUDPConnection()
	{
		DWORD res(0);
		if (type == stream::ClientType::direct_udp)
		{
			if(hThreadHandshakeDirectUDP)
			{
				WaitForSingleObject(hThreadHandshakeDirectUDP,INFINITE);
				CloseHandle(hThreadHandshakeDirectUDP);
				hThreadHandshakeDirectUDP = 0;
			}
			unsigned long mills(500);
			if(udpConn && udpConn->IsWrite())
				udpConn->GetWriteResult(mills);
			/*if(udpConn && udpConn->IsRead())
				udpConn->Break*/

			directUDP = VS_IPPortAddress();
			directUdpConnEstablished = false;
			connectType = vs_stream_client_connect_type_unconnected;
			conn = 0;
			m_udpConnLogger->TPrintf("Close Udp Conference");
			/**
				что сделать с udpConn?
			*/
		}
		/***/
	}

	inline bool SetConnectTimeout( const unsigned long connectTimeout )
	{
		if (connectTimeout < VS_MIN_CONNECT_TIMEOUT || connectTimeout > VS_MAX_CONNECT_TIMEOUT)
			return false;
		VS_StreamClient_Implementation::connectTimeout = connectTimeout;
		return true;
	}
	// end VS_StreamClient_Implementation::SetConnectTimeout

	inline unsigned long ConnectTimeout( void )
	{
		return connectTimeout;
	}
	// end VS_StreamClient_Implementation::ConnectTimeout

	void *GetReceiveEvent( void )
	{
		void *res(0);
		unsigned long mills = INFINITE;
		WaitEnterRecvMutex(mills);
		if (type == stream::ClientType::direct_udp)
			res = !udpConn ? 0 : udpConn->OvReadEvent();
		else
			res = !conn ? connEvent : conn->OvReadEvent();
		LeaveRecvMutex();
		return res;
	}
	// end VS_StreamClient_Implementation::GetReceiveEvent

	void *GetSendEvent( void )
	{
		void *res(0);
		unsigned long mills = INFINITE;
		WaitEnterSendMutex(mills);
		if (type == stream::ClientType::direct_udp)
			res = !udpConn ? 0 : udpConn->OvWriteEvent();
		else
			res = !conn ? connEvent : conn->OvWriteEvent();
		LeaveSendMutex();
		return res;
	}
	// end VS_StreamClient_Implementation::GetSendEvent

	inline int SendUdpFrameAct( const void *buffer, const int n_bytes,
							stream::Track track, unsigned long &milliseconds)
	{
		if (type == stream::ClientType::rtp || type == stream::ClientType::direct_udp)
			return SendRtpFrameAct(buffer, n_bytes, milliseconds);
		uSendFrame.head_length = sizeof(uSendFrame);
		uSendFrame.version = 1;
		uSendFrame.UID = UIDS;
		uSendFrame.length = (unsigned short)n_bytes;
		uSendFrame.tick_count = NextTickCount(); // GetTickCount();
		uSendFrame.track = track;
		uSendFrame.cksum = stream::GetFrameBodyChecksum(buffer, n_bytes);
		VS_Buffer buffers[] = {{ uSendFrame.head_length, (void *)&uSendFrame },
									{ (unsigned long)n_bytes, (void *)buffer }};

		if (!conn->RWrite( buffers, 2 ))
			return -1;

		sendBytes = uSendFrame.head_length + n_bytes;
		return n_bytes;

	}
	// end VS_StreamClient_Implementation::SendUdpFrameAct
	inline int SendRtpFrameAct(const void *buffer, const int n_bytes, unsigned long &milliseconds)
	{
		VS_Buffer	sndbuff;
		sndbuff.length = n_bytes;
		sndbuff.buffer = (void*)buffer;
		if (stream::ClientType::direct_udp == type)
		{
			if(directUDP.getAddressType() == VS_IPPortAddress::ADDR_IPV4)
			{
				if(!udpConn->RWriteTo(&sndbuff,1,directUDP.ipv4(),directUDP.port()))
					return -1;
			}
			else
			{
				if(!udpConn->RWriteToV6(&sndbuff,1,directUDP.ipv6(),directUDP.port()))
					return -1;
			}
		}
		else if(!conn->RWrite(&sndbuff,1))
			return -1;
		//else
		//{
		sendBytes = n_bytes;
		return n_bytes;
		//}
	}

	inline int SendFrameAct( const void *buffer, const int n_bytes,
							stream::Track track, unsigned long &milliseconds)
	{
		if (!ftracks[id(track)] && stream::ClientType::rtp != type && stream::ClientType::direct_udp != type)
			return -3;

		if (conn->IsWrite())
		{
			int   ret = conn->GetWriteResult( milliseconds );
			if (ret < 0)	return ret;
			if (sendBytes != ret)	return -1;
		}
		///Fix ambiguous
		if(conn->Type() == vs_connection_type_dgram )
			return SendUdpFrameAct( buffer , n_bytes , track , milliseconds );
		///Fix ambiguous end
		sendFrame.length = (unsigned short)n_bytes;
		sendFrame.tick_count = NextTickCount(); // GetTickCount();
		sendFrame.track = track;
		sendFrame.cksum = stream::GetFrameBodyChecksum(buffer, n_bytes);
		VS_Buffer buffers[] = {{ sizeof(stream::FrameHeader), (void *)&sendFrame },
									{ (unsigned long)n_bytes, (void *)buffer }};
		if (!conn->RWrite( buffers, 2 ))	return -1;
		sendBytes = sizeof(stream::FrameHeader) + n_bytes;
		return n_bytes;
	}
	// end VS_StreamClient_Implementation::SendFrameAct

	inline int SendFrame( const void *buffer, const int n_bytes,
							stream::Track track, unsigned long *milliseconds)
	{

		if ((unsigned long)n_bytes > VS_STREAM_MAX_SIZE_FRAME || (n_bytes && !buffer))	return -1;
		unsigned long   zero_mills = 0;		if (!milliseconds)	milliseconds = &zero_mills;
		const unsigned long   disc_mills = *milliseconds;
		int   ret;
		bool   flag;
		do
		{
			flag = false;
			ret = WaitConnEvent( *milliseconds );
			if (ret)
			{
				break;
			}
			ret = WaitEnterSendMutex( *milliseconds );
			if (ret)
			{
				return ret;
			}
			if (!conn)
			{
				switch (connectType)
				{
				case vs_stream_client_connect_type_accept :
				case vs_stream_client_connect_type_connect :
				case vs_stream_client_connect_type_host_accept :
				case vs_stream_client_connect_type_host_connect :	flag = true;	break;
				default :	ret = -1;
				}
				LeaveSendMutex();
			}
			else
			{
				ret = SendFrameAct( buffer, n_bytes, track, *milliseconds );
				LeaveSendMutex();
				if (ret == -2 && sendConnectTimeout >= 10000)
				{
					EnterMutexes();
					if (conn)
					{
						DeleteConn();//delete conn;
						ResetEvent( connEvent );
					}
					StartConnectThread();
					LeaveMutexes();
				}
				else if (ret == -1)
				{
					EnterMutexes();
					DeleteConn();
					ResetEvent( connEvent );
					switch (connectType)
					{
					case vs_stream_client_connect_type_accept :
						flag = true;
						LeaveMutexes();
						break;
					case vs_stream_client_connect_type_connect :
						if (StartConnectThread())
							flag = true;
						LeaveMutexes();
						break;
//					case vs_stream_client_connect_type_host_accept :
//					case vs_stream_client_connect_type_host_connect :
//						if (StartConnectHostThread( connectTimeout ))	flag = true;
//						break;
					default :
						LeaveMutexes();
						CloseConnectionAct();
					}
				}
			}
			//LeaveSendMutex();
		}
		while (flag);
		if (ret == -2)
		{
			sendConnectTimeout += disc_mills;
			if (sendConnectTimeout >= connectTimeout)
			{
				CloseConnection();
				sendConnectTimeout = connectTimeout;
				ret = -1;
		}	}
		else 	sendConnectTimeout = 0;
		return ret;
	}
	// end VS_StreamClient_Implementation::SendFrame
	char rbuff[257];
	inline int TestLength(stream::UDPFrameHeader* ph)
	{

	}
	inline int ReceiveUdpFrameAct(void *buffer, const int s_buffer, stream::Track* track,
														unsigned long &milliseconds )
	{
		if (stream::ClientType::rtp == type || stream::ClientType::direct_udp == type)
			return ReceiveRtpFrameAct(buffer, s_buffer, milliseconds);
		char LostSynAck[SYN_ACK_FRAME_SIZE];
		auto ptrH = reinterpret_cast<stream::UDPFrameHeader*>(rbuff);


		if (!conn->IsRead())
		{
			stateRecv = 0;
			memset( rbuff , 0 , 256 );
			if (!conn->Read((void *)rbuff, sizeof(stream::UDPFrameHeader)))
			{
				dbgprintf(" |0| ");
				return -1;
			}
		}
		int   ret = 0;
		while (1)
		{
			void   *buff = NULL;
			ret = conn->GetReadResult( milliseconds, &buff );
			if (ret < 0)
			{
				if (ret==-1) ret =-2;
				dbgprintf(" |1/%d/%d| ",stateRecv,ret);
				return ret;
			}
			switch (stateRecv)
			{
			case 0 :
				if (ret != sizeof(stream::UDPFrameHeader))
					return 0;
				if (!ptrH->head_length)
					return 0;

				if (ptrH->head_length < sizeof(stream::UDPFrameHeader))
				{
					memcpy(LostSynAck, rbuff, sizeof(stream::UDPFrameHeader));
					stateRecv = 3;
					if (!conn->RRead(SYN_ACK_FRAME_SIZE - sizeof(stream::UDPFrameHeader)))
						return -1;
					break;
				}

				if (!ptrH->length)
				{
					if (ptrH->cksum != stream::GetFrameBodyChecksum(nullptr, 0))
						return 0;
					ret = 0;
					goto frame_ok;
				}
				if (ptrH->length > VS_STREAM_MAX_SIZE_FRAME)
					return 0;

				if (ptrH->head_length == sizeof(stream::UDPFrameHeader))
				{
					stateRecv = 2;
					if (!conn->RRead( ptrH->length ))
					{
						dbgprintf(" |2/%d| ",stateRecv);
						return -1;
					}

				}
				else
				{
					stateRecv = 1;
					if (!conn->RRead(ptrH->head_length - sizeof(stream::UDPFrameHeader)))
						return -1;
				}
				break;
			case 1 :
				if (ret != ptrH->head_length - sizeof(stream::UDPFrameHeader))
					return 0;
				stateRecv = 2;
				if (!conn->RRead( ptrH->length ))
				{
					dbgprintf(" |2/%d| ",stateRecv);
					return -1;
				}

			case 2 :
				if ((unsigned long)ret != ptrH->length)
				{
					conn->Free( buff );
					dbgprintf(" |3/%d| ",stateRecv);
					return -1;
				}
				if (ptrH->cksum != stream::GetFrameBodyChecksum(buff, ret))
				{
					conn->Free( buff );
					dbgprintf(" |4/%d| ",stateRecv);
					return -1;
				}
				if (ptrH->UID != UIDR)
				{
					conn->Free( buff );
					goto frame_ok;
				}
				if (buffer)
					memcpy( buffer, buff, (size_t)( ret < s_buffer ? ret : s_buffer ));
				conn->Free( buff );

frame_ok:		stateRecv = 0;
				if (track)
					*track = ptrH->track;
				if (!conn->Read((void *)rbuff, sizeof(stream::UDPFrameHeader)))
				{
					dbgprintf(" |5/%d| ",stateRecv);
					return -1;
				}
				return ret;
			case 3:
				memcpy(LostSynAck + sizeof(stream::UDPFrameHeader), buff, ret);
				conn->Free(buff);
				if (VS_NHP_Handshake_Impl::IsSynAckData(LostSynAck, ret + sizeof(stream::UDPFrameHeader)))
				{
					stateRecv = 0;
					if (!conn->Read((void *)rbuff, sizeof(stream::UDPFrameHeader)))
					{
						dbgprintf(" |7/%d| ",stateRecv);
						return -1;
					}
				}
				else
				{
					NHP_PACK_TYPE type;
					unsigned long size;
					memcpy(&type,LostSynAck,sizeof(type));
					if(ACK == type)
					{
						memcpy(&size,LostSynAck+sizeof(type),sizeof(size));
						stateRecv = 4;
						if(!conn->RRead(size - SYN_ACK_FRAME_SIZE))
						{
							dbgprintf(" |8/%d| ",stateRecv);
							return -1;
						}
					}
					else
						return 0;
				}
				break;
			case 4://Отбросить пакет
				stateRecv = 0;
				if(buff)
					conn->Free(buff);
				if (!conn->Read((void *)rbuff, sizeof(stream::UDPFrameHeader)))
				{
					dbgprintf(" |7/%d| ",stateRecv);
					return -1;
				}
				break;
			default :
				dbgprintf(" |6/%d| ",stateRecv);
				return -1;
			}
		}
	}
	// end VS_StreamClient_Implementation::ReceiveUdpFrameAct

	inline int ReceiveRtpFrameAct( void *buffer, const int s_buffer, unsigned long &milliseconds)
	{
		int ret(0);
		char tmpbuf[65535];
		if(rtpDataLen)
		{
			memcpy(buffer,rbufrtp,rtpDataLen > s_buffer ? s_buffer : rtpDataLen);
			if(s_buffer < rtpDataLen)
			{
				memcpy(tmpbuf,rbufrtp + s_buffer,rtpDataLen - s_buffer);
				memcpy(rbufrtp,tmpbuf,rtpDataLen - s_buffer);
				rtpDataLen -= s_buffer;
				return s_buffer;
			}
			else
			{
				memset(rbufrtp,0,65535);
				ret = rtpDataLen;
				rtpDataLen = 0;
				return ret;
			}
		}
		else
		{
			if(!conn->IsRead())
			{

				if (stream::ClientType::direct_udp == type)
				{
					if(!udpConn->IsIPv6())
					{
						lastAddressFamily = VS_IPPortAddress::ADDR_IPV4;
						if(!udpConn->AsynchReceiveFrom(rbufrtp,sizeof(rbufrtp),lastAddrFrom,pLastIPV4From,pLastPortFrom))
							return -1;
					}
					else
					{
						lastAddressFamily = VS_IPPortAddress::ADDR_IPV6;
						if(!udpConn->AsynchReceiveFrom(rbufrtp,sizeof(rbufrtp),lastAddrFrom,pLastIPV6From,pLastPortFrom))
							return -1;
					}
				}
				else if (!conn->Read(rbufrtp, sizeof(rbufrtp)))
				{
					DTRACE(VSTM_NETWORK, "conn->Read(rbufrtp, sizeof(rbufrtp) return false. Case 1");
					return -1;
				}
			}
			void *buff = 0;
			ret = conn->GetReadResult(milliseconds, &buff,true);
			if (ret < 0)
			{
				if (ret == -1)
				{
					DTRACE(VSTM_NETWORK, "ret = conn->GetReadResult(milliseconds, &buff,true); return - 1");
				}
				return ret;
			}
			if (stream::ClientType::direct_udp == type)
			{
				VS_IPPortAddress lastAddress;
				lastAddress.port_netorder(pLastPortFrom ? *pLastPortFrom : 0);
				if(lastAddressFamily == VS_IPPortAddress::ADDR_IPV4)
				{
					if(pLastIPV4From) lastAddress.ipv4_netorder(*pLastIPV4From);
				}
				else
				{
					if(pLastIPV6From) lastAddress.ipv6(*pLastIPV6From);
				}

				//unsigned long ip_from = pLastIPFrom ? vs_ntohl(*pLastIPFrom) : 0;
				//unsigned short port_from = pLastPortFrom ? vs_ntohs(*pLastPortFrom) : 0;
				if(lastAddress.isZero() || !lastAddress.port() || directUDP != lastAddress)
				{
					m_udpConnLogger->TPrintf("Address from for packet in Streams is alien. DISCARD!");
					if(!udpConn->IsIPv6())
					{
						lastAddressFamily = VS_IPPortAddress::ADDR_IPV4;
						if(!udpConn->AsynchReceiveFrom(rbufrtp,sizeof(rbufrtp),lastAddrFrom,pLastIPV4From,pLastPortFrom))
							return -1;
						else
							return 0;
					}
					else
					{
						lastAddressFamily = VS_IPPortAddress::ADDR_IPV6;
						if(!udpConn->AsynchReceiveFrom(rbufrtp,sizeof(rbufrtp),lastAddrFrom,pLastIPV6From,pLastPortFrom))
							return -1;
						else
							return 0;
					}
				}
				else
				{
					if(ret>0)
					{
						const char	udp_conn_header[] = "UDP_DIRECT_CONNECT";
						const char udp_conn_ok[] = "UDP_CONN_OK";
						int header_sz = sizeof(udp_conn_header) - 1;
						if(ret>=header_sz)
						{
							if(!_strnicmp(rbufrtp,udp_conn_header,header_sz))
							{
								m_udpConnLogger->TPrintf("Pack %s in stream. DISCARD!",udp_conn_header);

								if(!udpConn->IsIPv6())
								{
									lastAddressFamily = VS_IPPortAddress::ADDR_IPV4;
									if(!udpConn->AsynchReceiveFrom(rbufrtp,sizeof(rbufrtp),lastAddrFrom,pLastIPV4From,pLastPortFrom))
										return -1;
									else
										return 0;
								}
								else
								{
									lastAddressFamily = VS_IPPortAddress::ADDR_IPV6;
									if(!udpConn->AsynchReceiveFrom(rbufrtp,sizeof(rbufrtp),lastAddrFrom,pLastIPV6From,pLastPortFrom))
										return -1;
									else
										return 0;
								}
							}
						}
						header_sz = sizeof(udp_conn_ok) - 1;
						if(ret>=header_sz)
						{
							if(!_strnicmp(rbufrtp,udp_conn_ok,header_sz))
							{
								m_udpConnLogger->TPrintf("Pack %s in stream. Discard!",udp_conn_ok);
								if(!udpConn->IsIPv6())
								{
									lastAddressFamily = VS_IPPortAddress::ADDR_IPV4;
									if(!udpConn->AsynchReceiveFrom(rbufrtp,sizeof(rbufrtp),lastAddrFrom,pLastIPV4From,pLastPortFrom))
										return -1;
									else
										return 0;
								}
								else
								{
									lastAddressFamily = VS_IPPortAddress::ADDR_IPV6;
									if(!udpConn->AsynchReceiveFrom(rbufrtp,sizeof(rbufrtp),lastAddrFrom,pLastIPV6From,pLastPortFrom))
										return -1;
									else
										return 0;
								}
							}
						}
					}
				}
			}
			rtpDataLen = ret;
			memcpy(buffer,rbufrtp,rtpDataLen > s_buffer ? s_buffer : rtpDataLen);
			if(s_buffer < rtpDataLen)
			{
				memcpy(tmpbuf,rbufrtp + s_buffer,rtpDataLen - s_buffer);
				memcpy(rbufrtp,tmpbuf,rtpDataLen - s_buffer);
				rtpDataLen -= s_buffer;
				if (stream::ClientType::direct_udp == type)
				{
					if(!udpConn->IsIPv6())
					{
						lastAddressFamily = VS_IPPortAddress::ADDR_IPV4;
						if(!udpConn->AsynchReceiveFrom(rbufrtp,sizeof(rbufrtp),lastAddrFrom,pLastIPV4From,pLastPortFrom))
							return -1;
						else
							return s_buffer;
					}
					else
					{
						lastAddressFamily = VS_IPPortAddress::ADDR_IPV6;
						if(!udpConn->AsynchReceiveFrom(rbufrtp,sizeof(rbufrtp),lastAddrFrom,pLastIPV6From,pLastPortFrom))
							return -1;
						else
							return s_buffer;
					}
				}
				else if (!conn->Read(rbufrtp, sizeof(rbufrtp)))
				{
					DTRACE(VSTM_NETWORK, "conn->Read(rbufrtp, sizeof(rbufrtp) return false. Case 2");
					return -1;
				}
				else
					return s_buffer;
			}
			else
			{
				memset(rbufrtp,0,65535);
				ret = rtpDataLen;
				rtpDataLen = 0;
				if (stream::ClientType::direct_udp == type)
				{
					if(!udpConn->IsIPv6())
					{
						lastAddressFamily = VS_IPPortAddress::ADDR_IPV4;
						if(!udpConn->AsynchReceiveFrom(rbufrtp,sizeof(rbufrtp),lastAddrFrom,pLastIPV4From,pLastPortFrom))
							return -1;
						else
							return ret;
					}
					else
					{
						lastAddressFamily = VS_IPPortAddress::ADDR_IPV6;
						if(!udpConn->AsynchReceiveFrom(rbufrtp,sizeof(rbufrtp),lastAddrFrom,pLastIPV6From,pLastPortFrom))
							return -1;
						else
							return ret;
					}
				}
				else if (!conn->Read(rbufrtp, sizeof(rbufrtp)))
				{
					DTRACE(VSTM_NETWORK, "conn->Read(rbufrtp, sizeof(rbufrtp) return false. Case 3");
					return -1;
				}
				else
					return ret;
			}
		}
	}
	inline int ReceiveFrameAct(void *buffer, const int s_buffer, stream::Track* track,
														unsigned long &milliseconds )
	{
		/*if (type == stream::ClientType::rtp &&
			(conn->Type() == vs_connection_type_dgram))
			return ReceiveRtpFrameAct( buffer, s_buffer, milliseconds );*/
		if( conn->Type() == vs_connection_type_dgram)
		{
			int ret = ReceiveUdpFrameAct( buffer , s_buffer , track , milliseconds );
			return ret;
		}
		/////////TCP//////////////////
		if (!conn->IsRead())
		{
			stateRecv = 0;
			if (!conn->Read((void *)&recvFrame, sizeof(stream::FrameHeader)))
				return -1;
		}
		while (1)
		{
			void   *buff = NULL;
			int   ret = conn->GetReadResult( milliseconds, &buff );
			if (ret < 0)	return ret;
			switch (stateRecv)
			{
			case 0 :
				if (ret != sizeof(stream::FrameHeader))	return -1;
				if (!recvFrame.length)
				{
					if (recvFrame.cksum != stream::GetFrameBodyChecksum(nullptr, 0))
						return -1;
					ret = 0;	goto frame_ok;
				}
				if (recvFrame.length > VS_STREAM_MAX_SIZE_FRAME)		return -1;
				stateRecv = 1;
				if (!conn->RRead( recvFrame.length ))	return -1;
				break;
			case 1 :
				if ((unsigned long)ret != recvFrame.length)
				{	conn->Free( buff );		return -1;	}
				if (recvFrame.cksum != stream::GetFrameBodyChecksum(buff, ret))
				{	conn->Free( buff );		return -1;	}
				if (buffer)		memcpy( buffer, buff, (size_t)( ret < s_buffer ? ret : s_buffer ));
				conn->Free( buff );
				stateRecv = 0;
frame_ok:		if (track)	*track = recvFrame.track;
				if (!conn->Read( (void *)&recvFrame, sizeof(recvFrame) ))	return -1;
				return ret;
			default :	return -1;
	}	}	}
	// end VS_StreamClient_Implementation::ReceiveFrameAct

	inline int ReceiveFrame(void *buffer, const int s_buffer, stream::Track* track,
															unsigned long *milliseconds )
	{
		if (s_buffer < 0 || (s_buffer > 0 && !buffer))		return -1;
		unsigned long   zero_mills = 0;		if (!milliseconds)	milliseconds = &zero_mills;
		const unsigned long   disc_mills = *milliseconds;
		int   ret;
		bool   flag;
		do
		{
			flag = false;
			ret = WaitConnEvent( *milliseconds );
			if (ret)
			{
				break;
			}
			ret = WaitEnterRecvMutex( *milliseconds );
			if (ret)
			{
				return ret;
			}
			if (!conn)
			{
				switch (connectType)
				{
				case vs_stream_client_connect_type_accept :
				case vs_stream_client_connect_type_connect :
				case vs_stream_client_connect_type_host_accept :
				case vs_stream_client_connect_type_host_connect :	flag = true;	break;
				default :	ret = -1;
				}
				LeaveRecvMutex();
			}
			else
			{
				ret = ReceiveFrameAct( buffer, s_buffer, track, *milliseconds );
				LeaveRecvMutex();
				if (ret == -2 && recvConnectTimeout >= 10000 //&&///this comment do UDP unBrakable
					//&& type != stream::ClientType::transmitter ///this comment do UDP unBrakable
					)
				{
					EnterMutexes();
					if (conn)
					{
						DeleteConn();
						ResetEvent( connEvent );
					}
					StartConnectThread();
					LeaveMutexes();
				}
				else if (ret == -1)
				{
					EnterMutexes();
					DeleteConn();
					ResetEvent( connEvent );
					switch (connectType)
					{
					case vs_stream_client_connect_type_accept :
						flag = true;
						LeaveMutexes();
						break;
					case vs_stream_client_connect_type_connect :
						if (StartConnectThread())
							flag = true;
						LeaveMutexes();
						break;
					default :
						LeaveMutexes();
						CloseConnectionAct();
					}
				}
			}
			//LeaveRecvMutex();
		}
		while (flag);
		if (ret == -2)
		{
			recvConnectTimeout += disc_mills;
			if (recvConnectTimeout >= connectTimeout //&&///this comment do UDP unBrakable
				//type != stream::ClientType::transmitter///this comment do UDP unBrakable
				)
			{

				CloseConnection();
				recvConnectTimeout = connectTimeout;
				ret = -1;
			}	else recvConnectTimeout = 0;
		}
		else 	recvConnectTimeout = 0;
		return ret;
	}


	inline void SetFastStream(bool is_nagle_disable = true)
	{
		isNagleDisable = is_nagle_disable;
	}
	// end VS_StreamClient_Implementation::SetFastStream
	inline void SetQOSStream(bool qos = false, _QualityOfService *qos_params = NULL)
	{
		m_qos = qos;
		m_qos_params = qos_params;
	}
	// end VS_StreamClient_Implementation::SetQOSStream
	inline void SetQOSStreamEx(bool qos = false, _QualityOfService *qos_params = 0)
	{
		m_qos = qos;
		m_qos_params = qos_params;
		//udpConn->SetQoS(qos, qos_params);
	}
	// end VS_StreamClient_Implementation::SetQOSStreamEx
};
// end VS_StreamClient_Implementation struct

#endif  // VS_STREAM_CLIENT_TYPES_H
