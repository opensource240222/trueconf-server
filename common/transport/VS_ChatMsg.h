#pragma once

class VS_RoamingChatMsg
{
public:
	VS_SimpleStr to_callId;
	VS_Container* cnt;

	VS_RoamingChatMsg(): cnt(0)
	{	}
};
typedef std::vector<VS_RoamingChatMsg> VS_ChatMsgs;