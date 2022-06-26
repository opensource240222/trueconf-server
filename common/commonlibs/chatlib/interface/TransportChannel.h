#pragma once
#include "chatlib/chat_defs.h"
#include "chatlib/layers/ChatLayerAbstract.h"
#include <vector>
namespace chat
{
class TransportChannel: public LayerAboveNotifier
{
	public:
		virtual void Send(
			msg::ChatMessagePtr &&m,std::vector<CallID> &&endpoints) = 0;
};
}
