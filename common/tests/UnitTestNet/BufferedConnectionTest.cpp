#include "tests/common/ASIOEnvironment.h"
#include "tests/common/GMockOverride.h"
#include "tests/common/GTestMatchers.h"
#include "tests/mocks/asio_socket_mock.h"
#include "net/BufferedConnection.h"
#include "net/BufferedConnection_impl.h"
#include "std/cpplib/event.h"
#include "std-generic/cpplib/ignore.h"
#include "std-generic/cpplib/string_view.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/asio/buffer.hpp>

namespace boost { namespace asio {
static const_buffers_1 buffer(::string_view x)
{
	return const_buffers_1(x.data(), x.size());
}
}}

namespace net_test {

const string_view data_1 = "First block";
const string_view data_2 = "Second block";
const string_view data_3 = "Block three";

class BufferedTestConnection : public net::BufferedConnection<test::asio::tcp_socket_mock>
{
	using base_t = net::BufferedConnection<test::asio::tcp_socket_mock>;
public:
	using base_t::SetSocket;
	using base_t::Close;
	using base_t::Shutdown;
	using base_t::Send;

	BufferedTestConnection(boost::asio::io_service& ios)
		: base_t(ios)
	{
	}

	MOCK_METHOD2_OVERRIDE(OnReceive, size_t(const void* data, size_t size));
	MOCK_METHOD1_OVERRIDE(OnSend, void(size_t bytes_transferred));
	MOCK_METHOD1_OVERRIDE(OnError, bool(const boost::system::error_code& ec));
	MOCK_METHOD0_OVERRIDE(OnClose, void());
};

struct BufferedConnectionTest : public ::testing::Test
{
	BufferedConnectionTest()
		: connection(std::make_shared<BufferedTestConnection>(g_asio_environment->IOService()))
		, socket_mock(nullptr)
		, done(false)
	{
	}

	test::asio::tcp_socket_mock NewSocket()
	{
		using ::testing::_;

		test::asio::tcp_socket_mock socket(g_asio_environment->IOService());
		EXPECT_CALL(*socket.impl_, open(_,_)).Times(1);
		socket.open(boost::asio::ip::tcp::v4(), vs::ignore<boost::system::error_code>());
		return socket;
	}

	void WaitForCompletion(const char* operation)
	{
		EXPECT_TRUE(done.wait_for(std::chrono::seconds(1))) << operation << " wasn't completed in time.";
	}

	void OpenConnection()
	{
		using ::testing::_;

		auto socket = NewSocket();
		socket_mock = socket.impl_.get();

		EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
		connection->SetSocket(std::move(socket));
		socket_mock->wait_read();
	}

	void CloseConnection()
	{
		using ::testing::_;
		using ::testing::InvokeWithoutArgs;

		EXPECT_CALL(*socket_mock, close(_)).Times(1);
		EXPECT_CALL(*connection, OnClose()).Times(1).WillOnce(InvokeWithoutArgs([&]() {
			::testing::Mock::VerifyAndClearExpectations(socket_mock);
			done.set();
		}));
		connection->Close();
		WaitForCompletion("OnClose() call");
	}

	std::shared_ptr<BufferedTestConnection> connection;
	test::asio::tcp_socket_mock::impl* socket_mock;
	vs::event done;
};

TEST_F(BufferedConnectionTest, OpenClose)
{
	OpenConnection();
	CloseConnection();
}

TEST_F(BufferedConnectionTest, Receive)
{
	using ::testing::_;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	OpenConnection();
	EXPECT_CALL(*connection, OnError(_)).Times(0);

	Sequence seq;

	EXPECT_CALL(*connection, OnReceive(_,_))
		.With(ArgsAsArray<char, 0, 1>(ElementsAreArray(data_1)))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
			return 0;
		}));
	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(data_1));
	socket_mock->complete_read(data_1.size());
	WaitForCompletion("OnReceive() call");
	socket_mock->wait_read();

	CloseConnection();
}

