#pragma once
#include "AppLayer.h"

#include "chatlib/helpers/AccountInfo.h"
#include "chatlib/helpers/ExternalComponentsInterface.h"
#include "chatlib/interface/GlobalConfigInterface.h"
#include "chatlib/interface/SyncChatInterface.h"
#include "chatlib/msg/ChatMessage.h"
#include "chatlib/msg/attr.h"
#include "chatlib/msg/chat_messages_construct.h"
#include "chatlib/notify/GlobalChatEvents.h"
#include "chatlib/utils/chat_utils.h"
#include "chatlib/utils/msg_utils.h"

#include "std-generic/cpplib/VS_RemoveTranscoderID.h"
#include "chatlib/log/chatlog.h"

#include <cassert>

#define DEBUG_CURRENT_MODULE VS_DM_CHATS

namespace chat
{
namespace detail
{
bool operator<(const detail::WaitingInviteResponseSt& lhs, const detail::WaitingInviteResponseRef& rhs)
{
	return std::tie(lhs.chatId, lhs.chatMessageId) < std::tie(rhs.chatId, rhs.chatMessageId);
}
bool operator<(const detail::WaitingInviteResponseRef& lhs, const detail::WaitingInviteResponseSt& rhs)
{
	return std::tie(lhs.chatId, lhs.chatMessageId) < std::tie(rhs.chatId, rhs.chatMessageId);
}
bool operator<(const detail::ChatIdCallIdSt& lhs, const detail::ChatIdCallIdRef& rhs)
{
	return std::tie(lhs.chatId, lhs.callId) < std::tie(rhs.chatId, rhs.callId);
}
bool operator<(const detail::ChatIdCallIdRef& lhs, const detail::ChatIdCallIdSt& rhs)
{
	return std::tie(lhs.chatId, lhs.callId) < std::tie(rhs.chatId, rhs.callId);
}
}
template<typename LayersTraits, typename MainStoragePtr>
template<typename ...Args>
AppLayer<LayersTraits, MainStoragePtr>::AppLayer(const GlobalConfigPtr& cfg,
	const MainStoragePtr& storageLayer,
	Args&&... args)
	: LayerHelper<LayersTraits>(std::forward<Args>(args)...)
	, m_currentAccount(cfg->GetCurrentAccount())
	, m_storageLayer(storageLayer)
	, m_resolver(cfg->GetResolver())
	, m_cfg(cfg)
{
	m_recvMsgHandlers.emplace(
		MessageType::invite,
		[this](msg::ChatMessagePtr&&msg)
	{
		OnInviteRecv(std::move(msg));
	});
	m_recvMsgHandlers.emplace(
		MessageType::invite_response,
		[this](msg::ChatMessagePtr&&msg)
	{
		OnInviteRespRecv(std::move(msg));
	});
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::SetOnInviteCallBack(const cb::OnInviteArrivedCallBack&cb)
{
	m_fireOnInvite = cb;
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::SetOnInviteResponseCallBack(
	const cb::OnInviteResponseArrivedCallBack &cb)
{
	m_fireOnInviteRespose = cb;
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::GetChatTailFrom(
	ChatIDRef id,
	uint32_t tailLen,
	CallIDRef from,
	const cb::ChatMsgIDChainCallBack &cb)
{

	auto cfg = GetCfg();
	if (!cfg)
	{
		if (cb)
			cb(cb::ErrorCode::failed, {});
	}
	else
	{
		auto syncChat = cfg->GetSyncChat();
		if (!syncChat)
		{
			if (cb)
				cb(cb::ErrorCode::failed, {});
			return;
		}
		syncChat->SyncChatTail(id,
			tailLen, from, cb);
	}
}
template<typename LayersTraits, typename MainStoragePtr>
PersonalContextList AppLayer<LayersTraits, MainStoragePtr>::GetMyPersonalContexts(uint32_t pageSize, uint32_t pageNum)
{
	return GetCfg()->GetChatStorage()->GetUserPersonalContexts(m_currentAccount->GetCallID(), pageSize, pageNum);
}
template<typename LayersTraits, typename MainStoragePtr>
GlobalContext AppLayer<LayersTraits, MainStoragePtr>::GetGlobalContext(const ChatID& id)
{
	return GetCfg()->GetChatStorage()->GetGlobalContext(id);
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::SyncSystemChat()
{
	auto sysChatId = GetP2PChatID(m_currentAccount->GetCallID(), m_currentAccount->GetBS());
	GetChatTailFrom(sysChatId, GetCfg()->GetTailLength(), m_currentAccount->GetBS(), {});
	m_storageLayer->FetchAllUserPersonalContexts(
		[cfg =GetCfg()]()
		{
			cfg->GetEventsNotifier()->OnPersonalContextsFetched();
		});
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::SyncMyChat(chat::ChatIDRef chatId)
{
	// FIXME: check that chat is mine
	GetChatTailFrom(chatId, GetCfg()->GetTailLength(), m_currentAccount->GetBS(), {});
	m_storageLayer->RequestGlobalContext(chatId, m_currentAccount->GetBS(),
		[cfg = GetCfg()](GlobalContext&& gCtx) {
			cfg->GetEventsNotifier()->OnGlobalContextUpdated(gCtx);
		}
	);
}
template<typename LayersTraits, typename MainStoragePtr>
msg::ChatMessagePtr
AppLayer<LayersTraits, MainStoragePtr>::GetMessage(const ChatMessageID& msgId) const
{
	return GetCfg()->GetChatStorage()->GetMessage(msgId, m_currentAccount->GetCallID());
}
template<typename LayersTraits, typename MainStoragePtr>
std::vector<MessageWithOrder>
AppLayer<LayersTraits, MainStoragePtr>::GetMessages(
	const ChatID& id,
	const ChatMessageID& msgBefore,
	size_t count) const
{
	return GetCfg()->GetChatStorage()->GetMessages(id, m_currentAccount->GetCallID(), msgBefore, count);
}
template<typename LayersTraits, typename MainStoragePtr>
std::vector<ChatMessageID>
AppLayer<LayersTraits, MainStoragePtr>::GetUndeliveredMessagesByChatId(const ChatID& chatId) const
{
	return GetCfg()->GetChatStorage()->GetUndeliveredMessagesByChatId(chatId);
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::CreateChat(
	ChatType tp,
	std::string title,
	const cb::CreateChatCallBack &resCb)
{
	this->PostCall([this, tp, title = std::move(title), resCb]()
	{
		ChatID chatId = GenerateChatID();
		if (!chatId.empty())
		{
			auto addPartAfterCreate = [chatId, resCb, this]()
			{
				// add current account
				// add bs
				//run callback
				AddParticipant(chatId, m_currentAccount->GetCallID(), [resCb, chatId](
					cb::ProcessingResult r,
					ChatMessageIDRef,
					const cb::MsgIdAndOrderInChain&)
				{
					if (resCb)
					{
						resCb(r == cb::ProcessingResult::ok
								? cb::CreateChatResult::ok
								: cb::CreateChatResult::failed,
							chatId);
					}
				});
			};
			using namespace msg;
			CreateChatMessage msg(
				chatId, current_chat_version,
				tp, title, m_currentAccount->GetCallID(),
				m_currentAccount->GetExactCallID(),
				m_currentAccount->GetCallIDType(),
				[
					chatId,
					addPartAfterCreate = std::move(addPartAfterCreate),
					resCb
				](
					cb::ProcessingResult r,
					ChatMessageIDRef,
					const cb::MsgIdAndOrderInChain&)
			{
				if (cb::ProcessingResult::ok != r)
				{
					if (resCb)
						resCb(cb::CreateChatResult::failed, {});
				}
				else
					addPartAfterCreate();
			});
			this->Send(msg.AcquireMessage(), {});
		}
		else
		{
			if (resCb)
				resCb(cb::CreateChatResult::failed, ChatID());
		}
	});
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::InviteToChat(
	const ChatID& chatId,
	CallID callId,
	const cb::OnChainUpdateByMsgCb &cb)
{
	auto ctx = GetCfg()->GetChatStorage()->GetGlobalContext(chatId);
	this->PostCall([
		this,
		callId = std::move(callId),
		cb,
		ctx = std::move(ctx)]()
	{
		if (ctx.chatId.empty())
		{
			if (cb)
				cb(cb::ProcessingResult::failed, {}, {});
			return;
		}
		msg::InviteMessage msg(
			ctx.chatId,
			ctx.type,
			ctx.title,
			ctx.version,
			callId,
			m_currentAccount->GetCallID(),
			m_currentAccount->GetExactCallID(),
			m_currentAccount->GetCallIDType(),
			cb);
		auto m = msg.AcquireMessage();
		auto msgId = m->GetParamStrRef(attr::MESSAGE_ID_paramName);
		m_waitingInviteResponse.emplace(ctx.chatId, msgId);
		this->Send(std::move(m), {});
	});
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::AddParticipant(
	const ChatID& chatId,
	CallID callId,
	const cb::OnChainUpdateByMsgCb& cb)
{
	/**
		1. Get chat info
		2. resolve callId
		3. Add callId and bs for callId
	*/
	auto ctx = GetCfg()->GetChatStorage()->GetGlobalContext(chatId);
	if (ctx.chatId.empty()
		|| ctx.type == ChatType::p2p
		|| std::find_if(
			ctx.participants.begin(),
			ctx.participants.end(),
			[&](const ParticipantDescr& part){return part.partId == callId;})
		!= ctx.participants.end())
	{
		if (cb)
		{
			cb(cb::ProcessingResult::failed, {}, {});
		}
		return;
	}
	// todo(kt): debug if needed here
	auto hunt = VS_RemoveTranscoderID_sv(callId);
	m_resolver->Resolve(
		hunt,
		[
			this,
			cb,
			ctx = std::move(ctx)
		]
	(bool res, vs::ResolverInterface::ResolveInfo partInfo)
	{
		if (!res)
		{
			if (cb)
			{
				cb(cb::ProcessingResult::failed, {}, {});
			}
			return;
		}
		bool addBS = false;
		if(partInfo.type == vs::CallIDType::client
			&& std::find_if(
				ctx.participants.begin(),
				ctx.participants.end(),
				[&](const ParticipantDescr& part)
					{
						return part.partId == partInfo.bsInfo;
					}) == ctx.participants.end())
		{
			addBS = true;
		}
		if (addBS)
			AddWithBS(ctx, partInfo, cb);
		else
			AddUserOnly(ctx, partInfo, cb);
	});
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::RemoveParticipant(
	const ChatID& chatId,
	CallID callId,
	const cb::OnChainUpdateByMsgCb& cb)
{
	/**
		Remove only callid
		TODO: BS removes itself if there isn't no one user from this bs in chat
	*/
	auto ctx = GetCfg()->GetChatStorage()->GetGlobalContext(chatId);
	decltype(ctx.participants.begin()) partIter;
	if(ctx.chatId.empty()
		|| ctx.type == ChatType::p2p
		|| (partIter = std::find_if(
			ctx.participants.begin(),
			ctx.participants.end(),
			[&](const ParticipantDescr& part){return part.partId == callId;}))
		== ctx.participants.end())
	{
		if (cb)
		{
			cb(cb::ProcessingResult::failed, {}, {});
		}
		return;
	}
	this->Send(msg::RemovePartMessage(
		ctx.chatId,
		callId,
		partIter->tp,
		m_currentAccount->GetCallID(),
		m_currentAccount->GetExactCallID(),
		m_currentAccount->GetCallIDType(),
		cb).AcquireMessage(), {});
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::AddUserOnly(
	const GlobalContext& ctx,
	const vs::ResolverInterface::ResolveInfo &partInfo,
	const cb::OnChainUpdateByMsgCb& cb)
{
	msg::AddPartMessage msg(
		ctx,
		partInfo.callId,
		ParticipantType::client,
		m_currentAccount->GetCallID(),
		m_currentAccount->GetExactCallID(),
		m_currentAccount->GetCallIDType(),
		cb);
	this->Send(std::move(msg.AcquireMessage()), {});
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::AddWithBS(
	const GlobalContext& ctx,
	const vs::ResolverInterface::ResolveInfo& partInfo,
	const cb::OnChainUpdateByMsgCb& cb)
{
	AddUserOnly(ctx, partInfo,
		[
			this,
			partInfo,
			chatId = ctx.chatId,
			cb
		](
			cb::ProcessingResult res,
			ChatMessageIDRef msgId,
			const cb::MsgIdAndOrderInChain& orderInChain
		)
	{
		if (res != cb::ProcessingResult::ok)
		{
			if (cb)
				cb(res, msgId, orderInChain);
			return;
		}
		// Get new chat info
		auto info = GetCfg()->GetChatStorage()->GetGlobalContext(chatId);
		if (info.chatId.empty())
		{
			if(cb)
				cb(cb::ProcessingResult::failed, {}, {});
		}
		else
		{
			msg::AddPartMessage msg(
				info,
				partInfo.bsInfo,
				ParticipantType::server,
				m_currentAccount->GetCallID(),
				m_currentAccount->GetExactCallID(),
				m_currentAccount->GetCallIDType(),
				cb);
			this->Send(std::move(msg.AcquireMessage()), {});
		}
	});
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::SendGroup(
	const ChatID& id,
	chat::msg::ContentMessage&& message,
	const cb::OnChainUpdateByMsgCb &cb)
{
	auto info = GetCfg()->GetChatStorage()->GetGlobalContext(id);
	if (info.chatId.empty())
	{
		if (cb)
			cb(cb::ProcessingResult::failed, {}, {});
		return;
	}
	auto msg = std::move(message).Seal(
		info.chatId,
		m_currentAccount->GetCallID(),
		m_currentAccount->GetExactCallID(),
		m_currentAccount->GetCallIDType(), {}, cb);
	this->Send(std::move(msg), {});
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::SendP2P(
	CallIDRef to,
	chat::msg::ContentMessage&& message,
	const cb::OnChainUpdateByMsgCb &cb)
{
	auto chatId = GetP2PChatID(m_currentAccount->GetCallID(), to);
	auto msg = std::move(message).Seal(
		chatId,
		m_currentAccount->GetCallID(),
		m_currentAccount->GetExactCallID(),
		m_currentAccount->GetCallIDType(), {}, cb);
	msg::InsertParamStrIntoMsgContent(msg, msg::toKeyName, to);
	this->Send(std::move(msg), {});
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::RemoveMessage(
	ChatIDRef chatId,
	ChatMessageIDRef msgId,
	bool forAll,
	const cb::OnChainUpdateByMsgCb& cb)
{
	msg::MessageRemovalMessage msg(
		chatId,
		msgId,
		forAll,
		m_currentAccount->GetCallID(),
		m_currentAccount->GetExactCallID(),
		m_currentAccount->GetCallIDType(),
		cb);
	this->Send(std::move(msg.AcquireMessage()), {});
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::ClearHistory(
	ChatIDRef chatId,
	bool forAll,
	const cb::OnChainUpdateByMsgCb& cb)
{
	msg::ClearHistoryMessage msg(
		chatId,
		forAll,
		m_currentAccount->GetCallID(),
		m_currentAccount->GetExactCallID(),
		m_currentAccount->GetCallIDType(),
		cb);
	this->Send(std::move(msg.AcquireMessage()), {});
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::OnChatMessageArrived(
	msg::ChatMessagePtr&& msg,
	CallIDRef sender)
{
	/**
	what kind
	*/
	auto msgType = MessageType::undefined;
	// debug log
	{
		auto from = msg->GetParamStrRef(attr::FROM_paramName);
		uint32_t type{0};
		msg->GetParamI32(attr::MESSAGE_TYPE_paramName, type);
		auto msgId = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
		CHAT_TRACE(log_stream << "APPLAYER: my_call_id = "
			<< m_currentAccount->GetExactCallID()
			<< "; msgId = "
			<< (!msgId.empty() ? msgId : "<none>")
			<< "; type = " << (type ? std::to_string(type) : "<none>")
			<< "; author = " << (!from.empty() ? from : "<none>")
			<< "; sender = " << (!sender.empty() ? sender : "<none>"));
	}

	if (!msg->GetParamI32(attr::MESSAGE_TYPE_paramName, msgType))
		return;
	const auto &h = m_recvMsgHandlers.find(msgType);
	if (IsMessageStorable(msg))
	{
		const auto &inChainRes = msg->GetMsgInChainresult();
		assert(inChainRes.res == cb::ProcessingResult::ok);
		if (inChainRes.res == cb::ProcessingResult::ok)
		{
			m_fireOnMsgRecv(inChainRes.updatedOrder, msg);
		}
	}
	if (h != m_recvMsgHandlers.end())
		h->second(std::move(msg));
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::OnInviteRecv(msg::ChatMessagePtr&& m)
{
	auto chatId = m->GetParamStrRef(attr::CHAT_ID_paramName);
	auto title = GetParamStrFromMsgContent(m, msg::titleKeyName);
	auto version = GetParamStrFromMsgContent(m, msg::versionKeyName);
	auto type = ChatType::undef;
	auto msgId = m->GetParamStrRef(attr::MESSAGE_ID_paramName);
	auto from = m->GetParamStrRef(attr::FROM_paramName);
	auto to = GetParamStrFromMsgContent(m, msg::toKeyName);
	if (chatId.empty()
		|| version.empty()
		|| !GetParamFromMsgContent(m, msg::typeKeyName, type)
		|| msgId.empty()
		|| from.empty()
		|| to.empty()
		|| !m_currentAccount->IsMyCallId(to))
	{
		return;
	}
	if (m_fireOnInvite)
		m_fireOnInvite(
			cb::ChatDescription(chatId,
				ChatType(type), std::move(title), std::move(version)),
			[
				this,
				chatId = ChatID(chatId),
				msgId = ChatMessageID(msgId),
				from = CallID(from)] (const InviteResponseCode&r)
	{
		this->Send(msg::InviteResponseMessage(
			chatId,
			from,
			m_currentAccount->GetCallID(),
			m_currentAccount->GetExactCallID(),
			m_currentAccount->GetCallIDType(),
			msgId,
			r,
			nullptr).AcquireMessage(), {});
	});
}
template<typename LayersTraits, typename MainStoragePtr>
void AppLayer<LayersTraits, MainStoragePtr>::OnInviteRespRecv(msg::ChatMessagePtr&& m)
{
	/*
	if accept => add new part, add part's BS;
	*/
	if (!m)
		return;
	int32_t r(0);
	auto chatId = m->GetParamStrRef(attr::CHAT_ID_paramName);
	auto responseToId = GetParamStrFromMsgContent(m, msg::nameKeyName);
	if (responseToId.empty())
		return;
	auto from = m->GetParamStrRef(attr::FROM_paramName);
	auto waitingIter = m_waitingInviteResponse.find(
		detail::WaitingInviteResponseRef(chatId, std::move(responseToId)));
	if (waitingIter == m_waitingInviteResponse.end())
		return;
	if (!chatId.empty()
		&& !from.empty()
		&& m->GetParam(attr::RESPONSE_CODE_paramName, r))
	{
		InviteResponseCode code = static_cast<InviteResponseCode>(r);

		if (code == InviteResponseCode::accept)
		{
			auto hunt_from = VS_RemoveTranscoderID_sv(from);
			m_resolver->Resolve(
				hunt_from,
				[this, chatId = waitingIter->chatId]
			(bool res, vs::ResolverInterface::ResolveInfo partInfo)
			{
				if (!res)
					return;
				//Add to chat call id and Add bs
				AddParticipant(chatId, partInfo.callId, {});
			}
			);
		}
		if (m_fireOnInviteRespose)
			m_fireOnInviteRespose(chatId, from, code);
	}
	m_waitingInviteResponse.erase(waitingIter);
}
template<typename LayersTraits, typename MainStoragePtr>
GlobalConfigPtr AppLayer<LayersTraits, MainStoragePtr>::GetCfg() const
{
	auto cfg = m_cfg.lock();
	assert(cfg);
	return cfg;
}
}
#undef DEBUG_CURRENT_MODULE
