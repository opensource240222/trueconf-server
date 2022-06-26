#include "ChatEventsFuncs.h"

namespace chat
{
namespace notify
{
vs::SubscribeConnection
ChatEventsFuncs::SubscribeToChatCreate(const OnChatCreateCallBack&cb) const
{
	return m_onChatCreate.connect(cb);
}
vs::SubscribeConnection
ChatEventsFuncs::SubscribeToPartAdded(const OnPartAddedCallBack &cb) const
{
	return m_onPartAdd.connect(cb);
}
vs::SubscribeConnection
ChatEventsFuncs::SubscribeToPartRemoved(const OnPartRemoveCallBack& cb) const
{
	return m_onPartRemoved.connect(cb);
}
vs::SubscribeConnection
ChatEventsFuncs::SubscribeToPersonalContextUpdated(const OnPersonalContextUpdatedCallBack& cb) const
{
	return m_onPersonalContextUpdated.connect(cb);
}
vs::SubscribeConnection
ChatEventsFuncs::SubscribeToPersonalContextsFetched(const OnPersonalContextsFetchedCallBack& cb) const
{
	return m_onPersonalContextsFetched.connect(cb);
}
vs::SubscribeConnection
ChatEventsFuncs::SubscribeToGlobalContextUpdated(const OnGlobalContextUpdatedCallBack& cb) const
{
	return m_onGlobalContextUpdated.connect(cb);
}
vs::SubscribeConnection
ChatEventsFuncs::SubscribeToErrMessageMalformed(
	const OnErrMessageMalformedCallBack &cb) const
{
	return m_onErrMessageMailformed.connect(cb);
}
vs::SubscribeConnection
ChatEventsFuncs::SubscribeToError(const OnErrorCallBack& cb) const
{
	return m_onError.connect(cb);
}
vs::SubscribeConnection
ChatEventsFuncs::SubscribeToChainUpdated(
	const OnChainUpdatedCallBack &cb) const
{
	return m_onChainUpdated.connect(cb);
}
vs::SubscribeConnection
ChatEventsFuncs::SubscribeToOnMsgDelivered(
	const OnMsgDeliveredCallBack &cb) const
{
	return m_onMsgDelivered.connect(cb);
}
vs::SubscribeConnection
ChatEventsFuncs::SubscribeToMsgRead(const OnMsgReadCallBack &cb) const
{
	return m_onMsgRead.connect(cb);
}
void ChatEventsFuncs::OnChatCreated(
	const GlobalContext& chat_info) const
{
	m_onChatCreate(chat_info);
}

void ChatEventsFuncs::OnPartAdded(
	ChatIDRef chat_id,
	CallIDRef call_id,
	ChatMessageIDRef msg_id,
	CallIDRef msg_author) const
{
	m_onPartAdd(chat_id, call_id, msg_id, msg_author);
}
void ChatEventsFuncs::OnPartRemoved(
	ChatIDRef chat_id,
	CallIDRef call_id,
	ChatMessageIDRef msg_id) const
{
	m_onPartRemoved(chat_id, call_id, msg_id);
}
void ChatEventsFuncs::OnPersonalContextUpdated(
	const PersonalContext& ctx) const
{
	m_onPersonalContextUpdated(ctx);
}
void ChatEventsFuncs::OnPersonalContextsFetched() const
{
	m_onPersonalContextsFetched();
}
void ChatEventsFuncs::OnGlobalContextUpdated(
	const GlobalContext& ctx) const
{
	m_onGlobalContextUpdated(ctx);
}
void ChatEventsFuncs::OnErrMessageMalformed(
	const msg::ChatMessagePtr&m,
	string_view publisher) const
{
	m_onErrMessageMailformed(m, publisher);
}
void ChatEventsFuncs::OnChainUpdated(
	const msg::ChatMessagePtr& msg) const
{
	m_onChainUpdated(msg);
}
void ChatEventsFuncs::OnMsgDelivered(
	ChatMessageIDRef msg_id,
	CallIDRef call_id) const
{
	m_onMsgDelivered(msg_id, call_id);
}
void ChatEventsFuncs::OnMsgRead(
	ChatIDRef chat_id,
	ChatMessageIDRef msg_id,
	CallIDRef call_id) const
{
	m_onMsgRead(chat_id, msg_id, call_id);
}
void ChatEventsFuncs::OnError(
	string_view src,
	string_view descr) const
{
	m_onError(src, descr);
}
}
}