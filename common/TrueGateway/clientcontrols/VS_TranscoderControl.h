#pragma once

#include "VS_ClientControlInterface.h"
#include "acs/Lib/VS_IPPortAddress.h"
#include "std/cpplib/VS_SimpleStr.h"
#include "std/cpplib/VS_Lock.h"
#include "std/cpplib/VS_Protocol.h"

#include <string>
#include <vector>

class VS_TranscodersDispatcher;
class VS_Container;
class VS_H281Frame;
class VS_TranscoderLogin;

class VS_TranscoderControl : public VS_ClientControlInterface, VS_Lock
{
public:
	virtual ~VS_TranscoderControl();
	const std::string& GetTranscoderID() const override;

	bool Start(const char *server_addrs);//Start process
	void Stop();

	bool IsReady() const override;
	void SetDialogId(string_view dialogId) override;
	void ClearDialogId(string_view dialogId) override;
	std::string GetDialogID() override;

	std::string GetConfID() override;
	std::string GetStreamConfID() const override;
	std::string GetTrueconfID() override;
	std::string GetOwner() override;
	bool IsGroupConf() const override;
	std::string GetPeer() const override;

	bool TranscoderInit(const VS_SimpleStr &cid, VS_Container &cnt);
	bool SetMediaChannels(const std::vector<VS_MediaChannelInfo>& channels) override;
	void SetMediaChannelsResponse(VS_Container &cnt);

	// isIpv4:
	//  true  -  VS_IPPortAddress:ADDR_IPV4  - start transcoder's media connections on ipv4 address
	//  false - VS_IPPortAddress:ADDR_IPV6 - start transcoder's media connections on ipv6 address
	VS_CallInviteStatus PrepareForCall(string_view id, string_view idOrig, std::string& callId, bool isIpv4,
		VS_CallConfig::eUserStatusScheme userStatusScheme = VS_CallConfig::eUserStatusScheme::ONLY_USER_AVAIL, bool createSession = false) override;
	bool WaitForPrepare(const unsigned long ms);
	void ClearCall();

	void PrepareForCallResponse(VS_Container &cnt);
	void ClearCallResponse(VS_Container &cnt);

	// ipV4:
	// true  -  VS_IPPortAddress:ADDR_IPV4  - start transcoder's media connections on ipv4 address
	// false -  VS_IPPortAddress:ADDR_IPV6 - start transcoder's media connections on ipv6 address
	VS_CallInviteStatus InviteMethod(string_view fromId, string_view toId, const VS_ConferenceInfo& info, bool ipV4, bool newSession,
		bool forceCreate) override;

	bool InviteReply(VS_CallConfirmCode confirm_code) override;

	void SendChatMessage(const std::string& to, const std::string& message) override;

	void Hangup() override;

	bool KickFromConference(string_view /*fromId*/, string_view /*toId*/) override { return false; }

	bool ReqInviteMethod(string_view /*fromId*/, string_view /*toId*/, bool /*ipV4*/, bool /*newSession*/) override { return false; }

	bool ReqInviteReply(string_view /*fromId*/, bool /*accept*/) override { return false; }

	// Used for calncel an outcoming call.
	// If call was not established yet, invokes InviteReply() with 'reject' state.
	// If call was already established, invokes Hangup().
	// For h323 call.
	void HangupOutcomingCall() override;
	void FastUpdatePictureFromSIP() override;
	void UpdateDisplayName(string_view displayName, bool updateImmediately) override;
	void BitrateRestrictionFromH323(int type, int value, int scope) override;

	void HangupFromVisi();

	bool IsLoggedIn(string_view login, bool exactMatch = true) override;
	// ipV4:
	//  true  -  VS_IPPortAddress:ADDR_IPV4  - start transcoder's media connections on ipv4 address
	//  false -  VS_IPPortAddress:ADDR_IPV6 - start transcoder's media connections on ipv6 address
	void LoginUser(string_view login, string_view password, std::chrono::steady_clock::time_point expireTime,
		string_view externalName, std::function<void(bool)> resultCallback, std::function<void()> logoutCb,
		bool ipV4, const std::vector<std::string>& aliases) override;

	bool LoginAsUserResponse(VS_Container &cnt);
	void LogoutUser(std::function< void(void) > resultCallback) override;

	void SetUserEndpointAppInfo(string_view appName, string_view version) override;

	void FastUpdatePictureFromVisi();
	void InviteReplyFromVisi(VS_CallConfirmCode confimCode, bool isGroupConf);
	void InviteFromVisi(const char* from, const char* to, bool isGroupConf, const char* dn_from_utf8);

	void Timeout();
	void ChatFromVisi(const char* from, const char* to, const char* dn, const char* mess);
	void CommandFromVisi(const char *from, const char *command);
	void FECCFromVisi(const char *from, const char *to, eFeccRequestType type, long extra_param);
	void FillMediaChannelInfoMap(VS_Container & cnt);

	void ReleaseCallbacks() override;

private:
	enum InternalTranscoderState
	{
		e_not_init,
		e_stand_by,
		e_pending_preparation,
		e_in_call,
		e_pending_clearing
	}	m_state;

	std::string m_name;
	std::vector<std::string>	m_dialog_id_lst;

	VS_SimpleStr				m_last_login;
	VS_SimpleStr				m_CID;
	VS_SimpleStr				m_tc_participant;
	VS_SimpleStr				m_terminal_participant;
	VS_SimpleStr				m_user_login;
	VS_SimpleStr				m_passwd;
	VS_SimpleStr				m_extertnal_name;

	VS_Lock						m_dialog_id_lock;
	PROCESS_INFORMATION			m_pi;
	HANDLE						m_transcoderPreparedEvent; ////???
	unsigned long				m_preparation_start_tick;
	unsigned long				m_clearing_start_tick;

	bool						m_isStarted;
	VS_TranscodersDispatcher*	m_srv;
	std::weak_ptr <VS_TranscoderLogin> m_transLogin;

	std::function< void ( bool ) >	m_fireOnLogin;
	std::function< void (void) >		m_logout_cb;

	// ipV4:
	// true  -  VS_IPPortAddress:ADDR_IPV4  - start transcoder's media connections on ipv4 address
	// false -  VS_IPPortAddress:ADDR_IPV6 - start transcoder's media connections on ipv6 address

	bool SendPrepareForCall(string_view id, string_view idOrig, bool ipV4);
	bool ProceedInviteMethod();
	void TouchProcess();
	void CheckPreparation();

	VS_TranscoderControl(const char* name, VS_TranscodersDispatcher* srv, const std::weak_ptr <VS_TranscoderLogin>& transLogin);

	friend class boost::signals2::deconstruct_access;

	template<typename T> friend
	void adl_postconstruct(const boost::shared_ptr<T>& p, VS_TranscoderControl*);
};

template<typename T>
void adl_postconstruct(const boost::shared_ptr<T>& p, VS_TranscoderControl*)
{
}
