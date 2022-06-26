#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <unordered_map>
#include <queue>
#include <memory>

#include "acs_v2/ISetConnection.h"
#include "streams/Relay/VS_TransmitFrameInterface.h"
#include "std-generic/cpplib/synchronized.h"
#include "std/cpplib/fast_mutex.h"

class VS_AuthConnectionInterface;
class VS_NetworkRelayMessageBase;

/* transceiver */
namespace ts {
	class FrameTransmitter: public net::ISetConnection<>,
							public VS_TransmitFrameInterface,
							public vs::enable_shared_from_this<FrameTransmitter>
	{
	public:
		virtual ~FrameTransmitter();
		FrameTransmitter(const FrameTransmitter&) = delete;
		FrameTransmitter& operator=(const FrameTransmitter&) = delete;

		bool SetTCPConnection(boost::asio::ip::tcp::socket&& socket, acs::Handler::stream_buffer&& buffer) override;
		void TransmitFrame(const char *conf_name, const char *part, const stream::FrameHeader *frame_head, const void *frame_data) override;

	protected:
		explicit FrameTransmitter(const std::shared_ptr<VS_AuthConnectionInterface> &auth);

	private:
		struct ConnectionInfo
		{
			ConnectionInfo(boost::asio::ip::tcp::socket && s, const std::string &conf_name_) : socket(std::move(s)), conf_name(conf_name_) {}

			boost::asio::ip::tcp::socket								socket;
			std::string													conf_name;
			std::queue<std::shared_ptr<VS_NetworkRelayMessageBase>>		writeMessQueue;
			uint8_t														rcv_byte = 0;
		};

		void handle_read(const std::string& confName, const boost::system::error_code& error, std::size_t bytes_received);
		void handle_error(const std::string& confName, const boost::system::error_code& error);
		void handle_write(const std::string& confName, const boost::system::error_code& error);
		void SendMsg(ConnectionInfo& i, const std::shared_ptr<VS_NetworkRelayMessageBase> &m);

		std::weak_ptr<VS_AuthConnectionInterface>												m_auth_conn;
		vs::Synchronized<std::unordered_map<std::string, ConnectionInfo>, vs::fast_mutex>		m_conns_info;
	};
}