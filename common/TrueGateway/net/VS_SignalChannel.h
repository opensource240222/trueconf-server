#pragma once

#include "net/Address.h"
#include "net/Port.h"
#include "net/QoS.h"
#include "std-generic/cpplib/SharedBuffer.h"

#include <boost/asio/io_service.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/signals2/connection.hpp>

#include <functional>
#include <memory>

namespace net {
	class LoggerInterface;
} //namespace net

class VS_SignalChannel
{
public:
	static std::shared_ptr<VS_SignalChannel> Create(boost::asio::io_service& ios, const std::shared_ptr<net::LoggerInterface> &logger);
	template <class Factory>
	static void SetFactory(Factory&& factory)
	{
		s_factory = std::forward<Factory>(factory);
	}
private:
	static std::function<std::shared_ptr<VS_SignalChannel>(boost::asio::io_service& ios)> s_factory;

public:
	enum Flags : std::uint32_t
	{
		LISTEN_TCP  = 0x00000001,
		LISTEN_UDP  = 0x00000002,
		CONNECT_TCP = 0x00000004,
		CONNECT_UDP = 0x00000008,
		PROBE_UDP   = 0x00000010,
	};

	virtual ~VS_SignalChannel() {}

	typedef boost::signals2::signal<void()> ChannelOpenedSignalType;
	boost::signals2::connection ConnectToChannelOpened(const ChannelOpenedSignalType::slot_type& slot)
	{
		return m_signal_ChannelOpened.connect(slot);
	}

	typedef boost::signals2::signal<void()> ChannelClosedSignalType;
	boost::signals2::connection ConnectToChannelClosed(const ChannelClosedSignalType::slot_type& slot)
	{
		return m_signal_ChannelClosed.connect(slot);
	}

	typedef boost::signals2::signal<void(const void* data, size_t size)> DataReceivedSignalType;
	boost::signals2::connection ConnectToDataReceived(const DataReceivedSignalType::slot_type& slot)
	{
		return m_signal_DataReceived.connect(slot);
	}

	virtual bool Open(unsigned long flags, const net::address& bind_addr, net::port bind_port, const net::address& connect_addr, net::port connect_port, const net::QoSFlowSharedPtr& flow = nullptr) = 0;
	virtual bool Open(const net::QoSFlowSharedPtr &flow = nullptr) = 0;
	virtual void Close(bool wait_for_send = false) = 0;
	virtual void Send(vs::SharedBuffer&& buffer) = 0;
	virtual net::address LocalAddress() const = 0;
	virtual net::port LocalPort() const = 0;
	virtual net::address RemoteAddress() const = 0;
	virtual net::port RemotePort() const = 0;

protected:
	ChannelOpenedSignalType m_signal_ChannelOpened;
	ChannelClosedSignalType m_signal_ChannelClosed;
	DataReceivedSignalType m_signal_DataReceived;
};
