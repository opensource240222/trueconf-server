#pragma once
#include "http/handlers/Interface.h"
#include "streams_v2/Router/Router.h"
#include "acs_v2/Service.h"
#include "newtransport/Router/Router.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/weak_ptr.hpp>

namespace http {
namespace handlers {

	// TODO: (ap) RouterMonitor class name

class RouterMonitor :
	public http::handlers::Interface
{
private:
	std::weak_ptr<transport::Router> m_transport_router;
	std::weak_ptr<stream::RouterV2> m_stream_router;
	std::weak_ptr<acs::Service> m_acs;
public:
	RouterMonitor(const std::weak_ptr<transport::Router>& transport_router, const std::weak_ptr<stream::RouterV2>& stream_router, const std::weak_ptr<acs::Service>& acs)
		:m_transport_router(transport_router), m_stream_router(stream_router), m_acs(acs)
	{

	}
	boost::optional<std::string> HandleRequest(string_view in, const net::address& from_ip, const net::port from_port) override;
};

} //handlers
} //http
