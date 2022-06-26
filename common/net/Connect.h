#pragma once

#include "Address.h"
#include "Port.h"
#include "EndpointRegistry.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/strand.hpp>

#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>

namespace net {
namespace detail {

// TODO: MSVC140. Instead of using enable_shared_from_this pass unique_ptr<>(this) to lambdas (possibly with a workaround for move only handlers).
template <class Protocol>
class ConnectIPTaskBase : public std::enable_shared_from_this<ConnectIPTaskBase<Protocol>>
{
	using socket = typename Protocol::socket;
	using endpoint = typename Protocol::endpoint;
public:
	virtual ~ConnectIPTaskBase() {};

	void Start(const endpoint& ep)
	{
		m_socket.async_connect(ep, [this, self = this->shared_from_this()](const boost::system::error_code& ec) mutable {
			Complete(ec, std::move(self));
		});
	}

protected:
	explicit ConnectIPTaskBase(boost::asio::io_service &ios)
		: m_socket(ios)
	{
	}

private:
	virtual void Complete(const boost::system::error_code& ec, std::shared_ptr<ConnectIPTaskBase>&& self) = 0;

protected:
	socket m_socket;
};

template <class Protocol, class Handler>
class ConnectIPTask : public ConnectIPTaskBase<Protocol>
{
	using base_t = ConnectIPTaskBase<Protocol>;
public:
	template <class H>
	ConnectIPTask(boost::asio::io_service& ios, H&& handler)
		: base_t(ios)
		, m_handler(std::forward<H>(handler))
	{
	}

private:
	void Complete(const boost::system::error_code& ec, std::shared_ptr<base_t>&&) override
	{
		m_handler(ec, std::move(this->m_socket));
	}
	Handler m_handler;
};

template <class Protocol, class Handler>
class ConnectIPTask_Strand : public ConnectIPTaskBase<Protocol>
{
	using base_t = ConnectIPTaskBase<Protocol>;
public:
	template <class H>
	ConnectIPTask_Strand(boost::asio::io_service::strand& strand, H&& handler)
		: base_t(strand.get_io_service())
		, m_strand(strand)
		, m_handler(std::forward<H>(handler))
	{
	}

private:
	void Complete(const boost::system::error_code& ec, std::shared_ptr<base_t>&& self) override
	{
		m_strand.dispatch([this, self = std::move(self), ec]() { m_handler(ec, std::move(this->m_socket)); });
	}
	boost::asio::io_service::strand m_strand;
	Handler m_handler;
};

template <class Protocol>
class ConnectHostnameTaskBase : public std::enable_shared_from_this<ConnectHostnameTaskBase<Protocol>>
{
	using socket = typename Protocol::socket;
	using resolver = typename Protocol::resolver;
	using resolver_query = typename resolver::query;
	using resolver_iterator = typename resolver::iterator;
public:
	virtual ~ConnectHostnameTaskBase() {};

	void Start(const std::string& host, const std::string& service)
	{
		m_resolver.async_resolve(resolver_query(host, service), [this, self = this->shared_from_this()](const boost::system::error_code& ec, resolver_iterator it) mutable {
			if (ec)
			{
				Complete(ec, std::move(self));
				return;
			}
			m_resolve_result = it;
			TryConnect(std::move(self));
		});
	}

protected:
	explicit ConnectHostnameTaskBase(boost::asio::io_service &ios)
		: m_socket(ios)
		, m_resolver(ios)
	{
	}

private:
	virtual void Complete(const boost::system::error_code& ec, std::shared_ptr<ConnectHostnameTaskBase>&& self) = 0;

