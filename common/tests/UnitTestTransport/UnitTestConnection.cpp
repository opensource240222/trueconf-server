#include "Data.h"
#include "SecureProviderMock.h"
#include "tests/common/ASIOEnvironment.h"
#include "tests/common/GMockOverride.h"
#include "tests/common/GTestMatchers.h"
#include "tests/mocks/asio_socket_mock.h"
#include "transport/Message.h"
#include "newtransport/Router/Connection.h"
#include "newtransport/Router/Connection_impl.h"
#include "SecureLib/SecureTypes.h"
#include "net/BufferedConnection_impl.h"
#include "std/cpplib/event.h"
#include "std-generic/cpplib/ignore.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace transport_test {

static const auto crypt_alg = VS_SymmetricAlg::alg_sym_AES128;
static const auto crypt_alg_mode = VS_SymmetricCipherMode::mode_ECB;
static const unsigned char crypt_key[] = {
	0x0f, 0xbc, 0x3d, 0x24, 0x3e, 0x2f, 0x33, 0xf4, 0xde, 0x69, 0xa7, 0x46, 0x5d, 0x00, 0xf6, 0xdd,
};
static const unsigned char encrypted_serialized_message[] = {
	0x98, 0x84, 0x5f, 0xde, 0x6d, 0xa5, 0x76, 0xbf, 0x0e, 0xf1, 0x7f, 0x2b, 0xc5, 0xc7, 0x20, 0xf7,
	0xc3, 0xa3, 0x6d, 0xb8, 0x43, 0x38, 0xc1, 0x99, 0xef, 0x5a, 0x94, 0xfa, 0x6b, 0xfe, 0xb7, 0xe1,
	0x96, 0xe2, 0x58, 0x22, 0x47, 0x3b, 0x66, 0x9d, 0xa9, 0x55, 0x27, 0x3a, 0xc3, 0xf8, 0x57, 0xee,
	0xff, 0xf9, 0x4e, 0x73, 0x86, 0xfb, 0x11, 0xe0, 0x77, 0xce, 0x43, 0x24, 0xc0, 0x69, 0x70, 0xd3,
	0xd5, 0x7b, 0x35, 0x53, 0x80, 0xfd, 0xb7, 0x2f, 0x5f, 0x87, 0x08, 0x90, 0x5e, 0xc6, 0xf6, 0xc6,
	0x3e, 0x12, 0x0a, 0xf5, 0x78, 0x84, 0x08, 0x25, 0x9e, 0xa3, 0x8c, 0xdf, 0xe8, 0x2c, 0xd0, 0xc0,
	0x70, 0x8c, 0xaf, 0x06, 0x88, 0x7b, 0x00, 0x27, 0x61, 0x79, 0x19, 0x55, 0x7b, 0x7c, 0x39, 0x03,
	0xd8, 0x37, 0xb6, 0x58, 0xde, 0x7c, 0x9e, 0x92, 0x06, 0x6b, 0xa1, 0x6d, 0x7c, 0x19, 0xe1, 0x0b,
	0xf7, 0xa9, 0xcc, 0x6c, 0xf1, 0xfa, 0x7f, 0xa6, 0x38, 0x99, 0x1b, 0xae, 0xf9, 0xcf, 0x1a, 0xb2,
	0xaa, 0xe0, 0x16, 0x3d, 0xd0, 0x2a, 0xe4, 0xd5, 0xab, 0x37, 0x9b, 0x21, 0x4d, 0x08, 0x2f, 0x07,
	0x05, 0x81, 0xc2, 0x06, 0x15, 0xb5, 0x4c, 0xb2, 0xb7, 0x47, 0x6c, 0xb3, 0xb3, 0x97, 0x9d, 0x54,
	0x4d, 0xf8, 0xdd, 0x89, 0xd9, 0xf1, 0x1f, 0x33, 0x4e, 0x2a, 0x60, 0x95, 0xd9, 0x6b, 0x06, 0x51,
	0x1d, 0x91, 0xf1, 0x97, 0x1b, 0x55, 0xa9, 0x34, 0xf8, 0xc2, 0x60, 0xfd, 0xf9, 0x81, 0x78, 0xf8,
	0xba, 0x01, 0x80, 0xce, 0x63, 0x3e, 0xfe, 0x44, 0x36, 0xf7, 0x1e, 0x0b, 0xd3, 0x9c, 0x46, 0x83,
	0x56, 0x11, 0x9b, 0xb8, 0x94, 0xea, 0xde, 0xe4, 0x03, 0xee, 0x53, 0x81, 0x10, 0xf8, 0x4e, 0x3c,
};

