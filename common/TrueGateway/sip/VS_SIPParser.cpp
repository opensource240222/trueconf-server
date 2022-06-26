#include "VS_SIPParser.h"
#include "TrueGateway/net/VS_SignalChannel.h"
#include "TrueGateway/CallConfig/VS_CallConfigCorrector.h"
#include "TrueGateway/CallConfig/VS_IndentifierSIP.h"
#include "TrueGateway/clientcontrols/VS_ClientControlInterface.h"
#include "TrueGateway/VS_GatewayParticipantInfo.h"
#include "SIPParserLib/VS_SIPRequest.h"
#include "SIPParserLib/VS_SIPResponse.h"
#include "SIPParserLib/VS_SIPMetaField.h"
#include "SIPParserLib/VS_SDPMetaField.h"
#include "SIPParserLib/VS_SIPMessage.h"
#include "SIPParserLib/VS_SDPCodec.h"
#include "SIPParserLib/VS_SIPURI.h"
#include "SIPParserLib/VS_SIPAuthInfo.h"
#include "SIPParserLib/VS_SIPAuthScheme.h"
#include "SIPParserLib/VS_SIPAuthDigest.h"
#include "SIPParserLib/VS_SIPAuthGSS.h"
#include "SIPParserLib/VS_SIPField_Auth.h"
#include "SIPParserLib/VS_SIPField_To.h"
#include "SIPParserLib/VS_SIPField_From.h"
#include "SIPParserLib/VS_SIPField_Via.h"
#include "SIPParserLib/VS_SIPField_Contact.h"
#include "SIPParserLib/VS_SIPField_CSeq.h"
#include "SIPParserLib/VS_SIPField_CallID.h"
#include "SIPParserLib/VS_SIPField_StartLine.h"
#include "SIPParserLib/VS_SIPField_Expires.h"
#include "SIPParserLib/VS_SIPField_SessionExpires.h"
#include "SIPParserLib/VS_SIPField_MinExpires.h"
#include "SIPParserLib/VS_SDPField_Connection.h"
#include "SIPParserLib/VS_SDPField_MediaStream.h"
#include "SIPParserLib/VS_SIPField_UserAgent.h"
#include "SIPParserLib/VS_BFCPMessage.h"
#include "SIPParserLib/VS_BFCPSession.h"
#include "SIPParserLib/VS_UUID.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/VS_UserData.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/VS_Protocol.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/md5.h"
#include "SIPParserLib/VS_SIPInstantMessage.h"
#include "TrueGateway/CallConfig/VS_CallConfigStorage.h"
#include "VS_SIPSeqSSPI.h"
#include "FakeClient/VS_ConferenceInfo.h"
#include "std/cpplib/VS_Replace.h"
#include "Bwt/Handshake.h"
#include "tools/Server/VS_MediaChannelInfo.h"

#include "std/cpplib/base64.h"
#include "std/debuglog/VS_LogHelpers.h"
#include "std/debuglog/VS_Debug.h"

#define NULL_TICK std::chrono::steady_clock::time_point()

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>


#include "TrueGateway/sip/VS_SIPGetInfoImpl.h"
#include "TrueGateway/sip/VS_SIPUpdateInfoImpl.h"
#include "VS_ChatBotMessages.h"
#include "SIPParserBase/VS_Const.h"
#include "net/QoSSettings.h"
#include "std-generic/cpplib/ignore.h"
#include "net/DNSUtils/VS_DNSTools.h"
#include "std-generic/cpplib/ThreadUtils.h"

#define DEBUG_CURRENT_MODULE VS_DM_SIPPARSER

namespace
{
	const std::chrono::minutes CALL_START_TIMEOUT(2);				//  = 2 * 60 * 1000;	// 2 minute, but by RFC it must be 64*500 (64 * T1)
	const std::chrono::seconds CALL_BYE_TIMEOUT(30);				//  = 30 * 1000;		// 30 seconds
	const std::chrono::seconds SET_MEDIA_CHANNELS_TIMEOUT(5);		//  = 5 * 1000;			// 5 seconds
	const std::chrono::seconds OPTIONS_TIMEOUT(20);				    //  = 20 * 1000;		// 20 seconds
	const std::chrono::minutes DEFAULT_EXPIRES(1);				    //  = 1 * 60;			// 1 minute
	const std::chrono::seconds SHUTDOWN_TIME_LIMIT(5);				//  = 5 * 1000;			// 5 seconds
	const auto DEFAULT_DTMF_TRESHOLD(5);							//  = 5;				// 5 seconds default treshold to send dtmf digits
	const std::chrono::seconds AVERAGE_CALL_DURATION(1800);	    	// average call duration from sip standart is 1800 sec
	const std::chrono::seconds DEFAULT_RETRY_AFTER(5);		//  = 5 * 1000			// 5 seconds


	std::string CHAT_CTRLS_HELP_RTF;
	std::string CHAT_CTRLS_HELP_PLAIN_TEXT;

	VS_OurNonce g_our_nonce_storage;

	const unsigned int TRY_LOGIN = 777; //type for policy settings

										//used in this scope
	inline void disable_registration_on_call(VS_CallConfig &cfg, VS_ParserInterface::RegistrartionConfig &item)
	{
		if (create_call_config_manager(cfg).IsRegistrationOnCall())
		{
			cfg.IsValid = false;
		}
	}

	const boost::regex MAIL_FORM("[\\s]?<[\\w/]*:[^>]*>[ ]?", boost::regex_constants::optimize);
	const boost::regex CHAT_BOT_CMD_E("[^\\s\",]+"		// chat bot command, username, conference name
		"|\"[^\"]+\"",		// topic between quotation marks
		boost::regex_constants::optimize);


	// just add rtf header, foot and replace new line character
	inline std::string MakeSimpleRTF(string_view plainText)
	{
		std::string rtf_message(plainText);
		VS_ReplaceAll(rtf_message, "\\", "\\\\");
		VS_ReplaceAll(rtf_message, "\n", R"(\outl0\f1\lang1033  \f0\lang1049\par\n\r )");

		rtf_message = R"({\rtf1\fbidis\ansi\ansicpg1251\deff0\nouicompat\deflang1049{\fonttbl{\f0\fnil Segoe UI;}{\f1\fnil\fcharset0 Segoe UI;}})" "\n"
			R"({\colortbl ;\red0\green0\blue0;})" "\n"
			R"({\*\generator Riched20 16.0.4600}\viewkind4\uc1)" "\n"
			R"(\pard\cf1\outl\f0\fs20)" + rtf_message;
		rtf_message += R"({\*\lyncflags<rtf=1>}})";

		return rtf_message;
	}


	inline UserStatusInfo GetStatus(const std::shared_ptr<VS_SIPParserInfo> &ctx, VS_ConferenceProtocolInterface& confMethods, string_view userId) {
		UserStatusInfo us_info;
		us_info.real_id = std::string(userId);
		if (!ctx) return us_info;

		if (VS_IsRTPCallID(userId)) {
			auto cfg = ctx->GetConfig();
			if (!cfg.isAuthorized.get_value_or(false)) {
				dstream2 << "Activity from " << ctx->GetAliasRemote() << " to " << userId << " suppressed. Reason: not authorized call to third-party protocol (sip/h323/rtsp)\n";
				return us_info;
			}
		}

		return confMethods.GetUserStatus(userId);
	}

	struct PolicySettingsSipInit final : public VS_Policy::PolicySettingsInitInterface
	{
		unsigned int GetPolicyEndpointSettings(const PolicyEndpointSettings*& obj) const noexcept override;

		static const VS_Policy::PolicySettings POLICY_DEFAULT;
		static const VS_Policy::PolicySettings POLICY_INFO_OR_MESSAGE_METHOD;
		static const VS_Policy::PolicySettings POLICY_LOGIN;
		static const PolicyEndpointSettings SETTINGS[];
	};

	const VS_Policy::PolicySettings PolicySettingsSipInit::POLICY_DEFAULT
	{
		1,	  // use_ban
		1,	  // use_login
		0,	  // access_deny_on_ban
		1,	  // silent_on_ban
		3 * 60, // max_fail_before_ban = msgs in observe_interval (~3 msg per 1 sec)
		0,	  // max_fail_before_delay
		std::chrono::seconds(5),		// delay_interval
		std::chrono::seconds(60 * 60),	// delay_time
		std::chrono::hours(24),			// ban_time
		std::chrono::seconds(60),		// delay_ttl
		std::chrono::minutes(1)			// observe_interval
	};

	const VS_Policy::PolicySettings PolicySettingsSipInit::POLICY_INFO_OR_MESSAGE_METHOD
	{
		1,		// use_ban
		1,		// use_login
		0,		// access_deny_on_ban
		1,		// silent_on_ban
		50 * 60,// max_fail_before_ban = msgs in observe_interval (~50 msg per 1 sec)
		0,		// max_fail_before_delay
		std::chrono::seconds(5),		// delay_interval
		std::chrono::seconds(60 * 60),	// delay_time
		std::chrono::hours(24),			// ban_time
		std::chrono::seconds(60),		// delay_ttl
		std::chrono::minutes(1)			// observe_interval
	};

	const VS_Policy::PolicySettings PolicySettingsSipInit::POLICY_LOGIN
	{
		1,	// use_ban
		1,	// use_login
		0,	// access_deny_on_ban
		1,	// silent_on_ban
		10,	// max_fail_before_ban = msgs in observe_interval (~10 logins per 1 minues)
		0,  // max_fail_before_delay
		std::chrono::seconds(5),		// delay_interval
		std::chrono::seconds(60 * 60),	// delay_time
		std::chrono::hours(24),			// ban_time
		std::chrono::seconds(60),		// delay_ttl
		std::chrono::minutes(1)			// observe_interval
	};

	const VS_Policy::PolicySettingsInitInterface::PolicyEndpointSettings PolicySettingsSipInit::SETTINGS[]
	{
		{ eStartLineType::TYPE_INVITE, CONFIGURATION_SIP, CONFIGURATION_INVITE, &PolicySettingsSipInit::POLICY_DEFAULT },
		{ eStartLineType::TYPE_REGISTER, CONFIGURATION_SIP, CONFIGURATION_REGISTER, &PolicySettingsSipInit::POLICY_DEFAULT },
		{ eStartLineType::TYPE_UPDATE, CONFIGURATION_SIP, CONFIGURATION_UPDATE, &PolicySettingsSipInit::POLICY_DEFAULT },
		{ eStartLineType::TYPE_CANCEL, CONFIGURATION_SIP, CONFIGURATION_CANCEL, &PolicySettingsSipInit::POLICY_DEFAULT },
		{ eStartLineType::TYPE_ACK, CONFIGURATION_SIP, CONFIGURATION_ACK, &PolicySettingsSipInit::POLICY_DEFAULT },
		{ eStartLineType::TYPE_BYE, CONFIGURATION_SIP, CONFIGURATION_BYE, &PolicySettingsSipInit::POLICY_DEFAULT },
		{ eStartLineType::TYPE_INFO, CONFIGURATION_SIP, CONFIGURATION_INFO, &PolicySettingsSipInit::POLICY_INFO_OR_MESSAGE_METHOD },
		{ eStartLineType::TYPE_MESSAGE, CONFIGURATION_SIP, CONFIGURATION_MESSAGE, &PolicySettingsSipInit::POLICY_INFO_OR_MESSAGE_METHOD },
		{ eStartLineType::TYPE_NOTIFY, CONFIGURATION_SIP, CONFIGURATION_NOTIFY, &PolicySettingsSipInit::POLICY_DEFAULT },
		{ eStartLineType::TYPE_OPTIONS, CONFIGURATION_SIP, CONFIGURATION_OPTIONS, &PolicySettingsSipInit::POLICY_DEFAULT },
		{ eStartLineType::TYPE_PUBLISH, CONFIGURATION_SIP, CONFIGURATION_PUBLISH, &PolicySettingsSipInit::POLICY_DEFAULT },
		{ eStartLineType::TYPE_SUBSCRIBE, CONFIGURATION_SIP, CONFIGURATION_SUBSCRIBE, &PolicySettingsSipInit::POLICY_DEFAULT },
		{ TRY_LOGIN, CONFIGURATION_SIP, CONFIGURATION_TRY_LOGIN, &PolicySettingsSipInit::POLICY_LOGIN },
	};

	unsigned PolicySettingsSipInit::GetPolicyEndpointSettings(const PolicyEndpointSettings*& obj) const noexcept
	{
		obj = SETTINGS;
		return sizeof(SETTINGS) / sizeof(*SETTINGS);
	}

	std::chrono::seconds get_retry_after()
	{
		int32_t retryAfterSecs;
		VS_RegistryKey rKey(false, CONFIGURATION_KEY);
		if (rKey.GetValue(&retryAfterSecs, sizeof(retryAfterSecs), VS_REG_INTEGER_VT, SIP_RETRY_AFTER))
			return std::chrono::seconds(retryAfterSecs);

		return DEFAULT_RETRY_AFTER;
	}

	std::chrono::seconds get_reg_max_expires() noexcept
	{
		VS_RegistryKey key(false, SIP_PEERS_KEY);
		if(key.IsValid())
		{
			int32_t reg_max_expires;
			if (key.GetValue(&reg_max_expires, sizeof(reg_max_expires), VS_REG_INTEGER_VT, REGISTRATION_MAX_EXPIRES) > 0)
			{
				return std::chrono::seconds(reg_max_expires);
			}
		}
		return std::chrono::seconds::max();
	}

} //anonymous namespace

enum eSIPUserAgent : int
{
	Unknown, Lync
};

enum class e_ChatBotAnswer {
	not_recognized,
	user_busy
};


const bool VS_SIPParser::DEFAULT_SESSION_TIMERS_ENABLED = true; //for tcc2sip calls

VS_SIPParser::VS_SIPParser(boost::asio::io_service::strand& transportStrand, std::string userAgent, const std::shared_ptr<net::LoggerInterface> &logger)
	: m_get_app_property([](string_view) -> std::string { return std::string{}; })
	, m_transportStrand(transportStrand)
	, m_use_acl(true)
	, m_userAgent(std::move(userAgent))
	, m_regMaxExpires(get_reg_max_expires())
	, m_retryAfterHeader(get_retry_after())
	, m_logger(logger)
{
	m_acl.LoadACL(VS_NetworkConnectionACL::CONN_SIP);
	Connect_DialogFinished([](string_view dialogId)
	{
		dstream4 << "VS_SIPParser: dialog id = " << dialogId << "is finished.\n";
	});
}

VS_SIPParser::~VS_SIPParser()
{
	m_transaction_handler->Clear();

	std::vector<std::string> to_fire;

	m_ctx_lock.lock();
	for (auto it = m_ctx.begin(); it != m_ctx.cend(); ++it)
	{
		if (auto bfcp_channel = it->second->GetBFCPChannel())
			bfcp_channel->Close();
		to_fire.emplace_back(it->first);
	}
	m_ctx_connections.clear();
	m_ctx.clear();
	m_ctx_lock.unlock();

	m_ctx_garbage_lock.lock();
	for (auto it2 = m_ctx_garbage.cbegin(); it2 != m_ctx_garbage.cend(); ++it2)
	{
		to_fire.push_back(*it2);
	}
	m_ctx_garbage.clear();
	m_ctx_garbage_lock.unlock();


	for (auto it3 = to_fire.cbegin(); it3 != to_fire.cend(); ++it3)
	{
		m_fireDialogFinished(*it3);
	}
}

const VS_Policy::PolicySettingsInitInterface *VS_SIPParser::PolicySettings()
{
	static PolicySettingsSipInit settings{};
	return &settings;
}

CtxMsgPair VS_SIPParser::GetMsgForSend_SIP() {
	std::lock_guard<decltype(m_msg_queue_lock)> lck{ m_msg_queue_lock };
	if (!m_msg_queue_out.empty()) {
		auto msg = m_msg_queue_out.front();
		m_msg_queue_out.pop_front();

		if (msg.second && msg.first == nullptr) {
			msg.first = GetParserContext(msg.second->CallID());
		}
		return msg;
	}
	return std::make_pair(CtxMsgPair::first_type(), nullptr);
}

CtxMsgPair VS_SIPParser::GetMsgForSend_SIP(string_view dialogId) {

	std::lock_guard<decltype(m_msg_queue_lock)> lck{ m_msg_queue_lock };
	CtxMsgPair msg;
	for (auto it = m_msg_queue_out.cbegin(); it != m_msg_queue_out.cend(); ++it) {
		const string_view id = (*it).second->CallID();
		if (id == dialogId)
		{
			msg = (*it);
			m_msg_queue_out.erase(it);
			break;
		}
	}
	if (msg.second && msg.first == nullptr) {
		msg.first = GetParserContext(msg.second->CallID());
	}
	return msg;
}

CtxMsgPair VS_SIPParser::GetMsgForSend_SIP(const std::set<std::string, vs::str_less> &dialogIds)
{
	std::lock_guard<decltype(m_msg_queue_lock)> lck{ m_msg_queue_lock };
	CtxMsgPair msg;
	for (auto it = m_msg_queue_out.cbegin(); it != m_msg_queue_out.cend(); ++it)
	{
		const string_view id = (*it).second->CallID();
		if (std::find(dialogIds.cbegin(), dialogIds.cend(), id) != dialogIds.cend())
		{
			msg = (*it);
			m_msg_queue_out.erase(it);
			break;
		}
	}
	if (msg.second && msg.first == nullptr)
	{
		msg.first = GetParserContext(msg.second->CallID());
	}
	return msg;
}

CtxMsgPair VS_SIPParser::GetMsgForRetransmit_SIP()
{
	CtxMsgPair msg = m_transaction_handler->GetMessageForRetransmission();
	if (msg.second && msg.first == nullptr)
	{
		msg.first = GetParserContext(msg.second->CallID());
	}
	return msg;
}

bool VS_SIPParser::SetRecvMsg_SIP(const std::shared_ptr<VS_SIPMessage> &msg, const net::Endpoint& remoteEp)
{
	if (!msg)
	{
		return false;
	}

	if (msg->Branch().empty())
	{
		dprint3("in.GetBranch() return empty\n");
		return false;
	}

	auto meta = msg->GetSIPMetaField();
	if (!meta) {
		dprint3("in.GetSIPMetaField() return NULL\n");
		return false;
	}

	{
		if (IsACLUsed() && !m_acl.IsAllowed(remoteEp.addr))
		{

			dstream2 << "SIP ACL: " << remoteEp << " is not allowed. ACL enabled: "
				<< (IsACLUsed() ? "true" : "false") << ", mode: " <<
				(m_acl.GetMode() == VS_NetworkConnectionACL::ACL_NONE ? "none" :
				(m_acl.GetMode() == VS_NetworkConnectionACL::ACL_BLACKLIST ? "blacklist" : "whitelist"));
			return false;
		}
	}

	if (meta->iStartLine->GetMessageType() == MESSAGE_TYPE_RESPONSE)
		return OnResponseArrived(std::static_pointer_cast<VS_SIPResponse>(msg));
	return OnRequestArrived(std::static_pointer_cast<VS_SIPRequest>(msg), remoteEp);
}

bool VS_SIPParser::OnRequestArrived(const std::shared_ptr<VS_SIPRequest>& req, const net::Endpoint peerEp)
{
	dprint3("VS_SIPParser::OnRequestArrived()\n");
	if (!req || !req->IsValid())
		return false;

	auto meta = req->GetSIPMetaField();
	if (!meta || !meta->IsValid())
		return false;

	if (!meta->iStartLine || (meta->iStartLine->GetMessageType()!=MESSAGE_TYPE_REQUEST))
		return false;

	const int method = meta->iStartLine->GetRequestType();
	bool err = false;

	//TODO:FIXME(!!!maybe use address port)
	if (!m_policy->Request(peerEp.addr.to_string(vs::ignore<boost::system::error_code>()), "sip", method))
		return err;

	if (method != TYPE_ACK)
	{
		auto response = m_transaction_handler->GetResponse(req);
		if (response)
		{
			PutMessageForOutput(response);
			return true;
		}
	}

	switch (method)
	{
	case TYPE_INVITE:		err = OnRequest_Invite(req, peerEp);	break;		// INVITE or re-INVITE
	case TYPE_UPDATE:		err = OnRequest_Update(req, peerEp);	break;		// re-INVITE
	case TYPE_ACK:			err = OnRequest_ACK(req);			break;
	case TYPE_BYE:			err = OnRequest_BYE(req);			break;
	case TYPE_INFO:			err = OnRequest_INFO(req);			break;
	case TYPE_CANCEL:		err = OnRequest_CANCEL(req);		break;
	case TYPE_OPTIONS:		err = OnRequest_OPTIONS(req, peerEp);	break;
	case TYPE_REGISTER:		err = OnRequest_REGISTER(req, peerEp);	break;
	case TYPE_SUBSCRIBE:	err = OnRequest_SUBSCRIBE(req);		break;
	case TYPE_PUBLISH:		err = OnRequest_PUBLISH(req);		break;
	case TYPE_MESSAGE:		err = OnRequest_MESSAGE(req, peerEp);	break;
	default:                err = Request_Unsupported(req);     break;
	}

	return err;
}
bool VS_SIPParser::OnRequest_Update(const std::shared_ptr<VS_SIPRequest>& req, const net::Endpoint &recvFromEp) {
	dprint3("VS_SIPParser::OnRequest_Update()\n");

	if (!req || !req->IsValid())
		return false;

	auto ctx = GetParserContext(req->CallID());
	if (!ctx) return false;

	if (ctx->InCall())
		return OnRequest_Invite(req, recvFromEp);

		VS_SIPUpdateInfoImpl update_info(*ctx);
		const VS_SIPGetInfoImpl get_info(*ctx);
	auto rsp = std::make_shared<VS_SIPResponse>();
	if (!rsp->MakeOnUpdateResponseOK(req.get(), get_info, update_info) || !PutMessageForOutput(rsp, ctx))
	{
		return false;
	}
	return true;
}

