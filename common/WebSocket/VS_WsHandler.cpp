#ifdef _WIN32	// not ported
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <vector>

#include "VS_WsHandler.h"
#include "VS_WsClient.h"
#include "VS_WsEchoClient.h"
#include "SecureLib/VS_SSLConfigKeys.h"

#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "net/Handshake.h"
#include "SecureLib/VS_SecureConstants.h"
#include "ProtocolCheck.h"

// Change to 'true' to enable internal WebSocket echo server (for debugging purposes)
static const bool ENABLE_ECHO_SERVER = false;

static const size_t BUFSZ = 64 * 1024; // I/O buffer size: 64Kb

VS_WsHandler::VS_WsHandler( void )
{
}

VS_WsHandler::~VS_WsHandler( void )
{
}

bool VS_WsHandler::IsValid( void ) const
{	return VS_AccessConnectionHandler::IsValid();	}

bool VS_WsHandler::Init( const char *handler_name )
{
	m_handlerName = handler_name;
	return true;
}

VS_ACS_Response VS_WsHandler::Connection( unsigned long *in_len )
{
	if (in_len)
		*in_len = sizeof(net::HandshakeHeader);
	return vs_acs_next_step;
}

VS_ACS_Response VS_WsHandler::Protocol(const void *in_buffer, unsigned long *in_len,
											 void** /*out_buffer*/, unsigned long* /*out_len*/,
											 void** /*context*/ )
{
	return ws::ProtocolCheck(in_buffer, *in_len);
}

VS_ACS_Response VS_WsHandler::Protocol( const void *in_buffer, unsigned long *in_len )
{

	return ws::ProtocolCheck(in_buffer, *in_len);
}

void VS_WsHandler::Accept( VS_ConnectionTCP *conn, const void *in_buffer,
								const unsigned long in_len, const void *context )
{
	if (ENABLE_ECHO_SERVER)
	{
		auto set_con = std::make_shared<VS_WsEchoClient>();
		set_con->Init();
		set_con->SetTCPConnection(conn, in_buffer, in_len);
	}
	else
	{
		auto set_con = std::make_shared<VS_WsClient>();
		set_con->Init();
		set_con->SetTCPConnection(conn, in_buffer, in_len);
	}
}

void VS_WsHandler::Destructor( const void *context )
{
}

void VS_WsHandler::Destroy( const char *handler_name )
{
}
#endif