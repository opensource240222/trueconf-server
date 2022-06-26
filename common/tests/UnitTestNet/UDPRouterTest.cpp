#include "net/Address.h"
#include "net/Error.h"
#include "net/Port.h"
#include "net/UDPRouter.h"
#include "std/cpplib/event.h"
#include "tests/common/ASIOEnvironment.h"
#include "tests/common/TestHelpers.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "std-generic/compat/iterator.h"
#include <atomic>
#include <string>
#include <vector>

#if !defined(__has_feature)
#	define __has_feature(x) 0
#endif

namespace net_ur_test {

static const size_t c_read_buffer_size = 32;
static const std::string c_data_1 = "First packet";
static const std::string c_data_2 = "Second packet";
static const std::string c_data_3 = "Packet #3";
static const std::string c_data_4 = "A copy of 'First packet'";
static const std::string c_data_5 = "A copy of 'Second packet'";
static const std::string c_data_6 = "A copy of 'Packet #3'";
static const std::string c_data_large = "This is a packet that is larger than receve buffer size";

static std::atomic<unsigned> g_test_id {0};
static net::port GetListenPort()
{
	return 50000 + g_test_id.fetch_add(1, std::memory_order_relaxed) % 100
#if   defined(__clang__) && defined(__i386__)
	+ 1000
#elif defined(__clang__) && defined(__x86_64__)
	+ 2000
#elif defined(_MSC_VER) && defined(_M_IX86)
	+ 3000
#elif defined(_MSC_VER) && defined(_M_X64)
	+ 4000
#elif defined(__GNUC__) && defined(__i386__)
	+ 5000
#elif defined(__GNUC__) && defined(__x86_64__)
	+ 6000
#endif
#if defined(_DEBUG)
	+ 100
#elif __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
	+ 200
#elif __has_feature(memory_sanitizer)
	+ 300
#elif __has_feature(thread_sanitizer) || defined(__SANITIZE_THREAD__)
	+ 400
#endif
	;
}

using Protocol = boost::asio::ip::udp;
using Router = net::ur::Router<Protocol>;
using RouterPtr = net::ur::RouterPtr<Protocol>;
using Connection = net::ur::Connection<Protocol>;

using ::testing::Pair;
using ::testing::UnorderedElementsAre;

struct AcceptState
{
	vs::event done {false};
	boost::system::error_code ec;
	Connection connection;
};

struct ReadState
{
	vs::event done {false};
	boost::system::error_code ec;
	size_t bytes_transferred {0};
	char buffer[c_read_buffer_size];
};

struct WriteState
{
	vs::event done {false};
	boost::system::error_code ec;
	size_t bytes_transferred {0};
};

struct ClientReadState
{
	vs::event done {false};
	boost::system::error_code ec;
	Protocol::endpoint ep;
	size_t bytes_transferred {0};
	char buffer[c_read_buffer_size];
};

struct UDPRouterTest : public ::testing::Test
{
	void SetUp() override
	{
		boost::system::error_code ec;
		router = Router::Get(Protocol::endpoint(Protocol::v4(), GetListenPort()), g_asio_environment->IOService(), ec);
		ASSERT_FALSE(ec) << ec.message();
		ASSERT_NE(router, nullptr);
	}

	void TearDown() override
	{
		if (router)
			router->CloseWhenUnused();
	}

	Protocol::endpoint GetRouterConnectEndpoint() const
	{
		return Protocol::endpoint(net::address_v4::loopback(), router->LocalEndpoint().port());
	}

	std::shared_ptr<AcceptState> SyncAccept()
	{
		auto as = std::make_shared<AcceptState>();
		router->AsyncAccept([as](const boost::system::error_code& ec, Connection connection) {
			as->ec = ec;
			as->connection = std::move(connection);
			as->done.set();
		});
		EXPECT_TRUE(test::WaitFor("Accept handler", as->done));
		EXPECT_FALSE(as->ec) << as->ec.message();
		return as;
	}

	std::shared_ptr<ReadState> StartRead(Connection& connection)
	{
		auto rs = std::make_shared<ReadState>();
		connection.async_receive(boost::asio::buffer(rs->buffer), [rs](const boost::system::error_code& ec, std::size_t bytes_transferred) {
			rs->ec = ec;
			rs->bytes_transferred = bytes_transferred;
			rs->done.set();
		});
		return rs;
	}

