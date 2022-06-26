#include "ChatStorage.h"

#include "chatlib/msg/attr.h"
#include "chatlib/msg/chat_messages_construct.h"
#include "chatlib/storage/DBProcWrap.h"
#include "chatlib/storage/helpers.h"
#include "chatlib/msg/ChatMessage.h"
#include "chatlib/utils/chat_utils.h"
#include "chatlib/utils/msg_utils.h"

#include "chatlib/log/chatlog.h"

#include "std-generic/compat/memory.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <limits>
#include <sstream>

namespace rj = rapidjson;
namespace chat
{
namespace
{

bool
guess_bool(string_view s)
{
	return s.length() != 0 &&
		(s[0] == '1' || s[0] == 't' || s[0] == 'T');
}

// out argument
const std::string c_result = "result";

// out arguments
const std::string c_error = "error";
const std::string c_chat_id = "chat_id";
const std::string c_message_id = "message_id";
const std::string c_order_in_chain_integral = "int_order";
const std::string c_order_in_chain_fractional = "text_order";
const std::string c_time_stamp = "time_stamp";
const std::string c_bucket_number = "bucket_number";
const std::string c_type = "type";
const std::string c_name = "name";
const std::string c_sender = "sender";
const std::string c_sender_type = "sender_type";
const std::string c_parent_message_id = "parent_message_id";
const std::string c_original_message_id = "original_message_id";
const std::string c_content = "content";
const std::string c_container = "container";

// out argument
const std::string c_count = "cnt";

/*
out args
*/
const std::string c_chat_title = "chat_title";
const std::string c_chat_type = "chat_type";
const std::string c_chat_version = "chat_version";
const std::string c_chat_creator = "chat_creator";
const std::string c_chat_created_at = "chat_created_at";
const std::string c_context_created_at = "context_created_at";
const std::string c_participants = "participants";
const std::string c_ban_list = "ban_list";
const std::string c_participant_name = "participant_name";
const std::string c_participant_type = "participant_type";

/*
out args
*/
const std::string c_messages_cnt = "messages_cnt";
/*
out args
*/
const std::string c_is_deleted = "is_deleted";
const std::string c_get_notifications = "get_notifications";
const std::string c_unread_messages = "unread_messages";
const std::string c_draft = "draft";
const std::string c_last_message_type = "last_message_type";
const std::string c_last_message_sender = "last_message_sender";
const std::string c_last_message_sender_type = "last_message_sender_type";
const std::string c_last_message_db_timestamp = "last_message_db_timestamp";
const std::string c_last_message_content = "last_message_content";

// out args
const std::string c_owner = "owner";

std::vector<chat::ChatMessageID>
DecodeUnreadMessages(const std::string& /*json*/)
{
	// FIXME: Implement
	return {};
}
std::string
DecodeDraft(const std::string& /*json*/)
{
	// FIXME: Implement
	return {};
}
MessageWithOrder
GetMsgFromQueryResult(cppdb::result& queryRes, ChatID chatIdFilled, ChatMessageID msgIdFilled)
{
	msg::ChatMessagePtr msg = vs::make_unique<msg::ChatMessage>();
	auto chatId = chatIdFilled.empty()
		? queryRes.get<std::string>(c_chat_id, {})
		: std::move(chatIdFilled);
	if (chatId.empty())
		return {};
	auto msgId = msgIdFilled.empty()
		? queryRes.get<std::string>(c_message_id, {})
		: std::move(msgIdFilled);
	auto stamp = queryRes.get<uint64_t>(c_time_stamp, std::numeric_limits<uint64_t>::max());
	if (stamp == std::numeric_limits<uint64_t>::max())
		return {};
	auto timestamp = uint_to_timestamp(stamp);
	auto bucket = queryRes.get<uint64_t>(c_bucket_number, std::numeric_limits<uint64_t>::max());
	if (bucket == std::numeric_limits<uint64_t>::max())
		return {};
	auto msgType = queryRes.get<uint32_t>(c_type, std::numeric_limits<uint32_t>::max());
	if (msgType == std::numeric_limits<uint32_t>::max())
		return {};
	auto from = queryRes.get<std::string>(c_sender, {});
	if (from.empty())
		return {};
	auto senderType = queryRes.get<uint32_t>(c_sender_type, std::numeric_limits<uint32_t>::max());
	if (senderType == std::numeric_limits<uint32_t>::max())
		return {};
	auto&& orderInChainIntegral = queryRes.get<int64_t>(c_order_in_chain_integral);
	auto&& orderInChainFractional = queryRes.get<std::string>(c_order_in_chain_fractional);
	auto prevMsgId = queryRes.get<std::string>(c_parent_message_id, {});
	auto originMsgId = queryRes.get<std::string>(c_original_message_id, {});
	auto content = queryRes.get<std::string>(c_content, {});
	std::ostringstream binData;
	bool hasBlob = queryRes.fetch(c_container, binData);

	msg->SetParam(attr::CHAT_ID_paramName, chatId);
	msg->SetParam(attr::MESSAGE_ID_paramName, msgId);
	msg->SetParam(attr::MESSAGE_TYPE_paramName, msgType);
	msg->SetParam(attr::PREVIOUS_MESSAGE_ID_paramName, prevMsgId);
	msg->SetParam(attr::TIMESTAMP_paramName, timestamp);
	msg->SetParamI64(attr::BUCKET_paramName, bucket);
	msg->SetParam(attr::FROM_paramName, from);
	msg->SetParamI32(attr::SENDER_TYPE_paramName, senderType);
	if (hasBlob) {
		auto str = std::move(binData).str();
		msg->SetParam(attr::BLOB_paramName, str.data(), str.size());
	}
	if (!originMsgId.empty())
		msg->SetParam(attr::ORIGINAL_MESSAGE_ID_paramName, originMsgId);
	assert(!content.empty());
	msg->SetParam(attr::MESSAGE_CONTENT_paramName, content);
	return { std::move(msg), OrderInChain(orderInChainIntegral, orderInChainFractional) };
}
std::pair<int32_t, OrderInChain>
AddMessage(cppdb::session& ses, const msg::ChatMessagePtr& msg)
{
	auto msgId = msg->GetParamStr(attr::MESSAGE_ID_paramName);
	auto chatId = msg->GetParamStr(attr::CHAT_ID_paramName);
	ChatMessageTimestamp timestamp;
	msg->GetParam(attr::TIMESTAMP_paramName, timestamp);
	uint64_t bucket{0};
	msg->GetParamI64(attr::BUCKET_paramName, bucket);
	uint32_t msgType{0};
	msg->GetParamI32(attr::MESSAGE_TYPE_paramName, msgType);

	auto from = msg->GetParamStr(attr::FROM_paramName);
	int32_t senderType(std::numeric_limits<int32_t>::max());
	msg->GetParamI32(attr::SENDER_TYPE_paramName, senderType);
	assert(senderType != std::numeric_limits<int32_t>::max());
	auto parentMsgId = msg->GetParamStr(attr::PREVIOUS_MESSAGE_ID_paramName);
	auto originMsgId = msg->GetParamStr(attr::ORIGINAL_MESSAGE_ID_paramName);
	size_t blob_len;
	const void *blob_ptr = msg->GetParamBinRef(attr::BLOB_paramName, blob_len);
	auto content = msg->GetParamStr(attr::MESSAGE_CONTENT_paramName);
	assert(!content.empty());
	std::pair<int32_t, OrderInChain> ret;
	db_proc::AddMessage(ses, msgId, chatId, timestamp, bucket,
		msgType, from, senderType, parentMsgId, originMsgId, content, blob_ptr, blob_len, [&](cppdb::result&& res)
	{
		if (!res.next())
			return;
		ret = std::make_pair(
			res.get<int32_t>(c_error),
			OrderInChain(
				res.get<int64_t>(c_order_in_chain_integral),
				res.get<std::string>(c_order_in_chain_fractional)
			)
		);
	});
	return ret;
}
int AddFirstVisibleMessage(cppdb::session& ses, const ChatMessageID& msgId, const CallID& currentUserId)
{
	int32_t res{0};
	db_proc::AddFirstVisibleMessage(ses, msgId, currentUserId, [&](cppdb::result&& query_res)
	{
		if (!query_res.next())
		{
			res = -1;
			return;
		}
		res = query_res.get<int32_t>(0);
	});
	return res;
}
detail::ChainUpdateEvent
UpdateMessageChain(cppdb::session& ses, const ChatMessageID& msgId)
{
	detail::ChainUpdateEvent ret;
	db_proc::UpdateMessageChain(ses, msgId, [&](cppdb::result&& res)
	{
		cb::MsgIdAndOrderInChain idAndOrder;
		while (res.next())
		{
			auto msgId = res.get<std::string>(c_message_id);
			OrderInChain order = {
				res.get<int64_t>(c_order_in_chain_integral),
				res.get<std::string>(c_order_in_chain_fractional)
			};
			if (!msgId.empty())
			{
				// Database must return messages list ordered by order in chain
				assert(idAndOrder.empty() || (order > idAndOrder.back().second));
				idAndOrder.emplace_back(std::move(msgId), order);
			}
		}
		if (idAndOrder.empty())
			return;
		ret = { true, std::move(idAndOrder) };
	});
	return ret;
}
struct CreateGlobalContextParam
{
	VS_FORWARDING_CTOR2(CreateGlobalContextParam, msg, content) {}
	msg::ChatMessagePtr msg;
	std::string content;
};
std::vector<CreateGlobalContextParam>
PrepareCreateGlobalContextParams(cppdb::session& ses, const ChatMessageID& msgId)
{
	std::vector<CreateGlobalContextParam> paramList;
	db_proc::GetMessagesForGlobalContext(ses, msgId, [&](cppdb::result&& res)
	{
		while (res.next())
		{
			auto msg = GetMsgFromQueryResult(res, {}, {});
			auto content = res.get<std::string>(c_content);
			if (!msg.first || content.empty())
			{
				assert(false);
				continue;
			}
			paramList.emplace_back(std::move(msg.first), std::move(content));
		}
	});
	return paramList;
}
struct CreatePersonalContextParam
{
	VS_FORWARDING_CTOR5(CreatePersonalContextParam,
		msgId, owner, chatId, type, content){}
	ChatMessageID msgId;
	CallID owner;
	ChatID chatId;
	MessageType type;
	std::string content;
};
std::vector<CreatePersonalContextParam>
PrepareCreatePersonalContextParams(cppdb::session& ses, const ChatMessageID& msgId)
{
	std::vector<CreatePersonalContextParam> paramList;
	db_proc::GetMessagesForPersonalContext(ses, msgId, [&](cppdb::result&& res)
	{
		while (res.next())
		{
			auto messageId = res.get<std::string>(c_message_id);
			auto type = res.get<int32_t>(c_type, 0);
			auto content = res.get<std::string>(c_content);
			auto chatId = res.get<std::string>(c_chat_id);
			auto owner = res.get<std::string>(c_owner);
			if (messageId.empty()
				|| type == 0
				|| content.empty()
				|| owner.empty()
				|| chatId.empty())
			{
				assert(false);
				continue;
			}
			paramList.emplace_back(std::move(messageId), std::move(owner), std::move(chatId),
				static_cast<MessageType>(type), std::move(content));
		}
	});
	return paramList;
}
}

class ChatStorage::session_wrapper : public cppdb::session
{
	std::unique_lock<std::recursive_mutex> lock;
public:
	session_wrapper(const cppdb::session& ses, std::unique_lock<std::recursive_mutex>&& lock)
		: cppdb::session(ses), lock(std::move(lock))
	{}
};

struct ChatStorage::cppdb_pool_ptr_tag : vs::BoxTag<cppdb::pool::pointer> {};
struct ChatStorage::cppdb_connection_ptr_tag : vs::BoxTag<cppdb::ref_ptr<cppdb::backend::connection>> {};

std::map<std::string /*config*/, std::recursive_mutex> ChatStorage::mutexes = {};

template <>
ChatStorage::ChatStorage(string_view config, cppdb::ref_ptr<cppdb::backend::connection> db_holder)
	: m_mutex(mutexes[std::string(config)])
	, m_db_holder(db_holder)
{
	m_pool = cppdb::pool::create(std::string(config));
}
ChatStorage::~ChatStorage() = default;
chat::ChatStorage::session_wrapper ChatStorage::OpenSession() const
{
	if (!m_pool.get())
		return session_wrapper({}, {});
	auto ses = cppdb::session(m_pool.get()->open());
	if (!chat::IsSQLite3(ses)) {
		return session_wrapper(ses, {});
	}
	else {
		// All of our SQLite operations are implemented with
		// write transactions (that is required for arguments and
		// result tables) which leads to exceptions due to transaction
		// rollbacks during their execution. So here we serialize them all.
		std::unique_lock<std::recursive_mutex> lock(m_mutex);
		return session_wrapper(ses, std::move(lock));
	}
}
detail::SaveChatMessageResult
ChatStorage::SaveChatMessage(const msg::ChatMessagePtr &msg, const CallID& currentUserId, bool isIncoming) try
{
	if (!msg || !m_pool.get())
		return {};
	auto ses = OpenSession();
	cppdb::transaction guard(ses);
	auto addRes = AddMessage(ses, msg);
	if (addRes.first != 0)
	{
		return detail::SaveChatMessageResult(
			static_cast<detail::SaveMessageError>(addRes.first));
	}
	auto msgId = msg->GetParamStr(attr::MESSAGE_ID_paramName);
	auto chainUpdateEv = UpdateMessageChain(ses, msgId);
	if (!chainUpdateEv.filled)
	{
		chainUpdateEv = { true,
			cb::MsgIdAndOrderInChain{{msg->GetParamStr(attr::MESSAGE_ID_paramName), addRes.second}} };
	}
	else
		chainUpdateEv.msgIdAndOrder.emplace(
			std::lower_bound(
				chainUpdateEv.msgIdAndOrder.begin(), chainUpdateEv.msgIdAndOrder.end(),
				addRes.second, [](const auto& a, const OrderInChain& b) { return a.second < b; }
			),
			msg->GetParamStr(attr::MESSAGE_ID_paramName), addRes.second);
	uint32_t msgType{0};
	msg->GetParamI32(attr::MESSAGE_TYPE_paramName, msgType);
	detail::GlobalContextUpdateEvent globCtxUpdEv;
	detail::PersonalContextUpdateEvent personalCtxUpdEv;
	auto add_undelivered = [&] {
		if (isIncoming) {
			std::vector<ParticipantDescr> parts;
			db_proc::GetParticipants(ses, msg->GetParamStr(attr::CHAT_ID_paramName),
				[&](cppdb::result&& getPartsRes) {
					while (getPartsRes.next()) {
						parts.emplace_back(
							getPartsRes.get<std::string>(c_name),
							static_cast<ParticipantType>(getPartsRes.get<int32_t>(c_type)));
					}
				});
			return parts;
		}
		return this->AddUndeliveredMessage(ses, msgId);
	};
	auto deliveryPartList = add_undelivered();
	if (msgType >= 400 && msgType < 500)
	{
		auto createCtxParams = PrepareCreatePersonalContextParams(ses,
			chainUpdateEv.msgIdAndOrder.front().first);
		if(!createCtxParams.empty())
		{
			CallID owner;
			std::set<ChatID> chats;
			for (const auto& param : createCtxParams)
			{
				db_proc::CreatePersonalContext(ses, param.msgId, param.type, param.content);
				chats.emplace(param.chatId);
				if (owner.empty())
					owner = param.owner;
			}
			personalCtxUpdEv = {true, std::move(owner), std::move(chats)};
		}
	}
	else
	{
		auto createCtxParams = PrepareCreateGlobalContextParams(ses,
			chainUpdateEv.msgIdAndOrder.front().first);
		if(!createCtxParams.empty())
		{
			std::vector<msg::ChatMessagePtr> msgs;
			for (auto& param : createCtxParams)
			{
				auto id = param.msg->GetParamStr(attr::MESSAGE_ID_paramName);
				auto type = MessageType::undefined;
				param.msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type);
				db_proc::CreateGlobalContext(
					ses,
					id,
					param.msg->GetParamStr(attr::CHAT_ID_paramName),
					type,
					param.msg->GetParamStr(attr::FROM_paramName),
					param.content);
				if (id == msgId)
				{
					// Participant may have been added, add him to delivery list for this msg
					deliveryPartList = max(deliveryPartList, add_undelivered(),
						[](const auto &a, const auto &b) { return a.size() < b.size(); });
					msgs.emplace_back(
						vs::make_unique<msg::ChatMessage>(msg->GetContainer()));
				}
				else
				{
					msgs.push_back(std::move(param.msg));
				}
			}
			globCtxUpdEv = {true, std::move(msgs)};
		}
	}
	auto res = AddFirstVisibleMessage(ses, msg->GetParamStr(attr::MESSAGE_ID_paramName), currentUserId);
	assert(res != -1);
	guard.commit();
	return {detail::SaveMessageError::success, std::move(deliveryPartList),
		std::move(chainUpdateEv), std::move(globCtxUpdEv), std::move(personalCtxUpdEv)};
}
catch (cppdb::cppdb_error &e)
{
	CHAT_TRACE(log_stream << "ChatStorage: SaveChatMessage failed; msg_id = " <<
		msg->GetParamStrRef(attr::MESSAGE_ID_paramName) <<"; what: " << e.what());
	return {};
}
void ChatStorage::GetLastBucketInfo(const ChatID& chat_id,
	uint64_t& last_bucket, uint32_t& count_msg_in_bucket) const try
{
	db_proc::GetLastBucketNumber(OpenSession(), chat_id, [&](cppdb::result&& res)
	{
		if (!res.next())
			return;
		// We need last_bucket to get bucket num to insert into,
		// so let's default to zero instead of excepting and
		// relying on user's default (in case of NULL on empty chat)
		last_bucket = res.get<uint64_t>(c_bucket_number, 0);
		count_msg_in_bucket = res.get<uint32_t>(c_messages_cnt);
	});
}
catch (cppdb::cppdb_error &e)
{
	CHAT_TRACE(log_stream << "ChatStorage: SaveChatMessage failed: " << e.what());
}
std::vector<chain_item>
ChatStorage::GetLastMessages(const ChatID& chat_id, size_t len) const try
{
	if(chat_id.empty() || len == 0)
		return {};
	std::vector<chain_item> tail;
	db_proc::GetLastMessages(OpenSession(), chat_id, len, [&](cppdb::result&& res)
	{
		while (res.next())
		{
			auto message_id = res.get<std::string>(c_message_id, {});
			if (message_id.empty())
				continue;
			auto parent_msg_id = res.get<std::string>(c_parent_message_id, {});
			auto stamp = res.get<uint64_t>(c_time_stamp, std::numeric_limits<uint64_t>::max());
			if (stamp == std::numeric_limits<uint64_t>::max())
				continue;
			auto bucket = res.get<uint64_t>(c_bucket_number, std::numeric_limits<uint64_t>::max());
			if (bucket == std::numeric_limits<uint64_t>::max())
				continue;
			tail.emplace_back(std::move(message_id), std::move(parent_msg_id),
				uint_to_timestamp(stamp), bucket);
		}
	});
	return tail;
}
catch (cppdb::cppdb_error &e)
{
	CHAT_TRACE(log_stream << "ChatStorage: GetLastMessages failed: " << e.what());
	return {};
}
msg::ChatMessagePtr
ChatStorage::GetMessage(const ChatMessageID& msg_id, const CallID& currentUserId) const try
{
	if (msg_id.empty())
		return {};
	msg::ChatMessagePtr ret;
	db_proc::GetMessage(OpenSession(), msg_id, currentUserId, [&](cppdb::result&& res)
	{
		if (!res.next())
			return;
		ret = GetMsgFromQueryResult(res, {}, ChatMessageID(msg_id)).first;
	});
	return ret;
}
catch(cppdb::cppdb_error &e)
{
	CHAT_TRACE(log_stream << "ChatStorage: GetMessage failed: " << e.what());
	return {};
}
std::vector<MessageWithOrder>
ChatStorage::GetMessages(
	const ChatID& chatId, const CallID& currentUserId, const ChatMessageID& lastMsgId, size_t count) const try
{
	std::vector<MessageWithOrder> res;
	db_proc::GetMessages(OpenSession(), chatId, currentUserId, lastMsgId,
		count, [&](cppdb::result&& queryRes)
	{
		while (queryRes.next())
		{
			res.emplace_back(GetMsgFromQueryResult(queryRes, ChatID(chatId), {}));
		}
	});
	return res;
}
catch (cppdb::cppdb_error &e)
{
	CHAT_TRACE(log_stream << "ChatStorage: GetMessages failed: " << e.what());
	return {};
}
std::vector<msg::ChatMessagePtr>
ChatStorage::GetMessagesToRetransmit(const ChatID& chatId,
	const msg::MsgIdsByBuckets& msgIdsForSync) const try
{
	rj::StringBuffer s_buckets;
	rj::StringBuffer s_messages;
	rj::Writer<rj::StringBuffer> writer_buckets(s_buckets);
	rj::Writer<rj::StringBuffer> writer_messages(s_messages);
	writer_buckets.StartArray();
	writer_messages.StartArray();
	for (const auto& buckets : msgIdsForSync)
	{
		writer_buckets.Uint64(buckets.id);
		for (const auto& msgId : buckets.msgIds)
		{
			writer_messages.String(msgId.c_str());
		}
	}
	writer_messages.EndArray();
	writer_buckets.EndArray();
	std::vector<msg::ChatMessagePtr> msgs;
	db_proc::GetMessagesToRetransmit(OpenSession(), chatId, s_buckets.GetString(), s_messages.GetString(),
		[&](cppdb::result&& res)
		{
			while (res.next())
			{
				msgs.emplace_back(
					GetMsgFromQueryResult(res, ChatID(chatId), {}).first
				);
			}
		});
	return msgs;
}
catch (cppdb::cppdb_error& e)
{
	CHAT_TRACE(log_stream << "ChatStorage: GetMessagesToRetransmit failed: " << e.what());
	return {};
}
std::vector<ChatMessageID>
ChatStorage::GetMessagesInBucket(const ChatID& chat_id, uint64_t bucket_num) const try
{
	if (chat_id.empty())
		return {};
	std::vector<ChatMessageID> msgs;
	db_proc::GetMessagesByBucket(OpenSession(), chat_id, bucket_num, [&](cppdb::result&& res)
	{
		while (res.next())
		{
			msgs.emplace_back(res.get<std::string>(0));
		}
	});
	return msgs;
}
catch(cppdb::cppdb_error& e)
{
	CHAT_TRACE(log_stream << "ChatStorage: GetMessagesInBucket failed: " << e.what());
	return {};
}
bool ChatStorage::IsMessageExist(
	const ChatMessageID& msg_id) const try
{
	if(msg_id.empty() || !m_pool.get())
		return false;
	bool ret(false);
	db_proc::HasMessage(OpenSession(), msg_id, [&](cppdb::result&& res)
	{
		res.next();
		ret = guess_bool(res.get<std::string>(0));
	});
	return ret;
}
catch (cppdb::cppdb_error &e)
{
	CHAT_TRACE(log_stream << "ChatStorage: IsMessageExist failed: " << e.what());
	return false;
}
std::vector<ParticipantDescr>
ChatStorage::AddUndeliveredMessage(cppdb::session &ses, const ChatMessageID& msg_id) try
{
	if (msg_id.empty())
		return {};
	std::vector<ParticipantDescr> parts_list;
	db_proc::AddUndeliveredMessage(ses, msg_id, [&](cppdb::result&& res)
	{
		while (res.next())
		{
			auto name = res.get<std::string>(c_participant_name);
			auto tp = res.get<int32_t>(c_participant_type, -1);
			if (name.empty() || tp == 0)
				continue;
			parts_list.emplace_back(
				ParticipantDescr(std::move(name), static_cast<ParticipantType>(tp)));
		}
	});
	return parts_list;
}
catch (cppdb::cppdb_error& e)
{
	CHAT_TRACE(log_stream << "ChatStorage: AddUndeliveredMessage failed: " << e.what());
	return {};
}
bool ChatStorage::RemoveUndeliveredMessage(
	const ChatMessageID& msg_id, const CallID& part) try
{
	if (msg_id.empty() || !m_pool.get())
		return false;
	bool ret(false);
	db_proc::RemoveUndeliveredMessage(OpenSession(), msg_id, part, [&](cppdb::result&& res)
	{
		res.next();
		ret = res.get<int32_t>(0) == 1;
	});
	return ret;
}
catch(cppdb::cppdb_error& e)
{
	CHAT_TRACE(log_stream << "ChatStorage: RemoveUndeliveredMessage failed: " << e.what());
	return false;
}
std::vector<ChatMessageID>
ChatStorage::GetUndeliveredMessagesByChatId(const ChatID& chat_id) const try
{
	if (chat_id.empty() || !m_pool.get())
		return {};
	std::vector<ChatMessageID> msgs;
	db_proc::GetUndeliveredMessagesByChatId(OpenSession(), chat_id, [&](cppdb::result&& res)
	{
		while (res.next())
		{
			auto msg = res.get<std::string>(0);
			if (msg.empty())
				continue;
			msgs.emplace_back(std::move(msg));
		}
	});
	return msgs;
}
catch (cppdb::cppdb_error& e)
{
	CHAT_TRACE(log_stream << "ChatStorage: GetUndeliveredMessagesByChatId failed: " << e.what());
	return {};
}
std::vector<cb::UndeliveredMsg>
ChatStorage::GetAllUndeliveredMessages() const try
{
	if (!m_pool.get())
		return {};
	std::vector<cb::UndeliveredMsg> msgs;
	db_proc::GetAllUndeliveredMessages(OpenSession(), [&](cppdb::result&& res)
	{
		while (res.next())
		{
			auto msg = GetMsgFromQueryResult(res, {}, {});
			auto part_list_json = res.get<std::string>(c_participants, {});
			auto participants = msg::PartListFromJSON<std::vector<ParticipantDescr>>
				(part_list_json);
			if (!msg.first || participants.empty())
				continue;
			msgs.emplace_back(cb::UndeliveredMsg(
				std::move(msg.first), std::move(participants))
			);
		}
	});
	return msgs;
}
catch(cppdb::cppdb_error& e)
{
	CHAT_TRACE(log_stream << "ChatStorage: GetAllUndeliveredMessages failed: " << e.what());
	return {};
}
GlobalContext
ChatStorage::GetGlobalContext(const ChatID& chat_id) const try
{
	GlobalContext ctx;

	db_proc::GetGlobalContext(OpenSession(), chat_id, [&](cppdb::result&& res)
	{
		if (!res.next())
		{
			return;
		}
		auto chat_title = res.get<std::string>(c_chat_title, {});
		auto chat_type = res.get<uint32_t>(c_chat_type, std::numeric_limits<uint32_t>::max());
		if (chat_type == std::numeric_limits<uint32_t>::max())
			return;
		auto chat_version = res.get<std::string>(c_chat_version, {});
		auto chat_creator = res.get<std::string>(c_chat_creator, {});
		auto chat_created_at = res.get<uint64_t>(c_chat_created_at, -1);
		auto context_created_at = res.get<uint64_t>(c_context_created_at, -1);
		auto message_id = res.get<std::string>(c_message_id, {});
		auto partlist_json = res.get<std::string>(c_participants, {});
		auto ban_list_json = res.get<std::string>(c_ban_list, {});
		auto participants = msg::PartListFromJSON<vs::set<ParticipantDescr, vs::less<>>>
			(partlist_json);
		// FIXME: Not used until ban list will be supported
		auto ban_list = msg::PartListFromJSON<vs::set<ParticipantDescr, vs::less<>>>
			(ban_list_json);
		ctx = { chat_id, std::move(chat_title),
		  static_cast<ChatType>(chat_type), std::move(chat_version),
		  std::move(chat_creator), uint_to_timestamp(chat_created_at),
		  uint_to_timestamp(context_created_at), std::move(message_id),
		  std::move(participants), std::move(ban_list) };
	});
	return ctx;
}
catch (cppdb::cppdb_error& e)
{
	CHAT_TRACE(log_stream << "ChatStorage: GetChatContext failed: " << e.what());
	return {};
}
uint64_t ChatStorage::CountMessagesInChat(const ChatID& chat_id) const try
{
	uint64_t count(0);
	db_proc::CountChatMessages(OpenSession(), chat_id, [&](cppdb::result&& res)
	{
		if (!res.next())
			return;
		count = res.get<uint64_t>(c_count);
	});
	return count;
}
catch (cppdb::cppdb_error& e)
{
	CHAT_TRACE(log_stream << "ChatStorage: CountMessagesInChat failed: " << e.what());
	return {};
}
PersonalContextList
ChatStorage::GetUserPersonalContexts(const CallID& call_id, uint32_t pageSize, uint32_t pageNum) const try
{
	PersonalContextList res;
	db_proc::GetUserPersonalContexts(OpenSession(), call_id, pageSize, pageNum,
		[&](cppdb::result&& query_result) {
			while (query_result.next())
			{
				auto chat_id = query_result.get<std::string>(c_chat_id, {});
				auto title = query_result.get<std::string>(c_chat_title, {});
				auto chat_type = query_result.get<uint32_t>(c_chat_type, std::numeric_limits<uint32_t>::max());
				auto version = query_result.get<std::string>(c_chat_version, {});
				auto creator = query_result.get<std::string>(c_chat_creator, {});
				auto chat_created_at = query_result.get<uint64_t>(c_chat_created_at,
					std::numeric_limits<uint64_t>::max());
				auto context_created_at = query_result.get<uint64_t>(c_context_created_at,
					std::numeric_limits<uint64_t>::max());
				auto message_id = query_result.get<std::string>(c_message_id, {});
				auto is_deleted = guess_bool(query_result.get<std::string>(c_is_deleted, {}));
				auto get_notifications = guess_bool(query_result.get<std::string>(
					c_get_notifications, {}));
				auto unread_messages = DecodeUnreadMessages(
					query_result.get<std::string>(c_unread_messages, {}));
				auto draft = DecodeDraft(query_result.get<std::string>(c_draft, {}));
				auto last_message_type = query_result.get<uint32_t>(c_last_message_type,
					std::numeric_limits<uint32_t>::max());
				auto last_message_sender = query_result.get<std::string>(c_last_message_sender, {});
				auto last_message_sender_type = query_result.get<uint32_t>(c_last_message_sender_type,
					std::numeric_limits<uint32_t>::max());
				auto last_message_db_timestamp = query_result.get<uint64_t>(c_last_message_db_timestamp,
					std::numeric_limits<uint64_t>::max());
				auto last_message_content = query_result.get<std::string>(c_last_message_content, {});

				if (chat_id.empty()
					|| chat_type == std::numeric_limits<uint32_t>::max()
					|| version.empty()
					|| creator.empty()
					|| chat_created_at == std::numeric_limits<uint64_t>::max()
					|| context_created_at == std::numeric_limits<uint64_t>::max()
					|| message_id.empty())
				{
					continue;
				}
				res.emplace_back(
					call_id,
					std::move(chat_id),
					std::move(title),
					static_cast<ChatType>(chat_type),
					std::move(version),
					std::move(creator),
					uint_to_timestamp(chat_created_at),
					uint_to_timestamp(context_created_at),
					std::move(message_id),
					is_deleted,
					get_notifications,
					std::move(unread_messages),
					std::move(draft),
					LastVisibleMessage{
						static_cast<MessageType>(last_message_type),
						std::move(last_message_sender),
						static_cast<ParticipantType>(last_message_sender_type),
						uint_to_timestamp(last_message_db_timestamp),
						std::move(last_message_content)
					}
				);
		}
	});
	return res;
}
catch(cppdb::cppdb_error& e)
{
	CHAT_TRACE(log_stream << "ChatStorage: GetUserPersonalContexts failed: " << e.what());
	return {};
}
void ChatStorage::SavePersonalContext(const PersonalContext& ctx) const try
{
	if (!m_pool.get())
		return;
	db_proc::AddPersonalContext(OpenSession(), ctx);
}
catch (cppdb::cppdb_error& e)
{
	CHAT_TRACE(log_stream << "ChatStorage: SavePersonalContext failed: " << e.what());
}
PersonalContext
ChatStorage::GetPersonalContext(const ChatID& chat_id, const CallID& owner) const try
{
	PersonalContext ctx;
	db_proc::GetPersonalContext(OpenSession(), chat_id, owner, [&](cppdb::result&& query_result)
	{
		if (!query_result.next())
			return;
		auto title = query_result.get<std::string>(c_chat_title, {});
		auto chat_type = query_result.get<uint32_t>(c_chat_type, std::numeric_limits<uint32_t>::max());
		auto version = query_result.get<std::string>(c_chat_version, {});
		auto creator = query_result.get<std::string>(c_chat_creator, {});
		auto chat_created_at = query_result.get<uint64_t>(c_chat_created_at,
			std::numeric_limits<uint64_t>::max());
		auto context_created_at = query_result.get<uint64_t>(c_context_created_at,
			std::numeric_limits<uint64_t>::max());
		auto message_id = query_result.get<std::string>(c_message_id, {});
		auto is_deleted = guess_bool(query_result.get<std::string>(c_is_deleted, {}));
		auto get_notifications = guess_bool(query_result.get<std::string>(
			c_get_notifications, {}));
		auto unread_messages = DecodeUnreadMessages(
			query_result.get<std::string>(c_unread_messages, {}));
		auto draft = DecodeDraft(query_result.get<std::string>(c_draft, {}));
		auto last_message_type = query_result.get<uint32_t>(c_last_message_type,
			std::numeric_limits<uint32_t>::max());
		auto last_message_sender = query_result.get<std::string>(c_last_message_sender, {});
		auto last_message_sender_type = query_result.get<uint32_t>(c_last_message_sender_type,
			std::numeric_limits<uint32_t>::max());
		auto last_message_db_timestamp = query_result.get<uint64_t>(c_last_message_db_timestamp,
			std::numeric_limits<uint64_t>::max());
		auto last_message_content = query_result.get<std::string>(c_last_message_content, {});

		if (chat_id.empty()
			|| chat_type == std::numeric_limits<uint32_t>::max()
			|| version.empty()
			|| creator.empty()
			|| chat_created_at == std::numeric_limits<uint64_t>::max()
			|| context_created_at == std::numeric_limits<uint64_t>::max()
			|| message_id.empty())
		{
			return;
		}
		ctx = {
				owner,
				chat_id,
				std::move(title),
				static_cast<ChatType>(chat_type),
				std::move(version),
				std::move(creator),
				uint_to_timestamp(chat_created_at),
				uint_to_timestamp(context_created_at),
				std::move(message_id),
				is_deleted,
				get_notifications,
				std::move(unread_messages),
				std::move(draft),
				LastVisibleMessage{
					static_cast<MessageType>(last_message_type),
					std::move(last_message_sender),
					static_cast<ParticipantType>(last_message_sender_type),
					uint_to_timestamp(last_message_db_timestamp),
					std::move(last_message_content)
					}
		};
	});
	return ctx;
}
catch (cppdb::cppdb_error& e)
{
	CHAT_TRACE(log_stream << "ChatStorage: GetPersonalContext failed: " << e.what());
	return {};
}
void ChatStorage::SaveGlobalContext(
	const GlobalContext &ctx) const try
{
	db_proc::AddGlobalContext(OpenSession(), ctx, [&](cppdb::result&& query_res)
	{
		if (!query_res.next())
			return;
		auto res = query_res.get<uint32_t>(0);
		if (res != 0)
			CHAT_TRACE(log_stream << "ChatStorage::SaveChatContextStamp; DB return = " << res);
	});
}
catch (cppdb::cppdb_error& e)
{
	CHAT_TRACE(log_stream << "ChatStorage: SaveChatContextStamp failed: " << e.what());
}
}
