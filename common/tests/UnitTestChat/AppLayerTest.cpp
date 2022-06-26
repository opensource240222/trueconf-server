#include "ChatTestFixture.h"
#include "router/ChatMessageRouter.h"
#include "tests/UnitTestChat/globvars.h"
#include "tests/common/TestHelpers.h"
#include "tests/common/GMockOverride.h"

#include "chatlib/helpers/AccountInfo.h"
#include "chatlib/msg/ChatMessage.h"
#include "chatlib/msg/attr.h"
#include "chatlib/notify/ChatEventsFuncs.h"
#include "chatlib/utils/chat_utils.h"
#include "chatlib/utils/msg_utils.h"

#include "std-generic/cpplib/scope_exit.h"

#include <gmock/gmock.h>

#include <chrono>
#include <list>


using ::testing::_;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::InvokeArgument;
using ::testing::InvokeWithoutArgs;
using ::testing::NotNull;
using ::testing::StrEq;

MATCHER_P(IsStrViewEq, str, "") { return arg == str; }
class AppLayerTestInviteWithAnswer : public ChatTestFixture,
	public ::testing::WithParamInterface<chat::InviteResponseCode>
{
};
class AppLayerTest : public ChatTestFixture
{};
TEST_F(AppLayerTest, CreateChatAppLayer)
{
	InitParticipantsWithMockLayer();

	auto creator_p = participants_ctx_.find(CREATOR_CALL_ID);
	std::string test_title = "Test Chat Title";
	ASSERT_TRUE(creator_p != participants_ctx_.end());
	auto part_ctx = creator_p->second;
	auto creator_part = creator_p->second->accaunt_info;

	auto ev1 = std::make_shared<vs::event>(true);
	auto ev2 = std::make_shared<vs::event>(true);
	auto ev3 = std::make_shared<vs::event>(true);
	auto next_layer = creator_p->second->mocked_layer;
	auto fAddOwner = [&, ev1](auto msg, auto)
	{
		VS_SCOPE_EXIT{ ev1->set(); };
		this->SendAddPartMess(
			chat::ParticipantType::client,
			creator_part->GetCallID(),
			creator_part->GetCallID(),
			msg);
	};
	auto fAddBS = [&, ev2](auto msg, auto)
	{
		VS_SCOPE_EXIT{ ev2->set(); };
		this->SendAddPartMess(
			chat::ParticipantType::server,
			creator_part->GetCallID(),
			creator_part->GetBS(),
			msg);
	};

	EXPECT_CALL(*next_layer, ForwardBelowMessage(NotNull(), _))
		.WillOnce(Invoke([this, &part_ctx, &test_title](auto msg, auto) // CreateChat Message
	{
		/**
			Check the fields
			attr::MESSAGE_TYPE_paramName
			msg::typeKeyName
			msg::titleKeyName
			msg::versionKeyName
			attr::CHAT_ID_paramName[]
			attr::FROM_paramName[]
			msg::fromInstanceKeyName
			attr::TIMESTAMP_paramName[]
			attr::MESSAGE_ID_paramName[]
			attr::SENDER_TYPE_paramName[]
		*/
		using namespace chat;

		auto & msg_ref = *msg;
		std::string ver, from, from_instance, chat_id, msg_id, title;

		auto msgType = MessageType::undefined;
		ASSERT_TRUE(msg_ref->GetParamI32(attr::MESSAGE_TYPE_paramName, msgType));
		ASSERT_EQ(msgType, MessageType::create_chat);

		auto chatType = ChatType::undef;
		ASSERT_TRUE(GetParamFromMsgContent(msg_ref, msg::typeKeyName, chatType));
		ASSERT_EQ(chatType, ChatType::symmetric);

		ASSERT_TRUE(GetParamFromMsgContent(msg_ref, msg::titleKeyName, title));
		ASSERT_EQ(title, test_title);

		ASSERT_TRUE(GetParamFromMsgContent(msg_ref, msg::versionKeyName, ver));
		ASSERT_EQ(ver, "2");

		ASSERT_TRUE(msg_ref->GetParam(attr::CHAT_ID_paramName, chat_id));
		ASSERT_EQ(chat_id.length(), 40u); //sha1 20 bytes in hex

		ASSERT_TRUE(msg_ref->GetParam(attr::FROM_paramName, from));
		ASSERT_EQ(from, part_ctx->accaunt_info->GetCallID());

		ASSERT_TRUE(GetParamFromMsgContent(msg_ref, msg::fromInstanceKeyName, from_instance));
		ASSERT_EQ(from_instance, part_ctx->accaunt_info->GetExactCallID());

		ChatMessageTimestamp timestamp;
		ASSERT_TRUE(msg_ref->GetParam(attr::TIMESTAMP_paramName, timestamp)); // timestamp

		ASSERT_TRUE(msg_ref->GetParam(attr::MESSAGE_ID_paramName, msg_id));
		ASSERT_EQ(msg_id.length(), 36u); // uuid

		auto sender_tp = vs::CallIDType::undef;
		ASSERT_TRUE(msg_ref->GetParamI32(attr::SENDER_TYPE_paramName, sender_tp));
		ASSERT_EQ(sender_tp, part_ctx->accaunt_info->GetCallIDType());

		auto chat_info = msg::CreateChatMessage::GetGlobalContext(msg_ref);
		ASSERT_EQ(chat_info.chatId, chat_id);
		part_ctx->events_notifier->OnChatCreated(chat_info);
		current_chat_id_ = chat_id;
		msg_ref->OnChainUpdateByMsg(cb::ProcessingResult::ok, {});
	})
	)
	.WillRepeatedly(Invoke([&](auto msg, auto dummy)
	{
		auto type = chat::MessageType::undefined;
		(*msg)->GetParamI32(chat::attr::MESSAGE_TYPE_paramName, type);
		if (type == chat::MessageType::get_global_ctx_req)
			return;
		static int numberOfCall = 0;
		ASSERT_LE(numberOfCall, 1);
		if (numberOfCall == 0)
		{
			++numberOfCall;
			fAddOwner(msg, dummy);
		}
		else if (numberOfCall == 1)
		{
			++numberOfCall;
			fAddBS(msg, dummy);
		}
	}));

	bool is_call_back_called(false);
	CreateChat(CREATOR_CALL_ID, chat::ChatType::symmetric, test_title,
		[&is_call_back_called, this, ev3](chat::cb::CreateChatResult res, chat::ChatIDRef id)
	{
		VS_SCOPE_EXIT{ ev3->set(); };
		is_call_back_called = true;
		ASSERT_EQ(res, chat::cb::CreateChatResult::ok);
		ASSERT_EQ(id, current_chat_id_);
	});
	EXPECT_TRUE(test::WaitFor("CreateChat",
		[ev1, ev2, ev3]() { return ev1->try_wait() && ev2->try_wait() && ev3->try_wait(); },
		10, g_max_wait_timeout_ms));
	ASSERT_TRUE(is_call_back_called);
}

