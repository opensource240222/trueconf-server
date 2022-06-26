#include "tests/common/ASIOEnvironment.h"
#include "tests/common/GMockOverride.h"
#include "tests/common/GTestMatchers.h"
#include "acs_v2/Service.h"
#include "acs_v2/Handler.h"
#include "acs_v2/Error.h"
#include "net/EndpointRegistry.h"
#include "net/InterfaceInfo.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/event.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>

#include <memory>
#include <string>
#include <vector>

#if 0
namespace boost { namespace asio {
template <class charT, class traits>
const_buffers_1 buffer(basic_string_view<charT, traits> x)
{
	return const_buffers_1(x.data(), x.size());
}
}}
#endif

namespace asc_test {

static const net::port test_port_1 = 43007;
static const net::port test_port_2 = 7034;
static const std::string test_endpoint_name("UnitTest_ACS_1");
static const string_view test_handler_name_1("handler 1");
static const string_view test_handler_name_2("handler 2");
static const std::string test_data[] = { "test data", " please", " ignore" };
static const std::string test_data_inc[] = {
	test_data[0],
	test_data[0] + test_data[1],
	test_data[0] + test_data[1] + test_data[2],
};
struct channel_1_tag {};
struct channel_2_tag {};

class ACSHandlerMock : public acs::Handler
{
public:
	ACSHandlerMock()
	{
		using ::testing::_;
		using ::testing::Invoke;
		using ::testing::Return;

		ON_CALL(*this, Init(_)).WillByDefault(Invoke([this](string_view name) { return Handler::Init(name); }));
		ON_CALL(*this, Protocol_TCP(_, _)).WillByDefault(Return(acs::Response::not_my_connection));
		ON_CALL(*this, Protocol_UDP(_, _)).WillByDefault(Return(acs::Response::not_my_connection));
	}

	void DelegateTo(acs::Handler* impl, boost::shared_ptr<void> track = nullptr)
	{
		using ::testing::_;
		using ::testing::Invoke;

		ON_CALL(*this, Init(_)).WillByDefault(Invoke([this, impl](string_view name) {
			return Handler::Init(name) && impl->Init(name);
		}));
		ON_CALL(*this, Protocol_TCP(_, _)).WillByDefault(Invoke(impl, static_cast<acs::Response (acs::Handler::*)(const stream_buffer& buffer, unsigned channel_token)>(&Handler::Protocol)));
		ON_CALL(*this, Protocol_UDP(_, _)).WillByDefault(Invoke(impl, static_cast<acs::Response (acs::Handler::*)(const packet_buffer& buffer, unsigned channel_token)>(&Handler::Protocol)));
		ON_CALL(*this, Accept_TCP(_, _)).WillByDefault(Invoke([this, impl](boost::asio::ip::tcp::socket& socket, stream_buffer& buffer) {
			return impl->Accept(std::move(socket), std::move(buffer));
		}));
		ON_CALL(*this, Accept_UDP(_, _)).WillByDefault(Invoke([this, impl](net::UDPConnection& connection, packet_buffer& buffer) {
			return impl->Accept(std::move(connection), std::move(buffer));
		}));

		m_track = track;
	}

	MOCK_METHOD1_OVERRIDE(Init, bool(string_view name));
	MOCK_METHOD2(Protocol_TCP, acs::Response(const stream_buffer& buffer, unsigned channel_token));
	acs::Response Protocol(const stream_buffer& buffer, unsigned channel_token) override
	{
		return Protocol_TCP(buffer, channel_token);
	}
	MOCK_METHOD2(Protocol_UDP, acs::Response(const packet_buffer& buffer, unsigned channel_token));
	acs::Response Protocol(const packet_buffer& buffer, unsigned channel_token) override
	{
		return Protocol_UDP(buffer, channel_token);
	}
	MOCK_METHOD2(Accept_TCP, void(boost::asio::ip::tcp::socket& socket, stream_buffer& buffer));
	void Accept(boost::asio::ip::tcp::socket&& socket, stream_buffer&& buffer) override
	{
		return Accept_TCP(socket, buffer);
	}
	MOCK_METHOD2(Accept_UDP, void(net::UDPConnection& connection, packet_buffer& buffer));
	void Accept(net::UDPConnection&& connection, packet_buffer&& buffer) override
	{
		return Accept_UDP(connection, buffer);
	}

private:
	boost::shared_ptr<void> m_track;
};

class ACSTest : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		acs_srv = acs::Service::Create(g_asio_environment->IOService());
		auto start_f = acs_srv->Start();
		EXPECT_EQ(std::future_status::ready, start_f.wait_for(std::chrono::seconds(1))) << "acs::Service::Start took too long";
		if (start_f.valid())
		{
			EXPECT_TRUE(start_f.get()) << "acs::Service::Start failed";
		}
	}

	virtual void TearDown()
	{
		VS_SCOPE_EXIT
		{
			if (acs_srv)
			{
				EXPECT_EQ(std::future_status::ready, acs_srv->Stop().wait_for(std::chrono::seconds(1))) << "acs::Service::Stop took too long";
			}
		};
	}

	std::shared_ptr<acs::Service> acs_srv;
};

TEST(ACSMiscTest, ChannelToken)
{
	auto token_1 = acs::ChannelToken<channel_1_tag>();
	EXPECT_NE(0, token_1);
	auto token_2 = acs::ChannelToken<channel_2_tag>();
	EXPECT_NE(0, token_2);

	EXPECT_EQ(token_1, acs::ChannelToken<channel_1_tag>());
	EXPECT_EQ(token_2, acs::ChannelToken<channel_2_tag>());
	EXPECT_TRUE(token_1 != token_2);
}

