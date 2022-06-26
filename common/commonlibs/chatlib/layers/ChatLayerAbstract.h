#pragma once
#include "chatlib/chat_defs.h"
#include "chatlib/chatinfo/info_types.h"
#include "chatlib/layers/LayersTraits.h"
#include "std-generic/cpplib/move_handler.h"

#include <atomic>
#include <memory>
#include <queue>
#include <type_traits>

namespace chat
{
using LayerAboveNotifierPtr = std::shared_ptr<class LayerAboveNotifier> ;
using LayerBelowInterfacePtr = std::shared_ptr<class LayerBelowInterface>;
using OnChatMessageRecvCallBack = std::function<void(msg::ChatMessagePtr&&, CallIDRef)>;
class LayerBelowInterface
{
public:
	virtual ~LayerBelowInterface() = default;
	// dstParts - transmit messge all participants from list
	virtual void ForwardBelowMessage(msg::ChatMessagePtr&& msg,
		std::vector<ParticipantDescr>&& dstParts) = 0;
	virtual void ShutDown() = 0;
};
class LayerAboveNotifier
{
	OnChatMessageRecvCallBack fireOnMessageRecv_;
protected:
	void ForwardAboveMessage(msg::ChatMessagePtr&&msg, CallIDRef sender);
public:
	virtual ~LayerAboveNotifier() = default;
	void SetOnMessageRecvCallBack(const OnChatMessageRecvCallBack &cb)
	{
		fireOnMessageRecv_ = cb;
	}
};
class LayerInterface :
	public LayerBelowInterface,
	public LayerAboveNotifier
{
};

template<typename TimerType>
class TimerHelper
{
	TimerType m_timer;
public:
	template<typename TimerInitType>
	TimerHelper(TimerInitType&& param)
		: m_timer(std::forward<TimerInitType>(param))
	{}
protected:
	template<typename DurationType>
	std::size_t ExpiresFromNow(const DurationType& expTime)
	{
		return m_timer.ExpiresFromNow(expTime);
	}
	std::size_t Cancel()
	{
		return m_timer.Cancel();
	}
	template<typename Handler>
	void AsyncWait(Handler&& h)
	{
		m_timer.AsyncWait(std::forward<Handler>(h));
	}
	void ShutDown()
	{
		Cancel();
	}
};
template<>
class TimerHelper<void>
{
public:
	template<typename ...Args>
	TimerHelper(Args&& ...)
	{}
protected:
	void ShutDown()
	{}
};
template<typename LayersTraits>
class LayerHelper : public TimerHelper<typename LayersTraits::TimerType>
{
	using TimerType = typename LayersTraits::TimerType;
	using StrandType = typename LayersTraits::StrandType;

	mutable StrandType m_strand_impl;
	LayerInterfacePtr m_next;
	std::atomic_bool m_is_shutdown { false };
public:
	template<
		class TimerInitType,
		typename Y = typename LayersTraits::TimerType,
		class = typename std::enable_if<!std::is_void<Y>::value>::type
	>
		LayerHelper(StrandType&& strand, TimerInitType&& timerParam)
		: TimerHelper<TimerType>(std::forward<TimerInitType>(timerParam))
		, m_strand_impl(std::forward<StrandType>(strand))
	{}
	template <
		typename Y = typename LayersTraits::TimerType,
		class = typename std::enable_if<std::is_void<Y>::value>::type
	>
		LayerHelper(StrandType&& strand)
		: m_strand_impl(std::forward<StrandType>(strand))
	{}
	void SetNextLayer(const LayerInterfacePtr &next)
	{
		m_next = next;
		next->SetOnMessageRecvCallBack(
			[this](msg::ChatMessagePtr&&msg, CallIDRef sender)
		{
			m_strand_impl.post(vs::move_handler(
				[this, msg = std::move(msg),sender = CallID(sender)]() mutable
			{
				OnChatMessageArrived(std::move(msg),sender);
			}
			));
		});
	}
	void ShutDown()
	{
		m_is_shutdown.store(true, std::memory_order_relaxed);
		TimerHelper<TimerType>::ShutDown();
		if (m_next)
			m_next->ShutDown();
	}
protected:
	template<typename F>
	void PostCall(F&&f) const
	{
		m_strand_impl.post(std::forward<F>(f));
	}
	void Send(msg::ChatMessagePtr&& msg,
		std::vector<ParticipantDescr>&& dstParts)
	{
		m_strand_impl.post(
			vs::move_handler(
			[
				this,
				msg = std::move(msg),
				dstParts = std::move(dstParts)
			]() mutable
		{
			if (m_next)
				m_next->ForwardBelowMessage(std::move(msg), std::move(dstParts));
		}));
	}
	bool IsShutdowned() const
	{
		return m_is_shutdown.load(std::memory_order_relaxed);
	}
	virtual void OnChatMessageArrived(
		msg::ChatMessagePtr&&msg,
		CallIDRef sender) = 0;
};
}


