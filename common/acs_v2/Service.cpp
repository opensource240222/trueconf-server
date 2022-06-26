#include "Service.h"
#include "Handler.h"
#include "Error.h"
#include "Internal.h"
#include "../net/EndpointRegistry.h"
#include "../net/InterfaceInfo.h"
#include "net/UDPRouter.h"
#include "../std/cpplib/bind_tools.h"
#include "../std/cpplib/latch.h"
#include "../std/debuglog/VS_Debug.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/ignore.h"
#include "std/debuglog/VS_LogHelpers.h"

#include <boost/asio/write.hpp>

#include <atomic>
#include <algorithm>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

#define ACS_VERBOSE_LOGS 0

namespace acs {

static const size_t c_read_size = 1024;
static const size_t c_max_read_size = 64 * 1024;
static const auto c_connections_cleanup_period = std::chrono::seconds(30);

static const auto c_selftest_timeout = std::chrono::seconds(10);
static const unsigned char c_selftest_data[] = "AcsTestAttemptStr";

class Listener
{
public:
	const net::address address;
	const net::port port;
	const net::protocol protocol;
	const unsigned channel_token;
	const bool hidden;
	uint16_t rcvd_connections=0;
	uint16_t unseccessful_connections=0;

	Listener(const net::address& address_, net::port port_, net::protocol protocol_, unsigned channel_token_, bool hidden_);
	virtual ~Listener() {}
	virtual void Start(const std::shared_ptr<Listener>& self, std::shared_ptr<Service> acs, boost::system::error_code& ec) = 0;
	virtual void Stop(boost::system::error_code& ec) = 0;
	virtual bool IsOpen() = 0;
	void FillMonitorStruct(Monitor::AcsReply::Listener & listener);

protected:
};

class ListenerTCP : public Listener
{
public:
	ListenerTCP(const net::address& address_, net::port port_, net::protocol protocol_, unsigned channel_token_, bool hidden_, boost::asio::io_service& ios);
	void Start(const std::shared_ptr<Listener>& self, std::shared_ptr<Service> acs, boost::system::error_code& ec) override;
	void Stop(boost::system::error_code& ec) override;
	bool IsOpen() override;

private:
	void StartAccept(std::shared_ptr<Listener> self);

	std::shared_ptr<Service> m_acs; // When listener is started holds a pointer to the parent Service to prevent its destruction, equals nullptr when listener is stopped.
	boost::asio::ip::tcp::acceptor m_acceptor;
};

class ListenerUDP : public Listener
{
public:
	ListenerUDP(const net::address& address_, net::port port_, net::protocol protocol_, unsigned channel_token_, bool hidden_);
	void Start(const std::shared_ptr<Listener>& self, std::shared_ptr<Service> acs, boost::system::error_code& ec) override;
	void Stop(boost::system::error_code& ec) override;
	bool IsOpen() override;

private:
	void StartAccept(std::shared_ptr<Listener> self);

	std::shared_ptr<Service> m_acs; // When listener is started holds a pointer to the parent Service to prevent its destruction, equals nullptr when listener is stopped.
	net::UDPRouterPtr m_router;
};

class Connection
{
public:
	Connection(std::shared_ptr<Service> acs, unsigned channel_token);
	virtual ~Connection() {}
	virtual void Close() = 0;

protected:
	struct handler_response_info : public Service::handler_info
	{
		Response response;

		// cppcheck-suppress noExplicitConstructor
		handler_response_info(const handler_info& hi)
			: handler_info(hi)
			, response(Response::next_step)
		{
		}
	};

	std::shared_ptr<Service> m_acs;
	const unsigned m_channel_token;
	std::vector<handler_response_info> m_handlers;
};

class ConnectionTCP : public Connection
{
public:
	ConnectionTCP(std::shared_ptr<Service> acs, unsigned channel_token);
	void Start(const std::shared_ptr<Connection>& self);
	void Close() override;

	      boost::asio::ip::tcp::socket& Socket()       { return m_socket; }
	const boost::asio::ip::tcp::socket& Socket() const { return m_socket; }

private:
	void StartRead(std::shared_ptr<Connection> self);
	const std::string& LogID() const;

	boost::asio::ip::tcp::socket m_socket;
	Handler::stream_buffer m_buffer;
	size_t m_data_size;
	bool m_claimed_by_handler;

	mutable std::string m_log_id;
};

class ConnectionUDP : public Connection
{
public:
	ConnectionUDP(std::shared_ptr<Service> acs, net::UDPConnection udp_conn, unsigned channel_token);
	void Start(const std::shared_ptr<Connection>& self);
	void Close() override;

private:
	void StartRead(std::shared_ptr<Connection> self);
	const std::string& LogID() const;

	net::UDPConnection m_connection;
	Handler::packet_buffer m_buffer;
	bool m_claimed_by_handler;

