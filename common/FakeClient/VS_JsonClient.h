#pragma once

#include "std/cpplib/json/elements.h"
#include "VS_FakeClient.h"
#include "std/cpplib/VS_Lock.h"
#include "std-generic/compat/map.h"

class VS_JsonClient :
	public VS_FakeClient, public VS_Lock
{
	std::weak_ptr<void> m_responce_instance;
	boost::function< bool(const json::Object&) > m_responce_handler;
	vs::map<std::string, VS_Participant_Role> m_rolequery;
	vs::event	m_waitLogoff {true};
	uint32_t m_webversion;
protected:
	VS_JsonClient(std::unique_ptr<VS_FakeEndpoint> endpoint);
	static void PostConstruct(const std::shared_ptr<VS_JsonClient>& p)
	{
		VS_FakeClient::PostConstruct(p);
	}

public:
	virtual ~VS_JsonClient(void);
	static const char* Version() { return "28"; } // see https://projects.trueconf.com/bin/view/Projects/JSONInterfaceToServer

	bool Request( json::Object &req, const std::string &ip);
	void OnError(unsigned error) override;
	void onWSError(unsigned err);

	void SetResponceCallBack(std::shared_ptr<void> &&holder, boost::function< bool(const json::Object&) > &&handler);
	void RegisterAtWebrtcSRV();

private:
	void DispatchResponse(const json::Object&obj);

	bool ProcessRequest(json::Object &req, const std::string &ip);
	void SendCurrentState();
	void SendAppProps();

	// virtual from VS_FakeClient

	void onLoginResponse(VS_UserLoggedin_Result result) override;
	void onLogoutResponse(VS_UserLoggedout_Cause cause) override;
	void onConferenceStateChange(const char *method, const VS_ConferenceDescriptor &conf) override;
	void JoinStreamConference(VS_ConferenceDescriptor &d) override;
	void LeaveStreamConference(VS_ConferenceDescriptor &d) override;
	void onCommand(const char *from, const char *command) override;
	void onChat(const char *to, const char *conf, const char *from, const char *dname, const char *text) override;
	void onFile(const char *to, const char *conf, const char *from, const char *dname, const char *text, const char* fname, const char* link, const char* url, const char* about) override;
	void onPartList(const VS_ConferenceDescriptor &conf, const char* user, int32_t type) override;
	void onRoleEvent(const VS_ConferenceDescriptor &conf, VS_RoleEvent_Type type, VS_Participant_Role role, VS_Broadcast_Status bs, VS_RoleInqury_Answer result, const char* user) override;
	void onDeviceStatus(const char *name, long device_status) override;
	void onListenersFltr(int32_t fltr) override;
	void onChangeDevice(string_view type, string_view id, string_view name) override;
	void onSetDeviceMute(string_view type, string_view id, bool mute) override;

	void onRequestKeyFrame() override;
	void onReqInvite(const char* from, const char* dn) override;
		// internal
	void FillJsonByConference(json::Object &obj, const VS_ConferenceDescriptor &conf) const;
	void EnshureClientAlive(); // called by timeout, checks pending connections
	void onWebPeerMessage(json::Object&& obj);
};
