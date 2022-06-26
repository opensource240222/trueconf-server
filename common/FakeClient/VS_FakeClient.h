#pragma once

#include "FakeClient/VS_FakeClientInterface.h"
#include "FakeClient/VS_FakeEndpoint.h"
#include "std/cpplib/event.h"
#include "std/cpplib/fast_mutex.h"
#include "std-generic/cpplib/macro_utils.h"
#include "std-generic/cpplib/string_view.h"
#include "std/statistics/TConferenceStatistics.h"

#include <chrono>
#include <list>
#include <mutex>
#include <string>
#include <vector>

class VS_FakeClient
	: public VS_FakeClientInterface
	, public VS_FakeEndpoint::Receiver
	, public vs::enable_shared_from_this<VS_FakeClient>
{
	friend struct FakeClientControlTest;
protected:
	VS_FakeClient(std::unique_ptr<VS_FakeEndpoint> endpoint);
	static void PostConstruct(const std::shared_ptr<VS_FakeClient>& p)
	{
		p->m_endpoint->SetReceiver(p);
	}

public:
	virtual ~VS_FakeClient(void);

	void LoginUserAsync(string_view login, string_view passwd, string_view passwdMD5, VS_ClientType clienttype, string_view ip, const std::vector<std::string> &aliases) override;

	void Logout() override;

	void SetDefaultCaps(const VS_ClientCaps& caps) override;
	bool CreateConference(long maxPart, VS_Conference_Type confType, long subType, const char* name, const std::string& topic, const char* passwd, bool is_public, const std::string& transcReserveToken) override;
	bool JoinAsync(const std::string& to, const VS_ConferenceInfo& info, const std::string& transcReserveToken) override;
	virtual bool InviteAsync(const std::string &to, bool force_create, const std::string& transcReserveToken) override;
	void SendChatMessage(const std::string& to, const std::string& msg) override;
	void SendChatMessage(const std::string& stream_conf_id, const std::string& to, const std::string& msg) override;
	void SendChatCommand(const std::string& to, const std::string& msg, const VS_Container& cnt = {}) override;
	void SendFECC(string_view to, eFeccRequestType type, std::int32_t extraParam) override;
	std::string GetAppProperty(const std::string& name) override;
	void SetAppProperty(const std::string& name, const std::string& val) override;
	void SendAppProperties() override;
	const std::string& CID() const override;
	void SetAlias(string_view toId) override;
	void SetAppId(const char* appname) override;
	void UpdateDisplayName(string_view displayName, bool updateImmediately) override;
	void SendDeviceStatus(unsigned value) override;
	bool KickFromConference(string_view to) override;
	bool ReqInviteAsync(string_view to) override;
	bool AnswerReqInvite(string_view to, bool accept) override;

	bool QueryRole(const char* name, long role) override;
	bool AnswerRole(const char* name, long role, long result) override;
	bool ConnectSender(long fltr) override;

	bool ManageLayoutFunc(const char* func, const char* id1, const char* id2) override;
	bool ManageLayoutPT(const long pt) override;

	bool SendUsageStat() override;

	void DevicesList(string_view type, const std::vector < std::pair < std::string, std::string>> &list) override;
	void DeviceChanged(string_view type, string_view id, string_view name) override;
	void DeviceMute(string_view type, string_view id, bool mute) override;
	void DeviceVolume(string_view type, string_view id, int32_t volume) override;

	VS_FakeEndpoint& GetEndpoint() const override;

	std::shared_ptr<VS_ConferenceDescriptor> GetCurrentConference() override;
	ClientState GetClientState() override;
	std::string GetTrueconfID() override;

protected:
	bool JoinAsync(const std::string &to, const VS_ConferenceInfo& info, const VS_ClientCaps &caps, const std::string& transcReserveToken);

	void StartWaiting(std::string id, std::chrono::steady_clock::duration delay, std::function<void ()> handler);
	bool CancelWait(string_view id);
	bool CallNowAndCancelWait(string_view id);
	bool IsWaitingFor(string_view id);
	VS_Container GetPropsFromServer();

	std::unique_ptr<VS_FakeEndpoint> m_endpoint;
	ClientState m_state;
	TConferenceStatistics	m_conf_stat;

private:
	void LoginUserAllowed(bool is_allowed, const std::string &login,
								const std::string &passwd, const std::string &passwd_md5,
								VS_ClientType clienttype, const std::string &ip, const std::vector<std::string> &aliases);

	bool TimerOperationInternal(const std::string &id, bool call, bool remove);

	bool	m_IsGuest;
	bool isAllowedToSend(string_view id);

	void OnReceive(const transport::Message& msg) override;
	void OnError(unsigned error) override;
	void Timeout() override;

	void _onLoginResponse(VS_Container &cnt);
	void _onLogoutResponse(VS_Container &cnt);
	void _onConfCreateResponse(VS_Container &cnt);
	void _onJoinResponse(VS_Container &cnt);
	void _onRoleEvent(VS_Container &cnt);
	void _onReqInvite(VS_Container &cnt);
	void _onDeleteConference(VS_Container &cnt);
	void _onUserRegistrationInfo(VS_Container &cnt);
	void _onSendCommandToConfSrv(VS_Container &cnt);
	void _onChat(VS_Container &cnt);
	void _onCommand(VS_Container &cnt);
	void _onInite(VS_Container &cnt);
	void _onIniteToMulti(VS_Container &cnt);
	void _onAccept(VS_Container &cnt);
	void _onPartList(VS_Container &cnt);
	void _onSetProperties(VS_Container &cnt);
	void _onFECC(VS_Container &cnt);
	void _onReject(VS_Container &cnt);
	void _onDeviceStatus(VS_Container &cnt);
	void _onListenersFltr(VS_Container &cnt);

	void _onChangeDevice(VS_Container &cnt);
	void _onSetDeviceState(VS_Container &cnt);

	void TransportPing();
	void PingConferences();
	void UpdateStatus();

	bool Accept(const std::shared_ptr<VS_ConferenceDescriptor>& conf) override;
	bool Reject(const std::shared_ptr<VS_ConferenceDescriptor>& conf, VS_Reject_Cause reason) override;
	bool Hangup(const std::shared_ptr<VS_ConferenceDescriptor>& conf, bool forall) override;

	bool CreateConference(VS_ConferenceDescriptor &d, long maxPart, VS_Conference_Type confType, long subType, const char *name, const std::string &topic, const char *passwd, bool is_public, const std::string& transcReserveToken);
	bool InviteToConference(VS_ConferenceDescriptor &d, const char *to);

	void ComposeSend(VS_Container &cnt, const char* service,
									const char* server = 0, const char *user = 0, unsigned long timeout = 20 * 1000);

	std::mutex m_timeout_handlers_mutex;
	struct timeout_info
	{
		VS_FORWARDING_CTOR3(timeout_info, id, expire_time, handler) {}
		std::string id;
		std::chrono::steady_clock::time_point expire_time;
		std::function<void ()> handler;
	};
	std::list<timeout_info> m_timeout_handlers;

	void initDefaultCodecs();
	bool CapsFromContainer(VS_Container &cnt, VS_ClientCaps &caps);
	bool CapsToContainer(VS_Container &cnt, const VS_ClientCaps &caps);

	VS_Container m_propertiesFromServer;
	vs::event	 m_server_properties_arrived { true };
	friend struct VS_ConferenceDescriptor;

	std::string								m_display_name_for_update;
	std::string								m_alias;
	bool									m_update_dn_immediately;
	void UpdateDisplayNameImp(string_view displayName, bool updateImmediately);

	std::vector<VS_Container>				m_req_invites;

	vs::fast_recursive_mutex m_lock;	// TODO: make different locks (not recursive, if possible) for different purposes, not one for all cases
	//SIP To TC
	eFeccRequestType m_feccState;
};