	mutable std::string m_log_id;
};

std::atomic<unsigned> g_last_channel_token(0);

std::function<std::shared_ptr<Service>(boost::asio::io_service& ios)> Service::s_factory;

std::shared_ptr<Service> Service::Create(boost::asio::io_service& ios)
{
	if (s_factory)
		return s_factory(ios);
	else
		return std::make_shared<Service>(ios);
}

Service::Service(boost::asio::io_service& ios)
	: m_strand(ios)
	, m_resolver(ios)
	, m_connections_cleanup_timer(ios)
{
}

Service::~Service()
{
}

std::future<bool> Service::Start()
{
	std::promise<bool> p;
	std::future<bool> f(p.get_future());
	m_strand.post(vs::forward_async_call(&Service::Start_impl, shared_from_this(), p));
	return f;
}

std::future<void> Service::Stop()
{
	std::promise<void> p;
	std::future<void> f(p.get_future());
	m_strand.post(vs::forward_async_call(&Service::Stop_impl, shared_from_this(), p));
	return f;
}

boost::system::error_code Service::AddHandler(string_view name, const std::weak_ptr<Handler>& handler, bool is_final)
{
	std::promise<boost::system::error_code> p;
	std::future<boost::system::error_code> f(p.get_future());
	m_strand.post(vs::forward_sync_call(&Service::AddHandler_impl, this, p, name, handler, is_final));
	return f.get();
}

boost::system::error_code Service::RemoveHandler(string_view name)
{
	std::promise<boost::system::error_code> p;
	std::future<boost::system::error_code> f(p.get_future());
	m_strand.post(vs::forward_sync_call(&Service::RemoveHandler_impl, this, p, name));
	return f.get();
}

void Service::RemoveAllHandlers()
{
	std::promise<void> p;
	std::future<void> f(p.get_future());
	m_strand.post(vs::forward_sync_call(&Service::RemoveAllHandlers_impl, this, p));
	return f.get();
}

boost::system::error_code Service::AddListener(const net::address& address, net::port port, net::protocol protocol, unsigned channel_token, bool hidden)
{
	std::promise<boost::system::error_code> p;
	std::future<boost::system::error_code> f(p.get_future());
	m_strand.post(vs::forward_sync_call(static_cast<void(Service::*)(std::promise<boost::system::error_code>&&, const net::address&, net::port, net::protocol, unsigned, bool)>(&Service::AddListener_impl), this, p, address, port, protocol, channel_token, hidden));
	return f.get();
}

boost::system::error_code Service::RemoveListener(const net::address& address, net::port port, net::protocol protocol)
{
	std::promise<boost::system::error_code> p;
	std::future<boost::system::error_code> f(p.get_future());
	m_strand.post(vs::forward_sync_call(static_cast<void(Service::*)(std::promise<boost::system::error_code>&&, const net::address&, net::port, net::protocol)>(&Service::RemoveListener_impl), this, p, address, port, protocol));
	return f.get();
}

unsigned Service::RemoveListeners(const std::function<bool(const net::address&, net::port, net::protocol)>& predicate)
{
	std::promise<unsigned> p;
	std::future<unsigned> f(p.get_future());
	m_strand.post(vs::forward_sync_call(static_cast<void(Service::*)(std::promise<unsigned>&&, const std::function<bool(const net::address&, net::port, net::protocol)>&)>(&Service::RemoveListeners_impl), this, p, predicate));
	return f.get();
}

unsigned Service::AddListeners(string_view endpoint_name, unsigned channel_token, bool hidden)
{
	std::promise<unsigned> p;
	std::future<unsigned> f(p.get_future());
	m_strand.post(vs::forward_sync_call(&Service::AddListeners_impl, this, p, endpoint_name, channel_token, hidden));
	return f.get();
}

boost::system::error_code Service::AddListener(const char* host, net::port port, net::protocol protocol, unsigned channel_token, bool hidden)
{
	std::promise<boost::system::error_code> p;
	std::future<boost::system::error_code> f(p.get_future());
	m_strand.post(vs::forward_sync_call(static_cast<void(Service::*)(std::promise<boost::system::error_code>&&, const char*, net::port, net::protocol, unsigned, bool)>(&Service::AddListener_impl), this, p, host, port, protocol, channel_token, hidden));
	return f.get();
}

boost::system::error_code Service::RemoveListener(const char* host, net::port port, net::protocol protocol)
{
	std::promise<boost::system::error_code> p;
	std::future<boost::system::error_code> f(p.get_future());
	m_strand.post(vs::forward_sync_call(static_cast<void(Service::*)(std::promise<boost::system::error_code>&&, const char*, net::port, net::protocol)>(&Service::RemoveListener_impl), this, p, host, port, protocol));
	return f.get();
}

unsigned Service::RemoveListeners(const char* host)
{
	std::promise<unsigned> p;
	std::future<unsigned> f(p.get_future());
	m_strand.post(vs::forward_sync_call(static_cast<void(Service::*)(std::promise<unsigned>&&, const char*)>(&Service::RemoveListeners_impl), this, p, host));
	return f.get();
}

void Service::GetListenerList(address_list& addresses, net::protocol protocol)
{
	std::promise<void> p;
	std::future<void> f(p.get_future());
	m_strand.post(vs::forward_sync_call(&Service::GetListenerList_impl, this, p, addresses, protocol));
	return f.get();
}

unsigned Service::GetListenerList(std::string& connection_string, net::protocol protocol)
{
	connection_string.clear();
	std::stringstream ss;

	address_list addresses;
	GetListenerList(addresses, protocol);

	if (addresses.empty())
		return 0;

	bool first = true;
	for (const auto& x: addresses)
	{
		if (!first)
			ss << ",";
		ss << x.first << ":"  << x.second;
		first = false;
	}
	assert(addresses.size() < std::numeric_limits<unsigned>::max());
	connection_string = ss.str();
	return static_cast<unsigned>(addresses.size());
}

bool Service::Test()
{
	std::promise<bool> p_internal;
	std::future<bool> f_internal(p_internal.get_future());
	m_strand.post(vs::forward_async_call(&Service::TestInternal_impl, this, p_internal));
	std::promise<bool> p_external;
	std::future<bool> f_external(p_external.get_future());
	// Not posted on strand because it waits on tasks executed on strand to complete
	get_io_service().post(vs::forward_async_call(&Service::TestExternal_impl, this, p_external));

	const auto test_deadline = std::chrono::steady_clock::now() + c_selftest_timeout;
	if (f_internal.wait_until(test_deadline) != std::future_status::ready)
		return false;
	if (f_external.wait_until(test_deadline) != std::future_status::ready)
		return false;
	return f_internal.get() && f_external.get();
}

void Service::GetMonitorInfo(Monitor::AcsReply & reply)
{
	vs::event ready(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ ready.set(); };
		for (const auto& x : m_handlers)
			if (auto h = x.handler.lock())
				reply.handlers.emplace_back(h->Name(), h->m_processed_connections, h->m_accepted_connections);
		for (auto listeners_iter = m_listeners.begin(); listeners_iter != m_listeners.end(); ++listeners_iter)
		{
			Monitor::AcsReply::Listener listener;
			(*listeners_iter)->FillMonitorStruct(listener);
			reply.listeners.push_back(listener);
		}
	});
	ready.wait();
}

