#pragma once

#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <queue>
#include "../std/cpplib/VS_Lock.h"
#include "std-generic/cpplib/macro_utils.h"

template <class T>
class VS_ClientResponseBuffer
{
	class CallBackHolder
	{
		boost::weak_ptr<void> p;
		boost::function< bool (const T&) > handler;

	public:
		VS_FORWARDING_CTOR2(CallBackHolder, p, handler) {}

		bool operator()(const T& x)
		{
			boost::shared_ptr<void> lock = p.lock();
			return lock.get() && handler( x );
		}

		bool isValid() { return !p.expired(); }
	};

public:
	VS_ClientResponseBuffer(void)
	{
		m_last_keepalive = boost::posix_time::second_clock::universal_time();
		m_delete = false;
	}

	virtual ~VS_ClientResponseBuffer(void)
	{
		ReleaseAllHandlers();
	}

	template <class P>
	bool AddHandler(boost::shared_ptr<P> holder, boost::function< bool (const T&) > handler )
	{
		if (m_delete) return false;

		VS_AutoLock l(&m_lock);

		if (m_handlers.size() < 3)
		{
			m_handlers.emplace_back(boost::static_pointer_cast<void>(holder), handler);
			Dispatch();
			return true;
		}
		return false;
	}

protected:

	void DispatchResponse(const T &obj)
	{
		VS_AutoLock l(&m_lock);

		// performance enh
		if (m_resp.empty() && !m_handlers.empty() && (m_handlers.front())( obj ) )
		{
			m_last_keepalive = boost::posix_time::second_clock::universal_time();
			m_handlers.pop_front();
			return;
		}

		m_resp.push_back(obj);
		return Dispatch();
	}

	bool IsTimedOut()
	{
		VS_AutoLock l(&m_lock);
		if (!m_handlers.empty()) m_last_keepalive = boost::posix_time::second_clock::universal_time();

		for (std::deque<CallBackHolder>::iterator i = m_handlers.begin(); i != m_handlers.end(); )
		{
			if (! (*i).isValid() ) i = m_handlers.erase( i );
			else ++i;
		}

		using namespace boost::posix_time;
		return m_last_keepalive + seconds(10) <= second_clock::universal_time();
	}

	void ReleaseAllHandlers()
	{
		m_delete = true;

		VS_AutoLock l(&m_lock);

		while(!m_handlers.empty())
		{
			(m_handlers.front())( T() );
			m_handlers.pop_front();
		}
	}

private:
	boost::posix_time::ptime m_last_keepalive;
	std::deque< CallBackHolder > m_handlers;
	std::deque<const T> m_resp;
	bool m_delete;
	VS_Lock m_lock;

	void Dispatch()
	{
		while (!m_resp.empty() && !m_handlers.empty())
		{
			CallBackHolder handler = m_handlers.front();
			T data = m_resp.front();

			m_handlers.pop_front();
			m_resp.pop_front();

			if (!handler(data))
			{
				m_resp.push_front(data);
			} else
			{
				m_last_keepalive = boost::posix_time::second_clock::universal_time();
			}
 		}
	}
};

