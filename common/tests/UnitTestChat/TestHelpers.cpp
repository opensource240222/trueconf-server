#include "TestHelpers.h"
#include "chatlib/msg/attr.h"
#include "chatlib/msg/chat_messages_construct.h"
#include "chatlib/utils/chat_utils.h"

#include "std-generic/cpplib/VS_CppDBIncludes.h"

#include <boost/algorithm/string/replace.hpp>

#include "sql/TestDBDump.h"

namespace chat_test
{
void PostgreSQLCleanChat(const chat::ChatID& chat_id) try
{
	if (testing::Test::HasFailure())
		return;
	std::string test_id = ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name();
	test_id += ".";
	test_id += ::testing::UnitTest::GetInstance()->current_test_info()->name();
	static const std::string purge_chat = "select chat.purge_chat(?,?);";
	auto cppdb_pool = cppdb::pool::create(GetPostgreSQLConfiguration());
	auto session = cppdb::session(cppdb_pool->open());
	auto st = session.prepare(purge_chat);
	st << chat_id << test_id;
	st.query();
}
catch (cppdb::cppdb_error&)
{
}
std::vector<chat::ChatID> GetAllSystemChatsForDump()
{
	std::vector<chat::ChatID> res;
	std::transform(g_dump_users.begin(), g_dump_users.end(),
		std::back_inserter(res),
		[&](const chat::CallID& call_id) {return chat::GetP2PChatID(call_id, g_dump_bs); });
	return res;
}
// return all chat ids from dump and all system chat for users from dump;
std::vector<chat::ChatID> GetAllDumpChats()
{
	std::vector<chat::ChatID> res = GetAllSystemChatsForDump();
	std::copy(g_dump_chat_id.begin(), g_dump_chat_id.end(),
		std::back_inserter(res));
	return res;
}
void CleanChatsFromPostgreSQL()
{
	if (HasPostgreSQLCfg())
	{
		for (const auto& id : GetAllDumpChats())
		{
			PostgreSQLCleanChat(id);
		}
	}
}

constexpr const char * db_conn_prefix = "sqlite3:busy_timeout=10000;db=";

std::pair<std::string, std::string> PrepareConnString(std::string name, bool in_memory)
{
	boost::replace_all(name, "/", "_");
	name += ".sqlite";
	std::string db_name = name;
	boost::replace_all(name, "#", "%23");
	name = "file:" + name;
	if (in_memory)
	{
		name += "?mode=memory&cache=shared";
		return { name, "" };
	}
	return { name, db_name };
}
CreateDBRes
CreateSharedDBInMemory(std::string name)
{
	auto p = PrepareConnString(name, !g_sqlite_on_disk);
	auto& db_uri = p.first;
	auto& db_name = p.second;
	if (!db_name.empty())
		std::remove(db_name.c_str());
	try
	{
		CppSQLite3DB db;
		db.open(db_uri.c_str());
		return { ::testing::AssertionSuccess(), std::move(db), SQLiteDBParam(db_conn_prefix + db_uri, db_uri) };
	}
	catch (CppSQLite3Exception &e)
	{
		return { ::testing::AssertionFailure()
			<< "CreateSharedDBInMemory(): uri = "
			<< db_uri << "; errorMessage() = " << e.errorMessage(), CppSQLite3DB(), SQLiteDBParam() };
	}
}
SQLiteDBParam
GetSQLITEConnectionString(std::string name, bool in_memory)
{
	auto p = PrepareConnString(name, in_memory);
	return SQLiteDBParam(db_conn_prefix + p.first, p.first);
}
CreateDBRes LoadDBDump(bool empty, std::string config)
{
	static auto preloaded_db = [] {
		CppSQLite3DB res;
		res.open(":memory:");
		res.execDML(g_test_db_dump.c_str());
		return res;
	}();
	try
	{
		auto create_db = CreateSharedDBInMemory(config);
		if (!empty) {
			preloaded_db.copyTo(create_db.dbConnParam.dbUri.c_str());
		}
		return create_db;
	}
	catch (CppSQLite3Exception &e)
	{
		return { ::testing::AssertionFailure()
			<< "LoadDBDump(): file_name = "
			<< config << "; errorMessage() = " << e.errorMessage(), CppSQLite3DB(), SQLiteDBParam() };
	}
}
std::vector<ChatMessageCommonFields>
GetChatMsgsTailFromDB(
	chat::ChatIDRef chat_id,
	chat::ChatMessageIDRef before_msg,//if empty then get from the end
	uint32_t len,
	const std::string &db_name)
{
	CppSQLite3DB db;
	try
	{
		std::vector<ChatMessageCommonFields> res;
		db.open(db_name.c_str());
		std::stringstream ss;
		ss <<
			"select id, parent_message_id, client_timestamp from messages where chat_id = \"" <<
			chat_id << "\"";
		if (!before_msg.empty())
		{
			ss <<
				" and (int_order, text_order) < (select int_order, text_order from messages where chat_id =\"" <<
				chat_id << "\" and id = \"" <<
				before_msg << "\")";

		}
		ss <<
			" order by int_order desc, text_order desc limit " << len;
		auto q = db.execQuery(ss.str().c_str());
		for (; !q.eof(); q.nextRow())
			res.emplace_back(chat_id,
				q.getStringField(0, ""),
				q.getStringField(1, ""),
				chat::uint_to_timestamp(q.getInt64Field(2)));
		std::reverse(res.begin(), res.end());
		return res;
	}
	catch (CppSQLite3Exception &)
	{
		return {};
	}
	assert(false);
	return {};
}
chat::msg::ChatMessagePtr
CreateMessageThatFollowsID(chat::ChatIDRef chat_id,
	chat::ChatMessageIDRef prev_id, chat::ChatMessageTimestamp timestamp,
	std::string prefix)
{
	// create main message
	// replace timestamp
	std::string msg_text = prefix + "message for test";
	auto msg = chat::msg::ContentMessage{}.Text(msg_text).Seal(chat_id, "test_from",
		"test_from_instance", vs::CallIDType::client,
		timestamp, {});
	// bucket = 0
	msg->SetParamI64(chat::attr::BUCKET_paramName, 0);
	msg->SetParam(chat::attr::PREVIOUS_MESSAGE_ID_paramName, prev_id);
	return msg;
}
chat::chain::ChainContainer
PrepareChain(const std::vector<chat::chain_item>&items)
{
	chat::chain::ChainContainer chain;
	for (const auto &i : items)
		chain.insert(i.msg_id, i.prev_id, i.timestamp);
	return chain;
}
void SaveDBToDisk(const std::map<std::string/**ep name*/, std::string /**uri*/> &parts)
{
	auto info = ::testing::UnitTest::GetInstance()->current_test_info();
	std::stringstream ss;
	ss <<
		info->test_case_name() <<
		"." <<
		info->name() <<
		".";
	for (const auto &part : parts)
	{
		auto name = ss.str() + part.first;
		boost::replace_all(name, "/", "%2F");
		boost::replace_all(name, "#", "%23");
		name += ".sqlite";
		CppSQLite3DB db;
		db.open(part.second.c_str());
		db.copyTo(name.c_str());
	}
}
bool CompareMsgsInDifferentView(
	const chat::ChatMessageID &lhs,
	const std::pair<chat::ChatMessageID, uint64_t> &rhs)
{
	return lhs == rhs.first;
}
bool CompareMsgsInDifferentView(
	const chat::msg::ChatMessagePtr& msg,
	const chat::chain_item& rhs)
{
	auto item = chat::GetMsgChainItem(msg);
	return item.msg_id == rhs.msg_id
		&& item.prev_id == rhs.prev_id
		&& item.timestamp == rhs.timestamp;
}
bool CompareMsgsInDifferentView(
	const chat::MessageWithOrder& lhs,
	const ChatMessageCommonFields& rhs)
{
	auto item = chat::GetMsgChainItem(lhs.first);
	return item.msg_id == rhs.msg_id
		&& item.prev_id == rhs.prev_id
		&& item.timestamp == rhs.time_stamp;
}
bool CompareMsgsInDifferentView(
	const chat::MessageWithOrder& lhs,
	const chat::chain_item& rhs)
{
	auto item = chat::GetMsgChainItem(lhs.first);
	return item.msg_id == rhs.msg_id
		&& item.prev_id == rhs.prev_id
		&& item.timestamp == rhs.timestamp;
}
bool CompareMsgsInDifferentView(
	chat::ChatMessageIDRef lhs,
	const chat::chain_item& rhs)
{
	return lhs == rhs.msg_id;
}
}