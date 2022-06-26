#pragma once
#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "CallConfig/VS_CallConfigStorage.h"
#include "net/Address.h"
#include "net/Port.h"
#include "std-generic/compat/functional.h"
#include "std-generic/compat/memory.h"
#include "std-generic/compat/set.h"
#include "std-generic/cpplib/VS_ClockWrapper.h"

class VS_ParserInterface;

class VS_GatewayService :
	public VS_TransportRouterServiceReplyHelper
{
public:
	class EnumValue
	{
	public:
		explicit EnumValue(const unsigned value, const char *strValue);
		unsigned GetValue() const;
		const char *GetStrValue() const;
	private:
		unsigned value_;
		const char *valueStr_;
	};

	struct ExecuteEvent final : public EnumValue
	{
		enum
		{
			UNDEFINED = 0,
			ALL,
			NEW,
			EDIT,
			DEL
		};
		ExecuteEvent();
		ExecuteEvent(const unsigned value);
		ExecuteEvent(const char *str);
	};

	struct Protocol final : public EnumValue
	{
		enum
		{
			UNDEFINED = 0,
			H323,
			SIP
		};
		Protocol();
		Protocol(const unsigned value);
		Protocol(const char *str);
	};

	struct ReloadConfiguration
	{
		ExecuteEvent execute;
		Protocol protocol;
		std::string peerId;
	};

	class GatewayEventListener
	{
	public:
		virtual ~GatewayEventListener() {}
		virtual void HandleEvent(const ReloadConfiguration &reloadCfg) = 0;
		virtual void Start() = 0;
	};
	class AbstractGatewayEventListener : public GatewayEventListener, public vs::enable_shared_from_this<AbstractGatewayEventListener>
	{
	public:
		bool HasTelCfg() const;
		void Start() override;
		void HandleEvent(const ReloadConfiguration& reloadCfg) override;
	protected:
		explicit AbstractGatewayEventListener(std::shared_ptr<VS_CallConfigStorage> &&peerConfig);
		const std::shared_ptr<VS_CallConfigStorage> m_peer_config;
		//////////////////////////////////////////////////////////////////////////
		void ReinitializeConfiguration();
		//////////////////////////////////////////////////////////////////////////
		virtual void SetRegistrationConfiguration(VS_CallConfig config) = 0;
		virtual void UpdateStatusRegistration(const net::address& address, net::port, std::function<void(const std::shared_ptr<VS_ParserInterface>&)>&& exec) = 0;
		virtual void ResetAllConfigsStatus() = 0;
		virtual const char *GetPeerName() = 0;
	protected:
		steady_clock_wrapper &clock() const noexcept
		{
			return m_clock;
		}
	private:
		void ReloadAll();
		void ReloadNew(const ReloadConfiguration &reloadCfg);
		void ReloadEdit(const ReloadConfiguration &reloadCfg);
		void ReloadDel(const ReloadConfiguration &reloadCfg);
	private:
		template<class T>
		using set_reload_t = vs::set<T, vs::less<>>;

		set_reload_t<std::string> m_reload_cfgs;
	private:
		VS_CallConfigStorage::update_result_t m_update_result;
		std::atomic_bool m_has_tel_cfg;
		std::chrono::steady_clock::time_point m_last_update;
		std::chrono::seconds m_wait_reload_configs;
		bool m_updated;
	private:
		mutable steady_clock_wrapper m_clock;
	};
	VS_GatewayService() = default;
	bool Processing(std::unique_ptr<VS_RouterMessage>&&recvMess) override;
	bool Init(const char *ourEndpoint, const char *ourService, const bool permittedAll = false) override;
	void SetSipListener(const std::shared_ptr<GatewayEventListener>& listener);
	void SetH323Listener(const std::shared_ptr<GatewayEventListener>& listener);
private:
	bool Processing(const VS_Container &cnt) const;
private:
	std::weak_ptr<GatewayEventListener> m_sip_listener;
	std::weak_ptr<GatewayEventListener> m_h323_listener;
};
