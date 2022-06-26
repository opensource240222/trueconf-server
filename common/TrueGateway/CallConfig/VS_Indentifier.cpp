#include "VS_Indentifier.h"
#include "VS_IndentifierH323.h"
#include "VS_IndentifierRTSP.h"
#include "VS_IndentifierH225RAS.h"
#include "VS_IndentifierSIP.h"
#include "SIPParserBase/VS_Const.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std-generic/cpplib/StrCompare.h"
#include "TrueGateway/interfaces/VS_ParserInterface.h"
#include "TrueGateway/VS_GatewayStarter.h"

#include <boost/make_shared.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "std-generic/compat/functional.h"
#include "net/DNSUtils/VS_DNSTools.h"

boost::shared_ptr<VS_Indentifier> VS_Indentifier::MakeIndentifierChain(const std::vector<VS_CallConfig::eSignalingProtocol> &protocols, boost::asio::io_service &io, const std::string& serverInfo)
{
	boost::shared_ptr<VS_Indentifier> head, tail;

	for (auto p : protocols) {
		boost::shared_ptr<VS_Indentifier> next;
		if (p == VS_CallConfig::RTSP) {
			next = boost::make_shared<VS_IndentifierRTSP>(io);
		} else if (p == VS_CallConfig::SIP) {
			next = boost::make_shared<VS_IndentifierSIP>(io, serverInfo);
		} else if (p == VS_CallConfig::H323) {
			next = boost::make_shared<VS_IndentifierH323>(io);
		} else if (p == VS_CallConfig::H225RAS) {
			next = boost::make_shared<VS_IndentifierH225RAS>(io);
		}

		if (!head) {
			head = next;
		}

		if (tail) {
			tail->SetNext(next);
		}

		tail = next;
	}

	return head;
}

boost::shared_ptr<VS_Indentifier> VS_Indentifier::GetCommonIndentifierChain(boost::asio::io_service &io, const std::string& serverInfo)
{
	return MakeIndentifierChain({ VS_CallConfig::RTSP, VS_CallConfig::SIP, VS_CallConfig::H323, VS_CallConfig::H225RAS }, io, serverInfo);
}

void VS_Indentifier::SetNext(const boost::shared_ptr< VS_Indentifier > &i)
{
	m_next = i;
}

acs::Response VS_Indentifier::Protocol_Impl(const void*, std::size_t) const
{
	return acs::Response::not_my_connection;
}

bool VS_Indentifier::IsMyCallId(string_view callId) const
{
	bool res = IsMyCallId_Impl(callId);
	if (!res && m_next) res = m_next->IsMyCallId(callId) ;
	return res;
}

acs::Response VS_Indentifier::Protocol(const void* buf, std::size_t bufSz) const
{
	auto res = Protocol_Impl(buf, bufSz);
	if (res == acs::Response::not_my_connection && m_next)
		res = m_next->Protocol(buf, bufSz);
	return res;
}


bool VS_Indentifier::Resolve(VS_CallConfig &cfg, string_view callId, VS_UserData *from)
{
	bool res = Resolve_Impl(cfg, callId, from);
	if (!res && m_next) res = m_next->Resolve(cfg, callId, from);
	return res;
}

bool VS_Indentifier::PostResolve(VS_CallConfig &config, string_view callId, VS_UserData *from, bool block)
{
	bool res = PostResolve_Impl(config, callId, from, block);
	if (!res && m_next) res = m_next->PostResolve(config, callId, from,block);
	return res;
}

void VS_Indentifier::LoadConfigurations(std::vector<VS_CallConfig> &users, std::vector<VS_CallConfig> &hosts, const char *peerId)
{
	LoadConfigurations_Impl(users, hosts, peerId);
	if (m_next) m_next->LoadConfigurations(users, hosts, peerId);
}

bool VS_Indentifier::ConvertConfiguration(VS_CallConfig &cfg, string_view tcId, const VS_ExternalAccount &a)
{
	bool res = ConvertConfiguration_Impl(cfg, tcId, a);
	if (!res && m_next) res = m_next->ConvertConfiguration(cfg, tcId, a);
	return res;
}

bool VS_Indentifier::CreateDefaultConfiguration(VS_CallConfig& cfg, const net::Endpoint &ep,
												VS_CallConfig::eSignalingProtocol protocol,
                                                string_view username)
{
    bool res = CreateDefaultConfiguration_Impl(cfg, ep, protocol, username);
	if (!res && m_next)
		res = m_next->CreateDefaultConfiguration(cfg, ep, protocol, username);
	return res;
}

std::shared_ptr<VS_ParserInterface> VS_Indentifier::CreateParser(boost::asio::io_service::strand& strand, const void* buf, std::size_t bufSz, const std::shared_ptr<net::LoggerInterface>& logger)
{
	if (Protocol_Impl(buf, bufSz) == acs::Response::accept_connection)
		return CreateParser_Impl(strand, logger);
	if (m_next)
		return m_next->CreateParser(strand, buf, bufSz, logger);
	return nullptr;
}

