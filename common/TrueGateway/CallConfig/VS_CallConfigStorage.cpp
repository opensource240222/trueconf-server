#include "VS_CallConfigStorage.h"
#include "VS_Indentifier.h"
#include "../../SIPParserBase/VS_Const.h"
#include "std/cpplib/VS_ExternalAccount.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std-generic/cpplib/StrCompare.h"
#include "VS_CallConfigCorrector.h"

#include "std-generic/compat/functional.h"
#include "std-generic/compat/map.h"
#include "net/DNSUtils/VS_DNSTools.h"
#include "tools/Server/CommonTypes.h"

namespace
{
	constexpr int CACHE_SIZE_LIMIT = 100;

	struct DataResolve final
	{
		net::address addr;
		net::port	 port;
		std::vector<net::protocol> types;
	};

	typedef vs::map<std::string /*hostname*/, DataResolve, vs::str_less> to_resolve_t;

	inline void FetchDomaintsToResolve(std::vector<VS_CallConfig> &configs, to_resolve_t &outToResolve) {
		for (auto pConfig : configs)
		{
			if (pConfig.HostName.empty() || pConfig.HostName == BASE_PEERS_CONFIGURATION_TAG)
				continue;

			boost::system::error_code ec;
			pConfig.Address.addr = net::address::from_string(pConfig.HostName, ec);

			if (ec) // error
				outToResolve.emplace(pConfig.HostName, DataResolve{ pConfig.Address.addr, pConfig.Address.port, pConfig.ConnectionTypeSeq });
		}
	}

}

VS_CallConfigStorage::~VS_CallConfigStorage(void)
{
	Clear( true );
}

void VS_CallConfigStorage::RegisterProtocol(const boost::shared_ptr<VS_Indentifier> &ident)
{
	m_indetifier = ident;
}

bool VS_CallConfigStorage::Resolve(VS_CallConfig &cfg, string_view callId, VS_UserData *fromUser )
{
	if (!m_indetifier->Resolve(cfg, callId, fromUser)) return false;
	cfg = MergeWithHostAndUserConfiguration(cfg, *m_reg_configs.lock());
	return m_indetifier->PostResolve(cfg, callId, fromUser, true);
}

VS_CallConfig VS_CallConfigStorage::GetConfiguration(const net::Endpoint &ep,
                                                     VS_CallConfig::eSignalingProtocol protocol,
                                                     string_view remoteUsername, const std::string &userAgent)
{
	VS_CallConfig cfg;
	const bool res = m_indetifier->CreateDefaultConfiguration(cfg, ep, protocol, remoteUsername );
	assert( res );
	cfg = MergeWithHostAndUserConfiguration(cfg, *m_reg_configs.lock(), true, userAgent);

	if(cfg.HostName.empty())
		cfg.HostName = cfg.Address.addr.to_string(); //throw boost::system::system_error
	return cfg;
}

bool VS_CallConfigStorage::CheckUserName(const VS_CallConfig::eSignalingProtocol protocol, string_view remoteUsername)
{
	bool found = false;
	{
		auto pLockedConfigs = m_reg_configs.lock();
		for (auto &v : pLockedConfigs->m_hosts)
		{
			if (!v.Login.empty() && v.Login == remoteUsername &&
				v.SignalingProtocol == protocol)
			{
				found = true;
				break;
			}
			// check for dialedDigits
			if ((protocol == VS_CallConfig::eSignalingProtocol::H225RAS ||
				protocol == VS_CallConfig::eSignalingProtocol::H323) &&
				v.SignalingProtocol == protocol &&
				!v.h323.DialedDigit.empty() && v.h323.DialedDigit == remoteUsername)
			{
				found = true;
				break;
			}
		}
	}
	return found;
}

