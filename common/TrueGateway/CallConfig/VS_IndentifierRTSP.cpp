#include "VS_IndentifierRTSP.h"
#include "TrueGateway/RTSP/VS_RTSPParser.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/curl_deleters.h"
#include "std/cpplib/MakeShared.h"
#include "std-generic/cpplib/IntConv.h"

#include <boost/make_shared.hpp>
#include <boost/regex.hpp>

#include <curl/curl.h>
#include "net/DNSUtils/VS_DNSTools.h"

static const boost::regex rtsp_uri_re(
	"[Rr][Tt][Ss][Pp]:(?://)?"
	"(?:" // optional login and password
		"([^:@]+)" // login
		"(?::([^@]*))?" // optional password
	"@)?"
	"(" // host
		"(?:[^:/]+)" // domain name or IPv4
		"|"
		"(?:\\[[0-9a-f:]+\\])" // IPv6
	")"
	"(?::([0-9]+))?" // optional port
	"((?:/|$).*)" // path on server
, boost::regex::optimize);

bool VS_IndentifierRTSP::IsMyCallId_Impl(string_view callId) const
{
	return VS_IsRTSPCallID(callId);
}

acs::Response VS_IndentifierRTSP::Protocol_Impl(const void* /*buf*/, std::size_t /*bufSz*/) const
{
	return acs::Response::not_my_connection;
}

bool VS_IndentifierRTSP::Resolve_Impl(VS_CallConfig &res, string_view callId, VS_UserData *from)
{
	if (!IsMyCallId_Impl(callId)) return false;

	res.SignalingProtocol = VS_CallConfig::RTSP;

	RTSPCallID rci(callId);
	auto set = [](std::string& s, string_view sv) {
		if (!sv.empty())
			s.assign(sv.data(), sv.size());
	};
	set(res.Login, rci.login);
	set(res.Password, rci.password);
	set(res.HostName, rci.hostname);
	res.Address.addr = net::dns_tools::single_make_a_aaaa_lookup(res.HostName);
	res.Address.port = vs::atoi_sv(rci.port);
	return !res.Address.addr.is_unspecified();
}

bool VS_IndentifierRTSP::CreateDefaultConfiguration_Impl(VS_CallConfig& cfg, const net::Endpoint &ep, VS_CallConfig::eSignalingProtocol protocol, string_view username)
{
	if (protocol != VS_CallConfig::RTSP)
	{
		return false;
	}

	cfg.SignalingProtocol = VS_CallConfig::RTSP;
	cfg.Address = ep;
	cfg.Login = std::string(username);
	if (!cfg.Address.addr.is_unspecified())
	{
		auto reply = net::dns::make_ptr_lookup(cfg.Address.addr).get();
		if (!reply.second)
			cfg.HostName = std::move(reply.first.name);
	}
	return true;
}

bool VS_IndentifierRTSP::PostResolve_Impl(VS_CallConfig& config, string_view callId, VS_UserData* /*from*/, bool /*block*/)
{
	if (config.SignalingProtocol != VS_CallConfig::RTSP)
		return false;

	if (config.Address.port == 0)
		config.Address.port = 554;

	if (config.Address.protocol == net::protocol::any)
		config.Address.protocol = net::protocol::TCP;

	config.resolveResult.NewCallId = RTSPCallID(callId).GetCallId();
	return true;
}

std::shared_ptr<VS_ParserInterface> VS_IndentifierRTSP::CreateParser_Impl(boost::asio::io_service::strand& /*strand*/, const std::shared_ptr<net::LoggerInterface>& /*logger*/)
{
	return vs::MakeShared<VS_RTSPParser>();
}

VS_IndentifierRTSP::RTSPCallID::RTSPCallID(string_view call_id)
{
	if (call_id.empty())
		return;
	assert(call_id[0] == '#');

	std::unique_ptr<CURL, CURL_deleter> curl(::curl_easy_init());
	assert(curl);

	int url_len = 0;
	std::unique_ptr<char[], curl_free_deleter> unescaped(::curl_easy_unescape(curl.get(), call_id.data() + 1, call_id.length() - 1, &url_len));
	url = unescaped.get();

	boost::smatch match;
	if (boost::regex_match(url, match, rtsp_uri_re))
	{
		auto to_sv = [](const boost::ssub_match& sm) {
			return sm.first != sm.second ? string_view(&*sm.first , sm.second - sm.first) : string_view();
		};
		login = to_sv(match[1]);
		password = to_sv(match[2]);
		hostname = to_sv(match[3]);
		port = to_sv(match[4]);
		path = to_sv(match[5]);
	}
}

std::string VS_IndentifierRTSP::RTSPCallID::GetCallId() const
{
	std::string result;
	result.reserve(sizeof("#rtsp:%2f%2f:/") + hostname.size() + port.size() + path.size() + 10/*space to escape 5 slashes*/);
	result += "#rtsp:%2f%2f";
	result += hostname;
	if (!port.empty())
	{
		result += ':';
		result += port;
	}
	for (char c : path)
		if (c == '/')
			result += "%2f";
		else
			result += c;
	return result;
}
