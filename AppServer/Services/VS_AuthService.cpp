#ifdef _WIN32
#include <stdio.h>
#include "VS_AuthService.h"
#include "../../ServerServices/Common.h"
#include "VS_ConferenceService.h"
#include "VS_ChatService.h"
#include "std/cpplib/md5.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "../../common/std/cpplib/VS_UserData.h"
#include "../../common/std/cpplib/VS_IntConv.h"
#include "../../common/std/cpplib/VS_Utils.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "../../common/TrueGateway/VS_GatewayStarter.h"
#include "../../common/TrueGateway/clientcontrols/VS_TranscoderLogin.h"

#define DEBUG_CURRENT_MODULE VS_DM_LOGINS

bool VS_AuthService::Init( const char* /*our_endpoint*/, const char* /*our_service*/, const bool /*permittedAll*/ )
{
	char buff[256] = {0};
	VS_RegistryKey cfg(false, CONFIGURATION_KEY, false, true);
	if (cfg.GetValue(buff, 256, VS_REG_STRING_VT, "Debug HomeBS") > 0 && *buff)
		m_debugHomeBS = buff;
	if (cfg.GetValue(buff, 256, VS_REG_STRING_VT, "DefaultDomain") > 0 && *buff)
		m_defaultDomain = buff;

	m_OnUserLoggedIn.connect( boost::bind(&VS_AuthService::OnUserLoginEnd_Event, this, _1, _2) );
	m_OnUserLoggedOut.connect( boost::bind(&VS_AuthService::OnUserLogoff_Event, this, _1, _2) );

	return true;
}

bool VS_AuthService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)	return true;

	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		{
			m_recvMess = recvMess.get();

			VS_Container cnt;
			if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
				const char* method = 0;
				if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0) {
					dprint3("Processing %20s; cid:%s user:%s srv:%s \n", method, recvMess->SrcCID(), recvMess->SrcUser(), recvMess->SrcServer());
					// Process methods
					if (strcasecmp(method, LOGINUSER_METHOD) == 0) {
						LoginUser_Method(cnt);
					} else if (strcasecmp(method, LOGINUSER2_METHOD) == 0) {
						LoginUser2_Method(cnt);
					} else if (strcasecmp(method, USERLOGGEDIN_METHOD) == 0) {
						UserLoggedIn_Method(cnt);
					} else if (strcasecmp(method, LOGOUTUSER_METHOD) == 0) {
						vs_user_id user;
						const char* uplink = 0;
						if (recvMess->IsFromServer() && ST_BS == VS_GetServerType(recvMess->SrcServer_sv()))
						{
							user = cnt.GetStrValueRef(USERNAME_PARAM);
							uplink = recvMess->SrcServer();
						}
						else if (strcmp(recvMess->SrcServer(), OurEndpoint())==0 ){
							user=m_recvMess->SrcUser();
						}
						LogoutUser_Method(user, uplink);
					} else if (strcasecmp(method, CHECKUSERLOGINSTATUS_METHOD) == 0) {
						CheckUserLoginStatus_Method();
					} else if (strcasecmp(method, POINTDISCONNECTED_METHOD) == 0)
					{
						VS_UserData ud;
						if (g_storage->FindUser(cnt.GetStrValueView(USERNAME_PARAM), ud))
							m_OnUserLoggedOut(ud, cnt.GetStrValueRef(NAME_PARAM));
					} else if (strcasecmp(method, UPDATEACCOUNT_METHOD) == 0) {
						UpdateAccount_Method(cnt);
					} else if (strcasecmp(method, REQUPDATEACCOUNT_METHOD) == 0) {
						ReqUpdateAccount_Method(cnt);
					} else if (strcasecmp(method, ONUSERCHANGE_METHOD) == 0) {
						OnUserChange_Method(cnt);
					} else if (strcasecmp(method, SETREGID_METHOD) == 0) {
						SetRegID_Method(cnt);
					} else if (strcasecmp(method, UPDATE_PEERCFG_METHOD) == 0) {
						UpdatePeerCfg_Method(cnt);
					}
				}
			}
		}
		break;
	case transport::MessageType::Notify:
		break;
	}
	m_recvMess = nullptr;
	return true;
}

