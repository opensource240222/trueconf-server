#pragma once

#include "net/Connect.h"
#include "std-generic/cpplib/move_handler.h"
#include "std-generic/cpplib/string_view.h"
#include "std/cpplib/ForEachHost.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>

#include "std-generic/compat/memory.h"
#include <algorithm>
#include <atomic>
#include <chrono>

namespace ts {
namespace detail {

template <class Protocol>
class ConnectToServerTaskBase : public vs::enable_shared_from_this<ConnectToServerTaskBase<Protocol>>
{
public:
	virtual ~ConnectToServerTaskBase() = default;

	void Start(string_view addresses, std::chrono::steady_clock::duration timeout)
	{
		if (addresses.empty())
		{
			Complete(boost::asio::error::not_found, typename Protocol::socket(m_timer.get_io_service()), this->shared_from_this());
			return;
		}

		m_attempts.store(std::count(addresses.begin(), addresses.end(), ',') + 1, std::memory_order_release);

		const auto w_self = this->weak_from_this();
		ForEachHost(addresses, [&](string_view host, string_view port)
		{
			if (port.empty())
				port = "4307";
			net::Connect<Protocol>(m_timer.get_io_service(), std::string(host), std::string(port), [this, w_self](const boost::system::error_code& ec, typename Protocol::socket&& socket)
			{
				auto self = w_self.lock();
				if (!self)
					return;
				if (ec)
				{
					if (m_attempts.fetch_sub(1, std::memory_order_acq_rel) == 1)
					{
						m_timer.cancel();
						Complete(ec, std::move(socket), std::move(self));
					}
					return;
				}
				if (m_attempts.exchange(0, std::memory_order_acq_rel) != 0)
				{
					m_timer.cancel();
					Complete(ec, std::move(socket), std::move(self));
				}
			});
		});

		m_timer.expires_from_now(timeout);
		m_timer.async_wait([this, self = this->shared_from_this()](const boost::system::error_code& ec) mutable
		{
			if (ec == boost::asio::error::operation_aborted)
				return;
			if (m_attempts.exchange(0, std::memory_order_acq_rel) != 0)
				Complete(boost::asio::error::timed_out, typename Protocol::socket(m_timer.get_io_service()), std::move(self));
		});
	}

protected:
	explicit ConnectToServerTaskBase(boost::asio::io_service &ios)
		: m_timer(ios)
	{
	}

private:
	virtual void Complete(const boost::system::error_code& ec, typename Protocol::socket&& socket, std::shared_ptr<ConnectToServerTaskBase>&& self) = 0;

	std::atomic<unsigned> m_attempts;
	boost::asio::steady_timer m_timer;
};

template <class Protocol, class Handler>
class ConnectToServerTask : public ConnectToServerTaskBase<Protocol>
{
	using base_t = ConnectToServerTaskBase<Protocol>;
public:
	template <class H>
	ConnectToServerTask(boost::asio::io_service& ios, H&& handler)
		: base_t(ios)
		, m_handler(std::forward<H>(handler))
	{
	}

private:
	void Complete(const boost::system::error_code& ec, typename Protocol::socket&& socket, std::shared_ptr<base_t>&&) override
	{
		m_handler(ec, std::move(socket));
	}
	Handler m_handler;
};

template <class Protocol, class Handler>
class ConnectToServerTask_Strand : public ConnectToServerTaskBase<Protocol>
{
	using base_t = ConnectToServerTaskBase<Protocol>;
public:
	template <class H>
	ConnectToServerTask_Strand(boost::asio::io_service::strand& strand, H&& handler)
		: base_t(strand.get_io_service())
		, m_strand(strand)
		, m_handler(std::forward<H>(handler))
	{
	}

private:
	void Complete(const boost::system::error_code& ec, typename Protocol::socket&& socket, std::shared_ptr<base_t>&& self) override
	{
		m_strand.dispatch(vs::move_handler([this, self = std::move(self), ec, socket = std::move(socket)]() mutable { m_handler(ec, std::move(socket)); }));
	}
	boost::asio::io_service::strand m_strand;
	Handler m_handler;
};

}

template <class Protocol, class Handler>
void ConnectToServer(boost::asio::io_service& ios, string_view addresses, std::chrono::steady_clock::duration timeout, Handler&& handler)
{
	std::make_shared<detail::ConnectToServerTask<Protocol, typename std::decay<Handler>::type>>(ios, std::forward<Handler>(handler))->Start(addresses, timeout);
}

template <class Protocol, class Handler>
void ConnectToServer(boost::asio::io_service::strand& strand, string_view addresses, std::chrono::steady_clock::duration timeout, Handler&& handler)
{
	std::make_shared<detail::ConnectToServerTask_Strand<Protocol, typename std::decay<Handler>::type>>(strand, std::forward<Handler>(handler))->Start(addresses, timeout);
}

}
