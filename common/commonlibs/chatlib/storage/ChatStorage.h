#pragma once
#include "chatlib/callbacks.h"
#include "chatlib/storage/helpers.h"
#include "chatlib/chatinfo/info_types.h"
#include "chatlib/chat_defs.h"
#include "std-generic/cpplib/Box.h"

#include <mutex>

// For not defining GetMessage -> GetMessage(A/W)
#include "std-generic/undef_windows.h"

namespace cppdb {
	class session;
}

namespace chat
{
	namespace msg {
		struct MsgIdsInBucket;
		using MsgIdsByBuckets = std::vector<MsgIdsInBucket>;
	}
class ChatStorage
{
public:
	template <class CppDBConnectionPtr /* = cppdb::ref_ptr<cppdb::backend::connection> */>
	explicit ChatStorage(string_view config, CppDBConnectionPtr db_holder);
	~ChatStorage();
	detail::SaveChatMessageResult
		SaveChatMessage(
			const msg::ChatMessagePtr &m,
			const CallID& currentUserId,
			bool isIncoming);
	void GetLastBucketInfo(
		const ChatID& chat_id,
		uint64_t& last_bucket,
		uint32_t& count_msg_in_bucket) const;
	std::vector<chain_item>
		GetLastMessages(
			const ChatID& chat_id,
			size_t len) const;
	msg::ChatMessagePtr
		GetMessage(const ChatMessageID& msg_id, const CallID& currentUserId) const;
	std::vector<MessageWithOrder>
		GetMessages(const ChatID& chatId, const CallID& currentUserId, const ChatMessageID& lastMsgId, size_t count) const;
	std::vector<msg::ChatMessagePtr>
		GetMessagesToRetransmit(const ChatID& msgId,
			const msg::MsgIdsByBuckets& msgIdsForSync) const;
	std::vector<ChatMessageID>
		GetMessagesInBucket(const ChatID& chat_id, uint64_t bucket_num) const;
	// FIXME: Try to get rid of this
	bool IsMessageExist(
		const ChatMessageID& msg_id) const;
	bool RemoveUndeliveredMessage(
		const ChatMessageID& msg_id,
		const CallID& part);
	std::vector<ChatMessageID>
		GetUndeliveredMessagesByChatId(const ChatID& chat_id) const;
	std::vector<cb::UndeliveredMsg>
		GetAllUndeliveredMessages() const;
	GlobalContext
		GetGlobalContext(const ChatID& chat_id) const;
	uint64_t CountMessagesInChat(const ChatID& chat_id) const;
	PersonalContextList
		GetUserPersonalContexts(const CallID& callId, uint32_t pageSize, uint32_t pageNum) const;
	void SavePersonalContext(const PersonalContext& ctx) const;
	PersonalContext
		GetPersonalContext(const ChatID& chat_id, const CallID& owner) const;
	void SaveGlobalContext(const GlobalContext &ctx_stamp) const;

	PersonalContextList GetAllUserPersonalContexts(const CallID& call_id)
	{
		return GetUserPersonalContexts(call_id, std::numeric_limits<uint32_t>::max(), 1);
	}

private:
	static std::vector<ParticipantDescr>
		AddUndeliveredMessage(cppdb::session &ses, const ChatMessageID& msg_id);

	class session_wrapper;

	session_wrapper OpenSession() const;
	struct cppdb_pool_ptr_tag;
	mutable vs::Box<cppdb_pool_ptr_tag, sizeof(void*)> m_pool;
	std::recursive_mutex &m_mutex;
	static std::map<std::string /*config*/, std::recursive_mutex> mutexes;
	struct cppdb_connection_ptr_tag;
	vs::Box<cppdb_connection_ptr_tag, sizeof(void*)> m_db_holder;
};
}