#include "WsChannel.h"
#include "WebSocket/VS_WsChannel.h"

bool ws::Channel::SetTCPConnection(boost::asio::ip::tcp::socket && socket, acs::Handler::stream_buffer && buffer)
{
	SetSocket(std::move(socket), nullptr, std::move(buffer));
	return true;
}

size_t ws::Channel::OnReceive(const void * data, size_t size)
{
	auto buff = static_cast<const char*>(data);
	if(!VS_WsChannel::check_handshake(*this, m_was_handshake, buff, buff + size)) return 0;

	return m_cptr->read_some(static_cast<const char*>(data), size);
}

void ws::Channel::OnSend(size_t bytes_transferred)
{
	if (m_closing)
	{
		Shutdown();
	}
}

ws::Channel::Channel(boost::asio::io_service & ios)
	: base_connection(ios)
	, ChannelBase(
		[this]() { Shutdown(); },
		[this](vs::SharedBuffer && data) {return Send(std::move(data)); }
	)
{
}