	void WaitRead(const ReadState& rs, bool check_success = true)
	{
		EXPECT_TRUE(test::WaitFor("Read handler", rs.done));
		if (check_success)
		{
			EXPECT_FALSE(rs.ec) << rs.ec.message();
		}
	}

	template <class ConstBufferSequence>
	void CheckedWrite(Connection& connection, const ConstBufferSequence& buffers)
	{
		auto ws = std::make_shared<WriteState>();
		connection.async_send(buffers, 0, [ws](const boost::system::error_code& ec, std::size_t bytes_transferred) {
			ws->ec = ec;
			ws->bytes_transferred = bytes_transferred;
			ws->done.set();
		});
		EXPECT_TRUE(test::WaitFor("Write handler", ws->done));
		EXPECT_FALSE(ws->ec) << ws->ec.message();
		EXPECT_EQ(ws->bytes_transferred, boost::asio::buffer_size(buffers));
	}

	std::shared_ptr<ClientReadState> SyncRead(Protocol::socket& socket)
	{
		auto crs = std::make_shared<ClientReadState>();
		socket.async_receive_from(boost::asio::buffer(crs->buffer), crs->ep, [crs](const boost::system::error_code& ec, std::size_t bytes_transferred) {
			crs->ec = ec;
			crs->bytes_transferred = bytes_transferred;
			crs->done.set();
		});
		EXPECT_TRUE(test::WaitFor("Client read handler", crs->done));
		EXPECT_FALSE(crs->ec) << crs->ec.message();
		return crs;
	}

	template <class... RS>
	std::vector<std::pair<std::string, boost::system::error_code>> ReadResults(const RS&... rs)
	{
		std::vector<std::pair<std::string, boost::system::error_code>> results;
		results.reserve(sizeof...(rs));
		(void)std::initializer_list<int> {
			(results.emplace_back(std::string(rs.buffer, rs.bytes_transferred), rs.ec), 0)...
		};
		return results;
	};