TEST_F(AppLayerTest, DISABLED_InviteToChat)
{
	InitParticipantsWithMockLayer();

	auto creator_part = participants_ctx_.find(CREATOR_CALL_ID);
	ASSERT_TRUE(creator_part != participants_ctx_.end());
	auto creator_ctx = creator_part->second;
	auto test_title = "Test chat title";
	auto ev1 = std::make_shared<vs::event>(true);
	auto ev2 = std::make_shared<vs::event>(true);
	bool is_invite_arrived(false);
	creator_ctx->appLayer->CreateChat(chat::ChatType::symmetric, test_title,
		[
			&creator_ctx,
			&is_invite_arrived,
			test_title,
			ev1, ev2
		](chat::cb::CreateChatResult r, chat::ChatIDRef id)
	{
		ASSERT_EQ(r, chat::cb::CreateChatResult::ok);
		auto invited_part = "invited_part@trueconf.com";
		EXPECT_CALL(
			*creator_ctx->mocked_layer,
			ForwardBelowMessage(NotNull(), _))
			.WillRepeatedly(Invoke(
				[
					id = chat::ChatID(id),
					test_title,
					invited_part,
					&creator_ctx,
					&is_invite_arrived,
					ev1
				] (auto ptr, auto)
		{
			using namespace chat;
			auto & msg_ref= *ptr;
			std::string chat_id, to, from, from_instance, msg_id, ver, title;

			auto msg_type = MessageType::undefined;
			msg_ref->GetParamI32(attr::MESSAGE_TYPE_paramName, msg_type);
			if (msg_type != MessageType::invite)
				return;
			VS_SCOPE_EXIT{ ev1->set(); };

			ASSERT_TRUE(msg_ref->GetParam(attr::CHAT_ID_paramName, chat_id));
			ASSERT_EQ(chat_id, id);

			auto chat_type = ChatType::undef;
			ASSERT_TRUE(GetParamFromMsgContent(msg_ref, chat::msg::typeKeyName, chat_type));

			ASSERT_TRUE(GetParamFromMsgContent(msg_ref, chat::msg::titleKeyName, title));
			ASSERT_EQ(title, test_title);

			ASSERT_TRUE(GetParamFromMsgContent(msg_ref, chat::msg::versionKeyName, ver));

			ASSERT_TRUE(GetParamFromMsgContent(msg_ref, chat::msg::nameKeyName, to));
			ASSERT_EQ(to, invited_part);

			ASSERT_TRUE(msg_ref->GetParam(attr::FROM_paramName, from));
			ASSERT_EQ(from, creator_ctx->accaunt_info->GetCallID());

			ASSERT_TRUE(GetParamFromMsgContent(msg_ref, chat::msg::fromInstanceKeyName, from_instance));
			ASSERT_EQ(from_instance, creator_ctx->accaunt_info->GetExactCallID());

			chat::ChatMessageTimestamp timestamp;
			ASSERT_TRUE(msg_ref->GetParam(attr::TIMESTAMP_paramName, timestamp)); // timestamp

			ASSERT_TRUE(msg_ref->GetParam(attr::MESSAGE_ID_paramName, msg_id));
			ASSERT_EQ(msg_id.length(), 36u);

			auto sender_tp = vs::CallIDType::undef;
			ASSERT_TRUE(msg_ref->GetParamI32(attr::SENDER_TYPE_paramName, sender_tp));
			ASSERT_EQ(sender_tp, creator_ctx->accaunt_info->GetCallIDType());

			is_invite_arrived = true;
			msg_ref->OnChainUpdateByMsg(cb::ProcessingResult::ok, {});
		}));
		creator_ctx->appLayer->InviteToChat(chat::ChatID(id), invited_part,
			[ev2](
				chat::cb::ProcessingResult r,
				chat::ChatMessageIDRef,
				const chat::cb::MsgIdAndOrderInChain&)
		{
			VS_SCOPE_EXIT{ ev2->set(); };
			ASSERT_EQ(r, chat::cb::ProcessingResult::ok);
		});
	}
	);
	EXPECT_TRUE(test::WaitFor("InviteToChat",
		[ev1, ev2]() { return ev1->try_wait() && ev2->try_wait(); },
		10, g_max_wait_timeout_ms));
	EXPECT_TRUE(is_invite_arrived);
}

