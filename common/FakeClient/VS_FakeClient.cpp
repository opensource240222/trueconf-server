#include "FakeClient/VS_FakeClient.h"
#include "VS_FakeClientManager.h"
#include "FakeClient/VS_ConferenceInfo.h"
#include "statuslib/status_types.h"
#include "std/cpplib/md5.h"
#include "std-generic/cpplib/ignore.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/VS_ConferenceID.h"
#include "std/cpplib/VS_Policy.h"
#include "std/cpplib/VS_RcvFunc.h"
#include "std/cpplib/VS_UserData.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/MakeShared.h"
#include "std/debuglog/VS_Debug.h"
#include "transport/Message.h"

#include <boost/bind.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_FAKE_CLIENT
// 50 - SENDPARTSLIST_METHOD whith TYPE (difference list)
#define FC_PROTOCOL_VER	"50"

VS_FakeClient::VS_FakeClient(std::unique_ptr<VS_FakeEndpoint> endpoint)
	: m_endpoint(std::move(endpoint))
	, m_IsGuest(false)
	, m_update_dn_immediately(false)
	, m_feccState(eFeccRequestType::NONE)
{
	m_state.m_conf = vs::MakeShared<VS_ConferenceDescriptor>();
	m_state.m_conf->m_client = this;

	initDefaultCodecs();
	StartWaiting("TransportPing", std::chrono::seconds(8), [this]() { TransportPing(); });
}

VS_FakeClient::~VS_FakeClient(void)
{
	m_endpoint->Stop();
}

void VS_FakeClient::LoginUserAsync(string_view login, string_view passwd, string_view passwdMD5, VS_ClientType clienttype,
									string_view ip, const std::vector<std::string> &aliases)
{
	VS_FakeClientManager::Instance().LoginPolicy().Request(ip, login,
		boost::bind(	&VS_FakeClient::LoginUserAllowed,
						std::static_pointer_cast<VS_FakeClient>(shared_from_this()),
						_1,
						std::string(login),
						std::string(passwd),
						std::string(passwdMD5),
						clienttype,
						std::string(ip),
						aliases
					)
	);
}

void VS_FakeClient::LoginUserAllowed(bool is_allowed, const std::string &login,
							const std::string &passwd, const std::string &passwd_md5,VS_ClientType clienttype, const std::string &ip, const std::vector<std::string> &aliases)
{
	if( !is_allowed )
	{
		VS_UserLoggedin_Result res = VS_FakeClientManager::Instance().LoginPolicy().FailResult();
		onLoginResponse( res );
		return;
	}

	std::string trueconfId;
	bool encryptPasswd = true;

	{
		std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
		trueconfId = m_state.trueconfId;
		m_state.used_for_login_name = login;
		m_state.used_for_login_ip = ip;
		if (clienttype==CT_WEB_CLIENT)
			m_state.app_name = "TrueConf WebClient";
		if (clienttype==CT_TRANSCODER_CLIENT)
			m_state.app_name = "SIP-H.323 Client";
		if (clienttype == CT_TRANSCODER)
			m_state.app_name = "Transcoder";

		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, GETAPPPROPERTIES_METHOD);
		rCnt.AddValue(NAME_PARAM, m_state.app_name);
		ComposeSend(rCnt, CONFIGURATION_SRV);

	//	TODO: check auth type from app properties
	//	const char *encryptType = m_propertiesFromServer.GetStrValueRef("authentication_method");
	//	if (encryptType && strcmp("1", encryptType) == 0)
	//	{
	//		encryptPasswd = false;
	//	}

	//	temporarely fix
		encryptPasswd = passwd.empty();
	}

	if (!trueconfId.empty()) {
		onLoginResponse(USER_LOGGEDIN_OK);
		return;
	}
	else if (login.empty() || (passwd.empty() && passwd_md5.empty() && m_state.autologin_key.empty())) {
		onLoginResponse(SILENT_REJECT_LOGIN);
		return ;
	}

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, LOGINUSER_METHOD);
	rCnt.AddValue(LOGIN_PARAM, login);
	rCnt.AddValue(USER_DEFAULT_DOMAIN, "");

	if (m_state.autologin_key.empty()) {
		if (passwd_md5.empty()) {
			if (encryptPasswd) {
				char md5_pass[256]; *md5_pass = 0;
				VS_ConvertToMD5(passwd, md5_pass);
				rCnt.AddValue(PASSWORD_PARAM, md5_pass);
			}
			else {
				rCnt.AddValue(PASSWORD_PARAM, passwd);
			}
		}
		else
			rCnt.AddValue(PASSWORD_PARAM, passwd_md5);
	}
	else {
		rCnt.AddValue(KEY_PARAM, m_state.autologin_key);
	}

	rCnt.AddValueI32(CLIENTTYPE_PARAM, clienttype);
	rCnt.AddValue(APPNAME_PARAM, m_state.app_name);
	rCnt.AddValue(HASH_PARAM, m_state.app_id);
	rCnt.AddValueI32(FIELD1_PARAM, 0); // timeout ?

	for (const auto& alias : aliases) {
		rCnt.AddValue(ALIAS_PARAM, alias);
	}
	StartWaiting(LOGINUSER_METHOD, std::chrono::seconds(10), [this]() { _onLoginResponse(vs::ignore<VS_Container>()); });
	ComposeSend(rCnt, AUTH_SRV);
}

void VS_FakeClient::_onLoginResponse(VS_Container &cnt)
{
	int32_t res = ACCESS_DENIED;
	std::string tc_id;

	{
		std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);

		bool is_waiting_for_login = CancelWait( LOGINUSER_METHOD ) || cnt.IsEmpty();
		if (!is_waiting_for_login) return;

		cnt.GetValue(RESULT_PARAM, res);
		const char * session_key = cnt.GetStrValueRef(SESSION_PARAM);
		if (session_key) {
			m_state.sessionKey = session_key;
		}

		if (res == USER_LOGGEDIN_OK)
		{
			const char *p = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
			if (p)
				m_state.displayName = p;
			p = cnt.GetStrValueRef(USERNAME_PARAM);
			if (p)
				tc_id = m_state.trueconfId = p;
			p = cnt.GetStrValueRef(KEY_PARAM);
			if (p)
				m_state.autologin_key = p;

			//int32_t TarifRestrictions(0);
			//cnt.GetValueI32(TARIFRESTR_PARAM, TarifRestrictions);
			cnt.GetValueI32(RIGHTS_PARAM, m_state.rigths);

			UpdateStatus();
			m_IsGuest = VS_RealUserLogin::IsGuest(tc_id);

			SetAppProperty("app_name", m_state.app_name);
			SetAppProperty("DisplayName", m_state.displayName);
		}

		VS_FakeClientManager::Instance().LoginPolicy().SetResult(m_state.used_for_login_ip, m_state.used_for_login_name, res == USER_LOGGEDIN_OK);
	}

	onLoginResponse( (VS_UserLoggedin_Result)(res) );

	SendAppProperties();

	if (res == USER_LOGGEDIN_OK && !m_display_name_for_update.empty() && !m_alias.empty())
		UpdateDisplayNameImp(m_display_name_for_update, m_update_dn_immediately);
}

void VS_FakeClient::Logout()
{
	if (!m_state.m_conf->stream_conf_id.empty())
		Hangup(m_state.m_conf, false);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, LOGOUTUSER_METHOD);
	ComposeSend(rCnt, AUTH_SRV);
	StartWaiting(LOGOUTUSER_METHOD, std::chrono::seconds(10), [this]() { _onLogoutResponse(vs::ignore<VS_Container>()); });
}

void VS_FakeClient::_onLogoutResponse(VS_Container &cnt)
{
	CancelWait(LOGOUTUSER_METHOD);
	_onDeleteConference(vs::ignore<VS_Container>()); // delete all conferences

	int32_t cause = USER_LOGGEDOUT_BY_REQUEST;
	cnt.GetValue(CAUSE_PARAM, cause);
	onLogoutResponse((VS_UserLoggedout_Cause)cause);

	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	m_state.trueconfId.clear();
	m_state.autologin_key.clear();
	//VS_FakeEndpointManager::Instance().DeleteEndpoint(CID());
	m_endpoint->Stop();
	m_alias.clear();
	m_update_dn_immediately = false;
	m_IsGuest = false;
}

