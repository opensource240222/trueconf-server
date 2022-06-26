#include "std/cpplib/VS_RegistryConst.h"
#include "VS_SIPCallResolver.h"

#include <boost/make_shared.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "VS_TranscoderKeeper.h"

#include "../interfaces/VS_ParserInterface.h"
#include "../CallConfig/VS_CallConfig.h"
#include "../CallConfig/VS_CallConfigStorage.h"
#include "../CallConfig/VS_Indentifier.h"

#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_UserData.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/debuglog/VS_Debug.h"
#include "../../statuslib/VS_CallIDInfo.h"
#include "std/cpplib/event.h"
#include "std-generic/cpplib/deleters.h"

#define DEBUG_CURRENT_MODULE VS_DM_MULTIGATEWAY

VS_SIPCallResolver::VS_SIPCallResolver(boost::asio::io_service::strand &strand, InitInfo &&init)
	: AbstractGatewayEventListener(std::move(init.peerConfig))
	, m_strand(strand)
	, m_tr_keeper(std::move(init.trKeeper))
	, m_parser (std::move(init.parser))
	, m_timer(strand.get_io_service())
{
}

bool VS_SIPCallResolver::Resolve(std::string& call_id, VS_CallIDInfo& ci, VS_UserData* fromUser) {
	if (!IsMyCallId(call_id.c_str()))
		return false;

	bool is_s4b = false;
	VS_CallConfig config;
	if (m_peer_config->Resolve(config, call_id, fromUser) && config.sip.UserStatusScheme == VS_CallConfig::eUserStatusScheme::SKYPE4BUSINESS)
		is_s4b = true;

	VS_UserPresence_Status status = USER_AVAIL;
	std::string transcoder_id;
	if (is_s4b) {
		status = GetSIPUserStatus(call_id);
		if (status == USER_MULTIHOST) {
		// find transcoder for conference
		auto &&tr = m_tr_keeper->GetLoggedIn(call_id, false);
		if (!tr) return false;
		transcoder_id = tr->trans->GetTrueconfID();
		}
		else {
			auto tr = m_tr_keeper->GetLoggedIn(call_id, false);
			if (tr) {
				transcoder_id = tr->trans->GetTrueconfID();
			}
			else {
			// prepare new transcoder
			assert(status == USER_AVAIL);
			transcoder_id = PrepareTranscoder(call_id, fromUser);
		}
	}
	} else {	// bug#51495: normal sip (not skype4business) should always allocate new transcoder, so PresenceStatus will be USER_AVAIL always, s4b needs status for ReqInvite
		transcoder_id = PrepareTranscoder(call_id, fromUser);
	}

	if (transcoder_id.empty()) {
		ci.m_status = USER_INVALID;
		return false;
	}
	const auto our_endpoint = OurEndpoint();
	ci.m_status = status;
	ci.m_serverID = !!our_endpoint ? our_endpoint : std::string{};
	ci.m_realID = std::move(transcoder_id);
	//m_logginTime
	ci.m_homeServer = !!our_endpoint ? our_endpoint : std::string{};
	dstream3 << "VS_SIPCallResolver::Resolve call_id = " << call_id << "; status = " << ci.m_status << "; real_id = " << ci.m_realID;
	return true;
}

bool VS_SIPCallResolver::IsRegisteredTransId(const char *transId){
	return m_tr_keeper->IsRegisteredTransID(transId);
}

void VS_SIPCallResolver::ScheduleTimer(const std::chrono::milliseconds period)
{
	m_timer.expires_from_now(period);
	m_timer.async_wait([this, w_this=weak_from_this(), period](const boost::system::error_code& ec) {
		if (ec == boost::asio::error::operation_aborted)
			return;

		auto self = w_this.lock();
		if (!self)
			return;
		Timeout();
		ScheduleTimer(period);
	});
}

void VS_SIPCallResolver::Subscribe(const char *callId) {
	dprint3("VS_SIPCallResolver::Subscribe call_id = %s\n", callId);
	if (!IsMyCallId(callId))
		return ;

	auto status = USER_AVAIL;

	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);	//VS_AutoLock lock(this);
		m_subscribes.insert(callId);
		status = m_statuses.emplace(callId, status).first->second;
	}

	VS_Container status_cnt;
	status_cnt.AddValue(METHOD_PARAM, PUSHSTATUSDIRECTLY_METHOD);
	status_cnt.AddValue(CALLID_PARAM, callId);
	status_cnt.AddValueI32(USERPRESSTATUS_PARAM, status);
	status_cnt.AddValue(SERVER_PARAM, OurEndpoint());
	status_cnt.AddValueI32(MULTI_LOGIN_CAPABILITY_PARAM, VS_MultiLoginCapability::SINGLE_USER);
	void* body = nullptr;
	size_t body_size;
	const auto res = status_cnt.SerializeAlloc(body, body_size);
	assert(res);
	std::unique_ptr<void, free_deleter> body_free{ body };
	std::unique_ptr<VS_RouterMessage> msg(new VS_RouterMessage(OurService(), nullptr, PRESENCE_SRV, nullptr, nullptr, OurEndpoint(),
		OurEndpoint(), default_timeout, body, body_size));
	if (PostMes(msg.get()))
	{
		msg.release();
	}
}

