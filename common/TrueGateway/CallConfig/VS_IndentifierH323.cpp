#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/MakeShared.h"
#include "std/cpplib/VS_UserData.h"
#include "TrueGateway/h323/VS_H323ExternalGatekeeper.h"
#include "TrueGateway/h323/VS_H323GatekeeperStorage.h"
#include "TrueGateway/h323/VS_H323Parser.h"
#include "tools/H323Gateway/Lib/src/VS_Q931.h"
#include "tools/Server/vs_messageQueue.h"
#include "VS_IndentifierH323.h"
#include "std/cpplib/VS_CallIDUtils.h"

#include <boost/range/algorithm/remove_if.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <curl/curl.h>
#include "std/cpplib/curl_deleters.h"
#include "net/DNSUtils/VS_DNSTools.h"

#include <cctype>

#define DEBUG_CURRENT_MODULE VS_DM_MULTIGATEWAY

bool VS_IndentifierH323::IsMyCallId_Impl(string_view callId) const
{
	return VS_IsH323CallID(callId) || IsTelephone(callId);
}

acs::Response VS_IndentifierH323::Protocol_Impl(const void* buf, std::size_t sz) const
{
	acs::Response res = acs::Response::not_my_connection;

	unsigned char* in_buff = new unsigned char[sz];
	memcpy(in_buff, buf, sz);

	VS_TearMessageQueue queue;
	queue.PutTearMessage(in_buff, sz);

	if (queue.GetState() == VS_TearMessageQueue::e_tearBody)
		return acs::Response::next_step;

	if ( queue.IsExistEntireMessage() )
	{
		std::unique_ptr<unsigned char[]> ent_mess{};
		uint32_t ent_mess_sz = 0;
		if ( !queue.GetEntireMessage(ent_mess.get(), ent_mess_sz) )
		{
			if (ent_mess_sz)
			{
				ent_mess.reset(new unsigned char[ent_mess_sz]);
				if ( !queue.GetEntireMessage(ent_mess.get(), ent_mess_sz) )
				{
					return res;
				}
			}else{
				return res;
			}
		}else{
			return res;
		}

		VS_PerBuffer in_per_buff{ ent_mess.get(), ent_mess_sz * 8 };

		VS_Q931 theQ931_In;
		if ( theQ931_In.DecodeMHeader(in_per_buff))
		{
			res = acs::Response::accept_connection;
		}
		else
		{
			VS_H245MultimediaSystemControlMessage theMessage;
			if (theMessage.Decode( in_per_buff ))
			{
				res = acs::Response::accept_connection;
			}
		}
	}

	return res;
}

