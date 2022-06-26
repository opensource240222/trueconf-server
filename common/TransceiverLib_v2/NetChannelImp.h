#pragma once

#include "NetChannel.h"
#include "TransceiverLib/VS_MainRelayMessage.h"
#include "TransceiverLib/VS_AuthConnectionInterface.h"
#include "net/Handshake.h"
#include "std-generic/cpplib/ignore.h"

#include <boost/make_shared.hpp>

template<class Socket>
void ts::NetChannel<Socket>::handle_read(const boost::system::error_code& error, std::size_t bytes_received)
{
	if (error) {
		m_sock.close(vs::ignore<boost::system::error_code>());
		if (m_fireOnConnDie)
			m_fireOnConnDie();
		return;
	}

	m_rcvMess->SetReadBytes(bytes_received);
	m_fireProcessRecvMessage(m_rcvMess);

	unsigned long read_sz(0);
	unsigned char *buf = m_rcvMess->GetBufToRead(read_sz);

	auto&& readHandler = [w_this = this->weak_from_this()]
		(const boost::system::error_code error, std::size_t bytes_received) {
			if (auto self = w_this.lock()) {
				self->handle_read(error, bytes_received);
		}
	};

	m_sock.async_receive(boost::asio::buffer(buf, read_sz),std::move(readHandler));
}

template<class Socket>
void ts::NetChannel<Socket>::handle_write(const boost::system::error_code & error, std::size_t bytes_written)
{
	if (error) {
		m_sock.close(vs::ignore<boost::system::error_code>());
		if (m_fireOnConnDie)
			m_fireOnConnDie();
		return;
	}

	{
	auto pOutQueue = m_out_mess_queue.lock();
	assert(!pOutQueue->empty());
	unsigned long messageSize(0);
	const unsigned char* buf = pOutQueue->front()->GetMess(messageSize);
	assert(bytes_written <= messageSize);
	if (bytes_written + m_out_mess_offset == messageSize)
	{
		pOutQueue->pop();
		m_out_mess_offset = 0;
		if (pOutQueue->empty())
			return;
		buf = pOutQueue->front()->GetMess(messageSize);
	} else
	{
//		Sometimes tcp::socket is unable to send everything we requested,
//		so we need to offset our buffer and resend it
		m_out_mess_offset += bytes_written;
		buf += bytes_written;
		messageSize -= bytes_written;
	}
	auto&& writeHandler = [w_this = this->weak_from_this()]
		(const boost::system::error_code error, std::size_t bytes_written)
		{
			if (auto self = w_this.lock())
				self->handle_write(error, bytes_written);
		};
	m_sock.async_send(boost::asio::buffer(buf, messageSize), std::move(writeHandler));
	}
}

template<class Socket>
inline ts::NetChannel<Socket>::NetChannel(boost::asio::io_service & io)
	: m_sock(io)
	, m_out_mess_offset(0)
{
}

template<class Socket>
ts::NetChannel<Socket>::NetChannel(boost::asio::io_service & io, const std::shared_ptr<VS_AuthConnectionInterface> &auth_conn, const std::function<void(const std::string&)>& cb)
	: m_sock(io)
	, m_out_mess_offset(0)
	, m_auth_conn(auth_conn)
	{
		SetOnTransceiverReady(cb);
	}

template<class Socket>
bool ts::NetChannel<Socket>::SetChannelConnection(Socket && sock)
{
	if (!sock.is_open() || !m_fireProcessRecvMessage) return false;

	m_sock = std::move(sock);
	m_rcvMess = boost::make_shared<VS_MainRelayMessage>();
	return true;
}

template<class Socket>
void ts::NetChannel<Socket>::RequestRead()
{

	unsigned long read_sz(0);
	unsigned char *rcv_buf = m_rcvMess->GetBufToRead(read_sz);

	auto&& readHandler = [w_this = this->weak_from_this()]
		(const boost::system::error_code error, std::size_t bytes_received) {
			if (auto self = w_this.lock()) {
				self->handle_read(error, bytes_received);
		}
	};
	m_sock.async_receive(boost::asio::buffer(rcv_buf, read_sz),std::move(readHandler));
}

template<class Socket>
bool ts::NetChannel<Socket>::SendMsg(const boost::shared_ptr<VS_NetworkRelayMessageBase>& mess)
{
	if (!mess || mess->Empty())
		return true;
	if (!m_sock.is_open())
		return false;

	bool write_in_progress(false);
	{auto pOutQueue = m_out_mess_queue.lock();
	write_in_progress = !pOutQueue->empty();
	pOutQueue->push(mess);
	} // end of lock
	if (!write_in_progress)
	{
		unsigned long sz(0);
		const unsigned char *buf = mess->GetMess(sz);

		auto&& writeHandler = [w_this = this->weak_from_this()]
			(boost::system::error_code error, std::size_t bytes_written) {
				if (auto self = w_this.lock()) {
					self->handle_write(error, bytes_written);
			}
		};
		m_sock.async_send(boost::asio::buffer(buf,sz),std::move(writeHandler));
	}

	return true;
}

template<class Socket>
net::address ts::NetChannel<Socket>::GetRemoteAddress() const
{
	return m_sock.remote_endpoint(vs::ignore<boost::system::error_code>()).address();
}

template<class Socket>
bool ts::NetChannel<Socket>::SetTCPConnection(Socket && socket, acs::Handler::stream_buffer && buffer)
{
	auto auth = m_auth_conn.lock();
	if (!auth || !auth->AuthConnection(buffer.data() + sizeof(net::HandshakeHeader), buffer.size() - sizeof(net::HandshakeHeader)))
		return false;

	auto res = SetChannelConnection(std::move(socket));
	RequestRead();
	if (res && m_fireTransceiverReady && m_fireSetTransName) {
		const auto& transName = auth->GetAuthenticatedName();
		m_fireSetTransName(transName);
		m_fireTransceiverReady(transName);
	}
	return res;
}

template<class Socket>
void ts::NetChannel<Socket>::StopActivity()
{
	m_sock.close(vs::ignore<boost::system::error_code>());
	if (m_fireOnConnDie)
		m_fireOnConnDie();
}
