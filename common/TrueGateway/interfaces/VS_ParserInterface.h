#pragma once

#include "VS_ConferenceProtocolInterface.h"
#include "../CallConfig/VS_CallConfig.h"
#include "acs_v2/Handler.h"

#include <boost/signals2.hpp>

#include <utility>
#include <vector>
#include <string>
#include "VS_ParserInfo.h"
#include "std-generic/compat/map.h"
#include "std-generic/compat/memory.h"
#include "std-generic/compat/functional.h"
#include "std-generic/cpplib/VS_ClockWrapper.h"
#include "std/cpplib/fast_mutex.h"

class VS_Policy;
class VS_CallConfigStorage;
namespace gw { struct Participant; }

/**
	»нтерфейс дл€ нашего протокола
	предоставл€ет методы управлени€ конференци€ми в нашей системе

	ѕостепенно будут добавл€тьс€ методы и аргументы в них
*/

class VS_ParserInterface:	public VS_ConferenceProtocolInterface,		// visi2sip
							public vs::enable_shared_from_this<VS_ParserInterface>
{
public:

	enum class SourceClean
	{
		UNDEFINED,
		PARSER,
		TRANSPORT
	};


	struct RegistrartionConfig final
	{
		VS_CallConfig callConfig;
		std::chrono::steady_clock::time_point lastTime;
	};

private:
	typedef vs::fast_recursive_mutex mutex_t;

	std::chrono::steady_clock::time_point m_last_register_time;
	mutable mutex_t		m_reg_configs_lock;

	vs::map<VS_CallConfig::RegistrationIdentifier, RegistrartionConfig, vs::less<>> m_reg_configs;

	// Next registrantion will be sent after EXPIRE_TIME * REGISTER_EXPIRES_PART seconds.
	const static double REGISTER_EXPIRES_PART;
protected:

	struct RegistrationIdentDialogId final
	{
		std::string registryConfigName;
		std::string userName;

		friend bool operator==(const RegistrationIdentDialogId& lhs, const RegistrationIdentDialogId& rhs)
		{
			return std::tie(lhs.registryConfigName, lhs.userName) == std::tie(rhs.registryConfigName, rhs.userName);
		}

		friend bool operator!=(const RegistrationIdentDialogId& lhs, const RegistrationIdentDialogId& rhs)
		{
			return !(lhs == rhs);
		}

		friend bool operator<(const RegistrationIdentDialogId& lhs, const RegistrationIdentDialogId& rhs)
		{
			return std::tie(lhs.registryConfigName, lhs.userName) < std::tie(rhs.registryConfigName, rhs.userName);
		}

		friend bool operator<=(const RegistrationIdentDialogId& lhs, const RegistrationIdentDialogId& rhs)
		{
			return !(rhs < lhs);
		}

		friend bool operator>(const RegistrationIdentDialogId& lhs, const RegistrationIdentDialogId& rhs)
		{
			return rhs < lhs;
		}

		friend bool operator>=(const RegistrationIdentDialogId& lhs, const RegistrationIdentDialogId& rhs)
		{
			return !(lhs < rhs);
		}
	};

	struct RegistrationIdentDialogIdView final
	{
		string_view registryConfigName;
		string_view userName;

		RegistrationIdentDialogIdView(string_view aRegistryConfigName, string_view aUserName);
		RegistrationIdentDialogIdView(const RegistrationIdentDialogId& ident);

		friend bool operator==(const RegistrationIdentDialogIdView& lhs, const RegistrationIdentDialogIdView& rhs) noexcept
		{
			return std::tie(lhs.registryConfigName, lhs.userName) == std::tie(rhs.registryConfigName, rhs.userName);
		}

		friend bool operator!=(const RegistrationIdentDialogIdView& lhs, const RegistrationIdentDialogIdView& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		friend bool operator<(const RegistrationIdentDialogIdView& lhs, const RegistrationIdentDialogIdView& rhs) noexcept
		{
			return std::tie(lhs.registryConfigName, lhs.userName) < std::tie(rhs.registryConfigName, rhs.userName);
		}

		friend bool operator<=(const RegistrationIdentDialogIdView& lhs, const RegistrationIdentDialogIdView& rhs) noexcept
		{
			return !(rhs < lhs);
		}

		friend bool operator>(const RegistrationIdentDialogIdView& lhs, const RegistrationIdentDialogIdView& rhs) noexcept
		{
			return rhs < lhs;
		}

		friend bool operator>=(const RegistrationIdentDialogIdView& lhs, const RegistrationIdentDialogIdView& rhs) noexcept
		{
			return !(lhs < rhs);
		}

		explicit operator RegistrationIdentDialogId() const
		{
			return RegistrationIdentDialogId{ std::string(registryConfigName), std::string(userName) };
		}
	};

	vs::map<RegistrationIdentDialogId, std::string, vs::less<>> m_register_dialog_id; // UserName -> CallID
	mutex_t		m_register_dialog_id_lock;

public:
	typedef boost::signals2::signal<void(string_view)> DialogFinishedSignal;
	typedef DialogFinishedSignal::slot_type DialogFinishedSlot;
protected:
	DialogFinishedSignal m_fireDialogFinished;

	std::weak_ptr<VS_ConferenceProtocolInterface>	m_confMethods;	// sip2visi

	net::Endpoint m_myCsEp;
	net::address	 m_my_media_ip;
	//eConnectionType	m_my_cs_conn_type;

	boost::shared_ptr<VS_Policy> m_policy;
	std::shared_ptr<VS_CallConfigStorage> m_CallConfigStorage;

	virtual void SetRegistrationConfigurationImpl(const VS_CallConfig& config);
	void RemovePermanentRegistrations(const bool invalidAllCfgs = true);
public:
	template<class T, typename ...Args>
	using collection_list_t = std::vector<T, Args...>;

	VS_ParserInterface();	// sip2visi
	virtual ~VS_ParserInterface() {}
	void SetConfCallBack(const std::shared_ptr<VS_ConferenceProtocolInterface>& confMethods);
	void SetPolicy(const boost::shared_ptr<VS_Policy> &policy);
	void SetCallConfigStorage( std::shared_ptr<VS_CallConfigStorage> storage);
	bool CheckUserNameInCallConfigStorage(const VS_CallConfig::eSignalingProtocol protocol, string_view remoteUsername) const;

	VS_CallConfig CreateCallConfig(const char *removeIp, string_view login, const std::string &userAgent = {});
	VS_CallConfig CreateCallConfig(const net::Endpoint &remoteEp, string_view login, const std::string& userAgent = {});

	virtual acs::Response Protocol(const void *buf, std::size_t sz) = 0;

	virtual std::string NewDialogID(string_view sipTo, string_view dtmf, const VS_CallConfig &config, string_view myName = {}) = 0;

	///for test
	virtual std::string SetNewDialogTest(string_view/*new_dialog*/, string_view/*sip_to*/, string_view /*dtmf*/,
		const VS_CallConfig &/*config*/, string_view /*my_name*/ = {})
	{
		return {};
	}
	///

	virtual void Shutdown() = 0;
	virtual void Timeout() = 0;

	virtual bool FillMediaChannels(string_view /*dialogId*/, std::vector<VS_MediaChannelInfo>& /*channels*/) /*= 0 abstract method */ { return false; }
	virtual bool ReqInvite(string_view /*dialogId*/, string_view /*fromId*/) { return false; }
	virtual void TakeTribuneReply(string_view /*dialogId*/, bool /*result*/) { }
	virtual void LeaveTribuneReply(string_view /*dialogId*/, bool /*result*/) { }

	/*virtual void SetMyCsAddress(unsigned long ip, unsigned short port, eConnectionType conn_type);
	virtual void SetMyCsAddress(in6_addr ip6, unsigned short port, eConnectionType conn_type);*/
	virtual void SetMyMediaAddress(const net::address &addr);
	virtual void SetMyCsAddress(const net::Endpoint& ep);
	virtual void SetPeerCSAddress(string_view /*dialogId*/, const net::Endpoint &/*ep*/) {}

	boost::signals2::connection Connect_DialogFinished(const DialogFinishedSlot &slot)
	{
		return m_fireDialogFinished.connect(slot);
	}

	virtual bool IsTrunkFull() = 0;
	virtual void SetUserToDialogIdCallback(std::function<void(string_view login, string_view dialogId)> /*f*/) = 0;

	void SetRegistrationConfiguration(VS_CallConfig config, const std::function<void(VS_CallConfig &)> &failureHandler = {});
	bool UpdateRegistrationConfig(VS_CallConfig::RegistrationIdentifierView regIdentifier, const std::function<void(RegistrartionConfig&)>& update, bool erase = false);
	void ResetAllConfigsStatus();

	// Try to determinate channelID from net packet (buff, sz).
	// Parameter is_fragmented == true when net packet is fragmented
	// and we need more data to decode it.
	virtual VS_ChannelID GetChannelID(const void *buf, std::size_t sz, bool& isFragmented);
	virtual VS_ChannelID GetDefaultChannelID() = 0;

	bool NeedPermanentConnection() const;
	virtual VS_CallConfig::eSignalingProtocol MySignallingProtocol() = 0;
	void CheckPermanentRegistrations();
	boost::optional<std::string> GetRegDialogIdByRegIdentDialogId(RegistrationIdentDialogIdView regIdentView);
	collection_list_t<boost::optional<std::string>> GetRegDialogIdsByUsername(string_view name);

	virtual bool DoRegister(string_view dialogId, const bool updateData = false);
	boost::optional<std::string> UpdateRegisterContext(const VS_CallConfig &config, bool force=false);

	virtual std::shared_ptr<VS_ParserInfo> GetParserContextBase(string_view /*dialogId*/, bool /*createNew*/ = false)
	{
		return {};
	}

	virtual void SetDigestChecker(const std::function<bool(const std::string&, const std::string&)>& /*f*/) {}
	void AsyncInvite(string_view, const gw::Participant&, string_view, const VS_ConferenceInfo&,
		std::function<void(bool, ConferenceStatus, const std::string&)> inviteResult, string_view /*dnFromUTF8*/, bool,
		bool) override
	{
		inviteResult(false, ConferenceStatus::UNDEFINED, {});
	}

	virtual int SetRecvBuf(const void* /*buf*/, std::size_t /*sz*/, const VS_ChannelID /*channelId*/,
		const net::address & /*remoteAddr*/, net::port /*remotePort*/, const net::address & /*localAddr*/, net::port /*localPort*/) { return 0; }
	virtual int GetBufForSend(void* /*buf*/, std::size_t &/*sz*/, const VS_ChannelID /*channelId*/,
		const net::address & /*remoteAddr*/, net::port /*remotePort*/, const net::address & /*localAddr*/, net::port /*localPort*/) { return 0; }
	virtual void CleanParserContext(string_view /*dialogId*/, SourceClean /*cause*/ = SourceClean::UNDEFINED) {}
	// It is supposed to be used in unit tests.
	static std::shared_ptr<VS_ParserInfo> GetParserContextByDialogID(const std::shared_ptr<VS_ParserInterface> &parser, string_view dialogId);

protected:
	steady_clock_wrapper &clock() const noexcept { return m_clock; }
private:
	mutable steady_clock_wrapper m_clock;
};