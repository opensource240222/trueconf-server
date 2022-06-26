#include "VS_GatewayService.h"
#include "std-generic/clib/strcasecmp.h"
#include "std-generic/cpplib/VS_Container.h"
#include "interfaces/VS_ParserInterface.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/debuglog/VS_Debug.h"

#include "std-generic/compat/iterator.h"

#define DEBUG_CURRENT_MODULE VS_DM_MULTIGATEWAY

namespace
{
	const std::chrono::seconds WAIT_FOR_RESOLVE_DATA(2); //2sec
	const std::chrono::minutes WAIT_FOR_RELOAD_CONFIGS(5); //5min
	const std::chrono::seconds MIN_WAIT_FOR_RELOAD_CONFIGS(10); //10sec

	template<typename R>
	inline bool is_ready(const std::future<R> &f, const std::chrono::milliseconds time = std::chrono::milliseconds(600))
	{
		return f.wait_for(time) == std::future_status::ready;
	}

	//////////////////////////////////////////////////////////////////////////

	const char RELOAD_PEER_ID[] = "Id";
	const char RELOAD_COMMAND[] = "Command";
	const char RELOAD_PROTOCOL[] = "Protocol";

	bool extractor_update_config(VS_GatewayService::ReloadConfiguration &reload, const VS_Container &cnt)
	{
		//extract
		{
			const char *protocol_ptr;
			if ((protocol_ptr = cnt.GetStrValueRef(RELOAD_PROTOCOL)))
			{
				reload.protocol = protocol_ptr;
			}

			const char *peer_id_ptr;
			std::size_t len = 0;
			if ((peer_id_ptr = cnt.GetStrValueRef(RELOAD_PEER_ID)) != nullptr
				&& ((len = std::char_traits<char>::length(peer_id_ptr))))
			{
				reload.peerId = std::string{ peer_id_ptr, len };
			}

			const char *command_ptr;
			if ((command_ptr = cnt.GetStrValueRef(RELOAD_COMMAND)))
			{
				if (peer_id_ptr)
				{
					reload.execute = command_ptr;
				}
			}
			else
			{
				reload.execute = ::VS_GatewayService::ExecuteEvent::ALL;
			}
		}

		//validate
		{
			if (reload.protocol.GetValue() != ::VS_GatewayService::Protocol::UNDEFINED)
			{
				if (!reload.peerId.empty())
				{
					return reload.execute.GetValue() != ::VS_GatewayService::ExecuteEvent::UNDEFINED;
				}
				if (reload.execute.GetValue() == ::VS_GatewayService::ExecuteEvent::ALL)
				{
					return true;
				}
			}
		}
		return false;
	}
	//////////////////////////////////////////////////////////////////////////
	const char EMPTY_STR[] = "";
	//////////////////////////////////////////////////////////////////////////
	const char ALL[] = "all";
	const char NEW[] = "new";
	const char EDIT[] = "edit";
	const char DEL[] = "del";

	unsigned extract_val_execute_event(const unsigned value)
	{
		switch (value)
		{
		case ::VS_GatewayService::ExecuteEvent::ExecuteEvent::NEW:
		case ::VS_GatewayService::ExecuteEvent::ExecuteEvent::EDIT:
		case ::VS_GatewayService::ExecuteEvent::ExecuteEvent::DEL:
		case ::VS_GatewayService::ExecuteEvent::ExecuteEvent::ALL:
			return value;
		}
		return ::VS_GatewayService::ExecuteEvent::ExecuteEvent::UNDEFINED;
	}

	const char *extract_val_str_execute_event(const unsigned value)
	{
		switch (value)
		{
		case ::VS_GatewayService::ExecuteEvent::ExecuteEvent::NEW:  return NEW;
		case ::VS_GatewayService::ExecuteEvent::ExecuteEvent::EDIT: return EDIT;
		case ::VS_GatewayService::ExecuteEvent::ExecuteEvent::DEL:  return DEL;
		case ::VS_GatewayService::ExecuteEvent::ExecuteEvent::ALL:  return ALL;
		}
		return EMPTY_STR;
	}


	unsigned extract_val_execute_event(string_view valueStr)
	{
		if (valueStr == NEW)  return ::VS_GatewayService::ExecuteEvent::ExecuteEvent::NEW;
		if (valueStr == EDIT)  return ::VS_GatewayService::ExecuteEvent::ExecuteEvent::EDIT;
		if (valueStr == DEL)  return ::VS_GatewayService::ExecuteEvent::ExecuteEvent::DEL;
		return ::VS_GatewayService::ExecuteEvent::ExecuteEvent::UNDEFINED;
	}

