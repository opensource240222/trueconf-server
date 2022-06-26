#pragma once

#include "../../acs_v2/Handler.h"

namespace transport
{
	class Router;

	class MonitorHandler : public acs::Handler
	{
	 public:
	 	MonitorHandler(Router* router);
		acs::Response Protocol(const acs::Handler::stream_buffer& buffer, unsigned channel_token = 0) override;
		void Accept(boost::asio::ip::tcp::socket&& sock, acs::Handler::stream_buffer&& buffer) override;
	private:
		Router* m_router;
	};
}
