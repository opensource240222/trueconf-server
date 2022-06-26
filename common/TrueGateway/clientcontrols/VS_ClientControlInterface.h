#pragma once

#include "../../statuslib/status_types.h"
#include "std/cpplib/VS_Protocol.h"
#include "tools/Server/CommonTypes.h"
#include "TrueGateway/CallConfig/VS_CallConfig.h"

#include <boost/signals2.hpp>

#include <string>
#include <vector>
#include "std-generic/attributes.h"

struct VS_MediaChannelInfo;
struct VS_ConferenceInfo;
class VS_RTPModuleControlInterface;

enum VS_CallConfirmCode :int;

enum class VS_CallInviteStatus;

class VS_ClientControlInterface
{
public:

	typedef boost::signals2::signal<void(string_view)> ZombieSignal;
	typedef ZombieSignal::slot_type	ZombieSignalSlot;

	typedef boost::signals2::signal<void(string_view, string_view)> HangupFromVisiSignal;
	typedef ZombieSignal FastUpdatePicSignal;
	typedef HangupFromVisiSignal::slot_type HangupFromVisiSignalSlot;
	typedef ZombieSignalSlot FastUpdatePicSignalSlot;
	typedef ZombieSignal LoggedOutAsUserSignal;
	typedef ZombieSignalSlot LoggedOutAsUserSignalSlot;

	typedef boost::signals2::signal<void(string_view dialogId, const std::vector<VS_MediaChannelInfo> &channels, long bandwRcv)> SetMediaChannelsSignal;
	typedef SetMediaChannelsSignal::slot_type SetMediaChannelsSignalSlot;

	typedef boost::signals2::signal<void(string_view dialogId, VS_CallConfirmCode confirmCode, bool isGroupConf, string_view confName, string_view to_displayName)> InviteReplaySignal;
	typedef InviteReplaySignal::slot_type InviteReplaySignalSlot;
	typedef boost::signals2::signal<void(string_view dialogId, string_view from, string_view to, bool isGroupConf, bool isPublicConf, bool useNewDialogId, string_view dnFromUtf8)> InviteSignal;
	typedef InviteSignal::slot_type InviteSignalSlot;

	typedef boost::signals2::signal<void(string_view dialogId, string_view from, string_view to, string_view dn, const char* mess)> ChatSignal;
	typedef ChatSignal::slot_type ChatSignalSlot;
	typedef boost::signals2::signal<void(string_view dialogId, string_view from, string_view to, string_view dn, const char *mess, const FileTransferInfo &)> FileSignal;
	typedef FileSignal::slot_type FileSignalSlot;
	typedef boost::signals2::signal<void(string_view dialogId, string_view from, string_view command)> CommandSignal;
	typedef CommandSignal::slot_type CommandSignalSlot;
	typedef boost::signals2::signal<void(string_view dialogId, string_view from, string_view to, eFeccRequestType type, long extraParam)> FECCSignal;
	typedef FECCSignal::slot_type FECCSignalSlot;
	typedef boost::signals2::signal<void(string_view callId, VS_UserPresence_Status status)> UpdateStatusSignal;
	typedef UpdateStatusSignal::slot_type UpdateStatusSlot;

	typedef boost::signals2::signal<void(string_view dialogId, string_view name)> ReqInviteSignal;
	typedef ReqInviteSignal::slot_type ReqInviteSlot;

	typedef boost::signals2::signal<void(string_view dialogId, bool result)> TakeTribuneReplySignal;
	typedef TakeTribuneReplySignal::slot_type TakeTribuneReplySlot;
	typedef boost::signals2::signal<void(string_view dialogId, bool result)> LeaveTribuneReplySignal;
	typedef LeaveTribuneReplySignal::slot_type LeaveTribuneReplySlot;

	virtual ~VS_ClientControlInterface(){}
	virtual const std::string& GetTranscoderID() const = 0;
	virtual bool IsReady() const = 0;
	virtual bool RtpControlIsReady() const { return m_RTPModuleControl != nullptr; };
	virtual void SetRTPModuleInterface(const std::shared_ptr<VS_RTPModuleControlInterface> &/*rtpModuleConrol*/) { assert(false); }
	virtual void SetProxyReservation(const std::string& /*reservationToken*/) { assert(false); }

	virtual void SetDialogId(string_view /*dialogId*/) = 0;

	virtual void ClearDialogId(string_view /*dialogId*/) = 0;

	virtual std::string GetDialogID() = 0;

	virtual std::string GetConfID() = 0;
	virtual std::string GetStreamConfID() const = 0;
	virtual std::string GetTrueconfID() = 0;
	virtual std::string GetOwner() = 0;
	virtual bool IsGroupConf() const = 0;
	virtual std::string GetPeer() const = 0;

	virtual bool SetMediaChannels(const std::vector<VS_MediaChannelInfo>& channels) = 0;

	virtual VS_CallInviteStatus PrepareForCall(string_view /*id*/, string_view /*idOrig*/, std::string&/*callId*/, bool /*isIpv4*/,
		VS_CallConfig::eUserStatusScheme /*userStatusScheme*/ = VS_CallConfig::eUserStatusScheme::ONLY_USER_AVAIL, bool /*createSession*/ = false) = 0;

	virtual VS_CallInviteStatus InviteMethod(string_view fromId, string_view toId, const VS_ConferenceInfo &info, bool ipV4, bool newSession = true, bool forceCreate = false) = 0;

	virtual bool InviteReply(VS_CallConfirmCode confirm_code) = 0;
	virtual void Hangup() = 0;
	virtual void SendChatMessage(const std::string& to, const std::string& message) = 0;

