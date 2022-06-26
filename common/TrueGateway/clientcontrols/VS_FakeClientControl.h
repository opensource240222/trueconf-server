#pragma once

#include "FakeClient/VS_FakeClientInterface.h"
#include "TransceiverLib/VS_RTPModuleControlInterface.h"
#include "std/cpplib/event.h"
#include "std-generic/cpplib/atomic_shared_ptr.h"
#include "std-generic/compat/boost/enable_shared_from_this.hpp"
#include "VS_ClientControlInterface.h"

#include <vector>
#include <mutex>

struct SlideInfo;
namespace ts { struct IPool; }
class VS_TranscoderLogin;
struct DeviceInfo;

class VS_FakeClientControl : public VS_ClientControlInterface,
	public vs_boost::enable_shared_from_this<VS_FakeClientControl>
{
	VS_CallConfig::eUserStatusScheme m_user_status_scheme = VS_CallConfig::eUserStatusScheme::ONLY_USER_AVAIL;
	bool CreateConf(const VS_ConferenceInfo& info);
	void DoInviteAsync();
public:
	friend struct FakeClientControlTest;
	VS_FakeClientControl(const std::weak_ptr<ts::IPool> &pool, const std::weak_ptr<VS_TranscoderLogin>& transLogin);
	~VS_FakeClientControl();

	void SetFakeClientInterface(const std::shared_ptr<VS_FakeClientInterface>& fakeClient);
	void SetRTPModuleInterface(const std::shared_ptr<VS_RTPModuleControlInterface> &rtpModuleConrol) override;

	const std::string& GetTranscoderID() const override;
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
	VS_CallConfig::eUserStatusScheme GetUserStatusScheme() const override;

	bool SetMediaChannels(const std::vector<VS_MediaChannelInfo>& channels) override;
	void SetProxyReservation(const std::string& reservationToken) override;

	VS_CallInviteStatus PrepareForCall(string_view id, string_view idOrig, std::string& callId, bool isIpv4,
		VS_CallConfig::eUserStatusScheme userStatusScheme = VS_CallConfig::eUserStatusScheme::ONLY_USER_AVAIL, bool createSession = false) override;
	VS_CallInviteStatus InviteMethod(string_view fromId, string_view toId, const VS_ConferenceInfo &info,
		bool ipV4, bool newSession = true, bool forceCreate = false) override;
	bool InviteReply(VS_CallConfirmCode confirm_code) override;
	void Hangup() override;
	void SendChatMessage(const std::string& to, const std::string& message) override;

	bool KickFromConference(string_view fromId, string_view toId) override;
	bool ReqInviteMethod(string_view fromId, string_view toId, bool ipV4,
		bool newSession = true) override;
	bool ReqInviteReply(string_view fromId, bool accept) override;

	void TakeTribune() override;
	void LeaveTribune() override;
	VS_Participant_Role GetMyTribuneRole() const override;
	VS_Participant_Role GetMyConferenceRole() const override;
	void InviteToTribune(const std::string &to_id) override;
	void ExpelFromTribune(const std::string &to_id) override;
	void TakeRemark() override;
	void LeaveRemark() override;

	void HangupOutcomingCall() override;

	void FastUpdatePictureFromSIP() override;
	void UpdateDisplayName(string_view displayName, bool updateImmediately) override;
	void BitrateRestrictionFromH323(int type, int value, int scope) override {}

	bool IsLoggedIn(string_view login, bool exactMatch = true) override;

	void LoginUser(string_view login, string_view password, std::chrono::steady_clock::time_point expireTime, string_view externalName,
		std::function< void(bool) > resultCallback, std::function< void(void)> logoutCb, bool ipV4, const std::vector<std::string>& aliases) override;

	void LogoutUser(std::function< void(void) > resultCallback) override;

	void SetUserEndpointAppInfo(string_view /*appName*/, string_view /*version*/) override {}

	void ReleaseCallbacks() override;

	void ConnectUpdateStatus(const UpdateStatusSlot &slot) override;

	std::string GetAppProperty(const std::string &prop_name) override;

protected:
	bool WaitForLoginComplete();
	bool CreateRTPSession();
	void DestroyRTPSession();
	void CheckVisibleParticipants(const VS_FakeClientInterface::VS_ConferenceDescriptor& conf);
	bool IsVideoNeeded(const VS_FakeClientInterface::VS_ConferenceDescriptor& conf);
	void UpdateVideoState(const VS_FakeClientInterface::VS_ConferenceDescriptor& conf);
	void HandleVideoChangeRequest(const char* from, eSDP_ContentType content);
	void HandleDeviceControlRequest(const char* from, boost::optional<bool> audio, boost::optional<bool> video);
	void ReqFastUpdatePictureFromSIP();
	void SetDevicesList(const std::vector<VS_MediaChannelInfo>& channels);
	void AddFakeDevices();
	void AddFakeSlidesDevice();

	// m_fakeClient Callbacks
	void JoinStreamConference(VS_FakeClientInterface::VS_ConferenceDescriptor &d);
	void LeaveStreamConference(VS_FakeClientInterface::VS_ConferenceDescriptor &d);
	void onLoginResponse(VS_UserLoggedin_Result res);
	void onLogoutResponse();
	void onConferenceStateChange(const char *method, const VS_FakeClientInterface::VS_ConferenceDescriptor &conf);
	void onCommand(const char *from, const char *command);
	void onChat(const char *to, const char *from, const char *dname, const char *text);
	void onFile(const char *to, const char *from, const char *dname, const char *text, const char * fileName, const char * magnet, const char * url, const char * about);
	void onPartList(const VS_FakeClientInterface::VS_ConferenceDescriptor &conf);
	void onRoleEvent(const VS_FakeClientInterface::VS_ConferenceDescriptor &conf, VS_RoleEvent_Type type, VS_Participant_Role receivedRole, VS_Broadcast_Status bs, VS_RoleInqury_Answer result, const char* user);
	void onFECC(const char *from, const char *to, eFeccRequestType type, long extra_param);
	void onUpdateStatus(const char *name, VS_UserPresence_Status status);
	void ReqInvite(const char *name);
	void onChangeDevice(string_view type, string_view id, string_view name);
	void onSetDeviceMute(string_view type, string_view id, bool mute);

	// m_RTPModuleControl callbacks
	void onSetMediaChannels(string_view id, const std::vector<VS_MediaChannelInfo>& channels);
	void onFullIntraframeRequest(string_view id);
	void onVideoStatus(string_view id, eSDP_ContentType content, bool slides_available);
	void onDeviceStatus(string_view id, uint32_t value);
	void onShowSlide(string_view id, const char* url, const SlideInfo &info);
	void onEndSlideShow(string_view id);
	void onFECCToVisi(string_view id, eFeccRequestType type, long extra_param);

private:
	VS_ClientCaps GetMyClientCaps();

	std::shared_ptr<VS_FakeClientInterface> m_fakeClient;

	std::string m_groupconf_username;
	std::string m_mconf_prefix;

	std::vector<std::string> m_dialog_ids;
	std::vector<std::string> m_invite_later;

	VS_Participant_Role m_last_tribune_role = PR_COMMON;
	VS_Participant_Role m_last_conference_role = PR_COMMON;
	VS_Participant_Role m_last_queried_tribune_role = PR_COMMON;
	void ChangeTribuneRoleTo(const VS_Participant_Role);

	std::string m_lastLogin;
	std::string m_externalName;
	std::chrono::steady_clock::time_point m_expireTime;
	std::function< void(bool) > m_loginCallback;
	std::function< void(void)> m_logoutCallback;

	vs::event	m_waitForLoginComplete;
	mutable std::mutex	m_lock;
	std::string			m_transceiverReservationToken;
	std::weak_ptr<ts::IPool>	m_transcPool;
	std::weak_ptr<VS_TranscoderLogin> m_transLogin;
	vs::atomic_shared_ptr< const std::vector<DeviceInfo>> m_DevicesList;
	bool m_slides_initialized;
	bool m_camera_enabled;
	bool m_disable_role_notification = false;
	mutable bool m_waitForLoginEventFlag;
};
