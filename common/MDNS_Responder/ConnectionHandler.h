#pragma once
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <functional>
#include <map>
#include <mutex>

#include "std-generic/compat/memory.h"

namespace mdns
{

class ConnectionHandler: public vs::enable_shared_from_this<ConnectionHandler>
{
public:

	typedef std::function<void(const boost::system::error_code& error,
			size_t size, uint16_t socket)> SendCallback;
	typedef std::function<void(const boost::system::error_code& error,
			const void* buffer, size_t size, uint16_t socket)> RecvCallback;

	bool init();
	void shutdown();
	void cancel();
	void sendMsg(const boost::asio::mutable_buffers_1& buffer,
		const SendCallback& sendHandler,
		int index = -1);
	void recvMsg(const RecvCallback& recvHandler,
		int index = -1);
private:
	static const uint32_t DEFAULT_BUFFER_SIZE = 4096;

	struct Connection
	{
		boost::asio::ip::udp::socket socket;
//		Endpoint must live longer than connectionhandler itself
		const boost::asio::ip::udp::endpoint& multicastEndpoint;
		std::array<uint8_t, DEFAULT_BUFFER_SIZE> recvBuffer;

		Connection(boost::asio::ip::udp::socket&& socket_,
			const boost::asio::ip::udp::endpoint& multicastEndpoint_)
			: socket(std::move(socket_))
			, multicastEndpoint(multicastEndpoint_)
			{}
	};

	boost::asio::ip::udp::endpoint senderEndpoint_;

	std::map<uint32_t /*index*/, std::shared_ptr<Connection>> connections_;

	uint32_t uniqueIndex_;

	boost::asio::io_service& service_;
	boost::asio::io_service::strand& strand_;

	void try_v4(const boost::asio::ip::address_v4& address);
	void try_v6(unsigned int interfaceIndex);

protected:
	explicit ConnectionHandler(boost::asio::io_service &service, boost::asio::io_service::strand &strand)
		: uniqueIndex_(0)
		, service_(service)
		, strand_(strand)
	{}

};

}
