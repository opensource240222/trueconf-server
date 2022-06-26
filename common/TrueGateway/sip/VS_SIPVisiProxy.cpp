#include "VS_SIPVisiProxy.h"

#include <boost/algorithm/string/predicate.hpp>

#include "VS_SIPCallResolver.h"
#include "TrueGateway/sip/SIPTransportLayer.h"
#include "VS_TranscoderKeeper.h"
#include "TrueGateway/TransportTools.h"
#include "TrueGateway/VS_GatewayParticipantInfo.h"

#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std-generic/cpplib/ignore.h"

#include "TrueGateway/clientcontrols/VS_ClientControlInterface.h"
#include "FakeClient/VS_ConferenceInfo.h"

extern std::string g_tr_endpoint_name;

namespace
{
	inline VS_ConferenceProtocolInterface::ConferenceStatus user_st_to_conference_st(VS_UserPresence_Status st, bool alowedJoinGroupConf) noexcept
	{
		switch (st)
		{
		case USER_AVAIL : return VS_ConferenceProtocolInterface::ConferenceStatus::AVAILABLE;
		case USER_LOGOFF: return  VS_ConferenceProtocolInterface::ConferenceStatus::TMP_UNAVAILABLE;
		case USER_BUSY: return VS_ConferenceProtocolInterface::ConferenceStatus::BUSY_HERE;
		case USER_INVALID: VS_FALLTHROUGH;
		case USER_STATUS_UNDEF: return VS_ConferenceProtocolInterface::ConferenceStatus::UNDEFINED;
		default:
			if(st >= USER_PUBLIC)
			{
				return alowedJoinGroupConf ? VS_ConferenceProtocolInterface::ConferenceStatus::AVAILABLE : VS_ConferenceProtocolInterface::ConferenceStatus::BUSY_HERE;
			}
			break;
		}
		return VS_ConferenceProtocolInterface::ConferenceStatus::NOT_FOUND;
	}

	inline void send_mail(const std::shared_ptr<VS_TransportRouterServiceHelper> &service, string_view fromName, string_view dnFromUTF8, string_view toName) noexcept
	{
		if (!service)
			return;

		VS_Container cnt;
		cnt.AddValue(METHOD_PARAM, SENDMAIL_METHOD);
		cnt.AddValue(CALLID_PARAM, fromName); //from
		cnt.AddValue(DISPLAYNAME_PARAM, dnFromUTF8);
		cnt.AddValue(CALLID2_PARAM, toName); //to
		cnt.AddValue(USERNAME_PARAM, toName);
		cnt.AddValue(TIME_PARAM, std::chrono::system_clock::now());

		service->PostRequest(service->OurEndpoint(), nullptr, cnt, LOG_SRV, LOCATE_SRV);
	}
}

VS_ConferenceProtocolInterface::ConferenceStatus to_conference_status(VS_CallInviteStatus loginStatus)
{
	switch (loginStatus)
	{
	case VS_CallInviteStatus::SUCCESS: return VS_ConferenceProtocolInterface::ConferenceStatus::AVAILABLE;
	case VS_CallInviteStatus::TIME_OUT: return VS_ConferenceProtocolInterface::ConferenceStatus::SRV_UNAVAILABLE;
	default: return VS_ConferenceProtocolInterface::ConferenceStatus::UNDEFINED;
	}
}

VS_SIPVisiProxy::VS_SIPVisiProxy(boost::asio::io_service::strand &strand, InitInfo &&init)
	: m_strand(strand)
	, m_transceiversPool(std::move(init.transcPool))
	, m_trKeeper(std::move(init.trKeeper))
	, m_sipTransport(std::move(init.sipTransport))
	, m_sipCallResolver(std::move(init.sipCallResolver))
	, m_userStatus(std::move(init.getUserStatus))
{
	assert(m_transceiversPool.lock() != nullptr);
	m_trKeeper->SetConnectClientToTransceiver([this](const std::string &dialogID, const std::string& confID) {return ConnectClientToTransceiver(dialogID, confID); });
}

