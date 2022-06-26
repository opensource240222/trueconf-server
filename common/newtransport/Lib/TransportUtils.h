#pragma once

#include "../Error.h"
#include "../Router/Connection.h"
#include "../../net/Connect.h"
#include "std-generic/gcc_version.h"

#include <boost/asio/ip/tcp.hpp>

#include <chrono>

namespace transport {
namespace detail {

template <class Protocol>
class GetServerNameTaskBase : public Connection<typename Protocol::socket>
{
	using base_t = Connection<typename Protocol::socket>;
	using endpoint = typename Protocol::endpoint;
public:
	explicit GetServerNameTaskBase(boost::asio::io_service& ios)
		: base_t(ios)
	{
	}

	void Start(const endpoint& ep)
	{
		net::Connect<Protocol>(this->m_strand.get_io_service(), ep, [this, self = this->shared_from_this()](const boost::system::error_code& ec, typename Protocol::socket&& socket)
		{
			if (ec)
			{
				Complete(ec, nullptr);
				return;
			}

			auto temp_epname = "you.are.who" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
#if defined(GCC_VERSION) && GCC_VERSION < 50000
			BaseStart(std::move(socket), CreateHandshake(temp_epname, "hwo.are.you", 1, true, false).get());
#else
			base_t::Start(std::move(socket), CreateHandshake(temp_epname, "hwo.are.you", 1, true, false).get());
#endif
		});
	}
#if defined(GCC_VERSION) && GCC_VERSION < 50000
	// GCC < 5.0 can't access protected members inside a lambda.
	// MSVC120 emits error C2885 (Start() cannot access protected member declared in class Connection<>) for a call to Start() inside lambda
	void BaseStart(typename base_t::socket_type&& socket, const net::HandshakeHeader* handshake)
	{
		base_t::Start(std::move(socket), handshake);
	}
#endif

private:
	void OnHandshakeReply(HandshakeResult result, uint16_t, uint8_t, uint8_t, const char* server_id, const char*, bool) override
	{
		if (result != HandshakeResult::ok &&
			result != HandshakeResult::alien_server/*acceptable value when use 'hwo.are.you' SID*/)
		{
			Complete(errc::handshake_error, nullptr);
			return;
		}
		this->Close();
		Complete({}, server_id);
	}
	void OnMessage(Message&&) override
	{
	}

	bool OnError(const boost::system::error_code& ec) override
	{
		Complete(ec, nullptr);
		return false;
	}

	virtual void Complete(const boost::system::error_code& ec, const char* name) = 0;
};
template <class Protocol, class Handler>
class GetServerNameTask : public GetServerNameTaskBase<Protocol>
{
	using base_t = GetServerNameTaskBase<Protocol>;
public:
	template <class H>
	GetServerNameTask(boost::asio::io_service& ios, H&& handler)
		: base_t(ios)
		, m_handler(std::forward<H>(handler))
	{
	}

private:
	void Complete(const boost::system::error_code& ec, const char* name) override
	{
		m_handler(ec, name);
	}
	Handler m_handler;
};

}

template <class Protocol = boost::asio::ip::tcp, class Handler>
void GetServerNameByAddress(boost::asio::io_service& ios, const typename Protocol::endpoint& ep, Handler&& handler)
{
	std::make_shared<detail::GetServerNameTask<Protocol, typename std::decay<Handler>::type>>(ios, std::forward<Handler>(handler))->Start(ep);
}

template <class Protocol = boost::asio::ip::tcp, class Handler>
void GetServerNameByAddress(boost::asio::io_service& ios, const net::address& address, net::port port, Handler&& handler)
{
	GetServerNameByAddress(ios, typename Protocol::endpoint(address, port), std::forward<Handler>(handler));
}

}
