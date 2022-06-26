#include "Address.h"

#include <boost/regex.hpp>

static const boost::regex REGEX_DOMAIN{ "^(?:(?:(?:[a-zA-Z]{1,2})|([0-9]{1,2})|[a-zA-Z0-9]{1,2}|[a-zA-Z0-9][a-zA-Z0-9-]{1,61}[a-zA-Z0-9])\\.)+[a-zA-Z]{2,6}$" };

//ip&255	== 10*256^3;				10.0.0.0/8
//ip&4095	== 172*256^3 + 16*256^2;	172.16.0.0/12
//ip&65535	== 192*256^3 + 168*256^2	192.168.0.0/16
static constexpr std::uint32_t SUBNET_MASK_1(0xFF000000);
static constexpr std::uint32_t SUBNET_MASK_2(0xFFF00000);
static constexpr std::uint32_t SUBNET_MASK_3(0xFFFF0000);

static constexpr std::uint32_t ADDRESS_SPACE_1(0x0A000000);
static constexpr std::uint32_t ADDRESS_SPACE_2(0xAC100000);
static constexpr std::uint32_t ADDRESS_SPACE_3(0xC0A80000);

template<typename Address>
static inline bool is_ip(string_view in, char *buff, unsigned buffLen) noexcept
{
	const auto in_len = in.length();

	if(in_len == 0 || in_len > buffLen)
	{
		return false;
	}

	::memcpy(buff, in.data(), in_len);
	buff[in_len] = '\0';

	boost::system::error_code ec;
	Address::from_string(buff, ec);

	return !ec;
}

namespace net {

bool is_domain_name(string_view in) noexcept
{
	return boost::regex_match(in.begin(), in.end(), REGEX_DOMAIN);
}

bool is_ipv4(string_view in) noexcept
{
	char tmp_buff[sizeof("255.255.255.255")];
	return is_ip<net::address_v4>(in, tmp_buff, sizeof(tmp_buff) - 1);
}

bool is_ipv6(string_view in) noexcept
{
	char tmp_buff[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff")];
	return is_ip<net::address_v6>(in, tmp_buff, sizeof(tmp_buff) - 1);
}

bool is_private_address(const net::address &addr)
{
	if (addr.is_v6())
		return addr.to_v6().is_link_local();

	const std::uint32_t ip = addr.to_v4().to_ulong();
	return	(ip & SUBNET_MASK_1) == ADDRESS_SPACE_1 ||
			(ip & SUBNET_MASK_2) == ADDRESS_SPACE_2 ||
			(ip & SUBNET_MASK_3) == ADDRESS_SPACE_3;
}

}
