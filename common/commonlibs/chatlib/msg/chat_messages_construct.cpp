#include "chat_messages_construct.h"
#include "chatlib/msg/attr.h"
#include "chatlib/utils/chat_utils.h"
#include "chatlib/utils/msg_utils.h"

#include "std-generic/compat/memory.h"
#include "std-generic/cpplib/VS_Container.h"

#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <bitset>
#include <cassert>
#include <iomanip>
#include <sstream>

namespace rj = rapidjson;

namespace chat
{
namespace msg
{
// JSON key names

// Common keys
const std::string typeKeyName = "type";
const std::string versionKeyName = "version";
const std::string titleKeyName = "title";
const std::string chatTitleKeyName = "chat_title";
const std::string participantsKeyName = "participants";
const std::string nameKeyName = "name";
const std::string toKeyName = "to";
const std::string contentKeyName = "content";
const std::string codeKeyName = "code";
const std::string msgIdKeyName = "message_id";
const std::string chatIdKeyName = "chat_id";
const std::string ownerKeyName = "owner";
const std::string whoAddedKeyName = "who_added";
const std::string p2pPartKeyName = "p2p_part";
const std::string replyIdKeyName = "reply_id";
const std::string forwardTitleKeyName = "forward_title";
const std::string permissionsKeyName = "permissions";
const std::string fromInstanceKeyName = "from_instance";
// Global context keys
const std::string creatorKeyName = "creator";
const std::string banListKeyName = "banlist";
const std::string chatCreateTimestampKeyName = "chat_create_timestamp";
const std::string ctxCreateTimestampKeyName = "ctx_create_timestamp";
const std::string chatCreatedTimeKeyName = "chat_created_time";
const std::string ctxCreatedTimeKeyName = "ctx_created_time";
const std::string deletedKeyName = "deleted";
const std::string getNotificationKeyName = "get_notification";
const std::string unreadMsgsKeyName = "unread_msgs";
const std::string draftKeyName = "draft";
const std::string lastMessageType = "last_message_type";
const std::string lastMessageSender = "last_message_sender";
const std::string lastMessageSenderType = "last_message_sender_type";
const std::string lastMessageDBTimestamp = "last_message_db_timestamp";
const std::string lastMessageContent = "last_message_content";
} // namespace msg
} // namespace chat

