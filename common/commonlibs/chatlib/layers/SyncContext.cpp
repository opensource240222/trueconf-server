#include "SyncContext.h"

#include "chatlib/helpers/AccountInfo.h"
#include "chatlib/interface/GlobalConfigInterface.h"
#include "chatlib/msg/attr.h"
#include "chatlib/notify/GlobalChatEvents.h"
#include "std-generic/cpplib/VS_Container_io.h"

#include "chatlib/log/chatlog.h"

#include <random>

namespace chat
{
namespace detail
{
namespace
{
std::random_device s_rand_device;
auto s_sync_duration_limit = std::chrono::seconds(30);
}
void sync_context::on_chain_updated(const msg::ChatMessagePtr& msg)
{
	auto msg_id = msg->GetParamStrRef(attr::MESSAGE_ID_paramName);
	auto chat_id = msg->GetParamStrRef(attr::CHAT_ID_paramName);
	CHAT_TRACE(log_stream << "sync_context::on_chain_updated; sync_id = "
		<< sync_id_ << "; chat_id = "
		<< chat_id_ <<"; msg_id = "
		<< msg_id << "; call_id = "
		<< call_id_ << "; my_call_id = "
		<< current_account_->GetExactCallID() << "; state_ = "
		<< int(state_));
	if (chat_id != chat_id_)
		return;
	auto iter = expected_insert_msg_.find(msg_id);
	if (iter == expected_insert_msg_.end())
	{
		auto method = msg->GetParamStrRef(attr::BUCKETSYNC_METHOD_paramName);
		reset_sync(
			method == attr::INSERT_MSG
			? notify::SyncResult::reset_by_sync_race
			: notify::SyncResult::reset_by_update);
		return;
	}
	iter->second = expected_msg_st::in_chain;
	if (!diff_msg_ids_.empty())
		try_sync_check();
}
sync_context::sync_context(const GlobalConfigPtr &cfg,
	// cppcheck-suppress passedByValue ; move semantic used
	string_view sync_id, ChatIDRef chat_id, CallIDRef call_id,
	uint32_t tail_len, ChatMessageIDRef init_msg,
	string_view tail_hash,
	const GetTailBucketListFunc &get_tail_bucket_lst,
	const GetMessagesInBucketFunc& get_msgs_in_bucket,
	const GetMessagesToRetransmitFunc& get_msgs_to_retransmit,
	const GetTailHashFunc &get_tail_hash,
	const SendToFunc& send_to_fn,
	const SyncResultCallBack &result_cb)
	: cfg_(cfg), current_account_(cfg->GetCurrentAccount())
	, sync_id_(std::move(sync_id))
	, chat_id_(std::move(chat_id))
	, call_id_(std::move(call_id))
	, msgs_in_tail_(tail_len)
	, tail_hash_(tail_hash)
	, get_tail_bucket_list_(get_tail_bucket_lst)
	, get_msgs_in_bucket_(get_msgs_in_bucket)
	, get_msgs_to_retransmit_(get_msgs_to_retransmit)
	, get_tail_hash_(get_tail_hash)
	, send_to_(send_to_fn)
	, cb_sync_result_(result_cb)
	, clock_wrapper_(cfg->GetClockWrapper())
	, start_point_(clock_wrapper_->now())
{
	on_chain_upd_conn_ = cfg->GetEventsSubscription()->SubscribeToChainUpdated(
		[
			this,
			tracker = std::weak_ptr<char>(lifetime_tracker_)
		](const msg::ChatMessagePtr& msg)
	{
		auto lock = tracker.lock();
		if (lock)
			on_chain_updated(msg);
	});
	if (!init_msg.empty())
		expected_insert_msg_ =
	{ { ChatMessageID(init_msg),expected_msg_st::arrived } };
}
sync_context::~sync_context()
{
	on_chain_upd_conn_.disconnect();
}
void sync_context::start()
{
	CHAT_TRACE(log_stream << "sync_context::start; sync_id = "
		<< sync_id_ << "; chat_id = "
		<< chat_id_ << "; call_id = "
		<< call_id_ << "; my_call_id = "
		<< current_account_->GetExactCallID());
	if (clock_wrapper_)
		start_point_ = clock_wrapper_->now();
	req_buckets_id(true);
}
void sync_context::req_buckets_id(bool sync_start)
{
	msg::BucketSyncMessage m(sync_id_);
	m.MakeBucketListReq(
		chat_id_,
		current_account_->GetExactCallID(),
		msgs_in_tail_, tail_hash_, sync_start);
	state_ = sync_state::bucket_list;
	CHAT_TRACE(log_stream << "sync_context::req_buckets_id; sync_id = "
		<< sync_id_ << "; chat_id = "
		<< chat_id_ << "; call_id = "
		<< call_id_ << "; my_call_id = "
		<< current_account_->GetExactCallID() << "; msgs_in_tail_ = "
		<< msgs_in_tail_ << "; tail_hash_ = "
		<< tail_hash_ << "; sync_start = "
		<< (sync_start ? "true" : "false ") << "; state_ = "
		<< int(state_));
	send_to_(call_id_, m.AcquireMessage());
}
bool sync_context::processing_sync_bucket_list_req(const msg::ChatMessagePtr &m)
{
	if (!get_tail_bucket_list_)
		return true;
	CHAT_TRACE(log_stream << "sync_context::processing_sync_bucket_list_req; sync_id = "
		<< sync_id_ << "; chat_id = "
		<< chat_id_ << "; call_id = "
		<< call_id_ << "; my_call_id = "
		<< current_account_->GetExactCallID() << "; state_ = "
		<< int(state_));
	uint32_t tail_len(0);
	if (!m->GetParamI32(attr::TAIL_LENGTH_paramName, tail_len))
		tail_len = get_cfg()->GetTailLength();
	if (state_ < sync_state::bucket_list)
	{
		auto my_tail_hash = get_tail_hash_(chat_id_, tail_len);
		auto hash_from_msg = m->GetParamStrRef(attr::TAIL_HASH_paramName);
		assert(!hash_from_msg.empty());
		if (my_tail_hash.tail_hash == hash_from_msg)
		{
			msg::BucketSyncMessage result(sync_id_);
			result.MakeCheckSyncResp(
				chat_id_,
				current_account_->GetExactCallID(),
				msg::BucketSyncMessage::SyncRes::ok);
			CHAT_TRACE(log_stream << "Already synced. Good!");
			send_to_(call_id_, result.AcquireMessage());
			done(notify::SyncResult::success);
			return true;
		}
		tail_hash_ = my_tail_hash.tail_hash;
	}
	auto lst = get_tail_bucket_list_(chat_id_, tail_len);
	msg::BucketSyncMessage response(sync_id_);
	response.MakeBucketListResp(
		chat_id_,
		current_account_->GetExactCallID(),
		std::move(lst));
	send_to_(call_id_, response.AcquireMessage());
	if (state_ < sync_state::bucket_list)
		req_buckets_id(false);
	return true;
}
bool sync_context::processing_sync_bucket_list_resp(const msg::ChatMessagePtr& m)
{
	CHAT_TRACE(log_stream << "sync_context::processing_sync_bucket_list_resp; sync_id = "
		<< sync_id_ << "; chat_id = "
		<< chat_id_ << "; call_id = "
		<< call_id_ << "; my_call_id = "
		<< current_account_->GetExactCallID() << "; state_ = "
		<< int(state_));
	if (state_ == sync_state::bucket_list)
	{
		if (!msg::BucketSyncMessage::IsMyMessage(m))
		{
			assert(false);
			reset_sync(notify::SyncResult::failed);
			return false;
		}
		auto recv_buckets = msg::BucketSyncMessage::GetBucketList(m);
		auto my_buckets = get_tail_bucket_list_(chat_id_, msgs_in_tail_);
		auto min_bucket_recv = std::min_element(
			recv_buckets.begin(), recv_buckets.end(),
			[](const msg::BucketSyncMessage::BucketIdHash& a,
				const msg::BucketSyncMessage::BucketIdHash& b)
		{
			return a.id < b.id;
		});
		auto min_bucket_my = std::min_element(
			my_buckets.begin(), my_buckets.end(),
			[](const msg::BucketSyncMessage::BucketIdHash& a,
				const msg::BucketSyncMessage::BucketIdHash& b)
		{
			return a.id < b.id;
		});
		if (min_bucket_recv != recv_buckets.end()
			|| min_bucket_my != my_buckets.end())
		{
			min_bucket_id_ = min_bucket_recv == recv_buckets.end()
				? min_bucket_my->id
				: min_bucket_my == my_buckets.end()
				? min_bucket_recv->id
				: std::min(min_bucket_recv->id, min_bucket_my->id);
		}
		std::vector<uint64_t> id_for_sync;
		decltype(recv_buckets) for_sync;
		std::set_difference(recv_buckets.begin(), recv_buckets.end(),
			my_buckets.begin(), my_buckets.end(),
			std::back_inserter(for_sync));
		std::transform(for_sync.begin(), for_sync.end(),
			std::back_inserter(id_for_sync),
			[](const msg::BucketSyncMessage::BucketIdHash &transfer)
		{return transfer.id; });
		if (id_for_sync.empty())
		{
			if (recv_buckets.size() == my_buckets.size())
			{
				CHAT_TRACE(log_stream << "id_for_sync is empty, current buckets is syncronized, try sync previous bucket;");
				if (min_bucket_id_ > 0 && min_bucket_id_ != UINT32_MAX)
					id_for_sync.push_back(--min_bucket_id_);
				else
				{
					assert(false);
					reset_sync(notify::SyncResult::failed);
					return false;
				}
			}
			else // peer has missing buckets. Wait DiffReq from peer;
			{
				state_ = sync_state::diff_resp_got;
				req_sync_check();
				return true;
			}
		}
		req_diff_for_buckets(id_for_sync);
	}
	else
	{
		reset_sync(notify::SyncResult::failed);
		return false;
	}
	return true;
}
bool sync_context::processing_get_diff_req(const msg::ChatMessagePtr& m)
{
	CHAT_TRACE(log_stream << "sync_context::processing_get_diff_req; sync_id = "
		<< sync_id_ << "; chat_id = "
		<< chat_id_ << "; call_id = "
		<< call_id_ << "; my_call_id = "
		<< current_account_->GetExactCallID() << "; state_ = "
		<< int(state_));
	auto req_info = msg::BucketSyncMessage::GetDiffReqInfo(m);
	if (req_info.empty())
	{
		reset_sync(notify::SyncResult::failed);
		return false;
	}
	auto missing_msgs = get_msgs_to_retransmit_(chat_id_, req_info);
	std::vector<ChatMessageID> ins_msgs;
	for (auto&& m : missing_msgs)
	{
		assert(m);
		if (!m)
			continue;
		CHAT_TRACE(log_stream << "insert msg = " << m->GetContainer());
		m->SetParam(attr::BUCKETSYNC_METHOD_paramName,
			attr::INSERT_MSG);
		m->SetParam(attr::SYNC_ID_paramName, sync_id_);
		ins_msgs.emplace_back(m->GetParamStr(attr::MESSAGE_ID_paramName));
		send_to_(call_id_, std::move(m));
	}
	msg::BucketSyncMessage resp(sync_id_);
	resp.MakeDiffResp(chat_id_, current_account_->GetExactCallID(), ins_msgs);
	send_to_(call_id_, resp.AcquireMessage());
	return true;
}

bool sync_context::processing_get_diff_resp(const msg::ChatMessagePtr& m)
{
	CHAT_TRACE(log_stream << "sync_context::processing_get_diff_resp; sync_id = "
		<< sync_id_ << "; chat_id = "
		<< chat_id_ << "; call_id = "
		<< call_id_ << "; my_call_id = "
		<< current_account_->GetExactCallID() << "; state_ = "
		<< int(state_));
	if (state_ == sync_state::wait_diff_resp
		&& msg::BucketSyncMessage::IsMyMessage(std::move(m)))
	{
		diff_msg_ids_ = msg::BucketSyncMessage::GetDiffMsgIds(m);
		state_ = sync_state::diff_resp_got;
		try_sync_check();
	}
	else
	{
		reset_sync(notify::SyncResult::failed);
		return false;
	}
	return true;
}

bool sync_context::processing_sync_check_req(const msg::ChatMessagePtr& m)
{
	CHAT_TRACE(log_stream << "sync_context::processing_sync_check_req; sync_id = "
		<< sync_id_ << "; chat_id = "
		<< chat_id_ << "; call_id = "
		<< call_id_ << "; my_call_id = "
		<< current_account_->GetExactCallID() << "; state_ = "
		<< int(state_));
	if (!msg::BucketSyncMessage::IsMyMessage(m))
		return false;
	auto info = msg::BucketSyncMessage::GetCheckSyncInfo(m);
	if (info.chatId != chat_id_ || info.from != call_id_)
		return false;
	check_sync_peer_.hash_ = info.hash;
	check_sync_peer_.master_weight_ = info.masterWeight;
	check_sync_peer_.tail_len_ = info.msgsInTail;
	if (state_ < sync_state::check_sync)
		return true;
	else if (state_ == sync_state::check_sync)
	{
		if (check_sync_my_.master_weight_ > info.masterWeight)
			//wait response
			return true;
		else if (check_sync_my_.master_weight_ < info.masterWeight)
			process_check_sync(info.msgsInTail, info.hash);
		else if (check_sync_my_.master_weight_ == info.masterWeight)
			req_sync_check();
	}
	return true;
}
bool sync_context::processing_sync_check_resp(const msg::ChatMessagePtr& m)
{
	CHAT_TRACE(log_stream << "sync_context::processing_sync_check_resp; sync_id = "
		<< sync_id_ << "; chat_id = "
		<< chat_id_ << "; call_id = "
		<< call_id_ << "; my_call_id = "
		<< current_account_->GetExactCallID() << "; state_ = "
		<< int(state_));
	if (!msg::BucketSyncMessage::IsMyMessage(m))
		return false;
	auto result = msg::BucketSyncMessage::GetCheckSyncResult(m);
	if (result.from != call_id_ || result.chatId != chat_id_)
	{
		assert(false);
		return false;
	}
	CHAT_TRACE(log_stream << "CheckSyncResult: result = " << int(result.result));
	if (result.result == msg::BucketSyncMessage::SyncRes::ok)
		done(notify::SyncResult::success);
	else if (min_bucket_id_ > 0 && min_bucket_id_ != std::numeric_limits<uint64_t>::max())
		req_diff_for_buckets({ --min_bucket_id_ });
	else
		reset_sync(notify::SyncResult::failed);
	return true;
}
bool sync_context::processing_reset_req()
{
	CHAT_TRACE(log_stream << "sync_context::processing_reset_req; sync_id = "
		<< sync_id_ << "; chat_id = "
		<< chat_id_ << "; call_id = "
		<< call_id_ << "; my_call_id = "
		<< current_account_->GetExactCallID() << "; state_ = "
		<< int(state_));
	done(notify::SyncResult::reset_by_peer);
	return true;
}
bool sync_context::processing_insert_msg(const msg::ChatMessagePtr & m)
{
	CHAT_TRACE(log_stream << "sync_context::processing_insert_msg; sync_id = "
		<< sync_id_ << "; chat_id = "
		<< chat_id_ << "; call_id = "
		<< call_id_ << "; my_call_id = "
		<< current_account_->GetExactCallID() << "; state_ = "
		<< int(state_));
	auto chat_id = m->GetParamStrRef(attr::CHAT_ID_paramName);
	auto msg_id = m->GetParamStrRef(attr::MESSAGE_ID_paramName);
	CHAT_TRACE(log_stream << "chat_id:msg_id = " << chat_id << ":" << msg_id);
	if (chat_id == chat_id_ && !msg_id.empty())
		expected_insert_msg_.emplace(msg_id, expected_msg_st::arrived);
	return true;
}
void sync_context::req_diff_for_buckets(const std::vector<uint64_t>& bucket_id)
{
	msg::MsgIdsByBuckets msg_by_bucket;
	for (const auto &i : bucket_id)
		msg_by_bucket.emplace_back(i, get_msgs_in_bucket_(chat_id_, i));
	msg::BucketSyncMessage diff_req_msg(sync_id_);
	diff_req_msg.MakeDiffReq(chat_id_,
		current_account_->GetExactCallID(),
		msg_by_bucket);
	state_ = sync_state::wait_diff_resp;
	send_to_(call_id_, diff_req_msg.AcquireMessage());
}
void sync_context::req_sync_check()
{
	CHAT_TRACE(log_stream << "sync_context::req_sync_check; sync_id = "
		<< sync_id_ << "; chat_id = "
		<< chat_id_ << "; call_id = "
		<< call_id_ << "; my_call_id = "
		<< current_account_->GetExactCallID() << "; state_ = "
		<< int(state_));
	msg::BucketSyncMessage check_sync(sync_id_);
	if (check_sync_peer_.tail_len_ > 0)
		process_check_sync(check_sync_peer_.tail_len_, check_sync_peer_.hash_);
	else
	{
		//wait all inserted msgs
		auto hash = get_tail_hash_(chat_id_, msgs_in_tail_);
		check_sync_my_.hash_ = std::move(hash.tail_hash);
		check_sync_my_.tail_len_ = hash.tail_len;
		std::uniform_int_distribution<uint32_t> distr;
		check_sync_my_.master_weight_ = distr(s_rand_device);
		check_sync.MakeCheckSyncReq(
			chat_id_, current_account_->GetExactCallID(),
			check_sync_my_.tail_len_, check_sync_my_.hash_,
			check_sync_my_.master_weight_);
		state_ = sync_state::check_sync;
		send_to_(call_id_, check_sync.AcquireMessage());
	}
}
void sync_context::try_sync_check()
{
	CHAT_TRACE(log_stream << "sync_context::try_sync_check; sync_id = "
		<< sync_id_ << "; chat_id = "
		<< chat_id_ << "; call_id = "
		<< call_id_ << "; my_call_id = "
		<< current_account_->GetExactCallID() << "; state_ = "
		<< int(state_));
	if (state_ != sync_state::diff_resp_got)
		return;
	for (const auto &i : diff_msg_ids_)
	{
		auto iter = expected_insert_msg_.find(i);
		if (iter == expected_insert_msg_.end()
			|| iter->second != expected_msg_st::in_chain)
		{
			return;
		}
	}
	req_sync_check();
}
void sync_context::process_check_sync(uint32_t tail_len, const std::string &hash)
{
	CHAT_TRACE(log_stream << "sync_context::process_check_sync; sync_id = "
		<< sync_id_ << "; chat_id = "
		<< chat_id_ << "; call_id = "
		<< call_id_ << "; my_call_id = "
		<< current_account_->GetExactCallID() << "; state_ = "
		<< static_cast<int>(state_));
	auto tail = get_tail_hash_(chat_id_, tail_len);
	if (tail.tail_hash == hash && tail.tail_len == tail_len)
	{
		msg::BucketSyncMessage result(sync_id_);
		result.MakeCheckSyncResp(
			chat_id_,
			current_account_->GetExactCallID(),
			msg::BucketSyncMessage::SyncRes::ok);
		CHAT_TRACE(log_stream << "Sync finished. Good!");
		send_to_(call_id_, result.AcquireMessage());
		done(notify::SyncResult::success);
	}
	else
	{
		msg::BucketSyncMessage result(sync_id_);
		result.MakeCheckSyncResp(
			chat_id_,
			current_account_->GetExactCallID(),
			msg::BucketSyncMessage::SyncRes::failed);
		CHAT_TRACE(log_stream << "Sync failed! Try previous bucket. Current min bucket = " << min_bucket_id_);
		send_to_(call_id_, result.AcquireMessage());
		if (min_bucket_id_ > 0 && min_bucket_id_ != std::numeric_limits<uint64_t>::max())
			req_diff_for_buckets({ --min_bucket_id_ });
		else
		{
			assert(false);
			reset_sync(notify::SyncResult::failed);
			// done failed
		}
	}
}
void sync_context::reset_sync(notify::SyncResult reason)
{
	CHAT_TRACE(log_stream << "sync_context::reset_sync; sync_id = "
		<< sync_id_ << "; chat_id = "
		<< chat_id_ << "; call_id = "
		<< call_id_ << "; my_call_id = "
		<< current_account_->GetExactCallID() << "; reason = "
		<< static_cast<int>(reason) <<"; state_ = "
		<< static_cast<int>(state_));
	msg::BucketSyncMessage reset(sync_id_);
	reset.MakeSyncResetReq(chat_id_, current_account_->GetExactCallID());
	send_to_(call_id_, reset.AcquireMessage());
	done(reason);
}
void sync_context::done(notify::SyncResult res)
{
	state_ = sync_state::sync_finish;
	sync_result_ = res;
	on_chain_upd_conn_.disconnect();
	cb_sync_result_(res);
}
bool sync_context::processing(const msg::ChatMessagePtr &m, CallIDRef sender)
{
	if (!msg::BucketSyncMessage::IsMyMessage(m) || sender != call_id_)
		return false;
	auto method = m->GetParamStrRef(attr::BUCKETSYNC_METHOD_paramName);
	if (method.empty())
		return true; //skip message
	if (method == attr::SYNC_BUCKET_LIST_REQ)
		return processing_sync_bucket_list_req(m);
	else if (method == attr::SYNC_BUCKET_LIST_RESP)
		return processing_sync_bucket_list_resp(m);
	else if (method == attr::GET_DIFF_MSG_REQ)
		return processing_get_diff_req(m);
	else if (method == attr::GET_DIFF_MSG_RESP)
		return processing_get_diff_resp(m);
	else if (method == attr::SYNC_CHECK_REQ)
		return processing_sync_check_req(m);
	else if (method == attr::SYNC_CHECK_RESP)
		return processing_sync_check_resp(m);
	else if (method == attr::RESET_SYNC_REQ)
		return processing_reset_req();
	else if (method == attr::INSERT_MSG)
		return processing_insert_msg(m);
	return false;
}
void sync_context::Tick()
{
	if (clock_wrapper_->now() - start_point_ >= s_sync_duration_limit)
		reset_sync(notify::SyncResult::reset_by_timeout);
}
GlobalConfigPtr sync_context::get_cfg() const
{
	auto cfg = cfg_.lock();
	assert(cfg);
	return cfg;
}
}
}
