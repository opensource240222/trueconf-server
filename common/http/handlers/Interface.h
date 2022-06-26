#pragma once
#include "std-generic/cpplib/string_view.h"
#include "net/Address.h"
#include "net/Port.h"

#include <boost/optional/optional.hpp>

namespace http {
namespace handlers {

static const string_view NewLine("\r\n");
static const string_view HeadersEnd("\r\n\r\n");
static const string_view ContentLength("Content-Length:");
static const string_view BadRequest_400("HTTP/1.1 400 Bad request\r\n\r\n");

struct Interface
{
	virtual ~Interface() {}
	virtual boost::optional<std::string> HandleRequest(string_view in, const net::address& from_ip, const net::port from_port) = 0;
	string_view get_text_between(string_view s, char begin, char end)
	{
		auto pos1 = s.find_first_of(begin);
		if (pos1 == string_view::npos)
			return {};
		s.remove_prefix(++pos1);
		auto finish = s.find_first_of(end);
		if (finish == string_view::npos)
			return {};
		s.remove_suffix(s.length() - finish);
		return s;
	};
};

} // handlers
} // http