void Service::Start_impl(std::promise<bool>&& p)
{
	m_work.emplace(get_io_service());
	ScheduleConnectionsCleanup();
	for (const auto& x : m_listeners)
		x->Start(x, shared_from_this(), vs::ignore<boost::system::error_code>());
	p.set_value(true);
}

void Service::Stop_impl(std::promise<void>&& p)
{
	for (const auto& x : m_listeners)
		x->Stop(vs::ignore<boost::system::error_code>());
	for (const auto& weak_p: m_connections)
		if (auto p = weak_p.lock())
			p->Close();
	m_connections_cleanup_timer.cancel();
	m_work.reset();
	p.set_value();
}

void Service::AddHandler_impl(std::promise<boost::system::error_code>&& p, string_view name, const std::weak_ptr<Handler>& handler, bool is_final)
{
	auto h = handler.lock();
	if (!h)
	{
		p.set_value(make_error_code(boost::system::errc::invalid_argument));
		return;
	}
	if (!h->Init(name))
	{
		p.set_value(errc::handler_initialization_failed);
		return;
	}
	m_handlers.emplace_back(handler, is_final);
	p.set_value(boost::system::error_code());
}

void Service::RemoveHandler_impl(std::promise<boost::system::error_code>&& p, string_view name)
{
	bool found = false;
	for (auto h_it = m_handlers.begin(); h_it != m_handlers.end(); /**/)
	{
		auto h = h_it->handler.lock();
		if (h && h->Name() == name)
		{
			found = true;
			h_it = m_handlers.erase(h_it);
		}
		else
			++h_it;
	}
	p.set_value(found ? boost::system::error_code() : errc::handler_not_found);
}

void Service::RemoveAllHandlers_impl(std::promise<void>&& p)
{
	m_handlers.clear();
	p.set_value();
}

void Service::AddListener_impl(std::promise<boost::system::error_code>&& p, const net::address& address, net::port port, net::protocol protocol, unsigned channel_token, bool hidden)
{
	assert((protocol == net::protocol::TCP || protocol == net::protocol::UDP) && "protocol shouldn't be a mask");

	auto l_it = std::find_if(m_listeners.begin(), m_listeners.end(), [&](const std::shared_ptr<Listener>& x) {
		return x->address == address && x->port == port && x->protocol == protocol;
	});

	if (l_it != m_listeners.end())
	{
		p.set_value(errc::listener_already_exists);
		return;
	}

	std::shared_ptr<Listener> listener;
	switch (protocol)
	{
	case net::protocol::TCP:
		listener = std::make_shared<ListenerTCP>(address, port, protocol, channel_token, hidden, get_io_service());
		break;
	case net::protocol::UDP:
		listener = std::make_shared<ListenerUDP>(address, port, protocol, channel_token, hidden);
		break;
	case net::protocol::TLS:
	case net::protocol::none:
		break;
	}
	if (!listener)
	{
		p.set_value(make_error_code(boost::system::errc::invalid_argument));
		return;
	}

	boost::system::error_code ec;
	listener->Start(listener, shared_from_this(), ec);
	if (ec)
	{
		p.set_value(ec);
		return;
	}

	m_listeners.push_back(std::move(listener));
	p.set_value(boost::system::error_code());
}

void Service::RemoveListener_impl(std::promise<boost::system::error_code>&& p, const net::address& address, net::port port, net::protocol protocol)
{
	assert((protocol == net::protocol::TCP || protocol == net::protocol::UDP) && "protocol shouldn't be a mask");

	auto l_it = std::find_if(m_listeners.begin(), m_listeners.end(), [&](const std::shared_ptr<Listener>& x) {
		return x->address == address && x->port == port && x->protocol == protocol;
	});

	if (l_it == m_listeners.end())
	{
		p.set_value(errc::listener_not_found);
		return;
	}

	boost::system::error_code ec;
	(*l_it)->Stop(ec);
	if (ec)
	{
		p.set_value(ec);
		return;
	}

	l_it->swap(m_listeners.back());
	m_listeners.pop_back();
	p.set_value(ec);
}

void Service::RemoveListeners_impl(std::promise<unsigned>&& p, const std::function<bool(const net::address&, net::port, net::protocol)>& predicate)
{
	unsigned n_removed = 0;
	auto l_it = m_listeners.begin();
	auto l_it_end = m_listeners.end();
	while (l_it != l_it_end)
	{
		if (!predicate((*l_it)->address, (*l_it)->port, (*l_it)->protocol))
		{
			++l_it;
			continue;
		}

		boost::system::error_code ec;
		(*l_it)->Stop(ec);
		if (ec)
		{
			++l_it;
			continue;
		}

		l_it->swap(m_listeners.back());
		--l_it_end;
		++n_removed;
	}
	if (l_it != m_listeners.end())
		m_listeners.erase(l_it, m_listeners.end());
	p.set_value(n_removed);
}

