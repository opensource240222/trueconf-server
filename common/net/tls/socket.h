#pragma once

#include <memory>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/optional.hpp>

namespace net
{
struct tls
{
typedef boost::asio::ip::tcp::endpoint endpoint;
typedef boost::asio::ip::tcp::resolver resolver;
// This class imitates boost::asio::ip::tcp::socket interface and is designed to support TLS
class socket
{
public:

	socket(boost::asio::io_service& service);
	socket(socket&& other);
	socket& operator=(socket&& other);

	boost::asio::io_service& get_io_service();
	boost::asio::ip::tcp::socket::native_handle_type native_handle();
	void close(boost::system::error_code& ec);
	bool is_open() const;
	net::tls::endpoint local_endpoint(boost::system::error_code& ec) const;
	net::tls::endpoint remote_endpoint(boost::system::error_code& ec) const;
	template<typename ConnectHandler>
	void async_connect(const net::tls::endpoint& endpoint, ConnectHandler&& handler);
	template<typename ConstBufferSequence, typename WriteHandler>
	void async_send(const ConstBufferSequence& buffers, WriteHandler&& handler);
	template<typename MutableBufferSequence, typename ReadHandler>
	void async_receive(const MutableBufferSequence& buffers, ReadHandler&& handler);
private:
	boost::optional<boost::asio::ssl::context> m_ctx;
	std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> m_sslStream;

	bool AttemptLoadCerts();
};
};// struct tls

template<typename ConnectHandler>
void tls::socket::async_connect(const net::tls::endpoint& endpoint, ConnectHandler&& handler)
{
	m_sslStream->lowest_layer().async_connect(
		endpoint,
		[this, handler = std::forward<ConnectHandler>(handler)](const boost::system::error_code& error) mutable
		{
			if (error)
				handler(error);
			else
			{
				m_sslStream->lowest_layer().set_option(boost::asio::ip::tcp::no_delay(true));
				m_sslStream->async_handshake(
					boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::handshake_type::client,
					std::move(handler));
			}
		}
	);
}

template<typename ConstBufferSequence, typename WriteHandler>
void tls::socket::async_send(const ConstBufferSequence& buffers, WriteHandler&& handler)
{
	m_sslStream->async_write_some(buffers, handler);
}

template<typename MutableBufferSequence, typename ReadHandler>
void tls::socket::async_receive(const MutableBufferSequence& buffers, ReadHandler&& handler)
{
//	Boost 1.62.0 is bugged, and if you do async_recv before async_send, even in the
//	same thread, you may receive remaining handshake message just when you reach async_send() function.
//	As a result, they both start using the same SSL context, and the message to be sent is lost.
//	It looks like it's safe to use once the handshake is fully finished.
	m_sslStream->async_read_some(buffers, handler);
}

}// namespace net