bool VS_SIPParser::OnRequest_Invite(const std::shared_ptr<VS_SIPRequest>& req, const net::Endpoint &recvFromEp)
{
	dprint3("VS_SIPParser::OnRequest_Invite()\n");
	if (!req || !req->IsValid())
		return false;

	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return false;

	bool mayBeDublicate = false;

	auto ctx = GetParserContext(req->CallID(), true);
	if (!ctx)
		return false;

	{
		auto &&cfg = ctx->GetConfig();

		if (cfg.SignalingProtocol == VS_CallConfig::UNDEFINED)
		{
			cfg.SignalingProtocol = VS_CallConfig::SIP;
		}
	}

	if (ctx->GetRingingStartTick() != NULL_TICK && !ctx->IsAnswered())	// before visi answered
		return true;

	/*if (ctx->GetConfig()->Address.isZero())
	{
	ctx->SetConfig(CreateCallConfig(recvFrom, ""));
	}*/

	const VS_CallConfig &config = ctx->GetConfig();

	ctx->SIPDialogID(std::string(req->CallID()));
	ctx->SetLocalBandwidth(config.Bandwidth.get_value_or(0) * 1024);	// kbit/s
	if (config.sip.SessionTimers.Enabled.get_value_or(DEFAULT_SESSION_TIMERS_ENABLED))
		ctx->EnableSessionTimer();
	else
		ctx->DisableSessionTimer();

	if (config.sip.SessionTimers.AddToRequireHeader.get_value_or(true))
		ctx->AddSessionTimerToRequireHeader();
	else
		ctx->RemoveSessionTimerFromRequireHeader();

	if (config.sip.BFCPEnabled.get_value_or(true))
	{
		ctx->EnableBFCP();
		ctx->SetBFCPSupportedRoles(config.sip.BFCPRoles.get_value_or(SDP_FLOORCTRL_ROLE_C_S));
	}
	else
		ctx->DisableBFCP();

	ctx->SetH224Enable(config.H224Enabled.get_value_or(DEFAULT_FECC_ENABLED));

	VS_SIPUpdateInfoImpl update_info(*ctx);
	const VS_SIPGetInfoImpl get_info(*ctx);

	if (!req->FillInfoByInviteMessage(update_info))
		return false;

	// recalculate call config
	UpdateCallConfig(ctx, recvFromEp);

	/*VS_SimpleStr addr;
	//recvFrom.GetAddrString(addr);
	ctx->GetConfig()->Address.GetAddrString(addr);
	puts(addr.m_str);*/
	if (ctx->GetTimerExtention().refresher == REFRESHER::REFRESHER_UAC)
	{
		ctx->GetTimerExtention().lastUpdate = clock().now();
		ctx->GetTimerExtention().IsUpdating = true;
	}
	else if (ctx->GetTimerExtention().refresher != REFRESHER::REFRESHER_INVALID) {
		ctx->GetTimerExtention().lastUpdate = clock().now();
		ctx->GetTimerExtention().IsUpdating = false;
	}
	if (ctx->GetTimerExtention().refresher == REFRESHER::REFRESHER_UAS && ctx->IsSessionTimerEnabled()) {
		ctx->IsWeRefresh(true);
	}

	std::string call_to = ctx->GetAliasMy();
	bool is_tel_invite = false;
	auto sip_meta = req->GetSIPMetaField();
	if (sip_meta && sip_meta->IsValid())
	{
		VS_SIPURI* uri = sip_meta->iTo->GetURI();
		if (!call_to.empty() && !ctx->IsTel() && uri != nullptr && uri->IsValid() && uri->URIType() == SIPURI_TEL)
		{
			call_to = "#sip:" + uri->User();

			is_tel_invite = true;
			ctx->SetIsTel(true);
			{
				std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };
				for (auto &c : m_ctx)
				{
					auto voip_cfg = c.second->GetConfig();
					if (voip_cfg.UseAsTel && voip_cfg.IsFromRegistry && voip_cfg.sip.RegistrationBehavior != VS_CallConfig::eRegistrationBehavior::REG_DO_NOT_REGISTER
						&& voip_cfg.sip.RegistrationBehavior != VS_CallConfig::eRegistrationBehavior::REG_UNDEFINED)
					{
						call_to += "@";
						call_to += voip_cfg.HostName;
					}
				}
			}
		}
	}

	{
		string_view sv(call_to);
		auto pos = sv.find('@');
		if (pos != sv.npos)
		{
			sv.remove_suffix(sv.length() - pos);
			ctx->SetConfig(CreateCallConfig(recvFromEp, sv, ctx->GetUserAgent()));


			mayBeDublicate = !call_to.empty() ? (ctx->GetUser() == call_to || ctx->GetUser() == sv) : false;
			ctx->SetUser(std::string(sv));
		}
		else
		{
			mayBeDublicate = ctx->GetUser().empty()/* == nullptr*/;
		}
	}

	const auto &updated_config = ctx->GetConfig();
	if (updated_config.sip.RegistrationBehavior.get_value_or(VS_CallConfig::REG_UNDEFINED) == VS_CallConfig::REG_REGISTER_ALWAYS
		&& !updated_config.TcId.empty())
	{
		call_to = updated_config.TcId;
	}

	if (!is_tel_invite)
	{

		if (!strchr(call_to.c_str(), '@'))
		{
			call_to = DEFAULT_DESTINATION_CALLID_SIP;
		}
		else
		{
			const auto pos = call_to.find('@');
			if (!GetRegDialogIdsByUsername(string_view{ call_to.c_str(), (pos == std::string::npos ? call_to.length() : pos) }).empty())
				call_to = DEFAULT_DESTINATION_CALLID_SIP;
		}
	}
	VS_RegistryKey cfg(false, CONFIGURATION_KEY);
	if (cfg.IsValid())
	{
		std::string external_ip; // external sip ip from registry
		if (cfg.GetString(external_ip, "SIP External Host") && !external_ip.empty())
		{
			// if remote peer is local peer, then set local stream ip (not external NAT ip)
			auto sdp = req->GetSDPMetaField();
			if (sdp)
			{

				net::address peer_addr;

				VS_SDPField_Connection *c = sdp->iConnection;
				if (c)
				{
					// search for Connection at "Session description" block
					const std::string &host = c->GetHost();
					if (!host.empty())
						peer_addr = net::dns_tools::single_make_a_aaaa_lookup(host);
				}
				else
				{
					// search Connection at "Media description" block
					for (auto& ms : sdp->iMediaStreams)
					{
						const std::string &host = ms->GetHost();
						if (!host.empty())
						{
							peer_addr = net::dns_tools::single_make_a_aaaa_lookup(host);
							break;
						}
					}
				}

				if (!peer_addr.is_unspecified() && net::is_private_address(peer_addr))
					external_ip.clear();
			}

			if (!external_ip.empty())
				ctx->SetMyExternalCsAddress(std::move(external_ip));
		}
	}

	auto contact = sip_meta->iContact;
	if (contact) {
		ctx->SetSIPContact(contact);
	}

	{
		auto meta = req->GetSIPMetaField();
		if (!meta || (meta->iVia.empty()))
			return false;

		req->FillInfoByRequest(get_info, update_info);
	}

	ctx->SetLastInvite(req);

	AddAuthInfoFromRegContext(ctx, req);
	if (ctx->SRTPEnabled() && ctx->HaveAuthenticatedTLSConnection())
	{
		auto &cs_my_address = static_cast<const VS_SIPParserInfo &>(*ctx).GetMyCsAddress();
		ctx->SetSRTPKey(m_fireGetSRTPKey(ctx->SIPDialogID(), cs_my_address, updated_config.Address));
	}

	// ReINVITE
	if (ctx->IsAnswered())
	{
		if (req->FillInfoFromSDP(get_info, update_info, true))
			SendSetMediaChannels(ctx, VS_SIPParserInfo::SMCContinuation::SendResponse);
		else
		{
			auto rsp = std::make_shared<VS_SIPResponse>();
			if (!rsp->MakeOnInviteResponseOK(req.get(), get_info, update_info) || !PutMessageForOutput(rsp, ctx)) {
				return false;
			}
		}
		if (ctx->IsBFCPEnabled())
			InitBFCP(ctx);
		return true;
	}


	string_view user = ctx->GetAliasRemote();
	std::string from;
	if (!user.empty())
	{
		const auto pos = user.find('@');
		if (pos != string_view::npos)
		{
			user = user.substr(0, pos);
		}
		std::shared_ptr<VS_SIPParserInfo> ctx2 = GetParserRegisterContext(user);

		if (!ctx2)
			ctx2 = GetParserRegisterContext(ctx->GetAliasRemote());

		if (ctx2)
		{
			if (m_set_dialog_id_callback) m_set_dialog_id_callback(ctx2->SIPDialogID(), req->CallID());
		}

		from = ctx->GetAliasRemote();
		if (from.find('@') == std::string::npos)
			from.insert(0, "@");
		VS_AddSIPPrefix(from);
		if (ctx->GetConfig().UseAsTel)
		{
			from = "#tel:" + ctx->GetAliasRemote();
			if (from.find('@') != std::string::npos) from.erase(from.find('@'));
		}
	}

	if (ctx->IsTel())
	{
		auto a = req->GetAuthInfo();
		if (a == nullptr) // Unauthorized
		{
			// We are going to send a notifications to the call initiator (see below).
			ctx->SetAliasMy(call_to);
		}
		else
		{
			ctx->SetAliasMy(from);
		}
	}

	/* Auth */
	bool is_call_authorized = false;
	if (confMethods->GetLoggedTranscoder(from)) {
		auto a = req->GetAuthInfo();
		if (!!a) {
			if (!g_our_nonce_storage.IsOurNonce(a->nonce())) {
				dprint3("VS_SIPParser::OnRequest_INVITE 403, not our nonce\n");

				auto rsp = std::make_shared<VS_SIPResponse>();
				if (!rsp->MakeOnRegisterResponseForbidden(req.get(), get_info, update_info)) return false;
				return PutMessageForOutput(rsp);
			}

			std::string pass = std::string("INVITE:") + a->uri();
			char res[400];
			VS_ConvertToMD5(pass, res);
			pass = std::string() + "$4*" + a->nonce() + "*" + res + "*" + a->response();
			std::transform(pass.begin(), pass.end(), pass.begin(), ::tolower);

			VS_RealUserLogin r(a->login());
			if (!!m_check_digest && !m_check_digest(r.GetUser(), pass)) {
				dprint3("VS_SIPParser::OnRequest_INVITE Digest check failed\n");
				auto rsp = std::make_shared<VS_SIPResponse>();
				if (!rsp->MakeOnRegisterResponseForbidden(req.get(), get_info, update_info)) return false;
				return PutMessageForOutput(rsp);
			}
			is_call_authorized = true;
		}
		else {
			std::string new_nonce = VS_SIPAuthDigest::GenerateNonceValue();
			g_our_nonce_storage.AddOurNonce(new_nonce);

			auto rsp = std::make_shared<VS_SIPResponse>();
			if (!rsp->MakeOnInviteResponseUnauthorized(req.get(), get_info, update_info, new_nonce)) return false;
			if (!PutMessageForOutput(rsp)) return false;

			return true;
		}
	}

	/* \Auth */

	bool is_CallAllowed = mayBeDublicate || m_policy->Request(recvFromEp.addr.to_string(vs::ignore<boost::system::error_code>()) + "_incomming_invite", "sip", eStartLineType::TYPE_INVITE);
	{
		is_call_authorized = is_call_authorized || ctx->GetConfig().isAuthorized.get_value_or(false);
		//supress call from sip to sip/h323/rtsp;
		if (!is_call_authorized
			&& VS_IsRTPCallID(call_to))
		{
			dstream2 << "Call from " << ctx->GetAliasRemote() << "(" << recvFromEp << ")" << " to " << call_to << " suppressed. Call from sip to third-party protocol (sip/h323/rtsp)\n";
			is_CallAllowed = false;
		}
	}
	if (!is_CallAllowed)
	{
		AsyncInviteResult(false, ConferenceStatus::UNDEFINED, ctx, {});
	} else
	{
		//100 Trying
		{
			auto resp_100_try = std::make_shared<VS_SIPResponse>();

			if (!resp_100_try->MakeOnInviteResponseTrying(get_info, update_info))
				return false;

			const auto res = PutMessageForOutput(resp_100_try);
			assert(res);
		}

		const auto *sdp = req->GetSDPMetaField();
		if (sdp && !sdp->iMediaStreams.empty()) {
			const auto *ms = sdp->iMediaStreams[0];
			if (ms->GetMediaType() == SDPMediaType::message) {
				ctx->SetAnswered(clock().now());

				ctx->SetAcceptedTextTypesRemote(req->GetAcceptedTextFormats());
				InitMSGMediaStream(ctx);

				auto resp = std::make_shared<VS_SIPResponse>();
				resp->MakeOnInviteResponseOK(req.get(), get_info, update_info);

				ctx->IncreaseMySequenceNumber();

				return PutMessageForOutput(resp);
			}
		}

		ctx->SetDisplayNameSip((req->IsValid() &&
			req->GetSIPMetaField() && req->GetSIPMetaField()->IsValid() &&
			req->GetSIPMetaField()->iFrom && req->GetSIPMetaField()->iFrom->IsValid() &&
			req->GetSIPMetaField()->iFrom->GetURI()) ? req->GetSIPMetaField()->iFrom->GetURI()->Name() : std::string{});

		if (ctx->GetDisplayNameSip().empty())
			ctx->SetDisplayNameSip(ctx->GetAliasRemote());
		UpdateSIPDisplayName(ctx, false);

		bool isSFB = IdentifyUserAgent(ctx) == Lync;
		if (isSFB && req->GetSIPMetaField()->iStartLine->GetRequestType() != TYPE_UPDATE &&
			boost::iequals(call_to, string_view(DEFAULT_DESTINATION_CALLID_SIP)) &&
			m_get_app_property("default_call_destination_sip").empty()) {
			// cancel call
			auto rsp = std::make_shared<VS_SIPResponse>();
			if (!rsp->MakeOnInviteResponseBusyHere(get_info, update_info) || !PutMessageForOutput(rsp)) {
				return false;
			}

			// send hint to chat
			auto dialog_id = NewDialogID(ctx->GetSIPRemoteTarget(), {}, ctx->GetConfig());
			auto msg_ctx = GetParserContext(dialog_id, true);

			if (ctx->RTFSupported()) {
				if (CHAT_CTRLS_HELP_RTF.empty()) CHAT_CTRLS_HELP_RTF = MakeChatBotHelpMSG(CONTENTTYPE_TEXT_RTF);
				msg_ctx->AddPengingMessage(CHAT_CTRLS_HELP_RTF, CONTENTTYPE_TEXT_RTF);
			}
			else {
				if (CHAT_CTRLS_HELP_PLAIN_TEXT.empty()) CHAT_CTRLS_HELP_PLAIN_TEXT = MakeChatBotHelpMSG(CONTENTTYPE_TEXT_PLAIN);
				msg_ctx->AddPengingMessage(CHAT_CTRLS_HELP_PLAIN_TEXT, CONTENTTYPE_TEXT_PLAIN);
			}
			msg_ctx->SetAliasMy(ctx->GetAliasMy());
			msg_ctx->SetEpidSip(ctx->GetEpidSip());
			AuthenticateAndSendInstantMessage(msg_ctx, 0);

		}
		else {
			bool force_create = false;
			auto target = ctx->GetSIPRemoteTarget();
			if (!target.empty())
			{
				std::lock_guard<decltype(m_ctx_lock)> _{ m_ctx_lock };
				for (const auto &ctx_ : m_ctx) {
					if (ctx_.second && ctx != ctx_.second) {
						auto current_target = ctx_.second->GetSIPRemoteTarget();
						if (boost::iequals(target, current_target) && ctx_.second->GetAnswered() != std::chrono::steady_clock::time_point()) {
							force_create = true;
							break;
						}
					}
				}
			}

			gw::Participant fromPart(from, isSFB);
			confMethods->AsyncInvite(req->CallID(), fromPart, call_to, VS_ConferenceInfo(false, false),
				boost::bind(&VS_SIPParser::AsyncInviteResult,
					std::static_pointer_cast<VS_SIPParser>(shared_from_this()), _1, _2, ctx, _3), ctx->GetDisplayNameSip(), true, force_create);
		}
	}

	return true;
}

bool VS_SIPParser::OnRequest_BYE(const std::shared_ptr<VS_SIPRequest>& req)
{
	auto ctx = GetParserContext(req->CallID());
	if (!ctx)
		return false;
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return false;

	ctx->SetByeTick(clock().now());

	VS_SIPGetInfoImpl get_info(*ctx);
	VS_SIPUpdateInfoImpl update_info(*ctx);

	req->FillInfoByRequest(get_info, update_info);
	auto rsp = std::make_shared<VS_SIPResponse>();

	if (!rsp->MakeOnByeResponseOK(get_info, update_info))
		return false;
	if (!PutMessageForOutput(rsp)) {
		return false;
	}
	confMethods->Hangup(req->CallID());
	CleanParserContext(req->CallID(), SourceClean::PARSER);
	return true;
}

bool VS_SIPParser::OnRequest_ACK(const std::shared_ptr<VS_SIPRequest>& req)
{
	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(req->CallID());

	m_transaction_handler->ServerTransactionCompleted(req);

	if (!ctx)
		return false;

	if (ctx->GetAnswered() != NULL_TICK && !ctx->IsAnswered())
	{
		ctx->IsAnswered(true);
	}

	// ACK when we answered 486 Busy here (were no free transcoders and we set ByeTick)
	if (ctx->GetByeTick() != NULL_TICK)
	{
		CleanParserContext(req->CallID(), SourceClean::PARSER);
		return true;
	}

	VS_SIPUpdateInfoImpl sip_update(*ctx);

	if (req->FillInfoFromSDP(VS_SIPGetInfoImpl(*ctx), sip_update, false))
		SendSetMediaChannels(ctx, VS_SIPParserInfo::SMCContinuation::Nothing);
	if (ctx->IsBFCPEnabled())
		InitBFCP(ctx);
	return true;
}

bool VS_SIPParser::OnRequest_INFO(const std::shared_ptr<VS_SIPRequest>& req)
{
	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(req->CallID());
	if (!ctx)
		return false;
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return false;

	VS_SIPUpdateInfoImpl update_info(*ctx);
	const VS_SIPGetInfoImpl get_info(*ctx);

	if (req->IsFastUpdatePicture())
	{
		confMethods->FastUpdatePicture(req->CallID());

		auto rsp = std::make_shared<VS_SIPResponse>();
		if (!rsp->MakeOnInfoResponseOK(req.get(), get_info, update_info))
			return false;
		if (!PutMessageForOutput(rsp))
			return false;
	}
	else if (req->GetContentType() == CONTENTTYPE_BFCP && ctx->IsBFCPEnabled())
	{
		auto rsp = std::make_shared<VS_SIPResponse>();
		if (!rsp->MakeOnInfoResponseOK(req.get(), get_info, update_info))
			return false;
		if (!PutMessageForOutput(rsp))
			return false;
	}
	// unsupported Content-Type
	else
	{
		auto rsp = std::make_shared<VS_SIPResponse>();
		if (!rsp->MakeOnInfoResponseOK(req.get(), get_info, update_info))
			return false;
		if (!PutMessageForOutput(rsp))
			return false;
	}
	return true;
}


bool VS_SIPParser::OnRequest_OPTIONS(const std::shared_ptr<VS_SIPRequest>& req, const net::Endpoint &recvFromEp)
{
	auto ctx = GetParserContext(req->CallID(), true);
	if (!ctx)
		return true;

	VS_SIPUpdateInfoImpl update_info(*ctx);
	const VS_SIPGetInfoImpl get_info(*ctx);
	req->FillInfoByInviteMessage(update_info);

	// recalculate call config
	UpdateCallConfig(ctx, recvFromEp);

	auto res = std::make_shared<VS_SIPResponse>();
	bool r = res->MakeOnOptionsResponseOK(req.get(), get_info, update_info) && PutMessageForOutput(res, ctx);
	return r;
}

bool VS_SIPParser::OnResponseArrived(const std::shared_ptr<VS_SIPResponse>& rsp)
{
	dprint3("VS_SIPParser::OnResponseArrived()\n");
	if (!rsp || !rsp->IsValid())
		return false;

	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return false;

	auto sip_meta = rsp->GetSIPMetaField();
	if (!sip_meta || !sip_meta->IsValid())
		return false;

	if (!sip_meta->iStartLine || (sip_meta->iStartLine->GetMessageType() != MESSAGE_TYPE_RESPONSE))
		return false;

	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(rsp->CallID());
	if (!ctx)
		return false;
	if (rsp->GetSIPMetaField()->iCSeq->GetType() != TYPE_REGISTER)
	{
		if (!sip_meta->iTo)
			return false;

		VS_SIPURI* uri = sip_meta->iTo->GetURI();
		if (!uri || !uri->IsValid())
			return false;

		ctx->SetTagSip(uri->Tag());
		ctx->SetEpidSip(uri->Epid());
	}

	const std::string &call_id_1 = ctx->SIPDialogID();
	const std::string &call_id_2 = sip_meta->iCallID->Value();

	if (call_id_1.empty() || call_id_2.empty())
		return false;

	if (strcasecmp(call_id_1.c_str(), call_id_2.c_str()) != 0)
		return false;

	const int code = sip_meta->iStartLine->GetResponseCode();
	const int code_class = sip_meta->iStartLine->GetResponseCodeClass();
	const int message_type = sip_meta->iCSeq->GetType();

	if (message_type == TYPE_INVITE && code_class == 2 && ctx->GetByeTick() != NULL_TICK)
		return true;

	bool duplicatedOrUnknownResponse = false;
	{auto locked_transaction_handler = m_transaction_handler.lock();

	duplicatedOrUnknownResponse = !locked_transaction_handler->IsActiveClientTransaction(rsp);
	if (!duplicatedOrUnknownResponse) {
		if (code_class != 1)
			locked_transaction_handler->ClientTransactionCompleted(rsp);
		else if (message_type == TYPE_INVITE)
			locked_transaction_handler->ClientTransactionProceeding(rsp);
	}
	} /* end of lock section*/

	VS_SIPUpdateInfoImpl update_info(*ctx);
	const VS_SIPGetInfoImpl get_info(*ctx);

	if (!duplicatedOrUnknownResponse && code_class == 2 && !ctx->IsDialogEstablished()
		&& rsp->GetSIPMetaField()->iCSeq->GetType() != TYPE_REGISTER) // dialog established
	{
		rsp->FillUriSetForEstablishedDialog(get_info, update_info);
		ctx->DialogEstablished();
	}

	if (message_type == TYPE_INVITE && code_class != 1)
	{
		if (!duplicatedOrUnknownResponse)
		{
			auto req = std::make_shared<VS_SIPRequest>();
			//VS_SIPRequest req;
			if (!req->MakeACK(get_info, update_info, code_class == 2))
				return false;
			if (rsp->GetConnectionType() == net::protocol::UDP || code_class == 2)
			{
				//std::string mess;
				//if (req->Encode(mess) == e_ok)
				//{
				m_transaction_handler->AckTransaction(rsp->Branch(), req);
				//}
			}
			if (!PutMessageForOutput(req, ctx))
				return false;
		}
		else
		{
			auto mess = m_transaction_handler->GetAck(std::string(rsp->Branch()));

			if (mess) {
				PutMessageForOutput(mess);
			}
		}
	}

	auto reject_call = [&]()
	{
		if (code_class == 3 || // 3xx Redirection
		    code_class == 4 || // 4xx Client Failure
		    code_class == 5 || // 5xx Server Failure
		    code_class == 6)   // 6xx Global Failure
		{
			confMethods->InviteReplay(rsp->CallID(), e_call_rejected, false, {}, {});
			if (ctx->IsAnswered())
				ctx->SetByeTick(clock().now());
			else
				CleanParserContext(rsp->CallID(), SourceClean::PARSER);
		}
	};

	if (duplicatedOrUnknownResponse)
		dstream4 << "Discarding SIP response code=" << code << ", dialog=" << ctx->SIPDialogID() << ". Duplicate or unknown response!\n";

		if (!duplicatedOrUnknownResponse)
			switch (code)
			{
			case 180:	// Ringing
			{
				ctx->SetRingingStartTick(clock().now());
				// save user agent
				if (rsp->GetSIPMetaField()->iUserAgent && rsp->GetSIPMetaField()->iUserAgent->IsValid())
				{
					auto &user_agent = rsp->GetSIPMetaField()->iUserAgent->GetUserAgent();
					ctx->SetUserAgent(user_agent);
					// recalculate call config
					{
						UpdateCallConfig(ctx, ctx->GetConfig().Address);
					}
				}
				if (sip_meta->iContact && sip_meta->iContact->GetLastURI() && !sip_meta->iContact->GetLastURI()->Name().empty())
				{
					ctx->SetDisplayNameSip(sip_meta->iContact->GetLastURI()->Name());
					if (ctx->GetDisplayNameSip().empty())
						ctx->SetDisplayNameSip(ctx->GetAliasRemote());
					UpdateSIPDisplayName(ctx, true);
				}
			}
			break;
			case 200:	// OK
			{
				OnResponse_Code200(rsp);
			}
			break;
			case 401:	// Unauthorized
			case 407:	// Proxy Authentication Required
			{
				std::shared_ptr<VS_SIPAuthInfo> rsp_auth_info = rsp->GetAuthInfo();
				std::string rsp_nonce;
				std::string ctx_nonce;

				if (!!rsp_auth_info)
					rsp_nonce = rsp_auth_info->nonce();

				if (ctx->GetAuthScheme())
					ctx_nonce = ctx->GetAuthScheme()->nonce();

				if (!rsp_nonce.empty() && ctx_nonce != rsp_nonce) {
					auto &&login = ctx->GetUser();
					auto &&pass = ctx->GetPassword();
					auto &&to_domain = ctx->GetDomain();
					std::string auth_name = login;
					if (!ctx->GetConfig().sip.AuthName.empty()) {
						auth_name = ctx->GetConfig().sip.AuthName;
					}

					if (login.empty() || pass.empty() || to_domain.empty()) {
						if (message_type == TYPE_INVITE)
							confMethods->InviteReplay(rsp->CallID(), e_call_rejected, false, {}, {});
						CleanParserContext(rsp->CallID(), SourceClean::PARSER);
						return false;
					}
					auto digest_info = std::make_shared<VS_SIPAuthDigest>();
					digest_info->login(std::move(auth_name));
					digest_info->password(pass);
					digest_info->realm(rsp_auth_info->realm());
					digest_info->nonce(rsp_auth_info->nonce());
					if (rsp_auth_info->qop() > SIP_AAA_QOP_INVALID) {
						digest_info->cnonce("71cd2326969c");
						digest_info->nc(1);
					}
					digest_info->method(rsp_auth_info->method());
					digest_info->qop(rsp_auth_info->qop());
					digest_info->algorithm(rsp_auth_info->algorithm());
					digest_info->opaque(rsp_auth_info->opaque());
					digest_info->uri("sip:" + ctx->GetAliasRemote());
					if (code == 401)
						digest_info->auth_type(VS_SIPAuthInfo::TYPE_AUTH_USER_TO_USER);
					else
						digest_info->auth_type(VS_SIPAuthInfo::TYPE_AUTH_PROXY_TO_USER);

					ctx->SetAuthScheme(digest_info);
					ctx->SetTagSip({});
					ctx->SetAliasMy((std::string(login) += "@") += to_domain);

					auto req = std::make_shared<VS_SIPRequest>();
					if (sip_meta->iCSeq->GetType() == TYPE_REGISTER) {
						if (!req->MakeREGISTER(get_info, update_info, code == 401 ? VS_SIPObjectFactory::SIPHeader::SIPHeader_Authorization : VS_SIPObjectFactory::SIPHeader::SIPHeader_ProxyAuthorization))
							return false;
					}
					else
						if (!req->MakeINVITE(get_info, update_info)) 	return false;

					if (!PutMessageForOutput(req)) {
						return false;
					}
				}
				else {
					string_view term_id_view = rsp->Server();
					if (!term_id_view.empty()) {
						std::string term_id(term_id_view);
						VS_CallConfigCorrector::GetInstance().CorrectCallConfig(ctx->GetConfig(), VS_CallConfig::SIP, term_id.c_str());
					}

					auto &&exec = [](VS_CallConfig &cfg)
					{
						auto call_config_manager = create_call_config_manager(cfg);
						if (call_config_manager.NeedVerification())
						{
							call_config_manager.SetForbiddenVerification();
						}
					};

					for (VS_SIPField_Auth *auth_header : rsp->GetSIPMetaField()->iAuthHeader) {
						auto scheme = auth_header->scheme();
						if (scheme == SIP_AUTHSCHEME_NTLM) {
							auto auth = std::dynamic_pointer_cast<VS_SIPAuthGSS>(auth_header->GetAuthInfo());

							auto &&config = ctx->GetConfig();

							if (!ctx->secureCtx) {
								ctx->secureCtx = std::make_shared<VS_RegCtx>(VS_SIPSeqSSPI::NTLM);

								const std::string &login = ctx->GetUser();
								const std::string &pass = ctx->GetPassword();
								std::string from_domain = ctx->GetDomain();
								if (!config.sip.FromDomain.empty())
								{
									from_domain = config.sip.FromDomain;
								}

								auto gss_info = std::make_shared<VS_SIPAuthGSS>(SIP_AUTHSCHEME_NTLM);
								gss_info->login(login);
								gss_info->password(pass);
								gss_info->realm(auth->realm());
								gss_info->qop(SIP_AAA_QOP_AUTH);
								gss_info->gssapi_data("");

								gss_info->method(auth->method());
								gss_info->opaque(auth->opaque());
								gss_info->targetname(auth->targetname());
								gss_info->version(4);
								gss_info->auth_type(code == 401 ? VS_SIPAuthInfo::TYPE_AUTH_USER_TO_USER
									: VS_SIPAuthInfo::TYPE_AUTH_PROXY_TO_USER);

								std::string self_uri = login + '@' + from_domain;

								ctx->SetAuthScheme(gss_info);
								ctx->SetAliasMy(self_uri);
								ctx->SetAliasRemote(std::move(self_uri));


							std::string auth_login;
							if (config.sip.AuthName.empty())
								auth_login = std::string(login);
							else
								auth_login = config.sip.AuthName;

							std::string auth_domain;
							if (config.sip.AuthDomain.empty()) {
								const auto pos = auth_login.find('@');
								if (pos != std::string::npos) {
									auth_domain = auth_login.substr(pos + 1, std::string::npos);
									auth_login.erase(pos, std::string::npos);
								} else {
									auth_domain = std::string(from_domain);
								}
							}
							else
								auth_domain = config.sip.AuthDomain;

							dprint3("Use login='%s', domain='%s' for authentication\n", auth_login.c_str(), auth_domain.c_str());
							if (!ctx->secureCtx->sspi.AcquireCred(auth_login.c_str(), std::string(pass).c_str(), auth_domain.c_str())) {
								dprint3("VS_SIPParser::OnResponseArrived: VS_SIPSeqSSPI::AcquireCred error\n");
								return false;
							}

								std::vector<unsigned char> out_buf;
								if (ctx->secureCtx->sspi.InitContext(nullptr, 0, out_buf) != VS_SIPSeqSSPI::ContinueNeeded) {
									dprint3("VS_SIPParser::OnResponseArrived: VS_SIPSeqSSPI::InitContext error\n");
									return false;
								}
							}
							else {
								auto ctx_auth = ctx->GetAuthScheme();
								if (!ctx_auth || ctx_auth->scheme() != SIP_AUTHSCHEME_NTLM) {
									return false;
								}

								auto& gss_data = auth->gssapi_data();
								if (!gss_data.is_initialized()) {
									dprint3("VS_SIPParser::OnResponseArrived: Verification of security association failed(gss_data error). Cleaning context.\n");
									ctx->ReadyToDie(true);
									return false;
								}

								size_t out_len;
								base64_decode(gss_data->c_str(), gss_data->length(), nullptr, out_len);
								auto dec = vs::make_unique_default_init<unsigned char[]>(out_len);
								base64_decode(gss_data->c_str(), gss_data->length(), dec.get(), out_len);

								std::vector<unsigned char> out_buf;
								if (ctx->secureCtx->sspi.InitContext(dec.get(), out_len, out_buf) != VS_SIPSeqSSPI::Ok) {
									dprint3("VS_SIPParser::OnResponseArrived: VS_SIPSeqSSPI::InitContext error\n");
									return false;
								}

								base64_encode(&out_buf[0], out_buf.size(), nullptr, out_len);
								auto gss_resp = vs::make_unique_default_init<char[]>(out_len);
								base64_encode(&out_buf[0], out_buf.size(), gss_resp.get(), out_len);

								auto ctx_auth_gss = std::dynamic_pointer_cast<VS_SIPAuthGSS>(ctx_auth);
								if (ctx_auth_gss)
								{
									auto new_ctx_gss = std::make_shared<VS_SIPAuthGSS>(ctx_auth->scheme());
									*new_ctx_gss = *ctx_auth_gss;
									new_ctx_gss->opaque(auth->opaque());
									new_ctx_gss->gssapi_data(gss_resp.get(), out_len);
									new_ctx_gss->crand("0b5f113e");
									ctx->SetAuthScheme(new_ctx_gss);
								}
								ctx->secureCtx->get_incr_cnum();
							}

							auto req = std::make_shared<VS_SIPRequest>();
							if (sip_meta->iCSeq->GetType() == TYPE_REGISTER) {
								if (!req->MakeREGISTER(get_info, update_info, code == 401 ? VS_SIPObjectFactory::SIPHeader::SIPHeader_Authorization : VS_SIPObjectFactory::SIPHeader::SIPHeader_ProxyAuthorization)) {
									return false;
								}

								UpdateRegistrationConfig(create_call_config_manager(config).GetRegistrationIdentifierView(), [&exec](RegistrartionConfig &item)
								{
									exec(item.callConfig);
								});
							}

							return PutMessageForOutput(req);
						}
					}

					if (message_type == TYPE_INVITE)
						confMethods->InviteReplay(rsp->CallID(), e_call_rejected, false, {}, {});
					if (message_type == TYPE_REGISTER)
					{
						auto &&config = ctx->GetConfig();

						if (!UpdateRegistrationConfig(create_call_config_manager(config).GetRegistrationIdentifierView(), [&exec](RegistrartionConfig &item)
						{
							exec(item.callConfig);
							disable_registration_on_call(item.callConfig, item);
						}))
						{
							if (ctx->IsInviteAfterRegister())
							{
								exec(ctx->GetConfig());
							}
						}
					}
					CleanParserContext(rsp->CallID(), SourceClean::PARSER);
				}
			}
		break;
	case 486:	// Busy Here
	case 600:	// Busy Everywhere
		if (!NeedToRetryCall(rsp)) {
			CleanParserContext(rsp->CallID(), SourceClean::PARSER);
			confMethods->InviteReplay(rsp->CallID(), e_call_busy, false, {}, {});
		}
		break;
	case 403: // AccessReject
	case 405: // Method Not Allowed
		if (message_type == TYPE_INVITE)
		{
			confMethods->InviteReplay(rsp->CallID(), e_call_rejected, false, {}, {});
		}
		else if (message_type == TYPE_REGISTER)
		{
			if (ctx->IsInviteAfterRegister().get())
			{
				auto i = ctx->IsInviteAfterRegister();
				ctx->SetInviteAfterRegister( std::shared_ptr<VS_SIPParserInfo>() );

				auto req = std::make_shared<VS_SIPRequest>();

				const VS_SIPGetInfoImpl get_info_i{*i};
				VS_SIPUpdateInfoImpl update_info_i{*i};

				if (!(req->MakeINVITE(get_info_i, update_info_i) && PutMessageForOutput(req)))
					CleanParserContext(i->SIPDialogID(), SourceClean::PARSER);
			}


			auto &&config = ctx->GetConfig();
			{
				auto &&exec = [](VS_CallConfig &cfg, unsigned int code)
				{
					auto call_config = create_call_config_manager(cfg);

					if (call_config.NeedVerification())
					{
						if (code == 403)
						{
							call_config.SetForbiddenVerification();
						}
						else
						{
							call_config.SetCanNotCheckVerification();
						}
					}
				};
				if (!UpdateRegistrationConfig(create_call_config_manager(config).GetRegistrationIdentifierView(), [code, &exec](RegistrartionConfig &item)
				{
					exec(item.callConfig, code);
					disable_registration_on_call(item.callConfig, item);
				}))
				{
					if (ctx->IsInviteAfterRegister())
					{
						exec(ctx->GetConfig(), code);
					}
				}
			}
		}

		CleanParserContext(rsp->CallID(), SourceClean::PARSER);
		break;
			case 481: // Transaction does not exist
				CleanParserContext(rsp->CallID(), SourceClean::PARSER);
				break;
			case 422:
				// just make period bigger (actually new value should be read from req->iMin_SE)
				ctx->GetTimerExtention().refreshPeriod *= 2;
				if (std::chrono::duration_cast<std::chrono::seconds>(ctx->GetTimerExtention().refreshPeriod).count() > 90 * 1024)
				{
					CleanParserContext(rsp->CallID(), SourceClean::PARSER);
				}
				else
				{
					auto r = std::make_shared<VS_SIPRequest>();;
					if (r->MakeRefreshINVITE(get_info, update_info))
						PutMessageForOutput(r);
				}
				break;
			case 423: // Interval Too Brief

				if (rsp->GetSIPMetaField() && rsp->GetSIPMetaField()->iMinExpires
					&& rsp->GetSIPMetaField()->iMinExpires->GetValue() > ctx->GetExpires()
					&& rsp->GetSIPMetaField()->iCSeq && rsp->GetSIPMetaField()->iCSeq->GetType() == TYPE_REGISTER)
				{
					ctx->SetExpires(rsp->GetSIPMetaField()->iMinExpires->GetValue());
					auto r = std::make_shared<VS_SIPRequest>();
					if (r->MakeREGISTER(get_info, update_info, VS_SIPObjectFactory::SIPHeader::SIPHeader_Authorization))
						PutMessageForOutput(r);
				}
				else
				{
					CleanParserContext(rsp->CallID(), SourceClean::PARSER);
				}

				break;
			case 300:
			case 305:
			case 301:
			case 302:
			{
				auto call_ctx = GetParserContext(rsp->CallID());
				if (call_ctx) {
					std::vector<std::string> contact_URIs;
					rsp->CopyAllContacts(contact_URIs);
					call_ctx->InsertCallIdsToRedirect(contact_URIs);
			if (!call_ctx->HaveAddressesToRedirect())
			{
				reject_call();
				break;
			}
					call_ctx->NeedAddressRedirection(true);
					call_ctx->SetResponseCode(code);
					break; // next step in Timeout
				}
			} VS_FALLTHROUGH;
			// codes where we expect Retry-After
			// case 603: DECLINE (see bug#45379)
			case 404:
			case 413:
			case 503:
			case 480:
			{	// Temporarily unaviable
				if (NeedToRetryCall(rsp)) break;	// do not go to default if retry-after needed
				VS_FALLTHROUGH;
			}
			case 487: // Request Terminated
			{
				auto req = std::make_shared<VS_SIPRequest>();

				if (!req->MakeACK(get_info, update_info, false))
					break;
				if (rsp->GetConnectionType() == net::protocol::UDP) {
					m_transaction_handler->AckTransaction(rsp->Branch(), req);
				}
				PutMessageForOutput(req, ctx);
			} break;
			default:
			if ((code == 415 || code == 500) && message_type == TYPE_INFO)
			{
				// Ignore some non-critical error responses to INFO:
				//   * Huawei MC850 answers 415 "Unsupported Media Type" to "Picture Fast Update". (bug14265)
				//   * Blink answers 500 "Unhandled by dialog usages" to "Picture Fast Update".
				break;
			}

			{
				// we can try another address if call with current address have failed
				auto call_ctx = GetParserContext(rsp->CallID());
				if (call_ctx && call_ctx->HaveAddressesToRedirect()) {
					call_ctx->NeedAddressRedirection(true);
					break; // do not cancel call, next step in Timeout
				}
			}

		reject_call();
	}

	return true;
}

