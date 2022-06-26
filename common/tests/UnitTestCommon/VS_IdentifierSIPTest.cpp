#include <tests/common/ASIOEnvironment.h>
#include "VS_DNSResolverMock.h"
#include <boost/smart_ptr/make_shared.hpp>

#include "TrueGateway/CallConfig/VS_IndentifierSIP.h"
#include "std/cpplib/VS_UserData.h"
#include "tests/common/GTestMatchers.h"

struct IdentifierSIPTest : public ::testing::Test {
	boost::shared_ptr<VS_Indentifier> identifier;
	net::dns::ResolverMock dnsResolverMock;

	static void SetUpTestCase()
	{
		m_oldResolver = net::dns::get_resolver_TEST();
	}

	void SetUp() override
	{
		dnsResolverMock.DelegateTo(m_oldResolver);
		net::dns::set_resolver_TEST(&dnsResolverMock);
		identifier = boost::make_shared<VS_IndentifierSIP>(g_asio_environment->IOService(),"ServerVendor");
	}

	static void TearDownTestCase()
	{
		net::dns::set_resolver_TEST(m_oldResolver);
	}
private:
	static net::dns::Resolver *m_oldResolver;
};

net::dns::Resolver *IdentifierSIPTest::m_oldResolver;

// to make sure if we failed to find SRV records but A resolve is not broken
TEST_F(IdentifierSIPTest, FailSRV_SucceedA) {
	using ::testing::_;
	using ::testing::AtLeast;
	using ::testing::StrNe;
	using ::testing::WithArgs;
	using ::testing::Invoke;

	// 1. Make SRV query always fail
	EXPECT_CALL(dnsResolverMock, Make_SRV_Lookup(StrNe(""), _, _)).Times(AtLeast(1));

	EXPECT_CALL(dnsResolverMock, Make_A_AAAA_Lookup(_, _, _)).Times(AtLeast(1)).WillRepeatedly(WithArgs<0, 1>(Invoke([](const std::string &query, const net::dns::a_aaaa_callback &cb)
	{
		cb(
			{ { {}, { net::address::from_string("127.0.0.1") } , query }, net::dns::errors::no_error } // ipv4
			, { { {}, { net::address::from_string("::1") }, query }, net::dns::errors::no_error } //ipv6
		);
	})));

	VS_UserData from; from.m_name = "from@server.trueconf.name";

	// 2. Make sure A resolve still working
	VS_CallConfig config;
	const bool res = identifier->Resolve(config,"#sip:user6@localhost", &from);
	ASSERT_TRUE(res);
	ASSERT_EQ(config.Address.addr, net::address::from_string("127.0.0.1"));
}

TEST_F(IdentifierSIPTest, SRV_resolve) {
	using ::testing::_;
	using ::testing::StrNe;
	using ::testing::WithArgs;
	using ::testing::AtLeast;
	using ::testing::Invoke;
	using ::testing::WithArg;

	// 1. Make fake endpoint that will be returned as result of DNS SRV query
	EXPECT_CALL(dnsResolverMock, Make_SRV_Lookup(StrNe(""), _, _)).Times(AtLeast(1)).WillRepeatedly(WithArg<1>(Invoke([](const net::dns::srv_callback &cb)
	{
		cb({ {"localhost", 1, 1, 5061} }, net::dns::errors::no_error);
	})));

	EXPECT_CALL(dnsResolverMock, Make_A_AAAA_Lookup(_, _, _)).Times(AtLeast(1)).WillRepeatedly(WithArgs<0, 1>(Invoke([](const std::string &query, const net::dns::a_aaaa_callback &cb)
	{
		cb(
			{ { {}, { net::address::from_string("127.0.0.1") } , query }, net::dns::errors::no_error } // ipv4
		  , { { {}, { net::address::from_string("::1") }, query }, net::dns::errors::no_error } //ipv6
		  );
	})));

	VS_UserData from; from.m_name = "from@server.trueconf.name";

	// 2. Make sure our endpoint is resolved like we expect i.e. : _tls._sip.somehost.com => localhost + port=5061 => 127.0.0.1 + port=5061
	VS_CallConfig config;
	const bool res = identifier->Resolve(config,"#sip:user6@somehost.com", &from);
	ASSERT_TRUE(res);
	ASSERT_EQ(config.Address.addr, net::address::from_string("127.0.0.1"));
	ASSERT_EQ(config.Address.port, 5061);
}

TEST_F(IdentifierSIPTest, PTR_resolve)
{
	using ::testing::_;
	using ::testing::Eq;
	using ::testing::WithArgs;
	using ::testing::AtLeast;
	using ::testing::Invoke;

	std::string hostname;
	const auto expect_addr = net::address::from_string("10.130.1.178");

	const auto test_default_cfg = [&]() noexcept
	{
		VS_CallConfig cfg;
		ASSERT_TRUE(identifier->CreateDefaultConfiguration(cfg, net::Endpoint{ expect_addr, 5060, net::protocol::UDP }, VS_CallConfig::SIP, "login"));

		ASSERT_STREQ(cfg.HostName.c_str(), hostname.c_str());
	};

	test_default_cfg();

	EXPECT_CALL(dnsResolverMock, Make_A_AAAA_Lookup(_, _, _)).Times(AtLeast(1)).WillRepeatedly(WithArgs<0, 1>(Invoke([&hostname, expect_addr](const std::string &q, const net::dns::a_aaaa_callback &cb)
	{
		hostname = q;
		cb(
			{ { {}, { expect_addr } , q }, net::dns::errors::no_error } // ipv4
			, { {}, net::dns::errors::not_found } //ipv6
		);
	})));

	EXPECT_CALL(dnsResolverMock, Make_PTR_Lookup(Eq(expect_addr), _, AllBitSet(net::dns::use_cache | net::dns::force_search | net::dns::insensitive_ttl))).Times(AtLeast(1)).WillRepeatedly(WithArgs<1>(Invoke([&hostname, expect_addr](const net::dns::ptr_callback &cb)
	{
		cb({ {}, {expect_addr}, { hostname } }, net::dns::errors::no_error);
	})));

	//reg or any lookup operation
	const auto res = net::dns::make_a_aaaa_lookup("test_hostname").get();
	ASSERT_TRUE(!res.first.ec || !res.second.ec);

	test_default_cfg();
}