void VS_FakeClient::SetDefaultCaps(const VS_ClientCaps &caps)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	m_state.default_caps = caps;
}

bool VS_FakeClient::InviteAsync(const std::string &to, bool force_create, const std::string& transcReserveToken)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	if ( m_state.trueconfId.empty() ) return false;
	if (force_create) return false;
	auto& d = *m_state.m_conf;
	if ( d.conf_id.empty() || d.stream_conf_id.empty() )	return CreateConference( d, 2, CT_PRIVATE, 0, to.c_str(), std::string(), "", false, transcReserveToken);
	else return InviteToConference( d, to.c_str() );
}

bool VS_FakeClient::CreateConference(VS_ConferenceDescriptor &d, long maxPart, VS_Conference_Type confType, long subType, const char *name, const std::string &topic, const char *passwd, bool is_public, const std::string& transcReserveToken)
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, CREATECONFERENCE_METHOD);
	rCnt.AddValueI32(MAXPARTISIPANTS_PARAM, maxPart);
	rCnt.AddValueI32(DURATION_PARAM, 0);
	rCnt.AddValueI32(TYPE_PARAM, confType);
	rCnt.AddValueI32(SUBTYPE_PARAM, subType);
	rCnt.AddValueI32(SCOPE_PARAM, is_public ? GS_PUBLIC : GS_PERSONAL);
	rCnt.AddValue(NAME_PARAM, name);
	rCnt.AddValue(PASSWORD_PARAM, passwd);
	rCnt.AddValue(TOPIC_PARAM, topic);
	rCnt.AddValue(RESERVATION_TOKEN, transcReserveToken);
	CapsToContainer(rCnt, m_state.default_caps);

	if (name)
		d.conf_id = name;
	d.password = passwd;
	d.conf_type = confType;
	d.stream_conf_id.clear();
	d.m_state = VS_ConferenceDescriptor::e_conf_creating;

	StartWaiting(CREATECONFERENCE_METHOD, std::chrono::seconds(10), [this]() { _onConfCreateResponse(vs::ignore<VS_Container>()); });
	ComposeSend(rCnt, CONFERENCE_SRV);
	return true;
}

bool VS_FakeClient::InviteToConference(VS_ConferenceDescriptor &d, const char *to)
{
	if (d.stream_conf_id.empty() || d.m_state != VS_ConferenceDescriptor::e_conf_active) return false;
	if (d.conf_type != CT_MULTISTREAM && d.conf_type != CT_PUBLIC) {
		d.m_state = VS_ConferenceDescriptor::e_conf_wait_for_user_response;
	}
	d.is_incomming = false;
	d.peer = to;
	d.peer_displayName.clear();

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, INVITE_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, d.stream_conf_id);
	rCnt.AddValue(CALLID_PARAM, to);
	rCnt.AddValueI32(DIRECTCONNECT_PARAM, NO_DIRECT_CONNECT);
	rCnt.AddValueI32(TYPE_PARAM, d.conf_type);
	CapsToContainer(rCnt, m_state.default_caps);


	ComposeSend(rCnt, CONFERENCE_SRV);
	return true;
}


void VS_FakeClient::_onConfCreateResponse(VS_Container &cnt)
{
	bool is_waiting_for_create = CancelWait( CREATECONFERENCE_METHOD ) || cnt.IsEmpty() ;
	if (!is_waiting_for_create) return;

	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	auto& d = *m_state.m_conf;

	int32_t result;
	if (cnt.GetValue(RESULT_PARAM, result) && result==CONFERENCE_CREATED_OK)
	{
		const char *conf = cnt.GetStrValueRef(NAME_PARAM);
		if (!conf) conf = ""; // should not happen
		d.stream_conf_id = conf;
		if (d.conf_id.empty()) d.conf_id = d.stream_conf_id;	// unnamed conf was created, use name that was given to us by MC Service
		d.m_state = VS_ConferenceDescriptor::e_conf_active;
		bool res = d.conf_type != CT_PRIVATE || InviteToConference(d, d.conf_id.c_str());

		if (!res) onConferenceStateChange("InviteFailed", d);
		else onConferenceStateChange("ConferenceCreated", d);

		if (d.conf_type == CT_MULTISTREAM) {
			d.my_id = m_state.trueconfId;
			StartWaiting(JOIN_METHOD, std::chrono::seconds(10), [this]() { _onJoinResponse(vs::ignore<VS_Container>()); });
		}
	}
	else
	{
		onConferenceStateChange("CreateconferenceFailed", d);
		d.conf_id.clear();
	}
}

bool VS_FakeClient::CreateConference(long maxPart, VS_Conference_Type confType, long subType, const char *name, const std::string &topic, const char *passwd, bool is_public, const std::string& transcReserveToken)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	if (m_state.trueconfId.empty()) return false;

	auto& d = *m_state.m_conf;
	return CreateConference(d, maxPart, confType, subType, name, topic, passwd, is_public, transcReserveToken);
}

bool VS_FakeClient::JoinAsync(const std::string &to, const VS_ConferenceInfo& info, const std::string& transcReserveToken)
{
	return JoinAsync(to, info, m_state.default_caps, transcReserveToken);
}

bool VS_FakeClient::JoinAsync(const std::string &to, const VS_ConferenceInfo& info, const VS_ClientCaps &caps, const std::string& transcReserveToken)
{
	VS_Container rCnt;

	{
		std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
		if (m_state.trueconfId.empty() || IsWaitingFor(JOIN_METHOD)) return false;

		if (m_state.m_conf->conf_id == to)
		{
			onConferenceStateChange( "alreadyInConference", *m_state.m_conf);

			// re-send slides
			VS_Container cnt2;
			cnt2.AddValue(METHOD_PARAM, SENDSLIDESTOUSER_METHOD);
			cnt2.AddValue(CONFERENCE_PARAM, m_state.m_conf->stream_conf_id);
			cnt2.AddValue(CALLID_PARAM, m_state.trueconfId);
			ComposeSend(cnt2, CHAT_SRV);
			return true;
		}

		rCnt.AddValue(APPID_PARAM, m_state.app_id);
		rCnt.AddValue(USERNAME_PARAM, m_state.trueconfId);
		rCnt.AddValue(DISPLAYNAME_PARAM, m_state.displayName);
		CapsToContainer(rCnt, caps);

		m_state.m_conf->is_incomming = false;
		m_state.m_conf->conf_id = to;
		m_state.m_conf->m_state = VS_ConferenceDescriptor::e_conf_inviting;
		m_state.m_conf->conf_type = (VS_Conference_Type)-1;
		m_state.m_conf->my_id = m_state.trueconfId;
		m_state.m_conf->conf_subtype = GCST_FULL;
	}

	rCnt.AddValue(METHOD_PARAM, JOIN_METHOD);
	rCnt.AddValueI32(TYPE_PARAM, CT_MULTISTREAM);
	rCnt.AddValue(PASSWORD_PARAM, "");
	rCnt.AddValue(TOPIC_PARAM, info.topic);
	rCnt.AddValueI32(SCOPE_PARAM, info.is_public_conf ? GS_PUBLIC : GS_PERSONAL);
	rCnt.AddValue(NAME_PARAM, to);
	rCnt.AddValue(RESERVATION_TOKEN, transcReserveToken);

	StartWaiting(JOIN_METHOD, std::chrono::seconds(10), [this]() { _onJoinResponse(vs::ignore<VS_Container>()); });

	ComposeSend(rCnt, CONFERENCE_SRV);
	return true;
}

void VS_FakeClient::_onJoinResponse(VS_Container &cnt)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const char *str_ref = NULL;
	auto d = m_state.m_conf;

	bool is_waiting_for_join = CancelWait( JOIN_METHOD ) || cnt.IsEmpty() ;
	if (!is_waiting_for_join) return;
	if ( d->conf_id.empty() ) return;

	d->m_last_reject_reason = REJECTED_BY_TIMEOUT;
	cnt.GetValueI32(RESULT_PARAM, d->m_last_reject_reason);
	cnt.GetValueI32(TYPE_PARAM, d->conf_type);
	cnt.GetValueI32(SUBTYPE_PARAM, d->conf_subtype);
	cnt.GetValueI32(CMR_FLAGS_PARAM, d->m_cmr_flags);

	str_ref = cnt.GetStrValueRef(CONFERENCE_PARAM);
	if (str_ref) d->stream_conf_id = str_ref;
	str_ref = cnt.GetStrValueRef(NAME_PARAM);
	if (str_ref) d->topic = str_ref;
	str_ref = cnt.GetStrValueRef(USERNAME_PARAM);
	if (str_ref) d->owner = str_ref;

	d->m_state = VS_ConferenceDescriptor::e_conf_active;

	onConferenceStateChange("joinResponse", *d);

	if (d->m_last_reject_reason != JOIN_OK) d->conf_id.clear();
	else
	{
		JoinStreamConference( *d );
		UpdateStatus();
		if (!IsWaitingFor(PING_METHOD))
			PingConferences();
	}
}

