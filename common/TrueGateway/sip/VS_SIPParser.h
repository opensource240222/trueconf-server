#pragma once
#include "TrueGateway/VS_NetworkConnectionACL.h"

#include "TrueGateway/interfaces/VS_ParserInterface.h"
#include "VS_SIPParserInfo.h"
#include "SIPParserLib/VS_SIPTransactionHandler.h"
#include "tools/Server/CommonTypes.h"
#include "std-generic/cpplib/synchronized.h"
#include "std-generic/cpplib/StrCompare.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

#include <chrono>
#include <string>
#include <vector>
#include "std/cpplib/VS_Policy.h"
#include <boost/circular_buffer.hpp>

class VS_SIPMessage;
class VS_SIPRequest;
class VS_SIPResponse;
namespace net {
	class LoggerInterface;
} //namespace net

struct VS_ConferenceInfo;

class VS_OurNonce final
{
public:
	VS_OurNonce(std::size_t capacity = 1000) : m_ourNonces(capacity) {}
	void AddOurNonce(std::string nonce)
	{
		std::lock_guard<mutex_t> lock{ m_ourNoncesLock };
		m_ourNonces.push_back(std::move(nonce));
	}
	bool IsOurNonce(string_view nonce) const
	{
		std::lock_guard<mutex_t> lock{ m_ourNoncesLock };
		return std::find(m_ourNonces.begin(), m_ourNonces.end(), nonce) != m_ourNonces.end();
	}
private:
	typedef vs::fast_mutex mutex_t;
private:
	mutable mutex_t										m_ourNoncesLock;
	boost::circular_buffer_space_optimized<std::string> m_ourNonces;
};

enum eSIPUserAgent : int;

class VS_SIPParser: public VS_ParserInterface
{
	template <class T> friend class SIPParserTestBase;
	friend struct SIPChat;

public:
	virtual ~VS_SIPParser();

	static const VS_Policy::PolicySettingsInitInterface *PolicySettings();

	CtxMsgPair GetMsgForSend_SIP();
	CtxMsgPair GetMsgForSend_SIP(string_view dialogId);
	CtxMsgPair GetMsgForSend_SIP(const std::set<std::string, vs::str_less> &dialogIds);
	CtxMsgPair GetMsgForRetransmit_SIP();

	bool SetRecvMsg_SIP(const std::shared_ptr<VS_SIPMessage> &msg, const net::Endpoint& remoteEp);

	void CleanParserContext(string_view dialogId, SourceClean event) override;

	bool InviteMethod(string_view dialogId, string_view fromId, string_view toId, const VS_ConferenceInfo &i,
		string_view dnFromUTF8 = {}, bool newSession = true, bool forceCreate = false) override;
	bool InviteReplay(string_view dialogId, VS_CallConfirmCode confirmCode, bool isGroupConf, string_view confName = {}, string_view to_displayName = {}) override;
	void Hangup(string_view dialogId) override;
	void LoggedOutAsUser(string_view dialogId) override;
	void FastUpdatePicture(string_view dialogId) override;
	void Chat(string_view dialogId, const std::string &from, const std::string &to, const std::string &dn, const char *mess) override;
	void Command(string_view dialogId, string_view from, string_view command) override;
	acs::Response Protocol(const void* buf, std::size_t sz) override;
	void Timeout() override;
	std::string NewDialogID(string_view sipTo, string_view dtmf, const VS_CallConfig &config = {}, string_view myName = {}) override;
	std::string SetNewDialogTest(string_view newDialog, string_view sipTo, string_view dtmf, const VS_CallConfig &config, string_view myName = {}) override;
	void Shutdown() override;
	bool SetMediaChannels(string_view dialogId, const std::vector<VS_MediaChannelInfo>& channels, const std::string& existingConfID, std::int32_t bandwRcv) override;
	bool FillMediaChannels(string_view dialogId, std::vector<VS_MediaChannelInfo>& channels) override;

	bool ReqInvite(string_view dialogId, string_view fromId) override;

	void TakeTribuneReply(string_view dialogId, bool result) override;
	void LeaveTribuneReply(string_view dialogId, bool result) override;

