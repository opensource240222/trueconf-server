#pragma once
#include "chatlib/callbacks.h"
#include "chatlib/chat_defs.h"
namespace chat
{
/**
	FIXME: Check interface necessariness.
	Goal is one configured environment for all objects.
	It may be better to pass needed parameters directly to object;
*/
class GlobalConfigInterface
{
public:
	virtual ~GlobalConfigInterface() {}
	virtual ChatStoragePtr GetChatStorage() const = 0;
	virtual ChatEventsNotifierPtr GetEventsNotifier() const = 0;
	virtual ChatEventsSubscriptionPtr GetEventsSubscription() const = 0;
	virtual vs::ResolverPtr GetResolver() const = 0;
	virtual AccountInfoPtr GetCurrentAccount() const = 0;
	virtual ClockWrapperPtr GetClockWrapper() const = 0;
	virtual uint32_t GetMaxChainLen() const = 0;
	virtual uint32_t GetBucketCapacity() const = 0;
	virtual uint32_t GetTailLength() const = 0;
	virtual std::shared_ptr<SyncChatInterface>
		GetSyncChat() const = 0;
};
}
