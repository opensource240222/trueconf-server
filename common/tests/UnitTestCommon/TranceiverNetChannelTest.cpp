#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "TransceiverLib_v2/NetChannelImp.h"
#include "tests/common/ASIOEnvironment.h"
#include "tests/common/GMockOverride.h"
#include "TransceiverLib/VS_TransceiverAuthenticator.h"
#include "tests/mocks/asio_socket_mock.h"
#include "std-generic/cpplib/ignore.h"
#include "std/cpplib/MakeShared.h"
#include "TransceiverLib/VS_ControlRelayMessage.h"

namespace ts_test{

	struct AuthConnectionMock : public auth::Transceiver {
		MOCK_METHOD2_OVERRIDE(AuthConnection, bool(const unsigned char *data, const unsigned long data_sz));
	};
struct TranceiverNetChannelTest : public ::testing::Test
{
	void SetUp() {
		using ::testing::_;
		using ::testing::Return;

		m_auth_connection = std::make_shared<AuthConnectionMock>();
		ON_CALL(*m_auth_connection, AuthConnection(_,_)).WillByDefault(Return(true));

		ts_net_channel = vs::MakeShared<ts::NetChannel<test::asio::tcp_socket_mock>>(
			g_asio_environment->IOService(),
			m_auth_connection,
			[](const std::string&) {} /*dummy transceiver ready callback*/);

		ts_net_channel->SetRecvMessageCallBack([](boost::shared_ptr<VS_MainRelayMessage>& msg) {});
		ts_net_channel->SetTransceiverName([](const std::string&) {});
	}
	void TearDown() {
		ts_net_channel = nullptr;
	}

	test::asio::tcp_socket_mock NewSocket()
	{
		using ::testing::_;

		test::asio::tcp_socket_mock socket(g_asio_environment->IOService());
		EXPECT_CALL(*socket.impl_, open(_, _)).Times(1);
		socket.open(boost::asio::ip::tcp::v4(), vs::ignore<boost::system::error_code>());
		return socket;
	}

	std::shared_ptr<ts::NetChannel<test::asio::tcp_socket_mock>> ts_net_channel;
	std::shared_ptr<AuthConnectionMock> m_auth_connection;
};


TEST_F(TranceiverNetChannelTest, SetSocket) {
	using ::testing::_;

	auto socket = NewSocket();
	test::asio::tcp_socket_mock::impl* socket_mock = socket.impl_.get();

	acs::Handler::stream_buffer input_b = {};
	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	EXPECT_TRUE(ts_net_channel->SetTCPConnection(std::move(socket), std::move(input_b)));
}

TEST_F(TranceiverNetChannelTest, SendMsg) {
	using ::testing::_;

	auto socket = NewSocket();
	test::asio::tcp_socket_mock::impl* socket_mock = socket.impl_.get();
	ts_net_channel->SetTCPConnection(std::move(socket), {});

	auto mess = boost::make_shared<VS_ControlRelayMessage>();
	EXPECT_TRUE(mess->MakeConferenceStart("conf_name"));

	EXPECT_CALL(*socket_mock, async_send(_)).Times(1);
	EXPECT_TRUE(ts_net_channel->SendMsg(mess));
}

}