VS_CallConfig& VS_CallConfigStorage::MergeWithHostAndUserConfiguration(VS_CallConfig &base, RegConfigs& configs, bool cache, const std::string &userAgent)
{
	auto &&call_manager = create_call_config_manager(base);

	{auto pLckCache = m_cache.lock();
	std::size_t cache_index = -1;
	if (cache) cache_index = GetConfigIndex( *pLckCache, base, false );

	if (cache_index != std::size_t(-1))
	{
		call_manager.MergeWith((*pLckCache)[cache_index]);
		(*pLckCache)[cache_index] = base;
		return base;
	}

	const std::size_t user_config_index = GetConfigIndex(configs.m_users, base, false);
	const std::size_t host_config_index = GetConfigIndex(configs.m_hosts, base, true);
 	const std::size_t root_config_index = GetRootConfigIndex(configs.m_hosts, base);


	if (root_config_index != std::size_t(-1))
	{
		auto root_config_copy = configs.m_hosts[root_config_index];
		root_config_copy.HostName = {}; // ignore #base tag
		call_manager.MergeWith(root_config_copy);
	}

	// make corrections for dummy terminals
	VS_CallConfigCorrector::GetInstance().CorrectCallConfig(base, base.SignalingProtocol, userAgent.c_str());

	if (host_config_index != std::size_t(-1)) call_manager.MergeWith(configs.m_hosts[host_config_index]);
	if (user_config_index != std::size_t(-1)) call_manager.MergeWith(configs.m_users[user_config_index]);
	}

	if (!base.Bandwidth.get_value_or(0))
		base.Bandwidth = video_presets::default_gateway_bandwidth;

	if (cache)
	{
		auto pLckCache = m_cache.lock();
		if (pLckCache->size() >= CACHE_SIZE_LIMIT) pLckCache->resize( CACHE_SIZE_LIMIT * 2 / 3 );
		pLckCache->push_back( base );
	}

	return base;
}

template <class T>
std::size_t VS_CallConfigStorage::GetConfigIndex(std::vector<T> &data, VS_CallConfig &baseConf, bool ignoreUsername )
{
	// do not allow both TcId and Login be empty for user-dependent settings
	if (!ignoreUsername && baseConf.TcId.empty() && baseConf.Login.empty()) return std::size_t(-1);

	for (std::size_t i = 0; i < data.size(); i++)
	{
		VS_CallConfig &c = data[i];
		auto &&cfg_manager = create_call_config_manager(c);

		const bool endointEqual = cfg_manager.TestEndpointsEqual(baseConf);
		if ( !endointEqual ) continue;

		if (!ignoreUsername)
		{
			const bool tc_equal = (baseConf.TcId.empty() || data[i].TcId == baseConf.TcId);
			if (!tc_equal) continue;

			const bool user_equal = (baseConf.Login.empty() || data[i].Login == baseConf.Login);
			if (!user_equal) continue;
		}

		if (c.HostName.empty()) c.HostName = baseConf.HostName;
		if (c.Address.addr.is_unspecified()) c.Address.addr = baseConf.Address.addr;

		return i;
	}
	return std::size_t(-1);
}

std::size_t VS_CallConfigStorage::GetRootConfigIndex(const std::vector<VS_CallConfig> &data, const VS_CallConfig &conf) const
{
	for (size_t i = 0; i < data.size(); ++i)
	{
		const VS_CallConfig &c = data[i];

		const bool sameSignalingProtocol = c.SignalingProtocol == conf.SignalingProtocol ||
			(c.SignalingProtocol == VS_CallConfig::H225RAS && conf.SignalingProtocol == VS_CallConfig::H323) ||
			(c.SignalingProtocol == VS_CallConfig::H323 && conf.SignalingProtocol == VS_CallConfig::H225RAS);

		if (sameSignalingProtocol && c.HostName == BASE_PEERS_CONFIGURATION_TAG)
			return i;
	}

	return std::size_t(-1);
}

void VS_CallConfigStorage::Clear(  bool removeAll )
{
	{
		auto pLockedConfigs = m_reg_configs.lock();
		auto pLckCache = m_cache.lock();
		for (auto &&it = pLockedConfigs->m_users.cbegin(); it != pLockedConfigs->m_users.cend();)
		{
			if (removeAll || it->IsFromRegistry)
			{
				it = pLockedConfigs->m_users.erase(it);
			}
			else
			{
				it = std::next(it);
			}
		}
		pLockedConfigs->m_hosts.clear();
		pLckCache->clear();
	}
}


