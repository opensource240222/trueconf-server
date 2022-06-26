#pragma once
#include "SystemChatLayer.h"

#include "chatlib/msg/chat_messages_construct.h"

namespace chat
{
template<typename LayersTraits>
template<typename ...Args>
SystemChatLayer<LayersTraits>::SystemChatLayer(const GlobalConfigPtr& cfg, Args&&... args)
	: LayerHelper<LayersTraits>(std::forward<Args>(args)...)
	, m_cfg(cfg)
{
	cfg->GetEventsSubscription()->SubscribeToPartAdded(
		[this](ChatIDRef chatId, CallIDRef partId,
			ChatMessageIDRef msgId, CallIDRef author)
	{
		this->PostCall(
			[
				this,
				chatId = ChatID(chatId),
				partId = CallID(partId),
				msgId = ChatMessageID(msgId),
				author = CallID(author)
			]() mutable
		{
			HandleOnAddPart(chatId, partId, std::move(msgId), std::move(author));
		});
	});
	cfg->GetEventsSubscription()->SubscribeToPartRemoved(
		[this](ChatIDRef chatId, CallIDRef part, ChatMessageIDRef msgId)
	{
		this->PostCall(
			[
				this,
				chatId = ChatID(chatId),
				part = CallID(part),
				msgId = ChatMessageID(msgId)
			]() mutable
		{
			HandleOnRemovePart(chatId, part, std::move(msgId));
		});
	});
	cfg->GetEventsSubscription()->SubscribeToChatCreate(
		[this](const GlobalContext& chatInfo)
	{
		this->PostCall(
			[
				this,
				chatInfo = chatInfo
			]() mutable
		{
			HandleOnChatCreated(std::move(chatInfo));
		});
	}
	);
}
template<typename LayersTraits>
void SystemChatLayer<LayersTraits>::ForwardBelowMessage(msg::ChatMessagePtr&& msg,
	std::vector<ParticipantDescr>&& dstParts)
{
	this->Send(std::move(msg), std::move(dstParts));
}
template<typename LayersTraits>
void SystemChatLayer<LayersTraits>::OnChatMessageArrived(
	msg::ChatMessagePtr&& msg,
	CallIDRef sender)
{
	ForwardAboveMessage(std::move(msg), sender);
}
template<typename LayersTraits>
void SystemChatLayer<LayersTraits>::ShutDown()
{
	LayerHelper<LayersTraits>::ShutDown();
}
template<typename LayersTraits>
template<typename CallBack>
void SystemChatLayer<LayersTraits>::GetSysChatForSend(CallIDRef part, ChatIDRef chatWhere, CallBack&& cb)
{
	auto account = GetCfg()->GetCurrentAccount();
	if (account->GetCallIDType() == vs::CallIDType::client
		&& part == account->GetCallID())
	{
		auto sysChatId = GetP2PChatID(part, account->GetBS());
		if (sysChatId != chatWhere)
		{
			cb(true, sysChatId, account->GetBS());
		}
		else
			cb(false, {}, {});
	}
	else if(account->GetCallIDType() == vs::CallIDType::server
		&& account->GetCallID() != part)
	{
		auto resolver = GetCfg()->GetResolver();
		resolver->Resolve(part,
			[
				account,
				chatWhere = ChatID(chatWhere),
				cb = std::move(cb)
			](bool success, vs::ResolverInterface::ResolveInfo&& info) mutable
		{
			if (!success || info.bsInfo != account->GetCallID())
			{
				cb(false, {}, {});
				return;
			}
			else
			{
				auto sysChatId = GetP2PChatID(info.bsInfo, info.callId);
				if (sysChatId != chatWhere)
					cb(true, sysChatId, info.callId);
				else
					cb(false, ChatID(), CallID());
			}
		});
	}
	else
	{
		cb(false, {}, {});
	}
}
template<typename LayersTraits>
void SystemChatLayer<LayersTraits>::HandleOnRemovePart(ChatIDRef whereRemovedChatId,
	CallIDRef part, ChatMessageID&& msgId)
{
	GetSysChatForSend(part, whereRemovedChatId,
		[
			this,
			msgId = std::move(msgId),
			part = CallID(part),
			whereRemovedChatId = ChatID(whereRemovedChatId)
		](bool needSend, ChatIDRef sysChatId, CallIDRef peer)
	{
		if (!needSend)
			return;
		auto cfg = GetCfg();
		auto account = cfg->GetCurrentAccount();
		// FIXME: Hardcode until permission logic is implemented
		std::string leave_permission = "{\"show_history\" : true}";
		auto msg = msg::PartRemovedFromChatMessage(
			sysChatId,
			account->GetCallID(),
			account->GetExactCallID(),
			account->GetCallIDType(),
			peer, whereRemovedChatId, part,
			msgId, std::move(leave_permission),
			{}, {}).AcquireMessage();
		this->Send(std::move(msg), {});
	});
}
template<typename LayersTraits>
void SystemChatLayer<LayersTraits>::HandleOnAddPart(ChatIDRef whereAdded, CallIDRef part,
	ChatMessageID&& msgId, CallID&& author)
{
	GetSysChatForSend(part, whereAdded,
		[
			this,
			whereAdded = ChatID(whereAdded),
			part = CallID(part),
			msgId = std::move(msgId),
			author = std::move(author)
		](bool needSend, ChatIDRef sysChatId, CallIDRef peer)
	{
		if (!needSend)
			return;
		auto storage = GetCfg()->GetChatStorage();
		auto ctx = storage->GetGlobalContext(whereAdded);
		if (!ctx.chatId.empty())
			SendPartAddedToChatMessage(sysChatId, peer, ctx, msgId,
				part, author, {});
	});
}
template<typename LayersTraits>
void SystemChatLayer<LayersTraits>::HandleOnChatCreated(GlobalContext&& chatInfo)
{
	if (chatInfo.type != ChatType::p2p)
		return;
	std::vector<ParticipantDescr> parts;
	// filter only clients
	std::copy_if(chatInfo.participants.begin(), chatInfo.participants.end(),
		std::back_inserter(parts),
		[](const ParticipantDescr& part) { return part.tp == ParticipantType::client; });
	// search only user-user chats, skip chat with user-server (system chats)
	if (parts.size() != 2)
		return;
	auto callId = parts.front().partId;
	auto chatId = chatInfo.chatId;
	GetSysChatForSend(callId, chatId,
		[
			this,
			chatInfo = std::move(chatInfo),
			parts = std::move(parts)
		](bool needSend, ChatIDRef sysChatId, CallIDRef peer) mutable
	{
		if (needSend)
		{
			auto& addedPart = parts.front().partId;
			auto& p2pPart = parts.back().partId;
			chatInfo.title = p2pPart;
			SendPartAddedToChatMessage(sysChatId, peer, chatInfo,
				chatInfo.msgId, addedPart, chatInfo.creator, p2pPart);
			return;
		}
		// check second
		auto callId = parts.back().partId;
		auto chatId = chatInfo.chatId;
		GetSysChatForSend(callId, chatId,
			[
				this,
				chatInfo = std::move(chatInfo),
				parts = std::move(parts)
			](bool needSend, ChatIDRef sysChatId, CallIDRef peer) mutable
		{
			if(needSend)
			{
				auto& addedPart = parts.back().partId;
				auto& p2pPart = parts.front().partId;
				chatInfo.title = p2pPart;
				SendPartAddedToChatMessage(sysChatId, peer, chatInfo,
					chatInfo.msgId, addedPart, chatInfo.creator, p2pPart);
			}
		});
	});

}
template<typename LayersTraits>
void SystemChatLayer<LayersTraits>::SendPartAddedToChatMessage(ChatIDRef sysChatId, CallIDRef to,
	const GlobalContext& chatInfo, ChatMessageIDRef triggerMsg, CallIDRef addedPart,
	CallIDRef whoAdd, CallIDRef p2pPart)
{
	auto account = GetCfg()->GetCurrentAccount();
	this->Send(msg::PartAddedToChatMessage(
		sysChatId,
		account->GetCallID(),
		account->GetExactCallID(),
		account->GetCallIDType(),
		to,
		chatInfo.chatId,
		chatInfo.title,
		triggerMsg,
		addedPart,
		whoAdd,
		p2pPart,
		{}, {}).AcquireMessage(), {});
}
template<typename LayersTraits>
GlobalConfigPtr SystemChatLayer<LayersTraits>::GetCfg() const
{
	auto cfg = m_cfg.lock();
	assert(cfg);
	return cfg;
}
}