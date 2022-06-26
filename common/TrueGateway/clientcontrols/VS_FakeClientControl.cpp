#include "VS_FakeClientControl.h"
#include "VS_TranscoderLogin.h"
#include "tools/Server/CommonTypes.h"
#include "tools/Server/VS_MediaChannelInfo.h"
#include "tools/SingleGatewayLib/FakeVideo.h"
#include "tools/Server/VS_ServerComponentsInterface.h"
#include "FakeClient/VS_ConferenceInfo.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_VideoLevelCaps.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/VS_Protocol.h"
#include "std-generic/cpplib/string_view.h"
#include "std/cpplib/VS_RcvFunc.h"
#include "std-generic/cpplib/ignore.h"
#include "std/cpplib/md5.h"
#include "TrueGateway/TransportTools.h"
#include "TrueGateway/clientcontrols/VS_TranscoderLogin.h"
#include "TrueGateway/interfaces/VS_ConferenceProtocolInterface.h"
#include "TransceiverLib/TransceiversPoolInterface.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>

#include <algorithm>

struct dev
{
	std::string type;
	std::string id;
	std::string name;
};

const dev cam{ "camera", "1", "Camera" };
const dev content{ "camera", "2", "Content" };
const dev mic{ "microphone", "3", "Microphone" };


struct DeviceInfo
{
	std::vector < std::pair<std::string, std::string>> list;
	std::string type;
	
	VS_FORWARDING_CTOR2(DeviceInfo, list, type) {}
};

VS_FakeClientControl::VS_FakeClientControl(const std::weak_ptr<ts::IPool> &pool, const std::weak_ptr<VS_TranscoderLogin>& transLogin)
    : m_waitForLoginComplete(false)
	, m_transcPool(pool)
	, m_transLogin(transLogin)
	, m_slides_initialized(false)
	, m_camera_enabled(true)
{
	VS_RegistryKey cfg_key(false, CONFIGURATION_KEY);
	VS_RegistryKey tr_key(false, TRANSCODERS_KEY);

	cfg_key.GetString(m_groupconf_username, "Gateway GroupConf Name");
	if (m_groupconf_username.empty())
		tr_key.GetString(m_groupconf_username, "Gateway GroupConf Name");
	if (m_groupconf_username.empty())
		m_groupconf_username = "groupconf";

	cfg_key.GetString(m_mconf_prefix, "Gateway GroupConf Prefix");
	if (m_mconf_prefix.empty())
		tr_key.GetString(m_mconf_prefix, "Gateway GroupConf Prefix");
	
	m_DevicesList.store(std::make_shared<std::vector<DeviceInfo>>());
}

VS_FakeClientControl::~VS_FakeClientControl()
{
}

void VS_FakeClientControl::SetFakeClientInterface(const std::shared_ptr<VS_FakeClientInterface>& fakeClient)
{
	m_fakeClient = fakeClient;
	m_fakeClient->SetDefaultCaps(GetMyClientCaps());

	m_fakeClient->m_fireJoinStreamConference.connect(VS_FakeClientInterface::JoinStreamConferenceSignalType
		::slot_type(&VS_FakeClientControl::JoinStreamConference, this, _1).track(shared_from_this()));

	m_fakeClient->m_fireLoginResponse.connect(VS_FakeClientInterface::LoginResponseSignalType
		::slot_type(&VS_FakeClientControl::onLoginResponse, this, _1).track(shared_from_this()));

	m_fakeClient->m_fireLogoutResponse.connect(VS_FakeClientInterface::LogoutResponseSignalType
		::slot_type(&VS_FakeClientControl::onLogoutResponse, this).track(shared_from_this()));

	m_fakeClient->m_fireConferenceStateChange.connect(VS_FakeClientInterface::ConferenceStateChangeSignalType
		::slot_type(&VS_FakeClientControl::onConferenceStateChange, this, _1, _2).track(shared_from_this()));

	m_fakeClient->m_fireCommand.connect(VS_FakeClientInterface::CommandSignalType
		::slot_type(&VS_FakeClientControl::onCommand, this, _1, _2).track(shared_from_this()));

	m_fakeClient->m_fireChat.connect(VS_FakeClientInterface::ChatSignalType
		::slot_type(&VS_FakeClientControl::onChat, this, _1, _2, _3, _4).track(shared_from_this()));

	m_fakeClient->m_fireFile.connect(VS_FakeClientInterface::FileSignalType
		::slot_type(&VS_FakeClientControl::onFile, this, _1, _2, _3, _4, _5, _6, _7, _8).track(shared_from_this()));

	m_fakeClient->m_firePartList.connect(VS_FakeClientInterface::PartListSignalType
		::slot_type(&VS_FakeClientControl::onPartList, this, _1).track(shared_from_this()));

	m_fakeClient->m_fireRoleEvent.connect(VS_FakeClientInterface::RoleEventSignalType
		::slot_type(&VS_FakeClientControl::onRoleEvent, this, _1, _2, _3, _4, _5, _6).track(shared_from_this()));

	m_fakeClient->m_fireRequestKeyFrame.connect(VS_FakeClientInterface::RequestKeyFrameSignalType
		::slot_type([this]() {ReqFastUpdatePictureFromSIP();}).track(shared_from_this()));

	m_fakeClient->m_fireFECC.connect(VS_FakeClientInterface::FECCSignalType
		::slot_type(&VS_FakeClientControl::onFECC, this, _1, _2, _3, _4).track(shared_from_this()));

	m_fakeClient->m_fireReqInvite.connect(VS_FakeClientInterface::ReqInviteSignalType
		::slot_type(&VS_FakeClientControl::ReqInvite, this, _1).track(shared_from_this()));

	m_fakeClient->m_fireChangeDevice.connect(VS_FakeClientInterface::ChangeDeviceType
		::slot_type(&VS_FakeClientControl::onChangeDevice, this, _1, _2, _3).track(shared_from_this()));

	m_fakeClient->m_fireSetDeviceMute.connect(VS_FakeClientInterface::SetDeviceMuteType
		::slot_type(&VS_FakeClientControl::onSetDeviceMute, this, _1, _2, _3).track(shared_from_this()));

}