bool VS_IndentifierH323::Resolve_Impl(VS_CallConfig &cfg, string_view callId, VS_UserData *from)
{
	if (!IsMyCallId_Impl(callId)) return false;

	cfg.SignalingProtocol = VS_CallConfig::H323;

	std::string url;
	{
		std::unique_ptr<CURL, CURL_deleter> curl{ ::curl_easy_init() };
		assert(curl);

		int len = 0;
		std::unique_ptr<char[], curl_free_deleter> unescaped{::curl_easy_unescape(curl.get(), callId.data(), callId.length(), &len) } ;

		url.assign(unescaped.get(), len);
	}

	H323CallID hci{ url };
	cfg.UseAsTel = IsTelephone(url);
	cfg.UseAsDefault = !cfg.UseAsTel &&  hci.host.empty();
	H323CallID hci2(url, false);
	std::vector<std::pair<net::address, net::port>> ips;

	if (VS_H323ExternalGatekeeper::Instance().IsUseGatekeeperMode() && (cfg.UseAsDefault || cfg.UseAsTel)) {
		if (!VS_H323ExternalGatekeeper::Instance().ResolveOnExternalGatekeeper(string_view{ from->m_name.m_str, static_cast<size_t>(from->m_name.Length()) }, url, cfg.Address.addr, cfg.Address.port))
			return false;
	}else if (((hci.host.empty() && VS_H323GatekeeperStorage::Instance().GetRegisteredTerminalInfo(hci.name,ips))||
		(hci2.host.empty() && VS_H323GatekeeperStorage::Instance().GetRegisteredTerminalInfo(hci2.name, ips)))
		&& !ips.empty()) { // resolve on our gatekeeper
		auto ip_it = ips.begin();
		cfg.Address.addr = ip_it->first;
		cfg.HostName = ip_it->first.to_string();
	}
	else
	{
		const char *host_name = url.c_str() + 6; // skip #h323:
		const char *atPos = strchr(host_name, '@');
		const char *semicol_pos = strchr(host_name, ';'); // the start of parameters or DTMF
		const char *comma_pos = strchr(host_name, ','); // the start of DTMF
		if (atPos) host_name = atPos + 1;	 // skip "at" (#h323:<username>@ip)
		if ( *host_name == '/') host_name += 2;	 // skip two slashes

		int host_length = strchr( host_name, '/') - host_name;
		if ( host_length <= 0)
			host_length = strlen( host_name );

		if (semicol_pos)
		{
			const int semicol_host_length = semicol_pos - host_name;
			if (semicol_host_length >= 0 && semicol_host_length < host_length)
			{
				host_length = semicol_host_length;
			}
		}

		if (comma_pos)
		{
			const int comma_host_length = comma_pos - host_name;
			if (comma_host_length >= 0 && comma_host_length < host_length)
			{
				host_length = comma_host_length;
			}
		}

		char *host = static_cast<char *>(malloc(host_length + 1));
		assert(host);
		VS_SCOPE_EXIT{ free(host); };

		strncpy(host, host_name, host_length);
		host[ host_length ] = 0;

		char* post_pos = strrchr(host, ':');
		char* bracket_pos = strrchr(host, ']');
		// h 3 2 3 : @ [ : : 1 ] : 1 7 2 0
		//                     ^ ^
		//                     | |
		// an ipv6 address also contains ':'
		// if this is ipv4 address, then bracket_pos == 0
		if (post_pos && bracket_pos < post_pos)
		{
			int port_temp = 0;
			sscanf(post_pos,":%d", &port_temp);
			cfg.Address.port = port_temp;
			post_pos[ 0 ] = 0;
		}

		boost::system::error_code ec;
		cfg.Address.addr = net::address::from_string(host, ec);
		if (ec)
		{
			char* new_host = host;
			size_t new_length = strlen(host);
			// for ipv6 address
			if (host[0] == '[')
			{
				new_host++;
				new_length -= 2;
			}

			new_host[new_length] = '\0';

			std::string tcp_srv = "_h323cs._tcp.";
			tcp_srv += new_host;

		auto res = net::dns::make_srv_lookup(tcp_srv).get();
		dstream3 << "Found " << res.first.size() << " SRV entries for " << tcp_srv;
		if(!res.second && !res.first.empty()) // no error and not empty vector
		{
			cfg.Address.addr = net::dns_tools::single_make_a_aaaa_lookup(res.first.front().host);
			cfg.Address.port = res.first.front().port;
			cfg.HostName = res.first.front().host;
			dstream3 << "Using " << res.first.front().host << ":" << res.first.front().port;
		}
		else {
			cfg.Address.addr = net::dns_tools::single_make_a_aaaa_lookup(new_host);

			if (cfg.Address.addr.is_unspecified())
				return false;

			cfg.HostName = new_host;
		}

		}
	}

	dstream3 << "VS_IndentifierH323::Resolve_Impl resolved address " << cfg.Address.addr << (cfg.Address.port ? ":" + std::to_string(cfg.Address.port) : "");

	return true;
}

bool VS_IndentifierH323::CreateDefaultConfiguration_Impl(VS_CallConfig& cfg, const net::Endpoint &ep, VS_CallConfig::eSignalingProtocol protocol, string_view username)
{
	if (protocol != VS_CallConfig::H323)
	{
		return false;
	}

	cfg.H224Enabled = DEFAULT_FECC_ENABLED;
	cfg.SignalingProtocol = VS_CallConfig::H323;
	cfg.Address = ep;
	cfg.Login = std::string(username);

	if (!cfg.Address.addr.is_unspecified())
	{
		auto res = net::dns::make_ptr_lookup(cfg.Address.addr).get();
		if (!res.second)
			cfg.HostName = std::move(res.first.name);
	}
	return true;
}

bool VS_IndentifierH323::PostResolve_Impl(VS_CallConfig &config, string_view callId, VS_UserData *from,bool block)
{
	if (config.SignalingProtocol != VS_CallConfig::H323)
		return false;

	if (config.Address.port == 0)
		config.Address.port = 1720;

	config.Address.protocol = net::protocol::TCP;

	H323CallID hcid(callId);
	config.resolveResult.NewCallId = hcid.GetCallId();
	config.resolveResult.dtmf = hcid.GetDTMF();
	ReplaceTelephonePrefix(config);

	return true;
}

VS_CallConfig::eSignalingProtocol VS_IndentifierH323::GetSignalongProtocol_Impl() const
{
	return VS_CallConfig::H323;
}

std::shared_ptr<VS_ParserInterface> VS_IndentifierH323::CreateParser_Impl(boost::asio::io_service::strand& strand, const std::shared_ptr<net::LoggerInterface>& logger)
{
	return vs::MakeShared<VS_H323Parser>(strand, logger);
}

void VS_IndentifierH323::LoadConfigurations_Impl(std::vector<VS_CallConfig> &users, std::vector<VS_CallConfig> &hosts, const char *peerId)
{
	// * Moved to VS_H225RASParser
	//LoadConfigurations_Impl_Base(H323_PEERS_KEY, users, hosts);

	LoadVoipProtocolConfiguration();
}