	bool IsTrunkFull() override;

	bool ProcessTransaction(const std::shared_ptr<VS_SIPParserInfo> &ctx, const std::shared_ptr<VS_SIPMessage> &msg);

	void NotificateAboutReject(string_view dialogId) override;
	void NotificateAboutReject(const std::shared_ptr<VS_SIPParserInfo> &ctx);

	void SetUserToDialogIdCallback(std::function<void(string_view login, string_view dialogId)> f) override
	{
		m_set_dialog_id_callback = std::move(f);
	}

	VS_ChannelID GetChannelID(const void *buf, std::size_t sz, bool & isFragmented) override;
	VS_ChannelID GetDefaultChannelID() override
	{ return e_SIP_CS; }

	VS_CallConfig::eSignalingProtocol MySignallingProtocol() override
	{	return VS_CallConfig::SIP; }

	void SetDigestChecker(const std::function<bool(const std::string&, const std::string&)>& f) override;

	bool IsConfOwner(std::shared_ptr<VS_SIPParserInfo> &callCtx, string_view from) const;
	bool IsGroupConf(const std::shared_ptr<VS_SIPParserInfo>& callCtx) const;
	std::string GetPeer(const std::shared_ptr<VS_SIPParserInfo>& callCtx) const;
	std::string GetConferenceID(const std::shared_ptr<VS_SIPParserInfo>& callCtx) const;
	static eSIPUserAgent IdentifyUserAgent(const std::shared_ptr<VS_SIPParserInfo> &ctx);

	typedef boost::signals2::signal<std::string(string_view /*dialog*/, const net::Endpoint &/*bindEp*/,
		const net::Endpoint &/*peerEp*/)> GetSRTPKeySignal;

	template<class Callable>
	void Connect_GetSRTPKey(Callable && fire)
	{
		m_fireGetSRTPKey = std::forward<Callable>(fire);
	}

	std::string GetAdminContact() const;
	std::string MakeAnswerToContactAdmin() const;

	boost::signals2::connection Connect_FreeDialogFromChannel(const boost::signals2::signal<void(string_view)>::slot_type &slot)
	{
		return m_fireFreeDialogFromChannel.connect(slot);
	}

	std::string ProcessCallCommand(const std::shared_ptr<VS_SIPParserInfo> &newCtx, string_view from, const std::vector<std::string>& args);
	virtual std::string ProcessConfCommand(const std::shared_ptr<VS_SIPParserInfo>& newCtx, string_view from, const std::vector<std::string>& args);
	std::string ProcessWebinarCommand(const std::shared_ptr<VS_SIPParserInfo>& newCtx, string_view from, const std::vector<std::string>& args);
	std::string ProcessPodiumCommand(const std::shared_ptr<VS_SIPParserInfo> &ctx, string_view from, const std::vector<std::string>& args, bool toPodium);
	void ProcessChangeMyTribuneStateCommand(const std::shared_ptr<VS_SIPParserInfo>& ctx, bool toPodium, std::string& outAnswer);
	std::string ProcessCommandWithConferenceParticipant(const std::shared_ptr<VS_SIPParserInfo>& ctx, string_view from, const std::vector<std::string>& args);
	std::string ProcessReqInviteAnswer(const std::shared_ptr<VS_SIPParserInfo>& ctx, string_view from, const std::vector<std::string>& args);
	void SendChatBotAnswer(std::shared_ptr<VS_SIPParserInfo>& ctx, string_view from, bool hasAuthenticatedMsgCtx, const std::string &answer, bool makeRTFFromText, eContentType desiredContentType);
	bool ProcessChatCommand(std::shared_ptr<VS_SIPParserInfo> &ctx, string_view from, const std::string &msg);
	virtual std::shared_ptr<VS_SIPParserInfo> FindActiveCallCtx(const std::shared_ptr<VS_SIPParserInfo>& ctx);
	virtual std::shared_ptr<VS_SIPParserInfo> FindActiveMsgCtx(const std::shared_ptr<VS_SIPParserInfo>& ctx);
	static std::string RtfToTxt(const std::string &s);
	std::string MakeChatBotHelpMSG(eContentType ct) const;

