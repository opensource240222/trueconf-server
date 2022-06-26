#pragma once
#include "chatlib/layers/ChatLayerAbstract.h"
#include "std-generic/cpplib/synchronized.h"
#include <atomic>
#include <queue>
namespace chat_test
{
class SetLayerHelperSimple
{
	void ProcessQueue();
	chat::LayerInterfacePtr	next_;
	struct queue_item
	{
		queue_item(queue_item&&) = default;
		queue_item& operator=(queue_item&&) = default;
		enum class pass_direction
		{
			below,
			above
		} direction_;
		chat::msg::ChatMessagePtr m;
		chat::CallID sender;
		queue_item(
			pass_direction direction,
			chat::msg::ChatMessagePtr&&m, chat::CallID sender = {});
	};
	vs::Synchronized<std::queue<queue_item>> msg_queue_;
	std::atomic<bool> processing_acq_ { false };
protected:
	virtual ~SetLayerHelperSimple() {}
	virtual void OnChatMessageArrived(
		chat::msg::ChatMessagePtr&&msg,
		chat::CallIDRef sender) = 0;
	void Send(chat::msg::ChatMessagePtr&& msg);
public:
	void SetNextLayer(const chat::LayerInterfacePtr &next);
	bool QueueEmpty() const
	{
		return msg_queue_->empty();
	}
	void ShutDown()
	{
		if (next_)
			next_->ShutDown();
	}
};
}
