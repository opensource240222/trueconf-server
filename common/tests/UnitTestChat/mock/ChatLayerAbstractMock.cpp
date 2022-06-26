#include "ChatLayerAbstractMock.h"

#include "chatlib/msg/attr.h"
#include "chatlib/msg/chat_messages_construct.h"
#include "chatlib/notify/GlobalChatEvents.h"
#include "chatlib/utils/chat_utils.h"

LayerInterfaceAgregator_Stub::LayerInterfaceAgregator_Stub()
{
	ON_CALL(*leak_control_, MethodForMock())
		.WillByDefault(::testing::Invoke([](){}));
}
void LayerInterfaceAgregator_Stub::ForwardBelowMessage(chat::msg::ChatMessagePtr* msg,
	std::vector<chat::ParticipantDescr>*)
{
	Send(std::move(*msg));
}
void LayerInterfaceAgregator_Stub::OnChatMessageArrived(
	chat::msg::ChatMessagePtr *m, chat::CallIDRef sender)
{
	ForwardAboveMessage(std::move(*m), sender);
}