class TestConnection : public transport::Connection<test::asio::tcp_socket_mock>
{
	using base_t = transport::Connection<test::asio::tcp_socket_mock>;
public:
	using base_t::Start;
	using base_t::Accept;
	using base_t::Send;
	using base_t::HandshakeCompleted;
	using base_t::SecureHandshakeCompleted;
	using base_t::Close;

	TestConnection(boost::asio::io_service& ios)
		: base_t(ios)
	{
	}

	void SetSecureProvider(transport::SecureProvider* sec)
	{
		m_sec = sec;
	}

	MOCK_METHOD7_OVERRIDE(OnHandshakeReply, void(transport::HandshakeResult result, uint16_t max_conn_silence_ms, uint8_t fatal_silence_coef, uint8_t hops, const char* server_id, const char* client_id, bool tcp_keep_alive_support));
	MOCK_METHOD1(OnMessage_mocked, void(transport::Message& message));
	void OnMessage(transport::Message&& message) override
	{
		OnMessage_mocked(message);
	}

	MOCK_METHOD1_OVERRIDE(OnError, bool(const boost::system::error_code& ec));
	MOCK_METHOD0_OVERRIDE(OnClose, void());
};

struct TransportConnectionTest : public ::testing::Test
{
	TransportConnectionTest()
		: connection(std::make_shared<TestConnection>(g_asio_environment->IOService()))
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
		using ::testing::AtLeast;
		using ::testing::InvokeWithoutArgs;

		auto socket = NewSocket();
		socket_mock = socket.impl_.get();

		char hsr_data[sizeof(net::HandshakeHeader) + 1] = {};
		const auto hsr = reinterpret_cast<net::HandshakeHeader*>(hsr_data);
		hsr->body_length = 0;
		EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
		size_t write_size = 0;
		EXPECT_CALL(*socket_mock, async_send(_))
			.Times(AtLeast(1))
			.WillRepeatedly(InvokeWithoutArgs([&]() {
				const auto size = boost::asio::buffer_size(socket_mock->write_buffers_);
				write_size += size;
				if (write_size >= sizeof(hsr_data))
					done.set();
				socket_mock->complete_write(size);
			}));
		connection->Accept(std::move(socket), hsr);
		socket_mock->wait_read();
		WaitForCompletion("Handshake reply write");
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

	std::shared_ptr<TestConnection> connection;
	test::asio::tcp_socket_mock::impl* socket_mock;
	std::unique_ptr<transport::SecureProvider> sec;
	vs::event done;
};

TEST_F(TransportConnectionTest, Start)
{
	using ::testing::_;
	using ::testing::AtLeast;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;
	using ::testing::StrEq;

	EXPECT_FALSE(connection->HandshakeCompleted());

	auto socket = NewSocket();
	socket_mock = socket.impl_.get();

	auto hs = transport::CreateHandshake("ClientID", "ServerID", 0, true, true);
	const size_t hs_size = sizeof(net::HandshakeHeader) + hs->body_length + 1;

	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	std::vector<uint8_t> write_data;
	EXPECT_CALL(*socket_mock, async_send(_))
		.Times(AtLeast(1))
		.WillRepeatedly(InvokeWithoutArgs([&]() {
			const auto size = boost::asio::buffer_size(socket_mock->write_buffers_);
			const auto data_size = write_data.size();
			write_data.resize(data_size + size);
			boost::asio::buffer_copy(boost::asio::buffer(write_data.data() + data_size, size), socket_mock->write_buffers_);
			if (write_data.size() >= hs_size)
				done.set();
			socket_mock->complete_write(boost::asio::buffer_size(socket_mock->write_buffers_));
		}));
	connection->Start(std::move(socket), hs.get());
	socket_mock->wait_read();
	WaitForCompletion("Handshake write");
	EXPECT_THAT(write_data, ElementsAreArray(reinterpret_cast<uint8_t*>(hs.get()), hs_size));

	auto hsr = transport::CreateHandshakeReply("ServerID", "ClientID", transport::HandshakeResult::ok, 12345, 67, 89, true, true);
	const size_t hsr_size = sizeof(net::HandshakeHeader) + hsr->body_length + 1;

	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	EXPECT_CALL(*connection, OnHandshakeReply(transport::HandshakeResult::ok, 12345, 67, 89, StrEq("ServerID"), StrEq("ClientID"), true))
		.Times(1)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
		}));
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(hsr.get(), hsr_size));
	socket_mock->complete_read(hsr_size);
	socket_mock->wait_read();
	WaitForCompletion("OnHandshake() call");
	EXPECT_TRUE(connection->HandshakeCompleted());

	CloseConnection();
}