bool VS_FakeClient::Reject(const std::shared_ptr<VS_ConferenceDescriptor>& conf, VS_Reject_Cause reason)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, REJECT_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, conf->stream_conf_id);
	rCnt.AddValue(NAME_PARAM, conf->owner);
	rCnt.AddValue(CAUSE_PARAM, static_cast<int32_t>(reason));
	rCnt.AddValue(USERNAME_PARAM, m_state.trueconfId);
	ComposeSend(rCnt, CONFERENCE_SRV, VS_GetConfEndpoint(conf->stream_conf_id.c_str()));
	conf->stream_conf_id.clear();
	conf->conf_id.clear();
	return true;
}

void VS_FakeClient::_onInite(VS_Container &cnt)
{
	const char *conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char *user = cnt.GetStrValueRef(NAME_PARAM);
	const char *owner_displayname = cnt.GetStrValueRef(DISPLAYNAME_PARAM);

	int32_t type = CT_PRIVATE;
	cnt.GetValue(TYPE_PARAM, type);

	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);

	if (!m_state.m_conf->conf_id.empty())
	{
		// already in conf
		auto tmp = vs::MakeShared<VS_ConferenceDescriptor>();
		tmp->stream_conf_id = conf;
		tmp->conf_id = user;
		Reject(tmp, PARTISIPANT_IS_BUSY);
		return;
	}
	else
	{
		VS_ConferenceDescriptor &d = *m_state.m_conf;
		d.is_incomming = true;
		d.stream_conf_id = conf;
		d.conf_id = user;
		d.topic.clear();
		d.peer = user;
		d.peer_displayName.clear();
		d.owner = user ? user : ( cnt.GetStrValueRef(SERVER_PARAM) ? cnt.GetStrValueRef(SERVER_PARAM) : "");
		if (owner_displayname)
		{
			d.owner_displayname = owner_displayname;
		}
		d.conf_type = CT_PRIVATE;
		CapsFromContainer(cnt, d.remoteCaps);
		d.m_state = VS_ConferenceDescriptor::e_conf_wait_for_user_response;

		onConferenceStateChange("Invite", d);
	}
}

void VS_FakeClient::_onAccept(VS_Container &cnt)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	auto& d = *m_state.m_conf;
	if (d.m_state != VS_ConferenceDescriptor::e_conf_wait_for_user_response) return;

	bool is_waiting_for_accept = CancelWait( ACCEPT_METHOD ) || cnt.IsEmpty() ;

	const char *confName = cnt.GetStrValueRef(NAME_PARAM);
	const char *streamConfName = cnt.GetStrValueRef(CONFERENCE_PARAM);
	cnt.GetValueI32(CMR_FLAGS_PARAM, d.m_cmr_flags);

	if (confName)
		d.conf_id = confName;
	if (streamConfName)
		d.stream_conf_id = streamConfName;
	CapsFromContainer(cnt, d.remoteCaps);

	d.m_last_reject_reason = JOIN_OK;
	d.m_state = VS_ConferenceDescriptor::e_conf_active;

	onConferenceStateChange("joinResponse", d);

	UpdateStatus();
	if (!IsWaitingFor(PING_METHOD))
		PingConferences();

}

void VS_FakeClient::_onIniteToMulti(VS_Container &cnt)
{
	const char *conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char *user = cnt.GetStrValueRef(USERNAME_PARAM);
	const char *conf_name = cnt.GetStrValueRef(NAME_PARAM);

	int32_t type = CT_MULTISTREAM;
	int32_t sub_type = GCST_UNDEF;
	cnt.GetValue(TYPE_PARAM, type);
	cnt.GetValue(SUBTYPE_PARAM, sub_type);

	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);

	if (!m_state.m_conf->conf_id.empty())
	{
		auto tmp = vs::MakeShared<VS_ConferenceDescriptor>();
		tmp->stream_conf_id = conf ? conf : "";
		tmp->conf_id = conf_name ? conf_name : "";
		Reject(tmp, REJECTED_BY_PARTICIPANT);
		return;
	}
	else
	{
		VS_ConferenceDescriptor &d = *m_state.m_conf;

		d.is_incomming = true;
		d.stream_conf_id = conf ? conf : "";
		d.conf_id = cnt.GetStrValueRef(NAME_PARAM) ? cnt.GetStrValueRef(NAME_PARAM) : "";
		d.password = cnt.GetStrValueRef(PASSWORD_PARAM) ? cnt.GetStrValueRef(PASSWORD_PARAM) : "";
		d.topic = cnt.GetStrValueRef(DISPLAYNAME_PARAM) ? cnt.GetStrValueRef(DISPLAYNAME_PARAM) : "";
		d.owner_displayname = d.topic = cnt.GetStrValueRef(DISPLAYNAME_PARAM) ? cnt.GetStrValueRef(DISPLAYNAME_PARAM) : "";
		d.owner = user ? user : (cnt.GetStrValueRef(SERVER_PARAM) ? cnt.GetStrValueRef(SERVER_PARAM) : "");
		d.conf_type = (VS_Conference_Type)type;
		d.conf_subtype = (VS_GroupConf_SubType)sub_type;
		d.my_id = m_state.trueconfId;
		CapsFromContainer(cnt, d.remoteCaps);
		d.m_state = VS_ConferenceDescriptor::e_conf_wait_for_user_response;

		onConferenceStateChange("Invite", d);
	}
}

bool VS_FakeClient::Accept(const std::shared_ptr<VS_ConferenceDescriptor>& conf)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const auto& d = conf;

	if (d->m_state != VS_ConferenceDescriptor::e_conf_wait_for_user_response)
	{
		d->conf_id.clear();
		return false;
	}

	VS_Container rCnt;

	const bool is_groupconf = d->IsGroup();
	const char* method = is_groupconf ? JOIN_METHOD : ACCEPT_METHOD;

	rCnt.AddValue(METHOD_PARAM, method);
	rCnt.AddValue(APPID_PARAM, m_state.app_id);
	rCnt.AddValue(USERNAME_PARAM, m_state.trueconfId);
	rCnt.AddValue(NAME_PARAM, conf->conf_id);
	rCnt.AddValueI32(TYPE_PARAM, d->conf_type);
	rCnt.AddValue(PASSWORD_PARAM, d->password);
	rCnt.AddValue(CONFERENCE_PARAM, conf->stream_conf_id);
	rCnt.AddValueI32(DIRECTCONNECT_PARAM, NO_DIRECT_CONNECT);
	CapsToContainer( rCnt, m_state.default_caps );
	StartWaiting(method, std::chrono::seconds(60), [this, is_groupconf]() {
		if (is_groupconf)
			_onJoinResponse(vs::ignore<VS_Container>());
		else
			_onAccept(vs::ignore<VS_Container>());
	});

	ComposeSend(rCnt, CONFERENCE_SRV, VS_GetConfEndpoint(d->stream_conf_id.c_str()));
	return true;
}

std::shared_ptr<VS_FakeClient::VS_ConferenceDescriptor> VS_FakeClient::GetCurrentConference()
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	return m_state.m_conf;
}

bool VS_FakeClient::QueryRole(const char *name, long role)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const auto& d = m_state.m_conf;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, ROLEEVENT_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, d->stream_conf_id);
	rCnt.AddValueI32(TYPE_PARAM, RET_INQUIRY);
	rCnt.AddValue(USERNAME_PARAM, name);
	rCnt.AddValueI32(ROLE_PARAM, role);

	ComposeSend(rCnt, CONFERENCE_SRV);
	return true;
}

