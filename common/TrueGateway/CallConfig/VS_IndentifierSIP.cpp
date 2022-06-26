#include "net/DNSUtils/VS_DNS.h"
#include "VS_IndentifierSIP.h"
#include "SIPParserLib/VS_SIPField_StartLine.h"
#include "std-generic/cpplib/string_view.h"
#include "std/cpplib/MakeShared.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_Replace.h"
#include "std/cpplib/VS_UserData.h"
#include "std/debuglog/VS_Debug.h"
#include "TrueGateway/sip/VS_SIPParser.h"

#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm/remove_if.hpp>
#include <codecvt>
#include <sstream>
#include <iomanip>


#include "std/debuglog/VS_Debug.h"
#include "net/DNSUtils/VS_DNSTools.h"

#define DEBUG_CURRENT_MODULE VS_DM_MULTIGATEWAY

namespace
{

	// https://en.wikipedia.org/wiki/UTF-8#Description
	//					   Encoding
	//   0 -  7 bits       0xxxxxxx	- ascii char
	inline bool is_ascii(const char ch) { return !(ch & (1 << 7)); }

	inline bool ContainsNonASCIIchars(const std::string &utf8_str) {
		return std::find_if_not(utf8_str.begin(), utf8_str.end(), [](const char ch) {return is_ascii(ch); }) != utf8_str.end();
}

	inline std::string EscapeNonASCII(const std::string &s) {
		std::string res;

		auto ws = vs::UTF8ToUTF32Convert(s);
		for (const int32_t& i : ws)
{
			if (i < 128 && i >= 0) {	// if is ascii
				res.push_back(static_cast<char>(i));
			}
			else {
				std::stringstream ss;
				int number_width = i >= 0x10000 ? 8 : 4;
				const char *escape_char = i >= 0x10000 ? "\\U" : "\\u"; // escape rules https://tools.ietf.org/html/rfc5137#section-6.1
				ss << escape_char << std::setw(number_width) << std::setfill('0') << std::hex << i;
				res += ss.str();
			}
		}
		return res;
	}
}

void VS_IndentifierSIP::InitBindAddrSet(vs::set<std::string, vs::str_less>&& bindAddrSet)
{
	m_bind_addr_set = std::move(bindAddrSet);
}

bool VS_IndentifierSIP::IsMyCallId_Impl(string_view callId) const
{
	return VS_IsSIPCallID(callId) || IsTelephone(callId);
}

acs::Response VS_IndentifierSIP::Protocol_Impl(const void* buf, size_t bufSz) const
{
	VS_SIPField_StartLine s;
	VS_SIPBuffer sip_buff(static_cast<const char *>(buf), bufSz);
	return (s.Decode(sip_buff) != TSIPErrorCodes::e_ok || s.GetSIPProto() != SIPPROTO_SIP20) ?
		acs::Response::not_my_connection : acs::Response::accept_connection;
}

bool VS_IndentifierSIP::IsVCSCallID(string_view callId)
{
	size_t atPosition = string_view::npos;
	if (VS_IsSIPCallID(callId)
		&& string_view::npos != (atPosition = callId.find('@')))
	{
		callId = callId.substr(atPosition + 1);
		if (callId.empty())
			return false;
		if (m_bind_addr_set.find(callId) != m_bind_addr_set.end())
			return true;
	}
	return false;
}

bool VS_IndentifierSIP::Resolve_Impl(VS_CallConfig &cfg, string_view callId, VS_UserData *from)
{
	if (! IsMyCallId_Impl(callId) ) return false;

	cfg.SignalingProtocol = VS_CallConfig::SIP;
	if (from) cfg.TcId = from->m_name;

	std::string callIdCopy(callId);
	if (IsVCSCallID(callId))
	{
		size_t pos = callIdCopy.find('@');
		if (pos != std::string::npos)
			callIdCopy.resize(pos);
	}
	SIPCallID sci{ callIdCopy };
	CleanTelephoneSeparators(sci);

	cfg.UseAsTel = IsTelephone(callId);
	cfg.UseAsDefault = !cfg.UseAsTel &&  sci.host.empty();

	if (cfg.UseAsTel || cfg.UseAsDefault) return true;

	cfg.HostName = sci.host;
	cfg.Address.port = sci.port;
	SIPCallID_to_ConnectionTypeSeq(sci, cfg.ConnectionTypeSeq, cfg.Address.protocol);

	return ResolveThroughDNS(sci.host, sci.port, cfg.ConnectionTypeSeq, cfg.Address.addr, cfg.Address.port, true);
}