void VS_FakeClientControl::SetRTPModuleInterface( const std::shared_ptr<VS_RTPModuleControlInterface> &rtpModuleConrol )
{
	m_RTPModuleControl = rtpModuleConrol;

	m_RTPModuleControl->ConnectToSetMediaChannels(VS_RTPModuleControlInterface::SetMediaChannelsSignalType::slot_type(
		&VS_FakeClientControl::onSetMediaChannels, this, _1, _2).track(shared_from_this())
	);

	m_RTPModuleControl->ConnectToFullIntraframeRequest(VS_RTPModuleControlInterface::FullIntraframeRequestSignalType::slot_type(
		&VS_FakeClientControl::onFullIntraframeRequest, this, _1).track(shared_from_this())
	);

	m_RTPModuleControl->ConnectToVideoStatus(VS_RTPModuleControlInterface::VideoStatusSignalType::slot_type(
		&VS_FakeClientControl::onVideoStatus, this, _1, _2, _3).track(shared_from_this())
	);

	m_RTPModuleControl->ConnectToDeviceStatus(VS_RTPModuleControlInterface::DeviceStatusSignalType::slot_type(
		&VS_FakeClientControl::onDeviceStatus, this, _1, _2).track(shared_from_this())
	);

	m_RTPModuleControl->ConnectToShowSlide(VS_RTPModuleControlInterface::ShowSlideSignalType::slot_type(
		&VS_FakeClientControl::onShowSlide, this, _1, _2, _3).track(shared_from_this())
	);

	m_RTPModuleControl->ConnectToEndSlideShow(VS_RTPModuleControlInterface::EndSlideshowSignalType::slot_type(
		&VS_FakeClientControl::onEndSlideShow, this, _1).track(shared_from_this())
	);
	m_RTPModuleControl->ConnectToFECC(VS_RTPModuleControlInterface::FECCSignalType::slot_type(
		&VS_FakeClientControl::onFECCToVisi, this, _1, _2, _3).track(shared_from_this())
	);
}

const std::string& VS_FakeClientControl::GetTranscoderID() const
{
	return m_fakeClient->CID();
}

bool VS_FakeClientControl::IsReady() const
{
	return true;
}

void VS_FakeClientControl::SetDialogId(string_view dialogId)
{
	std::lock_guard<decltype(m_lock)> _(m_lock);
	const auto it = std::find(m_dialog_ids.cbegin(), m_dialog_ids.cend(), dialogId);
	if (it != m_dialog_ids.end())
		return;
	m_dialog_ids.emplace_back(dialogId);
}

void VS_FakeClientControl::ClearDialogId(string_view dialogId)
{
	std::lock_guard<decltype(m_lock)> _(m_lock);
	const auto it = std::find(m_dialog_ids.cbegin(), m_dialog_ids.cend(), dialogId);
	if (it != m_dialog_ids.end())
	{
		m_dialog_ids.erase(it);
	}

	if (!m_transceiverReservationToken.empty()) {
		if(auto pool = m_transcPool.lock())
			pool->UnreserveProxy(m_transceiverReservationToken);
	}
}

std::string VS_FakeClientControl::GetDialogID()
{
	std::lock_guard<decltype(m_lock)> _(m_lock);
	if (m_dialog_ids.empty())
	{
		return {};
	}
	return m_dialog_ids.back();
}

std::string VS_FakeClientControl::GetConfID()
{
	std::lock_guard<std::mutex> _(m_lock);
	return m_fakeClient->GetCurrentConference()->conf_id;
}

std::string VS_FakeClientControl::GetStreamConfID() const
{
	std::lock_guard<std::mutex> _(m_lock);
	return m_fakeClient->GetCurrentConference()->stream_conf_id;
}


std::string VS_FakeClientControl::GetTrueconfID()
{
	std::lock_guard<std::mutex> _(m_lock);
	return m_fakeClient->GetTrueconfID();
}

std::string VS_FakeClientControl::GetOwner()
{
	std::lock_guard<std::mutex> _(m_lock);
	auto c = m_fakeClient->GetCurrentConference();
	if (c) {
		return c->owner;
	} else {
		return "";
	}
}

bool VS_FakeClientControl::IsGroupConf() const
{
	std::lock_guard<std::mutex> _(m_lock);
	auto c = m_fakeClient->GetCurrentConference();
	return c && c->IsGroup();
}

std::string VS_FakeClientControl::GetPeer() const
{
	std::lock_guard<std::mutex> _(m_lock);
	auto c = m_fakeClient->GetCurrentConference();
	if (!c) return std::string();

	return c->peer;
}

bool VS_FakeClientControl::SetMediaChannels(const std::vector<VS_MediaChannelInfo>& channels)
{
	if (!m_RTPModuleControl) return false;
	if (!CreateRTPSession())
		return false;
		
	SetDevicesList(channels);
	return m_RTPModuleControl->SetMediaChannels(m_fakeClient->CID(), channels);
}

void VS_FakeClientControl::SetProxyReservation(const std::string& reservationToken)
{
	m_transceiverReservationToken = reservationToken;
}

VS_CallInviteStatus VS_FakeClientControl::PrepareForCall(string_view id, string_view idOrig, std::string& callId, bool isIpv4, VS_CallConfig::eUserStatusScheme userStatusScheme, bool createSession)
{
	m_user_status_scheme = userStatusScheme;
	auto trueconf_id = m_fakeClient->GetTrueconfID();

	// already logged in
	if (!trueconf_id.empty())
	{
		callId = std::move(trueconf_id);
		return VS_CallInviteStatus::SUCCESS;
	}

	auto transcoder_login = m_transLogin.lock();
	if (!transcoder_login)
		return VS_CallInviteStatus::FAILURE;

	auto pass = transcoder_login->GenerateTranscoderPass(id);
	char pass_md5[33] = {0};
	VS_ConvertToMD5(pass, pass_md5);

	char app_id[33] = { 0 };
	VS_GenKeyByMD5(app_id);
	m_fakeClient->SetAppId(app_id);
	m_fakeClient->SetAlias(idOrig);

	std::string cid_lower = m_fakeClient->CID();
	std::transform(cid_lower.begin(), cid_lower.end(), cid_lower.begin(), ::tolower);
	auto login = std::string(id) + "/" + cid_lower;

	// loggin as transcoder
	m_fakeClient->LoginUserAsync(
		login,
		pass,
		pass_md5,
		CT_TRANSCODER,
		{},
		std::vector<std::string>()
	);

	m_lastLogin = std::move(login);

	if (!WaitForLoginComplete())
		return VS_CallInviteStatus::TIME_OUT;

	trueconf_id = m_fakeClient->GetTrueconfID();
	if (trueconf_id.empty())
		return VS_CallInviteStatus::FAILURE;

	callId = std::move(trueconf_id);

	return VS_CallInviteStatus::SUCCESS;
}

