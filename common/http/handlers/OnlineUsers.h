#pragma once

#include "http/handlers/Interface.h"
#include "statuslib/status_types.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/weak_ptr.hpp>

namespace http {
namespace handlers {

class OnlineUsers : public Interface
{
	boost::weak_ptr<UsersStatusesInterface> m_users_statuses_interface;
	std::string m_user_agent;
public:
	OnlineUsers(const boost::weak_ptr<UsersStatusesInterface>& users_statuses_interface, const std::string& user_agent) :
		m_users_statuses_interface(users_statuses_interface), m_user_agent(user_agent)
	{}
	boost::optional<std::string> HandleRequest(string_view in, const net::address& from_ip, const net::port from_port) override;
};

} // handlers
} // http