void VS_AuthService::SetTransLogin(const std::weak_ptr<VS_TranscoderLogin>& transLogin)
{
	assert(transLogin.lock() != nullptr);
	m_transLogin = transLogin;
}

////////////////////////////////////////////////////////////////////////////////
// LOGINUSER_METHOD()
////////////////////////////////////////////////////////////////////////////////
void VS_AuthService::LoginUser_Method(VS_Container& cnt)
{
	VS_SimpleStr cid = m_recvMess->SrcCID();
	if (!cid) {
		const char* user = m_recvMess->SrcUser();
		std::string temp_id;
		if (user)
			temp_id = GetCIDByUID(user);
		if (temp_id.empty())
		{
			dprint0("EMPTY Ep in Login\n");
			return;
		}
		cid = temp_id.c_str();
	}

	//login timecheck here
	if (m_avetime > LoginBarrier) {
		VS_Container rCnt;
		long result(RETRY_LOGIN);
		long timeout(20000 + m_avetime); // login timeout + current loadtime
		rCnt.AddValue(METHOD_PARAM, USERLOGGEDIN_METHOD);
		rCnt.AddValueI32(RESULT_PARAM, result);
		rCnt.AddValueI32(FIELD1_PARAM, timeout);
		PostUnauth(cid, rCnt);
		dprint3("BS load detected. Login %ld, Time %ld\n", m_login_count, m_avetime);
	}

	VS_TempUserData udt;
	char* login = (char*)cnt.GetStrValueRef(LOGIN_PARAM);
	if(!login)
		return;
	// TODO: remove it
	// fix video-port.com
	{
		char * p = strchr(login, '@');
		if (p && strcasecmp(p+1, "video-port.com")==0)
			*p = 0;
	}
	// fill data
	udt.m_login = login;
	udt.m_password = cnt.GetStrValueRef(PASSWORD_PARAM);
	udt.m_key = cnt.GetStrValueRef(KEY_PARAM);
	udt.m_appID = cnt.GetStrValueRef(HASH_PARAM);
	udt.m_protocolVersion = atoi(m_recvMess->AddString());
	udt.m_defaultDomain = cnt.GetStrValueRef(USER_DEFAULT_DOMAIN);
	udt.m_appName = cnt.GetStrValueRef(APPNAME_PARAM);
	cnt.GetValueI32(CLIENTTYPE_PARAM, udt.m_client_type);
	// check data
	if (!udt.m_login || !udt.m_appID || (!udt.m_password && !udt.m_key)) {
		dprint0("Empty login parameters: %d\n", !udt.m_login + 2*!udt.m_appID + 4*!udt.m_password +8*!udt.m_key);
		return;
	}
	// fill seq
	MD5 md5;
	md5.Update(SimpleStrToStringView(udt.m_login));
	md5.Update(SimpleStrToStringView(udt.m_password));
	md5.Update(SimpleStrToStringView(udt.m_key));
	md5.Update(SimpleStrToStringView(udt.m_appID));
	md5.Final();
	udt.m_seq.Resize(100);
	md5.GetString(udt.m_seq.m_str);

	// decide where do login
	VS_SimpleStr BS;
	VS_TempUserData *cache = 0;
	VS_TempUserDataMap::ConstIterator ui= m_loginData[udt.m_seq];
	if (!!ui)
		cache = ui->data;
	unsigned long uuid = GetTickCount();

	VS_SimpleStr domain;
	GetUserDomain(udt.m_login,domain); // domain from login, if exists

	bool isBS = g_appServer->GetBSByDomain(domain, BS) || g_appServer->GetBSByDomain(udt.m_defaultDomain, BS) || g_appServer->GetBSByDomain(m_defaultDomain.c_str(), BS);

	if (udt.m_client_type==CT_TRANSCODER)
	{
		// CheckTranscoderPassword
		auto transcoder_login = m_transLogin.lock();
		if (transcoder_login.get() == nullptr || !transcoder_login->Login(string_view{ udt.m_login.m_str, (size_t)udt.m_login.Length() }, string_view{ udt.m_password.m_str, (size_t)udt.m_password.Length() }))
		{
			VS_Container rCnt;
			rCnt.AddValue(METHOD_PARAM, LOGINUSER2_METHOD);
			rCnt.AddValueI32(RESULT_PARAM, ACCESS_DENIED);
			LoginUser2_Method(rCnt);
			return;
		}

		m_loginTime.push_back(uuid);
		m_loginData.Assign(udt.m_seq, udt);

		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, LOGINUSER2_METHOD);
		rCnt.AddValueI32(RESULT_PARAM, USER_LOGGEDIN_OK);
		rCnt.AddValue(SEQUENCE_PARAM, udt.m_seq);
		rCnt.AddValueI32(CLIENTTYPE_PARAM, udt.m_client_type);
		rCnt.AddValueI32(PROTOCOLVERSION_PARAM, udt.m_protocolVersion);
		rCnt.AddValue(ENDPOINT_PARAM, cid);

		//user data
		rCnt.AddValue(USERNAME_PARAM, udt.m_login);
		rCnt.AddValue(DISPLAYNAME_PARAM, udt.m_displayName);
		rCnt.AddValueI32(TYPE_PARAM, udt.m_type);
		rCnt.AddValueI32(RIGHTS_PARAM,		VS_UserData::UR_LOGIN|
											VS_UserData::UR_COMM_MULTI|
											VS_UserData::UR_COMM_PASSWORDMULTI|
											VS_UserData::UR_COMM_SEARCHEXISTS|
											VS_UserData::UR_COMM_BROADCAST|
											VS_UserData::UR_COMM_CALL|
											VS_UserData::UR_COMM_HDVIDEO);
		LoginUser2_Method(rCnt);
		return;
	}

	if (cache && (!isBS || uuid-cache->m_lTime <30000) ) {
		dprint1("LOCAL Login for (%s) [cid:%s]\n", udt.m_login.m_str, cid.m_str);

		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, LOGINUSER2_METHOD);
		rCnt.AddValueI32(RESULT_PARAM, USER_LOGGEDIN_OK);
		rCnt.AddValue(SEQUENCE_PARAM, cache->m_seq);
		rCnt.AddValueI32(CLIENTTYPE_PARAM, cache->m_client_type);
		rCnt.AddValueI32(PROTOCOLVERSION_PARAM, cache->m_protocolVersion);
		rCnt.AddValue(ENDPOINT_PARAM, cid);

		//user data
		rCnt.AddValue(USERNAME_PARAM, cache->m_name);
		rCnt.AddValue(DISPLAYNAME_PARAM, cache->m_displayName);
		rCnt.AddValueI32(TYPE_PARAM, cache->m_type);
		rCnt.AddValueI32(RIGHTS_PARAM, cache->m_rights);
		rCnt.AddValueI32(RATING_PARAM, cache->m_rating);
		rCnt.AddValueI32(SEPARATIONGROUP_PARAM, cache->m_SeparationGroup);
		rCnt.AddValue(TARIFNAME_PARAM, cache->m_tarif_name);
		rCnt.AddValueI32(TARIFRESTR_PARAM, cache->m_tarif_restrictions);
		rCnt.AddValue(SESSION_PARAM, cache->m_login_session_key);
		rCnt.AddValue(SERVER_PARAM, cache->m_homeServer);
		//autologin data
		rCnt.AddValue(KEY_PARAM, cache->m_key);

		for (VS_StrI_IntMap::ConstIterator i = cache->m_aliases.Begin(); !!i; ++i)
			rCnt.AddValue(ALIAS_PARAM, i->key);

		if (isBS) {
			// renew session
			VS_Container scnt;
			scnt.AddValue(METHOD_PARAM, REQUPDATEACCOUNT_METHOD);
			scnt.AddValue(CALLID_PARAM, cache->m_name);
			PostRequest(cache->m_homeServer, 0, scnt);
		}

		LoginUser2_Method(rCnt);
	}
	else if (isBS) {
		// go to BS
		std::string ip;
		if (m_policy.ShouldBeChecked(udt.m_client_type) && GetIPByCID(cid, ip) )
		{
			 if (!m_policy.Request(ip, udt.m_login.m_str))
			 {
				 VS_UserLoggedin_Result r = m_policy.FailResult();
				 if (r == SILENT_REJECT_LOGIN) return;

				 VS_Container cnt2;
				 cnt2.AddValue(METHOD_PARAM, USERLOGGEDIN_METHOD);
				 cnt2.AddValueI32(RESULT_PARAM, r);
				 PostUnauth(cid, cnt2);
				 return;
			 }
		}

		cnt.AddValue(APPNAME_PARAM, udt.m_appName);
		cnt.AddValue(ENDPOINT_PARAM, cid);
		cnt.AddValue(SEQUENCE_PARAM, udt.m_seq);
		cnt.AddValueI32(CLIENTTYPE_PARAM, udt.m_client_type);
		cnt.AddValueI32(PROTOCOLVERSION_PARAM, udt.m_protocolVersion);
		// login stat section
		cnt.AddValueI32(SEQUENCE_ID_PARAM, uuid);
		if (!m_debugHomeBS)
			PostRequest(BS, 0, cnt, AUTH_SRV, LOCATE_SRV);
		else
			PostRequest(m_debugHomeBS, 0, cnt, 0, AUTH_SRV);

		m_loginTime.push_back(uuid);
		m_loginData.Assign(udt.m_seq, udt);
	}
}

