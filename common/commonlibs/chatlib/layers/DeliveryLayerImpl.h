#include "DeliveryLayer.h"
#include "chatlib/helpers/AccountInfo.h"
#include "chatlib/helpers/ExternalComponentsInterface.h"
#include "chatlib/helpers/ResolverInterface.h"

#include "chatlib/interface/GlobalConfigInterface.h"
#include "chatlib/msg/ChatMessage.h"
#include "chatlib/msg/attr.h"
#include "chatlib/msg/chat_messages_construct.h"
#include "chatlib/storage/ChatStorage.h"

#include "std-generic/compat/memory.h"
#include "chatlib/log/chatlog.h"
#include "std-generic/cpplib/VS_RemoveTranscoderID.h"

// For not defining GetMessage -> GetMessage(A/W)
#include "std-generic/undef_windows.h"

namespace chat
{
namespace detail
{
bool operator<(const MsgInfoForConfirmSt& lhs, const MsgInfoForConfirmRef& rhs)
{
	return std::tie(lhs.call_id, lhs.msg_id) < std::tie(rhs.call_id, rhs.msg_id);
}
bool operator<(const MsgInfoForConfirmRef& lhs, const MsgInfoForConfirmSt& rhs)
{
	return std::tie(lhs.call_id, lhs.msg_id) < std::tie(rhs.call_id, rhs.msg_id);
}
inline void AddDstParts(std::vector<ParticipantDescr>&& src,
	vs::set<detail::call_id_info>& dst)
{
	for (auto&& part : src)
	{
		dst.emplace(std::move(part.partId), detail::call_id_info::common);
	}
}
}
inline vs::set<CallID> to_call_id_set(const vs::set<detail::call_id_info> &src)
{
	vs::set<CallID> res;
	std::transform(src.begin(),
		src.end(),
		std::inserter(res, res.begin()),
		[](const detail::call_id_info &info) {return CallID(info.call_id); });
	return res;
}
template<typename LayersTraits>
template<typename ... Args>
DeliveryLayer<LayersTraits>::DeliveryLayer(const GlobalConfigPtr &cfg, Args&&... args)
	: LayerHelper<LayersTraits>(std::forward<Args>(args)...)
	, cfg_(cfg)
	, current_account_(cfg->GetCurrentAccount())
	, storage_(cfg->GetChatStorage())
	, notifier_(cfg->GetEventsNotifier())
	, clock_wrap_(cfg->GetClockWrapper())
{
	assert(current_account_ && clock_wrap_);
	Timeout();
	auto undelivered_msgs = storage_->GetAllUndeliveredMessages();
	for (auto&& um : undelivered_msgs)
	{
		StartDeliveryPartList(std::move(um.msg), std::move(um.parts));
	}
}
template<typename LayersTraits>
void DeliveryLayer<LayersTraits>::AddRelatedEndpoints(vs::set<detail::call_id_info> &dst) const
{
	auto related_ep = current_account_->GetAllEndpoints();
	for (auto && i : related_ep)
	{
		if (i != current_account_->GetCallID()
			&& i != current_account_->GetExactCallID())
			dst.emplace(std::move(i), detail::call_id_info::endpoint);
	}
}
template<typename LayersTraits>
void DeliveryLayer<LayersTraits>::ForwardBelowMessage(msg::ChatMessagePtr&& msg,
	std::vector<ParticipantDescr>&& dstParts)
{
	if (!IsMessageStorable(msg))
	{
		SendWithoutConfirm(std::move(msg), std::move(dstParts));
		return;
	}
	auto dst_instance = msg->GetParamStrRef(attr::DST_ENDPOINT_paramName);
	if (dst_instance.empty())
	{
		StartDeliveryPartList(std::move(msg), std::move(dstParts));
	}
	else
	{
		this->PostCall(vs::move_handler(
			[
				this,
				msg = std::move(msg)
			]()
		{
			StartDelivery(*msg,
				msg->GetParamStrRef(attr::DST_ENDPOINT_paramName),
				{}, {}, clock_wrap_->now() + m_stop_retransmit_timeout);
		}));
	}
}
template<typename LayersTraits>
void DeliveryLayer<LayersTraits>::StartDeliveryPartList(
	msg::ChatMessagePtr&& msg, std::vector<ParticipantDescr>&& part_list)
{
	std::vector<CallID> call_idx;
	call_idx.reserve(part_list.size() + 1);
	std::transform(
		part_list.begin(), part_list.end(),
		std::back_inserter(call_idx),
		[](const auto& part) { return part.partId; });
	// Add destination for InviteMessage and InviteResponse
	// Exclude AddPart message
	auto call_id = GetParamStrFromMsgContent(msg, msg::nameKeyName);
	if (!call_id.empty()
		&& call_idx.end() == std::find(call_idx.begin(), call_idx.end(), call_id))
	{
		call_idx.emplace_back(std::move(call_id));
	}
	auto resolver = GetCfg()->GetResolver();
	auto msg_copy = std::shared_ptr<msg::ChatMessage>(std::move(msg));
	resolver->ResolveList(std::move(call_idx),
		[this, msg_copy = std::move(msg_copy)](auto&& res) mutable
	{
		this->PostCall(
			[this, msg_copy = std::move(msg_copy), res = std::move(res)]()
		{
			for (const auto& resolveInfo : res)
			{
				if (!resolveInfo.first)
					continue;
				if (resolveInfo.second.callId == current_account_->GetCallID() &&
					resolveInfo.second.epList.size() < 2)
				{
					// dont start delivery to self
					storage_->RemoveUndeliveredMessage(
						msg_copy->GetParamStr(attr::MESSAGE_ID_paramName),
						current_account_->GetCallID());
					continue;
				}
				this->StartDelivery(*msg_copy, resolveInfo.second.callId,
					resolveInfo.second.type == vs::CallIDType::server
					? BSInfoRef()
					: resolveInfo.second.bsInfo);
			}
		});
	});
}
template<typename LayersTraits>
void DeliveryLayer<LayersTraits>::StartDelivery(const msg::ChatMessage& msg, CallIDRef to,
	BSInfoRef bs, std::chrono::steady_clock::time_point when_send,
	std::chrono::steady_clock::time_point stop_retransmit)
{
	auto msg_copy = std::make_shared<msg::ChatMessage>(msg.GetContainer());
	msg_copy->SetParam(attr::DST_CALLID_paramName, to);
	auto msg_id = msg.GetParamStr(attr::MESSAGE_ID_paramName);
	wait_confirm_.emplace(
		detail::MsgInfoForConfirmSt(
			to, msg_id, bs),
		msg_copy);
	if (!bs.empty())
	{
		auto bs_record = confirm_by_bs_.emplace(std::make_pair(bs, MsgInfoForConfirmByMsgID()));
		auto info_set = bs_record.first->second.emplace(msg_id, MsgInfoForConfirmSet());
		info_set.first->second.emplace(detail::MsgInfoForConfirmSt(to, msg_id, bs));
	}
	auto nearest_resend = when_send == std::chrono::steady_clock::time_point()
		? clock_wrap_->now() + m_retransmit_timeout
		: when_send;
	wait_confirm_by_timeout_.emplace(
		msg_copy,
		nearest_resend,
		stop_retransmit);
	// if when_send isn't set, then send first time message now
	if(when_send == std::chrono::steady_clock::time_point())
	{
		this->Send(vs::make_unique<msg::ChatMessage>(msg_copy->GetContainer()), {});
	}
}
template<typename LayersTraits>
void DeliveryLayer<LayersTraits>::SendWithoutConfirm(
	msg::ChatMessagePtr&& msg, std::vector<ParticipantDescr>&& parts)
{
	auto dst_instance = msg->GetParamStrRef(attr::DST_ENDPOINT_paramName);
	if (!dst_instance.empty())
		this->Send(std::move(msg), {});
	else
	{
		for (const auto& part : parts)
		{
			auto copy_msg = vs::make_unique<msg::ChatMessage>(msg->GetContainer());
			copy_msg->SetParam(attr::DST_CALLID_paramName, part.partId);
			this->Send(std::move(copy_msg), {});
		}
	}
}
template<typename LayersTraits>
GlobalConfigPtr DeliveryLayer<LayersTraits>::GetCfg() const
{
	auto cfg = cfg_.lock();
	assert(cfg);
	return cfg;
}
template<typename LayersTraits>
void DeliveryLayer<LayersTraits>::OnChatMessageArrived(
	msg::ChatMessagePtr&&m,
	CallIDRef sender)
{
	if (msg::DeliveryConfirm::IsMyMessage(m))
	{
		auto info = msg::DeliveryConfirm::GetDeliveryInfo(m);
		MessageDelivered(info.msgId, info.from, info.fromInstance);
	}
	else
	{
		auto type = MessageType::undefined;
		m->GetParamI32(attr::MESSAGE_TYPE_paramName, type);
		auto msg_id = m->GetParamStr(attr::MESSAGE_ID_paramName);
		if (type != MessageType::undefined
			&& !msg_id.empty()
			&& IsMessageStorable(m))
		{
			if (storage_->IsMessageExist(msg_id))
			{
				SendDeliveryConfirm(msg_id, sender);
				return;
			}
			else
			{
				m->AddOnMsgIsStoredCallBack(
					[this, sender = CallID(sender)](
						cb::ProcessingResult res,
						ChatIDRef,
						ChatMessageIDRef msg_id)
				{
					if (cb::ProcessingResult::ok == res)
					{
						SendDeliveryConfirm(msg_id, sender);
					}
				});
			}
		}
		ForwardAboveMessage(std::move(m), sender);
	}
}
template<typename LayersTraits>
void DeliveryLayer<LayersTraits>::SendDeliveryConfirm(
	ChatMessageIDRef msgId, CallIDRef to)
{
	this->Send(
		msg::DeliveryConfirm(
			msgId, current_account_->GetCallID(),
			current_account_->GetExactCallID(), to
		).AcquireMessage(), {});
}
template<typename LayersTraits>
void DeliveryLayer<LayersTraits>::ShutDown()
{
	LayerHelper<LayersTraits>::ShutDown();
}
template<typename LayersTraits>
void DeliveryLayer<LayersTraits>::ProcessConfirm(const CallID& call_id, const ChatMessageID& msg_id)
{
	auto confirm_info = wait_confirm_.find(detail::MsgInfoForConfirmRef(call_id, msg_id, BSInfoRef()));
	if (confirm_info == wait_confirm_.end())
		return;
	if (!confirm_info->first.bs.empty())
	{
		auto bs_iter = confirm_by_bs_.find(confirm_info->first.bs);
		if (bs_iter != confirm_by_bs_.end())
		{
			auto msg_iter = bs_iter->second.find(msg_id);
			if (msg_iter != bs_iter->second.end())
			{
				auto iter = msg_iter->second.find(detail::MsgInfoForConfirmRef(call_id, msg_id, BSInfoRef()));
				if(iter != msg_iter->second.end())
					msg_iter->second.erase(iter);
			}
		}
	}
	wait_confirm_.erase(confirm_info);
	//  from BS
	auto bs_iter = confirm_by_bs_.find(call_id);
	if (bs_iter == confirm_by_bs_.end())
		return;
	auto msg_wait_iter = bs_iter->second.find(msg_id);
	if (msg_wait_iter == bs_iter->second.end())
		return;
	for (const auto& info : msg_wait_iter->second)
	{
		auto info_with_msg = wait_confirm_.find(info);
		if(info_with_msg != wait_confirm_.end())
		{
			StartDelivery(*info_with_msg->second, info_with_msg->first.call_id,
				{}, clock_wrap_->now() + m_stop_retransmit_timeout);
			wait_confirm_.erase(info_with_msg);
		}
	}
	bs_iter->second.erase(msg_wait_iter);
}
template<typename LayersTraits>
void DeliveryLayer<LayersTraits>::MessageDelivered(
	const ChatMessageID& msg_id,
	const CallID& from,
	const CallID& from_instance)
{
	storage_->RemoveUndeliveredMessage(msg_id, from);
	if(!from_instance.empty())
		ProcessConfirm(from_instance, msg_id);
	if(from != from_instance)
		ProcessConfirm(from, msg_id);
	notifier_->OnMsgDelivered(msg_id, from);
}
template<typename LayersTraits>
bool DeliveryLayer<LayersTraits>::StopRetransmitAchived(const msg::ChatMessage& msg)
{
	auto msg_id = msg.GetParamStr(attr::MESSAGE_ID_paramName);
	auto instance = msg.GetParamStr(attr::DST_ENDPOINT_paramName);
	auto call_id = msg.GetParamStr(attr::DST_CALLID_paramName);
	if (call_id.empty() && instance.empty())
	{
		return true;
	}
	if (call_id.empty())
	{
		call_id = CallID(VS_RemoveTranscoderID_sv(instance));
	}
	MessageDelivered(msg_id, call_id, instance);
	return true;
}
template<typename LayersTraits>
void DeliveryLayer<LayersTraits>::Timeout()
{
	if (this->IsShutdowned())
		return;
	this->ExpiresFromNow(m_timeout_duration);
	this->AsyncWait([this](TimerResult res)
	{
		if (res == TimerResult::canceled)
			return;
		this->PostCall([this]()
		{
			auto now = clock_wrap_->now();
			while (!wait_confirm_by_timeout_.empty()
				&& wait_confirm_by_timeout_.top().retransmitAfter <= now)
			{
				auto item(wait_confirm_by_timeout_.top());
				wait_confirm_by_timeout_.pop();
				++item.counter;
				auto m = item.msg.lock();
				if (m)
				{
					auto msg_for_resend = vs::make_unique<msg::ChatMessage>(
						m->GetContainer());
					msg_for_resend->SetParam(attr::RETRANSMITS_paramName, item.counter);
					this->Send(std::move(msg_for_resend), {});
					item.retransmitAfter = clock_wrap_->now() + m_retransmit_timeout;
					bool stopRetransmit(false);
					if (item.stopRetransmitAfter != std::chrono::steady_clock::time_point()
						&& item.stopRetransmitAfter <= now)
					{
						stopRetransmit = StopRetransmitAchived(*m);
					}
					if(!stopRetransmit)
						wait_confirm_by_timeout_.push(std::move(item));
				}
			}
			Timeout();
		});
	});
}
}

#undef DEBUG_CURRENT_MODULE
