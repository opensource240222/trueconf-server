#include "DBProcWrap.h"
#include "chatlib/msg/chat_messages_construct.h"
#include "chatlib/storage/PostgreSQLProcDecl.h"
#include "chatlib/storage/SQLITEProcDecl.h"
#include "chatlib/utils/chat_utils.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <chatlib/log/chatlog.h>

#include <std-generic/cpplib/VS_CppDBIncludes.h>

#include <sstream>

template<typename Stream>
struct logged_stream
	// We only use this struct for DB statement logging, here, so verbose
	: chat::VerboseLogStream
{
	Stream &stream;
	logged_stream(Stream &stream, const char *header) : stream(stream) {
		static_cast<chat::VerboseLogStream&>(*this) << header << ":";
	}
	template<typename T>
	logged_stream & operator<<(const T& t) {
		static_cast<chat::VerboseLogStream&>(*this) << "\n\t(" << t << ")";
		stream << t;
		return *this;
	}
};

template<typename T>
std::ostream&
operator<<(std::ostream &str, const cppdb::tags::use_tag<T> &t) {
	if (t.tag == cppdb::null_tag_type::not_null_value) {
		return str << t.value;
	}
	else {
		return str << "<NULL>";
	}
}

template<typename Stream>
logged_stream<Stream> make_logged(Stream &st, const char *header) {
	return { st, header };
}

#define LOGGED(st) make_logged(st, __func__)

