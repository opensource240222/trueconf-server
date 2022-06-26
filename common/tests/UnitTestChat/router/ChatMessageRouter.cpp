#include "ChatMessageRouter.h"
#include "tests/UnitTestChat/SetLayerHelperSimple.h"

#include "chatlib/helpers/AccountInfo.h"
#include "chatlib/helpers/ResolverInterface.h"
#include "chatlib/msg/ChatMessage.h"
#include "chatlib/msg/attr.h"
#include "chatlib/msg/chat_messages_construct.h"
#include "chatlib/storage/ChatStorage.h"
#include "chatlib/utils/msg_utils.h"

#include "std/debuglog/VS_Debug.h"
#include "std-generic/cpplib/VS_Container_io.h"

#include <gtest/gtest.h>

#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_CHATS

namespace
{
	class ChatMessageFakeRouterAdapter : public chat::LayerBelowInterface
	{
		std::string endpoint_;
		std::weak_ptr<ChatMessageFakeRouter> router_;

		void ForwardBelowMessage(chat::msg::ChatMessagePtr&& msg,
			std::vector<chat::ParticipantDescr>&& /*parts*/) override
		{
			dstream4 << "CHT_MSG_LOG: " << msg->GetContainer();
			auto lock = router_.lock();
			if(lock)
				lock->ForwardBelowMessage(std::move(msg), endpoint_);
		}
		void ShutDown() override
		{}
	public:
		ChatMessageFakeRouterAdapter(
			const std::shared_ptr<ChatMessageFakeRouter> &router,
			std::string endpoint)
			: endpoint_(std::move(endpoint))
			, router_(router)
		{}
		~ChatMessageFakeRouterAdapter()
		{}
	};
}
bool is_msg_tp_known(chat::MessageType tp)
{
	return
		tp == chat::MessageType::add_part
		|| tp == chat::MessageType::create_chat
		|| tp == chat::MessageType::invite
		|| tp == chat::MessageType::invite_response
		|| tp == chat::MessageType::text
		|| tp == chat::MessageType::sync
		|| tp == chat::MessageType::delivered;
}

class ChatMessageFakeRouter::OnMsgArivedTo : public chat::LayerInterface
{
	std::shared_ptr<chat::LayerBelowInterface> forward_to_;
	chat::CallID	call_id_;

	void ForwardBelowMessage(chat::msg::ChatMessagePtr&& m,
		std::vector<chat::ParticipantDescr>&& dstParts) override
	{
		if (forward_to_)
			forward_to_->ForwardBelowMessage(std::move(m), std::move(dstParts));
	}
	void ShutDown() override
	{
		if (forward_to_)
			forward_to_->ShutDown();
	}
public:
	OnMsgArivedTo(
		const chat::LayerBelowInterfacePtr &forward_to,
		chat::CallID call_id)
		: forward_to_(forward_to)
		, call_id_(std::move(call_id))
	{}
	~OnMsgArivedTo()
	{
	}
	void SendMessage(chat::msg::ChatMessagePtr&&m, chat::CallIDRef sender)
	{
		dstream4 << "CHT_MSG_LOG: sender = " << sender << '\n'
			<< m->GetContainer();
		ForwardAboveMessage(std::move(m), sender);
	}
};

