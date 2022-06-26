#pragma once

#include "../interfaces/VS_ConferenceProtocolInterface.h"
#include "statuslib/status_types.h"

#include "std-generic/compat/memory.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include "std-generic/cpplib/VS_ClockWrapper.h"

class VS_TranscoderKeeper;
class VS_SIPCallResolver;
struct VS_ConferenceInfo;
namespace ts { struct IPool; }
namespace sip { class TransportLayer; }

class VS_SIPVisiProxy : public VS_ConferenceProtocolInterface,
						public vs::enable_shared_from_this<VS_SIPVisiProxy>
{
public:
	struct InitInfo final
	{
		std::shared_ptr<VS_TranscoderKeeper> trKeeper;
		std::weak_ptr<sip::TransportLayer> sipTransport;
		std::weak_ptr<VS_SIPCallResolver> sipCallResolver;
		std::weak_ptr<ts::IPool> transcPool;
		UserStatusFunction getUserStatus;
	};

	///VS_ConferenceProtocolInterface

	void LoginUser(string_view dialogId, string_view login, string_view password, std::chrono::steady_clock::time_point expireTime, string_view externalName,
		std::function<void(bool) > resultCallback, std::function<void(void)> logoutCb, const std::vector<std::string>& aliases) override;
	bool InviteReplay(string_view dialogId, VS_CallConfirmCode confirmCode, bool isGroupConf, string_view confName, string_view to_displayName) override;

	void Hangup(string_view dialogId) override;

	bool S4B_InitBeforeCall(string_view dialogId, string_view fromId, bool createSession) override;

	bool KickFromConference(string_view dialogId, string_view fromId, string_view toId) override;
	bool ReqInviteReply(string_view dialogId, string_view fromId, bool accept) override;
	bool SetMediaChannels(string_view dialogId, const std::vector<VS_MediaChannelInfo>& channels, const std::string& existingConfID, std::int32_t bandwRcv) override;
	void UpdateDisplayName(string_view dialogId, string_view displayName, bool updateImmediately) override;

	void LoggedOutAsUser(string_view) override {}

	bool InviteMethod(string_view /*dialogId*/, string_view /*fromId*/, string_view /*toId*/, const VS_ConferenceInfo &/*cfgInfo*/,
		string_view /*dnFromUTF8*/ = {}, bool /*newSession*/ = true, bool /*forceCreate*/ = false) override
	{
		assert(false);
		return false;
	}

	void AsyncInvite(string_view dialogId, const gw::Participant &from, string_view toId, const VS_ConferenceInfo &info,
		std::function<void(bool /*redirect*/, ConferenceStatus /*status*/, const std::string &/*ip*/)> inviteResult, string_view dnFromUTF8, bool newSession = true, bool forceCreate = false) override;

	boost::shared_ptr<VS_ClientControlInterface> GetTranscoder(string_view dialogId) override;
	boost::shared_ptr<VS_ClientControlInterface> GetLoggedTranscoder(string_view fromId) override;

	void FastUpdatePicture(string_view dialogId) override;
	void SetUserEndpointAppInfo(string_view dialogId, string_view appName, string_view version) override;

	UserStatusInfo GetUserStatus(string_view callId) override
	{
		return m_userStatus(callId, true, true);
	}

	void TakeTribune(string_view dialogId) override;
	void LeaveTribune(string_view dialogId) override;
	VS_Participant_Role GetMyTribuneRole(string_view dialogId) override;
	VS_Participant_Role GetMyConferenceRole(string_view dialogId) override;
	bool ConnectClientToTransceiver(string_view dialogId, const std::string& confID);
	void InviteToTribune(string_view dialogId, const std::string & toId) override;
	void ExpelFromTribune(string_view dialogId, const std::string &toId) override;
	void Chat(string_view dialogId, const std::string &from, const std::string &to, const std::string &dn, const char *mess) override;

protected:
	steady_clock_wrapper &clock() const noexcept { return m_clock; }

	VS_SIPVisiProxy(boost::asio::io_service::strand &strand, InitInfo &&init);
private:
	ConferenceStatus Invite(string_view dialogId, const gw::Participant &from, string_view toId, const VS_ConferenceInfo &info, string_view dnFrom_utf8 = {}, bool newSession = true, bool forceCreate = false);
	void AsyncInviteResult(string_view dialogId, string_view fromId, string_view toId, const VS_ConferenceInfo& info,
		bool redirect, ConferenceStatus st, string_view ip, std::function<void(bool /*redirect*/, ConferenceStatus /*status*/, const std::string&/*ip*/ /*TODO: fixes string_view*/)> inviteResult, string_view dnFromUTF8, bool newSession);

private:
	boost::asio::io_service::strand &m_strand;
	std::weak_ptr<ts::IPool> m_transceiversPool;
	std::shared_ptr<VS_TranscoderKeeper> m_trKeeper;
	std::weak_ptr<sip::TransportLayer> m_sipTransport;
	std::weak_ptr<VS_SIPCallResolver> m_sipCallResolver;
	UserStatusFunction m_userStatus;

	mutable steady_clock_wrapper m_clock;
};