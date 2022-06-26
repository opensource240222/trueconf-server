#include "ContentHandler.h"
#include "Helpers.h"
#include <vector>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <thread>
#include <algorithm>
#include <iostream>
#include "std-generic/cpplib/ThreadUtils.h"

using namespace std::chrono;
using namespace boost::asio::ip;

namespace bwt
{

	ContentHandler::ContentHandler(boost::asio::ip::tcp::socket&& sock, const Handshake& handshake, std::shared_ptr<Intermediate> callback) :
		m_sock(std::move(sock)),
		m_resolver(sock.get_io_service()),
		m_total_compensate_time_ms(0),
		m_bytes_transferred(0),
		m_handshake(handshake),
		m_callback(callback),
		m_handler_buf(m_handshake.content_size + sizeof(stream::FrameHeader))
	{
	}

	ContentHandler::ContentHandler(boost::asio::io_service &srv, const Endpoint& endpoint, const Handshake& handshake, std::shared_ptr<Intermediate> callback) :
		m_sock(srv),
		m_resolver(srv),
		m_bytes_transferred(0),
		m_endpoint(endpoint),
		m_handshake(handshake),
		m_callback(callback),
		m_handler_buf(m_handshake.content_size + sizeof(stream::FrameHeader))
	{
	}

	void ContentHandler::start_recv()
	{
		if (!m_sock.is_open())
		{
			start_connect();
		}
		else
		{
			m_callback->Result(VS_BWT_ST_START_TEST, 0, m_handshake.type);
			m_start_time = m_cur_time = steady_clock::now();
			m_handler_buf.resize(m_handshake.content_size + sizeof(stream::FrameHeader));
			m_sock.async_receive(boost::asio::buffer(m_handler_buf),
				[this, self = shared_from_this()](const boost::system::error_code& err, std::size_t bytes_transferred)
			{
				handle_read(err, bytes_transferred);
			});
		}
	}

	void ContentHandler::handle_read(const boost::system::error_code& err, std::size_t bytes_transferred)
	{
		if (err)
		{
			m_callback->Result((err.value() == boost::asio::error::eof) ? VS_BWT_ST_FINISH_TEST : VS_BWT_ST_CONNECTION_ERROR, err.message().c_str(), m_handshake.type);
		}
		else
		{
			int64_t recv_time_ms = duration_cast<milliseconds>(steady_clock::now() - m_cur_time).count();
			m_cur_time = steady_clock::now();
			m_callback->in_bytes += bytes_transferred;
			m_callback->m_in_tcs += recv_time_ms;
			m_callback->UpdateStat(m_handshake.type, recv_time_ms, duration_cast<milliseconds>(m_cur_time - m_start_time).count());
			m_sock.async_receive(boost::asio::buffer(m_handler_buf),
				[this, self = shared_from_this()](const boost::system::error_code& err, std::size_t bytes_transferred)
			{
				handle_read(err, bytes_transferred);
			});
		}
	}

	void ContentHandler::start_send()
	{
		if (!m_sock.is_open())
		{
			start_connect();
		}
		else
		{
			m_callback->Result(VS_BWT_ST_START_TEST, 0, m_handshake.type);
			m_handler_buf = create_test_frame(m_handshake);
			m_start_time = m_cur_time = steady_clock::now();
			m_sock.async_send(boost::asio::buffer(m_handler_buf),
				[this, self = shared_from_this()](const boost::system::error_code& err, std::size_t bytes_transferred)
			{
				handle_write(err, bytes_transferred);
			});
		}
	}

	void ContentHandler::handle_write(const boost::system::error_code& err, size_t bytes_transferred)
	{
		if (   (steady_clock::now() - m_start_time) > milliseconds(m_handshake.send_time_ms) ||
			err)
		{
			m_callback->Result((!err || (err.value() == boost::asio::error::eof)) ? VS_BWT_ST_FINISH_TEST : VS_BWT_ST_CONNECTION_ERROR, err.message().c_str(), m_handshake.type);
		}
		else
		{
			int64_t send_time_ms = duration_cast<milliseconds>(steady_clock::now() - m_cur_time).count();
			m_callback->out_bytes += bytes_transferred;
			m_cur_time = steady_clock::now();
			m_callback->m_out_tcs += send_time_ms;
			m_callback->UpdateStat(m_handshake.type, send_time_ms, duration_cast<milliseconds>(m_cur_time - m_start_time).count());
			m_bytes_transferred += bytes_transferred;
			if (m_bytes_transferred == m_handler_buf.size())
				m_bytes_transferred = 0;
			if (m_handshake.send_period_ms > 0 && m_bytes_transferred == 0)
			{
				if (send_time_ms >= m_handshake.send_period_ms)
				{
					m_total_compensate_time_ms += send_time_ms - m_handshake.send_period_ms;
				}
				else
				{
					int64_t compensate_time_ms = std::min(m_handshake.send_period_ms - send_time_ms, m_total_compensate_time_ms);
					vs::SleepFor(milliseconds(m_handshake.send_period_ms - send_time_ms - compensate_time_ms));
					m_total_compensate_time_ms -= compensate_time_ms;
				}
			}
			m_sock.async_send(boost::asio::buffer(m_handler_buf.data() + m_bytes_transferred, m_handler_buf.size() - m_bytes_transferred),
				[this, self = shared_from_this()](const boost::system::error_code& err, std::size_t bytes_transferred)
			{
				handle_write(err, bytes_transferred);
			});
		}

	}

