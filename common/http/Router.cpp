#include "http/Router.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string.hpp>


namespace http {

static const boost::regex regex_ContentLength(".*\\r\\nContent-Length\\h*:\\h*(\\d+)\\h*.*", boost::regex::icase);
//static const boost::regex regex_UserAgent(".*\\r\\nUser-Agent:\\h*(.*?)\\h*\\r\\n.*", boost::regex::icase);
static auto c_get = string_view("GET ");
static auto c_post = string_view("POST ");

template <class InputIterator>
int getValue(InputIterator first, InputIterator last, const boost::regex &r, const int default_value)
{
	boost::match_results<InputIterator> m;
	if (!boost::regex_match(first, last, m, r)) return default_value;
	int res;
	sscanf(m.str(1).c_str(), "%d", &res);
	return res;
}

void Router::AddHandler(string_view key, const std::shared_ptr<http::handlers::Interface>& handler)
{
	m_request_handlers.emplace(key, handler);
}

Router::FindHandlerResult Router::FindHandler(string_view buffer_sv, std::shared_ptr<http::handlers::Interface>& handler)
{
	if (buffer_sv.substr(0, c_get.length()) != c_get &&
		buffer_sv.substr(0, c_post.length()) != c_post)
		return FindHandlerResult::not_my;
	auto headers_end = buffer_sv.find(http::handlers::HeadersEnd);
	if (headers_end == string_view::npos)
		return FindHandlerResult::need_more;

	auto headers = buffer_sv.substr(0, headers_end);
	auto content_length = getValue(headers.begin(), headers.end(), regex_ContentLength, 0);
	if (content_length > 0)
	{
		auto payload_sv(buffer_sv);
		payload_sv.remove_prefix(headers_end + http::handlers::HeadersEnd.length());
		if (content_length > (int)payload_sv.length())	// check if we have enough payload
			return FindHandlerResult::need_more;
	}

	// find corresponding handler
	for (const auto& h : m_request_handlers)
	{
		if (buffer_sv.substr(0, h.first.length()) == h.first)
		{
			handler = h.second;
			return FindHandlerResult::accept;
		}
	}
	return FindHandlerResult::not_my;
}

}  // namespace net::http
