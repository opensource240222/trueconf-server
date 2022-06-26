#include "tests/mocks/VS_FakeEndpointMock.h"
#include "TrueGateway/clientcontrols/VS_FakeClientControl.h"
#include "TrueGateway/clientcontrols/VS_TranscoderLogin.h"
#include "FakeClient/VS_ConferenceInfo.h"
#include "transport/Message.h"
#include "std/cpplib/MakeShared.h"
#include "std-generic/cpplib/deleters.h"
#include "VS_FakeClientMock.h"
#include "RTPModuleControlMock.h"
#include "TransceiversPoolFake.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "std/cpplib/VS_UserData.h"

#include <boost/make_shared.hpp>

#include "std-generic/compat/memory.h"

struct FakeSignalReceiver {
	MOCK_METHOD2(TakeTribuneReply, void(string_view dialogId, bool result));
	MOCK_METHOD2(LeaveTribuneReply,void(string_view dialogId, bool result));
};

struct FakeClientControlTest : public ::testing::Test {
	boost::shared_ptr<VS_FakeClientControl> pClentControl;
	std::shared_ptr<FakeClientMock>		pFake_clientMock;
	std::shared_ptr<RTPModuleControlMock> pRTPControlMock;
	std::shared_ptr<test::TransceiversPoolFake> pFakeTransPool;
	std::shared_ptr<VS_TranscoderLogin> pTransLogin;
	FakeSignalReceiver signal_receiver;	// just to verify how signals will be called

	const char* curr_conf_id = "12345";
	const char* dialog_id = "ABCD12345";
	std::string cid = "CID";
	std::string server_name = "our.server.name#vcs";

	void SetUp() {
		using ::testing::_;
		using ::testing::AnyNumber;
		using ::testing::Return;
		using ::testing::ReturnRef;

		pFakeTransPool = std::make_shared<test::TransceiversPoolFake>();
		pTransLogin = vs::MakeShared<VS_TranscoderLogin>();
		pClentControl = boost::make_shared<VS_FakeClientControl>(pFakeTransPool, pTransLogin);
		auto fake_endpoint = vs::make_unique<VS_FakeEndpointMock>();
		EXPECT_CALL(*fake_endpoint, SetReceiver(_)).Times(1);
		EXPECT_CALL(*fake_endpoint, Stop()).Times(1);
		EXPECT_CALL(*fake_endpoint, CID())
			.Times(AnyNumber())
			.WillRepeatedly(ReturnRef(cid));
		EXPECT_CALL(*fake_endpoint, Send_mocked(_))
			.Times(AnyNumber())
			.WillRepeatedly(Return(true));
		pFake_clientMock = vs::MakeShared<FakeClientMock>(std::move(fake_endpoint));
		pRTPControlMock = std::make_shared<RTPModuleControlMock>();
		pClentControl->SetFakeClientInterface(pFake_clientMock);
		pClentControl->SetConnectMeToTransceiver([](const std::string& /*dialog*/, const std::string& /*confID*/) {return true; });
		pClentControl->SetRTPModuleInterface(pRTPControlMock);
		pClentControl->SetDialogId(dialog_id);
		pClentControl->ConnectTakeTribuneReply(boost::bind(&FakeSignalReceiver::TakeTribuneReply, &signal_receiver, _1, _2));
		pClentControl->ConnectLeaveTribuneReply(boost::bind(&FakeSignalReceiver::LeaveTribuneReply, &signal_receiver, _1, _2));
	}
	void TearDown() {
		pClentControl = nullptr;
		pFake_clientMock = nullptr;
	}

	void SetClientStateByForce(const vs_user_id& user) {
		pFake_clientMock->m_state.displayName = "displayName";
		pFake_clientMock->m_state.app_id = "app_id";
		pFake_clientMock->m_state.app_name = "app_name";
		pFake_clientMock->m_state.m_conf = vs::MakeShared<VS_FakeClientInterface::VS_ConferenceDescriptor>();
		pFake_clientMock->m_state.m_conf->conf_id = curr_conf_id;
		pFake_clientMock->m_state.m_conf->stream_conf_id = curr_conf_id;
		pFake_clientMock->m_state.m_conf->my_id = user;
	}

