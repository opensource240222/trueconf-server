#include "VS_ParserInterface.h"
#include "../CallConfig/VS_CallConfigStorage.h"
#include "../CallConfig/VS_Indentifier.h"
#include "../interfaces/VS_ParserInfo.h"
#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

const double VS_ParserInterface::REGISTER_EXPIRES_PART = 0.7;

VS_ParserInterface::VS_ParserInterface()
	: m_myCsEp{{}, 0, net::protocol::none}
{}


/*void VS_ParserInterface::SetMyCsAddress(unsigned long ip, unsigned short port, eConnectionType conn_type)
{
	m_my_cs_ipport.ipv4(ip);
	m_my_cs_ipport.port(port);
	m_my_cs_conn_type = conn_type;
}

void VS_ParserInterface::SetMyCsAddress(in6_addr ip6, unsigned short port, eConnectionType conn_type)
{
	m_my_cs_ipport.ipv6(ip6);
	m_my_cs_ipport.port(port);
	m_my_cs_conn_type = conn_type;
}*/

void VS_ParserInterface::SetMyMediaAddress(const net::address &addr)
{
	m_my_media_ip = addr;
}

void VS_ParserInterface::SetMyCsAddress(const net::Endpoint& ep)
{
	m_myCsEp = ep;
}


void VS_ParserInterface::SetConfCallBack(const std::shared_ptr<VS_ConferenceProtocolInterface>& confMethods)
{
	m_confMethods = confMethods;
}

VS_ChannelID VS_ParserInterface::GetChannelID(const void *buf, std::size_t sz, bool& is_fragmented)
{
	is_fragmented = false;
	return GetDefaultChannelID();
}

void VS_ParserInterface::SetPolicy( const boost::shared_ptr<VS_Policy> &policy )
{
	m_policy = policy;
}

void VS_ParserInterface::SetCallConfigStorage( std::shared_ptr<VS_CallConfigStorage> storage )
{
	m_CallConfigStorage = std::move(storage);
}

VS_CallConfig VS_ParserInterface::CreateCallConfig(const char *remoteIp, string_view login, const std::string &userAgent)
{
	assert(remoteIp);

	boost::system::error_code ec;
	net::Endpoint remoteEp;
	remoteEp.addr = net::address::from_string(remoteIp, ec);

	return CreateCallConfig(remoteEp, login, userAgent);
}

VS_CallConfig VS_ParserInterface::CreateCallConfig(const net::Endpoint &remoteEp,
	string_view login, const std::string& userAgent)
{
	return m_CallConfigStorage->GetConfiguration(remoteEp, MySignallingProtocol(), std::string(login), userAgent);
}

void VS_ParserInterface::CheckPermanentRegistrations()
{
	std::lock_guard<decltype(m_reg_configs_lock)> _{ m_reg_configs_lock };
	std::chrono::steady_clock::time_point curretTime = m_clock.now();

	// recheck every 5 seconds
	//if m_my_cs_ip == 0 connection is not established, but m_my_cs_ip should be known before MakeREGISTER

	if (curretTime.time_since_epoch() < m_last_register_time.time_since_epoch() + std::chrono::milliseconds(5000)/* || m_my_cs_ipport.isZero()*/) return;
	m_last_register_time = curretTime;

	for (auto it = m_reg_configs.begin(); it != m_reg_configs.cend();)
	{
		bool has_next = true;

		auto call_manager = create_call_config_manager(it->second.callConfig);
		auto dialog_id = GetRegDialogIdByRegIdentDialogId({ it->second.callConfig.RegistryConfigName,
			call_manager.GetConfigIdentificator() }).get_value_or({});

		auto ctx = GetParserContextBase(dialog_id);

		if (call_manager.NeedPermanentConnection())
		{
			std::chrono::seconds expires(0);
			if (!ctx)
				ctx = GetParserContextBase(dialog_id, true);

			if (ctx)
			{
				expires = ctx->GetExpires();
				// this code invokes every 5 seconds
				// + we have some value of net ping
				// so we have to make registration earlier, otherwise
				// RRQ can be received by GK too late.
				expires = std::chrono::duration_cast<std::chrono::seconds>(expires * REGISTER_EXPIRES_PART);
				// Minimum value. To prevent potential infinity cycle.
				const std::chrono::seconds min_expires(5);
				if (expires <= min_expires) expires = min_expires; /* seconds */;
			}
			if (curretTime >= it->second.lastTime + expires)
			{
				if (call_manager.NeedVerification() && call_manager.GetLastVerificationResult() != VS_CallConfig::e_Pending)
				{
					call_manager.SetPendingVerification();
				}
				UpdateRegisterContext(it->second.callConfig);

				it->second.lastTime = curretTime;
			}
		}
		else
		{
			// registration was disabled, unregister...
			if (ctx)
			{
				if (ctx->GetExpires() > std::chrono::seconds(0))
				{
					ctx->SetExpires(std::chrono::seconds(0));
					DoRegister(dialog_id);
					{
						std::lock_guard<decltype(m_register_dialog_id_lock)> lock(m_register_dialog_id_lock);
						const auto erase_it = m_register_dialog_id.find(
							RegistrationIdentDialogIdView(it->second.callConfig.RegistryConfigName, it->second.callConfig.Login));
						if (erase_it != m_register_dialog_id.cend())
						{
							m_register_dialog_id.erase(erase_it);
						}
					}
				}
			}

			if (!ctx)
			{
				it = m_reg_configs.erase(it);
				has_next = false;
			}
		}

		if (has_next)
		{
			++it;
		}

	}
}