bool VS_SIPParser::InviteReplay(string_view dialogId, VS_CallConfirmCode confirmCode, bool isGroupConf, string_view confName, string_view to_displayName)
{
	dstream3 << "VS_SIPParser::InviteReplay(" << dialogId << "): " << confirmCode;
	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(dialogId);
	if (!ctx)
		return false;

	VS_SIPUpdateInfoImpl update_info(*ctx);
	const VS_SIPGetInfoImpl get_info(*ctx);

	if (confirmCode != e_call_ok)
	{
		std::shared_ptr<VS_SIPMessage> answer = nullptr;
		bool res = false;
		if (ctx->CreatedByChatBot()) {	// when we was invited via chat bot, we must finish dialog by request BYE, because invite was send by us
			auto req = std::make_shared<VS_SIPRequest>();
			res = req->MakeBYE(get_info, update_info);
			answer = req;
			this->NotificateAboutReject(ctx);
		}
		else {
			auto rsp = std::make_shared<VS_SIPResponse>();

			// "Busy here" for other situations
			if (confirmCode == e_call_busy)     res = rsp->MakeOnInviteResponseBusyHere(get_info, update_info);
			else if (confirmCode == e_call_rejected) res = rsp->MakeOnInviteResponseBusyHere(get_info, update_info);
			answer = rsp;
		}
		if (!res)
			return false;

		if (!PutMessageForOutput(answer))
			return false;
		ctx->SetByeTick(clock().now());
		return true;
	}

	if (ctx->IsPublicConf() && !confName.empty()) {
		auto msg_ctx = FindActiveMsgCtx(ctx);
		if (!msg_ctx)	return false;

		std::string m = "Webinar ";
		if (!ctx->GetConfTopic().empty()) {
			m += "with name '";
			m += ctx->GetConfTopic();
			m += "' ";
		}
		m += "was created. You can share following URL for webinar users: ";
		m += m_get_app_property("site_url");
		m += "/c/";
		m += confName;

		auto msg = std::make_shared<VS_SIPRequest>();

		VS_SIPUpdateInfoImpl update_info_msg(*msg_ctx);
		const VS_SIPGetInfoImpl get_info_msg(*msg_ctx);

		msg->MakeMESSAGE(get_info_msg, update_info_msg, m);
		PutMessageForOutput(msg);

		return true;
	}

	auto invite = ctx->GetLastInvite();
	if (!invite)
	{
		dprint1("SIP INVITE missing in InviteReplay\n");
		return false;
	}

	if (invite->GetContentType() == CONTENTTYPE_SDP)
	{
		invite->FillInfoFromSDP(get_info, update_info, true);
	}
	else
		InitMediaStreams(ctx);
	if (ctx->IsBFCPEnabled())
		InitBFCP(ctx);

	if (!to_displayName.empty())
		ctx->SetDisplayNameMy(std::string(to_displayName));
	ctx->SetGroupConf(isGroupConf);
	if (isGroupConf)
		ctx->LimitH264Level(video_presets::max_h264_level_groupconf); //bug#57248: restrict H264 level when received first INVITE from terminal that tries join our conf
	SendSetMediaChannels(ctx, VS_SIPParserInfo::SMCContinuation::SendResponse);
	return true;
}

bool VS_SIPParser::InviteMethod(string_view dialogId, string_view fromId, string_view toId, const VS_ConferenceInfo &i,
	string_view dnFromUTF8, bool newSession, bool forceCreate)
{
	dstream3 << "VS_SIPParser::InviteMethod(dialog:" << dialogId << ", from:" << fromId << ", to:" << toId << ")";

	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(dialogId);
	if (!ctx) return false;

	/*unsigned long ipv4;
	in6_addr ipv6;
	unsigned short port;
	eConnectionType connType;*/

	ctx->DialogEstablished(false);	// for cases where we make reinvite
	auto &address = (static_cast<const VS_SIPParserInfo &>(*ctx)).GetMyCsAddress();
	if (address.addr.is_unspecified())
	{
		ctx->SetMyCsAddress(m_myCsEp);
	}

	std::string alias,
				user,
				domain,
				password;

	std::string from_host = GetFromHost();
	const VS_CallConfig &config = ctx->GetConfig();
	if (!config.Login.empty()) {
		user = config.Login;
		domain = config.HostName;
		password = config.Password;

		alias = user + '@' + domain;
	}
	else if (!config.sip.FromDomain.empty()) {
		user = config.sip.FromUser;
		domain = config.sip.FromDomain;

		if (user.empty()) {
			user = std::string(fromId);
			const auto at_pos = user.find('@');
			if (at_pos != std::string::npos) {
				user.erase(at_pos);
			}
			else {
				user.clear();
			}
		}

		alias = user + '@' + domain;
	}
	else if (!from_host.empty()) {
		alias = std::string(fromId);
		size_t atPos;
		if ((atPos = alias.find('@')) != std::string::npos) {
			alias.erase(atPos);
			alias += '@';
			alias += from_host;
		}
		else {
			alias = from_host;
		}

		user = std::string(fromId);
		atPos = user.find('@');
		if (atPos != std::string::npos) {
			user.erase(atPos);
		}
		else {
			user.clear();
		}

		domain.clear();
		password.clear();
	}
	else {
		std::string from_id(fromId);
		VS_RealUserLogin from(from_id);
		user = from.GetUser();
		domain = std::string(VS_RemoveServerType(g_tr_endpoint_name));

		if (domain.empty()) domain = config.HostName;

		if (strchr(from_id.c_str(), '@') == nullptr) {
			user = "Administrator";
		}

		if (!domain.empty()) {
			alias = user + '@' + domain;
		}
		else {
			alias = std::move(from_id);
		}
	}

	ctx->SetAliasMy(std::move(alias));
	ctx->SetUser(std::move(user));
	ctx->SetDomain(std::move(domain));
	ctx->SetPassword(std::move(password));

	AddAuthInfoFromRegContext(ctx, nullptr);

	if (ctx->GetMyCsAddress().protocol == net::protocol::TCP && config.sip.IsKeepAliveSendEnabled.get_value_or(false))
		ctx->EnableKeepAlive();

	if (config.sip.SessionTimers.Enabled.get_value_or(DEFAULT_SESSION_TIMERS_ENABLED))
	{
		ctx->EnableSessionTimer();
		ctx->UseSessionTimer();
		ctx->GetTimerExtention().refresher = REFRESHER::REFRESHER_UAC;
		if (config.sip.SessionTimers.RefreshPeriod.is_initialized())
			ctx->GetTimerExtention().refreshPeriod = std::chrono::seconds(config.sip.SessionTimers.RefreshPeriod.get());
		else
			ctx->GetTimerExtention().refreshPeriod = std::chrono::seconds(1800);
		ctx->GetTimerExtention().IsUpdating = false;
		ctx->GetTimerExtention().lastUpdate = clock().now();
		ctx->IsWeRefresh(true);
	}
	else {
		ctx->UnuseSessionTimer();
		ctx->DisableSessionTimer();
	}

	if (ctx->GetTimerExtention().refresher == REFRESHER::REFRESHER_UAC && ctx->IsSessionTimerUsed())
		ctx->IsWeRefresh(true);

	if (config.sip.BFCPEnabled.get_value_or(true))
	{
		ctx->EnableBFCP();
		ctx->SetBFCPSupportedRoles(config.sip.BFCPRoles.get_value_or(SDP_FLOORCTRL_ROLE_C_S));
	}
	else
		ctx->DisableBFCP();

	ctx->SetH224Enable(config.H224Enabled.get_value_or(DEFAULT_FECC_ENABLED));

	// no display name when from_id doesn't contain username part
	if (fromId.find('@') != string_view::npos)
	{
		ctx->SetDisplayNameMy(std::string(!dnFromUTF8.empty() ? dnFromUTF8 : fromId));
	}
	else
	{
		ctx->SetDisplayNameMy({});
	}

	string_view to(toId);
	if (!to.empty())
	{
		if (to.front() == '#')
		{
			const auto pos = to.find(':');
			if (pos != string_view::npos)
				to = to.substr(pos + 1);
		}

		const auto pos = to.find('/');
		if (pos != string_view::npos)
			to = to.substr(0, pos);
	}

	if (config.Address.protocol != net::protocol::UDP) {
		auto regCtx = GetTCPRegCxtByContact(to);
		if(regCtx)
			ctx->SetRegCtxDialogID(regCtx->SIPDialogID());
	}

	if (ctx->SRTPEnabled() && ctx->HaveAuthenticatedTLSConnection())
	{
		auto &cs_my_addr = static_cast<const VS_SIPParserInfo &>(*ctx).GetMyCsAddress();
		ctx->SetSRTPKey(m_fireGetSRTPKey(dialogId, cs_my_addr,
			config.Address));
	}

	InitMediaStreams(ctx);
	if (ctx->IsBFCPEnabled())
		InitBFCP(ctx);

	ctx->SetGroupConf(i.is_group_conf);
	if (ctx->IsGroupConf())
		ctx->LimitH264Level(video_presets::max_h264_level_groupconf);
	ctx->SetPublicConf(i.is_public_conf);

	SendSetMediaChannels(ctx, VS_SIPParserInfo::SMCContinuation::SendINVITE);

	return true;
}

bool VS_SIPParser::PutMessageForOutput(const std::shared_ptr<VS_SIPMessage> &msg, const std::shared_ptr<VS_SIPParserInfo> &ctx) {
	if (!msg) {
		return false;
	}

	std::lock_guard<decltype(m_msg_queue_lock)> lock{ m_msg_queue_lock };
	m_msg_queue_out.emplace_back(ctx, msg);

	return true;
}

bool VS_SIPParser::ProcessTransaction(const std::shared_ptr<VS_SIPParserInfo> &ctx, const std::shared_ptr<VS_SIPMessage> &msg)
{
	if (msg->Branch().empty())
		return false;

	if (msg->GetMessageType() == MESSAGE_TYPE_REQUEST && msg->GetMethod() != TYPE_ACK) {
		if (msg->GetConnectionType() == net::protocol::UDP) {
			m_transaction_handler->ClientTransactionUnreliable(msg, std::make_pair(ctx, msg), msg->CallID());
		}
		else
		{
			m_transaction_handler->ClientTransaction(msg);
		}
	}
	else if (msg->GetMessageType() == MESSAGE_TYPE_RESPONSE) {
		auto sip_meta = msg->GetSIPMetaField();
		bool startLine = sip_meta && sip_meta->IsValid() && sip_meta->iStartLine;
		int codeClass = startLine ? sip_meta->iStartLine->GetResponseCodeClass() : 0;

		if (msg->GetConnectionType() == net::protocol::UDP && msg->GetMethod() == TYPE_BYE)
			m_transaction_handler->DialogTerminated(msg->CallID());
		if (msg->GetConnectionType() == net::protocol::UDP || (msg->GetMethod() == TYPE_INVITE && codeClass == 2 && msg->GetConnectionType() != net::protocol::TLS)) {
			if (codeClass != 1)
				m_transaction_handler->ServerTransactionFinalResponse(msg, std::make_pair(ctx, msg), msg->CallID());
			else
				m_transaction_handler->ServerTransactionProceeding(msg, std::make_pair(ctx, msg));
		}
	}
	return true;
}

void VS_SIPParser::NotificateAboutReject(string_view dialogId)
{
	NotificateAboutReject(GetParserContext(dialogId));
}

void VS_SIPParser::NotificateAboutReject(const std::shared_ptr<VS_SIPParserInfo>& ctx) {
	if (!ctx || !ctx->CreatedByChatBot()) return;

	const auto chat_dialog = NewDialogID(ctx->GetAliasRemote(), {}, ctx->GetConfig());
	this->Chat(chat_dialog, ctx->GetAliasMy(), ctx->GetAliasRemote(), ctx->GetDisplayNameMy(), "Rejected");
}

void VS_SIPParser::Hangup(string_view dialogId)
{
	dstream3 << "VS_SIPParser::Hangup(" << dialogId << ")";
	auto ctx = GetParserContext(dialogId);
	if (!ctx)
	{
		m_fireDialogFinished(dialogId);
		return;
	}

	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;

	const VS_SIPGetInfoImpl get_info(*ctx);
	VS_SIPUpdateInfoImpl update_info(*ctx);

	if (ctx->IsDirection_ToSIP()) {
		if (ctx->IsAnswered()) {
			if (!ctx->IsByeSent())
			{
				auto req_bye = std::make_shared<VS_SIPRequest>();
				if (req_bye->MakeBYE(get_info, update_info))
					if (PutMessageForOutput(req_bye))
						ctx->ByeIsSent();
			}

			confMethods->Hangup(dialogId);
			if (ctx->GetByeTick() == NULL_TICK)
				ctx->SetByeTick(clock().now());
		}
		else {
			bool IsRegistrationCtx = false;
			if (!ctx->GetUser().empty())
			for (const auto &item : GetRegDialogIdsByUsername(ctx->GetUser()))
			{
				if (item.get_value_or({}) == ctx->SIPDialogID())
				{
					IsRegistrationCtx = true;
					break;
				}
			}

			if (IsRegistrationCtx)
			{
				CleanParserContext(dialogId, SourceClean::PARSER);
			}
			else
			{	// if (INVITE or already sent INVITE in RegestryStrategy=2).
				if (!ctx->IsInviteAfterRegister() || !ctx->IsInviteAfterRegister()->IsInviteAfterRegister())
				{
					auto req_cancel = std::make_shared<VS_SIPRequest>();
					if (req_cancel->MakeCANCEL(get_info, update_info))
					{
						//TransactionId id = {req_cancel->GetBranch(), TYPE_INVITE};
						auto branch = VS_SIPField_Via::make_request_branch(ctx->MyBranch());
						m_transaction_handler->RequestCancelled(ctx, branch, TYPE_INVITE);
						PutMessageForOutput(req_cancel);
					} else {
						// small hack: we could not MakeCANCEL, so no need to wait for the answer
						// so on next Timeout() dialog will be finished and transcoder freed.
						// It can be when call with pause_before_dial, but cancel before even sent an invite
						ctx->SetByeTick(clock().now() - CALL_BYE_TIMEOUT);
					}
				}
				// here should be other code. for instance e_call_not_answered.
				confMethods->InviteReplay(dialogId, e_call_none, false, {}, {});
				if (ctx->GetByeTick() == NULL_TICK)
					ctx->SetByeTick(clock().now());
			}
		}
	}
	else { // from sip call
		if (ctx->IsAnswered()) {
			if (!ctx->IsByeSent())
			{
				auto req_bye = std::make_shared<VS_SIPRequest>();
				if (req_bye->MakeBYE(get_info, update_info))
					if (PutMessageForOutput(req_bye))
						ctx->ByeIsSent();
			}
			confMethods->Hangup(dialogId);
			if (ctx->GetByeTick() == NULL_TICK)
				ctx->SetByeTick(clock().now());
		}
		else {
			if (ctx->GetByeTick() == NULL_TICK && ctx->GetMessageType() == TYPE_INVITE)
			{
				auto rsp = std::make_shared<VS_SIPResponse>();
				if (!rsp->MakeOnInviteResponseBusyHere(get_info, update_info) ||
					!PutMessageForOutput(rsp))
					return;
			}
			confMethods->Hangup(dialogId);
			CleanParserContext(dialogId, SourceClean::PARSER);
		}
	}
}

void VS_SIPParser::LoggedOutAsUser(string_view dialogId)
{
	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(dialogId);
	if (!ctx)
	{
		m_fireDialogFinished(dialogId);
	}
	// is there something to do?
}

void VS_SIPParser::FastUpdatePicture(string_view dialogId)
{
	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(dialogId);
	if (!ctx ||
		!ctx->IsAnswered() ||
		ctx->IsNoMoreFUP()) {
		return;
	}

	if (IdentifyUserAgent(ctx) == Lync) {
		return;
	}

	auto req = std::make_shared<VS_SIPRequest>();
	const VS_SIPGetInfoImpl get_info(*ctx);
	VS_SIPUpdateInfoImpl update_info(*ctx);

	if (!req->MakeINFO_FastUpdatePicture(get_info, update_info) ||
		!PutMessageForOutput(req)) {
		dstream3 << "VS_SIPParser::FastUpdatePicture failed to send INFO " << dialogId;
	}
}

