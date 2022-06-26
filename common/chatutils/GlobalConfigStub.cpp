#include "GlobalConfigStub.h"

void GlobalConfigStub::SetChatStorage(const chat::ChatStoragePtr& storage)
{
	chat_storage_ = storage;
}
void GlobalConfigStub::SetSyncChat(
	const std::shared_ptr<chat::SyncChatInterface> &sync_chat)
{
	sync_chat_= sync_chat;
}
void GlobalConfigStub::SetAccountInfo(const chat::AccountInfoPtr & account)
{
	current_account_ = account;
}
void GlobalConfigStub::SetClockWrapper(const chat::ClockWrapperPtr & clock)
{
	clock_wrapper_ = clock;
}
void GlobalConfigStub::SetMaxChainLen(uint32_t len)
{
	max_chain_len_ = len;
}
void GlobalConfigStub::SetBucketCapacity(uint32_t capacity)
{
	bucket_capacity_ = capacity;
}
void GlobalConfigStub::SetTailLength(uint32_t tail_len)
{
	tail_length_ = tail_len;
}
chat::ChatStoragePtr GlobalConfigStub::GetChatStorage() const
{
	return chat_storage_.lock();
}
chat::ChatEventsNotifierPtr GlobalConfigStub::GetEventsNotifier() const
{
	return chat_events_;
}
chat::ChatEventsSubscriptionPtr GlobalConfigStub::GetEventsSubscription() const
{
	return chat_events_;
}
chat::AccountInfoPtr GlobalConfigStub::GetCurrentAccount() const
{
	return current_account_;
}
chat::ClockWrapperPtr GlobalConfigStub::GetClockWrapper() const
{
	return clock_wrapper_;
}
vs::ResolverPtr GlobalConfigStub::GetResolver() const
{
	return resolver_;
}
uint32_t GlobalConfigStub::GetMaxChainLen() const
{
	return max_chain_len_;
}
uint32_t GlobalConfigStub::GetBucketCapacity() const
{
	return bucket_capacity_;
}
uint32_t GlobalConfigStub::GetTailLength() const
{
	assert(tail_length_ > 0);
	return tail_length_;
}
std::shared_ptr<chat::SyncChatInterface>
GlobalConfigStub::GetSyncChat() const
{
	return sync_chat_.lock();
}