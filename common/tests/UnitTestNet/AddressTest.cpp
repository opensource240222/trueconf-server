#include <gtest/gtest.h>
#include <functional>
#include "../net/Address.h"


struct AddresParam
{
	const char *inStr;
	const std::function<bool(string_view)> func;
	bool isValid;
};

class AddressValidationTest : public ::testing::TestWithParam<AddresParam> {};


TEST_P(AddressValidationTest, ValidateAddres)
{
	const AddresParam &param = GetParam();
	EXPECT_EQ(param.isValid, param.func(param.inStr));
}

INSTANTIATE_TEST_CASE_P(TestDomain, AddressValidationTest, ::testing::Values(
	AddresParam{ "hello.ru", net::is_domain_name, true },
	AddresParam{ "trueconf.com.ru", net::is_domain_name , true },
	AddresParam{ "errro.", net::is_domain_name, false },
	AddresParam{ "192.168.61.178", net::is_domain_name, false },
	AddresParam{ "127.0.0.1", net::is_domain_name, false },
	AddresParam{ "fd00:380:57::8ac6", net::is_domain_name, false },
	AddresParam{ "0000:0000:0000:0000:0000:0000:0000:0001", net::is_domain_name, false }
));

INSTANTIATE_TEST_CASE_P(TestIPv4, AddressValidationTest, ::testing::Values(
	AddresParam{ "trueconf.com.ru", net::is_ipv4, false },
	AddresParam{ "192.168.61.178", net::is_ipv4, true },
	AddresParam{ "trueconf.com.ru", net::is_ipv4, false },
	AddresParam{ "errro.", net::is_ipv4, false },
	AddresParam{ "127.0.0.1",  net::is_ipv4, true },
	AddresParam{ "fd00:380:57::8ac6", net::is_ipv4, false },
	AddresParam{ "0000:0000:0000:0000:0000:0000:0000:0001", net::is_ipv4, false }
));


INSTANTIATE_TEST_CASE_P(TestIPv6, AddressValidationTest, ::testing::Values(
	AddresParam{ "trueconf.com.ru", net::is_ipv6, false },
	AddresParam{ "192.168.61.178", net::is_ipv6, false },
	AddresParam{ "trueconf.com.ru", net::is_ipv6, false },
	AddresParam{ "errro.", net::is_ipv6, false },
	AddresParam{ "127.0.0.1",  net::is_ipv6, false },
	AddresParam{ "fd00:380:57::8ac6", net::is_ipv6, true },
	AddresParam{ "0000:0000:0000:0000:0000:0000:0000:0001", net::is_ipv6, true }
));