bool VS_SIPParser::OnResponse_Code200(const std::shared_ptr<VS_SIPResponse>& rsp)
{
	if (!rsp || !rsp->IsValid())
		return false;

	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return false;

	auto sip_meta = rsp->GetSIPMetaField();
	if (!sip_meta || !sip_meta->IsValid())
		return false;

	int message_type = sip_meta->iCSeq->GetType();
	switch (message_type)
	{
	case TYPE_INVITE:
	{

		auto sdp_meta = rsp->GetSDPMetaField();
		if (!sdp_meta || !sdp_meta->IsValid())
			return false;
		std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(rsp->CallID());
		if (!ctx)
			return false;

		auto contact = sip_meta->iContact;
		if (contact)
			ctx->SetSIPContact(contact);

		// timer extension
		if (ctx->IsAnswered()
			&& ctx->GetTimerExtention().refresher != REFRESHER::REFRESHER_INVALID
			&& ctx->GetByeTick() == NULL_TICK)
		{

			auto sdp_meta = rsp->GetSDPMetaField();
			if (!sdp_meta || !sdp_meta->IsValid())
				return false;

			ctx->GetTimerExtention().lastUpdate = clock().now();
			ctx->GetTimerExtention().IsUpdating = false;
			if (!sip_meta->iSessionExpires)
			{
				VS_CallConfig &config = ctx->GetConfig();
				ctx->GetTimerExtention().refreshPeriod = std::chrono::seconds(config.sip.SessionTimers.RefreshPeriod.is_initialized() ? config.sip.SessionTimers.RefreshPeriod.get() : 1800);
			}
			else
			{
				ctx->GetTimerExtention().refreshPeriod = sip_meta->iSessionExpires->GetRefreshInterval();
			}
		}

		ctx->SetRingingStartTick(clock().now());

		auto sip_meta = rsp->GetSIPMetaField();
		if (!sip_meta || !sip_meta->IsValid())
			return false;
		if (ctx->IsKeepAliveEnabled())
		{
			size_t interval = sip_meta->iVia[0]->KeepAliveInterval(); // in sec
			if (sip_meta->iVia[0]->IsKeepAlive() && interval)
				ctx->SetKeepAliveInterval(std::chrono::seconds(interval)); // in msec
		}
		if (rsp->GetContentType() != CONTENTTYPE_SDP)
			return false;

		ctx->SetAnswered(clock().now());
		ctx->SetDTMFRequestTime(clock().now());
		ctx->IsAnswered(true);

		if (ctx->GetDisplayNameSip().empty())
		{
			if (sip_meta->iContact && sip_meta->iContact->GetLastURI())
			{
				ctx->SetDisplayNameSip(sip_meta->iContact->GetLastURI()->Name());
				if (ctx->GetDisplayNameSip().empty())
					ctx->SetDisplayNameSip(ctx->GetAliasRemote());
				UpdateSIPDisplayName(ctx, true);
			}
		}
		////////////////////////////////////////////////////////////////////////

		VS_SDPField_MediaStream *ms = ctx->MediaStream(0);
		if (ms && ms->GetMediaType() == SDPMediaType::message) {
			ctx->SetAcceptedTextTypesRemote(rsp->GetAcceptedTextFormats());
			AuthenticateAndSendInstantMessage(ctx, 1);
		}

		////////////////////////////////////////////////////////////////////////

			const VS_SIPGetInfoImpl get_info{ *ctx };
			VS_SIPUpdateInfoImpl update_info{ *ctx };
		if (rsp->FillInfoFromSDP(get_info, update_info, false))
			SendSetMediaChannels(ctx, VS_SIPParserInfo::SMCContinuation::Nothing);
		if (ctx->IsBFCPEnabled())
			InitBFCP(ctx);
		confMethods->InviteReplay(rsp->CallID(), e_call_ok, false, {}, {});

		std::vector<std::string> invites = ctx->PopPendingInvites();
		for (const auto &i : invites) {
			confMethods->AsyncInvite(string_view{ ctx->SIPDialogID() }, { ctx->GetAliasMy(), true }, i, ctx->GetConfInfo(),
				[](bool redirect, ConferenceStatus st, const std::string &ip) {}, ctx->GetDisplayNameSip(), false);
		}

		////////////////////////////////////////////////////////////////////////
	}
	break;
	case TYPE_BYE:
	{
		CleanParserContext(rsp->CallID(), SourceClean::PARSER);
	}
	break;
	case TYPE_REGISTER:
	{
		auto ctx = GetParserContext(rsp->CallID());
		if (!ctx)
			return false;

		auto exec = [ctx](VS_CallConfig &cfg)
		{
			auto call_config_manager = create_call_config_manager(cfg);
			if (call_config_manager.NeedVerification() && ctx->GetExpires().count())
			{
				call_config_manager.SetValidVerification();
			}
		};

		if (!UpdateRegistrationConfig(create_call_config_manager(ctx->GetConfig()).GetRegistrationIdentifierView(), [&exec](RegistrartionConfig &item)
		{
			exec(item.callConfig);
			disable_registration_on_call(item.callConfig, item);
		}))
		{
			if (ctx->IsInviteAfterRegister())
			{
				exec(ctx->GetConfig());
			}
		}

		if (rsp->GetSIPMetaField()->iContact) ctx->SetContactGruu(rsp->GetSIPMetaField()->iContact->LastGruu());

		if (ctx->IsInviteAfterRegister().get())
		{
			std::shared_ptr<VS_SIPParserInfo> i = ctx->IsInviteAfterRegister();

				const VS_SIPGetInfoImpl get_info_i{*i};
				VS_SIPUpdateInfoImpl update_info_i{*i};

			auto req = std::make_shared<VS_SIPRequest>();
			if (!req->MakeINVITE(get_info_i, update_info_i)) 	return false;

			vs::SleepFor(std::chrono::milliseconds(5000));

			if (GetParserContext(i->SIPDialogID()).get() == nullptr) // request was canceled
			{
				i->SetInviteAfterRegister(std::shared_ptr<VS_SIPParserInfo>());
				return false;
			}
			if (!PutMessageForOutput(req))
					return false;
			ctx->SetInviteAfterRegister(std::shared_ptr<VS_SIPParserInfo>());
		}
		else if (!create_call_config_manager(ctx->GetConfig()).NeedPermanentConnection() && !ctx->GetExpires().count())
		{
			// dialog was initiated just for verification, see VS_CallConfig::NeedVerification
			CleanParserContext(rsp->CallID(), SourceClean::PARSER);
		}
	}
	break;
	case TYPE_INFO:
	{
		auto ctx = GetParserContext(rsp->CallID());
		if (!ctx)	return false;

		if (ctx->MyBranch() == ctx->GetLastDTMFBranch())
		{
			ctx->HasDTMFAcknowledge(true);
		}
	} break;
	case TYPE_OPTIONS: {
		auto ctx = GetParserContext(rsp->CallID());
		if (!ctx)
		{
			return false;
		}

		if (!ctx->GetInviteAfterOptions().empty() && ctx->GetOptionsTick() != NULL_TICK)
		{
			ctx->SetOptionsTick(NULL_TICK);
			ctx->SetInviteAfterOptions({});

			bool update_config;
			VS_SIPUpdateInfoImpl sip_update_info{ *ctx };
			if (rsp->GetContentType() == CONTENTTYPE_SDP)
			{
				update_config = rsp->FillInfoByInviteMessage(sip_update_info);
			}
			else
			{
				update_config = rsp->FillInfoUserAgent(sip_update_info);
			}

			if (update_config)
				UpdateCallConfig(ctx, ctx->GetConfig().Address);

			ctx->SetTagSip({});
			return DoInvite(ctx);
		}
		return true;
	} break;
	default:
		break;
	}

	return true;
}

bool VS_SIPParser::OnRequest_CANCEL(const std::shared_ptr<VS_SIPRequest>& req)
{
	auto ctx = GetParserContext(req->CallID());
	if (!ctx)
		return false;

	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return false;

	const VS_SIPGetInfoImpl get_info(*ctx);
	VS_SIPUpdateInfoImpl update_info(*ctx);

	req->FillInfoByRequest(get_info, update_info);

	std::shared_ptr<VS_SIPResponse> rsp = std::make_shared<VS_SIPResponse>();
	if (!rsp->MakeOnCancelResponseOK(get_info, update_info))
		return false;
	if (!PutMessageForOutput(rsp)) {
		return false;
	}

	confMethods->Hangup(req->CallID());
	return true;
}

acs::Response VS_SIPParser::Protocol(const void* buf, std::size_t sz)
{
	if (!buf || sz == 0)
		return acs::Response::next_step;
	bool is_fragmented;
	VS_ChannelID channel_id = GetChannelID(buf, sz, is_fragmented);
	if (is_fragmented)
		return acs::Response::next_step;
	if (channel_id == e_SIP_CS)
		return acs::Response::accept_connection;
	return acs::Response::not_my_connection;
}

VS_ChannelID VS_SIPParser::GetChannelID(const void *buf, std::size_t sz, bool &isFragmented)
{
	isFragmented = false;

	VS_SIPField_StartLine s;
	VS_SIPBuffer buff(static_cast<const char *>(buf), sz);
	if (s.Decode(buff) == TSIPErrorCodes::e_ok && s.GetSIPProto() == SIPPROTO_SIP20)
		return e_SIP_CS;

	return e_noChannelID;
}

std::shared_ptr<VS_ParserInfo> VS_SIPParser::GetParserContextBase(string_view dialogId, bool create)
{
	return GetParserContext(dialogId, create);
}

std::shared_ptr<VS_SIPParserInfo> VS_SIPParser::GetParserContext(string_view dialogId, bool create)
{
	if (dialogId.empty()) return nullptr;

	std::lock_guard <decltype(m_ctx_lock)> lock{ m_ctx_lock };

	auto it = m_ctx.find(dialogId);

	if (it != m_ctx.cend()) return it->second;

	if (create)
	{
		auto new_ctx = std::make_shared<VS_SIPParserInfo>(m_userAgent);
		new_ctx->SetConfig(CreateCallConfig("", {}));

		auto &config = new_ctx->GetConfig();
		new_ctx->SetH224Enable(config.H224Enabled.get_value_or(DEFAULT_FECC_ENABLED));

		m_ctx_connections.emplace_back(new_ctx->ConnectToDie([this](string_view dialogId) { OnContextDestructor(dialogId); }));
		m_ctx.emplace(static_cast<std::string>(dialogId), new_ctx);
		it = m_ctx.find(dialogId);
		if (it == m_ctx.end())
			return nullptr;
		it->second->SetMyCsAddress(m_myCsEp);
		it->second->SetExpires(DEFAULT_EXPIRES);
		return it->second;
	}
	return nullptr;
}

void VS_SIPParser::CleanParserContext(string_view dialogId, SourceClean source)
{
	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(dialogId);

	if (!ctx)
		return;

	if (auto bfcp_channel = ctx->GetBFCPChannel())
		bfcp_channel->Close(true);

	auto &call_cfg = ctx->GetConfig();

	auto call_config_manager = create_call_config_manager(call_cfg);

	if (source == SourceClean::TRANSPORT && call_config_manager.NeedVerification())
	{
		if (!UpdateRegistrationConfig(call_config_manager.GetRegistrationIdentifierView(), [](RegistrartionConfig& item)
		{
			create_call_config_manager(item.callConfig).SetVerificationResult(VS_CallConfig::e_ServerUnreachable, true);
			disable_registration_on_call(item.callConfig, item);
		}, true) && ctx->IsInviteAfterRegister())
		{
			call_config_manager.SetVerificationResult(VS_CallConfig::e_ServerUnreachable, true);
		}
	}

	auto &&user = ctx->GetUser();

	std::string reg_dialog_id;
	if(!user.empty())
	 reg_dialog_id = GetRegDialogIdByRegIdentDialogId({ call_cfg.RegistryConfigName, user }).get_value_or({});

	// if RegisterStrategy == 'register before call and unregister after'
	// and dialog_id belongs to invite, not to registration context. =>
	// it is end of call and we should unregister now.
	if (ctx->IsInviteAfterRegister() && reg_dialog_id != dialogId)
	{
		ctx->IsInviteAfterRegister()->SetExpires(std::chrono::seconds(0));
		DoRegister(ctx->IsInviteAfterRegister()->SIPDialogID());

		ctx->SetInviteAfterRegister(std::shared_ptr<VS_SIPParserInfo>());
	}

	if (reg_dialog_id == dialogId)
	{
		std::lock_guard<decltype(m_register_dialog_id_lock)> _(m_register_dialog_id_lock);
		const auto res = m_register_dialog_id.find(RegistrationIdentDialogIdView{ call_cfg.RegistryConfigName, user });
		if (res != m_register_dialog_id.cend())
		{
			m_register_dialog_id.erase(res);
		}
	}

	{
		std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };
		const auto it = m_ctx.find(dialogId);
		if (it != m_ctx.cend())
		{
			m_ctx.erase(it);
		}
	}

}

std::shared_ptr<VS_SIPParserInfo> VS_SIPParser::GetParserRegisterContext(string_view userId)
{
	if (userId.empty())
		return nullptr;

	std::shared_ptr<VS_SIPParserInfo> null_ptr;

	std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };
	for (auto it = m_ctx.begin(); it != m_ctx.cend(); ++it)
	{
		if (it->second && it->second->GetRegisterTick() != NULL_TICK)
		{
			auto &&a = it->second->GetAliasRemote();
			auto &&t = it->second->GetSIPRemoteTarget();
			if ((!a.empty() && boost::iequals(a, userId)) ||
				(!t.empty() && boost::iequals(t, userId)))
				return it->second;
			auto auth = it->second->GetAuthScheme();
			if (auth)
			{
				if (auth->login() == userId)
				{
					return it->second;
				}
			}
		}
	}

	return null_ptr;
}

std::shared_ptr<VS_SIPParserInfo> VS_SIPParser::GetRegCxtByContact(string_view contact)
{
	for (const auto& ctxPair : m_ctx)
	{
		const auto &ctx = ctxPair.second;
		assert(ctx != nullptr);
		if (!ctx || ctx->GetRegisterTick() == NULL_TICK)
			continue;
		auto pContactHeader = ctx->GetSIPContact();
		if (!pContactHeader || !pContactHeader->GetLastURI())
			continue;

		std::string storedContact;
		pContactHeader->GetLastURI()->GetRequestURI(storedContact);
		if (storedContact == contact)
			return ctx;
	}

	return nullptr;
}

std::shared_ptr<VS_SIPParserInfo> VS_SIPParser::GetTCPRegCxtByContact(string_view contact)
{
	auto ctx = GetRegCxtByContact(contact);
	if (!ctx)
		return nullptr;
	if (ctx->GetConfig().Address.protocol != net::protocol::TCP)
		return nullptr;

	return ctx;
}

std::shared_ptr<VS_SIPParserInfo> VS_SIPParser::GetRegContextOnRemoteServer(const std::shared_ptr<VS_SIPParserInfo> &ctx, const std::shared_ptr<VS_SIPRequest> &req) {
	assert(ctx != nullptr);
	if (!ctx) return nullptr;
	auto &&pConfig = ctx->GetConfig();

	const VS_SIPMetaField *meta = nullptr;
	VS_SIPURI *uri = nullptr;
	std::string req_user, req_host;
	if (req && (meta = req->GetSIPMetaField()) && meta->iTo && (uri = meta->iTo->GetURI())) {
		req_user = uri->User();
		req_host = uri->Host();
	}

	std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };
	for (const auto &i : m_ctx)
	{
		auto &&c = i.second->GetConfig();
		if (ctx == i.second) continue;
		if (!i.second->IsRegisterContext()) continue;

		if (i.second->secureCtx || (i.second->GetAuthScheme() && i.second->GetAuthScheme()->scheme() != SIP_AUTHSCHEME_NTLM)) {
			if ((pConfig.Login == c.Login && pConfig.Password == c.Password) ||
				(req_user == c.Login && req_host == c.sip.FromDomain)) {
				return  i.second;
			}
		}
	}

	return nullptr;
}

void VS_SIPParser::OnContextDestructor(string_view dialogId)
{
	std::lock_guard<decltype(m_ctx_garbage_lock)> _{ m_ctx_garbage_lock };
	m_ctx_garbage.emplace_back(dialogId);

	//m_fireDialogFinished(dialog_id);
}

void VS_SIPParser::Timeout()
{
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;

	m_policy->Timeout();
	while (true)
	{
		std::string dialog_id;
		{
			std::lock_guard<decltype(m_ctx_garbage_lock)> lock{ m_ctx_garbage_lock };
			const auto iter = m_ctx_garbage.cbegin();
			if (iter != m_ctx_garbage.cend())
			{
				dialog_id = *iter;
				m_ctx_garbage.erase(iter);
			}
			else
			{
				break;
			}
		}
		dstream3 << "fireDialogFinished(" <<  dialog_id << ")";
		m_fireDialogFinished(dialog_id);
	}

	{
		auto locked_transaction_handler = m_transaction_handler.lock();
		if (m_shutdown_tick == NULL_TICK) {
			locked_transaction_handler->Cleanup();
		}
		else if (clock().now() - m_shutdown_tick > SHUTDOWN_TIME_LIMIT) {
			locked_transaction_handler->Clear();
		}
	} /* end of lock section */

	std::vector<std::string> to_clean;
	std::vector<std::string> to_hangup;
	std::vector<std::string> to_logout;
	std::vector<std::shared_ptr<VS_SIPParserInfo>>  to_retry_call;

	{
		std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };

		auto it = m_ctx.begin();
		while (it != m_ctx.end())
		{
			const auto bye_tick = it->second->GetByeTick();
			const auto msg_tick = it->second->MsgAliveTick();
			// skype keep sending messages in same context during the call, so do not delete those auth contexts until AVERAGE_CALL_DURATION
			if (it->second->ReadyToDie() || (msg_tick != NULL_TICK && clock().now() - msg_tick >= AVERAGE_CALL_DURATION)) {
				to_clean.push_back(it->first);
			}
			if (bye_tick != NULL_TICK && (clock().now() - bye_tick >= CALL_BYE_TIMEOUT))
			{
				to_clean.push_back(it->first);
			}
			if (!it->second->VerifyNTLM_SA_lifetime()) {
				dstream2 << "Warning! NTLM Security Association lifetime exprired. Registration will be reinitialized.\n";
				to_clean.push_back(it->first);
			}

			const auto register_tick = it->second->GetRegisterTick();
			if (register_tick != NULL_TICK && (clock().now() - register_tick >= std::chrono::seconds(it->second->GetExpires())))		// registration expired
			{
				bool is_incall = false;
				for (auto &c : m_ctx)
				{
					std::shared_ptr<VS_SIPParserInfo> ctx = c.second;
					if (ctx && ctx->GetRingingStartTick() != NULL_TICK &&
						(ctx->IsAnswered() || clock().now() - ctx->GetRingingStartTick() < CALL_START_TIMEOUT) &&
						ctx->GetSIPRemoteTarget() == it->second->GetSIPRemoteTarget())
					{
						is_incall = true;
						break;
					}
				}
				if (!is_incall) {
					to_logout.push_back(it->first);
				}
				else {
					// extend registration during call
					it->second->SetExpires(it->second->GetExpires() + std::chrono::seconds(30));
				}
			}

			auto user = it->second->GetUser();
			bool IsRegistrationCtx = ((!user.empty() &&
				GetRegDialogIdByRegIdentDialogId({ it->second->GetConfig().RegistryConfigName,  user })
				.get_value_or({}) == it->second->SIPDialogID())) || register_tick != NULL_TICK;

			auto invite_tick = it->second->GetRingingStartTick();

			if (!IsRegistrationCtx)
			{
				if (invite_tick != NULL_TICK && !it->second->IsAnswered() && clock().now() - invite_tick >= CALL_START_TIMEOUT)
				{
					to_hangup.push_back(it->first);
				}

				const auto create_ctx_tick = it->second->GetCreateCtxTick();
				if (invite_tick == NULL_TICK && (clock().now() - create_ctx_tick >= CALL_START_TIMEOUT))
				{
					to_hangup.push_back(it->first);
					to_clean.push_back(it->first);
				}
			}

			if (it->second->IsKeepAliveSendNeeded())
			{
				/*unsigned char *msg = new unsigned char[5];
				strcpy((char*)msg, "\r\n\r\n");
				m_queue_out.PutMessage(msg, 4, (VS_ChannelID) e_SIP_CS);*/

				auto msg = std::make_shared<VS_SIPMessage>();
				msg->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, VS_SIPGetInfoImpl(*it->second));
				PutMessageForOutput(msg);
			}

			// timer extention... support only for incomming calls
			// we are UAS
			TimerExtention &timeExt = it->second->GetTimerExtention();

			if (timeExt.refresher != REFRESHER::REFRESHER_INVALID
				&& (timeExt.IsUpdating && (timeExt.lastUpdate + timeExt.refreshPeriod) < clock().now())
				&& it->second->GetByeTick() == NULL_TICK
				&& it->second->IsAnswered()
				&& (it->second->IsSessionTimerEnabled() || it->second->IsSessionTimerUsed()))
			{
				to_hangup.push_back(it->first);
			}
			else
				if (it->second->IsWeRefresh()
					&& timeExt.refreshPeriod / 2 + timeExt.lastUpdate < clock().now()
					&& !timeExt.IsUpdating
					&& it->second->GetByeTick() == NULL_TICK
					&& it->second->IsAnswered())
				{
					if (it->second->GetTimerExtention().refresher == REFRESHER::REFRESHER_UAS) {
						it->second->GetTimerExtention().refresher = REFRESHER::REFRESHER_UAC;
					}
					timeExt.IsUpdating = true;
					auto r = std::make_shared<VS_SIPRequest>();

				const VS_SIPGetInfoImpl get_info_it{*it->second};
				VS_SIPUpdateInfoImpl update_info_it{*it->second};

					if (r->MakeRefreshINVITE(get_info_it, update_info_it)) {
						PutMessageForOutput(r);
					}
				}

			boost::shared_ptr<bfcp::SessionBase> bfcp_session;
			std::shared_ptr<VS_SignalChannel> bfcp_channel;
			if (((bfcp_session = it->second->GetBFCPClientSession())
				|| (bfcp_session = it->second->GetBFCPServerSession()))
				&& (bfcp_channel = it->second->GetBFCPChannel()))
			{
				size_t size = 512;
				vs::SharedBuffer buffer(size);
				bool result = bfcp_session->GetSendData(buffer.data(), size);
				if (!result && size > 0)
				{
					buffer = vs::SharedBuffer(size);
					result = bfcp_session->GetSendData(buffer.data(), size);
				}
				if (result && size > 0)
				{
					buffer.shrink(0, size);
					bfcp_channel->Open();
					bfcp_channel->Send(std::move(buffer));
				}
			}

			// OPTIONS
			if (it->second->GetOptionsTick() != NULL_TICK && it->second->NeedsOptionsBeforeInvite() &&
				clock().now() - it->second->GetOptionsTick() > OPTIONS_TIMEOUT) {
				it->second->SetOptionsTick(NULL_TICK);
				// Response on options request was not received, send invite request anyway
				DoInvite(it->second);
				// Stop retransmiting request
				auto &invite_after_ops = it->second->GetInviteAfterOptions();
				if (!invite_after_ops.empty())
				{
					//m_transaction_lock.Lock();
					m_transaction_handler->RequestCancelled(it->second, invite_after_ops, TYPE_OPTIONS);
				}
				//m_transaction_lock.UnLock();
				it->second->SetInviteAfterOptions({});
				it->second->SetTagSip({});
			}

			if (it->second) {
				auto ctx = it->second.get();

				if (ctx->NeedRetryAfter() && ctx->RetryAfterTime() <= clock().now()) {
					ctx->NeedRetryAfter(false);
					to_retry_call.emplace_back(it->second);
				}
				if (!ctx->IsAnswered() && ctx->NeedAddressRedirection() && ctx->HaveAddressesToRedirect()) {
					std::string address;
					ctx->GetAddressToRedirect(address);
					ctx->NeedAddressRedirection(false);

					const auto resp_code = ctx->GetResponseCode();
					const bool ntlm_reg_redirect = ctx->IsNTLMContext() && ctx->GetMessageType() == TYPE_REGISTER;
					const bool change_sip_aliases = resp_code != 305 && !ntlm_reg_redirect;
					if (change_sip_aliases) {	// when 305 have arrived we don't need change 'To' in header, just redirect request to another address
						VS_SkipSIPPrefix(address);
						ctx->SetDisplayNameSip(address);
						ctx->SetAliasRemote(address);
						ctx->SetSIPRemoteTarget(address);
					}

					/* if address was changed we need remove dialog_id from old channel and create new one but parser context with dialog_id must be safe*/
					auto alias_remote = ctx->GetAliasRemote();
					if (SetToIPAddress(it->second, ctx->GetAliasMy(), ctx->GetDisplayNameMy(), (change_sip_aliases)? alias_remote : address))
						m_fireFreeDialogFromChannel(ctx->SIPDialogID());

					if (ntlm_reg_redirect) {
						ctx->secureCtx = nullptr; 	// force to begin registration from the first step on new server
						DoRegister(ctx->SIPDialogID());
					}
					else to_retry_call.emplace_back(it->second);
				}
			}

			++it;
		}
	}

	for (auto it = to_hangup.cbegin(); it != to_hangup.cend(); ++it)
	{
		Hangup(*it);
	}

	for (auto it = to_clean.cbegin(); it != to_clean.cend(); ++it)
	{
		CleanParserContext(*it, SourceClean::PARSER);
	}

	for (auto it = to_logout.cbegin(); it != to_logout.cend(); ++it)
	{
		std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(*it);
		if (ctx)
		{
			auto a = ctx->GetAuthScheme();
			if (!a)
				continue;

			const VS_SIPGetInfoImpl get_info_tmp(*ctx);
			VS_SIPUpdateInfoImpl update_info_tmp(*ctx);

			std::shared_ptr<VS_SIPResponse> fake = std::make_shared<VS_SIPResponse>();
			fake->InsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_CallID, get_info_tmp);

			confMethods->LoginUser(string_view{ ctx->SIPDialogID() }, a->login(), ctx->GetPasswordTranscoder(), NULL_TICK,/* contact.c_str()*/{},
				boost::bind(&VS_SIPParser::onLogoutResponse, std::static_pointer_cast<VS_SIPParser>(shared_from_this()),
					_1, fake, fake, false),
				boost::bind(&VS_SIPParser::onLogout, std::static_pointer_cast<VS_SIPParser>(shared_from_this()),
					ctx->SIPDialogID()), std::vector<std::string>());
		}
	}

	for (const auto &ctx : to_retry_call)
	{
		if (ctx) {
			ctx->SetTagSip({});
//			Making a copy of MyAlias as it will get invalid inside
//			Vitalya do not delete this
			InviteMethod(ctx->SIPDialogID(), std::string(ctx->GetAliasMy()), ctx->GetAliasRemote(), VS_ConferenceInfo(ctx->IsGroupConf(), ctx->IsPublicConf()));
		}
	}

	// dtmf
	{
		std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };
		auto it = m_ctx.begin();
		while (it != m_ctx.cend())
		{
			std::shared_ptr<VS_SIPParserInfo> ctx = it->second;
			if (ctx->IsAnswered() &&
				((clock().now() - ctx->GetAnswered() >= std::chrono::seconds(ctx->GetDTMF_Treshold()))))
			{
				char digit(0);
				if (ctx->WaitDTMFAcknowledge()) {
					// if has dtmf symbol and have 200 ok received in VS_SIPParser::OnResponse_Code200 or have timeout
					if ((ctx->HasDTMFAcknowledge() || ctx->HasDTMFAnswerTimeout()) && ctx->DTMFPausePassed() && (digit = ctx->GetDTMF())) {
						if (digit == ',' || digit == 'p' || digit == 'w')  //wait 2 seconds more
							ctx->AddDTMFPauseTime(std::chrono::milliseconds(2000));
						else
						{
							ctx->SetDTMFPauseTime(std::chrono::milliseconds(0));
							SendDTMF(ctx, digit);
						}
					}
				}
				else {	// first time wll be here
					if (ctx->DTMFPausePassed() && (digit = ctx->GetDTMF())) {			// if has dtmf symbol send it
						if (digit == ',' || digit == 'p' || digit == 'w') {	//wait 2 seconds more
							ctx->AddDTMFPauseTime(std::chrono::milliseconds(2000));
							ctx->SetDTMFRequestTime(clock().now());	// for ',,,1234' case
						}
						else {
							if (SendDTMF(ctx, digit)) {
								ctx->WaitDTMFAcknowledge(true);
								ctx->SetDTMFPauseTime(std::chrono::milliseconds(0));
							}
						}
					}
				}
			}
			++it;
		}
	}

	{
		std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };
		for (const auto& dialog : m_ctx)
		{
			const auto& ctx = dialog.second;
			if (ctx->GetSMCContinuation() == VS_SIPParserInfo::SMCContinuation::SendResponse && clock().now() - ctx->GetSMCTick() >= SET_MEDIA_CHANNELS_TIMEOUT)
			{
				const VS_SIPGetInfoImpl get_info(*ctx);
				VS_SIPUpdateInfoImpl update_info(*ctx);
				do
				{
					std::shared_ptr<VS_SIPRequest> invite(ctx->GetLastInvite());
					if (!invite)
						break;

					dprint1("SetMediaChannels(%s) to transcoder timed out: sending 408 Request Timeout, cseq=%i\n", dialog.first.c_str(), invite->GetCSeq());
					std::shared_ptr<VS_SIPResponse> rsp = std::make_shared<VS_SIPResponse>();
					if (!rsp->MakeRequestTimeout(invite.get(), get_info, update_info))
						break;
					if (!PutMessageForOutput(rsp)) {
						break;
					}
					ctx->SetSMCContinuation(VS_SIPParserInfo::SMCContinuation::Nothing);
				} while (false);
			}
		}
	}

	CheckPermanentRegistrations();
}