std::shared_ptr<VS_ParserInterface> VS_Indentifier::CreateParser(boost::asio::io_service::strand& strand, string_view callId, const std::shared_ptr<net::LoggerInterface>& logger)
{
	if (IsMyCallId_Impl(callId))
		return CreateParser_Impl(strand, logger);
	if (m_next)
		return m_next->CreateParser(strand, callId, logger);
	return nullptr;
}

std::shared_ptr<VS_ParserInterface> VS_Indentifier::CreateParser(boost::asio::io_service::strand& strand, const VS_CallConfig &config, const std::shared_ptr<net::LoggerInterface>& logger)
{
	if (GetSignalongProtocol_Impl() == config.SignalingProtocol)
		return CreateParser_Impl(strand, logger);
	if (m_next)
		return m_next->CreateParser(strand, config, logger);
	return nullptr;
}

bool VS_Indentifier::AsyncResolve(std::function<void()>&& resolveTask) const
{
	if (!AsyncResolveImpl(resolveTask) && m_next)
		return m_next->AsyncResolve(std::move(resolveTask));
	return false;
}

void VS_Indentifier::LoadConfigurations_Impl_Base(const char* keyName, std::vector<VS_CallConfig> &users, std::vector<VS_CallConfig> &hosts, const char *peerId)
{
	if (!keyName||!*keyName)
		return ;
	VS_RegistryKey peers(false, keyName );
	if (!peers.IsValid())
		return;

	peers.ResetKey();

	class KeyNext final
	{
	public:
		KeyNext(VS_RegistryKey &peers, const char *peerId) :
			peers_(peers), peerId_(peerId), counter_(true) {}
		bool Next(VS_RegistryKey &key)
		{
			if (!peerId_)
			{
				return peers_.NextKey(key);
			}

			if (peerId_ && counter_)
			{
				key = VS_RegistryKey{ false, peers_.GetName() + std::string{ "\\" } +peerId_ };
				counter_ = false;
				return key.IsValid();
			}
			return false;
		}
	private:
		VS_RegistryKey &peers_;
		const char *peerId_;
		bool counter_;
	};

	KeyNext key_next{ peers, peerId };

	VS_CallConfig res_cfg;
	bool res = LoadConfigForomKey(res_cfg, peers, nullptr, BASE_PEERS_CONFIGURATION_TAG);
	assert(res);
	hosts.push_back(std::move(res_cfg));

	const std::size_t base_h_config_index = hosts.size() - 1;

	LoadVoipProtocolConfiguration();

	VS_RegistryKey key{};

	while (key_next.Next(key))
	{
		const char *configurationName = key.GetName();

		vs::map<std::string, std::string, vs::str_less> sip_to_tc;

		key.ResetValues();
		std::string name_buf;
		std::unique_ptr<char, free_deleter> value_buf;

		const char tc_to_sip[] = "TCtoSIP_";

		while( key.NextValue(value_buf, VS_REG_STRING_VT, name_buf) )
		{
			if (boost::istarts_with(name_buf, tc_to_sip))
			{
				auto it = sip_to_tc.find(string_view{ tc_to_sip });
				if(it == sip_to_tc.cend())
				{
					sip_to_tc.emplace(std::string(tc_to_sip), static_cast<const char *>(VS_RealUserLogin(name_buf.c_str() + (sizeof(tc_to_sip) - 1))));
				}
				else
				{
					it->second = std::string(static_cast<const char *>(VS_RealUserLogin(name_buf.c_str() + (sizeof(tc_to_sip) - 1))));
				}
			}
		}
		res_cfg = {}; //reset
		res = LoadConfigForomKey(res_cfg, key, configurationName, nullptr, hosts[base_h_config_index].DefaultProxyConfigurationName.c_str());
		(void)res;
		assert(res);
		hosts.push_back(std::move(res_cfg));

		const std::size_t h_config_index = hosts.size() - 1;

		VS_RegistryKey subkey;

		key.ResetKey();
		res_cfg = {}; //reset
		while ( key.NextKey( subkey ) )
		{
			if (LoadConfigForomKey(res_cfg, subkey, configurationName, hosts[h_config_index].HostName.c_str(), hosts[base_h_config_index].DefaultProxyConfigurationName.c_str()))
			{
				res_cfg.TcId = sip_to_tc[ subkey.GetName() ];
				users.push_back(std::move(res_cfg));
				res_cfg = {}; //reset
			}
		}
	}
}