bool VS_FakeClient::AnswerRole(const char *name, long role, long result)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const auto& d = m_state.m_conf;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, ROLEEVENT_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, d->stream_conf_id);
	rCnt.AddValueI32(TYPE_PARAM, RET_ANSWER);
	rCnt.AddValue(USERNAME_PARAM, name);
	rCnt.AddValueI32(ROLE_PARAM, role);
	rCnt.AddValueI32(RESULT_PARAM, result);

	ComposeSend(rCnt, CONFERENCE_SRV);
	return true;
}

bool VS_FakeClient::ConnectSender(long fltr)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const auto& d = m_state.m_conf;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, CONNECTSENDER_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, d->stream_conf_id);
	rCnt.AddValue(USERNAME_PARAM, m_state.trueconfId);
	rCnt.AddValueI32(MEDIAFILTR_PARAM, fltr);

	ComposeSend(rCnt, CONFERENCE_SRV);
	return true;
}


bool VS_FakeClient::ManageLayoutFunc(const char* func, const char* id1, const char* id2)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const auto& d = m_state.m_conf;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, MANAGELAYOUT_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, d->stream_conf_id);
	rCnt.AddValue(USERNAME_PARAM, m_state.trueconfId);
	rCnt.AddValue(FUNC_PARAM, func);
	rCnt.AddValue(CALLID_PARAM, id1);
	rCnt.AddValue(CALLID2_PARAM, id2);

	ComposeSend(rCnt, CONFERENCE_SRV);
	return true;
}

bool VS_FakeClient::ManageLayoutPT(const long pt)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const auto& d = m_state.m_conf;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, MANAGELAYOUT_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, d->stream_conf_id);
	rCnt.AddValue(USERNAME_PARAM, m_state.trueconfId);
	rCnt.AddValue(FUNC_PARAM, "PriorityType");
	rCnt.AddValueI32(TYPE_PARAM, pt);

	ComposeSend(rCnt, CONFERENCE_SRV);
	return true;
}

bool VS_FakeClient::SendUsageStat()
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const auto& d = m_state.m_conf;

	unsigned bytes_s = m_conf_stat.avg_send_bitrate*m_conf_stat.participant_time >> 3;
	unsigned bytes_r = m_conf_stat.avg_rcv_bitrate*m_conf_stat.broadcast_time >> 3;

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, LOGPARTSTAT_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, d->stream_conf_id);
	cnt.AddValue(CALLID_PARAM, d->my_id);
	cnt.AddValueI32(BYTES_SENT_PARAM, bytes_s);
	cnt.AddValueI32(BYTES_RECEIVED_PARAM, bytes_r);
	cnt.AddValue(CONF_BASE_STAT_PARAM, &m_conf_stat, m_conf_stat.size_of_stat);

	ComposeSend(cnt, LOG_SRV);

	char buff[1024]; *buff = 0;
	sprintf(buff, "Name        %s\nBytes   s/r %5u/%-5u kB\nBitrate s/r %5d/%-5d kbit\nDuration    %02d:%02d:%02d",
		d->stream_conf_id.c_str(), bytes_s, bytes_r, m_conf_stat.avg_send_bitrate, m_conf_stat.avg_rcv_bitrate,
		(m_conf_stat.broadcast_time / 3600) % 24, (m_conf_stat.broadcast_time / 60) % 60, m_conf_stat.broadcast_time % 60);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, SETPROPERTIES_METHOD);
	rCnt.AddValue(HASH_PARAM, m_state.app_id);
	rCnt.AddValue(NAME_PARAM, "Last Conf");
	rCnt.AddValue(PROPERTY_PARAM, buff);

	ComposeSend(rCnt, CONFIGURATION_SRV);
	return true;
}

void VS_FakeClient::DevicesList(string_view type, const std::vector<std::pair<std::string, std::string>> &list)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const auto& d = m_state.m_conf;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, DEVICESLIST_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, d->stream_conf_id);
	cnt.AddValue(USERNAME_PARAM, d->my_id);
	cnt.AddValue(TYPE_PARAM, type);
	for (auto &i : list) {
		cnt.AddValue(ID_PARAM, i.first);
		cnt.AddValue(NAME_PARAM, i.second);
	}

	ComposeSend(cnt, CONFERENCE_SRV);
}

void VS_FakeClient::DeviceChanged(string_view type, string_view id, string_view name)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const auto& d = m_state.m_conf;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, DEVICECHANGED_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, d->stream_conf_id);
	cnt.AddValue(USERNAME_PARAM, d->my_id);
	cnt.AddValue(TYPE_PARAM, type);
	cnt.AddValue(ID_PARAM, id);
	cnt.AddValue(NAME_PARAM, name);

	ComposeSend(cnt, CONFERENCE_SRV);
}

void VS_FakeClient::DeviceMute(string_view type, string_view id, bool mute)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const auto& d = m_state.m_conf;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, DEVICESTATE_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, d->stream_conf_id);
	cnt.AddValue(USERNAME_PARAM, d->my_id);
	cnt.AddValue(TYPE_PARAM, type);
	cnt.AddValue(ID_PARAM, id);
	cnt.AddValue(MUTE_PARAM, mute);

	ComposeSend(cnt, CONFERENCE_SRV);
}

void VS_FakeClient::DeviceVolume(string_view type, string_view id, int32_t volume)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const auto& d = m_state.m_conf;
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, DEVICESTATE_METHOD);
	cnt.AddValue(CONFERENCE_PARAM, d->stream_conf_id);
	cnt.AddValue(USERNAME_PARAM, d->my_id);
	cnt.AddValue(TYPE_PARAM, type);
	cnt.AddValue(ID_PARAM, id);
	cnt.AddValue(VOLUME_PARAM, volume);

	ComposeSend(cnt, CONFERENCE_SRV);
}

VS_FakeEndpoint& VS_FakeClient::GetEndpoint() const
{
	return *m_endpoint;
}

void VS_FakeClient::_onRoleEvent(VS_Container &cnt)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);

	int32_t val;
	VS_RoleEvent_Type type = RET_INQUIRY;
	if (cnt.GetValue(TYPE_PARAM, val))
		type = (VS_RoleEvent_Type)val;
	VS_Participant_Role role = PR_EQUAL;
	VS_Broadcast_Status bs = (VS_Broadcast_Status)0;
	if (cnt.GetValue(ROLE_PARAM, val))
	{
		role = (VS_Participant_Role)(val&0xff);
		bs = (VS_Broadcast_Status)((val>>8)&0xffff);
	}
	VS_RoleInqury_Answer result = RIA_POSITIVE;
	if (cnt.GetValue(RESULT_PARAM, val))
		result = (VS_RoleInqury_Answer)val;

	const char* user = cnt.GetStrValueRef(USERNAME_PARAM);
	const char* conf = cnt.GetStrValueRef(CONFERENCE_PARAM);

	const auto& d = m_state.m_conf;

	if (d->stream_conf_id == conf) {
		if (type == RET_NOTIFY)
		{
			auto part_it = d->partList.find(string_view(user));
			if (part_it != d->partList.end())
				part_it->second.role = role;
			if (d->my_id == user)
				d->m_role = role;
		}

		onRoleEvent(*d, type, role, bs, result, user);
	}
}


void VS_FakeClient::_onReqInvite(VS_Container &cnt)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	m_req_invites.push_back(cnt);

	const char *user = cnt.GetStrValueRef(CALLID_PARAM);
	const char *dn = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
	onReqInvite(user, dn);
}


bool VS_FakeClient::Hangup(const std::shared_ptr<VS_ConferenceDescriptor>& conf, bool forall)
{
	VS_Container cnt;
	std::string key;
	VS_Container temp_cnt;
	{
		std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);

		//clean FECC
		m_feccState = eFeccRequestType::NONE;

		if (conf->conf_id.empty())
			return false;
		key.reserve(16/*HANGUP_METHOD*/ + 1 + conf->stream_conf_id.size());
		key.append(HANGUP_METHOD).append("_").append(conf->stream_conf_id);

		if (IsWaitingFor(key)) // hungup is pending
			return false;

		cnt.AddValue(METHOD_PARAM, HANGUP_METHOD);
		cnt.AddValue(NAME_PARAM, m_state.trueconfId);
		cnt.AddValue(CONFERENCE_PARAM, conf->stream_conf_id);
		if (forall)
			cnt.AddValueI32(RESULT_PARAM, 1);

		LeaveStreamConference(*conf);

		temp_cnt.AddValue(NAME_PARAM, conf->stream_conf_id);

		//conf->conf_id.clear();
		//conf->stream_conf_id.clear();
		//conf->m_state = VS_ConferenceDescriptor::e_terminating;
	}

	StartWaiting(std::move(key), std::chrono::seconds(10), [this, temp_cnt = std::move(temp_cnt)]() mutable { _onDeleteConference(temp_cnt); });
	ComposeSend(cnt, CONFERENCE_SRV, VS_GetConfEndpoint(conf->stream_conf_id.c_str()));

	return true;
}

