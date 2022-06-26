#include "Handler.h"
#include "TransceiverLib/Handshake.h"
#include "acs_v2/ConvertResponse.h"

/* transceiver */
namespace ts {
	Handler::Handler(const std::shared_ptr<net::ISetConnection<> > &proxiesPool) : m_proxiesPool(proxiesPool)
	{
	}
	Handler::~Handler()
	{
	}

	acs::Response Handler::Protocol(const acs::Handler::stream_buffer& buffer, unsigned /*channel_token*/)
	{
		unsigned long size = buffer.size();
		return acs::ConvertResponseCode(ts::ProtocolHandshake(buffer.data(), &size));
	}

	void Handler::Accept(boost::asio::ip::tcp::socket&& sock, acs::Handler::stream_buffer&& buffer)
	{
		if (auto pool = m_proxiesPool.lock()) {
			pool->SetTCPConnection(std::move(sock), std::move(buffer));
		}
	}

}