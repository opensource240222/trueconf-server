#pragma once
#include "chatlib/callbacks.h"
#include "chatlib/chat_defs.h"
namespace chat
{
class SyncChatInterface
{
public:
	virtual ~SyncChatInterface() {}
	virtual void SyncChatTail(ChatIDRef chat_id, uint32_t tail, CallIDRef with_who,
		const cb::ChatMsgIDChainCallBack &cb) = 0;
};
}