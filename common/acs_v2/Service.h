#pragma once

#include "../net/Types.h"
#include "std-generic/attributes.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/macro_utils.h"
#include "../tools/Watchdog/VS_Testable.h"
#include "Monitor.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/optional.hpp>

#include <atomic>
#include <future>
#include <list>
#include <vector>

#include "std-generic/undef_windows.h" // this should be last

namespace acs {

class Handler;

extern std::atomic<unsigned> g_last_channel_token;

template <class Tag>
inline unsigned ChannelToken()
{
	// TODO: Replace with plain static unsigned in MSVS2015 (magic statics)
	static std::atomic<unsigned> value(++g_last_channel_token);
	return value.load();
}

class Connection;
class ConnectionTCP;
class ConnectionUDP;
class Listener;
class ListenerTCP;
class ListenerUDP;

class Service : public VS_Testable, public std::enable_shared_from_this<Service>
{
public:
	static std::shared_ptr<Service> Create(boost::asio::io_service& ios);
	template <class Factory>
	static void SetFactory(Factory&& factory)
	{
		s_factory = std::forward<Factory>(factory);
	}
private:
	static std::function<std::shared_ptr<Service>(boost::asio::io_service& ios)> s_factory;

public:
	explicit Service(boost::asio::io_service& ios); // TODO: make private, (create interface?)
	virtual ~Service();

	boost::asio::io_service& get_io_service()
	{
		return m_strand.get_io_service();
	}

	std::future<bool> Start();
	std::future<void> Stop();

	boost::system::error_code AddHandler(string_view name, const std::weak_ptr<Handler>& handler, bool is_final = true);
	boost::system::error_code RemoveHandler(string_view name);
	void RemoveAllHandlers();

	boost::system::error_code AddListener(const net::address& address, net::port port, net::protocol protocol, unsigned channel_token = 0, bool hidden = false);
	boost::system::error_code RemoveListener(const net::address& address, net::port port, net::protocol protocol);
	unsigned RemoveListeners(const std::function<bool(const net::address&, net::port, net::protocol)>& predicate);
	void RemoveAllListeners()
	{
		RemoveListeners([](const net::address&, net::port, net::protocol) { return true; });
	}
	unsigned RemoveListeners(const net::address& address)
	{
		return RemoveListeners([&address](const net::address& address_, net::port, net::protocol) { return address_ == address; });
	}
	unsigned RemoveListeners(net::port port)
	{
		return RemoveListeners([&port](const net::address&, net::port port_, net::protocol) { return port_ == port; });
	}
	unsigned RemoveListeners(net::protocol protocol)
	{
		return RemoveListeners([&protocol](const net::address&, net::port, net::protocol protocol_) { return protocol_ == protocol; });
	}
	unsigned AddListeners(string_view endpoint_name, unsigned channel_token = 0, bool hidden = false);

	VS_DEPRECATED boost::system::error_code AddListener(const char* host, net::port port, net::protocol protocol, unsigned channel_token = 0, bool hidden = false);
	VS_DEPRECATED boost::system::error_code RemoveListener(const char* host, net::port port, net::protocol protocol);
	VS_DEPRECATED unsigned RemoveListeners(const char* host);

	typedef std::vector<std::pair<net::address, net::port>> address_list;
	void GetListenerList(address_list& addresses, net::protocol protocol);
	unsigned GetListenerList(std::string& connection_string, net::protocol protocol);

	VS_NODISCARD bool Test() override;

	void GetMonitorInfo(Monitor::AcsReply& reply);

private:
	void Start_impl(std::promise<bool>&& p);
	void Stop_impl(std::promise<void>&& p);

	void AddHandler_impl(std::promise<boost::system::error_code>&& p, string_view name, const std::weak_ptr<Handler>& handler, bool is_final);
	void RemoveHandler_impl(std::promise<boost::system::error_code>&& p, string_view name);
	void RemoveAllHandlers_impl(std::promise<void>&& p);

	void AddListener_impl(std::promise<boost::system::error_code>&& p, const net::address& address, net::port port, net::protocol protocol, unsigned channel_token, bool hidden);
	void RemoveListener_impl(std::promise<boost::system::error_code>&& p, const net::address& address, net::port port, net::protocol protocol);
	void RemoveListeners_impl(std::promise<unsigned>&& p, const std::function<bool(const net::address&, net::port, net::protocol)>& predicate);
	void AddListeners_impl(std::promise<unsigned>&& p, string_view endpoint_name, unsigned channel_token, bool hidden);

	void AddListener_impl(std::promise<boost::system::error_code>&& p, const char* host, net::port port, net::protocol protocol, unsigned channel_token, bool hidden);
	void RemoveListener_impl(std::promise<boost::system::error_code>&& p, const char* host, net::port port, net::protocol protocol);
	void RemoveListeners_impl(std::promise<unsigned>&& p, const char* host);

	void GetListenerList_impl(std::promise<void>&& p, address_list& addresses, net::protocol protocol);
	void TestInternal_impl(std::promise<bool>&& p);
	void TestExternal_impl(std::promise<bool>&& p);

private:
	friend Connection;
	friend ConnectionTCP;
	friend ConnectionUDP;
	friend ListenerTCP;
	friend ListenerUDP;

	boost::asio::io_service::strand m_strand;
	boost::optional<boost::asio::io_service::work> m_work;
	boost::asio::ip::tcp::resolver m_resolver;

	struct handler_info
	{
		VS_FORWARDING_CTOR2(handler_info, handler, is_final) {}
		std::weak_ptr<Handler> handler;
		bool is_final;
	};
	std::list<handler_info> m_handlers;
	std::vector<std::shared_ptr<Listener>> m_listeners;
	std::vector<std::weak_ptr<Connection>> m_connections;
	boost::asio::steady_timer m_connections_cleanup_timer;

	void ScheduleConnectionsCleanup();
};

}