VS_ConferenceProtocolInterface::ConferenceStatus VS_SIPVisiProxy::Invite(string_view dialogId, const gw::Participant &from,
	string_view toId, const VS_ConferenceInfo& info, string_view dnFromUTF8, bool newSession, bool forceCreate)
{
	if (!toId.empty())
	{
		auto status = m_userStatus(toId, true, true);

		auto user_p = boost::get<UserStatusInfo::User>(&status.info);
		if (user_p)
		{
			if (user_p->status < USER_AVAIL || user_p->status >= USER_BUSY)
			{
				if (user_p->status == USER_LOGOFF)
					send_mail(m_sipCallResolver.lock(), VS_GetNameByCallID(from.callID), dnFromUTF8, VS_GetNameByCallID(toId));

				return user_st_to_conference_st(user_p->status, from.allowedToJoinGroupConf);
			}
		}
		else
		{
			auto conf_p = boost::get<UserStatusInfo::Conf>(&status.info);
			if (!conf_p)
				return ConferenceStatus::UNDEFINED;

			if (!conf_p->conf_exist)
				return ConferenceStatus::NOT_FOUND;
		}
	}

	auto sip_transport = m_sipTransport.lock();
	if (!sip_transport)
		return ConferenceStatus::UNDEFINED;

	string_view from_str = from.callID;
	size_t n1 = from_str.find(':');
	size_t n2 = from_str.find('@');
	if (n1 != string_view::npos && n2 != string_view::npos) {
		from_str = from_str.substr(n1 + 1, n2 - (n1 + 1));
	}

	auto t = m_trKeeper->GetLoggedIn(from_str);
	if (t && t->trans) {
		t->trans->SetDialogId(dialogId);
		m_trKeeper->AddTranscoder(dialogId, t);
	}
	else {
		t = m_trKeeper->NewTranscoder(dialogId);
	}

	auto cs_info = sip_transport->GetCSInfo(dialogId);
	const bool ip_v4 = cs_info.bindEp.addr.is_v4();

	if (!t || !t->trans)
		return ConferenceStatus::UNDEFINED;

	if (!dnFromUTF8.empty())
		t->trans->UpdateDisplayName(dnFromUTF8, false);

	return to_conference_status(t->trans->InviteMethod(from.callID, toId, info, ip_v4, newSession, forceCreate));
}

void VS_SIPVisiProxy::LoginUser(string_view dialogId, string_view login, string_view password, std::chrono::steady_clock::time_point expireTime, string_view externalName,
								std::function< void(bool)> resultCallback, std::function<void(void)> logoutCb, const std::vector<std::string>& aliases)
{
	auto &&on_logout = [this, self_weak = weak_from_this(), callback = std::move(logoutCb)]() mutable noexcept
	{
		const auto self = self_weak.lock();
		if (self)
		{
			m_strand.dispatch([callback = std::move(callback)]
			{
				callback();
			});
		}
	};

	boost::shared_ptr<VS_ClientControlInterface> result;
	if (const auto &&tr = m_trKeeper->GetLoggedIn(login))
	{
		result = tr->trans;
	}

	if (expireTime <= m_clock.now())
	{
		if (result) {
			result->LogoutUser(on_logout);
		} else
		{
			resultCallback(true);
		}
		return;
	}

	if (result)
	{
		resultCallback(true); // it us regester timeout update
		return;
	}

	auto sip_transport = m_sipTransport.lock();
	if (!sip_transport) {
		resultCallback(false);
		return;
	}

	const auto t = m_trKeeper->NewTranscoder(dialogId);
	if (t && t->trans)
	{
		result = t->trans;

		auto &&cs_info = sip_transport->GetCSInfo(dialogId);
		const bool ip_v4 = cs_info.bindEp.addr.is_v4();

		result->LoginUser(login, password, expireTime, externalName, [self_weak = weak_from_this(), this, callback = std::move(resultCallback)](bool res) mutable
		{
			const auto self = self_weak.lock();
			if (self)
			{
				m_strand.dispatch([callback = std::move(callback), res]
				{
					callback(res);
				});
			}
		}, std::move(on_logout), ip_v4, aliases);
	}
	else
	{
		resultCallback(false);
	}
}



