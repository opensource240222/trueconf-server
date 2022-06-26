#pragma once
#include <vector>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include "../clientcontrols/VS_ClientControlInterface.h"
#include "../../statuslib/VS_ExternalPresenceInterface.h"
#include "../../statuslib/status_types.h"
#include "std-generic/compat/map.h"
#include "std/cpplib/VS_ExternalAccount.h"
#include "std-generic/cpplib/StrCompare.h"
#include <boost/asio/steady_timer.hpp>

#include "transport/Router/VS_TransportRouterServiceBase.h"
#include "../../transport/Router/VS_TransportRouterServiceHelper.h"
#include "../VS_GatewayService.h"

struct VS_CallConfig;
class VS_CallConfigStorage;
class VS_ParserInterface;
class VS_TranscoderKeeper;
class VS_UserData;

class VS_SIPCallResolver :	public VS_ExternalPresenceInterface,
							public VS_TransportRouterServiceReplyHelper,
							public VS_GatewayService::AbstractGatewayEventListener
							{
public:

	struct InitInfo final
	{
		std::shared_ptr<VS_TranscoderKeeper> trKeeper;
		std::shared_ptr<VS_ParserInterface> parser;
		std::shared_ptr<VS_CallConfigStorage> peerConfig;
	};

	virtual ~VS_SIPCallResolver() {}

	void NewPeerCfg(string_view callId, const std::vector<VS_ExternalAccount> &v);
	std::string GetNameFromCallID(string_view callId) const;
	void UpdateStatus(string_view callId, VS_UserPresence_Status status);
	void ReqInvite(string_view dialogId, string_view name);
	void TakeTribuneReply(string_view dialogId, bool result);
	void LeaveTribuneReply(string_view dialogId, bool result);
	void HandleEvent(const VS_GatewayService::ReloadConfiguration& reloadCfg) override;
protected:
	VS_SIPCallResolver(boost::asio::io_service::strand &strand, InitInfo &&init);
	static void PostConstruct(std::shared_ptr<VS_SIPCallResolver>& p)
	{
		p->ScheduleTimer();
	}

	void SetRegistrationConfiguration(VS_CallConfig config) override;
	void UpdateStatusRegistration(const net::address& address, net::port, std::function<void(const std::shared_ptr<VS_ParserInterface>&)>&& exec) override;
	const char* GetPeerName() override;
	void ResetAllConfigsStatus() override;
	void Timeout();

private:
	VS_UserPresence_Status GetSIPUserStatus(string_view sipId);
	std::string PrepareTranscoder(string_view toSipId, VS_UserData *fromUser);		// return TranscoderID
	std::string PrepareTranscoder(string_view toSipId, VS_UserData *fromUser, const VS_CallConfig &config);

	////Service
	bool Processing(std::unique_ptr<VS_RouterMessage> &&recvMess) override;
	bool Init(const char *ourEndpoint, const char *ourService, bool permittedAll = false) override;
	void SendMessage_Method(VS_Container &cnt);
	void SendCommand_Method(VS_Container &cnt);
	std::string NewDialogIDFromParser(string_view sipTo, string_view dtmf, const VS_CallConfig &config, string_view myName = {});

	////VS_ExternalPresenceInterface
	bool Resolve(std::string& callId, VS_CallIDInfo& ci, VS_UserData* fromUser) override;
	void Subscribe(const char *callId) override;
	void Unsubscribe(const char *callId) override;
	bool IsMyCallId(const char *callId) const override;
	bool CanICall(VS_UserData *fromUde, const char *toCallId, bool IsVCS) override;
	bool IsRegisteredTransId(const char *transId) override;
	void ScheduleTimer(const std::chrono::milliseconds period = std::chrono::milliseconds(500));
private:
	typedef std::recursive_mutex mutex_t;
	mutable mutex_t m_mutex;

	boost::asio::io_service::strand &m_strand;
	std::shared_ptr<VS_TranscoderKeeper> m_tr_keeper;
	std::shared_ptr<VS_ParserInterface> m_parser;
	vs::set<std::string, vs::str_less> m_subscribes;
	vs::map<std::string, VS_UserPresence_Status, vs::str_less> m_statuses;
	boost::asio::steady_timer	m_timer;
};