#include "VCSAuthService.h"
#include "../../AppServer/Services/VS_ConferenceService.h"
#include "../../ServerServices/VS_ReadLicense.h"
#include "VS_ConfMemStorage.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "../../common/std/cpplib/VS_CallIDUtils.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/clib/strcasecmp.h"
#include "../../common/std/cpplib/VS_JsonConverter.h"
#include <boost/bind.hpp>

#include "../../common/std/cpplib/json/writer.h"
#include "../../common/std/cpplib/json/elements.h"

#define DEBUG_CURRENT_MODULE VS_DM_LOGINS

bool VS_VCSAuthService::Init( const char *our_endpoint, const char *our_service, const bool permittedAll )
{
	m_login_session_secret.Resize(256);
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	key.GetValue(m_login_session_secret, 256, VS_REG_STRING_VT, "Status Security");
	m_OnUserLoggedIn.connect( boost::bind(&VS_VCSAuthService::OnUserLoginEnd_Event, this, _1, _2) );
	m_OnUserLoggedOut.connect( boost::bind(&VS_VCSAuthService::OnUserLogoff_Event, this, _1, _2) );
	m_invites_storage.SetPresenceService(m_presenceService);
	if (!lic::ShareService::Init()) return false;
	return true;
}

void VS_VCSAuthService::AsyncDestroy()
{
	m_is_stopping = true;
}

bool VS_VCSAuthService::Processing(std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	auto dbStorage = g_dbStorage;
	if (!dbStorage)
	{
		m_recvMess = nullptr;
		return true;
	}
#ifdef _DEBUG
	dbStorage->SetAuthThreadID();
#endif
	if (recvMess == 0)
		return true;

	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		{
			m_recvMess = recvMess.get();

			VS_Container cnt;
			if (cnt.Deserialize(m_recvMess->Body(), m_recvMess->BodySize())) {
				const char* method = 0;
				if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0) {
					dprint3("Processing %20s; cid:%s user:%s srv:%s \n", method, recvMess->SrcCID(), recvMess->SrcUser(), recvMess->SrcServer());
					// Process methods
					if (strcasecmp(method, LOGINUSER_METHOD) == 0) {
						LoginUser_Method(cnt);
					} else if (strcasecmp(method, LOGOUTUSER_METHOD) == 0) {
						LogoutUser_Method(m_recvMess->SrcUser(), e_invalid);
					} else if (strcasecmp(method, CHECKUSERLOGINSTATUS_METHOD) == 0) {
						CheckUserLoginStatus_Method();
					} else if (strcasecmp(method, POINTDISCONNECTED_METHOD) == 0) {
						OnPointDisconnected_Method(cnt);
					}
					else if (strcasecmp(method, POINTCONNECTED_METHOD) == 0) {
						OnPointConnected_Method(cnt);
					}
					else if (strcasecmp(method, AUTHORIZE_METHOD) == 0) {
						Authorize_Method(recvMess->SrcCID(), &cnt);
					} else if (strcasecmp(method, ONUSERCHANGE_METHOD) == 0) {
						int32_t type = 0;
						cnt.GetValue(TYPE_PARAM, type);
						OnUserChange_Method(cnt.GetStrValueRef(USERNAME_PARAM), static_cast<OnUserChangeType>(type), cnt.GetStrValueRef(SESSION_PARAM));
					} else if (strcasecmp(method, ONADDRESSBOOKCHANGE_METHOD) == 0){
						OnAddressBookChange_Method( cnt );
					} else if (strcasecmp(method, REQUPDATEACCOUNT_METHOD) == 0) {
						ReqUpdateAccount_Method(cnt);
					} else if (strcasecmp(method, AUTOINVITE_METHOD) == 0) {
						AutoInvite_Method(cnt);
					}
					else if (strcasecmp(method, SETREGID_METHOD) == 0) {
						SetRegID_Method(cnt);
					}
					else if (strcasecmp(method, INVITEUSERS_METHOD) == 0) {
						InviteUsers_Method(cnt);
					}
#ifdef _SVKS_M_BUILD_
					else if (strcasecmp(method, AUTH_BY_ECP_METHOD) == 0) {
						AuthByECP_Method(recvMess->SrcCID(), cnt);
					}
#endif
					else {
						lic::ShareService::Processing(std::move(recvMess));
					}
				}
			}
		}
		break;
	default:
		break;
	};
	m_recvMess = nullptr;
	return false;
}


bool VS_VCSAuthService::AddSessionKey(const char* call_id, VS_Container& cnt)
{
	if (!call_id || !*call_id)
		return false;

	const auto first_jan_2000 = std::chrono::system_clock::from_time_t(946684800);	// 2000-01-01 00:00:00+00:00
	const auto tommorrow = std::chrono::system_clock::now() + std::chrono::hours(24);
	int diff = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(tommorrow - first_jan_2000).count() / 60);

	char date_hash[9] = {0};
	snprintf(date_hash, 9, "%08X", diff);

	MD5 md5;
	md5.Update(call_id);
	md5.Update(date_hash);
	md5.Update(SimpleStrToStringView(m_login_session_secret));
	md5.Final();
	char md5_hash[33];
	md5.GetString(md5_hash);

	VS_SimpleStr out(date_hash); out+= md5_hash;
	cnt.AddValue(SESSION_PARAM, out);
	return true;
}