boost::optional<std::string> VS_ParserInterface::GetRegDialogIdByRegIdentDialogId(RegistrationIdentDialogIdView regIdentView)
{
	std::lock_guard<decltype(m_register_dialog_id_lock)> lock(m_register_dialog_id_lock);
	const auto res = m_register_dialog_id.find(regIdentView);
	if (res != m_register_dialog_id.cend())
	{
		return boost::optional<std::string>{ res->second };
	}
	return boost::none; //empty optional
}

bool VS_ParserInterface::NeedPermanentConnection() const
{
	std::lock_guard<decltype(m_reg_configs_lock)> _{ m_reg_configs_lock };
	return !m_reg_configs.empty();
}

void VS_ParserInterface::SetRegistrationConfiguration(VS_CallConfig config, const std::function<void(VS_CallConfig &)> &failureHandler)
{
	{
		auto &&config_manager = create_call_config_manager(config);

		std::unique_lock<mutex_t> lock{ m_reg_configs_lock };
		auto find_it = m_reg_configs.find(config_manager.GetRegistrationIdentifierView());

		bool find_cfg = find_it != m_reg_configs.cend();
		if (!find_cfg)
		{
			for (auto it = m_reg_configs.begin(); it != m_reg_configs.cend(); ++it)
			{
				if (it->first.registryConfigName == config.RegistryConfigName)
				{
					it->second.callConfig.IsValid = false;
					find_cfg = true;
				}
			}
			if (find_cfg)
			{
				lock.unlock();
				if(failureHandler) failureHandler(config);
				return;
			}
		}

		SetRegistrationConfigurationImpl(config);

		if (find_cfg)
		{
			find_it->second = RegistrartionConfig{ config, std::chrono::steady_clock::time_point() };
		}
		else
		{
			auto reg_ident = config_manager.GetRegistrationIdentifier();
			const auto &&res = m_reg_configs.emplace(std::move(reg_ident), RegistrartionConfig{ config, std::chrono::steady_clock::time_point() });
			assert(res.second);
			find_it = res.first;
		}
		m_last_register_time = std::chrono::steady_clock::time_point();
	}
	CheckPermanentRegistrations();
}

bool VS_ParserInterface::UpdateRegistrationConfig(VS_CallConfig::RegistrationIdentifierView regIdentifier,
                                                  const std::function<void(RegistrartionConfig&)>& update, bool erase)
{
	std::lock_guard<decltype(m_reg_configs_lock)> _{ m_reg_configs_lock };
	auto res = m_reg_configs.find(regIdentifier);
	if (res != m_reg_configs.cend() && res->second.callConfig.IsValid)
	{
		if(update) update(res->second);
		if(erase) m_reg_configs.erase(res);
		return true;
	}
	return false;
}

void VS_ParserInterface::ResetAllConfigsStatus()
{
	std::lock_guard<decltype(m_reg_configs_lock)> _{ m_reg_configs_lock };
	for(auto &item : m_reg_configs)
	{
		item.second.callConfig.IsValid = false;
	}
	m_last_register_time = std::chrono::steady_clock::time_point();
}

VS_ParserInterface::collection_list_t<boost::optional<std::string>> VS_ParserInterface::GetRegDialogIdsByUsername(string_view name)
{
	collection_list_t<boost::optional<std::string>> res_dialog_ids;
	std::lock_guard<decltype(m_register_dialog_id_lock)> lock(m_register_dialog_id_lock);

	for(auto &item : m_register_dialog_id)
	{
		if(name == item.first.userName)
		{
			res_dialog_ids.emplace_back(item.second);
		}
	}
	return res_dialog_ids;
}

