#include "BucketsOfMessages.h"
#include "chatlib/interface/GlobalConfigInterface.h"
#include "chatlib/msg/ChatMessage.h"
#include "chatlib/msg/attr.h"
#include "chatlib/storage/ChatStorage.h"

namespace chat
{
namespace detail
{
BucketInfo::BucketInfo(
	const ChatID &id,
	const uint32_t capacity,
	const uint64_t last_bucket,
	const uint32_t count)
	: id_(id)
	, bucket_capacity_(capacity)
	, current_bucket_(last_bucket)
	, counter_(count)
{
}
BucketInfo::BucketInfo(BucketInfo&&src)
	: id_(std::move(src.id_))
	, bucket_capacity_(std::move(src.bucket_capacity_))
	, current_bucket_(std::move(src.current_bucket_))
	, counter_(std::move(src.counter_))
{}

bool BucketInfo::PutMessage(const msg::ChatMessagePtr&m, bool is_our_msg)
{
	if (is_our_msg)
	{
		m->SetParamI64(attr::BUCKET_paramName, current_bucket_);
		auto current_bucket = current_bucket_;
		m->AddOnMsgIsStoredCallBack([this, current_bucket](
			cb::ProcessingResult r,
			ChatIDRef,
			ChatMessageIDRef)
			{
				if (cb::ProcessingResult::ok == r)
				{
					std::lock_guard<std::mutex> lock(counter_lock_);
					IncrementCounter(current_bucket);
				}
			});
	}
	else
	{
		int32_t bucket(0);
		if (!m->GetParamI64(attr::BUCKET_paramName, bucket))
			return false;
		std::lock_guard<std::mutex> lock(counter_lock_);
		if (static_cast<uint32_t>(bucket) > current_bucket_)
		{
			current_bucket_ = bucket;
			counter_ = 1;
		}
		else if (static_cast<uint32_t>(bucket) == current_bucket_)
			IncrementCounter(bucket);
	}
	return true;
}
void BucketInfo::IncrementCounter(const uint64_t current_bucket)
{
	if (current_bucket == current_bucket_)
	{
		++counter_;
		if (counter_ >= bucket_capacity_)
		{
			++current_bucket_;
			counter_ = 0;
		}
	}
}
uint64_t BucketInfo::GetCurrentBucket()const
{
	return current_bucket_;
}
}
BucketsOfMessages::BucketsOfMessages() : bucket_capacity_(0)
{}
void BucketsOfMessages::Init(const GlobalConfigPtr &cfg)
{
	global_config_ = cfg;
	storage_ = cfg->GetChatStorage();
	bucket_capacity_ = cfg->GetBucketCapacity();
}
bool BucketsOfMessages::PutMessage(
	const msg::ChatMessagePtr&msg,
	const bool is_our_msg)
{
	ChatID chat_id;
	if (!msg->GetParam(attr::CHAT_ID_paramName, chat_id))
		return false;
	auto iter = buckets_by_chat_.find(chat_id);
	if (iter == buckets_by_chat_.end())
	{
		uint64_t last_bucket(0);
		uint32_t count_msg(0);
		auto g_cfg = global_config_.lock();
		auto storage = storage_.lock();
		if (!g_cfg || !storage)
			return false;
		storage->GetLastBucketInfo(chat_id,last_bucket,count_msg);
		iter = buckets_by_chat_.emplace(
			chat_id,
			detail::BucketInfo(
				chat_id,
				bucket_capacity_,
				last_bucket,
				count_msg)).first;
	}
	return iter->second.PutMessage(msg, is_our_msg);
}
uint64_t BucketsOfMessages::GetCurrentBucket(ChatIDRef id) const
{
	auto iter = buckets_by_chat_.find(id);
	return iter == buckets_by_chat_.end()
		? 0
		: iter->second.GetCurrentBucket();
}
}