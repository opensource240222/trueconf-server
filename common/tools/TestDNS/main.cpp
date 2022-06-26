#include <iostream>

#include "net/DNSUtils/VS_DNS.h"
#include "net/Address.h"

#include "std/cpplib/event.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"

#include "std-generic/cpplib/ThreadUtils.h"

#include "net/DNSUtils/VS_DNSTools.h"

int main(/*int argc, char **argv*/)
{
	if(!VS_RegistryKey::InitDefaultBackend("memory:force_lm=false"))
		throw std::runtime_error("Can't initialize registry backend!");

	{
		VS_RegistryKey key{ false, CONFIGURATION_KEY, false, true };
		const char servers[] = "1.2.3.4,1.2.3.5,1.2.3.6,1.2.3.7,8.8.8.8";

		bool res = key.SetString(servers, "DNS Servers");
		assert(res);

		int value = 1000; //1sec.
		res = key.SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "DNS Timeout");
		assert(res);

		value = 1;
		key.SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "DNS Tries");
		assert(res);
	}

	net::dns_tools::init_loaded_options();

	const net::address_v4::bytes_type ipv4 = { { 8,8,4,4} };
	net::dns::async_make_ptr_lookup(net::address_v4(ipv4), [](net::dns::hostent_reply host, boost::system::error_code ec)
	{
		assert(!ec || ec == net::dns::errors::no_data || ec == net::dns::errors::not_found);
		std::cerr << "PTR response for 8.8.4.4 query: " << host << std::endl;
	});

	const net::address_v6::bytes_type ipv6 = { { 0x20, 0x01, 0x48, 0x60, 0x48, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x44 } };
	net::dns::async_make_ptr_lookup(net::address_v6(ipv6), [](net::dns::hostent_reply host, boost::system::error_code ec)
	{
		assert(!ec || ec == net::dns::errors::no_data || ec == net::dns::errors::not_found);
		std::cerr << "PTR response for 2001:4860:4860::8844 query: " << host << std::endl;
	});

	net::dns::async_make_srv_lookup("_h323cs._tcp.cisco.com", [](net::dns::srv_reply_list reply, boost::system::error_code ec)
	{
		assert(!ec || ec == net::dns::errors::no_data || ec == net::dns::errors::not_found);
		std::cerr << "srv '_h323cs._tcp.cisco.com': {";
		for (auto &item : reply)
			std::cerr << " " << item;
		std::cerr << std::endl;
	});

	net::dns::async_make_srv_lookup("_h323cs._tcp.lifesize.com", [](net::dns::srv_reply_list reply, boost::system::error_code ec)
	{
		assert(!ec || ec == net::dns::errors::no_data || ec == net::dns::errors::not_found);
		std::cerr << "srv '_h323cs._tcp.lifesize.com': {";
		for (auto &item : reply)
			std::cerr << " " << item;
		std::cerr << std::endl;
	});

	net::dns::async_make_a_lookup("cisco.com", [](net::dns::hostent_reply host, boost::system::error_code ec)
	{
		assert(!ec || ec == net::dns::errors::no_data || ec == net::dns::errors::not_found);
		std::cerr << host << std::endl;
	});

	net::dns::async_make_aaaa_lookup("facebook.com", [](net::dns::hostent_reply host, boost::system::error_code ec)
	{
		assert(!ec || ec == net::dns::errors::no_data || ec == net::dns::errors::not_found);
		std::cerr << host << std::endl;
	});

	net::dns::async_make_a_lookup("localhost", [](net::dns::hostent_reply host, boost::system::error_code ec)
	{
		assert(!ec || ec == net::dns::errors::no_data || ec == net::dns::errors::not_found);
		std::cerr << host << std::endl;
	});

	//lookup file
	net::dns::async_make_a_lookup("example.com", [](net::dns::hostent_reply host, boost::system::error_code ec)
	{
		assert(!ec || ec == net::dns::errors::no_data || ec == net::dns::errors::not_found);
		std::cerr << host << std::endl;
	});


	net::dns::async_make_naptr_lookup("+441865332244", net::dns::naptr_type::e164, [](net::dns::naptr_reply_list reply, boost::system::error_code ec)
	{
		assert(!ec || ec == net::dns::errors::no_data || ec == net::dns::errors::not_found);

		std::cerr << "4.4.2.2.3.3.5.6.8.1.4.4.e164.arpa has NAPTR record: {";
		for (auto &item : reply)
			std::cerr << " " << item;
		std::cerr << std::endl;
	});

	net::dns::async_make_naptr_lookup("441865332243", net::dns::naptr_type::e164, [](net::dns::naptr_reply_list reply, boost::system::error_code ec)
	{
		assert(!ec || ec == net::dns::errors::no_data || ec == net::dns::errors::not_found);

		std::cerr << "3.4.2.2.3.3.5.6.8.1.4.4.e164.arpa has NAPTR record: {";
		for (auto &item : reply)
			std::cerr << " " << item;
		std::cerr << std::endl;
	});

	vs::SleepFor(std::chrono::seconds(6));
	std::cerr << "============== DNS CACHE ==============" << std::endl;
	net::dns::print_dns_cache(std::cerr);

	//////////////////////////////////////////////////////////////////////////

	std::cout << "|--------------------------------------------------------------|" << std::endl;

	auto res = net::dns::make_a_lookup("cisco.com").get();
	assert(!res.second || res.second == net::dns::errors::no_data || res.second == net::dns::errors::not_found);
	std::cerr << res.first << std::endl;

	auto a_aaaa_res = net::dns::make_a_aaaa_lookup("wikipedia.org", net::dns::real_inet).get();
	assert(!a_aaaa_res.first.ec || a_aaaa_res.first.ec == net::dns::errors::no_data || a_aaaa_res.first.ec == net::dns::errors::not_found);
	assert(!a_aaaa_res.second.ec || a_aaaa_res.second.ec == net::dns::errors::no_data || a_aaaa_res.second.ec == net::dns::errors::not_found);

	std::cout << a_aaaa_res.first.host << std::endl;
	std::cout << a_aaaa_res.second.host << std::endl;

	return 0;
}
