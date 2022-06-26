#if defined(_WIN32) // Not ported yet

#include <boost/shared_ptr.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "TrueGateway/VS_FakeClientControl.h"
#include "TrueGateway/VS_ClientControlInterface.h"

namespace
{
	class VS_FakeClientMock : public VS_FakeClientInterface
	{
	public:
		MOCK_METHOD5(LoginUserAsync, void(const std::string &login, const std::string &passwd,
			const std::string &passwd_md5, VS_ClientType clienttype, const std::string &ip));

		MOCK_METHOD0(Logout, void());
		MOCK_METHOD1(SetDefaultCaps, void(const VS_ClientCaps &caps));
		MOCK_METHOD1(JoinAsync, bool(const std::string &to));
		MOCK_METHOD1(InviteAsync, bool(const std::string &to));
		MOCK_METHOD2(SendChatMessage, void(const std::string &to, const std::string &msg));
		MOCK_METHOD2(SendCommnad, void(const std::string &to, const std::string &msg));
		MOCK_METHOD2(SetAppProperty, void(const std::string &name, const std::string &val));
		MOCK_METHOD0(SendAppProperties, void());
		MOCK_METHOD0(GetCurrentConference, boost::shared_ptr<VS_ConferenceDescriptor>());
		MOCK_METHOD0(GetCID, const char *());

		MOCK_METHOD1(Accept, bool(boost::shared_ptr<VS_ConferenceDescriptor> &conf));
		MOCK_METHOD2(Reject, bool(boost::shared_ptr<VS_ConferenceDescriptor> &conf, VS_Reject_Cause reason));
		MOCK_METHOD2(SendChatMessage, bool(boost::shared_ptr<VS_ConferenceDescriptor> &conf, const std::string &message));
		MOCK_METHOD1(Hangup, bool(boost::shared_ptr<VS_ConferenceDescriptor> &conf));
		MOCK_METHOD2(QueryRole, bool(boost::shared_ptr<VS_ConferenceDescriptor> &conf, long role));
		MOCK_METHOD0(GetClientState, ClientState());
	};


	class VS_RTPModuleControlMock : public VS_RTPModuleControlInterface
	{
	public:
		MOCK_METHOD1(CreateNewRTPSession, bool(CreateSessionInfo &inf));
		MOCK_METHOD3(SetConference, void(const char *id, const char *conf_name, const char *receive_from));
		MOCK_METHOD1(Remove, void(const char *id));
		MOCK_METHOD5(UpdateModes, void(const char *id, const VS_GatewayMediaModes &mediaModes,
			const VS_SIPMediaChannelInfo &a_info, const VS_SIPMediaChannelInfo &v_info, const VS_ClientCaps &conferenceCaps));
		MOCK_METHOD1(FullIntraframeRequest, void(const char *id));
	};

	class VS_FakeClientControlMock : public VS_FakeClientControl
	{
	public:
		MOCK_METHOD1(WaitForLoginComplete, void());


		MOCK_METHOD1(LoginResultCallback, void(bool));
		MOCK_METHOD0(LogoutResultCallback, void());
		MOCK_METHOD3(InviteReplayCallback, void(const char*, VS_CallConfirmCode, bool));
		MOCK_METHOD1(FirCallback, void(const char*));

	};

	using ::testing::Return;
	using ::testing::AtLeast;
	using ::testing::_;
	using ::testing::SetArgReferee;
	using ::testing::DoAll;
	using ::testing::NiceMock;
	using ::testing::StrEq;

	class FakeClientControlTets : public ::testing::Test
	{
	protected:
		virtual void SetUp() {
			VS_FakeClientControlMock *client = new VS_FakeClientControlMock();
			m_client.reset(client);
			m_fakeClient.reset(new NiceMock<VS_FakeClientMock>());
			m_RTPControl.reset(new NiceMock<VS_RTPModuleControlMock>());

			client->SetFakeClientInterface(m_fakeClient);
			client->SetRTPModuleInterface(m_RTPControl);

			initDefaults();
		}

		void initDefaults()
		{

			VS_RTPModuleControlInterface::CreateSessionInfo rtp_ans;
			rtp_ans.rtpAudioPort = 100;
			rtp_ans.rtcpAudioPort = 101;
			rtp_ans.rtpVideoPort = 200;
			rtp_ans.rtcpVideoPort = 201;

			ON_CALL(*m_RTPControl, CreateNewRTPSession(_))
				.WillByDefault(DoAll(SetArgReferee<0>(rtp_ans), Return(true)));

			ON_CALL(*m_fakeClient, GetCID())
				.WillByDefault(Return("testCID"));


		}

		virtual void TearDown() {
		}

		boost::shared_ptr<NiceMock<VS_FakeClientMock> > m_fakeClient;
		boost::shared_ptr<NiceMock<VS_RTPModuleControlMock> > m_RTPControl;
		boost::shared_ptr<VS_FakeClientControlMock> m_client;
	};

	TEST_F(FakeClientControlTets, GetTranscoderIDTest)
	{
		EXPECT_CALL(*m_fakeClient, GetCID())
			.Times(AtLeast(1))
			.WillRepeatedly(Return("testCID"));

		EXPECT_EQ("testCID", m_client->GetTranscoderID());
	}

	TEST_F(FakeClientControlTets, IsReayTest)
	{
		EXPECT_EQ(true, m_client->IsReady());
	}