TEST_F(ACSTest, Simple)
{
}

TEST_F(ACSTest, AddHandler)
{
	boost::system::error_code ec;

	auto h1 = std::make_shared<ACSHandlerMock>();
	EXPECT_CALL(*h1, Init(test_handler_name_1)).Times(1);
	ec = acs_srv->AddHandler(test_handler_name_1, h1);
	EXPECT_FALSE(ec) << ec.message();
}

TEST_F(ACSTest, AddHandler_InitFailure)
{
	using ::testing::Return;

	boost::system::error_code ec;

	auto h1 = std::make_shared<ACSHandlerMock>();
	EXPECT_CALL(*h1, Init(test_handler_name_1)).Times(1).WillOnce(Return(false));
	ec = acs_srv->AddHandler(test_handler_name_1, h1);
	EXPECT_EQ(acs::errc::handler_initialization_failed, ec) << ec.message();
}

TEST_F(ACSTest, RemoveHandler)
{
	boost::system::error_code ec;

	auto h1 = std::make_shared<ACSHandlerMock>();
	EXPECT_CALL(*h1, Init(test_handler_name_1)).Times(1);
	ec = acs_srv->AddHandler(test_handler_name_1, h1);
	EXPECT_FALSE(ec) << ec.message();

	ec = acs_srv->RemoveHandler(test_handler_name_1);
	EXPECT_FALSE(ec) << ec.message();
}

TEST_F(ACSTest, RemoveHandler_WrongName)
{
	boost::system::error_code ec;

	auto h1 = std::make_shared<ACSHandlerMock>();
	EXPECT_CALL(*h1, Init(test_handler_name_1)).Times(1);
	ec = acs_srv->AddHandler(test_handler_name_1, h1);
	EXPECT_FALSE(ec) << ec.message();

	ec = acs_srv->RemoveHandler("different name");
	EXPECT_EQ(acs::errc::handler_not_found, ec) << ec.message();
}

TEST_F(ACSTest, RemoveHandler_DeadHandler)
{
	boost::system::error_code ec;

	auto h1 = std::make_shared<ACSHandlerMock>();
	EXPECT_CALL(*h1, Init(test_handler_name_1)).Times(1);
	ec = acs_srv->AddHandler(test_handler_name_1, h1);
	EXPECT_FALSE(ec) << ec.message();

	h1.reset();

	ec = acs_srv->RemoveHandler(test_handler_name_1);
	EXPECT_EQ(acs::errc::handler_not_found, ec) << ec.message();
}

TEST_F(ACSTest, GetListenerList_Loopback)
{
	using ::testing::AllOf;
	using ::testing::Contains;
	using ::testing::Field;

	boost::system::error_code ec;

	ec = acs_srv->AddListener(net::address_v4::loopback(), test_port_1, net::protocol::TCP);
	EXPECT_FALSE(ec) << ec.message();
	ec = acs_srv->AddListener(net::address_v4::loopback(), test_port_2, net::protocol::TCP);
	EXPECT_FALSE(ec) << ec.message();
	ec = acs_srv->AddListener(net::address_v6::loopback(), test_port_2, net::protocol::TCP);
	EXPECT_FALSE(ec) << ec.message();

	acs::Service::address_list listener_list;
	acs_srv->GetListenerList(listener_list, net::protocol::TCP);
	EXPECT_THAT(listener_list, Contains(AllOf(
		Field(&acs::Service::address_list::value_type::first, net::address_v4::loopback()),
		Field(&acs::Service::address_list::value_type::second, test_port_1)
	)));
	EXPECT_THAT(listener_list, Contains(AllOf(
		Field(&acs::Service::address_list::value_type::first, net::address_v4::loopback()),
		Field(&acs::Service::address_list::value_type::second, test_port_2)
	)));
	EXPECT_THAT(listener_list, Contains(AllOf(
		Field(&acs::Service::address_list::value_type::first, net::address_v6::loopback()),
		Field(&acs::Service::address_list::value_type::second, test_port_2)
	)));
}

TEST_F(ACSTest, GetListenerList_Any)
{
	using ::testing::AllOf;
	using ::testing::Contains;
	using ::testing::Field;

	boost::system::error_code ec;

	ec = acs_srv->AddListener(net::address_v4::any(), test_port_1, net::protocol::TCP);
	EXPECT_FALSE(ec) << ec.message();
	ec = acs_srv->AddListener(net::address_v6::any(), test_port_2, net::protocol::TCP);
	EXPECT_FALSE(ec) << ec.message();

	acs::Service::address_list listener_list;
	acs_srv->GetListenerList(listener_list, net::protocol::TCP);
	const auto interfaces = net::GetInterfaceInfo();
	for (const auto& ii : *interfaces)
	{
		for (const auto& i_addr: ii.addr_local_v4)
		{
			EXPECT_THAT(listener_list, Contains(AllOf(
				Field(&acs::Service::address_list::value_type::first, i_addr),
				Field(&acs::Service::address_list::value_type::second, test_port_1)
			)));
		}
		for (const auto& i_addr: ii.addr_local_v6)
		{
			EXPECT_THAT(listener_list, Contains(AllOf(
				Field(&acs::Service::address_list::value_type::first, i_addr),
				Field(&acs::Service::address_list::value_type::second, test_port_2)
			)));
		}
	}
}

