#pragma once
#include "chatlib/chat_defs.h"
#include "chatlib/notify/GlobalChatEvents.h"

namespace chat
{
namespace notify
{
class ChatEventsFuncs : public ChatEventsSubscription,
						public ChatEventsNotifier
{
	mutable vs::Signal<
		void(
			const GlobalContext&)> m_onChatCreate;
	mutable vs::Signal<
		void(
			ChatIDRef,
			CallIDRef,
			ChatMessageIDRef,
			CallIDRef)> m_onPartAdd;
	mutable vs::Signal<
		void(
			ChatIDRef,
			CallIDRef,
			ChatMessageIDRef)> m_onPartRemoved;
	mutable vs::Signal<
		void(
			const PersonalContext&
			)> m_onPersonalContextUpdated;
	mutable vs::Signal<void()> m_onPersonalContextsFetched;
	mutable vs::Signal<
		void(
			const GlobalContext&
			)> m_onGlobalContextUpdated;
	mutable vs::Signal<
		void(
			const msg::ChatMessagePtr&,
			string_view)> m_onErrMessageMailformed;
	mutable vs::Signal<
		void(
			string_view,
			string_view)> m_onError;
	mutable vs::Signal<
		void(const msg::ChatMessagePtr&)> m_onChainUpdated;
	mutable vs::Signal<
		void(
			ChatMessageIDRef,
			CallIDRef)> m_onMsgDelivered;
	mutable vs::Signal<
		void(
			ChatIDRef,
			ChatMessageIDRef,
			CallIDRef)> m_onMsgRead;
public:
	vs::SubscribeConnection
		SubscribeToChatCreate(const OnChatCreateCallBack&) const override;
	vs::SubscribeConnection
		SubscribeToPartAdded(const OnPartAddedCallBack&) const override;
	vs::SubscribeConnection
		SubscribeToPartRemoved(const OnPartRemoveCallBack&) const override;
	vs::SubscribeConnection
		SubscribeToPersonalContextUpdated(
			const OnPersonalContextUpdatedCallBack&) const override;
	vs::SubscribeConnection
		SubscribeToPersonalContextsFetched(
			const OnPersonalContextsFetchedCallBack&) const override;
	vs::SubscribeConnection
		SubscribeToGlobalContextUpdated(
			const OnGlobalContextUpdatedCallBack&) const override;
	vs::SubscribeConnection
		SubscribeToErrMessageMalformed(
			const OnErrMessageMalformedCallBack&) const override;
	vs::SubscribeConnection
		SubscribeToError(const OnErrorCallBack&) const override;
	vs::SubscribeConnection
		SubscribeToChainUpdated(const OnChainUpdatedCallBack&) const override;
	vs::SubscribeConnection
		SubscribeToOnMsgDelivered(const OnMsgDeliveredCallBack&) const override;
	vs::SubscribeConnection
		SubscribeToMsgRead(const OnMsgReadCallBack &) const override;

	void OnChatCreated(
		const GlobalContext& chat_info) const override;
	void OnPartAdded(
		ChatIDRef chat_id,
		CallIDRef call_id,
		ChatMessageIDRef msg_id,
		CallIDRef msg_author) const override;
	void OnPartRemoved(
		ChatIDRef chat_id,
		CallIDRef call_id,
		ChatMessageIDRef msg_id) const override;
	void OnPersonalContextUpdated(const PersonalContext& ctx) const override;
	void OnPersonalContextsFetched() const override;
	void OnGlobalContextUpdated(const GlobalContext& ctx) const override;
	void OnErrMessageMalformed(
		const msg::ChatMessagePtr&,
		string_view) const override;
	void OnChainUpdated(
		const msg::ChatMessagePtr&)  const override;
	void OnMsgDelivered(
		ChatMessageIDRef msg_id,
		CallIDRef call_id) const override;
	void OnMsgRead(
		ChatIDRef chat_id,
		ChatMessageIDRef msg_id,
		CallIDRef call_id) const override;
	void OnError(
		string_view src,
		string_view descr) const override;
};
}
}