namespace
{
void SetCommonFields(
	chat::msg::ChatMessagePtr &message,
	chat::MessageType msgType,
	chat::ChatIDRef chatId,
	chat::CallIDRef from,
	vs::CallIDType senderType,
	chat::ChatMessageTimestamp timestamp,
	const chat::cb::OnChainUpdateByMsgCb &cb)
{
	message->SetParamI32(chat::attr::MESSAGE_TYPE_paramName, msgType);
	message->SetParam(chat::attr::CHAT_ID_paramName, chatId);
	message->SetParam(chat::attr::FROM_paramName, from);
	message->SetParamI32(chat::attr::SENDER_TYPE_paramName, senderType);
	if (timestamp != chat::ChatMessageTimestamp())
		message->SetParam(chat::attr::TIMESTAMP_paramName, timestamp);
	else
		message->SetParam(
			chat::attr::TIMESTAMP_paramName,
			std::chrono::system_clock::now());
	chat::ChatMessageID msgId = chat::GenerateUUID();
	message->SetParam(chat::attr::MESSAGE_ID_paramName, msgId);
	message->AddOnChainUpdateByMsgCallBack(cb);
}
void PackGlobalContext(const chat::GlobalContext& info,
	chat::msg::ChatMessagePtr &msg)
{
	rj::StringBuffer s;
	rj::Writer<rj::StringBuffer> writer(s);
	writer.StartObject();

	writer.Key(chat::msg::chatIdKeyName.c_str(),
		static_cast<rj::SizeType>(chat::msg::chatIdKeyName.length()));
	writer.String(info.chatId.c_str(),
		static_cast<rj::SizeType>(info.chatId.length()));

	writer.Key(chat::msg::titleKeyName.c_str(),
		static_cast<rj::SizeType>(chat::msg::titleKeyName.length()));
	writer.String(info.title.c_str(),
		static_cast<rj::SizeType>(info.title.length()));

	writer.Key(chat::msg::typeKeyName.c_str(),
		static_cast<rj::SizeType>(chat::msg::typeKeyName.length()));
	writer.Uint(static_cast<uint32_t>(info.type));

	writer.Key(chat::msg::versionKeyName.c_str(),
		static_cast<rj::SizeType>(chat::msg::versionKeyName.length()));
	writer.String(info.version.c_str(),
		static_cast<rj::SizeType>(info.version.length()));

	writer.Key(chat::msg::creatorKeyName.c_str(),
		static_cast<rj::SizeType>(chat::msg::creatorKeyName.length()));
	writer.String(info.creator.c_str(),
		static_cast<rj::SizeType>(info.creator.length()));

	writer.Key(chat::msg::chatCreateTimestampKeyName.c_str(),
		static_cast<rj::SizeType>(chat::msg::chatCreateTimestampKeyName.length()));
	writer.Uint64(chat::timestamp_to_uint(info.createTimestamp));

	writer.Key(chat::msg::ctxCreateTimestampKeyName.c_str(),
		static_cast<rj::SizeType>(chat::msg::ctxCreateTimestampKeyName.length()));
	writer.Uint64(chat::timestamp_to_uint(info.ctxCreateTimestamp));

	writer.Key(chat::msg::msgIdKeyName.c_str(),
		static_cast<rj::SizeType>(chat::msg::msgIdKeyName.length()));
	writer.String(info.msgId.c_str(),
		static_cast<rj::SizeType>(info.msgId.length()));

	writer.Key(chat::msg::participantsKeyName.c_str(),
		static_cast<rj::SizeType>(chat::msg::participantsKeyName.length()));
	writer.StartArray();
	for (const auto& i : info.participants)
	{
		writer.StartObject();
		writer.Key(chat::msg::nameKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::nameKeyName.length()));
		writer.String(i.partId.c_str(),
			static_cast<rj::SizeType>(i.partId.length()));
		writer.Key(chat::msg::typeKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::typeKeyName.length()));
		writer.Uint(static_cast<uint32_t>(i.tp));
		writer.EndObject();
	}
	writer.EndArray();

	writer.Key(chat::msg::banListKeyName.c_str(),
		static_cast<rj::SizeType>(chat::msg::banListKeyName.length()));
	writer.StartArray();
	for (const auto& i : info.banList)
	{
		writer.StartObject();
		writer.Key(chat::msg::nameKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::nameKeyName.length()));
		writer.String(i.partId.c_str(),
			static_cast<rj::SizeType>(i.partId.length()));
		writer.Key(chat::msg::typeKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::typeKeyName.length()));
		writer.Uint(static_cast<uint32_t>(i.tp));
		writer.EndObject();
	}
	writer.EndArray();

	writer.EndObject();
	msg->SetParam(chat::attr::GLOBAL_CONTEXT_paramName,
		string_view(s.GetString(), s.GetLength()));
}
chat::GlobalContext UnpackGlobalContext(const chat::msg::ChatMessagePtr &msg)
{
	auto content = msg->GetParamStrRef(chat::attr::GLOBAL_CONTEXT_paramName);
	if (content.empty())
		return {};
	rj::Document doc;
	if (doc.Parse(content.c_str()).HasParseError() || !doc.IsObject()) {
		return {};
	}
	chat::ChatID chatId;
	std::string title;
	chat::ChatType type(chat::ChatType::undef);
	chat::Version version;
	chat::CallID creator;
	chat::ChatMessageTimestamp chatCreateTimestamp;
	chat::ChatMessageTimestamp ctxCreateTimestamp;
	chat::ChatMessageID msgId;
	vs::set<chat::ParticipantDescr, vs::less<>> participants;
	vs::set<chat::ParticipantDescr, vs::less<>> banList;

	for (const auto& el : doc.GetObject())
	{
		auto keyName = string_view(el.name.GetString(), el.name.GetStringLength());
		if(keyName == chat::msg::chatIdKeyName)
		{
			chatId = chat::ChatID(el.value.GetString(), el.value.GetStringLength());
		}
		else if(keyName == chat::msg::titleKeyName)
		{
			title = std::string(el.value.GetString(), el.value.GetStringLength());
		}
		else if(keyName == chat::msg::typeKeyName)
		{
			type = static_cast<chat::ChatType>(el.value.GetUint());
		}
		else if (keyName == chat::msg::versionKeyName)
		{
			version = chat::Version(el.value.GetString(), el.value.GetStringLength());
		}
		else if(keyName == chat::msg::creatorKeyName)
		{
			creator = chat::CallID(el.value.GetString(), el.value.GetStringLength());
		}
		else if(keyName == chat::msg::chatCreateTimestampKeyName)
		{
			chatCreateTimestamp = chat::uint_to_timestamp(el.value.GetUint64());
		}
		else if(keyName == chat::msg::ctxCreateTimestampKeyName)
		{
			ctxCreateTimestamp = chat::uint_to_timestamp(el.value.GetUint64());
		}
		else if(keyName == chat::msg::msgIdKeyName)
		{
			msgId = chat::ChatMessageID(el.value.GetString(), el.value.GetStringLength());
		}
		else if(keyName == chat::msg::participantsKeyName)
		{
			if (!el.value.IsArray())
				return {};
			for (const auto& arr : el.value.GetArray())
			{
				auto obj = arr.GetObject();
				auto name = obj.FindMember(chat::msg::nameKeyName.c_str());
				if (name == obj.MemberEnd())
					return {};
				auto tp = obj.FindMember(chat::msg::typeKeyName.c_str());
				if (tp == obj.MemberEnd())
					return {};

				participants.emplace(
					chat::ParticipantDescr(
						chat::CallID(name->value.GetString(), name->value.GetStringLength()),
						static_cast<chat::ParticipantType>(tp->value.GetUint())));
			}
		}
		else if(keyName == chat::msg::banListKeyName)
		{
			if (!el.value.IsArray())
				return {};
			for (const auto& arr : el.value.GetArray())
			{
				auto obj = arr.GetObject();
				auto name = obj.FindMember(chat::msg::nameKeyName.c_str());
				if (name == obj.MemberEnd())
					return {};
				auto tp = obj.FindMember(chat::msg::typeKeyName.c_str());
				if (tp == obj.MemberEnd())
					return {};

				banList.emplace(
					chat::ParticipantDescr(
						chat::CallID(name->value.GetString(), name->value.GetStringLength()),
						static_cast<chat::ParticipantType>(tp->value.GetUint())));
			}
		}
	}
	return {
		std::move(chatId), std::move(title),
		type, std::move(version), std::move(creator),
		chatCreateTimestamp, ctxCreateTimestamp,
		std::move(msgId), std::move(participants),
		std::move(banList)
	};
}
void PackPersonalContexts(const chat::PersonalContextList& ctxs,
	chat::msg::ChatMessagePtr& msg)
{
	rj::StringBuffer s;
	rj::Writer<rj::StringBuffer> writer(s);
	writer.StartArray();
	for (const auto& ctx : ctxs)
	{
		writer.StartObject();
		writer.Key(chat::msg::ownerKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::ownerKeyName.length()));
		writer.String(ctx.owner.c_str(),
			static_cast<rj::SizeType>(ctx.owner.length()));
		writer.Key(chat::msg::chatIdKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::chatIdKeyName.length()));
		writer.String(ctx.chatId.c_str(),
			static_cast<rj::SizeType>(ctx.chatId.length()));
		writer.Key(chat::msg::titleKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::titleKeyName.length()));
		writer.String(ctx.title.c_str(),
			static_cast<rj::SizeType>(ctx.title.length()));
		writer.Key(chat::msg::typeKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::typeKeyName.length()));
		writer.Uint(static_cast<uint32_t>(ctx.chatType));
		writer.Key(chat::msg::versionKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::versionKeyName.length()));
		writer.String(ctx.version.c_str(),
			static_cast<rj::SizeType>(ctx.version.length()));
		writer.Key(chat::msg::creatorKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::creatorKeyName.length()));
		writer.String(ctx.creator.c_str(),
			static_cast<rj::SizeType>(ctx.creator.length()));
		writer.Key(chat::msg::chatCreatedTimeKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::chatCreatedTimeKeyName.length()));
		writer.Uint64(chat::timestamp_to_uint(ctx.chatCreatedTime));
		writer.Key(chat::msg::ctxCreatedTimeKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::ctxCreatedTimeKeyName.length()));
		writer.Uint64(chat::timestamp_to_uint(ctx.ctxCreatedTime));
		writer.Key(chat::msg::msgIdKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::msgIdKeyName.length()));
		writer.String(ctx.msgId.c_str(),
			static_cast<rj::SizeType>(ctx.msgId.length()));
		writer.Key(chat::msg::deletedKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::deletedKeyName.length()));
		writer.Bool(ctx.isDeleted);
		writer.Key(chat::msg::getNotificationKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::getNotificationKeyName.length()));
		writer.Bool(ctx.getNotification);
		writer.Key(chat::msg::unreadMsgsKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::unreadMsgsKeyName.length()));
		writer.StartArray();
		for (const auto& id : ctx.unreadMsgs)
		{
			writer.String(id.c_str(),
				static_cast<rj::SizeType>(id.length()));
		}
		writer.EndArray();
		writer.Key(chat::msg::draftKeyName.c_str(),
			static_cast<rj::SizeType>(chat::msg::draftKeyName.length()));
		writer.String(ctx.draft.c_str(),
			static_cast<rj::SizeType>(ctx.draft.length()));
		writer.Key(chat::msg::lastMessageType.c_str(),
			static_cast<rj::SizeType>(chat::msg::lastMessageType.length()));
		writer.Uint(static_cast<uint32_t>(ctx.lastVisibleMessage.msgType));
		writer.Key(chat::msg::lastMessageSender.c_str(),
			static_cast<rj::SizeType>(chat::msg::lastMessageSender.length()));
		writer.String(ctx.lastVisibleMessage.sender.c_str(),
			static_cast<rj::SizeType>(ctx.lastVisibleMessage.sender.length()));
		writer.Key(chat::msg::lastMessageSenderType.c_str(),
			static_cast<rj::SizeType>(chat::msg::lastMessageSenderType.length()));
		writer.Uint(static_cast<uint32_t>(ctx.lastVisibleMessage.senderType));
		writer.Key(chat::msg::lastMessageDBTimestamp.c_str(),
			static_cast<rj::SizeType>(chat::msg::lastMessageDBTimestamp.length()));
		writer.Uint64(chat::timestamp_to_uint(ctx.lastVisibleMessage.DBTimestamp));
		writer.Key(chat::msg::lastMessageContent.c_str(),
			static_cast<rj::SizeType>(chat::msg::lastMessageContent.length()));
		writer.String(ctx.lastVisibleMessage.content.c_str(),
			static_cast<rj::SizeType>(ctx.lastVisibleMessage.content.length()));
		writer.EndObject();
	}
	writer.EndArray();
	msg->SetParam(chat::attr::PERSONAL_CONTEXTS_paramName,
		string_view(s.GetString(), s.GetLength()));
}
chat::PersonalContextList
UnPackPersonalContexts(const chat::msg::ChatMessagePtr& msg)
{
	auto content = msg->GetParamStrRef(chat::attr::PERSONAL_CONTEXTS_paramName);
	if (content.empty())
		return {};
	rj::Document doc;
	doc.Parse(content.c_str());
	chat::PersonalContextList res;
	for(const auto& item : doc.GetArray())
	{
		auto ownerIt = item.FindMember(chat::msg::ownerKeyName.c_str());
		auto chatIdIt = item.FindMember(chat::msg::chatIdKeyName.c_str());
		auto titleIt = item.FindMember(chat::msg::titleKeyName.c_str());
		auto typeIt = item.FindMember(chat::msg::typeKeyName.c_str());
		auto versionIt = item.FindMember(chat::msg::versionKeyName.c_str());
		auto creatorIt = item.FindMember(chat::msg::creatorKeyName.c_str());
		auto chatCreatedTimeIt = item.FindMember(chat::msg::chatCreatedTimeKeyName.c_str());
		auto ctxCreatedTimeIt = item.FindMember(chat::msg::ctxCreatedTimeKeyName.c_str());
		auto msgIdIt = item.FindMember(chat::msg::msgIdKeyName.c_str());
		auto isDeletedIt = item.FindMember(chat::msg::deletedKeyName.c_str());
		auto getNotificationIt = item.FindMember(chat::msg::getNotificationKeyName.c_str());
		auto unreadMsgsIt = item.FindMember(chat::msg::unreadMsgsKeyName.c_str());
		auto draftIt= item.FindMember(chat::msg::draftKeyName.c_str());
		auto lastMessageTypeIt = item.FindMember(chat::msg::lastMessageType.c_str());
		auto lastMessageSenderIt = item.FindMember(chat::msg::lastMessageSender.c_str());
		auto lastMessageSenderTypeIt = item.FindMember(chat::msg::lastMessageSenderType.c_str());
		auto lastMessageDBTimestampIt = item.FindMember(chat::msg::lastMessageDBTimestamp.c_str());
		auto lastMessageContentIt = item.FindMember(chat::msg::lastMessageContent.c_str());
		if (!ownerIt->value.IsString()
			|| !chatIdIt->value.IsString()
			|| !titleIt->value.IsString()
			|| !typeIt->value.IsUint()
			|| !versionIt->value.IsString()
			|| !creatorIt->value.IsString()
			|| !chatCreatedTimeIt->value.IsUint64()
			|| !ctxCreatedTimeIt->value.IsUint64()
			|| !msgIdIt->value.IsString()
			|| !isDeletedIt->value.IsBool()
			|| !getNotificationIt->value.IsBool()
			|| !unreadMsgsIt->value.IsArray()
			|| !draftIt->value.IsString()
			|| !lastMessageTypeIt->value.IsUint()
			|| !lastMessageSenderIt->value.IsString()
			|| !lastMessageSenderTypeIt->value.IsUint()
			|| !lastMessageDBTimestampIt->value.IsUint64()
			|| !lastMessageContentIt->value.IsString())
		{
			return {};
		}
		std::vector<chat::ChatMessageID> unreadMsgs;
		for (const auto& id : unreadMsgsIt->value.GetArray())
		{
			if (!id.IsString())
				return {};
			unreadMsgs.emplace_back(chat::ChatMessageID(id.GetString(), id.GetStringLength()));
		}
		res.emplace_back(
			ownerIt->value.GetString(),
			chatIdIt->value.GetString(),
			titleIt->value.GetString(),
			static_cast<chat::ChatType>(typeIt->value.GetUint()),
			versionIt->value.GetString(),
			creatorIt->value.GetString(),
			chat::uint_to_timestamp(chatCreatedTimeIt->value.GetUint64()),
			chat::uint_to_timestamp(ctxCreatedTimeIt->value.GetUint64()),
			msgIdIt->value.GetString(),
			isDeletedIt->value.GetBool(),
			getNotificationIt->value.GetBool(),
			std::move(unreadMsgs),
			draftIt->value.GetString(),
			chat::LastVisibleMessage{
				static_cast<chat::MessageType>(lastMessageTypeIt->value.GetUint()),
				lastMessageSenderIt->value.GetString(),
				static_cast<chat::ParticipantType>(lastMessageSenderTypeIt->value.GetUint()),
				chat::uint_to_timestamp(lastMessageDBTimestampIt->value.GetUint64()),
				lastMessageContentIt->value.GetString()
			});
	}
	return res;
}
}
namespace chat
{
namespace msg
{
std::string PartListToJSON(const vs::set<ParticipantDescr, vs::less<>>& parts)
{
	rj::StringBuffer s;
	rj::Writer<rj::StringBuffer> writer(s);
	writer.StartArray();
	for (const auto& i : parts)
	{
		writer.StartObject();
		writer.Key(nameKeyName.c_str(), static_cast<rj::SizeType>(nameKeyName.length()));
		writer.String(i.partId.c_str(), static_cast<rj::SizeType>(i.partId.length()));
		writer.Key(typeKeyName.c_str(), static_cast<rj::SizeType>(typeKeyName.length()));
		writer.Uint(static_cast<uint32_t>(i.tp));
		writer.EndObject();
	}
	writer.EndArray();
	return { s.GetString(), s.GetLength() };
}
template<typename Res>
Res PartListFromJSONGeneric(const std::string& json)
{
	rj::Document doc;
	doc.Parse(json.c_str());
	if (!doc.IsArray())
		return {};
	Res parts;
	for (const auto& i : doc.GetArray())
	{
		if (!i.IsObject())
			return {};
		auto name_it = i.FindMember(nameKeyName.c_str());
		auto type_it = i.FindMember(typeKeyName.c_str());
		if (name_it == i.MemberEnd()
			|| type_it == i.MemberEnd()
			|| !name_it->value.IsString()
			|| !type_it->value.IsUint())
		{
			return {};
		}
		parts.insert(parts.end(), 
			ParticipantDescr(
				CallID(name_it->value.GetString(), name_it->value.GetStringLength()),
				static_cast<ParticipantType>(type_it->value.GetInt())));
	}
	return parts;
}
template<>
vs::set<ParticipantDescr, vs::less<>> PartListFromJSON<vs::set<ParticipantDescr, vs::less<>> >(const std::string& json)
{
	return PartListFromJSONGeneric<vs::set<ParticipantDescr, vs::less<>>>(json);
}
template<>
std::vector<ParticipantDescr> PartListFromJSON<std::vector<ParticipantDescr>>(const std::string& json)
{
	return PartListFromJSONGeneric<std::vector<ParticipantDescr>>(json);
}
bool CreateChatMessage::IsMyMessage(const ChatMessagePtr &m)
{
	auto type = MessageType::undefined;
	if (m->GetParamI32(attr::MESSAGE_TYPE_paramName, type))
	{
		return type == MessageType::create_chat;
	}
	return false;
}
chat::GlobalContext
CreateChatMessage::GetGlobalContext(const ChatMessagePtr& msg)
{
	auto parsed = Parse(msg);
	if (!parsed.success)
		return {};
	return {
		std::move(parsed.msg.chat_id),
		std::move(parsed.msg.content.title),
		parsed.msg.content.type,
		std::move(parsed.msg.content.ver),
		std::move(parsed.msg.creator),
		parsed.msg.timestamp,
		parsed.msg.timestamp,
		std::move(parsed.msg.msg_id),
		std::set<ParticipantDescr, vs::less<>>(),
		std::set<ParticipantDescr, vs::less<>>()
	};
}
detail::parse_msg_res<create_chat>
CreateChatMessage::Parse(const ChatMessagePtr &msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto contentParsRes = ParseMsgContent(msg);
	if (!contentParsRes.success)
		return {};
	auto chatId = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	auto msgId = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	auto author = msg->GetParamStrRef(attr::FROM_paramName);
	ChatMessageTimestamp timestamp;
	if (!msg->GetParam(attr::TIMESTAMP_paramName, timestamp)
		|| chatId.empty()
		|| msgId.empty()
		|| author.empty())
	{
		return {};
	}
	return {true, create_chat(chatId, msgId, timestamp, author, contentParsRes.content)};
}
detail::parse_msg_content_res<create_chat_content>
CreateChatMessage::ParseMsgContent(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto content = msg->GetParamStrRef(attr::MESSAGE_CONTENT_paramName);
	rj::Document doc;
	if (doc.Parse(content.c_str()).HasParseError() || !doc.IsObject())
		return {};
	auto verIt = doc.FindMember(versionKeyName.c_str());
	auto typeIt = doc.FindMember(typeKeyName.c_str());
	auto titleIt = doc.FindMember(titleKeyName.c_str());
	auto fromInstanceIt = doc.FindMember(fromInstanceKeyName.c_str());
	if (verIt == doc.MemberEnd()
		|| typeIt == doc.MemberEnd()
		|| !verIt->value.IsString()
		|| !typeIt->value.IsUint()
		|| (titleIt != doc.MemberEnd() && !titleIt->value.IsString())
		|| (fromInstanceIt != doc.MemberEnd() && !fromInstanceIt->value.IsString()))
	{
		return {};
	}
	std::string title;
	if (titleIt != doc.MemberEnd())
		title = std::string{titleIt->value.GetString(), titleIt->value.GetStringLength()};
	CallID fromInstance;
	if (fromInstanceIt != doc.MemberEnd())
		fromInstance = std::string{fromInstanceIt->value.GetString(), fromInstanceIt->value.GetStringLength()};
	return {true, create_chat_content(
		Version{verIt->value.GetString(), verIt->value.GetStringLength()},
		static_cast<ChatType>(typeIt->value.GetUint()),
		std::move(title),
		std::move(fromInstance)
	)};
}
CreateChatMessage::CreateChatMessage(std::unique_ptr<ChatMessage> &&message)
	: ChatMessageKeeper(std::move(message))
{
	assert(!message);
	assert(false); // check that message was moved;
}
CreateChatMessage::CreateChatMessage(
	ChatIDRef chatId,
	VersionRef ver,
	ChatType type,
	string_view title,
	CallIDRef creator,
	CallIDRef fromInstance,
	vs::CallIDType senderType,
	const cb::OnChainUpdateByMsgCb &cb)
{
	rj::StringBuffer s;
	rj::Writer<rj::StringBuffer> writer(s);
	writer.StartObject();
	writer.Key(typeKeyName.c_str(), static_cast<rj::SizeType>(typeKeyName.length()));
	writer.Uint(static_cast<uint32_t>(type));
	writer.Key(versionKeyName.c_str(), static_cast<rj::SizeType>(versionKeyName.length()));
	writer.String(ver.data(), static_cast<rj::SizeType>(ver.size()));
	if (!title.empty())
	{
		writer.Key(titleKeyName.c_str(), static_cast<rj::SizeType>(titleKeyName.length()));
		writer.String(title.data(), static_cast<rj::SizeType>(title.size()));
	}
	writer.Key(fromInstanceKeyName.c_str(), static_cast<rj::SizeType>(fromInstanceKeyName.length()));
	writer.String(fromInstance.data(), static_cast<rj::SizeType>(fromInstance.size()));
	writer.EndObject();
	m_message->SetParam(attr::MESSAGE_CONTENT_paramName,
		string_view{s.GetString(), s.GetLength()});
	SetCommonFields(m_message,
		MessageType::create_chat,
		chatId, creator, senderType,
		{}, cb);
}
bool CreateP2PChatMessage::IsMyMessage(const ChatMessagePtr& msg)
{
	auto type = MessageType::undefined;
	if (msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type))
	{
		return type == MessageType::create_p2p_chat;
	}
	return false;
}
chat::GlobalContext
CreateP2PChatMessage::GetGlobalContext(const ChatMessagePtr& msg)
{
	auto parsed = Parse(msg);
	if (!parsed.success)
		return {};
	return {
		std::move(parsed.msg.chat_id),
		std::string(),
		ChatType::p2p,
		std::move(parsed.msg.content.ver),
		CallID(),
		parsed.msg.timestamp,
		parsed.msg.timestamp,
		std::move(parsed.msg.msg_id),
		std::move(parsed.msg.content.part_list),
		std::set<ParticipantDescr, vs::less<>>()
	};
}
CreateP2PChatMessage::CreateP2PChatMessage(
	ChatIDRef chatId,
	VersionRef ver,
	const vs::set<ParticipantDescr, vs::less<>>& partList,
	CallIDRef from,
	CallIDRef fromInstance,
	vs::CallIDType senderType,
	const cb::OnChainUpdateByMsgCb& cb)
{
	rj::StringBuffer s;
	rj::Writer<rj::StringBuffer> writer(s);
	writer.StartObject();
	writer.Key(versionKeyName.c_str(), static_cast<rj::SizeType>(versionKeyName.length()));
	writer.String(ver.data(), static_cast<rj::SizeType>(ver.length()));
	writer.Key(participantsKeyName.c_str(), static_cast<rj::SizeType>(participantsKeyName.length()));
	writer.StartArray();
	for (const auto& part : partList)
	{
		writer.StartObject();
		writer.Key(nameKeyName.c_str(), static_cast<rj::SizeType>(nameKeyName.length()));
		writer.String(part.partId.c_str(), static_cast<rj::SizeType>(part.partId.length()));
		writer.Key(typeKeyName.c_str(), static_cast<rj::SizeType>(typeKeyName.length()));
		writer.Uint(static_cast<uint32_t>(part.tp));
		writer.EndObject();
	}
	writer.EndArray();
	writer.Key(fromInstanceKeyName.c_str(), static_cast<rj::SizeType>(fromInstanceKeyName.length()));
	writer.String(fromInstance.data(), static_cast<rj::SizeType>(fromInstance.size()));
	writer.EndObject();
	m_message->SetParam(attr::MESSAGE_CONTENT_paramName,
		string_view{s.GetString(), s.GetLength()});
	SetCommonFields(m_message,
		MessageType::create_p2p_chat,
		chatId, from, senderType,
		{}, cb);
}
detail::parse_msg_res<create_p2p_chat>
CreateP2PChatMessage::Parse(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto contentParsRes = ParseMsgContent(msg);
	if (!contentParsRes.success)
		return {};
	auto chatId = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	auto msgId = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	ChatMessageTimestamp timestamp;
	if (!msg->GetParam(attr::TIMESTAMP_paramName, timestamp)
		|| chatId.empty()
		|| msgId.empty())
	{
		return {};
	}
	return {true, create_p2p_chat(chatId, msgId, timestamp, std::move(contentParsRes.content))};
}
detail::parse_msg_content_res<create_p2p_chat_content>
CreateP2PChatMessage::ParseMsgContent(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	rj::Document doc;
	auto content = msg->GetParamStrRef(attr::MESSAGE_CONTENT_paramName);
	if (doc.Parse(content.c_str()).HasParseError() || !doc.IsObject())
		return {};
	auto verIt = doc.FindMember(versionKeyName.c_str());
	auto partsIt = doc.FindMember(participantsKeyName.c_str());
	auto fromInstanceIt = doc.FindMember(fromInstanceKeyName.c_str());
	if (verIt == doc.MemberEnd()
		|| partsIt == doc.MemberEnd()
		|| !verIt->value.IsString()
		|| !partsIt->value.IsArray()
		|| (fromInstanceIt != doc.MemberEnd() && !fromInstanceIt->value.IsString()))
		return {};
	auto parts_descr = vs::set<ParticipantDescr, vs::less<>>{};
	for (const auto& p : partsIt->value.GetArray())
	{
		if (!p.IsObject())
			return {};
		auto nameIt = p.FindMember(nameKeyName.c_str());
		auto typeIt = p.FindMember(typeKeyName.c_str());
		if (nameIt == p.MemberEnd()
			|| typeIt == p.MemberEnd()
			|| !nameIt->value.IsString()
			|| !typeIt->value.IsUint())
			return {};
		parts_descr.insert(
			{
				CallID{nameIt->value.GetString(), nameIt->value.GetStringLength()},
				static_cast<ParticipantType>(typeIt->value.GetUint())
			}
		);
	}
	CallID fromInstance;
	if (fromInstanceIt != doc.MemberEnd())
		fromInstance = std::string{fromInstanceIt->value.GetString(), fromInstanceIt->value.GetStringLength()};
	return {true, create_p2p_chat_content(
		Version{verIt->value.GetString(), verIt->value.GetStringLength()},
		std::move(parts_descr),
		std::move(fromInstance)
	)};
}
bool InviteMessage::IsMyMessage(const ChatMessagePtr &m)
{
	auto type = MessageType::undefined;
	if (m->GetParamI32(attr::MESSAGE_TYPE_paramName, type))
	{
		return type == MessageType::invite;
	}
	return false;
}
detail::parse_msg_res<invite>
InviteMessage::Parse(const ChatMessagePtr &msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto contentParsRes = ParseMsgContent(msg);
	if (!contentParsRes.success)
		return {};
	auto chatId = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	auto msgId = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	auto inviter = msg->GetParamStrRef(attr::FROM_paramName);
	ChatMessageTimestamp timestamp;
	if (!msg->GetParam(attr::TIMESTAMP_paramName, timestamp)
		|| chatId.empty()
		|| msgId.empty()
		|| inviter.empty())
	{
		return {};
	}
	return {true, invite(chatId, msgId, timestamp, inviter, contentParsRes.content)};
}
detail::parse_msg_content_res<invite_content>
InviteMessage::ParseMsgContent(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto content = msg->GetParamStrRef(attr::MESSAGE_CONTENT_paramName);
	rj::Document doc;
	if (doc.Parse(content.c_str()).HasParseError() || !doc.IsObject())
		return {};
	auto nameIt = doc.FindMember(nameKeyName.c_str());
	auto verIt = doc.FindMember(versionKeyName.c_str());
	auto titleIt = doc.FindMember(titleKeyName.c_str());
	auto typeIt = doc.FindMember(typeKeyName.c_str());
	auto fromInstanceIt = doc.FindMember(fromInstanceKeyName.c_str());
	if (nameIt == doc.MemberEnd()
		|| verIt == doc.MemberEnd()
		|| titleIt == doc.MemberEnd()
		|| typeIt == doc.MemberEnd()
		|| !nameIt->value.IsString()
		|| !verIt->value.IsString()
		|| !titleIt->value.IsString()
		|| !typeIt->value.IsUint()
		|| (fromInstanceIt != doc.MemberEnd() && !fromInstanceIt->value.IsString()))
	{
		return {};
	}
	CallID fromInstance;
	if (fromInstanceIt != doc.MemberEnd())
		fromInstance = std::string{fromInstanceIt->value.GetString(), fromInstanceIt->value.GetStringLength()};
	return {true, invite_content(
		Version(verIt->value.GetString(), verIt->value.GetStringLength()),
		static_cast<ChatType>(typeIt->value.GetUint()),
		std::string(titleIt->value.GetString(), titleIt->value.GetStringLength()),
		CallID(nameIt->value.GetString(), nameIt->value.GetStringLength()),
		std::move(fromInstance)
	)};
}
InviteMessage::InviteMessage(
	ChatIDRef chatId,
	ChatType type,
	string_view title,
	VersionRef ver,
	CallIDRef to,
	CallIDRef from,
	CallIDRef fromInstance,
	vs::CallIDType senderType,
	const cb::OnChainUpdateByMsgCb &onSave)
{
	rj::StringBuffer s;
	rj::Writer<rj::StringBuffer> writer(s);
	writer.StartObject();
	writer.Key(versionKeyName.c_str(), static_cast<rj::SizeType>(versionKeyName.length()));
	writer.String(ver.data(), static_cast<rj::SizeType>(ver.length()));
	writer.Key(typeKeyName.c_str(), static_cast<rj::SizeType>(typeKeyName.length()));
	writer.Uint(static_cast<uint32_t>(type));
	writer.Key(titleKeyName.c_str(), static_cast<rj::SizeType>(titleKeyName.length()));
	writer.String(title.data(), static_cast<rj::SizeType>(title.length()));
	writer.Key(nameKeyName.c_str(), static_cast<rj::SizeType>(nameKeyName.length()));
	writer.String(to.data(), static_cast<rj::SizeType>(to.length()));
	writer.Key(fromInstanceKeyName.c_str(), static_cast<rj::SizeType>(fromInstanceKeyName.length()));
	writer.String(fromInstance.data(), static_cast<rj::SizeType>(fromInstance.size()));
	writer.EndObject();
	m_message->SetParam(attr::MESSAGE_CONTENT_paramName,
		string_view{s.GetString(), s.GetLength()});
	SetCommonFields(m_message,
		MessageType::invite, chatId,
		from, senderType,
		{}, onSave);
}
bool InviteResponseMessage::IsMyMessage(const ChatMessagePtr& msg)
{
	auto type = MessageType::undefined;
	if (msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type))
	{
		return type == MessageType::invite_response;
	}
	return false;
}
detail::parse_msg_res<invite_response>
InviteResponseMessage::Parse(const ChatMessagePtr &msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto contentParsRes = ParseMsgContent(msg);
	if (!contentParsRes.success)
		return {};
	auto chatId = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	auto msgId = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	auto responseFrom = msg->GetParamStrRef(attr::FROM_paramName);
	ChatMessageTimestamp timestamp;
	if (!msg->GetParam(attr::TIMESTAMP_paramName, timestamp)
		|| chatId.empty()
		|| msgId.empty()
		|| responseFrom.empty())
	{
		return {};
	}
	return {true, invite_response(chatId, msgId, timestamp, responseFrom, contentParsRes.content)};
}
detail::parse_msg_content_res<invite_response_content>
InviteResponseMessage::ParseMsgContent(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto content = msg->GetParamStrRef(attr::MESSAGE_CONTENT_paramName);
	rj::Document doc;
	if (doc.Parse(content.c_str()).HasParseError() || !doc.IsObject())
		return {};
	auto codeIt = doc.FindMember(codeKeyName.c_str());
	auto msgIdIt = doc.FindMember(msgIdKeyName.c_str());
	auto callIdIt = doc.FindMember(nameKeyName.c_str());
	auto fromInstanceIt = doc.FindMember(fromInstanceKeyName.c_str());
	if (codeIt == doc.MemberEnd()
		|| msgIdIt == doc.MemberEnd()
		|| callIdIt == doc.MemberEnd()
		|| !codeIt->value.IsUint()
		|| !msgIdIt->value.IsString()
		|| !callIdIt->value.IsString()
		|| (fromInstanceIt != doc.MemberEnd() && !fromInstanceIt->value.IsString()))
	{
		return {};
	}
	CallID fromInstance;
	if (fromInstanceIt != doc.MemberEnd())
		fromInstance = std::string{fromInstanceIt->value.GetString(), fromInstanceIt->value.GetStringLength()};
	return {true, invite_response_content(
		static_cast<InviteResponseCode>(codeIt->value.GetUint()),
		ChatMessageID{msgIdIt->value.GetString(), msgIdIt->value.GetStringLength()},
		CallID{callIdIt->value.GetString(), callIdIt->value.GetStringLength()},
		std::move(fromInstance)
	)};
}
InviteResponseMessage::InviteResponseMessage(
	ChatIDRef chatId,
	CallIDRef to,
	CallIDRef from,
	CallIDRef fromInstance,
	vs::CallIDType senderType,
	ChatMessageIDRef inviteMsgId,
	InviteResponseCode code,
	const cb::OnChainUpdateByMsgCb &cb)
{
	rj::StringBuffer s;
	rj::Writer<rj::StringBuffer> writer(s);
	writer.StartObject();
	writer.Key(codeKeyName.c_str(), static_cast<rj::SizeType>(codeKeyName.length()));
	writer.Uint(static_cast<uint32_t>(code));
	writer.Key(msgIdKeyName.c_str(), static_cast<rj::SizeType>(msgIdKeyName.length()));
	writer.String(inviteMsgId.data(), static_cast<rj::SizeType>(inviteMsgId.length()));
	writer.Key(nameKeyName.c_str(), static_cast<rj::SizeType>(nameKeyName.length()));
	writer.String(to.data(), static_cast<rj::SizeType>(to.length()));
	writer.Key(fromInstanceKeyName.c_str(), static_cast<rj::SizeType>(fromInstanceKeyName.length()));
	writer.String(fromInstance.data(), static_cast<rj::SizeType>(fromInstance.size()));
	writer.EndObject();
	m_message->SetParam(attr::MESSAGE_CONTENT_paramName,
		string_view{s.GetString(), s.GetLength()});
	SetCommonFields(m_message,
		MessageType::invite_response, chatId,
		from, senderType,
		{}, cb);
}
bool AddPartMessage::IsMyMessage(const ChatMessagePtr& msg)
{
	auto type = MessageType::undefined;
	if (msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type))
	{
		return type == MessageType::add_part;
	}
	return false;
}
detail::parse_msg_res<add_part>
AddPartMessage::Parse(const ChatMessagePtr &msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto contentParsRes = ParseMsgContent(msg);
	if (!contentParsRes.success)
		return {};
	auto chatId = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	auto msgId = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	auto from = msg->GetParamStrRef(attr::FROM_paramName);
	ChatMessageTimestamp timestamp;
	if (!msg->GetParam(attr::TIMESTAMP_paramName, timestamp)
		|| chatId.empty()
		|| msgId.empty()
		|| from.empty())
	{
		return {};
	}
	return {true, add_part(chatId, msgId, timestamp, from, contentParsRes.content)};
}
detail::parse_msg_content_res<add_part_content>
AddPartMessage::ParseMsgContent(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto content = msg->GetParamStrRef(attr::MESSAGE_CONTENT_paramName);
	rj::Document doc;
	if (doc.Parse(content.c_str()).HasParseError() || !doc.IsObject())
		return {};
	auto nameIt = doc.FindMember(nameKeyName.c_str());
	auto typeIt = doc.FindMember(typeKeyName.c_str());
	auto fromInstanceIt = doc.FindMember(fromInstanceKeyName.c_str());
	if (nameIt == doc.MemberEnd()
		|| typeIt == doc.MemberEnd()
		|| !nameIt->value.IsString()
		|| !typeIt->value.IsUint()
		|| (fromInstanceIt != doc.MemberEnd() && !fromInstanceIt->value.IsString()))
	{
		return {};
	}
	CallID fromInstance;
	if (fromInstanceIt != doc.MemberEnd())
		fromInstance = std::string{fromInstanceIt->value.GetString(), fromInstanceIt->value.GetStringLength()};
	return {true, add_part_content(
		CallID{nameIt->value.GetString(), nameIt->value.GetStringLength()},
		static_cast<ParticipantType>(typeIt->value.GetUint()),
		std::move(fromInstance)
	)};
}
AddPartMessage::AddPartMessage(
	const chat::GlobalContext& ctx,
	CallIDRef partId,
	ParticipantType type,
	CallIDRef from,
	CallIDRef fromInstance,
	vs::CallIDType senderType,
	const cb::OnChainUpdateByMsgCb &cb)
{
	rj::StringBuffer s;
	rj::Writer<rj::StringBuffer> writer(s);
	writer.StartObject();
	writer.Key(nameKeyName.c_str(), static_cast<rj::SizeType>(nameKeyName.length()));
	writer.String(partId.data(), static_cast<rj::SizeType>(partId.length()));
	writer.Key(typeKeyName.c_str(), static_cast<rj::SizeType>(typeKeyName.length()));
	writer.Uint(static_cast<uint32_t>(type));
	writer.Key(fromInstanceKeyName.c_str(), static_cast<rj::SizeType>(fromInstanceKeyName.length()));
	writer.String(fromInstance.data(), static_cast<rj::SizeType>(fromInstance.size()));
	writer.EndObject();
	m_message->SetParam(attr::MESSAGE_CONTENT_paramName,
		string_view{s.GetString(), s.GetLength()});
	SetCommonFields(m_message,
		MessageType::add_part, ctx.chatId,
		from, senderType,
		{}, cb);
	PackGlobalContext(ctx, m_message);
}
chat::GlobalContext
AddPartMessage::GetGlobalContext(const ChatMessagePtr& msg)
{
	return UnpackGlobalContext(msg);
}
bool RemovePartMessage::IsMyMessage(const ChatMessagePtr& msg)
{
	auto type = MessageType::undefined;
	if (msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type))
	{
		return type == MessageType::remove_part;
	}
	return false;
}
detail::parse_msg_res<remove_part>
RemovePartMessage::Parse(const ChatMessagePtr &msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto contentParsRes = ParseMsgContent(msg);
	if (!contentParsRes.success)
		return {};
	auto chatId = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	auto msgId = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	auto from = msg->GetParamStrRef(attr::FROM_paramName);
	ChatMessageTimestamp timestamp;
	if (!msg->GetParam(attr::TIMESTAMP_paramName, timestamp)
		|| chatId.empty()
		|| msgId.empty()
		|| from.empty())
	{
		return {};
	}
	return {true, remove_part(chatId, msgId, timestamp, from, contentParsRes.content)};
}
detail::parse_msg_content_res<remove_part_content>
RemovePartMessage::ParseMsgContent(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto content = msg->GetParamStrRef(attr::MESSAGE_CONTENT_paramName);
	rj::Document doc;
	if (doc.Parse(content.c_str()).HasParseError() || !doc.IsObject())
		return {};
	auto nameIt = doc.FindMember(nameKeyName.c_str());
	auto typeIt = doc.FindMember(typeKeyName.c_str());
	auto fromInstanceIt = doc.FindMember(fromInstanceKeyName.c_str());
	if (nameIt == doc.MemberEnd()
		|| typeIt == doc.MemberEnd()
		|| !nameIt->value.IsString()
		|| !typeIt->value.IsUint()
		|| (fromInstanceIt != doc.MemberEnd() && !fromInstanceIt->value.IsString()))
	{
		return {};
	}
	CallID fromInstance;
	if (fromInstanceIt != doc.MemberEnd())
		fromInstance = std::string{fromInstanceIt->value.GetString(), fromInstanceIt->value.GetStringLength()};
	return {true, remove_part_content(
		CallID{nameIt->value.GetString(), nameIt->value.GetStringLength()},
		static_cast<ParticipantType>(typeIt->value.GetUint()),
		std::move(fromInstance)
	)};
}
RemovePartMessage::RemovePartMessage(
	ChatIDRef chatId,
	CallIDRef partId,
	ParticipantType type,
	CallIDRef from,
	CallIDRef fromInstance,
	vs::CallIDType senderType,
	const cb::OnChainUpdateByMsgCb& cb)
{
	rj::StringBuffer s;
	rj::Writer<rj::StringBuffer> writer(s);
	writer.StartObject();
	writer.Key(nameKeyName.c_str(), static_cast<rj::SizeType>(nameKeyName.length()));
	writer.String(partId.data(), static_cast<rj::SizeType>(partId.length()));
	writer.Key(typeKeyName.c_str(), static_cast<rj::SizeType>(typeKeyName.length()));
	writer.Uint(static_cast<uint32_t>(type));
	writer.Key(fromInstanceKeyName.c_str(), static_cast<rj::SizeType>(fromInstanceKeyName.length()));
	writer.String(fromInstance.data(), static_cast<rj::SizeType>(fromInstance.size()));
	writer.EndObject();
	m_message->SetParam(attr::MESSAGE_CONTENT_paramName,
		string_view{s.GetString(), s.GetLength()});
	SetCommonFields(m_message,
		MessageType::remove_part, chatId,
		from, senderType,
		{}, cb);
}
bool TextChatMessage::IsMyMessage(const ChatMessagePtr &m)
{
	auto type = MessageType::undefined;
	if (m->GetParamI32(attr::MESSAGE_TYPE_paramName, type))
	{
		return type == MessageType::text;
	}
	return false;
}
detail::parse_msg_res<text_message>
TextChatMessage::Parse(const ChatMessagePtr &msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto contentParsRes = ParseMsgContent(msg);
	if (!contentParsRes.success)
		return {};
	auto chatId = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	auto msgId = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	auto originalId = msg->GetParamStrRef(attr::ORIGINAL_MESSAGE_ID_paramName);
	auto author = msg->GetParamStrRef(attr::FROM_paramName);
	ChatMessageTimestamp timestamp;
	if (!msg->GetParam(attr::TIMESTAMP_paramName, timestamp)
		|| chatId.empty()
		|| msgId.empty()
		|| author.empty())
	{
		return {};
	}
	return {true, text_message(chatId, msgId, originalId, author, timestamp, contentParsRes.content)};
}
detail::parse_msg_content_res<text_message_content>
TextChatMessage::ParseMsgContent(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto content = msg->GetParamStrRef(attr::MESSAGE_CONTENT_paramName);
	rj::Document doc;
	if (doc.Parse(content.c_str()).HasParseError() || !doc.IsObject())
		return {};
	auto contentIt = doc.FindMember(contentKeyName.c_str());
	auto toIt = doc.FindMember(toKeyName.c_str());
	auto replyIdIt = doc.FindMember(replyIdKeyName.c_str());
	auto forwardIdIt = doc.FindMember(forwardTitleKeyName.c_str());
	auto fromInstanceIt = doc.FindMember(fromInstanceKeyName.c_str());
	if (contentIt == doc.MemberEnd()
		|| !contentIt->value.IsString()
		|| (fromInstanceIt != doc.MemberEnd() && !fromInstanceIt->value.IsString())
		|| (toIt != doc.MemberEnd() && !toIt->value.IsString())
		|| (replyIdIt != doc.MemberEnd() && !replyIdIt->value.IsString())
		|| (forwardIdIt != doc.MemberEnd() && !forwardIdIt->value.IsString()))
	{
		return {};
	}
	CallID to;
	if (toIt != doc.MemberEnd())
		to = CallID{toIt->value.GetString(), toIt->value.GetStringLength()};
	ChatMessageID replyId;
	if (replyIdIt != doc.MemberEnd())
		replyId = ChatMessageID{replyIdIt->value.GetString(), replyIdIt->value.GetStringLength()};
	ChatMessageID forwardId;
	if (forwardIdIt != doc.MemberEnd())
		forwardId = ChatMessageID{forwardIdIt->value.GetString(), forwardIdIt->value.GetStringLength()};
	CallID fromInstance;
	if (fromInstanceIt != doc.MemberEnd())
		fromInstance = std::string{fromInstanceIt->value.GetString(), fromInstanceIt->value.GetStringLength()};
	return {true, text_message_content(
		std::string{contentIt->value.GetString(), contentIt->value.GetStringLength()},
		std::move(to),
		std::move(replyId),
		std::move(forwardId),
		std::move(fromInstance)
	)};
}
ContentMessage&& ContentMessage::Text(
	const std::string& text)
{
	assert(m_type == MessageType::undefined);
	m_type = MessageType::text;
	rj::StringBuffer s;
	rj::Writer<rj::StringBuffer> writer(s);
	writer.StartObject();
	writer.Key(contentKeyName.c_str(), static_cast<rj::SizeType>(contentKeyName.length()));
	writer.String(text.c_str(), static_cast<rj::SizeType>(text.length()));
	writer.EndObject();
	m_message->SetParam(attr::MESSAGE_CONTENT_paramName,
		string_view{s.GetString(), s.GetLength()});
	return std::move(*this);
}
bool PartAddedToChatMessage::IsMyMessage(const ChatMessagePtr& msg)
{
	auto type = MessageType::undefined;
	if (msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type))
	{
		return type == MessageType::add_part_notification;
	}
	return false;
}
detail::parse_msg_res<part_added_to_chat>
PartAddedToChatMessage::Parse(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto contentParsRes = ParseMsgContent(msg);
	if (!contentParsRes.success)
		return {};
	auto chatId = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	auto msgId = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	if (chatId.empty()
		|| msgId.empty())
	{
		return {};
	}
	return {true, part_added_to_chat(chatId, msgId, contentParsRes.content)};
}
detail::parse_msg_content_res<part_added_to_chat_content>
PartAddedToChatMessage::ParseMsgContent(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto content = msg->GetParamStrRef(attr::MESSAGE_CONTENT_paramName);
	rj::Document doc;
	if (doc.Parse(content.c_str()).HasParseError() || !doc.IsObject())
		return {};
	auto whereChatIdIt = doc.FindMember(chatIdKeyName.c_str());
	auto chatTitleIt = doc.FindMember(chatTitleKeyName.c_str());
	auto triggerMsgIdIt = doc.FindMember(msgIdKeyName.c_str());
	auto addedPartIt = doc.FindMember(ownerKeyName.c_str());
	auto whoAddedIt = doc.FindMember(whoAddedKeyName.c_str());
	auto p2pPartIt = doc.FindMember(p2pPartKeyName.c_str());
	auto toIt = doc.FindMember(toKeyName.c_str());
	auto fromInstanceIt = doc.FindMember(fromInstanceKeyName.c_str());
	if (whereChatIdIt == doc.MemberEnd()
		|| chatTitleIt == doc.MemberEnd()
		|| triggerMsgIdIt == doc.MemberEnd()
		|| addedPartIt == doc.MemberEnd()
		|| whoAddedIt == doc.MemberEnd()
		|| !whereChatIdIt->value.IsString()
		|| !chatTitleIt->value.IsString()
		|| !triggerMsgIdIt->value.IsString()
		|| !addedPartIt->value.IsString()
		|| !whoAddedIt->value.IsString()
		|| (fromInstanceIt != doc.MemberEnd() && !fromInstanceIt->value.IsString())
		|| (p2pPartIt != doc.MemberEnd() && !p2pPartIt->value.IsString())
		|| (toIt != doc.MemberEnd() && !toIt->value.IsString()))
	{
		return {};
	}
	CallID p2pPart;
	if (p2pPartIt != doc.MemberEnd())
		p2pPart = CallID{p2pPartIt->value.GetString(), p2pPartIt->value.GetStringLength()};
	CallID to;
	if(toIt != doc.MemberEnd())
		to = CallID{toIt->value.GetString(), toIt->value.GetStringLength()};
	CallID fromInstance;
	if (fromInstanceIt != doc.MemberEnd())
		fromInstance = std::string{fromInstanceIt->value.GetString(), fromInstanceIt->value.GetStringLength()};
	return {true, part_added_to_chat_content(
		ChatID{whereChatIdIt->value.GetString(), whereChatIdIt->value.GetStringLength()},
		std::string{chatTitleIt->value.GetString(), chatTitleIt->value.GetStringLength()},
		ChatMessageID{triggerMsgIdIt->value.GetString(), triggerMsgIdIt->value.GetStringLength()},
		CallID{addedPartIt->value.GetString(), addedPartIt->value.GetStringLength()},
		CallID{whoAddedIt->value.GetString(), whoAddedIt->value.GetStringLength()},
		std::move(p2pPart),
		std::move(to),
		std::move(fromInstance)
	)};
}
PartAddedToChatMessage::PartAddedToChatMessage(
	ChatIDRef chatId,
	CallIDRef from,
	CallIDRef fromInstance,
	vs::CallIDType senderType,
	CallIDRef to,
	ChatIDRef whereWasAdded, // id of chat to which user was added
	string_view title,
	ChatMessageIDRef addPartMsgId, // AddPart message id
	CallIDRef addedPart, // participant who was added
	CallIDRef whoAdded, // who added participant
	CallIDRef p2pPart, // Peer for p2p chat
	ChatMessageTimestamp timestamp,
	const cb::OnChainUpdateByMsgCb& cb)
{
	rj::StringBuffer s;
	rj::Writer<rj::StringBuffer> writer(s);
	writer.StartObject();
	writer.Key(chatIdKeyName.c_str(),
		static_cast<rj::SizeType>(chatIdKeyName.length()));
	writer.String(whereWasAdded.data(),
		static_cast<rj::SizeType>(whereWasAdded.length()));
	writer.Key(chatTitleKeyName.c_str(), static_cast<rj::SizeType>(chatTitleKeyName.length()));
	writer.String(title.data(),
		static_cast<rj::SizeType>(title.length()));
	writer.Key(msgIdKeyName.c_str(), static_cast<rj::SizeType>(msgIdKeyName.length()));
	writer.String(addPartMsgId.data(),
		static_cast<rj::SizeType>(addPartMsgId.length()));
	writer.Key(ownerKeyName.c_str(), static_cast<rj::SizeType>(ownerKeyName.length()));
	writer.String(addedPart.data(), static_cast<rj::SizeType>(addedPart.length()));
	writer.Key(whoAddedKeyName.c_str(), static_cast<rj::SizeType>(whoAddedKeyName.length()));
	writer.String(whoAdded.data(), static_cast<rj::SizeType>(whoAdded.length()));
	if (!p2pPart.empty())
	{
		writer.Key(p2pPartKeyName.c_str(), static_cast<rj::SizeType>(p2pPartKeyName.length()));
		writer.String(p2pPart.data(), static_cast<rj::SizeType>(p2pPart.length()));
	}
	writer.Key(toKeyName.c_str(), static_cast<rj::SizeType>(toKeyName.length()));
	writer.String(to.data(), static_cast<rj::SizeType>(to.length()));
	writer.Key(fromInstanceKeyName.c_str(), static_cast<rj::SizeType>(fromInstanceKeyName.length()));
	writer.String(fromInstance.data(), static_cast<rj::SizeType>(fromInstance.size()));
	writer.EndObject();
	m_message->SetParam(attr::MESSAGE_CONTENT_paramName,
		string_view{s.GetString(), s.GetLength()});
	SetCommonFields(m_message,
		MessageType::add_part_notification,
		chatId, from, senderType, timestamp, cb);
}
bool PartRemovedFromChatMessage::IsMyMessage(const ChatMessagePtr& msg)
{
	auto type = MessageType::undefined;
	if (msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type))
	{
		return type == MessageType::remove_part_notification;
	}
	return false;
}
detail::parse_msg_res<part_removed_from_chat>
PartRemovedFromChatMessage::Parse(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto contentParsRes = ParseMsgContent(msg);
	if (!contentParsRes.success)
		return {};
	auto chatId = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	auto msgId = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	if (chatId.empty()
		|| msgId.empty())
	{
		return {};
	}
	return {true, part_removed_from_chat(chatId, msgId, contentParsRes.content)};
}
detail::parse_msg_content_res<part_removed_from_chat_content>
PartRemovedFromChatMessage::ParseMsgContent(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto content = msg->GetParamStrRef(attr::MESSAGE_CONTENT_paramName);
	rj::Document doc;
	if (doc.Parse(content.c_str()).HasParseError() || !doc.IsObject())
		return {};
	auto whereChatIdIt = doc.FindMember(chatIdKeyName.c_str());
	auto removedPartIdIt = doc.FindMember(ownerKeyName.c_str());
	auto triggerMsgIdIt = doc.FindMember(msgIdKeyName.c_str());
	auto permissionsIt = doc.FindMember(permissionsKeyName.c_str());
	auto toIt = doc.FindMember(toKeyName.c_str());
	auto fromInstanceIt = doc.FindMember(fromInstanceKeyName.c_str());
	if (whereChatIdIt == doc.MemberEnd()
		|| removedPartIdIt == doc.MemberEnd()
		|| triggerMsgIdIt == doc.MemberEnd()
		|| !whereChatIdIt->value.IsString()
		|| !removedPartIdIt->value.IsString()
		|| !triggerMsgIdIt->value.IsString()
		|| (fromInstanceIt != doc.MemberEnd() && !fromInstanceIt->value.IsString())
		|| (permissionsIt != doc.MemberEnd() && !permissionsIt->value.IsString())
		|| (toIt != doc.MemberEnd() && !toIt->value.IsString()))
	{
		return {};
	}
	std::string permissions;
	if (permissionsIt != doc.MemberEnd())
		permissions = std::string{permissionsIt->value.GetString(), permissionsIt->value.GetStringLength()};
	CallID to;
	if (toIt != doc.MemberEnd())
		to = CallID{toIt->value.GetString(), toIt->value.GetStringLength()};
	CallID fromInstance;
	if (fromInstanceIt != doc.MemberEnd())
		fromInstance = std::string{fromInstanceIt->value.GetString(), fromInstanceIt->value.GetStringLength()};
	return {true, part_removed_from_chat_content(
		ChatID{whereChatIdIt->value.GetString(), whereChatIdIt->value.GetStringLength()},
		CallID{removedPartIdIt->value.GetString(), removedPartIdIt->value.GetStringLength()},
		ChatMessageID{triggerMsgIdIt->value.GetString(), triggerMsgIdIt->value.GetStringLength()},
		std::move(permissions),
		std::move(to),
		std::move(fromInstance)
	)};
}
PartRemovedFromChatMessage::PartRemovedFromChatMessage(
	ChatIDRef chatId,
	CallIDRef from,
	CallIDRef fromInstance,
	vs::CallIDType senderType,
	CallIDRef to,
	ChatIDRef whereRemoved,
	CallID removedPart,
	ChatMessageIDRef removePartMsgId, // RemovePart message id
	std::string leavePermissions, // Set of flags in json {show history, ban, ...}. Not implement yet
	ChatMessageTimestamp timestamp,
	const cb::OnChainUpdateByMsgCb& cb)
{
	rj::StringBuffer s;
	rj::Writer<rj::StringBuffer> writer(s);
	writer.StartObject();
	writer.Key(chatIdKeyName.c_str(), static_cast<rj::SizeType>(chatIdKeyName.length()));
	writer.String(whereRemoved.data(), static_cast<rj::SizeType>(whereRemoved.length()));
	writer.Key(ownerKeyName.c_str(), static_cast<rj::SizeType>(ownerKeyName.length()));
	writer.String(removedPart.data(), static_cast<rj::SizeType>(removedPart.length()));
	writer.Key(msgIdKeyName.c_str(), static_cast<rj::SizeType>(msgIdKeyName.length()));
	writer.String(removePartMsgId.data(), static_cast<rj::SizeType>(removePartMsgId.length()));
	if (!leavePermissions.empty())
	{
		writer.Key(permissionsKeyName.c_str(), static_cast<rj::SizeType>(permissionsKeyName.length()));
		writer.String(leavePermissions.c_str(), static_cast<rj::SizeType>(leavePermissions.length()));
	}
	writer.Key(toKeyName.c_str(), static_cast<rj::SizeType>(toKeyName.length()));
	writer.String(to.data(), static_cast<rj::SizeType>(to.length()));
	writer.Key(fromInstanceKeyName.c_str(), static_cast<rj::SizeType>(fromInstanceKeyName.length()));
	writer.String(fromInstance.data(), static_cast<rj::SizeType>(fromInstance.length()));
	writer.EndObject();
	m_message->SetParam(attr::MESSAGE_CONTENT_paramName,
		string_view{s.GetString(), s.GetLength()});
	SetCommonFields(m_message,
		MessageType::remove_part_notification,
		chatId, from, senderType,
		timestamp, cb);
}
// BucketSyncMessage
void BucketSyncMessage::PreFill(
	ChatIDRef chatId,
	CallIDRef from,
	string_view method)
{
	m_message->Clear();
	m_message->SetParamI32(attr::MESSAGE_TYPE_paramName, MessageType::sync);
	m_message->SetParam(attr::FROM_paramName, from);
	m_message->SetParam(attr::CHAT_ID_paramName, chatId);
	m_message->SetParam(attr::BUCKETSYNC_METHOD_paramName, method);
	m_message->SetParam(attr::SYNC_ID_paramName, m_sync_id);
}