void Service::AddListeners_impl(std::promise<unsigned>&& p, string_view endpoint_name, unsigned channel_token, bool hidden)
{
	// This is a poor man's MapReduce:
	// Resolve all enpoints asyncronously and in parallel (Map)
	// Use endpoints to create listeners (Reduce)

	const auto n_tcp_endpoints = net::endpoint::GetCountAcceptTCP(endpoint_name, false);
	std::vector<std::vector<boost::asio::ip::tcp::endpoint>> tcp_endpoint_addresses;
	tcp_endpoint_addresses.resize(n_tcp_endpoints);
	const auto n_udp_endpoints = net::endpoint::GetCountAcceptUDP(endpoint_name, false);
	std::vector<std::vector<boost::asio::ip::tcp::endpoint>> udp_endpoint_addresses;
	udp_endpoint_addresses.resize(n_udp_endpoints);
	vs::latch resolve_completion(n_tcp_endpoints + n_udp_endpoints);
	for (unsigned i = 0; i < n_tcp_endpoints; ++i)
	{
		auto ra_tcp = net::endpoint::ReadAcceptTCP(i+1, endpoint_name);
		if (!ra_tcp)
		{
			resolve_completion.count_down(1);
			continue;
		}

		boost::system::error_code ec;
		auto address = net::address::from_string(ra_tcp->host, ec);
		if (!ec)
		{
			tcp_endpoint_addresses[i].emplace_back(address, ra_tcp->port);
			resolve_completion.count_down(1);
			continue;
		}

		m_resolver.async_resolve(boost::asio::ip::tcp::resolver::query(ra_tcp->host, std::to_string(ra_tcp->port)),
			[&resolve_completion, &tcp_endpoint_addresses, i](const boost::system::error_code& /*ec*/, boost::asio::ip::tcp::resolver::iterator r_it)
			{
				for (; r_it != boost::asio::ip::tcp::resolver::iterator(); ++r_it)
					tcp_endpoint_addresses[i].emplace_back(r_it->endpoint());
				resolve_completion.count_down(1);
			}
		);
	}
	for (unsigned i = 0; i < n_udp_endpoints; ++i)
	{
		auto ra_udp = net::endpoint::ReadAcceptUDP(i+1, endpoint_name);
		if (!ra_udp)
		{
			resolve_completion.count_down(1);
			continue;
		}

		boost::system::error_code ec;
		auto address = net::address::from_string(ra_udp->host, ec);
		if (!ec)
		{
			udp_endpoint_addresses[i].emplace_back(address, ra_udp->port);
			resolve_completion.count_down(1);
			continue;
		}

		m_resolver.async_resolve(boost::asio::ip::tcp::resolver::query(ra_udp->host, std::to_string(ra_udp->port)),
			[&resolve_completion, &udp_endpoint_addresses, i](const boost::system::error_code& /*ec*/, boost::asio::ip::tcp::resolver::iterator r_it)
			{
				for (; r_it != boost::asio::ip::tcp::resolver::iterator(); ++r_it)
					udp_endpoint_addresses[i].emplace_back(r_it->endpoint());
				resolve_completion.count_down(1);
			}
		);
	}
	resolve_completion.wait();

	unsigned n_added = 0;
	for (const auto& x: tcp_endpoint_addresses)
	{
		for (const auto& ep: x)
		{
			auto listener = std::make_shared<ListenerTCP>(ep.address(), ep.port(), net::protocol::TCP, channel_token, hidden, get_io_service());
			boost::system::error_code ec;
			listener->Start(listener, shared_from_this(), ec);
			if (ec)
				continue;

			m_listeners.push_back(std::move(listener));
			++n_added;
			break;
		}
	}
	for (const auto& x: udp_endpoint_addresses)
	{
		for (const auto& ep: x)
		{
			auto listener = std::make_shared<ListenerUDP>(ep.address(), ep.port(), net::protocol::UDP, channel_token, hidden);
			boost::system::error_code ec;
			listener->Start(listener, shared_from_this(), ec);
			if (ec)
				continue;

			m_listeners.push_back(std::move(listener));
			++n_added;
			break;
		}
	}
	p.set_value(n_added);
}

void Service::AddListener_impl(std::promise<boost::system::error_code>&& p, const char* host, net::port port, net::protocol protocol, unsigned channel_token, bool hidden)
{
	assert((protocol == net::protocol::TCP || protocol == net::protocol::UDP) && "protocol shouldn't be a mask");

	boost::system::error_code ec;
	auto r_it = m_resolver.resolve(boost::asio::ip::tcp::resolver::query(std::string(host), std::to_string(port)), ec);
	if (ec)
	{
		p.set_value(ec);
		return;
	}
	for (; r_it != boost::asio::ip::tcp::resolver::iterator(); ++r_it)
	{
		std::shared_ptr<Listener> listener;
		switch (protocol)
		{
		case net::protocol::TCP:
			listener = std::make_shared<ListenerTCP>(r_it->endpoint().address(), port, net::protocol::TCP, channel_token, hidden, get_io_service());
			break;
		case net::protocol::UDP:
			listener = std::make_shared<ListenerUDP>(r_it->endpoint().address(), port, net::protocol::UDP, channel_token, hidden);
			break;
		case net::protocol::TLS:
		case net::protocol::none:
			break;
		}
		if (!listener)
		{
			p.set_value(make_error_code(boost::system::errc::invalid_argument));
			return;
		}

		ec = boost::system::error_code();
		listener->Start(listener, shared_from_this(), ec);
		if (ec)
			continue;

		m_listeners.push_back(std::move(listener));
		break;
	}
	p.set_value(ec);
}

