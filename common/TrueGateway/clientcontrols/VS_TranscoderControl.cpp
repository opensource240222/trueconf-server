#ifdef _WIN32 // not ported
#include "VS_TranscoderControl.h"
#include "VS_TranscoderLogin.h"
#include "VS_TranscodersDispatcher.h"
#include "TrueGateway/interfaces/VS_ConferenceProtocolInterface.h"
#include "tools/Server/CommonTypes.h"
#include "std/cpplib/VS_WideStr.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/debuglog/VS_Debug.h"
#include "tools/Server/VS_MediaChannelInfo.h"

#include <boost/algorithm/string/predicate.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_MULTIGATEWAY

#define MAX_TIME_FOR_CALL_PREPARATION 10000


VS_TranscoderControl::VS_TranscoderControl(const char *name, VS_TranscodersDispatcher* srv, const std::weak_ptr<VS_TranscoderLogin> &transLogin)
	: m_name(name ? name : ""), m_srv(srv), m_transLogin(transLogin)
	, m_isStarted(false),m_state(e_not_init), m_preparation_start_tick(0),m_clearing_start_tick(0),
	m_user_login("")
{
	m_transcoderPreparedEvent	= CreateEvent(0,FALSE,FALSE,0);

	memset( (void *)&m_pi, 0, sizeof(m_pi) );
	dprint4("VS_TranscoderControl created. name = %s\n", m_name.c_str());
}
VS_TranscoderControl::~VS_TranscoderControl()
{
	Stop();
	if(m_pi.hProcess)
	{
		WaitForSingleObject(m_pi.hProcess,INFINITE);
		CloseHandle(m_pi.hThread);
		CloseHandle(m_pi.hProcess);
	}
	CloseHandle(m_transcoderPreparedEvent);

	memset( (void *)&m_pi, 0, sizeof(m_pi) );
	VS_RegistryKey rKey(false, TRANSCODERS_KEY, false);
	rKey.RemoveKey(m_name);
	dprint4("VS_TranscoderControl deleted name = %s; pointer = %p\n", m_name.c_str(), this);
}
const std::string& VS_TranscoderControl::GetTranscoderID() const
{
	return m_name;
}

bool VS_TranscoderControl::Start(const char *serverAddrs)
{
	wchar_t tcFile[MAX_PATH + 256];
	GetModuleFileNameW(NULL, tcFile, MAX_PATH);
	std::wstring cmd_line = tcFile;
	size_t pos = cmd_line.find_last_of(L"\\");
	VS_WideStr s = cmd_line.substr(0,pos+1).c_str();
	if(!serverAddrs || !*serverAddrs)
		return false;

	STARTUPINFOW   si;	memset( (void *)&si, 0, sizeof(si) );	si.cb = sizeof(si);

	VS_SimpleStr arg = "\"tc_transcoder.exe\" ";
	arg += m_name.c_str();
	arg += " ";
	arg += serverAddrs;

	dprint4("VS_TranscoderControl::Start name = %s; serverAddrs = %s; args = %s\n", m_name.c_str(), serverAddrs, arg.m_str);
	VS_WideStr exe_str = s;
	exe_str+=L"tc_transcoder.exe";
	VS_WideStr argW;
	argW.AssignStr(arg);
	return m_isStarted = !!CreateProcessW(exe_str,argW,0,0,FALSE,CREATE_NEW_CONSOLE/*CREATE_NO_WINDOW*/,0,0,&si,&m_pi);
}

void VS_TranscoderControl::Timeout()
{
	TouchProcess();
	CheckPreparation();
}


void VS_TranscoderControl::CheckPreparation()
{
	if(TryLock())
	{
		if(m_state == e_pending_clearing && MAX_TIME_FOR_CALL_PREPARATION < GetTickCount() - m_clearing_start_tick)
			Stop();
		else if(m_preparation_start_tick!=0 && MAX_TIME_FOR_CALL_PREPARATION < GetTickCount() - m_preparation_start_tick)
		{
			m_preparation_start_tick = 0;
			ClearCall();
			UnLock();
			InviteReplyFromVisi(e_call_none, false);
			return;
		}
		UnLock();
	}
}

bool VS_TranscoderControl::IsReady() const
{
	return !!m_pi.hProcess && !m_name.empty() && e_stand_by == m_state && !m_user_login;
}

void VS_TranscoderControl::Stop()
{
	dprint4("VS_TranscoderControl::Stop name = %s\n", m_name.c_str());
	if(m_pi.hProcess)
		TerminateProcess(m_pi.hProcess,0);
	m_state = e_not_init;
}

