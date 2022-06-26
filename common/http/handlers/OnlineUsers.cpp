#include "http/handlers/OnlineUsers.h"
#include "std/cpplib/VS_Replace.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/md5.h"

#include <boost/algorithm/string.hpp>
#include <curl/curl.h>
#include "std/cpplib/curl_deleters.h"

#include "std-generic/cpplib/IntConv.h"

namespace http {
namespace handlers {

static const string_view CALLBACK_PARAM("callback=");

static const string_view LOGIN_STRING("s2");
static const string_view USERS_ONLINE_LIST_STRING("s4");

static const std::string InternalServerError_500("HTTP/1.1 500 Internal Server Error\r\n\r\n");

boost::optional<std::string> OnlineUsers::HandleRequest(string_view in, const net::address& /*from_ip*/, const net::port /*from_port*/)
{
	// request-line
	auto sv(in);
	auto newline = sv.find(NewLine);
	if (newline == string_view::npos)
		return { std::string(BadRequest_400) };
	sv.remove_suffix(sv.length() - newline);

	// callback param
	string_view callback("vp_status");
	auto pos = sv.find(CALLBACK_PARAM);
	if (pos != string_view::npos)
	{
		auto sv2(sv);
		sv2.remove_prefix(pos + CALLBACK_PARAM.length());
		auto end = sv2.find_first_of("?&= ");
		sv2.remove_suffix(sv2.length() - end);
		if (!sv2.empty())
			callback = sv2;
	}

	auto req = get_text_between(sv, ' ', ' ');
	if (req.empty())
		return { std::string(BadRequest_400) };

	auto path(req);
	pos = path.find_first_of("?&");
	if (pos != decltype(path)::npos)
		path.remove_suffix(path.length() - pos);

	std::vector<string_view> strs;
	typedef boost::algorithm::find_iterator<string_view::const_iterator> find_iterator;
	for (find_iterator params_it(path, boost::algorithm::token_finder(
		[](char x) { return x != '/'; }, boost::algorithm::token_compress_on));
		!params_it.eof(); ++params_it)
	{
		strs.emplace_back(params_it->begin(), boost::distance(*params_it));
	}
	if (strs.size() < 3)
		return { std::string(BadRequest_400) };

	std::string to_hash;
	to_hash += "/";
	to_hash += strs[0];
	to_hash += "/";
	to_hash += strs[1];

	string_view type = strs[0];
	string_view timestamp;
	string_view client_hash;
	UsersStatusesInterface::UsersList users_statuses;
	users_statuses.reserve(100);
	if (type == USERS_ONLINE_LIST_STRING) {
		timestamp = strs[1];
		client_hash = strs[2];
	} else if (type == LOGIN_STRING) {
		if (strs.size() < 4)
			return { std::string(BadRequest_400) };
		timestamp = strs[2];
		client_hash = strs[3];
		to_hash += "/";
		to_hash += strs[2];

		std::unique_ptr<CURL, CURL_deleter> curl{ ::curl_easy_init() };
		typedef boost::algorithm::find_iterator<string_view::const_iterator> find_iterator;
		for (find_iterator params_it(strs[1], boost::algorithm::token_finder(
			[](char x) { return x != ','; }, boost::algorithm::token_compress_on));
			!params_it.eof(); ++params_it)
		{
			string_view user(params_it->begin(), boost::distance(*params_it));
			int new_len = 0;
			std::unique_ptr<char, curl_free_deleter> unescaped_user{ ::curl_easy_unescape(curl.get(), user.data(), user.length(), &new_len) };
			if (!!unescaped_user)
				users_statuses.emplace_back(unescaped_user.get(), USER_LOGOFF);
		}
	} else
		return { std::string(BadRequest_400) };
	if (timestamp.empty())
		return { std::string(BadRequest_400) };

	long long t = vs::atoll_sv(timestamp);
	
	auto now = time(0);
	if (now > t)
		return { std::string(BadRequest_400) };

	std::string status_security;
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	if (key.GetString(status_security, STATUS_KEY_TAG_NAME))
		to_hash += status_security;
	char server_hash[33] = { 0 };
	VS_ConvertToMD5(to_hash, server_hash);

	if (client_hash != server_hash)
		return{ std::string(BadRequest_400) };

	std::string html = "HTTP/1.1 200 Ok\r\n";
	if (!m_user_agent.empty())
	{
		html += "Server: ";
		html += m_user_agent;
		html += "\r\n";
	}
	html += "Cache-Control: private, no-cache, must-revalidate \r\nPragma: no-cache \r\nExpires: Mon, 26 Jul 1997 05:00:00 GMT\r\n";
	html += "Connection: close\r\n";
	html += "Content-type: application/x-javascript; charset=utf-8\r\n\r\n";

	if (!callback.empty())
		html += callback;
	else
		html += "vp_status";
	html += "({";

	auto imp = m_users_statuses_interface.lock();
	if (!imp)
		return { std::string(InternalServerError_500) };

	if (type == USERS_ONLINE_LIST_STRING)
		imp->ListOfOnlineUsers(users_statuses);
	else if (type == LOGIN_STRING) {
		imp->UsersStatuses(users_statuses);
	} else
		return { std::string(BadRequest_400) };

	size_t i = 0;
	for (const auto& it : users_statuses)
	{
		auto login = it.first;
		VS_ReplaceAll(login, "\\", "\\\\");
		html += "\"";
		html += login;
		html += "\":";
		html += std::to_string((int8_t)it.second);
		if (++i < users_statuses.size())
			html += ",";
	}

	html += "})\r\n\r\n";

	return { std::move(html) };
}

} // handlers
} // http