void VS_SIPCallResolver::Unsubscribe(const char *callId) {
	dprint3("VS_SIPProtocolRouter::Unsubscribe call_id = %s\n", callId);
	if (!IsMyCallId(callId))
		return;

	std::lock_guard<decltype(m_mutex)> lock{ m_mutex };

	string_view call_id_view{ callId };
	auto it_subs = m_subscribes.find(call_id_view);
	if(it_subs != m_subscribes.cend())
		m_subscribes.erase(it_subs);

	auto it = m_statuses.find(call_id_view);
	if (it != m_statuses.cend() && it->second == USER_AVAIL)
	{
		m_statuses.erase(it);
	}
}

bool VS_SIPCallResolver::IsMyCallId(const char *callId) const {
	return m_peer_config->GetCommonIdentifier()->IsMyCallId(callId);
}

bool VS_SIPCallResolver::CanICall(VS_UserData* fromUde, const char* toCallId, bool IsVCS) {
	if (!fromUde || !toCallId || !*toCallId) {
		return false;
	}
	if (IsVCS) {
		return true;
	}
	if (!(fromUde->m_rights&VS_UserData::UR_COMM_DIALER)) {
		return false;
	}
	return true;
}

void VS_SIPCallResolver::NewPeerCfg(string_view callId, const std::vector<VS_ExternalAccount> &v)
{
	dstream3 << "SIPCallResolver::NewPeerCfg for " << callId << " num=" << v.size();
	if (callId.empty()) return;

	std::lock_guard<decltype(m_mutex)> lock(m_mutex);

	auto peer_config = m_peer_config;
	peer_config->RemoveUser(callId);

	if (!v.empty())
		peer_config->AddUser(callId, v);

	auto &&lock_configs = peer_config->GetRegistrationSettings().lock();
	for (auto &reg_config : *lock_configs)
	{
		if (reg_config.TcId == callId)
		{
			auto &&call_manager = create_call_config_manager(reg_config);
			if (call_manager.NeedVerification())
				call_manager.ResetVerificationResults();

			SetRegistrationConfiguration(std::move(reg_config));
		}
	}
}

std::string VS_SIPCallResolver::GetNameFromCallID(string_view callId) const
{
	auto pos = callId.find(":@");
	if (pos != string_view::npos)
	{
		return std::string(callId);
	}

	pos = callId.find('/');

	if (pos != string_view::npos)
	{
		callId = callId.substr(0, pos);
	}

	std::string default_domain;
	if (boost::starts_with(callId, "#sip") &&
		DefaultCallManager::GetSIPParam("fromdomain", default_domain, "#default") &&
		boost::ends_with(callId, default_domain))
	{
		callId = callId.substr(0, callId.length() - default_domain.length() - 1);
	}

	return std::string(callId);
}

void VS_SIPCallResolver::UpdateStatus(string_view callId, VS_UserPresence_Status status) {
	auto tr = m_tr_keeper->GetLoggedIn(callId, false);
	if (tr && tr->trans->GetUserStatusScheme() != VS_CallConfig::eUserStatusScheme::SKYPE4BUSINESS)	// bug#51495: normal sip should always be USER_AVAIL; save status only for s4b
		return;

	std::string id = GetNameFromCallID(callId);

	bool should_push_status = false;

	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex); //VS_AutoLock _(this);

		auto it = m_statuses.find(id);

		if (it != m_statuses.cend())
		{
			if (status == USER_AVAIL && m_subscribes.find(id) == m_subscribes.cend()) {
				m_statuses.erase(id);
			}
			else
			{
				should_push_status = it->second != status;
				it->second = status;
			}
		}
		else
		{
			auto &st = m_statuses[id];
			st = status;
		}
	}

	if (should_push_status)
	{
		VS_Container status_cnt;
		status_cnt.AddValue(METHOD_PARAM, PUSHSTATUSDIRECTLY_METHOD);
		status_cnt.AddValue(CALLID_PARAM, id);
		status_cnt.AddValueI32(USERPRESSTATUS_PARAM, status);
		status_cnt.AddValue(SERVER_PARAM, OurEndpoint());
		status_cnt.AddValueI32(MULTI_LOGIN_CAPABILITY_PARAM, VS_MultiLoginCapability::SINGLE_USER);
		void* body = nullptr;
		size_t body_size;
		const auto res = status_cnt.SerializeAlloc(body, body_size);
		assert(res);
		std::unique_ptr<void, free_deleter> body_free{ body };
		std::unique_ptr<VS_RouterMessage> msg{ new VS_RouterMessage(OurService(), NULL, PRESENCE_SRV, 0, 0, OurEndpoint(), OurEndpoint(), 	default_timeout, body, body_size) };
		if(PostMes(msg.get()))
			msg.release();
	}
}