	RouterPtr router;
};

TEST_F(UDPRouterTest, GetExisting)
{
	auto router_2 = Router::Get(router->LocalEndpoint());
	EXPECT_EQ(router_2, router);
}

TEST_F(UDPRouterTest, OpenExisting)
{
	boost::system::error_code ec;
	auto router_2 = Router::Get(router->LocalEndpoint(), g_asio_environment->IOService(), ec);
	EXPECT_EQ(ec, boost::asio::error::already_open);
	EXPECT_EQ(router_2, router);
}

TEST_F(UDPRouterTest, AsyncAccept_SendThenAccept)
{
	boost::system::error_code ec;
	Protocol::socket cl_socket(g_asio_environment->IOService());
	cl_socket.open(Protocol::v4(), ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket.non_blocking(true, ec);
	EXPECT_FALSE(ec) << ec.message();
	EXPECT_EQ(cl_socket.send_to(boost::asio::buffer(c_data_1), GetRouterConnectEndpoint()), vs::size(c_data_1));

	auto as = SyncAccept();
	ASSERT_TRUE(as->connection.is_open());
	EXPECT_EQ(as->connection.remote_endpoint().port(), cl_socket.local_endpoint().port());
}

TEST_F(UDPRouterTest, AsyncAccept_AcceptThenSend)
{
	auto as = std::make_shared<AcceptState>();
	router->AsyncAccept([as](const boost::system::error_code& ec, Connection connection) {
		as->ec = ec;
		as->connection = std::move(connection);
		as->done.set();
	});

	boost::system::error_code ec;
	Protocol::socket cl_socket(g_asio_environment->IOService());
	cl_socket.open(Protocol::v4(), ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket.non_blocking(true, ec);
	EXPECT_FALSE(ec) << ec.message();
	EXPECT_EQ(cl_socket.send_to(boost::asio::buffer(c_data_1), GetRouterConnectEndpoint()), vs::size(c_data_1));

	EXPECT_TRUE(test::WaitFor("Accept handler", as->done));
	EXPECT_FALSE(as->ec) << as->ec.message();
	ASSERT_TRUE(as->connection.is_open());
	EXPECT_EQ(as->connection.remote_endpoint().port(), cl_socket.local_endpoint().port());
}

TEST_F(UDPRouterTest, TryAccept)
{
	boost::system::error_code ec;
	Protocol::socket cl_socket(g_asio_environment->IOService());
	cl_socket.open(Protocol::v4(), ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket.non_blocking(true, ec);
	EXPECT_FALSE(ec) << ec.message();
	EXPECT_EQ(cl_socket.send_to(boost::asio::buffer(c_data_1), GetRouterConnectEndpoint()), vs::size(c_data_1));

	Connection connection;
	EXPECT_TRUE(test::WaitFor("TryAccept", [&]() {
		connection = router->TryAccept();
		return connection.is_open();
	}));
	ASSERT_TRUE(connection.is_open());
	EXPECT_EQ(connection.remote_endpoint().port(), cl_socket.local_endpoint().port());
}

TEST_F(UDPRouterTest, Connect)
{
	boost::system::error_code ec;
	Protocol::socket cl_socket(g_asio_environment->IOService());
	cl_socket.open(Protocol::v4(), ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket.non_blocking(true, ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket.bind(Protocol::endpoint(Protocol::v4(), 0), ec);
	EXPECT_FALSE(ec) << ec.message();

	auto connection = router->Connect(Protocol::endpoint(net::address_v4::loopback(), cl_socket.local_endpoint().port()), ec);
	EXPECT_FALSE(ec) << ec.message();
	ASSERT_TRUE(connection.is_open());
	connection.async_send(boost::asio::buffer(c_data_1), 0, [](const boost::system::error_code&, std::size_t) {});

	auto crs = SyncRead(cl_socket);
	EXPECT_EQ(crs->ep.port(), connection.local_endpoint().port());
}

TEST_F(UDPRouterTest, Connect_ConnectionExists)
{
	boost::system::error_code ec;
	Protocol::socket cl_socket(g_asio_environment->IOService());
	cl_socket.open(Protocol::v4(), ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket.non_blocking(true, ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket.bind(Protocol::endpoint(Protocol::v4(), 0), ec);
	EXPECT_FALSE(ec) << ec.message();

	auto connection_1 = router->Connect(Protocol::endpoint(net::address_v4::loopback(), cl_socket.local_endpoint().port()), ec);
	EXPECT_FALSE(ec) << ec.message();
	ASSERT_TRUE(connection_1.is_open());

	auto connection_2 = router->Connect(Protocol::endpoint(net::address_v4::loopback(), cl_socket.local_endpoint().port()), ec);
	EXPECT_EQ(ec, boost::asio::error::already_open);
	ASSERT_FALSE(connection_2.is_open());
}

TEST_F(UDPRouterTest, ReadFromAccepted)
{
	boost::system::error_code ec;
	Protocol::socket cl_socket(g_asio_environment->IOService());
	cl_socket.open(Protocol::v4(), ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket.non_blocking(true, ec);
	EXPECT_FALSE(ec) << ec.message();
	EXPECT_EQ(cl_socket.send_to(boost::asio::buffer(c_data_1), GetRouterConnectEndpoint()), vs::size(c_data_1));

	auto as = SyncAccept();
	ASSERT_TRUE(as->connection.is_open());

	// Read already received packet
	auto rs_1 = StartRead(as->connection);
	WaitRead(*rs_1);
	// Multiple unfinished reads
	auto rs_2 = StartRead(as->connection);
	auto rs_3 = StartRead(as->connection);
	EXPECT_EQ(cl_socket.send_to(boost::asio::buffer(c_data_2), GetRouterConnectEndpoint()), vs::size(c_data_2));
	EXPECT_EQ(cl_socket.send_to(boost::asio::buffer(c_data_3), GetRouterConnectEndpoint()), vs::size(c_data_3));
	WaitRead(*rs_2);
	WaitRead(*rs_3);

	const auto reads = ReadResults(*rs_1, *rs_2, *rs_3);
	EXPECT_THAT(reads, UnorderedElementsAre(
		Pair(c_data_1, boost::system::error_code()),
		Pair(c_data_2, boost::system::error_code()),
		Pair(c_data_3, boost::system::error_code())
	));
}

TEST_F(UDPRouterTest, ReadFromConnected)
{
	boost::system::error_code ec;
	Protocol::socket cl_socket(g_asio_environment->IOService());
	cl_socket.open(Protocol::v4(), ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket.non_blocking(true, ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket.bind(Protocol::endpoint(Protocol::v4(), 0), ec);
	EXPECT_FALSE(ec) << ec.message();

	auto connection = router->Connect(Protocol::endpoint(net::address_v4::loopback(), cl_socket.local_endpoint().port()), ec);
	EXPECT_FALSE(ec) << ec.message();
	ASSERT_TRUE(connection.is_open());
	connection.async_send(boost::asio::buffer(c_data_1), 0, [](const boost::system::error_code&, std::size_t) {});

	EXPECT_EQ(cl_socket.send_to(boost::asio::buffer(c_data_1), GetRouterConnectEndpoint()), vs::size(c_data_1));
	// Read already received packet
	auto rs_1 = StartRead(connection);
	WaitRead(*rs_1);
	// Multiple unfinished reads
	auto rs_2 = StartRead(connection);
	auto rs_3 = StartRead(connection);
	EXPECT_EQ(cl_socket.send_to(boost::asio::buffer(c_data_2), GetRouterConnectEndpoint()), vs::size(c_data_2));
	EXPECT_EQ(cl_socket.send_to(boost::asio::buffer(c_data_3), GetRouterConnectEndpoint()), vs::size(c_data_3));
	WaitRead(*rs_2);
	WaitRead(*rs_3);

	const auto reads = ReadResults(*rs_1, *rs_2, *rs_3);
	EXPECT_THAT(reads, UnorderedElementsAre(
		Pair(c_data_1, boost::system::error_code()),
		Pair(c_data_2, boost::system::error_code()),
		Pair(c_data_3, boost::system::error_code())
	));
}

TEST_F(UDPRouterTest, Read_Routing)
{
	boost::system::error_code ec;
	Protocol::socket cl_socket_1(g_asio_environment->IOService());
	cl_socket_1.open(Protocol::v4(), ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket_1.non_blocking(true, ec);
	EXPECT_FALSE(ec) << ec.message();
	EXPECT_EQ(cl_socket_1.send_to(boost::asio::buffer(c_data_1), GetRouterConnectEndpoint()), vs::size(c_data_1));

	Protocol::socket cl_socket_2(g_asio_environment->IOService());
	cl_socket_2.open(Protocol::v4(), ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket_2.non_blocking(true, ec);
	EXPECT_FALSE(ec) << ec.message();
	EXPECT_EQ(cl_socket_2.send_to(boost::asio::buffer(c_data_4), GetRouterConnectEndpoint()), vs::size(c_data_4));

	auto as_1 = SyncAccept();
	ASSERT_TRUE(as_1->connection.is_open());

	auto as_2 = SyncAccept();
	ASSERT_TRUE(as_2->connection.is_open());

	// Fix connection order (as_1->connection isn't guarantied to be connected to cl_socket_1)
	if (as_1->connection.remote_endpoint().port() != cl_socket_1.local_endpoint().port())
		std::swap(as_1, as_2);

	// Do reads
	auto rs_1_1 = StartRead(as_1->connection);
	EXPECT_EQ(cl_socket_2.send_to(boost::asio::buffer(c_data_5), GetRouterConnectEndpoint()), vs::size(c_data_5));
	WaitRead(*rs_1_1);
	auto rs_2_1 = StartRead(as_2->connection);
	auto rs_1_2 = StartRead(as_1->connection);
	auto rs_2_2 = StartRead(as_2->connection);
	WaitRead(*rs_2_1);
	auto rs_1_3 = StartRead(as_1->connection);
	EXPECT_EQ(cl_socket_2.send_to(boost::asio::buffer(c_data_6), GetRouterConnectEndpoint()), vs::size(c_data_6));
	WaitRead(*rs_2_2);
	auto rs_2_3 = StartRead(as_2->connection);
	EXPECT_EQ(cl_socket_1.send_to(boost::asio::buffer(c_data_2), GetRouterConnectEndpoint()), vs::size(c_data_2));
	EXPECT_EQ(cl_socket_1.send_to(boost::asio::buffer(c_data_3), GetRouterConnectEndpoint()), vs::size(c_data_3));
	WaitRead(*rs_1_2);
	WaitRead(*rs_1_3);
	WaitRead(*rs_2_3);

	const auto reads_1 = ReadResults(*rs_1_1, *rs_1_2, *rs_1_3);
	EXPECT_THAT(reads_1, UnorderedElementsAre(
		Pair(c_data_1, boost::system::error_code()),
		Pair(c_data_2, boost::system::error_code()),
		Pair(c_data_3, boost::system::error_code())
	));
	const auto reads_2 = ReadResults(*rs_2_1, *rs_2_2, *rs_2_3);
	EXPECT_THAT(reads_2, UnorderedElementsAre(
		Pair(c_data_4, boost::system::error_code()),
		Pair(c_data_5, boost::system::error_code()),
		Pair(c_data_6, boost::system::error_code())
	));
}

TEST_F(UDPRouterTest, Read_TruncatedPacket)
{
	router->SetMaxPacketSize(c_read_buffer_size);

	boost::system::error_code ec;
	Protocol::socket cl_socket(g_asio_environment->IOService());
	cl_socket.open(Protocol::v4(), ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket.non_blocking(true, ec);
	EXPECT_FALSE(ec) << ec.message();
	EXPECT_EQ(cl_socket.send_to(boost::asio::buffer(c_data_1), GetRouterConnectEndpoint()), vs::size(c_data_1));

	auto as = SyncAccept();
	ASSERT_TRUE(as->connection.is_open());

	// Read already received packet
	auto rs_1 = StartRead(as->connection);
	WaitRead(*rs_1);
	// Multiple unfinished reads
	auto rs_2 = StartRead(as->connection);
	auto rs_3 = StartRead(as->connection);
	EXPECT_EQ(cl_socket.send_to(boost::asio::buffer(c_data_large), GetRouterConnectEndpoint()), vs::size(c_data_large));
	EXPECT_EQ(cl_socket.send_to(boost::asio::buffer(c_data_3), GetRouterConnectEndpoint()), vs::size(c_data_3));
	WaitRead(*rs_2, false);
	WaitRead(*rs_3, false);

	const auto reads = ReadResults(*rs_1, *rs_2, *rs_3);
	EXPECT_THAT(reads, UnorderedElementsAre(
		Pair(c_data_1, boost::system::error_code()),
		Pair(c_data_large.substr(0, c_read_buffer_size), net::errc::packet_truncated),
		Pair(c_data_3, boost::system::error_code())
	));
}

TEST_F(UDPRouterTest, WriteToAccepted)
{
	boost::system::error_code ec;
	Protocol::socket cl_socket(g_asio_environment->IOService());
	cl_socket.open(Protocol::v4(), ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket.non_blocking(true, ec);
	EXPECT_FALSE(ec) << ec.message();
	EXPECT_EQ(cl_socket.send_to(boost::asio::buffer(c_data_1), GetRouterConnectEndpoint()), vs::size(c_data_1));

	auto as = SyncAccept();
	ASSERT_TRUE(as->connection.is_open());

	CheckedWrite(as->connection, boost::asio::buffer(c_data_1));
	CheckedWrite(as->connection, boost::asio::buffer(c_data_2));
	auto crs_1 = SyncRead(cl_socket);
	CheckedWrite(as->connection, boost::asio::buffer(c_data_3));
	auto crs_2 = SyncRead(cl_socket);
	auto crs_3 = SyncRead(cl_socket);

	const auto reads = ReadResults(*crs_1, *crs_2, *crs_3);
	EXPECT_THAT(reads, UnorderedElementsAre(
		Pair(c_data_1, boost::system::error_code()),
		Pair(c_data_2, boost::system::error_code()),
		Pair(c_data_3, boost::system::error_code())
	));
}

TEST_F(UDPRouterTest, WriteToConnected)
{
	boost::system::error_code ec;
	Protocol::socket cl_socket(g_asio_environment->IOService());
	cl_socket.open(Protocol::v4(), ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket.non_blocking(true, ec);
	EXPECT_FALSE(ec) << ec.message();
	cl_socket.bind(Protocol::endpoint(Protocol::v4(), 0), ec);
	EXPECT_FALSE(ec) << ec.message();

	auto connection = router->Connect(Protocol::endpoint(net::address_v4::loopback(), cl_socket.local_endpoint().port()), ec);
	EXPECT_FALSE(ec) << ec.message();
	ASSERT_TRUE(connection.is_open());

	CheckedWrite(connection, boost::asio::buffer(c_data_1));
	CheckedWrite(connection, boost::asio::buffer(c_data_2));
	auto crs_1 = SyncRead(cl_socket);
	CheckedWrite(connection, boost::asio::buffer(c_data_3));
	auto crs_2 = SyncRead(cl_socket);
	auto crs_3 = SyncRead(cl_socket);

	const auto reads = ReadResults(*crs_1, *crs_2, *crs_3);
	EXPECT_THAT(reads, UnorderedElementsAre(
		Pair(c_data_1, boost::system::error_code()),
		Pair(c_data_2, boost::system::error_code()),
		Pair(c_data_3, boost::system::error_code())
	));
}

}