	//////////////////////////////////////////////////////////////////////////

	const char SIP[] = "SIP";
	const char H323[] = "H323";

	const char *extract_val_str_protocol(const unsigned value)
	{
		switch (value)
		{
		case ::VS_GatewayService::Protocol::SIP: return SIP;
		case ::VS_GatewayService::Protocol::H323: return H323;
		}
		return EMPTY_STR;
	}

	unsigned extract_val_protocol(const unsigned value)
	{
		switch (value)
		{
		case ::VS_GatewayService::Protocol::SIP:
		case ::VS_GatewayService::Protocol::H323:
			return value;
		}
		return ::VS_GatewayService::Protocol::UNDEFINED;
	}

	unsigned extract_val_protocol(string_view valueStr)
	{
		if (valueStr == SIP)  return ::VS_GatewayService::Protocol::SIP;
		if (valueStr == H323) return ::VS_GatewayService::Protocol::H323;
		return ::VS_GatewayService::Protocol::UNDEFINED;
	}

	//////////////////////////////////////////////////////////////////////////
}

VS_GatewayService::EnumValue::EnumValue(const unsigned value, const char* strValue)
	:value_(value), valueStr_(strValue)
{
}

unsigned VS_GatewayService::EnumValue::GetValue() const
{
	return value_;
}

const char* VS_GatewayService::EnumValue::GetStrValue() const
{
	return valueStr_;
}

VS_GatewayService::ExecuteEvent::ExecuteEvent()
	: EnumValue(UNDEFINED, (EMPTY_STR))
{
}

::VS_GatewayService::ExecuteEvent::ExecuteEvent(const unsigned value)
	: EnumValue(extract_val_execute_event(value), (extract_val_str_execute_event(value)))
{
}

::VS_GatewayService::ExecuteEvent::ExecuteEvent(const char *str)
	: EnumValue(extract_val_execute_event(str), extract_val_str_execute_event(extract_val_execute_event(str)))
{
}

VS_GatewayService::Protocol::Protocol()
	: EnumValue(UNDEFINED, EMPTY_STR)
{
}

VS_GatewayService::Protocol::Protocol(const unsigned value)
	: EnumValue((extract_val_protocol(value)), (extract_val_str_protocol(value)))
{
}

VS_GatewayService::Protocol::Protocol(const char* str)
	: EnumValue(extract_val_protocol(str), extract_val_str_protocol(extract_val_protocol(str)))
{
}


VS_GatewayService::AbstractGatewayEventListener::AbstractGatewayEventListener(std::shared_ptr<VS_CallConfigStorage> &&peerConfig)
	: m_peer_config(std::move(peerConfig))
	, m_has_tel_cfg(false)
	, m_wait_reload_configs(WAIT_FOR_RELOAD_CONFIGS)
	, m_updated(false)
{
}

bool VS_GatewayService::AbstractGatewayEventListener::HasTelCfg() const
{
	return m_has_tel_cfg.load(std::memory_order_acquire);
}

void VS_GatewayService::AbstractGatewayEventListener::Start()
{
	VS_RegistryKey key{ false, GetPeerName(), false };
	if(key.IsValid())
	{
		int32_t time_wait_reload;
		if(key.GetValue(&time_wait_reload, sizeof(time_wait_reload), VS_REG_INTEGER_VT, TIME_WAIT_RELOAD) > 0)
		{
			const auto time = std::chrono::seconds(std::chrono::seconds::rep(time_wait_reload));
			if(MIN_WAIT_FOR_RELOAD_CONFIGS <= time)
			{
				m_wait_reload_configs = time;
			}
		}
	}

	const ReloadConfiguration reload_cfg{ ExecuteEvent::ALL, GetPeerName(), {}};
	HandleEvent(reload_cfg);
}

void VS_GatewayService::AbstractGatewayEventListener::HandleEvent(const ReloadConfiguration& reloadCfg)
{
	dprint4("VS_GatewayService::AbstractGatewayEventListener::HandleEvent(%s)\n", reloadCfg.execute.GetStrValue());
	switch (reloadCfg.execute.GetValue())
	{
	case ExecuteEvent::ALL:  ReloadAll(); break;
	case ExecuteEvent::NEW:  ReloadNew(reloadCfg); break;
	case ExecuteEvent::EDIT: ReloadEdit(reloadCfg); break;
	case ExecuteEvent::DEL:  ReloadDel(reloadCfg); break;
	default: break;
	}
}