bool VS_IndentifierSIP::ResolveThroughDNS(const std::string& host, net::port port, const std::vector<net::protocol>& desiredProtos, net::address &setAddr, net::port &setPort, bool /*block*/) const
{
	dstream3 << "VS_SIPIdentifier::ResolveThroughDNS host=" << host << ", port=" << port;

	if ( host.empty() ) return false;

	boost::system::error_code ec;
	setAddr = net::address::from_string(host, ec);

	if (ec)
	{
		const int flags = net::dns::real_inet | net::dns::flags_t::use_cache;

		std::vector<std::string> srvs;

		string_view tcp_service;
		string_view udp_service;
		string_view tls_service;

		auto res_naptr = net::dns::make_naptr_lookup(host, net::dns::naptr_type::domain, flags).get();
		bool find = false;

		if (!res_naptr.second) //no error
		{
			for (auto &item : res_naptr.first)
			{
				if (item.service == "SIP+D2T")
				{
					tcp_service = item.replacement;
					find = true;
				}
				else if (item.service == "SIP+D2U")
				{
					udp_service = item.replacement;
					find = true;
				}
				else if (item.service == "SIPS+D2T")
				{
					tls_service = item.replacement;
					find = true;
				}
			}
		}

		const auto set_srv_func = [&](net::protocol proto) noexcept -> void
		{
			switch (proto)
			{
			case net::protocol::TLS:
			{
				if (!find)
				{
					srvs.emplace_back("_sipinternaltls._tcp." + host);
					srvs.emplace_back("_sip._tls." + host);
				}
				else if (!tls_service.empty())
				{
					srvs.emplace_back(tls_service);
				}
				break;
			}
			case net::protocol::any: VS_FALLTHROUGH;
			case net::protocol::TCP:
			{
				if (!find)
				{
					srvs.emplace_back("_sip._tcp." + host);
				}
				else if (!tcp_service.empty())
				{
					srvs.emplace_back(tcp_service);
				}

				assert(proto == net::protocol::any || proto == net::protocol::TCP);

				if (!(proto == net::protocol::any))
					break;

				VS_FALLTHROUGH;
			}
			case net::protocol::UDP:
			{
				if (!find)
				{
					srvs.emplace_back("_sip._udp." + host);
				}
				else if (!udp_service.empty())
				{
					srvs.emplace_back(udp_service);
				}
				break;
			}
			default:;
			}
		};

		if (desiredProtos.empty())
		{
			set_srv_func(net::protocol::TLS);
			set_srv_func(net::protocol::any);
		}
		else
		{
			for (const auto &item : desiredProtos)
			{
				set_srv_func(item);
			}
		}

		std::string hostname = host;

		for (const auto &srv : srvs)
		{
			auto result = net::dns::make_srv_lookup(srv, flags).get();
			dstream3 << "Found " << result.first.size() << " SRV entries for " << srv;
			if (!result.second && !result.first.empty()) // no error and not empty vector
			{
				hostname = std::move(result.first.front().host);

				const auto port_res = result.first.front().port;
				if (port_res != 0)
					port = port_res;

				dstream3 << "Using " << hostname << ":" << port << "(" << port_res << ")";
				break;
			}
		}

		assert(setAddr.is_unspecified());

		//SRV: Therefore the target cannot be an IP address.
		auto res_a_aaaa = net::dns::make_a_aaaa_lookup(std::move(hostname), flags).get();
		for (auto item : { &res_a_aaaa.first, &res_a_aaaa.second })
		{
			assert(item);
			if (!item->ec) // no error
			{
				assert(!item->host.addrs.empty());
				assert(!item->host.addrs.front().is_unspecified());
				setAddr = item->host.addrs.front();
				break;
			}
		}

		if (setAddr.is_unspecified())
		{
			dstream3 << "VS_SIPIdentifier::ResolveThroughDNS resolve failed for '" << host << "'";
			return false;					// TODO: need to properly destroy VS_SIPTerminal
		}
	}

	if (setPort == 0)
		setPort = port;

	assert(!setAddr.is_unspecified());

	dstream3 << "VS_SIPIdentifier::ResolveThroughDNS resolved address " << setAddr << ":" << setPort;
	return true;
}