VS_CallInviteStatus VS_FakeClientControl::InviteMethod(string_view fromId, string_view toId, const VS_ConferenceInfo& info, bool ipV4, bool newSession,
	bool forceCreate)
{
	if (toId.empty())
	{
		// empty conference creation
		if (!info.is_group_conf) return VS_CallInviteStatus::FAILURE;

		auto cd = m_fakeClient->GetCurrentConference();
		if (cd && !cd->conf_id.empty() && !cd->stream_conf_id.empty())
			return VS_CallInviteStatus::FAILURE;

		return CreateConf(info) ? VS_CallInviteStatus::SUCCESS : VS_CallInviteStatus::FAILURE;
	}

	auto cd = m_fakeClient->GetCurrentConference();

	if (cd &&
		cd->m_state == VS_FakeClientInterface::VS_ConferenceDescriptor::e_conf_active &&
		cd->conf_type == CT_MULTISTREAM)
	{
		return m_fakeClient->InviteAsync(std::string(toId), forceCreate, m_transceiverReservationToken) ? VS_CallInviteStatus::SUCCESS : VS_CallInviteStatus::FAILURE;
	}

	if (cd &&
		cd->m_state == VS_FakeClientInterface::VS_ConferenceDescriptor::e_conf_creating &&
		cd->conf_type == CT_MULTISTREAM)
	{
		m_invite_later.emplace_back(toId);
		return VS_CallInviteStatus::SUCCESS;
	}

	if (info.is_group_conf &&
		(!cd || cd->conf_id.empty() || cd->stream_conf_id.empty()) &&
		!boost::istarts_with(toId, string_view(GROUPCONF_PREFIX)))
	{
		if (cd->conf_id.empty() && cd->stream_conf_id.empty() && !CreateConf(info))
			return VS_CallInviteStatus::FAILURE;

		std::lock_guard<std::mutex> _(m_lock);
		m_invite_later.emplace_back(toId);
		return VS_CallInviteStatus::SUCCESS;
	}

	if (m_fakeClient->GetTrueconfID().empty()) {
		const auto loginStatus = PrepareForCall(fromId, fromId, vs::ignore<std::string>(), ipV4);
		if (loginStatus != VS_CallInviteStatus::SUCCESS)
			return loginStatus;
	}

	std::string to_id(toId);
	if      (boost::iequals(to_id, string_view(DEFAULT_DESTINATION_CALLID_SIP))) {
		to_id = m_fakeClient->GetAppProperty("default_call_destination_sip");
	} else if (boost::iequals(to_id, string_view(DEFAULT_DESTINATION_CALLID_H323)))
		to_id = m_fakeClient->GetAppProperty("default_call_destination_h323");
	else if (boost::iequals(to_id, string_view(DEFAULT_DESTINATION_CALLID)))
		to_id = m_fakeClient->GetAppProperty("default_call_destination");

	const std::string username(to_id.cbegin(), std::find(to_id.cbegin(), to_id.cend(), '@'));
	if (boost::iequals(username, m_groupconf_username))
	{
		std::string confname(m_fakeClient->GetAppProperty("default_mconf_name"));
		if (confname.empty())
			confname = "groupconf";
		//std::string confpassword(m_fakeClient->GetAppProperty("default_mconf_password"));
		return m_fakeClient->JoinAsync(confname, info/*, confpassword*/, m_transceiverReservationToken) ? VS_CallInviteStatus::SUCCESS : VS_CallInviteStatus::FAILURE;
	}

	if (boost::istarts_with(username, string_view(GROUPCONF_PREFIX)))
	{
		const std::string confname(username.begin()+strlen(GROUPCONF_PREFIX), username.end());
		return m_fakeClient->JoinAsync(confname, info, m_transceiverReservationToken) ? VS_CallInviteStatus::SUCCESS : VS_CallInviteStatus::FAILURE;
	}

	if (!m_mconf_prefix.empty() && boost::istarts_with(username, m_mconf_prefix))
	{
		const std::string confname(username.begin() + m_mconf_prefix.length(), username.end());
		return m_fakeClient->JoinAsync(confname, info, m_transceiverReservationToken) ? VS_CallInviteStatus::SUCCESS : VS_CallInviteStatus::FAILURE;
	}

	return m_fakeClient->InviteAsync(to_id, false, m_transceiverReservationToken) ? VS_CallInviteStatus::SUCCESS : VS_CallInviteStatus::FAILURE;
}

bool VS_FakeClientControl::InviteReply( VS_CallConfirmCode confirm_code )
{
	auto d = m_fakeClient->GetCurrentConference();
	if (!d) return false;

	if (confirm_code == e_call_ok) d->Accept();
	else
		if (confirm_code == e_call_busy) d->Reject(PARTISIPANT_IS_BUSY);
	else
		if (confirm_code == e_call_rejected) d->Reject(REJECTED_BY_PARTICIPANT);
	else
		d->Reject(REJECTED_BY_PARTICIPANT);

	return true;
}

void VS_FakeClientControl::Hangup()
{
	auto d = m_fakeClient->GetCurrentConference();
	if (!d) return;

	bool forall = d->owner == d->my_id;
	if (forall || d->partList.empty())
	{
		DestroyRTPSession();
		}
	d->Hangup(forall);
}

void VS_FakeClientControl::SendChatMessage(const std::string & to, const std::string & message)
{
	assert(m_fakeClient != nullptr);

	m_fakeClient->SendChatMessage(to, message);
}

bool VS_FakeClientControl::KickFromConference(string_view fromId, string_view toId)
{
	auto cd = m_fakeClient->GetCurrentConference();

	if (cd &&
		cd->m_state == VS_FakeClientInterface::VS_ConferenceDescriptor::e_conf_active &&
		cd->conf_type == CT_MULTISTREAM)
	{
		return m_fakeClient->KickFromConference(toId);
	}

	return false;
}

bool VS_FakeClientControl::ReqInviteMethod(string_view fromId, string_view toId, bool ipV4, bool newSession)
{
	auto cd = m_fakeClient->GetCurrentConference();

	bool logged_in = !m_fakeClient->GetTrueconfID().empty() || PrepareForCall(fromId, fromId, vs::ignore<std::string>(), ipV4) == VS_CallInviteStatus::SUCCESS;
	if (!logged_in) return false;

	return m_fakeClient->ReqInviteAsync(toId);
}

bool VS_FakeClientControl::ReqInviteReply(string_view fromId, bool accept)
{
	auto cd = m_fakeClient->GetCurrentConference();

	if (cd &&
		cd->m_state == VS_FakeClientInterface::VS_ConferenceDescriptor::e_conf_active &&
		cd->conf_type == CT_MULTISTREAM) {
		return m_fakeClient->AnswerReqInvite(fromId, accept);
	}

	return false;
}

void VS_FakeClientControl::ChangeTribuneRoleTo(const VS_Participant_Role r) {
	assert(m_fakeClient != nullptr);

	auto cd = m_fakeClient->GetCurrentConference();
	if (!cd)
		return;

	const auto trueconf_id = m_fakeClient->GetTrueconfID();
	m_disable_role_notification = false;
	if (m_last_tribune_role != PR_LEADER && cd->owner != trueconf_id) {	// we have PR_LEADER role when we are owners or when moderators
		m_fakeClient->QueryRole(trueconf_id.c_str(), r);
	}
	else {
		// due to misunderstanding in 'enum VS_Participant_Role' we can't change role for owner or moderator (i.e. PR_LEADER)
		// so we just change media filter stream without changing role
		switch (r) {
		case PR_PODIUM:	m_fakeClient->ConnectSender(VS_RcvFunc::FLTR_DEFAULT_MULTIS); break;
		case PR_REMARK:	m_fakeClient->ConnectSender(VS_RcvFunc::FLTR_RCV_AUDIO); break;
		case PR_COMMON:
		default:
			m_fakeClient->ConnectSender(VS_RcvFunc::FLTR_SILENT); break;
		}

		// trace role ourself
		m_last_queried_tribune_role = r;
	}
}