class RegistryCallConfigValueReader :
	public VS_CallConfig::ValueReaderInterface
{
public:
	explicit RegistryCallConfigValueReader(const VS_RegistryKey &key) :
		m_key(key)
	{}

	bool ReadBool(const char *name, bool &val) override
	{
		int32_t reg_integer = 0;
		if (m_key.GetValue(&reg_integer, sizeof(reg_integer), VS_REG_INTEGER_VT, name))
		{
			val = reg_integer != 0;
			return true;
		}
		return false;
	}

	bool ReadInteger(const char *name, int32_t &val) override
	{
		int32_t reg_integer = 0;
		if (m_key.GetValue(&reg_integer, sizeof(reg_integer), VS_REG_INTEGER_VT, name))
		{
			val = reg_integer;
			return true;
		}
		return false;
	}

	bool ReadString(const char *name, std::string &val, bool canBeEmpty = false) override
	{
		std::string reg_string;
		if (m_key.GetString(reg_string, name) && (canBeEmpty || !reg_string.empty()))
		{
			val = std::move(reg_string);
			return true;
		}
		return false;
	}

	bool ReadProtocolSeq(const char *name, std::vector<net::protocol> &seq) override
	{
		std::string value;
		if (m_key.GetString(value, name) && !value.empty())
		{
			return DefaultCallManager::ParseProtocolSeqString(value.c_str(), seq);
		}
		return false;
	}

private:
	const VS_RegistryKey &m_key;
};

bool VS_Indentifier::LoadConfigForomKey(VS_CallConfig &cfg, const VS_RegistryKey &key, const char *configName, const char *hostname, const char *defConfigName)
{
	cfg.SignalingProtocol = GetSignalongProtocol_Impl();

	if (!m_voipGatewayParamName.empty() && configName && strcasecmp(configName, m_voipGatewayParamName.c_str()) == 0) cfg.UseAsTel = true;
	if (defConfigName && *defConfigName && configName && strcasecmp(configName, defConfigName) == 0) cfg.UseAsDefault = true;

	RegistryCallConfigValueReader reader(key);
	auto &&call_manager = create_call_config_manager(cfg);
	call_manager.LoadValues(reader);

	// Set hostname in the case it was not set yet.
	if (cfg.HostName.empty())
	{
		if (hostname && *hostname)
		{
			cfg.HostName = hostname;
		}
		else
		{
            if (cfg.Login.empty() && cfg.Password.empty()) {
                // All fields are empty
				call_manager.SetVerificationResult(VS_CallConfig::e_Unknown);
            } else {
				call_manager.SetVerificationResult(VS_CallConfig::e_CanNotCheck);
            }
			return false;
		}
	}

	assert(cfg.HostName == BASE_PEERS_CONFIGURATION_TAG ? configName == nullptr : true);

	if (cfg.HostName != BASE_PEERS_CONFIGURATION_TAG)
	{
		if (configName && configName[0] != '\0')
		{
			cfg.RegistryConfigName = configName;
		}

		cfg.Address.addr = net::dns_tools::single_make_a_aaaa_lookup(cfg.HostName);
	}

	return true;
}

void  VS_Indentifier::LoadVoipProtocolConfiguration(){
	char buffer[128];
	VS_RegistryKey key_conf(false, CONFIGURATION_KEY);

	m_voipProtocol = VS_CallConfig::SIP;
	if (key_conf.IsValid() && key_conf.GetValue(buffer, 128, VS_REG_STRING_VT, VOIP_GATEWAY_TAG))
	{
		string_view v(buffer);
		size_t pos;
		if (buffer[0] == '\0') {
			m_voipGatewayParamName = "";
			return;
		}
		if ((pos = v.find_first_of('\\')) == string_view::npos) return;	// use default = #tel

		const char *paramName = buffer + pos + 1;
		if (!paramName || !*paramName) return;
		m_voipGatewayParamName = paramName;

		if (boost::istarts_with(buffer, "h323"))
		{
			m_voipProtocol = VS_CallConfig::H323;
		}
	}
}

void VS_Indentifier::ReplaceTelephonePrefix(VS_CallConfig &config){
	if (config.TelephonePrefixReplace.empty()) config.TelephonePrefixReplace = DEFAULT_TELEPHONE_PREFIX;
	auto prefix = VS_GetTelephonePrefix();
	static auto tel = std::string(TEL_CALL_ID_PREFIX);
	if (boost::istarts_with(config.resolveResult.NewCallId, prefix))			// +
		config.resolveResult.NewCallId = tel + config.resolveResult.NewCallId;
	if (boost::istarts_with(config.resolveResult.NewCallId, tel + prefix))		// #tel:+
	{
		config.resolveResult.NewCallId.replace(tel.length(),
			prefix.length(), config.TelephonePrefixReplace);
	}
}

bool VS_Indentifier::IsVisualSeparator(int ch) noexcept {
	return strchr("().-/{}[] ", ch) != nullptr;
}

VS_CallConfig::eSignalingProtocol  VS_Indentifier::GetVoipProtocol() const{
	return m_voipProtocol;
}

void VS_Indentifier::SetVoipProtocol(const VS_CallConfig::eSignalingProtocol voip_proto)
{
	m_voipProtocol = voip_proto;
}

bool VS_Indentifier::ResolveThroughDNS(const std::string &host, net::port /*port*/, const std::vector<net::protocol> &/*desiredProtos*/, net::address &setAddr, net::port &/*setPort*/, bool /*block*/) const
{
	if (host.empty())
		return false;

	setAddr = net::dns_tools::single_make_a_aaaa_lookup(host);

	return !setAddr.is_unspecified();
}

boost::shared_ptr<VS_Indentifier> VS_Indentifier::GetNext() const{
	return m_next;
}