TEST_F(TransportConnectionTest, Accept)
{
	using ::testing::_;
	using ::testing::AtLeast;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;

	EXPECT_FALSE(connection->HandshakeCompleted());

	auto socket = NewSocket();
	socket_mock = socket.impl_.get();

	auto hsr = transport::CreateHandshakeReply("ServerID", "ClientID", transport::HandshakeResult::ok, 12345, 67, 89, true, true);
	const size_t hsr_size = sizeof(net::HandshakeHeader) + hsr->body_length + 1;

	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	std::vector<uint8_t> write_data;
	EXPECT_CALL(*socket_mock, async_send(_))
		.Times(AtLeast(1))
		.WillRepeatedly(InvokeWithoutArgs([&]() {
			const auto size = boost::asio::buffer_size(socket_mock->write_buffers_);
			const auto data_size = write_data.size();
			write_data.resize(data_size + size);
			boost::asio::buffer_copy(boost::asio::buffer(write_data.data() + data_size, size), socket_mock->write_buffers_);
			if (write_data.size() >= hsr_size)
				done.set();
			socket_mock->complete_write(boost::asio::buffer_size(socket_mock->write_buffers_));
		}));
	connection->Accept(std::move(socket), hsr.get());
	WaitForCompletion("Handshake reply write");
	EXPECT_TRUE(connection->HandshakeCompleted());
	EXPECT_THAT(write_data, ElementsAreArray(reinterpret_cast<uint8_t*>(hsr.get()), hsr_size));

	CloseConnection();
}

TEST_F(TransportConnectionTest, SecureHandshake_Success)
{
	using ::testing::_;
	using ::testing::AtLeast;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Return;
	using ::testing::Sequence;

	const char shs_1[] = "First block";
	const char shs_2[] = "Second block";
	const char shs_3[] = "Block 3";

	transport_test::SecureProviderMock sec;
	sec.SetState(transport::SecureProvider::State::handshake_in_progress);
	connection->SetSecureProvider(&sec);

	Sequence seq;

	EXPECT_CALL(sec, GetPacket())
		.Times(1).InSequence(seq)
		.WillOnce(Return(std::vector<uint8_t>()));
	OpenConnection();
	EXPECT_CALL(*connection, OnError(_)).Times(0);

	EXPECT_FALSE(connection->SecureHandshakeCompleted());

	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	std::vector<uint8_t> write_data;
	EXPECT_CALL(*socket_mock, async_send(_))
		.Times(AtLeast(1))
		.WillRepeatedly(InvokeWithoutArgs([&]() {
			const auto size = boost::asio::buffer_size(socket_mock->write_buffers_);
			const auto data_size = write_data.size();
			write_data.resize(data_size + size);
			boost::asio::buffer_copy(boost::asio::buffer(write_data.data() + data_size, size), socket_mock->write_buffers_);
			if (write_data.size() >= sizeof(shs_2))
				done.set();
			socket_mock->complete_write(boost::asio::buffer_size(socket_mock->write_buffers_));
		}));
	EXPECT_CALL(sec, HandlePacket(_,_))
		.With(ArgsAsArray<uint8_t, 0, 1>(ElementsAreArray(shs_1)))
		.Times(1).InSequence(seq)
		.WillOnce(Return(sizeof(shs_1)));
	EXPECT_CALL(sec, GetPacket())
		.Times(1).InSequence(seq)
		.WillOnce(Return(std::vector<uint8_t>(std::begin(shs_2), std::end(shs_2))));
	EXPECT_CALL(sec, GetPacket())
		.Times(1).InSequence(seq)
		.WillOnce(Return(std::vector<uint8_t>()));
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(shs_1));
	socket_mock->complete_read(sizeof(shs_1));
	socket_mock->wait_read();
	WaitForCompletion("Secure handshake part 2 (write)");
	EXPECT_FALSE(connection->SecureHandshakeCompleted());

	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	EXPECT_CALL(sec, HandlePacket(_,_))
		.With(ArgsAsArray<uint8_t, 0, 1>(ElementsAreArray(shs_3)))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			sec.SetState(transport::SecureProvider::State::handshake_completed);
			done.set();
			return sizeof(shs_3);
		}));
	EXPECT_CALL(sec, HandlePacket(_,_))
		.Times(0).InSequence(seq);
	EXPECT_CALL(sec, GetPacket())
		.Times(0).InSequence(seq);
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(shs_3));
	socket_mock->complete_read(sizeof(shs_3));
	socket_mock->wait_read();
	WaitForCompletion("Secure handshake part 3 (read)");
	EXPECT_TRUE(connection->SecureHandshakeCompleted());

	CloseConnection();
}