void VS_CallConfigStorage::ResolveHostnames() {

	to_resolve_t to_resolve;
	{
		auto pLockedConfigs = m_reg_configs.lock();
		FetchDomaintsToResolve(pLockedConfigs->m_users, to_resolve);
		FetchDomaintsToResolve(pLockedConfigs->m_hosts, to_resolve);
	}

	// make dns resolve when m_user and m_hosts unlocked
	for (auto& it : to_resolve)
	{
		auto& info_for_resolve = it.second;
		m_indetifier->ResolveThroughDNS(it.first, info_for_resolve.port, info_for_resolve.types, info_for_resolve.addr, info_for_resolve.port, false);
	}

	{
	auto pLockedConfigs = m_reg_configs.lock();
	for (auto& resolved : to_resolve)
	{
		for (auto& pConfig : pLockedConfigs->m_hosts)
		{
			if (pConfig.HostName != resolved.first) continue;
			pConfig.Address.addr = resolved.second.addr;
			pConfig.Address.port = resolved.second.port;
		}
		for (auto& pConfig : pLockedConfigs->m_users)
		{
			if (pConfig.HostName != resolved.first) continue;
			pConfig.Address.addr = resolved.second.addr;
			pConfig.Address.port = resolved.second.port;
		}
	}
	}
}

VS_CallConfigStorage::update_result_t VS_CallConfigStorage::UpdateSettings()
{
	return UpdateSettings(nullptr);
}

VS_CallConfigStorage::update_result_t VS_CallConfigStorage::UpdateSettingByPeerId(const char* peerId)
{
	assert(peerId);
	return UpdateSettings(peerId);
}

void VS_CallConfigStorage::AddUser(string_view tcId, const std::vector<VS_ExternalAccount> &accounts)
{
	{
		auto pLockedConfigs = m_reg_configs.lock();
		for (std::size_t i = 0; i < accounts.size(); i++)
		{
			VS_CallConfig c;
			if (m_indetifier->ConvertConfiguration(c, tcId, accounts[i]))
			{
				c.IsFromRegistry = false;

				const int index = GetConfigIndex(pLockedConfigs->m_users, c, false);
				if (index == -1)
				{
					pLockedConfigs->m_users.push_back(std::move(c));
				}
				else
				{
					pLockedConfigs->m_users[index] = std::move(c);
				}
			}
		}
	}
	UdpateRegistrationSettings();
}

void VS_CallConfigStorage::RemoveUser(string_view tcId)
{
	{
		auto pLockedConfigs = m_reg_configs.lock();
		pLockedConfigs->m_users.erase(std::remove_if(pLockedConfigs->m_users.begin(), pLockedConfigs->m_users.end(), [tcId](const VS_CallConfig &cfg)
		{
			return cfg.TcId == tcId;
		}), pLockedConfigs->m_users.end());
	}
	UdpateRegistrationSettings();
}

VS_CallConfigStorage::update_result_t VS_CallConfigStorage::UpdateSettings(const char* peerId)
{
	auto sp = boost::make_shared<std::promise<void>>();
	Clear(false);
	{
		auto pLockedConfigs = m_reg_configs.lock();
		m_indetifier->LoadConfigurations(pLockedConfigs->m_users, pLockedConfigs->m_hosts, peerId);
	}
	m_indetifier->AsyncResolve([self = shared_from_this(), sp]() {
		self->ResolveHostnames();
		self->UdpateRegistrationSettings();
		sp->set_value();
	});
	return sp->get_future();
}