void VS_FakeClientControl::TakeTribune()
{
	ChangeTribuneRoleTo(PR_PODIUM);
}

void VS_FakeClientControl::LeaveTribune()
{
	ChangeTribuneRoleTo(PR_COMMON);
}

VS_Participant_Role VS_FakeClientControl::GetMyTribuneRole() const {
	return m_last_conference_role != PR_LEADER ? m_last_tribune_role : m_last_queried_tribune_role;	// when we are leaders we trace our role only by last queried
}

VS_Participant_Role VS_FakeClientControl::GetMyConferenceRole() const {
	return m_last_conference_role;
}

void VS_FakeClientControl::InviteToTribune(const std::string & to_id)
{
	assert(m_fakeClient != nullptr);
	m_disable_role_notification = true;	// must disable because we are leaders and will receive direct answer for not our role change
	m_fakeClient->QueryRole(to_id.c_str(), PR_REPORTER);
}

void VS_FakeClientControl::ExpelFromTribune(const std::string & to_id)
{
	assert(m_fakeClient != nullptr);
	m_disable_role_notification = true; // must disable because we are leaders and will receive direct answer for not our role change
	m_fakeClient->QueryRole(to_id.c_str(), PR_COMMON);
}

void VS_FakeClientControl::TakeRemark()
{
	ChangeTribuneRoleTo(PR_REMARK);
}

void VS_FakeClientControl::LeaveRemark()
{
	ChangeTribuneRoleTo(PR_COMMON);
}

void VS_FakeClientControl::HangupOutcomingCall()
{
	auto d = m_fakeClient->GetCurrentConference();
	if (!d) return;
	d->Hangup();
}
// sip requests FUP from TC
void VS_FakeClientControl::FastUpdatePictureFromSIP()
{
	m_RTPModuleControl->FullIntraframeRequest(m_fakeClient->CID(), false);
}
// TC requests FUP from SIP
void VS_FakeClientControl::ReqFastUpdatePictureFromSIP()
{
	m_RTPModuleControl->FullIntraframeRequest(m_fakeClient->CID(), true);
}

bool VS_FakeClientControl::IsLoggedIn(string_view login, bool exactMatch)
{
	if (m_lastLogin.empty())
		return false;

	if (exactMatch)
	{
		return m_lastLogin == login;
	}

	return boost::starts_with(m_lastLogin, login);
}

void VS_FakeClientControl::LoginUser(string_view login, string_view password, std::chrono::steady_clock::time_point expireTime, string_view externalName,
	std::function< void(bool) > resultCallback, std::function< void(void)> logoutCb, bool ipV4, const std::vector<std::string>& aliases)
{
	{
		std::lock_guard<decltype(m_lock)> _(m_lock);
		m_loginCallback = std::move(resultCallback);
		m_logoutCallback = std::move(logoutCb);
		m_expireTime = expireTime;
		m_externalName = std::string(externalName);
		m_lastLogin = std::string(login);
	}

	//char app_id[33] = { 0 };
	//VS_GenKeyByMD5(app_id);
	m_fakeClient->SetAppId(nullptr);

	m_fakeClient->LoginUserAsync(login, {}, password, CT_TRANSCODER_CLIENT, {}, aliases);
}

void VS_FakeClientControl::LogoutUser( std::function< void(void) > resultCallback )
{
	{
		std::lock_guard<std::mutex> _(m_lock);
		m_logoutCallback = std::move(resultCallback);
		m_lastLogin.clear();
		m_user_status_scheme = VS_CallConfig::eUserStatusScheme::ONLY_USER_AVAIL;
	};

	m_fakeClient->Logout();
}

void VS_FakeClientControl::ReleaseCallbacks()
{
	m_loginCallback = {};
	m_logoutCallback = {};
}

void VS_FakeClientControl::ConnectUpdateStatus(const UpdateStatusSlot &slot)
{
	if (m_fakeClient) {
		m_fakeClient->m_fireUpdateStatus.connect(slot);
	}
}

std::string VS_FakeClientControl::GetAppProperty(const std::string &prop_name)
{
	if (m_fakeClient) {
		return m_fakeClient->GetAppProperty(prop_name);
	}
	return "";
}

bool VS_FakeClientControl::WaitForLoginComplete()
{
	return m_waitForLoginComplete.wait_for(std::chrono::seconds(5));
}

bool VS_FakeClientControl::CreateRTPSession()
{
	assert(m_RTPModuleControl != nullptr);

	auto c = m_fakeClient->GetClientState();
	return m_RTPModuleControl->CreateSession(m_fakeClient->CID(), c.trueconfId, c.sessionKey);
}

void VS_FakeClientControl::DestroyRTPSession()
{
	if (m_RTPModuleControl)
		m_RTPModuleControl->DestroySession(m_fakeClient->CID());
}

void VS_FakeClientControl::JoinStreamConference(VS_FakeClientInterface::VS_ConferenceDescriptor &d)
{

}

void VS_FakeClientControl::LeaveStreamConference(VS_FakeClientInterface::VS_ConferenceDescriptor &d)
{

}

void VS_FakeClientControl::ReqInvite(const char *name)
{
	assert(name != nullptr);
	if (!name)
		return;
	m_fireReqInvite(GetDialogID(), name);
}

void VS_FakeClientControl::onLoginResponse(VS_UserLoggedin_Result res)
{
	std::function<void(bool)> f;
	{
		std::lock_guard<std::mutex> _(m_lock);
		f = m_loginCallback;
		if (res != USER_LOGGEDIN_OK)
		{
			m_lastLogin.clear();
		}
	}

	if (f) f(res == USER_LOGGEDIN_OK);
	m_waitForLoginComplete.set();
}

void VS_FakeClientControl::onLogoutResponse()
{
	DestroyRTPSession();

	std::function<void()> f;
	{
		std::lock_guard<std::mutex> _(m_lock);
		f = m_logoutCallback;
		m_lastLogin.clear();
	}

	{
		m_fireLoggedOutAsUser(GetDialogID());
	}

	if (f) f();
}

