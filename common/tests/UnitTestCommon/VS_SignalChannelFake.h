#pragma once

#include "TrueGateway/net/VS_SignalChannel.h"

class VS_SignalChannelFake : public VS_SignalChannel
{
public:
	VS_SignalChannelFake();

	bool Open(unsigned long flags, const net::address& bind_addr, net::port bind_port, const net::address& connect_addr, net::port connect_port, const net::QoSFlowSharedPtr& flow) override;
	bool Open(const net::QoSFlowSharedPtr &flow) override;
	void Close(bool wait_for_send) override;
	void Send(vs::SharedBuffer&& buffer) override;
	net::address LocalAddress() const override;
	net::port LocalPort() const override;
	net::address RemoteAddress() const override;
	net::port RemotePort() const override;

	void CallChannelOpened()
	{
		m_signal_ChannelOpened();
	}
	void CallChannelClosed()
	{
		m_signal_ChannelClosed();
	}
	void CallDataReceived(const void* data, size_t size)
	{
		m_signal_DataReceived(data, size);
	}

	bool connected;
	net::address local_addr;
	net::address remote_addr;
	net::port local_port;
	net::port remote_port;
	std::vector<vs::SharedBuffer> out_queue;
};
