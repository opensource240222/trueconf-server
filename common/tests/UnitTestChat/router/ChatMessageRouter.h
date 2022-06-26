#pragma once
#include "tests/UnitTestChat/SetLayerHelperSimple.h"
#include "std-generic/compat/map.h"

class ChatMessageFakeRouter :
	public std::enable_shared_from_this<ChatMessageFakeRouter>
{
	class OnMsgArivedTo;
	vs::map<
		chat::CallID,
		std::shared_ptr<OnMsgArivedTo>>	participants_;
	std::weak_ptr<chat::ChatStorage> chat_storage_;
	std::weak_ptr<vs::ResolverInterface> resolver_;
	std::queue<std::pair<chat::msg::ChatMessagePtr,chat::CallID>> msg_queue_; // first - message, second - sender

	void ProcessForwardBelowMessage(
		chat::msg::ChatMessagePtr&&m,
		chat::CallIDRef from);
public:
	void RegisterNewPart(
		const chat::CallID&id,
		const std::shared_ptr<chat_test::SetLayerHelperSimple> &layer);
	void SetChatStorage(const chat::ChatStoragePtr& storage);
	void SetResolver(const vs::ResolverPtr & resolver);
	void ForwardBelowMessage(
		chat::msg::ChatMessagePtr&&m,
		chat::CallIDRef from);

protected:
	ChatMessageFakeRouter() = default;
};