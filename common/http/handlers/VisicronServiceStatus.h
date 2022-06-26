#pragma once

#include "http/handlers/Interface.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/weak_ptr.hpp>

namespace http {
namespace handlers {

class VisicronServiceStatus : public Interface
{
	std::string m_user_agent;
public:
	VisicronServiceStatus(const std::string& user_agent): m_user_agent(user_agent)
	{}
	boost::optional<std::string> HandleRequest(string_view in, const net::address& from_ip, const net::port from_port) override
	{
		std::string html = "HTTP/1.1 200 Ok\r\n";
		if (!m_user_agent.empty())
		{
			html += "Server: ";
			html += m_user_agent;
			html += "\r\n";
		}
		html += "Connection: close\r\n";
		html += "Content-type: text/html\r\n\r\n";
		html += "<html>\n<head><title> Status of ";
		html += m_user_agent;
		html += "</title> </head>\n";
		html += "<body>";
		html += m_user_agent;
		html += "<br/>Status:";
		html += "OK <br/>";
		html += "</body>\n</html>\n";
		html += "\r\n";

		return { std::move(html) };
	}
};

} // handlers
} // http