void VS_TranscoderControl::TouchProcess()
{
	if(!TryLock())
		return;
	bool IsZombie(false);
	std::function< void ( bool ) > cb;
	if(m_pi.hProcess)
	{
		if(WAIT_TIMEOUT!=WaitForSingleObject(m_pi.hProcess,0))
		{
			CloseHandle(m_pi.hProcess);
			CloseHandle(m_pi.hThread);
			memset( (void *)&m_pi, 0, sizeof(m_pi) );
			IsZombie = true;
			m_state = e_not_init;
		}
	}
	else
	{
		IsZombie = true;
		cb = m_fireOnLogin;
		m_fireOnLogin = {};
	}
	UnLock();
	if(IsZombie)
	{
		if(cb) cb(false);
		m_fireOnZombie(m_name);
	}
}

bool VS_TranscoderControl::TranscoderInit(const VS_SimpleStr &cid, VS_Container &cnt)
{
	m_CID = cid;
	dprint4("VS_TranscoderControl::TranscoderInit name = %s\n", m_name.c_str());
	m_state = e_stand_by;
	return true;
}

bool VS_TranscoderControl::SetMediaChannels(const std::vector<VS_MediaChannelInfo>& channels)
{
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, SETMEDIACHANNELS_METHOD);
	for (const auto& channel: channels)
		channel.Serialize(cnt, "MediaChannel");
	m_srv->SendToUnAuthTranscoder(m_CID, cnt);
	return true;
}

void VS_TranscoderControl::SetMediaChannelsResponse(VS_Container &cnt)
{
	int32_t res(0);
	cnt.GetValue(RESULT_PARAM, res);
	if (!res)
		return;

	std::vector<VS_MediaChannelInfo> channels;
	cnt.Reset();
	while (cnt.Next())
	{
		if (strcmp(cnt.GetName(), "MediaChannel") == 0)
		{
			channels.emplace_back(0);
			if (!channels.back().Deserialize(cnt, nullptr))
				return;
		}
	}
	std::string dialog_id;
	{
		VS_AutoLock lock(&m_dialog_id_lock);
		if (m_dialog_id_lst.empty()) return;
		dialog_id = m_dialog_id_lst.back();
	}
	m_fireSetMediaChannels(dialog_id, channels, 0);
}

VS_CallInviteStatus VS_TranscoderControl::InviteMethod(string_view fromId, string_view toId, const VS_ConferenceInfo& /*info*/, bool ipV4, bool /*newSession*/, bool /*forceCreate*/)
{
	{
		VS_AutoLock lock(this);
		if(m_state!=e_stand_by)
			return VS_CallInviteStatus::FAILURE;
		m_state = e_pending_preparation;
		m_tc_participant = StringViewToSimpleStr(toId);
		m_terminal_participant = StringViewToSimpleStr(fromId);
		dstream3 << "VS_TranscoderControl::InviteMethod name = " << m_name << "; from_id = " << fromId << "; to_id = " << toId;
		m_preparation_start_tick = GetTickCount();
	}

	if(!SendPrepareForCall(fromId, fromId, ipV4))
	{
		VS_AutoLock lock(this);
		m_tc_participant.Empty();
		m_terminal_participant.Empty();
		m_preparation_start_tick = 0;
		m_state = e_stand_by;
		return VS_CallInviteStatus::FAILURE;
	}
	return VS_CallInviteStatus::SUCCESS;
}

void VS_TranscoderControl::PrepareForCallResponse(VS_Container &cnt)
{
	{
		VS_AutoLock lock(this);
		if(m_state == e_in_call)
			return;
		else if(m_state!=e_pending_preparation)
		{
			ClearCall();
			return;
		}
		else
			m_state = e_in_call;
	}
	int32_t res(0);
	cnt.GetValue(RESULT_PARAM,res);
	m_last_login = cnt.GetStrValueRef(CALLID_PARAM);

	if(m_preparation_start_tick!=0)
	{
		m_preparation_start_tick =0;
		if(!res || !ProceedInviteMethod())
		{
			InviteReplyFromVisi(e_call_none, false);
			VS_AutoLock lock(this);
			m_state = e_stand_by;
		}
		else
		{
			VS_AutoLock lock(this);
			m_state = e_in_call;
		}
	}
	else
	{
		{
			VS_AutoLock lock(this);
			m_state = res==1 ? e_in_call : e_stand_by;
		}
		SetEvent(m_transcoderPreparedEvent);
	}
}