void VS_FakeClient::SendChatMessage(const std::string &to, const std::string &msg)
{
	bool to_conf = false;
	bool allowedSend = true;
	{
		std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
		to_conf = to == m_state.m_conf->stream_conf_id;
		if (!to_conf)
			allowedSend = isAllowedToSend(to);
	}
	if (!allowedSend)
		return;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, SENDMESSAGE_METHOD);
	rCnt.AddValue(FROM_PARAM, m_state.trueconfId);
	rCnt.AddValue(DISPLAYNAME_PARAM, m_state.displayName);
	rCnt.AddValue(MESSAGE_PARAM, msg);
	rCnt.AddValue(to_conf ? CONFERENCE_PARAM : TO_PARAM, to);

	ComposeSend(rCnt, CHAT_SRV);
}

void VS_FakeClient::SendChatMessage(const std::string &stream_conf_id, const std::string &to, const std::string &msg)
{
	std::string current_conf;

	{
		std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
		current_conf = m_state.m_conf->stream_conf_id;
	}

	std::string valid_msg = VS_ValidateHtml(msg);

	if (to.empty())
	{
		if (stream_conf_id != current_conf)
			return;

		SendChatMessage(stream_conf_id, valid_msg);
	}
	else
	{
		SendChatMessage(to, valid_msg);
	}
}

void VS_FakeClient::SendFECC(string_view to, eFeccRequestType type, std::int32_t extraParam)
{
	bool allowedSend = true;
	{
		std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
		const bool to_conf = to == m_state.m_conf->stream_conf_id;
		if (!to_conf && !isAllowedToSend(to))
		{
			return;
		}

		if (type != eFeccRequestType::MY_STATE)
		{
			switch (m_feccState)
			{
			case eFeccRequestType::REQUEST_ACCESS: VS_FALLTHROUGH;
			case eFeccRequestType::DENY_ACCESS: return;
			case eFeccRequestType::NONE: VS_FALLTHROUGH;
			case eFeccRequestType::DENY_BY_TIMEOUT_ACCESS:
			{
				m_feccState = eFeccRequestType::REQUEST_ACCESS;
				allowedSend = false;
				break;
			}
			default: break;
			}
		}
	}

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, FECC_METHOD);
	rCnt.AddValue(FROM_PARAM, m_state.trueconfId);
	rCnt.AddValue(TO_PARAM, to);

	assert(!allowedSend ? type != eFeccRequestType::MY_STATE : true /*any type*/);

	if (!allowedSend)
	{
		rCnt.AddValueI32(TYPE_PARAM, eFeccRequestType::REQUEST_ACCESS);
	}
	else
	{
		rCnt.AddValueI32(TYPE_PARAM, type);
		if (type == eFeccRequestType::SAVE_PRESET ||
			type == eFeccRequestType::USE_PRESET)
			rCnt.AddValueI32(PRESET_NUM_PARAM, extraParam);
		else if (type == eFeccRequestType::SET_STATE ||
			type == eFeccRequestType::MY_STATE)
			rCnt.AddValueI32(FECC_STATE_PARAM, extraParam);
	}

	ComposeSend(rCnt, CONFERENCE_SRV);
}

void VS_FakeClient::SendChatCommand(const std::string &to, const std::string &msg, const VS_Container &cnt)
{
	bool to_conf = false;
	bool allowedSend = true;
	{
		std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
		to_conf = to == m_state.m_conf->stream_conf_id;
		if (!to_conf)
			allowedSend = isAllowedToSend(to);
	}
	if (!allowedSend)
		return;

	VS_Container rCnt(cnt);
	rCnt.AddValue(METHOD_PARAM, SENDCOMMAND_METHOD);
	rCnt.AddValue(FROM_PARAM, m_state.trueconfId);
	rCnt.AddValue(MESSAGE_PARAM, msg);
	rCnt.AddValue(to_conf ? CONFERENCE_PARAM : TO_PARAM, to);

	if (to_conf)
		ComposeSend(rCnt, CHAT_SRV, VS_GetConfEndpoint(to.c_str()));
	else
		ComposeSend(rCnt, CHAT_SRV, 0, to.c_str());
}

void VS_FakeClient::_onDeleteConference( VS_Container &cnt )
{
	const char *p = cnt.GetStrValueRef(NAME_PARAM);
	// if p == NULL delete all conferences
	std::string key;
	{
		std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
		if (m_state.m_conf->stream_conf_id.empty())
			return;

		if (NULL == p || m_state.m_conf->stream_conf_id == p)
		{
			key.reserve(16/*HANGUP_METHOD*/ + 1 + m_state.m_conf->stream_conf_id.size());
			key.append(HANGUP_METHOD).append("_").append(m_state.m_conf->stream_conf_id);
			LeaveStreamConference(*m_state.m_conf);
			onConferenceStateChange("deleteConference", *m_state.m_conf);
			m_state.m_conf->conf_id.clear();
			m_state.m_conf->stream_conf_id.clear();
			m_state.m_conf->m_state = VS_ConferenceDescriptor::e_terminating;
		}
	}
	if (!key.empty())
		CancelWait(key);

	if (p) UpdateStatus();
}

void VS_FakeClient::_onUserRegistrationInfo( VS_Container &cnt )
{
	const char *user = cnt.GetStrValueRef(USERNAME_PARAM);
	const char *call_id = cnt.GetStrValueRef(CALLID_PARAM);

	if (!user || !call_id)
		return;

	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	if (m_state.m_conf->m_state != VS_ConferenceDescriptor::e_conf_active
	 && m_state.m_conf->m_state != VS_ConferenceDescriptor::e_conf_wait_for_user_response)
		return;

	if (m_state.m_conf->peer == call_id)
	{
		m_state.m_conf->peer = user;
		auto dn = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
		if (dn && *dn)
			m_state.m_conf->peer_displayName = dn;
	}
}

void VS_FakeClient::_onFECC(VS_Container &cnt)
{
	int32_t type; cnt.GetValue(TYPE_PARAM, type);
	const char *to = cnt.GetStrValueRef(TO_PARAM);
	const char *from = cnt.GetStrValueRef(FROM_PARAM);

	eFeccRequestType frt_type = static_cast<eFeccRequestType>(type);

	if (!from || !to)
		return;

	if (frt_type == eFeccRequestType::ALLOW_ACCESS || frt_type == eFeccRequestType::DENY_ACCESS || frt_type == eFeccRequestType::DENY_BY_TIMEOUT_ACCESS)
	{
		//for send access sip->tc
		std::lock_guard<decltype(m_lock)> _{ m_lock };
		m_feccState = frt_type;
		return;
	}

	int32_t extra_param = -1;
	if (frt_type == eFeccRequestType::SAVE_PRESET ||
		frt_type == eFeccRequestType::USE_PRESET)
		cnt.GetValue(PRESET_NUM_PARAM, extra_param);
	else if (frt_type == eFeccRequestType::SET_STATE ||
		frt_type == eFeccRequestType::MY_STATE)
		cnt.GetValue(FECC_STATE_PARAM, extra_param);
	else if (frt_type == eFeccRequestType::GET_STATE)
	{
		bool IsOperator(false);
		cnt.GetValue(IS_OPERATOR_PARAM, IsOperator);
		if (IsOperator)
			extra_param = 1;
	}

	onFECC(from, to, frt_type, extra_param);
}

void VS_FakeClient::_onReject(VS_Container &cnt)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	LeaveStreamConference(*m_state.m_conf);
	onConferenceStateChange("reject", *m_state.m_conf);
}

void VS_FakeClient::_onDeviceStatus(VS_Container & cnt)
{
	const char * name = cnt.GetStrValueRef(USERNAME_PARAM);
	int32_t DeviceStatus = 1 | (1 << 16);
	cnt.GetValue(DEVICESTATUS_PARAM, DeviceStatus);
	onDeviceStatus(name, DeviceStatus);
}

