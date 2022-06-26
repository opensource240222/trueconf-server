#pragma once

#include "chatlib/callbacks.h"

#include<vector>
class VS_Container;
namespace chat
{
ChatID GenerateChatID();
ChatID GetP2PChatID(CallIDRef user1, CallIDRef user2);
// check from and to attribute
bool IsChatP2P(const msg::ChatMessagePtr& msg);
std::string GenerateSalt(); //4 bytes in hex
uint32_t Generate32();
bool IsMessageStorable(const msg::ChatMessagePtr &msg);
bool IsMessageViewable(const msg::ChatMessage &msg);
chain_item GetMsgChainItem(const chat::msg::ChatMessagePtr &msg);
std::string GenerateUUID();
inline uint64_t timestamp_to_uint(ChatMessageTimestamp tp)
{
	// convert to milliseconds
	return tp.time_since_epoch().count();
}
inline ChatMessageTimestamp uint_to_timestamp(uint64_t t)
{
	return ChatMessageTimestamp(ChatMessageTimestamp::duration(t));
}
inline std::string timestamp_to_string(ChatMessageTimestamp tp)
{
	return std::to_string(tp.time_since_epoch().count());
}
// FIXME: put function to call id utils;
inline CallIDRef GetCallIDByEpName(CallIDRef ep)
{
	return ep.substr(0, ep.find(CALL_ID_SEPARATOR));
}
void SetUUIDGeneratorFunc(const std::function<std::string()> &gen_func);
}