TEST_F(ACSTest, GetListenerList_CorrectProtocol)
{
	using ::testing::AllOf;
	using ::testing::Contains;
	using ::testing::Field;
	using ::testing::Not;

	boost::system::error_code ec;

	ec = acs_srv->AddListener(net::address_v4::loopback(), test_port_1, net::protocol::TCP);
	EXPECT_FALSE(ec) << ec.message();
	ec = acs_srv->AddListener(net::address_v4::loopback(), test_port_2, net::protocol::UDP);
	EXPECT_FALSE(ec) << ec.message();

	{
		acs::Service::address_list listener_list;
		acs_srv->GetListenerList(listener_list, net::protocol::TCP);
		EXPECT_THAT(listener_list, Contains(AllOf(
			Field(&acs::Service::address_list::value_type::first, net::address_v4::loopback()),
			Field(&acs::Service::address_list::value_type::second, test_port_1)
		)));
		EXPECT_THAT(listener_list, Not(Contains(
			Field(&acs::Service::address_list::value_type::second, test_port_2)
		)));
	}

	{
		acs::Service::address_list listener_list;
		acs_srv->GetListenerList(listener_list, net::protocol::UDP);
		EXPECT_THAT(listener_list, Contains(AllOf(
			Field(&acs::Service::address_list::value_type::first, net::address_v4::loopback()),
			Field(&acs::Service::address_list::value_type::second, test_port_2)
		)));
		EXPECT_THAT(listener_list, Not(Contains(
			Field(&acs::Service::address_list::value_type::second, test_port_1)
		)));
	}
}

TEST_F(ACSTest, GetListenerList_Hidden)
{
	using ::testing::AllOf;
	using ::testing::Not;
	using ::testing::Contains;
	using ::testing::Field;

	boost::system::error_code ec;

	ec = acs_srv->AddListener(net::address_v4::loopback(), test_port_1, net::protocol::TCP, 0, true);
	EXPECT_FALSE(ec) << ec.message();
	ec = acs_srv->AddListener(net::address_v4::loopback(), test_port_2, net::protocol::TCP);
	EXPECT_FALSE(ec) << ec.message();

	acs::Service::address_list listener_list;
	acs_srv->GetListenerList(listener_list, net::protocol::TCP);
	EXPECT_THAT(listener_list, Not(Contains(
		Field(&acs::Service::address_list::value_type::second, test_port_1)
	)));
	EXPECT_THAT(listener_list, Contains(AllOf(
		Field(&acs::Service::address_list::value_type::first, net::address_v4::loopback()),
		Field(&acs::Service::address_list::value_type::second, test_port_2)
	)));
}

TEST_F(ACSTest, WatchdogTest)
{
	boost::system::error_code ec;

	ec = acs_srv->AddListener(net::address_v4::any(), test_port_1, net::protocol::TCP);
	EXPECT_FALSE(ec) << ec.message();
	ec = acs_srv->AddListener(net::address_v6::loopback(), test_port_2, net::protocol::TCP);
	EXPECT_FALSE(ec) << ec.message();

	EXPECT_TRUE(acs_srv->Test());
}

TEST_F(ACSTest, WatchdogTest_NoListeners)
{
	EXPECT_FALSE(acs_srv->Test());
}

TEST_F(ACSTest, AddListener_EndpointRegistry)
{
	using ::testing::AllOf;
	using ::testing::Contains;
	using ::testing::Field;

	boost::system::error_code ec;

	VS_SCOPE_EXIT{ net::endpoint::Remove(test_endpoint_name); };
	net::endpoint::Remove(test_endpoint_name);

	net::address_v4 local_addr_v4;
	net::address_v6 local_addr_v6;
	const auto interfaces = net::GetInterfaceInfo();
	for (const auto& ii : *interfaces)
	{
		for (const auto& i_addr: ii.addr_local_v4)
			if (local_addr_v4.is_unspecified() && !i_addr.is_unspecified())
				local_addr_v4 = i_addr;
		for (const auto& i_addr: ii.addr_local_v6)
			if (local_addr_v6.is_unspecified() && !i_addr.is_unspecified())
				local_addr_v6 = i_addr;
	}
	if (local_addr_v4.is_unspecified())
		std::cout << "Note: No local IPv4 address found, test will use loopback addresses.\n";
	if (local_addr_v6.is_unspecified())
		std::cout << "Note: No local IPv6 address found, test will use loopback addresses.\n";
	if (local_addr_v4.is_unspecified() || local_addr_v6.is_unspecified())
	{
		local_addr_v4 = net::address_v4::loopback();
		local_addr_v6 = net::address_v6::loopback();
	}

	EXPECT_EQ(1u, net::endpoint::AddAcceptTCP({ local_addr_v4.to_string(ec), test_port_1, net::endpoint::protocol_tcp }, test_endpoint_name));
	EXPECT_FALSE(ec) << ec.message();

	EXPECT_EQ(2u, net::endpoint::AddAcceptTCP({ local_addr_v6.to_string(ec), test_port_2, net::endpoint::protocol_tcp }, test_endpoint_name));
	EXPECT_FALSE(ec) << ec.message();

	EXPECT_EQ(1u, net::endpoint::AddAcceptUDP({ local_addr_v4.to_string(ec), test_port_2, "udp" }, test_endpoint_name));
	EXPECT_FALSE(ec) << ec.message();

	EXPECT_EQ(2u, net::endpoint::AddAcceptUDP({ local_addr_v6.to_string(ec), test_port_1, "udp" }, test_endpoint_name));
	EXPECT_FALSE(ec) << ec.message();

	EXPECT_EQ(4u, acs_srv->AddListeners(test_endpoint_name));

	{
		acs::Service::address_list listener_list;
		acs_srv->GetListenerList(listener_list, net::protocol::TCP);
		EXPECT_THAT(listener_list, Contains(AllOf(
			Field(&acs::Service::address_list::value_type::first, local_addr_v4),
			Field(&acs::Service::address_list::value_type::second, test_port_1)
		)));
		EXPECT_THAT(listener_list, Contains(AllOf(
			Field(&acs::Service::address_list::value_type::first, local_addr_v6),
			Field(&acs::Service::address_list::value_type::second, test_port_2)
		)));
	}
	{
		acs::Service::address_list listener_list;
		acs_srv->GetListenerList(listener_list, net::protocol::UDP);
		EXPECT_THAT(listener_list, Contains(AllOf(
			Field(&acs::Service::address_list::value_type::first, local_addr_v4),
			Field(&acs::Service::address_list::value_type::second, test_port_2)
		)));
		EXPECT_THAT(listener_list, Contains(AllOf(
			Field(&acs::Service::address_list::value_type::first, local_addr_v6),
			Field(&acs::Service::address_list::value_type::second, test_port_1)
		)));
	}
}