void VS_AuthService::GetUserDomain(const VS_SimpleStr &call_id, VS_SimpleStr &domain)
{
	char * p = strchr(call_id.m_str,'@');
	if (p) {
		p++;
		while(strchr(p,'@')) {
			p = strchr(p,'@');
			p++;
		}
		domain = p;
	}
}

void VS_AuthService::LoginUser2_Method(VS_Container& cnt)
{
	const char* seq = cnt.GetStrValueRef(SEQUENCE_PARAM);
	const char* cid = cnt.GetStrValueRef(ENDPOINT_PARAM);
	eLoginStrategy login_strategy(eLoginStrategy::LS_DEFAULT);
	cnt.GetValueI32(LOGINSTRATEGY_PARAM, login_strategy);
	if (!cid || !seq)
		return;

	VS_TempUserData udt;
	VS_TempUserDataMap::ConstIterator ui= m_loginData[seq];
	if (!!ui)
		udt=ui;
	else {
		if (login_strategy != eLoginStrategy::LS_FROM_BSEVENT)
		{
			dprint1("L2: NO cache for seq:%s [cid:%s]\n", seq, cid);
			return;
		}
		else {
			dstream4 << "BSEvent:LoginUser2\n";
			auto ref = cnt.GetLongValueRef(PROTOCOLVERSION_PARAM);
			if (ref)
				udt.m_protocolVersion = *ref;
		}
	}

	int32_t result = SILENT_REJECT_LOGIN;
	cnt.GetValue(RESULT_PARAM,result);
	const char *nameUserForLogin = cnt.GetStrValueRef(LOGIN_PARAM);

	if (!cnt.GetValueI32(CLIENTTYPE_PARAM, udt.m_client_type)) {
		dstream4 << "ERROR\tBS " << m_recvMess->SrcServer() << " doesn't return client type for " << nameUserForLogin << ". Deny access for client!\n";
		result = ACCESS_DENIED;
	}

	std::string ip;
	if (m_policy.ShouldBeChecked( udt.m_client_type ) &&  nameUserForLogin  && GetIPByCID(cid, ip))
			m_policy.SetResult(ip, nameUserForLogin, USER_LOGGEDIN_OK == result);

	int32_t uuid(0);
	cnt.GetValue(SEQUENCE_ID_PARAM, uuid);
	std::vector<int>::iterator _it = std::find(m_loginTime.begin(), m_loginTime.end(), uuid);
	if (_it != m_loginTime.end() && uuid) {
		m_avetime = 0;
		m_login_stat[m_login_count % nAuthLogin] = GetTickCount() - (*_it);
		for (int i = 0; i < nAuthLogin; ++i) {
			m_avetime += m_login_stat[i];
		}
		m_avetime /= nAuthLogin;
		m_loginTime.erase(m_loginTime.begin(), _it);
		++m_login_count;
	}

    if (result==USER_LOGGEDIN_OK) {
		udt.m_name = cnt.GetStrValueRef(USERNAME_PARAM);
		auto dn = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
		if (dn != nullptr) udt.m_displayName = dn;
		cnt.GetValueI32(TYPE_PARAM, udt.m_type);
		cnt.GetValueI32(RIGHTS_PARAM, udt.m_rights);

		if (VS_GatewayStarter::GetInstance()==0)
		{
			udt.m_rights &= ~(VS_UserData::UR_COMM_DIALER|
							  VS_UserData::UR_COMM_EDITDIAL);
		}
		cnt.GetValueI32(RATING_PARAM, udt.m_rating);
		cnt.GetValueI32(SEPARATIONGROUP_PARAM, udt.m_SeparationGroup);
		udt.m_tarif_name = cnt.GetStrValueRef(TARIFNAME_PARAM);
		cnt.GetValueI32(TARIFRESTR_PARAM, udt.m_tarif_restrictions);
		udt.m_homeServer = cnt.GetStrValueRef(SERVER_PARAM);
		udt.m_key = cnt.GetStrValueRef(KEY_PARAM);
		udt.m_login_session_key = cnt.GetStrValueRef(SESSION_PARAM);

		const void* prop_ptr = nullptr;
		size_t prop_sz = 0;
		if ((prop_ptr=cnt.GetBinValueRef(PROPERTY_PARAM, prop_sz)) && prop_sz)
			udt.m_props.Set(prop_ptr, prop_sz);

		cnt.Reset();
		while(cnt.Next())
		{
			if(strcasecmp(cnt.GetName(),ALIAS_PARAM)==0 )
				udt.m_aliases.Insert(cnt.GetStrValueRef(),1);
			else
			if (strcasecmp(cnt.GetName(),EXTERNAL_ACCOUNT)==0)
				udt.m_external_accounts.emplace_back(cnt.GetStrValueRef());
		}

		udt.m_lTime = GetTickCount();
		m_loginData.Assign(seq, udt);

		// Search for logged user on other client on this server
		auto ep = GetCIDByUID(udt.m_name);
		if (!ep.empty())
		{
			if (ep != cid) {
				// Notify Client
				VS_Container rCnt;
				rCnt.AddValue(METHOD_PARAM, USERLOGGEDOUT_METHOD);
				rCnt.AddValueI32(RESULT_PARAM, USER_LOGGEDOUT_OK);
				rCnt.AddValueI32(CAUSE_PARAM, USER_LOGGEDIN);
				// Remove relation
				m_OnUserLoggedOut(udt, ep);
				PostUnauth(ep.c_str(), rCnt);
				dprint3("L2: (%s) logged out locally| %s %s\n", udt.m_name.m_str, ep.c_str(), cid);
			} else {	// same app loggedin already
				dstream3 << "L2: cid=" << cid << " is the same, no logout\n";
				if (login_strategy == eLoginStrategy::LS_FROM_BSEVENT)
					return ;
			}
		}

	   //register in BS
		if(udt.m_client_type!=VS_ClientType::CT_TRANSCODER)
		{
			VS_Container cnt2;
			cnt2.AddValue(METHOD_PARAM, REGISTERSTATUS_METHOD);
			cnt2.AddValue(REALID_PARAM, udt.m_name);
			cnt2.AddValue(ENDPOINT_PARAM, cid);
			cnt2.AddValue(SEQUENCE_PARAM, seq);
			cnt2.AddValueI32(TYPE_PARAM, udt.m_type);
			cnt2.AddValue(LOCATORBS_PARAM, udt.m_homeServer);
			cnt2.AddValue(DISPLAYNAME_PARAM, udt.m_displayName);
			for (VS_StrI_IntMap::ConstIterator i = udt.m_aliases.Begin(); !!i; ++i) {
				const char* alias = i->key;
				cnt2.AddValue(ALIAS_PARAM, alias);
			}
			PostRequest(udt.m_homeServer, 0, cnt2, 0, PRESENCE_SRV);
		}
		else
			UserLoggedIn_Method(cnt);
	}
	else {
		VS_Container cnt2;
		cnt2.AddValue(METHOD_PARAM, USERLOGGEDIN_METHOD);
		cnt2.AddValue(RESULT_PARAM, result);

		m_loginData.Erase(seq);
		PostUnauth(cid, cnt2);
	}
}