void VS_TranscoderControl::ClearCallResponse(VS_Container &cnt)
{
	int32_t cause(0);
	cnt.GetValue(CAUSE_PARAM,cause);
	std::function< void ( bool ) > cb;
	std::function<void (void)>	lcb;
	if(cause == 1)
	{
		{
			VS_AutoLock lock(this);
			cb = m_fireOnLogin;
			m_fireOnLogin = {};
			lcb = m_logout_cb;
			m_user_login = m_extertnal_name = "";
		}

		std::string dialog_id;
		{
			VS_AutoLock lock(&m_dialog_id_lock);
			if (m_dialog_id_lst.empty()) return;
			dialog_id = m_dialog_id_lst.back();
		}
		m_fireLoggedOutAsUser(dialog_id);

		if (cb)
			cb(true);
		if(lcb)
			lcb();
		return;
	}
	VS_AutoLock lock(this);
	if(m_state != e_pending_clearing)
		return;
	m_state = e_stand_by;
	m_clearing_start_tick = 0;
}

bool VS_TranscoderControl::ProceedInviteMethod()
{
	dprint3("VS_TranscoderControl::ProceedInviteMethod name = %s; from_id = %s; to_id = %s\n", m_name.c_str(), m_terminal_participant.m_str, m_tc_participant.m_str);

	VS_Container	cnt;
	cnt.AddValue(METHOD_PARAM,INVITE_METHOD);
	cnt.AddValue(FROM_PARAM,m_terminal_participant);
	cnt.AddValue(TO_PARAM,m_tc_participant);
	m_srv->SendToUnAuthTranscoder(m_CID,cnt);
	return true;
}

bool VS_TranscoderControl::InviteReply(VS_CallConfirmCode confirm_code)
{
	dprint3("VS_TranscoderControl::InviteReply name = %s; confirm_code = %d\n", m_name.c_str(), confirm_code);

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM,INVITEREPLY_METHOD);
	cnt.AddValueI32(RESULT_PARAM, confirm_code);
	m_srv->SendToUnAuthTranscoder(m_CID,cnt);

	if(confirm_code != e_call_ok)
		ClearCall();
	else
		if (!!m_user_login) m_state = e_in_call;

	return true;
}

void VS_TranscoderControl::Hangup()
{
	dprint3("VS_TranscoderControl::Hangup name = %s, state = %d\n", m_name.c_str(), m_state);
	VS_Container cnt;
	bool make_hangup(false);
	{
		VS_AutoLock lock(this);
		if(m_state == e_stand_by)
			return;
		make_hangup = m_state == e_in_call;
	}
	if(make_hangup)
	{
		cnt.AddValue(METHOD_PARAM, HANGUP_METHOD);
		m_srv->SendToUnAuthTranscoder(m_CID,cnt);
	}
	ClearCall();
}

void VS_TranscoderControl::HangupOutcomingCall()
{
	if(m_state == e_in_call)
		Hangup();
	else
		InviteReply(e_call_rejected);
}

void VS_TranscoderControl::FastUpdatePictureFromSIP()
{
	dprint4("VS_TranscoderControl::FastUpdatePictureFromSIP name = %s\n", m_name.c_str());
	VS_Container cnt;
	cnt.AddValueI32(H323_APP_DATA_TYPE, e_fastUpdatePicture);

	VS_Container cnt2;
	cnt2.AddValue(METHOD_PARAM, "SetAppData");
	cnt2.AddValue(DATA_PARAM, cnt);

	m_srv->SendToUnAuthTranscoder(m_CID,cnt2);
}

void VS_TranscoderControl::UpdateDisplayName(string_view displayName, bool updateImmediately)
{
	dprint4("VS_TranscoderControl::UpdateDisplayName name = %s\n", m_name.c_str());
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, USERREGISTRATIONINFO_METHOD);
	cnt.AddValue(DISPLAYNAME_PARAM, displayName);
	if (updateImmediately)
		cnt.AddValueI32(CMD_PARAM, 1);
	m_srv->SendToUnAuthTranscoder(m_CID, cnt);
}