	void ReceiveChangeState(const vs_user_id& user, VS_RoleEvent_Type t, VS_Participant_Role role, VS_RoleInqury_Answer result) {
		VS_Container cnt;
		cnt.AddValue(METHOD_PARAM, ROLEEVENT_METHOD);
		cnt.AddValue(CONFERENCE_PARAM, curr_conf_id);
		cnt.AddValueI32(TYPE_PARAM, t);
		cnt.AddValue(USERNAME_PARAM, user);
		cnt.AddValueI32(ROLE_PARAM, role);
		cnt.AddValueI32(RESULT_PARAM, result);

		void* body = nullptr;
		size_t bodySize = 0;
		cnt.SerializeAlloc(body, bodySize);
		std::unique_ptr<void, free_deleter> pBody(body);

		pFake_clientMock->OnReceive(transport::Message::Make()
			.TimeLimit(0)
			.AddString("add_string")
			.SrcService("our_service")
			.SrcUser("from_user")
			.SrcServer("from_server")
			.DstService(CONFERENCE_SRV)
			.DstUser("to_user")
			.DstServer("to_server")
			.Body(body, bodySize)
		);
	}

	void ReceiveChangeStateAnswer(const vs_user_id& user, VS_Participant_Role role, VS_RoleInqury_Answer result) {
		ReceiveChangeState(user, RET_ANSWER, role, result);
	}

	void ReceiveChangeStateNotification(const vs_user_id& user, VS_Participant_Role role, VS_RoleInqury_Answer result) {
		ReceiveChangeState(user, RET_NOTIFY, role, result);
	}

	void ReceiveChangeStateConfirmation(const vs_user_id& user, VS_Participant_Role role, VS_RoleInqury_Answer result) {
		ReceiveChangeState(user, RET_CONFIRM, role, result);
	}

	void ReceiveChangeStateRequest(const vs_user_id& user, VS_Participant_Role role, VS_RoleInqury_Answer result) {
		ReceiveChangeState(user, RET_INQUIRY, role, result);
	}

	void ClientControlConferenceStateChange(const char *method, const VS_FakeClientInterface::VS_ConferenceDescriptor &conf) {
		pClentControl->onConferenceStateChange(method, conf);
	}
};

// webinar with empty 'to' is empty conference that must be created
TEST_F(FakeClientControlTest, EmptyWebinar) {
	using ::testing::_;

	EXPECT_CALL(*pFake_clientMock, JoinAsync(_, _, _));
	pClentControl->InviteMethod("from", {}/*to*/, VS_ConferenceInfo(true, true), true, true);
}

// when /webinar command arrived we must create special conference(name must begin with "$c" ) and topic must be set if provided
TEST_F(FakeClientControlTest, InviteToWebinar) {
	using ::testing::_;
	EXPECT_CALL(*pFake_clientMock, JoinAsync(_, _, _)).Times(2);
	pClentControl->InviteMethod("from", "to", VS_ConferenceInfo(true, true, "WebinarTopic"), true, true);
	pClentControl->InviteMethod("from", "to", VS_ConferenceInfo(true, true), true, true);
}

// to make sure we make reply only for answers
TEST_F(FakeClientControlTest, StatusNotifyingOnlyOnAnswer) {
	using ::testing::_;
	const vs_user_id user = "user";
	SetClientStateByForce(user);

	EXPECT_CALL(signal_receiver, TakeTribuneReply(_, _)).Times(1);
	ReceiveChangeStateAnswer(user, PR_PODIUM, RIA_POSITIVE);
	ReceiveChangeStateNotification(user, PR_PODIUM, RIA_POSITIVE);
	ReceiveChangeStateConfirmation(user, PR_PODIUM, RIA_POSITIVE);
	ReceiveChangeStateRequest(user, PR_PODIUM, RIA_POSITIVE);

	EXPECT_CALL(signal_receiver, LeaveTribuneReply(_, _)).Times(1);
	ReceiveChangeStateAnswer(user, PR_COMMON, RIA_POSITIVE);
	ReceiveChangeStateNotification(user, PR_COMMON, RIA_POSITIVE);
	ReceiveChangeStateConfirmation(user, PR_COMMON, RIA_POSITIVE);
	ReceiveChangeStateRequest(user, PR_COMMON, RIA_POSITIVE);
}