bool VS_SIPVisiProxy::InviteReplay(string_view dialogId, VS_CallConfirmCode confirmCode, bool isGroupConf, string_view confName, string_view to_displayName) {
	auto t = m_trKeeper->GetTranscoder(dialogId);
	return (!t || !t->trans) ? false : t->trans->InviteReply(confirmCode);
}



void VS_SIPVisiProxy::Hangup(string_view dialogId)
{
	auto t = m_trKeeper->GetTranscoder(dialogId);
	if (t && t->trans) {
		t->trans->Hangup();
	}

	auto sip_transport = m_sipTransport.lock();
	if (sip_transport) {
		sip_transport->Write(dialogId);
	}
}

bool VS_SIPVisiProxy::S4B_InitBeforeCall(string_view dialogId, string_view fromId, bool createSession)
{
	string_view from_str = fromId;
	size_t n1 = from_str.find(':');
	size_t n2 = from_str.find('@');
	if (n1 != std::string::npos && n2 != std::string::npos) {
		from_str = from_str.substr(n1 + 1, n2 - (n1 + 1));
	}

	auto t = m_trKeeper->GetLoggedIn(from_str);
	if (t && t->trans) {
		t->trans->SetDialogId(dialogId);
		m_trKeeper->AddTranscoder(dialogId, t);
	}
	else {
		t = m_trKeeper->NewTranscoder(dialogId);
		if (!t)
			return false;
	}

	auto sip_transport = m_sipTransport.lock();
	if (!sip_transport) return false;

	auto cs_info = sip_transport->GetCSInfo(dialogId);
	const bool ip_v4 = cs_info.bindEp.addr.is_v4();

	return t->trans->PrepareForCall(fromId, fromId, vs::ignore<std::string>(), ip_v4, VS_CallConfig::eUserStatusScheme::SKYPE4BUSINESS, createSession) == VS_CallInviteStatus::SUCCESS;
}

bool VS_SIPVisiProxy::KickFromConference(string_view dialogId, string_view fromId, string_view toId)
{
	auto t = m_trKeeper->GetTranscoder(dialogId);
	return (!t || !t->trans) ? false : t->trans->KickFromConference(fromId, toId);
}

bool VS_SIPVisiProxy::ReqInviteReply(string_view dialogId, string_view fromId, bool accept)
{
	auto t = m_trKeeper->GetTranscoder(dialogId);
	return (!t || !t->trans) ? false : t->trans->ReqInviteReply(fromId, accept);
}

bool VS_SIPVisiProxy::SetMediaChannels(string_view dialogId, const std::vector<VS_MediaChannelInfo>& channels, const std::string& existingConfID, std::int32_t bandwRcv) {
	auto &&t = m_trKeeper->GetTranscoder(dialogId);
	if (!t || !t->trans)
		return false;

	std::string proxyReservationToken;
	if (!t->trans->RtpControlIsReady()) {
		auto confID = t->trans->GetStreamConfID();
		if (!gw::InitWithRtpControl(m_transceiversPool, t->trans, confID.empty()? existingConfID : confID, proxyReservationToken)) return false;
	}

	if(!proxyReservationToken.empty()) t->trans->SetProxyReservation(proxyReservationToken);
	return t->trans->SetMediaChannels(channels);
}


void VS_SIPVisiProxy::UpdateDisplayName(string_view dialogId, string_view displayName, bool updateImmediately) {
	auto t = m_trKeeper->GetTranscoder(dialogId);
	if (t && t->trans) {
		t->trans->UpdateDisplayName(displayName, updateImmediately);
	}
}

