#include "RouterMonitor.h"
#include "std/cpplib/json/writer.h"

#include "std/cpplib/md5.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_Utils.h"

#include <boost/algorithm/string.hpp>
#include <vector>
#include <boost/lexical_cast.hpp>

namespace http {
namespace handlers {

static const char GET_RM_TR[] = "GET /rm/tr";
static const char GET_RM_ACS[] = "GET /rm/acs";
static const char GET_RM_SR[] = "GET /rm/sr";

boost::optional<std::string> http::handlers::RouterMonitor::HandleRequest(string_view in, const net::address & /*from_ip*/, const net::port /*from_port*/)
{
	auto sv(in);
	auto newline = sv.find(NewLine);
	if (newline == string_view::npos)
		return { std::string(BadRequest_400) };
	sv.remove_suffix(sv.length() - newline);

	auto req = get_text_between(sv, ' ', ' ');
	if (req.empty())
		return { std::string(BadRequest_400) };

	auto sv2(req);
	////parse url and get key-value array
	std::vector<std::pair<string_view, string_view>> kv;
	kv=VS_ParseUrlParams(sv2);

	//parse param auth
	auto it = std::find_if(kv.begin(), kv.end(), [](const std::pair<string_view, string_view>& p) {
		return boost::iequals(p.first, string_view("auth"));
	});
	if (kv.empty() || it == kv.end() || it->second.empty())
		return { std::string(BadRequest_400) };

	// parse auth format: [RAND]*[TIMESTAMP]*[SIGN]
	// [SIGN] = md5(RAND + TIMESTAMP + SECRET)
	// SECRET - value CFG\Session Id
	std::vector<string_view> strs;
	typedef boost::algorithm::find_iterator<string_view::const_iterator> find_iterator;
	for (find_iterator params_it(it->second, boost::algorithm::token_finder(
		[](char x) { return x != '*'; }, boost::algorithm::token_compress_on));
		!params_it.eof(); ++params_it)
	{
		strs.emplace_back(params_it->begin(), boost::distance(*params_it));
	}
	if (strs.size() != 3)
		return { std::string(BadRequest_400) };

	string_view rand(strs[0]);
	string_view timestamp(strs[1]);
	string_view arrived_sign(strs[2]);

	//[RAND]*[TIMESTAMP]*[SIGN] => split *
	time_t timestamp_query = ::strtoll(timestamp.data(), NULL, 10); // timestamp.data() => valid, because split *

	if(std::chrono::system_clock::from_time_t(timestamp_query) < std::chrono::system_clock::now()) //bad timestamp
		return { std::string(BadRequest_400) };

	std::string secret;
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	key.GetString(secret, SESSIONID_TAG_NAME);

	MD5 md5;
	md5.Update(rand);
	md5.Update(timestamp);
	md5.Update(secret);
	md5.Final();
	char sign[33];
	md5.GetString(sign);

	string_view hashed_val(sign);
	if (hashed_val!= arrived_sign)
		return { std::string(BadRequest_400) };

	std::stringstream ss;
	json::Object obj;

	bool result_monitor = false;

	sv = sv.substr(0, sv.find('?'));

	if (boost::equal(sv, string_view{ GET_RM_TR, sizeof(GET_RM_TR) - 1 }))
	{
		auto impl = m_transport_router.lock();
		if (impl)
		{
			transport::Monitor::TmReply reply;
			impl->GetMonitorInfo(reply);
			reply.ToJson(obj);
			result_monitor = true;
		}
	}
	else if (boost::equal(sv, string_view{ GET_RM_ACS, sizeof(GET_RM_ACS) - 1 }))
	{
		auto impl = m_acs.lock();
		if (impl)
		{
			acs::Monitor::AcsReply reply;
			impl->GetMonitorInfo(reply);
			reply.ToJson(obj);
			result_monitor = true;
		}
	}
	else if (boost::equal(sv, string_view{ GET_RM_SR, sizeof(GET_RM_SR) - 1 }))
	{
		auto impl = m_stream_router.lock();
		if (impl)
		{
			stream::Monitor::StreamReply reply;
			impl->GetMonitorInfo(reply);
			reply.ToJson(obj);
			result_monitor = true;
		}
	}

	if(!result_monitor)
		return { std::string(BadRequest_400) };

	json::Writer::Write(obj, ss);

	std::string html = "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\n"
		"Content Length: ";
	html += std::to_string(ss.str().size());
	html += "\r\n\r\n";
	html += ss.str();

	return { std::move(html) };
}

} //handlers
} //http