void VS_FakeClient::_onListenersFltr(VS_Container & cnt)
{
	int32_t fltr = 0; cnt.GetValue(MEDIAFILTR_PARAM, fltr);
	bool rKeyFrame = false;
	{
		std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
		VS_ConferenceDescriptor &d = *m_state.m_conf;

		rKeyFrame = (fltr&VS_RcvFunc::FLTR_RCV_VIDEO) && !(d.m_lfltr&VS_RcvFunc::FLTR_RCV_VIDEO);
		d.m_lfltr = fltr;
	}
	if (rKeyFrame)
		onRequestKeyFrame();
	onListenersFltr(fltr);
}

void VS_FakeClient::_onChangeDevice(VS_Container & cnt)
{
	auto type = cnt.GetStrValueView(TYPE_PARAM);
	auto id = cnt.GetStrValueView(ID_PARAM);
	auto name = cnt.GetStrValueView(NAME_PARAM);
	onChangeDevice(type, id, name);
}

void VS_FakeClient::_onSetDeviceState(VS_Container & cnt)
{
	auto type = cnt.GetStrValueView(TYPE_PARAM);
	auto id = cnt.GetStrValueView(ID_PARAM);
	bool mute = false;
	if (cnt.GetValue(MUTE_PARAM, mute))
		onSetDeviceMute(type, id, mute);
}

void VS_FakeClient::_onSendCommandToConfSrv(VS_Container &cnt)
{
	bool req_key_frame(false);
	cnt.GetValue(REQESTKEYFRAME_PARAM, req_key_frame);
	// SmirnovK: keyframe request and svc bitrate is sent by VS_ConfControlInterface
	//if (req_key_frame)
	//	onRequestKeyFrame();
}

void VS_FakeClient::_onChat( VS_Container &cnt )
{
	const char *type = cnt.GetStrValueRef(TYPE_PARAM);
	const char *from = cnt.GetStrValueRef(FROM_PARAM);
	const char *msg = cnt.GetStrValueRef(MESSAGE_PARAM);
	const char *dname  = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
	if (dname == NULL) dname = from;
	const char *conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char *to = cnt.GetStrValueRef(TO_PARAM);

	if (!from || !msg || !(conf || to))
		return;

	if (type && strcasecmp(type, "application/x-bittorrent") == 0)
	{
		const char * fileName = cnt.GetStrValueRef(NAME_PARAM);
		const char * magnet = cnt.GetStrValueRef(LINK_PARAM);
		const char * url = cnt.GetStrValueRef(URL_PARAM);
		const char * about = cnt.GetStrValueRef(ABOUT_PARAM);
		onFile(to, conf, from, dname, msg, fileName, magnet, url, about);
	}
	else
		onChat(to, conf, from, dname, msg);


}


void VS_FakeClient::_onCommand( VS_Container &cnt )
{
	const char *from = cnt.GetStrValueRef(FROM_PARAM);
	const char *msg = cnt.GetStrValueRef(MESSAGE_PARAM);
	if (!from || !msg) return;
	onCommand(from, msg);
}

void VS_FakeClient::_onPartList(VS_Container &cnt)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const char* Conf = cnt.GetStrValueRef(CONFERENCE_PARAM);
	VS_ParticipantListType type = PLT_OLD; cnt.GetValueI32(TYPE_PARAM, type);
	const char* User = nullptr;
	VS_ConferenceDescriptor &d = *m_state.m_conf;

	bool fill = false;
	if (d.stream_conf_id == Conf) {
		if (type == PLT_OLD || type == PLT_ALL) {
			d.partList.clear();
			fill = true;
		}
		else if (type == PLT_ADD || type == PLT_UPD) {
			fill = true;
			User = cnt.GetStrValueRef(USERNAME_PARAM);
		}
		else if (type == PLT_DEL) {
			User = cnt.GetStrValueRef(USERNAME_PARAM);
			d.partList.erase(User);
		}

		if (fill) {
			cnt.Reset();
			decltype(d.partList)::iterator it = d.partList.end();
			while (cnt.Next()) {
				if (strcasecmp(cnt.GetName(), USERNAME_PARAM) == 0) {
					std::string us = cnt.GetStrValueRef();
					it = type == PLT_UPD ? d.partList.find(us) : d.partList.end();
					if (it == d.partList.end())
						it = d.partList.emplace(us, PartInfo()).first;
					it->second.user_id = it->first;
				}
				else if (it == d.partList.end()) {
					continue;
				}
				else if (strcasecmp(cnt.GetName(), DISPLAYNAME_PARAM) == 0) {
					it->second.dn = cnt.GetStrValueRef();
					if (it->second.dn.length() == 0)
						it->second.dn = it->first;
				}
				else if (strcasecmp(cnt.GetName(), ROLE_PARAM) == 0) {
					cnt.GetValueI32(it->second.role);
					it->second.onpodium = ((it->second.role >> 8) & BS_SND_PAUSED) == 0;
					it->second.role &= 0xff;
				}
				else if (strcasecmp(cnt.GetName(), DEVICESTATUS_PARAM) == 0) {
					cnt.GetValueI32(it->second.device_status);
				}
				else if (strcasecmp(cnt.GetName(), IS_OPERATOR_PARAM) == 0) {
					cnt.GetValue(it->second.is_operator);
				}
			}
		}
	}
	onPartList(d, User, type);
}


void VS_FakeClient::_onSetProperties( VS_Container &cnt )
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	cnt.CopyTo(m_propertiesFromServer);
	m_server_properties_arrived.set();
}

VS_Container VS_FakeClient::GetPropsFromServer()
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	return m_propertiesFromServer;
}

void VS_FakeClient::PingConferences()
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	bool is_any_active = false;

	VS_ConferenceDescriptor &d = *m_state.m_conf;
	if (d.m_state == VS_ConferenceDescriptor::e_conf_active)
	{
		is_any_active = true;
		VS_Container ping_cnt;
		ping_cnt.AddValue(METHOD_PARAM, PING_METHOD);
		ping_cnt.AddValue(CONFERENCE_PARAM, d.stream_conf_id);
		ping_cnt.AddValue(USERNAME_PARAM, m_state.trueconfId);
		ping_cnt.AddValueI32(SYSLOAD_PARAM, 50);
		ComposeSend(ping_cnt, CONFERENCE_SRV, VS_GetConfEndpoint(d.stream_conf_id.c_str()));
	}

	if (is_any_active)
		StartWaiting(PING_METHOD, std::chrono::seconds(10), [this]() { PingConferences(); });
}

void VS_FakeClient::TransportPing()
{
	StartWaiting("TransportPing", std::chrono::seconds(8), [this]() { TransportPing(); });

	m_endpoint->Send(transport::Message::Make()
		.SeqNumber(0xffffffff)
		.TimeLimit(5000)
		.AddString("p") // TODO: Replace with tranport::c_ping_opcode
		.Body("", 1)
	);
}

void VS_FakeClient::UpdateStatus()
{
	// this is already locked. do not lock it again.

	VS_UserPresence_Status status = USER_AVAIL;
	const auto& d = m_state.m_conf;

	if (d->m_state == VS_ConferenceDescriptor::e_conf_active)
	{
		status =  USER_BUSY;
		if (d->conf_type == CT_PUBLIC && d->owner == m_state.trueconfId)
			status = USER_PUBLIC;
		if ((d->conf_type == CT_BROADCAST || d->conf_type == CT_MULTISTREAM) && d->owner == m_state.trueconfId)
			status = USER_MULTIHOST;
	}

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, UPDATESTATUS_METHOD);
	rCnt.AddValueI32(USERPRESSTATUS_PARAM, status);
	ComposeSend(rCnt, PRESENCE_SRV);

	onUpdateStatus(m_state.trueconfId.c_str(), status);
}

VS_FakeClient::ClientState VS_FakeClient::GetClientState()
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	return m_state;
}

std::string VS_FakeClient::GetTrueconfID()
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	return m_state.trueconfId;
}

