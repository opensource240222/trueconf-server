#pragma once
#include "chatlib/chat_defs.h"

#include "std-generic/compat/functional.h"
#include "std-generic/compat/map.h"
#include "std-generic/cpplib/StrCompare.h"

#include <mutex>
namespace chat
{
namespace detail
{
class BucketInfo
{
	const ChatID id_;
	uint32_t bucket_capacity_;
	uint64_t current_bucket_;
	uint32_t counter_;
	std::mutex counter_lock_;
	void IncrementCounter(const uint64_t current_bucket);
public:
	BucketInfo& operator=(const BucketInfo&) = delete;
	BucketInfo(const BucketInfo&) = delete;
	BucketInfo(BucketInfo&&);
	BucketInfo(
		const ChatID &id,
		const uint32_t capacity,
		const uint64_t last_bucket = 0,
		const uint32_t count = 0);
	bool PutMessage(const msg::ChatMessagePtr&, const bool is_our_msg);
	uint64_t GetCurrentBucket() const;
};
}
class BucketsOfMessages
{
	std::weak_ptr<GlobalConfigInterface> global_config_;
	std::weak_ptr<ChatStorage> storage_;
	vs::map<ChatID, detail::BucketInfo, vs::str_less> buckets_by_chat_;
	uint32_t	bucket_capacity_;
public:
	BucketsOfMessages();
	void Init(const GlobalConfigPtr &cfg);
	bool PutMessage(const msg::ChatMessagePtr &msg, bool is_our_msg);
	uint64_t GetCurrentBucket(ChatIDRef) const;
};
}