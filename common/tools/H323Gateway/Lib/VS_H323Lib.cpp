/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 10.12.03     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_H323Lib.cpp
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#include "VS_H323Lib.h"

namespace
{
	template<class I, class T>
	inline bool get_ip_port_h225(T &obj, net::address &addr, net::port &port) noexcept
	{
		if (!obj.filled || !obj.ip.filled || !obj.port.filled) return false;
		typename I::bytes_type ip_data;
		if (!obj.ip.value.GetBits(ip_data.data(), ip_data.size() * 8)) return false;
		addr = I{ ip_data };
		port = obj.port.value;
		return true;
	}

	template<class T, class I>
	inline void set_ip_port_h225(T &obj, const I &addr, net::port port)
	{
		obj.ip.value.FreeMemory();
		auto &&bytes_ip = addr.to_bytes();
		obj.ip.value.AddBits(bytes_ip.data(), bytes_ip.size() * 8);
		obj.port.value = port;
		obj.ip.filled = obj.port.filled = obj.filled = true;
	}

	template<typename I, class T>
	inline bool get_ip_port_h245(T& obj, net::address& addr, net::port& port) noexcept
	{
		typename I::bytes_type ip_data;
		if (!obj.network.value.GetBits(ip_data.data(), ip_data.size() * 8))
			return false;
		addr = I{ ip_data };
		port = obj.tsapIdentifier.value;
		obj.network.value.ResetIndex();
		return true;
	}

	template<class T, typename I>
	inline void set_ip_port_h245(T& obj, const I& addr, net::port port)
	{
		auto&& bytes_ip = addr.to_bytes();
		obj.network.value.AddBits(bytes_ip.data(), bytes_ip.size() * 8);
		obj.network.filled = true;
		obj.tsapIdentifier.value = port;
		obj.tsapIdentifier.filled = true;
		obj.filled = true;
	}
} //anonymous namespace


void set_ip_address(VS_H225TransportAddress& obj_, const net::address& addr, net::port port)
{
	obj_.FreeChoice();
	VS_AsnSequence* h225addr;
	if (addr.is_v6())
	{
		VS_H225Ipv6Address* tmp = new VS_H225Ipv6Address();
		set_ip_port_h225(*tmp, addr.to_v6(), port);
		h225addr = tmp;
		obj_.tag = VS_H225TransportAddress::e_ip6Address;
	}
	else
	{
		VS_H225IpAddress* tmp = new VS_H225IpAddress();
		set_ip_port_h225(*tmp, addr.to_v4(), port);
		h225addr = tmp;
		obj_.tag = VS_H225TransportAddress::e_ipAddress;
	}

	obj_.choice = h225addr;
	obj_.filled = true;
}

bool get_ip_address(VS_H225TransportAddress& obj_, net::address& addr, net::port& port) noexcept
{
	// passes only ipv4 and ipv6 addresses
	bool res = false;
	if (obj_.tag == VS_H225TransportAddress::e_ipAddress)
	{
		auto&& ipAddress = static_cast<VS_H225IpAddress*>(obj_.choice);
		res = get_ip_port_h225<net::address_v4>(*ipAddress, addr, port);
	}
	else if (obj_.tag == VS_H225TransportAddress::e_ip6Address)
	{
		auto&& ipAddress = static_cast<VS_H225Ipv6Address*>(obj_.choice);
		res = get_ip_port_h225<net::address_v6>(*ipAddress, addr, port);
	}
	return res;
}

void set_ip_address(VS_H245UnicastAddress& obj_, const net::address& addr, net::port port)
{
	obj_.FreeChoice();
	if (addr.is_v6())
	{
		VS_H245UnicastAddress_IP6Address *ia = new VS_H245UnicastAddress_IP6Address;
		set_ip_port_h245(*ia, addr.to_v6(), port);
		obj_ = ia;
	}
	else
	{
		VS_H245UnicastAddress_IPAddress *ia = new VS_H245UnicastAddress_IPAddress;
		set_ip_port_h245(*ia, addr.to_v4(), port);
		obj_ = ia;
	}
}