void VS_FakeClientControl::onConferenceStateChange(const char *method, const VS_FakeClientInterface::VS_ConferenceDescriptor &conf)
{
	assert(m_fakeClient != nullptr);
	std::string _method = method;

	if (_method == "Invite")
	{
		// logged in as transcoder
		if ( m_externalName.empty() )
			m_fireInvite(GetDialogID(), conf.owner, m_fakeClient->GetTrueconfID(),
			conf.IsGroup(), conf.IsPublic(), false, conf.owner_displayname
			);
		else
			// logged in as user
			m_fireInvite(GetDialogID(), conf.owner, m_externalName,
			conf.IsGroup(), conf.IsPublic(), true, conf.owner_displayname
			);
	}

	if (_method == "deleteConference")
	{
		m_fireHangupFromVisi(GetDialogID(), _method);
		DestroyRTPSession();

		// if logged in as transcoder
		if (m_externalName.empty())
		{
			m_logoutCallback = {};
			m_fakeClient->Logout();
		}
	}

	if (_method == "joinResponse")
	{
		AddFakeDevices();

		if (!m_transceiverReservationToken.empty()) {
			gw::ConnectReservedProxyToConf(m_transcPool, m_transceiverReservationToken, conf.stream_conf_id);
			m_transceiverReservationToken.clear();
		}

		if (conf.m_last_reject_reason != JOIN_OK)
		{
			if (!conf.is_incomming)
				m_fireInviteReply(GetDialogID(),
				conf.m_last_reject_reason == REJECTED_BY_PARTICIPANT ? e_call_rejected : e_call_busy,
				conf.IsGroup(),	{}, {}
				);
			else
				m_fireHangupFromVisi(GetDialogID(), _method);

			if (m_externalName.empty())
			{
				m_logoutCallback = {};
				m_fakeClient->Logout();
			}
		}
		else
		{
			if (!m_RTPModuleControl)
			{
				m_fireConnectMeToTransceiver(GetDialogID(), conf.stream_conf_id);
				assert(m_RTPModuleControl != nullptr);
				if (!m_RTPModuleControl) return;
			}
			if (!CreateRTPSession())
				return;

			string_view r;
			if (!conf.IsGroup())
				r = conf.conf_id;
			m_RTPModuleControl->SetConference(m_fakeClient->CID(), conf.stream_conf_id, r, conf.owner, conf.conf_subtype, conf.remoteCaps);

			string_view dn = (conf.conf_type == CT_MULTISTREAM || conf.conf_type == CT_INTERCOM) ? conf.topic: conf.peer_displayName;
			if (!conf.is_incomming)
				m_fireInviteReply(GetDialogID(), e_call_ok, conf.IsGroup(), conf.conf_id, dn);

			// if we are creaters of conference we will be on podium by default
			if(conf.owner == conf.my_id) m_last_queried_tribune_role = PR_PODIUM;
			DoInviteAsync();
		}
	}

	if (_method == "reject")
	{
		m_fireHangupFromVisi(GetDialogID(), _method);
		DestroyRTPSession();

		if (m_externalName.empty())
		{
			m_logoutCallback = {};
			m_fakeClient->Logout();
		}
	}

	if (_method == "ConferenceCreated")
	{
		m_transceiverReservationToken.clear();
		DoInviteAsync();
	}
}

static const boost::regex command_param_re("([^\r\n=]+)=([^\r\n=]+)", boost::regex::optimize);

void VS_FakeClientControl::onCommand(const char *from, const char *command)
{
	m_fireCommand(GetDialogID(), from, command);
	string_view command_v(command);
	if      (boost::starts_with(command_v, string_view(SHOW_SLIDE_COMMAND)))
	{
		std::string url;
		for (boost::cregex_iterator param_it(command_v.begin() + strlen(SHOW_SLIDE_COMMAND), command_v.end() - strlen(SHOW_SLIDE_COMMAND), command_param_re); param_it != boost::cregex_iterator(); ++param_it)
		{
			if (param_it->empty())
				continue;
			if ((*param_it)[1] == "URL_PARAM")
				url = (*param_it)[2];
		}
		if (url.empty())
			return;
		m_RTPModuleControl->ShowSlide(m_fakeClient->CID(), url.c_str());
	}
	else if (boost::starts_with(command_v, string_view(END_SLIDESHOW_COMMAND)))
		m_RTPModuleControl->ShowSlide(m_fakeClient->CID(), nullptr);
	else if (boost::equals(command_v, string_view(CCMD_RECORD_QUERY)))
		m_fakeClient->SendChatCommand(from, CCMD_RECORD_ACCEPT);
	else if (boost::equals(command_v, string_view(SHOWCAMERA_REQ)))
		HandleVideoChangeRequest(from, SDP_CONTENT_MAIN);
	else if (boost::equals(command_v, string_view(SHOWCONTENT_REQ)))
		HandleVideoChangeRequest(from, SDP_CONTENT_SLIDES);
	else if (boost::equals(command_v, string_view(CCMD_CAM_ON)))
		HandleDeviceControlRequest(from, boost::none, true);
	else if (boost::equals(command_v, string_view(CCMD_CAM_OFF)))
		HandleDeviceControlRequest(from, boost::none, false);
	else if (boost::equals(command_v, string_view(CCMD_MIC_ON)))
		HandleDeviceControlRequest(from, true, boost::none);
	else if (boost::equals(command_v, string_view(CCMD_MIC_OFF)))
		HandleDeviceControlRequest(from, false, boost::none);
	else if (boost::equals(command_v, string_view(CONTENTFORWARD_PULL)))
		m_RTPModuleControl->ContentForward_Pull(m_fakeClient->CID());
	else if (boost::equals(command_v, string_view(CONTENTFORWARD_PUSH)))
		m_RTPModuleControl->ContentForward_Push(m_fakeClient->CID());
	else if (boost::equals(command_v, string_view(CONTENTFORWARD_STOP)))
		m_RTPModuleControl->ContentForward_Stop(m_fakeClient->CID());
}

void VS_FakeClientControl::onChat(const char *to, const char *from, const char *dname, const char *text)
{
	m_fireChat(GetDialogID(), from ? from : string_view{}, to ? to : string_view{}, dname ? dname : string_view{}, text);
}

void VS_FakeClientControl::onFile(const char * to, const char * from, const char * dname, const char * text, const char * fileName, const char * magnet, const char * url, const char * about)
{
	m_fireFile(GetDialogID(), from ? from : string_view{}, to ? to : string_view{}, dname ? dname : string_view{}, text, std::make_tuple(fileName, magnet, url,about));
}

void VS_FakeClientControl::onPartList(const VS_FakeClientInterface::VS_ConferenceDescriptor &conf)
{
	if (conf.conf_type == CT_BROADCAST || conf.conf_type == CT_MULTISTREAM)
	{
		CheckVisibleParticipants(conf);
		UpdateVideoState(conf);
	}
}