struct ipv4_tcp
{
	typedef net::address_v4 address;
	static const net::protocol protocol_id = net::protocol::TCP;
};
struct ipv6_tcp
{
	typedef net::address_v6 address;
	static const net::protocol protocol_id = net::protocol::TCP;
};
struct ipv4_udp
{
	typedef net::address_v4 address;
	static const net::protocol protocol_id = net::protocol::UDP;
};
struct ipv6_udp
{
	typedef net::address_v6 address;
	static const net::protocol protocol_id = net::protocol::UDP;
};

template <class Param>
class ACSTestT : public ACSTest {};
typedef ::testing::Types<ipv4_tcp, ipv6_tcp, ipv4_udp, ipv6_udp> ACSTestT_params;
TYPED_TEST_CASE(ACSTestT, ACSTestT_params);

TYPED_TEST(ACSTestT, AddListener_Any)
{
	boost::system::error_code ec;

	ec = this->acs_srv->AddListener(TypeParam::address::any(), test_port_1, TypeParam::protocol_id);
	EXPECT_FALSE(ec) << ec.message();

#if 0
	boost::system::error_code testsock_ec;
	typename TypeParam::protocol::socket testsock(g_asio_environment->IOService());
	typename TypeParam::protocol::endpoint ep(TypeParam::address::loopback(), test_port_1);
	testsock.connect(ep, testsock_ec);
	EXPECT_FALSE(testsock_ec) << "Conection to " << ep << " failed: " << testsock_ec.message();
#endif
}

TYPED_TEST(ACSTestT, AddListener_Fixed)
{
	boost::system::error_code ec;

	ec = this->acs_srv->AddListener(TypeParam::address::loopback(), test_port_1, TypeParam::protocol_id);
	EXPECT_FALSE(ec) << ec.message();

#if 0
	boost::system::error_code testsock_ec;
	typename TypeParam::protocol::socket testsock(g_asio_environment->IOService());
	typename TypeParam::protocol::endpoint ep(TypeParam::address::loopback(), test_port_1);
	testsock.connect(ep, testsock_ec);
	EXPECT_FALSE(testsock_ec) << "Conection to " << ep << " failed: " << testsock_ec.message();
#endif
}

TYPED_TEST(ACSTestT, AddListener_Duplicate)
{
	boost::system::error_code ec;

	ec = this->acs_srv->AddListener(TypeParam::address::any(), test_port_1, TypeParam::protocol_id);
	EXPECT_FALSE(ec) << ec.message();

	ec = this->acs_srv->AddListener(TypeParam::address::any(), test_port_1, TypeParam::protocol_id);
	EXPECT_EQ(acs::errc::listener_already_exists, ec) << ec.message();
}

TYPED_TEST(ACSTestT, RemoveListener_Any)
{
	boost::system::error_code ec;

	ec = this->acs_srv->AddListener(TypeParam::address::any(), test_port_1, TypeParam::protocol_id);
	EXPECT_FALSE(ec) << ec.message();

	ec = this->acs_srv->RemoveListener(TypeParam::address::any(), test_port_1, TypeParam::protocol_id);
	EXPECT_FALSE(ec) << ec.message();
}

TYPED_TEST(ACSTestT, RemoveListener_Fixed)
{
	boost::system::error_code ec;

	ec = this->acs_srv->AddListener(TypeParam::address::loopback(), test_port_1, TypeParam::protocol_id);
	EXPECT_FALSE(ec) << ec.message();

	ec = this->acs_srv->RemoveListener(TypeParam::address::loopback(), test_port_1, TypeParam::protocol_id);
	EXPECT_FALSE(ec) << ec.message();
}

TYPED_TEST(ACSTestT, RemoveListener_Nonexistent)
{
	boost::system::error_code ec;

	ec = this->acs_srv->RemoveListener(TypeParam::address::any(), test_port_1, TypeParam::protocol_id);
	EXPECT_EQ(acs::errc::listener_not_found, ec) << ec.message();
}

TYPED_TEST(ACSTestT, RemoveListeners)
{
	boost::system::error_code ec;

	ec = this->acs_srv->AddListener(TypeParam::address::loopback(), test_port_1, TypeParam::protocol_id);
	EXPECT_FALSE(ec) << ec.message();

	ec = this->acs_srv->AddListener(TypeParam::address::loopback(), test_port_2, TypeParam::protocol_id);
	EXPECT_FALSE(ec) << ec.message();

	auto n_removed = this->acs_srv->RemoveListeners([](const net::address& address, net::port, net::protocol) {
		return address == TypeParam::address::loopback();
	});
	EXPECT_EQ(2u, n_removed);
}