bool VS_IndentifierSIP::AsyncResolveImpl(std::function<void()>& resolveTask) const
{
	m_io.post(std::move(resolveTask));
	return true;
}

void VS_IndentifierSIP::SIPCallID_to_ConnectionTypeSeq(const SIPCallID &sci, std::vector<net::protocol>& connectionTypeSeq, net::protocol& proto)
	{
	if (sci.use_tcp)
	{
		proto = net::protocol::TCP;
		connectionTypeSeq.clear();
		connectionTypeSeq.emplace_back(net::protocol::TCP);
	}
	else if (sci.use_tls)
	{
		proto = net::protocol::TLS;
		connectionTypeSeq.clear();
		connectionTypeSeq.emplace_back(net::protocol::TLS);
	}
	else if (sci.use_udp)
	{
		proto = net::protocol::UDP;
		connectionTypeSeq.clear();
		connectionTypeSeq.emplace_back(net::protocol::UDP);
	}
	else
	{
		if (connectionTypeSeq.empty())
		{
			connectionTypeSeq.emplace_back(net::protocol::TCP);
			connectionTypeSeq.emplace_back(net::protocol::UDP);
			connectionTypeSeq.emplace_back(net::protocol::TLS);
		}
	}
}

bool VS_IndentifierSIP::PostResolve_Impl(VS_CallConfig &config, string_view callId, VS_UserData *from, bool block)
{
	if (config.SignalingProtocol != VS_CallConfig::SIP) return false;

	SIPCallID sci{ std::string(callId) };
	CleanTelephoneSeparators(sci);

	// https://tools.ietf.org/html/rfc5137#section-1
	// Unicode escaping used to encode characters that cannot be represented or transmitted directly with ASCII
	// i.e. we not escape spaces '@' '\' etc with Unicode escaping
	if (config.sip.UriEscapeMethod == VS_CallConfig::EscapeMethod::Unicode && ContainsNonASCIIchars(sci.name)){
			sci.name = EscapeNonASCII(sci.name);		// Example: ????client123 -> \u043a\u0443\u043a\u0443client123
	}

	config.resolveResult.NewCallId = sci.ToCallId();
	config.resolveResult.dtmf = sci.dtmf;

	SIPCallID_to_ConnectionTypeSeq(sci, config.ConnectionTypeSeq, config.Address.protocol);

	if ( sci.host.empty() ) sci.host = config.HostName;

	if (config.sip.RegistrationBehavior.get_value_or(VS_CallConfig::REG_UNDEFINED) == VS_CallConfig::REG_UNDEFINED)
		config.sip.RegistrationBehavior = VS_CallConfig::REG_DO_NOT_REGISTER;

	if (config.Bandwidth.get_value_or(0) == 0)
	{
		long data = config.Bandwidth.get_value_or(0);

		VS_RegistryKey key(false, TRANSCODERS_KEY);
		if (!key.IsValid() || key.GetValue(&data, sizeof(data), VS_REG_INTEGER_VT, "Gateway Bandwidth") <= 0)
		{
			data = video_presets::default_gateway_bandwidth;
		}

		config.Bandwidth = data;
	}

	if (config.Address.addr.is_unspecified())
		ResolveThroughDNS(sci.host, sci.port, config.ConnectionTypeSeq, config.Address.addr, config.Address.port, block);

	if ( config.Address.addr.is_unspecified())
	{
		auto &&call_manager = create_call_config_manager(config);
		call_manager.SetVerificationResult(VS_CallConfig::e_ServerUnreachable, true);
		return false;
	}

	if (config.Address.port == 0)
		config.Address.port = 5060;

	ReplaceTelephonePrefix(config);
	return true;
}

