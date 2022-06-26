#include "tests/common/ASIOEnvironment.h"
#include "tests/mocks/transport/EndpointMock.h"
#include "tests/mocks/transport/ServiceMock.h"
#include "transport/Message.h"
#include "newtransport/Router/Router.h"
#include "std/cpplib/event.h"
#include "std/cpplib/MakeShared.h"
#include "std-generic/cpplib/scope_exit.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace transport_test {

TEST(TransportRouter, AddService)
{
	using ::testing::AtLeast;

	auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), "ub1lx.trueconf.name#vcs");
	ServiceMock service_1("AUTH");
	EXPECT_CALL(service_1, GetName()).Times(AtLeast(1));
	ASSERT_TRUE(router->AddService(&service_1));
	VS_SCOPE_EXIT { router->RemoveService("AUTH"); };

	ServiceMock service_2("AUTH");
	EXPECT_CALL(service_2, GetName()).Times(AtLeast(1));
	ASSERT_FALSE(router->AddService(&service_2));
	VS_SCOPE_EXIT { router->RemoveService("AUTH"); };
}

TEST(TransportRouter, RemoveService)
{
	using ::testing::AtLeast;

	auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), "ub1lx.trueconf.name#vcs");
	ASSERT_FALSE(router->RemoveService("AUTH"));
	ServiceMock service("AUTH");
	EXPECT_CALL(service, GetName()).Times(AtLeast(1));
	ASSERT_TRUE(router->AddService(&service));
	VS_SCOPE_EXIT { router->RemoveService("AUTH"); };
	ASSERT_TRUE(router->RemoveService("AUTH"));
}

TEST(TransportRouter, EndpointExists)
{
	using ::testing::AtLeast;

	auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), "ub1lx.trueconf.name#vcs");
	auto endpoint = std::make_shared<EndpointMock>("4AE6D6ACEF52A8F6D7D60BCD935F9CD4");
	EXPECT_CALL(*endpoint, GetId()).Times(AtLeast(1));
	router->TEST_AddEndpoint(endpoint);
	ASSERT_TRUE(router->EndpointExists("4AE6D6ACEF52A8F6D7D60BCD935F9CD4"));
	endpoint.reset();
	ASSERT_FALSE(router->EndpointExists("4AE6D6ACEF52A8F6D7D60BCD935F9CD4"));
}

TEST(TransportRouter, RemoveEndpoint)
{
	using ::testing::AtLeast;

	auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), "ub1lx.trueconf.name#vcs");
	auto endpoint = std::make_shared<EndpointMock>("4AE6D6ACEF52A8F6D7D60BCD935F9CD4");
	EXPECT_CALL(*endpoint, GetId()).Times(AtLeast(1));
	router->TEST_AddEndpoint(endpoint);
	ASSERT_TRUE(router->EndpointExists("4AE6D6ACEF52A8F6D7D60BCD935F9CD4"));
	router->RemoveEndpoint(endpoint);
	ASSERT_FALSE(router->EndpointExists("4AE6D6ACEF52A8F6D7D60BCD935F9CD4"));
}

TEST(TransportRouter, RequestResponse_Callback)
{
	using ::testing::AllOf;
	using ::testing::AtLeast;
	using ::testing::Invoke;
	using ::testing::Property;
	using ::testing::StrEq;

	auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), "ub1lx.trueconf.name#vcs");
	ServiceMock service("TEST_SRV");
	EXPECT_CALL(service, GetName()).Times(AtLeast(1));
	ASSERT_TRUE(router->AddService(&service));
	VS_SCOPE_EXIT { router->RemoveService("TEST_SRV"); };

	transport::Message request = transport::Message::Make()
		.SrcCID("OurCID")
		.DstService("TEST_SRV")
		.DstServer("ub1lx.trueconf.name#vcs")
		.AddString("This is request")
		.Body("", 1)
		;
	EXPECT_TRUE(request.IsValid());

	EXPECT_CALL(service, ProcessMessage_mocked(AllOf(
			Property(&transport::Message::DstService, StrEq("TEST_SRV")),
			Property(&transport::Message::AddString, StrEq("This is request"))
		)))
		.Times(1)
		.WillOnce(Invoke([&](transport::Message& message) {
			router->PostMessage(transport::Message::Make()
				.ReplyTo(message)
				.AddString("This is response")
				.Body("", 1)
			);
			return true;
		}));

	auto result_p = std::make_shared<std::promise<transport::Message>>();
	auto result_f = result_p->get_future();
	EXPECT_TRUE(router->RequestResponse(std::move(request), [result_p](transport::Message&& message) {
		result_p->set_value(std::move(message));
	}, std::chrono::seconds(1)));
	ASSERT_EQ(std::future_status::ready, result_f.wait_for(std::chrono::milliseconds(200)));

	auto result = result_f.get();
	EXPECT_TRUE(result.IsReply());
	EXPECT_THAT(result.SrcService(), StrEq("TEST_SRV"));
	EXPECT_THAT(result.DstCID(), StrEq("OurCID"));
	EXPECT_THAT(result.AddString(), StrEq("This is response"));
}

TEST(TransportRouter, RequestResponse_Future)
{
	using ::testing::AllOf;
	using ::testing::AtLeast;
	using ::testing::Invoke;
	using ::testing::Property;
	using ::testing::StrEq;

	auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), "ub1lx.trueconf.name#vcs");
	ServiceMock service("TEST_SRV");
	EXPECT_CALL(service, GetName()).Times(AtLeast(1));
	ASSERT_TRUE(router->AddService(&service));
	VS_SCOPE_EXIT { router->RemoveService("TEST_SRV"); };

	transport::Message request = transport::Message::Make()
		.SrcCID("OurCID")
		.DstService("TEST_SRV")
		.DstServer("ub1lx.trueconf.name#vcs")
		.AddString("This is request")
		.Body("", 1)
		;
	EXPECT_TRUE(request.IsValid());

	EXPECT_CALL(service, ProcessMessage_mocked(AllOf(
			Property(&transport::Message::DstService, StrEq("TEST_SRV")),
			Property(&transport::Message::AddString, StrEq("This is request"))
		)))
		.Times(1)
		.WillOnce(Invoke([&](transport::Message& message) {
			router->PostMessage(transport::Message::Make()
				.ReplyTo(message)
				.AddString("This is response")
				.Body("", 1)
			);
			return true;
		}));

	auto result_f = router->RequestResponse(std::move(request), std::chrono::seconds(1));
	ASSERT_EQ(std::future_status::ready, result_f.wait_for(std::chrono::milliseconds(200)));

	auto result = result_f.get();
	EXPECT_TRUE(result.IsReply());
	EXPECT_THAT(result.SrcService(), StrEq("TEST_SRV"));
	EXPECT_THAT(result.DstCID(), StrEq("OurCID"));
	EXPECT_THAT(result.AddString(), StrEq("This is response"));
}

}