// to make sure we not make reply when aswer is not arrived
TEST_F(FakeClientControlTest, DontNotifyNoMyStatus) {
	using ::testing::_;
	const vs_user_id user = "user";
	const vs_user_id other_user = "other_user";
	SetClientStateByForce(user);

	EXPECT_CALL(signal_receiver, TakeTribuneReply(_, _)).Times(0);
	ReceiveChangeStateNotification(other_user, PR_PODIUM, RIA_POSITIVE);
	ReceiveChangeStateConfirmation(other_user, PR_PODIUM, RIA_POSITIVE);
	ReceiveChangeStateRequest(other_user, PR_PODIUM, RIA_POSITIVE);

	EXPECT_CALL(signal_receiver, LeaveTribuneReply(_, _)).Times(0);
	ReceiveChangeStateNotification(other_user, PR_COMMON, RIA_POSITIVE);
	ReceiveChangeStateConfirmation(other_user, PR_COMMON, RIA_POSITIVE);
	ReceiveChangeStateRequest(other_user, PR_COMMON, RIA_POSITIVE);
}

TEST_F(FakeClientControlTest, PositiveNegativeAnswer0) {
	using ::testing::_;
	const vs_user_id user = "user";
	SetClientStateByForce(user);

	EXPECT_CALL(signal_receiver, TakeTribuneReply(_, true)).Times(1);
	ReceiveChangeStateAnswer(user, PR_PODIUM, RIA_POSITIVE);

	EXPECT_CALL(signal_receiver, LeaveTribuneReply(_, true)).Times(1);
	ReceiveChangeStateAnswer(user, PR_COMMON, RIA_POSITIVE);

	// after successfull leaving of tribune receive 4 not successfull replies to take the tribune
	EXPECT_CALL(signal_receiver, TakeTribuneReply(_, false)).Times(4);
	ReceiveChangeStateAnswer(user, PR_PODIUM, RIA_ROLE_BUSY);
	ReceiveChangeStateAnswer(user, PR_PODIUM, RIA_BY_PARTICIPANT);
	ReceiveChangeStateAnswer(user, PR_PODIUM, RIA_PATRICIPANT_BUSY);
	ReceiveChangeStateAnswer(user, PR_PODIUM, RIA_PARTICIPANT_ABSENT);
}

TEST_F(FakeClientControlTest, PositiveNegativeAnswer1) {
	using ::testing::_;
	const vs_user_id user = "user";
	SetClientStateByForce(user);

	EXPECT_CALL(signal_receiver, TakeTribuneReply(_, true)).Times(1);
	ReceiveChangeStateAnswer(user, PR_PODIUM, RIA_POSITIVE);

	// after successfull takinging of tribune receive 4 not successfull replies to live the tribune
	EXPECT_CALL(signal_receiver, LeaveTribuneReply(_, false)).Times(4);
	ReceiveChangeStateAnswer(user, PR_COMMON, RIA_ROLE_BUSY);
	ReceiveChangeStateAnswer(user, PR_COMMON, RIA_BY_PARTICIPANT);
	ReceiveChangeStateAnswer(user, PR_COMMON, RIA_PATRICIPANT_BUSY);
	ReceiveChangeStateAnswer(user, PR_COMMON, RIA_PARTICIPANT_ABSENT);
}

// due to misunderstanding in 'enum VS_Participant_Role' we do not change role for owner or moderator (i.e. PR_LEADER)
// make sure we remember ourselves what status must be
TEST_F(FakeClientControlTest, RememberStatusWhenLeader) {
	using ::testing::_;
	const vs_user_id user = "user";
	SetClientStateByForce(user);

	// 1. Receive response that conference is created by us i.e. we are owner and notification that we are PR_LEADER
	auto conf = vs::MakeShared<VS_FakeClientInterface::VS_ConferenceDescriptor>();
	conf->owner = conf->my_id = user;
	conf->m_last_reject_reason = JOIN_OK;

	EXPECT_CALL(signal_receiver, TakeTribuneReply(_, true)).Times(1);
	ClientControlConferenceStateChange("joinResponse", *conf);
	ReceiveChangeStateNotification(user, PR_LEADER, RIA_POSITIVE);

	// 1.1. We can receive other messages with same state, but TakeTribuneReply must be call once
	ReceiveChangeStateAnswer(user, PR_LEADER, RIA_POSITIVE);
	ReceiveChangeStateConfirmation(user, PR_LEADER, RIA_POSITIVE);

	// 2. Receive PR_LEADER but remember real status and make normal call back
	pClentControl->LeaveTribune();
	EXPECT_CALL(signal_receiver, LeaveTribuneReply(_, true)).Times(1);
	ReceiveChangeStateAnswer(user, PR_LEADER, RIA_POSITIVE);
}
