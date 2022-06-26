#pragma once
#include "chatlib/helpers/vs_def.h"

#include "std-generic/cpplib/macro_utils.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/VS_ClockWrapper.h"

#include <chrono>
#include <functional>
#include <memory>
#include <string>

namespace chat
{
namespace msg
{
class ChatMessage;
using ChatMessagePtr = std::unique_ptr<ChatMessage>;
}
namespace notify
{
class ChatEventsNotifier;
class ChatEventsSubscription;
}
class ChatContextStorageInterface;
class ChatEventsNotifier;
class ChatEventsSubscription;
class ChatStorageInterface;
class ChatStorage;
class GlobalConfigInterface;
class LayerInterface;
class MainMessage;
class SyncChatInterface;
struct GlobalContext;
struct PersonalContext;

enum class InviteResponseCode
{
	undef = -1,
	accept,
	reject,
	timeout,
	failed// message was not saved to storage
};
enum class ChatType
{
	undef,
	p2p,
	symmetric
};
using ParticipantType = vs::CallIDType;

enum class MessageReadState
{
	undef = -1,
	unread = 0,
	read
};
enum class TimerResult
{
	success,
	canceled
};

using AccountInfo = vs::AccountInfo;
using AccountInfoPtr = std::shared_ptr<AccountInfo>;
using BSInfo = vs::BSInfo;
using BSInfoRef = string_view;
using CallID = vs::CallID;
using CallIDRef = string_view;
using ChatEventsNotifierPtr = std::shared_ptr<notify::ChatEventsNotifier>;
using ChatEventsSubscriptionPtr = std::shared_ptr<notify::ChatEventsSubscription>;
using ChatID = std::string;
using ChatIDRef = string_view;
using ChatMessageID = std::string;
using ChatMessageIDRef = string_view;
using ChatMessageForwardTitle = std::string;
using ChatMessageTimestamp = std::chrono::time_point<
	std::chrono::system_clock,
	std::chrono::milliseconds>;
using ChatStoragePtr = std::shared_ptr<ChatStorage>;
using ClockWrapper = steady_clock_wrapper;
using ClockWrapperPtr = std::shared_ptr<ClockWrapper>;
using GlobalConfigPtr = std::shared_ptr<GlobalConfigInterface>;
using LayerInterfacePtr = std::shared_ptr<LayerInterface> ;
using Version = std::string;
using VersionRef = string_view;
struct OrderInChain {
	VS_FORWARDING_CTOR2(OrderInChain, integral, fractional) {}
	OrderInChain() {}
	int64_t integral{};
	std::string fractional;
};
inline bool operator <(const OrderInChain& a, const OrderInChain& b)
{
	return std::tie(a.integral, a.fractional) < std::tie(b.integral, b.fractional);
}
inline bool operator >(const OrderInChain& a, const OrderInChain& b)
{
	return b < a;
}
inline bool operator ==(const OrderInChain& a, const OrderInChain& b)
{
	return std::tie(a.integral, a.fractional) == std::tie(b.integral, b.fractional);
}
using MessageWithOrder = std::pair<msg::ChatMessagePtr, OrderInChain>;
struct chain_item
{
	VS_FORWARDING_CTOR4(chain_item, msg_id, prev_id, timestamp, bucket)
	{}
	chain_item()
	{}
	bool operator==(const chain_item& other) const
	{
		return msg_id == other.msg_id
			&& prev_id == other.prev_id
			&& timestamp == other.timestamp
			&& bucket == other.bucket;
	}
	ChatMessageID msg_id;
	ChatMessageID prev_id;
	ChatMessageTimestamp timestamp;
	uint64_t bucket;
};

static Version current_chat_version = "2";
//FIXME: put to common place with multilogin code
const char CALL_ID_SEPARATOR = '/';
}