void Service::RemoveListener_impl(std::promise<boost::system::error_code>&& p, const char* host, net::port port, net::protocol protocol)
{
	assert((protocol == net::protocol::TCP || protocol == net::protocol::UDP) && "protocol shouldn't be a mask");

	boost::system::error_code ec;
	auto r_it = m_resolver.resolve(boost::asio::ip::tcp::resolver::query(std::string(host), std::to_string(port)), ec);
	if (ec)
	{
		p.set_value(ec);
		return;
	}
	for (; r_it != boost::asio::ip::tcp::resolver::iterator(); ++r_it)
	{
		const auto address = r_it->endpoint().address();
		for (auto l_it = m_listeners.begin(); l_it != m_listeners.end(); /**/)
		{
			if (!((*l_it)->address == address && (*l_it)->port == port && (*l_it)->protocol == protocol))
			{
				++l_it;
				continue;
			}

			ec = boost::system::error_code();
			(*l_it)->Stop(ec);
			if (ec)
			{
				++l_it;
				continue;
			}

			l_it->swap(m_listeners.back());
			m_listeners.pop_back();
			break;
		}
	}
	p.set_value(ec);
}

void Service::RemoveListeners_impl(std::promise<unsigned>&& p, const char* host)
{
	unsigned n_removed = 0;
	boost::system::error_code ec;
	auto r_it = m_resolver.resolve(boost::asio::ip::tcp::resolver::query(std::string(host), std::string()), ec);
	if (ec)
	{
		p.set_value(n_removed);
		return;
	}
	for (; r_it != boost::asio::ip::tcp::resolver::iterator(); ++r_it)
	{
		const auto address = r_it->endpoint().address();
		for (auto l_it = m_listeners.begin(); l_it != m_listeners.end(); /**/)
		{
			if (!((*l_it)->address == address))
			{
				++l_it;
				continue;
			}

			ec = boost::system::error_code();
			(*l_it)->Stop(ec);
			if (ec)
			{
				++l_it;
				continue;
			}

			l_it->swap(m_listeners.back());
			m_listeners.pop_back();
			++n_removed;
		}
	}
	p.set_value(n_removed);
}

void Service::GetListenerList_impl(std::promise<void>&& p, address_list& addresses, net::protocol protocol)
{
	addresses.clear();

	const bool is_listening_loopback_only = std::none_of(m_listeners.begin(), m_listeners.end(), [&](const std::shared_ptr<Listener>& x) {
		return (x->protocol & protocol) != net::protocol::none && !x->hidden && !x->address.is_loopback();
	});
	for (const auto& l : m_listeners)
	{
		if ((l->protocol & protocol) == net::protocol::none)
			continue;
		if (l->hidden)
			continue;
		if (l->address.is_loopback())
		{
			if (is_listening_loopback_only)
				addresses.emplace_back(l->address, l->port);
		}
		else if (l->address.is_unspecified())
		{
			auto iis = net::GetInterfaceInfo();
			if (l->address.is_v4())
				for (const auto& ii: *iis)
					for (const auto& i_addr: ii.addr_local_v4)
						addresses.emplace_back(i_addr, l->port);
			else if (l->address.is_v6())
				for (const auto& ii: *iis)
					for (const auto& i_addr : ii.addr_local_v6)
						addresses.emplace_back(i_addr, l->port);
		}
		else
			addresses.emplace_back(l->address, l->port);
	}
	p.set_value();
}

void Service::TestInternal_impl(std::promise<bool>&& p)
{
	unsigned n_visible = 0;
	unsigned n_faulty = 0;
	for (const auto& l : m_listeners)
	{
		if (l->hidden)
			continue;
		++n_visible;

		if (!l->IsOpen())
			++n_faulty;
	}

	if (n_visible == 0)
	{
		dstream0 << "ACS: self test: no listeners\n";
		p.set_value(false);
		return;
	}

	if (n_faulty > 0)
	{
		dstream0 << "ACS: self test: " << n_faulty << "faulty listeners\n";
		p.set_value(false);
		return;
	}

	p.set_value(true);
}

void Service::TestExternal_impl(std::promise<bool>&& p)
{
	address_list addresses;
	GetListenerList(addresses, net::protocol::TCP);

	std::vector<boost::asio::ip::tcp::socket> sockects;
	sockects.reserve(addresses.size());
	std::atomic<unsigned> n_connected(0);
	vs::latch connect_completion(addresses.size());
	for (auto& x: addresses)
	{
		if (x.first.is_unspecified())
		{
			if (x.first.is_v4())
				x.first = net::address_v4::loopback();
			else if (x.first.is_v6())
				x.first = net::address_v6::loopback();
		}

		sockects.emplace_back(get_io_service());
		auto socket = &sockects.back();
		boost::asio::ip::tcp::endpoint ep(x.first, x.second);
		socket->async_connect(ep,
			[&connect_completion, &n_connected, socket, ep](const boost::system::error_code& ec)
			{
				if (ec)
				{
					dstream1 << "ACS: self test: connection to " << ep << " failed: " << ec.message();
					connect_completion.count_down(1);
					return;
				}

				boost::asio::async_write(*socket, boost::asio::buffer(c_selftest_data),
					[&connect_completion, &n_connected, ep](const boost::system::error_code& ec, size_t bytes_transferred)
					{
						if (ec)
						{
							dstream1 << "ACS: self test: write to " << ep << " failed: " << ec.message();
							connect_completion.count_down(1);
							return;
						}

						const auto expectced_write_size = sizeof(c_selftest_data);
						if (bytes_transferred != expectced_write_size)
						{
							dstream1 << "ACS: self test: write to " << ep << " is incomplete, only " << bytes_transferred << " out of " << expectced_write_size << " bytes were written\n";
							connect_completion.count_down(1);
							return;
						}

						n_connected.fetch_add(1, std::memory_order_relaxed);
						connect_completion.count_down(1);
					}
				);
			}
		);
	}
	connect_completion.wait();

	p.set_value(n_connected == addresses.size());
}