bool AddApplicationSettings(const VS_UserData &user, VS_Container &cnt)
{
	json::Object jsonAppSettings;
	for (const auto &param : user.m_appSettings) {
		json::Object paramJson;

		auto& name = param.first;
		auto& value = param.second;

		paramJson.Insert(json::Object::Member("Value", json::Number(value.integer)));
		paramJson.Insert(json::Object::Member("IsLocked", json::Number(value.IsLocked)));

		jsonAppSettings.Insert(json::Object::Member(name, paramJson));
	}

	std::stringstream resultStream;
	json::Writer::Write(jsonAppSettings, resultStream);
	std::string appSettings = resultStream.str();

	cnt.AddValue(APPLICATION_SETTINGS_PARAM, appSettings.c_str());

	return true;
}

void VS_VCSAuthService::LoginUser_Method(VS_Container &cnt)
{
	const char* login = cnt.GetStrValueRef(LOGIN_PARAM);

	if (login == NULL) return;

	VS_ClientType client_type(CT_SIMPLE_CLIENT);
	cnt.GetValueI32(CLIENTTYPE_PARAM, client_type);

	boost::function< void (bool) > cb = boost::bind(&VS_VCSAuthService::LoginUser, this, _1, cnt,
					VS_SimpleStr(m_recvMess->SrcCID()),
					VS_SimpleStr(m_recvMess->SrcUser()),
					VS_SimpleStr(m_recvMess->AddString())
				);

	std::string ip;
	if (m_policy.ShouldBeChecked( client_type ) && GetIPByCID( m_recvMess->SrcCID(), ip) )
		m_policy.Request(ip, login, cb);
	else
		cb( true );
}

