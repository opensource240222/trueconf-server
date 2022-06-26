#ifdef _WIN32	// not ported
#include <stdio.h>

#include "../../ServerServices/Common.h"

#include "VS_BaseAuthService.h"
#include "std/cpplib/md5.h"
#include "../../common/std/cpplib/VS_MemoryLeak.h"
#include "../../common/std/cpplib/VS_FileTime.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "VS_BSLogService.h"

#define DEBUG_CURRENT_MODULE VS_DM_LOGINS

bool VS_BaseAuthService::Init( const char *our_endpoint, const char *our_service, const bool permittedAll )
{
	auto dbStorage = g_dbStorage;
	m_login_session_secret = dbStorage->GetLoginSessionSecret();
	return true;
}

bool VS_BaseAuthService::Processing( std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == 0)	return true;
	switch (recvMess->Type())
	{
	case transport::MessageType::Invalid:
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		m_recvMess = recvMess.get();
		{
			VS_Container cnt;
			if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
				const char* method = 0;
				if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0) {
					dprint3("Processing %20s; cid:%s user:%s \n", method, recvMess->SrcCID(), recvMess->SrcUser());
					// Process methods
					if (_stricmp(method, LOGINUSER_METHOD) == 0) {

						LoginUser_Method(cnt);
					} else if (_stricmp(method, REQUPDATEACCOUNT_METHOD) == 0) {
						ReqUpdateAccount_Method(cnt);
					} else if (_stricmp(method, SETREGID_METHOD) == 0) {
						SetRegID_Method(cnt);
					} else if (_stricmp(method, UPDATE_PEERCFG_METHOD) == 0) {
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

////////////////////////////////////////////////////////////////////////////////
// LOGINUSER_METHOD()
////////////////////////////////////////////////////////////////////////////////
void VS_BaseAuthService::LoginUser_Method(VS_Container& cnt)
{
	const char* login = cnt.GetStrValueRef(LOGIN_PARAM);
	const char* password = cnt.GetStrValueRef(PASSWORD_PARAM);
	VS_SimpleStr key = cnt.GetStrValueRef(KEY_PARAM);
	const char* appID = cnt.GetStrValueRef(HASH_PARAM);
	const char* seq = cnt.GetStrValueRef(SEQUENCE_PARAM);
	const char* cid = cnt.GetStrValueRef(ENDPOINT_PARAM);
	const char* appName = cnt.GetStrValueRef(APPNAME_PARAM);
	const char* srv = m_recvMess->SrcServer();
	eLoginStrategy ls(eLoginStrategy::LS_DEFAULT);
	cnt.GetValueI32(LOGINSTRATEGY_PARAM, ls);
	const char* to_server = cnt.GetStrValueRef(SERVER_PARAM);

	if (ls == eLoginStrategy::LS_FROM_BSEVENT)
		srv = to_server;
	int32_t uuid(0);
	cnt.GetValue(SEQUENCE_ID_PARAM, uuid);
	VS_ClientType client_type(CT_SIMPLE_CLIENT);
	cnt.GetValueI32(CLIENTTYPE_PARAM, client_type);
	int protocolVersion(0);
	auto ref = cnt.GetLongValueRef(PROTOCOLVERSION_PARAM);
	if (ref)
		protocolVersion = *ref;

	dprint4("LoginUser %s, p=%s, k=%s, a=%s, s=%s, cid=%s, APP=%s, srv=%s, uuid=%d, ct=%d, protocolVersion=%d\n", login, password, key.m_str, appID, seq, cid, appName, srv, uuid, client_type, protocolVersion);

	if (!login || !*login ) return;

	VS_Container prop_cnt;
	VS_UserData ud;		ud.m_appName = appName;
	bool bkey = !!key;
	auto dbStorage = g_dbStorage;
	long result = dbStorage->LoginUser(login, password, appID, key, srv, ud, prop_cnt, client_type);
	if (bkey && !password && result != USER_LOGGEDIN_OK && result != SESSION_WAIT)
		result = ACCESS_DENIED;

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, LOGINUSER2_METHOD);
	rCnt.AddValueI32(RESULT_PARAM, result);
	rCnt.AddValue(SEQUENCE_PARAM, seq);
	rCnt.AddValue(ENDPOINT_PARAM, cid);
	rCnt.AddValue(SEQUENCE_ID_PARAM, uuid);
	rCnt.AddValue(LOGIN_PARAM, login);
	rCnt.AddValueI32(CLIENTTYPE_PARAM, client_type);
	rCnt.AddValueI32(PROTOCOLVERSION_PARAM, protocolVersion);
	rCnt.AddValueI32(LOGINSTRATEGY_PARAM, ls);

	if (result == USER_LOGGEDIN_OK) {
		//if (ud.IsProTarif() || ud.IsCorpTarif())
		//{
		//	ud.m_rights |= VS_UserData::UR_COMM_CREATEMULTI;
		//	ud.m_rights |= VS_UserData::UR_COMM_SLIDESHOW;
		//	ud.m_rights |= VS_UserData::UR_COMM_DSHARING;
		//} else if (ud.IsFreeTarif()) {
		//	ud.m_rights &= ~VS_UserData::UR_COMM_CREATEMULTI;
		//	ud.m_rights &= ~VS_UserData::UR_COMM_SLIDESHOW;
		//	ud.m_rights &= ~VS_UserData::UR_COMM_DSHARING;
		//}

		//user data
		rCnt.AddValue(USERNAME_PARAM, ud.m_name);
		rCnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
		rCnt.AddValueI32(TYPE_PARAM, ud.m_type);
		rCnt.AddValueI32(RIGHTS_PARAM, ud.m_rights);
		rCnt.AddValueI32(RATING_PARAM, ud.m_rating);
		rCnt.AddValueI32(SEPARATIONGROUP_PARAM, ud.m_SeparationGroup);
		rCnt.AddValue(TARIFNAME_PARAM, ud.m_tarif_name);
		rCnt.AddValueI32(TARIFRESTR_PARAM, ud.m_tarif_restrictions);
		rCnt.AddValue(SERVER_PARAM, OurEndpoint());
		//autologin data
		rCnt.AddValue(KEY_PARAM, key);

		for (VS_StrI_IntMap::ConstIterator i = ud.m_aliases.Begin(); !!i; ++i)
			rCnt.AddValue(ALIAS_PARAM, i->key);

		SerializeExternalAccounts(ud.m_external_accounts,rCnt);

		rCnt.AddValue(SESSION_PARAM, GenerateSessionKey(ud.m_name));

		if (prop_cnt.IsValid())
			rCnt.AddValue(PROPERTY_PARAM, prop_cnt);
	}
	else if (result == SESSION_WAIT){
		// notify web about waiting registration
		VS_Container notify_cnt;
		notify_cnt.AddValue(METHOD_PARAM, SENDMAIL_METHOD);
		notify_cnt.AddValueI32(TYPE_PARAM, e_notifyviaweb_user_waits_for_letter);
		PostRequest(OurEndpoint(), "", notify_cnt, 0, LOG_SRV);

		cnt.AddValue(SERVER_PARAM, srv);
		dbStorage->SaveTempLoginData(login, appID, cnt);
	}

	if (ls == eLoginStrategy::LS_FROM_BSEVENT)
		PostRequest(to_server, "", rCnt, 0, AUTH_SRV);
	else
		PostReply(rCnt);
}

VS_SimpleStr VS_BaseAuthService::GenerateSessionKey(const char* call_id)
{
	if (!call_id)
		return 0;

	SYSTEMTIME st;
	st.wYear = 2000;
	st.wMonth = 1;
	st.wDay = 1;
	st.wHour = 0;
	st.wMinute = 0;
	st.wSecond = 0;
	st.wMilliseconds = 0;
	VS_FileTime ft;

	SystemTimeToFileTime(&st, &ft.m_filetime);

	static const LONGLONG minute = 60*1000*1000*10;
	VS_FileTime ft_now;		ft_now.Now();
	for(unsigned int i=0; i < 24*60; i++)	// add 24 hours
		ft_now += minute;

	ft_now -= ft;
	int diff = ft_now.GetTimeInSec() / 60;

	char date_hash[9] = {0};
	sprintf_s(date_hash, 9, "%08X", diff);

	MD5 md5;
	md5.Update(call_id);
	md5.Update(date_hash);
	md5.Update(SimpleStrToStringView(m_login_session_secret));
	md5.Final();
	char md5_hash[33];
	md5.GetString(md5_hash);

	VS_SimpleStr session_key = date_hash;
				 session_key += md5_hash;
	return session_key;
}

void VS_BaseAuthService::ReqUpdateAccount_Method(VS_Container& cnt)
{
	const char* call_id = cnt.GetStrValueRef(CALLID_PARAM);
	if (!call_id||!*call_id)
		return ;

	int32_t type(RUAT_SESSION);
	cnt.GetValue(TYPE_PARAM, type);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, UPDATEACCOUNT_METHOD);
	if (type == RUAT_SESSION)
		rCnt.AddValue(SESSION_PARAM, GenerateSessionKey(call_id));
	else if (type == RUAT_RIGHTS) {
		VS_UserData ud;
		auto dbStorage = g_dbStorage;
		if (dbStorage->FindUser(call_id, ud)) {
			long rights(ud.m_rights);
			dprint4("ReqUpdateAccount(type=%d, call_id=%s):r=%lx\n", type, call_id, rights);
			rCnt.AddValueI32(RIGHTS_PARAM, rights);
		}
	}
	else if (type == RUAT_PASSWORD) {
		long result(-1);
		const char* src_user = cnt.GetStrValueRef(TRANSPORT_SRCUSER_PARAM);
		const char* old_password = cnt.GetStrValueRef("OldPassword");
		const char* new_password = cnt.GetStrValueRef("NewPassword");
		VS_SimpleStr from_app_id = cnt.GetStrValueRef(APPID_PARAM);
		dprint4("ReqUpdateAccount: ChangePassword:%s/%s,old=%s,new=%s,app_id=%s\n", src_user, call_id, old_password, new_password, from_app_id.m_str);
		if (src_user&&*src_user&&!_stricmp(src_user, call_id)&&
			old_password&&*old_password&&
			new_password&&*new_password)
		{
			auto dbStorage = g_dbStorage;
			result = g_dbStorage->ChangePassword(call_id, old_password, new_password, from_app_id);
		}
		rCnt.AddValue(TYPE_PARAM, type);
		rCnt.AddValue(CALLID_PARAM, call_id);
		rCnt.AddValueI32(RESULT_PARAM, result);
		dprint4("ReqUpdateAccount: ChangePassword:%s/%s, result=%ld\n", src_user, call_id, result);
	}else if (type == RUAT_PROFILE) {
		long result(-1);
		const char* src_user = cnt.GetStrValueRef(TRANSPORT_SRCUSER_PARAM);
		dprint4("ReqUpdateAccount: UpdatePerson:%s/%s\n", src_user, call_id);
		if (src_user&&*src_user&&!_stricmp(src_user, call_id))
		{
			auto dbStorage = g_dbStorage;
			result = g_dbStorage->UpdatePerson(call_id, cnt);
		}
		rCnt.AddValue(TYPE_PARAM, type);
		rCnt.AddValue(CALLID_PARAM, call_id);
		rCnt.AddValueI32(RESULT_PARAM, result);
		dprint4("ReqUpdateAccount: UpdatePerson:%s/%s, result=%ld\n", src_user, call_id, result);
	}
	PostRequest(m_recvMess->SrcServer(), call_id, rCnt);
}

void VS_BaseAuthService::SetRegID_Method(VS_Container& cnt)
{
	long result(-1);
	const char* src_user = cnt.GetStrValueRef(TRANSPORT_SRCUSER_PARAM);
	const char* call_id = cnt.GetStrValueRef(CALLID_PARAM);
	const char* reg_id = cnt.GetStrValueRef(REGID_PARAM);
	VS_RegID_Register_Type reg_type = (cnt.GetLongValueRef(SUBTYPE_PARAM))? (VS_RegID_Register_Type)(*cnt.GetLongValueRef(SUBTYPE_PARAM)): REGID_INVALID;
	dprint4("SetRegID: %s/%s, reg_id=%s\n", src_user, call_id, reg_id);
	if ((src_user&&*src_user && reg_type==REGID_REGISTER) || REGID_UNREGISTER == reg_type)
	{
		auto dbStorage = g_dbStorage;
		result = g_dbStorage->SetRegID(call_id, reg_id, reg_type);
	}
	dprint3("SetRegID: %s/%s, reg_id=%s, res=%ld\n", src_user, call_id, reg_id, result);
}

void VS_BaseAuthService::UpdatePeerCfg_Method(VS_Container& cnt)
{
	const char* call_id = cnt.GetStrValueRef(CALLID_PARAM);
	const char* server = cnt.GetStrValueRef(SERVER_PARAM);
	if (!call_id || !*call_id || !server || !*server)
		return ;

	std::vector<VS_ExternalAccount> external_accounts;
	auto dbStorage = g_dbStorage;
	g_dbStorage->GetSipProviderByCallId(call_id, external_accounts);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, UPDATE_PEERCFG_METHOD);
	rCnt.AddValue(CALLID_PARAM, call_id);
	SerializeExternalAccounts(external_accounts, rCnt);
	PostRequest(server, 0, rCnt);
}

bool VS_BaseAuthService::SerializeExternalAccounts(const std::vector<VS_ExternalAccount>& v, VS_Container& cnt)
{
	if (!v.size())
		return false;
	for (unsigned i = 0; i < v.size(); i++)
	{
		std::string data = v[ i ].Serealize();
		cnt.AddValue(EXTERNAL_ACCOUNT, data.c_str());
	}
	return true;
}
#endif