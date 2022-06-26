#if defined(_WIN32) //not ported

#include "VS_TorrentStarter.h"

#include <boost/signals2.hpp>
#include <boost/make_shared.hpp>

#include "std/cpplib/VS_Lock.h"
#include "std/cpplib/VS_WorkThreadIOCP.h"

#include "VS_TorrentService.h"

#include "acs/AccessConnectionSystem/VS_AccessConnectionHandler.h"
#include "acs/connection/VS_BufferedConnectionBase.h"

#include "tools/Server/VS_Server.h"
#include "tools/Server/VS_ServerComponentsInterface.h"
#include "acs/AccessConnectionSystem/VS_AccessConnectionSystem.h"

namespace
{
	class VS_ProxyConnection : public VS_BufferedConnectionBase, VS_Lock
	{
		std::shared_ptr<char> m_Counter;
		std::shared_ptr<VS_ProxyConnection> m_pairConnection;

		size_t onReceive(const void* data, size_t size) override
		{
			VS_AutoLock l(this);
			auto c = m_pairConnection;
			if (c)
			{
				vs::SharedBuffer sb(size);
				std::memcpy(sb.data<void>(), data, size);
				c->Send(std::move(sb));
				return size;
			}
			return 0;
		}

		void onError(unsigned /*err*/) override
		{
			VS_AutoLock l(this);
			if (m_pairConnection) m_pairConnection->Shutdown();
			m_pairConnection.reset();
		}

		void onConnect(bool result) override
		{
			if (result == false)
			{
				VS_AutoLock l(this);
				Shutdown();
				if (m_pairConnection) m_pairConnection->Shutdown();
				m_pairConnection.reset();
			}
		}

	public:
		VS_ProxyConnection(const std::shared_ptr<char> &c, const std::shared_ptr<VS_ProxyConnection> &pair)
		{
			m_Counter = c;
			m_pairConnection = pair;
		}

		VS_ProxyConnection(const std::shared_ptr<char> &c)
		{
			m_Counter = c;
		}

		void SetRemoteEndpoint(const VS_IPPortAddress &addr, const boost::shared_ptr<VS_WorkThread> &thread)
		{
			VS_AutoLock l(this);
			m_pairConnection = std::make_shared<VS_ProxyConnection>(m_Counter,
				std::static_pointer_cast<VS_ProxyConnection>(shared_from_this()));
			m_pairConnection->Init();
			m_pairConnection->Connect(addr, thread);
		}

		void Shutdown()
		{
			decltype(m_pairConnection) tmp;
			{
				VS_AutoLock l(this);
				std::swap(tmp, m_pairConnection);
			}
			if (tmp)
				tmp->Shutdown();
			VS_BufferedConnectionBase::Shutdown();
		}
	};
}// anonymous namespace

class VS_TorrentProxy : public VS_AccessConnectionHandler, VS_Lock
{
public:
	VS_TorrentProxy(std::weak_ptr<VS_TorrentService> service, string_view name);
	~VS_TorrentProxy() {}

private:
	bool			Init(const char *handler_name) override;
	VS_ACS_Response	Connection(unsigned long *in_len) override;
	void			Accept(VS_ConnectionTCP *conn, const void *in_buffer,
		const unsigned long in_len, const void *context) override;
	void			Destructor(const void *context) override;
	void			Destroy(const char *handler_name) override;
	char *			HandlerName(void) const override;

	VS_ACS_Response	Protocol(const void *in_buffer, unsigned long *in_len,
		void **/*out_buffer*/, unsigned long */*out_len*/,
		void **/*context*/) override
	{
		char signature[] = "\x13" "BitTorrent protocol";	// start of handshake message <pstrlen><pstr>...

		long len = *in_len;
		if (*in_len > sizeof(signature) - 1) len = sizeof(signature) - 1;
		if (memcmp(signature, in_buffer, len) != 0) return vs_acs_connection_is_not_my;

		if (len == sizeof(signature) - 1) return vs_acs_accept_connections;

		*in_len = sizeof(signature) - 1;
		return vs_acs_next_step;
	};
private:
	std::weak_ptr<VS_TorrentService> m_torrentService;
	std::string m_handlerName;
	std::string m_ip;

	boost::signals2::signal< void() > m_Shutdown;
	std::weak_ptr<char> m_Counter;
	bool m_is_shutting_down;
	boost::shared_ptr<VS_WorkThread> m_thread;
};