void VS_FakeClientControl::onRoleEvent(const VS_FakeClientInterface::VS_ConferenceDescriptor &conf, VS_RoleEvent_Type type, VS_Participant_Role receivedRole, VS_Broadcast_Status bs, VS_RoleInqury_Answer result, const char* user)
{
	assert(m_fakeClient != nullptr);

	if ((conf.conf_type == CT_BROADCAST || conf.conf_type == CT_MULTISTREAM) && type == RET_NOTIFY)
	{
		CheckVisibleParticipants(conf);
		UpdateVideoState(conf);
	}
	// role requests auto aproving
	if (type == RET_INQUIRY) {
		m_fakeClient->AnswerRole(user, receivedRole, RIA_POSITIVE);
	}

	if (conf.my_id != user)		// role not about us
		return;

	if (conf.conf_subtype == GCST_ROLE) {
		if ((receivedRole == PR_LEADER || receivedRole == PR_COMMON) && result == RIA_POSITIVE) m_last_conference_role = receivedRole;
	}
	else {
		if ((receivedRole == PR_LEADER || receivedRole == PR_EQUAL) && result == RIA_POSITIVE) m_last_conference_role = receivedRole;
	}


	// due to misunderstanding in 'enum VS_Participant_Role' we do not change role for owner or moderator (i.e. PR_LEADER)
	// as a result we have not any clue about what our role is, so we trace role by last role we try to query
	VS_Participant_Role role = receivedRole;
	if (role == PR_LEADER) role = m_last_queried_tribune_role;

	if ((role == PR_PODIUM || role == PR_REPORTER) && m_last_tribune_role == PR_COMMON) {
		if(!m_disable_role_notification) m_fireTakeTribuneReply(GetDialogID(), result == RIA_POSITIVE);
	}
	else if (role == PR_COMMON && (m_last_tribune_role == PR_PODIUM || m_last_tribune_role == PR_REPORTER)) {
		if (!m_disable_role_notification) m_fireLeaveTribuneReply(GetDialogID(), result == RIA_POSITIVE);
	}

	if (result == RIA_POSITIVE) m_last_tribune_role = role;
}

void VS_FakeClientControl::onFECC(const char *from, const char *to, eFeccRequestType type, long extra_param)
{
	if (!from || !to)
		return;

	switch (type) {
	case eFeccRequestType::GET_STATE: {
		m_fakeClient->SendFECC(from, eFeccRequestType::MY_STATE, static_cast<long>((extra_param == 1)? eFeccState::ALLOWED: eFeccState::DENIED));
	} break;
	case eFeccRequestType::MY_STATE: {
		// we got someone's state, should store it somehow
	} break;
	case eFeccRequestType::SET_STATE: {
		// we got someone's state we've asked for before, should store it somehow
	} break;

		// ignore ACCESS
	case eFeccRequestType::REQUEST_ACCESS:
	case eFeccRequestType::ALLOW_ACCESS:
	case eFeccRequestType::DENY_ACCESS:
		break;
	default:
		m_RTPModuleControl->FarEndCameraControl(m_fakeClient->CID(), type, extra_param);
		break;
	}
}

void VS_FakeClientControl::onSetMediaChannels(string_view id, const std::vector<VS_MediaChannelInfo>& channels)
{
	if (!boost::iequals(id, m_fakeClient->CID()))
		return;
	auto cd = m_fakeClient->GetCurrentConference();
	long bandw_rcv = cd ? cd->remoteCaps.GetBandWRcv() : 0;
	m_fireSetMediaChannels(GetDialogID(), channels, bandw_rcv);
}

void VS_FakeClientControl::onFullIntraframeRequest(string_view id)
{
	if (!boost::iequals(id, m_fakeClient->CID()))
		return;
	m_fireFastUpdatePicture(GetDialogID());
}

void VS_FakeClientControl::onVideoStatus(string_view id, eSDP_ContentType content, bool slides_available)
{
	if (!boost::iequals(id, m_fakeClient->CID()))
		return;

	if (slides_available)
		AddFakeSlidesDevice();
	else
		AddFakeDevices();

	std::string msg;
	msg.reserve(256);
	msg.append(EXTRAVIDEOFLOW_NOTIFY).append("\n").append(CONTENTAVAILABLE_PARAM).append("=").append(slides_available ? "true" : "false").append("\n");
	switch (content)
	{
	case SDP_CONTENT_MAIN:
		msg.append(CURRENTVIDEO_PARAM).append("=").append(MAIN_VIDEO).append("\n");
		break;
	case SDP_CONTENT_SLIDES:
		msg.append(CURRENTVIDEO_PARAM).append("=").append(CONTENT_VIDEO).append("\n");
		break;
	}

	auto conf = m_fakeClient->GetCurrentConference();
	if (conf->IsGroup())
		m_fakeClient->SendChatCommand(conf->stream_conf_id, msg);
	else
		m_fakeClient->SendChatCommand(conf->peer, msg);
}

void VS_FakeClientControl::onDeviceStatus(string_view id, uint32_t value)
{
	if (!boost::iequals(id, m_fakeClient->CID()))
		return;

	if (m_camera_enabled)
	{
		// Video may be paused either because participant's camera was turned off or because participant is not on the tribune.
		// In the second case camera is not really disabled, so clear DVS_SND_PAUSED flag.
		value &= ~(DVS_SND_PAUSED << 16);
	}

	m_fakeClient->SendDeviceStatus(value);
}

void VS_FakeClientControl::onShowSlide(string_view id, const char* url, const SlideInfo &info)
{
	if (!boost::iequals(id, m_fakeClient->CID()))
		return;

	std::string msg;
	msg.reserve(256);
	msg += SHOW_SLIDE_COMMAND;
	msg.append(URL_PARAM).append("=").append(url).append("\n");
	msg.append(IMG_TYPE_PARAM).append("=").append(info.img_type).append("\n");
	msg.append(WIDTH_PARAM).append("=").append(std::to_string(info.w)).append("\n");
	msg.append(HEIGHT_PARAM).append("=").append(std::to_string(info.h)).append("\n");
	msg.append(SIZE_PARAM).append("=").append(std::to_string(info.size)).append("\n");
	if (!info.about.empty()) {
		msg.append(ABOUT_PARAM).append("=").append(info.about).append("\n");
	}
	msg.append(SLIDE_N_PARAM).append("=").append(std::to_string(info.slide_n)).append("\n");
	msg.append(SLIDE_COUNT_PARAM).append("=").append(std::to_string(info.slide_count));

	VS_Container cnt;
	cnt.AddValue(FROM_RTP_PARAM, true);

	auto conf = m_fakeClient->GetCurrentConference();
	if (conf->IsGroup())
		m_fakeClient->SendChatCommand(conf->stream_conf_id, msg, cnt);
	else
		m_fakeClient->SendChatCommand(conf->peer, msg, cnt);
}

void VS_FakeClientControl::onEndSlideShow(string_view id)
{
	if (!boost::iequals(id, m_fakeClient->CID()))
		return;

	auto conf = m_fakeClient->GetCurrentConference();
	if (conf->IsGroup())
		m_fakeClient->SendChatCommand(conf->stream_conf_id, END_SLIDESHOW_COMMAND);
	else
		m_fakeClient->SendChatCommand(conf->peer, END_SLIDESHOW_COMMAND);
}

void VS_FakeClientControl::onFECCToVisi(string_view /*id*/, eFeccRequestType type, long extra_param)
{
	auto conf = m_fakeClient->GetCurrentConference();
	if (!conf->IsGroup())
		m_fakeClient->SendFECC(conf->peer, type, extra_param);
}