void VS_SIPVisiProxy::AsyncInvite(string_view dialogId, const gw::Participant &from, string_view toId, const VS_ConferenceInfo &info,
	std::function<void(bool /*redirect*/, ConferenceStatus /*status*/, const std::string &/*ip*/ /*TODO: fixed to string_view*/)> inviteResult, string_view dnFromUTF8, bool newSession, bool forceCreate)
{
	if (toId.empty())
		return;

	// empty conference creation
	if (toId == EMPTY_CONFERENCE_TAG && info.is_group_conf)
	{
		Invite(dialogId, from, {}, info, dnFromUTF8, newSession);
		return;
	}

	// don't perform NetworkResolve if ip specified
	{
		string_view servername{};

		if (toId.compare(DEFAULT_DESTINATION_CALLID) != 0 &&
			toId.compare(DEFAULT_DESTINATION_CALLID_SIP) != 0 &&
			toId.compare(DEFAULT_DESTINATION_CALLID_H323) != 0 &&
			toId.compare("groupconf") != 0 &&
			toId.compare(0, strlen(GROUPCONF_PREFIX), GROUPCONF_PREFIX) != 0)
		{
			// Process call to some user (like #sip:user@server)
			const auto pos = toId.find('@');
			if (pos != string_view::npos)
				servername = toId.substr(pos + 1);
		}
		else
		{
			// Process default destination call (like #sip:@server)
			inviteResult(false, Invite(dialogId, from, toId, info, dnFromUTF8, newSession), {});
			return;
		}

		if (servername.empty())
		{
			inviteResult(false, ConferenceStatus::UNDEFINED, {});
			return;
		}

		char buffer[512];
		assert(sizeof(buffer) > servername.length());
		auto len = servername.copy(buffer, sizeof(buffer) - 1);
		buffer[len] = '\0';
		char* buff = buffer;

		// the shortest servername is "[::]"
		if (len >= 4)
		{
			if (buff[0] == '[')
			{
				buff++;							// remove '['
				--len;
			}

			if (buff[len - 1] == ']')
			{
				--len;
				buff[len] = '\0';				// remove ']'
			}


			boost::system::error_code ec;
			net::address::from_string(buff, ec);
			if (!ec) //no error
			{
				inviteResult(false, Invite(dialogId, from, toId, info, dnFromUTF8, newSession, forceCreate), {});
				return;
			}
		}
	}

	auto sip_transport = m_sipTransport.lock();
	if (!sip_transport) {
		inviteResult(false, ConferenceStatus::UNDEFINED, {});
		return;
	}

	auto &&cs_info = sip_transport->GetCSInfo(dialogId);
	if (!cs_info)
	{
		inviteResult(false, ConferenceStatus::UNDEFINED, {});
		return;
	}

	m_strand.get_io_service().post([this, self_weak = weak_from_this(),
		to_id = std::string(toId),
		dialog_id = std::string(dialogId),
		from,
		dn_from_utf8 = std::string(dnFromUTF8),
		invite_result = std::move(inviteResult),
		newSession, info
	]() mutable noexcept
	{
		const auto self = self_weak.lock();
		if (!self)
			return;

		auto &&handle_async_invite_result = [&](bool redirect, ConferenceStatus stat, string_view ip) noexcept
		{
			self->AsyncInviteResult(dialog_id, from.callID, to_id, info, redirect, stat, ip, std::move(invite_result), dn_from_utf8, newSession);
		};

		auto status = m_userStatus(to_id, false, true);

		string_view server;
		ConferenceStatus conf_st = ConferenceStatus::UNDEFINED;
		bool use_sip_redirect = false;

		const auto user_p = boost::get<UserStatusInfo::User>(&status.info);

		if (user_p)
		{
			conf_st = user_st_to_conference_st(user_p->status, from.allowedToJoinGroupConf);
			server = user_p->server;

			unsigned long val = 0;
			VS_RegistryKey key(false, CONFIGURATION_KEY);
			if (key.IsValid() && key.GetValue(&val, sizeof(val), VS_REG_INTEGER_VT, "Use SIP Redirect") > 0 && val != 0)
				use_sip_redirect = true;

		}
		else
		{
			const auto conf_p = boost::get<UserStatusInfo::Conf>(&status.info);
			if (conf_p && conf_p->conf_exist)
				conf_st = ConferenceStatus::AVAILABLE;
		}

		assert(!user_p ? conf_st == ConferenceStatus::AVAILABLE || conf_st == ConferenceStatus::UNDEFINED : true);

		if (conf_st != ConferenceStatus::AVAILABLE)
		{
			handle_async_invite_result(false, conf_st, {});
			return;
		}

		if (!use_sip_redirect || server == g_tr_endpoint_name)
			handle_async_invite_result(false, ConferenceStatus::AVAILABLE, {});
		else
		{
			auto cut_from = server.find_last_of('#');
			if (string_view::npos != cut_from)
				server = server.substr(0, cut_from);
			handle_async_invite_result(true, ConferenceStatus::UNDEFINED, server);
		}

	});
}