namespace rj = rapidjson;
namespace chat
{

namespace db_proc
{
cppdb::ref_ptr<cppdb::backend::connection> InitSQLiteDB(string_view config)
{
	auto pool = cppdb::pool::create(std::string(config));
	assert(pool);
	auto conn = pool->open();
	auto ses = cppdb::session(conn);
	if (!IsSQLite3(ses))
	{
		assert(false);
		return {};
	}
	for (const auto& st : sqlite_qry::initScript)
	{
		ses.prepare(st.query).exec();
	}
	return conn;
}
void AddMessage(cppdb::session& ses, const ChatMessageID& msgId, const ChatID& chatId,
	ChatMessageTimestamp stamp, uint64_t bucketNum, uint32_t msgType, const CallID& sender,
	int32_t senderType, const ChatMessageID& parentMsgId, const ChatMessageID& originMsgId,
	const std::string& content, const void *blob_ptr, size_t blob_len, const QueryResultCallBack& cb)
{
	const auto& query = IsPostgreSQL(ses)
		? pg_qry::addMessage
		: sqlite_qry::addMessage;

	auto parentMsgIdTag = parentMsgId.empty()
		? cppdb::null_value
		: cppdb::not_null_value;
	auto originMsgIdTag = originMsgId.empty()
		? cppdb::null_value
		: cppdb::not_null_value;
	for (const auto& q : query)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) <<
				msgId <<
				chatId <<
				timestamp_to_uint(stamp) <<
				bucketNum <<
				msgType <<
				sender <<
				senderType <<
				cppdb::use(parentMsgId, parentMsgIdTag) <<
				cppdb::use(originMsgId, originMsgIdTag) <<
				content);
			if (blob_ptr != nullptr) {
				auto blob_char_ptr = static_cast<const char *>(blob_ptr);
				std::istringstream binDataIstream{ std::string(blob_char_ptr, blob_char_ptr + blob_len) };
				st << static_cast<std::istream&>(binDataIstream);
			}
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void UpdateMessageChain(cppdb::session& ses, const ChatMessageID& msgId,
	const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::updateMessageChain
		: sqlite_qry::updateMessageChain;
	for(const auto& q: queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
			CHAT_TRACE(LOGGED(st) << msgId);
		if(q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void CreateGlobalContext(cppdb::session& ses,
	const ChatMessageID& msgId, const ChatID& chatId,
	chat::MessageType type, const CallID& sender,
	const std::string& content)
{
	cppdb::statement st;
	const std::vector<chat::detail::ScriptDescr>* queries(nullptr);
	if (IsPostgreSQL(ses))
	{
		queries = &pg_qry::createGlobalContext;
	}
	else
	{
		switch (type)
		{
		case MessageType::create_chat:
			queries = &sqlite_qry::createGlobalContext100;
			break;
		case MessageType::create_p2p_chat:
			queries = &sqlite_qry::createGlobalContext101;
			break;
		case MessageType::add_part:
			queries = &sqlite_qry::createGlobalContext110;
			break;
		case MessageType::remove_part:
			queries = &sqlite_qry::createGlobalContext111;
			break;
		default:
			assert(false);
			return;
		}
	}
	for (const auto& q : *queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) <<
				msgId <<
				chatId <<
				static_cast<int32_t>(type) <<
				sender <<
				content);
		}
		if (q.how == chat::detail::ExecType::execute)
			st.exec();
		else
			st.query();
	}
}
void CreatePersonalContext(cppdb::session& ses, const ChatMessageID& msgId,
	MessageType type, const std::string& content)
{
	const std::vector<chat::detail::ScriptDescr>* queries(nullptr);
	if (IsPostgreSQL(ses))
	{
		queries = &pg_qry::createPersonalContext;
	}
	else
	{
		switch (type)
		{
		case MessageType::add_part_notification:
			queries = &sqlite_qry::createPersonalContext400;
			break;
		case MessageType::remove_part_notification:
			queries = &sqlite_qry::createPersonalContext401;
			break;
		default:
			assert(false);
			return;
		}
	}
	for(const auto& q: *queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) <<
				msgId <<
				static_cast<int32_t>(type) <<
				content);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			st.query();
		}
	}
}
void GetParticipants(cppdb::session& ses, const ChatID& chatId, const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::getParticipants
		: sqlite_qry::getParticipants;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << chatId);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}

	}
}
void HasMessage(cppdb::session&& ses, const ChatMessageID& msgId, const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::hasMessage
		: sqlite_qry::hasMessage;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if(q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << msgId);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void GetLastBucketNumber(cppdb::session&& ses, const ChatID& chatId,
	const QueryResultCallBack& cb)
{
	const auto& queries =IsPostgreSQL(ses)
		? pg_qry::getLastBucketNumber
		: sqlite_qry::getLastBucketNumber;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << chatId);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void GetLastMessages(cppdb::session&& ses, const ChatID& chatId, size_t len,
	const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::getLastMessages
		: sqlite_qry::getLastMessages;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << chatId << len);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
 void GetMessage(cppdb::session&& ses, const ChatMessageID& msgId, const CallID& currentUserId, const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::getMessage
		: sqlite_qry::getMessage;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << msgId << currentUserId);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void GetMessages(cppdb::session&& ses, const ChatID& chatId, const CallID& currentUserId,
	const ChatMessageID& lastMsgId, size_t count, const QueryResultCallBack& cb)
{
	auto lastMsgIdTag = lastMsgId.empty()
		? cppdb::null_value
		: cppdb::not_null_value;
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::getMessages
		: sqlite_qry::getMessages;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) <<
				chatId <<
				currentUserId <<
				count <<
				cppdb::use(lastMsgId, lastMsgIdTag));
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void GetMessagesByBucket(cppdb::session&& ses, const ChatID& chatId, uint64_t bucketNum,
	const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::getMessagesByBucketNumber
		: sqlite_qry::getMessagesByBucketNumber;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << chatId << bucketNum);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void GetGlobalContext(cppdb::session&& ses, const ChatID& chatId,
	const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::getGlobalContext
		: sqlite_qry::getGlobalContext;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << chatId);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void GetUserPersonalContexts(cppdb::session&& ses, const CallID& callId,
	uint32_t pageSize, uint32_t pageNum, const QueryResultCallBack& cb)
{
	// No support for ints above 2^31 - 1 in SQL
	// With this, client can pass std::numeric_limits<uint32_t>::max()
	// (note unsigned) here to get all the pages at once
	constexpr auto max_db_int = std::numeric_limits<int32_t>::max();
	if (pageSize >= max_db_int) {
		pageSize = max_db_int;
	}
	// No support for ints above 2^31 - 1 in SQL
	if (pageNum >= max_db_int) {
		pageNum = max_db_int;
	}
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::getUserPersonalContexts
		: sqlite_qry::getUserPersonalContexts;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << callId << pageSize << pageNum);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void GetMessagesToRetransmit(cppdb::session&& ses,const ChatID& chatId, const std::string& bucketsJsonArray,
	const std::string& messagesJsonArray, const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::getMessagesToRetransmit
		: sqlite_qry::getMessagesToRetransmit;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << chatId
				<< bucketsJsonArray
				<< messagesJsonArray);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void CountChatMessages(cppdb::session&& ses, const ChatID& chatId,
	const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::countChatMessages
		: sqlite_qry::countChatMessages;
	cppdb::result result;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << chatId);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void AddFirstVisibleMessage(cppdb::session& ses, const ChatMessageID& msgId, const CallID& currentUserId,
	const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::addFirstVisibleMessage
		: sqlite_qry::addFirstVisibleMessage;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << msgId << currentUserId);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void AddGlobalContext(cppdb::session&& ses, const GlobalContext& ctx,
	const QueryResultCallBack& cb)
{
	auto parts_json = msg::PartListToJSON(ctx.participants);
	auto ban_list_json = msg::PartListToJSON(ctx.banList);
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::addGlobalContext
		: sqlite_qry::addGlobalContext;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) <<
				ctx.chatId <<
				static_cast<uint32_t>(ctx.type) <<
				ctx.version <<
				ctx.creator <<
				timestamp_to_uint(ctx.createTimestamp) <<
				timestamp_to_uint(ctx.ctxCreateTimestamp) <<
				ctx.msgId <<
				ctx.title <<
				parts_json <<
				ban_list_json);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void AddPersonalContext(cppdb::session&& ses, const PersonalContext& ctx)
{
	rj::StringBuffer sb;
	rj::Writer<rj::StringBuffer> writer(sb);
	writer.StartArray();
	for (const auto& id : ctx.unreadMsgs)
	{
		writer.String(id.c_str(),
			static_cast<rj::SizeType>(id.length()));
	}
	writer.EndArray();
	auto unreadMsgs = std::string(sb.GetString(), sb.GetLength());
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::addPersonalContext
		: sqlite_qry::addPersonalContext;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) <<
				ctx.owner <<
				ctx.chatId <<
				static_cast<uint32_t>(ctx.chatType) <<
				ctx.version <<
				ctx.creator <<
				timestamp_to_uint(ctx.chatCreatedTime) <<
				timestamp_to_uint(ctx.ctxCreatedTime) <<
				ctx.msgId <<
				ctx.isDeleted <<
				ctx.getNotification <<
				ctx.title <<
				unreadMsgs <<
				ctx.draft);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			st.query();
		}
	}
}
void GetPersonalContext(cppdb::session&& ses, const ChatID& chatId, const CallID& owner,
	const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::getPersonalContext
		: sqlite_qry::getPersonalContext;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << chatId << owner);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void AddUndeliveredMessage(cppdb::session& ses, const ChatMessageID& msgId,
	const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::addUndeliveredMessage
		: sqlite_qry::addUndeliveredMessage;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << msgId);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void RemoveUndeliveredMessage(cppdb::session&& ses, const ChatMessageID& msgId, const CallID& partName,
	const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::removeUndeliveredMessage
		: sqlite_qry::removeUndeliveredMessage;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << msgId << partName);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void GetUndeliveredMessagesByChatId(cppdb::session&& ses, const ChatID& chatId,
	const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::getUndeliveredMessagesByChatId
		: sqlite_qry::getUndeliveredMessagesByChatId;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << chatId);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void GetAllUndeliveredMessages(cppdb::session&& ses, const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::getAllUndeliveredMessages
		: sqlite_qry::getAllUndeliveredMessages;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void GetMessagesForGlobalContext(cppdb::session& ses, const ChatMessageID& msgId,
	const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::getMessagesForGlobalContext
		: sqlite_qry::getMessagesForGlobalContext;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << msgId);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
void GetMessagesForPersonalContext(cppdb::session& ses, const ChatMessageID& msgId,
	const QueryResultCallBack& cb)
{
	const auto& queries = IsPostgreSQL(ses)
		? pg_qry::getMessagesForPersonalContext
		: sqlite_qry::getMessagesForPersonalContext;
	for (const auto& q : queries)
	{
		auto st = ses.prepare(q.query);
		if (q.usePlaceholder)
		{
			CHAT_TRACE(LOGGED(st) << msgId);
		}
		if (q.how == chat::detail::ExecType::execute)
		{
			st.exec();
		}
		else
		{
			cb(st.query());
		}
	}
}
}
}