void VS_GatewayService::AbstractGatewayEventListener::ReinitializeConfiguration()
{
	if (!((this->m_updated && m_clock.now() - m_last_update > WAIT_FOR_RESOLVE_DATA)
		|| m_wait_reload_configs < m_clock.now() - m_last_update))
		return;

	if (!m_updated)
	{
		m_update_result = m_peer_config->UpdateSettings();
		m_updated = true;
	}

	assert(m_updated);

	if (is_ready(m_update_result, std::chrono::milliseconds(100)))
	{
		const bool reload_all = m_reload_cfgs.find(string_view(EMPTY_STR)) != m_reload_cfgs.cend();
		bool HasUseAsTel(false);
		{
			auto &&registration_configs = m_peer_config->GetRegistrationSettings();
			auto &&locked_config = registration_configs.lock();
			for (auto &reg_config : *locked_config)
			{
				if (reg_config.UseAsTel)
				{
					HasUseAsTel = true;
				}

				if (reload_all || m_reload_cfgs.find(reg_config.RegistryConfigName) != m_reload_cfgs.cend())
				{
					auto &&call_manager = create_call_config_manager(reg_config);
					if (call_manager.NeedVerification())
					{
						call_manager.SetPendingVerification();
						SetRegistrationConfiguration(reg_config);
					}
				}
			}
		}
		m_has_tel_cfg.store(HasUseAsTel, std::memory_order_release);
		m_updated = false;
	}
	m_last_update = m_clock.now();
}

void VS_GatewayService::AbstractGatewayEventListener::ReloadAll()
{
	ResetAllConfigsStatus();
	m_update_result = m_peer_config->UpdateSettings();
	m_reload_cfgs.emplace(EMPTY_STR);
	m_updated = true;
	m_last_update = std::chrono::steady_clock::time_point();
}

void VS_GatewayService::AbstractGatewayEventListener::ReloadNew(const ReloadConfiguration& reloadCfg)
{
	VS_RegistryKey key{ false, GetPeerName() + ("\\" + reloadCfg.peerId) };
	if (key.IsValid())
	{
		m_update_result = m_peer_config->UpdateSettingByPeerId(reloadCfg.peerId.c_str());
		m_reload_cfgs.emplace(reloadCfg.peerId);
		m_updated = true;
		m_last_update = std::chrono::steady_clock::time_point();
	}
}

void VS_GatewayService::AbstractGatewayEventListener::ReloadEdit(const ReloadConfiguration& reloadCfg)
{
	bool ready = false;
	if (is_ready(m_update_result))
	{
		auto &&registration_configs = m_peer_config->GetRegistrationSettings();
		auto &&locked_config = registration_configs.lock();

		auto &&res = std::find_if((*locked_config).cbegin(), (*locked_config).cend(),
			[&](const VS_CallConfig &cfg)
		{
			return cfg.RegistryConfigName == reloadCfg.peerId;
		});
		if(res != (*locked_config).cend())
		{
			UpdateStatusRegistration(res->Address.addr, res->Address.port,
			[identificator = create_call_config_manager(*res).GetRegistrationIdentifier()]
			(const std::shared_ptr<VS_ParserInterface> &parser)
			{
				assert(parser != nullptr);
				parser->UpdateRegistrationConfig(identificator, [](VS_ParserInterface::RegistrartionConfig &item)
				{
					item.callConfig.IsValid = false;
					create_call_config_manager(item.callConfig).SetVerificationResult(VS_CallConfig::e_Unknown);
				});
			});
		}
		ready = true;
	}

	if (ready)
	{
		VS_RegistryKey key{ false, GetPeerName() + ("\\"  + reloadCfg.peerId) };
		if (key.IsValid())
		{
			int32_t reg_strategy = 0;
			if (key.GetValue(&reg_strategy, sizeof(reg_strategy), VS_REG_INTEGER_VT, REGISTRY_STRATEGY) && reg_strategy > 0)
			{
				m_update_result = m_peer_config->UpdateSettingByPeerId(reloadCfg.peerId.c_str());
				m_reload_cfgs.insert(reloadCfg.peerId);
				m_updated = true;
				m_last_update = std::chrono::steady_clock::time_point();
			}
		}
	}
	else
	{
		m_update_result = m_peer_config->UpdateSettings();
		m_reload_cfgs.emplace(EMPTY_STR);
		m_updated = true;
		m_last_update = std::chrono::steady_clock::time_point();
	}
}