VS_TorrentProxy::VS_TorrentProxy(std::weak_ptr<VS_TorrentService> service, string_view name) : m_torrentService(std::move(service))
{
	m_thread = boost::make_shared<VS_WorkThreadIOCP>();
	m_thread->Start("TorrentProxy");

	m_handlerName = std::string{ name };
	m_ip = "127.0.0.1";
	m_is_shutting_down = false;
}

bool VS_TorrentProxy::Init(const char */*handler_name*/)
{
	return true;
}

VS_ACS_Response	VS_TorrentProxy::Connection(unsigned long *in_len)
{
	if (in_len)		*in_len = 1;
	return vs_acs_next_step;
}

void VS_TorrentProxy::Accept(VS_ConnectionTCP *conn, const void *in_buffer, const unsigned long in_len, const void *context)
{
	auto service = m_torrentService.lock();
	if (!service)
		return;

	const net::port port = service->ListenPort();

	if (port == 0)
		return;

	VS_AutoLock l(this);
	if (m_is_shutting_down) return;

	std::shared_ptr<char> cnt = m_Counter.lock();
	if (!cnt)
	{
		cnt.reset(new char);
		m_Counter = cnt;
	}

	VS_IPPortAddress addr;
	addr.SetIPFromHostName(m_ip.c_str());
	addr.port(port);
	auto c = std::make_shared<VS_ProxyConnection>(cnt);
	c->Init();
	c->SetRemoteEndpoint(addr, m_thread);
	c->SetConnection(conn, m_thread, in_buffer, in_len);

	std::weak_ptr<VS_ProxyConnection> w_c(c);
	m_Shutdown.connect([w_c]() {
		if (auto conn = w_c.lock())
			conn->Shutdown();
	});
}

void VS_TorrentProxy::Destructor(const void *context)
{
	// empty....
}

void VS_TorrentProxy::Destroy(const char *handler_name)
{
	VS_AutoLock l(this);
	m_is_shutting_down = true;
	m_Shutdown();
	while (m_Counter.lock())
	{
		::Sleep(1000);
	}
}

char *VS_TorrentProxy::HandlerName(void) const
{
	return const_cast<char *>(m_handlerName.c_str());
}

VS_TorrentStarter::VS_TorrentStarter(boost::asio::io_service& ios)
	: VS_TorrentStarterBase(ios)
{
}

VS_TorrentStarter::~VS_TorrentStarter()
{
}

void VS_TorrentStarter::AddHandler_ACS(string_view name)
{
	assert(Service() != nullptr);

	if (VS_Server::srv_components && VS_Server::srv_components->GetAcs()) {
		m_torrentProxy = vs::make_unique<VS_TorrentProxy>(Service(), name);
		VS_Server::srv_components->GetAcs()->AddHandler(std::string(name).c_str(), m_torrentProxy.get());
	}
}

void VS_TorrentStarter::RemoveHandler_ACS(string_view name)
{
	if (VS_Server::srv_components && VS_Server::srv_components->GetAcs())
		VS_Server::srv_components->GetAcs()->RemoveHandler(std::string(name).c_str());

	m_torrentProxy.reset();
}

vs::set<std::pair<std::string, net::port>> VS_TorrentStarter::Listeners_ACS()
{
	vs::set<std::pair<std::string, net::port>> addrs;

	if (VS_Server::srv_components && VS_Server::srv_components->GetAcs())
	{
		std::string addresses;
		VS_Server::srv_components->GetAcs()->GetListeners(addresses);
		if (!addresses.empty())
		{
			addresses.push_back(','); // Finalize string with address separator to simplify parsing
			auto addr_start = addresses.begin();
			while (true)
			{
				auto addr_sep = std::find(addr_start, addresses.end(), ',');
				if (addr_sep == addresses.cend())
					break;
				*addr_sep = '\0';

				net::port port = 0;
				// Try to find address and port separator, that is the last ':' unless it's a part of IPv6 address (located inside '[' ']')
				auto port_sep = addr_sep;
				for (; port_sep != addr_start; --port_sep)
					if (*port_sep == ':' || *port_sep == ']')
						break;
				if (*port_sep == ':')
				{
					*port_sep = '\0';
					auto port_start = port_sep + 1;
					if (port_start != addresses.cend())
						port = ::atoi(&*port_start); // if port -eq 0 it means usage default port for this protocol
				}

				const char * const host = &*addr_start;

				addrs.emplace(host, port);

				addr_start = addr_sep + 1;
			}
		}
	}

	return addrs;
}

#endif //not ported