VS_UserLoggedin_Result VS_VCSAuthService::LoginUser(bool is_allowed, VS_Container &cnt, VS_SimpleStr &cid, const char* user, const char* addString)
{
	VS_Container cnt2;
	std::string ip;
	VS_UserData	ud;
	VS_UserLoggedin_Result result(ACCESS_DENIED);

	cnt2.AddValue(METHOD_PARAM, USERLOGGEDIN_METHOD);
	const char* login = cnt.GetStrValueRef(LOGIN_PARAM);
	const char* appID = cnt.GetStrValueRef(HASH_PARAM);
	const char* appName = cnt.GetStrValueRef(APPNAME_PARAM);

	VS_SCOPE_EXIT{
		if (!login)
			login = "";
		VS_Container pcnt;
		pcnt.AddValue(LOGIN_PARAM, login);
		pcnt.AddValueI32(RESULT_PARAM, result);
		pcnt.AddValue(APPID_PARAM, appID);
		pcnt.AddValue(APPNAME_PARAM, appName);

		const char* luser = nullptr;
		if (result == USER_LOGGEDIN_OK) {
			luser = ud.m_name.m_str;
			pcnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
			char rights[33]; rights[32] = 0;
			uint32_t ur = (uint32_t)ud.m_rights;
			for (int i = 0; i < 32; i++)
			{
				rights[i] = ur & 0x80000000 ? '1' : '0';
				ur <<= 1;
			}
			pcnt.AddValue(RIGHTS_PARAM, rights);
		}
		else{
			luser = login;
		}

		if (ip.empty())
			GetIPByCID(cid, ip);
		if (!ip.empty())
			pcnt.AddValue(HOST_PARAM, ip);

		std::string payload(ConvertToJsonStr(pcnt));

		VS_Container log_cnt;
		log_cnt.AddValue(OBJECT_TYPE_PARAM, USER_OBJECT_TYPE);
		log_cnt.AddValue(OBJECT_NAME_PARAM, luser);
		log_cnt.AddValue(EVENT_TYPE_PARAM, LOGIN_USER_EVENT_TYPE);
		log_cnt.AddValue(PAYLOAD_PARAM, payload.c_str());
		PostRequest(OurEndpoint(), 0, log_cnt, 0, LOG_SRV);
	};

	if (!is_allowed) {
		result = m_policy.FailResult();
		if (result == SILENT_REJECT_LOGIN)
			return result;

		cnt2.AddValueI32(RESULT_PARAM, result);
		PostUnauth(cid, cnt2);
		return result;
	}

	if (!cid) {
		std::string temp_id;
		if (user)
			temp_id = GetCIDByUID(user);
		if (temp_id.empty()) {
			dprint0("EMPTY Ep in Login\n");
			return result;
		}
		cid = temp_id.c_str();
	}

	const char* password = cnt.GetStrValueRef(PASSWORD_PARAM);
	VS_SimpleStr key = cnt.GetStrValueRef(KEY_PARAM);
	VS_ClientType client_type(CT_SIMPLE_CLIENT);
	cnt.GetValueI32(CLIENTTYPE_PARAM, client_type);

	if (client_type < CT_SIMPLE_CLIENT || client_type > CT_SDK) {
		dprint3("Error: Unknown CLIENTTYPE_PARAM\n");
		return INVALID_CLIENT_TYPE;
	}
	if (!login || !*login )
		return ACCESS_DENIED;

	bool bkey = !!key;
	ud.m_appName = appName;
	VS_Container prop_cnt;

	auto dbStorage = g_dbStorage;
	{
		result = !dbStorage ? LICENSE_USER_LIMIT : static_cast<VS_UserLoggedin_Result>(dbStorage->LoginUser(login, password, appID, key, "", ud, prop_cnt, client_type));
		if (dbStorage && result == LICENSE_USER_LIMIT && p_licWrap->IsSlave()) {
			UserCtx login_info;
			login_info.emplace(ARRIVED_INFO_TAG, cnt);
			login_info.emplace(SRC_CID_TAG, cid.m_str ? std::string(cid.m_str) : std::string());
			login_info.emplace(SRC_USER_TAG, user ? std::string(user) : std::string());
			login_info.emplace(ADD_STRING_TAG, addString ? std::string(addString) : std::string());
			login_info.emplace(LOGIN_TYPE_TAG, LoginType::SIMPLE_LOGIN);
			ExpandLicenseOnLogin(std::move(login_info), nullptr, ExpandLicenseOnLoginStep::RequestLicense);
			return LICENSE_USER_LIMIT;
		}
		ud.m_protocolVersion = atoi(addString);

		if (bkey && !password && result!=USER_LOGGEDIN_OK && result != RETRY_LOGIN)
			result = SILENT_REJECT_LOGIN;
	}


	if ( m_policy.ShouldBeChecked( client_type ) && GetIPByCID(cid, ip))
		m_policy.SetResult(ip, login, result == USER_LOGGEDIN_OK);


    if (result==USER_LOGGEDIN_OK)
	{
		cnt.Reset();
		while (cnt.Next())
		{
			string_view name = cnt.GetName();
			if (name == ALIAS_PARAM)
			{
				auto val = cnt.GetStrValueRef();
				if (!val || !*val)
					continue;
				ud.m_aliases.Assign(val, 0);
			}
		}
		VS_UserData old_ud;
		auto ep = GetCIDByUID(ud.m_name);
		if (g_storage->FindUser(SimpleStrToStringView(ud.m_name), old_ud) && !ep.empty() && ep != cid.m_str) // Search for logged user on other client on this server
		{
			// Notify Client
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, USERLOGGEDOUT_METHOD);
			rCnt.AddValueI32(RESULT_PARAM, USER_LOGGEDOUT_OK);
			rCnt.AddValueI32(CAUSE_PARAM, USER_LOGGEDIN);
			// Remove relation
			m_OnUserLoggedOut(old_ud, ep);
			PostUnauth(ep.c_str(), rCnt);

			{ // Log to DB
				rCnt.AddValue(APPID_PARAM, old_ud.m_appID);
				rCnt.AddValue(APPNAME_PARAM, old_ud.m_appName);
				rCnt.AddValueI32(TYPE_PARAM, e_change_user);

				std::string payload(ConvertToJsonStr(rCnt, { METHOD_PARAM }));
				VS_Container log_cnt;
				log_cnt.AddValue(OBJECT_TYPE_PARAM, USER_OBJECT_TYPE);
				log_cnt.AddValue(OBJECT_NAME_PARAM, login);
				log_cnt.AddValue(EVENT_TYPE_PARAM, LOGOUT_USER_EVENT_TYPE);
				log_cnt.AddValue(PAYLOAD_PARAM, payload.c_str());
				PostRequest(OurEndpoint(), 0, log_cnt, 0, LOG_SRV);
			}

			dprint3("LOGIN L2: (%s) logged out locally| %s %s\n", ud.m_name.m_str, ep.c_str(), cid.m_str);
		}

		ud.m_homeServer = OurEndpoint();

		if (AuthorizeClient(cid, ud.m_name))
			g_storage->UpdateUser(SimpleStrToStringView(ud.m_name), ud);
		else
			result = ACCESS_DENIED;
	}

	cnt2.AddValueI32(RESULT_PARAM, result);
	if (result == USER_LOGGEDIN_OK) {
		//// TODO: ¬ременный костыль. Ќадо правильно обработать права в клиенте (иначе не показывает видео), потом убрать.
		if (!(ud.m_rights & VS_UserData::UR_COMM_CREATEMULTI))
		{
			ud.m_rights |= VS_UserData::UR_COMM_CREATEMULTI;
			ud.m_tarif_restrictions = 0;
		}
		VS_StorageUserData ude;
		cnt2.AddValue(USERNAME_PARAM, ud.m_name);
		cnt2.AddValue(DISPLAYNAME_PARAM, ud.m_displayName) ;
		cnt2.AddValueI32(RIGHTS_PARAM, ud.m_rights);/////ud.m_rights);
		cnt2.AddValueI32(TYPE_PARAM, ud.m_type);
		cnt2.AddValue(KEY_PARAM, key);
		cnt2.AddValue(TARIFNAME_PARAM, ud.m_tarif_name);
		cnt2.AddValueI32(TARIFRESTR_PARAM, ud.m_tarif_restrictions);

		AddApplicationSettings(ud, prop_cnt);

		AddSessionKey(ud.m_name, cnt2);

		if (prop_cnt.IsValid())
			cnt2.AddValue(PROPERTY_PARAM, prop_cnt);
	} else if (result == RETRY_LOGIN) {
		cnt2.AddValueI32(FIELD1_PARAM, 300000);	// 300sec
	}
	PostUnauth(cid, cnt2);

	if (result == USER_LOGGEDIN_OK) {
		m_OnUserLoggedIn(ud, cid.m_str);
	}

	return result;
}
void VS_VCSAuthService::OnUserLoginEnd_Event(const VS_UserData& ud, const std::string &cid)
{
	{	// offline chat messages
		const char* call_id = ud.m_name;
		const char* server = OurEndpoint();
		std::vector<VS_Container> vec;
		auto dbStorage = g_dbStorage;
		int n = dbStorage->GetOfflineChatMessages(call_id, vec);
		for (int i = 0; i < n; i++)
			PostRequest(server, call_id, vec[i], 0, CHAT_SRV);
	}

	VS_Container cnt;
	std::string call_id;
	if (ud.m_name)
	{
		call_id = ud.m_name;
	}
	m_invites_storage.OnUserLoggedIn(call_id, cnt);
	if (cnt.IsValid())
		PostRequest(OurEndpoint(), 0, cnt, 0, CONFERENCE_SRV);
}