	void SetGetAppPropertyFunction(std::function<std::string(string_view)> getAppProperty)
	{
		m_get_app_property = std::move(getAppProperty);
	}

	void SetGetWebManagerPropertyFunction(std::function<std::string(string_view)> getWebManagerProp)
	{
		m_get_web_manager_property = std::move(getWebManagerProp);
	}

	bool RetryCall(string_view dialogId, std::chrono::seconds interval = std::chrono::seconds(10));
	void UseACL(bool use);
	bool IsACLUsed(void) const;

protected:
	VS_SIPParser(boost::asio::io_service::strand& transportStrand, std::string userAgent, const std::shared_ptr<net::LoggerInterface> &logger);
	static void PostConstruct(std::shared_ptr<VS_SIPParser>& /*p*/) {}

	std::string GetFromHost() const;
	bool NeedToRetryCall(const std::shared_ptr<VS_SIPResponse>& rsp);
	bool SetToIPAddress(std::shared_ptr<VS_SIPParserInfo> &ctx, const std::string &from, const std::string &dn, const std::string &toServer) const;


private:
	std::function< std::string(string_view dialog, const net::Endpoint &bindEp,
		const net::Endpoint &peerEp)> m_fireGetSRTPKey;
	boost::signals2::signal<void(string_view)> m_fireFreeDialogFromChannel;

	std::function<std::string(string_view)> m_get_app_property;
	std::function<std::string(string_view)> m_get_web_manager_property;

	void OnContextDestructor(string_view dialogId);

	bool OnRequestArrived(const std::shared_ptr<VS_SIPRequest>& req, const net::Endpoint peerEp);
	bool OnResponseArrived(const std::shared_ptr<VS_SIPResponse>& rsp);

	bool OnRequest_Invite(const std::shared_ptr<VS_SIPRequest>& req, const net::Endpoint &recvFromEp);
	bool OnRequest_Update(const std::shared_ptr<VS_SIPRequest>& req, const net::Endpoint &recvFromEp);
	bool OnRequest_BYE(const std::shared_ptr<VS_SIPRequest>& req);
	bool OnRequest_ACK(const std::shared_ptr<VS_SIPRequest>& req);
	bool OnRequest_CANCEL(const std::shared_ptr<VS_SIPRequest>& req);
	bool OnRequest_INFO(const std::shared_ptr<VS_SIPRequest>& req);
	bool OnRequest_OPTIONS(const std::shared_ptr<VS_SIPRequest>& req, const net::Endpoint &recvFromEp);
	bool OnRequest_REGISTER(const std::shared_ptr<VS_SIPRequest>& req, const net::Endpoint &recvFromEp);
	bool OnRequest_SUBSCRIBE(const std::shared_ptr<VS_SIPRequest>& req);
	bool OnRequest_PUBLISH(const std::shared_ptr<VS_SIPRequest>& req);
	bool OnRequest_MESSAGE(const std::shared_ptr<VS_SIPRequest>& req, const net::Endpoint &recvFromEp);

	bool Request_Unsupported(const std::shared_ptr<VS_SIPRequest>& req);
	bool OnResponse_Code200(const std::shared_ptr<VS_SIPResponse>& rsp);

	bool PutMessageForOutput(const std::shared_ptr<VS_SIPMessage> &msg, const std::shared_ptr<VS_SIPParserInfo> &ctx = nullptr);

	bool DoRegister(string_view dialogId, const bool updateData = false) override;

	bool UpdateSIPDisplayName(const std::shared_ptr<VS_SIPParserInfo>& ctx, const bool updateImmediately);

