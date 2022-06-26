#pragma once

#include <boost/signals2.hpp>
#include <vector>
#include "std/cpplib/VS_Protocol.h"
#include "statuslib/status_types.h"
#include "net/Address.h"
#include "net/Port.h"
#include "net/Protocol.h"

class VS_ClientControlInterface;
struct VS_MediaChannelInfo;
struct VS_ConferenceInfo;
namespace gw { struct Participant; }

enum VS_ChannelID :int;
enum VS_CallConfirmCode :int;

enum class VS_CallInviteStatus {
	SUCCESS,
	TIME_OUT,
	FAILURE
};

/**
	»нтерфейс дл€ нашего протокола
	предоставл€ет методы управлени€ конференци€ми в нашей системе

	ѕостепенно будут добавл€тьс€ методы и аргументы в них
*/

class VS_ConferenceProtocolInterface
{
public:
	VS_ConferenceProtocolInterface() = default;
	VS_ConferenceProtocolInterface(const VS_ConferenceProtocolInterface& other) = delete;
	VS_ConferenceProtocolInterface& operator=(const VS_ConferenceProtocolInterface& other) = delete;
	VS_ConferenceProtocolInterface(VS_ConferenceProtocolInterface&& other) noexcept = default;
	VS_ConferenceProtocolInterface& operator=(VS_ConferenceProtocolInterface&& other) noexcept = default;
	virtual ~VS_ConferenceProtocolInterface(){}

	enum class ConferenceStatus
	{
		UNDEFINED = -1,
		AVAILABLE = 0,
		BUSY_HERE,
		NOT_FOUND,
		TMP_UNAVAILABLE,
		SRV_UNAVAILABLE,
	};

	virtual void Logout(string_view /*dialogId*/)
	{}

	virtual void PutSharedTranscoder(string_view /*dialogId*/, boost::shared_ptr<VS_ClientControlInterface> /*transcoder*/)
	{}

	virtual void CloseConnection(const net::address &/*addr*/, net::port /*port*/, net::protocol /*protocol*/)
	{}

	virtual void RegisterNewConnection(const net::address &/*addr*/, net::port /*port*/, net::protocol /*protocols*/, const VS_ChannelID /*channel_id*/)
	{}

	virtual UserStatusInfo GetUserStatus(string_view /*id*/)
	{
		return UserStatusInfo();
	}


	virtual void AsyncInvite(string_view dialogId, const gw::Participant &fromId, string_view toId, const VS_ConferenceInfo &confInfo,
		std::function<void(bool /*redirect*/, ConferenceStatus /*status*/, const std::string &/*ip*/)> inviteResult, string_view dnFromUTF8, bool newSession = true, bool forceCreate = false) = 0;

	virtual void Hangup(string_view /*dialogId*/) = 0;

	virtual boost::shared_ptr<VS_ClientControlInterface> GetTranscoder(string_view /*dialogId*/)
	{
		return nullptr;
	}

	virtual void LoginUser(string_view /*dialogId*/, string_view /*login*/, string_view /*password*/, std::chrono::steady_clock::time_point /*expireTime*/, string_view /*externalName*/,
		std::function<void(bool) > /*result*/, std::function<void(void)> /*logout*/, const std::vector<std::string>& /*h323Aliases*/) { }

	virtual void SetUserEndpointAppInfo(string_view /*dialogId*/, string_view /*appName*/, string_view /*version*/)
	{}


	virtual bool ReqInviteReply(string_view /*dialogId*/, string_view /*fromId*/, bool /*accept*/)
	{
		return false;
	}

	virtual bool SetMediaChannels(string_view dialogId, const std::vector<VS_MediaChannelInfo>& channels, const std::string& existingConfID, std::int32_t bandwRcv = 0) = 0;

	virtual void Chat(string_view /*dialogId*/, const std::string &/*from*/, const std::string &/*to*/, const std::string &/*dn*/, const char */*mess*/)
	{}

	virtual void LoggedOutAsUser(string_view dialogId) = 0;

	virtual bool InviteMethod(string_view dialogId, string_view fromId, string_view toId, const VS_ConferenceInfo &cfgInfo,
		string_view dnFromUTF8 = {}, bool newSession = true, bool forceCreate = false) = 0;


	virtual void FastUpdatePicture(string_view /*dialogId*/)
	{}

	virtual bool InviteReplay(string_view dialogId, VS_CallConfirmCode confirmCode, bool isGroupConf, string_view confName, string_view toDisplayName) = 0;

	virtual void Command(string_view /*dialogId*/, string_view /*from*/, string_view /*command*/)
	{}

	virtual VS_Participant_Role GetMyConferenceRole(string_view /*dialogId*/)
	{
		return PR_COMMON;
	}

	virtual void InviteToTribune(string_view /*dialogId*/, const std::string &/*toId*/)
	{}

	virtual void ExpelFromTribune(string_view /*dialogId*/, const std::string &/*toId*/)
	{}

	virtual VS_Participant_Role GetMyTribuneRole(string_view /*dialogId*/)
	{
		return PR_COMMON;
	}

	virtual bool KickFromConference(string_view /*dialogId*/, string_view /*fromId*/, string_view /*toId*/)
	{
		return false;
	}

	virtual void TakeTribune(string_view /*dialogId*/)
	{}


	virtual void LeaveTribune(string_view /*dialogId*/)
	{}

	virtual bool PrepareForCall(string_view /*dialogId*/, string_view /*fromId*/, bool /*createSession*/)
	{
		return false;
	}

	virtual void UpdateDisplayName(string_view /*dialogId*/, string_view /*displayName*/, bool /*updateImmediately*/)
	{
	}

	virtual boost::shared_ptr<VS_ClientControlInterface> GetLoggedTranscoder(string_view /*fromId*/)
	{
		return nullptr;
	}

	virtual void NotificateAboutReject(string_view /*dialogId*/)
	{
	}

	virtual bool S4B_InitBeforeCall(string_view /*dialogId*/, string_view /*fromId*/, bool /*createSession*/) { return false; }

	virtual void HangupOutcomingCall(string_view /*dialogId*/) {}

	// H323 only.
	virtual void SetForRegisteredUser() {}

	virtual void BitrateRestriction(string_view /*dialogId*/, int /*type*/, int /*value*/, int /*scope*/) {}

	// Call from <my_name> to <call_id>
	// Arguments' format:
	// <my_name>: user@server.com
	// <call_id>: #h323:user or #sip:user ...
	virtual bool ResolveOnExternalGatekeeper(string_view /*myName*/, string_view /*callId*/, net::address &/*addr*/, net::port &/*port*/)
	{
		return false;
	}
};