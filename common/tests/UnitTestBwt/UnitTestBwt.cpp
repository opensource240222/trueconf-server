#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "tests/common/ASIOEnvironment.h"
#include "acs_v2/Service.h"
#include "std-generic/cpplib/scope_exit.h"
#include "Bwt/RunTest.h"
#include "Bwt/Handler.h"

#include <memory>
#include <chrono>
#include "std-generic/cpplib/ThreadUtils.h"


using namespace std::chrono;

class BwtTest : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		acs_srv = acs::Service::Create(g_asio_environment->IOService());
		auto start_f = acs_srv->Start();
		EXPECT_EQ(std::future_status::ready, start_f.wait_for(std::chrono::seconds(1))) << "acs::Service::Start took too long";
		if (start_f.valid())
			EXPECT_TRUE(start_f.get()) << "acs::Service::Start failed";

		bwt_handler = std::make_shared<bwt::Handler>();
		acs_srv->AddHandler("Bwt Handler", bwt_handler);
		EXPECT_TRUE(!acs_srv->AddListener(net::address_v4::loopback(), 4037, net::protocol::TCP));
		endpoint.endpoint = "127.0.0.1#as";
		endpoint.host = "127.0.0.1";
		endpoint.port = "4037";
		intermediate = std::make_shared<bwt::Intermediate>();
	}

	virtual void TearDown()
	{
		VS_SCOPE_EXIT
		{
			if (acs_srv)
				EXPECT_EQ(std::future_status::ready, acs_srv->Stop().wait_for(std::chrono::seconds(1))) << "acs::Service::Stop took too long";
		};

		ASSERT_TRUE(static_cast<bool>(intermediate));
		auto start_time = system_clock::now();
		while ((system_clock::now() - start_time) < std::chrono::seconds(20) && !intermediate->Done())
		{
			vs::SleepFor(milliseconds(100));
		}
		EXPECT_EQ(intermediate->GetStatus(), VS_BWT_ST_FINISH_TEST);
	}

	std::shared_ptr<acs::Service> acs_srv;
	std::shared_ptr<bwt::Handler> bwt_handler;
	bwt::Endpoint endpoint;
	std::shared_ptr<bwt::Intermediate> intermediate;
};

static constexpr unsigned c_bwt_test_time_s = 2; // seconds

TEST_F(BwtTest, Bandwidth_Out_Test)
{
	bwt::RunTest(endpoint, intermediate, VS_BWT_MODE_OUT, c_bwt_test_time_s);
}

TEST_F(BwtTest, Bandwidth_In_Test)
{
	bwt::RunTest(endpoint, intermediate, VS_BWT_MODE_IN, c_bwt_test_time_s);
}

TEST_F(BwtTest, Bandwidth_Duplex_Test)
{
	bwt::RunTest(endpoint, intermediate, VS_BWT_MODE_DUPLEX, c_bwt_test_time_s);
}

TEST_F(BwtTest, Bandwidth_Halfduplex_Test)
{
	bwt::RunTest(endpoint, intermediate, VS_BWT_MODE_HALFDUPLEX, c_bwt_test_time_s);
}

TEST_F(BwtTest, Bandwidth_Out_AsyncTest)
{
	bwt::RunTestAsync(endpoint, intermediate, VS_BWT_MODE_OUT, c_bwt_test_time_s);
}

TEST_F(BwtTest, Bandwidth_In_AsyncTest)
{
	bwt::RunTestAsync(endpoint, intermediate, VS_BWT_MODE_IN, c_bwt_test_time_s);
}

TEST_F(BwtTest, Bandwidth_Duplex_AsyncTest)
{
	bwt::RunTestAsync(endpoint, intermediate, VS_BWT_MODE_DUPLEX, c_bwt_test_time_s);
}

TEST_F(BwtTest, Bandwidth_Halfduplex_AsyncTest)
{
	bwt::RunTestAsync(endpoint, intermediate, VS_BWT_MODE_HALFDUPLEX, c_bwt_test_time_s);
}