TEST_F(BufferedConnectionTest, Receive_MultipleOnReceiveCalls)
{
	using ::testing::_;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	OpenConnection();
	EXPECT_CALL(*connection, OnError(_)).Times(0);

	Sequence seq;

	EXPECT_CALL(*connection, OnReceive(_,_))
		.With(ArgsAsArray<char, 0, 1>(ElementsAreArray(data_1)))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			return 6;
		}));
	EXPECT_CALL(*connection, OnReceive(_,_))
		.With(ArgsAsArray<char, 0, 1>(ElementsAreArray(data_1.substr(6))))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
			return 0;
		}));
	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(data_1));
	socket_mock->complete_read(data_1.size());
	socket_mock->wait_read();
	WaitForCompletion("OnReceive() call");

	CloseConnection();
}

TEST_F(BufferedConnectionTest, Receive_NotConsumedDataPreserved)
{
	using ::testing::_;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	OpenConnection();
	EXPECT_CALL(*connection, OnError(_)).Times(0);

	Sequence seq;

	EXPECT_CALL(*connection, OnReceive(_,_))
		.With(ArgsAsArray<char, 0, 1>(ElementsAreArray(data_1)))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			return 6;
		}));
	EXPECT_CALL(*connection, OnReceive(_,_))
		.With(ArgsAsArray<char, 0, 1>(ElementsAreArray(data_1.substr(6))))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			return 0;
		}));
	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(data_1));
	socket_mock->complete_read(data_1.size());
	socket_mock->wait_read();

	EXPECT_CALL(*connection, OnReceive(_,_))
		.With(ArgsAsArray<char, 0, 1>(ElementsAreArray(std::string(data_1.substr(6)) + std::string(data_2))))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
			return 0;
		}));
	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(data_2));
	socket_mock->complete_read(data_2.size());
	socket_mock->wait_read();
	WaitForCompletion("OnReceive() call");

	CloseConnection();
}

TEST_F(BufferedConnectionTest, Receive_Error)
{
	using ::testing::_;
	using ::testing::Eq;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	OpenConnection();

	Sequence seq;

	EXPECT_CALL(*connection, OnError(Eq(boost::asio::error::broken_pipe)))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			return false;
		}));
	EXPECT_CALL(*socket_mock, async_receive(_)).Times(0);
	EXPECT_CALL(*socket_mock, close(_)).Times(1);
	EXPECT_CALL(*connection, OnClose())
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			::testing::Mock::VerifyAndClearExpectations(socket_mock);
			done.set();
		}));
	socket_mock->complete_read(0, boost::asio::error::broken_pipe);
	WaitForCompletion("OnClose() call");
}

TEST_F(BufferedConnectionTest, Receive_IgnoredError)
{
	using ::testing::_;
	using ::testing::Eq;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	OpenConnection();

	Sequence seq;

	EXPECT_CALL(*connection, OnError(Eq(boost::asio::error::broken_pipe)))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
			return true;
		}));
	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	socket_mock->complete_read(0, boost::asio::error::broken_pipe);
	socket_mock->wait_read();
	WaitForCompletion("OnError() call");

	CloseConnection();
}

TEST_F(BufferedConnectionTest, Send)
{
	using ::testing::_;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	OpenConnection();
	EXPECT_CALL(*connection, OnError(_)).Times(0);

	Sequence seq;

	EXPECT_CALL(*socket_mock, async_send(_)).Times(1);
	connection->Send(data_1.data(), data_1.size());
	socket_mock->wait_write();
	{
		std::vector<char> write_data(boost::asio::buffer_size(socket_mock->write_buffers_));
		boost::asio::buffer_copy(boost::asio::buffer(write_data), socket_mock->write_buffers_);
		EXPECT_THAT(write_data, ElementsAreArray(data_1));
	}

	EXPECT_CALL(*connection, OnSend(data_1.size()))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
		}));
	EXPECT_CALL(*socket_mock, async_send(_)).Times(0);
	socket_mock->complete_write(data_1.size());
	WaitForCompletion("OnSend() call");

	CloseConnection();
}

