#include "WsHandler.h"
#include "WsClient_v2.h"
#include "WsEchoClient_v2.h"
#include "acs_v2/ConvertResponse.h"
#include "WebSocket/ProtocolCheck.h"

// Change to 'true' to enable internal WebSocket echo server (for debugging purposes)
static const bool ENABLE_ECHO_SERVER = false;

ws::Handler::Handler(boost::asio::io_service & ios):m_ios(ios)
{
}

acs::Response ws::Handler::Protocol(const stream_buffer& buffer, unsigned /*channel_token*/) {
	return acs::ConvertResponseCode(ws::ProtocolCheck(buffer.data(), buffer.size()));
}

void ws::Handler::Accept(boost::asio::ip::tcp::socket&& socket, stream_buffer&& buffer) {
	if (ENABLE_ECHO_SERVER)
	{
		auto set_conn = std::make_shared<ws::EchoClient>(m_ios);
		set_conn->SetTCPConnection(std::move(socket), std::move(buffer));
	}
	else
	{
		auto set_conn = std::make_shared<ws::Client>(m_ios);
		set_conn->SetTCPConnection(std::move(socket), std::move(buffer));
	}
}