template <class Param>
class ACSTestTCP : public ACSTest {};
typedef ::testing::Types<ipv4_tcp, ipv6_tcp> ACSTestTCP_params;
TYPED_TEST_CASE(ACSTestTCP, ACSTestTCP_params);

TYPED_TEST(ACSTestTCP, Dispatch_Simple)
{
	using ::testing::_;
	using ::testing::Invoke;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Return;
	using ::testing::Sequence;

	boost::system::error_code ec;

	auto h1 = std::make_shared<ACSHandlerMock>();
	vs::event h1_done(true);
	Sequence h1_calls;
	EXPECT_CALL(*h1, Init(test_handler_name_1))
		.Times(1).InSequence(h1_calls);
	ec = this->acs_srv->AddHandler(test_handler_name_1, h1);
	EXPECT_FALSE(ec) << ec.message();

	ec = this->acs_srv->AddListener(TypeParam::address::any(), test_port_1, TypeParam::protocol_id);
	EXPECT_FALSE(ec) << ec.message();

	boost::system::error_code testsock_ec;
	boost::asio::ip::tcp::socket testsock(g_asio_environment->IOService());
	boost::asio::ip::tcp::endpoint ep(TypeParam::address::loopback(), test_port_1);
	testsock.connect(ep, testsock_ec);
	EXPECT_FALSE(testsock_ec) << "Conection to " << ep << " failed: " << testsock_ec.message();

	EXPECT_CALL(*h1, Protocol_TCP(SeqEq(test_data_inc[0]), 0))
		.Times(1).InSequence(h1_calls)
		.WillOnce(Return(acs::Response::accept_connection));
	EXPECT_CALL(*h1, Accept_TCP(_, SeqEq(test_data_inc[0])))
		.Times(1).InSequence(h1_calls)
		.WillOnce(InvokeWithoutArgs([&h1_done]() { h1_done.set(); }));
	{
		const auto& data = test_data[0];
		size_t written = boost::asio::write(testsock, boost::asio::buffer(data), testsock_ec);
		ASSERT_FALSE(testsock_ec) << "Write failed: " << testsock_ec.message();
		ASSERT_EQ(data.size(), written);
	}
	ASSERT_TRUE(h1_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h1_done.reset();

	ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(h1.get()));
}

