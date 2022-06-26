#include "VS_LoginManager.h"
#include "std/cpplib/md5.h"

VS_LoginManager::VS_LoginManager()
{
	VS_ReadClientHardwareKey(m_AppId);
}

VS_LoginManager::~VS_LoginManager()
{
	Logout();
}

unsigned long VS_LoginManager::Login(const char* user, const char* password)
{
	m_user = user;
	m_password = password;
	return ERR_OK;

	if (!user || !*user || !password || !*password) return VSTRCL_ERR_CURROP;
	DTRACE(VSTM_PRTCL, "LoginUser: %s", user);

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, LOGINUSER_METHOD);
	rCnt.AddValue(LOGIN_PARAM, user);
	char md5_pass[256]; *md5_pass = 0;
//	if (noEncript ||(GetProperties("authentication_method", md5_pass)==ERR_OK && strcmp("1", md5_pass)==0) ) {
//		rCnt.AddValue(PASSWORD_PARAM, password);
//	}
//	else {
		VS_ConvertToMD5(password, md5_pass);
		rCnt.AddValue(PASSWORD_PARAM, md5_pass);
//	}
	rCnt.AddValue(HASH_PARAM, m_AppId);
	rCnt.CopyTo(m_LoginRetryCnt);

	ComposeSend(rCnt, AUTH_SRV);
	//SetTimerObject(TIME_LGIN);
	return ERR_OK;
}

unsigned long VS_LoginManager::Logout(bool clearAutoLogin)
{
	return ERR_OK;
	DTRACE(VSTM_PRTCL, "LogoutUser");
	if (clearAutoLogin) {
		// clear in registry
		// SetAutoLogin(false);
		// clear current key
		//m_AutoLoginKey.Empty();
	}

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, LOGOUTUSER_METHOD);
	m_LoginRetryCnt.Clear();

	ComposeSend(rCnt, AUTH_SRV);
	//SetTimerObject(TIME_LGOUT);
	return ERR_OK;
}

const char *VS_LoginManager::GetCurrentUser()
{
	if (m_user.empty()) return 0;
	return m_user.c_str();
}

bool VS_LoginManager::ParseIncomimgMessage(VS_Container &cnt)
{
//					m_Status.MyInfo.UserName =		cnt.GetStrValueRef(USERNAME_PARAM);
//					m_Status.MyInfo.DisplayName =	cnt.GetStrValueRef(DISPLAYNAME_PARAM);
//					m_Status.MyInfo.CallId =		cnt.GetStrValueRef(CALLID_PARAM);
//					m_AutoLoginKey =				cnt.GetStrValueRef(KEY_PARAM);

	if (!cnt.IsValid()) return false;
	const char *Method = cnt.GetStrValueRef(METHOD_PARAM);
	long result, cause;
	if (Is(Method, USERLOGGEDIN_METHOD)) {
		if (cnt.GetValue(RESULT_PARAM, result)) {
			m_LoginRetryCnt.Clear();
			const char *userName = cnt.GetStrValueRef(USERNAME_PARAM);
			if (userName) m_user = userName;
			OnLogin(m_user.c_str(), result);
		}
	} else if (Is(Method, USERLOGGEDOUT_METHOD)) {
		if (cnt.GetValue(RESULT_PARAM, result) && cnt.GetValue(CAUSE_PARAM, cause)) {
			OnLogout(cause);
		}
	}
	cnt.Clear();
	return true;
}

unsigned long VS_LoginManager::AutoLogin()
{
	// remove to enable
	return ERR_OK;

/*	if (!m_AutoLoginKey)
		return ERR_OK;
	DTRACE(VSTM_PRTCL, "AutoLogin of %s", m_LastLogin);*/

	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, LOGINUSER_METHOD);
	rCnt.AddValue(LOGIN_PARAM, m_user);
	//rCnt.AddValue(KEY_PARAM, m_AutoLoginKey);
	rCnt.AddValue(HASH_PARAM, m_AppId);
	rCnt.CopyTo(m_LoginRetryCnt);

	ComposeSend(rCnt, AUTH_SRV);
//	SetTimerObject(TIME_LGIN);
	return ERR_OK;
}

