#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "tests/common/ASIOEnvironment.h"
#include "acs_v2/Service.h"
#include "newtransport/Router/TransportHandler.h"
#include "net/InterfaceInfo.h"
#include <future>
#include <thread>
#include "tests/UnitTestSecureLib/UnitTestConstants.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_Sign.h"

using namespace boost::asio;

/*

struct TransportRouter_SetConnection : transport::TransportRouter_SetConnection
{
	TransportRouter_SetConnection()
	{}

	virtual inline void   SetConnection(const char *cid,
		const uint32_t version,
		const char *sid,
		boost::asio::ip::tcp::socket&& sock,
		const bool isAccept,
		const uint16_t maxConnSilenceMs,
		const unsigned char fatalSilenceCoef,
		const unsigned char hop,
		const unsigned char *rnd_data,
		const uint32_t rnd_data_ln,
		const unsigned char *sign,
		const uint32_t sign_ln,
		const bool hs_error = false,
		const bool tcp_keep_alive = false)
	{
		m_cid = cid;
		m_sid = sid;
		m_version = version;
		m_hop = hop;
		m_tcp_keep_alive = tcp_keep_alive;
		set_connection_barrier.set_value();
		data_buf.assign(rnd_data, rnd_data + rnd_data_ln);
		sign_buf.assign(sign, sign + sign_ln);
	}

	std::string cid()
	{
		return m_cid;
	}

	std::string sid()
	{
		return m_sid;
	}

	uint32_t version()
	{
		return m_version;
	}

	bool tcp_keep_alive()
	{
		return m_tcp_keep_alive;
	}

	unsigned char hop()
	{
		return m_hop;
	}

	std::future<void> barrier()
	{
		return set_connection_barrier.get_future();
	}

	std::promise<void> set_connection_barrier;
	std::string m_cid, m_sid;
	uint32_t m_version;
	unsigned char m_hop;
	bool m_tcp_keep_alive;
	bool m_hs_error;
	std::vector<unsigned char> data_buf, sign_buf;
};


class TransportHandlerTest : public ::testing::Test
{
public:
	TransportHandlerTest() :buf(256){}
protected:
	virtual void SetUp()
	{
		acs_srv = acs::Service::Create(g_asio_environment->IOService());
		auto start_f = acs_srv->Start();
		EXPECT_EQ(std::future_status::ready, start_f.wait_for(std::chrono::seconds(1))) << "acs::Service::Start took too long";
		if (start_f.valid())
			EXPECT_TRUE(start_f.get()) << "acs::Service::Start failed";
		set_connection = std::make_shared<TransportRouter_SetConnection>();
		transport_handler = std::make_shared<transport::Handler>(set_connection.get());
		acs_srv->AddHandler("Transport Handler", transport_handler);
		ASSERT_TRUE(!acs_srv->AddListener(net::address_v4::loopback(), 4037, net::protocol::TCP));
		char default_str[1024] = { 0 };
		VS_RegistryKey::GetDefaultDB(default_str, sizeof(default_str));
		default_db = default_str;
		VS_RegistryKey::SetDefaultDB("trueconf_transporttest");
#ifdef USE_DATABASE_REGISTRY_KEY
		VS_RegistryKey::InitStructure(); //create tables
#endif
	}

	virtual void TearDown()
	{
		EXPECT_EQ(std::future_status::ready, acs_srv->Stop().wait_for(std::chrono::seconds(1))) << "acs::Service::Stop took too long";
		acs_srv->RemoveListener(net::address_v4::loopback(), 4037, net::protocol::TCP);
		acs_srv.reset();
		VS_RegistryKey::SetDefaultDB(default_db.c_str());
	}

	std::shared_ptr<acs::Service> acs_srv;
	std::shared_ptr<TransportRouter_SetConnection> set_connection;
	std::shared_ptr<transport::Handler> transport_handler;
	boost::system::error_code ec;
	std::vector<char> buf;
	std::string default_db;
};


static const std::string test_cid = "Test Client id";
static const std::string test_sid = "Test Service id";
static const uint32_t test_hop = 0;
static const bool ssl_suppport = true;
static const bool tcp_keep_alive = true;



TEST_F(TransportHandlerTest, Connect)
{
	auto transport_handshake = FormTransportHandshake(test_cid.c_str(), test_sid.c_str(), test_hop, ssl_suppport, tcp_keep_alive);
	boost::asio::ip::tcp::socket sock(g_asio_environment->IOService());
	sock.connect(ip::tcp::endpoint(ip::address::from_string("127.0.0.1"), 4037));
	ASSERT_TRUE(sock.is_open());
	auto bytes_to_send=transport_handshake->body_length + sizeof(ZeroHandshakeFixedPart);
	ASSERT_EQ(sock.send(buffer(transport_handshake.get(), bytes_to_send)), bytes_to_send);
	set_connection->barrier().wait_for(std::chrono::seconds(5));
	ASSERT_EQ(set_connection->cid(), test_cid);
	ASSERT_EQ(set_connection->sid(), test_sid);
	ASSERT_EQ(set_connection->version(), transport_handshake->version);
	ASSERT_EQ(set_connection->hop(), test_hop);
	ASSERT_EQ(set_connection->tcp_keep_alive(), tcp_keep_alive);
	ASSERT_EQ(bool(transport_handshake->version & c_ssl_support_mask), ssl_suppport);
}

TEST_F(TransportHandlerTest, RandomSend)
{
	auto transport_handshake = FormTransportHandshake(test_cid.c_str(), test_sid.c_str(), test_hop, false, false);
	boost::asio::ip::tcp::socket sock(g_asio_environment->IOService());
	sock.connect(ip::tcp::endpoint(ip::address::from_string("127.0.0.1"), 4037));
	ASSERT_TRUE(sock.is_open());
	auto bytes_to_send = transport_handshake->body_length + sizeof(ZeroHandshakeFixedPart);
	for (uint32_t rand_bytes_to_send(0), bytes_sent(0); bytes_to_send>0; bytes_to_send -= rand_bytes_to_send, bytes_sent += rand_bytes_to_send)
	{
		rand_bytes_to_send = (rand() % (bytes_to_send)) + 1;
		ASSERT_EQ(sock.send(buffer(reinterpret_cast<char*>(transport_handshake.get()) + bytes_sent, rand_bytes_to_send)), rand_bytes_to_send);
     	vs::SleepFor(std::chrono::seconds(1));
	}
	set_connection->barrier().wait_for(std::chrono::seconds(5));
	ASSERT_EQ(set_connection->cid(), test_cid);
	ASSERT_EQ(set_connection->sid(), test_sid);
	ASSERT_EQ(set_connection->version(), transport_handshake->version);
	ASSERT_EQ(set_connection->hop(), 0);
	ASSERT_EQ(set_connection->tcp_keep_alive(), false);
	ASSERT_FALSE(transport_handshake->version & c_ssl_support_mask);
}

TEST_F(TransportHandlerTest, HandshakeSign)
{
	VS_RegistryKey key(false, CONFIGURATION_KEY_NAME, false, true);
	ASSERT_TRUE(key.SetValue(constants::server_private_key, sizeof(constants::server_private_key), VS_REG_BINARY_VT, "PrivateKey"));
	auto transport_handshake = FormTransportHandshake(test_cid.c_str(), test_sid.c_str(), 1, false, true, constants::server_private_key_password);
	boost::asio::ip::tcp::socket sock(g_asio_environment->IOService());
	sock.connect(ip::tcp::endpoint(ip::address::from_string("127.0.0.1"), 4037));
	ASSERT_TRUE(sock.is_open());
	auto bytes_to_send = transport_handshake->body_length + sizeof(ZeroHandshakeFixedPart);
	ASSERT_EQ(sock.send(buffer(reinterpret_cast<char*>(transport_handshake.get()), bytes_to_send)), bytes_to_send);
	set_connection->barrier().wait_for(std::chrono::seconds(5));
	ASSERT_EQ(set_connection->cid(), test_cid);
	//verify signature
	VS_Certificate cert;
	cert.Init(prov_OPENSSL);
	ASSERT_TRUE(cert.SetCert(constants::server_cert, sizeof(constants::server_cert), store_PEM_BUF));
	VS_PKey public_key;
	ASSERT_TRUE(public_key.Init(prov_OPENSSL));
	ASSERT_TRUE(cert.GetCertPublicKey(&public_key));
	uint32_t buffer_size = 0;
	ASSERT_FALSE(public_key.GetPublicKey(store_PEM_BUF, 0, &buffer_size));
	std::vector<char> public_key_content(buffer_size);
	ASSERT_TRUE(public_key.GetPublicKey(store_PEM_BUF, public_key_content.data(), &buffer_size));
	VS_Sign verify_sign;
	ASSERT_TRUE(verify_sign.Init(prov_OPENSSL, VS_SignArg({ alg_pk_RSA, alg_hsh_SHA1 })));
	ASSERT_TRUE(verify_sign.SetPublicKey(public_key_content.data(), public_key_content.size(), store_PEM_BUF));
	ASSERT_TRUE(verify_sign.VerifySign(set_connection->data_buf.data(), set_connection->data_buf.size(), set_connection->sign_buf.data(), set_connection->sign_buf.size()));
}

TEST_F(TransportHandlerTest, WrongVersion)
{
	auto transport_handshake = FormTransportHandshake(test_cid.c_str(), test_sid.c_str(), 1, true);
	transport_handshake->version = 0;
	ip::tcp::socket sock(g_asio_environment->IOService());
	sock.connect(ip::tcp::endpoint(ip::address::from_string("127.0.0.1"), 4037));
	ASSERT_TRUE(sock.is_open());
	auto bytes_to_send = transport_handshake->body_length + sizeof(ZeroHandshakeFixedPart);
	ASSERT_EQ(sock.send(buffer(reinterpret_cast<char*>(transport_handshake.get()), bytes_to_send)), bytes_to_send);
	read(sock, buffer(buf), transfer_at_least(1), ec);
	ASSERT_TRUE(ec);
}

TEST_F(TransportHandlerTest, WrongBodyLength)
{
	auto transport_handshake = FormTransportHandshake(test_cid.c_str(), test_sid.c_str(), 1, true);
	transport_handshake->body_length += 100;
	ip::tcp::socket sock(g_asio_environment->IOService());
	sock.connect(ip::tcp::endpoint(ip::address::from_string("127.0.0.1"), 4037));
	ASSERT_TRUE(sock.is_open());
	auto bytes_to_send = transport_handshake->body_length + sizeof(ZeroHandshakeFixedPart);
	ASSERT_EQ(sock.send(buffer(reinterpret_cast<char*>(transport_handshake.get()), bytes_to_send)), bytes_to_send);
	read(sock, buffer(buf), transfer_at_least(1), ec);
	ASSERT_TRUE(ec);
}

TEST_F(TransportHandlerTest, ExcessiveBody)
{
	auto transport_handshake = FormTransportHandshake(test_cid.c_str(), test_sid.c_str(), 1, true);
	ip::tcp::socket sock(g_asio_environment->IOService());
	sock.connect(ip::tcp::endpoint(ip::address::from_string("127.0.0.1"), 4037));
	ASSERT_TRUE(sock.is_open());
	auto bytes_to_send = transport_handshake->body_length + sizeof(ZeroHandshakeFixedPart);
	ASSERT_EQ(sock.send(buffer(reinterpret_cast<char*>(transport_handshake.get()), bytes_to_send)), bytes_to_send);
	ASSERT_EQ(sock.send(buffer(reinterpret_cast<char*>(transport_handshake.get()), bytes_to_send)), bytes_to_send);
	read(sock, buffer(buf), transfer_at_least(1), ec);
	ASSERT_TRUE(ec);
}
*/