TYPED_TEST(ACSTestTCP, Dispatch_ThreeSteps)
{
	using ::testing::_;
	using ::testing::Invoke;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Return;
	using ::testing::DoAll;
	using ::testing::Sequence;

	boost::system::error_code ec;

	auto h1 = std::make_shared<ACSHandlerMock>();
	vs::event h1_done(true);
	Sequence h1_calls;
	EXPECT_CALL(*h1, Init(test_handler_name_1))
		.Times(1).InSequence(h1_calls);
	ec = this->acs_srv->AddHandler(test_handler_name_1, h1);
	EXPECT_FALSE(ec) << ec.message();

	auto h2 = std::make_shared<ACSHandlerMock>();
	vs::event h2_done(true);
	Sequence h2_calls;
	EXPECT_CALL(*h2, Init(test_handler_name_2))
		.Times(1).InSequence(h2_calls);
	ec = this->acs_srv->AddHandler(test_handler_name_2, h2);
	EXPECT_FALSE(ec) << ec.message();

	ec = this->acs_srv->AddListener(TypeParam::address::any(), test_port_1, TypeParam::protocol_id);
	EXPECT_FALSE(ec) << ec.message();

	boost::system::error_code testsock_ec;
	boost::asio::ip::tcp::socket testsock(g_asio_environment->IOService());
	boost::asio::ip::tcp::endpoint ep(TypeParam::address::loopback(), test_port_1);
	testsock.connect(ep, testsock_ec);
	EXPECT_FALSE(testsock_ec) << "Conection to " << ep << " failed: " << testsock_ec.message();
#if 0
	testsock.set_option(boost::asio::ip::tcp::no_delay(true), testsock_ec);
	ASSERT_FALSE(testsock_ec) << "Set no_delay(true) failed: " << testsock_ec.message();
#endif

	// Send part 1
	EXPECT_CALL(*h1, Protocol_TCP(SeqEq(test_data_inc[0]), 0))
		.Times(1).InSequence(h1_calls)
		.WillOnce(DoAll(
			InvokeWithoutArgs([&h1_done]() { h1_done.set(); }),
			Return(acs::Response::next_step)
		));
	EXPECT_CALL(*h2, Protocol_TCP(SeqEq(test_data_inc[0]), 0))
		.Times(1).InSequence(h2_calls)
		.WillOnce(DoAll(
			InvokeWithoutArgs([&h2_done]() { h2_done.set(); }),
			Return(acs::Response::next_step)
		));
	{
		const auto& data = test_data[0];
		size_t written = boost::asio::write(testsock, boost::asio::buffer(data), testsock_ec);
		ASSERT_FALSE(testsock_ec) << "Write failed: " << testsock_ec.message();
		ASSERT_EQ(data.size(), written);
	}
	ASSERT_TRUE(h1_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h1_done.reset();
	ASSERT_TRUE(h2_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h2_done.reset();

	// Send part 2
	EXPECT_CALL(*h1, Protocol_TCP(SeqEq(test_data_inc[1]), 0))
		.Times(1).InSequence(h1_calls)
		.WillOnce(DoAll(
			InvokeWithoutArgs([&h1_done]() { h1_done.set(); }),
			Return(acs::Response::next_step)
		));
	EXPECT_CALL(*h2, Protocol_TCP(SeqEq(test_data_inc[1]), 0))
		.Times(1).InSequence(h2_calls)
		.WillOnce(DoAll(
			InvokeWithoutArgs([&h2_done]() { h2_done.set(); }),
			Return(acs::Response::my_connection)
		));
	{
		const auto& data = test_data[1];
		size_t written = boost::asio::write(testsock, boost::asio::buffer(data), testsock_ec);
		ASSERT_FALSE(testsock_ec) << "Write failed: " << testsock_ec.message();
		ASSERT_EQ(data.size(), written);
	}
	ASSERT_TRUE(h1_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h1_done.reset();
	ASSERT_TRUE(h2_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h2_done.reset();

	// Send part 3
	EXPECT_CALL(*h2, Protocol_TCP(SeqEq(test_data_inc[2]), 0))
		.Times(1).InSequence(h2_calls)
		.WillOnce(Return(acs::Response::accept_connection));
	EXPECT_CALL(*h2, Accept_TCP(_, SeqEq(test_data_inc[2])))
		.Times(1).InSequence(h2_calls)
		.WillOnce(InvokeWithoutArgs([&h2_done]() { h2_done.set(); }));
	{
		const auto& data = test_data[2];
		size_t written = boost::asio::write(testsock, boost::asio::buffer(data), testsock_ec);
		ASSERT_FALSE(testsock_ec) << "Write failed: " << testsock_ec.message();
		ASSERT_EQ(data.size(), written);
	}
	ASSERT_TRUE(h2_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h2_done.reset();

	ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(h1.get()));
	ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(h2.get()));
}

TYPED_TEST(ACSTestTCP, Dispatch_ChannelToken)
{
	using ::testing::_;
	using ::testing::Invoke;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Return;
	using ::testing::DoAll;
	using ::testing::Sequence;

	boost::system::error_code ec;

	auto h1 = std::make_shared<ACSHandlerMock>();
	vs::event h1_done(true);
	Sequence h1_calls;
	EXPECT_CALL(*h1, Init(test_handler_name_1))
		.Times(1).InSequence(h1_calls);
	ec = this->acs_srv->AddHandler(test_handler_name_1, h1);
	EXPECT_FALSE(ec) << ec.message();

	auto h2 = std::make_shared<ACSHandlerMock>();
	vs::event h2_done(true);
	Sequence h2_calls;
	EXPECT_CALL(*h2, Init(test_handler_name_2))
		.Times(1).InSequence(h2_calls);
	ec = this->acs_srv->AddHandler(test_handler_name_2, h2);
	EXPECT_FALSE(ec) << ec.message();

	ec = this->acs_srv->AddListener(TypeParam::address::any(), test_port_1, TypeParam::protocol_id, acs::ChannelToken<channel_1_tag>());
	EXPECT_FALSE(ec) << ec.message();

	boost::system::error_code testsock_ec;
	boost::asio::ip::tcp::socket testsock(g_asio_environment->IOService());
	boost::asio::ip::tcp::endpoint ep(TypeParam::address::loopback(), test_port_1);
	testsock.connect(ep, testsock_ec);
	EXPECT_FALSE(testsock_ec) << "Conection to " << ep << " failed: " << testsock_ec.message();
#if 0
	testsock.set_option(boost::asio::ip::tcp::no_delay(true), testsock_ec);
	ASSERT_FALSE(testsock_ec) << "Set no_delay(true) failed: " << testsock_ec.message();
#endif

	// Send part 1
	EXPECT_CALL(*h1, Protocol_TCP(SeqEq(test_data_inc[0]), acs::ChannelToken<channel_1_tag>()))
		.Times(1).InSequence(h1_calls)
		.WillOnce(DoAll(
			InvokeWithoutArgs([&h1_done]() { h1_done.set(); }),
			Return(acs::Response::next_step)
		));
	EXPECT_CALL(*h2, Protocol_TCP(SeqEq(test_data_inc[0]), acs::ChannelToken<channel_1_tag>()))
		.Times(1).InSequence(h2_calls)
		.WillOnce(DoAll(
			InvokeWithoutArgs([&h2_done]() { h2_done.set(); }),
			Return(acs::Response::not_my_connection)
		));
	{
		const auto& data = test_data[0];
		size_t written = boost::asio::write(testsock, boost::asio::buffer(data), testsock_ec);
		ASSERT_FALSE(testsock_ec) << "Write failed: " << testsock_ec.message();
		ASSERT_EQ(data.size(), written);
	}
	ASSERT_TRUE(h1_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h1_done.reset();
	ASSERT_TRUE(h2_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h2_done.reset();

	// Send part 2
	EXPECT_CALL(*h1, Protocol_TCP(SeqEq(test_data_inc[1]), acs::ChannelToken<channel_1_tag>()))
		.Times(1).InSequence(h1_calls)
		.WillOnce(Return(acs::Response::accept_connection));
	EXPECT_CALL(*h1, Accept_TCP(_, SeqEq(test_data_inc[1])))
		.Times(1).InSequence(h1_calls)
		.WillOnce(InvokeWithoutArgs([&h1_done]() { h1_done.set(); }));
	{
		const auto& data = test_data[1];
		size_t written = boost::asio::write(testsock, boost::asio::buffer(data), testsock_ec);
		ASSERT_FALSE(testsock_ec) << "Write failed: " << testsock_ec.message();
		ASSERT_EQ(data.size(), written);
	}
	ASSERT_TRUE(h1_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h1_done.reset();

	ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(h1.get()));
	ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(h2.get()));
}

template <class Param>
class ACSTestUDP : public ACSTest {};
typedef ::testing::Types<ipv4_udp, ipv6_udp> ACSTestUDP_params;
TYPED_TEST_CASE(ACSTestUDP, ACSTestUDP_params);

TYPED_TEST(ACSTestUDP, Dispatch_Simple)
{
	using ::testing::_;
	using ::testing::Invoke;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Property;
	using ::testing::Return;
	using ::testing::Sequence;

	boost::system::error_code ec;

	auto h1 = std::make_shared<ACSHandlerMock>();
	vs::event h1_done(true);
	Sequence h1_calls;
	EXPECT_CALL(*h1, Init(test_handler_name_1))
		.Times(1).InSequence(h1_calls);
	ec = this->acs_srv->AddHandler(test_handler_name_1, h1);
	EXPECT_FALSE(ec) << ec.message();

	ec = this->acs_srv->AddListener(TypeParam::address::any(), test_port_1, TypeParam::protocol_id);
	EXPECT_FALSE(ec) << ec.message();

	boost::system::error_code testsock_ec;
	boost::asio::ip::udp::socket testsock(g_asio_environment->IOService());
	boost::asio::ip::udp::endpoint ep(TypeParam::address::loopback(), test_port_1);
	testsock.open(ep.protocol(), testsock_ec);
	EXPECT_FALSE(testsock_ec) << "Open failed: " << testsock_ec.message();

	EXPECT_CALL(*h1, Protocol_UDP(Property(&acs::Handler::packet_buffer::Back, SeqEq(test_data[0])), 0))
		.Times(1).InSequence(h1_calls)
		.WillOnce(Return(acs::Response::accept_connection));
	EXPECT_CALL(*h1, Accept_UDP(_, Property(&acs::Handler::packet_buffer::Back, SeqEq(test_data[0]))))
		.Times(1).InSequence(h1_calls)
		.WillOnce(InvokeWithoutArgs([&h1_done]() { h1_done.set(); }));
	{
		const auto& data = test_data[0];
		size_t written = testsock.send_to(boost::asio::buffer(data), ep, 0, testsock_ec);
		ASSERT_FALSE(testsock_ec) << "Write to " << ep << " failed: " << testsock_ec.message();
		ASSERT_EQ(data.size(), written);
	}
	ASSERT_TRUE(h1_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h1_done.reset();

	ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(h1.get()));
}

TYPED_TEST(ACSTestUDP, Dispatch_ThreeSteps)
{
	using ::testing::_;
	using ::testing::DoAll;
	using ::testing::ElementsAre;
	using ::testing::Invoke;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Property;
	using ::testing::Return;
	using ::testing::Sequence;

	boost::system::error_code ec;

	auto h1 = std::make_shared<ACSHandlerMock>();
	vs::event h1_done(true);
	Sequence h1_calls;
	EXPECT_CALL(*h1, Init(test_handler_name_1))
		.Times(1).InSequence(h1_calls);
	ec = this->acs_srv->AddHandler(test_handler_name_1, h1);
	EXPECT_FALSE(ec) << ec.message();

	auto h2 = std::make_shared<ACSHandlerMock>();
	vs::event h2_done(true);
	Sequence h2_calls;
	EXPECT_CALL(*h2, Init(test_handler_name_2))
		.Times(1).InSequence(h2_calls);
	ec = this->acs_srv->AddHandler(test_handler_name_2, h2);
	EXPECT_FALSE(ec) << ec.message();

	ec = this->acs_srv->AddListener(TypeParam::address::any(), test_port_1, TypeParam::protocol_id);
	EXPECT_FALSE(ec) << ec.message();

	boost::system::error_code testsock_ec;
	boost::asio::ip::udp::socket testsock(g_asio_environment->IOService());
	boost::asio::ip::udp::endpoint ep(TypeParam::address::loopback(), test_port_1);
	testsock.open(ep.protocol(), testsock_ec);
	EXPECT_FALSE(testsock_ec) << "Open failed: " << testsock_ec.message();

	// Send part 1
	EXPECT_CALL(*h1, Protocol_UDP(Property(&acs::Handler::packet_buffer::Back, SeqEq(test_data[0])), 0))
		.Times(1).InSequence(h1_calls)
		.WillOnce(DoAll(
			InvokeWithoutArgs([&h1_done]() { h1_done.set(); }),
			Return(acs::Response::next_step)
		));
	EXPECT_CALL(*h2, Protocol_UDP(Property(&acs::Handler::packet_buffer::Back, SeqEq(test_data[0])), 0))
		.Times(1).InSequence(h2_calls)
		.WillOnce(DoAll(
			InvokeWithoutArgs([&h2_done]() { h2_done.set(); }),
			Return(acs::Response::next_step)
		));
	{
		const auto& data = test_data[0];
		size_t written = testsock.send_to(boost::asio::buffer(data), ep, 0, testsock_ec);
		ASSERT_FALSE(testsock_ec) << "Write to " << ep << " failed: " << testsock_ec.message();
		ASSERT_EQ(data.size(), written);
	}
	ASSERT_TRUE(h1_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h1_done.reset();
	ASSERT_TRUE(h2_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h2_done.reset();

	// Send part 2
	EXPECT_CALL(*h1, Protocol_UDP(Property(&acs::Handler::packet_buffer::Back, SeqEq(test_data[1])), 0))
		.Times(1).InSequence(h1_calls)
		.WillOnce(DoAll(
			InvokeWithoutArgs([&h1_done]() { h1_done.set(); }),
			Return(acs::Response::next_step)
		));
	EXPECT_CALL(*h2, Protocol_UDP(Property(&acs::Handler::packet_buffer::Back, SeqEq(test_data[1])), 0))
		.Times(1).InSequence(h2_calls)
		.WillOnce(DoAll(
			InvokeWithoutArgs([&h2_done]() { h2_done.set(); }),
			Return(acs::Response::my_connection)
		));
	{
		const auto& data = test_data[1];
		size_t written = testsock.send_to(boost::asio::buffer(data), ep, 0, testsock_ec);
		ASSERT_FALSE(testsock_ec) << "Write to " << ep << " failed: " << testsock_ec.message();
		ASSERT_EQ(data.size(), written);
	}
	ASSERT_TRUE(h1_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h1_done.reset();
	ASSERT_TRUE(h2_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h2_done.reset();

	// Send part 3
	EXPECT_CALL(*h2, Protocol_UDP(Property(&acs::Handler::packet_buffer::Back, SeqEq(test_data[2])), 0))
		.Times(1).InSequence(h2_calls)
		.WillOnce(Return(acs::Response::accept_connection));
	EXPECT_CALL(*h2, Accept_UDP(_, ElementsAre(
			SeqEq(test_data[0]),
			SeqEq(test_data[1]),
			SeqEq(test_data[2])
	)))
		.Times(1).InSequence(h2_calls)
		.WillOnce(InvokeWithoutArgs([&h2_done]() { h2_done.set(); }));
	{
		const auto& data = test_data[2];
		size_t written = testsock.send_to(boost::asio::buffer(data), ep, 0, testsock_ec);
		ASSERT_FALSE(testsock_ec) << "Write to " << ep << " failed: " << testsock_ec.message();
		ASSERT_EQ(data.size(), written);
	}
	ASSERT_TRUE(h2_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h2_done.reset();

	ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(h1.get()));
	ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(h2.get()));
}

TYPED_TEST(ACSTestUDP, Dispatch_ChannelToken)
{
	using ::testing::_;
	using ::testing::Invoke;
	using ::testing::ElementsAre;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Return;
	using ::testing::DoAll;
	using ::testing::Sequence;

	boost::system::error_code ec;

	auto h1 = std::make_shared<ACSHandlerMock>();
	vs::event h1_done(true);
	Sequence h1_calls;
	EXPECT_CALL(*h1, Init(test_handler_name_1))
		.Times(1).InSequence(h1_calls);
	ec = this->acs_srv->AddHandler(test_handler_name_1, h1);
	EXPECT_FALSE(ec) << ec.message();

	auto h2 = std::make_shared<ACSHandlerMock>();
	vs::event h2_done(true);
	Sequence h2_calls;
	EXPECT_CALL(*h2, Init(test_handler_name_2))
		.Times(1).InSequence(h2_calls);
	ec = this->acs_srv->AddHandler(test_handler_name_2, h2);
	EXPECT_FALSE(ec) << ec.message();

	ec = this->acs_srv->AddListener(TypeParam::address::any(), test_port_1, TypeParam::protocol_id, acs::ChannelToken<channel_1_tag>());
	EXPECT_FALSE(ec) << ec.message();

	boost::system::error_code testsock_ec;
	boost::asio::ip::udp::socket testsock(g_asio_environment->IOService());
	boost::asio::ip::udp::endpoint ep(TypeParam::address::loopback(), test_port_1);
	testsock.open(ep.protocol(), testsock_ec);
	EXPECT_FALSE(testsock_ec) << "Open failed: " << testsock_ec.message();

	// Send part 1
	EXPECT_CALL(*h1, Protocol_UDP(Property(&acs::Handler::packet_buffer::Back, SeqEq(test_data[0])), acs::ChannelToken<channel_1_tag>()))
		.Times(1).InSequence(h1_calls)
		.WillOnce(DoAll(
			InvokeWithoutArgs([&h1_done]() { h1_done.set(); }),
			Return(acs::Response::next_step)
		));
	EXPECT_CALL(*h2, Protocol_UDP(Property(&acs::Handler::packet_buffer::Back, SeqEq(test_data[0])), acs::ChannelToken<channel_1_tag>()))
		.Times(1).InSequence(h2_calls)
		.WillOnce(DoAll(
			InvokeWithoutArgs([&h2_done]() { h2_done.set(); }),
			Return(acs::Response::not_my_connection)
		));
	{
		const auto& data = test_data[0];
		size_t written = testsock.send_to(boost::asio::buffer(data), ep, 0, testsock_ec);
		ASSERT_FALSE(testsock_ec) << "Write to " << ep << " failed: " << testsock_ec.message();
		ASSERT_EQ(data.size(), written);
	}
	ASSERT_TRUE(h1_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h1_done.reset();
	ASSERT_TRUE(h2_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h2_done.reset();

	// Send part 2
	EXPECT_CALL(*h1, Protocol_UDP(Property(&acs::Handler::packet_buffer::Back, SeqEq(test_data[1])), acs::ChannelToken<channel_1_tag>()))
		.Times(1).InSequence(h1_calls)
		.WillOnce(Return(acs::Response::accept_connection));
	EXPECT_CALL(*h1, Accept_UDP(_, ElementsAre(
			SeqEq(test_data[0]),
			SeqEq(test_data[1])
	)))
		.Times(1).InSequence(h1_calls)
		.WillOnce(InvokeWithoutArgs([&h1_done]() { h1_done.set(); }));
	{
		const auto& data = test_data[1];
		size_t written = testsock.send_to(boost::asio::buffer(data), ep, 0, testsock_ec);
		ASSERT_FALSE(testsock_ec) << "Write to " << ep << " failed: " << testsock_ec.message();
		ASSERT_EQ(data.size(), written);
	}
	ASSERT_TRUE(h1_done.wait_for(std::chrono::seconds(1))) << "Asynchronous operations took too long";
	h1_done.reset();

	ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(h1.get()));
	ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(h2.get()));
}

}