namespace {
	class OnInviteVirtual
	{
	public:
		virtual void OnInvite(chat::ChatIDRef, const chat::cb::InviteResponseCallBack&) = 0;
	};

	class OnInviteVitual_Mock : public OnInviteVirtual
	{
	public:
		MOCK_METHOD2_OVERRIDE(OnInvite, void(chat::ChatIDRef, const chat::cb::InviteResponseCallBack&));
	};

	class OnInviteResponseVirtual
	{
	public:
		virtual void OnInviteResponse(chat::ChatIDRef, chat::CallIDRef, const chat::InviteResponseCode &) = 0;
	};

	class OnInviteResponseVirtual_Mock : public OnInviteResponseVirtual
	{
	public:
		MOCK_METHOD3_OVERRIDE(OnInviteResponse, void(chat::ChatIDRef, chat::CallIDRef, const chat::InviteResponseCode&));
	};

}

TEST_P(AppLayerTestInviteWithAnswer, InviteWithAnswer)
{
	/**
		owner			participant
		CreateChat
		SendInvite	-->	RecvInvite
		RecvResponse <-- SendResponse from param

	*/
	InitParticipantsWithMainStorage();

	RegisterAllInstancesInFakeRotuer();

	auto iter = participants_ctx_.find(CREATOR_CALL_ID);
	ASSERT_TRUE(iter != participants_ctx_.end());
	auto owner = iter->second;
	chat::AccountInfoPtr part_account = *PARTICIPANTS_ACCOUNTS.begin();
	iter = participants_ctx_.find(part_account->GetCallID());
	ASSERT_TRUE(iter != participants_ctx_.end());
	auto part_for_invite = iter->second;
	auto test_title = "Test Chat Title";
	OnInviteVitual_Mock on_invite_mock;
	OnInviteResponseVirtual_Mock on_invite_response_mock;
	auto ev = std::make_shared<vs::event>(false);

	part_for_invite->appLayer->SetOnInviteCallBack(
		[&on_invite_mock](chat::cb::ChatDescription&& descr, const chat::cb::InviteResponseCallBack&cb)
		{
			on_invite_mock.OnInvite(descr.chatID, cb);
		});

	owner->appLayer->SetOnInviteResponseCallBack(
		[&on_invite_response_mock](
			chat::ChatIDRef id,
			chat::CallIDRef from,
			chat::InviteResponseCode code)
	{
		on_invite_response_mock.OnInviteResponse(id, from, code);
	});

	EXPECT_CALL(on_invite_mock, OnInvite(_, _))
		.WillOnce(InvokeArgument<1>(GetParam()));
	owner->appLayer->CreateChat(chat::ChatType::symmetric, test_title,
		[this, &owner,
		&part_for_invite, ev,
		&on_invite_response_mock](chat::cb::CreateChatResult r, chat::ChatIDRef chat_id)
	{
		ASSERT_EQ(r, chat::cb::CreateChatResult::ok);
		ASSERT_TRUE(!chat_id.empty());

		EXPECT_CALL(on_invite_response_mock,
			OnInviteResponse(
				IsStrViewEq(chat::ChatID(chat_id)),
				IsStrViewEq(part_for_invite->accaunt_info->GetCallID()),
				Eq(GetParam())))
			.Times(1)
			.WillOnce(InvokeWithoutArgs([ev](){ev->set();}));

		owner->appLayer->InviteToChat(
			chat::ChatID(chat_id),
			part_for_invite->accaunt_info->GetCallID());
	});
	EXPECT_TRUE(test::WaitFor("InviteResponse", *ev, g_max_wait_timeout_ms));
}