bool VS_ParserInterface::DoRegister(string_view dialogId, const bool updateData)
{
	if (updateData)
	{
		auto ctx = GetParserContextBase(dialogId, false);
		if (!ctx) return false;
		auto&& config = ctx->GetConfig();
		auto&& config_manager = create_call_config_manager(config);
		std::string&& user = config_manager.GetUser();
		ctx->SetSIPRemoteTarget(config.HostName);
		// The "userinfo" and "@" components of the SIP URI must not be present in the Request-Line for REGISTER requests
		ctx->SetAliasMy(user);
		ctx->SetAliasRemote(std::move(user));
		ctx->SetUser(config.Login);
		ctx->SetDomain(config.HostName);
		ctx->SetPassword(config.Password);
	}
	return true;
}

boost::optional<std::string> VS_ParserInterface::UpdateRegisterContext(const VS_CallConfig &config, bool force)
{
	auto config_manager = create_call_config_manager(config);
	if(!config_manager.IsValidRegistrationData())
	{
		return  boost::none;
	}

	if(!force && !config_manager.NeedPermanentConnection())
	{
		return  boost::none;
	}
	const auto ident = m_CallConfigStorage->GetIdentifier(config.SignalingProtocol);
	if (!ident || (config.UseAsTel && !config_manager.IsVoip(ident->GetVoipProtocol())))
	{
		return  boost::none;
	}

	const auto config_identificator = config_manager.GetConfigIdentificator();
	boost::optional<std::string> reg_dialog_id = GetRegDialogIdByRegIdentDialogId({ config.RegistryConfigName, config_identificator });

	if (reg_dialog_id.is_initialized() )
	{
		auto info = GetParserContextBase(*reg_dialog_id);
		if (!info)
		{
			return boost::none;
		}
		info->SetConfig(config);
		//info->SetExpires( 300 );

		if (info->GetExpires() != std::chrono::seconds(0))
		{
			DoRegister( *reg_dialog_id, true);
			return reg_dialog_id;
        }
		CleanParserContext(*reg_dialog_id, SourceClean::PARSER);
	}

	auto &&new_reg_dialog_id = NewDialogID(config_identificator, {}, config);

	auto ctx = GetParserContextBase(new_reg_dialog_id);
	if(!ctx)
		return boost::none;
	{
		std::lock_guard<decltype(m_register_dialog_id_lock)> lock(m_register_dialog_id_lock);
		m_register_dialog_id.emplace(RegistrationIdentDialogId{ config.RegistryConfigName, config_identificator }, 	new_reg_dialog_id);
	}

	DoRegister(new_reg_dialog_id, true);
	return boost::optional<std::string>(new_reg_dialog_id);
}

VS_ParserInterface::RegistrationIdentDialogIdView::RegistrationIdentDialogIdView(string_view aRegistryConfigName,
                                                                                 string_view aUserName):
	registryConfigName(aRegistryConfigName), userName(aUserName)
{
}

VS_ParserInterface::RegistrationIdentDialogIdView::
RegistrationIdentDialogIdView(const RegistrationIdentDialogId& ident): registryConfigName(ident.registryConfigName),
                                                                       userName(ident.userName)
{
}

void VS_ParserInterface::SetRegistrationConfigurationImpl(const VS_CallConfig& config)
{
}

void VS_ParserInterface::RemovePermanentRegistrations(const bool invalidAllCfgs)
{
	std::lock_guard<decltype(m_reg_configs_lock)> _( m_reg_configs_lock );
	for(auto &&item : m_reg_configs)
	{
		if (invalidAllCfgs || item.second.callConfig.IsValid)
		{
			auto &&call_config_manager = create_call_config_manager(item.second.callConfig);
			call_config_manager.SetVerificationResult(VS_CallConfig::e_Unknown);
			if (invalidAllCfgs)
			{
				item.second.callConfig.IsValid = false;
			}
		}
	}
}

bool VS_ParserInterface::CheckUserNameInCallConfigStorage(const VS_CallConfig::eSignalingProtocol protocol, string_view remoteUsername) const
{
	return m_CallConfigStorage->CheckUserName(protocol, remoteUsername);
}

std::shared_ptr<VS_ParserInfo> VS_ParserInterface::GetParserContextByDialogID(const std::shared_ptr<VS_ParserInterface> &parser, string_view dialogId)
{
	return parser->GetParserContextBase(dialogId, false);
}

#undef DEBUG_CURRENT_MODULE