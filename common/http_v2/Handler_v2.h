#pragma once

#include "acs_v2/Handler.h"
#include "http/Router.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

namespace http_v2 {

class ACSHandler : public acs::Handler
{
	boost::asio::io_service::strand m_strand;
	std::weak_ptr<http::Router> m_http_router;
public:
	ACSHandler(boost::asio::io_service& ios) : m_strand(ios)
	{}
	void SetHttpRouter(const std::weak_ptr<http::Router>& http_router);
	virtual acs::Response Protocol(const acs::Handler::stream_buffer& buffer, unsigned channel_token = 0) override;
	virtual void Accept(boost::asio::ip::tcp::socket&& sock, acs::Handler::stream_buffer&& buffer) override;
};

}	// namespace http