void VS_FakeClientControl::CheckVisibleParticipants(const VS_FakeClientInterface::VS_ConferenceDescriptor& conf)
{
	bool visible_participants = false;
	const auto trueconf_id = m_fakeClient->GetTrueconfID();
	auto reason = FVM_GROUPCONF_NOPEOPLE;
	switch (conf.conf_subtype)
	{
	case GCST_FULL:
		visible_participants = std::any_of(conf.partList.begin(), conf.partList.end(),
			[&](const decltype(conf.partList)::value_type &part) { return  part.first != trueconf_id; }
		);
		break;
	case GCST_ALL_TO_OWNER:
	case GCST_PENDING_ALL_TO_OWNER:
	{
		auto it = conf.partList.find(trueconf_id);
		if (it != conf.partList.end()) {
			if (it->second.role == PR_LEADER)
				visible_participants = conf.partList.size() > 1;
			else {
				visible_participants = std::any_of(conf.partList.begin(), conf.partList.end(),
					[&](const decltype(conf.partList)::value_type &part) { return part.second.role == PR_LEADER; }
				);
				reason = FVM_NOSPEAKERS;
			}
		}
	}
	break;
	case GCST_ROLE:
		visible_participants = std::any_of(conf.partList.begin(), conf.partList.end(),
			[&](const decltype(conf.partList)::value_type &part) { return part.second.role == PR_LEADER || part.second.role == PR_PODIUM || part.second.role == PR_REPORTER; }
		);
		reason = FVM_NOSPEAKERS;
		break;
	}
	if (visible_participants)
		m_RTPModuleControl->SetFakeVideoMode(m_fakeClient->CID(), FVM_DISABLED);
	else
		m_RTPModuleControl->SetFakeVideoMode(m_fakeClient->CID(), reason);
}

bool VS_FakeClientControl::IsVideoNeeded(const VS_FakeClientInterface::VS_ConferenceDescriptor& conf)
{
	bool needed = false;
	const auto trueconf_id = m_fakeClient->GetTrueconfID();
	switch (conf.conf_subtype)
	{
	case GCST_FULL:
	case GCST_ALL_TO_OWNER:
	case GCST_PENDING_ALL_TO_OWNER:
		needed = true;
		break;
	case GCST_ROLE:
		const auto part = conf.partList.find(trueconf_id);
		needed = part != conf.partList.end() && (part->second.role == PR_LEADER || part->second.role == PR_PODIUM || part->second.role == PR_REPORTER);
		break;
	}
	return needed;
}

void VS_FakeClientControl::UpdateVideoState(const VS_FakeClientInterface::VS_ConferenceDescriptor& conf)
{
	if (m_camera_enabled && IsVideoNeeded(conf))
		m_RTPModuleControl->ResumeVideo(m_fakeClient->CID());
	else
		m_RTPModuleControl->PauseVideo(m_fakeClient->CID());
}

void VS_FakeClientControl::HandleVideoChangeRequest(const char* from, eSDP_ContentType content)
{
	auto conf = m_fakeClient->GetCurrentConference();
	if (!conf)
		return; // no conference active

	bool allowed = false;
	if (conf->IsGroup())
	{
		auto part_it = conf->partList.find(string_view(from));
		allowed = part_it != conf->partList.end();

		switch (conf->conf_subtype)
		{
		case GCST_FULL:
		case GCST_ALL_TO_OWNER:
		case GCST_PENDING_ALL_TO_OWNER:
		case GCST_ROLE:
			allowed = allowed && part_it->second.role & PR_LEADER;
			break;
		default:
			allowed = false;
		}
	}
	else
	{
		allowed = from == conf->peer;
	}

	if (!allowed)
		return;

	m_RTPModuleControl->SelectVideo(m_fakeClient->CID(), content);
}

void VS_FakeClientControl::HandleDeviceControlRequest(const char* from, boost::optional<bool> audio, boost::optional<bool> video)
{
	auto conf = m_fakeClient->GetCurrentConference();
	if (!conf)
		return; // no conference active

	bool allowed = false;
	if (conf->IsGroup())
	{
		auto part_it = conf->partList.find(string_view(from));
		allowed = part_it != conf->partList.end();

		switch (conf->conf_subtype)
		{
		case GCST_FULL:
		case GCST_ALL_TO_OWNER:
		case GCST_PENDING_ALL_TO_OWNER:
		case GCST_ROLE:
			allowed = allowed && part_it->second.role & PR_LEADER;
			break;
		default:
			allowed = false;
		}
	}

	if (!allowed)
		return;

	if (audio)
	{
		if (audio.value())
			m_RTPModuleControl->ResumeAudio(m_fakeClient->CID());
		else
			m_RTPModuleControl->PauseAudio(m_fakeClient->CID());
	}
	if (video)
	{
		m_camera_enabled = video.value();
		UpdateVideoState(*conf);
	}
}

VS_ClientCaps VS_FakeClientControl::GetMyClientCaps()
{
	VS_MediaFormat fmt;
	VS_ClientCaps caps;
	fmt.SetVideo(640,360,VS_VCODEC_H264,15); // Doesn't really matter, all clients support reading format from stream
	fmt.SetAudio(16000,VS_ACODEC_OPUS_B0914);
	caps.SetMediaFormat(fmt);
	caps.SetAudioSnd(VSCC_AUDIO_DEFAULT | VSCC_AUDIO_ANYFREQ | VSCC_AUDIO_ANYBLEN);
	caps.SetVideoSnd(VSCC_VIDEO_DEFAULT | VSCC_VIDEO_ANYSIZE | VSCC_VIDEO_ANYCODEC | VSCC_VIDEO_DYNCHANGE | VSCC_VIDEO_MULTIPLICITY8);
	caps.ClearMediaFormatRcv();
	caps.SetBandWRcv(video_presets::bitrate(video_presets::mode::fhd));
	caps.SetAudioRcv(VSCC_AUDIO_DEFAULT | VSCC_AUDIO_ANYFREQ | VSCC_AUDIO_ANYBLEN);
	caps.SetVideoRcv(VSCC_VIDEO_DEFAULT | VSCC_VIDEO_ANYSIZE | VSCC_VIDEO_ANYCODEC | VSCC_VIDEO_DYNCHANGE | VSCC_VIDEO_MULTIPLICITY8);
	caps.SetStreamsDC(VSCC_STREAM_ADAPTIVE_DATA_DECODE | VSCC_STREAM_CAN_DECODE_SSL | VSCC_STREAM_CAN_CHANGE_MF_RCV | VSCC_STREAM_CAN_USE_SVC);
	// Rating and level are copied from client caps in VS_SingleGW after completed initialization (m_vsterm->m_Status.MyInfo.ClientCaps)
	caps.SetRating(870);
	caps.SetLevel(VS_VIDEOLEVEL_67);
	caps.SetLevelGroup(VS_VIDEOLEVEL_87);
	caps.SetClientType(CT_TRANSCODER_CLIENT);

	uint16_t acodecs[] = {
		VS_ACODEC_PCM,
		VS_ACODEC_G711a,
		VS_ACODEC_G711mu,
		VS_ACODEC_GSM610,
		VS_ACODEC_G723,
		VS_ACODEC_G728,
		VS_ACODEC_G729A,
		VS_ACODEC_G722,
		VS_ACODEC_G7221_24,
		VS_ACODEC_G7221_32,
		VS_ACODEC_SPEEX,
		VS_ACODEC_ISAC,
		VS_ACODEC_G7221C_24,
		VS_ACODEC_G7221C_32,
		VS_ACODEC_G7221C_48,
		VS_ACODEC_OPUS_B0914,
		VS_ACODEC_MP3,
	};
	caps.SetAudioCodecs(acodecs, sizeof(acodecs)/sizeof(acodecs[0]));

	uint32_t vcodecs[] = {
		VS_VCODEC_VPX,
		VS_VCODEC_H264,
		VS_VCODEC_H263P,
		VS_VCODEC_H263,
		VS_VCODEC_H261,
	};
	caps.SetVideoCodecs(vcodecs, sizeof(vcodecs)/sizeof(vcodecs[0]));

	return caps;
}