void VS_TranscoderControl::BitrateRestrictionFromH323(int type, int value, int scope)
{
	dprint4("VS_TranscoderControl::BitrateRestrictionFromH323 name = %s, type = %d, value = %d\n", m_name.c_str(), type, value);
	VS_Container cnt;
	cnt.AddValueI32(H323_APP_DATA_TYPE, type);
	if (type == e_restrictBitRate)
	{
		cnt.AddValueI32(H323_APP_DATA_SOME_LONG_1, value);
		cnt.AddValueI32(H323_APP_DATA_SOME_LONG_2, scope);
	}

	VS_Container cnt2;
	cnt2.AddValue(METHOD_PARAM, "SetAppData");
	cnt2.AddValue(DATA_PARAM, cnt);

	m_srv->SendToUnAuthTranscoder(m_CID, cnt2);
}

void VS_TranscoderControl::HangupFromVisi()
{
	dprint3("VS_TranscoderControl::HangupFromVisi name = %s\n", m_name.c_str());
	std::string dialog_id;
	{
		VS_AutoLock lock(&m_dialog_id_lock);
		if (m_dialog_id_lst.empty()) return;
		dialog_id = m_dialog_id_lst.back();
	}
	m_fireHangupFromVisi(dialog_id, {});
	ClearCall();
}

void VS_TranscoderControl::FastUpdatePictureFromVisi()
{
	dprint4("VS_TranscoderControl::FastUpdatePictureFromVisi name = %s\n", m_name.c_str());
	std::string dialog_id;
	{
		VS_AutoLock lock(&m_dialog_id_lock);
		if (m_dialog_id_lst.empty()) return;
		dialog_id = m_dialog_id_lst.back();
	}
	m_fireFastUpdatePicture(dialog_id);
}
void VS_TranscoderControl::InviteReplyFromVisi(VS_CallConfirmCode confirmCode, bool isGroupConf)
{
	dprint3("VS_TranscoderControl::InviteReplyFromVisi name = %s; confirm_code = %d\n", m_name.c_str(), confirmCode);
	std::string dialog_id;
	{
		VS_AutoLock lock(&m_dialog_id_lock);
		if (m_dialog_id_lst.empty()) return;
		dialog_id = m_dialog_id_lst.back();
	}
	m_fireInviteReply(dialog_id, confirmCode, isGroupConf, {}, {});
}

void VS_TranscoderControl::InviteFromVisi(const char * from, const char* to, bool isGroupConf, const char* dn_from_utf8)
{
	dprint3("VS_TranscoderControl::InviteFromVisi name = %s; from = %s; to = %s;\n", m_name.c_str(), from, to);
	std::string dialog_id;
	{
		VS_AutoLock lock(&m_dialog_id_lock);
		if (m_dialog_id_lst.empty()) return;
		dialog_id = m_dialog_id_lst.back();
		m_tc_participant = from;
		m_terminal_participant = to;
	}
	if (!m_extertnal_name)
		m_fireInvite(dialog_id, from ? from : string_view{}, to ? to : string_view{},
			isGroupConf, false, false, dn_from_utf8 ? dn_from_utf8 : string_view{});
	else
	{
		// transcoder logged in as user, use username as dialog_id
		m_fireInvite( dialog_id, from ? from : string_view{},
			SimpleStrToStringView(m_extertnal_name), isGroupConf, false, true, dn_from_utf8 ? dn_from_utf8 : string_view{});
	}
}

void VS_TranscoderControl::SetDialogId(string_view dialogId)
{
	dstream4 << "VS_TranscoderControl::SetDialogId name = " << m_name << "; dialog_id = " << dialogId;
	VS_AutoLock lock(&m_dialog_id_lock);
	if (std::find(m_dialog_id_lst.cbegin(), m_dialog_id_lst.cend(), dialogId) == m_dialog_id_lst.cend())
	{
		m_dialog_id_lst.emplace_back(dialogId);
	}
}

void VS_TranscoderControl::ClearDialogId(string_view dialogId)
{
	VS_AutoLock lock(&m_dialog_id_lock);
	auto &&i = std::find(m_dialog_id_lst.cbegin(),m_dialog_id_lst.cend(), dialogId);
	if (i != m_dialog_id_lst.cend())
	{
		m_dialog_id_lst.erase(i);
	}
}

void VS_TranscoderControl::ChatFromVisi(const char* from, const char* to, const char* dn, const char* mess)
{
	dprint4("VS_TranscoderControl::ChatFromVisi name = %s; from = %s; to = %s; dn = %s\n", m_name.c_str(), from, to, dn);
	std::string dialog_id;
	{
		VS_AutoLock lock(&m_dialog_id_lock);
		if (m_dialog_id_lst.empty()) return;
		dialog_id = m_dialog_id_lst.back();
	}
	m_fireChat(dialog_id, from ? from : string_view{}, to ? to : string_view{}, dn ? dn : string_view{}, mess);
}