void VS_AuthService::UserLoggedIn_Method(VS_Container& cnt)
{
	const char* seq = cnt.GetStrValueRef(SEQUENCE_PARAM);
	const char* cid = cnt.GetStrValueRef(ENDPOINT_PARAM);
	if (!cid || !seq)
		return;

	VS_TempUserData udt;
	VS_TempUserDataMap::ConstIterator ui= m_loginData[seq];
	if (!!ui)
		udt=ui;
	else {
		dprint1("L3: NO cache for seq:%s [cid:%s]\n", seq, cid);
		return;
	}

	int32_t result = 0;
	cnt.GetValue(RESULT_PARAM,result);

    if (result==USER_LOGGEDIN_OK) {
		if (AuthorizeClient(cid, udt.m_name))
			g_storage->UpdateUser(SimpleStrToStringView(udt.m_name), udt);
		else
			result = ACCESS_DENIED;
	}

	VS_Container cnt2;
	cnt2.AddValue(METHOD_PARAM, USERLOGGEDIN_METHOD);
	cnt2.AddValueI32(RESULT_PARAM, result);

	if (result == USER_LOGGEDIN_OK) {
		cnt2.AddValue(USERNAME_PARAM, udt.m_name);
		cnt2.AddValue(DISPLAYNAME_PARAM, udt.m_displayName) ;
		cnt2.AddValueI32(RIGHTS_PARAM, udt.m_rights);
		cnt2.AddValueI32(RATING_PARAM, udt.m_rating);
		cnt2.AddValueI32(SEPARATIONGROUP_PARAM, udt.m_SeparationGroup);
		cnt2.AddValueI32(TYPE_PARAM, udt.m_type);
		cnt2.AddValue(KEY_PARAM, udt.m_key);
		cnt2.AddValue(TARIFNAME_PARAM, udt.m_tarif_name);
		cnt2.AddValueI32(TARIFRESTR_PARAM, udt.m_tarif_restrictions);
		cnt2.AddValue(SESSION_PARAM, udt.m_login_session_key);
		if (udt.m_props.IsValid())
			cnt2.AddValue(PROPERTY_PARAM, udt.m_props.Buffer(), udt.m_props.Size());

		dprint2("Login of (%s), cid %s\n", udt.m_name.m_str, cid);
	}
	else {
		// erase for bad login-pass
		m_loginData.Erase(seq);
	}

	PostUnauth(cid, cnt2);

	if (result == USER_LOGGEDIN_OK) {
		m_OnUserLoggedIn(udt, cid);
	}
}

