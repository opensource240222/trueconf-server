#pragma once

#include "fwd.h"
#include "net/BufferedConnectionInterface.h"
#include "std-generic/cpplib/string_view.h"
#include "std/cpplib/MakeShared.h"

#include <boost/signals2.hpp>

#include "net/Port.h"
#include "net/Connect.h"

#include "std-generic/compat/memory.h"

namespace vs { class SharedBuffer; }
class VS_StreamBufferedConnection
	: public net::BufferedConnectionInterface::IOHandler
	, public vs::enable_shared_from_this<VS_StreamBufferedConnection>
{
public:
	template <class Protocol, class ConnectHandler>
	static void Create(
		boost::asio::io_service& ios,
		const typename Protocol::endpoint& ep,
		size_t max_write_data_size,
		size_t read_buffer_size,
		ConnectHandler&& connect_handler)
	{
		net::Connect<Protocol>(ios, ep,
			[
				max_write_data_size,
				read_buffer_size,
				connect_handler = std::forward<ConnectHandler>(connect_handler)
			](const boost::system::error_code& ec, typename Protocol::socket&& socket)
		{
			if (ec)
			{
				connect_handler(ec, nullptr);
				return;
			}

			auto connection = std::make_shared<net::BufferedConnectionInterface_impl<typename Protocol::socket>>(
				socket.get_io_service(), max_write_data_size, read_buffer_size
			);
			auto result = vs::MakeShared<VS_StreamBufferedConnection>(connection);
			connection->SetSocket(std::move(socket));
			connect_handler(ec, std::move(result));
		});
	}

protected:
	explicit VS_StreamBufferedConnection(std::shared_ptr<net::BufferedConnectionInterface> connection);
	static void PostConstruct(std::shared_ptr<VS_StreamBufferedConnection>& p)
	{
		p->m_connection->SetIOHandler(p);
	}

public:
	~VS_StreamBufferedConnection(void);

	boost::signals2::signal<void (const vs::SharedBuffer&)> m_fireNewBuffer;
	boost::signals2::signal<void ()> m_fireDisconnect;

	void StartHandshake(stream::ClientType type, string_view serverEP, string_view confName, string_view userName);
	bool IsHandshakeDone(void);
	void Send(vs::SharedBuffer&& frame);
	void Shutdown();

private:
	size_t OnReceive(const void* data, size_t size) override;
	bool OnError(const boost::system::error_code& ec) override;
	void OnClose() override;

	bool m_handshakeDone = false;
	bool m_closed = false;

	const std::shared_ptr<net::BufferedConnectionInterface> m_connection;
};