bool  VS_SIPParser::SendDTMF(const std::shared_ptr<VS_SIPParserInfo>& ctx, char dtmf) {
	auto req = std::make_shared<VS_SIPRequest>();
	const VS_SIPGetInfoImpl get_info(*ctx);
	VS_SIPUpdateInfoImpl update_info(*ctx);

	if (req->MakeINFO_DTMF(get_info, update_info, dtmf)) {
		PutMessageForOutput(req);
		ctx->SetDTMFRequestTime(clock().now());
		ctx->HasDTMFAcknowledge(false);
		ctx->SetLastDTMFBarnch(ctx->MyBranch());

		return true;
	}
	return false;
}

std::string VS_SIPParser::NewDialogID(string_view sipTo, string_view dtmf, const VS_CallConfig& config, string_view myName)
{
	if (sipTo.empty()) return {};

	char new_dialog_id[32 + 1] = { 0 };  //32 + '\0'
	VS_GenKeyByMD5(new_dialog_id);
	while (GetParserContext(new_dialog_id))
	{
		VS_GenKeyByMD5(new_dialog_id);
	}

	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(new_dialog_id, true);
	assert(ctx);
	auto &ctx_config = ctx->GetConfig();
	auto call_config_manager = create_call_config_manager(ctx_config);
	call_config_manager.MergeWith(config);
	ctx->SetConfig(ctx_config); // to update codecs

	std::string alias(sipTo);

	if (alias.find('@') == string_view::npos && !ctx->GetConfig().HostName.empty())
		alias += "@" + ctx->GetConfig().HostName;

	ctx->SIPDialogID(new_dialog_id);
	ctx->SetLocalBandwidth(ctx->GetConfig().Bandwidth.get_value_or(0) * 1024);	// kbit/s
	ctx->SetDirection(true);

	VS_SkipSIPPrefix(alias);
	VS_RemoveAtSign(alias);

	ctx->SetAliasRemote(alias);
	ctx->SetSIPRemoteTarget(std::move(alias));

	int32_t dtmf_treshold(0);
	if (!DefaultCallManager::GetSIPParam("DTMF Treshold", dtmf_treshold))
		dtmf_treshold = DEFAULT_DTMF_TRESHOLD;
	ctx->SetDTMF_Treshold(dtmf_treshold);
	for (std::size_t i = 0; i<dtmf.length(); ++i)
		ctx->AddDTMF(dtmf[i]);
	VS_RegistryKey cfg(false, CONFIGURATION_KEY);
	if (cfg.IsValid())
	{
		std::string host;
		if (cfg.GetString(host, "SIP External Host") && !host.empty())
		{
			ctx->SetMyExternalCsAddress(host);
		}
	}

	auto &dialog_id = ctx->SIPDialogID();
	dstream3 << "NewDialogID: " << dialog_id << "," << " sip_to: " << ctx->GetAliasRemote();
	return dialog_id;
}

void VS_SIPParser::Shutdown()
{
	dprint3("VS_SIPParser::Shutdown()\n");
	RemovePermanentRegistrations();

	std::vector<std::string> to_hangup;
	{
		std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };

		dprint3("Dialog count: %zu\n", m_ctx.size());
		for (auto it = m_ctx.begin(); it != m_ctx.cend(); )
		{
			// if it is REGISTER
			const std::string &user = it->second->GetUser();
			auto res = GetRegDialogIdByRegIdentDialogId({ it->second->GetConfig().RegistryConfigName, user });
			if (res.is_initialized() && res == it->first)
			{
				it = m_ctx.erase(it);
			}
			else
			{
				to_hangup.push_back((it++)->first);
			}
		}
	}

	for (auto it = to_hangup.begin(); it != to_hangup.cend(); ++it)
	{
		Hangup(*it);
	}

	{
		auto locked_transaction_handler = m_transaction_handler.lock();	// use locked_transaction_handler as mutex for m_shutdown_tick
		m_shutdown_tick = clock().now();
	}
}

void VS_SIPParser::Chat(string_view dialogId, const std::string &from, const std::string &to, const std::string &dn, const char *mess)
{
	dprint3("VS_SIPParser::Chat\n");
	assert(mess);
	if (!*mess)
		return;

	std::shared_ptr<VS_SIPParserInfo> pCallInfo = GetParserContext(dialogId);
	if (!pCallInfo) return;

	auto &&config = pCallInfo->GetConfig();

	char rand_str[33] = {};
	VS_GenKeyByMD5(rand_str);
	pCallInfo->MyBranch(rand_str);

	bool in_call = pCallInfo->InCall();
	if (!in_call)
		pCallInfo->SetByeTick(clock().now());	// make sure context will be deleted after CALL_BYE_TIMEOUT in Timeout() any way

	if (pCallInfo->GetRegisterTick() == NULL_TICK && config.Address.addr.is_unspecified()) {
		// set address or resolve if needed
		SetToIPAddress(pCallInfo, from, dn, to);
	}// else when context registered it knows ip already

	if (static_cast<const VS_SIPParserInfo &>(*pCallInfo).GetMyCsAddress().addr.is_unspecified())
	{
		if (!m_myCsEp.addr.is_unspecified())
		{
			pCallInfo->SetMyCsAddress(m_myCsEp);
		}
		else if (!config.Address.addr.is_unspecified())
		{
			pCallInfo->SetMyCsAddress(config.Address);
		}
		else
			return;
		}

	if (std::chrono::duration_cast<std::chrono::seconds>(pCallInfo->GetTimerExtention().refreshPeriod).count() <= 0) {
		// this value used in session expires field
		if (in_call) {
			pCallInfo->GetTimerExtention().refreshPeriod = std::chrono::seconds(1800);	// average call duration
		}
		else {
			pCallInfo->GetTimerExtention().refreshPeriod = std::chrono::seconds(90);		// the min value of session expires see https://www.ietf.org/rfc/rfc4028.txt
		}
	}
	const char prefix[] = "#sip:";

	string_view to_addr(to);
	if (boost::starts_with(to_addr, string_view{ prefix, sizeof(prefix) - 1 }))
		to_addr = to_addr.substr(to_addr.find(':') + 1); //to_addr.erase(0, to_addr.find(':') + 1); // remove '#sip:' because it'll be added in start line
														 //	if (to_addr.find('/') != std::string::npos)
	to_addr = to_addr.substr(0, to_addr.find('/'));


	std::string msg_with_sender;
	if (pCallInfo->IsGroupConf()) {
		if (!dn.empty()) msg_with_sender = dn;
		if (msg_with_sender.empty()) msg_with_sender = from;
		auto pos = msg_with_sender.find('@');
		if (pos != std::string::npos) msg_with_sender.erase(pos);

		msg_with_sender += ':';
		msg_with_sender += mess;
		mess = msg_with_sender.c_str();
	}

	// settings for start line
	pCallInfo->IsRequest(true);
	pCallInfo->SetMessageType(TYPE_MESSAGE);
	pCallInfo->SetSIPRemoteTarget(std::string(to_addr));

	if (config.Address.protocol != net::protocol::UDP) {
		auto regCtx = GetTCPRegCxtByContact(to_addr);
		if (regCtx)
			pCallInfo->SetRegCtxDialogID(regCtx->SIPDialogID());
	}


	// settings for call_id
	pCallInfo->SIPDialogID(std::string(dialogId));

	// settings for from
	auto &&our_host = GetFromHost();
	if (!our_host.empty())
	{
		string_view user = from;
		const auto pos = user.find('@');
		if (pos + 1 != string_view::npos)
			user = user.substr(0, pos + 1);
		pCallInfo->SetAliasMy(std::string(user) + our_host);
	}
	else
		pCallInfo->SetAliasMy(from);

	VS_GenKeyByMD5(rand_str);
	pCallInfo->SetTagMy(rand_str);

	// no display name when from_id doesn't contain username part
	if (strchr(from.c_str(), '@'))
		pCallInfo->SetDisplayNameMy(!dn.empty() ? dn : from);
	else
		pCallInfo->SetDisplayNameMy({});

	// settings for to
	pCallInfo->SetAliasRemote(std::string(to_addr));
	pCallInfo->SetTagSip({});
	pCallInfo->SetDisplayNameSip(std::string(to_addr));

	// settings for content type
	eContentType msg_ct = CONTENTTYPE_TEXT_PLAIN;
	if (boost::starts_with(mess, "{\\rtf")) msg_ct = CONTENTTYPE_TEXT_RTF;
	pCallInfo->SetContentType(msg_ct);

	auto msg_ctx = pCallInfo;

	const auto reg_ctx = GetRegContextOnRemoteServer(pCallInfo);
	if (reg_ctx && reg_ctx->IsNTLMContext()) {
		msg_ctx = FindActiveMsgCtx(reg_ctx);
		if (!msg_ctx) {
			pCallInfo->AddPengingMessage(mess, msg_ct);
			AuthenticateAndSendInstantMessage(pCallInfo, 0);
			return;
		}


		if (!pCallInfo->InCall() && !pCallInfo->ActiveMsgCtx()) pCallInfo->ReadyToDie(true);

		auto &&pTo = msg_ctx->GetAliasRemote();
		string_view from_sv = from;

		auto pos = from_sv.find(string_view{ prefix, sizeof(prefix) - 1 });
		if (pos == 0)
			from_sv.remove_prefix(sizeof(prefix) - 1);

		// do not send message to ourselves
		if (!pTo.empty() && from_sv == pTo)
			return;
	}
	auto req = std::make_shared<VS_SIPRequest>();

	const VS_SIPGetInfoImpl get_info(*msg_ctx);
	VS_SIPUpdateInfoImpl update_info(*msg_ctx);

	if (req->MakeMESSAGE(get_info, update_info, mess, msg_ct))
		PutMessageForOutput(req);
	}

/* function always returns true, because in other way call context will be deleted early than intended*/
bool  VS_SIPParser::OnRequest_MESSAGE(const std::shared_ptr<VS_SIPRequest>& req, const net::Endpoint &recvFromEp) {
	dprint3("VS_SIPParser::OnRequest_MESSAGE()\n");

	if (!req || !req->IsValid() || !req->GetSIPInstantMessage())
		return true;

	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return true;

	string_view dialog_id = req->CallID();
	if (dialog_id.empty())	return true;

	VS_SCOPE_EXIT
	{
		if (dialog_id.empty()) return;

		std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(dialog_id);
		if (ctx && !ctx->InCall() && !ctx->ActiveMsgCtx()) ctx->ReadyToDie(true);	// else context will be cleared by some tick in Timeout
	};

	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(dialog_id, true);
	if (!ctx) return true;

	VS_SIPUpdateInfoImpl update_info(*ctx);
	const VS_SIPGetInfoImpl get_info(*ctx);

	if (!req->FillInfoByInviteMessage(update_info))
	{
		return true;
	}

	UpdateCallConfig(ctx, recvFromEp, false);
	ctx->GetConfig().Address = recvFromEp;

	const auto& messageText = req->GetSIPInstantMessage()->GetMessageText();
	std::string to = req->GetTo();
	auto &&display_name = req->DisplayNameMy();

	auto &from_user = ctx->GetAliasRemote();
	std::string from;

	if (!from_user.empty())
	{
		string_view user = from_user;
		user = user.substr(0, user.find('@'));
		std::shared_ptr<VS_SIPParserInfo> ctx2 = GetParserRegisterContext(user);
		if (!ctx2)	ctx2 = GetParserRegisterContext(from_user);

		if(ctx2)
		{
			from = VS_RealUserLogin(ctx2->GetUser());
		}
		else
		{
			from = ctx->GetAliasRemote();
			if (from.find('@') == std::string::npos)	from.insert(0, "@");
			// user can't call or send message to sip without '#sip:' prefix
			// so if softfone not registered on our server this prefix will
			// be present in users address book
			from = "#sip:" + from;
		}

		if (ctx->GetSIPRemoteTarget().empty())	// if it is new dialog and remote target not set
			ctx->SetSIPRemoteTarget(from_user);
	}

	if (messageText.empty() || to.empty() || req->GetFrom().empty() || dialog_id.empty())	// empty display name is allowed
		return true;

	std::shared_ptr<VS_SIPResponse> rsp = std::make_shared<VS_SIPResponse>();

	if (!rsp->MakeOnMessageResponseOK(req.get(), get_info, update_info))
		return true;
	if (!PutMessageForOutput(rsp))
		return true;

	bool is_chat_command = false;

	if (messageText[0] == '/') {
		std::string msg_text = messageText;	boost::trim_right(msg_text);
		is_chat_command = ProcessChatCommand(ctx, from, msg_text);
	}

	if (!is_chat_command) {
		auto call_ctx = FindActiveCallCtx(ctx);
		if (call_ctx && IsGroupConf(call_ctx)) { // group conf
			confMethods->Chat(call_ctx->SIPDialogID(), from, {}, std::string(display_name), messageText.c_str());
			return true;
		}

		if (call_ctx) { // p2p
			to = GetPeer(call_ctx);
			confMethods->Chat(call_ctx->SIPDialogID(), from, to, std::string(display_name), messageText.c_str());
		}
		else {
			confMethods->Chat(dialog_id, from, to, std::string(display_name), messageText.c_str());
		}
	}
	return true;
}

void VS_SIPParser::Command(string_view dialogId, string_view from, string_view command)
{
	dstream3 << "VS_SIPParser::Command: " << command << ", from=" << from << ", dialog_id=" << dialogId;
	auto ctx = GetParserContext(dialogId);
	if (!ctx && dialogId.empty())	// try find by from in p2p call
	{
		std::lock_guard <decltype(m_ctx_lock)> lock{ m_ctx_lock };
		for (auto&& p : m_ctx)
		{
			auto&& i = p.second;
			if (i->IsAnswered() && !i->IsGroupConf() && i->GetAliasMy() == from)		// answered p2p call with from
			{
				ctx = i;
				break;
			}
		}
	}
	if (!ctx)
		return;

	static string_view dtmf_prefix = "#dtmf:";
	if (boost::starts_with(command, string_view(dtmf_prefix)) && command.length() > dtmf_prefix.length()) {
		command.remove_prefix(dtmf_prefix.length());
		char digit = command[0];
		dprint3("DTMF:%c\n", digit);
		ctx->AddDTMF(digit);
	} else if (boost::starts_with(command, string_view(SHOW_SLIDE_COMMAND)))
		UpdateSlideshowState(ctx, true);
	else if (boost::starts_with(command, string_view(END_SLIDESHOW_COMMAND)))
		UpdateSlideshowState(ctx, false);
	else if (boost::equals(command, string_view(CONTENTFORWARD_PULL)))
		UpdateSlideshowState(ctx, true);
	else if (boost::equals(command, string_view(CONTENTFORWARD_PUSH)) || boost::equals(command, string_view(CONTENTFORWARD_STOP)))
		UpdateSlideshowState(ctx, false);
}

void VS_SIPParser::InitMediaStreams(const std::shared_ptr<VS_SIPParserInfo> &ctx)
{
	auto& media_streams = ctx->MediaStreams();
	const VS_SIPGetInfoImpl get_info(*ctx);

	if (std::none_of(media_streams.begin(), media_streams.end(), [](VS_SDPField_MediaStream* ms) {
		return ms->GetMediaType() == SDPMediaType::audio && ms->GetContent() == SDP_CONTENT_MAIN;
	}))
	{
		VS_SDPField_MediaStream* const ms = ctx->MediaStream(media_streams.size(), true);
		ms->SetMediaType(SDPMediaType::audio);
		ms->SetMediaDirection(SDP_MEDIACHANNELDIRECTION_SENDRECV);
		ms->SetProto(SDP_RTPPROTO_RTP_AVP);
		ms->SetContent(SDP_CONTENT_MAIN);
		ms->ClearLocalCodecs();
		ms->CopyLocalCodecsFrom(get_info);

		if (ctx->ICEEnabled()) {
			VS_SIPMessage::InsertMediaStreamICE(get_info, ms);
		}

		if (ctx->SRTPEnabled())
		{
			VS_SIPMessage::InsertMediaStreamSRTP(get_info, ms);
		}
	}

	if (std::none_of(media_streams.cbegin(), media_streams.cend(), [](const VS_SDPField_MediaStream *ms) {
		return ms->GetMediaType() == SDPMediaType::video && ms->GetContent() == SDP_CONTENT_MAIN;
	}))
	{
		VS_SDPField_MediaStream* const ms = ctx->MediaStream(media_streams.size(), true);
		ms->SetMediaType(SDPMediaType::video);
		ms->SetMediaDirection(SDP_MEDIACHANNELDIRECTION_SENDRECV);
		ms->SetProto(SDP_RTPPROTO_RTP_AVP);
		ms->SetContent(SDP_CONTENT_MAIN);
		ms->ClearLocalCodecs();
		ms->CopyLocalCodecsFrom(get_info);

		if (ctx->ICEEnabled()) {
			VS_SIPMessage::InsertMediaStreamICE(get_info, ms);
		}

		if (ctx->SRTPEnabled())
		{
			VS_SIPMessage::InsertMediaStreamSRTP(get_info, ms);
		}
	}

	if (ctx->IsBFCPEnabled() && std::none_of(media_streams.begin(), media_streams.end(), [](VS_SDPField_MediaStream* ms) {
		return ms->GetMediaType() == SDPMediaType::application_bfcp;
	}))
	{
		VS_SDPField_MediaStream* const ms = ctx->MediaStream(media_streams.size(), true);
		ms->SetMediaType(SDPMediaType::application_bfcp);
		ms->SetMediaDirection(SDP_MEDIACHANNELDIRECTION_SENDRECV);
		switch (ctx->GetConfig().sip.DefaultBFCPProto.get_value_or(net::protocol::TCP))
		{
		case net::protocol::UDP:
			ms->SetProto(SDP_PROTO_UDP_BFCP);
			break;
		case net::protocol::TCP:
			ms->SetProto(SDP_PROTO_TCP_BFCP);
			break;
		}
		const auto supported_roles = ctx->GetBFCPSupportedRoles();
		ms->SetBFCPFloorCtrl(supported_roles);
		if (supported_roles & (SDP_FLOORCTRL_ROLE_S_ONLY | SDP_FLOORCTRL_ROLE_C_S))
		{
			ms->SetBFCPConfID(1);
			ms->SetBFCPUserID(2);
			ms->SetBFCPFloorID(1);
			ms->BFCPFloorLabels().emplace_back("3");
		}
		if (supported_roles & SDP_FLOORCTRL_ROLE_C_S || (supported_roles & SDP_FLOORCTRL_ROLE_C_ONLY && supported_roles & SDP_FLOORCTRL_ROLE_S_ONLY))
			ms->SetSetup(SDP_SETUP_ACTPASS);
		else if (supported_roles & SDP_FLOORCTRL_ROLE_C_ONLY)
			ms->SetSetup(SDP_SETUP_ACTIVE);
		else if (supported_roles & SDP_FLOORCTRL_ROLE_S_ONLY)
			ms->SetSetup(SDP_SETUP_PASSIVE);
		ms->SetConnectionAttr(SDP_CONNECTION_NEW);
	}

#if 1
	if (ctx->IsBFCPEnabled() && std::none_of(media_streams.begin(), media_streams.end(), [](VS_SDPField_MediaStream* ms) {
		return ms->GetMediaType() == SDPMediaType::video && ms->GetContent() == SDP_CONTENT_SLIDES;
	}))
	{
		VS_SDPField_MediaStream* const ms = ctx->MediaStream(media_streams.size(), true);
		ms->SetMediaType(SDPMediaType::video);
		ms->SetMediaDirection(SDP_MEDIACHANNELDIRECTION_SENDRECV);
		ms->SetProto(SDP_RTPPROTO_RTP_AVP);
		ms->SetContent(SDP_CONTENT_SLIDES);
		ms->SetLabel("3");
		ms->ClearLocalCodecs();
		ms->CopyLocalCodecsFrom(get_info);
	}
#endif

	if (ctx->IsH224Enabled() && std::none_of(media_streams.begin(), media_streams.end(), [](VS_SDPField_MediaStream* ms) {
		return ms->GetMediaType() == SDPMediaType::application_fecc;
	}))
	{
		VS_SDPField_MediaStream* const ms = ctx->MediaStream(media_streams.size(), true);
		ms->SetMediaType(SDPMediaType::application_fecc);
		ms->SetMediaDirection(SDP_MEDIACHANNELDIRECTION_SENDRECV);
		ms->SetProto(SDP_RTPPROTO_RTP_AVP);
		ms->ClearLocalCodecs();
		ms->CopyLocalCodecsFrom(get_info);
	}
}

void VS_SIPParser::AuthenticateAndSendInstantMessage(std::shared_ptr<VS_SIPParserInfo>& msg_ctx, const uint8_t step) {
	assert(msg_ctx != nullptr);

	switch (step)
	{
	case 0:	// step 0 - Authenticate
	{
		msg_ctx->SetByeTick(NULL_TICK);	// to allow receive 200 ok responses, ctx will be deleted in Timeout by CALL_START_TIMEOUT if will not be answered
		InitMSGMediaStream(msg_ctx);
		AddAuthInfoFromRegContext(msg_ctx, nullptr);
		if (msg_ctx->SRTPEnabled() && msg_ctx->HaveAuthenticatedTLSConnection())
		{
			auto &my_cs_addr = static_cast<const VS_SIPParserInfo&>(*msg_ctx).GetMyCsAddress();
			msg_ctx->SetSRTPKey(m_fireGetSRTPKey(msg_ctx->SIPDialogID(), my_cs_addr,
				msg_ctx->GetConfig().Address));
		}
		const VS_SIPGetInfoImpl get_info_msg(*msg_ctx);
		VS_SIPUpdateInfoImpl update_info_msg(*msg_ctx);

		auto req = std::make_shared<VS_SIPRequest>();
		if (!req->GenerateMyInfo(get_info_msg, update_info_msg) || !req->MakeINVITE(get_info_msg, update_info_msg) || !PutMessageForOutput(req)) {
			return;
		}
	}
	break;
	case 1:	// step 1 - Send msg
	{
		const VS_SIPGetInfoImpl get_info{ *msg_ctx };
		VS_SIPUpdateInfoImpl update_info{ *msg_ctx };

		auto pending_msgs = msg_ctx->PopPendingMessages();
		for (const auto& m : pending_msgs)
		{
			auto msg = std::make_shared<VS_SIPRequest>();
			msg->MakeMESSAGE(get_info, update_info, std::get<0>(m), std::get<1>(m));
			PutMessageForOutput(msg);
		}
	}
	break;
	default:
		break;
	}

}

void VS_SIPParser::InitMSGMediaStream(const std::shared_ptr<VS_SIPParserInfo>& ctx)
{
	auto& media_streams = ctx->MediaStreams();
	if (std::none_of(media_streams.begin(), media_streams.end(), [](VS_SDPField_MediaStream* ms) {
		return ms->GetMediaType() == SDPMediaType::message;
	})) {
		auto *ms = ctx->MediaStream(media_streams.size(), true);
		ms->SetMediaType(SDPMediaType::message);
		ms->SetLocalPort(5060);
		ms->SetProto(SDP_PROTO_SIP);
		ms->SetMessageURL(ctx->GetAliasMy());
		ms->AcceptTypes().emplace_back("text/plain");
	}
}

bool VS_SIPParser::SendSetMediaChannels(const std::shared_ptr<VS_SIPParserInfo>& ctx, VS_SIPParserInfo::SMCContinuation cont)
{
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return false;

	ctx->SetSMCContinuation(cont);
	ctx->SetSMCTick(clock().now());
	std::vector<VS_MediaChannelInfo> channels;
	if (FillMediaChannels(ctx->SIPDialogID(), channels))
	{
		dstream3 << "VS_SIPParser::SendSetMediaChannels: dialog=" << ctx->SIPDialogID();
		return confMethods->SetMediaChannels(string_view(ctx->SIPDialogID()), channels, ctx->GetConfID());
	}
	return false;
}

