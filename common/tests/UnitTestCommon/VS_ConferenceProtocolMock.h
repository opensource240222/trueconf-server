#pragma once

#include "tests/common/GMockOverride.h"
#include "TrueGateway/interfaces/VS_ConferenceProtocolInterface.h"
#include "../../FakeClient/VS_ConferenceInfo.h"
#include "TrueGateway/VS_GatewayParticipantInfo.h"
#include "tools/Server/VS_MediaChannelInfo.h"

#include <gmock/gmock.h>

class VS_ConferenceProtocolMock : public VS_ConferenceProtocolInterface
{
public:
	VS_ConferenceProtocolMock()
	{
		using ::testing::_;
		using ::testing::AnyNumber;

		EXPECT_CALL(*this, SetMediaChannels(_, _, _, _)).Times(AnyNumber());
	}

	void DelegateTo(VS_ConferenceProtocolInterface* impl)
	{
		using ::testing::_;
		using ::testing::Invoke;

		ON_CALL(*this, SetMediaChannels(::testing::An<string_view>(), _, _, _)).WillByDefault(Invoke([impl](string_view dialogId, const std::vector<VS_MediaChannelInfo> &channels, const std::string& existingConfID, std::int32_t bandwRcv = 0) -> bool
		{
			return impl->SetMediaChannels(dialogId, channels, existingConfID, bandwRcv);
		}));
		ON_CALL(*this, S4B_InitBeforeCall(::testing::An<string_view>(), _, _)).WillByDefault(Invoke([impl](string_view dialogId, string_view fromId, bool createSession) -> bool
		{
			return impl->S4B_InitBeforeCall(dialogId, fromId, createSession);
		}));
		ON_CALL(*this, PrepareForCall(::testing::An<string_view>(), _, _)).WillByDefault(Invoke([impl](string_view dialogId, string_view fromId, bool createSession) -> bool
		{
			return impl->PrepareForCall(dialogId, fromId, createSession);
		}));
		// Delegate other methods as needed, there are no point in writing 20+ simillar lines
	}

	MOCK_METHOD8_OVERRIDE(LoginUser, void(string_view dialogId, string_view login, string_view password, std::chrono::steady_clock::time_point expireTime, string_view externalName,
		std::function<void(bool) > result, std::function<void(void)> logout, const std::vector<std::string>& h323Aliases));
	MOCK_METHOD7_OVERRIDE(InviteMethod, bool(string_view dialogId, string_view fromId, string_view toId, const VS_ConferenceInfo &cfgInfo,
		string_view dnFromUTF8, bool newSession, bool forceCreate));
	MOCK_METHOD5_OVERRIDE(InviteReplay, bool(string_view dialogId, VS_CallConfirmCode confirm_code, bool isGroupConf, string_view confName, string_view to_displayName));
	MOCK_METHOD1_OVERRIDE(Hangup, void(string_view dialog_id));
	MOCK_METHOD8_OVERRIDE(AsyncInvite, void(string_view dialogId, const gw::Participant&, string_view toId, const VS_ConferenceInfo &confInfo,
		std::function<void(bool redirect, ConferenceStatus stat, const std::string &ip)> inviteResult, string_view dnFromUTF8, bool newSession, bool forceCreate));
	MOCK_METHOD1_OVERRIDE(LoggedOutAsUser, void(string_view dialogId));
	MOCK_METHOD4_OVERRIDE(SetMediaChannels, bool(string_view dialogId, const std::vector<VS_MediaChannelInfo> &channels, const std::string& existingConfID, std::int32_t bandwRcv));

	MOCK_METHOD5_OVERRIDE(Chat, void(string_view dialogId, const std::string &from, const std::string &to, const std::string &dn, const char *mess));
	MOCK_METHOD3_OVERRIDE(PrepareForCall, bool(string_view /*dialog_id*/, string_view /*from_id*/, bool /*create_session*/));
	MOCK_METHOD1_OVERRIDE(GetUserStatus, UserStatusInfo(string_view /*id*/));
	MOCK_METHOD1_OVERRIDE(TakeTribune, void(string_view /*dialog_id*/));
	MOCK_METHOD1_OVERRIDE(LeaveTribune, void(string_view /*dialog_id*/));
	MOCK_METHOD1_OVERRIDE(GetMyTribuneRole, VS_Participant_Role(string_view /*dialog_id*/));
	MOCK_METHOD1_OVERRIDE(GetMyConferenceRole, VS_Participant_Role(string_view /*dialog_id*/));
	MOCK_METHOD3_OVERRIDE(KickFromConference, bool(string_view /*dialog_id*/, string_view /*from_id*/, string_view /*to_id*/));
	MOCK_METHOD3_OVERRIDE(S4B_InitBeforeCall, bool (string_view, string_view, bool));
};