bool VS_IndentifierH323::IsTelephone(string_view callId) const {
	return m_voipProtocol == VS_CallConfig::H323 && VS_IsTelephoneCallID(callId);
}

std::string VS_IndentifierH323::H323CallID::GetCallId() const
{
	std::string s = this->prefix + this->name;
	if (!this->host.empty())
	{
		s.push_back('@');
		s.append(this->host);
	}
	return s;
}

std::string VS_IndentifierH323::H323CallID::GetDTMF() const
{
	return dtmf;
}

VS_IndentifierH323::H323CallID::H323CallID(string_view callId, const bool removeVisualSeparator) {

	const char *possible_prefixes[] = { TEL_CALL_ID_PREFIX, H323_CALL_ID_PREFIX };

	for (const auto& prefix : possible_prefixes)
	{
		string_view prefix_view{ prefix, strlen(prefix) };
		if (boost::istarts_with(callId, prefix))
		{
			this->prefix = prefix;
			callId = callId.substr(prefix_view.length());
			break;
		}
	}

	const auto pos = callId.find('@');
	std::string data;
	std::string dtmf;

	if (pos == string_view::npos)
	{
		data = std::string(callId);
	}
	else
	{
		this->name = std::string(callId.substr(0, pos));
		data = std::string(callId.substr(pos));
	}
	const char *semicol_pos = strchr(data.c_str(), ';'); // the start of parameters (or dtmf)
	const char *comma_pos = strchr(data.c_str(), ',');
	const char *dtmf_pos = comma_pos ? comma_pos : semicol_pos;
	// extract DTMF
	if (dtmf_pos)
	{
		const char *p = dtmf_pos;
		if (*p == ';')
		{
			const char *eq = strchr(p+1, '=');
			const char *next_semicol = strchr(p+1, ';');
			if (!eq || !next_semicol)
			{
				dtmf_pos = p;
				p++;
				while (*p)
				{
					dtmf.push_back(*p);
					p++;
				}
			}
			else if (next_semicol < eq)
			{
				dtmf_pos = p;
				p++;
				dtmf.assign(p, next_semicol);
				p = next_semicol;
			}
			// there is no DTMF right after the server part, only a list of parameters
		}
		else // there is comma
		{
			dtmf_pos = p;
			p++;
			while (*p != ';' && *p != '\0')
			{
				dtmf.push_back(*p);
				p++;
			}
		}
		semicol_pos = p;

		string_view no_dtmf(data.c_str(), dtmf_pos - data.c_str());
		const auto pos = no_dtmf.find('@');
		if (pos == string_view::npos)
		{
			this->name = std::string(no_dtmf);
		}
		else
		{
			this->host = std::string(no_dtmf.substr(pos + 1));
		}
	}
	else
	{
		const auto pos = data.find('@');
		if (pos == string_view::npos)
		{
			this->name = data;
		}
		else
		{
			this->host = std::string(data.substr(pos + 1));
		}
	}
	
	if (semicol_pos && (*semicol_pos != '\0')) // handle call parameters
	{
		const char *params = semicol_pos;
		std::string key, value;

		const char *p = params;
		auto match = [&p](const int ch) -> bool
		{
			return p[0] == ch;
		};
		auto advance = [&p](void) -> void
		{
			if (*p != '\0')
			{
				p++;
			}
		};

		auto skipto = [&p, &advance](const int ch) -> void
		{
			for (;;)
			{
				if (p[0] == ch || p[0] == '\0')
				{
					break;
				}
				advance();
			}
		};

		auto getpos = [&p](void) -> const char * {
			return p;
		};

		for (;;)
		{
			key.clear();
			value.clear();
			if (match(';'))
			{
				advance();
				auto *start_key = getpos();
				skipto('=');
				if (match('='))
				{
					key.assign(start_key, getpos());
					advance();
					auto *start_val = getpos();
					skipto(';');
					value.assign(start_val, getpos());
				}
			}
			else if (match('\0'))
			{
				break;
			}
			else
			{
				skipto(';');
			}

			// DTMF
			if (strcasecmp("extension", key.c_str()) == 0 ||
				strcasecmp("ext", key.c_str()) == 0)
			{
				dtmf = value;
			}
		}
	}

	for (const auto &c : dtmf)
	{
		if (isalnum(c) || c == '*' || c == '#' || c == '+')
		{
			this->dtmf.push_back(c);
		}
	}

	if (removeVisualSeparator)
	{
		this->name.erase(boost::remove_if(this->name, VS_Indentifier::IsVisualSeparator), this->name.end());	// remove separators
		this->dtmf.erase(boost::remove_if(this->dtmf, VS_Indentifier::IsVisualSeparator), this->dtmf.end());
	}
}

#undef DEBUG_CURRENT_MODULE