void VS_SIPVisiProxy::AsyncInviteResult(string_view dialogId, string_view fromId, string_view toId, const VS_ConferenceInfo &info,
	bool redirect, ConferenceStatus st, string_view ip, std::function<void(bool /*redirect*/, ConferenceStatus /*status*/, const std::string &/*ip*/ /*TODO: fixes string_view*/)> inviteResult, string_view dnFromUTF8, bool newSession)
{
	m_strand.dispatch([this, self_weak = weak_from_this(),
		dialog_id = std::string(dialogId),
		from_id = std::string(fromId),
		to_id = std::string(toId),
		info, redirect, st,
		ip = std::string(ip),
		invite_result = std::move(inviteResult),
		dn_from_utf8 = std::string(dnFromUTF8),
		newSession]() noexcept
	{

		const auto weak = self_weak.lock();
		if (!weak)
			return;
		auto stat = st;
		if (stat == ConferenceStatus::AVAILABLE)
		{
			auto sip_transport = m_sipTransport.lock();
			if (!sip_transport) return;

			auto t = m_trKeeper->GetTranscoder(dialog_id);
			if (!t || !t->trans) // If there is no any transcoder found, create a new one (This resembles behaviour of the VS_TransportConnection::AsyncInviteResult()).
			{
				t = m_trKeeper->NewTranscoder(dialog_id);
			}
			auto &&cs_info = sip_transport->GetCSInfo(dialog_id);

			const bool ip_v4 = cs_info.bindEp.addr.is_v4();

			auto &&status = m_userStatus(to_id, true, false);

			stat = ConferenceStatus::UNDEFINED;

			if (t && t->trans)
			{
				if (!dn_from_utf8.empty())
					t->trans->UpdateDisplayName(dn_from_utf8, false);

				auto user_p = boost::get<UserStatusInfo::User>(&status.info);
				if (user_p)
				{
					if (user_p->status != USER_MULTIHOST)
						stat = to_conference_status(t->trans->InviteMethod(from_id, to_id, info, ip_v4, newSession));
					else
						stat = t->trans->ReqInviteMethod(from_id, to_id, ip_v4, newSession) ? ConferenceStatus::AVAILABLE : ConferenceStatus::UNDEFINED;
				}
				else
				{
					auto conf_p = boost::get<UserStatusInfo::Conf>(&status.info);
					if (conf_p)
						stat = to_conference_status(t->trans->InviteMethod(from_id, to_id, info, ip_v4, newSession));
				}
			}
		}

		if (st == ConferenceStatus::TMP_UNAVAILABLE)
			send_mail(m_sipCallResolver.lock(), VS_GetNameByCallID(from_id), dn_from_utf8, VS_GetNameByCallID(to_id));

		invite_result(redirect, stat, ip);
	});
}

void VS_SIPVisiProxy::FastUpdatePicture(string_view dialogId)
{
	auto t = m_trKeeper->GetTranscoder(dialogId);
	if (t && t->trans)
	{
		t->trans->FastUpdatePictureFromSIP();
	}
}

boost::shared_ptr<VS_ClientControlInterface> VS_SIPVisiProxy::GetTranscoder(string_view dialogId)
{
	const auto t = m_trKeeper->GetTranscoder(dialogId);
	return t ? t->trans : nullptr;
}

