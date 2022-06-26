#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "tests/common/ASIOEnvironment.h"
#include "acs_v2/Service.h"
#include "transport/Message.h"
#include "newtransport/Router/Endpoint.h"
#include "newtransport/Router/Router.h"
#include "tests/UnitTestSecureLib/UnitTestConstants.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/MakeShared.h"
#include "std-generic/cpplib/string_view.h"
#include "newtransport/Router/JsonMonitorHandler.h"
#include "MonitorClient.h"
#include "newtransport/Lib/TransportUtils.h"
#include "std-generic/cpplib/ThreadUtils.h"

class EndpointTest : public ::testing::Test
{
public:
	EndpointTest(){}
protected:
	virtual void SetUp()
	{
		acs_srv = acs::Service::Create(g_asio_environment->IOService());
		auto start_f = acs_srv->Start();
		EXPECT_EQ(std::future_status::ready, start_f.wait_for(std::chrono::seconds(1))) << "acs::Service::Start took too long";
		if (start_f.valid())
			EXPECT_TRUE(start_f.get()) << "acs::Service::Start failed";

	}

	virtual void TearDown()
	{
	}

	std::shared_ptr<acs::Service> acs_srv;
};



TEST_F(EndpointTest, DISABLED_ConnectToRemoteEndpoint)
{
	try
	{
		auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), "ub1lx.trueconf.name#vcs");
		ASSERT_TRUE(router->SrvPrivateKey().SetPrivateKey(constants::server_private_key, store_PEM_BUF));
		ASSERT_TRUE(router->SrvCertChain().AddValue(CERTIFICATE_PARAM, constants::server_cert, sizeof(constants::server_cert)));
		ASSERT_TRUE(router->SrvCertChain().AddValue(CERTIFICATE_CHAIN_PARAM, constants::server_cert_chain, sizeof(constants::server_cert_chain)));

		boost::asio::ip::tcp::socket sock(g_asio_environment->IOService());
		//transport::Endpoint *endpoint_ptr = new transport::Endpoint(g_asio_environment->IOService(), router, "pmnv111.trueconf.name#vcs", std::move(sock), false, 1, 0x19 /*0x09*/, 0x02, false /*, "pmnv111.trueconf.name#vcs"*/);
		//auto endpoint = std::unique_ptr<transport::IEndpoint>( endpoint_ptr
		//); //reg.trueconf.com#regs
		//router->TEST_AddEndpoint(std::move(endpoint));
		auto endpoint_ptr = std::make_shared<transport::Endpoint>(g_asio_environment->IOService(), router, "pmnv111.trueconf.name#vcs");
		//endpoint_ptr->SetConnection(std::move(sock), false, 1, 0x19 /*0x09*/, 0x02, false);
		router->TEST_AddEndpoint(endpoint_ptr);

		//endpoint->SetConnection(std::move(sock), false, 1, 0x19 /*0x09*/, 0x02, false);
		/*VS_Container	cnt;
		cnt.AddValue("Method", "UpdateLicene");
		cnt.AddValue("arm_hw_key", 0); //(long)VS_ArmReadHardwareKey()
		void* body;
		size_t body_size;
		cnt.SerializeAlloc(body, body_size);
		auto router_message = std::make_unique<transport::RouterMessage>("reg.trueconf.com#regs", "", "REGISTRATION", "331", "ub1lx.trueconf.name#vcs", 20000, body, body_size);
		endpoint->AsyncSend(router_message.get());
		*/
		//for (;;)
		{
			//endpoint->SendPing();
			vs::SleepFor(std::chrono::milliseconds(5000));
			endpoint_ptr->SendPing();
			vs::SleepFor(std::chrono::milliseconds(5000));
		}
		vs::SleepFor(std::chrono::milliseconds(600000));
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}
}

TEST_F(EndpointTest, DISABLED_AcceptConnection)
{
	try
	{
		auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), "ub1lx.trueconf.name#vcs");
		ASSERT_TRUE(router->SrvPrivateKey().SetPrivateKey(constants::server_private_key, store_PEM_BUF));
		ASSERT_TRUE(router->SrvCertChain().AddValue(CERTIFICATE_PARAM, constants::server_cert, sizeof(constants::server_cert)));
		ASSERT_TRUE(router->SrvCertChain().AddValue(CERTIFICATE_CHAIN_PARAM, constants::server_cert_chain, sizeof(constants::server_cert_chain)));
		router->SetSrvCert(constants::server_cert);
		auto transport_handler = std::make_shared<transport::Handler>(router.get());
		acs_srv->AddHandler("Transport Handler", transport_handler);
		ASSERT_TRUE(!acs_srv->AddListener(net::address_v4::any(), 4307, net::protocol::TCP));

		for (;;)
		{
			//endpoint->SendPing();
			vs::SleepFor(std::chrono::milliseconds(5000));
			//endpoint_ptr->SendPing();
		}
		vs::SleepFor(std::chrono::milliseconds(600000));
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}
}

TEST_F(EndpointTest, DISABLED_TransportMonitorTest)
{
	try
	{
		auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), "ub1lx.trueconf.name#vcs");
		ASSERT_TRUE(router->SrvPrivateKey().SetPrivateKey(constants::server_private_key, store_PEM_BUF));
		ASSERT_TRUE(router->SrvCertChain().AddValue(CERTIFICATE_PARAM, constants::server_cert, sizeof(constants::server_cert)));
		ASSERT_TRUE(router->SrvCertChain().AddValue(CERTIFICATE_CHAIN_PARAM, constants::server_cert_chain, sizeof(constants::server_cert_chain)));
		router->SetSrvCert( constants::server_cert );
		auto transport_handler = std::make_shared<transport::Handler>(router.get());
		acs_srv->AddHandler("Transport Handler", transport_handler);
		ASSERT_TRUE(!acs_srv->AddListener(net::address_v4::any(), 4307, net::protocol::TCP));
		auto json_monitor_handler = std::make_shared<transport::JsonMonitorHandler>(router);
		acs_srv->AddHandler("Transport Monitor Handler", json_monitor_handler);
		MonitorClient client(g_asio_environment->IOService());
		client.Start();
		vs::SleepFor(std::chrono::milliseconds(600000));
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}
}
