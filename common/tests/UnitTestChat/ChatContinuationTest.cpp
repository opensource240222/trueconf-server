#include "tests/common/TestHelpers.h"
#include "tests/UnitTestChat/globvars.h"
#include "tests/UnitTestChat/sql/TestDBDump.h"
#include "tests/UnitTestChat/ChatContinuationFixture.h"

#include "chatlib/chain/ChainContainer.h"
#include "chatlib/msg/attr.h"
#include "chatlib/storage/ChatStorage.h"
#include "chatlib/utils/chat_utils.h"

#include "std-generic/cpplib/scope_exit.h"
#include <limits>
#include <iostream>

namespace chat_test
{
using ::testing::ContainerEq;
using ::testing::Contains;
using ::testing::Not;
using ::testing::Pointwise;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAreArray;

TEST(ChatContinuation, CreateDBFromDump)
{
	ASSERT_TRUE(LoadDBDump(false,"test").result);
}
TEST_F(ChatContinuationFixture, SendMessageInExisting)
{
	ASSERT_TRUE(InitPartsDBFilled());
	auto part = only_users_.begin()->second.front();
	std::string data("Message to old chat from "
		+ part->account_->GetCallID()
		+ "; ep "
		+ part->account_->GetExactCallID());
	const auto &chat_id_work = g_dump_chat_id.front();
	part->app_layer_->SendGroup(chat_id_work, chat::msg::ContentMessage{}.Text(data), {});
	// wait until all participants receive message
	EXPECT_TRUE(test::WaitFor("MessageRecv",
		[
			this,
			msg_count = EpCount() - 1 // sender does not receive
		]()
	{return msg_count == RecvMsgsCount(); },10, g_max_wait_timeout_ms));
	/**
		for each ep
		1. get chat context and compare them;
		2. check that last message is in db and order of messages is identical
	*/
	chat::ChatID chat_id_test;
	vs::set<chat::ParticipantDescr, vs::less<>> all_parts_test;
	std::vector<chat::ChatMessageID> all_chat_msg_id_test;
	for(const auto &p: all_parts_)
	{
		for (const auto &ep : p.second)
		{
			// check chat context
			auto cfg = ep->cfg_;
			auto storage = cfg->GetChatStorage();
			auto ctx = storage->GetGlobalContext(chat_id_work);
			ASSERT_FALSE(ctx.chatId.empty());
			auto chat_id = ctx.chatId;
			if (chat_id_test.empty())
				chat_id_test = chat_id;
			EXPECT_EQ(chat_id, chat_id_test);
			auto all_parts = ctx.participants;
			if (all_parts_test.empty())
				all_parts_test = all_parts;
			EXPECT_EQ(all_parts, all_parts_test);
			// check for identical message order
			std::vector<chat::ChatMessageID> all_chat_msg_id;
			auto chain = storage->GetLastMessages(chat_id,
				std::numeric_limits<int32_t>::max());
			std::transform(chain.begin(), chain.end(),
				std::back_inserter(all_chat_msg_id),
				[](const auto &item) {return item.msg_id; });
			if (all_chat_msg_id_test.empty())
				all_chat_msg_id_test = all_chat_msg_id;
			EXPECT_EQ(all_chat_msg_id, all_chat_msg_id_test);

			//check last message
			auto msg = storage->GetMessage(chain.back().msg_id, p.first);
			auto last_msg = chat::msg::detail::parse_text_msg(msg);
			ASSERT_TRUE(last_msg.success);
			ASSERT_EQ(last_msg.msg.content.text, data);
		}
	}
}
TEST_F(ChatContinuationFixture, ReqChatIDsFromBS)
{
	// FIXME: check that chat_info is stored
	const auto &user1 = g_dump_users.front();
	// user1 with empty db
	ASSERT_TRUE(InitPartsFillDBExcept({ user1 }));
	auto part = only_users_.find(user1)->second.front();
	auto ev = std::make_shared<vs::event>(false);
	part->storage_layer_->FetchAllUserPersonalContexts([ev, part]() {
		auto peersCtxs = part->app_layer_
			->GetMyPersonalContexts(std::numeric_limits<uint32_t>::max(), 1);
		VS_SCOPE_EXIT{ ev->set(); };
		EXPECT_FALSE(peersCtxs.empty());
		std::vector<chat::ChatID> ids;
		for (const auto &i : peersCtxs)
			ids.emplace_back(i.chatId);
		EXPECT_THAT(g_dump_chat_id, UnorderedElementsAreArray(ids));
	});
	EXPECT_TRUE(test::WaitFor("GetAllMayChats completion", *ev, g_max_wait_timeout_ms));
	// get chat info by chat id;
	part->storage_layer_->RequestGlobalContext(
		g_dump_chat_id.front(), part->account_->GetBS(),
		[ev](chat::GlobalContext &&ctx)
		{
			VS_SCOPE_EXIT{ ev->set(); };
			EXPECT_EQ(ctx.chatId, g_dump_chat_id.front());
			vs::set<chat::ParticipantDescr> parts;
			for (const auto &user : g_dump_users)
				parts.emplace(user, chat::ParticipantType::client);
			parts.emplace(g_dump_bs, chat::ParticipantType::server);
			EXPECT_THAT(ctx.participants, UnorderedElementsAreArray(parts));
		});
	// check that context, tail is exist, and all were saved to db;
	EXPECT_TRUE(test::WaitFor("GetChatInfo completion", *ev, g_max_wait_timeout_ms));
}

TEST_F(ChatContinuationFixture, ReqChatTailFromBS)
{
	const auto &user1 = g_dump_users.front();
	// user1 with empty db
	ASSERT_TRUE(InitPartsFillDBExcept({ user1 }));
	auto part = std::weak_ptr<PartInstance>(
		only_users_.find(user1)->second.front());
	auto ev = std::make_shared<vs::event>(false);
	auto tail_len = 30u;
	part.lock()->cfg_->GetSyncChat()->SyncChatTail(
		g_dump_chat_id.front(), tail_len, part.lock()->account_->GetBS(),
		[&, part, ev](chat::cb::ErrorCode res, std::vector<chat::ChatMessageID> &&tail)
	{
		VS_SCOPE_EXIT{ ev->set(); };
		EXPECT_EQ(res,chat::cb::ErrorCode::success);
		auto part_lock = part.lock();
		ASSERT_TRUE(part_lock);
		auto bs_ep = only_bs_.find(g_dump_bs);
		auto cfg = bs_ep->second.front()->cfg_;
		auto storage = cfg->GetChatStorage();
		auto bs_tail = storage->GetLastMessages(g_dump_chat_id.front(), tail_len);
		cfg = part_lock->cfg_;
		storage = cfg->GetChatStorage();
		auto part_tail = storage->GetLastMessages(g_dump_chat_id.front(), tail_len);
		EXPECT_THAT(tail, Pointwise(TailEq(), bs_tail));
		EXPECT_THAT(bs_tail, ContainerEq(part_tail));
	});
	EXPECT_TRUE(test::WaitFor("GetChatTail completion", *ev, g_max_wait_timeout_ms));
}
TEST_F(ChatContinuationFixture, GetChatMsgsTail)
{
	ASSERT_TRUE(InitPartsDBFilled());
	const auto &test_chat_id = g_dump_chat_id.front();
	auto part = std::weak_ptr<PartInstance>(
		only_users_.begin()->second.front());
	auto len = 30u;
	auto msgs = part.lock()->app_layer_->GetMessages(test_chat_id, {}, len);
	{
		auto part_lock = part.lock();
		ASSERT_TRUE(part_lock);
		EXPECT_THAT(msgs, SizeIs(len));
		auto chain = chat::chain::ChainContainer();
		for (const auto& i : msgs)
		{
			auto item = chat::GetMsgChainItem(i.first);
			chain.insert(item.msg_id, item.prev_id, item.timestamp);
		}
		std::vector<chat::chain_item> tail_for_cmp;
		std::transform(chain.begin(), chain.end(),
			std::back_inserter(tail_for_cmp),
			[](const chat::chain::detail::Data& data) -> chat::chain_item
		{ return { data.id, data.parent, data.timestamp, -1 }; });
		EXPECT_THAT(msgs, Pointwise(TailEq(), tail_for_cmp));
	}
}
TEST_F(ChatContinuationFixture, GetChatMsgsBeforeMsgID)
{
	ASSERT_TRUE(InitPartsDBFilled());
	const auto &test_chat_id = g_dump_chat_id.front();
	auto part = only_users_.begin()->second.front();
	auto len = 15u;
	chat::ChatMessageID first_in_tail;
	auto msgs = part->app_layer_->GetMessages(test_chat_id, {}, len);
	EXPECT_FALSE(msgs.empty());
	EXPECT_THAT(msgs, SizeIs(len));
	first_in_tail = chat::GetMsgChainItem(msgs.front().first).msg_id;
	msgs = part->app_layer_->GetMessages(test_chat_id, first_in_tail, len);
	EXPECT_THAT(msgs, SizeIs(len));
	auto tail_from_db = GetChatMsgsTailFromDB(
		test_chat_id, first_in_tail, len, part->db_conn_param.dbUri);
	EXPECT_THAT(msgs, Pointwise(TailEq(), tail_from_db));
}
TEST_P(ChatContinuationFixture, LostMessages)
{
	auto ev = std::make_shared<vs::event>(false);
	std::vector<chat::ChatMessageID> msgs;
	chat::ChatStoragePtr chat_storage;
	auto params = PrepareLostMessagesTestParams(g_dump_chat_id.front(),
		[ev,&msgs,&chat_storage, this](chat::msg::ChatMessagePtr &&msg, chat::CallIDRef)
		{
			msgs.erase(std::remove_if(msgs.begin(),msgs.end(),
				[&](const chat::ChatMessageID& check_msg_id)
				{
					return check_msg_id	==
						msg->GetParamStrRef(chat::attr::MESSAGE_ID_paramName);
				}),msgs.end());
			EXPECT_TRUE(OrderConsistencyCheck(g_dump_chat_id.front(), chat_storage)) <<
				"Processed message id = " <<
				msg->GetParamStrRef(chat::attr::MESSAGE_ID_paramName);
			if(!msgs.empty())
				return;
			msg.reset();
			ev->set();
		});
	ASSERT_TRUE(params.result);
	chat_storage = params.storage;
	std::transform(params.msgs_for_insert.begin(),params.msgs_for_insert.end(),
		std::back_inserter(msgs),[](const chat::msg::ChatMessagePtr &msg)
			{ return msg->GetParamStr(chat::attr::MESSAGE_ID_paramName);});
	for(auto && msg : params.msgs_for_insert)
	{
		params.msg_receiver->SendMsgAbove(std::move(msg),"fake_call_id");
	}
	EXPECT_TRUE(test::WaitFor("Processing messages by MainStorageLayer completion",
				*ev, g_max_wait_timeout_ms));
	EXPECT_TRUE(test::WaitFor("Message delivery completion",
		[&]() {return params.msg_sink->QueueEmpty(); }, 10, g_max_wait_timeout_ms));
}
INSTANTIATE_TEST_CASE_P(DBBackEndTest,
	ChatContinuationFixture,
	::testing::ValuesIn(GetSupportedBackEnds())
);
TEST_F(ChatContinuationFixture, RemoveParticipant)
{
	/**
		1. Init chat with filled db
		2. Check that part is in chat
		3. Remove participant
		4. Check that participant is removed from context of all participant
	*/
	ASSERT_TRUE(InitPartsDBFilled());
	const auto& chat_id = g_dump_chat_id.front();
	auto part = only_users_.begin()->second.front();
	auto part_id_for_remove = std::next(only_users_.begin())->first;
	/**
		check that all participants contain part_id_for_remove in chat context
	*/
	for (const auto& p : all_parts_)
	{
		for (const auto& instance : p.second)
		{
			auto ctx = instance->cfg_->GetChatStorage()->GetGlobalContext(chat_id);
			EXPECT_FALSE(ctx.chatId.empty());
			EXPECT_THAT(ctx.participants, Contains(PartByIDEq(part_id_for_remove)));
		}
	}
	part->app_layer_->RemoveParticipant(chat_id, part_id_for_remove, {});
	// wait untill all participants receive message
	auto instances = std::next(only_users_.begin())->second.size();
	EXPECT_TRUE(test::WaitFor("RemovePartMsg recv",
		[
			this,
			// all endpoints have to receive RemovePart msg except sender instance
			remov_part_count = EpCount() - 1,
			// Every instance of removed participant and BS send PartRemovedFromChat message to system chat,
			// and every instance receive all messages except its own.
			// So total count of received PartRemovedFromChatMessage is (instances + 1) * instances
			 part_removed_from_chat_count = (instances + 1) * instances
		]()
	{
		return remov_part_count == RecvMsgsCount(chat::MessageType::remove_part)
		&& part_removed_from_chat_count == RecvMsgsCount(chat::MessageType::remove_part_notification); },
		10, g_max_wait_timeout_ms));
	for (const auto& p : all_parts_)
	{
		if (p.first == part_id_for_remove)
			continue;
		for (const auto& instance : p.second)
		{
			auto ctx = instance->cfg_->GetChatStorage()->GetGlobalContext(chat_id);
			EXPECT_FALSE(ctx.chatId.empty());
			EXPECT_THAT(ctx.participants, Not(Contains(PartByIDEq(part_id_for_remove))));
		}
	}
}
TEST_P(ChatContinuationFixture, FetchAllPersonalContexts)
{
	/**
		Create users with empty db
		bs
	*/
	const auto &user1 = g_dump_users.front();
	ASSERT_TRUE(InitPartsFillDBExcept({ user1 }, GetParam() == DBBackEnd::postgresql ? true : false));
	auto part = only_users_.find(user1)->second.front();
	auto ev = std::make_shared<vs::event>(false);
	part->storage_layer_->FetchAllUserPersonalContexts(
		[ev, part]()
	{
		auto ctxs = part->app_layer_
			->GetMyPersonalContexts(std::numeric_limits<uint32_t>::max(), 1);
		VS_SCOPE_EXIT{ ev->set(); };
		ASSERT_FALSE(ctxs.empty());
		std::vector<chat::ChatID> chatIds;
		std::transform(ctxs.begin(), ctxs.end(), std::back_inserter(chatIds),
			[](auto&& ctx) {return std::move(ctx.chatId); });
		EXPECT_THAT(chatIds, UnorderedElementsAreArray(g_dump_chat_id));
	});
	EXPECT_TRUE(test::WaitFor("FetchAllPersonalContexts completion",
		*ev, g_max_wait_timeout_ms));
}
TEST_P(ChatContinuationFixture, ClearHistoryForAll)
{
	ClearHistory(true);
}
TEST_P(ChatContinuationFixture, ClearHistoryForParticipant)
{
	ClearHistory(false);
}
TEST_P(ChatContinuationFixture, RemoveMessageForAll)
{
	RemoveMessage(true);
}
TEST_P(ChatContinuationFixture, RemoveMessageForParticipant)
{
	RemoveMessage(false);
}
}