bool BucketSyncMessage::IsMyMessage(const ChatMessagePtr& m)
{
	auto type = MessageType::undefined;
	m->GetParamI32(attr::MESSAGE_TYPE_paramName, type);
	auto syncMethod = m->GetParamStrRef(attr::BUCKETSYNC_METHOD_paramName);
	auto chatId = m->GetParamStrRef(attr::CHAT_ID_paramName);
	return !chatId.empty()
		&& (type == MessageType::sync
			|| syncMethod == attr::INSERT_MSG);
}

BucketSyncMessage::BucketSyncMessage(string_view syncId) : m_sync_id(syncId)
{
}
void BucketSyncMessage::MakeBucketListReq(
	ChatIDRef chatId,
	CallIDRef from,
	uint32_t msgsInTail,
	string_view tailHash,
	bool syncStart)
{
	PreFill(chatId, from, attr::SYNC_BUCKET_LIST_REQ);
	m_message->SetParamI32(
		attr::TAIL_LENGTH_paramName,
		msgsInTail);// <<<< ???
	m_message->SetParam(attr::TAIL_HASH_paramName, tailHash);
	if (syncStart)
		m_message->SetParam(attr::SYNC_BUCKET_START, true);
}
void BucketSyncMessage::MakeBucketListResp(
	ChatIDRef chatId,
	CallIDRef from,
	const std::vector<BucketIdHash> &bucketsList)
{
	PreFill(chatId, from, attr::SYNC_BUCKET_LIST_RESP);
	for (const auto &i : bucketsList)
	{
		m_message->SetParamI64(
			attr::BUCKET_paramName,
			i.id);
		m_message->SetParam(attr::BUCKET_HASH_paramName, i.hash);
	}

}
void BucketSyncMessage::MakeDiffReq(
	ChatIDRef chatId,
	CallIDRef from,
	const MsgIdsByBuckets& msgsByBuckets)
{
	PreFill(chatId, from, attr::GET_DIFF_MSG_REQ);
	for (const auto &i : msgsByBuckets)
	{
		m_message->SetParamI64(
			attr::BUCKET_paramName,
			i.id);
		for (const auto &j : i.msgIds)
		{
			m_message->SetParam(attr::MESSAGE_ID_paramName, j);
		}
	}
}
void BucketSyncMessage::MakeDiffResp(
	ChatIDRef chatId,
	CallIDRef from,
	const std::vector<ChatMessageID>& insertedMsgs)
{
	PreFill(chatId, from, attr::GET_DIFF_MSG_RESP);
	for (const auto &i : insertedMsgs)
		m_message->SetParam(attr::INSERTED_MSG_ID_paramName, i);
}
void BucketSyncMessage::MakeCheckSyncReq(
	ChatIDRef chatId,
	CallIDRef from,
	uint32_t tailLen,
	string_view tailHash,
	uint32_t masterWeight)
{
	PreFill(chatId, from, attr::SYNC_CHECK_REQ);
	m_message->SetParamI32(
		attr::TAIL_LENGTH_paramName,
		tailLen);
	m_message->SetParam(attr::TAIL_HASH_paramName, tailHash);
	m_message->SetParam(
		attr::MASTER_WEIGHT_paramName,
		static_cast<int32_t>(masterWeight));

}
void BucketSyncMessage::MakeCheckSyncResp(
	ChatIDRef chatId,
	CallIDRef from, SyncRes result)
{
	PreFill(chatId, from, attr::SYNC_CHECK_RESP);
	m_message->SetParamI32(
		attr::RESPONSE_CODE_paramName,
		result);
}
void BucketSyncMessage::MakeSyncResetReq(ChatIDRef chatId, CallIDRef from)
{
	PreFill(chatId, from, attr::RESET_SYNC_REQ);
}
bool BucketSyncMessage::SetChatMessage(ChatMessagePtr && m)
{
	if (!IsMyMessage(m))
		return false;
	m_message = std::move(m);
	return true;
}
std::vector<BucketSyncMessage::BucketIdHash>
BucketSyncMessage::GetBucketList(const ChatMessagePtr &msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto method = msg->GetParamStrRef(attr::BUCKETSYNC_METHOD_paramName);
	if (method != attr::SYNC_BUCKET_LIST_RESP)
		return {};
	const auto &cnt = msg->GetContainer();
	cnt.Reset();
	string_view bucketIdName = attr::BUCKET_paramName;
	string_view bucketHashName = attr::BUCKET_HASH_paramName;
	uint64_t bucketId(0);
	std::vector<BucketIdHash> res;
	while (cnt.Next())
	{
		if (bucketIdName == cnt.GetName())
		{
			if (!cnt.GetValueI64(bucketId))
				return {};
		}
		else if (bucketHashName == cnt.GetName())
		{
			auto str = cnt.GetStrValueRef();
			if (str == nullptr)
				return {};
			res.emplace_back(static_cast<uint32_t>(bucketId), str);
		}
	}
	return res;
}

