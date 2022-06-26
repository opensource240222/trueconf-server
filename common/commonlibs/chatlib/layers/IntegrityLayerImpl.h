#include "IntegrityLayer.h"

#include "chatlib/helpers/AccountInfo.h"
#include "chatlib/interface/GlobalConfigInterface.h"
#include "chatlib/msg/attr.h"
#include "chatlib/msg/chat_messages_construct.h"
#include "chatlib/notify/GlobalChatEvents.h"
#include "chatlib/utils/chat_utils.h"

#include "std-generic/clib/sha1.h"
#include "std-generic/cpplib/scope_exit.h"

#include <algorithm>
#include <cassert>
#include <sstream>
#include <type_traits>

// For not defining GetMessage -> GetMessage(A/W)
#include "std-generic/undef_windows.h"

#define DEBUG_CURRENT_MODULE VS_DM_CHATS

namespace chat
{
template<typename LayersTraits>
const std::string IntegrityLayer<LayersTraits>::integrity_layer_id_ = "integrity";

inline std::string genereate_sync_id()
{
	SHA1 sha1;
	sha1.Update(std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
	sha1.Update(GenerateSalt());
	sha1.Final();
	char hash[41];
	sha1.GetString(hash);
	return hash;
}
const auto g_max_req_duration = std::chrono::seconds(30);

namespace detail
{
void PackTailHashInfo(const msg::ChatMessagePtr &m, const detail::tail_hash_info& tail_info)
{
	if (!tail_info.is_exist)
		return;
	CHAT_TRACE(log_stream << "PackTailHashInfo (" << GetParamStrFromMsgContent(m, msg::fromInstanceKeyName) <<"): "
		<< "msg_id = " << m->GetParamStrRef(attr::MESSAGE_ID_paramName) << "; "
		<< "tail.len = " << tail_info.tail_len << "; "
		<< "tail.hash = " << tail_info.tail_hash);
	m->SetParam(attr::TAIL_HASH_paramName, tail_info.tail_hash);
	m->SetParam(attr::TAIL_LENGTH_paramName, static_cast<int32_t>(tail_info.tail_len));
}
tail_hash_info UnPackTailInfo(const msg::ChatMessagePtr &m)
{
	auto tail_hash = m->GetParamStrRef(attr::TAIL_HASH_paramName);
	uint32_t len(0);
	if (!m->GetParam(attr::TAIL_LENGTH_paramName, len)
		|| tail_hash.empty())
		return {};
	return tail_hash_info(true,len,tail_hash);
}
bool operator<(const detail::chat_id_msg_id_st& lhs, const detail::chat_id_msg_id_ref& rhs)
{
	return std::tie(lhs.chat_id, lhs.msg_id) < std::tie(rhs.chat_id, rhs.msg_id);
}
bool operator<(const detail::chat_id_msg_id_ref& lhs, const detail::chat_id_msg_id_st& rhs)
{
	return std::tie(lhs.chat_id, lhs.msg_id) < std::tie(rhs.chat_id, rhs.msg_id);
}
}

template<typename LayersTraits>
template<typename ...Args>
IntegrityLayer<LayersTraits>::IntegrityLayer(
	const GlobalConfigPtr &cfg, Args&&...args)
	: LayerHelper<LayersTraits>(std::forward<Args>(args)...)
	, storage_(cfg->GetChatStorage())
	, current_account_(cfg->GetCurrentAccount())
	, events_notifier_(cfg->GetEventsNotifier())
	, chat_event_sub_(cfg->GetEventsSubscription())
	, cfg_(cfg)
{
	Timeout();
}
template<typename LayersTraits>
std::vector<ChatMessageID>
IntegrityLayer<LayersTraits>::GetMessagesInBucket(const ChatID& chat_id, uint64_t bucket)
{
	return storage_.lock()->GetMessagesInBucket(chat_id, bucket);
}
template<typename LayersTraits>
void IntegrityLayer<LayersTraits>::SyncResult(const std::string& sync_id, notify::SyncResult result)
{
	CHAT_TRACE(log_stream << "IntegrityLayer::SyncResult; sync_id = " << sync_id << " res = " << static_cast<int>(result));
	auto iter = sync_ctx_.find(sync_id);
	if (iter == sync_ctx_.end())
		return;
	auto sync_pend_i = sync_pending_.find(iter->second->get_chat_id());
	if (sync_pend_i != sync_pending_.end())
	{
		assert(sync_pend_i->second.count(iter->second->get_peer_call_id()) > 0);
		auto peer_i = sync_pend_i->second.find(iter->second->get_peer_call_id());
		if(peer_i != sync_pend_i->second.end())
		{
			sync_pend_i->second.erase(peer_i);
			if (sync_pend_i->second.size() == 0)
			{
				sync_pending_.erase(sync_pend_i);
				auto paused_recv_msgs_i = paused_recv_msg_.find(iter->second->get_chat_id());
				while (!paused_recv_msgs_i->second.empty())
				{
					auto item = std::move(paused_recv_msgs_i->second.front());
					OnChatMessageArrived(std::move(item.msg), item.sender);
					paused_recv_msgs_i->second.pop();
				}
				paused_recv_msg_.erase(paused_recv_msgs_i);
			}
		}
	}
	garbage_sync_ctx_.push_back(iter->second);
	sync_ctx_.erase(iter);
}
template<typename LayersTraits>
void IntegrityLayer<LayersTraits>::Timeout()
{
	if (this->IsShutdowned())
		return;
	auto sync_ctx_snapshot = sync_ctx_;
	for (auto &sc : sync_ctx_snapshot) {
		sc.second->Tick();
	}
	this->ExpiresFromNow(m_timeout_duration);
	this->AsyncWait([this](TimerResult res)
	{
		if (res == TimerResult::canceled)
			return;
		this->PostCall([this]()
		{
			garbage_sync_ctx_.clear();
			Timeout();
		});
	});
}
template<typename LayersTraits>
std::vector<msg::ChatMessagePtr>
IntegrityLayer<LayersTraits>::GetMessagesToRetransmit(const ChatID& chatId,
	const msg::MsgIdsByBuckets& msgIdsForSync) const
{
	auto storage_lock = storage_.lock();
	if (!storage_lock)
		return {};
	return storage_lock->GetMessagesToRetransmit(chatId, msgIdsForSync);
}
template<typename LayersTraits>
GlobalConfigPtr IntegrityLayer<LayersTraits>::GetCfg() const
{
	auto cfg = cfg_.lock();
	assert(cfg);
	return cfg;
}
template<typename LayersTraits>
void IntegrityLayer<LayersTraits>::ForwardBelowMessage(msg::ChatMessagePtr&&m,
	std::vector<ParticipantDescr>&& dstParts)
{
	this->PostCall(vs::move_handler(
		[
			this,
			m = std::move(m),
			dstParts = std::move(dstParts)
		]() mutable
	{
		if (!IsMessageStorable(m))
		{
			this->Send(std::move(m), std::move(dstParts));
			return;
		}
		if (SetTailHash(m))
			this->Send(std::move(m), std::move(dstParts));
		else
		{
			assert(false);
			//error notify
		}
	}));
}
template<typename LayersTraits>
void IntegrityLayer<LayersTraits>::OnChatMessageArrived(
	msg::ChatMessagePtr&&m,
	CallIDRef sender)
{
	if (ProcessingSync(std::move(m), sender))
		return;
	if(ProcessingGetTailHash(std::move(m), sender))
		return;
	if (!IsMessageStorable(m))
	{
		ForwardAboveMessage(std::move(m), sender);
		return;
	}
	auto chat_id = m->GetParamStr(attr::CHAT_ID_paramName);
	if(!chat_id.empty()
		&& sync_pending_.count(chat_id)>0)
	{
		auto sync_method = m->GetParamStrRef(attr::BUCKETSYNC_METHOD_paramName);
		if (sync_method.empty())
		{
			auto queue_i = paused_recv_msg_.emplace(chat_id,
				std::queue<paused_recv_msg_st>());
			queue_i.first->second.emplace(std::move(m), sender);
			return;
		}
	}
	auto tail_info = detail::UnPackTailInfo(m);
	if(tail_info.is_exist)
	{
		auto info = GetCfg()->GetChatStorage()->GetGlobalContext(chat_id);
		bool check_sync = false;
		if (!info.chatId.empty())
		{
			auto sender_user = m->GetParamStrRef(
				attr::FROM_paramName);
			bool is_part_in_chat = FindPartDescrById(
				info.participants.begin(),
				info.participants.end(),
				sender_user) != info.participants.end();
			check_sync = !sender_user.empty()
				? is_part_in_chat
				: false;
		}
		if (check_sync)
		{
			m->AddOnChainUpdateByMsgCallBack(
				[
					this,
					id = ChatID(m->GetParamStrRef(attr::CHAT_ID_paramName)),
					msg_sender = CallID(sender),
					tail_info = std::move(tail_info)
				] (
						cb::ProcessingResult res,
						ChatMessageIDRef msg_id,
						const cb::MsgIdAndOrderInChain&)
				{
					if (res != cb::ProcessingResult::ok)
						return;
					TryTailSync(
						id,
						msg_sender,
						tail_info.tail_hash,
						tail_info.tail_len,
						msg_id); });
		}
		ForwardAboveMessage(std::move(m), sender);
	}
	if(m)
		ForwardAboveMessage(std::move(m), sender);
}
template<typename LayersTraits>
void IntegrityLayer<LayersTraits>::TryTailSync(
	ChatIDRef chat_id,
	CallIDRef msg_sender,
	string_view tail_hash_from_msg,
	uint32_t msgs_in_tail,
	ChatMessageIDRef init_msg_id)
{
	this->PostCall(
		[
			this,
			chat_id = ChatID(chat_id),
			msg_sender = CallID(msg_sender),
			tail_hash_from_msg = std::string(tail_hash_from_msg),
			msgs_in_tail,
			init_msg_id = ChatMessageID(init_msg_id)
		]()
	{
		auto hash = GetTailHash(chat_id, msgs_in_tail);
		if (tail_hash_from_msg != hash.tail_hash)
		{
			CHAT_TRACE(log_stream << "SyncLost for chat = "
				<< chat_id << '\n'
				<< current_account_->GetExactCallID()
				<< " <===> " << msg_sender
				<< "; my_tail_hash = " << hash.tail_hash
				<< "; tail_from_msg "
				<< tail_hash_from_msg << '\n');
			SynchronizeTail(
				chat_id,
				msg_sender,
				std::max(
					hash.tail_len,
					msgs_in_tail),
				hash.tail_hash,
				init_msg_id);
		}
	});
}
template<typename LayersTraits>
detail::tail_hash_info
IntegrityLayer<LayersTraits>::GetTailHash(const ChatID& chat_id,
	uint32_t tail_len, ChatMessageIDRef msg_must_be_last)
{
	auto storage = storage_.lock();
	if (!storage)
		return { false, 0, std::string() };
	tail_len == 0
		? GetCfg()->GetTailLength()
		: tail_len;
	auto tail = storage->GetLastMessages(chat_id, tail_len);
	if (!msg_must_be_last.empty()
		&& tail.back().msg_id != msg_must_be_last)
		return {};
	SHA1 sha1;
	for (const auto &t : tail)
	{
		sha1.Update(t.msg_id);
	}
	sha1.Final();
	char hash[41] = {0};
	sha1.GetString(hash);
	return {
		true,
		static_cast<uint32_t>(tail_len), // tail_len may be less then message count
		hash};
}
template<typename LayersTraits>
bool IntegrityLayer<LayersTraits>::SetTailHash(const msg::ChatMessagePtr & m)
{
	auto chat_id = m->GetParamStr(attr::CHAT_ID_paramName);
	if (chat_id.empty())
		return false;
	auto len = GetCfg()->GetTailLength();
	PackTailHashInfo(m, GetTailHash(
		chat_id, len, m->GetParamStrRef(attr::MESSAGE_ID_paramName)));
	return true;
}
template<typename LayersTraits>
void IntegrityLayer<LayersTraits>::SynchronizeTail(
	ChatIDRef chat_id,
	CallIDRef sender,
	uint32_t msgs_in_tail,
	string_view my_tail_hash,
	ChatMessageIDRef init_msg_id,
	const ResultCallBack &cb)
{
	auto cfg = GetCfg();
	assert(cfg);
	auto sync_id = genereate_sync_id();
	auto iter = sync_ctx_.emplace(sync_id,
		std::make_shared<detail::sync_context>(
			cfg,
			sync_id,
			chat_id,
			sender,
			msgs_in_tail,
			init_msg_id,
			my_tail_hash,
			[this](const ChatID& chat_id, uint32_t tail_len)
			{return GetTailBucketList(chat_id, tail_len); },
			[this](const ChatID& chat_id, uint64_t bucket)
			{return GetMessagesInBucket(chat_id, bucket); },
			[this](const ChatID& chatId, const msg::MsgIdsByBuckets& msgIdsForSync)
			{return GetMessagesToRetransmit(chatId, msgIdsForSync); },
			[this](const ChatID& chat_id, uint32_t tail_len)
			{return GetTailHash(chat_id, tail_len); },
			[this](CallIDRef to, msg::ChatMessagePtr&&m)
			{SendTo(to, std::move(m)); },
			[this, sync_id = sync_id, cb](notify::SyncResult result)
			{
				SyncResult(sync_id, result);
				if (cb)
					cb(result);
			}));
	if (iter.second)
	{
		auto sync_pend_i = sync_pending_.emplace(chat_id, std::multiset<CallID>());
		sync_pend_i.first->second.emplace(iter.first->second->get_peer_call_id());
		if(sync_pend_i.second)
			paused_recv_msg_.emplace(chat_id, std::queue<paused_recv_msg_st>());
		iter.first->second->start();
	}
}
template<typename LayersTraits>
std::vector<msg::BucketSyncMessage::BucketIdHash>
IntegrityLayer<LayersTraits>::GetTailBucketList(const ChatID& chat_id, uint32_t tail_len)
{
	auto storage = storage_.lock();
	assert(storage);
	std::vector<msg::BucketSyncMessage::BucketIdHash> res;
	decltype(res) empty;
	if (!storage)
		return empty;
	else
	{
		auto tail = storage->GetLastMessages(chat_id, tail_len);
		vs::set<uint64_t> bucket_nums; // needed for ordering vector elements
		for (const auto &i : tail)
			bucket_nums.insert(i.bucket);
		for (const auto &i : bucket_nums)
		{
			auto msg_list = storage->GetMessagesInBucket(chat_id, i);
			SHA1 sha1;
			for (const auto &id : msg_list)
			{
				sha1.Update(id);
			}
			sha1.Final();
			char hash[41] = { 0 };
			sha1.GetString(hash);
			res.emplace_back(i, hash);
		}
	}
	return res;
}
template<typename LayersTraits>
void IntegrityLayer<LayersTraits>::SendTo(CallIDRef call_id, msg::ChatMessagePtr && m)
{
	m->SetParam(attr::DST_ENDPOINT_paramName, call_id);
	this->Send(std::move(m), {});
}
template<typename LayersTraits>
bool IntegrityLayer<LayersTraits>::ProcessingSync(msg::ChatMessagePtr &&m, CallIDRef sender)
{
	if (!msg::BucketSyncMessage::IsMyMessage(m))
		return false;
	auto chat_id = m->GetParamStr(attr::CHAT_ID_paramName);
	auto sync_id = m->GetParamStrRef(attr::SYNC_ID_paramName);
	auto method = m->GetParamStrRef(attr::BUCKETSYNC_METHOD_paramName);
	bool start = false;
	m->GetParam(attr::SYNC_BUCKET_START, start);
	uint32_t tail_len = GetCfg()->GetTailLength();
	std::string tail_hash;
	auto cfg = GetCfg();
	assert(cfg);
	if (!cfg || sync_id.empty())
		return true; // skip message
	auto ctx = sync_ctx_.find(sync_id);
	if (ctx == sync_ctx_.end())
	{
		if (method.empty())
			return true;// skip
		if (method == attr::INSERT_MSG)
			return false; // insert msg
		if (!start || method != attr::SYNC_BUCKET_LIST_REQ)
			return true; // skip
		if (method == attr::SYNC_BUCKET_LIST_REQ)
		{
			int32_t val(0);
			if(m->GetParam(attr::TAIL_LENGTH_paramName, val))
				tail_len = static_cast<uint32_t>(val);
			auto hash = GetTailHash(chat_id, tail_len);
			tail_hash = std::move(hash.tail_hash);
		}
		CHAT_TRACE(log_stream << "SyncRequest "
			<< current_account_->GetExactCallID()
			<< " <===> " << sender << '\n');
		auto ctx_i = sync_ctx_.emplace(sync_id,
			std::make_shared<detail::sync_context>(
				cfg,
				std::string(sync_id),
				ChatID(chat_id),
				CallID(sender),
				tail_len,
				ChatMessageIDRef{},
				tail_hash,
				[this](const ChatID& chat_id, uint32_t tail_len)
				{return GetTailBucketList(chat_id, tail_len); },
				[this](const ChatID& chat_id, uint64_t bucket)
				{return GetMessagesInBucket(chat_id, bucket); },
				[this](const ChatID& chatId, const msg::MsgIdsByBuckets& msgIdsForSync)
				{return GetMessagesToRetransmit(chatId, msgIdsForSync); },
				[this](const ChatID& chat_id, uint32_t tail_len)
				{return GetTailHash(chat_id, tail_len); },
				[this](CallIDRef to, msg::ChatMessagePtr&&m)
				{SendTo(to, std::move(m)); },
				[this, sync_id = std::string(sync_id)](notify::SyncResult result)
				{SyncResult(sync_id, result); }));
		if (ctx_i.second)
		{
			auto sync_pend_i = sync_pending_.emplace(chat_id,std::multiset<CallID>());
			sync_pend_i.first->second.emplace(ctx_i.first->second->get_peer_call_id());
			if (sync_pend_i.second)
				paused_recv_msg_.emplace(chat_id, std::queue<paused_recv_msg_st>());
		}
		ctx = ctx_i.first;
	}
	bool skip_this_message = method != attr::INSERT_MSG;
	auto res = ctx->second->processing(m, sender);
	assert(res);
	return skip_this_message;
}
template<typename LayersTraits>
bool IntegrityLayer<LayersTraits>::ProcessingGetTailHash(msg::ChatMessagePtr &&msg, CallIDRef sender)
{
	if(msg::GetTailHashReqMessage::IsMyMessage(msg))
	{
		// processing request
		auto req_data = msg::GetTailHashReqMessage::GetReqData(msg);
		assert(req_data.success);
		if(!req_data.success || sender != req_data.from)
			return true;
		auto ctx = GetCfg()->GetChatStorage()->GetGlobalContext(req_data.chatId);
		if (ctx.chatId.empty())
		{
			auto resp = msg::GetTailHashRespMessage(
					req_data.chatId,
					msg::GetTailHashRespMessage::ReqResult::chat_not_exist, req_data.tailLen,
					{},current_account_->GetExactCallID(),req_data.from);
			this->Send(resp.AcquireMessage(), {});
		}
		else
		{
			this->PostCall(
				[
					this,
					req_data = std::move(req_data)
				]()
			{
				auto tail = GetTailHash(req_data.chatId, req_data.tailLen);
				if (!tail.is_exist)
				{
					auto resp = msg::GetTailHashRespMessage(
						req_data.chatId,
						msg::GetTailHashRespMessage::ReqResult::failed, req_data.tailLen,
						{}, current_account_->GetExactCallID(), req_data.from);
					this->Send(resp.AcquireMessage(), {});
					return;
				}
				auto resp = msg::GetTailHashRespMessage(
					req_data.chatId,
					msg::GetTailHashRespMessage::ReqResult::success,
					tail.tail_len, tail.tail_hash,
					current_account_->GetExactCallID(), req_data.from);
				this->Send(resp.AcquireMessage(), {});
			});
		}
		return true;
	}
	else if(msg::GetTailHashRespMessage::IsMyMessage(msg))
	{
		// processing response
		auto resp_data = msg::GetTailHashRespMessage::GetResponseData(msg);
		assert(resp_data.success);
		if(!resp_data.success
			|| sender != resp_data.from)
		{
			return true;
		}
		auto pending_cb = pending_get_tail_hash_reqs_.find(resp_data.chatId);
		if(pending_cb == pending_get_tail_hash_reqs_.end())
			return true;
		auto cb = pending_cb->second.cb;
		pending_get_tail_hash_reqs_.erase(pending_cb);
		if(resp_data.reqResult != msg::GetTailHashRespMessage::ReqResult::success)
		{
			cb(cb::ErrorCode::failed,{});
		}
		else
		{
			auto hash = GetTailHash(resp_data.chatId,resp_data.tailLen);
			if(hash.is_exist && hash.tail_hash == resp_data.hash)
			{
				auto storage = storage_.lock();
				auto tail = storage->GetLastMessages(resp_data.chatId, resp_data.tailLen);
				std::vector<ChatMessageID> res;
				std::transform(tail.begin(),tail.end(),	std::back_inserter(res),
					[](auto &item){return std::move(item.msg_id);});
				cb(cb::ErrorCode::success, std::move(res));
			}
			else
			{
				SynchronizeTail(
					resp_data.chatId,
					resp_data.from,
					resp_data.tailLen,
					hash.tail_hash,
					{},
					[
						cb,
						tail_len = resp_data.tailLen,
						chat_id = resp_data.chatId,
						this
					](notify::SyncResult result)
				{
					if (result == notify::SyncResult::success)
					{
						auto storage = storage_.lock();
						auto tail = storage->GetLastMessages(chat_id, tail_len);
						std::vector<ChatMessageID> res;
						std::transform(tail.begin(), tail.end(), std::back_inserter(res),
							[](auto &item){ return std::move(item.msg_id);});
						cb(cb::ErrorCode::success, std::move(res));
					}
					else
					{
						cb(cb::ErrorCode::failed, {});
					}
				});
			}
		}
		return true;
	}
	return false;
}
template<typename LayersTraits>
void IntegrityLayer<LayersTraits>::SyncChatTail(
	ChatIDRef chat_id, uint32_t tail_len,
	CallIDRef with_who, const cb::ChatMsgIDChainCallBack &cb)
{
	/**
		send checksync
	*/
	/**
	 * req tail_hash with len
	 * compare hash with local
	 * if eq => read tail from db
	 * else sync tail;
	 * */
	if (!cb)
		return SyncChatTail(chat_id, tail_len, with_who, [](cb::ErrorCode, std::vector<ChatMessageID>&&) {});
	this->PostCall(
		[
			this,
			chat_id = ChatID(chat_id),
			tail_len,
			with_who = CallID(with_who),
			cb
		]()
	{
		auto it = pending_get_tail_hash_reqs_.emplace(chat_id,
				get_tail_hash_req_cb(
					std::chrono::steady_clock::now() + g_max_req_duration,
					cb));
		if(!it.second)
		{
			cb(cb::ErrorCode::pending,{});
		}
		auto get_tail = msg::GetTailHashReqMessage(
				chat_id,tail_len,
				current_account_->GetExactCallID(),
				with_who);
		this->Send(get_tail.AcquireMessage(), {});
	});
}
template<typename LayersTraits>
void IntegrityLayer<LayersTraits>::ShutDown()
{
	LayerHelper<LayersTraits>::ShutDown();
}
}
#undef DEBUG_CURRENT_MODULE