void ChatMessageFakeRouter::SetChatStorage(
	const chat::ChatStoragePtr& storage)
{
	chat_storage_ = storage;
}
void ChatMessageFakeRouter::SetResolver(const vs::ResolverPtr & resolver)
{
	resolver_ = resolver;
}
void ChatMessageFakeRouter::RegisterNewPart(
	const chat::CallID&id,
	const std::shared_ptr<chat_test::SetLayerHelperSimple> &layer)
{
	auto tmp = std::static_pointer_cast<chat::LayerBelowInterface>(
		std::make_shared<ChatMessageFakeRouterAdapter>(shared_from_this(), id));
	auto part = std::make_shared<OnMsgArivedTo>(tmp,id);
	participants_.emplace(id, part);
	layer->SetNextLayer(part);
}
void ChatMessageFakeRouter::ProcessForwardBelowMessage(
	chat::msg::ChatMessagePtr&&m, chat::CallIDRef from)
{
	chat::ChatID chat_id = m->GetParamStr(chat::attr::CHAT_ID_paramName);
	auto msgType = chat::MessageType::undefined;
	if (!m->GetParamI32(chat::attr::MESSAGE_TYPE_paramName, msgType)
		|| !is_msg_tp_known(msgType))
		return;
	auto storage = chat_storage_.lock();

	auto chat_ctx = storage
		? storage->GetGlobalContext(chat_id)
		: chat::GlobalContext();
	auto parts = !chat_ctx.chatId.empty()
		? chat_ctx.participants
		: decltype(chat_ctx.participants)();
	auto to = m->GetParamStrRef(chat::attr::DST_ENDPOINT_paramName);
	if (to.empty())
		to = m->GetParamStrRef(chat::attr::DST_CALLID_paramName);
	if (!to.empty())
	{
		auto resolver = resolver_.lock();
		assert(resolver);
		if(resolver)
		{
			resolver->GetCallIDByAlias(to,
				[
					this,
					alias = chat::CallID(to),
					from = chat::CallID(from),
					resolver,
					&m
				](bool res, vs::CallIDRef call_id)
			{
				ASSERT_TRUE(res);
				if (alias == call_id) //send all relations
				{
					resolver->Resolve(call_id,
						[
							this,
							&m,
							from = std::move(from)
						](bool res, vs::ResolverInterface::ResolveInfo &&info)
					{
						ASSERT_TRUE(res);
						for (const auto &i : info.epList)
						{
							dstream4 << "ChatMessageFakeRouter::ProcessForwardBelowMessage MainMessageRecv;"
								<< " sender = " << from << "; to="<<i;
							auto part = participants_.find(i);
							part->second->SendMessage(
								vs::make_unique<chat::msg::ChatMessage>(
									m->GetContainer()),
								from);
						}
					});
				}
				else
				{
					auto part = participants_.find(alias);
					ASSERT_NE(part, participants_.end());
					part->second->SendMessage(std::move(m), from);
				}
			});
		}
		return;
	}
	if (msgType == chat::MessageType::invite
		|| msgType == chat::MessageType::text
		|| msgType == chat::MessageType::add_part)
	{
		//send to all;

		for (const auto &to : parts)
		{
			if (to.partId == from)
				continue;
			auto part = participants_.find(to.partId);
			ASSERT_TRUE(part != participants_.end());
			part->second->SendMessage(
				vs::make_unique<chat::msg::ChatMessage>(m->GetContainer()),
				from);
		}
	}
	if (msgType == chat::MessageType::invite_response
		|| msgType == chat::MessageType::invite)
	{
		//send only to dst
		auto to = chat::msg::GetParamStrFromMsgContent(m, chat::msg::nameKeyName);
		auto part = participants_.find(to);
		bool is_part_in_chat = chat::FindPartDescrById(
			chat_ctx.participants.begin(),
			chat_ctx.participants.end(),
			to) != chat_ctx.participants.end();
		ASSERT_FALSE(part == participants_.end()
			|| (msgType == chat::MessageType::invite_response
				&& !is_part_in_chat));
		part->second->SendMessage(std::move(m),from);
	}
}
void ChatMessageFakeRouter::ForwardBelowMessage(
	chat::msg::ChatMessagePtr&&m,
	chat::CallIDRef sender)
{
	/**
		Only invite to chat and response for invite
	*/
	msg_queue_.emplace(std::move(m), std::string(sender));
	if (msg_queue_.size() == 1)
		do
		{
			auto &item = msg_queue_.front();
			ProcessForwardBelowMessage(std::move(item.first), item.second);
			msg_queue_.pop();
		} while (!msg_queue_.empty());
}

