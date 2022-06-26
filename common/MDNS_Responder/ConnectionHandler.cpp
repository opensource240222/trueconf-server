#include "ConnectionHandler.h"

#include <boost/asio/ip/multicast.hpp>

#include "net/InterfaceInfo.h"

#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

namespace mdns
{

namespace
{
// As inconsistent as it gets
const char MULTICAST_ADDRESS_V4[]="224.0.0.251";
const char MULTICAST_ADDRESS_V6[]="ff02::fb";
const unsigned int MULTICAST_PORT_NUMBER = 5353;

const char RESPONDER_NOTE[]="MDNS responder: note: ";
const char RESPONDER_FATAL_ERROR[]="MDNS responder: fatal error: ";

const boost::asio::ip::udp::endpoint multicastEndpoint(boost::asio::ip::address::from_string(
		MULTICAST_ADDRESS_V4), MULTICAST_PORT_NUMBER);
const boost::asio::ip::udp::endpoint multicastEndpointV6(boost::asio::ip::address::from_string(
		MULTICAST_ADDRESS_V6), MULTICAST_PORT_NUMBER);
}

void ConnectionHandler::try_v4(const boost::asio::ip::address_v4& address)
{
	try {
	boost::asio::ip::address multicastAddress =
		boost::asio::ip::address::from_string(MULTICAST_ADDRESS_V4);

	boost::asio::ip::udp::endpoint listenEndpoint(
		boost::asio::ip::address::from_string("0.0.0.0"),
		MULTICAST_PORT_NUMBER);

	boost::asio::ip::udp::endpoint listenInterfaceEndpoint(
		address, MULTICAST_PORT_NUMBER);
	boost::asio::ip::udp::socket newSocket(service_,
		boost::asio::ip::udp::v4());
	newSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	newSocket.set_option(boost::asio::ip::multicast::outbound_interface(
			listenInterfaceEndpoint.address().to_v4()));
	boost::system::error_code error;
	newSocket.bind(listenEndpoint, error);
	if (error)
	{
		dstream4 << RESPONDER_NOTE << "failed to setup socket on interface" << address.to_string()
			<< '\n';
		return;
	}
	newSocket.set_option(boost::asio::ip::multicast::join_group(
			multicastAddress.to_v4(),
			listenInterfaceEndpoint.address().to_v4()));

	connections_.emplace(uniqueIndex_++,
		std::make_shared<Connection>(std::move(newSocket), multicastEndpoint));

	}
	catch (...) // Why does boost throw exceptions in those cases anyway?
	{
		dstream4 << RESPONDER_NOTE << "failed to setup socket on interface" << address.to_string()
			<< '\n';
	}
}

void ConnectionHandler::try_v6(unsigned int interfaceIndex)
{
	if (!interfaceIndex)
		return;
	try {
	boost::asio::ip::address multicastAddress =
		boost::asio::ip::address::from_string(MULTICAST_ADDRESS_V6);

	boost::asio::ip::udp::endpoint listenEndpoint(
		boost::asio::ip::address::from_string("0::0"),
		MULTICAST_PORT_NUMBER);
	boost::asio::ip::udp::socket newSocket(service_,
		boost::asio::ip::udp::v6());
	newSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	newSocket.set_option(boost::asio::ip::multicast::outbound_interface(interfaceIndex));
	boost::system::error_code error;
	newSocket.bind(listenEndpoint, error);
	if (error)
	{
		dstream4 << RESPONDER_NOTE << "failed to setup socket on interface" << interfaceIndex
			<< '\n';
		return;
	}
	newSocket.set_option(boost::asio::ip::multicast::join_group(
			multicastAddress.to_v6(),
			interfaceIndex));

	connections_.emplace(uniqueIndex_++,
		std::make_shared<Connection>(std::move(newSocket), multicastEndpointV6));
	}
	catch (...)
	{
		dstream4 << RESPONDER_NOTE << "failed to setup socket on interface" << interfaceIndex
			<< '\n';
	}
}

void ConnectionHandler::sendMsg(const boost::asio::mutable_buffers_1& buffer,
	const SendCallback& sendHandler,
	int index)
{
	if (index >= 0)
	{
		auto result = connections_.find(static_cast<uint32_t>(index));
		if (result == connections_.end())
			return;
		auto connection = result->second;
		connection->socket.async_send_to(buffer, connection->multicastEndpoint,
			strand_.wrap(
			[myself = weak_from_this(), sendHandler, index]
			(const boost::system::error_code& error, size_t count)
			{
//				Do I still exist? (if not, my owner is probably dead as well)
				if (auto shared_myself = myself.lock())
				{
//					Does my connection still exist?
					if (shared_myself->connections_.find(index) != shared_myself->connections_.end())
						sendHandler(error, count, index);
				}
			}
			)
		);
	} else
	{
//		Yay, iterating through map!
		for (auto& iter: connections_)
		{
			auto connection = iter.second;
			uint32_t index = iter.first;
			connection->socket.async_send_to(buffer, connection->multicastEndpoint,
				strand_.wrap(
				[myself = weak_from_this(), sendHandler, index]
				(const boost::system::error_code& error, size_t count)
				{
//					Do I still exist? (if not, my owner is probably dead as well)
					if (auto shared_myself = myself.lock())
					{
//						Does my connection still exist?
						if (shared_myself->connections_.find(index) != shared_myself->connections_.end())
							sendHandler(error, count, index);
					}
				}
				)
			);
		}
	}
}

void ConnectionHandler::recvMsg(const RecvCallback& recvHandler,
	int index)
{
	if (index >= 0)
	{
		auto result = connections_.find(static_cast<uint32_t>(index));
		if (result == connections_.end())
			return;
		auto connection = result->second;
		connection->socket.async_receive_from(
			boost::asio::buffer(connection->recvBuffer),
			senderEndpoint_,
			strand_.wrap(
			[myself = weak_from_this(), recvHandler, index]
			(const boost::system::error_code& error, size_t count)
			{
//				Do I still exist? (if not, my owner is probably dead as well)
				if (auto shared_myself = myself.lock())
				{
					auto iter = shared_myself->connections_.find(index);
//					Does my connection still exist?
					if (iter != shared_myself->connections_.end())
						recvHandler(error, iter->second->recvBuffer.data(), count, index);
				}
			}
			)
		);
	} else
	{
//		Yay, iterating through map!
		for (auto& iter: connections_)
		{
			auto connection = iter.second;
			uint32_t index = iter.first;
			connection->socket.async_receive_from(
				boost::asio::buffer(connection->recvBuffer),
				senderEndpoint_,
				strand_.wrap(
				[myself = weak_from_this(), recvHandler, index]
				(const boost::system::error_code& error, size_t count)
				{
//					Do I still exist? (if not, my owner is probably dead as well)
					if (auto shared_myself = myself.lock())
					{
						auto iter = shared_myself->connections_.find(index);
//						Does my connection still exist?
						if (iter != shared_myself->connections_.end())
							recvHandler(error, iter->second->recvBuffer.data(), count, index);
					}
				}
				)
			);
		}
	}
}

bool ConnectionHandler::init()
{
	net::interface_info_list interfaces = net::GetInterfaceInfo(true);
	for (const auto& interface: *interfaces)
	{
		for (const auto& addr_v4: interface.addr_local_v4)
			try_v4(addr_v4);
		if (!(interface.addr_local_v6.empty()))
			try_v6(interface.index);
	}
	if (!connections_.empty())
		return true;
	dstream2 << RESPONDER_FATAL_ERROR << "couldn't find any working multicast (mDNS) interface!\n";
	return false;
}

void ConnectionHandler::shutdown()
{
	for (auto& iter: connections_)
		iter.second->socket.close();
	connections_.clear();
}

void ConnectionHandler::cancel()
{
	for (auto& iter: connections_)
		iter.second->socket.cancel(); // It's deprecated in Boost 1.62.0, but there is no replacement?
}

}