bool VS_SIPParser::SetMediaChannels(string_view dialogId, const std::vector<VS_MediaChannelInfo>& channels, const std::string& /*existingConfID*/, std::int32_t bandwRcv)
{
	dstream3 << "VS_SIPParser::SetMediaChannels(" << dialogId << "): " << channels.size() << " channels";

	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(dialogId);
	if (!ctx)
		return false;
	auto &&pConfig = ctx->GetConfig();

	auto& media_streams = ctx->MediaStreams();

	if (!ctx->IsGroupConf() && bandwRcv) {
		ctx->SetLocalBandwidth(std::min<unsigned int>(ctx->GetLocalBandwidth(), bandwRcv * 1024));
	}

	for (const auto& channel: channels)
	{
		if (channel.index >= media_streams.size())
			continue;

		VS_SDPField_MediaStream* const ms = ctx->MediaStream(channel.index);
		if (!ms)
			continue;

		if (ms->GetLocalPort() != 0)
			ms->SetLocalPort(channel.our_rtp_address.port());
	}

	if (std::any_of(media_streams.begin(), media_streams.end(), [](const VS_SDPField_MediaStream* ms) {
		return (ms->GetMediaType() == SDPMediaType::audio || ms->GetMediaType() == SDPMediaType::video) && ms->GetLocalPort() == DISCARD_PROTOCOL_PORT;
	}))
		return true; // Not all media channels are initialized by transcoder

	auto userAgent = IdentifyUserAgent(ctx);
	if (userAgent == Lync) {
		if (pConfig.Codecs.find("XH264UC") == std::string::npos)
			dstream3 << "Warning\t'XH264UC' codec is not present in 'Enabled codecs'. Video will not work!\n";
	}

	const VS_SIPGetInfoImpl get_info(*ctx);
	VS_SIPUpdateInfoImpl update_info(*ctx);

	switch (ctx->GetSMCContinuation())
	{
	case VS_SIPParserInfo::SMCContinuation::SendINVITE:
	{
		dstream3 << "VS_SIPParser::SetMediaChannels(" << dialogId << "): sending pending invite";
		auto req = std::make_shared<VS_SIPRequest>();
		//VS_SIPRequest req;

		// generate info if it was not set before (for unit testing)
		if (ctx->MyBranch().empty() ||
			ctx->GetTagMy().empty())
		{
			if (!req->GenerateMyInfo(get_info, update_info))
				return false;
		}

		if (pConfig.sip.RegistrationBehavior.get_value_or(VS_CallConfig::REG_UNDEFINED) == VS_CallConfig::REG_REGISTER_ON_CALL)
		{
			boost::optional<std::string> reg_dialog_id = UpdateRegisterContext(pConfig, true);
			std::shared_ptr<VS_SIPParserInfo> r_ctx;
			if (reg_dialog_id.is_initialized())
				r_ctx = GetParserContext(*reg_dialog_id);

			if (!r_ctx)
			{
				CleanParserContext(ctx->SIPDialogID(), SourceClean::PARSER);
				return false;
			}

			r_ctx->SetInviteAfterRegister(ctx); // make invite on ctx after register
			ctx->SetInviteAfterRegister(r_ctx);
		}
		else
		{
			if (ctx->NeedsOptionsBeforeInvite())
			{
				if (!DoInviteWithOptions(ctx))
					return false;
			}
			else
			{
				if (!DoInvite(ctx))
				{
					return false;
				}
			}
		}
		ctx->SetSMCContinuation(VS_SIPParserInfo::SMCContinuation::Nothing);
	}
	break;
	case VS_SIPParserInfo::SMCContinuation::SendResponse:
	{
		std::shared_ptr<VS_SIPRequest> invite(ctx->GetLastInvite());
		if (!invite)
			return true;

		dstream3 << "VS_SIPParser::SetMediaChannels(" << dialogId << "): sending pending response for invite: cseq=" << invite->GetCSeq();
		std::shared_ptr<VS_SIPResponse> rsp = std::make_shared<VS_SIPResponse>();
		if (!rsp->MakeOnInviteResponseOK(invite.get(), get_info, update_info) || !PutMessageForOutput(rsp)) {
			return false;
		}

		if (userAgent && ctx->IsGroupConf()) {
			auto &&dialog_id = NewDialogID(ctx->GetSIPRemoteTarget(), {}, ctx->GetConfig());
			auto msg_ctx = GetParserContext(dialog_id, true);

			msg_ctx->SetAliasMy(ctx->GetAliasMy());

			VS_SDPField_MediaStream *ms = msg_ctx->MediaStream(0, true);
			ms->SetMediaType(SDPMediaType::message);
			ms->SetLocalPort(5060);
			ms->SetProto(SDP_PROTO_SIP);
			ms->SetMessageURL("null");
			ms->SetAcceptTypes({ "text/plain" });

			AddAuthInfoFromRegContext(msg_ctx, nullptr);

			const VS_SIPGetInfoImpl get_info_msg(*msg_ctx);
			VS_SIPUpdateInfoImpl update_info_msg(*msg_ctx);

			auto req = std::make_shared<VS_SIPRequest>();
			if (!req->GenerateMyInfo(get_info_msg, update_info_msg) || !req->MakeINVITE(get_info_msg, update_info_msg) || !PutMessageForOutput(req)) {
				return false;
			}
		}

		if (!ctx->IsAnswered())
		{
			m_policy->SetResult(pConfig.Address.addr.to_string(vs::ignore<boost::system::error_code>()) + "_incomming_invite", "sip", true, eStartLineType::TYPE_INVITE);
			ctx->SetAnswered(clock().now());
			ctx->SetDTMFRequestTime(clock().now());
		}
		ctx->SetSMCContinuation(VS_SIPParserInfo::SMCContinuation::Nothing);
	}
	}

	return true;
}

bool VS_SIPParser::DoInvite(const std::shared_ptr<VS_SIPParserInfo> &ctx) {
	auto req = std::make_shared<VS_SIPRequest>();
	const VS_SIPGetInfoImpl get_info(*ctx);
	VS_SIPUpdateInfoImpl update_info(*ctx);

	if (ctx->GetTagMy().empty() || ctx->MyBranch().empty()) {
		if (!req->GenerateMyInfo(get_info, update_info)) {
			return false;
		}
	}
	auto &&config = ctx->GetConfig();
	if (config.sip.RegistrationBehavior == VS_CallConfig::REG_REGISTER_ON_CALL) {
		boost::optional<std::string> reg_dialog_id = UpdateRegisterContext(config, true);
		std::shared_ptr<VS_SIPParserInfo> r_ctx;
		if (reg_dialog_id.is_initialized())
			r_ctx = GetParserContext(*reg_dialog_id);

		if (!r_ctx) {
			CleanParserContext(ctx->SIPDialogID(), SourceClean::PARSER);
			return false;
		}

		r_ctx->SetInviteAfterRegister(ctx); // make invite on ctx after register
		ctx->SetInviteAfterRegister(r_ctx);
		return true;
	}
	else {
		return req->MakeINVITE(get_info, update_info) && PutMessageForOutput(req);
	}
}

bool VS_SIPParser::DoInviteWithOptions(const std::shared_ptr<VS_SIPParserInfo> &ctx) {
	auto req = std::make_shared<VS_SIPRequest>();

	const VS_SIPGetInfoImpl get_info(*ctx);
	VS_SIPUpdateInfoImpl update_info(*ctx);

	if (!req->GenerateMyInfo(get_info, update_info))
	{
		return false;
	}
	if (!req->MakeOPTIONS(get_info, update_info, true)) {
		return false;
	}
	// Options request will be timed out after OPTIONS_TIMEOUT
	ctx->SetOptionsTick(clock().now());
	// Save request branch to be able to cancel it from retransmission
	ctx->SetInviteAfterOptions(std::string(req->Branch()));
	// See sip::TransportLayer::FillMsgOutgoing()
	ctx->NeedUpdateOptionsBranch(true);
	if (!PutMessageForOutput(req)) {
		return false;
	}
	// INVITE will be sent eiter in:
	// - VS_SIPParser::OnResponse_Code200 or
	// - VS_SIPParser::Timeout
	return true;
}

bool VS_SIPParser::FillMediaChannels(string_view dialogId, std::vector<VS_MediaChannelInfo>& channels)
{
	dstream3 << "VS_SIPParser::FillMediaChannels(" << dialogId << ")";
	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(dialogId);
	if (!ctx)
		return false;

	channels.clear();
	const unsigned int our_bw = ctx->GetLocalBandwidth();
	unsigned int sdp_bw = ctx->GetRemoteBandwidth();
	const bool siren_swap_bytes = ctx->GetConfig().codecParams.siren_swap_bytes.get_value_or(false);
	for (size_t index = 0; index < ctx->MediaStreams().size(); ++index)
	{
		VS_SDPField_MediaStream* const ms = ctx->MediaStream(index);
		if (!ms)
			continue;
		if (!ms->IsEnabled())
			continue;
		if (ms->GetMediaType() != SDPMediaType::audio &&
			ms->GetMediaType() != SDPMediaType::video &&
			ms->GetMediaType() != SDPMediaType::application_fecc)
			continue;

		channels.emplace_back(index);
		VS_MediaChannelInfo& channel = channels.back();
		channel.type = ms->GetMediaType();
		channel.content = ms->GetContent();
		channel.direction = ms->GetMediaDirection();

		{
			boost::system::error_code ec;
			auto &host = ms->GetHost();
			net::address addr = net::address::from_string(host, ec);
			if (ec)
			{
				bool found_ip = false;
				auto res = net::dns::make_a_aaaa_lookup(host).get();
				for (auto item : { &res.first, &res.second })
				{
					assert(item);
					if (!item->ec) // no error
					{
						assert(!item->host.addrs.empty());
						found_ip = true;
						addr = item->host.addrs.front();
					}
				}

				if (!found_ip)
				{
					const auto &cfg_addr = ctx->GetConfig().Address;
					addr = cfg_addr.addr;
				}

			}
			channel.remote_rtp_address.address(std::move(addr));
			channel.remote_rtcp_address = channel.remote_rtp_address;
		}

		const net::port port = ms->GetPort();

		channel.remote_rtp_address.port(port);
		channel.remote_rtcp_address.port(port + 1);

		// modes
		if (ms->IsRecv())
		{
			for (auto& codec : ms->GetLocalCodecs())
			{
				if (channel.type == SDPMediaType::application_fecc)
				{
					VS_GatewayDataMode mode;
					mode.CodecType = static_cast<VS_H323DataCodec>(codec->GetCodecType());
					mode.PayloadType = codec->GetPT();
					mode.ExtendedCodec = false;
					channel.rcv_modes_data.push_back(mode);
				}
				else if (channel.type == SDPMediaType::audio)
				{
					VS_GatewayAudioMode mode;
					mode.PayloadType = codec->GetPT();
					mode.CodecType = codec->GetCodecType();
					mode.ClockRate = codec->GetClockRate();
					switch (mode.CodecType)
					{
					case e_rcvSIREN14_24:
					case e_rcvSIREN14_32:
					case e_rcvSIREN14_48:
						mode.SwapBytes = siren_swap_bytes;
						break;
					}
					channel.rcv_modes_audio.push_back(mode);
				}
				else if (channel.type == SDPMediaType::video)
				{
					VS_GatewayVideoMode mode;
					mode.PayloadType = codec->GetPT();
					mode.CodecType = codec->GetCodecType();
					mode.Bitrate = ms->GetBandwidth();
					codec->FillRcvVideoMode(mode);
					mode.IsFIRSupported = ms->GetFIRSupport();
					mode.ClockRate = codec->GetClockRate();
					channel.rcv_modes_video.push_back(mode);
				}
			}
			if (channel.type != SDPMediaType::application_fecc)
			{
			channel.our_ice_pwd = ms->GetOurIcePwd();
			channel.remote_ice_pwd = ms->GetRemoteIcePwd();
			channel.our_srtp_key = ms->GetOurCryptoKey();
			channel.remote_srtp_key = ms->GetRemoteCryptoKey();
		}
		}

		if (ms->IsSend())
		{
			if (channel.type == SDPMediaType::application_fecc)
			{
				channel.snd_mode_data.PayloadType =
					channel.rcv_modes_data.empty() ?
						-1 :
						channel.rcv_modes_data.front().PayloadType;
				channel.snd_mode_data.CodecType =
					channel.rcv_modes_data.empty() ?
						VS_H323DataCodec::dataNone :
						channel.rcv_modes_data.front().CodecType;
				channel.snd_mode_data.ExtendedCodec = false;
			}
			else
			{
			auto codec = ms->GetBestCodec();
			if (!codec)
				continue;

			if (channel.type == SDPMediaType::audio)
			{
				channel.snd_mode_audio.PayloadType = codec->GetPT();
				channel.snd_mode_audio.CodecType = codec->GetCodecType();
				channel.snd_mode_audio.ClockRate = codec->GetClockRate();
				switch (channel.snd_mode_audio.CodecType)
				{
				case e_rcvSIREN14_24:
				case e_rcvSIREN14_32:
				case e_rcvSIREN14_48:
					channel.snd_mode_audio.SwapBytes = siren_swap_bytes;
					break;
				}
			}
			else if (channel.type == SDPMediaType::video)
			{
				auto &&config = ctx->GetConfig();
				if (config.codecParams.h264_force_cif_mixer.is_initialized() && config.codecParams.h264_force_cif_mixer)
				{
					channel.snd_mode_video.IsMixerCIFMode = true;
				}
				if (config.codecParams.h264_snd_preferred_width.is_initialized() && *config.codecParams.h264_snd_preferred_width > 0 &&
					config.codecParams.h264_snd_preferred_height.is_initialized() && *config.codecParams.h264_snd_preferred_height > 0)
				{
					channel.snd_mode_video.preferred_width = *config.codecParams.h264_snd_preferred_width;
					channel.snd_mode_video.preferred_height = *config.codecParams.h264_snd_preferred_height;
				}
				if (config.codecParams.gconf_to_term_width.is_initialized() && config.codecParams.gconf_to_term_width &&
					config.codecParams.gconf_to_term_height.is_initialized() && config.codecParams.gconf_to_term_height)
				{
					channel.snd_mode_video.gconf_to_term_width = *config.codecParams.gconf_to_term_width;
					channel.snd_mode_video.gconf_to_term_height = *config.codecParams.gconf_to_term_height;
				}
				channel.snd_mode_video.PayloadType = codec->GetPT();
				channel.snd_mode_video.CodecType = codec->GetCodecType();
				channel.snd_mode_video.ClockRate = codec->GetClockRate();
				codec->FillSndVideoMode(channel.snd_mode_video);

				unsigned int ms_bw = ms->GetBandwidth();
				unsigned int bw = (sdp_bw&&ms_bw) ? std::min(sdp_bw, ms_bw) : (ms_bw) ? ms_bw : sdp_bw;
				std::uint32_t snd_video_bitrate;
				if (bw)
					snd_video_bitrate = our_bw ? std::min(bw, our_bw) : bw;
				else
				{
					if (our_bw && our_bw < 768 * 1024)
						snd_video_bitrate = our_bw;
					else
						snd_video_bitrate = 768 * 1024;
				}
				channel.snd_mode_video.Bitrate = !channel.snd_mode_video.Bitrate ? snd_video_bitrate : std::min<unsigned long>(snd_video_bitrate, channel.snd_mode_video.Bitrate);
			}
			}
			channel.ssrc_range = ms->GetOurSsrcRange();
		}
	}
	return true;
}

bool VS_SIPParser::ReqInvite(string_view dialogId, string_view fromId) {
	if (fromId.empty())
		return false;

	auto ctx = GetParserContext(dialogId);
	if (!ctx || !ctx->IsAnswered() || ctx->GetUser().empty()) return false;

	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return true;

	VS_RealUserLogin default_sip_id(m_get_app_property("default_call_destination_sip"));
	if (default_sip_id.GetID())
	{
		string_view id_view{ default_sip_id.GetID() };
		if (boost::iequals(id_view, fromId))
		{
			confMethods->ReqInviteReply(ctx->SIPDialogID(), fromId, true);
			return true;
		}
	}

	omembuf streambuf;
	std::ostream stream(&streambuf);
	const auto buff_size = 512;

	assert(std::string("User " + std::string(fromId) + " wants to join conference. Type /y " + std::string(fromId) + " to accept or /n "
		+ std::string(fromId) + " to reject.").length() < buff_size);

	char msg[buff_size] = {};
	streambuf.pubsetbuf(msg, buff_size - 1 /*0-terminator*/);
	stream << "User " << fromId << " wants to join conference. Type /y " << fromId << " to accept or /n " << fromId << " to reject.";
	const std::size_t len = streambuf.pubseekoff(0, std::ios_base::cur);

	auto chat_ctx = FindActiveMsgCtx(ctx);
	if (!chat_ctx)
	{
		auto new_dialog = NewDialogID(fromId, {}, ctx->GetConfig());
		this->Chat(new_dialog, ctx->GetAliasMy(), ctx->GetAliasRemote(), ctx->GetDisplayNameMy(), msg);
		auto new_ctx = GetParserContext(new_dialog);
		if (new_ctx) new_ctx->AddPendingReqInviteUser(std::string(fromId));
		return true;
	}

	auto req = std::make_shared<VS_SIPRequest>();
	const VS_SIPGetInfoImpl get_info(*chat_ctx);
	VS_SIPUpdateInfoImpl update_info(*chat_ctx);

	if (req->MakeMESSAGE(get_info, update_info, string_view{ msg, len }) &&
		PutMessageForOutput(req)) {
		chat_ctx->AddPendingReqInviteUser(std::string(fromId));
	}

	return true;
}

void VS_SIPParser::TakeTribuneReply(string_view dialogId, bool result) {
	auto ctx = GetParserContext(dialogId);
	if (!ctx) return;

	auto msg_ctx = FindActiveMsgCtx(ctx);
	if (msg_ctx) {
		string_view m;
		if (result) {
			m = "You are broadcasting to all participants.";
		}
		else {
			m = "Rejected.";
		}

		auto msg = std::make_shared<VS_SIPRequest>();

		const VS_SIPGetInfoImpl get_info(*msg_ctx);
		VS_SIPUpdateInfoImpl update_info(*msg_ctx);

		msg->MakeMESSAGE(get_info, update_info, m);
		PutMessageForOutput(msg);
	}
}

void VS_SIPParser::LeaveTribuneReply(string_view dialogId, bool result) {
	auto ctx = GetParserContext(dialogId);
	if (!ctx) return;

	auto msg_ctx = FindActiveMsgCtx(ctx);
	if (msg_ctx) {
		string_view m;
		if (result) {
			m = "You stopped broadcasting";
		}
		else {
			m = "You are not broadcasting";
		}

		auto msg = std::make_shared<VS_SIPRequest>();
		const VS_SIPGetInfoImpl get_info(*msg_ctx);
		VS_SIPUpdateInfoImpl update_info(*msg_ctx);
		msg->MakeMESSAGE(get_info, update_info, m);
		PutMessageForOutput(msg);
	}
}

bool VS_SIPParser::InitBFCP(const std::shared_ptr<VS_SIPParserInfo>& ctx)
{
	auto& media_streams = ctx->MediaStreams();
	auto bfcp_ms_it = std::find_if(media_streams.begin(), media_streams.end(), [](const VS_SDPField_MediaStream* ms) {
		return ms->GetMediaType() == SDPMediaType::application_bfcp;
	});
	if (bfcp_ms_it == media_streams.end())
		return false;
	VS_SDPField_MediaStream* const bfcp_ms = *bfcp_ms_it;
	if (!bfcp_ms->IsEnabled())
		return false;

	const bool udp = bfcp_ms->GetProto() == SDP_PROTO_UDP_BFCP;

	if ((bfcp_ms->GetBFCPFloorCtrl() & SDP_FLOORCTRL_ROLE_S_ONLY) != 0
	 || (bfcp_ms->GetBFCPFloorCtrl() & SDP_FLOORCTRL_ROLE_C_S) != 0)
	{
		ctx->SetBFCPClientSession(nullptr);
		auto conf_id = bfcp_ms->GetBFCPConfID();
		if (!ctx->GetBFCPServerSession() && conf_id != 0)
		{
			auto session = boost::make_shared<bfcp::ServerSession>(conf_id, udp);
			session->SendStatusForAllFloors(true); // Workaround for Cisco E20 not sending any floor ids in FloorQuery
			session->AddUser(bfcp_ms->GetBFCPUserID()); // Workaround for Polycom HDX8000 not sending FloorQuery at all
			ctx->SetBFCPServerSession(session);
			UpdateSlideshowState(ctx, ctx->GetSlideshowState());
		}
	}
	else if ((bfcp_ms->GetBFCPFloorCtrl() & SDP_FLOORCTRL_ROLE_C_ONLY) != 0)
	{
		ctx->SetBFCPServerSession(nullptr);
		auto conf_id = bfcp_ms->GetBFCPConfID();
		auto user_id = bfcp_ms->GetBFCPUserID();
		if (!ctx->GetBFCPClientSession() && conf_id != 0 && user_id != 0)
		{
			auto session = boost::make_shared<bfcp::ClientSession>(conf_id, user_id, udp);
			session->SubscribeToFloorStatus({ bfcp::FloorID(bfcp_ms->GetBFCPFloorID()) });
			ctx->SetBFCPClientSession(session);
			UpdateSlideshowState(ctx, ctx->GetSlideshowState());
		}
	}
	else
		return false;

	auto channel = ctx->GetBFCPChannel();
	if (!channel)
	{
		channel = VS_SignalChannel::Create(m_transportStrand.get_io_service(), m_logger);
		channel->ConnectToDataReceived([weak_ctx = std::weak_ptr<VS_SIPParserInfo>(ctx), w_this = this->weak_from_this(), this](const void* data, size_t size)
		{
			auto parser = w_this.lock();
			if (!parser)
				return;
			vs::SharedBuffer data_tmp(size);
			::memcpy(data_tmp.data(), data, size);
			m_transportStrand.post([weak_ctx, data_tmp = std::move(data_tmp), size]() {
				auto ctx = weak_ctx.lock();
				if (!ctx)
					return;
				boost::shared_ptr<bfcp::SessionBase> session;
				if (!(session = ctx->GetBFCPClientSession())
					&& !(session = ctx->GetBFCPServerSession()))
					return;

				session->SetRecvData(data_tmp.data<const void>(), size);
			});
		});
		ctx->SetBFCPChannel(channel);
	}

	net::address remote_address = net::address::from_string(bfcp_ms->GetHost(), vs::ignore<boost::system::error_code>());
	net::port remote_port = bfcp_ms->GetPort();
	if (remote_port == DISCARD_PROTOCOL_PORT)
		remote_port = 0;
	net::port port = channel->LocalPort();

	std::uint32_t channel_flags = 0;
	if (bfcp_ms->GetSetup() == SDP_SETUP_PASSIVE || bfcp_ms->GetSetup() == SDP_SETUP_ACTPASS)
		channel_flags |= udp ? VS_SignalChannel::LISTEN_UDP : VS_SignalChannel::LISTEN_TCP;
	if (bfcp_ms->GetSetup() == SDP_SETUP_ACTIVE || bfcp_ms->GetSetup() == SDP_SETUP_ACTPASS)
		channel_flags |= udp ? VS_SignalChannel::CONNECT_UDP : VS_SignalChannel::CONNECT_TCP;

	auto flow = net::QoSSettings::GetInstance().GetSIPQoSFlow(udp, false);
	if (channel_flags & (VS_SignalChannel::LISTEN_TCP | VS_SignalChannel::LISTEN_UDP) && port == 0)
	{
		VS_RegistryKey key(false, CONFIGURATION_KEY);
		int32_t minPort(0), maxPort(0);
		key.GetValue(&minPort, sizeof(minPort), VS_REG_INTEGER_VT, "BFCP MinPort");
		key.GetValue(&maxPort, sizeof(maxPort), VS_REG_INTEGER_VT, "BFCP MaxPort");
		if (!minPort || !maxPort || minPort > maxPort)
		{
			minPort = DEFAULT_BFCP_MINPORT;
			maxPort = DEFAULT_BFCP_MAXPORT;
		}
		static std::atomic<net::port> lastUsedPort{ 0 };
		net::port port;
		int32_t attempt;
		for (attempt = 0; attempt < maxPort - minPort + 1; ++attempt)
		{
//			Obtain a free port
			auto prevPort = lastUsedPort.load(std::memory_order_relaxed);// load last
			do
			{
				port = prevPort + 1;// set next
				if (port < minPort || port > maxPort) // check borders
					port = minPort;
			} while (!lastUsedPort.compare_exchange_weak(prevPort, port, std::memory_order_relaxed)); // check if someone else hasn't changed the value{
			if (channel->Open(channel_flags, net::address_v4::any(), port, remote_address, remote_port, flow))
				break;
		}
		if (attempt == maxPort - minPort + 1)
			return false;
	}
	else
		if (!channel->Open(channel_flags, net::address_v4::any(), port, remote_address, remote_port, flow))
			return false;
	bfcp_ms->SetLocalPort(channel->LocalPort());

	return true;
}

void VS_SIPParser::UpdateSlideshowState(const std::shared_ptr<VS_SIPParserInfo>& ctx, bool active)
{
	ctx->SetSlideshowState(active);

	auto& media_streams = ctx->MediaStreams();
	auto bfcp_ms_it = std::find_if(media_streams.cbegin(), media_streams.cend(), [](const VS_SDPField_MediaStream* ms) {
		return ms->GetMediaType() == SDPMediaType::application_bfcp;
	});
	if (bfcp_ms_it == media_streams.cend())
		return;
	VS_SDPField_MediaStream* const bfcp_ms = *bfcp_ms_it;

	if (auto client_session = ctx->GetBFCPClientSession())
	{
		if (active)
			client_session->RequestFloor(bfcp_ms->GetBFCPFloorID());
		else
			client_session->ReleaseFloor(bfcp_ms->GetBFCPFloorID());
	}
	else if (auto server_session = ctx->GetBFCPServerSession())
	{
		const bfcp::UserID our_user_id = bfcp_ms->GetBFCPUserID() + 10;
		if (active)
			server_session->RequestFloor(bfcp_ms->GetBFCPFloorID(), our_user_id);
		else
			server_session->ReleaseFloor(bfcp_ms->GetBFCPFloorID(), our_user_id);
	}
}

bool VS_SIPParser::IsTrunkFull()
{
	return false;
}