	void TryConnect(std::shared_ptr<ConnectHostnameTaskBase>&& self)
	{
		assert(m_resolve_result != resolver_iterator());
		m_socket.async_connect(m_resolve_result->endpoint(), [this, self = std::move(self)](const boost::system::error_code& ec) mutable {
			if (ec)
			{
				if (++m_resolve_result != resolver_iterator())
				{
					TryConnect(std::move(self));
					return;
				}
			}
			Complete(ec, std::move(self));
		});
	}

protected:
	socket m_socket;
private:
	resolver m_resolver;
	resolver_iterator m_resolve_result;
};

template <class Protocol, class Handler>
class ConnectHostnameTask : public ConnectHostnameTaskBase<Protocol>
{
	using base_t = ConnectHostnameTaskBase<Protocol>;
public:
	template <class H>
	ConnectHostnameTask(boost::asio::io_service& ios, H&& handler)
		: base_t(ios)
		, m_handler(std::forward<H>(handler))
	{
	}

private:
	void Complete(const boost::system::error_code& ec, std::shared_ptr<base_t>&&) override
	{
		m_handler(ec, std::move(this->m_socket));
	}
	Handler m_handler;
};

template <class Protocol, class Handler>
class ConnectHostnameTask_Strand : public ConnectHostnameTaskBase<Protocol>
{
	using base_t = ConnectHostnameTaskBase<Protocol>;
public:
	template <class H>
	ConnectHostnameTask_Strand(boost::asio::io_service::strand& strand, H&& handler)
		: base_t(strand.get_io_service())
		, m_strand(strand)
		, m_handler(std::forward<H>(handler))
	{
	}

private:
	void Complete(const boost::system::error_code& ec, std::shared_ptr<base_t>&& self) override
	{
		m_strand.dispatch([this, self = std::move(self), ec]() { m_handler(ec, std::move(this->m_socket)); });
	}
	boost::asio::io_service::strand m_strand;
	Handler m_handler;
};

template <class Protocol>
class ConnectEndpointTaskBase : public std::enable_shared_from_this<ConnectEndpointTaskBase<Protocol>>
{
	using socket = typename Protocol::socket;
	using resolver = typename Protocol::resolver;
	using resolver_query = typename resolver::query;
	using resolver_iterator = typename resolver::iterator;
public:
	virtual ~ConnectEndpointTaskBase() {};

	void Start(string_view endpoint_name)
	{
		m_endpoint_name = std::string(endpoint_name);
		m_connect_count = net::endpoint::GetCountConnectTCP(m_endpoint_name);
		if (m_connect_count == 0)
			m_socket.get_io_service().post([this, self = this->shared_from_this()]() mutable { Complete(boost::asio::error::not_found, std::move(self)); });
		else
			TryResolve(this->shared_from_this());
	}

protected:
	explicit ConnectEndpointTaskBase(boost::asio::io_service &ios)
		: m_socket(ios)
		, m_resolver(ios)
		, m_connect_id(1)
		, m_connect_count(0)
	{
	}

private:
	virtual void Complete(const boost::system::error_code& ec, std::shared_ptr<ConnectEndpointTaskBase>&& self) = 0;

	void TryResolve(std::shared_ptr<ConnectEndpointTaskBase>&& self)
	{
		for (; m_connect_id <= m_connect_count; ++m_connect_id)
		{
			auto x = net::endpoint::ReadConnectTCP(m_connect_id, m_endpoint_name);
			if (x && x->protocol_name == net::endpoint::protocol_tcp)
			{
				m_resolver.async_resolve(resolver_query(x->host, std::to_string(x->port)), [this, self](const boost::system::error_code& ec, resolver_iterator it) mutable {
					if (ec)
					{
						if (++m_connect_id <= m_connect_count)
						{
							TryResolve(std::move(self));
							return;
						}
						Complete(ec, std::move(self));
						return;
					}
					m_resolve_result = it;
					TryConnect(std::move(self));
				});
				return;
			}
		}
		Complete(boost::asio::error::not_found, std::move(self));
	}