Listener::Listener(const net::address& address_, net::port port_, net::protocol protocol_, unsigned channel_token_, bool hidden_)
	: address(address_)
	, port(port_)
	, protocol(protocol_)
	, channel_token(channel_token_)
	, hidden(hidden_)
{
}

void acs::Listener::FillMonitorStruct(Monitor::AcsReply::Listener & listener)
{
	listener.ip = this->address.to_string();
	listener.port = this->port;
	switch (protocol)
	{
	case net::protocol::none:
		listener.protocol = std::string("none");
		break;
	case net::protocol::TCP:
		listener.protocol = std::string("TCP");
		break;
	case net::protocol::UDP:
		listener.protocol = std::string("UDP");
		break;
	case net::protocol::TLS:
		listener.protocol = std::string("TLS");
		break;
	default:
		break;
	}

	listener.rcvd_connections = rcvd_connections;
	listener.unseccessful_connections = unseccessful_connections;
}

ListenerTCP::ListenerTCP(const net::address& address_, net::port port_, net::protocol protocol_, unsigned channel_token_, bool hidden_, boost::asio::io_service& ios)
	: Listener(address_, port_, protocol_, channel_token_, hidden_)
	, m_acceptor(ios)
{
}

void ListenerTCP::Start(const std::shared_ptr<Listener>& self, std::shared_ptr<Service> acs, boost::system::error_code& ec)
{
	assert(this == self.get());

	if (m_acceptor.is_open())
	{
		assert(m_acs);
		return;
	}
	m_acceptor.open(address.is_v4() ? boost::asio::ip::tcp::v4() : boost::asio::ip::tcp::v6(), ec);
	if (ec)
	{
		dstream2 << "ACS: ListenerTCP(" << address << ":" << port << "): open failed: " << ec.message();
		return;
	}
	m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
	if (ec)
	{
		dstream4 << "ACS: ListenerTCP(" << address << ":" << port << "): set SO_REUSEADDR failed: " << ec.message();
		// That isn't a fatal error, we can still try bind to desired address.
	}
	m_acceptor.bind(boost::asio::ip::tcp::endpoint(address, port), ec);
	if (ec)
	{
		dstream2 << "ACS: ListenerTCP(" << address << ":" << port << "): bind failed: " << ec.message();
		return;
	}
	m_acceptor.listen(boost::asio::ip::tcp::socket::max_connections, ec);
	if (ec)
	{
		dstream2 << "ACS: ListenerTCP(" << address << ":" << port << "): listen failed: " << ec.message();
		return;
	}
	m_acs = std::move(acs);
	StartAccept(self);
#if ACS_VERBOSE_LOGS
	dstream4 << "ACS: ListenerTCP(" << address << ':' << port << "): started\n";
#endif
}

void ListenerTCP::Stop(boost::system::error_code& ec)
{
	if (!m_acceptor.is_open())
	{
		assert(!m_acs);
		return;
	}

	m_acceptor.cancel(ec);
	if (ec)
	{
		dstream2 << "ACS: ListenerTCP(" << address << ":" << port << "): cancel failed: " << ec.message();
		return;
	}
	m_acceptor.close(ec);
	if (ec)
	{
		dstream2 << "ACS: ListenerTCP(" << address << ":" << port << "): close failed: " << ec.message();
		return;
	}
	m_acs = nullptr;
#if ACS_VERBOSE_LOGS
	dstream4 << "ACS: ListenerTCP(" << address << ':' << port << "): stopped\n";
#endif
}

bool ListenerTCP::IsOpen()
{
	return m_acceptor.is_open();
}

void ListenerTCP::StartAccept(std::shared_ptr<Listener> self)
{
	//TODO (ap): listener and their connections incr
	assert(m_acceptor.is_open());
	assert(m_acs);

	auto connection = std::make_shared<ConnectionTCP>(m_acs, channel_token);

	m_acceptor.async_accept(connection->Socket(), m_acs->m_strand.wrap(
		[this, connection, self = std::move(self)](const boost::system::error_code& ec) mutable
		{
			if (ec == boost::asio::error::operation_aborted)
				return;
			if (!m_acceptor.is_open())
				return;

			if (ec)
			{
				unseccessful_connections++;
				dstream2 << "ACS: ListenerTCP(" << address << ":" << port << "): accept failed: " << ec.message();
				return;
			}
			else
			{
				m_acs->m_connections.emplace_back(connection);
				connection->Start(connection);
				rcvd_connections++;
			}

			StartAccept(std::move(self));
		}
	));
}

ListenerUDP::ListenerUDP(const net::address& address_, net::port port_, net::protocol protocol_, unsigned channel_token_, bool hidden_)
	: Listener(address_, port_, protocol_, channel_token_, hidden_)
{
}

void ListenerUDP::Start(const std::shared_ptr<Listener>& self, std::shared_ptr<Service> acs, boost::system::error_code& ec)
{
	assert(this == self.get());

	if (m_router)
	{
		assert(m_acs);
		return;
	}
	m_router = net::UDPRouter::Get(net::UDPRouter::endpoint_type(address, port), acs->get_io_service(), ec);
	if (ec)
	{
		dstream2 << "ACS: ListenerUDP(" <<  address << ":" << port << "): open failed: " << ec.message();
		return;
	}
	m_acs = std::move(acs);
	StartAccept(self);
#if ACS_VERBOSE_LOGS
	dstream4 << "ACS: ListenerUDP(" <<  address << ":" << port << "): started\n";
#endif
}