void VS_TranscoderControl::FECCFromVisi(const char* from, const char *to, eFeccRequestType type, long extra_param)
{
	dprint4("VS_TranscoderControl::FECCFromVisi name = %s, type = %d, from = %s, to = %s\n", m_name.c_str(), type, from, to);
	std::string dialog_id;
	{
		VS_AutoLock lock(&m_dialog_id_lock);
		if (m_dialog_id_lst.empty()) return;
		dialog_id = m_dialog_id_lst.back();
	}
	m_fireFECC(dialog_id, from ? from : string_view{}, to ? to : string_view{}, type, extra_param);
}

void VS_TranscoderControl::SendChatMessage(const std::string & to, const std::string & message)
{
	throw std::runtime_error("VS_TranscoderControl::SendChatMessage not implemented");
}

void VS_TranscoderControl::CommandFromVisi(const char *from, const char *command)
{
	dprint4("VS_TranscoderControl::CommandFromVisi name = %s; from = %s; command = %s\n", m_name.c_str(), from, command);
	std::string dialog_id;
	{
		VS_AutoLock lock(&m_dialog_id_lock);
		if (m_dialog_id_lst.empty()) return;
		dialog_id = m_dialog_id_lst.back();
	}
	m_fireCommand(dialog_id, from ? from : string_view{}, command ? command : string_view{});
}

VS_CallInviteStatus VS_TranscoderControl::PrepareForCall(string_view id, string_view idOrig, std::string& callId, bool isIpv4, VS_CallConfig::eUserStatusScheme /*userStatusScheme*/, bool /*createSession*/)
{
	if(id.empty())
	{
		return VS_CallInviteStatus::FAILURE;
	}

	{
		VS_AutoLock lock(this);
		if(m_state!=e_stand_by)
			return VS_CallInviteStatus::FAILURE;
		m_state = e_pending_preparation;
	}
	::ResetEvent(m_transcoderPreparedEvent);
	if(!SendPrepareForCall(id, idOrig, isIpv4))
	{
		VS_AutoLock lock(this);
		m_state = e_stand_by;
		return VS_CallInviteStatus::FAILURE;
	}

	const bool prepared = WaitForPrepare(5000);
	if(prepared && !!m_last_login && m_state == e_in_call)
	{
		callId = m_last_login;
		return VS_CallInviteStatus::SUCCESS;
	}

	ClearCall();
	return prepared ? VS_CallInviteStatus::FAILURE : VS_CallInviteStatus::TIME_OUT;
}

bool VS_TranscoderControl::SendPrepareForCall(string_view id, string_view idOrig, bool ipV4)
{
	if (id.empty())
	{
		return false;
	}

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM,PREPARETRANSCODERFORCALL_METHOD);
	cnt.AddValue(CALLID_PARAM,id);
	cnt.AddValue(ALIAS_PARAM, idOrig);
	cnt.AddValueI32(IPADDRESSFAMILY_PARAM, ipV4 ? VS_IPPortAddress::AddressFamily::ADDR_IPV4 : VS_IPPortAddress::AddressFamily::ADDR_IPV6);
	{
		auto transcoder_login = m_transLogin.lock();
		if (transcoder_login != nullptr)
		{
			auto pass = transcoder_login->GenerateTranscoderPass(id);
			cnt.AddValue(PASSWORD_PARAM, pass);
		}
	}
	m_srv->SendToUnAuthTranscoder(m_CID,cnt);
	return true;
}
bool VS_TranscoderControl::WaitForPrepare(const unsigned long ms)
{
	return WAIT_OBJECT_0 == WaitForSingleObject(m_transcoderPreparedEvent,ms);
}
void VS_TranscoderControl::ClearCall()
{
	if (!!m_user_login)
	{
		VS_AutoLock lock(this);
		if (m_state == e_in_call) m_state  = e_stand_by;
		return;
	}

	{
		VS_AutoLock lock(this);
		if(m_state == e_stand_by || m_state == e_pending_clearing)
			return;
		m_state = e_pending_clearing;
		m_clearing_start_tick = GetTickCount();
	}
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM,CLEARCALL_METHOD);
	m_srv->SendToUnAuthTranscoder(m_CID,cnt);
}

