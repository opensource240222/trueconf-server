#include "FakeClient/VS_FakeEndpointV2.h"
#include "newtransport/Router/Router.h"
#include "std/cpplib/event.h"
#include "std-generic/cpplib/scope_exit.h"
#include "tests/common/ASIOEnvironment.h"
#include "tests/common/TestHelpers.h"
#include "tests/mocks/transport/ServiceMock.h"
#include "tests/mocks/VS_FakeEndpointMock.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "std-generic/compat/memory.h"
#include "std/cpplib/MakeShared.h"

namespace fake_client_test {

using transport_test::ServiceMock;

static const char c_router_ep_name[] = "v2.fake_endpoint.test";

TEST(FakeEndpointV2, Create)
{
	auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), c_router_ep_name);
	auto ep = vs::make_unique<VS_FakeEndpointV2>(router);
	ASSERT_NE(ep, nullptr);

	EXPECT_FALSE(ep->CID().empty());
	EXPECT_TRUE(router->EndpointExists(ep->CID()));
}

TEST(FakeEndpointV2, Send)
{
	using ::testing::AllOf;
	using ::testing::AtLeast;
	using ::testing::Eq;
	using ::testing::Property;

	auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), c_router_ep_name);
	auto ep = vs::make_unique<VS_FakeEndpointV2>(router);
	ASSERT_NE(ep, nullptr);

	ServiceMock service("ABC");
	EXPECT_CALL(service, GetName()).Times(AtLeast(1));
	router->AddService(&service);
	VS_SCOPE_EXIT { router->RemoveService("ABC"); };

	EXPECT_CALL(service, ProcessMessage_mocked(AllOf(
			Property(&transport::Message::SrcCID_sv, Eq(ep->CID())),
			Property(&transport::Message::SrcService_sv, Eq("ABC")),
			Property(&transport::Message::DstService_sv, Eq("ABC"))
		)))
		.Times(1);
	ep->Send(transport::Message::Make()
		.SrcService("ABC")
		.DstService("ABC")
		.Body("", 1)
	);
}

TEST(FakeEndpointV2, Receive)
{
	using ::testing::AllOf;
	using ::testing::AtLeast;
	using ::testing::Eq;
	using ::testing::Invoke;
	using ::testing::Property;

	auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), c_router_ep_name);
	auto ep = vs::make_unique<VS_FakeEndpointV2>(router);
	ASSERT_NE(ep, nullptr);

	auto receiver = std::make_shared<VS_FakeEndpointReceiverMock>();

	vs::event done(true);
	EXPECT_CALL(*receiver, OnReceive(AllOf(
			Property(&transport::Message::DstCID_sv, Eq(ep->CID())),
			Property(&transport::Message::SrcService_sv, Eq("ABC")),
			Property(&transport::Message::DstService_sv, Eq("ABC"))
		)))
		.Times(1)
		.WillOnce(Invoke([&](const transport::Message&) { done.set(); }));

	ep->SetReceiver(receiver);
	router->ProcessMessage(transport::Message::Make()
		.DstCID(ep->CID())
		.SrcService("ABC")
		.DstService("ABC")
		.Body("", 1)
	);
	EXPECT_TRUE(test::WaitFor("OnReceive() call", done));
}

TEST(FakeEndpointV2, Error)
{
	using ::testing::Invoke;

	auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), c_router_ep_name);
	auto ep = vs::make_unique<VS_FakeEndpointV2>(router);
	ASSERT_NE(ep, nullptr);

	auto receiver = std::make_shared<VS_FakeEndpointReceiverMock>();

	vs::event done(true);
	EXPECT_CALL(*receiver, OnError(-1))
		.Times(1)
		.WillOnce(Invoke([&](unsigned) { done.set(); }));

	ep->SetReceiver(receiver);
	router->DisconnectEndpoint(ep->CID());
	EXPECT_TRUE(test::WaitFor("OnError() call", done));
}

TEST(FakeEndpointV2, Timeout)
{
	using ::testing::AtLeast;
	using ::testing::Invoke;

	auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), c_router_ep_name);
	auto ep = vs::make_unique<VS_FakeEndpointV2>(router);
	ASSERT_NE(ep, nullptr);

	auto receiver = std::make_shared<VS_FakeEndpointReceiverMock>();

	vs::event done(true);
	EXPECT_CALL(*receiver, Timeout())
		.Times(AtLeast(1))
		.WillOnce(Invoke([&]() { done.set(); }));

	ep->SetReceiver(receiver);
	EXPECT_TRUE(test::WaitFor("Timeout() call", done, 1000));
}

}