	void TryConnect(std::shared_ptr<ConnectEndpointTaskBase>&& self)
	{
		assert(m_resolve_result != resolver_iterator());
		m_socket.async_connect(m_resolve_result->endpoint(), [this, self = std::move(self)](const boost::system::error_code& ec) mutable {
			if (ec)
			{
				if (++m_resolve_result != resolver_iterator())
				{
					TryConnect(std::move(self));
					return;
				}
				else if (++m_connect_id <= m_connect_count)
				{
					TryResolve(std::move(self));
					return;
				}
			}
			Complete(ec, std::move(self));
		});
	}

protected:
	socket m_socket;
private:
	resolver m_resolver;
	resolver_iterator m_resolve_result;
	std::string m_endpoint_name;
	unsigned m_connect_id;
	unsigned m_connect_count;
};

template <class Protocol, class Handler>
class ConnectEndpointTask : public ConnectEndpointTaskBase<Protocol>
{
	using base_t = ConnectEndpointTaskBase<Protocol>;
public:
	template <class H>
	ConnectEndpointTask(boost::asio::io_service& ios, H&& handler)
		: base_t(ios)
		, m_handler(std::forward<H>(handler))
	{
	}

private:
	void Complete(const boost::system::error_code& ec, std::shared_ptr<base_t>&&) override
	{
		m_handler(ec, std::move(this->m_socket));
	}
	Handler m_handler;
};

template <class Protocol, class Handler>
class ConnectEndpointTask_Strand : public ConnectEndpointTaskBase<Protocol>
{
	using base_t = ConnectEndpointTaskBase<Protocol>;
public:
	template <class H>
	ConnectEndpointTask_Strand(boost::asio::io_service::strand& strand, H&& handler)
		: base_t(strand.get_io_service())
		, m_strand(strand)
		, m_handler(std::forward<H>(handler))
	{
	}

private:
	void Complete(const boost::system::error_code& ec, std::shared_ptr<base_t>&& self) override
	{
		m_strand.dispatch([this, self = std::move(self), ec]() { m_handler(ec, std::move(this->m_socket)); });
	}
	boost::asio::io_service::strand m_strand;
	Handler m_handler;
};

}

// Connect() opens a new socket and asynchronously connects it to the specified endpoint.
// Type of the socket is determined by the Protocol template argument (can be boost::asio::ip::tcp, net::tls, etc).
// Endpoint can be specified as: Protocol::endpoint_type object, IP address and port, hostname, endpoint name.
// When the operation completes the supplied handler is called, it should have this signature:
//    void (boost::system::error_code, Protocol::socket&&)
// If the first argument is an io_service then the handler is called on the io_service.
// If the first argument is a strand then the handler is called on the strand.
// Handler is never called from inside this function (it is always posted to io_service or strand).

template <class Protocol, class Handler>
void Connect(boost::asio::io_service& ios, const typename Protocol::endpoint& ep, Handler&& handler)
{
	std::make_shared<detail::ConnectIPTask<Protocol, typename std::decay<Handler>::type>>(ios, std::forward<Handler>(handler))->Start(ep);
}

template <class Protocol, class Handler>
void Connect(boost::asio::io_service::strand& strand, const typename Protocol::endpoint& ep, Handler&& handler)
{
	std::make_shared<detail::ConnectIPTask_Strand<Protocol, typename std::decay<Handler>::type>>(strand, std::forward<Handler>(handler))->Start(ep);
}

template <class Protocol, class Handler>
void Connect(boost::asio::io_service& ios, const net::address& address, net::port port, Handler&& handler)
{
	Connect(ios, typename Protocol::endpoint(address, port), std::forward<Handler>(handler));
}

template <class Protocol, class Handler>
void Connect(boost::asio::io_service::strand& strand, const net::address& address, net::port port, Handler&& handler)
{
	Connect(strand, typename Protocol::endpoint(address, port), std::forward<Handler>(handler));
}

template <class Protocol, class Handler>
void Connect(boost::asio::io_service& ios, const std::string& host, const std::string& service, Handler&& handler)
{
	std::make_shared<detail::ConnectHostnameTask<Protocol, typename std::decay<Handler>::type>>(ios, std::forward<Handler>(handler))->Start(host, service);
}

template <class Protocol, class Handler>
void Connect(boost::asio::io_service::strand& strand, const std::string& host, const std::string& service, Handler&& handler)
{
	std::make_shared<detail::ConnectHostnameTask_Strand<Protocol, typename std::decay<Handler>::type>>(strand, std::forward<Handler>(handler))->Start(host, service);
}

template <class Protocol, class Handler>
void Connect(boost::asio::io_service& ios, string_view endpoint_name, Handler&& handler)
{
	std::make_shared<detail::ConnectEndpointTask<Protocol, typename std::decay<Handler>::type>>(ios, std::forward<Handler>(handler))->Start(endpoint_name);
}

template <class Protocol, class Handler>
void Connect(boost::asio::io_service::strand& strand, string_view endpoint_name, Handler&& handler)
{
	std::make_shared<detail::ConnectEndpointTask_Strand<Protocol, typename std::decay<Handler>::type>>(strand, std::forward<Handler>(handler))->Start(endpoint_name);
}

}
