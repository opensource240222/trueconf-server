#pragma once

#include "RunTest.h"
#include <boost/asio/ip/tcp.hpp>
#include <chrono>
#include "Handshake.h"
#include "../streams/Protocol.h"
#include "../acs_v2/Handler.h"

namespace bwt
{

	class ContentHandler : public vs::enable_shared_from_this<ContentHandler>
	{
	public:
		~ContentHandler();

		void send_reply();

		void start_recv();
		void start_send();

	protected:
		ContentHandler(boost::asio::io_service &srv, const Endpoint &endpoint, const Handshake &handshake, std::shared_ptr<Intermediate> callback);
		ContentHandler(boost::asio::ip::tcp::socket &&sock, const Handshake &handshake, std::shared_ptr<Intermediate> callback);

	private:

		void handle_read(const boost::system::error_code& err, std::size_t bytes_transferred);
		void handle_write(const boost::system::error_code& err, size_t bytes_transferred);
		void start_connect();
		void handle_connect(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
		void handle_handshake_sent(const boost::system::error_code& err, size_t bytes_transferred);

		boost::asio::ip::tcp::socket m_sock;
		boost::asio::ip::tcp::resolver m_resolver;
		std::chrono::steady_clock::time_point m_start_time, m_cur_time;
		int64_t m_total_compensate_time_ms;
		size_t m_bytes_transferred;
		Endpoint m_endpoint;
		Handshake m_handshake;
		std::shared_ptr<Intermediate> m_callback;
		acs::Handler::stream_buffer m_handler_buf;
	};

}