void VS_VCSAuthService::LogoutUser_Method (const vs_user_id& user, OnUserChangeType type)
{
	VS_UserLoggedout_Result result = USER_ALREADY_LOGGEDOUT;

	std::string temp_id;
	if (!!user)
		temp_id = GetCIDByUID(user);
	if (temp_id.empty() && m_recvMess)
		temp_id = std::string(m_recvMess->SrcCID_sv());

	if (!!user && IsAuthorized(user))
	{
		VS_UserData ud;
		g_storage->FindUser(SimpleStrToStringView(user), ud);
		m_OnUserLoggedOut(ud, temp_id);
		result = USER_LOGGEDOUT_OK;
	}
	else
		result = USER_ALREADY_LOGGEDOUT;

	// Make Body
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, USERLOGGEDOUT_METHOD);
	rCnt.AddValueI32(RESULT_PARAM, result);
	rCnt.AddValueI32(CAUSE_PARAM, (USER_LOGGEDOUT_BY_REQUEST));

	PostUnauth(temp_id.c_str(), rCnt);

	{ // Log to DB
		rCnt.AddValueI32(TYPE_PARAM, type);

		std::string payload(ConvertToJsonStr(rCnt, { METHOD_PARAM }));
		VS_Container log_cnt;
		log_cnt.AddValue(OBJECT_TYPE_PARAM, USER_OBJECT_TYPE);
		log_cnt.AddValue(OBJECT_NAME_PARAM, user.m_str);
		log_cnt.AddValue(EVENT_TYPE_PARAM, LOGOUT_USER_EVENT_TYPE);
		log_cnt.AddValue(PAYLOAD_PARAM, payload.c_str());
		PostRequest(OurEndpoint(), 0, log_cnt, 0, LOG_SRV);
	}
}

void VS_VCSAuthService::OnUserLogoff_Event(const VS_UserData& ud, const std::string &cid)
{
	auto dbStorage = g_dbStorage;
	vs_user_id user_id = ud.m_name;

	if (!user_id || !*user_id || !dbStorage)
		return;

	if (ud.m_type == ud.UT_GUEST || ud.m_client_type == CT_TRANSCODER)
		dbStorage->DeleteUser(user_id);

	if (ud.m_client_type!=CT_TRANSCODER)
		dbStorage->SetEndpointProperty(ud.m_appID, VS_SimpleStorage::LOGGEDUSER_TAG, "");

	g_storage->DeleteUser(SimpleStrToStringView(user_id));
	g_conferenceService->RemoveParticipant_Event(user_id, VS_ParticipantDescription::DISCONNECT_LOGOFF);

	UnauthorizeClient(user_id);
	dprint2("Logoff of (%s), cid %s\n", user_id.m_str, cid.c_str());

	dbStorage->LogoutUser(user_id);
	return;
}

void VS_VCSAuthService::CheckUserLoginStatus_Method()
{
	const char * cid = m_recvMess->SrcCID();
	long result = NO_USER_LOGGEDIN;

	if (cid && *cid) {
		// TODO: Check for too much logins
	} else {
		VS_UserData ud;
		if (g_storage->FindUser(m_recvMess->SrcUser_sv(), ud)) {
			result = USER_ALREADY_LOGGEDIN;
		}
		else {
			dprint0("Login Sync error in endpoints and g_storage!\n");
		}
	}

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, USERLOGGEDIN_METHOD);
	rCnt.AddValueI32(RESULT_PARAM, result);
	PostReply(rCnt);

	if(result==NO_USER_LOGGEDIN)
		Authorize_Method(cid,0);
}

void VS_VCSAuthService::OnPointConnected_Method(const VS_Container & cnt)
{
	auto type = VS_PointParams::PT_UNKNOWN;
	if (cnt.GetValueI32(TYPE_PARAM, type) && type == VS_PointParams::PT_SERVER) {
		lic::ShareService::OnPointConnected_Method(cnt);
	}
}

void VS_VCSAuthService::OnPointDisconnected_Method(const VS_Container & cnt)
{
	auto type = VS_PointParams::PT_UNKNOWN;
	cnt.GetValueI32(TYPE_PARAM, type);

	if (type == VS_PointParams::PT_CLIENT) {
		int32_t reason(0);
		cnt.GetValue(DISCONNECT_REASON_PARAM, reason);
		auto uid = cnt.GetStrValueRef(USERNAME_PARAM);
		auto cid = cnt.GetStrValueRef(NAME_PARAM);
		if (!cid || !uid || !*uid) return;
		dprint1("cid Disconnect: %s, uid=%s, r= %2d\n", cid, uid, reason);

		VS_UserData ud;
		if (g_storage->FindUser(cnt.GetStrValueRef(USERNAME_PARAM), ud))
			m_OnUserLoggedOut(ud, cnt.GetStrValueRef(NAME_PARAM));
	}
	else if (type == VS_PointParams::PT_SERVER) {
		lic::ShareService::OnPointDisconnected_Method(cnt);
	}
}