void ListenerUDP::Stop(boost::system::error_code&)
{
	if (!m_router)
	{
		assert(!m_acs);
		return;
	}

	m_router->CloseWhenUnused();
	m_acs = nullptr;
#if ACS_VERBOSE_LOGS
	dstream4 << "ACS: ListenerUDP(" <<  address << ":" << port << "): stopped\n";
#endif
}

bool ListenerUDP::IsOpen()
{
	return true;
}

void ListenerUDP::StartAccept(std::shared_ptr<Listener> self)
{
	assert(m_router);
	assert(m_acs);

	// We can't use strand::wrap because it doesn't support move only handler arguments.
	m_router->AsyncAccept([this, self = std::move(self)](const boost::system::error_code& ec, net::UDPConnection udp_conn) mutable
		{
			if (ec == boost::asio::error::operation_aborted)
				return;

			m_acs->m_strand.dispatch(vs::move_handler([this, self = std::move(self), ec, udp_conn = std::move(udp_conn)]() mutable
			{
				if (!m_router)
					return;

				if (ec)
				{
					dstream2 << "ACS: ListenerUDP(" <<  address << ":" << port << "): accept failed: " << ec.message();
					return;
				}
				else
				{
					auto connection = std::make_shared<ConnectionUDP>(m_acs, std::move(udp_conn), channel_token);
					m_acs->m_connections.emplace_back(connection);
					connection->Start(connection);
				}

				StartAccept(std::move(self));
			}));
		}
	);
}

void Service::ScheduleConnectionsCleanup()
{
	m_connections_cleanup_timer.expires_from_now(c_connections_cleanup_period);
	m_connections_cleanup_timer.async_wait(m_strand.wrap(
		[this, self = shared_from_this()](const boost::system::error_code& ec)
		{
			if (ec == boost::asio::error::operation_aborted || !m_work)
				return;

			const auto it = std::remove_if(m_connections.begin(), m_connections.end(), [](const std::weak_ptr<Connection>& p) { return p.expired(); });
#if ACS_VERBOSE_LOGS
			dstream4 << "ACS: ConnectionsCleanup: " << std::distance(it, m_connections.end()) << " cleaned out of " << m_connections.size();
#endif
			m_connections.erase(it, m_connections.end());
			ScheduleConnectionsCleanup();
		}
	));
}

Connection::Connection(std::shared_ptr<Service> acs, unsigned channel_token)
	: m_acs(std::move(acs))
	, m_channel_token(channel_token)
{
}

ConnectionTCP::ConnectionTCP(std::shared_ptr<Service> acs, unsigned channel_token)
	: Connection(std::move(acs), channel_token)
	, m_socket(m_acs->m_strand.get_io_service())
	, m_data_size(0)
	, m_claimed_by_handler(false)
{
}

const std::string& ConnectionTCP::LogID() const
{
	if (m_log_id.empty())
		m_log_id = logh::GetSocketEndpointsStr(m_socket);
	return m_log_id;
}

void ConnectionTCP::Start(const std::shared_ptr<Connection>& self)
{
	assert(this == self.get());
	m_handlers.assign(m_acs->m_handlers.begin(), m_acs->m_handlers.end());
#if ACS_VERBOSE_LOGS
	dstream4 << "ACS: ConnectionTCP(" << LogID() << "): new\n";
#endif
	StartRead(self);
}

void ConnectionTCP::Close()
{
	if (!m_socket.is_open())
		return;

	LogID(); // To cache log id

	boost::system::error_code ec;
	m_socket.close(ec);
	if (ec)
	{
		dstream2 << "ACS: ConnectionTCP(" << LogID() << "): close failed: " << ec.message();
		return;
	}
}

void ConnectionTCP::StartRead(std::shared_ptr<Connection> self)
{
	assert(m_data_size <= c_max_read_size);
	const size_t read_size = std::min(c_max_read_size - m_data_size, std::max(c_read_size, m_socket.available(vs::ignore<boost::system::error_code>{})));
	if (read_size == 0)
	{
		dstream2 << "ACS: ConnectionTCP(" << LogID() << "): buffer exceeded limit (" << m_data_size << ")\n";
		return;
	}
	m_buffer.resize(m_data_size + read_size);

	m_socket.async_read_some(boost::asio::buffer(m_buffer.data() + m_data_size, read_size), m_acs->m_strand.wrap(
		[this, self = std::move(self)](const boost::system::error_code& ec, size_t bytes_transferred) mutable
		{
			if (ec == boost::asio::error::operation_aborted)
				return;

			if (ec)
			{
				dstream2 << "ACS: ConnectionTCP(" << LogID() << "): read failed: " << ec.message();
				return;
			}
			if (!m_socket.is_open())
			{
				dstream4 << "ACS: ConnectionTCP(" << LogID() << "): connection was closed before dispatch was finished\n";
				return;
			}
#if ACS_VERBOSE_LOGS
			dstream4 << "ACS: ConnectionTCP(" << LogID() << "): read " << bytes_transferred << " bytes\n";
#endif
			if (bytes_transferred == 0)
			{
				StartRead(std::move(self));
				return;
			}

			m_data_size += bytes_transferred;
			m_buffer.resize(m_data_size);

			bool dispatched = true; // default in case connection will be rejected by all handlers
			for (auto& hi: m_handlers)
			{
				assert(hi.response != Response::accept_connection);
				if (hi.response == Response::not_my_connection)
					continue;
				if (m_claimed_by_handler && hi.response != Response::my_connection)
					continue;
				auto h = hi.handler.lock();
				if (!h)
					continue;

				hi.response = h->Protocol(m_buffer, m_channel_token);
#if ACS_VERBOSE_LOGS
				dstream4 << "ACS: ConnectionTCP(" << LogID() << "): handler \"" << h->Name() << "\" response: " << hi.response;
#endif
				if (hi.response == Response::next_step)
				{
					dispatched = false;
					if (m_claimed_by_handler)
					{
						dstream4 << "ACS: Handler(" << h->Name() << "): incorrect behaviour: returned next_step after my_connection\n";
						hi.response = Response::my_connection;
					}
				}
				else if (hi.response == Response::my_connection)
				{
					dispatched = false;
					m_claimed_by_handler = true;
					++h->m_processed_connections;
				}
				else if (hi.response == Response::accept_connection)
				{
#if ACS_VERBOSE_LOGS
					dstream4 << "ACS: ConnectionTCP(" << LogID() << "): accepted by handler \"" << h->Name() << "\"\n";
#endif
					dispatched = true;
					m_claimed_by_handler = true;
					++h->m_accepted_connections;
					h->Accept(std::move(m_socket), std::move(m_buffer));
					break;
				}
			}
			if (dispatched)
			{
#if ACS_VERBOSE_LOGS
				if (!m_claimed_by_handler)
					dstream4 << "ACS: ConnectionTCP(" << LogID() << "): rejected by all handlers\n";
#endif
			}
			else
				StartRead(std::move(self));
		}
	));
}