TEST_F(TransportConnectionTest, SecureHandshake_Error)
{
	using ::testing::_;
	using ::testing::ElementsAreArray;
	using ::testing::Eq;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Return;
	using ::testing::Sequence;

	const char shs_1[] = "First block";

	transport_test::SecureProviderMock sec;
	sec.SetState(transport::SecureProvider::State::handshake_in_progress);
	connection->SetSecureProvider(&sec);

	Sequence seq;

	EXPECT_CALL(sec, GetPacket())
		.Times(1).InSequence(seq)
		.WillOnce(Return(std::vector<uint8_t>()));
	OpenConnection();

	EXPECT_FALSE(connection->SecureHandshakeCompleted());

	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	EXPECT_CALL(sec, HandlePacket(_,_))
		.With(ArgsAsArray<uint8_t, 0, 1>(ElementsAreArray(shs_1)))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			sec.SetState(transport::SecureProvider::State::error);
			return 0;
		}));
	EXPECT_CALL(sec, GetPacket())
		.Times(0).InSequence(seq);
	EXPECT_CALL(*connection, OnError(Eq(transport::errc::secure_handshake_error)))
		.Times(1).InSequence(seq)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
			return true; // Ignore error to simplify test
		}));
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(shs_1));
	socket_mock->complete_read(sizeof(shs_1));
	socket_mock->wait_read();
	WaitForCompletion("OnError() call");
	EXPECT_FALSE(connection->SecureHandshakeCompleted());

	CloseConnection();
}

TEST_F(TransportConnectionTest, SendMessage)
{
	using ::testing::_;
	using ::testing::AtLeast;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;

	transport::Message msg(serialized_message, sizeof(serialized_message));
	EXPECT_TRUE(msg.IsValid());

	OpenConnection();
	EXPECT_CALL(*connection, OnError(_)).Times(0);

	std::vector<uint8_t> write_data;
	EXPECT_CALL(*socket_mock, async_send(_))
		.Times(AtLeast(1))
		.WillRepeatedly(InvokeWithoutArgs([&]() {
			const auto size = boost::asio::buffer_size(socket_mock->write_buffers_);
			const auto data_size = write_data.size();
			write_data.resize(data_size + size);
			boost::asio::buffer_copy(boost::asio::buffer(write_data.data() + data_size, size), socket_mock->write_buffers_);
			if (write_data.size() >= sizeof(serialized_message))
				done.set();
			socket_mock->complete_write(boost::asio::buffer_size(socket_mock->write_buffers_));
		}));
	connection->Send(msg);
	WaitForCompletion("Date write");
	EXPECT_THAT(write_data, ElementsAreArray(serialized_message));

	CloseConnection();
}