void VS_FakeClient::OnReceive(const transport::Message& msg)
{
	static const struct {
		const char* service;
		const char* method;
		void (VS_FakeClient::*handler)(VS_Container&);
	} dispatch_table[] = {
		{ AUTH_SRV,          USERLOGGEDIN_METHOD,         &VS_FakeClient::_onLoginResponse },
		{ AUTH_SRV,          USERLOGGEDOUT_METHOD,        &VS_FakeClient::_onLogoutResponse },
		{ CONFERENCE_SRV,    CONFERENCECREATED_METHOD,    &VS_FakeClient::_onConfCreateResponse },
		{ CONFERENCE_SRV,    INVITE_METHOD,               &VS_FakeClient::_onInite },
		{ CONFERENCE_SRV,    ACCEPT_METHOD,               &VS_FakeClient::_onAccept },
		{ CONFERENCE_SRV,    INVITETOMULTI_METHOD,        &VS_FakeClient::_onIniteToMulti },
		{ CONFERENCE_SRV,    JOIN_METHOD,                 &VS_FakeClient::_onJoinResponse },
		{ CONFERENCE_SRV,    ROLEEVENT_METHOD,            &VS_FakeClient::_onRoleEvent },
		{ CONFERENCE_SRV,    REQINVITE_METHOD,            &VS_FakeClient::_onReqInvite },
		{ CONFERENCE_SRV,    CONFERENCEDELETED_METHOD,    &VS_FakeClient::_onDeleteConference },
		{ CONFERENCE_SRV,    USERREGISTRATIONINFO_METHOD, &VS_FakeClient::_onUserRegistrationInfo },
		{ CONFERENCE_SRV,    FECC_METHOD,                 &VS_FakeClient::_onFECC },
		{ CONFERENCE_SRV,    REJECT_METHOD,               &VS_FakeClient::_onReject },
		{ CONFERENCE_SRV,    DEVICESTATUS_PARAM,          &VS_FakeClient::_onDeviceStatus },
		{ CONFERENCE_SRV,    CHANGEDEVICE_METHOD,         &VS_FakeClient::_onChangeDevice },
		{ CONFERENCE_SRV,    SETDEVICESTATE_METHOD,       &VS_FakeClient::_onSetDeviceState },
		{ CONFERENCE_SRV,    LISTENERSFLTR_METHOD,        &VS_FakeClient::_onListenersFltr },
		{ CONFERENCE_SRV,    SENDCOMMAND_METHOD,          &VS_FakeClient::_onSendCommandToConfSrv },
		{ CHAT_SRV,          SENDMESSAGE_METHOD,          &VS_FakeClient::_onChat },
		{ CHAT_SRV,          SENDCOMMAND_METHOD,          &VS_FakeClient::_onCommand },
		{ PRESENCE_SRV,      SENDPARTSLIST_METHOD,        &VS_FakeClient::_onPartList },
		{ PRESENCE_SRV,      SEARCHADDRESSBOOK_METHOD,    &VS_FakeClient::_onPartList },
		{ PRESENCE_SRV,      ADDTOADDRESSBOOK_METHOD,     &VS_FakeClient::_onPartList },
		{ PRESENCE_SRV,      REMOVEFROMADDRESSBOOK_METHOD,&VS_FakeClient::_onPartList },
		{ CONFIGURATION_SRV, SETPROPERTIES_METHOD,        &VS_FakeClient::_onSetProperties }
	};

	const char *service = msg.DstService();

	VS_Container cnt;
	if (service && cnt.Deserialize(msg.Body(), msg.BodySize()))
	{
		const char *method = cnt.GetStrValueRef(METHOD_PARAM);
		if (method)
			for (const auto& x : dispatch_table)
				if (::strcmp(method, x.method) == 0 && ::strcmp(service, x.service) == 0)
				{
					auto start = std::chrono::steady_clock::now();
					(this->*x.handler)(cnt);
					const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::steady_clock::now() - start);
					if (elapsed >= std::chrono::milliseconds(10))
					{
						dstream4 << "VS_FakeClient(" << CID() << "): message "
							<< service << ':' << method << " handled in " << elapsed.count() << "ms";
					}
					return;
				}
	}

	// you are here only if message can not be dispatched with dispatch_table
}

void VS_FakeClient::OnError(unsigned /*error*/)
{
	// connection is no more valid, delete it
	// but this error should only happen on stop server
}


void VS_FakeClient::ComposeSend(VS_Container &cnt, const char* service, const char* server, const char *user, unsigned long timeout)
{
	void* body = nullptr;
	VS_SCOPE_EXIT { ::free(body); };
	size_t bodySize;
	if (!cnt.SerializeAlloc(body, bodySize))
		return;

	m_endpoint->Send(transport::Message::Make()
		.TimeLimit(timeout)
		.AddString(FC_PROTOCOL_VER)
		.SrcService(service)
		.DstService(service)
		.DstUser(user ? user : "")
		.DstServer(server ? server : "")
		.Body(body, bodySize)
	);
}

//----------------------- Timer

void VS_FakeClient::StartWaiting(std::string id, std::chrono::steady_clock::duration delay, std::function<void ()> handler)
{
	std::lock_guard<decltype(m_timeout_handlers_mutex)> lock(m_timeout_handlers_mutex);
	const auto expire_time = std::chrono::steady_clock::now() + delay;
	auto it = std::find_if(m_timeout_handlers.begin(), m_timeout_handlers.end(), [&](const timeout_info& x) {
		return x.expire_time > expire_time;
	});
	m_timeout_handlers.emplace(it, std::move(id), expire_time, std::move(handler));
}

bool VS_FakeClient::CancelWait(string_view id)
{
	std::lock_guard<decltype(m_timeout_handlers_mutex)> lock(m_timeout_handlers_mutex);
	auto it = std::find_if(m_timeout_handlers.begin(), m_timeout_handlers.end(), [&](const timeout_info& x) {
		return x.id == id;
	});
	if (it == m_timeout_handlers.end())
		return false;
	m_timeout_handlers.erase(it);
	return true;
}

bool VS_FakeClient::CallNowAndCancelWait(string_view id)
{
	std::unique_lock<decltype(m_timeout_handlers_mutex)> lock(m_timeout_handlers_mutex);
	auto it = std::find_if(m_timeout_handlers.begin(), m_timeout_handlers.end(), [&](const timeout_info& x) {
		return x.id == id;
	});
	if (it == m_timeout_handlers.end())
		return false;
	auto handler = std::move(it->handler);
	lock.unlock();
	handler();
	return true;
}

bool VS_FakeClient::IsWaitingFor(string_view id)
{
	std::lock_guard<decltype(m_timeout_handlers_mutex)> lock(m_timeout_handlers_mutex);
	auto it = std::find_if(m_timeout_handlers.begin(), m_timeout_handlers.end(), [&](const timeout_info& x) {
		return x.id == id;
	});
	return it != m_timeout_handlers.end();
}

void VS_FakeClient::Timeout()
{
	std::vector<std::function<void ()>> to_call;
	const auto now = std::chrono::steady_clock::now();

	std::unique_lock<decltype(m_timeout_handlers_mutex)> lock(m_timeout_handlers_mutex);
	auto it = m_timeout_handlers.begin();
	for ( ; it != m_timeout_handlers.end(); ++it)
	{
		if (it->expire_time > now)
			break;
		to_call.emplace_back(std::move(it->handler));
	}
	m_timeout_handlers.erase(m_timeout_handlers.begin(), it);


	lock.unlock();
	for (auto& handler : to_call)
		handler();
}

bool VS_FakeClient::isAllowedToSend(string_view id)
{
	if (!m_IsGuest)
		return true;

	// std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	const auto& d = m_state.m_conf;
	auto part_it = d->partList.find(id);
	return part_it != d->partList.end();
}


