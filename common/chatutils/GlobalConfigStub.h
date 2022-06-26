#include "chatlib/chat_defs.h"
#include "chatlib/interface/GlobalConfigInterface.h"
#include "chatlib/notify/ChatEventsFuncs.h"


class GlobalConfigStub : public chat::GlobalConfigInterface
{
	vs::ResolverPtr resolver_;
	std::shared_ptr<chat::notify::ChatEventsFuncs> chat_events_;
	chat::ChatEventsSubscriptionPtr event_subscr_;
	std::weak_ptr<chat::ChatStorage> chat_storage_;
	std::weak_ptr<chat::SyncChatInterface> sync_chat_;
	chat::AccountInfoPtr current_account_;
	chat::ClockWrapperPtr clock_wrapper_;
	uint32_t max_chain_len_;
	uint32_t bucket_capacity_;
	uint32_t tail_length_;

public:
	GlobalConfigStub(
		const vs::ResolverPtr& resolver,
		const std::shared_ptr<chat::notify::ChatEventsFuncs> &chat_events
			= std::make_shared<chat::notify::ChatEventsFuncs>())
		: resolver_(resolver)
		, chat_events_(chat_events)
		, max_chain_len_(0)
		, bucket_capacity_(0)
		, tail_length_(0)
	{}
	void SetAccountInfo(const chat::AccountInfoPtr &account);
	void SetChatStorage(const chat::ChatStoragePtr&);
	void SetSyncChat(const std::shared_ptr<chat::SyncChatInterface> & sync_chat);
	void SetClockWrapper(const chat::ClockWrapperPtr &clock);
	void SetMaxChainLen(uint32_t len);
	void SetBucketCapacity(uint32_t capacity);
	void SetTailLength(uint32_t tail_len);


	chat::AccountInfoPtr            GetCurrentAccount() const override;
	chat::ChatEventsNotifierPtr     GetEventsNotifier() const override;
	chat::ChatEventsSubscriptionPtr GetEventsSubscription() const override;
	chat::ChatStoragePtr            GetChatStorage() const override;
	chat::ClockWrapperPtr           GetClockWrapper() const override;
	vs::ResolverPtr                 GetResolver() const override;
	uint32_t                        GetMaxChainLen() const override;
	uint32_t                        GetBucketCapacity() const override;
	uint32_t                        GetTailLength() const override;
	std::shared_ptr<
		chat::SyncChatInterface>    GetSyncChat() const override;
};