void VS_IndentifierSIP::LoadConfigurations_Impl(std::vector< VS_CallConfig> &users, std::vector<VS_CallConfig> &hosts, const char *peerId)
{
	LoadConfigurations_Impl_Base(SIP_PEERS_KEY, users, hosts, peerId);
}

bool VS_IndentifierSIP::ConvertConfiguration_Impl(VS_CallConfig &c, string_view tcId, const VS_ExternalAccount &a)
{
	const char tel[] = "#tel";
	const char sip[] = "#sip";
	if (a.m_protocol != string_view{tel, sizeof(tel) - 1} && a.m_protocol != string_view{ sip, sizeof(sip) - 1}) return false;

	c.SignalingProtocol = VS_CallConfig::SIP;
	c.TcId = std::string(tcId);
	c.UseAsTel = a.m_protocol == string_view{ tel, sizeof(tel) - 1 } || a.m_isVoIPServer;
	c.HostName = a.m_host;
	c.Address.port = a.m_port;
	c.Login = a.m_login;
	c.Password = a.m_password;
	c.TelephonePrefixReplace = a.m_TelephonePrefixReplace;
	c.sip.RegistrationBehavior = static_cast<VS_CallConfig::eRegistrationBehavior>(a.m_registrationBehavior);

	const int flags = net::dns::real_inet | net::dns::use_cache;
	c.Address.addr = net::dns_tools::single_make_a_aaaa_lookup(a.m_host, flags);
	return true;
}

bool VS_IndentifierSIP::CreateDefaultConfiguration_Impl(VS_CallConfig& cfg, const net::Endpoint &ep,
                                                        VS_CallConfig::eSignalingProtocol protocol,
                                                        string_view username)
{
	if (protocol != VS_CallConfig::SIP)
		return false;

	cfg.H224Enabled = DEFAULT_FECC_ENABLED;
	cfg.sip.TelephoneEventSignallingEnabled = DEFAULT_TELEPHONE_EVENT_ENABLED;
	cfg.SignalingProtocol = VS_CallConfig::SIP;
	cfg.Address = ep;
	cfg.Login = std::string(username);

	if (!cfg.Address.addr.is_unspecified())
	{
		const int flags = net::dns::use_cache | net::dns::force_search | net::dns::insensitive_ttl;
		auto res = net::dns::make_ptr_lookup(cfg.Address.addr, flags).get();
		if (!res.second)
			cfg.HostName = std::move(res.first.name);
	}
	return true;
}