void VS_VCSAuthService::Authorize_Method(const char *cid, VS_Container *in_cnt)
{
	auto dbStorage = g_dbStorage;
	if(!dbStorage->IsAutoAuthAvailable())
		return;

	VS_ClientType client_type(CT_SIMPLE_CLIENT);
	if (in_cnt)
		in_cnt->GetValueI32(CLIENTTYPE_PARAM, client_type);

	if (client_type < CT_SIMPLE_CLIENT || client_type > CT_TRANSCODER_CLIENT)
	{
		dprint3("Authorize Error: Unknown CLIENTTYPE_PARAM\n");
		return;
	}

	VS_Container out_cnt;
	VS_UserData ud;
	bool request;
	if(!dbStorage->Authorize(cid, in_cnt, out_cnt, request, ud, client_type)) {
		dprint1("Authorization failed from %s\n", cid);
		return;
	}
	if(request) {
		out_cnt.AddValue(METHOD_PARAM,AUTHORIZE_METHOD);
		PostUnauth(cid, out_cnt);
	}
	else {
		// todo(kt): get from storage autoKey, prop_cnt, and pass to func
		VS_SimpleStr fake_autoKey;
		VS_Container fake_prop_cnt;
		//////////

		auto pAddStr = m_recvMess->AddString();
		AuthorizedLogin(cid,ud,fake_autoKey,fake_prop_cnt,client_type, pAddStr? pAddStr : "");
	}
}

VS_UserLoggedin_Result VS_VCSAuthService::AuthorizedLogin(const char *cid, VS_UserData &ud, const VS_SimpleStr& autoKey, VS_Container& prop_cnt, const VS_ClientType client_type, const std::string& addString)
{
	VS_UserLoggedin_Result result(USER_LOGGEDIN_OK);
	ud.m_protocolVersion = strtoul(addString.c_str(), nullptr, 10);

	if (ud.m_type != VS_UserData::UT_GUEST && !VS_CheckLicense(LE_LOGIN))
		result = LICENSE_USER_LIMIT;
	else if ( (CT_GATEWAY == client_type || CT_TRANSCODER == client_type || CT_TRANSCODER_CLIENT == client_type) && !VS_CheckLicense(LE_GATEWAYLOGIN))
		result = LICENSE_USER_LIMIT;

	if (result == LICENSE_USER_LIMIT && p_licWrap->IsSlave()) {
		UserCtx login_info;
		login_info.emplace(SRC_CID_TAG, cid ? std::string(cid) : std::string());
		login_info.emplace(USER_DATA_TAG, ud);
		login_info.emplace(KEY_PARAM, autoKey.m_str ? std::string(autoKey.m_str) : std::string());
		login_info.emplace(PROPERTY_PARAM, prop_cnt);
		login_info.emplace(CLIENTTYPE_PARAM, client_type);
		login_info.emplace(LOGIN_TYPE_TAG, LoginType::AUTHORIZED_LOGIN);
		login_info.emplace(ADD_STRING_TAG, addString);

		ExpandLicenseOnLogin(std::move(login_info), nullptr, ExpandLicenseOnLoginStep::RequestLicense);
		return result;
	}

	if(USER_LOGGEDIN_OK==result)
	{
		const auto ep = GetCIDByUID(ud.m_name);
		if (!ep.empty() && ep != cid)
		{
			VS_UserData old_ud;
			g_storage->FindUser(SimpleStrToStringView(ud.m_name), old_ud);

			// Notify Client
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, USERLOGGEDOUT_METHOD);
			rCnt.AddValueI32(RESULT_PARAM, USER_LOGGEDOUT_OK);
			rCnt.AddValueI32(CAUSE_PARAM, USER_LOGGEDIN);
			// Remove relation
			m_OnUserLoggedOut(ud, ep);
			PostUnauth(ep.c_str(), rCnt);

			{ // Log to DB
				rCnt.AddValue(APPID_PARAM, old_ud.m_appID);
				rCnt.AddValue(APPNAME_PARAM, old_ud.m_appName);
				rCnt.AddValueI32(TYPE_PARAM, e_change_user);

				std::string payload(ConvertToJsonStr(rCnt, { METHOD_PARAM }));
				VS_Container log_cnt;
				log_cnt.AddValue(OBJECT_TYPE_PARAM, USER_OBJECT_TYPE);
				log_cnt.AddValue(OBJECT_NAME_PARAM, ud.m_name);
				log_cnt.AddValue(EVENT_TYPE_PARAM, LOGOUT_USER_EVENT_TYPE);
				log_cnt.AddValue(PAYLOAD_PARAM, payload.c_str());
				PostRequest(OurEndpoint(), 0, log_cnt, 0, LOG_SRV);
			}

			dprint3("AUTH L2: (%s) logged out locally| %s %s\n", ud.m_name.m_str, ep.c_str(), cid);
		}

		ud.m_homeServer = OurEndpoint();

		if (AuthorizeClient(cid, ud.m_name))
			g_storage->UpdateUser(SimpleStrToStringView(ud.m_name), ud);
		else
			result = ACCESS_DENIED;
	}
	VS_Container	cnt2;
	cnt2.AddValue(METHOD_PARAM, USERLOGGEDIN_METHOD);
	cnt2.AddValueI32(RESULT_PARAM, result);
	if (result == USER_LOGGEDIN_OK) {
		cnt2.AddValue(USERNAME_PARAM, ud.m_name);
		cnt2.AddValue(DISPLAYNAME_PARAM, ud.m_displayName) ;
		cnt2.AddValueI32(RIGHTS_PARAM, ud.m_rights);
		cnt2.AddValueI32(TYPE_PARAM, ud.m_type);
		cnt2.AddValue(KEY_PARAM, autoKey);
		cnt2.AddValue(TARIFNAME_PARAM, ud.m_tarif_name);
		cnt2.AddValueI32(TARIFRESTR_PARAM, ud.m_tarif_restrictions);

		AddApplicationSettings(ud, prop_cnt);

		AddSessionKey(ud.m_name, cnt2);

		if (prop_cnt.IsValid())
			cnt2.AddValue(PROPERTY_PARAM, prop_cnt);
	}
	PostUnauth(cid, cnt2);

	if (result == USER_LOGGEDIN_OK)
		m_OnUserLoggedIn(ud, cid);

	{ // Log to DB
		VS_Container pcnt;
		pcnt.AddValueI32(RESULT_PARAM, result);
		const char* luser = "(null)";
		if (result == USER_LOGGEDIN_OK) {
			luser = ud.m_name.m_str;
			pcnt.AddValue(USERNAME_PARAM, ud.m_name);
			pcnt.AddValue(APPID_PARAM, ud.m_appID);
			pcnt.AddValue(APPNAME_PARAM, ud.m_appName);
			pcnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
			char rights[33]; rights[32] = 0;
			uint32_t ur = (uint32_t)ud.m_rights;
			for (int i = 0; i < 32; i++)
			{
				rights[i] = ur & 0x80000000 ? '1' : '0';
				ur <<= 1;
			}
			pcnt.AddValue(RIGHTS_PARAM, rights);
		}

		std::string ip;
		if (GetIPByCID(cid, ip))
			pcnt.AddValue(HOST_PARAM, ip);

		std::string payload(ConvertToJsonStr(pcnt));
		VS_Container log_cnt;
		log_cnt.AddValue(OBJECT_TYPE_PARAM, USER_OBJECT_TYPE);
		log_cnt.AddValue(OBJECT_NAME_PARAM, ud.m_name);
		log_cnt.AddValue(EVENT_TYPE_PARAM, AUTHORIZE_USER_EVENT_TYPE);
		log_cnt.AddValue(PAYLOAD_PARAM, payload.c_str());
		PostRequest(OurEndpoint(), 0, log_cnt, 0, LOG_SRV);
	}
	return result;
}