////////////////////////////////////////////////////////////////////////////////
// LOGOUTUSER_METHOD()
////////////////////////////////////////////////////////////////////////////////
void VS_AuthService::LogoutUser_Method(const vs_user_id& user, const char* uplink)
{
	VS_UserLoggedout_Result result = USER_ALREADY_LOGGEDOUT;

	std::string temp_id;
	if (!!user)
		temp_id = GetCIDByUID(user);
	if (temp_id.empty() && !uplink)
		temp_id = std::string(m_recvMess->SrcCID_sv());
	if (temp_id.empty()) {
		if (uplink) {
			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, UNREGISTERSTATUS_METHOD);
			cnt.AddValue(CALLID_PARAM, user);

			PostRequest(uplink, 0, cnt, 0, PRESENCE_SRV);
		}
		else {
			dprint0("can't find CID for %s\n", user.m_str);
		}
		return;
	}

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
	rCnt.AddValueI32(CAUSE_PARAM, uplink ? USER_LOGGEDIN : USER_LOGGEDOUT_BY_REQUEST);

	PostUnauth(temp_id.c_str(), rCnt);
}

////////////////////////////////////////////////////////////////////////////////
// CHECKUSERLOGINSTATUS_METHOD()
////////////////////////////////////////////////////////////////////////////////
void VS_AuthService::CheckUserLoginStatus_Method()
{
	const char * cid = m_recvMess->SrcCID();
	//TODO: remove to allow to join old vzo clients
	if (atou_s(m_recvMess->AddString()) < 41)
		return;

	long result = NO_USER_LOGGEDIN;
	if (cid && *cid) {
		if (m_avetime > noBSbarrier) {
			dprint1("Skip CheckUserLoginStatus - %ld sec timeout - BS busy!\n", m_avetime);
			m_avetime = m_avetime*(2*nAuthLogin-1)/(2*nAuthLogin+1);
			return;
		}
	}
	else {
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
}

void VS_AuthService::OnUserLoginEnd_Event(const VS_UserData& ud, const std::string &cid)
{
	if (ud.m_protocolVersion < 34) { // without inverse ab query
		long hash = 0;
		{
			std::lock_guard<std::mutex> lock(g_storage->m_BookLock);
			VS_Storage::VS_UserBook* user_book = g_storage->GetUserBook(ud.m_name, AB_INVERSE);
			if (user_book)
				hash = (long)user_book->hash;
		}
		VS_Container rCnt;
		rCnt.AddValue(METHOD_PARAM, SEARCHADDRESSBOOK_METHOD);
		rCnt.AddValue(QUERY_PARAM, "");
		rCnt.AddValueI32(HASH_PARAM, hash);
		rCnt.AddValueI32(ADDRESSBOOK_PARAM, AB_INVERSE);
		void*	body;
		size_t bodySize;
		rCnt.SerializeAlloc(body, bodySize);

		VS_RouterMessage* msg = new VS_RouterMessage(ADDRESSBOOK_SRV, "33", ADDRESSBOOK_SRV, 0, ud.m_name, OurEndpoint(), OurEndpoint(), 30000, body, bodySize);
		bool result = PostMes(msg);
		if (!result)
			delete msg;
		free(body);
	}
}

/******************************************************************************
 * 1 - Notifiy Conference Service
 * 2 - Notifiy User Presence Service
 *****************************************************************************/
void VS_AuthService::OnUserLogoff_Event(const VS_UserData &ud, const std::string &cid)
{
	vs_user_id user_id = ud.m_name;

	if (!user_id || !*user_id)
		return;

	g_storage->DeleteUser(SimpleStrToStringView(user_id));
	g_conferenceService->RemoveParticipant_Event(user_id, VS_ParticipantDescription::DISCONNECT_LOGOFF);

	UnauthorizeClient(user_id);
	dprint2("Logoff of (%s), cid %s\n", user_id.m_str, cid.c_str());

	assert(ud.m_client_type == VS_ClientType::CT_TRANSCODER || ud.m_homeServer.Length() > 0);
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, UNREGISTERSTATUS_METHOD);
	cnt.AddValue(CALLID_PARAM, user_id);
	PostRequest(ud.m_homeServer, 0, cnt, 0, PRESENCE_SRV);

	return;
}