VS_IndentifierSIP::SIPCallID::SIPCallID(std::string callId)
	: orig_call_id(std::move(callId))
{
	string_view p(orig_call_id);

	if (boost::istarts_with(p, string_view(SIP_CALL_ID_PREFIX)))
	{
		this->prefix = SIP_CALL_ID_PREFIX;
		p = p.substr(prefix.length());
	}
	else
	if (boost::istarts_with(p, string_view(TEL_CALL_ID_PREFIX)))
	{
		this->prefix = TEL_CALL_ID_PREFIX;
		p = p.substr(prefix.length());
	}

	std::string tmp(p);

	auto before_replace_len = tmp.length();

	boost::ireplace_all(tmp, ";transport=tcp", "");
	this->use_tcp = before_replace_len != tmp.length();

	before_replace_len = tmp.length();
	boost::ireplace_all(tmp, ";transport=tls", "");
	this->use_tls = before_replace_len != tmp.length();

	before_replace_len = tmp.length();
	boost::ireplace_all(tmp, ";transport=udp", "");
	this->use_udp = before_replace_len != tmp.length();

	bool has_dtmf = false;
	auto ext_dtmf = strstr(tmp.c_str(), ";extension=");
	if (!ext_dtmf) 	ext_dtmf = strstr(tmp.c_str(), ";ext=");
	if (ext_dtmf){
		auto dtmf_end = strchr(ext_dtmf + 1, ';');
		has_dtmf = true;

		if (!dtmf_end)	{
			this->dtmf = strchr(ext_dtmf, '=') + 1;
			tmp.resize(ext_dtmf - tmp.c_str());
		}
		else{	// in case when dtmf will be not the last
			string_view v(ext_dtmf);
			this->dtmf = static_cast<std::string>(v.substr(v.find('='), v.find(';')));
			VS_ReplaceAll(tmp, v.substr(0, v.find(';')), "");
		}
	}

	if (!has_dtmf)
	{
		const char *host = strchr(tmp.c_str(), '@');
		const char *dtmf = host;

		if (host == NULL) dtmf = tmp.c_str();
		if (*dtmf) ++dtmf;
		dtmf = strchr(dtmf, DEFAULT_DTMF_PREFIX);
		if ( dtmf )
		{
			this->dtmf = dtmf + sizeof(DEFAULT_DTMF_PREFIX);
			tmp.resize( dtmf - tmp.c_str() );
		}
		else if ((dtmf = strchr(tmp.c_str(), ';')) != nullptr){
			string_view v(dtmf + 1);
			if (std::all_of(v.begin(), v.end(), [this](char ch){return IsValidDTMF(ch); })){
				this->dtmf = dtmf + 1;
				tmp.resize(dtmf - tmp.c_str());
			}
		}
		else if ((dtmf = strchr(tmp.c_str(), ',')) != nullptr){
			this->dtmf = dtmf;				// first ',' will be included in pause
			tmp.resize(dtmf - tmp.c_str());
		}
	}

	this->port = 0;
	{
		const char *host = strchr(tmp.c_str(), '@');
		if (host)
		{
			const char *ipv6 = strchr(host, ']');
			const char *port = ipv6 ? strchr(ipv6, ':') : strchr(host, ':');
			if (port)
			{
				this->port = atoi(port + 1);
				*(char *)port = 0;
			}

			this->host = host + 1;
			*(char *)host = 0;
		}
	}

	const char *username = tmp.c_str();
	this->name = username;
}

std::string VS_IndentifierSIP::SIPCallID::ToCallId() const
{
	std::string s = this->prefix + this->name;
	if (! this->host.empty())
	{
			s += '@';
			s += this->host;
	}

	return s;
}

bool VS_IndentifierSIP::SIPCallID::IsValidDTMF(char ch) const{
	return strchr("0123456789*#,WPABCD", toupper(ch)) != nullptr;	// 'W' and 'P' - is pause like ','
}

VS_CallConfig::eSignalingProtocol VS_IndentifierSIP::GetSignalongProtocol_Impl() const
{
	return VS_CallConfig::SIP;
}

std::shared_ptr<VS_ParserInterface> VS_IndentifierSIP::CreateParser_Impl(boost::asio::io_service::strand& strand, const std::shared_ptr<net::LoggerInterface>& logger)
{
	return vs::MakeShared<VS_SIPParser>(strand, m_serverInfo, logger);
}

bool VS_IndentifierSIP::IsTelephone(string_view callId) const {
	return m_voipProtocol == VS_CallConfig::SIP && VS_IsTelephoneCallID(callId);
}

void VS_IndentifierSIP::CleanTelephoneSeparators(SIPCallID &sipCallId) const
{
	// remove visual separators from username for #tel: calls
	if (IsTelephone(sipCallId.orig_call_id))
		sipCallId.name.erase(boost::remove_if(sipCallId.name, VS_Indentifier::IsVisualSeparator),	sipCallId.name.end());	// remove separators
}

#undef DEBUG_CURRENT_MODULE