bool VS_VCSAuthService::OnPointConnected_Event(const VS_PointParams * prm)
{
	if (!prm) return true;

	if (prm->reazon <= 0) {
		dprint1("Error {%d} while Connect to (%s)\n", prm->reazon, prm->uid);
		return true;
	}


	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, POINTCONNECTED_METHOD);
	cnt.AddValue(USERNAME_PARAM, prm->uid);
	cnt.AddValue(NAME_PARAM, prm->cid);
	cnt.AddValue(TYPE_PARAM, prm->type);

	PostRequest(OurEndpoint(), 0, cnt);
	return true;
}

bool VS_VCSAuthService::OnPointDisconnected_Event(const VS_PointParams* prm)
{
	if (!prm) return true;

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, POINTDISCONNECTED_METHOD);
	cnt.AddValue(USERNAME_PARAM, prm->uid);
	cnt.AddValue(NAME_PARAM, prm->cid);
	cnt.AddValue(TYPE_PARAM, prm->type);
	cnt.AddValue(DISCONNECT_REASON_PARAM, prm->reazon);

	PostRequest(OurEndpoint(), 0, cnt);
	return true;

}

void VS_VCSAuthService::TrySendInvites()
{
	std::vector<BSEvent> v;
	m_invites_storage.OnTimer_SendInvites(v);
	for(unsigned int i=0; i < v.size(); i++)
	{
		BSEvent ev = v[i];
		if (!ev.cnt)
			continue;

		if (ev.cnt->IsValid())
			PostRequest(OurEndpoint(), 0, *(ev.cnt), 0, ev.to_service);

		if (ev.cnt) { delete ev.cnt; ev.cnt = 0; }
	}
}

bool VS_VCSAuthService::Timer( unsigned long ticks)
{
	m_policy.Timeout();

	auto dbStorage = g_dbStorage;
	if (!g_storage || !dbStorage)
		return true;

	dbStorage->UpdateAliasList();

	TrySendInvites();

	lic::ShareService::Timer(ticks);

	if (dbStorage)
		dbStorage->Timer(ticks, this);

	return true;
}

