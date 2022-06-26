#pragma once
#include "chatlib/notify/notify_cb.h"
namespace chat
{
//interface for subscribe for events
namespace notify
{
class ChatEventsSubscription
{
public:
	virtual ~ChatEventsSubscription() = default;

	virtual vs::SubscribeConnection
		SubscribeToChatCreate(const OnChatCreateCallBack&) const = 0;
	virtual vs::SubscribeConnection
		SubscribeToPartAdded(const OnPartAddedCallBack&) const = 0;
	virtual vs::SubscribeConnection
		SubscribeToPartRemoved(const OnPartRemoveCallBack&) const = 0;
	virtual vs::SubscribeConnection
		SubscribeToPersonalContextUpdated(const OnPersonalContextUpdatedCallBack&) const = 0;
	virtual vs::SubscribeConnection
		SubscribeToPersonalContextsFetched(const OnPersonalContextsFetchedCallBack&) const = 0;
	virtual vs::SubscribeConnection
		SubscribeToGlobalContextUpdated(const OnGlobalContextUpdatedCallBack&) const = 0;
	virtual vs::SubscribeConnection
		SubscribeToErrMessageMalformed(const OnErrMessageMalformedCallBack&) const = 0;
	virtual vs::SubscribeConnection
		SubscribeToError(const OnErrorCallBack&) const = 0;
	virtual vs::SubscribeConnection
		SubscribeToChainUpdated(const OnChainUpdatedCallBack&) const = 0;
	virtual vs::SubscribeConnection
		SubscribeToOnMsgDelivered(const OnMsgDeliveredCallBack&) const = 0;
	virtual vs::SubscribeConnection
		SubscribeToMsgRead(const OnMsgReadCallBack &) const = 0;
};

// interface for notify about events
class ChatEventsNotifier
{
public:
	virtual ~ChatEventsNotifier() {}
	virtual void OnChatCreated(
		const GlobalContext&) const = 0;
	virtual void OnPartAdded(
		ChatIDRef chat_id,
		CallIDRef call_id,
		ChatMessageIDRef msg_id,
		CallIDRef msg_author) const = 0;
	virtual void OnPartRemoved(
		ChatIDRef chat_id,
		CallIDRef call_id,
		ChatMessageIDRef msg_id) const = 0;
	virtual void OnPersonalContextUpdated(
		const PersonalContext&) const = 0;
	virtual void OnPersonalContextsFetched() const = 0;
	virtual void OnGlobalContextUpdated(
		const GlobalContext&) const = 0;
	virtual void OnChainUpdated(
		const msg::ChatMessagePtr& msg)  const = 0;
	virtual void OnMsgDelivered(
		ChatMessageIDRef msg_id,
		CallIDRef call_id) const = 0;
	virtual void OnMsgRead(
		ChatIDRef chat_id,
		ChatMessageIDRef msg_id,
		CallIDRef call_id) const = 0;
	virtual void OnErrMessageMalformed(
		const msg::ChatMessagePtr&,
		string_view src) const = 0;
	virtual void OnError(string_view src, string_view descr) const = 0;
};
}
}