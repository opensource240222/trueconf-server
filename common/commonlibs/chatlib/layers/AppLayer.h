#pragma once
#include "chatlib/callbacks.h"
#include "chatlib/chat_defs.h"
#include "chatlib/helpers/ResolverInterface.h"
#include "chatlib/layers/ChatLayerAbstract.h"
#include "chatlib/msg/chat_messages_construct.h"

#include "std-generic/compat/functional.h"
#include "std-generic/compat/map.h"
#include "std-generic/compat/set.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/undef_windows.h"

namespace chat
{
namespace detail
{
struct WaitingInviteResponseSt
{
	VS_FORWARDING_CTOR2(WaitingInviteResponseSt, chatId, chatMessageId) {}
	ChatID chatId;
	ChatMessageID chatMessageId;
	bool operator<(const WaitingInviteResponseSt &rhs) const
	{
		return std::tie(chatId, chatMessageId)
			< std::tie(rhs.chatId, rhs.chatMessageId);
	}
};
struct WaitingInviteResponseRef
{
	VS_FORWARDING_CTOR2(WaitingInviteResponseRef, chatId, chatMessageId) {}
	ChatIDRef chatId;
	ChatMessageIDRef chatMessageId;
	bool operator<(const WaitingInviteResponseRef &rhs) const
	{
		return std::tie(chatId, chatMessageId)
			< std::tie(rhs.chatId, rhs.chatMessageId);
	}
	explicit operator WaitingInviteResponseSt() const
	{
		return { chatId, chatMessageId };
	}
};
struct ChatIdCallIdSt
{
	VS_FORWARDING_CTOR2(ChatIdCallIdSt, chatId, callId) {}
	ChatID chatId;
	CallID callId;
	bool operator<(const ChatIdCallIdSt &rhs) const
	{
		return std::tie(chatId, callId)
			< std::tie(rhs.chatId, rhs.callId);
	}
};
struct ChatIdCallIdRef
{
	VS_FORWARDING_CTOR2(ChatIdCallIdRef, chatId, callId) {}
	ChatIDRef chatId;
	CallIDRef callId;
	bool operator<(const ChatIdCallIdRef &rhs) const
	{
		return std::tie(chatId, callId)
			< std::tie(rhs.chatId, rhs.callId);
	}
	explicit operator ChatIdCallIdSt() const
	{
		return { chatId, callId };
	}
};
}
// FIXME: MainStoragePtr usage makes layers template params inconsistent. Try to fix this;
template <typename LayersTraits, typename MainStoragePtr>
class AppLayer final : public LayerHelper<LayersTraits>
{
public:
	template<typename ...Args>
	AppLayer(const GlobalConfigPtr& cfg, const MainStoragePtr& storageLayer, Args&&... args);
	PersonalContextList GetMyPersonalContexts(uint32_t pageSize, uint32_t pageNum);
	GlobalContext GetGlobalContext(const ChatID& id);
	void SyncSystemChat();
	void SyncMyChat(chat::ChatIDRef chatId);
	msg::ChatMessagePtr GetMessage(const ChatMessageID& msgId) const;
	std::vector<MessageWithOrder> GetMessages(
		const ChatID& id,
		const ChatMessageID& msgBefore,
		size_t count) const;
	std::vector<ChatMessageID>
		GetUndeliveredMessagesByChatId(const ChatID& chatId) const;
	void CreateChat(ChatType tp, std::string title,
		const cb::CreateChatCallBack &resCb);
	//chat function
	void InviteToChat(
		const ChatID&,
		CallID callId,
		const cb::OnChainUpdateByMsgCb& cb = {});
	void AddParticipant(
		const ChatID& chatId,
		CallID callId,
		const cb::OnChainUpdateByMsgCb& cb = {});
	void RemoveParticipant(
		const ChatID& chatId,
		CallID callId,
		const cb::OnChainUpdateByMsgCb& cb = {});
	void SendGroup(
		const ChatID&,
		chat::msg::ContentMessage&& message,
		const cb::OnChainUpdateByMsgCb &);
	void SendP2P(
		CallIDRef,
		chat::msg::ContentMessage&& message,
		const cb::OnChainUpdateByMsgCb &);
	void RemoveMessage(
		ChatIDRef chatId,
		ChatMessageIDRef msgId,
		bool forAll,
		const cb::OnChainUpdateByMsgCb& cb = {});
	void ClearHistory(
		ChatIDRef chatId,
		bool forAll,
		const cb::OnChainUpdateByMsgCb& cb = {});

	void SetOnInviteCallBack(const cb::OnInviteArrivedCallBack&cb);
	void SetOnInviteResponseCallBack(const cb::OnInviteResponseArrivedCallBack&cb);
	nod::connection SetOnMsgRecvSlot(const cb::OnMsgRecv::slot_type &slot)
	{
		return this->m_fireOnMsgRecv.connect(slot);
	}
private:
	using RecvMsgHandlerFunc = std::function<void(
		msg::ChatMessagePtr&&)>;
	using ChatEventHandlerFunc = std::function<void()>;
	void GetChatTailFrom(ChatIDRef id,
		uint32_t tailLen,
		CallIDRef from,
		const cb::ChatMsgIDChainCallBack &cb);
	void AddWithBS(
		const GlobalContext& ctx,
		const vs::ResolverInterface::ResolveInfo& partInfo,
		const cb::OnChainUpdateByMsgCb& cb);
	void AddUserOnly(
		const GlobalContext& ctx,
		const vs::ResolverInterface::ResolveInfo& partInfo,
		const cb::OnChainUpdateByMsgCb& cb);
	void OnChatMessageArrived(
		msg::ChatMessagePtr &&m,
		CallIDRef sender) override;
	void OnInviteRecv(msg::ChatMessagePtr&& m);
	void OnInviteRespRecv(msg::ChatMessagePtr&& m);
	GlobalConfigPtr GetCfg() const;

	cb::OnMsgRecv                       m_fireOnMsgRecv;
	cb::OnInviteArrivedCallBack         m_fireOnInvite;
	cb::OnInviteResponseArrivedCallBack m_fireOnInviteRespose;
	AccountInfoPtr                      m_currentAccount;
	MainStoragePtr                      m_storageLayer;
	vs::ResolverPtr                     m_resolver;
	std::weak_ptr<
		GlobalConfigInterface>	        m_cfg;

	vs::map<MessageType, RecvMsgHandlerFunc>             m_recvMsgHandlers;
	vs::set<detail::WaitingInviteResponseSt, vs::less<>> m_waitingInviteResponse;
};
}