void VS_VCSAuthService::OnUserChange_Method(const char* user_, const OnUserChangeType type, const char* pass)
{
	dprint3("user=%s, type=%d, sess=%s\n", user_, type, pass);
	auto dbStorage = g_dbStorage;

	if(!dbStorage->CheckSessionID(pass))
		return;

	auto UpdateUsersGroups = [this, self_weak = weak_from_this()](void){
		auto lock = self_weak.lock();
		if (!lock)
			return;

		auto dbStorage = g_dbStorage;
		if (!dbStorage)
			return;

		auto is_stopping = [this]() { return m_is_stopping.load(); };
		dbStorage->UpdateUsersGroups(is_stopping);
		if (is_stopping())
			return;

		vs_user_id* users = 0;
		auto NumOfUsers = g_storage->GetUsers(users);
		VS_SCOPE_EXIT{ delete[] users; };
		for (decltype(NumOfUsers) i = 0; i < NumOfUsers; i++)
		{
			const char* to_user = users[i];
			if (!to_user || VS_IsNotTrueConfCallID(to_user))
				continue;

			// Update UserRights
			VS_UserData ud;
			if (dbStorage->FindUser(to_user, ud))
			{
				if (!(ud.m_rights & VS_UserData::UR_COMM_CREATEMULTI))
				{
					ud.m_rights |= VS_UserData::UR_COMM_CREATEMULTI;
					ud.m_tarif_restrictions = 0;
				}

				VS_Container rCnt;
				rCnt.AddValue(METHOD_PARAM, UPDATEACCOUNT_METHOD);
				rCnt.AddValueI32(RIGHTS_PARAM, ud.m_rights);
				rCnt.AddValue(TARIFNAME_PARAM, ud.m_tarif_name);
				rCnt.AddValueI32(TARIFRESTR_PARAM, ud.m_tarif_restrictions);
				rCnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
				VS_Container prop_cnt;
				AddApplicationSettings(ud, prop_cnt);
				if (prop_cnt.IsValid())
					rCnt.AddValue(PROPERTY_PARAM, prop_cnt);
				PostRequest(OurEndpoint(), ud.m_name, rCnt, 0, AUTH_SRV);

				{// bug#3569: for check of UR_COMM_CALL at VS_ConferenceService
					VS_UserData ud2;
					if (g_storage->FindUser(SimpleStrToStringView(ud.m_name), ud2))
					{
						ud2.m_displayName = ud.m_displayName;
						ud2.m_rights = ud.m_rights;
						ud2.m_tarif_restrictions = ud.m_tarif_restrictions;
						g_storage->UpdateUser(SimpleStrToStringView(ud.m_name), ud2);
					}
				}
			}
		}

		// send all online users
		VS_Container update_ab_cnt;
		update_ab_cnt.AddValue(METHOD_PARAM, ONADDRESSBOOKCHANGE_METHOD);
		for (unsigned int i = 0; i < NumOfUsers; i++)
		{
			const char* to_user = users[i];
			if (!to_user || VS_IsNotTrueConfCallID(to_user))
				continue;
			update_ab_cnt.AddValue(USERNAME_PARAM, to_user);
		}
		update_ab_cnt.AddValueI32(ADDRESSBOOK_PARAM, AB_COMMON);
		PostRequest(OurEndpoint(), 0, update_ab_cnt, 0, ADDRESSBOOK_SRV);

		int32_t* ab = update_ab_cnt.GetLongValueRef(ADDRESSBOOK_PARAM);
		*ab = AB_GROUPS;
		PostRequest(OurEndpoint(), 0, update_ab_cnt, 0, ADDRESSBOOK_SRV);

		*ab = AB_PHONES;
		PostRequest(OurEndpoint(), 0, update_ab_cnt, 0, ADDRESSBOOK_SRV);
	};
	m_strand.post(UpdateUsersGroups);

	if (!user_ || !pass || !(*user_)) return;

	if (type==e_remove_user_from_all_confs)	// remove user from all confs
	{
		g_conferenceService->RemoveParticipant_Event(user_, VS_ParticipantDescription::DISCONNECT_LOGOFF);
		return ;
	}

	if(!dbStorage)
		return;

	VS_RealUserLogin r(vs::UTF8ToLower(user_));
	VS_UserData ud;

	if (type==e_logout_user)	// just LogoutUser
	{
		LogoutUser_Method((const char*)r, e_logout_user);
		return ;
	}

	if (type==e_delete_user)
		dbStorage->FindUser((const char*) r, ud);

	dbStorage->OnUserChange(r, type, pass);

	if (type==e_add_user || type==e_change_user)
		dbStorage->FindUser((const char*) r, ud);

	bool need_logout = false;

	if (type == e_change_user)
	{
		if (!(ud.m_rights & VS_UserData::UR_COMM_CREATEMULTI))
		{
			ud.m_rights |= VS_UserData::UR_COMM_CREATEMULTI;
			ud.m_tarif_restrictions = 0;
		}

		{// bug#3569: for check of UR_COMM_CALL at VS_ConferenceService
			VS_UserData ud2;
			if (g_storage->FindUser(SimpleStrToStringView(ud.m_name), ud2))
			{
				ud2.m_displayName = ud.m_displayName;
				ud2.m_rights = ud.m_rights;
				ud2.m_tarif_restrictions = ud.m_tarif_restrictions;
				g_storage->UpdateUser(SimpleStrToStringView(ud.m_name), ud2);
			}
		}

		if (dbStorage->IsDisabledUser(ud.m_name))
			need_logout = true;

		// send info about user
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, UPDATEACCOUNT_METHOD);
		rCnt.AddValueI32(RIGHTS_PARAM, ud.m_rights);
		rCnt.AddValue(TARIFNAME_PARAM, ud.m_tarif_name);
		rCnt.AddValueI32(TARIFRESTR_PARAM, ud.m_tarif_restrictions);
		rCnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);

		VS_Container prop_cnt;
		AddApplicationSettings(ud, prop_cnt);
		if (prop_cnt.IsValid())
			rCnt.AddValue(PROPERTY_PARAM, prop_cnt);

		PostRequest(OurEndpoint(), ud.m_name, rCnt, 0, AUTH_SRV);
	}

	if ( type == e_delete_user || need_logout )
	{
		LogoutUser_Method((const char*)r, e_delete_user);
	}
	if(type == e_add_user || type == e_delete_user)
	{
		VS_Container presence_cnt;
		presence_cnt.AddValue(METHOD_PARAM,RESENDSTATUS_METHOD);
		presence_cnt.AddValue(CALLID_PARAM,user_);
		PostRequest(OurEndpoint(), 0, presence_cnt, 0, PRESENCE_SRV);
	}

	m_strand.post(UpdateUsersGroups);
}

void VS_VCSAuthService::OnAddressBookChange_Method( VS_Container& cnt )
{
	auto dbStorage = g_dbStorage;
	if(!dbStorage->CheckSessionID( cnt.GetStrValueRef(SESSION_PARAM)))
		return;

	const char *call_id = cnt.GetStrValueRef(USERNAME_PARAM);

	//if no user specified add all online users
	if(!call_id || !*call_id)
	{
		vs_user_id* users = 0;
		auto NumOfUsers = g_storage->GetUsers(users); // delete after use
		for(decltype(NumOfUsers) i = 0; i < NumOfUsers; i++)
			cnt.AddValue(USERNAME_PARAM, users[i].m_str);

		delete [] users;
	}

	PostRequest(OurEndpoint(), NULL, cnt, 0, ADDRESSBOOK_SRV);
}

