#pragma once
#include "chatlib/layers/ChatLayerAbstract.h"

#include "std-generic/compat/functional.h"
#include "std-generic/compat/map.h"
#include "std-generic/compat/set.h"
#include "std-generic/cpplib/macro_utils.h"

namespace chat
{
namespace detail
{

struct MsgInfoForConfirmSt
{
	VS_FORWARDING_CTOR3(MsgInfoForConfirmSt, call_id, msg_id, bs) {}
	CallID        call_id;
	ChatMessageID msg_id;
	// bs used only for search in confirm_by_bs_ and clean record.
	// It is cause why bs field isn't used in less-operator.
	BSInfo bs;
	bool operator<(const MsgInfoForConfirmSt &rhs) const
	{
		return std::tie(call_id, msg_id)
			< std::tie(rhs.call_id, rhs.msg_id);
	}
};
struct MsgInfoForConfirmRef
{
	VS_FORWARDING_CTOR3(MsgInfoForConfirmRef, call_id, msg_id, bs) {}
	CallIDRef        call_id;
	ChatMessageIDRef msg_id;
	BSInfoRef bs;
	bool operator<(const MsgInfoForConfirmRef &rhs) const
	{
		return std::tie(call_id, msg_id)
			< std::tie(rhs.call_id, rhs.msg_id);
	}
	explicit operator MsgInfoForConfirmSt() const
	{
		return { call_id, msg_id, bs };
	}
};
struct call_id_info
{
	VS_FORWARDING_CTOR2(call_id_info, call_id, type) {}
	CallID call_id;
	enum id_type
	{
		common,
		endpoint
	} type;
	bool operator<(const call_id_info &other) const
	{
		return call_id < other.call_id;
	}
};
}
template<typename LayersTraits>
class DeliveryLayer
	: public LayerInterface
	, public LayerHelper<LayersTraits>
{
	const std::chrono::seconds m_timeout_duration = std::chrono::seconds(1);
	const std::chrono::seconds m_retransmit_timeout = std::chrono::seconds(5);
	const std::chrono::seconds m_stop_retransmit_timeout = std::chrono::seconds(60);

	void ForwardBelowMessage(msg::ChatMessagePtr&& msg,
		std::vector<ParticipantDescr>&& dstParts) override;
	void OnChatMessageArrived(msg::ChatMessagePtr&&m, CallIDRef sender) override;
	void ShutDown() override;

	void AddRelatedEndpoints(vs::set<detail::call_id_info> &dst) const;
	void MessageDelivered(
		const ChatMessageID& msg_id,
		const CallID& from,
		const CallID& from_instance);
	void SendDeliveryConfirm(ChatMessageIDRef msgId, CallIDRef to);
	void Timeout();
	// if true - retransmit stopped
	bool StopRetransmitAchived(const msg::ChatMessage& msg);

	void StartDelivery(const msg::ChatMessage& msg, CallIDRef to,
		BSInfoRef bs, std::chrono::steady_clock::time_point when_send = {},
		std::chrono::steady_clock::time_point when_stop = {});
	void StartDeliveryPartList(msg::ChatMessagePtr&& msg, std::vector<ParticipantDescr>&& part_list);
	void ProcessConfirm(const CallID& call_id, const ChatMessageID& msg_id);
	void SendWithoutConfirm(msg::ChatMessagePtr&& msg, std::vector<ParticipantDescr>&& parts);
	GlobalConfigPtr GetCfg() const;

	std::weak_ptr<GlobalConfigInterface> cfg_;
	AccountInfoPtr current_account_;
	ChatStoragePtr storage_;
	ChatEventsNotifierPtr notifier_;
	ClockWrapperPtr	clock_wrap_;
	vs::map<
		detail::MsgInfoForConfirmSt,
		std::shared_ptr<msg::ChatMessage>,
		vs::less<>> wait_confirm_;

	using MsgInfoForConfirmSet =
		vs::set<detail::MsgInfoForConfirmSt, vs::less<>>;
	using MsgInfoForConfirmByMsgID =
		vs::map<ChatMessageID, MsgInfoForConfirmSet, vs::istr_less>;
	vs::map<
		BSInfo,
		MsgInfoForConfirmByMsgID,
		vs::istr_less> confirm_by_bs_;
	struct WaitConfirmUntil
	{
		VS_FORWARDING_CTOR3(WaitConfirmUntil, msg, retransmitAfter, stopRetransmitAfter)
		{}
		std::weak_ptr<msg::ChatMessage> msg;
		std::chrono::steady_clock::time_point retransmitAfter;
		std::chrono::steady_clock::time_point stopRetransmitAfter;
		uint32_t counter = 0;
		bool operator<(const WaitConfirmUntil &src) const
		{
			return retransmitAfter > src.retransmitAfter;
		}
	};
	std::priority_queue <WaitConfirmUntil> wait_confirm_by_timeout_;
public:
	template<typename ... Args>
	DeliveryLayer(const GlobalConfigPtr &cfg, Args&&...args);
};
}