TEST_F(TransportConnectionTest, SendEncryptedMessage)
{
	using ::testing::_;
	using ::testing::AtLeast;
	using ::testing::ElementsAreArray;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Return;

	transport::Message msg(serialized_message, sizeof(serialized_message));
	EXPECT_TRUE(msg.IsValid());

	VS_SymmetricCrypt crypt;
	ASSERT_TRUE(crypt.Init(crypt_alg, crypt_alg_mode));
	ASSERT_TRUE(crypt.SetKey(sizeof(crypt_key), crypt_key));

	transport_test::SecureProviderMock sec;
	sec.SetState(transport::SecureProvider::State::handshake_completed);
	EXPECT_CALL(sec, WriteCrypt())
		.Times(AtLeast(1))
		.WillRepeatedly(Return(&crypt));
	connection->SetSecureProvider(&sec);

	OpenConnection();
	EXPECT_CALL(*connection, OnError(_)).Times(0);

	std::vector<uint8_t> write_data;
	EXPECT_CALL(*socket_mock, async_send(_))
		.Times(AtLeast(1))
		.WillRepeatedly(InvokeWithoutArgs([&]() {
			const auto size = boost::asio::buffer_size(socket_mock->write_buffers_);
			const auto data_size = write_data.size();
			write_data.resize(data_size + size);
			boost::asio::buffer_copy(boost::asio::buffer(write_data.data() + data_size, size), socket_mock->write_buffers_);
			if (write_data.size() >= sizeof(encrypted_serialized_message))
				done.set();
			socket_mock->complete_write(boost::asio::buffer_size(socket_mock->write_buffers_));
		}));
	connection->Send(msg);
	WaitForCompletion("Date write");
	EXPECT_THAT(write_data, ElementsAreArray(encrypted_serialized_message));

	CloseConnection();
}

TEST_F(TransportConnectionTest, ReceiveMessage)
{
	using ::testing::_;
	using ::testing::ElementsAreArray;
	using ::testing::Invoke;

	OpenConnection();
	EXPECT_CALL(*connection, OnError(_)).Times(0);

	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	std::vector<uint8_t> msg_data;
	EXPECT_CALL(*connection, OnMessage_mocked(_))
		.Times(1)
		.WillOnce(Invoke([&](transport::Message& message) {
			msg_data.assign(message.Data(), message.Data() + message.Size());
			done.set();
		}));
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(serialized_message));
	socket_mock->complete_read(sizeof(serialized_message));
	socket_mock->wait_read();
	WaitForCompletion("OnMessage() call");
	EXPECT_THAT(msg_data, ElementsAreArray(serialized_message));

	CloseConnection();
}

TEST_F(TransportConnectionTest, ReceiveMessage_PartialReceive)
{
	using ::testing::_;
	using ::testing::ElementsAreArray;
	using ::testing::Invoke;

	OpenConnection();
	EXPECT_CALL(*connection, OnError(_)).Times(0);

	static_assert(sizeof(serialized_message) >= 30, "");
	// Feed part of MessageFixedPart
	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(serialized_message, 10));
	socket_mock->complete_read(10);
	socket_mock->wait_read();

	// Feed rest of MessageFixedPart and part of the body
	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(serialized_message + 10, 10));
	socket_mock->complete_read(10);
	socket_mock->wait_read();

	// Feed the rest
	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	std::vector<uint8_t> msg_data;
	EXPECT_CALL(*connection, OnMessage_mocked(_))
		.Times(1)
		.WillOnce(Invoke([&](transport::Message& message) {
			msg_data.assign(message.Data(), message.Data() + message.Size());
			done.set();
		}));
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(serialized_message + 20, sizeof(serialized_message) - 20));
	socket_mock->complete_read(sizeof(serialized_message) - 20);
	socket_mock->wait_read();
	WaitForCompletion("OnMessage() call");
	EXPECT_THAT(msg_data, ElementsAreArray(serialized_message));

	CloseConnection();
}