	virtual bool KickFromConference(string_view fromId, string_view toId) = 0;

	virtual bool ReqInviteMethod(string_view fromId, string_view toId, bool ipV4, bool newSession = true) = 0;

	virtual bool ReqInviteReply(string_view fromId, bool accept) = 0;

	virtual void TakeTribune() {}
	virtual void LeaveTribune() {}
	virtual VS_Participant_Role GetMyTribuneRole() const { return PR_COMMON; }
	virtual VS_Participant_Role GetMyConferenceRole() const { return PR_COMMON; }
	virtual void InviteToTribune(const std::string &/*to_id*/) {}
	virtual void ExpelFromTribune(const std::string &/*to_id*/) {}
	virtual void TakeRemark() {};
	virtual void LeaveRemark() {};

	// Used for calncel an outcoming call.
	// If call was not established yet, invokes InviteReply() with 'reject' state.
	// If call was already established, invokes Hangup().
	// For h323 call.
	virtual void HangupOutcomingCall()
	{

	}

	virtual void FastUpdatePictureFromSIP() = 0;

	virtual void UpdateDisplayName(string_view displayName, bool updateImmediately) = 0;

	virtual void BitrateRestrictionFromH323(int type, int value, int scope) = 0;

	virtual bool IsLoggedIn(string_view login, bool exactMatch = true) = 0;

	virtual void LoginUser(string_view login, string_view password, std::chrono::steady_clock::time_point expireTime, string_view externalName,
		std::function< void(bool) > resultCallback, std::function< void(void)> logoutCb, bool ipV4, const std::vector<std::string>& aliases) = 0;

	virtual void LogoutUser(std::function< void(void) > resultCallback) = 0;

	virtual void SetUserEndpointAppInfo(string_view appName, string_view version) = 0;

	virtual void ReleaseCallbacks() = 0;

	virtual std::string GetAppProperty(const std::string& /*propName*/) { return {}; }
	virtual VS_CallConfig::eUserStatusScheme GetUserStatusScheme() const { return VS_CallConfig::eUserStatusScheme::ONLY_USER_AVAIL;  };

	boost::signals2::connection ConnectOnZombie(const ZombieSignalSlot &slot)
	{
		return m_fireOnZombie.connect(slot);
	}

	boost::signals2::connection ConnectHangupFromVisi(const HangupFromVisiSignalSlot &slot)
	{
		return m_fireHangupFromVisi.connect(slot);
	}

	boost::signals2::connection ConnectFastUpdatePicture(const FastUpdatePicSignalSlot &slot)
	{
		return m_fireFastUpdatePicture.connect(slot);
	}

	boost::signals2::connection ConnectLoggedOutAsUser(const LoggedOutAsUserSignalSlot &slot)
	{
		return m_fireLoggedOutAsUser.connect(slot);
	}

	boost::signals2::connection ConnectSetMediaChannels(const SetMediaChannelsSignalSlot &slot)
	{
		return m_fireSetMediaChannels.connect(slot);
	}

	boost::signals2::connection ConnectInviteReply(const InviteReplaySignalSlot &slot)
	{
		return m_fireInviteReply.connect(slot);
	}

	boost::signals2::connection ConnectInvite(const InviteSignalSlot &slot)
	{
		return m_fireInvite.connect(slot);
	}

	boost::signals2::connection ConnectChat(const ChatSignalSlot &slot)
	{
		return m_fireChat.connect(slot);
	}

	boost::signals2::connection ConnectFile(const FileSignalSlot &slot)
	{
		return m_fireFile.connect(slot);
	}

	boost::signals2::connection ConnectCommand(const CommandSignalSlot &slot)
	{
		return m_fireCommand.connect(slot);
	}

	boost::signals2::connection ConnectFECC(const FECCSignalSlot &slot)
	{
		return m_fireFECC.connect(slot);
	}

	boost::signals2::connection ConnectReqInvite(const ReqInviteSlot &slot)
	{
		return m_fireReqInvite.connect(slot);
	}

	boost::signals2::connection ConnectTakeTribuneReply(const TakeTribuneReplySlot &slot)
	{
		return m_fireTakeTribuneReply.connect(slot);
	}

	boost::signals2::connection ConnectLeaveTribuneReply(const LeaveTribuneReplySlot &slot)
	{
		return m_fireLeaveTribuneReply.connect(slot);
	}
	template<class Function>
	void SetConnectMeToTransceiver(Function&& f) {
		m_fireConnectMeToTransceiver = std::forward<Function>(f);
	}

	virtual void ConnectUpdateStatus(const UpdateStatusSlot& /*slot*/) {}
protected:

	HangupFromVisiSignal	m_fireHangupFromVisi;
	FastUpdatePicSignal		m_fireFastUpdatePicture;
	LoggedOutAsUserSignal	m_fireLoggedOutAsUser;
	SetMediaChannelsSignal	m_fireSetMediaChannels;
	InviteReplaySignal		m_fireInviteReply;
	InviteSignal			m_fireInvite;
	ZombieSignal			m_fireOnZombie; /** argumein - name of transcoder*/
	ChatSignal				m_fireChat;
	FileSignal				m_fireFile;
	CommandSignal			m_fireCommand;
	FECCSignal				m_fireFECC;
	ReqInviteSignal			m_fireReqInvite;
	TakeTribuneReplySignal  m_fireTakeTribuneReply;
	LeaveTribuneReplySignal m_fireLeaveTribuneReply;

	std::function<bool(const std::string& /*dialog*/, const std::string& /*confID*/)> m_fireConnectMeToTransceiver;
	std::shared_ptr<VS_RTPModuleControlInterface> m_RTPModuleControl;
};
