#pragma once

#include "../../acs_v2/Handler.h"

#include <memory>

namespace transport
{
	class Router;

	class JsonMonitorHandler : public acs::Handler
	{
	 public:
		 JsonMonitorHandler(const std::weak_ptr<Router>& router);
		 acs::Response Protocol(const acs::Handler::stream_buffer& buffer, unsigned channel_token = 0) override;
		 void Accept(boost::asio::ip::tcp::socket&& socket, acs::Handler::stream_buffer&& buffer) override;
	private:
		std::weak_ptr<Router> m_router;
	};
}