TEST_P(AppLayerTestInviteWithAnswer, InviteResponseMessageTest)
{
	InitParticipantsWithMainStorage();

	auto creator = participants_ctx_.find(CREATOR_CALL_ID);
	ASSERT_TRUE(creator != participants_ctx_.end());
	auto owner = creator->second;
	auto test_title = "Test Chat Title";


	auto invited_account = *PARTICIPANTS_ACCOUNTS.begin();
	const auto &iter = participants_ctx_.find(invited_account->GetCallID());
	ASSERT_TRUE(iter != participants_ctx_.end());
	auto invited_part = iter->second;
	// shared_ptr cycle!!! It will be resolved explicitly later
	auto invited_part_mock_layer = std::make_shared<LayerInterface_Mock>(
		invited_part->last_layer);
	invited_part_mock_layer->DelegateToDefault();
	invited_part->last_layer->SetNextLayer(invited_part_mock_layer);
	invited_part->last_layer = invited_part_mock_layer;

	RegisterAllInstancesInFakeRotuer();

	invited_part->appLayer->SetOnInviteCallBack([this](
		chat::cb::ChatDescription&&,
		const chat::cb::InviteResponseCallBack&cb)
	{
		cb(GetParam());
	});

	/**
		creator CreateChat
		creator - > InviteToChat - > invited part;
		catch the invite response message
	*/
	auto ev = std::make_shared<vs::event>(false);
	bool is_invite_response_arrived(false);
	owner->appLayer->CreateChat(chat::ChatType::symmetric, test_title,
		[this, &owner,
		&invited_part, ev,
		&is_invite_response_arrived,
		&invited_part_mock_layer](chat::cb::CreateChatResult r,  chat::ChatIDRef id)
	{
		ASSERT_EQ(r, chat::cb::CreateChatResult::ok);
		ASSERT_TRUE(!id.empty());

		EXPECT_CALL(*invited_part_mock_layer, ForwardBelowMessage(NotNull(), _))
			.WillRepeatedly(Invoke(
				[
					this,
					id = chat::ChatID(id),
					&owner,
					ev,
					&invited_part,
					&is_invite_response_arrived
				](chat::msg::ChatMessagePtr* msg, std::vector<chat::ParticipantDescr>*)
		{
			/**
			InviteResponse message

			attr::MESSAGE_TYPE_paramName = attr::MESSAGE_TYPE_INVITE_RESPONSE
			attr::CHAT_ID_paramName
			msg::nameKeyName
			attr::FROM_paramName
			msg::fromInstanceKeyName
			msg::msgIdKeyName
			msg::codeKeyName

			attr::TIMESTAMP_paramName
			attr::MESSAGE_ID_paramName
			attr::SENDER_TYPE_paramName

			**/
			auto & msg_ref = *msg;
			auto msg_type = chat::MessageType::undefined;
			ASSERT_TRUE(msg_ref->GetParamI32(
				chat::attr::MESSAGE_TYPE_paramName,
				msg_type));
			if (msg_type != chat::MessageType::invite_response)
				return;
			is_invite_response_arrived = true;
			ASSERT_EQ(msg_type, chat::MessageType::invite_response);

			chat::ChatID chat_id;
			ASSERT_TRUE(msg_ref->GetParam(chat::attr::CHAT_ID_paramName, chat_id));
			ASSERT_EQ(id, chat_id);

			chat::CallID to;
			ASSERT_TRUE(chat::msg::GetParamFromMsgContent(msg_ref, chat::msg::nameKeyName, to));
			ASSERT_EQ(to, owner->accaunt_info->GetCallID());

			chat::CallID from;
			ASSERT_TRUE(msg_ref->GetParam(chat::attr::FROM_paramName, from));
			ASSERT_EQ(from, invited_part->accaunt_info->GetCallID());

			chat::CallID from_instance;
			ASSERT_TRUE(chat::msg::GetParamFromMsgContent(msg_ref, chat::msg::fromInstanceKeyName, from_instance));
			ASSERT_EQ(from_instance, invited_part->accaunt_info->GetExactCallID());

			chat::ChatMessageID msgId;
			ASSERT_TRUE(chat::msg::GetParamFromMsgContent(msg_ref, chat::msg::msgIdKeyName, msgId));
			ASSERT_EQ(msgId.length(), 36u); // uuid

			auto r = chat::InviteResponseCode::undef;
			ASSERT_TRUE(chat::msg::GetParamFromMsgContent(msg_ref, chat::msg::codeKeyName, r));
			ASSERT_EQ(static_cast<chat::InviteResponseCode>(r), GetParam());

			chat::ChatMessageTimestamp timestamp;
			ASSERT_TRUE(msg_ref->GetParam(chat::attr::TIMESTAMP_paramName, timestamp));

			ASSERT_TRUE(msg_ref->GetParam(chat::attr::MESSAGE_ID_paramName, msgId));
			ASSERT_EQ(msgId.length(), 36u); // uuid

			auto sender_type = vs::CallIDType::undef;
			ASSERT_TRUE(msg_ref->GetParamI32(chat::attr::SENDER_TYPE_paramName, sender_type));
			ASSERT_EQ(sender_type, invited_part->accaunt_info->GetCallIDType());

			ev->set();
		}));
		owner->appLayer->InviteToChat(chat::ChatID(id), invited_part->accaunt_info->GetCallID());
	});
	EXPECT_TRUE(test::WaitFor("InviteResponseMessageTest", *ev, g_max_wait_timeout_ms));
	// clearing shared_ptr cycle explicitly is here
	invited_part_mock_layer->ClearDefault();
	ASSERT_TRUE(is_invite_response_arrived);
}
// FIXME: double of ChatTestFixture.StorageSimpleCase
TEST_F(AppLayerTest, SendMessageToChat)
{
	InitParticipantsWithMainStorage();
	RegisterAllInstancesInFakeRotuer();
	SendMessageToChatCase();
	/**
		1. Owner creates chat;
		2. Owner Invites user1 and user2 to chat (user1 and user2 accept the invitation)
		3. owner sends message1 - user1 and user2 get message1;
		4. user1 sends message2 after receiving message from owner - user2 and owner get message2
		5. user2 sends message3 after receiving messages from owner and user1 - user1 and user2 get message3
	*/
}
INSTANTIATE_TEST_CASE_P(DISABLED_AllAnswersForInvite,
	AppLayerTestInviteWithAnswer,
	::testing::Values(chat::InviteResponseCode::accept,
	chat::InviteResponseCode::reject,
	chat::InviteResponseCode::failed,
	chat::InviteResponseCode::timeout));