TEST_F(BufferedConnectionTest, Send_PartialWrite)
{
	using ::testing::_;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	OpenConnection();
	EXPECT_CALL(*connection, OnError(_)).Times(0);

	Sequence seq;

	EXPECT_CALL(*socket_mock, async_send(_)).Times(1);
	connection->Send(data_1.data(), data_1.size());
	socket_mock->wait_write();
	{
		std::vector<char> write_data(boost::asio::buffer_size(socket_mock->write_buffers_));
		boost::asio::buffer_copy(boost::asio::buffer(write_data), socket_mock->write_buffers_);
		EXPECT_THAT(write_data, ElementsAreArray(data_1));
	}

	EXPECT_CALL(*connection, OnSend(6))
		.Times(1).InSequence(seq);
	EXPECT_CALL(*socket_mock, async_send(_)).Times(1);
	socket_mock->complete_write(6);
	socket_mock->wait_write();
	{
		std::vector<char> write_data(boost::asio::buffer_size(socket_mock->write_buffers_));
		boost::asio::buffer_copy(boost::asio::buffer(write_data), socket_mock->write_buffers_);
		EXPECT_THAT(write_data, ElementsAreArray(data_1.substr(6)));
	}

	EXPECT_CALL(*connection, OnSend(data_1.substr(6).size()))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
		}));
	EXPECT_CALL(*socket_mock, async_send(_)).Times(0);
	socket_mock->complete_write(data_1.substr(6).size());
	WaitForCompletion("OnSend() call");

	CloseConnection();
}

TEST_F(BufferedConnectionTest, DISABLED_Send_ConsecutiveSendsAreBuffered)
{
	using ::testing::_;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	OpenConnection();
	EXPECT_CALL(*connection, OnError(_)).Times(0);

	Sequence seq;

	EXPECT_CALL(*socket_mock, async_send(_)).Times(1);
	connection->Send(data_1.data(), data_1.size());
	socket_mock->wait_write();
	{
		std::vector<char> write_data(boost::asio::buffer_size(socket_mock->write_buffers_));
		boost::asio::buffer_copy(boost::asio::buffer(write_data), socket_mock->write_buffers_);
		EXPECT_THAT(write_data, ElementsAreArray(data_1));
	}

	connection->Send(data_2.data(), data_2.size());
	connection->Send(data_3.data(), data_3.size());

	EXPECT_CALL(*connection, OnSend(data_1.size()))
		.Times(1).InSequence(seq);
	EXPECT_CALL(*socket_mock, async_send(_)).Times(1);
	socket_mock->complete_write(data_1.size());
	socket_mock->wait_write();
	{
		std::vector<char> write_data(boost::asio::buffer_size(socket_mock->write_buffers_));
		boost::asio::buffer_copy(boost::asio::buffer(write_data), socket_mock->write_buffers_);
		EXPECT_THAT(write_data, ElementsAreArray(std::string(data_2) + std::string(data_3)));
	}

	EXPECT_CALL(*connection, OnSend(data_2.size() + data_3.size()))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
		}));
	EXPECT_CALL(*socket_mock, async_send(_)).Times(0);
	socket_mock->complete_write(data_2.size() + data_3.size());
	WaitForCompletion("OnSend() call");

	CloseConnection();
}

TEST_F(BufferedConnectionTest, Send_Error)
{
	using ::testing::_;
	using ::testing::Eq;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	OpenConnection();

	Sequence seq;

	EXPECT_CALL(*socket_mock, async_send(_)).Times(1);
	connection->Send(data_1.data(), data_1.size());
	socket_mock->wait_write();

	EXPECT_CALL(*connection, OnError(Eq(boost::asio::error::broken_pipe)))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			return false;
		}));
	EXPECT_CALL(*socket_mock, async_send(_)).Times(0);
	EXPECT_CALL(*socket_mock, close(_)).Times(1);
	EXPECT_CALL(*connection, OnClose())
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			::testing::Mock::VerifyAndClearExpectations(socket_mock);
			done.set();
		}));
	socket_mock->complete_write(0, boost::asio::error::broken_pipe);
	WaitForCompletion("OnClose() call");
}