	void ContentHandler::start_connect()
	{
		m_callback->Result(VS_BWT_ST_START_CONNECT, 0, m_handshake.type);
		m_callback->Result(VS_BWT_ST_CONNECT_ATTEMPT, &m_endpoint, 0);
		m_resolver.async_resolve(tcp::resolver::query(m_endpoint.host, m_endpoint.port),
			[this, self = shared_from_this()](const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator)
		{
			if (err)
			{
				m_callback->Result(VS_BWT_ST_CONNECT_ERROR, err.message().c_str(), m_handshake.type);
			}
			else
			{
				tcp::endpoint endpoint = *endpoint_iterator;
				m_sock.async_connect(endpoint,
					[this, self, endpoint_iterator](const boost::system::error_code& err)
				{
					handle_connect(err, endpoint_iterator);
				});
			}
		});
	}


	void ContentHandler::handle_connect(const boost::system::error_code& err,
		tcp::resolver::iterator endpoint_iterator)
	{
		if (!err)
		{
			m_start_time = steady_clock::now();
			m_callback->Result(VS_BWT_ST_CONNECT_OK, 0, m_handshake.type);
			m_callback->Result(VS_BWT_ST_START_HANDSHAKE, 0, m_handshake.type);
			m_handler_buf = create_handshake(m_handshake, m_endpoint);
			m_sock.async_send(boost::asio::buffer(m_handler_buf),
				[this, self = shared_from_this()](const boost::system::error_code& err, std::size_t bytes_transferred)
			{
				handle_handshake_sent(err, bytes_transferred);
			});
		}
		else if (endpoint_iterator != tcp::resolver::iterator())
		{
			m_sock.close();
			tcp::endpoint endpoint = *(endpoint_iterator++);
			m_sock.async_connect(endpoint,
				[this, endpoint_iterator, self = shared_from_this()](const boost::system::error_code& err)
			{
				handle_connect(err, endpoint_iterator);
			});
		}
		else
		{
			m_callback->Result(VS_BWT_ST_CONNECT_ERROR, err.message().c_str(), m_handshake.type);
		}
	}

	void ContentHandler::handle_handshake_sent(const boost::system::error_code& err, size_t)
	{
		if (err)
		{
			m_callback->Result(VS_BWT_ST_HANDSHAKE_ERROR, err.message().c_str(), m_handshake.type);
		}
		else
		{
			m_sock.async_receive(boost::asio::buffer(m_handler_buf),
				[this, self = shared_from_this()](const boost::system::error_code& err, std::size_t)
			{
				if (err)
				{
					m_callback->Result(VS_BWT_ST_HANDSHAKE_ERROR, err.message().c_str(), m_handshake.type);
				}
				else
				{
					int64_t response_ms = duration_cast<milliseconds>(steady_clock::now() - m_start_time).count();
					m_callback->Result(VS_BWT_ST_HANDSHAKE_OK, 0, m_handshake.type);
					if (m_handshake.type == VS_ACS_LIB_SENDER)
					{
						m_callback->out_response_ms = response_ms;
						start_send();
					}
					else
					{
						m_callback->in_response_ms = response_ms;
						start_recv();
					}
				}
			});
		}
	}

	void ContentHandler::send_reply()
	{
		m_handler_buf = create_handshake_reply(m_handshake, m_endpoint);
		m_sock.async_send(boost::asio::buffer(m_handler_buf),
			[this, self = shared_from_this()](const boost::system::error_code& err, std::size_t)
		{
			if (err)
			{
				m_callback->Result(VS_BWT_ST_HANDSHAKE_ERROR, err.message().c_str(), m_handshake.type);
			}
			else
			{
				if (m_handshake.type == VS_ACS_LIB_SENDER)
				{
					start_send();
				}
				else
				{
					start_recv();
				}
			}
		});
	}


	ContentHandler::~ContentHandler()
	{
	}

}