bool VS_AuthService::OnPointConnected_Event(const VS_PointParams* prm)
{
	if (prm->type==VS_PointParams::PT_CLIENT) {
		dprint1("cid Connect: %s, r=%2d\n", prm->cid, prm->reazon);
	}
	return true;
}

bool VS_AuthService::OnPointDisconnected_Event(const VS_PointParams* prm)
{
	if (prm->type==VS_PointParams::PT_CLIENT) {
		dprint1("cid Disconnect: %s, uid=%s, r= %2d\n", prm->cid, prm->uid, prm->reazon);

		if (prm->uid && *prm->uid) {
			VS_Container cnt;
			cnt.AddValue(METHOD_PARAM, POINTDISCONNECTED_METHOD);
			cnt.AddValue(USERNAME_PARAM, prm->uid);
			cnt.AddValue(NAME_PARAM, prm->cid);

			PostRequest(OurEndpoint(), 0, cnt);
		}
	}
	return true;
}

bool VS_AuthService::OnPointDeterminedIP_Event(const char* uid, const char* ip)
{
	dprint2("Ip of (%s) is <%s>\n", uid, ip);

	VS_UserData ud;
	if (!uid || !g_storage->FindUser(uid, ud))
		return true;

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, SETPROPERTIES_METHOD);
	cnt.AddValue(HASH_PARAM, ud.m_appID);
	cnt.AddValue(NAME_PARAM, "wan_ip");
	cnt.AddValue(PROPERTY_PARAM, ip);

	PostRequest(ud.m_homeServer, 0, cnt, 0, CONFIGURATION_SRV);

	return true;
}