boost::shared_ptr<VS_ClientControlInterface> VS_SIPVisiProxy::GetLoggedTranscoder(string_view fromId)
{
	if (fromId.empty())
		return {};

	const auto n1 = fromId.find(':');
	const auto n2 = fromId.find('@');

	if (n1 != string_view::npos && n2 != string_view::npos)
	{
		fromId = fromId.substr(n1 + 1, n2 - (n1 + 1));
		if (fromId.empty()) return nullptr;
	}

	const auto t = m_trKeeper->GetLoggedIn(fromId);
	return t ? t->trans : nullptr;
}

void VS_SIPVisiProxy::SetUserEndpointAppInfo(string_view dialogId, string_view appName, string_view version)
{
	auto t = m_trKeeper->NewTranscoder(dialogId);
	if (t && t->trans)
	{
		t->trans->SetUserEndpointAppInfo(appName, version);
	}
}

void VS_SIPVisiProxy::TakeTribune(string_view dialogId)
{
	auto t = m_trKeeper->GetTranscoder(dialogId);
	if (t && t->trans) {
		t->trans->TakeTribune();
	}
}

void VS_SIPVisiProxy::LeaveTribune(string_view dialogId) {
	auto t = m_trKeeper->GetTranscoder(dialogId);
	if (t && t->trans) {
		t->trans->LeaveTribune();
	}
}

VS_Participant_Role VS_SIPVisiProxy::GetMyTribuneRole(string_view dialogId) {
	const auto &&t = m_trKeeper->GetTranscoder(dialogId);
	if (t && t->trans) {
		return t->trans->GetMyTribuneRole();
	}

	return PR_COMMON;
}

VS_Participant_Role VS_SIPVisiProxy::GetMyConferenceRole(string_view dialogId) {
	const auto &&t = m_trKeeper->GetTranscoder(dialogId);
	if (t && t->trans) {
		return t->trans->GetMyConferenceRole();
	}
	return PR_COMMON;
}

void VS_SIPVisiProxy::InviteToTribune(string_view dialogId, const std::string &toId)
{
	auto &&t = m_trKeeper->GetTranscoder( dialogId );
	if (t && t->trans) {
		t->trans->InviteToTribune(toId);
	}
}

void VS_SIPVisiProxy::ExpelFromTribune(string_view dialogId, const std::string &toId)
{
	auto &&t = m_trKeeper->GetTranscoder(dialogId);
	if (t && t->trans) {
		t->trans->ExpelFromTribune(toId);
	}
}

void VS_SIPVisiProxy::Chat(string_view dialogId, const std::string &from, const std::string &to, const std::string &dn, const char *mess)
{
	if (auto sip_transport = m_sipTransport.lock())
		sip_transport->Write(dialogId);	// for 200 OK that was inserted in VS_SIPParser::OnRequest_MESSAGE

	auto t = m_trKeeper->GetTranscoder(dialogId);
	if (t && t->trans) {	// chat in call
		if (!to.empty())
		{// p2p
			t->trans->SendChatMessage(to, mess);
		}
		else
		{	// groupconf
			t->trans->SendChatMessage(t->trans->GetStreamConfID(), mess);
		}
	}
	else {	// chat not in call
		auto call_rslv = m_sipCallResolver.lock();
		if (!call_rslv) return;

		VS_Container cnt;
		cnt.AddValue(METHOD_PARAM, SENDMESSAGE_METHOD);
		cnt.AddValue(DISPLAYNAME_PARAM, dn);
		cnt.AddValue(MESSAGE_PARAM, mess);
		cnt.AddValue(TO_PARAM, to);
		cnt.AddValue(FROM_PARAM, from);
		call_rslv->PostRequest(nullptr, to.c_str(), cnt, nullptr, CHAT_SRV, ~0, SIPCALL_SRV);
	}


}

bool VS_SIPVisiProxy::ConnectClientToTransceiver(string_view dialogId, const std::string &confId)
{
	auto t = m_trKeeper->GetTranscoder(dialogId);
	if (!t || !t->trans || confId.empty())
		return false;

	std::string dummy;
	return gw::InitWithRtpControl(m_transceiversPool, t->trans, confId, dummy);
}