	TEST_F(FakeClientControlTets, DialogIDTest)
	{
		EXPECT_EQ(std::string(""), m_client->GetDialogID());
		m_client->SetDialogId("di1");
		EXPECT_EQ(std::string("di1"), m_client->GetDialogID());
		m_client->SetDialogId("di2");
		EXPECT_EQ(std::string("di2"), m_client->GetDialogID());
		m_client->ClearDialogId("di2");
		EXPECT_EQ(std::string("di1"), m_client->GetDialogID());
		m_client->ClearDialogId("di1");
		EXPECT_EQ("", m_client->GetDialogID());
	}

	TEST_F(FakeClientControlTets, LoggingInTrue)
	{
		EXPECT_CALL(*m_fakeClient, LoginUserAsync("name", _, "pass", CT_TRANSCODER_CLIENT, _));

		m_client->LoginUser("name", "pass", 100, "ext_name",
			boost::bind(&VS_FakeClientControlMock::LoginResultCallback, m_client, _1),
			boost::bind(&VS_FakeClientControlMock::LogoutResultCallback, m_client), VS_IPPortAddress::ADDR_IPV4
			);

		EXPECT_CALL(*m_client, LoginResultCallback(true));
		m_fakeClient->m_fireLoginResponse(USER_LOGGEDIN_OK);


		EXPECT_CALL(*m_fakeClient, Logout());
		m_client->LogoutUser(boost::bind(&VS_FakeClientControlMock::LogoutResultCallback, m_client));

		EXPECT_CALL(*m_client, LogoutResultCallback());
		m_fakeClient->m_fireLogoutResponse();

		m_client->ReleaseCallbacks();
	}

	TEST_F(FakeClientControlTets, LoggingInFalse)
	{
		EXPECT_CALL(*m_fakeClient, LoginUserAsync("name", _, "pass", CT_TRANSCODER_CLIENT, _));

		m_client->LoginUser("name", "pass", 100, "ext_name",
			boost::bind(&VS_FakeClientControlMock::LoginResultCallback, m_client, _1),
			boost::bind(&VS_FakeClientControlMock::LogoutResultCallback, m_client), VS_IPPortAddress::ADDR_IPV4
			);

		EXPECT_CALL(*m_client, LoginResultCallback(false));
		m_fakeClient->m_fireLoginResponse(ACCESS_DENIED);
		m_client->ReleaseCallbacks();
	}

	TEST_F(FakeClientControlTets, OutgoingCallWithoutLogin)
	{
		VS_FakeClientInterface::ClientState emptyState;
		VS_FakeClientInterface::ClientState loggedInState;

		loggedInState.trueconfId = "#tel:testNum/testCID";

		EXPECT_CALL(*m_fakeClient, GetClientState())
			.WillOnce(Return( emptyState ))
			.WillRepeatedly(Return( loggedInState ));

		EXPECT_CALL(*m_fakeClient, LoginUserAsync("#tel:testNum/testCID", _, _, CT_TRANSCODER, _));
		EXPECT_CALL(*m_client, WaitForLoginComplete());

		bool prepareResult = m_client->PrepareForCall("#tel:testNum", {}, VS_IPPortAddress::ADDR_IPV4);
		EXPECT_TRUE(prepareResult);

		VS_RTPModuleControlInterface::CreateSessionInfo inf;
		inf.rtcpAudioPort = 101;
		inf.rtpAudioPort = 100;
		inf.rtpVideoPort = 101;
		inf.rtcpVideoPort = 100;

		EXPECT_CALL(*m_fakeClient, InviteAsync("testTo"))
			.WillOnce(Return(true));

		EXPECT_CALL(*m_RTPControl, CreateNewRTPSession(_))
			.WillOnce(DoAll(SetArgReferee<0>(inf) ,Return(true)));

		bool inviteResult = m_client->InviteMethod("testFrom", "testTo", VS_IPPortAddress::ADDR_IPV4);
		EXPECT_TRUE(inviteResult);

		boost::shared_ptr<VS_FakeClientInterface::VS_ConferenceDescriptor> conf (new VS_FakeClientInterface::VS_ConferenceDescriptor());
		conf->conf_id = "testConfID";
		conf->conf_type = CT_PRIVATE;
		conf->owner = "test@server.net";
		conf->m_last_reject_reason = JOIN_OK;
		conf->is_incomming = false;
		conf->stream_conf_id = "teststream@server.name";

		m_client->ConnectInviteReply(boost::bind(&VS_FakeClientControlMock::InviteReplayCallback, m_client.get(), _1, _2, _3));
		EXPECT_CALL(*m_client, InviteReplayCallback(_, e_call_ok, false));
		ON_CALL(*m_fakeClient, GetCurrentConference())
			.WillByDefault(Return(conf));

		m_fakeClient->m_fireConferenceStateChange("joinResponse", *conf);
	}

	//TODO: write test for incomming call from SIP

	TEST_F(FakeClientControlTets, FIRTest)
	{
		m_client->SetDialogId("testDialogId");
		m_client->ConnectFastUpdatePicture(boost::bind(&VS_FakeClientControlMock::FirCallback, m_client.get(), _1));
		EXPECT_CALL(*m_client, FirCallback(StrEq("testDialogId")));
		m_RTPControl->m_fireFullIntraframeRequest(m_fakeClient->GetCID());
	}

}

#endif