MsgIdsByBuckets BucketSyncMessage::GetDiffReqInfo(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto method = msg->GetParamStrRef(attr::BUCKETSYNC_METHOD_paramName);
	if (method != attr::GET_DIFF_MSG_REQ)
		return {};
	const auto &cnt = msg->GetContainer();
	cnt.Reset();
	string_view bucketIdName = attr::BUCKET_paramName;
	string_view msgIdName = attr::MESSAGE_ID_paramName;
	MsgIdsByBuckets res;
	while (cnt.Next())
	{
		string_view name = cnt.GetName();
		if (bucketIdName == name)
		{
			uint64_t val(0);
			cnt.GetValueI64(val);
			res.emplace_back(val);
		}
		else if (msgIdName == name)
		{
			const auto val = cnt.GetStrValueRef();
			if (!val)
				return {};
			res.back().msgIds.emplace_back(val);
		}
	}
	return res;
}
vs::set<ChatMessageID> BucketSyncMessage::GetDiffMsgIds(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto method = msg->GetParamStrRef(attr::BUCKETSYNC_METHOD_paramName);
	if (method != attr::GET_DIFF_MSG_RESP)
		return {};
	const auto &cnt = msg->GetContainer();
	vs::set<ChatMessageID> res;
	cnt.Reset();
	while (cnt.Next())
	{
		string_view name = cnt.GetName();
		if (attr::INSERTED_MSG_ID_paramName == name)
		{
			const auto val = cnt.GetStrValueRef();
			if (val == nullptr)
				return {};
			res.emplace(val);
		}
	}
	return res;
}
BucketSyncMessage::CheckSyncInfoSt
BucketSyncMessage::GetCheckSyncInfo(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto method = msg->GetParamStrRef(attr::BUCKETSYNC_METHOD_paramName);
	auto chatId = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	auto from = msg->GetParamStrRef(attr::FROM_paramName);
	auto hash = msg->GetParamStrRef(attr::TAIL_HASH_paramName);
	int32_t len(0);
	int32_t weight(0);
	if (method != attr::SYNC_CHECK_REQ
		|| chatId.empty()
		|| from.empty()
		|| hash.empty()
		|| !msg->GetParam(attr::TAIL_LENGTH_paramName, len)
		|| !msg->GetParam(attr::MASTER_WEIGHT_paramName, weight))
	{
		return {};
	}
	return { chatId, from, len, hash, weight };
}
BucketSyncMessage::CheckSyncResponseSt
BucketSyncMessage::GetCheckSyncResult(const ChatMessagePtr& msg)
{
	CheckSyncResponseSt res;
	if (!IsMyMessage(msg))
		return {};
	auto method = msg->GetParamStrRef(attr::BUCKETSYNC_METHOD_paramName);
	auto chatId = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	auto from = msg->GetParamStrRef(attr::FROM_paramName);
	auto result = SyncRes::undef;
	if (method.empty()
		|| chatId.empty()
		|| from.empty()
		|| !msg->GetParamI32(attr::RESPONSE_CODE_paramName, result))
	{
		return {};
	}
	res.chatId.assign(chatId.begin(), chatId.end());
	res.from.assign(from.begin(), from.end());
	res.result =result;
	return res;
}
bool DeliveryConfirm::IsMyMessage(const ChatMessagePtr& msg)
{
	auto type = MessageType::delivered;
	msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type);
	return type == MessageType::delivered;
}
DeliveryConfirm::DeliveryConfirm(ChatMessageIDRef msgId, CallIDRef from, CallIDRef fromInstance, CallIDRef to)
{
	m_message->SetParamI32(attr::MESSAGE_TYPE_paramName, MessageType::delivered);
	m_message->SetParam(attr::MESSAGE_ID_paramName, msgId);
	m_message->SetParam(attr::FROM_paramName, from);
	m_message->SetParam(attr::FROM_INSTANCE_paramName, fromInstance);
	m_message->SetParam(attr::DST_ENDPOINT_paramName, to);
}
DeliveryConfirm::DeliveryInfo
DeliveryConfirm::GetDeliveryInfo(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto msgId = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	auto from = msg->GetParamStrRef(attr::FROM_paramName);
	auto fromInstance = msg->GetParamStrRef(attr::FROM_INSTANCE_paramName);
	auto to = msg->GetParamStrRef(attr::DST_ENDPOINT_paramName);
	return {true, msgId, from, fromInstance, to};
}
bool FecthAllUserPersonalCtxsReqMessage::IsMyMessage(const ChatMessagePtr& msg)
{
	auto type = MessageType::undefined;
	msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type);
	return type == MessageType::fetch_all_personal_ctxs_req;
}
FecthAllUserPersonalCtxsReqMessage::GetReqResult
FecthAllUserPersonalCtxsReqMessage::GetReqData(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto from = msg->GetParamStrRef(attr::FROM_paramName);
	auto to = msg->GetParamStrRef(attr::DST_ENDPOINT_paramName);
	uint32_t id(-1);
	if (!msg->GetParamI32(attr::REQ_ID_paramName, id)
		|| from.empty()
		|| to.empty())
	{
		return {};
	}
	return {true, id, from, to};
}
FecthAllUserPersonalCtxsReqMessage::FecthAllUserPersonalCtxsReqMessage(
	uint32_t reqId, CallIDRef to, CallIDRef from)
{
	m_message->SetParamI32(attr::MESSAGE_TYPE_paramName, MessageType::fetch_all_personal_ctxs_req);
	m_message->SetParamI32(attr::REQ_ID_paramName, reqId);
	m_message->SetParam(attr::DST_ENDPOINT_paramName, to);
	m_message->SetParam(attr::FROM_paramName, from);
}
bool FetchAllUserPersonalCtxsRespMessage::IsMyMessage(const ChatMessagePtr& msg)
{
	auto type = MessageType::undefined;
	msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type);
	return type == MessageType::fetch_all_personal_ctxs_resp;
}
bool FetchAllUserPersonalCtxsRespMessage::MakeResponse(
	const FecthAllUserPersonalCtxsReqMessage::GetReqResult& req,
	const chat::PersonalContextList& ctxs)
{
	m_message->SetParamI32(attr::MESSAGE_TYPE_paramName, MessageType::fetch_all_personal_ctxs_resp);
	m_message->SetParamI32(attr::REQ_ID_paramName, req.reqId);
	m_message->SetParam(attr::DST_ENDPOINT_paramName, req.from);
	m_message->SetParam(attr::FROM_paramName, req.to);
	PackPersonalContexts(ctxs, m_message);
	return true;
}
FetchAllUserPersonalCtxsRespMessage::GetResponseResult
FetchAllUserPersonalCtxsRespMessage::GetResponseData(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {uint32_t(-1)};
	uint32_t reqId(-1);
	if(!msg->GetParamI32(attr::REQ_ID_paramName, reqId))
		return { uint32_t(-1) };
	auto allCtxs = UnPackPersonalContexts(msg);
	return {true, reqId, std::move(allCtxs)};
}
GetGlobalContextReqMessage::GetGlobalContextReqMessage(uint32_t reqId, ChatIDRef chatId,
	CallIDRef to, CallIDRef from)
{
	m_message->SetParamI32(attr::MESSAGE_TYPE_paramName, MessageType::get_global_ctx_req);
	m_message->SetParamI32(attr::REQ_ID_paramName, reqId);
	m_message->SetParam(attr::CHAT_ID_paramName, chatId);
	m_message->SetParam(attr::FROM_paramName, from);
	m_message->SetParam(attr::DST_ENDPOINT_paramName, to);
}
bool GetGlobalContextReqMessage::IsMyMessage(const ChatMessagePtr& msg)
{
	auto type = MessageType::undefined;
	msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type);
	return type == MessageType::get_global_ctx_req;
}
GetGlobalContextReqMessage::GetReqResult
GetGlobalContextReqMessage::GetReqData(const ChatMessagePtr &m)
{
	if (!IsMyMessage(m))
		return { false, -1, ChatIDRef(),CallIDRef() };
	uint32_t reqId(-1);
	if(!m->GetParam(attr::REQ_ID_paramName, reqId))
		return { false, -1, ChatIDRef(),CallIDRef() };
	auto chatId = m->GetParamStrRef(attr::CHAT_ID_paramName);
	auto from = m->GetParamStrRef(attr::FROM_paramName);
	return { !chatId.empty() && !from.empty(), reqId, chatId,from };
}
bool GetGlobalContextRespMessage::IsMyMessage(const ChatMessagePtr& msg)
{
	auto type = MessageType::undefined;
	msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type);
	return type == MessageType::get_global_ctx_resp;
}
bool GetGlobalContextRespMessage::MakeResponse(uint32_t reqId, CallIDRef from,
	const chat::GlobalContext &info)
{
	m_message->SetParamI32(attr::MESSAGE_TYPE_paramName, MessageType::get_global_ctx_resp);
	m_message->SetParamI32(attr::REQ_ID_paramName, reqId);
	m_message->SetParam(attr::DST_ENDPOINT_paramName, from);
	PackGlobalContext(info, m_message);
	return true;
}
GetGlobalContextRespMessage::GetResponseResult
GetGlobalContextRespMessage::GetResponseData(const ChatMessagePtr &m)
{
	if (!IsMyMessage(m))
		return {};
	uint32_t reqId(0);
	if (!m->GetParam(attr::REQ_ID_paramName, reqId))
		return{};

	auto res = UnpackGlobalContext(m);
	if (res.chatId.empty())
		return {};
	return { true, reqId, std::move(res) };
}
bool GetTailHashReqMessage::IsMyMessage(const ChatMessagePtr& msg)
{
	auto type = MessageType::undefined;
	msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type);
	return type == MessageType::get_tail_hash_req;
}
GetTailHashReqMessage::GetTailHashReqMessage(
		ChatIDRef chatId, uint32_t tailLen,
		CallIDRef from, CallIDRef to)
{
	m_message->SetParamI32(attr::MESSAGE_TYPE_paramName, MessageType::get_tail_hash_req);
	m_message->SetParam(attr::CHAT_ID_paramName, chatId);
	m_message->SetParamI32(attr::TAIL_LENGTH_paramName, tailLen);
	m_message->SetParam(attr::FROM_paramName, from);
	m_message->SetParam(attr::DST_ENDPOINT_paramName, to);
}
GetTailHashReqMessage::GetReqResult
GetTailHashReqMessage::GetReqData(const ChatMessagePtr &msg)
{
	if(!IsMyMessage(msg))
		return {};
	auto chatId = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	auto from = msg->GetParamStrRef(attr::FROM_paramName);
	uint32_t tailLen(0);
	if(chatId.empty()
		|| from.empty()
		|| !msg->GetParamI32(attr::TAIL_LENGTH_paramName,tailLen))
	{
		return {};
	}
	return {true, chatId, tailLen, from};
}
bool GetTailHashRespMessage::IsMyMessage(const ChatMessagePtr &msg)
{
	auto type = MessageType::undefined;
	msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type);
	return type == MessageType::get_tail_hash_resp;
}
GetTailHashRespMessage::GetTailHashRespMessage(ChatIDRef chatId, ReqResult reqResult,
		uint32_t tailLen, string_view hash, CallIDRef from, CallIDRef to)
{
	m_message->SetParamI32(attr::MESSAGE_TYPE_paramName, MessageType::get_tail_hash_resp);
	m_message->SetParam(attr::CHAT_ID_paramName, chatId);
	m_message->SetParamI32(attr::ERROR_CODE_paramName, reqResult);
	m_message->SetParamI32(attr::TAIL_LENGTH_paramName, tailLen);
	m_message->SetParam(attr::TAIL_HASH_paramName, hash);
	m_message->SetParam(attr::FROM_paramName, from);
	m_message->SetParam(attr::DST_ENDPOINT_paramName, to);
}

