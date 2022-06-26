#if defined(_WIN32) // Not ported yet

//#define   _MY_DEBUG_

#ifdef   _MY_DEBUG_
#include <stdio.h>
#endif
#include <stdlib.h>
#include "VS_StreamsHandler.h"
#include "VS_StreamsRouter.h"
#include "../Handshake.h"
#include "../../acs/Lib/VS_AcsLib.h"
#include "net/QoSSettings.h"
#include "../../std/cpplib/VS_MemoryLeak.h"

VS_StreamsHandler::VS_StreamsHandler( const char *endpoint, VS_StreamsRouter_SetConnection *sr_sc ) :
	endpoint(endpoint), sr_sc(sr_sc) {}
// end VS_StreamsHandler::VS_StreamsHandler

VS_StreamsHandler::~VS_StreamsHandler( void ) {}
// end VS_StreamsHandler::~VS_StreamsHandler

bool VS_StreamsHandler::Init( const char *handler_name )
{
#ifdef   _MY_DEBUG_
printf( "VS_StreamsHandler::Init( handler_name: %s )\n", _MD_STR_(handler_name) );
#endif
	return true;
}
// end VS_StreamsHandler::Init

VS_ACS_Response VS_StreamsHandler::Connection( unsigned long *in_len )
{
#ifdef   _MY_DEBUG_
printf( "VS_StreamsHandler::Connection( *in_len: %u )\n", *in_len );
#endif
	if (in_len)
		*in_len = sizeof(net::HandshakeHeader);
	return vs_acs_next_step;
}
// end VS_StreamsHandler::Connection

VS_ACS_Response VS_StreamsHandler::Protocol( const void *in_buffer, unsigned long *in_len,
												void **out_buffer, unsigned long *out_len,
												void **context )
{
#ifdef   _MY_DEBUG_
printf( "VS_StreamsHandler::Protocol( ..., *in_len: %u, ... )\n", *in_len );
#endif
	if (*in_len < sizeof(net::HandshakeHeader))
	{
		*in_len = sizeof(net::HandshakeHeader);
		return vs_acs_next_step;
	}
	const auto& hs = *static_cast<const net::HandshakeHeader*>(in_buffer);
	if (hs.head_cksum != net::GetHandshakeHeaderChecksum(hs)
			|| hs.version != 1 || hs.body_length < 9
			|| strncmp(hs.primary_field, stream::PrimaryField, sizeof(hs.primary_field)))
		return vs_acs_connection_is_not_my;
	const unsigned long body_length = hs.body_length + 1,
						handshake_length = sizeof(net::HandshakeHeader) + body_length;
	if (*in_len > handshake_length)		return vs_acs_connection_is_not_my;
	if (*in_len < handshake_length)
	{	*in_len = handshake_length;		return vs_acs_my_connections;	}
	if (hs.body_cksum != net::GetHandshakeBodyChecksum(hs))
		return vs_acs_connection_is_not_my;
	return vs_acs_accept_connections;
}
// end VS_StreamsHandler::Protocol

void VS_StreamsHandler::Accept( VS_ConnectionTCP *conn, const void *in_buffer,
										const unsigned long in_len, const void *context )
{
#ifdef   _MY_DEBUG_
printf( "VS_StreamsHandler::Accept( conn: %08X, ..., in_len: %u, ... )\n", _MD_POINT_(conn), in_len );
#endif
	const uint8_t* mtracks;
	const char* conferenceName;
	const char* participantName;
	const char* connectedParticipantName;
	const char* connectedEndpointName;
	stream::ClientType clnType;
	const auto& hs = *static_cast<const net::HandshakeHeader*>(in_buffer);
	if (!stream::GetHandshakeFields(&hs, clnType, conferenceName, participantName,
							connectedParticipantName, connectedEndpointName, mtracks ))
	{	delete conn;	return;		}
	unsigned type = ~0;
	switch (clnType)
	{
		case stream::ClientType::sender:	type = VS_ACS_LIB_SENDER;		break;
		case stream::ClientType::receiver:	type = VS_ACS_LIB_RECEIVER;		break;
		default :	delete conn;	return;
	}
	conn->SetQOSSocket(NULL);

	// enable QoS
	{
		bool ipv6 = conn->GetPeerAddress().getAddressType() == VS_IPPortAddress::ADDR_IPV6;
		conn->SetQoSFlow(net::QoSSettings::GetInstance().GetTCStreamQoSFlow(ipv6));
	}
	sr_sc->SetConnection( conferenceName, participantName, connectedParticipantName,
									type, conn, mtracks );
}
// end VS_StreamsHandler::Accept

void VS_StreamsHandler::Destructor( const void *context )
{
#ifdef   _MY_DEBUG_
printf( "VS_StreamsHandler::Destructor( context: %08X )\n", _MD_POINT_(context) );
#endif
}
// end VS_StreamsHandler::Destructor

void VS_StreamsHandler::Destroy( const char *handler_name )
{
#ifdef   _MY_DEBUG_
printf( "VS_StreamsHandler::Destroy( handler_name: %s )\n", handler_name );
#endif
}
// end VS_StreamsHandler::Destroy

#endif
