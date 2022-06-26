#include "tests/common/ASIOEnvironment.h"
#include "tests/mocks/transport/EndpointMock.h"
#include "tests/mocks/transport/ServiceMock.h"
#include "transport/Message.h"
#include "newtransport/Const.h"
#include "newtransport/Router/Router.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/MakeShared.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace transport_test {

TEST(ProcessMessage, ClientLogin)
{
	using ::testing::AllOf;
	using ::testing::AtLeast;
	using ::testing::Eq;
	using ::testing::Invoke;
	using ::testing::Property;
	using ::testing::StrEq;

	auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), "pmnv111.trueconf.name#vcs");
	ServiceMock service("AUTH");
	EXPECT_CALL(service, GetName()).Times(AtLeast(1));
	router->AddService(&service);
	VS_SCOPE_EXIT { router->RemoveService("AUTH"); };

	auto endpoint = std::make_shared<EndpointMock>("4AE6D6ACEF52A8F6D7D60BCD935F9CD4");
	EXPECT_CALL(*endpoint, GetId()).Times(AtLeast(1));
	router->TEST_AddEndpoint(endpoint);

	EXPECT_CALL(service, ProcessMessage_mocked(
			Property(&transport::Message::DstService, StrEq("AUTH"))
		))
		.Times(1)
		.WillOnce(Invoke([&](transport::Message& message) {
			router->PostMessage(transport::Message::Make()
				.ReplyTo(message)
				.AddString("This is response")
				.Body("", 1)
			);
			return true;
		}));
	EXPECT_CALL(*endpoint, SendToPeer(AllOf(
			Property(&transport::Message::IsReply, Eq(true)),
			Property(&transport::Message::DstCID, StrEq("4AE6D6ACEF52A8F6D7D60BCD935F9CD4")),
			Property(&transport::Message::AddString, StrEq("This is response"))
		)))
		.Times(1);
	router->ProcessMessage(transport::Message::Make()
		.SrcCID("4AE6D6ACEF52A8F6D7D60BCD935F9CD4")
		.SrcService("AUTH")
		.DstService("AUTH")
		.Body(" ", 1)
	);
}

TEST(ProcessMessage, ClientPing)
{
	using ::testing::AtLeast;
	using ::testing::Property;
	using ::testing::StrEq;

	auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), "pmnv111.trueconf.name#vcs");
	auto endpoint = std::make_shared<EndpointMock>("www.server.com#vcs");
	EXPECT_CALL(*endpoint, GetId()).Times(AtLeast(1));
	router->TEST_AddEndpoint(endpoint);

	const char opcode[] = { transport::c_ping_opcode, '\0' };
	EXPECT_CALL(*endpoint, ProcessMessage(
			Property(&transport::Message::AddString, StrEq(opcode))
		))
		.Times(1);
	router->ProcessMessage(transport::Message::Make()
		.SrcCID("www.server.com#vcs")
		.AddString(opcode)
		.TimeLimit(transport::c_ping_time_limit)
		.Body(" ", 1)
	);
}

TEST(ProcessMessage, OnlineChatMessage)
{
	using ::testing::_;
	using ::testing::AnyNumber;
	using ::testing::AtLeast;
	using ::testing::Invoke;
	using ::testing::Property;
	using ::testing::Return;
	using ::testing::StrEq;

	auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), "pmnv111.trueconf.name#vcs");
	auto endpoint_1 = std::make_shared<EndpointMock>("4AE6D6ACEF52A8F6D7D60BCD935F9CD4");
	EXPECT_CALL(*endpoint_1, GetId()).Times(AtLeast(1));
	router->TEST_AddEndpoint(endpoint_1);

	auto endpoint_2 = std::make_shared<EndpointMock>("B1B037F5AA09508B177B503E4C92FCC5");
	EXPECT_CALL(*endpoint_2, GetId()).Times(AtLeast(1));
	router->TEST_AddEndpoint(endpoint_2);

	EXPECT_CALL(*endpoint_1, GetUserId())
		.Times(AtLeast(1))
		.WillRepeatedly(Return("user@pmnv111.trueconf.name"));
	EXPECT_CALL(*endpoint_2, GetUserId())
		.Times(AtLeast(1))
		.WillRepeatedly(Return("user2@pmnv111.trueconf.name"));
	EXPECT_CALL(*endpoint_2, IsAuthorized())
		.Times(AnyNumber())
		.WillRepeatedly(Return(true));
	EXPECT_CALL(*endpoint_1, SendToPeer(_))
		.Times(0);
	EXPECT_CALL(*endpoint_2, SendToPeer(
			Property(&transport::Message::DstService, StrEq("CHAT"))
		))
		.Times(1);
	router->ProcessMessage(transport::Message::Make()
		.SrcService("CHAT")
		.SrcServer("pmnv111.trueconf.name#vcs")
		.SrcUser("user@pmnv111.trueconf.name")
		.DstService("CHAT")
		.DstUser("user2@pmnv111.trueconf.name")
		.Body(" ", 1)
	);
}

TEST(ProcessMessage, OfflineChatMessage)
{
	using ::testing::_;
	using ::testing::AtLeast;
	using ::testing::Invoke;
	using ::testing::Property;
	using ::testing::Return;
	using ::testing::StrEq;

	auto router = vs::MakeShared<transport::Router>(g_asio_environment->IOService(), "pmnv111.trueconf.name#vcs");
	ServiceMock service("RESOLVE");
	EXPECT_CALL(service, GetName()).Times(AtLeast(1));
	router->AddService(&service);
	VS_SCOPE_EXIT { router->RemoveService("RESOLVE"); };

	auto endpoint = std::make_shared<EndpointMock>("4AE6D6ACEF52A8F6D7D60BCD935F9CD4");
	EXPECT_CALL(*endpoint, GetId()).Times(AtLeast(1));
	router->TEST_AddEndpoint(endpoint);

	EXPECT_CALL(*endpoint, GetUserId())
		.Times(AtLeast(1))
		.WillRepeatedly(Return("user"));
	EXPECT_CALL(service, ProcessMessage_mocked(
			Property(&transport::Message::DstService, StrEq("CHAT"))
		))
		.Times(1)
		.WillOnce(Invoke([&](transport::Message& message) {
			router->PostMessage(transport::Message::Make()
				.ReplyTo(message)
				.Body("", 1)
			);
			return true;
		}));
	router->ProcessMessage(transport::Message::Make()
		.SrcService("CHAT")
		.SrcServer("pmnv111.trueconf.name#vcs")
		.SrcUser("user@pmnv111.trueconf.name")
		.DstService("CHAT")
		.DstUser("user2@pmnv111.trueconf.name")
		.Body(" ", 1)
	);
}

}