void VS_SIPCallResolver::ReqInvite(string_view dialogId, string_view name)
{
	m_strand.dispatch([dialog_id = std::string(dialogId), name = std::string(name), parser = m_parser]()
	{
		parser->ReqInvite(dialog_id, name);
	});
}

void VS_SIPCallResolver::TakeTribuneReply(string_view dialogId, bool result)
{
	m_strand.dispatch([dialog_id = std::string(dialogId), result, parser = this->m_parser]()
	{
		parser->TakeTribuneReply(dialog_id, result);
	});
}

void VS_SIPCallResolver::LeaveTribuneReply(string_view dialogId, bool result) {

	m_strand.dispatch([dialog_id = std::string(dialogId), result, parser = m_parser]()
	{
		parser->LeaveTribuneReply(dialog_id, result);
	});
}

void VS_SIPCallResolver::HandleEvent(const VS_GatewayService::ReloadConfiguration& reloadCfg)
{
	m_strand.dispatch([reloadCfg, self = shared_from_this()]()
	{
		self->AbstractGatewayEventListener::HandleEvent(reloadCfg);
	});
}

std::string VS_SIPCallResolver::PrepareTranscoder(string_view toSipId, VS_UserData *fromUser)
{
	VS_UserData fake;
	fake.m_name = "Administrator";
	if (!fromUser || !fromUser->m_name)
		fromUser = &fake;

	VS_CallConfig config;
	if (!m_peer_config->Resolve(config, toSipId, fromUser))
	{
		return {};
	}

	if (config.resolveResult.NewCallId.empty())
	{
		config.resolveResult.NewCallId = std::string(toSipId);
	}

	return PrepareTranscoder(toSipId, fromUser, config);
}

std::string VS_SIPCallResolver::PrepareTranscoder(string_view toSipId, VS_UserData *fromUser, const VS_CallConfig &config)
{
	std::string dialog_id = NewDialogIDFromParser(config.resolveResult.NewCallId, config.resolveResult.dtmf, config, string_view{ fromUser->m_name.m_str, (size_t)fromUser->m_name.Length() });

	if (dialog_id.empty())
		return {};

	boost::shared_ptr<VS_TranscoderKeeper::Transcoder_Descr> tr;
	boost::shared_ptr<VS_ClientControlInterface> t;

	if (!((tr = m_tr_keeper->NewTranscoder(dialog_id))) || !((t = tr->trans))) {
		m_tr_keeper->FreeTranscoder(dialog_id);
		return {};
	}
	std::string login;
	if (t->PrepareForCall(config.resolveResult.NewCallId, toSipId, login, config.Address.addr.is_v4()) == VS_CallInviteStatus::SUCCESS)
	{
		return login;
	}
	m_tr_keeper->FreeTranscoder(dialog_id);
	return {};
}

VS_UserPresence_Status VS_SIPCallResolver::GetSIPUserStatus(string_view sipId) {
	std::string id = GetNameFromCallID(sipId);
	if (id.empty()) return USER_AVAIL;

	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	auto it = m_statuses.find(id);
	if (it != m_statuses.cend())
	{
		return it->second;
	}
	return USER_AVAIL;
}

bool VS_SIPCallResolver::Processing(std::unique_ptr<VS_RouterMessage> &&recvMess) {
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	if (recvMess == nullptr)
		return true;
	VS_Container cnt;
	VS_SCOPE_EXIT{ m_recvMess = nullptr; };
	switch (recvMess->Type()) {
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		m_recvMess = recvMess.get();
		if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
			const char* method;
			if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != nullptr) {
				dprint4("VS_SIPCallResolver::Processing(%s)\n", method);

				if (strcasecmp(method, SENDMESSAGE_METHOD) == 0) {
					SendMessage_Method(cnt);
				} else if (strcasecmp(method, SENDCOMMAND_METHOD) == 0) {
					SendCommand_Method(cnt);
				}
			}
		}
		break;
	case transport::MessageType::Notify:
		break;
	}
	return true;
}

