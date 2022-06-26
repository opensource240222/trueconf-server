#include "Data.h"
#include "newtransport/Handshake.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace transport_test {

TEST(TransportHandshake, Create)
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	ASSERT_TRUE(key.SetValue(serialized_handshake_private_key, sizeof(serialized_handshake_private_key), VS_REG_BINARY_VT, SRV_PRIVATE_KEY));

	auto hs = transport::CreateHandshake("Client ID", "Server ID", 12, true, true, serialized_handshake_rnd_data, sizeof(serialized_handshake_rnd_data));
	ASSERT_NE(nullptr, hs);

	for (size_t i = 0; i < sizeof(serialized_handshake); ++i)
		EXPECT_EQ(reinterpret_cast<uint8_t*>(hs.get())[i], serialized_handshake[i]) << "index=" << i;
}

TEST(TransportHandshake, Create_NoSign)
{
	auto hs = transport::CreateHandshake("Client ID", "Server ID", 0, true, true);
	ASSERT_NE(nullptr, hs);

	for (size_t i = 0; i < sizeof(serialized_handshake_nosign); ++i)
		EXPECT_EQ(reinterpret_cast<uint8_t*>(hs.get())[i], serialized_handshake_nosign[i]) << "index=" << i;
}

TEST(TransportHandshake, Parse)
{
	using ::testing::StrEq;

	const char* cid = nullptr;
	const char* sid = nullptr;
	uint8_t hops = 0;
	const void* rnd_data = nullptr;
	size_t rnd_data_size = 0;
	const void* sign = nullptr;
	size_t sign_size = 0;
	bool tcp_keep_alive_support = false;

	EXPECT_TRUE(transport::ParseHandshake(reinterpret_cast<const net::HandshakeHeader*>(serialized_handshake), cid, sid, hops, rnd_data, rnd_data_size, sign, sign_size, tcp_keep_alive_support));

	EXPECT_THAT(cid, StrEq("Client ID"));
	EXPECT_THAT(sid, StrEq("Server ID"));
	EXPECT_EQ(12, hops);
	EXPECT_EQ(sizeof(serialized_handshake_rnd_data), rnd_data_size);
	for (size_t i = 0; i < rnd_data_size; ++i)
		EXPECT_EQ(reinterpret_cast<const uint8_t*>(rnd_data)[i], serialized_handshake_rnd_data[i]) << "index=" << i;
	// TODO: sign
	EXPECT_EQ(true, tcp_keep_alive_support);
}

TEST(TransportHandshake, Parse_NoSign)
{
	using ::testing::StrEq;

	const char* cid = nullptr;
	const char* sid = nullptr;
	uint8_t hops = 111;
	const void* rnd_data = "";
	size_t rnd_data_size = 1;
	const void* sign = "";
	size_t sign_size = 1;
	bool tcp_keep_alive_support = false;

	EXPECT_TRUE(transport::ParseHandshake(reinterpret_cast<const net::HandshakeHeader*>(serialized_handshake_nosign), cid, sid, hops, rnd_data, rnd_data_size, sign, sign_size, tcp_keep_alive_support));

	EXPECT_THAT(cid, StrEq("Client ID"));
	EXPECT_THAT(sid, StrEq("Server ID"));
	EXPECT_EQ(0, hops);
	EXPECT_EQ(nullptr, rnd_data);
	EXPECT_EQ(0, rnd_data_size);
	EXPECT_EQ(nullptr, sign);
	EXPECT_EQ(0, sign_size);
	EXPECT_EQ(true, tcp_keep_alive_support);
}

TEST(TransportHandshake, CreateReply)
{
	auto hs = transport::CreateHandshakeReply("Server ID", "Client ID", transport::HandshakeResult::ok, 3456, 78, 12, true, true);
	ASSERT_NE(nullptr, hs);

	for (size_t i = 0; i < sizeof(serialized_handshake_reply); ++i)
		EXPECT_EQ(reinterpret_cast<uint8_t*>(hs.get())[i], serialized_handshake_reply[i]) << "index=" << i;
}

TEST(TransportHandshake, ParseReply)
{
	using ::testing::StrEq;

	const char* sid = nullptr;
	const char* cid = nullptr;
	transport::HandshakeResult result = transport::HandshakeResult::oldarch;
	uint16_t max_conn_silence_ms = 0;
	uint8_t fatal_silence_coef = 0;
	uint8_t hops = 0;
	bool tcp_keep_alive_support = false;

	EXPECT_TRUE(transport::ParseHandshakeReply(reinterpret_cast<const net::HandshakeHeader*>(serialized_handshake_reply), sid, cid, result, max_conn_silence_ms, fatal_silence_coef, hops, tcp_keep_alive_support));

	EXPECT_THAT(sid, StrEq("Server ID"));
	EXPECT_THAT(cid, StrEq("Client ID"));
	EXPECT_EQ(transport::HandshakeResult::ok, result);
	EXPECT_EQ(3456, max_conn_silence_ms);
	EXPECT_EQ(78, fatal_silence_coef);
	EXPECT_EQ(12, hops);
	EXPECT_EQ(true, tcp_keep_alive_support);
}

TEST(TransportHandshake, CreateReply_OldArch)
{
	auto hs = transport::CreateHandshakeReply_OLDARCH();
	ASSERT_NE(nullptr, hs);

	for (size_t i = 0; i < sizeof(serialized_handshake_oldarch); ++i)
		EXPECT_EQ(reinterpret_cast<uint8_t*>(hs.get())[i], serialized_handshake_oldarch[i]) << "index=" << i;
}

}
