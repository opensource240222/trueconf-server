#pragma once
#include "chatlib/chat_defs.h"
#include <boost/asio/steady_timer.hpp>
namespace chat
{
namespace asio_sync
{
class SteadyTimerBoost
{
	boost::asio::steady_timer m_steadyTimer;
public:
	explicit SteadyTimerBoost(boost::asio::io_service& ios)
		: m_steadyTimer(ios)
	{}
	template<typename DurationType>
	std::size_t ExpiresFromNow(const DurationType& expTime)
	{
		 return m_steadyTimer.expires_from_now(expTime);
	}
	std::size_t Cancel()
	{
		return m_steadyTimer.cancel();
	}
	template<typename Handler>
	void AsyncWait(Handler&& h)
	{
		m_steadyTimer.async_wait([h = std::forward<Handler>(h)](const boost::system::error_code& err)
		{
			auto res = err == boost::asio::error::operation_aborted
				? TimerResult::canceled
				: TimerResult::success;
			h(res);
		});
	}

};
}
}