TEST_F(TransportConnectionTest, ReceiveMessage_InvalidMessageHeader)
{
	using ::testing::_;
	using ::testing::Eq;
	using ::testing::InvokeWithoutArgs;

	OpenConnection();

	char msg_data[sizeof(transport::MessageFixedPart) + 1] = {};
	const auto msg = reinterpret_cast<transport::MessageFixedPart*>(msg_data);
	msg->mark1 = 0;
	msg->head_length = sizeof(transport::MessageFixedPart);
	msg->body_length = 0;

	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	EXPECT_CALL(*connection, OnError(Eq(transport::errc::invalid_message_header)))
		.Times(1)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
			return true; // Ignore error to simplify test
		}));
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(msg_data));
	socket_mock->complete_read(sizeof(msg_data));
	socket_mock->wait_read();
	WaitForCompletion("OnError() call");

	CloseConnection();
}

TEST_F(TransportConnectionTest, ReceiveMessage_InvalidMessageHeaderChecksum)
{
	using ::testing::_;
	using ::testing::Eq;
	using ::testing::InvokeWithoutArgs;

	OpenConnection();

	char msg_data[sizeof(transport::MessageFixedPart) + 1] = {};
	const auto msg = reinterpret_cast<transport::MessageFixedPart*>(msg_data);
	msg->version = 2;
	msg->mark1 = 1;
	msg->head_length = sizeof(transport::MessageFixedPart);
	msg->body_length = 0;

	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	EXPECT_CALL(*connection, OnError(Eq(transport::errc::invalid_message_head_cksum)))
		.Times(1)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
			return true; // Ignore error to simplify test
		}));
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(msg_data));
	socket_mock->complete_read(sizeof(msg_data));
	socket_mock->wait_read();
	WaitForCompletion("OnError() call");

	CloseConnection();
}

TEST_F(TransportConnectionTest, ReceiveMessage_InvalidMessageBodyChecksum)
{
	using ::testing::_;
	using ::testing::Eq;
	using ::testing::InvokeWithoutArgs;

	OpenConnection();

	char msg_data[sizeof(transport::MessageFixedPart) + 1] = {};
	const auto msg = reinterpret_cast<transport::MessageFixedPart*>(msg_data);
	msg->version = 2;
	msg->mark1 = 1;
	msg->head_length = sizeof(transport::MessageFixedPart);
	msg->body_length = 0;
	msg->head_cksum = transport::GetMessageHeaderChecksum(*msg);

	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	EXPECT_CALL(*connection, OnError(Eq(transport::errc::invalid_message_body_cksum)))
		.Times(1)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
			return true; // Ignore error to simplify test
		}));
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(msg_data));
	socket_mock->complete_read(sizeof(msg_data));
	socket_mock->wait_read();
	WaitForCompletion("OnError() call");

	CloseConnection();
}

TEST_F(TransportConnectionTest, ReceiveEncryptedMessage)
{
	using ::testing::_;
	using ::testing::AtLeast;
	using ::testing::ElementsAreArray;
	using ::testing::Invoke;
	using ::testing::Return;

	VS_SymmetricCrypt crypt;
	ASSERT_TRUE(crypt.Init(crypt_alg, crypt_alg_mode));
	ASSERT_TRUE(crypt.SetKey(sizeof(crypt_key), crypt_key));

	transport_test::SecureProviderMock sec;
	sec.SetState(transport::SecureProvider::State::handshake_completed);
	EXPECT_CALL(sec, ReadCrypt())
		.Times(AtLeast(1))
		.WillRepeatedly(Return(&crypt));
	connection->SetSecureProvider(&sec);

	OpenConnection();
	EXPECT_CALL(*connection, OnError(_)).Times(0);

	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	std::vector<uint8_t> msg_data;
	EXPECT_CALL(*connection, OnMessage_mocked(_))
		.Times(1)
		.WillOnce(Invoke([&](transport::Message& message) {
			msg_data.assign(message.Data(), message.Data() + message.Size());
			done.set();
		}));
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(encrypted_serialized_message));
	socket_mock->complete_read(sizeof(encrypted_serialized_message));
	socket_mock->wait_read();
	WaitForCompletion("OnMessage() call");
	EXPECT_THAT(msg_data, ElementsAreArray(serialized_message));

	CloseConnection();
}

