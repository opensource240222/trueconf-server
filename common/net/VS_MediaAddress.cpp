#include "VS_MediaAddress.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "net/Lib.h"

std::string net::GetRTPInterface()
{
	std::string result;
	VS_RegistryKey(false, CONFIGURATION_KEY).GetString(result, RTP_INTERFACE_KEY_TAG);
	return result;
}

net::address net::GetRTPAddress(boost::asio::io_service & ios)
{
	boost::system::error_code ec;
	auto rtp_addr_str = GetRTPInterface();
	if (rtp_addr_str.empty()) return net::address();

	auto addr = net::address::from_string(rtp_addr_str, ec);
	if (addr.is_unspecified()) addr = MakeA_lookup(rtp_addr_str, ios);
	if (addr.is_unspecified()) addr = MakeA_lookup(rtp_addr_str, ios, true);	// ipv6

	return addr;
}
