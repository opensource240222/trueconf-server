#pragma once

#include "acs_v2/ISetConnection.h"
#include "std-generic/compat/memory.h"
#include "std-generic/cpplib/synchronized.h"
#include "std/cpplib/fast_mutex.h"
#include "TransceiverLib/NetChannelInterface.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/shared_ptr.hpp>

#include <memory>
#include <queue>
#include <functional>

class VS_MainRelayMessage;
class VS_AuthConnectionInterface;

/* transceiver */
namespace ts {
	using MessageQueue = vs::Synchronized<std::queue<boost::shared_ptr<VS_NetworkRelayMessageBase>>,vs::fast_mutex>;

	template<class Socket = boost::asio::ip::tcp::socket>
	class NetChannel : public NetChannelInterface,
					   public vs::enable_shared_from_this<NetChannel<Socket>>,
					   public net::ISetConnection<Socket>
	{
		Socket					m_sock;
		boost::shared_ptr<VS_MainRelayMessage>			m_rcvMess;
		MessageQueue									m_out_mess_queue;
		size_t                                          m_out_mess_offset;
		std::weak_ptr<VS_AuthConnectionInterface>		m_auth_conn;
		std::function<void()>							m_fireOnConnDie;

		void handle_read(const boost::system::error_code& error, std::size_t bytes_received);
		void handle_write(const boost::system::error_code& error, std::size_t bytes_written);
	public:
		NetChannel(const NetChannel&) = delete;
		NetChannel& operator=(const NetChannel&) = delete;

		template<class Callback>
		void SetOnConnDie(Callback &&call_back) {
			m_fireOnConnDie = std::forward<Callback>(call_back);
		}

		bool SetChannelConnection(Socket&& sock);
		void RequestRead();
		bool SendMsg(const boost::shared_ptr<VS_NetworkRelayMessageBase>& mess) override;
		net::address GetRemoteAddress() const override;
		bool SetTCPConnection(Socket&& socket, acs::Handler::stream_buffer&& buffer) override;
		void StopActivity() override;

	protected:
		NetChannel(boost::asio::io_service &io, const std::shared_ptr<VS_AuthConnectionInterface> &auth_conn, const std::function<void(const std::string &)> &cb);
		NetChannel(boost::asio::io_service &io);
	};
}