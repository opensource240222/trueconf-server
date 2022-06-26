#pragma once

#include "acs_v2/Handler.h"

namespace net {
	template<class Socket = boost::asio::ip::tcp::socket>
	class ISetConnection {
	public:
		virtual ~ISetConnection() {}
		virtual bool SetTCPConnection(Socket&& socket, acs::Handler::stream_buffer&& buffer) = 0;
	};
}