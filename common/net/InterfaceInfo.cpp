#include "InterfaceInfo.h"
#include "std-generic/cpplib/atomic_shared_ptr.h"
#include "std-generic/cpplib/scope_exit.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <utility>

#if defined(_WIN32)
#	include <IPHlpApi.h>
#	pragma comment(lib, "IPHlpApi.lib")
#elif defined(__linux__)
#	include <sys/types.h>
#	include <ifaddrs.h>
#	include <net/if.h>
#endif

namespace net {

template <class Array>
Array as_array(const typename Array::value_type* data)
{
	Array result;
	std::memcpy(result.data(), data, result.size());
	return result;
}

void LoadInterfaceInfo(std::vector<interface_info>& iis);
void AddLocalAddress(interface_info& ii, sockaddr* sa);

interface_info_list GetInterfaceInfo(bool update)
{
	static vs::atomic_shared_ptr<std::vector<interface_info>> current;
	auto result = current.load();
	if (update || !result)
	{
		result.reset(new std::vector<interface_info>);
		LoadInterfaceInfo(*result);
		current.store(result);
	}
	return result;
}

void AddLocalAddress(interface_info& ii, sockaddr* sa)
{
	if (!sa)
		return;
	if (sa->sa_family == AF_INET)
	{
		auto sa_in = reinterpret_cast< ::sockaddr_in*>(sa);
		ii.addr_local_v4.emplace_back(as_array<address_v4::bytes_type>(
			reinterpret_cast<address_v4::bytes_type::value_type*>(&sa_in->sin_addr.s_addr)
		));
		if (ii.addr_local_v4.back().is_loopback() || ii.addr_local_v4.back().is_unspecified())
			ii.addr_local_v4.pop_back();
	}
	else if (sa->sa_family == AF_INET6)
	{
		auto sa_in6 = reinterpret_cast< ::sockaddr_in6*>(sa);
		ii.addr_local_v6.emplace_back(as_array<address_v6::bytes_type>(
			reinterpret_cast<address_v6::bytes_type::value_type*>(sa_in6->sin6_addr.s6_addr)));
		if (ii.addr_local_v6.back().is_link_local())
			ii.addr_local_v6.back().scope_id(ii.index);
		if (ii.addr_local_v6.back().is_loopback() || ii.addr_local_v6.back().is_unspecified())
			ii.addr_local_v6.pop_back();
	}
}

#if defined(_WIN32)

void LoadInterfaceInfo(std::vector<interface_info>& iis)
{
	assert(iis.empty());

	::ULONG buffer_size = 16 * 1024;
	std::unique_ptr<char[]> buffer;
	while (true)
	{
		buffer.reset(new char[buffer_size]);
		auto old_size = buffer_size;
		auto ret = ::GetAdaptersAddresses(
			AF_UNSPEC,
			GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME,
			NULL,
			reinterpret_cast< ::IP_ADAPTER_ADDRESSES*>(buffer.get()),
			&buffer_size
		);
		if (ret == ERROR_SUCCESS)
			break;
		else if (ret == ERROR_BUFFER_OVERFLOW && buffer_size > old_size)
			continue;
		else
			return;
	}

	auto adapters = reinterpret_cast< ::IP_ADAPTER_ADDRESSES*>(buffer.get());
	for (auto adapter = adapters; adapter != nullptr; adapter = adapter->Next)
	{
		iis.emplace_back();
		auto& ii = iis.back();
		ii.name = adapter->AdapterName;
		ii.index = adapter->Ipv6IfIndex;
		if(adapter->OperStatus != ::IF_OPER_STATUS::IfOperStatusUp)
			continue; //Ignore disabled adaptors
		for (auto uaddr = adapter->FirstUnicastAddress; uaddr != nullptr; uaddr = uaddr->Next)
			AddLocalAddress(ii, uaddr->Address.lpSockaddr);
	}
}

#elif defined(__linux__)
// Linux implementation will need some changed later:
// to get interface index, different ioctl system call must be used
void LoadInterfaceInfo(std::vector<interface_info>& iis)
{
	assert(iis.empty());

	::ifaddrs* ifas = nullptr;
	VS_SCOPE_EXIT { if (ifas) ::freeifaddrs(ifas); };
	if (::getifaddrs(&ifas) == -1)
		return;

	for (auto ifa = ifas; ifa != nullptr; ifa = ifa->ifa_next)
	{
		auto ii_it = std::find_if(iis.begin(), iis.end(), [&](const interface_info& x) { return x.name == ifa->ifa_name; });
		if (ii_it == iis.end())
		{
			ii_it = iis.emplace(iis.end());
			ii_it->name = ifa->ifa_name;
			ii_it->index = if_nametoindex(ifa->ifa_name);
		}
		if (!(ifa->ifa_flags & IFF_UP))
			continue; // Ignore disabled adaptors
		AddLocalAddress(*ii_it, ifa->ifa_addr);
	}
}

#endif

}
