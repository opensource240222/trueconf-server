#pragma once
#include "chatlib/interface/TransportChannel.h"
#include "chatlib/msg/ChatMessage.h"
#include "std-generic/compat/functional.h"
#include "std-generic/compat/map.h"
#include "std-generic/compat/memory.h"
#include "std-generic/cpplib/macro_utils.h"

#include <memory>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

class TransportChannelRouter :
	public vs::enable_shared_from_this<TransportChannelRouter>
{
public:
	TransportChannelRouter(boost::asio::io_service& ios)
		: strand_(ios) {}

	void RegisterMsgReceiver(
		chat::CallIDRef ep,
		const chat::OnChatMessageRecvCallBack & recv_func);
	void FreeMsgReceiver(chat::CallIDRef ep);
	std::unique_ptr<chat::TransportChannel> GetChannel(chat::CallIDRef ep);
private:
	template<class SendFunc>
	class channel : public chat::TransportChannel
	{
		std::weak_ptr<TransportChannelRouter> router_;
		chat::CallID ep_;
		SendFunc send_func_;
	public:
		channel(
			SendFunc&& sf,
			chat::CallIDRef ep,
			const std::shared_ptr<TransportChannelRouter> &router)
			:router_(router)
			, ep_(ep)
			, send_func_(std::move(sf))
		{
			router->RegisterMsgReceiver(
				ep_,
				[this](chat::msg::ChatMessagePtr &&m, chat::CallIDRef sender)
				{
					ForwardAboveMessage(std::move(m), sender);
				}
			);
		}
		~channel()
		{
			auto router = router_.lock();
			if (router)
				router->FreeMsgReceiver(ep_);
		}
		void Send(chat::msg::ChatMessagePtr && m, std::vector<chat::CallID> &&eps) override
		{
			send_func_(std::move(m), ep_, std::move(eps));
		}
	};
	void Send(chat::msg::ChatMessagePtr &&m, chat::CallIDRef from_ep, chat::CallIDRef to_eps);

	vs::map<chat::CallID, chat::OnChatMessageRecvCallBack, vs::str_less> delivery_msg_;
	struct queue_item
	{
		VS_FORWARDING_CTOR3(queue_item, msg_, from_, to_) {}
		queue_item(queue_item&) = delete;
		queue_item(queue_item&&) = default;
		queue_item& operator=(queue_item&&) = default;
		chat::msg::ChatMessagePtr msg_;
		chat::CallID from_;
		chat::CallID to_;
	};
	std::queue<queue_item> msg_queue_;
	boost::asio::io_service::strand strand_;
};