void VS_AuthService::UpdateAccount_Method(VS_Container& cnt)
{
	const auto callId = cnt.GetStrValueView(TRANSPORT_SRCUSER_PARAM);
	VS_UserData ud;
	if (!g_storage->FindUser(callId, ud))
		return ;

	int32_t value = 0;
	const char* dn = 0;

	if (cnt.GetValue(RIGHTS_PARAM, value))
		ud.m_rights = value;
	if (cnt.GetValue(RATING_PARAM, value))
		ud.m_rating = value;
	if (cnt.GetValue(SEPARATIONGROUP_PARAM, value))
		ud.m_SeparationGroup = value;
	if ((dn=cnt.GetStrValueRef(DISPLAYNAME_PARAM))!=0)
		ud.m_displayName = dn;
	if (cnt.GetValue(TARIFRESTR_PARAM, value))
		ud.m_tarif_restrictions = value;
	if ((dn=cnt.GetStrValueRef(TARIFNAME_PARAM))!=0)
		ud.m_tarif_name = dn;

	g_storage->UpdateUser(callId, ud);

	PostRequest(OurEndpoint(), callId.c_str(), cnt, 0, AUTH_SRV);
}

void VS_AuthService::ReqUpdateAccount_Method(VS_Container& cnt)
{
	const char* call_id = m_recvMess->SrcUser();
	VS_UserData ud;
	if (!g_storage->FindUser(call_id, ud))
		return ;
	cnt.AddValue(TRANSPORT_SRCUSER_PARAM, call_id);
	cnt.AddValue(APPID_PARAM, ud.m_appID);
	PostRequest(ud.m_homeServer, 0, cnt);
}

