#include "SetLayerHelperSimple.h"
#include "chatlib/msg/ChatMessage.h"
namespace chat_test
{
SetLayerHelperSimple::queue_item::queue_item(pass_direction direction,
	chat::msg::ChatMessagePtr&&m, chat::CallID sender)
	: direction_(direction)
	, m(std::move(m))
	, sender(std::move(sender)){}
void SetLayerHelperSimple::Send(chat::msg::ChatMessagePtr&& msg)
{
	if (next_)
		msg_queue_->emplace(queue_item::pass_direction::below, std::move(msg));
	ProcessQueue();
}
void SetLayerHelperSimple::ProcessQueue()
{
	while (!msg_queue_->empty())
	{
		bool expect = false;
		if (!processing_acq_.compare_exchange_strong(expect, true, std::memory_order_acq_rel))
		{
			return;
		}

		auto& item = msg_queue_->front();
		switch (item.direction_)
		{
		case queue_item::pass_direction::below:
			next_->ForwardBelowMessage(std::move(item.m), {});
			break;
		case queue_item::pass_direction::above:
			OnChatMessageArrived(std::move(item.m), item.sender);
			break;
		}
		msg_queue_->pop();
		processing_acq_.store(false, std::memory_order_release);
	}
}
void SetLayerHelperSimple::SetNextLayer(const chat::LayerInterfacePtr &next)
{
	next_ = next;
	next_->SetOnMessageRecvCallBack([this](chat::msg::ChatMessagePtr&&msg,chat::CallIDRef sender)
	{
		msg_queue_->emplace(
			queue_item::pass_direction::above,
			std::move(msg),
			chat::CallID(sender));
		ProcessQueue();
	});
}
}