void VS_FakeClient::initDefaultCodecs()
{
	VS_MediaFormat fmt;
	VS_ClientCaps caps;
	fmt.SetVideo(1280, 720, VS_VCODEC_VPX, 15);
	fmt.SetAudio(16000,VS_ACODEC_OPUS_B0914);
	//fmt.SetAudio(16000,VS_ACODEC_SPEEX);
	caps.SetMediaFormat(fmt);
	caps.SetAudioSnd(VSCC_AUDIO_DEFAULT);
	caps.SetVideoSnd(VSCC_VIDEO_DEFAULT);
	caps.ClearMediaFormatRcv();

	caps.SetAudioRcv(VSCC_AUDIO_DEFAULT|VSCC_AUDIO_ANYFREQ|VSCC_AUDIO_ANYBLEN);
	int32_t dwVideoFlag = VSCC_VIDEO_DEFAULT | VSCC_VIDEO_ANYSIZE | VSCC_VIDEO_ANYCODEC | VSCC_VIDEO_DYNCHANGE | VSCC_VIDEO_MULTIPLICITY8;
	caps.SetVideoRcv(dwVideoFlag);
	int32_t StreamDC = VSCC_STREAM_ADAPTIVE_DATA_DECODE | VSCC_STREAM_CAN_DECODE_SSL | VSCC_STREAM_CAN_USE_SVC;
	caps.SetStreamsDC(StreamDC);
	//caps.SetRating(rating);
	//m_Status.MyInfo.ClientCaps.SetLevel(level);
	//m_Status.MyInfo.ClientCaps.SetLevelGroup(level_group);

	uint16_t acodecs[] = { VS_ACODEC_OPUS_B0914, VS_ACODEC_ISAC };
	caps.SetAudioCodecs(acodecs, sizeof(acodecs) / sizeof(acodecs[0]));

	uint32_t vcodecs[] = { VS_VCODEC_VPX, VS_VCODEC_H264 };
	caps.SetVideoCodecs(vcodecs, sizeof(vcodecs) / sizeof(vcodecs[0]));
	caps.SetClientType(VS_ClientType::CT_TRANSCODER); /// temporary hack for tc client compability

	m_state.default_caps = caps;
}

std::string VS_FakeClient::GetAppProperty(const std::string& name)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	auto it = m_state.app_properties.find(name);
	if (it != m_state.app_properties.end())
		return it->second;
	else
	{
		if (name == METHOD_PARAM)
			return std::string();

		if (!m_server_properties_arrived.wait_for(std::chrono::seconds(2))) {
			dstream3 << "Error!\t Get server property while server properties have not arrived!\n";
			return std::string();
		}
		const char* const value = m_propertiesFromServer.GetStrValueRef(name);
		if (value)
			return std::string(value);
		return std::string();
	}
}

void VS_FakeClient::SetAppProperty( const std::string &name, const std::string &val )
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	m_state.app_properties[name] = val;
}

void VS_FakeClient::SendAppProperties()
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, SETPROPERTIES_METHOD);
	rCnt.AddValue(HASH_PARAM, m_state.app_id);

	for (auto & i : m_state.app_properties) {
		rCnt.AddValue(NAME_PARAM, i.first);
		rCnt.AddValue(PROPERTY_PARAM, i.second);
	}

	ComposeSend(rCnt, CONFIGURATION_SRV);
}

bool VS_FakeClient::CapsFromContainer(VS_Container &cnt, VS_ClientCaps &caps)
{
	size_t size = 0;
	const void * buff = cnt.GetBinValueRef(CLIENTCAPS_PARAM, size);
	if ( buff ) caps.Set(buff, size);
	return buff != NULL;
}

bool VS_FakeClient::CapsToContainer(VS_Container &cnt, const VS_ClientCaps &caps)
{
	void* body;
	size_t bodySize;
	((VS_ClientCaps *)&caps)->Get(body, bodySize);
	cnt.AddValue(CLIENTCAPS_PARAM, body, bodySize);
	free(body); body = 0; bodySize = 0;
	return true;
}

const std::string& VS_FakeClient::CID() const
{
	return m_endpoint->CID();
}

void VS_FakeClient::SetAlias(string_view toId)
{
	if (!toId.empty())
		m_alias = std::string(toId);
}

void VS_FakeClient::SetAppId(const char * appname)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	if (appname && *appname)
		m_state.app_id = appname;
	else
		m_state.app_id = CID();
}

void VS_FakeClient::UpdateDisplayName(string_view displayName, bool updateImmediately)
{
	if (displayName.empty())
		return;

	m_display_name_for_update = std::string(displayName);
	m_update_dn_immediately = updateImmediately;

	UpdateDisplayNameImp(displayName, updateImmediately);
}

void VS_FakeClient::UpdateDisplayNameImp(string_view displayName, bool updateImmediately)
{
	std::string conf_peer;
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, USERREGISTRATIONINFO_METHOD);
	{
		std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
		rCnt.AddValue(FROM_PARAM, m_state.used_for_login_name);
		rCnt.AddValue(ALIAS_PARAM, m_alias);
		if (m_state.m_conf)
			conf_peer = m_state.m_conf->peer;
	}
	rCnt.AddValue(DISPLAYNAME_PARAM, displayName);
	if (updateImmediately)
	{
		rCnt.AddValueI32(CMD_PARAM, 1);
		if (!conf_peer.empty())
			rCnt.AddValue(NAME_PARAM, conf_peer);
	}

	ComposeSend(rCnt, CONFERENCE_SRV);
}

void VS_FakeClient::SendDeviceStatus(unsigned value)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, DEVICESTATUS_METHOD);
	rCnt.AddValue(USERNAME_PARAM, m_state.trueconfId);
	rCnt.AddValueI32(DEVICESTATUS_PARAM, value);
	ComposeSend(rCnt, CONFERENCE_SRV);
}

bool VS_FakeClient::KickFromConference(string_view to)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);

	auto& d = *m_state.m_conf;
	if (m_state.trueconfId.empty() ||
		d.stream_conf_id.empty() ||
		d.m_state != VS_ConferenceDescriptor::e_conf_active) return false;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, KICK_METHOD);
	rCnt.AddValue(CONFERENCE_PARAM, d.stream_conf_id);
	rCnt.AddValue(NAME_PARAM, to);

	ComposeSend(rCnt, CONFERENCE_SRV);
	return true;
}

bool VS_FakeClient::ReqInviteAsync(string_view to)
{
	VS_Container rCnt;

	{
		std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);
		if (m_state.trueconfId.empty() || IsWaitingFor(REQINVITE_METHOD)) return false;

		if (m_state.m_conf->conf_id == to) {
			onConferenceStateChange("alreadyInConference", *m_state.m_conf);
		}

		rCnt.AddValue(APPID_PARAM, m_state.app_id);
		rCnt.AddValue(USERNAME_PARAM, m_state.trueconfId);
		CapsToContainer(rCnt, m_state.default_caps);
	}

	rCnt.AddValue(METHOD_PARAM, REQINVITE_METHOD);
	rCnt.AddValueI32(TYPE_PARAM, CT_MULTISTREAM);
	rCnt.AddValue(CALLID_PARAM, to);

	StartWaiting(REQINVITE_METHOD, std::chrono::seconds(100), [this]() {  _onReqInvite(vs::ignore<VS_Container>()); });

	ComposeSend(rCnt, CONFERENCE_SRV);
	return true;
}

bool VS_FakeClient::AnswerReqInvite(string_view to, bool accept)
{
	std::lock_guard<vs::fast_recursive_mutex> lock(m_lock);

	bool answered = false;

	for (auto it = m_req_invites.begin(); it != m_req_invites.end(); ) {
		auto &cnt = *it;

		const char *user = cnt.GetStrValueRef(CALLID_PARAM);

		if (string_view(user) == to) {
			if (!answered) {
				const char* call_id_alias = cnt.GetStrValueRef(ALIAS_PARAM);

				const auto& d = m_state.m_conf;

				VS_Container rCnt;

				if (accept) {
					rCnt.AddValue(METHOD_PARAM, INVITE_METHOD);
					rCnt.AddValue(CONFERENCE_PARAM, d->stream_conf_id);
					rCnt.AddValueI32(TYPE_PARAM, CT_MULTISTREAM);
					rCnt.AddValue(CALLID_PARAM, user);
					rCnt.AddValue(ALIAS_PARAM, call_id_alias);
					rCnt.AddValue(NAME_PARAM, d->topic);
				} else {
					rCnt.AddValue(METHOD_PARAM, REJECT_METHOD);
					rCnt.AddValue(CONFERENCE_PARAM, d->stream_conf_id);
					rCnt.AddValue(NAME_PARAM, to);
					rCnt.AddValueI32(CAUSE_PARAM, REJECTED_BY_PARTICIPANT);
					rCnt.AddValue(USERNAME_PARAM, m_state.trueconfId);
					rCnt.AddValue(ALIAS_PARAM, call_id_alias);
				}

				ComposeSend(rCnt, CONFERENCE_SRV);
				answered = true;
			}

			it = m_req_invites.erase(it);
		} else {
			++it;
		}
	}

	return answered;
}
