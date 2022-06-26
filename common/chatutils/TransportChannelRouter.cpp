#include "chatlib/msg/ChatMessage.h"

#include "TransportChannelRouter.h"

#include "std-generic/compat/memory.h"

std::unique_ptr<chat::TransportChannel> TransportChannelRouter::GetChannel(chat::CallIDRef ep)
{
	if (delivery_msg_.find(ep) != delivery_msg_.end())
		return{};
	auto sender_func = [this, self_weak = weak_from_this()](chat::msg::ChatMessagePtr&&m, chat::CallIDRef from, std::vector<chat::CallID> &&eps_to)
	{
		auto self = self_weak.lock();
		if (!self)
			return;
		for (const auto &ep : eps_to)
		{
			Send(vs::make_unique<chat::msg::ChatMessage>(m->GetContainer()), from, ep);
		}
	};
	return vs::make_unique<channel<decltype(sender_func)>>(std::move(sender_func), ep, shared_from_this());
}
void TransportChannelRouter::Send(chat::msg::ChatMessagePtr&&m, chat::CallIDRef from_ep, chat::CallIDRef to_eps)
{
	strand_.post(vs::move_handler(
		[this, m = std::move(m), from_ep = chat::CallID(from_ep), to_eps = chat::CallID(to_eps)] () mutable {
			auto rcv_func_iter = delivery_msg_.find(to_eps);
			if (rcv_func_iter != delivery_msg_.end())
				rcv_func_iter->second(std::move(m), from_ep);
		}
	));
}
void TransportChannelRouter::RegisterMsgReceiver(chat::CallIDRef ep, const chat::OnChatMessageRecvCallBack&recv_func)
{
	delivery_msg_.emplace(ep, recv_func);
}
void TransportChannelRouter::FreeMsgReceiver(chat::CallIDRef ep)
{
	auto iter = delivery_msg_.find(ep);
	if (iter != delivery_msg_.end())
		delivery_msg_.erase(iter);
}