bool VS_SIPParser::DoRegister(string_view dialogId, const bool updateData)
{
	if (!VS_ParserInterface::DoRegister(dialogId, updateData)) return false;
	std::shared_ptr<VS_SIPParserInfo> info = GetParserContext(dialogId, false);
	if (info.get() == nullptr) return false;
	info->IsRegisterContext(true);

	auto ctx_auth = info->GetAuthScheme();
	if (ctx_auth && ctx_auth->scheme() == SIP_AUTHSCHEME_NTLM) {
		auto new_ctx_gss = std::make_shared<VS_SIPAuthGSS>(ctx_auth->scheme());
		auto ctx_auth_gss = std::dynamic_pointer_cast<VS_SIPAuthGSS>(ctx_auth);
		if (ctx_auth_gss)
		{
			*new_ctx_gss = *ctx_auth_gss;
			new_ctx_gss->gssapi_data(nullptr);
			new_ctx_gss->cnum(0);
			info->SetAuthScheme(new_ctx_gss);
		}
	}
	if (info->GetConfig().sip.UserStatusScheme == VS_CallConfig::eUserStatusScheme::SKYPE4BUSINESS) //case discovered in RT: we need this fields in first REGISTER msg
	{
		std::string epid = VS_UUID::GenEpid(info->GetAliasMy()/*self_uri*/, "tc", info->GetMyCsAddress().addr.to_string());
		std::string uuid = VS_UUID::GenUUID(epid);

		info->SetEpidMy(std::move(epid));
		info->SipInstance("<urn:uuid:" + std::move(uuid) + ">");
		info->SetExpires(std::chrono::seconds(300));
	}

	auto req = std::make_shared<VS_SIPRequest>();
	info->SetTagSip({});
	info->SetTagMy({});
	auto authScheme = info->GetAuthScheme();
	int authType = authScheme ? authScheme->auth_type() : VS_SIPAuthInfo::TYPE_AUTH_INVALID;

	VS_SIPObjectFactory::SIPHeader authHeader(VS_SIPObjectFactory::SIPHeader::SIPHeader_Invalid);
	switch (authType) {
	case VS_SIPAuthInfo::TYPE_AUTH_PROXY_TO_USER:	authHeader = VS_SIPObjectFactory::SIPHeader::SIPHeader_ProxyAuthorization;	break;
	case VS_SIPAuthInfo::TYPE_AUTH_USER_TO_USER:	authHeader = VS_SIPObjectFactory::SIPHeader::SIPHeader_Authorization;		break;
	case  VS_SIPAuthInfo::TYPE_AUTH_INVALID:
	default:
		authHeader = VS_SIPObjectFactory::SIPHeader::SIPHeader_Invalid; break;
	}

	const VS_SIPGetInfoImpl get_info{ *info };
	VS_SIPUpdateInfoImpl update_info{ *info };

	return req->MakeREGISTER(get_info, update_info, authHeader) && PutMessageForOutput(req);
}

bool VS_SIPParser::UpdateSIPDisplayName(const std::shared_ptr<VS_SIPParserInfo>& ctx, const bool updateImmediately)
{
	if (!ctx)
		return false;

	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return false;

	confMethods->UpdateDisplayName(ctx->SIPDialogID(), ctx->GetDisplayNameSip(), updateImmediately);
	return true;
}

bool VS_SIPParser::OnRequest_REGISTER(const std::shared_ptr<VS_SIPRequest>& req, const net::Endpoint &recvFromEp)
{
	if (!req || !req->IsValid())
		return false;

	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return false;

	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(req->CallID(), true);
	ctx->GetConfig().Address = recvFromEp;

	auto a = req->GetAuthInfo();
	const VS_SIPGetInfoImpl get_info{ *ctx };
	VS_SIPUpdateInfoImpl update_info{ *ctx };

	if (!req->FillInfoByInviteMessage(update_info))
		return false;

	if (!!a && g_our_nonce_storage.IsOurNonce(a->nonce()))
	{
		std::string pass = "REGISTER:" + a->uri();
		char res[400];
		VS_ConvertToMD5(pass, res);
		pass = "$4*" + a->nonce() + "*" + res + "*" + a->response();
		std::transform(pass.begin(), pass.end(), pass.begin(), ::tolower);

		{
			VS_RealUserLogin r(a->login());
			if (!!m_check_digest && !m_check_digest(r.GetUser(), pass))
			{
				dprint3("VS_SIPParser::OnRequest_REGISTER Digest check failed\n");
				std::shared_ptr<VS_SIPResponse> rsp = std::make_shared<VS_SIPResponse>();
				if (!rsp->MakeOnRegisterResponseForbidden(req.get(), get_info, update_info)) return false;
				return PutMessageForOutput(rsp);
			}
		}

		auto auth_scheme = std::make_shared<VS_SIPAuthDigest>();
		auth_scheme->AddInfo(*a);
		ctx->SetAuthScheme(auth_scheme);

		std::chrono::seconds expire(3600);		// proposed default value by RFC 3261
		if (req->GetSIPMetaField()->iContact != nullptr)		// "expires" parameter in Contact header has higher priority than value in Expires header
		{
			const auto expire_opt = req->GetSIPMetaField()->iContact->GetExpires();
			if (!expire_opt.is_initialized()) // no "expires" in Contact
			{
				if (req->GetSIPMetaField()->iExpires != nullptr)
					expire = req->GetSIPMetaField()->iExpires->Value();
			}
			else
				expire = expire_opt.get();
		}

		if(expire > m_regMaxExpires)
		{
			expire = m_regMaxExpires;
		}

		ctx->SetExpires(expire);


		ctx->SetUser(a->login());

		ctx->SetPasswordTranscoder(std::move(pass));

		std::string contact;
		if (req->GetSIPMetaField()->iContact && req->GetSIPMetaField()->iContact->GetLastURI()) {
			req->GetSIPMetaField()->iContact->GetLastURI()->GetRequestURI(contact);
			ctx->SetSIPContact(req->GetSIPMetaField()->iContact);
		}
		if (contact.find('@') == std::string::npos)
			contact = "@" + contact;
		contact = "#sip:" + contact;

		std::shared_ptr<VS_SIPResponse> res_ok = std::make_shared<VS_SIPResponse>();
		std::shared_ptr<VS_SIPResponse> res_forbidden = std::make_shared<VS_SIPResponse>();
		res_ok->MakeOnRegisterResponseOK(req.get(), get_info, update_info);
		res_forbidden->MakeOnRegisterResponseForbidden(req.get(), get_info, update_info);

		if (expire.count())
		{
			dprint3("VS_SIPParser::OnRequest_REGISTER try to register user = \"%s\" \n", ctx->GetUser().c_str());
			UpdateCallConfig(ctx, ctx->GetConfig().Address, false);
			confMethods->SetUserEndpointAppInfo(ctx->SIPDialogID(), req->UserAgent(), {});

			m_policy->Request(recvFromEp.addr.to_string(vs::ignore<boost::system::error_code>()), ctx->GetUser(),
				boost::bind(&VS_SIPParser::LoginAllowed,
					std::static_pointer_cast<VS_SIPParser>(shared_from_this()), _1, std::chrono::seconds(expire), contact, res_ok, res_forbidden
				), TRY_LOGIN
			);
		}
		else {
			dprint3("VS_SIPParser::OnRequest_REGISTER Logout %s\n", a->login().c_str());
			auto &&onLogoutRsp = [this, w_this = this->weak_from_this(), res_ok, res_forbidden](bool res) {
				auto self = w_this.lock();
				if (!self)
					return;
				onLogoutResponse(res, res_ok, res_forbidden, true);
			};
			auto &&logout = [onLogoutRsp]() {
				onLogoutRsp(true);
			};
			confMethods->LoginUser(ctx->SIPDialogID(), a->login(), ctx->GetPasswordTranscoder(), NULL_TICK, contact, onLogoutRsp, logout, {});
		}
	}
	else
	{
		dprint3("VS_SIPParser::OnRequest_REGISTER Unauthorized\n");

		std::string new_nonce = VS_SIPAuthDigest::GenerateNonceValue();
		g_our_nonce_storage.AddOurNonce(new_nonce);

		std::shared_ptr<VS_SIPResponse> rsp = std::make_shared<VS_SIPResponse>();
		if (!rsp->MakeOnRegisterResponseUnauthorized(req.get(), get_info, update_info, new_nonce)) return false;
		if (!PutMessageForOutput(rsp)) return false;
	}
	return true;
}

bool VS_SIPParser::OnRequest_SUBSCRIBE(const std::shared_ptr<VS_SIPRequest>& req)
{
	return Request_Unsupported(req);
}

bool VS_SIPParser::OnRequest_PUBLISH(const std::shared_ptr<VS_SIPRequest>& req)
{
	return Request_Unsupported(req);
}

bool VS_SIPParser::Request_Unsupported(const std::shared_ptr<VS_SIPRequest>& req)
{
	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(req->CallID());
	if (!ctx)
	{
		ctx = GetParserContext(req->CallID(), true);
	}
	VS_SIPUpdateInfoImpl update_info(*ctx);
	const VS_SIPGetInfoImpl get_info(*ctx);
	req->FillInfoByInviteMessage(update_info);

	std::shared_ptr<VS_SIPResponse> res = std::make_shared<VS_SIPResponse>();
	return res->MakeResponseUnsupported(req.get(), get_info, update_info) && PutMessageForOutput(res, ctx);;
}


void VS_SIPParser::LoginAllowed(bool isAllowed, std::chrono::seconds expireTime, std::string &externalName,
	std::shared_ptr<VS_SIPResponse> &ok, std::shared_ptr<VS_SIPResponse> &fail)
{
	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(ok->CallID());
	if (!ctx) return;
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;

	if (!isAllowed)
	{
		VS_UserLoggedin_Result r = m_policy->FailResult(TRY_LOGIN);
		if (r == SILENT_REJECT_LOGIN) return;
		PutMessageForOutput(fail, ctx);
		CleanParserContext( fail->CallID(), VS_ParserInterface::SourceClean::PARSER );
	} else
	{
		confMethods->LoginUser(ctx->SIPDialogID(), ctx->GetUser(), ctx->GetPasswordTranscoder(), expireTime + clock().now(), externalName,
			boost::bind(&VS_SIPParser::onLoginResponse, std::static_pointer_cast<VS_SIPParser>(shared_from_this()),
				_1, ok, fail),
			boost::bind(&VS_SIPParser::onLogout, std::static_pointer_cast<VS_SIPParser>(shared_from_this()),
				ctx->SIPDialogID()), std::vector<std::string>());
	}
}

void VS_SIPParser::onLoginResponse(bool res, std::shared_ptr<VS_SIPResponse> res_ok, std::shared_ptr<VS_SIPResponse> fail)
{
	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(res_ok->CallID());
	if (ctx)
		m_policy->SetResult(ctx->GetConfig().Address.addr.to_string(vs::ignore<boost::system::error_code>()), ctx->GetUser(), res, TRY_LOGIN);

	if (res) {
		if (ctx) {
			ctx->SetRegisterTick(clock().now());
		}
		PutMessageForOutput(res_ok);
	}
	else {
		PutMessageForOutput(fail, ctx);
		CleanParserContext( fail->CallID(), SourceClean::PARSER);
	}
}
void VS_SIPParser::onLogout(string_view dialogId)
{
	dstream4 << "VS_SIPParser::onLogout(" << dialogId << ")";
	if (!dialogId.length())
		return;
	CleanParserContext(dialogId, SourceClean::PARSER);
}

void VS_SIPParser::onLogoutResponse(bool res, std::shared_ptr<VS_SIPResponse> res_ok, std::shared_ptr<VS_SIPResponse> fail, bool doSend200ok)
{
	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(res_ok->CallID());
	if (doSend200ok) { // Called when handling the REGISTER message.
		// We should not delete the context too early because SIPTransportLayer
		// would not be able to create a response.
		PutMessageForOutput(res_ok, ctx);
		return; // It is going to be called in SIPParser::Timeout() where it is safe to clear the parser.
	}
	CleanParserContext( res_ok->CallID(), SourceClean::PARSER);
}

void VS_SIPParser::AsyncInviteResult(bool redirect, ConferenceStatus status, const std::shared_ptr<VS_SIPParserInfo> &ctx, string_view ip)
{
	std::shared_ptr<VS_SIPResponse> rsp = std::make_shared<VS_SIPResponse>();
	VS_SIPUpdateInfoImpl update_info(*ctx);
	const VS_SIPGetInfoImpl get_info(*ctx);
	if (redirect)
	{
		assert(status == ConferenceStatus::UNDEFINED);
		//ctx->SetMyCsAddress(1, 0, net::protocol::INVALID);
		ctx->SetMyCsAddress({ {}, 0, net::protocol::none });
		ctx->SetMyExternalCsAddress(std::string(ip));
		bool res = rsp->MakeMovedPermanently(get_info, update_info);
		CleanParserContext(ctx->SIPDialogID(), SourceClean::PARSER);
		if (!res) return;
	}
	else
		if (status == ConferenceStatus::AVAILABLE) //ringing
		{
			if (!rsp->MakeOnInviteResponseRinging(get_info, update_info))
				return;
			ctx->SetRingingStartTick(clock().now());
		}
		else
		{
			if (status == ConferenceStatus::BUSY_HERE)
			{
				if (!rsp->MakeOnInviteResponseBusyHere(get_info, update_info))
					return;
			}
			else if (status == ConferenceStatus::TMP_UNAVAILABLE)
			{
				if (!rsp->MakeOnInviteResponseTemporarilyUnavailable(get_info, update_info))
					return;
			}
			else if (status == ConferenceStatus::SRV_UNAVAILABLE)
			{
				ctx->SetRetryAfterValue(m_retryAfterHeader);
				if (!rsp->MakeOnInviteResponseServiceUnavailable(get_info, update_info))
					return;
			}
			else
			{
				if (!rsp->MakeOnInviteResponseNotFound(get_info, update_info))
					return;
			}

			ctx->SetByeTick(clock().now());
		}

	const auto res = PutMessageForOutput(rsp);
	assert(res);
}


void VS_SIPParser::SetDigestChecker(const std::function<bool(const std::string&, const std::string&)>& f)
{
	m_check_digest = f;
}

bool VS_SIPParser::IsConfOwner(std::shared_ptr<VS_SIPParserInfo> &callCtx, string_view from) const
{
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return false;
	auto t = confMethods->GetTranscoder(callCtx->SIPDialogID());
	if (!t) return false;

	auto &&owner = t->GetOwner();
	const char prefix[] = "#sip:";

	if (owner.compare(0, sizeof(prefix) - 1, prefix) == 0
		&& from.compare(0, sizeof(prefix) - 1, prefix) != 0)
	{
		owner = owner.substr(5);
	}

	const size_t len = from.length();
	return t && owner.compare(0, len, from.data()) == 0 && owner.length() > len && owner[len] == '/';
}

bool VS_SIPParser::IsGroupConf(const std::shared_ptr<VS_SIPParserInfo> &callCtx) const {
	assert(callCtx != nullptr);
	if (!callCtx) return false;
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return false;
	auto t = confMethods->GetTranscoder(callCtx->SIPDialogID());
	if (!t) return false;

	return t->IsGroupConf();
}

std::string VS_SIPParser::GetPeer(const std::shared_ptr<VS_SIPParserInfo> &callCtx) const {
	assert(callCtx != nullptr);
	if (!callCtx) return std::string();
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return {};
	auto t = confMethods->GetTranscoder(callCtx->SIPDialogID());
	if (!t) return std::string();

	return t->GetPeer();
}

std::string VS_SIPParser::GetConferenceID(const std::shared_ptr<VS_SIPParserInfo> &callCtx) const {
	assert(callCtx != nullptr);
	if (!callCtx) return std::string();
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return {};
	auto t = confMethods->GetTranscoder(callCtx->SIPDialogID());
	if (!t) return std::string();

	return t->GetConfID();
}

eSIPUserAgent VS_SIPParser::IdentifyUserAgent(const std::shared_ptr<VS_SIPParserInfo> &ctx)
{
	static const char *sfb_user_agents[] = { "RTC","UCCAPI" };

	auto &&user_agent = ctx->GetUserAgent();
	if (user_agent.empty())
		return Unknown;

	const auto it = std::find_if(sfb_user_agents, sfb_user_agents + sizeof(sfb_user_agents) / sizeof(*sfb_user_agents), [&user_agent](string_view uaStr)
	{
		return boost::starts_with(user_agent, uaStr);
	});

	if (it != (sfb_user_agents + sizeof(sfb_user_agents) / sizeof(*sfb_user_agents)))
	{
		return Lync;
	}
	return Unknown;
}

std::string VS_SIPParser::GetFromHost() const {
	char buf[256] = { 0 };
	int32_t len;
	VS_RegistryKey cfg(false, CONFIGURATION_KEY);
	if (cfg.IsValid() && (len = cfg.GetValue(buf, sizeof(buf), VS_REG_STRING_VT, SIP_FROM_HOST)) > 0 && buf[0]) {
		std::string res_str;
		const unsigned int min_len_scope_ipv6 = 3; //::1 or [ipv6]\0 -> ipv6\0
		if (net::is_domain_name(buf) || net::is_ipv4(buf))
		{
			res_str = std::string{ buf, static_cast<std::size_t>(len - 1) };
		}
		else if (static_cast<unsigned int>(len) > min_len_scope_ipv6)
		{
			const char first_ch_ipv6 = '[';
			const char last_ch_ipv6 = ']';
			char *ptr_buf = buf;

			bool is_scope_ipv6 = true;
			if((ptr_buf[0] == first_ch_ipv6 && ptr_buf[len - 2] == last_ch_ipv6)
				&& ((is_scope_ipv6 = ((min_len_scope_ipv6 << 1) <= static_cast<unsigned int>(len))))) //buf = [ipv6]\0 -> ipv6\0
			{
				ptr_buf += 1; //ptr_buf = [ipv6]\0 -> ipv6]\0
				len -= min_len_scope_ipv6;
				*(ptr_buf + len) = '\0'; // ptr_buf = ipv6]\0 -> ipv6\0
			}

			if (is_scope_ipv6 &&  net::is_ipv6(ptr_buf))
			{
				res_str = first_ch_ipv6 + std::string{ ptr_buf } +last_ch_ipv6;
			}
		}
		return res_str;
	}
	return {};
}

std::string VS_SIPParser::SetNewDialogTest(string_view newDialog, string_view sipTo, string_view dtmf, const VS_CallConfig
	&config, string_view myName)
{
	if (sipTo.empty()) return {};

	std::string sip_to(sipTo);

	if (sipTo.find('@') == string_view::npos && !config.HostName.empty())
	{
		sip_to += "@" + config.HostName;
	}

	std::shared_ptr<VS_SIPParserInfo> ctx = GetParserContext(newDialog, true);
	ctx->SIPDialogID(std::string(newDialog));

	dstream3 << "NewDialogID: " << newDialog << ", sip_to: " << sip_to;

	return std::string(newDialog);
}

void VS_SIPParser::AddInfoFromRegConfig(std::shared_ptr<VS_SIPParserInfo> &ctx, const std::shared_ptr<VS_SIPRequest> &req)
{
	assert(ctx != nullptr);
	if (!ctx) return;

	auto &&config = ctx->GetConfig();
	const std::shared_ptr<VS_SIPParserInfo> reg_ctx = GetRegContextOnRemoteServer(ctx, req);

	if (reg_ctx) {
		auto &&c = reg_ctx->GetConfig();

		config.sip.ICEEnabled = c.sip.ICEEnabled;
		config.sip.SRTPEnabled = c.sip.SRTPEnabled;
		config.sip.BFCPEnabled = c.sip.BFCPEnabled;
		config.H224Enabled = c.H224Enabled;
	}
}

void VS_SIPParser::AddAuthInfoFromRegContext(std::shared_ptr<VS_SIPParserInfo> &ctx, const std::shared_ptr<VS_SIPRequest> &req)
{
	assert(ctx != nullptr);
	if (!ctx) return;

	auto &&config = ctx->GetConfig();
	const std::shared_ptr<VS_SIPParserInfo> reg_ctx = GetRegContextOnRemoteServer(ctx, req);

	if (reg_ctx) {
		auto &&c = reg_ctx->GetConfig();
		create_call_config_manager(config).MergeWith(c);

		auto auth = reg_ctx->GetAuthScheme();
		if (auth && auth->scheme() == SIP_AUTHSCHEME_NTLM) {
			auto reg_auth = std::dynamic_pointer_cast<VS_SIPAuthGSS>(auth);
			auto inv_auth = std::make_shared<VS_SIPAuthGSS>(SIP_AUTHSCHEME_NTLM);

			inv_auth->login(reg_auth->login());
			inv_auth->password(reg_auth->password());
			inv_auth->realm(reg_auth->realm());
			inv_auth->qop(SIP_AAA_QOP_AUTH);

			inv_auth->method(auth->method());
			inv_auth->opaque(auth->opaque());
			inv_auth->targetname(auth->targetname());
			inv_auth->version(4);
			inv_auth->auth_type(auth->auth_type());
			inv_auth->crand(reg_auth->crand());

			ctx->secureCtx = reg_ctx->secureCtx;
			ctx->SetAuthScheme(inv_auth);
			ctx->SetEpidMy(reg_ctx->GetEpidMy());
			ctx->SipInstance(reg_ctx->SipInstance());
			ctx->SetContactGruu(reg_ctx->GetContactGruu());
			ctx->SetAliasMy(reg_ctx->GetAliasMy());

			// for TLS we use the same connection for register and usual context i.e. same bind addr and peer addr
			auto &my_cs_addr = static_cast<const VS_SIPParserInfo&>(*reg_ctx).GetMyCsAddress();
			ctx->SetMyCsAddress(my_cs_addr);

			config.Address = c.Address;					// peer addr

			if (!req)
			{
				auto alias = std::string(ctx->GetAliasRemote());
				size_t n = alias.find('@');
				if (n != std::string::npos) {
					alias = alias.substr(0, n);

					auto domain = reg_ctx->GetAliasMy();
					n = domain.find('@');
					if (n != std::string::npos) {
						domain = domain.substr(n + 1);
					}

					alias.append("@").append(domain);
					ctx->SetAliasRemote(alias);
					ctx->SetSIPRemoteTarget(alias);
				}
			} else {
				VS_SIPUpdateInfoImpl update_info(*ctx);
				const VS_SIPGetInfoImpl get_info(*ctx);
				req->GenerateMyInfo(get_info, update_info);
			}

			if (!req || req->GetMethod() != TYPE_REGISTER) {
				ctx->SetContactHost(config.sip.FromDomain);
			}

			ctx->SetUser(reg_auth->login());
			ctx->SetExpires(std::chrono::seconds(0));
		}
	}
}

void VS_SIPParser::UpdateCallConfig(std::shared_ptr<VS_SIPParserInfo> &ctx, const net::Endpoint& remoteEp, bool withMedia)
{
	auto ua = ctx->GetUserAgent();
	if (ua.empty())
		return;
	auto &&old_config = ctx->GetConfig();
	const auto new_config = CreateCallConfig(remoteEp, {}, ua);	// todo: pass ua as string_view to CreateCallConfig()

	create_call_config_manager(old_config).MergeWith(new_config);
	ctx->SetConfig(old_config); // to update codecs

		if (withMedia)
		{
			// update BFCP state
			if (old_config.sip.BFCPEnabled.get_value_or(true))
			{
				ctx->EnableBFCP();
				ctx->SetBFCPSupportedRoles(old_config.sip.BFCPRoles.get_value_or(SDP_FLOORCTRL_ROLE_C_S));
				if (ctx->GetMessageType() == eStartLineType::TYPE_OPTIONS)
				{
					auto& media_streams = ctx->MediaStreams();
					auto bfcp_ms_it = std::find_if(media_streams.begin(), media_streams.end(), [](const VS_SDPField_MediaStream* ms) {
						return ms->GetMediaType() == SDPMediaType::application_bfcp;
					});
					if (bfcp_ms_it == media_streams.end())
						InitMediaStreams(ctx);
					else
					{
						VS_SDPField_MediaStream* const bfcp_ms = *bfcp_ms_it;
						// If the media stream already exists we need to update the protocol because it might be different now.
						switch (old_config.sip.DefaultBFCPProto.get_value_or(net::protocol::TCP))
						{
						case net::protocol::UDP:
							bfcp_ms->SetProto(SDP_PROTO_UDP_BFCP);
							break;
						case net::protocol::TCP:
							bfcp_ms->SetProto(SDP_PROTO_TCP_BFCP);
							break;
						}
					}
					InitBFCP(ctx);
				}
			}
			else
				ctx->DisableBFCP();
			// update H224 state
			ctx->SetH224Enable(old_config.H224Enabled.get_value_or(DEFAULT_FECC_ENABLED));
			if (ctx->IsDialogEstablished())
			{
				SendSetMediaChannels(ctx, ctx->GetSMCContinuation());
			}
		}
	}


std::string VS_SIPParser::GetAdminContact() const{
	std::string admin_contact;
	if (m_get_web_manager_property)
	{
		auto email = m_get_web_manager_property("admin_email");
		auto fio = m_get_web_manager_property("admin_fio");
		if (!fio.empty())
		{
			admin_contact += fio;
			admin_contact += " ";
		}
		if (!email.empty())
		{
			if (fio.empty()) {
				admin_contact += email;
				admin_contact += " ";
			}
			else {
				admin_contact += "(";
				admin_contact += email;
				admin_contact += ") ";
			}
		}
	}

	return admin_contact;
}

std::string VS_SIPParser::MakeAnswerToContactAdmin() const
{
	// text template from enh#42355 comment#8
	std::string answer = "The call was terminated as the maximum number of simultaneous SIP connections with TrueConf Server has been exceeded.\n"
		"Please contact your administrator ";
	answer += GetAdminContact();
	answer += "to upgrade the license.\n";

	return answer;
}


