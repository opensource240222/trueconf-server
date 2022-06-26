// The code below is used only in Win32 specific code.
#ifdef _WIN32

#include "VS_QoSEnabledSocket.h"
#include <algorithm>

#include <cstring>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <qos2.h>


// !!! VS_QoSEnabledSocket starts here !!!
VS_QoSEnabledSocket::VS_QoSEnabledSocket()
	: m_socket(0)
{
}

VS_QoSEnabledSocket::~VS_QoSEnabledSocket()
{
	if (m_socket)
	{
		RemoveSocketFromFlow();
	}
}

void VS_QoSEnabledSocket::SetQoSFlow(const net::QoSFlowSharedPtr & flow)
{
	if (m_socket)
		return;

	m_flow = flow;

	if (IsConnected())
	{
		if (!AddSocketToFlow(nullptr)) // There is no need to pass address information for an already established TCP network connection.
		{
			m_flow = nullptr;
		}
	}
}

net::QoSFlowSharedPtr VS_QoSEnabledSocket::GetQoSFlow(void)
{
	return m_flow;
}

bool VS_QoSEnabledSocket::AddSocketToFlow(const void *dst_sockaddr)
{
	if (m_flow == nullptr || m_socket)
		return false;

	auto socket = GetSocketHandle();
	auto res = m_flow->AddSocket(socket, dst_sockaddr);

	if (res)
	{
		m_socket = socket;
	}

	return res;
}

bool VS_QoSEnabledSocket::RemoveSocketFromFlow(void)
{
	if (m_socket ==  0 || m_flow == nullptr)
	{
		return false;
	}

	auto res = m_flow->RemoveSocket(m_socket);
	if (res)
	{
		m_socket = 0;
	}

	return res;
}

bool VS_QoSEnabledSocket::AddExtraFlow(const net::QoSFlowSharedPtr &flow, const VS_IPPortAddress &addr)
{
	auto found = std::find_if(m_extra_flows.begin(), m_extra_flows.end(), [&flow](net::QoSFlowSharedPtr &e) { return e == flow; });
	if (found != m_extra_flows.end())
	{
		return true;
	}

	{
		void *sockaddr = nullptr;
		sockaddr_in sin;
		sockaddr_in6 sin6;

		if (addr.getAddressType() == VS_IPPortAddress::ADDR_IPV4)
		{
			memset(&sin, 0, sizeof(sin));
			sin.sin_family = AF_INET;
			sin.sin_addr.S_un.S_addr = addr.ipv4_netorder();
			sin.sin_port = addr.port_netorder();

			sockaddr = &sin;
		}
		else if (addr.getAddressType() == VS_IPPortAddress::ADDR_IPV6)
		{
			memset(&sin6, 0, sizeof(sin6));
			sin6.sin6_family = AF_INET6;
			sin6.sin6_addr = addr.ipv6();
			sin6.sin6_port = addr.port_netorder();

			sockaddr = &sin6;
		}

		if (flow->AddSocket(GetSocketHandle(), sockaddr))
		{
			m_extra_flows.push_back(flow);
			return true;
		}

	}

	return false;
}

bool VS_QoSEnabledSocket::RemoveExtraFlow(const net::QoSFlowSharedPtr &flow)
{
	if (m_extra_flows.empty())
	{
		return false;
	}

	auto found = std::find_if(m_extra_flows.begin(), m_extra_flows.end(), [&flow](net::QoSFlowSharedPtr &e) { return e == flow; });
	if (found == m_extra_flows.end())
	{
		return false;
	}

	if (flow->RemoveSocket(GetSocketHandle()))
	{
		m_extra_flows.erase(found);
		return true;
	}

	return false;
}

void VS_QoSEnabledSocket::ClearExtraFlows(void)
{
	auto socket = GetSocketHandle();
	for (auto &v : m_extra_flows)
	{
		v->RemoveSocket(socket);
	}

	m_extra_flows.clear();
}

#endif // _WIN32
