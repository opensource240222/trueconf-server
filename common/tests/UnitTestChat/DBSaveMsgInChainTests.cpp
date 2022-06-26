#include "tests/UnitTestChat/sql/TestDBDump.h"
#include "tests/UnitTestChat/TestHelpers.h"

#include "chatlib/storage/make_chat_storage.h"
#include "chatlib/utils/chat_utils.h"

#include "std-generic/compat/memory.h"
#include "std-generic/cpplib/macro_utils.h"
#include "std-generic/cpplib/scope_exit.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <limits>

namespace chat_test
{
static const std::string empty_db_name = "empty";
static const std::string origin_db_name = "origin";
class ChatStorageWrap
{
	chat::ChatStoragePtr m_storage;
	std::string m_uri;
	CppSQLite3DB db_holder;
public:
	ChatStorageWrap(std::string uri)
		: m_uri(std::move(uri))
	{
		m_storage = chat::make_chat_storage(m_uri);
		CleanChatsFromPostgreSQL();
	}
	ChatStorageWrap(std::string uri, CppSQLite3DB &&db)
		: m_uri(std::move(uri)), db_holder(std::move(db))
	{
		m_storage = chat::make_chat_storage(m_uri);
	}
	ChatStorageWrap(ChatStorageWrap &&) = default;
	const std::string &GetName() const
	{
		return m_uri;
	}
	chat::ChatStorage& Storage()
	{
		return *m_storage;
	}
};
struct SaveInChainTestParam
{
	VS_FORWARDING_CTOR4(SaveInChainTestParam,
			test_db, chat_id, msg_for_insert, expected_order){}
	std::shared_ptr<ChatStorageWrap> test_db; // prefilled db;
	chat::ChatID chat_id;
	chat::msg::ChatMessagePtr msg_for_insert;
	std::vector<chat::ChatMessageID> expected_order;
};
std::shared_ptr<ChatStorageWrap>
GetOriginDB()
{
	static std::weak_ptr<ChatStorageWrap> orig_storage;
	auto instance = orig_storage.lock();
	if(!instance)
	{
		auto res = LoadDBDump(false, origin_db_name);
		EXPECT_TRUE(res.result);
		orig_storage = instance = std::make_shared<ChatStorageWrap>(res.dbConnParam.connectionString, std::move(res.db));
	}
	return instance;
}
std::vector<chat::chain_item> GetChainFromOrigin(const chat::ChatID& chat_id, uint32_t skip_from_begin)
{
	auto storage = GetOriginDB();
	chat::ChatStorage& db_i = storage->Storage();
	auto msg_count = static_cast<size_t>(db_i.CountMessagesInChat(chat_id));
	return db_i.GetLastMessages(chat_id, msg_count - skip_from_begin);
}
std::unique_ptr<ChatStorageWrap>
CreateEmptyDB(bool is_pg)
{
	if (is_pg)
		return vs::make_unique<ChatStorageWrap>(GetPostgreSQLConfiguration());

	static size_t test_db_counter = 0;
	auto db_name = std::to_string(++test_db_counter) + empty_db_name;
	auto res = CreateSharedDBInMemory(std::move(db_name));
	return vs::make_unique<ChatStorageWrap>(res.dbConnParam.connectionString, std::move(res.db));
}
using InitSaveInChainTestParamFunc = std::function<SaveInChainTestParam()>;
InitSaveInChainTestParamFunc EmptyDBParentEmpty(bool is_pg = false)
{
	return (
		[is_pg]() -> SaveInChainTestParam
	{
		chat::ChatID chat_id = g_dump_chat_id.front();
		auto empty_db = CreateEmptyDB(is_pg);
		auto chain_items = GetChainFromOrigin(chat_id, 0);
		auto msg = GetOriginDB()->Storage().GetMessage(chain_items.front().msg_id, {});
		auto chain = PrepareChain({ chain_items.front() });
		std::vector<chat::ChatMessageID> expected_order;
		std::transform(chain.begin(), chain.end(),
			std::back_inserter(expected_order), [](const auto &data) {return data.id; });
		return { std::move(empty_db),
			chat_id,std::move(msg),
			std::move(expected_order)};
	});
}
InitSaveInChainTestParamFunc EmptyDBParentFilled(bool is_pg = false)
{
	return
		[is_pg]() -> SaveInChainTestParam
	{
		auto chat_id = g_dump_chat_id.front();
		auto empty_db = CreateEmptyDB(is_pg);
		auto chain_items = GetChainFromOrigin(chat_id, 0);
		auto insert_item = std::find_if(chain_items.begin(), chain_items.end(),
			[](const chat::chain_item& i) {return !i.prev_id.empty(); });
		auto chain = PrepareChain({ *insert_item });
		auto msg = GetOriginDB()->Storage().GetMessage(insert_item->msg_id, {});
		std::vector<chat::ChatMessageID> expected_order;
		std::transform(chain.begin(), chain.end(),
			std::back_inserter(expected_order), [](const chat::chain::detail::Data &data) {return data.id; });
		return { std::move(empty_db),
			chat_id,std::move(msg),
			std::move(expected_order) };
	};
}
enum class ParentPos
{
	undef,
	last,
	first,
	middle,
	empty,
	not_found
};
SaveInChainTestParam DBFilled(ParentPos position, bool from_begin, bool is_pg = false)
{
	/**
	 * 1. get all messages from origin
	 * 2. insert in chain
	 * 3. create message that follows last message, insert in chain (must be in back)
	 * 4. prepare param
	 * */
	auto chat_id = g_dump_chat_id.front();
	auto chain_items = GetChainFromOrigin(chat_id, from_begin ? 0 : 1);
	auto chain = PrepareChain(chain_items);
	if(from_begin)
		EXPECT_TRUE(chain.front().parent.empty());
	else
		EXPECT_FALSE(chain.front().parent.empty());
	auto test_db = CreateEmptyDB(is_pg);
	for (const auto& item : chain_items)
	{
		auto msg = GetOriginDB()->Storage().GetMessage(item.msg_id, {});
		test_db->Storage().SaveChatMessage(msg, "test", true);
	}
	chat::msg::ChatMessagePtr msg_for_insert;
	switch (position)
	{
	case ParentPos::empty:
		// insert before first by timeout
		msg_for_insert = CreateMessageThatFollowsID(chat_id, {},
			chain.front().timestamp - std::chrono::milliseconds(1), {});
		break;
	case ParentPos::first:
		// insert after second by timeout
		msg_for_insert = CreateMessageThatFollowsID(chat_id, chain.front().id,
			(++chain.begin())->timestamp + std::chrono::milliseconds(1), {});
		break;
	case ParentPos::last:
		// timestamp should not be affected. Use first message timestamp
		msg_for_insert = CreateMessageThatFollowsID(chat_id, chain.back().id,
			chain.front().timestamp, {});
		break;
	case ParentPos::middle:
		// parent is tenth element, insert after twentieth by timeout (if ordered by timeout)
		msg_for_insert = CreateMessageThatFollowsID(chat_id,
			std::next(chain.begin(), 9)->id,
			std::next(chain.begin(), 19)->timestamp + std::chrono::milliseconds(1), {});
		break;
	case ParentPos::not_found:
		// parent is absent (generate id), insert after 25 by timestamp
		msg_for_insert = CreateMessageThatFollowsID(chat_id,
			chat::GenerateUUID(),
			std::next(chain.begin(), 24)->timestamp + std::chrono::milliseconds(1), {});
		break;
	default:
		assert(false);
		break;
	}
	auto msg_item = chat::GetMsgChainItem(msg_for_insert);

	std::vector<chat::ChatMessageID> expected_order;
	// insert in chain
	chain.insert(msg_item.msg_id, msg_item.prev_id, msg_item.timestamp);
	std::transform(chain.begin(), chain.end(),
		std::back_inserter(expected_order),
		[](const chat::chain::detail::Data &item) {return item.id; });
	return { std::move(test_db), std::move(chat_id),
		std::move(msg_for_insert), std::move(expected_order) };
}
InitSaveInChainTestParamFunc DBFromBegin(ParentPos position, bool is_pg = false)
{
	return [position, is_pg]()->SaveInChainTestParam
	{
		return DBFilled(position, true, is_pg);
	};
}
InitSaveInChainTestParamFunc DBWithoutFirst(ParentPos position, bool is_pg = false)
{
	return [position, is_pg]() -> SaveInChainTestParam
	{
		return DBFilled(position, false, is_pg);
	};
}
class SaveMsgInChainTest:
	public ::testing::TestWithParam<InitSaveInChainTestParamFunc>
{
public:
	~SaveMsgInChainTest()
	{
		if (HasFailure())
		{
			std::map<std::string, std::string> uri;
			uri.emplace(std::string(), m_test_db->GetName());
			chat_test::SaveDBToDisk(uri);
		}
		else
			CleanChatsFromPostgreSQL();
	}
protected:
	SaveInChainTestParam GetInitParam()
	{
		auto res = GetParam()();
		m_test_db = res.test_db;
		return res;
	}
private:
	std::shared_ptr<ChatStorageWrap> m_test_db;
};
TEST_P(SaveMsgInChainTest, Ordering)
{
	auto param = GetInitParam();
	auto &db = param.test_db->Storage();
	auto save_result = db.SaveChatMessage(param.msg_for_insert, "test", true);
	ASSERT_EQ(save_result.error, chat::detail::SaveMessageError::success);
	// get order from db
	auto order_from_db = db.GetLastMessages(param.chat_id, std::numeric_limits<int32_t>::max());
	std::vector<chat::ChatMessageID> msg_id_only;
	std::transform(order_from_db.begin(), order_from_db.end(),
		std::back_inserter(msg_id_only), [](const auto& i) {return i.msg_id; });
	EXPECT_THAT(msg_id_only, param.expected_order);
}
INSTANTIATE_TEST_CASE_P(
		SaveChainInSQLite,
		SaveMsgInChainTest,
		testing::Values( EmptyDBParentEmpty()
			, EmptyDBParentFilled()
			, DBFromBegin(ParentPos::last)
			, DBFromBegin(ParentPos::first)
			, DBFromBegin(ParentPos::empty)
			, DBFromBegin(ParentPos::middle)
			, DBFromBegin(ParentPos::not_found)
			, DBWithoutFirst(ParentPos::last)
			, DBWithoutFirst(ParentPos::first)
			, DBWithoutFirst(ParentPos::middle)
			, DBWithoutFirst(ParentPos::empty)
			, DBWithoutFirst(ParentPos::not_found)
			)
		);
#ifdef CHAT_PG_CONFIG
INSTANTIATE_TEST_CASE_P(
		SaveChainInPostgreSQL,
		SaveMsgInChainTest,
		testing::Values( EmptyDBParentEmpty(true)
			, EmptyDBParentFilled(true)
			, DBFromBegin(ParentPos::last, true)
			, DBFromBegin(ParentPos::first, true)
			, DBFromBegin(ParentPos::empty, true)
			, DBFromBegin(ParentPos::middle, true)
			, DBFromBegin(ParentPos::not_found, true)
			, DBWithoutFirst(ParentPos::last, true)
			, DBWithoutFirst(ParentPos::first, true)
			, DBWithoutFirst(ParentPos::middle, true)
			, DBWithoutFirst(ParentPos::empty, true)
			, DBWithoutFirst(ParentPos::not_found, true)
			)
		);
#endif
}
