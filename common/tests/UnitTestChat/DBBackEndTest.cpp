#include "tests/UnitTestChat/DBBackEndFixture.h"
#include "tests/UnitTestChat/ChatContinuationFixture.h"
#include "tests/UnitTestChat/sql/TestDBDump.h"
#include "tests/UnitTestChat/TestHelpers.h"
#include "chatlib/chat_defs.h"
#include "chatlib/msg/attr.h"
#include "chatlib/msg/chat_messages_construct.h"
#include "chatlib/msg/parse_message.h"
#include "chatlib/storage/make_chat_storage.h"
#include "chatlib/utils/chat_utils.h"
#include "chatlib/utils/msg_utils.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/VS_Container_io.h"
#include "std-generic/cpplib/VS_CppDBIncludes.h"
#include "std/debuglog/VS_Debug.h"

#include <gtest/gtest.h>
#include <boost/algorithm/string/join.hpp>
#include <random>
#include <limits>

#define DEBUG_CURRENT_MODULE VS_DM_CHATS

using ::testing::UnorderedElementsAreArray;
using ::testing::Pointwise;
using ::testing::ContainerEq;
namespace
{
inline void AddBucketAndPrevious(
	chat::msg::ChatMessagePtr &msg,
	uint64_t bucket_id,
	chat::ChatMessageIDRef prev_msg)
{
	msg->SetParamI64(chat::attr::BUCKET_paramName, bucket_id);
	if (prev_msg.empty())
		return;
	msg->SetParam(chat::attr::PREVIOUS_MESSAGE_ID_paramName, prev_msg);
}
std::set<chat::ChatID> GetSystemChats(const std::vector<chat::CallID>& users,
	const std::vector<chat::BSInfo>& servers)
{
	std::set<chat::ChatID> res;
	for (const auto& user : users)
	{
		for (const auto& bs : servers)
		{
			res.emplace(chat::GetP2PChatID(user, bs));
		}
	}
	return res;
}
// Generate and save some messages in DB. Return list of the messages and list of chats
std::pair<
	std::vector<chat::msg::ChatMessagePtr>,
	std::set<chat::ChatID>
>
SaveSomeMessages(chat::ChatStoragePtr db, uint32_t bucket_capacity = 100)
{
	std::pair<
		std::vector<chat::msg::ChatMessagePtr>,
		std::set<chat::ChatID>
	> res;
	auto chat_id = chat::GenerateChatID();
	chat::CallID creator = "creator@trueconf.com";
	chat::CallID creator_ep = creator + "/1";
	chat::BSInfo creator_bs = "bs1.trueconf.com#bs";
	chat::CallID user = "user@trueconf.com";
	chat::CallID user_ep = "user@trueconf.com/1";
	chat::BSInfo user_bs = "bs2.trueconf.com#bs";
	std::string title = "Chat title";
	uint64_t current_bucket = 0;
	uint32_t msgs_in_curent_bucket = 0;
	res.second = GetSystemChats({ creator, user }, { creator_bs, user_bs });
	res.second.emplace(chat_id);
	auto process_bucket = [&]()
	{
		++msgs_in_curent_bucket;
		if (msgs_in_curent_bucket >= bucket_capacity)
		{
			++current_bucket;
			msgs_in_curent_bucket = 0;
		}
	};
	auto msg_ptr = chat::msg::CreateChatMessage(chat_id,
		chat::current_chat_version, chat::ChatType::symmetric,
		title, creator, creator_ep,
		vs::CallIDType::client, {}).AcquireMessage();
	// SaveChatMessage
	AddBucketAndPrevious(msg_ptr, current_bucket, {});
	process_bucket();
	auto msg_id = chat::ChatMessageID(msg_ptr->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(db->SaveChatMessage(msg_ptr, "test", false).error,
		chat::detail::SaveMessageError::success) << msg_ptr->GetContainer();
	res.first.emplace_back(std::move(msg_ptr));
	// 2.
	auto ctxStamp = chat::GlobalContext(
		chat_id, "Title", chat::ChatType::symmetric,
		chat::current_chat_version, creator,
		chat::ChatMessageTimestamp(),
		chat::ChatMessageTimestamp(),
		chat::ChatMessageID(),
		vs::set<chat::ParticipantDescr, vs::less<>>(),
		vs::set<chat::ParticipantDescr, vs::less<>>());
	msg_ptr = chat::msg::AddPartMessage(
		ctxStamp,
		creator,
		chat::ParticipantType::client,
		creator,
		creator_ep,
		vs::CallIDType::client,
		{}).AcquireMessage();
	AddBucketAndPrevious(msg_ptr, current_bucket, msg_id);
	process_bucket();
	msg_id = chat::ChatMessageID(msg_ptr->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(db->SaveChatMessage(msg_ptr, "test", false).error,
		chat::detail::SaveMessageError::success) << msg_ptr->GetContainer();
	res.first.emplace_back(std::move(msg_ptr));
	// 3.
	msg_ptr = chat::msg::AddPartMessage(
		ctxStamp,
		creator_bs,
		chat::ParticipantType::server,
		creator,
		creator_ep,
		vs::CallIDType::client,
		{}).AcquireMessage();
	AddBucketAndPrevious(msg_ptr, current_bucket, msg_id);
	process_bucket();
	msg_id = chat::ChatMessageID(msg_ptr->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(db->SaveChatMessage(msg_ptr, "test", false).error,
		chat::detail::SaveMessageError::success) << msg_ptr->GetContainer();
	res.first.emplace_back(std::move(msg_ptr));
	// 4.
	msg_ptr = chat::msg::AddPartMessage(
		ctxStamp,
		user,
		chat::ParticipantType::client,
		creator,
		creator_ep,
		vs::CallIDType::client,
		{}).AcquireMessage();
	AddBucketAndPrevious(msg_ptr, current_bucket, msg_id);
	process_bucket();
	msg_id = chat::ChatMessageID(msg_ptr->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(db->SaveChatMessage(msg_ptr, "test", false).error,
		chat::detail::SaveMessageError::success) << msg_ptr->GetContainer();
	res.first.emplace_back(std::move(msg_ptr));
	// 5.
	msg_ptr = chat::msg::AddPartMessage(
		ctxStamp,
		user_bs,
		chat::ParticipantType::server,
		creator,
		creator_ep,
		vs::CallIDType::client,
		{}).AcquireMessage();
	AddBucketAndPrevious(msg_ptr, current_bucket, msg_id);
	process_bucket();
	msg_id = chat::ChatMessageID(msg_ptr->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(db->SaveChatMessage(msg_ptr, "test", false).error,
		chat::detail::SaveMessageError::success) << msg_ptr->GetContainer();
	res.first.emplace_back(std::move(msg_ptr));
	// 6.
	auto msg_content = "Hello, PostgreSQL!";
	msg_ptr = chat::msg::ContentMessage{}.Text(msg_content).Seal(
		chat_id,
		creator,
		creator_ep,
		vs::CallIDType::client,
		{}, {});
	AddBucketAndPrevious(msg_ptr, current_bucket, msg_id);
	process_bucket();
	msg_id = chat::ChatMessageID(msg_ptr->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(db->SaveChatMessage(msg_ptr, "test", false).error,
		chat::detail::SaveMessageError::success) << msg_ptr->GetContainer();
	res.first.emplace_back(std::move(msg_ptr));

	// p2p chat
	current_bucket = 0;
	msgs_in_curent_bucket = 0;
	auto p2p_chat_id = chat::GetP2PChatID(creator, user);
	auto msg_content1 = "Msg1";
	auto msg_content2 = "Msg2";
	res.second.emplace(p2p_chat_id);
	msg_ptr = chat::msg::ContentMessage{}.Text(msg_content1).Seal(
		p2p_chat_id, creator, creator_ep, vs::CallIDType::client,
		{}, {});
	chat::msg::InsertParamStrIntoMsgContent(msg_ptr, chat::msg::toKeyName, user);
	AddBucketAndPrevious(msg_ptr, current_bucket, {});
	process_bucket();
	msg_id = chat::ChatMessageID(msg_ptr->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(db->SaveChatMessage(msg_ptr, "test", false).error,
		chat::detail::SaveMessageError::success) << msg_ptr->GetContainer();
	res.first.emplace_back(std::move(msg_ptr));

	msg_ptr = chat::msg::ContentMessage{}.Text(msg_content2).Seal(
		p2p_chat_id, user, user_ep, vs::CallIDType::client,
		{}, {});
	chat::msg::InsertParamStrIntoMsgContent(msg_ptr, chat::msg::toKeyName, creator);
	AddBucketAndPrevious(msg_ptr, current_bucket, msg_id);
	process_bucket();
	msg_id = chat::ChatMessageID(msg_ptr->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(db->SaveChatMessage(msg_ptr, "test", false).error,
		chat::detail::SaveMessageError::success) << msg_ptr->GetContainer();
	res.first.emplace_back(std::move(msg_ptr));
	return res;
}
}
namespace chat_test
{
TEST_P(DBBackEndFixture, Init)
{
	auto db = chat::make_chat_storage(GetConnString());
	ASSERT_TRUE(db);
}
TEST_P(DBBackEndFixture, MessageWithBinData)
{
	auto db = chat::make_chat_storage(GetConnString());
	ASSERT_TRUE(db);

	chat::ChatID chat_id = chat::GenerateChatID();
	VS_SCOPE_EXIT{
		CleanChat(chat_id);
	};
	chat::CallID creator = "creator@trueconf.com";
	chat::CallID creator_ep = creator + "/1";
	auto msg_content = "Hello, PostgreSQL!";
	auto msg_ptr = chat::msg::ContentMessage{}.Text(msg_content).Seal(
		chat_id,
		creator,
		creator_ep,
		vs::CallIDType::client,
		{}, {});
	std::string blob;
	constexpr auto blob_len = 256;
	for (int i = 0; i != blob_len; ++i) {
		blob.push_back(i);
	}
	msg_ptr->SetParam(chat::attr::BLOB_paramName, blob.data(), blob.size());
	auto res = db->SaveChatMessage(msg_ptr, "test", false);
	std::string msg_id;
	msg_ptr->GetParam(chat::attr::MESSAGE_ID_paramName, msg_id);
	auto new_msg_ptr = db->GetMessage(msg_id, creator);
	size_t new_blob_len;
	auto new_blob_ptr = static_cast<const char *>(
		new_msg_ptr->GetParamBinRef(chat::attr::BLOB_paramName, new_blob_len)
	);
	std::string new_blob{ new_blob_ptr, new_blob_ptr + new_blob_len };
	ASSERT_EQ(blob, new_blob);
}
TEST_P(DBBackEndFixture, MessageRemovalMessage)
{
	auto db = chat::make_chat_storage(GetConnString());
	ASSERT_TRUE(db);

	chat::ChatID chat_id = chat::GenerateChatID();
	VS_SCOPE_EXIT{
		CleanChat(chat_id);
	};
	for (auto for_all = 0; for_all != 2; ++for_all) {
		chat::CallID creator = "creator@trueconf.com";
		chat::CallID creator_ep = creator + "/1";
		auto deletee = chat::GenerateUUID();
		auto msg_ptr = chat::msg::MessageRemovalMessage(
			chat_id,
			deletee,
			for_all,
			creator,
			creator_ep,
			vs::CallIDType::client,
			{}).AcquireMessage();

		auto msg_id = msg_ptr->GetParamStr(chat::attr::MESSAGE_ID_paramName);

		EXPECT_TRUE(chat::msg::MessageRemovalMessage::IsMyMessage(msg_ptr));
		auto parse_res = chat::msg::MessageRemovalMessage::Parse(msg_ptr);
		EXPECT_TRUE(parse_res.success);
		EXPECT_EQ(parse_res.msg.chat_id, chat_id);
		EXPECT_EQ(parse_res.msg.msg_id, msg_id);
		EXPECT_EQ(parse_res.msg.for_all, for_all);
		EXPECT_EQ(parse_res.msg.content.removed_msg_id, deletee);

		auto res = db->SaveChatMessage(msg_ptr, "test", false);
		auto new_msg_ptr = db->GetMessage(msg_id, creator);
		ASSERT_EQ(chat::msg::GetParamStrFromMsgContent(msg_ptr, chat::msg::fromInstanceKeyName),
			chat::msg::GetParamStrFromMsgContent(new_msg_ptr, chat::msg::fromInstanceKeyName));
		EXPECT_TRUE(chat::msg::MessageRemovalMessage::IsMyMessage(new_msg_ptr));
		auto new_parse_res = chat::msg::MessageRemovalMessage::Parse(msg_ptr);
		EXPECT_TRUE(new_parse_res.success);
		EXPECT_EQ(new_parse_res.msg, parse_res.msg);
	}
}
TEST_P(DBBackEndFixture, ClearHistoryMessage)
{
	auto db = chat::make_chat_storage(GetConnString());
	ASSERT_TRUE(db);

	chat::ChatID chat_id = chat::GenerateChatID();
	VS_SCOPE_EXIT{
		CleanChat(chat_id);
	};
	for (auto for_all = 0; for_all != 2; ++for_all) {
		chat::CallID creator = "creator@trueconf.com";
		chat::CallID creator_ep = creator + "/1";
		auto msg_ptr = chat::msg::ClearHistoryMessage(
			chat_id,
			for_all,
			creator,
			creator_ep,
			vs::CallIDType::client,
			{}).AcquireMessage();

		std::string msg_id;
		msg_ptr->GetParam(chat::attr::MESSAGE_ID_paramName, msg_id);

		ASSERT_TRUE(chat::msg::ClearHistoryMessage::IsMyMessage(msg_ptr));
		auto parse_res = chat::msg::ClearHistoryMessage::Parse(msg_ptr);
		ASSERT_TRUE(parse_res.success);
		ASSERT_EQ(parse_res.msg.chat_id, chat_id);
		ASSERT_EQ(parse_res.msg.msg_id, msg_id);
		ASSERT_EQ(parse_res.msg.for_all, for_all);

		auto res = db->SaveChatMessage(msg_ptr, "test", false);
		auto new_msg_ptr = db->GetMessage(msg_id, creator);
		ASSERT_EQ(chat::msg::GetParamStrFromMsgContent(msg_ptr, chat::msg::fromInstanceKeyName),
			chat::msg::GetParamStrFromMsgContent(new_msg_ptr, chat::msg::fromInstanceKeyName));
		ASSERT_TRUE(chat::msg::ClearHistoryMessage::IsMyMessage(new_msg_ptr));
		auto new_parse_res = chat::msg::ClearHistoryMessage::Parse(msg_ptr);
		ASSERT_TRUE(new_parse_res.success);
		ASSERT_EQ(new_parse_res.msg, parse_res.msg);
	}
}
TEST_P(DBBackEndFixture, AddMessage)
{
	/**
		1. CreateChatMessage
		2. Add creator
		3. Add creator BS
		4. Add user
		5. Add user BS
		6. Send message to chat
	*/
	// create db
	auto db = chat::make_chat_storage(GetConnString());
	ASSERT_TRUE(db);
	// make CreateChatMessage
	std::set<chat::ChatID> chats_for_delete;
	VS_SCOPE_EXIT{
		for (const auto &id : chats_for_delete)
		{
			CleanChat(id);
		};
	};
	auto chat_id = chat::GenerateChatID();
	chats_for_delete.emplace(chat_id);

	chat::CallID creator = "creator@trueconf.com";
	chat::CallID creator_ep = creator + "/1";
	chat::BSInfo creator_bs = "bs1.trueconf.com#bs";
	chat::CallID user = "user@trueconf.com";
	chat::CallID user_ep = "user@trueconf.com/1";
	chat::BSInfo user_bs = "bs2.trueconf.com#bs";

	std::string title = "Chat title";
	auto system_chats = GetSystemChats({ creator, user }, { creator_bs, user_bs });
	std::move(system_chats.begin(), system_chats.end(), std::inserter(chats_for_delete, chats_for_delete.begin()));
	// 1.
	auto msg_ptr = chat::msg::CreateChatMessage(chat_id,
		chat::current_chat_version, chat::ChatType::symmetric,
		title, creator, creator_ep, vs::CallIDType::client, {}).AcquireMessage();
	// SaveChatMessage
	AddBucketAndPrevious(msg_ptr, 1, {});
	auto msg_id = chat::ChatMessageID(msg_ptr->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(db->SaveChatMessage(msg_ptr, "test", false).error,
		chat::detail::SaveMessageError::success) << msg_ptr->GetContainer();
	// 2.
	auto ctxStamp = chat::GlobalContext(
		chat_id, "Title", chat::ChatType::symmetric,
		chat::current_chat_version, creator,
		chat::ChatMessageTimestamp(),
		chat::ChatMessageTimestamp(),
		chat::ChatMessageID(),
		vs::set<chat::ParticipantDescr, vs::less<>>(),
		vs::set<chat::ParticipantDescr, vs::less<>>());
	msg_ptr = chat::msg::AddPartMessage(
		ctxStamp,
		creator,
		chat::ParticipantType::client,
		creator,
		creator_ep,
		vs::CallIDType::client,
		{}).AcquireMessage();
	AddBucketAndPrevious(msg_ptr, 1, msg_id);
	msg_id = chat::ChatMessageID(msg_ptr->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(db->SaveChatMessage(msg_ptr, "test", false).error,
		chat::detail::SaveMessageError::success) << msg_ptr->GetContainer();
	// 3.
	msg_ptr = chat::msg::AddPartMessage(
		ctxStamp,
		creator_bs,
		chat::ParticipantType::server,
		creator,
		creator_ep,
		vs::CallIDType::client,
		{}).AcquireMessage();
	AddBucketAndPrevious(msg_ptr, 1, msg_id);
	msg_id = chat::ChatMessageID(msg_ptr->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(db->SaveChatMessage(msg_ptr, "test", false).error,
		chat::detail::SaveMessageError::success) << msg_ptr->GetContainer();
	// 4.
	msg_ptr = chat::msg::AddPartMessage(
		ctxStamp,
		user,
		chat::ParticipantType::client,
		creator,
		creator_ep,
		vs::CallIDType::client,
		{}).AcquireMessage();
	AddBucketAndPrevious(msg_ptr, 1, msg_id);
	msg_id = chat::ChatMessageID(msg_ptr->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(db->SaveChatMessage(msg_ptr, "test", false).error,
		chat::detail::SaveMessageError::success) << msg_ptr->GetContainer();
	// 5.
	msg_ptr = chat::msg::AddPartMessage(
		ctxStamp,
		user_bs,
		chat::ParticipantType::server,
		creator,
		creator_ep,
		vs::CallIDType::client,
		{}).AcquireMessage();
	AddBucketAndPrevious(msg_ptr, 1, msg_id);
	msg_id = chat::ChatMessageID(msg_ptr->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(db->SaveChatMessage(msg_ptr, "test", false).error,
		chat::detail::SaveMessageError::success) << msg_ptr->GetContainer();
	// 6.
	auto msg_content = "Hello, PostgreSQL!";
	msg_ptr = chat::msg::ContentMessage{}.Text(msg_content).Seal(
		chat_id,
		creator,
		creator_ep,
		vs::CallIDType::client,
		{}, {});
	AddBucketAndPrevious(msg_ptr, 1, msg_id);
	msg_id = chat::ChatMessageID(msg_ptr->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(db->SaveChatMessage(msg_ptr, "test", false).error,
		chat::detail::SaveMessageError::success) << msg_ptr->GetContainer();
}
TEST_P(DBBackEndFixture, GetMessage)
{
	std::set<chat::ChatID> chats_for_delete;
	VS_SCOPE_EXIT{
		for (const auto &id : chats_for_delete)
		{
			CleanChat(id);
		};
	};
	auto db = chat::make_chat_storage(GetConnString());
	ASSERT_TRUE(db);
	auto saved_msg_info = SaveSomeMessages(db);
	auto& saved_msgs = saved_msg_info.first;
	chats_for_delete = std::move(saved_msg_info.second);
	ASSERT_FALSE(saved_msgs.empty());
	for (const auto &msg : saved_msgs)
	{
		auto msg_from_db = db->GetMessage(
			msg->GetParamStr(chat::attr::MESSAGE_ID_paramName), {});
		parse_msg_handler h1, h2;
		chat::msg::parse_message(msg, h1);
		chat::msg::parse_message(msg_from_db, h2);
		EXPECT_EQ(h1, h2) << "Original msg:\n" << msg->GetContainer() << '\n' <<
			"DB message:\n" << msg_from_db->GetContainer();
	}
}
TEST_P(ChatContinuationFixture, GetMessagesTail)
{
	auto res = LoadDBDump(false, "test_db");
	ASSERT_TRUE(res.result);
	auto sqlite_db = chat::make_chat_storage(res.dbConnParam.connectionString);
	auto tmp_db = CreateSharedDBInMemory(g_db_backend_name);
	ASSERT_TRUE(ResurrectDB(g_dump_bs, g_dump_bs, { GetParam() == DBBackEnd::postgresql ? GetConnString() : tmp_db.dbConnParam.connectionString, "" }, false, { res.dbConnParam.connectionString, "" }));
	auto db_backend = chat::make_chat_storage(GetConnString());
	for (const auto& id : g_dump_chat_id)
	{
		auto tail_from_sqlite = sqlite_db->GetLastMessages(id, std::numeric_limits<int32_t>::max());
		auto get_messages_res = db_backend->GetMessages(id, {}, {}, tail_from_sqlite.size());
		std::sort(get_messages_res.begin(), get_messages_res.end(),
			[](const auto& lhs, const auto& rhs) {return lhs.second < rhs.second; });
		std::vector<chat::chain_item> tail_from_db;
		std::transform(get_messages_res.begin(), get_messages_res.end(),
			std::back_inserter(tail_from_db),
			[](const auto& item)
		{
			return chat::GetMsgChainItem(item.first);
		});
		EXPECT_THAT(tail_from_sqlite, ContainerEq(tail_from_db));
	}
}
TEST_P(DBBackEndFixture, CountMessages)
{
	std::set<chat::ChatID> chats_for_delete;
	VS_SCOPE_EXIT{
		for (const auto &id : chats_for_delete)
		{
			CleanChat(id);
		};
	};
	auto db = chat::make_chat_storage(GetConnString());
	ASSERT_TRUE(db);
	std::map<chat::ChatID, uint32_t> msg_count;
	auto saved_msg_info = SaveSomeMessages(db);
	auto& saved_msgs = saved_msg_info.first;
	chats_for_delete = std::move(saved_msg_info.second);
	for (const auto& msg : saved_msgs)
	{
		++msg_count.emplace(msg->GetParamStrRef(chat::attr::CHAT_ID_paramName), 0).first->second;
	}
	for (const auto& counts : msg_count)
	{
		EXPECT_EQ(counts.second, db->CountMessagesInChat(counts.first)) <<
			"chat_id = " << counts.first;
	}
}
TEST_P(DBBackEndFixture, IsMessageExist)
{
	std::set<chat::ChatID> chats_for_delete;
	VS_SCOPE_EXIT{
		for (const auto &id : chats_for_delete)
		{
			CleanChat(id);
		};
	};
	auto db = chat::make_chat_storage(GetConnString());
	ASSERT_TRUE(db);
	// make CreateChatMessage
	auto chat_id = chat::GenerateChatID();
	chats_for_delete.emplace(chat_id);
	chat::CallID creator = "creator@trueconf.com";
	chat::CallID creator_ep = creator + "/1";
	chat::BSInfo creator_bs = "bs1.trueconf.com#bs";
	chat::CallID user = "user@trueconf.com";
	chat::CallID user_ep = "user@trueconf.com/1";
	chat::BSInfo user_bs = "bs2.trueconf.com#bs";

	std::string title = "Chat title";
	// 1.
	auto msg_ptr = chat::msg::CreateChatMessage(chat_id,
		chat::current_chat_version, chat::ChatType::symmetric,
		title, creator, creator_ep, vs::CallIDType::client,
		{}).AcquireMessage();
	// SaveChatMessage
	AddBucketAndPrevious(msg_ptr, 1, {});
	auto msg_id = chat::ChatMessageID(msg_ptr->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	EXPECT_EQ(db->SaveChatMessage(msg_ptr, "test", false).error,
		chat::detail::SaveMessageError::success);

	// Check existence
	EXPECT_TRUE(db->IsMessageExist(msg_id));
	auto msg_not_exist = chat::GenerateUUID();
	EXPECT_FALSE(db->IsMessageExist(msg_not_exist));
}
TEST_P(ChatContinuationFixture, GetUserPersonalConexts)
{
	auto res = LoadDBDump(false, g_dump_bs);
	ASSERT_TRUE(res.result);
	auto tmp_db = CreateSharedDBInMemory(g_db_backend_name);
	ASSERT_TRUE(ResurrectDB(g_dump_bs, g_dump_bs, { GetParam() == DBBackEnd::postgresql ? GetConnString() : tmp_db.dbConnParam.connectionString, "" }, false, { res.dbConnParam.connectionString, "" }));
	auto db = chat::make_chat_storage(GetConnString());
	ASSERT_TRUE(db);
	std::vector<chat::CallID> parts = g_dump_users;
	for (const auto& id : parts)
	{
		auto info_from_db = db->GetUserPersonalContexts(id, std::numeric_limits<int32_t>::max(), 1);
		std::vector<chat::ChatID> id_list;
		std::transform(info_from_db.begin(), info_from_db.end(), std::back_inserter(id_list),
			[](const auto& ctx)
		{
			return ctx.chatId;
		});
		EXPECT_THAT(g_dump_chat_id, UnorderedElementsAreArray(id_list));
	}
}
TEST_P(DBBackEndFixture, GetLastMessages)
{
	std::set<chat::ChatID> chats_for_delete;
	VS_SCOPE_EXIT{
		for (const auto &id : chats_for_delete)
		{
			CleanChat(id);
		};
	};
	auto db = chat::make_chat_storage(GetConnString());
	ASSERT_TRUE(db);
	auto saved_msg_info = SaveSomeMessages(db);
	auto& saved_msgs = saved_msg_info.first;
	chats_for_delete = std::move(saved_msg_info.second);
	ASSERT_FALSE(saved_msgs.empty());
	auto chat_id = saved_msgs.front()->GetParamStr(chat::attr::CHAT_ID_paramName);
	saved_msgs.erase(std::remove_if(saved_msgs.begin(), saved_msgs.end(),
		[&](const chat::msg::ChatMessagePtr& msg)
	{
		return msg->GetParamStrRef(chat::attr::CHAT_ID_paramName) != chat_id;
	}), saved_msgs.end());
	auto tail = db->GetLastMessages(chat_id, saved_msgs.size());
	ASSERT_FALSE(tail.empty());
	EXPECT_THAT(saved_msgs, Pointwise(TailEq(), tail));
}
TEST_P(DBBackEndFixture, GetMessagesInBucket)
{
	std::set<chat::ChatID> chats_for_delete;
	VS_SCOPE_EXIT{
		for (const auto &id : chats_for_delete)
		{
			CleanChat(id);
		};
	};
	auto db = chat::make_chat_storage(GetConnString());
	ASSERT_TRUE(db);
	auto saved_msg_info = SaveSomeMessages(db);
	auto& saved_msgs = saved_msg_info.first;
	chats_for_delete = std::move(saved_msg_info.second);
	ASSERT_FALSE(saved_msgs.empty());
	typedef std::set<chat::ChatMessageID, vs::str_less> MessagesSet;
	typedef std::map<uint64_t, MessagesSet> MessagesByBuckets;
	std::map<chat::ChatID, MessagesByBuckets> buckets_by_chats;
	for (const auto& msg : saved_msgs)
	{
		auto chat_id = msg->GetParamStrRef(chat::attr::CHAT_ID_paramName);
		uint64_t bucket_num(-1);
		ASSERT_TRUE(msg->GetParamI64(chat::attr::BUCKET_paramName, bucket_num));
		auto it = buckets_by_chats.emplace(chat_id, MessagesByBuckets()).first;
		auto buckets_it = it->second.emplace(bucket_num, MessagesSet()).first;
		buckets_it->second.emplace(
			msg->GetParamStrRef(chat::attr::MESSAGE_ID_paramName));
	}
	for (const auto& i : buckets_by_chats)
	{
		for (const auto& bucket : i.second)
		{
			auto msgs_in_bucket_from_db = db->GetMessagesInBucket(i.first, bucket.first);
			EXPECT_THAT(msgs_in_bucket_from_db, ::testing::UnorderedElementsAreArray(bucket.second));
		}
	}
}
TEST_P(DBBackEndFixture, AddGetGlobalCtx)
{
	std::set<chat::ChatID> chats_for_delete;
	VS_SCOPE_EXIT{
		for (const auto &id : chats_for_delete)
		{
			CleanChat(id);
		};
	};
	auto db = chat::make_chat_storage(GetConnString());
	ASSERT_TRUE(db);

	auto chat_id = chat::GenerateChatID();
	chats_for_delete.emplace(chat_id);
	auto tp = chat::ChatType::symmetric;
	std::string title = "Chat title";
	std::string ver = chat::current_chat_version;
	chat::CallID creator = "user1@trueconf.com";
	vs::set<chat::ParticipantDescr, vs::less<>> participants{
		{creator, chat::ParticipantType::client},
		{"user2@trueconf.com", chat::ParticipantType::client},
		{"bs1.trueconf.com#bs", chat::ParticipantType::server}
	};
	vs::set<chat::ParticipantDescr, vs::less<>> banList{
		{"banUser@trueconf.com", chat::ParticipantType::client}
	};
	auto msg_id = chat::GenerateUUID();
	auto chatCreateTimestamp = std::chrono::time_point_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now());
	auto ctxCreateTimestamp = std::chrono::time_point_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now()) + std::chrono::seconds(1);

	auto ctx = chat::GlobalContext(
		chat_id,
		title,
		tp,
		ver,
		creator,
		chatCreateTimestamp,
		ctxCreateTimestamp,
		msg_id,
		participants, banList);
	db->SaveGlobalContext(ctx);

	auto ctx_from_db = db->GetGlobalContext(chat_id);
	EXPECT_EQ(ctx_from_db.chatId, ctx.chatId);
	EXPECT_EQ(ctx_from_db.title, ctx.title);
	EXPECT_EQ(ctx_from_db.type, ctx.type);
	EXPECT_EQ(ctx_from_db.version, ctx.version);
	EXPECT_EQ(ctx_from_db.creator, ctx.creator);
	EXPECT_EQ(ctx_from_db.createTimestamp, ctx.createTimestamp);
	EXPECT_EQ(ctx_from_db.ctxCreateTimestamp, ctx.ctxCreateTimestamp);
	EXPECT_EQ(ctx_from_db.msgId, ctx.msgId);
	EXPECT_EQ(ctx_from_db.participants, ctx.participants);
	EXPECT_EQ(ctx_from_db.banList, ctx.banList);
}
TEST_P(ChatContinuationFixture, ChainConstructTest)
{
	auto sqlite_db = LoadDBDump(false, "test");
	ASSERT_TRUE(sqlite_db.result);
	auto sqlite_backend = chat::make_chat_storage(sqlite_db.dbConnParam.connectionString);
	auto db_backend = chat::make_chat_storage(GetConnString());
	ASSERT_TRUE(sqlite_backend);
	ASSERT_TRUE(db_backend);
	// for every chat get all messages from sqlite, shuffle and save in postgreSQL and ChatinContainer
	// get tail from postgresql and compare with result from ChainContainer
	for (const auto& chat_id : g_dump_chat_id)
	{
		auto items = sqlite_backend->GetLastMessages(chat_id, std::numeric_limits<int32_t>::max());
		chat::chain::ChainContainer chain;
		// shuffle messages
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(items.begin(), items.end(), g);
		for (const auto& i : items)
		{
			auto msg = sqlite_backend->GetMessage(i.msg_id, {});
			ASSERT_TRUE(msg);
			auto senderType = vs::CallIDType::undef;
			if (!msg->GetParamI32(chat::attr::SENDER_TYPE_paramName, senderType))
				msg->SetParamI32(chat::attr::SENDER_TYPE_paramName, vs::CallIDType::client);
			ASSERT_EQ(db_backend->SaveChatMessage(msg, "test", true).error,
				chat::detail::SaveMessageError::success);
			auto item = chat::GetMsgChainItem(msg);
			chain.insert(item.msg_id, item.prev_id, item.timestamp);
			auto chain_from_db = db_backend->GetLastMessages(chat_id, std::numeric_limits<int32_t>::max());
			std::vector<chat::ChatMessageID> chain_from_cnt;
			std::transform(chain.begin(), chain.end(), std::back_inserter(chain_from_cnt),
				[](const chat::chain::detail::Data& data) { return data.id; });
			std::vector<chat::ChatMessageID> msg_id_only_from_db;
			std::transform(chain_from_db.begin(), chain_from_db.end(),
				std::back_inserter(msg_id_only_from_db),
				[](const auto& item) {return item.msg_id; });
			EXPECT_EQ(chain_from_cnt, msg_id_only_from_db)
				<< "Chain construct failed for chat_id = " << chat_id << "\n"
				<< "Expected message order is:\n\t"
				<< boost::algorithm::join(chain_from_cnt, "\n\t")
				<< "\nActual message order is:\n\t"
				<< boost::algorithm::join(msg_id_only_from_db, "\n\t");
		}
	}
}
TEST_P(ChatContinuationFixture, GetMessagesToRetransmit)
{
	/*
		1. Init chat storage;
		2. Choose chat ID;
		3. Get last bucket number;
		4. Choose two buckets:
			bucket_0 - template, no manipulations with messages,
			bucket_n - for manipulations with messages;
		5. Get message IDs from choosen buckets;
		6. Checking that the function returns all messages from buckets when passing empty buckets;
		7. Save first and last message IDs from bucket_n
			and delete them in bucket_n;
		8. Add new message IDs in bucket_n;
		9. Fill struct with data from bucket_0 and bucket_n for synchronization;
		10. Check returned value of function ChatStorage::GetMessagesToRetransmit(...):
			for bucket_0, it must be empty;
			for bucket_n, it must return only deleted IDs from bucket_n.
	*/

	// 1.
	auto db = LoadDBDump(false, g_db_backend_name);
	ASSERT_TRUE(db.result);
	auto test = db.dbConnParam;
	if (GetParam() == DBBackEnd::postgresql)
		ASSERT_TRUE(FillChatWithPGBackEnd(db.dbConnParam.connectionString));
	auto chat_storage = chat::make_chat_storage(GetConnString());
	ASSERT_TRUE(chat_storage);
	// 2.
	const auto& chat_id = g_dump_chat_id.front();
	ASSERT_FALSE(chat_id.empty());
	// 3.
	uint64_t last_bucket = 0u;
	uint32_t unused = 0u;
	chat_storage->GetLastBucketInfo(chat_id, last_bucket, unused);
	// 4.
	uint64_t bucket_0 = 0u;
	ASSERT_FALSE(last_bucket == 0u);
	uint64_t bucket_n = last_bucket - 1u;
	ASSERT_FALSE(bucket_0 == bucket_n);
	// 5.
	auto msg_ids_from_bucket_0 = chat_storage->GetMessagesInBucket(chat_id, bucket_0);
	EXPECT_GT(msg_ids_from_bucket_0.size(), 0u);
	auto msg_ids_from_bucket_n = chat_storage->GetMessagesInBucket(chat_id, bucket_n);
	ASSERT_GT(msg_ids_from_bucket_n.size(), 1u);
	// 6.
	chat::msg::MsgIdsByBuckets forSync;
	forSync.emplace_back(bucket_0, std::vector<chat::ChatMessageID>{});
	forSync.emplace_back(bucket_n, std::vector<chat::ChatMessageID>{});
	auto fromBase = chat_storage->GetMessagesToRetransmit(chat_id, forSync);
	ASSERT_EQ(msg_ids_from_bucket_0.size() + msg_ids_from_bucket_n.size(), fromBase.size());
	std::set<chat::ChatMessageID, vs::str_less> msg_ids_from_base;
	std::transform(std::begin(fromBase), std::end(fromBase),
		std::inserter(msg_ids_from_base, std::end(msg_ids_from_base)),
		[&](const auto& item) { return item->GetParamStr(chat::attr::MESSAGE_ID_paramName); }
	);
	ASSERT_THAT(msg_ids_from_bucket_0, ::testing::IsSubsetOf(msg_ids_from_base));
	ASSERT_THAT(msg_ids_from_bucket_n, ::testing::IsSubsetOf(msg_ids_from_base));
	// 7.
	std::set<chat::ChatMessageID, vs::str_less> deleted_msg_ids_from_bucket_n;
	auto msg_id_it = std::begin(msg_ids_from_bucket_n);
	deleted_msg_ids_from_bucket_n.insert(std::move(*msg_id_it));
	msg_ids_from_bucket_n.erase(msg_id_it);
	msg_id_it = std::prev(msg_ids_from_bucket_n.end(), 1);
	deleted_msg_ids_from_bucket_n.insert(std::move(*msg_id_it));
	msg_ids_from_bucket_n.erase(msg_id_it);
	// 8.
	EXPECT_THAT(msg_ids_from_bucket_n, ::testing::Not(::testing::Contains("12345678-1234-1234-1234-123456789abc")));
	EXPECT_THAT(msg_ids_from_bucket_n, ::testing::Not(::testing::Contains("87654321-4321-4321-4321-cba987654321")));
	// 9.
	forSync.at(0).msgIds = std::move(msg_ids_from_bucket_0);
	forSync.at(1).msgIds = std::move(msg_ids_from_bucket_n);
	// 10.
	fromBase = chat_storage->GetMessagesToRetransmit(chat_id, forSync);
	ASSERT_EQ(fromBase.size(), 2u);
	msg_ids_from_base.clear();
	std::transform(std::begin(fromBase), std::end(fromBase),
		std::inserter(msg_ids_from_base, std::end(msg_ids_from_base)),
		[&](const auto& item) { return item->GetParamStr(chat::attr::MESSAGE_ID_paramName); }
	);
	ASSERT_EQ(deleted_msg_ids_from_bucket_n, msg_ids_from_base);
}
INSTANTIATE_TEST_CASE_P(DBBackEndTest,
	DBBackEndFixture,
	::testing::ValuesIn(GetSupportedBackEnds())
);
}