TEST_F(BufferedConnectionTest, Send_IgnoredError)
{
	using ::testing::_;
	using ::testing::ElementsAreArray;
	using ::testing::Eq;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	OpenConnection();

	Sequence seq;

	EXPECT_CALL(*socket_mock, async_send(_)).Times(1);
	connection->Send(data_1.data(), data_1.size());
	socket_mock->wait_write();

	EXPECT_CALL(*connection, OnError(Eq(boost::asio::error::broken_pipe)))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			return true;
		}));
	EXPECT_CALL(*socket_mock, async_send(_)).Times(1);
	socket_mock->complete_write(0, boost::asio::error::broken_pipe);
	socket_mock->wait_write();
	{
		std::vector<char> write_data(boost::asio::buffer_size(socket_mock->write_buffers_));
		boost::asio::buffer_copy(boost::asio::buffer(write_data), socket_mock->write_buffers_);
		EXPECT_THAT(write_data, ElementsAreArray(data_1));
	}

	EXPECT_CALL(*connection, OnSend(data_1.size()))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
		}));
	socket_mock->complete_write(data_1.size());
	WaitForCompletion("OnSend() call");

	CloseConnection();
}

TEST_F(BufferedConnectionTest, Shutdown_AllWritesCompleted)
{
	using ::testing::_;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	OpenConnection();
	EXPECT_CALL(*connection, OnError(_)).Times(0);

	Sequence seq;

	EXPECT_CALL(*socket_mock, async_send(_)).Times(1);
	connection->Send(data_1.data(), data_1.size());
	socket_mock->wait_write();
	{
		std::vector<char> write_data(boost::asio::buffer_size(socket_mock->write_buffers_));
		boost::asio::buffer_copy(boost::asio::buffer(write_data), socket_mock->write_buffers_);
		EXPECT_THAT(write_data, ElementsAreArray(data_1));
	}

	connection->Send(data_2.data(), data_2.size());
	connection->Shutdown();

	EXPECT_CALL(*connection, OnSend(data_1.size()))
		.Times(1).InSequence(seq);
	EXPECT_CALL(*socket_mock, async_send(_)).Times(1);
	socket_mock->complete_write(data_1.size());
	socket_mock->wait_write();
	{
		std::vector<char> write_data(boost::asio::buffer_size(socket_mock->write_buffers_));
		boost::asio::buffer_copy(boost::asio::buffer(write_data), socket_mock->write_buffers_);
		EXPECT_THAT(write_data, ElementsAreArray(data_2));
	}

	EXPECT_CALL(*connection, OnSend(data_2.size()))
		.Times(1).InSequence(seq);
	EXPECT_CALL(*socket_mock, async_send(_)).Times(0);
	EXPECT_CALL(*socket_mock, close(_)).Times(1);
	EXPECT_CALL(*connection, OnClose())
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			::testing::Mock::VerifyAndClearExpectations(socket_mock);
			done.set();
		}));
	socket_mock->complete_write(data_2.size());
	WaitForCompletion("OnClose() call");
}

TEST_F(BufferedConnectionTest, Shutdown_NoMoreReads)
{
	using ::testing::_;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Sequence;

	OpenConnection();
	EXPECT_CALL(*connection, OnError(_)).Times(0);

	Sequence seq;

	EXPECT_CALL(*socket_mock, async_send(_)).Times(1);
	connection->Send(data_1.data(), data_1.size());
	socket_mock->wait_write();

	connection->Shutdown();

	EXPECT_CALL(*connection, OnReceive(_,_)).Times(0);
	EXPECT_CALL(*socket_mock, async_receive(_)).Times(0);
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(data_2));
	socket_mock->complete_read(data_2.size());

	EXPECT_CALL(*connection, OnSend(data_1.size()))
		.Times(1).InSequence(seq);
	EXPECT_CALL(*socket_mock, async_send(_)).Times(0);
	EXPECT_CALL(*socket_mock, close(_)).Times(1);
	EXPECT_CALL(*connection, OnClose())
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			::testing::Mock::VerifyAndClearExpectations(socket_mock);
			done.set();
		}));
	socket_mock->complete_write(data_1.size());
	WaitForCompletion("OnClose() call");
}

}