bool get_ip_address(VS_H245UnicastAddress& obj_, net::address& addr, net::port& port) noexcept
{
	bool res = false;
	switch (obj_.tag)
	{
	case VS_H245MulticastAddress::e_iPAddress:
		{
			res = get_ip_port_h245<net::address_v4>(*static_cast<VS_H245UnicastAddress_IPAddress *>(obj_.choice), addr, port);
		}
		break;
	case VS_H245UnicastAddress::e_iP6Address:
		{
			res = get_ip_port_h245<net::address_v6>(*static_cast<VS_H245UnicastAddress_IP6Address *>(obj_.choice), addr, port);
		}
		break;
	default: break;
	}
	return res;
}

VS_GwH245OpenLogicalChannel::VS_GwH245OpenLogicalChannel(void) {}
// end of VS_GwH245OpenLogicalChannel constructor

VS_GwH245OpenLogicalChannelAck::VS_GwH245OpenLogicalChannelAck(void) {}

bool VS_GwH245OpenLogicalChannelAck::GetRtpRtcpIpPort(net::address& rtpAddr, net::port& rtpPort,
	net::address& rtcpAddr, net::port& rtcpPort)
{
	VS_H245H2250LogicalChannelAckParameters* lcap = forwardMultiplexAckParameters;
	if (!lcap) return false;
	VS_H245UnicastAddress* rtp_ua = lcap->mediaChannel;
	if (!rtp_ua) return false;
	VS_H245UnicastAddress* rtcp_ua = lcap->mediaControlChannel;
	if (!get_ip_address(*rtp_ua, rtpAddr, rtpPort)) return false;
	if (!rtcp_ua) return true;
	return get_ip_address(*rtcp_ua, rtcpAddr, rtcpPort);
}

// end of VS_GwH245OpenLogicalChannelAck constructor

bool VS_GwH245OpenLogicalChannel::GetReverseRtcpIpPort(net::address& rtcp_addr, net::port& port)
{
	VS_H245H2250LogicalChannelParameters* lcp = forwardLogicalChannelParameters.multiplexParameters;
	if (!lcp) return false;
	if (lcp->mediaControlGuaranteedDelivery.value) return false;
	VS_H245UnicastAddress* ua = lcp->mediaControlChannel;
	if (!ua) return false;
	return get_ip_address(*ua, rtcp_addr, port);
}

bool VS_GwH245OpenLogicalChannel::GetSeparateStackIpPort(net::address& addr, net::port& port)
{
	VS_H245TransportAddress* ta = separateStack.networkAddress;
	if (!ta) return false;
	VS_H245UnicastAddress* ua = *ta;
	if (!ua) return false;
	return get_ip_address(*ua, addr, port);
}

VS_GwH245CloseLogicalChannel::VS_GwH245CloseLogicalChannel(void) {}
// end of VS_GwH245CloseLogicalChannel constructor

VS_GwH245CloseLogicalChannelAck::VS_GwH245CloseLogicalChannelAck(void) {}
// end of VS_GwH245CloseLogicalChannelAck constructor

bool VS_GwH245OpenLogicalChannelAck::GetSeparateStackIpPort(net::address& addr, net::port& port)
{
	VS_H245TransportAddress* ta = separateStack.networkAddress;
	if (!ta) return false;
	VS_H245UnicastAddress* ua = *ta;
	if (!ua) return false;
	return get_ip_address(*ua, addr, port);
}

VS_GwH245MultimediaSystemControlMessage::VS_GwH245MultimediaSystemControlMessage(void) {}
// end of VS_GwH245MultimediaSystemControlMessage constructor

VS_GwH245H2250Capability::VS_GwH245H2250Capability(void) {}
// end of VS_GwH245H2250Capability constructor

VS_GwH245Capability::VS_GwH245Capability(void) {}
// end of VS_GwH245Capability constructor

VS_GwH245CapabilityTableEntry::VS_GwH245CapabilityTableEntry(void) {}
// end of VS_GwH245CapabilityTableEntry constructor

/////////////////////////////////////////////////////////////////////////////////////////
