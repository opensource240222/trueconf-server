#include "Data.h"
#include "tests/common/ASIOEnvironment.h"
#include "tests/mocks/asio_protocol_mock.h"
#include "tests/mocks/asio_socket_mock.h"
#include "newtransport/Lib/TransportUtils.h"
#include "newtransport/Router/Connection_impl.h"
#include "net/BufferedConnection_impl.h"
#include "std/cpplib/event.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace transport_test {

TEST(GetServerNameByAddress, Success)
{
	using ::testing::_;
	using ::testing::AllOf;
	using ::testing::AnyNumber;
	using ::testing::Expectation;
	using ::testing::InvokeWithoutArgs;

	using protocol = test::asio::tcp_mock;
	using socket = protocol::socket;

	const protocol::endpoint ep(boost::asio::ip::address_v4(0x18273645), 4242);

	socket::impl::on_new([&](socket::impl*) {}); // Skip uninteresting socket from transport::Connection
	socket::impl::on_new([&](socket::impl* s) {
		Expectation exp_connect = EXPECT_CALL(*s, async_connect(ep))
			.Times(1)
			.WillOnce(InvokeWithoutArgs([s]() {
				s->is_open_ = true;
				s->complete_connect();
			}));
		Expectation exp_read_reply = EXPECT_CALL(*s, async_receive(_))
			.Times(1).After(exp_connect)
			.WillOnce(InvokeWithoutArgs([s]() {
				boost::asio::buffer_copy(s->read_buffers_, boost::asio::buffer(serialized_handshake_reply));
				s->complete_read(sizeof(serialized_handshake_reply));
			}));
		EXPECT_CALL(*s, close(_)).Times(1);
		// Ignore subsequent reads/writes
		EXPECT_CALL(*s, async_receive(_))
			.Times(AnyNumber()).After(exp_read_reply);
		EXPECT_CALL(*s, async_send(_))
			.Times(AnyNumber()).After(exp_connect)
			.WillRepeatedly(InvokeWithoutArgs([s]() {
				s->complete_write(boost::asio::buffer_size(s->write_buffers_));
			}));
	});

	vs::event done(false);
	transport::GetServerNameByAddress<protocol>(g_asio_environment->IOService(), ep, [&](const boost::system::error_code& ec, const char* name) {
		EXPECT_FALSE(static_cast<bool>(ec));
		ASSERT_NE(name, nullptr);
		EXPECT_STREQ(name, "Server ID");
		done.set();
	});
	EXPECT_TRUE(done.wait_for(std::chrono::seconds(1))) << "Callback wasn't called in time.";
}

TEST(GetServerNameByAddress, ReadError)
{
	using ::testing::_;
	using ::testing::AllOf;
	using ::testing::AnyNumber;
	using ::testing::Expectation;
	using ::testing::InvokeWithoutArgs;

	using protocol = test::asio::tcp_mock;
	using socket = protocol::socket;

	const protocol::endpoint ep(boost::asio::ip::address_v4(0x18273645), 4242);

	vs::event done_socket(false);
	socket::impl::on_new([&](socket::impl*) {}); // Skip uninteresting socket from transport::Connection
	socket::impl::on_new([&](socket::impl* s) {
		Expectation exp_connect = EXPECT_CALL(*s, async_connect(ep))
			.Times(1)
			.WillOnce(InvokeWithoutArgs([s]() {
				s->is_open_ = true;
				s->complete_connect();
			}));
		Expectation exp_read_reply = EXPECT_CALL(*s, async_receive(_))
			.Times(1).After(exp_connect)
			.WillOnce(InvokeWithoutArgs([s]() {
				s->complete_read(0, boost::asio::error::connection_reset);
			}));
		EXPECT_CALL(*s, close(_))
			.Times(1)
			.WillOnce(InvokeWithoutArgs([&, s]() {
				// To ensure that socket mock won't outlive the test we would need to wait until GetServerNameTask object is destroyed.
				// We can't do this by design, so we suppress leak check for the socket mock.
				::testing::Mock::AllowLeak(s);
				done_socket.set();
			}));
		// Ignore subsequent reads/writes
		EXPECT_CALL(*s, async_receive(_))
			.Times(AnyNumber()).After(exp_read_reply);
		EXPECT_CALL(*s, async_send(_))
			.Times(AnyNumber()).After(exp_connect)
			.WillRepeatedly(InvokeWithoutArgs([s]() {
				s->complete_write(boost::asio::buffer_size(s->write_buffers_));
			}));
	});

	vs::event done(false);
	transport::GetServerNameByAddress<protocol>(g_asio_environment->IOService(), ep, [&](const boost::system::error_code& ec, const char*) {
		EXPECT_EQ(ec, boost::asio::error::connection_reset);
		done.set();
	});
	EXPECT_TRUE(done.wait_for(std::chrono::seconds(1))) << "Callback wasn't called in time.";
	EXPECT_TRUE(done_socket.wait_for(std::chrono::seconds(1))) << "Socket wasn't called in time.";
}

TEST(GetServerNameByAddress, ConnectError)
{
	using ::testing::_;
	using ::testing::AllOf;
	using ::testing::AnyNumber;
	using ::testing::Expectation;
	using ::testing::InvokeWithoutArgs;

	using protocol = test::asio::tcp_mock;
	using socket = protocol::socket;

	const protocol::endpoint ep(boost::asio::ip::address_v4(0x18273645), 4242);

	socket::impl::on_new([&](socket::impl*) {}); // Skip uninteresting socket from transport::Connection
	socket::impl::on_new([&](socket::impl* s) {
		Expectation exp_connect = EXPECT_CALL(*s, async_connect(ep))
			.Times(1)
			.WillOnce(InvokeWithoutArgs([s]() {
				s->is_open_ = true;
				s->complete_connect(boost::asio::error::host_unreachable);
			}));
		EXPECT_CALL(*s, async_receive(_))
			.Times(0).After(exp_connect);
		EXPECT_CALL(*s, async_send(_))
			.Times(0).After(exp_connect);
	});

	vs::event done(false);
	transport::GetServerNameByAddress<protocol>(g_asio_environment->IOService(), ep, [&](const boost::system::error_code& ec, const char*) {
		EXPECT_EQ(ec, boost::asio::error::host_unreachable);
		done.set();
	});
	EXPECT_TRUE(done.wait_for(std::chrono::seconds(1))) << "Callback wasn't called in time.";
}

}
