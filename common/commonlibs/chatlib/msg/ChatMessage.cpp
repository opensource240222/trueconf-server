#include "ChatMessage.h"
#include "attr.h"

#include "std-generic/compat/memory.h"
#include <chrono>
#include <iostream>
#include <sstream>

namespace chat
{
namespace msg
{
ChatMessage::ChatMessage()
{
}
ChatMessage::ChatMessage(const VS_Container & cnt)
	: m_cnt(cnt)
{
}
bool ChatMessage::SetParam(string_view param, const std::string &val)
{
	return m_cnt.AddValue(param, val);
}
bool ChatMessage::GetParam(const char *param, std::string &out) const
{
	const char * str = m_cnt.GetStrValueRef(param);
	if (!str)
		return false;
	out = str;
	return true;
}
bool ChatMessage::GetParam(const char *param, ChatMessageTimestamp &out) const
{
	std::chrono::system_clock::time_point point;
	if (!m_cnt.GetValue(param, point))
		return false;
	auto durr = std::chrono::duration_cast<
		ChatMessageTimestamp::duration>(
			point.time_since_epoch());
	out = ChatMessageTimestamp(durr);
	return true;
}
cstring_view ChatMessage::GetParamStrRef(const char *param) const
{
	return m_cnt.GetStrValueView(param);
}
std::string ChatMessage::GetParamStr(const char* param) const
{
	return std::string(GetParamStrRef(param));
}
const VS_Container &ChatMessage::GetContainer() const
{
	return m_cnt;
}
void ChatMessage::OnChainUpdateByMsg(
	cb::ProcessingResult r,
	cb::MsgIdAndOrderInChain&& updatedOrder)
{
	auto currentMsgId = GetParamStrRef(attr::MESSAGE_ID_paramName);
	if (currentMsgId.empty())
	{
		r = cb::ProcessingResult::failed;
	}
	m_chainUpdateRes.res = r;
	m_chainUpdateRes.updatedOrder = std::move(updatedOrder);
	m_onChainUpdateByMsg(r, currentMsgId, m_chainUpdateRes.updatedOrder);
	m_onChainUpdateByMsg.disconnect_all_slots();
}
void ChatMessage::AddOnChainUpdateByMsgCallBack(
	const cb::OnChainUpdateByMsgCb &cb)
{
	if (!cb)
		return;
	if (cb::ProcessingResult::undef
		!= m_chainUpdateRes.res)
	{
		auto msgId = GetParamStrRef(attr::MESSAGE_ID_paramName);
		cb(m_chainUpdateRes.res, msgId,
			m_chainUpdateRes.updatedOrder);
		return;
	}
	m_onChainUpdateByMsg.connect(cb);
}
void ChatMessage::OnMsgIsStored(cb::ProcessingResult res)
{
	auto chatId = GetParamStrRef(attr::CHAT_ID_paramName);
	auto msgId = GetParamStrRef(attr::MESSAGE_ID_paramName);
	if (chatId.empty()
		|| msgId.empty())
	{
		res = cb::ProcessingResult::failed;
	}
	m_onMsgStoredRes = res;
	m_onMsgStored(res, chatId, msgId);
	m_onMsgStored.disconnect_all_slots();
}
void ChatMessage::AddOnMsgIsStoredCallBack(
	const cb::OnMessageIsStoredCallBack &cb)
{
	if (!cb)
		return;
	if (cb::ProcessingResult::undef
		!= m_onMsgStoredRes)
	{
		auto chatId = GetParamStrRef(attr::CHAT_ID_paramName);
		auto msgId = GetParamStrRef(attr::MESSAGE_ID_paramName);
		cb(m_onMsgStoredRes, chatId, msgId);
		return;
	}
	m_onMsgStored.connect(cb);
}
size_t ChatMessage::count() const
{
	m_cnt.Reset();
	size_t res(0);
	while (m_cnt.Next())
		res++;
	return res;
}
ChatMessage::~ChatMessage()
{
	OnChainUpdateByMsg(cb::ProcessingResult::failed, {});
	OnMsgIsStored(cb::ProcessingResult::failed);
}
ChatMessageKeeper::ChatMessageKeeper(ChatMessagePtr&&msg)
	: m_message(std::move(msg))
{}
ChatMessageKeeper::ChatMessageKeeper()
	: m_message(vs::make_unique<ChatMessage>())
{}
ChatMessagePtr ChatMessageKeeper::AcquireMessage()
{
	return std::move(m_message);
}
}
}