unsigned long VS_LoginManager::RetryLastLogin()
{
	// remove to enable
	return ERR_OK;

	if (m_LoginRetryCnt.IsValid()) {
		ComposeSend(m_LoginRetryCnt, AUTH_SRV);
		return ERR_OK;
	} else {
		return VSTRCL_ERR_CONTEYNER;
	}
}

#if 0
/**
****************************************************************************
* User Login service Messages interpreter.
*
* \param msg				- pointer to Window Message;
* \date    18-11-2002
******************************************************************************/
void CVSTrClientProc::LoginUserSrv(VS_ClientMessage &tMsg)
{
	unsigned long bodySize;	void *body;
	DWORD dwRet = ERR_OK;
	const char * Method = 0;

	bodySize = tMsg.Body(&body);
	VS_Container cnt;
	cnt.Deserialize(body, bodySize);
	Method = cnt.GetStrValueRef(METHOD_PARAM);

	switch(tMsg.Type())
	{
	case transport::MessageType::Invalid:
		dwRet = VSTRCL_ERR_INTERNAL;
		break;
	case transport::MessageType::Request:
	case transport::MessageType::Reply:
		DTRACE(VSTM_PRTCL, "method    = %20s", Method);
		if (Method && _stricmp(Method, USERLOGGEDIN_METHOD) == 0) {
			long result;
			if (cnt.GetValue(RESULT_PARAM, result))	{
				SetOnlineStatus(true);
				switch((VS_UserLoggedin_Result )result)
				{
				case USER_LOGGEDIN_OK:
					m_CleanOnSubscribe = true;
					m_LoginRetryCnt.Clear();
					m_Status.dwStatus |= STATUS_LOGGEDIN;
					m_Status.MyInfo.Clean();
					for (int i = 0; i<m_Status.MAX_CONFINFO; i++) // reset all
						m_Status.ConfInfo[i].Clean();
					m_Status.MyInfo.UserName =		cnt.GetStrValueRef(USERNAME_PARAM);
					m_Status.MyInfo.DisplayName =	cnt.GetStrValueRef(DISPLAYNAME_PARAM);
					m_Status.MyInfo.CallId =		cnt.GetStrValueRef(CALLID_PARAM);
					m_AutoLoginKey =				cnt.GetStrValueRef(KEY_PARAM);
					cnt.GetValue(RIGHTS_PARAM, m_Status.MyInfo.Rights);

					VS_SetEndpointName(m_Status.MyInfo.UserName);
					VS_RemoveAcceptPorts();
					if (m_DirectPort) {
						VS_UPnPInterface* unpn = VS_UPnPInterface::Instance();
						unsigned long *ports = 0;
						unsigned long num = 0;
						VS_GetAppPorts(ports, num);
						ports[0] = MAKELONG(m_DirectPort, m_DirectPort);
						char ip[256] = {0};
						long lenght = 255;
						VS_GetEndpointSourceIP(tMsg.SrcServer(), ip, lenght);
						unpn->FullAsyncPortMapping(m_dThreadID, ports, num, ip);
					}
					else
						UpdateConfiguration(false);

					ReadPropNetConfig();
					SetAutoLogin(m_Status.dwStatus&STATUS_LOGGEDIN ? m_AutoLogin : 0);
					dwRet = VSTRCL_LOGIN_OK;
					break;
				case USER_ALREADY_LOGGEDIN:
					m_LoginRetryCnt.Clear();
					if (!(m_Status.dwStatus&STATUS_LOGGEDIN))
						AutoLogin();
					dwRet = VSTRCL_LOGIN_OK;
					{	// check ip-address change
						VS_RemoveAcceptPorts();
						if (m_DirectPort) {
							VS_UPnPInterface* unpn = VS_UPnPInterface::Instance();
							unsigned long *ports = 0;
							unsigned long num = 0;
							VS_GetAppPorts(ports, num);
							ports[0] = MAKELONG(m_DirectPort, m_DirectPort);
							char ip[256] = {0};
							long lenght = 255;
							VS_GetEndpointSourceIP(tMsg.SrcServer(), ip, lenght);
							unpn->FullAsyncPortMapping(m_dThreadID, ports, num, ip);
						}
						else
							UpdateConfiguration(false);
					}
					break;
				case NO_USER_LOGGEDIN:
					m_Status.dwStatus&=~STATUS_LOGGEDIN;
					AutoLogin();
					dwRet = VSTRCL_LOGIN_OK;
					break;
				case ACCESS_DENIED:
					m_LoginRetryCnt.Clear();
					m_Status.dwStatus&=~STATUS_LOGGEDIN;
					dwRet = VSTRCL_LOGIN_INCPAR;
					break;
				case SILENT_REJECT_LOGIN:
					m_LoginRetryCnt.Clear();
					m_Status.dwStatus&=~STATUS_LOGGEDIN;
					dwRet = VSTRCL_LOGIN_OK;
					break;
				case LICENSE_USER_LIMIT:
					m_LoginRetryCnt.Clear();
					m_Status.dwStatus&=~STATUS_LOGGEDIN;
					dwRet = VSTRCL_LOGIN_USERLIMIT;
					break;
				case USER_DISABLED:
					m_LoginRetryCnt.Clear();
					m_Status.dwStatus&=~STATUS_LOGGEDIN;
					dwRet = VSTRCL_LOGIN_USDISBLD;
					break;
				case RETRY_LOGIN:
					{
						// increase time for login resending, min time ~ 30 sec
						long timeout = 30000;
						cnt.GetValue(FIELD1_PARAM, timeout);
						if (timeout > 300000)
							timeout = 300000;
						if (timeout < 30000)
							timeout = 30000;
						SetTimerObject(TIME_LGIN, timeout);
					}
					dwRet = VSTRCL_LOGIN_OK;
					break;
				default:
					m_LoginRetryCnt.Clear();
					dwRet = VSTRCL_ERR_PARAM;
					break;
				}
				if (!m_LoginRetryCnt.IsValid())
					RemoveTimerObjects(TIME_LGIN);
				RemoveTimerObjects(TIME_CHUSLS);
			}
			else
				dwRet = VSTRCL_ERR_CONTEYNER;
		}
		else if (Method && _stricmp(Method, USERLOGGEDOUT_METHOD) == 0) {
			long result, cause;
			if (cnt.GetValue(RESULT_PARAM, result))	{
				if (m_Status.dwStatus&STATUS_CONFERENCE)
					Hangup();
				m_Status.dwStatus&=~STATUS_LOGGEDIN;
				switch((VS_UserLoggedout_Result )result)
				{
				case USER_ALREADY_LOGGEDOUT:
					dwRet = VSTRCL_LOGIN_OK;		break;
				case USER_LOGGEDOUT_OK:
					m_Status.MyInfo.Clean();
					cnt.GetValue(CAUSE_PARAM, cause);
					switch((VS_UserLoggedout_Cause )cause)
					{
					case USER_LOGGEDOUT_BY_REQUEST:
						// autologin have been cleaned
						dwRet = VSTRCL_LOGIN_OK;
						break;
					case USER_LOGGEDIN:
						// clear user to prevent autologin, must be filled again by user on startup
						m_AutoLoginKey.Empty();
						dwRet = VSTRCL_LOGIN_LGFROMS;
						break;
					default: 						dwRet = VSTRCL_ERR_PARAM;		break;
					}
					break;
				default:
					dwRet = VSTRCL_ERR_PARAM;			break;
				}
			}
			else	dwRet = VSTRCL_ERR_CONTEYNER;
			RemoveTimerObjects(TIME_LGOUT);
			SetEvent(m_LogoutEvent);
		}
		else if (Method && _stricmp(Method, AUTHORIZE_METHOD) == 0) {
			Authorize(cnt);
			dwRet = VSTRCL_LOGIN_OK;
		}
		else if (Method && _stricmp(Method, UPDATEACCOUNT_METHOD) == 0) {
			cnt.GetValue(RIGHTS_PARAM, m_Status.MyInfo.Rights);
			dwRet = VSTRCL_LOGIN_UPDATEACC;
		}
		else		dwRet = VSTRCL_ERR_METHOD;
		break;
	case transport::MessageType::Notify:
		dwRet = VSTRCL_ERR_TIMEOUT;
		break;
	default: 						dwRet = VSTRCL_ERR_UNKMESS;		break;
	}

	PostProc(dwRet, m_Status.MyInfo.Rights);
}
#endif