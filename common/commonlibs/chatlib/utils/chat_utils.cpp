#include "chat_utils.h"

#include "chatlib/msg/attr.h"
#include "chatlib/msg/chat_messages_construct.h"
#include "chatlib/utils/msg_utils.h"

#include "std-generic/clib/sha1.h"
#include "std-generic/cpplib/VS_Container.h"

#include <cassert>
#include <iomanip>
#include <random>
#include <sstream>

namespace chat
{
std::random_device s_chatRandDevice;
std::function<ChatMessageID()> s_UUIDGenFunc;

ChatID GenerateChatID()
{
	SHA1 sha1;
	sha1.Update(GenerateUUID());
	sha1.Final();
	char result[41] = {0};
	sha1.GetString(result);
	return result;
}
std::string GenerateUUID()
{
	return s_UUIDGenFunc();
}
ChatID GetP2PChatID(CallIDRef user1, CallIDRef user2)
{
	SHA1 sha1;
	if(user1 < user2)
	{
		sha1.Update(user1);
		sha1.Update(user2);
	}
	else
	{
		sha1.Update(user2);
		sha1.Update(user1);
	}
	sha1.Final();
	char hash[41] = {0};
	sha1.GetString(hash);
	return hash;
}
bool IsChatP2P(const msg::ChatMessagePtr &msg)
{
	if (IsMessageStorable(msg)
		&& !msg->GetParamStrRef(attr::FROM_paramName).empty()
		&& !GetParamStrFromMsgContent(msg, msg::toKeyName).empty())
	{
		return true;
	}
	return false;
}
std::string GenerateSalt()
{
	std::stringstream ss;
	std::uniform_int_distribution<uint16_t> distr(0, 0xff);
	for (auto i = 0; i < 4; i++)
		ss << std::hex << std::setfill('0') << std::setw(2) << distr(s_chatRandDevice);
	return ss.str();
}
uint32_t Generate32()
{
	std::uniform_int_distribution<uint32_t> distr(0, std::numeric_limits<uint32_t>::max());
	return distr(s_chatRandDevice);
}
bool IsMessageStorable(const msg::ChatMessagePtr &msg)
{
	return msg::CreateChatMessage::IsMyMessage(msg)
		|| msg::CreateP2PChatMessage::IsMyMessage(msg)
		|| msg::InviteMessage::IsMyMessage(msg)
		|| msg::InviteResponseMessage::IsMyMessage(msg)
		|| msg::AddPartMessage::IsMyMessage(msg)
		|| msg::RemovePartMessage::IsMyMessage(msg)
		|| msg::TextChatMessage::IsMyMessage(msg)
		|| msg::PartAddedToChatMessage::IsMyMessage(msg)
		|| msg::PartRemovedFromChatMessage::IsMyMessage(msg)
		|| msg::MessageRemovalMessage::IsMyMessage(msg)
		|| msg::ClearHistoryMessage::IsMyMessage(msg)
		;
}
bool IsMessageViewable(const msg::ChatMessage &msg)
{
	auto type = MessageType::undefined;
	if (msg.GetParamI32(attr::MESSAGE_TYPE_paramName, type))
	{
		return type == MessageType::text;
	}
	return false;
}
chain_item GetMsgChainItem(const chat::msg::ChatMessagePtr &msg)
{
	auto msg_id = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	auto prev_id = msg->GetParamStrRef(attr::PREVIOUS_MESSAGE_ID_paramName);
	ChatMessageTimestamp stamp;
	msg->GetParam(attr::TIMESTAMP_paramName, stamp);
	uint64_t bucket_num(std::numeric_limits<uint64_t>::max());
	msg->GetParamI64(attr::BUCKET_paramName, bucket_num);
	assert(bucket_num != std::numeric_limits<uint64_t>::max());
	return {msg_id,prev_id,stamp, bucket_num};
}
void SetUUIDGeneratorFunc(const std::function<std::string()> &gen_func)
{
	s_UUIDGenFunc = gen_func;
}
}