void VS_SIPCallResolver::SendMessage_Method(VS_Container &cnt){
	auto chat_message = cnt.GetStrValueRef(MESSAGE_PARAM);
	auto from = cnt.GetStrValueRef(FROM_PARAM);
	auto disp_name = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
	auto to_addr = cnt.GetStrValueRef(TO_PARAM);

	assert(chat_message != nullptr);
	assert(from != nullptr);
	assert(to_addr != nullptr);

	if (!chat_message || !from || !to_addr)
	{
		dstream4 << "Error\tCan't send chat message! No enough parameters!\n";
		return;
	}

	m_strand.get_io_service().post([self_weak = weak_from_this(), this,
		peer_cfg = m_peer_config,
		from_callid = std::string(from),
		to_callid = std::string(to_addr),
		disp_name = std::string(disp_name ? disp_name : std::string()),
		message = std::string(chat_message)]() mutable noexcept
	{
		assert(peer_cfg != nullptr);

		auto &&from_user = boost::make_shared<VS_UserData>(from_callid.c_str(), disp_name.c_str());
		VS_CallConfig config;
		const bool res = peer_cfg->Resolve(config, to_callid, from_user.get());
		assert(res);

		const auto self = self_weak.lock();
		if (self)
		{
			// when resolve is done using usual work thread
			m_strand.dispatch([parser = m_parser,
				from = std::move(from_callid),
				to = std::move(to_callid),
				disp_name = std::move(disp_name),
				config = std::move(config),
				message = std::move(message)]() noexcept
			{
				assert(parser);
				const auto &&dialog_id = parser->NewDialogID(to, {}, config);
				parser->Chat(dialog_id, from, to, disp_name, message.c_str());
			});
		}
	});
}

void VS_SIPCallResolver::SendCommand_Method(VS_Container &cnt) {
	auto cmd = cnt.GetStrValueRef(MESSAGE_PARAM);
	auto from = cnt.GetStrValueRef(FROM_PARAM);

	assert(cmd != nullptr);
	assert(from != nullptr);

	if (!cmd || !from)
	{
		dstream4 << "Error\tCan't send command! No enough parameters!\n";
		return;
	}

	m_strand.dispatch([parser = m_parser,
		from = std::string(from),
		cmd = std::string(cmd)]() noexcept
	{
		assert(parser);
		parser->Command({}, from, cmd);
	});
}

bool  VS_SIPCallResolver::Init(const char *ourEndpoint, const char *ourService, bool permittedAll) {
	/**
	GatewayStarted_Method
	*/
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, EXTERNALPRESENCESTARTED_METHOD);
	size_t body_size;
	void *body;
	const auto res = cnt.SerializeAlloc(body, body_size);
	assert(res);
	std::unique_ptr<void, free_deleter> body_free{ body };
	std::unique_ptr<VS_RouterMessage> msg = vs::make_unique<VS_RouterMessage>(OurService(), nullptr, PRESENCE_SRV, nullptr, nullptr, OurEndpoint(),
		OurEndpoint(), default_timeout, body, body_size);
	if (PostMes(msg.get()))
	{
		msg.release();
	}
	return true;
}

void VS_SIPCallResolver::Timeout()
{
	m_strand.dispatch([self = shared_from_this(), this]()
	{
		ReinitializeConfiguration();
	});
}

inline std::string VS_SIPCallResolver::NewDialogIDFromParser(string_view sipTo, string_view dtmf, const VS_CallConfig &config, string_view myName) {

	vs::event ev{ false };
	std::string res;
	m_strand.dispatch([&]()
	{
		res = m_parser->NewDialogID(sipTo, dtmf, config, myName);
		ev.set();
	});
	ev.wait();
	return res;
}

void VS_SIPCallResolver::SetRegistrationConfiguration(VS_CallConfig config)
{
	m_strand.dispatch([this, parser = m_parser, self_weak = weak_from_this(), config = std::move(config)]() mutable noexcept
	{
		parser->SetRegistrationConfiguration(std::move(config), [this, &self_weak](VS_CallConfig &cfg) noexcept
		{
			auto self = self_weak.lock();
			if (self)
			{
				m_strand.post([self = std::move(self), cfg = std::move(cfg), this]() mutable noexcept
				{
					SetRegistrationConfiguration(std::move(cfg));
				});
			}
		});
	});
}

void VS_SIPCallResolver::UpdateStatusRegistration(const net::address& /*address*/, net::port /*port*/,
	std::function<void(const std::shared_ptr<VS_ParserInterface>&)> &&exec)
{
	m_strand.dispatch([parser = m_parser, exec = std::move(exec)]
	{
		exec(parser);
	});
}

inline const char* VS_SIPCallResolver::GetPeerName()
{
	return SIP_PEERS_KEY;
}

void VS_SIPCallResolver::ResetAllConfigsStatus()
{
	m_strand.dispatch([parser = m_parser]()
	{
		assert(parser);
		parser->ResetAllConfigsStatus();
	});
}