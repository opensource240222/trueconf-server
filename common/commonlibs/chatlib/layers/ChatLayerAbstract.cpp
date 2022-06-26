#include "ChatLayerAbstract.h"
#include "chatlib/msg/ChatMessage.h"
namespace chat
{
void LayerAboveNotifier::ForwardAboveMessage(
	msg::ChatMessagePtr&&msg, CallIDRef sender)
{
	if (!!fireOnMessageRecv_)
		fireOnMessageRecv_(std::move(msg),sender);
}
}