void VS_VCSAuthService::ReqUpdateAccount_Method(VS_Container& cnt)
{
	const char *src_user = m_recvMess->SrcUser();
	const char* call_id = cnt.GetStrValueRef(CALLID_PARAM);

	if (!call_id || !*call_id || !src_user || !*src_user)
		return;

	if (strcasecmp(src_user, call_id) != 0)
		return;

	VS_UserData ud;
	if (!g_storage->FindUser(call_id, ud)) // only for local users!
		return;

	int32_t type(RUAT_SESSION);
	cnt.GetValue(TYPE_PARAM, type);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, UPDATEACCOUNT_METHOD);

	if (type == RUAT_SESSION) {
		AddSessionKey(call_id, rCnt);
		dprint4("ReqUpdateAccount: UpdateSession: %s\n", call_id);
	}
	else if (type == RUAT_PASSWORD) {
		int result(-1);

		const char* old_password = cnt.GetStrValueRef("OldPassword");
		const char* new_password = cnt.GetStrValueRef("NewPassword");
		VS_SimpleStr from_app_id = cnt.GetStrValueRef(APPID_PARAM);

		if (old_password && *old_password && new_password && *new_password)
			result = g_dbStorage->ChangePassword(call_id, old_password, new_password, from_app_id);

		rCnt.AddValue(TYPE_PARAM, type);
		rCnt.AddValue(CALLID_PARAM, call_id);
		rCnt.AddValueI32(RESULT_PARAM, result);

		dprint4("ReqUpdateAccount: ChangePassword: %s, old=%s, new=%s, result=%d\n", call_id, old_password? old_password:"", new_password? new_password:"", result);
	}
	else if (type == RUAT_PROFILE) {
		VS_Container cnt2;
		cnt2.AddValue(DISPLAYNAME_PARAM, cnt.GetStrValueRef(DISPLAYNAME_PARAM));
		cnt2.AddValue(FIRSTNAME_PARAM, cnt.GetStrValueRef(FIRSTNAME_PARAM));
		cnt2.AddValue(LASTNAME_PARAM, cnt.GetStrValueRef(LASTNAME_PARAM));
		cnt2.AddValue(USERCOMPANY_PARAM, cnt.GetStrValueRef(USERCOMPANY_PARAM));

		int result = g_dbStorage->UpdatePerson(call_id, cnt2);

		rCnt.AddValue(TYPE_PARAM, type);
		rCnt.AddValue(CALLID_PARAM, call_id);
		rCnt.AddValueI32(RESULT_PARAM, result);

		dprint4("ReqUpdateAccount: UpdatePerson: %s, result=%d\n", call_id, result);
	}

	PostRequest(OurEndpoint(), call_id, rCnt);
}

void VS_VCSAuthService::AutoInvite_Method(VS_Container &cnt)
{
	const char *conf_name = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char *user_id = cnt.GetStrValueRef(CALLID_PARAM);

	m_invites_storage.ResetAutoInvites(conf_name, user_id);
	TrySendInvites();
}

void VS_VCSAuthService::SetRegID_Method(VS_Container & cnt)
{
	string_view src_user = m_recvMess->SrcUser_sv();
	if (src_user.empty())
		src_user = m_recvMess->SrcCID_sv();
	auto call_id = cnt.GetStrValueView(CALLID_PARAM);
	auto reg_id = cnt.GetStrValueView(REGID_PARAM);
	VS_RegID_Register_Type type = REGID_INVALID;
	cnt.GetValueI32(SUBTYPE_PARAM, type);
	long result = g_dbStorage->SetRegID(call_id.c_str(), reg_id.c_str(), type);
	dstream3 << "SetRegID: from " << src_user << ", call_id=" << call_id << ", reg_id=" << reg_id << ", res=" << result << "\n";
}

void VS_VCSAuthService::InviteUsers_Method(VS_Container & cnt)
{
	m_invites_storage.OnInviteUsersReply(cnt);
}

#ifdef _SVKS_M_BUILD_
void VS_VCSAuthService::AuthByECP_Method(const char *cid, VS_Container& cnt)
{
	if (!cid || !*cid)
		return ;
	VS_ClientType client_type(CT_SIMPLE_CLIENT);
	cnt.GetValue(CLIENTTYPE_PARAM,(long&)client_type);
	if (client_type!=CT_SIMPLE_CLIENT && client_type!=CT_MOBILE && client_type != CT_GATEWAY && client_type != CT_TERMINAL)
	{
		dprint3("AuthByECP: Error empty CLIENTTYPE_PARAM\n");
		return;
	}

	auto dbStorage = g_dbStorage;
	if (!dbStorage)
		return ;

	VS_Container rCnt;
	VS_SimpleStr autoKey;
	VS_Container prop_cnt;
	VS_UserData ud;
	bool finished = dbStorage->AuthByECP(cnt, rCnt, ud, autoKey, prop_cnt, client_type);
	if (!finished)
	{
		if (!rCnt.IsEmpty())
			PostReply(rCnt);
		return ;
	}

	AuthorizedLogin(cid, ud, autoKey, prop_cnt, client_type, m_recvMess->AddString());
}
#endif	// _SVKS_M_BUILD_