void VS_AuthService::OnUserChange_Method(VS_Container& cnt)
{
	const char* callId = cnt.GetStrValueRef(TRANSPORT_SRCUSER_PARAM);
	if (!callId)
		return ;

	LogoutUser_Method(callId, 0);

	// delete info for login when no BS
	for(VS_TempUserDataMap::Iterator it = m_loginData.Begin(); it != m_loginData.End(); ++it)
	{
		if (it->data && it->data->m_name == callId)
		{
			m_loginData.Erase(it);
			it = m_loginData.Begin();
		}
	}
}

void VS_AuthService::SetRegID_Method(VS_Container& cnt)
{
	const char* call_id = cnt.GetStrValueRef(CALLID_PARAM);
	VS_UserData ud;
	if (!call_id || !g_storage->FindUser(call_id, ud)) { // kt: send to GetBSbyDomain of user at CALLID_PARAM, through LOCATOR_SRV
		VS_SimpleStr BS;
		VS_SimpleStr domain;
		GetUserDomain(call_id,domain); // domain from login, if exists
		bool isBS = g_appServer->GetBSByDomain(domain, BS) /*|| g_appServer->GetBSByDomain(udt.m_defaultDomain, BS)*/ || g_appServer->GetBSByDomain(m_defaultDomain.c_str(), BS);
		if (isBS && !!BS)
		{
			const char* src_user = (m_recvMess->SrcUser() && *m_recvMess->SrcUser()) ? m_recvMess->SrcUser() : call_id;
			cnt.AddValue(TRANSPORT_SRCUSER_PARAM, src_user);
			cnt.AddValue(USERNAME_PARAM, src_user);		// for LocatorSRV
			PostRequest(BS, 0, cnt, AUTH_SRV, LOCATE_SRV);
		}
	}else{
		cnt.AddValue(TRANSPORT_SRCUSER_PARAM, m_recvMess->SrcUser());
		PostRequest(ud.m_homeServer, 0, cnt);
	}
}

void VS_AuthService::UpdatePeerCfg_Method(VS_Container& cnt)
{
	const char* call_id = cnt.GetStrValueRef(CALLID_PARAM);
	if (!call_id || !*call_id)
		return ;
	std::vector<VS_ExternalAccount> v;
	cnt.Reset();
	while(cnt.Next())
	{
		if (strcasecmp(cnt.GetName(),EXTERNAL_ACCOUNT)==0)
			v.emplace_back(cnt.GetStrValueRef());
	}
	m_OnNewPeerCfg(call_id, v);
}
#endif