ConnectionUDP::ConnectionUDP(std::shared_ptr<Service> acs, net::UDPConnection udp_conn, unsigned channel_token)
	: Connection(std::move(acs), channel_token)
	, m_connection(std::move(udp_conn))
	, m_buffer(c_max_read_size)
	, m_claimed_by_handler(false)
{
}

const std::string& ConnectionUDP::LogID() const
{
	if (m_log_id.empty())
	{
		std::stringstream ss;
		ss << m_connection.local_endpoint() << "<->" << m_connection.remote_endpoint();
		m_log_id = ss.str();
	}
	return m_log_id;
}

void ConnectionUDP::Start(const std::shared_ptr<Connection>& self)
{
	assert(this == self.get());
	m_handlers.assign(m_acs->m_handlers.begin(), m_acs->m_handlers.end());
#if ACS_VERBOSE_LOGS
	dstream4 << "ACS: ConnectionUDP(" << LogID() << "): new\n";
#endif
	StartRead(self);
}

void ConnectionUDP::Close()
{
	m_connection = {};
}

void ConnectionUDP::StartRead(std::shared_ptr<Connection> self)
{
	const size_t read_size = m_buffer.Empty() ? m_buffer.GetMaxBlockSize() : m_buffer.GetChunkFreeSpace();
	if (read_size == 0)
	{
		dstream2 << "ACS: ConnectionUDP(" << LogID() << "): buffer exceeded limit";
		return;
	}

	m_buffer.PushBack(nullptr, read_size);
	m_connection.async_receive(boost::asio::buffer(m_buffer.Back().Data(), m_buffer.Back().Size()), m_acs->m_strand.wrap(
		[this, self = std::move(self)](const boost::system::error_code& ec, size_t bytes_transferred) mutable
		{
			if (ec == boost::asio::error::operation_aborted)
				return;

			if (ec)
			{
				dstream2 << "ACS: ConnectionUDP(" << LogID() << "): read failed: " << ec.message();
				return;
			}
			if (!m_connection.is_open())
			{
				dstream4 << "ACS: ConnectionUDP(" << LogID() << "): connection was closed before dispatch was finished\n";
				return;
			}
#if ACS_VERBOSE_LOGS
			dstream4 << "ACS: ConnectionUDP(" << LogID() << "): read " << bytes_transferred << " bytes\n";
#endif

			assert(bytes_transferred <= m_connection.GetRouter().GetMaxPacketSize());
			m_buffer.ResizeBack(bytes_transferred);

			bool dispatched = true; // default in case connection will be rejected by all handlers
			for (auto& hi: m_handlers)
			{
				assert(hi.response != Response::accept_connection);
				if (hi.response == Response::not_my_connection)
					continue;
				if (m_claimed_by_handler && hi.response != Response::my_connection)
					continue;
				auto h = hi.handler.lock();
				if (!h)
					continue;

				hi.response = h->Protocol(m_buffer, m_channel_token);
#if ACS_VERBOSE_LOGS
				dstream4 << "ACS: ConnectionUDP(" << LogID() << "): handler \"" << h->Name() << "\" response: " << hi.response;
#endif
				if (hi.response == Response::next_step)
				{
					dispatched = false;
					if (m_claimed_by_handler)
					{
						dstream4 << "ACS: Handler(" << h->Name() << "): incorrect behaviour: returned next_step after my_connection\n";
						hi.response = Response::my_connection;
					}
				}
				else if (hi.response == Response::my_connection)
				{
					dispatched = false;
					m_claimed_by_handler = true;
					++h->m_processed_connections;
				}
				else if (hi.response == Response::accept_connection)
				{
#if ACS_VERBOSE_LOGS
					dstream4 << "ACS: ConnectionUDP(" << LogID() << "): accepted by handler \"" << h->Name() << "\"\n";
#endif
					dispatched = true;
					m_claimed_by_handler = true;
					++h->m_accepted_connections;
					h->Accept(std::move(m_connection), std::move(m_buffer));
					break;
				}
			}
			if (dispatched)
			{
#if ACS_VERBOSE_LOGS
				if (!m_claimed_by_handler)
					dstream4 << "ACS: ConnectionUDP(" << LogID() << "): rejected by all handlers\n";
#endif
			}
			else
				StartRead(std::move(self));
		}
	));
}

}
