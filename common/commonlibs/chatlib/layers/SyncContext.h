#pragma once

#include "chatlib/chat_defs.h"
#include "chatlib/msg/chat_messages_construct.h"
#include "chatlib/notify/notify_cb.h"

#include "std-generic/compat/map.h"
#include "std-generic/cpplib/macro_utils.h"
#include "std-generic/cpplib/StrCompare.h"

namespace chat
{
namespace detail
{
struct tail_hash_info
{
	VS_FORWARDING_CTOR3(tail_hash_info, is_exist, tail_len, tail_hash) {}
	tail_hash_info()
		: is_exist(false)
		, tail_len(0)
	{}
	bool is_exist;
	uint32_t tail_len;
	std::string tail_hash;
};

class sync_context
{
	static const std::chrono::steady_clock::duration s_sync_max_duration_;

	using GetMessagesToRetransmitFunc = std::function<
		std::vector<msg::ChatMessagePtr>(const ChatID&,
			const msg::MsgIdsByBuckets&)>;
	using GetMessagesInBucketFunc = std::function<
		std::vector<ChatMessageID>(const ChatID&, uint64_t)>; // id -> hash
	using GetTailBucketListFunc = std::function<
		std::vector<msg::BucketSyncMessage::BucketIdHash>(
			const ChatID&,
			uint32_t)>;
	using GetTailHashFunc = std::function<
		tail_hash_info(
			const ChatID& chat_id,
			uint32_t msgs_in_tail)>;
	using SendToFunc = std::function<void(CallIDRef, msg::ChatMessagePtr&&)>;
	using SyncResultCallBack = std::function<void(notify::SyncResult)>;

	enum class sync_state
	{
		none = 0,
		bucket_list,
		wait_diff_resp,
		diff_resp_got,
		check_sync,
		sync_finish
	};
	std::weak_ptr<GlobalConfigInterface> cfg_;
	AccountInfoPtr current_account_;
	std::string sync_id_;
	ChatID chat_id_;
	CallID call_id_;
	uint32_t msgs_in_tail_ = 0;
	sync_state state_ = sync_state::none;
	std::string tail_hash_;

	GetTailBucketListFunc get_tail_bucket_list_;
	GetMessagesInBucketFunc get_msgs_in_bucket_;
	GetMessagesToRetransmitFunc get_msgs_to_retransmit_;
	GetTailHashFunc get_tail_hash_;
	SendToFunc send_to_;
	SyncResultCallBack cb_sync_result_;

	ClockWrapperPtr	clock_wrapper_;
	std::chrono::steady_clock::time_point start_point_;
	vs::SubscribeConnection on_chain_upd_conn_;
	enum class expected_msg_st
	{
		arrived,
		in_chain,
	};
	vs::map<ChatMessageID, expected_msg_st, vs::str_less> expected_insert_msg_;
	vs::set<ChatMessageID> diff_msg_ids_;

	uint64_t min_bucket_id_ = std::numeric_limits<uint64_t>::max();
	struct check_sync_info
	{
		uint32_t tail_len_ =0;
		std::string hash_;
		uint32_t master_weight_ = 0;
	} check_sync_my_, check_sync_peer_;
	notify::SyncResult sync_result_ = notify::SyncResult::failed;
	std::shared_ptr<char> lifetime_tracker_ = std::make_shared<char>();

	bool processing_get_diff_req(const msg::ChatMessagePtr& m);
	bool processing_get_diff_resp(const msg::ChatMessagePtr& m);
	bool processing_insert_msg(const msg::ChatMessagePtr& m);
	bool processing_reset_req();
	bool processing_sync_bucket_list_req(const msg::ChatMessagePtr& m);
	bool processing_sync_bucket_list_resp(const msg::ChatMessagePtr& m);
	bool processing_sync_check_req(const msg::ChatMessagePtr& m);
	bool processing_sync_check_resp(const msg::ChatMessagePtr& m);
	void process_check_sync(uint32_t tail_len, const std::string &hash);

	void req_buckets_id(bool sync_start);
	void req_diff_for_buckets(const std::vector<uint64_t> &bucket_id);
	void req_sync_check();

	void try_sync_check();

	void reset_sync(notify::SyncResult reason);
	void done(notify::SyncResult res);

	void on_chain_updated(const msg::ChatMessagePtr& msg);
	GlobalConfigPtr get_cfg() const;
public:
	sync_context(
		const GlobalConfigPtr &cfg,
		string_view sync_id,
		ChatIDRef chat_id,
		CallIDRef call_id,
		uint32_t msgs_in_tail,
		ChatMessageIDRef init_msg,
		string_view tail_hash,
		const GetTailBucketListFunc &get_tail_bucket_lst,
		const GetMessagesInBucketFunc& get_msgs_in_bucket,
		const GetMessagesToRetransmitFunc& get_msgs_to_retransmit,
		const GetTailHashFunc &get_tail_hash,
		const SendToFunc& send_to_fn,
		const SyncResultCallBack &result_cb);
	sync_context(sync_context&&src);
	~sync_context();
	void start();
	bool processing(const msg::ChatMessagePtr &m, CallIDRef sender);
	void Tick();
	const ChatID &get_chat_id() const { return chat_id_; }
	const CallID& get_peer_call_id() const { return call_id_; }
};
}
}
