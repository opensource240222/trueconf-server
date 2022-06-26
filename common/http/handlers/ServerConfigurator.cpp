#include "http/handlers/ServerConfigurator.h"

#include <boost/algorithm/string.hpp>

namespace http {
namespace handlers {

boost::optional<std::string> ServerConfigurator::HandleRequest(string_view in, const net::address& /*from_ip*/, const net::port /*from_port*/)
{
	auto content(in);
	auto pos = content.find(HeadersEnd);
	if (pos == string_view::npos)
		return boost::none;
	content.remove_prefix(pos + HeadersEnd.length());	// get content

	m_firePushPostMethodData(content.data(), content.length());

	std::string html = "HTTP/1.1 200 Ok\r\n"
		"Cache-Control: private, no-cache, must-revalidate\r\n"
		"Pragma: no-cache\r\n"
		"Expires: Mon, 26 Jul 1997 05:00:00 GMT\r\n"
		"Connection: close\r\n";
	if (!m_user_agent.empty())
	{
		html += "Server: ";
		html += m_user_agent;
		html += "\r\n";
	}
	html += "\r\n";
	return { std::move(html) };
}

} // handlers
} // http