GetTailHashRespMessage::GetResponseResult
GetTailHashRespMessage::GetResponseData(const ChatMessagePtr &msg)
{
	if(!IsMyMessage(msg))
		return {};
	auto chatId = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	auto reqResult(ReqResult::failed);
	if(chatId.empty()
		|| !msg->GetParamI32(attr::ERROR_CODE_paramName,reqResult))
		return {};
	uint32_t tailLen(0);
	msg->GetParamI32(attr::TAIL_LENGTH_paramName, tailLen);
	auto hash = msg->GetParamStrRef(attr::TAIL_HASH_paramName);
	auto from = msg->GetParamStrRef(attr::FROM_paramName);
	return{true, chatId, reqResult, tailLen, hash, from };
}
bool MessageRemovalMessage::IsMyMessage(const ChatMessagePtr& msg)
{
	auto type = MessageType::undefined;
	if (msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type))
	{
		return type == MessageType::del_msg_self ||
			type == MessageType::del_msg_all;
	}
	return false;
}
detail::parse_msg_res<remove_message>
MessageRemovalMessage::Parse(const ChatMessagePtr &msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto contentParsRes = ParseMsgContent(msg);
	if (!contentParsRes.success)
		return {};
	auto type = MessageType::undefined;
	msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type);
	auto for_all = type == MessageType::del_msg_all;
	auto chatId = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	auto msgId = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	auto from = msg->GetParamStrRef(attr::FROM_paramName);
	ChatMessageTimestamp timestamp;
	if (!msg->GetParam(attr::TIMESTAMP_paramName, timestamp)
		|| chatId.empty()
		|| msgId.empty()
		|| from.empty())
	{
		return {};
	}
	return {true, remove_message(chatId, msgId, timestamp, from, for_all, contentParsRes.content)};
}
detail::parse_msg_content_res<remove_message_content>
MessageRemovalMessage::ParseMsgContent(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto content = msg->GetParamStrRef(attr::MESSAGE_CONTENT_paramName);
	rj::Document doc;
	if (doc.Parse(content.c_str()).HasParseError() || !doc.IsObject())
		return {};
	auto msgIdIt = doc.FindMember(msgIdKeyName.c_str());
	auto fromInstanceIt = doc.FindMember(fromInstanceKeyName.c_str());
	if (msgIdIt == doc.MemberEnd()
		|| !msgIdIt->value.IsString()
		|| (fromInstanceIt != doc.MemberEnd() && !fromInstanceIt->value.IsString()))
	{
		return {};
	}
	CallID fromInstance;
	if (fromInstanceIt != doc.MemberEnd())
		fromInstance = std::string{fromInstanceIt->value.GetString(), fromInstanceIt->value.GetStringLength()};
	return {true, remove_message_content(
		ChatMessageID{msgIdIt->value.GetString(), msgIdIt->value.GetStringLength()},
		std::move(fromInstance)
	)};
}
MessageRemovalMessage::MessageRemovalMessage(
	ChatIDRef chatId,
	ChatMessageIDRef msgId,
	bool forAll,
	CallIDRef from,
	CallIDRef fromInstance,
	vs::CallIDType senderType,
	const cb::OnChainUpdateByMsgCb &cb)
{
	rj::StringBuffer s;
	rj::Writer<rj::StringBuffer> writer(s);
	rj::Document doc;
	writer.StartObject();
	writer.Key(msgIdKeyName.c_str(), static_cast<rj::SizeType>(msgIdKeyName.length()));
	writer.String(msgId.data(), static_cast<rj::SizeType>(msgId.size()));
	writer.Key(fromInstanceKeyName.c_str(), static_cast<rj::SizeType>(fromInstanceKeyName.length()));
	writer.String(fromInstance.data(), static_cast<rj::SizeType>(fromInstance.size()));
	writer.EndObject();
	m_message->SetParam(attr::MESSAGE_CONTENT_paramName,
		string_view{s.GetString(), s.GetLength()});

	auto type = forAll
		? MessageType::del_msg_all
		: MessageType::del_msg_self;

	SetCommonFields(m_message,
		type, chatId,
		from, senderType,
		{}, cb);
}
bool ClearHistoryMessage::IsMyMessage(const ChatMessagePtr& msg)
{
	auto type = MessageType::undefined;
	if (msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type))
	{
		return type == MessageType::clear_history_for_part ||
			type == MessageType::clear_history_for_all;
	}
	return false;
}
detail::parse_msg_res<clear_history>
ClearHistoryMessage::Parse(const ChatMessagePtr &msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto contentParsRes = ParseMsgContent(msg);
	if (!contentParsRes.success)
		return {};
	auto type = MessageType::undefined;
	msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type);
	auto for_all = type == MessageType::clear_history_for_all;
	auto chatId = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	auto msgId = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	auto from = msg->GetParamStrRef(attr::FROM_paramName);
	ChatMessageTimestamp timestamp;
	if (!msg->GetParam(attr::TIMESTAMP_paramName, timestamp)
		|| chatId.empty()
		|| msgId.empty()
		|| from.empty())
	{
		return {};
	}
	return {true, clear_history(chatId, msgId, timestamp, from, for_all, contentParsRes.content)};
}
detail::parse_msg_content_res<clear_history_content>
ClearHistoryMessage::ParseMsgContent(const ChatMessagePtr& msg)
{
	if (!IsMyMessage(msg))
		return {};
	auto content = msg->GetParamStrRef(attr::MESSAGE_CONTENT_paramName);
	rj::Document doc;
	if (doc.Parse(content.c_str()).HasParseError() || !doc.IsObject())
		return {};
	auto fromInstanceIt = doc.FindMember(fromInstanceKeyName.c_str());
	if (fromInstanceIt != doc.MemberEnd() && !fromInstanceIt->value.IsString())
		return {};
	CallID fromInstance;
	if (fromInstanceIt != doc.MemberEnd())
		fromInstance = std::string{fromInstanceIt->value.GetString(), fromInstanceIt->value.GetStringLength()};
	return {true, clear_history_content(
		std::move(fromInstance)
		)};
}
ClearHistoryMessage::ClearHistoryMessage(
	chat::ChatIDRef chatId,
	bool forAll,
	CallIDRef from,
	CallIDRef fromInstance,
	vs::CallIDType senderType,
	const cb::OnChainUpdateByMsgCb &cb)
{
	rj::StringBuffer s;
	rj::Writer<rj::StringBuffer> writer(s);
	rj::Document doc;
	writer.StartObject();
	writer.Key(fromInstanceKeyName.c_str(), static_cast<rj::SizeType>(fromInstanceKeyName.length()));
	writer.String(fromInstance.data(), static_cast<rj::SizeType>(fromInstance.size()));
	writer.EndObject();
	m_message->SetParam(attr::MESSAGE_CONTENT_paramName,
		string_view{s.GetString(), s.GetLength()});

	auto type = forAll
		? MessageType::clear_history_for_all
		: MessageType::clear_history_for_part;

	SetCommonFields(m_message,
		type, chatId,
		from, senderType,
		{}, cb);
}
ContentMessage&& ContentMessage::Reply(ChatMessageIDRef reply_msg_id)
{
	assert(GetParamStrFromMsgContent(m_message, replyIdKeyName).empty()); // Not yet set
	assert(m_message->GetParamStrRef(attr::ORIGINAL_MESSAGE_ID_paramName).empty()); // Can't be both edit and reply
	assert(GetParamStrFromMsgContent(m_message, replyIdKeyName).empty()); // Same for forward message title

	InsertParamStrIntoMsgContent(m_message, replyIdKeyName, reply_msg_id);
	return std::move(*this);
}
ContentMessage&& ContentMessage::Edit(ChatMessageIDRef original_msg_id)
{
	assert(GetParamStrFromMsgContent(m_message, replyIdKeyName).empty()); // Can't be both edit and reply
	assert(m_message->GetParamStrRef(attr::ORIGINAL_MESSAGE_ID_paramName).empty()); // Not yet set
	assert(GetParamStrFromMsgContent(m_message, replyIdKeyName).empty()); // Same for forward message title

	m_message->SetParam(attr::ORIGINAL_MESSAGE_ID_paramName, original_msg_id);
	return std::move(*this);
}
ContentMessage&& ContentMessage::Forward(string_view title)
{
	assert(GetParamStrFromMsgContent(m_message, replyIdKeyName).empty()); // Can't be both edit and reply
	assert(m_message->GetParamStrRef(attr::ORIGINAL_MESSAGE_ID_paramName).empty()); // Not yet set
	assert(GetParamStrFromMsgContent(m_message, replyIdKeyName).empty()); // Same for forward message title

	InsertParamStrIntoMsgContent(m_message, forwardTitleKeyName, title);
	return std::move(*this);
}
ChatMessagePtr ContentMessage::Seal(
	ChatIDRef id, CallIDRef from,
	CallIDRef fromInstance,
	vs::CallIDType senderType,
	ChatMessageTimestamp timestamp,
	const cb::OnChainUpdateByMsgCb & cb) &&
{
	SetCommonFields(m_message, m_type, id, from, senderType, timestamp, cb);
	InsertParamStrIntoMsgContent(m_message, fromInstanceKeyName, fromInstance);
	return std::move(m_message);
}
}
}