bool VS_TranscoderControl::IsLoggedIn(string_view login, bool exactMatch)
{
	if (m_user_login.Length() == 0) return false;
	if (exactMatch) {
		return SimpleStrToStringView(m_user_login) == login;
	}
	return boost::starts_with(m_user_login.m_str, login) && m_user_login.m_str[login.length()] == '/';
}

// ipV4:
//  true  - VS_IPPortAddress:ADDR_IPV4  - start transcoder's media connections on ipv4 address
//  false - VS_IPPortAddress:ADDR_IPV6 - start transcoder's media connections on ipv6 address
void VS_TranscoderControl::LoginUser(string_view login, string_view password, std::chrono::steady_clock::time_point expireTime,
	string_view externalName, std::function<void(bool)> resultCallback, std::function<void()> logoutCb,
	bool ipV4, const std::vector<std::string>& /*aliases*/)
{
	VS_AutoLock lock(this);

	if(login.empty() || password.empty())
	{
		resultCallback(false);
		return;
	}

	if ( m_user_login.Length() )
	{
		resultCallback( SimpleStrToStringView(m_user_login) == login);
		return;
	}	// password is checked at SIPParser already

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, LOGINUSER_METHOD);
	cnt.AddValue(LOGIN_PARAM, login);
	cnt.AddValue(PASSWORD_PARAM, password);
	cnt.AddValue(NAME_PARAM, m_name);
	cnt.AddValueI32(IPADDRESSFAMILY_PARAM, ipV4 ? VS_IPPortAddress::AddressFamily::ADDR_IPV4 : VS_IPPortAddress::AddressFamily::ADDR_IPV6);

	m_srv->SendToUnAuthTranscoder(m_CID,cnt);
	m_passwd = StringViewToSimpleStr(password);
	m_extertnal_name = StringViewToSimpleStr(externalName);

	m_fireOnLogin = std::move(resultCallback);
	m_logout_cb = std::move(logoutCb);
}

bool VS_TranscoderControl::LoginAsUserResponse( VS_Container &cnt )
{
	VS_AutoLock lock(this);
	bool res = !!(*cnt.GetLongValueRef( RESULT_PARAM ));
	if (res)
	{
		m_user_login = cnt.GetStrValueRef( CALLID_PARAM );
	} else m_user_login = m_extertnal_name = "";

	if (m_fireOnLogin) m_fireOnLogin( res );
	m_fireOnLogin = {};

	return res;
}

void VS_TranscoderControl::LogoutUser(std::function< void ( void ) > resultCallback)
{
	VS_AutoLock lock(this);
	m_fireOnLogin = {};

	if ( !m_user_login ) return;
	m_user_login = m_extertnal_name = "";
	m_logout_cb = resultCallback;

	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, LOGOUTUSER_METHOD);
	m_srv->SendToUnAuthTranscoder(m_CID,cnt);
	return;
}

void VS_TranscoderControl::SetUserEndpointAppInfo(string_view appName, string_view version)
{
	VS_AutoLock lock(this);
	VS_Container cnt;
	cnt.AddValue(METHOD_PARAM, SETUSERENDPAPPINFO_METHOD);
	cnt.AddValue(APPNAME_PARAM, appName);
	cnt.AddValue(CLIENTVERSION_PARAM, version);
	m_srv->SendToUnAuthTranscoder(m_CID, cnt);
}

void VS_TranscoderControl::ReleaseCallbacks()
{
	VS_AutoLock lock(this);
	m_fireOnLogin = {};
	m_logout_cb = {};
}

std::string VS_TranscoderControl::GetDialogID()
{
	throw std::exception( "VS_TranscoderControl::GetDialogID not implemented" );
}

std::string VS_TranscoderControl::GetConfID()
{
	throw std::exception( "VS_TranscoderControl::GetConfID not implemented" );
}

std::string VS_TranscoderControl::GetStreamConfID() const
{
	throw std::exception("VS_TranscoderControl::GetStreamConfID not implemented");
}

std::string VS_TranscoderControl::GetTrueconfID()
{
	throw std::exception( "VS_TranscoderControl::GetTrueconfID not implemented" );
}

std::string VS_TranscoderControl::GetOwner()
{
	throw std::exception( "VS_TranscoderControl::GetOwner not implemented" );
}

bool VS_TranscoderControl::IsGroupConf() const {
	throw std::exception("VS_TranscoderControl::IsGroupConf not implemented");
}

std::string VS_TranscoderControl::GetPeer() const {
	throw std::exception("VS_TranscoderControl::GetPeer not implemented");
}
#endif