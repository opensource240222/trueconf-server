#pragma once

#include "acs_v2/ISetConnection.h"
#include "net/BufferedConnection.h"
#include "WebSocket/VS_WsChannelBase.h"
#include "std-generic/cpplib/SharedBuffer.h"

namespace ws {

class Channel : public net::ISetConnection<boost::asio::ip::tcp::socket>,
				public net::BufferedConnection<boost::asio::ip::tcp::socket>,
				public ChannelBase
{
public:
	using base_connection = net::BufferedConnection<boost::asio::ip::tcp::socket>;
	explicit Channel(boost::asio::io_service &ios);

	bool SetTCPConnection(boost::asio::ip::tcp::socket&& socket, acs::Handler::stream_buffer&& buffer) override;
	size_t OnReceive(const void* data, size_t size) override;
	bool OnError(const boost::system::error_code& /*ec*/) override { return false; }
	void OnSend(size_t bytes_transferred) override;

	bool Send(vs::SharedBuffer && data) {
		base_connection::Send(std::move(data));
		return true;
	}
	void Shutdown() { base_connection::Shutdown(); }
};

}