void VS_GatewayService::AbstractGatewayEventListener::ReloadDel(const ReloadConfiguration& reloadCfg)
{
	bool ready = false;
	if (is_ready(m_update_result))
	{
		auto &&registration_configs = m_peer_config->GetRegistrationSettings();
		auto &&locked_config = registration_configs.lock();

		auto &&res_cfg = std::find_if(vs::cbegin(*locked_config), vs::cend(*locked_config),
			[&](const VS_CallConfig &cfg)
		{
			return cfg.RegistryConfigName == reloadCfg.peerId;
		});
		if(res_cfg != (*locked_config).cend())
		{
			UpdateStatusRegistration(res_cfg->Address.addr, res_cfg->Address.port,
				[identificator = create_call_config_manager(*res_cfg).GetRegistrationIdentifier()](const std::shared_ptr<VS_ParserInterface> &parser)
			{
				assert(parser != nullptr);
				parser->UpdateRegistrationConfig(identificator, [](VS_ParserInterface::RegistrartionConfig &item)
				{
					item.callConfig.IsValid = false;
					create_call_config_manager(item.callConfig).SetVerificationResult(VS_CallConfig::e_Unknown);
				});
			});
			locked_config->erase(res_cfg);
		}
		ready = true;
	}

	if(!ready)
	{
		ResetAllConfigsStatus();
		m_update_result = m_peer_config->UpdateSettings();
		m_reload_cfgs.emplace(EMPTY_STR);
		m_updated = true;
		m_last_update = std::chrono::steady_clock::time_point();
	}
}

bool VS_GatewayService::Processing(std::unique_ptr<VS_RouterMessage>&& recvMess)
{
	if (recvMess == nullptr)
	{
		return true;
	}
	VS_Container cnt;
	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		m_recvMess = recvMess.get();
		if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
			const char* method;
			if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != nullptr) {
				dprint4("VS_GatewayService::Processing::Processing(%s)\n", method);

				if (strcasecmp(method, RELOAD_CONFIGURATION) == 0)
				{
					return Processing(cnt);
				}
			}
		}
		break;
	case transport::MessageType::Notify:
		break;
	}
	m_recvMess = nullptr;
	return false;
}

bool VS_GatewayService::Init(const char* ourEndpoint, const char* ourService, const bool permittedAll)
{
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, EXTERNALPRESENCESTARTED_METHOD);
	void* body = nullptr;
	std::size_t bodySize;
	const auto res = cnt.SerializeAlloc(body, bodySize);
	assert(res);
	std::unique_ptr<void, free_deleter> body_free(body);
	std::unique_ptr<VS_RouterMessage> msg = vs::make_unique<VS_RouterMessage>(
		OurService(), nullptr, PRESENCE_SRV, nullptr, nullptr, OurEndpoint(),
		OurEndpoint(), default_timeout, body, bodySize);
	if (PostMes(msg.get()))
	{
		msg.release();
	}
	return true;
}

void VS_GatewayService::SetSipListener(const std::shared_ptr<GatewayEventListener>& listener)
{
	m_sip_listener = listener;
}

void VS_GatewayService::SetH323Listener(const std::shared_ptr<GatewayEventListener>& listener)
{
	m_h323_listener = listener;
}

bool VS_GatewayService::Processing(const VS_Container& cnt) const
{
	ReloadConfiguration reload_info;
	if (extractor_update_config(reload_info, cnt))
	{
		dprint4("VS_GatewayService::Processing(%s)\n", reload_info.protocol.GetStrValue());

		std::shared_ptr<GatewayEventListener> tmp;
		if (reload_info.protocol.GetValue() == Protocol::SIP && ((tmp = m_sip_listener.lock())))
		{
			tmp->HandleEvent(reload_info);
		}
		else if (reload_info.protocol.GetValue() == Protocol::H323 && ((tmp = m_h323_listener.lock())))
		{
			tmp->HandleEvent(reload_info);
		}
		return true;
	}
	return false;
}

#undef DEBUG_CURRENT_MODULE