void VS_CallConfigStorage::UdpateRegistrationSettings()
{
	std::vector<VS_CallConfig> cfgs;
	{
	auto pLockedConfigs = m_reg_configs.lock();
	for (std::size_t i = 0; i < pLockedConfigs->m_users.size(); i++)
	{
		if (create_call_config_manager(pLockedConfigs->m_users[ i ]).NeedPermanentConnection() )
		{
			if (pLockedConfigs->m_users[ i ].Login.empty() || pLockedConfigs->m_users[ i ].Password.empty()) continue;

			VS_CallConfig c = pLockedConfigs->m_users[i];
			c = MergeWithHostAndUserConfiguration(c,*pLockedConfigs, false);

			// fill defaults
			if (!m_indetifier->PostResolve(c, {}, nullptr, false)) continue;
			cfgs.push_back(std::move(c));
		}
	}

	for (std::size_t i = 0; i < pLockedConfigs->m_hosts.size(); i++)
	{
		if (create_call_config_manager(pLockedConfigs->m_hosts[ i ]).NeedPermanentConnection())
		{
			// For h323 registration without password and login.
			/* if (m_hosts[ i ]->Login.empty() || m_hosts[ i ]->Password.empty()) continue; */

			VS_CallConfig c = pLockedConfigs->m_hosts[i];
			if (!m_indetifier->PostResolve(c, {}, nullptr, false)) continue;
			cfgs.push_back(std::move(c));
		}
	}

	VS_CallConfig dummy;
	std::size_t pos;
	dummy.SignalingProtocol = VS_CallConfig::SIP;
	if ((pos = GetRootConfigIndex(pLockedConfigs->m_hosts, dummy)) != std::size_t(-1))
	{
		if (pLockedConfigs->m_hosts[pos].Address.protocol != net::protocol::any)
		{
			const auto protocol = pLockedConfigs->m_hosts[pos].Address.protocol;
			for (auto &cfg : cfgs)
			{
				if (cfg.SignalingProtocol == VS_CallConfig::SIP && cfg.Address.protocol == net::protocol::any)
				{
					cfg.Address.protocol = protocol;
				}
			}
		}
	}
	dummy.SignalingProtocol = VS_CallConfig::H323;
	if ((pos = GetRootConfigIndex(pLockedConfigs->m_hosts, dummy)) != std::size_t(-1))
	{
		if (pLockedConfigs->m_hosts[pos].Address.protocol != net::protocol::any)
		{
			const auto protocol = pLockedConfigs->m_hosts[pos].Address.protocol;
			for (auto &cfg : cfgs)
			{
				if ((cfg.SignalingProtocol == VS_CallConfig::H323 || cfg.SignalingProtocol == VS_CallConfig::H225RAS) &&
					cfg.Address.protocol == net::protocol::any)
				{
					cfg.Address.protocol = protocol;
				}
			}
		}
	}
	}

	//	std::vector< boost::shared_ptr<VS_CallConfig> > _result;
	{
		auto locked_registration = m_registration.lock();

		for (std::size_t i = 0; i < locked_registration->size();)
		{
			VS_CallConfig &current = (*locked_registration)[i];
			bool found = false;
			for (std::size_t j = 0; !found && j < cfgs.size(); j++)
			{
				found = create_call_config_manager(current).TestEndpointsEqual(cfgs[j]) && current.Login == cfgs[j].Login
					&& current.Password == cfgs[j].Password && current.sip.AuthName == cfgs[j].sip.AuthName &&
					current.sip.RegistrationBehavior.get_value_or(VS_CallConfig::REG_UNDEFINED) ==
					cfgs[j].sip.RegistrationBehavior.get_value_or(VS_CallConfig::REG_UNDEFINED) &&
					current.UseAsTel == cfgs[j].UseAsTel &&
					current.UseAsDefault == cfgs[j].UseAsDefault && cfgs[j].Address.protocol == current.Address.protocol;

				if (found) cfgs.erase(cfgs.begin() + j);
			}
			if (!found)
			{
				current.IsValid = false;
				locked_registration->erase(locked_registration->begin() + i);
			}
			else i++;
		}

		if (locked_registration->empty())
		{
			*locked_registration = std::move(cfgs);
		}
		else
		{
			locked_registration->reserve(locked_registration->size() + cfgs.size());
			std::move(cfgs.begin(), cfgs.end(), std::back_inserter(*locked_registration));
		}
	}
}

boost::shared_ptr<VS_Indentifier> VS_CallConfigStorage::GetIdentifier(VS_CallConfig::eSignalingProtocol protocol) const
{
	auto ident = m_indetifier;
	while (ident){
		if (ident->GetSignalongProtocol_Impl() == protocol) return ident;
		ident = ident->GetNext();
	}
	return nullptr;
}
