#pragma once

#include "http/handlers/Interface.h"
#include "ldap_core/common/Common.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/weak_ptr.hpp>

namespace http {
namespace handlers {

class ServerConfigurator : public http::handlers::Interface
{
	VS_PushDataSignal m_firePushPostMethodData;
	std::string m_user_agent;
public:
	ServerConfigurator(const VS_PushDataSignalSlot& slot, const std::string& user_agent):
		m_user_agent(user_agent)
	{
		m_firePushPostMethodData.connect(slot);
	}
	boost::optional<std::string> HandleRequest(string_view in, const net::address& from_ip, const net::port from_port) override;
};

} // handlers
} // http