void VS_FakeClientControl::UpdateDisplayName(string_view displayName, bool updateImmediately)
{
	m_fakeClient->UpdateDisplayName(displayName, updateImmediately);
}

bool VS_FakeClientControl::CreateConf(const VS_ConferenceInfo& info)
{
    if (info.is_public_conf) {
        char key[32 + 1] = { 0 };
        VS_GenKeyByMD5(key);
        string_view sv(key, 9);		// take only nine random chars for more user-friendly url

        std::string conf_name = SPECIAL_CONF_PREFIX;
        conf_name += std::to_string(CT_MULTISTREAM);
        conf_name += std::to_string(GCST_ROLE);
        conf_name += (std::string)sv;

        return m_fakeClient->JoinAsync(conf_name, info, m_transceiverReservationToken);
    }
    else {
        return m_fakeClient->CreateConference(100, CT_MULTISTREAM, GCST_FULL, nullptr, info.topic, "", info.is_public_conf, m_transceiverReservationToken);
    }
}

void VS_FakeClientControl::DoInviteAsync()
{
	std::vector<std::string> to_invite;
	{	std::lock_guard<std::mutex> _(m_lock);
	to_invite = std::move(m_invite_later);
	}
	for (const auto &i : to_invite) {
		m_fakeClient->InviteAsync(i, false, m_transceiverReservationToken);
	}
}

VS_CallConfig::eUserStatusScheme VS_FakeClientControl::GetUserStatusScheme() const
{
	return m_user_status_scheme;
}

void VS_FakeClientControl::SetDevicesList(const std::vector<VS_MediaChannelInfo>& channels)
{
	auto new_list = std::make_shared<std::vector<DeviceInfo>>();
	for (const auto& channel : channels)
	{
		if (channel.type == SDPMediaType::video)
		{
			if (channel.content == SDP_CONTENT_MAIN)
			{
				std::vector < std::pair<std::string, std::string>> list;
				list.emplace_back(cam.id, cam.name);
				new_list->emplace_back(std::move(list), cam.type);
			}
			else if (channel.content == SDP_CONTENT_SLIDES)
			{
				std::vector < std::pair<std::string, std::string>> list;
				list.emplace_back(content.id, content.name);
				new_list->emplace_back(std::move(list), content.type);
			}
		}
		else if (channel.type == SDPMediaType::audio)
		{
			if (channel.content == SDP_CONTENT_MAIN)
			{
				std::vector < std::pair<std::string, std::string>> list;
				list.emplace_back(mic.id, mic.name);
				new_list->emplace_back(std::move(list), mic.type);
			}
		}
	}
	m_DevicesList.store(std::move(new_list));
}

void VS_FakeClientControl::AddFakeDevices()
{
	assert(m_fakeClient != nullptr);
	const auto dev_list = m_DevicesList.load();
	for (const auto& dev : *dev_list)
	{
		if (dev.type == cam.type && dev.list.cbegin()->second == cam.name)
		{
			m_fakeClient->DevicesList(dev.type, dev.list);
			m_fakeClient->DeviceChanged(dev.type, dev.list.cbegin()->first, dev.list.cbegin()->second);
		}
		else if (dev.type == mic.type)
		{
			m_fakeClient->DevicesList(dev.type, dev.list);
			m_fakeClient->DeviceChanged(dev.type, dev.list.cbegin()->first, dev.list.cbegin()->second);
			m_fakeClient->DeviceVolume(dev.type, dev.list.cbegin()->first, 100);
		}
	}
	m_slides_initialized = false;
}

void VS_FakeClientControl::AddFakeSlidesDevice()
{
	assert(m_fakeClient != nullptr);
	if (!m_slides_initialized)
	{
		const auto dev_list = m_DevicesList.load();
		for (const auto& dev : *dev_list)
		{
			if (dev.type == content.type && dev.list.cbegin()->second == content.name)
				m_fakeClient->DevicesList(dev.type, dev.list);
		}
		for (const auto& dev : *dev_list)
		{
			if (dev.type == cam.type && dev.list.cbegin()->second == cam.name)
				m_fakeClient->DeviceChanged(dev.type, dev.list.cbegin()->first, dev.list.cbegin()->second);
		}
	}
	m_slides_initialized = true;
}

void VS_FakeClientControl::onChangeDevice(string_view type, string_view id, string_view name)
{
	m_fakeClient->DeviceChanged(type, id, name);
	if (type == cam.type)
	{
		auto conf = m_fakeClient->GetCurrentConference();
		if (!conf)
			return;
		if (id == "none")
		{
			m_camera_enabled = false;
			UpdateVideoState(*conf);
		}
		else if (id == cam.id)
		{
			m_RTPModuleControl->SelectVideo(m_fakeClient->CID(), SDP_CONTENT_MAIN);
			m_camera_enabled = true;
			UpdateVideoState(*conf);
		}
		else if (id == content.id)
		{
			m_RTPModuleControl->SelectVideo(m_fakeClient->CID(), SDP_CONTENT_SLIDES);
			m_camera_enabled = true;
			UpdateVideoState(*conf);
		}
	}
	else if (type == mic.type)
	{
		if (id == "none")
			m_RTPModuleControl->PauseAudio(m_fakeClient->CID());
		else
			m_RTPModuleControl->ResumeAudio(m_fakeClient->CID());
	}
}

void VS_FakeClientControl::onSetDeviceMute(string_view type, string_view id, bool mute)
{
	m_fakeClient->DeviceMute(type, id, mute);
	if (type == cam.type)
	{
		auto conf = m_fakeClient->GetCurrentConference();
		if (!conf)
			return;
		m_camera_enabled = !mute;
		UpdateVideoState(*conf);	
	}
	else if (type == mic.type)
	{
		if (mute)
			m_RTPModuleControl->PauseAudio(m_fakeClient->CID());
		else
			m_RTPModuleControl->ResumeAudio(m_fakeClient->CID());
	}
}
