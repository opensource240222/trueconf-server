#pragma once
#include "chatlib/callbacks.h"
#include "chatlib/chain/ChainContainer.h"
#include "chatlib/chat_defs.h"
#include "chatlib/messages_types.h"

#include "std-generic/cpplib/macro_utils.h"
#include "std-generic/sqlite/CppSQLite3.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <boost/variant.hpp>

#include "chatutils/helpers.h"

#if defined(__has_include)
#	if __has_include("tests/ChatPostgreSQLBackend_config.h")
#		include "tests/ChatPostgreSQLBackend_config.h"
#	endif
#endif

namespace chat_test
{
constexpr auto g_sqlite_on_disk = false;
const std::string g_db_backend_name("db_backend_test");
enum class DBBackEnd
{
	sqlite,
	postgresql
};
// FIXME: Delete struct
struct ChatMessageCommonFields
{
	VS_FORWARDING_CTOR4(ChatMessageCommonFields,
		chat_id,
		msg_id,
		prev_id,
		time_stamp) {}
	chat::ChatID chat_id;
	chat::ChatMessageID msg_id;
	chat::ChatMessageID prev_id;
	chat::ChatMessageTimestamp time_stamp;
};
struct CreateDBRes
{
	VS_FORWARDING_CTOR3(CreateDBRes, result, db, dbConnParam) {}
	::testing::AssertionResult result;
	CppSQLite3DB db;
	SQLiteDBParam dbConnParam;
};
using MessagesUnion = boost::variant<
	chat::msg::create_chat,
	chat::msg::create_p2p_chat,
	chat::msg::invite,
	chat::msg::invite_response,
	chat::msg::add_part,
	chat::msg::remove_part,
	chat::msg::text_message,
	chat::msg::part_added_to_chat,
	chat::msg::part_removed_from_chat,
	chat::msg::remove_message,
	chat::msg::clear_history,
	chat::msg::unrecognized_msg>;

struct parse_msg_handler
{
	MessagesUnion parsed_msg;
	bool operator==(const parse_msg_handler& op) const
	{
		return parsed_msg == op.parsed_msg;
	}
	void operator()(chat::msg::create_chat&& msg)
	{
		save(std::move(msg));
	}
	void operator()(chat::msg::create_p2p_chat&& msg)
	{
		save(std::move(msg));
	}
	void operator()(chat::msg::invite&& msg)
	{
		save(std::move(msg));
	}
	void operator()(chat::msg::invite_response&& msg)
	{
		save(std::move(msg));
	}
	void operator()(chat::msg::add_part&& msg)
	{
		save(std::move(msg));
	}
	void operator()(chat::msg::remove_part&& msg)
	{
		save(std::move(msg));
	}
	void operator()(chat::msg::text_message&& msg)
	{
		save(std::move(msg));
	}
	void operator()(chat::msg::part_added_to_chat&& msg)
	{
		save(std::move(msg));
	}
	void operator()(chat::msg::part_removed_from_chat&& msg)
	{
		save(std::move(msg));
	}
	void operator()(chat::msg::remove_message&& msg)
	{
		save(std::move(msg));
	}
	void operator()(chat::msg::clear_history&& msg)
	{
		save(std::move(msg));
	}
	void operator()(chat::msg::unrecognized_msg &&msg)
	{
		save(std::move(msg));
	}
	void save(MessagesUnion &&msg)
	{
		parsed_msg = std::move(msg);
	}
};
CreateDBRes CreateSharedDBInMemory(std::string name);
SQLiteDBParam
GetSQLITEConnectionString(std::string name, bool in_memory = false);
CreateDBRes LoadDBDump(bool empty, std::string config);
inline std::string GetPostgreSQLConfiguration()
{
#ifdef CHAT_PG_CONFIG
	return CHAT_PG_CONFIG;
#else
	return {};
#endif
}
inline bool HasPostgreSQLCfg()
{
#ifdef CHAT_PG_CONFIG
	return true;
#else
	return false;
#endif
}
inline std::vector<DBBackEnd> GetSupportedBackEnds()
{
	if (HasPostgreSQLCfg())
		return { DBBackEnd::sqlite, DBBackEnd::postgresql };
	else
		return { DBBackEnd::sqlite };
}
void PostgreSQLCleanChat(const chat::ChatID& chat_id);
std::vector<chat::ChatID> GetAllSystemChatsForDump();
void CleanChatsFromPostgreSQL();
// FIXME: Delete function
std::vector<ChatMessageCommonFields>
GetChatMsgsTailFromDB(
	chat::ChatIDRef chat_id,
	chat::ChatMessageIDRef before_msg,//if empty then get from the end
	uint32_t len,
	const std::string& db_name);
chat::msg::ChatMessagePtr
CreateMessageThatFollowsID(chat::ChatIDRef chat_id,
	chat::ChatMessageIDRef prev_id, chat::ChatMessageTimestamp timestamp,
	std::string prefix);
chat::chain::ChainContainer
PrepareChain(const std::vector<chat::chain_item>& items);
void SaveDBToDisk(const std::map<std::string/**ep name*/, std::string /**uri*/> &parts);
bool CompareMsgsInDifferentView(
	const chat::ChatMessageID &lhs,
	const std::pair<chat::ChatMessageID, uint64_t> &rhs);
bool CompareMsgsInDifferentView(
	const chat::msg::ChatMessagePtr& msg,
	const chat::chain_item& item);
bool CompareMsgsInDifferentView(
	const chat::MessageWithOrder& lhs,
	const ChatMessageCommonFields& rhs);
bool CompareMsgsInDifferentView(
	const chat::MessageWithOrder& lhs,
	const chat::chain_item& rhs);
bool CompareMsgsInDifferentView(
	chat::ChatMessageIDRef lhs,
	const chat::chain_item& rhs);


MATCHER(TailEq, "")
{
	return CompareMsgsInDifferentView(::testing::get<0>(arg), ::testing::get<1>(arg));
}

MATCHER_P(PartByIDEq, partId, "")
{
	return arg.partId == partId;
}
}