TEST_F(TransportConnectionTest, ReceiveEncryptedMessage_PartialReceive)
{
	using ::testing::_;
	using ::testing::AtLeast;
	using ::testing::ElementsAreArray;
	using ::testing::Invoke;
	using ::testing::Return;

	VS_SymmetricCrypt crypt;
	ASSERT_TRUE(crypt.Init(crypt_alg, crypt_alg_mode));
	ASSERT_TRUE(crypt.SetKey(sizeof(crypt_key), crypt_key));

	transport_test::SecureProviderMock sec;
	sec.SetState(transport::SecureProvider::State::handshake_completed);
	EXPECT_CALL(sec, ReadCrypt())
		.Times(AtLeast(1))
		.WillRepeatedly(Return(&crypt));
	connection->SetSecureProvider(&sec);

	OpenConnection();
	EXPECT_CALL(*connection, OnError(_)).Times(0);

	static_assert(sizeof(encrypted_serialized_message) >= 30, "");
	// Feed part of MessageFixedPart
	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(encrypted_serialized_message, 10));
	socket_mock->complete_read(10);
	socket_mock->wait_read();

	// Feed rest of MessageFixedPart and part of the body
	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(encrypted_serialized_message + 10, 10));
	socket_mock->complete_read(10);
	socket_mock->wait_read();

	// Feed the rest
	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	std::vector<uint8_t> msg_data;
	EXPECT_CALL(*connection, OnMessage_mocked(_))
		.Times(1)
		.WillOnce(Invoke([&](transport::Message& message) {
			msg_data.assign(message.Data(), message.Data() + message.Size());
			done.set();
		}));
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(encrypted_serialized_message + 20, sizeof(encrypted_serialized_message) - 20));
	socket_mock->complete_read(sizeof(encrypted_serialized_message) - 20);
	socket_mock->wait_read();
	WaitForCompletion("OnMessage() call");
	EXPECT_THAT(msg_data, ElementsAreArray(serialized_message));

	CloseConnection();
}

TEST_F(TransportConnectionTest, ReceiveEncryptedMessage_InvalidMessage)
{
	using ::testing::_;
	using ::testing::AtLeast;
	using ::testing::Eq;
	using ::testing::InvokeWithoutArgs;
	using ::testing::Return;

	VS_SymmetricCrypt crypt;
	ASSERT_TRUE(crypt.Init(crypt_alg, crypt_alg_mode));
	ASSERT_TRUE(crypt.SetKey(sizeof(crypt_key), crypt_key));

	transport_test::SecureProviderMock sec;
	sec.SetState(transport::SecureProvider::State::handshake_completed);
	EXPECT_CALL(sec, ReadCrypt())
		.Times(AtLeast(1))
		.WillRepeatedly(Return(&crypt));
	connection->SetSecureProvider(&sec);

	OpenConnection();

	char msg_data[sizeof(transport::MessageFixedPart) + 1] = {};
	const auto msg = reinterpret_cast<transport::MessageFixedPart*>(msg_data);
	msg->mark1 = 0;
	msg->head_length = sizeof(transport::MessageFixedPart);
	msg->body_length = 0;

	char enc_msg_data[sizeof(msg_data) + 64] = {};
	uint32_t enc_msg_size = sizeof(enc_msg_data);
	ASSERT_TRUE(crypt.Encrypt(reinterpret_cast<unsigned char*>(msg_data), sizeof(msg_data), reinterpret_cast<unsigned char*>(enc_msg_data), &enc_msg_size));

	EXPECT_CALL(*socket_mock, async_receive(_)).Times(1);
	EXPECT_CALL(*connection, OnError(Eq(transport::errc::invalid_message_header)))
		.Times(1)
		.WillOnce(InvokeWithoutArgs([&]() {
			done.set();
			return true; // Ignore error to simplify test
		}));
	boost::asio::buffer_copy(socket_mock->read_buffers_, boost::asio::buffer(enc_msg_data));
	socket_mock->complete_read(sizeof(enc_msg_data));
	socket_mock->wait_read();
	WaitForCompletion("OnError() call");

	CloseConnection();
}

}
