#pragma once

#include "chatlib/interface/SyncChatInterface.h"
#include "chatlib/layers/ChatLayerAbstract.h"
#include "chatlib/layers/SyncContext.h"

#include "std-generic/compat/functional.h"
#include "std-generic/compat/map.h"
#include "std-generic/compat/set.h"
#include "std-generic/cpplib/macro_utils.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/synchronized.h"
#include "std-generic/cpplib/VS_ClockWrapper.h"

namespace chat
{
namespace detail
{
struct chat_id_msg_id_st
{
	VS_FORWARDING_CTOR2(chat_id_msg_id_st, chat_id, msg_id) {}
	ChatID        chat_id;
	ChatMessageID msg_id;
	bool operator<(const chat_id_msg_id_st&rhs) const
	{
		return std::tie(chat_id, msg_id)
			< std::tie(rhs.chat_id, rhs.msg_id);
	}
};
struct chat_id_msg_id_ref
{
	VS_FORWARDING_CTOR2(chat_id_msg_id_ref, chat_id, msg_id) {}
	ChatIDRef        chat_id;
	ChatMessageIDRef msg_id;
	bool operator<(const chat_id_msg_id_ref &rhs) const
	{
		return std::tie(chat_id, msg_id)
			< std::tie(rhs.chat_id, rhs.msg_id);
	}
	explicit operator chat_id_msg_id_st() const
	{
		return { chat_id, msg_id };
	}
};
}
template<typename LayersTraits>
class IntegrityLayer
	: public LayerInterface
	, public LayerHelper<LayersTraits>
	, public SyncChatInterface
{
	const static std::string integrity_layer_id_;
	const std::chrono::seconds m_timeout_duration = std::chrono::seconds(1);
	using ResultCallBack = std::function<void(notify::SyncResult)>;
	using SyncPendingCnt = vs::map<ChatID, std::multiset<CallID>, vs::str_less>;

	void ForwardBelowMessage(msg::ChatMessagePtr&& msg,
		std::vector<ParticipantDescr>&& dstParts) override;
	void OnChatMessageArrived(
		msg::ChatMessagePtr&&msg,
		CallIDRef sender) override;
	void SyncChatTail(ChatIDRef chat_id, uint32_t tail, CallIDRef with_who,
		const cb::ChatMsgIDChainCallBack &cb) override;
	void ShutDown() override;
	void TryTailSync(
		ChatIDRef chat_id,
		CallIDRef msg_sender,
		string_view tail_hash_from_msg,
		uint32_t msgs_in_tail,
		ChatMessageIDRef init_msg_id);

	bool ProcessingSync(msg::ChatMessagePtr&&m, CallIDRef sender);
	bool ProcessingGetTailHash(msg::ChatMessagePtr&&msg,CallIDRef sender);

	bool SetTailHash(const msg::ChatMessagePtr&m);
	// Return tail_hash if msg_must_be_last is last in got chain, return empty otherwise
	// Don't check msg_must_be_last if empty
	detail::tail_hash_info
		GetTailHash(const ChatID& chat_id,
			uint32_t tail_len = 0,
			ChatMessageIDRef msg_must_be_last = {}
			);
	void SynchronizeTail(
		ChatIDRef chat_id,
		CallIDRef sender,
		uint32_t msgs_in_tail,
		string_view my_tail_hash,
		ChatMessageIDRef init_msg_id,
		const ResultCallBack &cb = {});
	std::vector<msg::BucketSyncMessage::BucketIdHash>
		GetTailBucketList(const ChatID& chat_id, uint32_t tail_len);
	void SendTo(CallIDRef call_id, msg::ChatMessagePtr&&m);
	std::vector<ChatMessageID> GetMessagesInBucket(const ChatID& chat_id, uint64_t bucket);
	void SyncResult(const std::string& sync_id, notify::SyncResult result);
	void Timeout();
	std::vector<msg::ChatMessagePtr> GetMessagesToRetransmit(const ChatID& chatId,
		const msg::MsgIdsByBuckets& msgIdsForSync) const;
	GlobalConfigPtr GetCfg() const;

	vs::map<detail::chat_id_msg_id_st, std::string,vs::less<>> msg_hashs_;
	std::weak_ptr<ChatStorage>	storage_;
	AccountInfoPtr current_account_;
	ChatEventsNotifierPtr events_notifier_;
	ChatEventsSubscriptionPtr chat_event_sub_;
	vs::map<std::string, std::shared_ptr<detail::sync_context>, vs::str_less> sync_ctx_; /**key - > sync_id*/
	// what chats are syncing now and with whom
	SyncPendingCnt sync_pending_;
	std::vector<std::shared_ptr<detail::sync_context>> garbage_sync_ctx_;
	std::weak_ptr<GlobalConfigInterface> cfg_;
	struct paused_recv_msg_st
	{
		VS_FORWARDING_CTOR2(paused_recv_msg_st, msg, sender) {}
		msg::ChatMessagePtr msg;
		CallID sender;
	};
	vs::map<ChatID, std::queue<paused_recv_msg_st>> paused_recv_msg_;
	struct get_tail_hash_req_cb
	{
		VS_FORWARDING_CTOR2(get_tail_hash_req_cb, expired_time,cb){}
		std::chrono::steady_clock::time_point expired_time;
		cb::ChatMsgIDChainCallBack cb;
	};
	// FIXME: remove after expiring. Add timeout processing first
	vs::map<ChatID, get_tail_hash_req_cb, vs::str_less> pending_get_tail_hash_reqs_;
public:
	template<typename ...Args>
	IntegrityLayer(const GlobalConfigPtr &cfg, Args&&... args);
};
}