std::string VS_SIPParser::ProcessCallCommand(const std::shared_ptr<VS_SIPParserInfo>& newCtx, string_view from, const std::vector<std::string>& args) {
	std::string call_id_for_message;
	std::string answer;

	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return {};

	try {
		if (args.size() != 2) throw e_ChatBotAnswer::not_recognized;	// call command only for one argument: "/call <arg>"

		auto st = GetStatus(newCtx, *confMethods, args[1]);

		auto user_p = boost::get<UserStatusInfo::User>(&st.info);
		if(user_p)
		{

			call_id_for_message = std::string(VS_RemoveTranscoderID_sv(st.real_id));

			if (user_p->status < USER_AVAIL)
			{
				if (user_p->status == USER_LOGOFF)
					return "User " + call_id_for_message + " is not available\n";

				return "User " + call_id_for_message + " not found\n";
			}

			if (user_p->status == USER_BUSY) throw e_ChatBotAnswer::user_busy;

			if (user_p->status == USER_AVAIL)	answer = "Inviting " + call_id_for_message + "\n";
			else if (user_p->status == USER_MULTIHOST) 	answer = "Asking " + call_id_for_message + " to join group conference\n";

			newCtx->AddPendingInviteUser(st.real_id); // will be called when user answers
			newCtx->SetConfID(st.confStreamID);
		}else
		{
			auto conf_p = boost::get<UserStatusInfo::Conf>(&st.info);
			if(!conf_p || !conf_p->conf_exist)
				return "User " + call_id_for_message + " not found\n";

			answer = "Joining " + args[1] + '\n';
			newCtx->AddPendingInviteUser(args[1]);
			newCtx->SetGroupConf(true);
			newCtx->SetConfID(st.confStreamID);
		}


		if (!confMethods->S4B_InitBeforeCall(newCtx->SIPDialogID(), from, true)) {
			answer += MakeAnswerToContactAdmin();
			CleanParserContext(newCtx->SIPDialogID(), SourceClean::PARSER);
		}
		else {
			InitMediaStreams(newCtx);
			if (!SendSetMediaChannels(newCtx, VS_SIPParserInfo::SMCContinuation::SendINVITE)) {
				dstream4 << "VS_SIPParser::ProcessCallCommand: SendSetMediaChannels failed!\n";
				answer = MakeAnswerToContactAdmin();
				CleanParserContext(newCtx->SIPDialogID(), SourceClean::PARSER);
			}
		}


	}
	catch (e_ChatBotAnswer e_answer) {
		switch (e_answer)
		{
		case e_ChatBotAnswer::not_recognized:
			answer = bot::NOT_RECOGNIZED_ANSWER;
			break;
		case e_ChatBotAnswer::user_busy:
			answer = "User " + call_id_for_message + " is busy\n";
			break;
		default:
			break;
		}
	}

	return answer;
}

std::string VS_SIPParser::ProcessConfCommand(const std::shared_ptr<VS_SIPParserInfo>& newCtx, string_view from, const std::vector<std::string>& args) {
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return {};

	bool has_CID = (std::find_if(args.begin(), args.end(), [](const std::string &arg) { return boost::starts_with(arg, string_view(GROUPCONF_PREFIX)); }) != args.end());
	if (has_CID) return ProcessCallCommand(newCtx, from, args);

	std::string answer;
	newCtx->SetGroupConf(true);

	for (size_t i = 1; i < args.size(); ++i) {
		if (boost::starts_with(args[i], "\"") && boost::ends_with(args[i], "\"")) {						// skype4business 2015 format for topic
			auto topic = boost::trim_copy_if(args[i], [](char c) {return c == '\"'; });
			newCtx->SetConfTopic(topic);
		}
		else if (boost::starts_with(args[i], "&quot;") && boost::ends_with(args[i], "&quot;")) {		// lync2013 format for topic
			auto topic = boost::erase_first_copy(args[i], "&quot;");
			boost::erase_last(topic, "&quot;");
			newCtx->SetConfTopic(topic);
		}
		else {
			auto st = GetStatus(newCtx, *confMethods, args[i]);
			std::string call_id_for_message(VS_RemoveTranscoderID_sv(st.real_id));

			auto user_p = boost::get<UserStatusInfo::User>(&st.info);

			bool not_found = true;
			if(user_p)
			{
				not_found = false;
				if (user_p->status == USER_AVAIL)
				{
					newCtx->AddPendingInviteUser(st.real_id); // will be called when user answers
					answer += "Inviting " + std::string(call_id_for_message) + "\n";
				}
				else if (user_p->status == USER_LOGOFF)
					answer += "User " + std::string(call_id_for_message) + " is not available\n";
				else if (user_p->status == USER_BUSY)
					answer += "User " + std::string(call_id_for_message) + " is busy\n";
				else
					not_found = true;
			}

			if(not_found)
				answer += "User " + call_id_for_message + " not found\n";
		}
	}

	if (newCtx->GetPendingInvitesSize() == 0) newCtx->AddPendingInviteUser(EMPTY_CONFERENCE_TAG);

	if (!confMethods->S4B_InitBeforeCall(newCtx->SIPDialogID(), from, true)) {
		answer += MakeAnswerToContactAdmin();
		CleanParserContext(newCtx->SIPDialogID(), SourceClean::PARSER);
	}
	else {
		InitMediaStreams(newCtx);
		if (!SendSetMediaChannels(newCtx, VS_SIPParserInfo::SMCContinuation::SendINVITE)) {
			dstream4 << "VS_SIPParser::ProcessConfCommand: SendSetMediaChannels failed!\n";
			answer = MakeAnswerToContactAdmin();
			CleanParserContext(newCtx->SIPDialogID(), SourceClean::PARSER);
		}
	}

	return answer;
}

std::string VS_SIPParser::ProcessWebinarCommand(const std::shared_ptr<VS_SIPParserInfo>& newCtx, string_view from, const std::vector<std::string>& args) {
	bool has_CID = (std::find_if(args.begin(), args.end(), [](const std::string &arg) { return boost::starts_with(arg, string_view(GROUPCONF_PREFIX)); }) != args.end());
	if (has_CID) return bot::NOT_RECOGNIZED_ANSWER;

	newCtx->SetPublicConf(true);
	return ProcessConfCommand(newCtx, from, args);
}

std::string VS_SIPParser::ProcessPodiumCommand(const std::shared_ptr<VS_SIPParserInfo> &ctx, string_view from, const std::vector<std::string> &args, bool to_podium) {
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return {};

	if (args.size() != 2 || boost::starts_with(args[1], string_view(GROUPCONF_PREFIX)))
		return bot::NOT_RECOGNIZED_ANSWER;

	auto call_ctx = FindActiveCallCtx(ctx);

	if (!call_ctx)
		return bot::NOT_RECOGNIZED_ANSWER;

	if (!call_ctx->IsPublicConf())
		return bot::ALLOWED_ONLY_IN_WEBINAR_ANSWER;

	if (confMethods->GetMyConferenceRole(call_ctx->SIPDialogID()) != PR_LEADER)
		return bot::YOU_MUST_BE_MODERATOR_ANSWER;

	auto st = GetStatus(ctx, *confMethods, args[1]);
	std::string call_id_for_message(VS_RemoveTranscoderID_sv(st.real_id));

	auto user_p = boost::get<UserStatusInfo::User>(&st.info);
	if (user_p)
	{
		if (user_p->status == USER_LOGOFF)
			return "User " + call_id_for_message + " is not available\n";
		if (user_p->status < USER_LOGOFF)
			return "User " + call_id_for_message + " not found\n";
	}
	else
		return bot::NOT_RECOGNIZED_ANSWER;

	if (to_podium) {
		confMethods->InviteToTribune(call_ctx->SIPDialogID(), st.real_id);
		return "Inviting " + call_id_for_message + " to podium\n";
	}
	else {
		confMethods->ExpelFromTribune(call_ctx->SIPDialogID(), st.real_id);
		return "Expelling " + call_id_for_message + " from podium\n";
	}
}

void VS_SIPParser::ProcessChangeMyTribuneStateCommand(const std::shared_ptr<VS_SIPParserInfo> &ctx, bool toPodium, std::string &outAnswer) {
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return;

	auto call_ctx = FindActiveCallCtx(ctx);
	if (!call_ctx) { outAnswer = bot::NOT_RECOGNIZED_ANSWER; return; }
	if (!call_ctx->IsPublicConf()) { outAnswer = bot::ALLOWED_ONLY_IN_WEBINAR_ANSWER; return; }

	auto my_current_role = confMethods->GetMyTribuneRole(call_ctx->SIPDialogID());
	if (toPodium) {
		if (my_current_role == PR_PODIUM || my_current_role == PR_REPORTER) outAnswer = "You are broadcasting already\n";
		else confMethods->TakeTribune(call_ctx->SIPDialogID());
	}
	else {
		if (my_current_role == PR_COMMON || my_current_role == PR_EQUAL) outAnswer = "You are not broadcasting now\n";
		else confMethods->LeaveTribune(call_ctx->SIPDialogID());
	}
}

std::string VS_SIPParser::ProcessCommandWithConferenceParticipant(const std::shared_ptr<VS_SIPParserInfo> &ctx, string_view from, const std::vector<std::string> &args) {
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return {};

	if (args.empty()) return bot::NOT_RECOGNIZED_ANSWER;

	const auto& cmd = args[0];
	if (args.size() == 1) {
		if (cmd == "/invite") {
			return "Username required.\nTo invite user into group conference or webinar\n/invite username1[, username2[, ...]]\n";
		}
		if (cmd == "/remove") {
			return "Username required.\nTo remove user from group conference or webinar\n/remove username\n";
		}

		return bot::NOT_RECOGNIZED_ANSWER;
	}

	auto call_ctx = FindActiveCallCtx(ctx);
	if (!call_ctx || !call_ctx->IsGroupConf()) return bot::NOT_RECOGNIZED_ANSWER;

	if (confMethods->GetMyConferenceRole(call_ctx->SIPDialogID()) != PR_LEADER)	return bot::YOU_MUST_BE_MODERATOR_ANSWER;

	std::string answer;
	for (std::size_t i = 1; i < args.size(); ++i) {
		auto st = GetStatus(ctx, *confMethods, args[i]);
		auto user_p = boost::get<UserStatusInfo::User>(&st.info);

		string_view call_id_for_message = string_view{ st.real_id }.substr(0, st.real_id.find('/'));

		if (user_p)
		{
			if (user_p->status == USER_LOGOFF)
			{
				answer.append("User ").append(call_id_for_message.data(), call_id_for_message.length()).append(" is not available\n"); continue;
			}
		}
		else
		{
			answer.append("User ").append(call_id_for_message.data(), call_id_for_message.length()).append(" not found\n"); continue;
		}

		if (cmd == "/invite") {

			if (user_p->status != USER_AVAIL) {
				answer.append("User ").append(call_id_for_message.data(), call_id_for_message.length()).append(" not found\n"); continue;
			}

			confMethods->AsyncInvite(call_ctx->SIPDialogID(), {from, true}, st.real_id, VS_ConferenceInfo(true, false),
				boost::bind(&VS_SIPParser::AsyncInviteResult,
					std::static_pointer_cast<VS_SIPParser>(shared_from_this()), _1, _2, ctx, _3), ctx->GetDisplayNameSip(), false);

			answer.append("Inviting ").append(call_id_for_message.data(), call_id_for_message.length()).push_back('\n');
		}
		else if (cmd == "/remove")
		{
			if (user_p->status < USER_AVAIL) { answer.append("User ").append(call_id_for_message.data(), call_id_for_message.length()).append(" not found\n"); continue; }

			confMethods->KickFromConference(call_ctx->SIPDialogID(), from, st.real_id);
			answer.append("Kicking ").append(call_id_for_message.data(), call_id_for_message.length()).push_back('\n');
		}

	}

	return answer;
}

std::string VS_SIPParser::ProcessReqInviteAnswer(const std::shared_ptr<VS_SIPParserInfo>& ctx, string_view from, const std::vector<std::string>& args)
{
	auto confMethods = m_confMethods.lock();
	if (!confMethods)
		return {};
	std::string from_id;
	if (args.empty() || args.size() > 2 || !ctx->GetPendingReqInvite(*args.rbegin(), from_id))
		return bot::USER_NOT_RECOGNIZED_ANSWER;

	auto call_ctx = FindActiveCallCtx(ctx);

	if (!call_ctx || from_id.empty())
		return bot::USER_NOT_RECOGNIZED_ANSWER;

	const auto& cmd = args[0];
	confMethods->ReqInviteReply(call_ctx->SIPDialogID(), from_id, cmd == "/y");
	std::string bot_answer = cmd == "/y" ? "Accepting " : "Rejecting ";
	bot_answer += from_id;

	return bot_answer;
}


void VS_SIPParser::SendChatBotAnswer(std::shared_ptr<VS_SIPParserInfo>& ctx, string_view from, bool hasAuthenticatedMsgCtx, const std::string &answer, bool makeRTFFromText, eContentType desiredContentType) {

	if (!hasAuthenticatedMsgCtx) {
		auto new_dialog = NewDialogID(from, {}, ctx->GetConfig());
		auto new_ctx = GetParserContext(new_dialog);
		if (!new_ctx) return;

		new_ctx->SetEpidSip(ctx->GetEpidSip());
		this->Chat(new_dialog, ctx->GetAliasMy(), ctx->GetAliasRemote(), ctx->GetDisplayNameMy(), answer.c_str());
	}
	else {
		auto req = std::make_shared<VS_SIPRequest>();

		const VS_SIPGetInfoImpl get_info(*ctx);
		VS_SIPUpdateInfoImpl update_info(*ctx);

		if (makeRTFFromText && desiredContentType == CONTENTTYPE_TEXT_RTF)
			req->MakeMESSAGE(get_info, update_info, MakeSimpleRTF(answer), desiredContentType);
		else
			req->MakeMESSAGE(get_info, update_info, answer, desiredContentType);
		PutMessageForOutput(req);
	}
}



bool VS_SIPParser::ProcessChatCommand(std::shared_ptr<VS_SIPParserInfo> &ctx, string_view from, const std::string &msg) {

	auto corrected_msg = boost::regex_replace(msg, MAIL_FORM, "");	// "us@server <mailto:us@server>" ->  "us@server"
	boost::sregex_iterator it(corrected_msg.begin(), corrected_msg.end(), CHAT_BOT_CMD_E);
	decltype(it) end;
	if (it == end) return false;

	std::vector<std::string> args;
	while (it != end) {
		args.emplace_back(*it->begin());
		++it;
	}
	std::sort(args.begin() + 1, args.end());
	args.erase(std::unique(args.begin(), args.end()), args.end());

	std::string answer;
	bool command_recognized(true);
	bool has_authenticated_msg_ctx(false);
	eContentType message_ct = CONTENTTYPE_TEXT_PLAIN;
	if (ctx->RTFSupported()) {
		message_ct = CONTENTTYPE_TEXT_RTF;	// to avoid smiles, send text as rtf to skype

		auto& media_streams = ctx->MediaStreams();
		has_authenticated_msg_ctx = !std::none_of(media_streams.begin(), media_streams.end(), [](VS_SDPField_MediaStream* ms) {
			return ms->GetMediaType() == SDPMediaType::message;
		});
	}

	const std::string &cmd = args[0];

	auto create_call_context = [this](const std::shared_ptr<VS_SIPParserInfo> &chat_ctx, string_view from) -> std::shared_ptr<VS_SIPParserInfo>
	{
		auto new_dialog_id = NewDialogID(from, {}, chat_ctx->GetConfig());
		auto new_ctx = GetParserContext(new_dialog_id, true);
		auto &new_config = new_ctx->GetConfig();

		new_ctx->SetAliasMy(chat_ctx->GetAliasMy());
		if (!new_config.Login.empty()) {
			std::string alias, user, domain;

			user = new_config.Login;
			domain = new_config.HostName;

			alias = user + "@" + domain;

			new_ctx->SetAliasMy(std::move(alias));
			new_ctx->SetUser(std::move(user));
			new_ctx->SetDomain(std::move(domain));
		}

		AddAuthInfoFromRegContext(new_ctx, {});
		VS_CallConfigCorrector::GetInstance().CorrectCallConfig(new_config, VS_CallConfig::SIP, chat_ctx->GetUserAgent().c_str());

		if (new_ctx->SRTPEnabled() && new_ctx->HaveAuthenticatedTLSConnection())
		{
			new_ctx->SetSRTPKey(m_fireGetSRTPKey(new_dialog_id,
				new_ctx->GetMyCsAddress(), new_config.Address));
		}

		if (new_config.sip.BFCPEnabled.get_value_or(false)) {
			new_ctx->EnableBFCP();
		} else {
			new_ctx->DisableBFCP();
		}

		new_ctx->SetH224Enable(new_config.H224Enabled.get_value_or(false));
		new_ctx->CreatedByChatBot(true);

		return new_ctx;
	};

	if (cmd == "/invite") {
		answer = ProcessCommandWithConferenceParticipant(ctx, from, args);
	} else if (cmd == "/podium") {
		answer = ProcessPodiumCommand(ctx, from, args, true);
	} else if (cmd == "/free") {
		answer = ProcessPodiumCommand(ctx, from, args, false);
	} else if (cmd == "/remove") {
		answer = ProcessCommandWithConferenceParticipant(ctx, from, args);
	} else if (cmd == "/y" || cmd == "/n") {
		answer = ProcessReqInviteAnswer(ctx, from, args);
	} else if (cmd == "/call") {
		answer = ProcessCallCommand(create_call_context(ctx, from), from, args);
	} else if (cmd == "/conf") {
		answer = ProcessConfCommand(create_call_context(ctx, from), from, args);
	} else if (cmd == "/webinar") {
		answer = ProcessWebinarCommand(create_call_context(ctx, from), from, args);
	} else if (cmd == "/take") {
		if (args.size() != 1) answer = bot::NOT_RECOGNIZED_ANSWER;
		else ProcessChangeMyTribuneStateCommand(ctx, true, answer);
	} else if (cmd == "/leave") {
		if (args.size() != 1) answer = bot::NOT_RECOGNIZED_ANSWER;
		else ProcessChangeMyTribuneStateCommand(ctx, false, answer);
	}
	else{
		if (cmd != "/help") {
			SendChatBotAnswer(ctx, from, has_authenticated_msg_ctx, std::string(bot::NOT_RECOGNIZED_ANSWER), message_ct == CONTENTTYPE_TEXT_RTF, message_ct);
		}
		command_recognized = false;
		if (message_ct == CONTENTTYPE_TEXT_RTF) {
			if (CHAT_CTRLS_HELP_RTF.empty()) CHAT_CTRLS_HELP_RTF = MakeChatBotHelpMSG(CONTENTTYPE_TEXT_RTF);
			answer = CHAT_CTRLS_HELP_RTF;
		}
		else {
			if (CHAT_CTRLS_HELP_PLAIN_TEXT.empty()) CHAT_CTRLS_HELP_PLAIN_TEXT = MakeChatBotHelpMSG(CONTENTTYPE_TEXT_PLAIN);
			answer = CHAT_CTRLS_HELP_PLAIN_TEXT;
		}
	}

	SendChatBotAnswer(ctx, from, has_authenticated_msg_ctx, answer, command_recognized && message_ct == CONTENTTYPE_TEXT_RTF, message_ct);
	return true;
}

bool HasMsgMediaStream(const std::shared_ptr<VS_SIPParserInfo>& c) {
	assert(c != nullptr);
	const auto pMs = c->MediaStream(0);
	if (!pMs)
		return false;
	return pMs->GetMediaType() == SDPMediaType::message;
}

bool AliasesEqual(const std::shared_ptr<VS_SIPParserInfo>& c1, const std::shared_ptr<VS_SIPParserInfo>& c2) {
	assert(c1 != nullptr);
	assert(c2 != nullptr);
	return c1->GetAliasRemote() == c2->GetAliasRemote() && c1->GetAliasMy() == c2->GetAliasMy();
}

template<class Func>
std::shared_ptr<VS_SIPParserInfo> FindActiveCtx(const vs::map<std::string, std::shared_ptr<VS_SIPParserInfo>, vs::str_less>& ctxs, Func && ctxTester) {
	for (const auto& it : ctxs){
		const auto &ctx = it.second;
		assert(ctx != nullptr);
		if (ctxTester(ctx))
			return ctx;
	}
	return nullptr;
}

std::shared_ptr<VS_SIPParserInfo> VS_SIPParser::FindActiveCallCtx(const std::shared_ptr<VS_SIPParserInfo>& ctx) {
	assert(ctx != nullptr);
	if (ctx->GetAliasRemote().empty() || ctx->GetAliasMy().empty())
		return nullptr;

	std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };
	return FindActiveCtx(m_ctx, [ctx](const std::shared_ptr<VS_SIPParserInfo>& candidateCtx) -> bool{
		assert(candidateCtx != nullptr);
		if (!AliasesEqual(ctx, candidateCtx))
			return false;
		if (candidateCtx->MediaStream(0) == nullptr)
			return false;
		if (HasMsgMediaStream(candidateCtx))
			return false;
		return true;
	});
}

std::shared_ptr<VS_SIPParserInfo> VS_SIPParser::FindActiveMsgCtx(const std::shared_ptr<VS_SIPParserInfo>& ctx) {
	assert(ctx != nullptr);
	if (ctx->GetAliasRemote().empty() || ctx->GetAliasMy().empty())
		return nullptr;

	std::lock_guard<decltype(m_ctx_lock)> lock{ m_ctx_lock };
	return FindActiveCtx(m_ctx, [ctx](const std::shared_ptr<VS_SIPParserInfo>& candidateCtx) -> bool {
		assert(candidateCtx != nullptr);
		if (!AliasesEqual(ctx, candidateCtx))
			return false;
		if (!HasMsgMediaStream(candidateCtx))
			return false;
		return true;
	});
}

std::string VS_SIPParser::RtfToTxt(const std::string &s) {
	std::string ret;
	int state_cnt = 0;

	for (unsigned i = 0; i < s.length(); i++) {
		switch (s[i]) {
		case '{':
			state_cnt++;
			break;
		case '}':
			state_cnt--;
			if (state_cnt < 0) return "";
			break;
		case '\\':
			if (i + 1 < s.length() && s[i + 1] == '\\') {
				i++;
				ret += '\\';
			}
			else if (i + 4 < s.length() && s[i + 1] == 'p' && s[i + 2] == 'a' && s[i + 3] == 'r' && s[i + 4] == ' ') {
				i += 4;
				ret += '\n';
			}
			else {
				for (; i < s.length() && s[i] != ' '; i++);
			}
			break;
		case '\r':
		case '\n':
			break;
		default:
			ret += s[i];
			break;
		}
	}

	return ret;
}

std::string VS_SIPParser::MakeChatBotHelpMSG(eContentType ct) const
{
	if (!m_get_app_property) return std::string();

	auto server_url = m_get_app_property("site_url");
	if (server_url.empty()) return std::string();

	if (ct == CONTENTTYPE_TEXT_RTF) {
		size_t buff_size = bot::CHAT_CTRLS_HELP_RTF_TEMPLATE.length() + (server_url.length() + 1) * 2;
		auto help_msg = vs::make_unique<char[]>(buff_size);
		snprintf(help_msg.get(), buff_size, bot::CHAT_CTRLS_HELP_RTF_TEMPLATE.c_str(), server_url.c_str(), server_url.c_str());
		return std::string(help_msg.get());
	}

	if (ct == CONTENTTYPE_TEXT_PLAIN) {
		size_t buff_size = bot::CHAT_CTRLS_HELP_PLAIN_TEXT_TEMPLATE.length() + server_url.length() + 1;
		auto help_msg = vs::make_unique<char[]>(buff_size);
		snprintf(help_msg.get(), buff_size, bot::CHAT_CTRLS_HELP_PLAIN_TEXT_TEMPLATE.c_str(), server_url.c_str());
		return std::string(help_msg.get());
	}

	return std::string();
}


bool VS_SIPParser::NeedToRetryCall(const std::shared_ptr<VS_SIPResponse>& rsp) {
	std::chrono::steady_clock::duration retryAfterInterval;
	auto pContextWithReinvite = GetParserContext(rsp->CallID());
	if (pContextWithReinvite && (retryAfterInterval = rsp->GetRetryAfterInterval()) > std::chrono::steady_clock::duration()
		&& retryAfterInterval <= std::chrono::seconds(10)) // do not hold transcoder to long periods
	{
		pContextWithReinvite->NeedRetryAfter(true);
		pContextWithReinvite->RetryAfterTime(clock().now() + retryAfterInterval);
		return true;
	}
	return false;
}

bool VS_SIPParser::RetryCall(string_view dialogId, std::chrono::seconds interval)
{
	if (dialogId.empty() || interval == std::chrono::seconds(0))
	{
		return false;
	}

	auto ctx = GetParserContext(dialogId);
	if (ctx == nullptr)
	{
		return false;
	}

	ctx->NeedRetryAfter(true);
	ctx->RetryAfterTime(clock().now() + interval);
	return true;
}

bool VS_SIPParser::SetToIPAddress(std::shared_ptr<VS_SIPParserInfo> &ctx, const std::string &from, const std::string &dn, const std::string &toServer) const {

	if (!ctx)
		return false;

	auto &&c = ctx->GetConfig();

	const char *to_ = toServer.c_str();

	string_view to_view{ toServer };
	bool prefix = false;
	if (to_[0] == '#') {
		prefix = true;
		to_view = to_view.substr(to_view.find_first_of(":") + 1);	// #prefix:server -> server
	}

	to_view = to_view.substr(0, to_view.find_first_of(";"));	// server;param=value -> server

	int port(5060);
	auto port_pos = to_view.find_last_of(':');
	if (port_pos != string_view::npos) port = strtol(static_cast<std::string>(to_view.substr(port_pos + 1)).c_str(), nullptr, 10);
	to_view = to_view.substr(0, port_pos);	// server:port -> server

	std::string to_server;
	size_t host_pos;
	if ((host_pos = to_view.find_last_of('@')) != string_view::npos)	to_server = static_cast<std::string>(to_view.substr(host_pos + 1));	// user@server -> server
	else to_server = static_cast<std::string>(to_view);


	boost::system::error_code ec;
	net::address addr = net::address::from_string(to_server, ec);
	if (ec && m_CallConfigStorage)
	{

		std::string to_id;
		if (host_pos == string_view::npos) {	// hasn't server part separator
			to_id = '@'; to_id += toServer;
			to_ = to_id.c_str();
		}

		if (!prefix) {
			to_id = toServer;
			VS_AddSIPPrefix(to_id);
			to_ = to_id.c_str();
		}

		VS_UserData from_user(from.c_str(), dn.c_str());
		VS_CallConfig config;
		if (!m_CallConfigStorage->Resolve(config, to_/*to_sip_id*/, &from_user)) return false;

		config.Address.port = port;
		dstream4 << "Resolved address for redirection '" << toServer << "' => " << config.Address;
		if (
			// Do not compare Connection Types (variable "type").
			config.Address.addr != c.Address.addr ||
			config.Address.port != c.Address.port
			)
		{
			if (ctx->IsNTLMContext())
			{ // for now for NTLM just change address to save all NTLM registration info
				c.Address = config.Address;
				return true;
			}
			ctx->SetConfig(std::move(config));
			return true;	// address was changed
		}
	}
	else
	{
		if (addr != c.Address.addr || port != c.Address.port)
		{
			c.Address.addr = std::move(addr);
			c.Address.port = port;
			return true; // address was changed
		}
	}

	return false;
}

void VS_SIPParser::UseACL(bool use)
{
	m_use_acl = use;
}


bool VS_SIPParser::IsACLUsed(void) const
{
	return m_use_acl && m_acl.GetMode() != VS_NetworkConnectionACL::ACL_NONE;
}

#undef NULL_TICK
#undef DEBUG_CURRENT_MODULE