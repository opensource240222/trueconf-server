#if defined(_WIN32) // Not ported yet

//#define   _MY_DEBUG_

#ifdef   _MY_DEBUG_
#include <stdio.h>
#endif

#include "VS_TransportHandler.h"
#include "../Lib/VS_TransportLib.h"
#include "net/QoSSettings.h"

VS_TransportHandler::VS_TransportHandler( const char *endpoint, VS_TransportRouter_SetConnection *tr_sc ) :
	endpoint(endpoint), tr_sc(tr_sc) {}
// end VS_TransportHandler::VS_TransportHandler

VS_TransportHandler::~VS_TransportHandler( void ) {}
// end VS_TransportHandler::~VS_TransportHandler

bool VS_TransportHandler::IsValid( void ) const
{	return VS_AccessConnectionHandler::IsValid() && endpoint && tr_sc;	}
// end VS_TransportHandler::IsValid

bool VS_TransportHandler::Init( const char *handler_name )
{
#ifdef   _MY_DEBUG_
printf( "VS_TransportHandler::Init( handler_name: %s )\n", handler_name );
#endif
	return true;
}
// end VS_TransportHandler::Create

VS_ACS_Response VS_TransportHandler::Connection( unsigned long *in_len )
{
#ifdef   _MY_DEBUG_
printf( "VS_TransportHandler::Connection( *in_len: %u )\n", *in_len );
#endif
	if (in_len)
		*in_len = sizeof(net::HandshakeHeader);
	return vs_acs_next_step;
}
// end VS_TransportHandler::Connection

VS_ACS_Response VS_TransportHandler::Protocol( const void *in_buffer, unsigned long *in_len,
												void **out_buffer, unsigned long *out_len,
												void **context )
{
#ifdef   _MY_DEBUG_
printf( "VS_TransportHandler::Protocol( ..., *in_len: %u, ... )\n", *in_len );
#endif
	if (*in_len < sizeof(net::HandshakeHeader))
	{
		*in_len = sizeof(net::HandshakeHeader);
		return vs_acs_next_step;
	}
	const auto& hs = *static_cast<const net::HandshakeHeader*>(in_buffer);
	if (hs.head_cksum != net::GetHandshakeHeaderChecksum(hs)
			|| hs.version < 1 || hs.body_length < (( 2 + 1 + 1 ) - 1 )
			|| strncmp(hs.primary_field, VS_Transport_PrimaryField, sizeof(hs.primary_field)))
		return vs_acs_connection_is_not_my;
	const unsigned long body_length = hs.body_length + 1,
						handshake_length = sizeof(net::HandshakeHeader) + body_length;
	if (*in_len > handshake_length)		return vs_acs_connection_is_not_my;
	if (*in_len < handshake_length)
	{	*in_len = handshake_length;		return vs_acs_my_connections;	}
	if (hs.body_cksum != net::GetHandshakeBodyChecksum(hs))
		return vs_acs_connection_is_not_my;
	const unsigned char* body = &((unsigned char *)&hs)[sizeof(net::HandshakeHeader)];
	const unsigned long		endpoint_length = (unsigned long)body[0];
	if (body_length < (2 + endpoint_length + 1) || (body[1 + endpoint_length] && (body[6] != ':')/*old clients support for disable flood*/))
		return vs_acs_connection_is_not_my;
	return vs_acs_accept_connections;
}
// end VS_TransportHandler::Protocol

void VS_TransportHandler::Accept( VS_ConnectionTCP *conn, const void *in_buffer,
										const unsigned long in_len, const void *context )
{
#ifdef   _MY_DEBUG_
printf( "VS_TransportHandler::Accept( conn: %08X, ..., in_len: %u, ... )\n", _MD_POINT_(conn), in_len );
#endif
	const auto& hs = *static_cast<const net::HandshakeHeader*>(in_buffer);
	char	*cid = 0, *sid = 0; unsigned char hops(0);

	// say error to old architecture and prevent flood from clients
	if ((hs.version & ~VS_SSL_SUPPORT_BITMASK) < VS_NEWARCH_TRANSPORT_VERSION)	{
		tr_sc->SetConnection("", 1, "", conn, true, 0, 0, hops,0,0,0,0, false, false);
		return;
	}
	unsigned long rnd_data_ln(0);
	const unsigned char *rnd_data(0);
	unsigned long sign_ln(0);
	const unsigned char *sign(0);
	bool tcpKeepAliveSupport(false);
	if (!VS_TransformTransportHandshake(const_cast<net::HandshakeHeader*>(&hs), cid, sid, hops, rnd_data_ln, rnd_data, sign_ln, sign, tcpKeepAliveSupport))
	{	delete conn;	return;		}

	// enable QoS
	{
		bool ipv6 = conn->GetPeerAddress().getAddressType() == VS_IPPortAddress::ADDR_IPV6;
		conn->SetQoSFlow(net::QoSSettings::GetInstance().GetTCTransportQoSFlow(ipv6));
	}
	tr_sc->SetConnection(cid, hs.version, sid, conn, true, 0, 0, hops, rnd_data_ln, rnd_data, sign_ln, sign, false, tcpKeepAliveSupport);
}
// end VS_TransportHandler::Accept

void VS_TransportHandler::Destructor( const void *context )
{
#ifdef   _MY_DEBUG_
printf( "VS_TransportHandler::Destructor( context: %08X )\n", _MD_POINT_(context) );
#endif
}
// end VS_TransportHandler::Destructor

void VS_TransportHandler::Destroy( const char *handler_name )
{
#ifdef   _MY_DEBUG_
printf( "VS_TransportHandler::Destroy( handler_name: %s )\n", handler_name );
#endif
}
// end VS_TransportHandler::Destroy

#endif
