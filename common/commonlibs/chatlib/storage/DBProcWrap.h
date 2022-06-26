#pragma once
#include "chatlib/chat_defs.h"
#include "chatlib/storage/helpers.h"

#include "std-generic/cpplib/VS_CppDBIncludes.h"

// For not defining GetMessage -> GetMessage(A/W)
#include "std-generic/undef_windows.h" // this should be last

namespace chat
{

inline bool IsSQLite3(cppdb::session& ses)
{
	return ses.engine() == "sqlite3";
}
inline bool IsPostgreSQL(cppdb::session& ses)
{
	return ses.engine() == "postgresql";
}

namespace db_proc
{
using QueryResultCallBack = std::function<void(cppdb::result&&)>;
cppdb::ref_ptr<cppdb::backend::connection> InitSQLiteDB(string_view config);

void AddMessage(cppdb::session& ses, const ChatMessageID& msgId, const ChatID& chatId,
	ChatMessageTimestamp stamp, uint64_t bucketNum, uint32_t msgType, const CallID& sender,
	int32_t senderType, const ChatMessageID& parentMsgId, const ChatMessageID& originMsgId,
	const std::string& content, const void * blob_ptr, size_t blob_len, const QueryResultCallBack& cb);
void UpdateMessageChain(cppdb::session& ses, const ChatMessageID& msdId,
	const QueryResultCallBack& cb);
void CreateGlobalContext(cppdb::session& ses, const ChatMessageID& msgId, const ChatID& chatId,
	MessageType type, const CallID& sender, const std::string& content);
void CreatePersonalContext(cppdb::session& ses, const ChatMessageID& msgId,
	MessageType type, const std::string& content);
void GetParticipants(cppdb::session& ses, const ChatID& msgId,
	const QueryResultCallBack& cb);
void HasMessage(cppdb::session&& ses, const ChatMessageID& chatId,
	const QueryResultCallBack& cb);
void GetLastBucketNumber(cppdb::session&& ses, const ChatID& chatId,
	const QueryResultCallBack& cb);
void GetLastMessages(cppdb::session&& ses, const ChatID& chatId, size_t len,
	const QueryResultCallBack& cb);
void GetMessage(cppdb::session&& ses, const ChatMessageID& msgId, const CallID& currentUserId,
	const QueryResultCallBack& cb);
void GetMessages(cppdb::session&& ses, const ChatID& chatId, const CallID& currentUserId,
	const ChatMessageID& lastMsgId, size_t count, const QueryResultCallBack& cb);
void GetMessagesByBucket(cppdb::session&& ses, const ChatID& chatId, uint64_t bucketNum,
	const QueryResultCallBack& cb);
void GetGlobalContext(cppdb::session&& ses, const ChatID& chatId,
	const QueryResultCallBack& cb);
void GetUserPersonalContexts(cppdb::session&& ses, const CallID& callId,
	uint32_t pageSize, uint32_t pageNum, const QueryResultCallBack& cb);
void GetMessagesToRetransmit(cppdb::session&& ses, const ChatID& chatId, const std::string& bucketsJsonArray,
	const std::string& messagesJsonArray, const QueryResultCallBack& cb);
void CountChatMessages(cppdb::session&& ses, const ChatID& chatId,
	const QueryResultCallBack& cb);
void AddFirstVisibleMessage(cppdb::session& ses, const ChatMessageID& msgId, const CallID& currentUserId,
	const QueryResultCallBack& cb);
void AddGlobalContext(cppdb::session&& ses, const GlobalContext& ctx,
	const QueryResultCallBack& cb);
void AddPersonalContext(cppdb::session&& ses, const PersonalContext& ctx);
void GetPersonalContext(cppdb::session&& ses, const ChatID& chatId, const CallID& owner,
	const QueryResultCallBack& cb);
void AddUndeliveredMessage(cppdb::session& ses, const ChatMessageID& msgId,
	const QueryResultCallBack& cb);
void RemoveUndeliveredMessage(cppdb::session&& ses, const ChatMessageID& msgId, const CallID& partName,
	const QueryResultCallBack& cb);
void GetUndeliveredMessagesByChatId(cppdb::session&& ses, const ChatID& chatId,
	const QueryResultCallBack& cb);
void GetAllUndeliveredMessages(cppdb::session&& ses,
	const QueryResultCallBack& cb);
void GetMessagesForGlobalContext(cppdb::session& ses, const ChatMessageID& magId,
	const QueryResultCallBack& cb);
void GetMessagesForPersonalContext(cppdb::session& ses, const ChatMessageID& msgId,
	const QueryResultCallBack& cb);
}
}