	std::shared_ptr<VS_ParserInfo> GetParserContextBase(string_view dialogId, bool create = false) override;
public:
	virtual std::shared_ptr<VS_SIPParserInfo> GetParserContext(string_view dialogId, bool create = false);
private:
	std::shared_ptr<VS_SIPParserInfo> GetParserRegisterContext(string_view userId);								// when someone registered on our server
	virtual std::shared_ptr<VS_SIPParserInfo> GetRegContextOnRemoteServer(const std::shared_ptr<VS_SIPParserInfo> &ctx,	// when we registered on some server
		const std::shared_ptr<VS_SIPRequest> &req = nullptr);
	std::shared_ptr<VS_SIPParserInfo> GetRegCxtByContact(string_view contact);
	std::shared_ptr<VS_SIPParserInfo> GetTCPRegCxtByContact(string_view contact);

	void InitMediaStreams(const std::shared_ptr<VS_SIPParserInfo>& ctx);
	void InitMSGMediaStream(const std::shared_ptr<VS_SIPParserInfo>& ctx);
	void AuthenticateAndSendInstantMessage(std::shared_ptr<VS_SIPParserInfo>& msg_ctx, const uint8_t step);
	bool SendSetMediaChannels(const std::shared_ptr<VS_SIPParserInfo>& ctx, VS_SIPParserInfo::SMCContinuation cont);
	bool InitBFCP(const std::shared_ptr<VS_SIPParserInfo>& ctx);
	void UpdateSlideshowState(const std::shared_ptr<VS_SIPParserInfo>& ctx, bool active);

	void UpdateCallConfig(std::shared_ptr<VS_SIPParserInfo> &ctx, const net::Endpoint& remoteEp, bool withMedia = true);

	void LoginAllowed(bool isAllowed, std::chrono::seconds expireTime, std::string &externalName,
		std::shared_ptr<VS_SIPResponse> &ok, std::shared_ptr<VS_SIPResponse>&fail);

	void onLoginResponse(bool res, std::shared_ptr<VS_SIPResponse> ok, std::shared_ptr<VS_SIPResponse> fail);
	void onLogoutResponse(bool res, std::shared_ptr<VS_SIPResponse> ok, std::shared_ptr<VS_SIPResponse> fail, bool do_send_200ok);
	void onLogout(string_view dialogId);
	void AsyncInviteResult(bool redirect, ConferenceStatus status, const std::shared_ptr<VS_SIPParserInfo>& ctx, string_view ip);
	std::function<bool(const std::string&, const std::string&)> m_check_digest;

	bool SendDTMF(const std::shared_ptr<VS_SIPParserInfo>& ctx, const char dtmf);

	bool DoInviteWithOptions(const std::shared_ptr<VS_SIPParserInfo> &ctx);
	bool DoInvite(const std::shared_ptr<VS_SIPParserInfo> &ctx);

	void AddInfoFromRegConfig(std::shared_ptr<VS_SIPParserInfo> &ctx, const std::shared_ptr<VS_SIPRequest> &req);
	void AddAuthInfoFromRegContext(std::shared_ptr<VS_SIPParserInfo> &ctx, const std::shared_ptr<VS_SIPRequest> &req);

private:
	static const bool DEFAULT_SESSION_TIMERS_ENABLED;

	typedef std::recursive_mutex mutex_t;
protected:
	boost::asio::io_service::strand& m_transportStrand;
private:
	vs::map<std::string, std::shared_ptr<VS_SIPParserInfo>, vs::str_less> m_ctx;
	mutex_t m_ctx_lock;
	std::deque<CtxMsgPair> m_msg_queue_out;
	mutex_t m_msg_queue_lock;
	std::vector<std::string> m_ctx_garbage;
	mutex_t m_ctx_garbage_lock;
	std::vector<boost::signals2::scoped_connection> m_ctx_connections;
	std::vector<char> m_bfcp_send_buf;
	std::function<void(string_view, string_view)> m_set_dialog_id_callback;
	vs::Synchronized<VS_SIPTransactionHandler> m_transaction_handler;
	VS_NetworkConnectionACL m_acl;
	std::chrono::steady_clock::time_point m_shutdown_tick;
	bool m_use_acl;
	const std::string m_userAgent;
	const std::chrono::seconds m_regMaxExpires;
	const std::chrono::seconds m_retryAfterHeader;
	std::shared_ptr<net::LoggerInterface> m_logger;
};
