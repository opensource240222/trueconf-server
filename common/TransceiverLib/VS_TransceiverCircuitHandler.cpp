#ifdef _WIN32 // not ported
#include "VS_TransceiverCircuitHandler.h"
#include "VS_SetConnectionInterface.h"

#include "net/Handshake.h"
#include "Handshake.h"
#include <cassert>


VS_TransceiverCircuitHandler::VS_TransceiverCircuitHandler(const std::shared_ptr<VS_SetConnectionInterface> &proxiesPool) : m_proxiesPool(proxiesPool)
{}
VS_TransceiverCircuitHandler::~VS_TransceiverCircuitHandler()
{}

bool VS_TransceiverCircuitHandler::Init(const char *handler_name)
{
	return true;
}
VS_ACS_Response VS_TransceiverCircuitHandler::Connection(unsigned long *in_len)
{
	if (in_len)
		*in_len = sizeof(net::HandshakeHeader);
	return vs_acs_next_step;
}

VS_ACS_Response VS_TransceiverCircuitHandler::Protocol(const void *in_buffer, unsigned long *in_len, void **out_buffer, unsigned long *out_len, void **context)
{
	return ts::ProtocolHandshake(in_buffer, in_len);
}

void VS_TransceiverCircuitHandler::Accept(VS_ConnectionTCP *conn, const void *in_buffer, const unsigned long in_len, const void *context)
{
	if(auto pool = m_proxiesPool.lock())
		pool->SetTCPConnection(conn, in_buffer, in_len);
}
#endif