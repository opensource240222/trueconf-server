#include "VS_SignalChannelFake.h"

#include <cassert>

VS_SignalChannelFake::VS_SignalChannelFake()
	: connected(false)
	, local_port(0)
	, remote_port(0)
{
}

bool VS_SignalChannelFake::Open(unsigned long flags, const net::address& bind_addr, net::port bind_port, const net::address& connect_addr, net::port connect_port, const net::QoSFlowSharedPtr& /*flow*/)
{
	if (flags & (CONNECT_TCP | CONNECT_UDP) && !connect_addr.is_unspecified() && connect_port != 0)
	{
		if (!bind_addr.is_unspecified())
			local_addr = bind_addr;
		if (local_addr.is_unspecified())
			local_addr = net::address::from_string("10.11.12.13");
		if (bind_port != 0)
			local_port = bind_port;
		if (local_port == 0)
			local_port = 4242;
		remote_addr = connect_addr;
		remote_port = connect_port;
		assert(!local_addr.is_unspecified());
		assert(local_port != 0);
		assert(!remote_addr.is_unspecified());
		assert(remote_port != 0);
		return connected = true;
	}
	else if (flags & (LISTEN_TCP | LISTEN_UDP) && bind_port != 0 && !remote_addr.is_unspecified() && remote_port != 0)
	{
		if (!bind_addr.is_unspecified())
			local_addr = bind_addr;
		if (local_addr.is_unspecified())
			local_addr = net::address::from_string("10.11.12.13");
		local_port = bind_port;
		assert(!local_addr.is_unspecified());
		assert(local_port != 0);
		assert(!remote_addr.is_unspecified());
		assert(remote_port != 0);
		return connected = true;
	}
	return connected = false;
}

bool VS_SignalChannelFake::Open(const net::QoSFlowSharedPtr &flow)
{
	return connected;
}

void VS_SignalChannelFake::Close(bool wait_for_send)
{
	connected = false;
}

void VS_SignalChannelFake::Send(vs::SharedBuffer&& buffer)
{
	out_queue.push_back(std::move(buffer));
}

net::address VS_SignalChannelFake::LocalAddress()  const { return connected ? local_addr  : net::address{}; }
net::port    VS_SignalChannelFake::LocalPort()     const { return connected ? local_port  : 0; }
net::address VS_SignalChannelFake::RemoteAddress() const { return connected ? remote_addr : net::address{}; }
net::port    VS_SignalChannelFake::RemotePort()    const { return connected ? remote_port : 0; }