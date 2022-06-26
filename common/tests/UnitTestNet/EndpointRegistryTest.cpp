#include "net/EndpointRegistry.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std-generic/cpplib/scope_exit.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cstring>
#include <memory>

template <class T, std::size_t N>
inline std::size_t array_size(T(&)[N]) { return N; }

namespace er_test {

const char ep_1[] = "UnitTest_ER_1";

const char host_1[] = "vcs.example.com";
const char host_2[] = "server.local";
const unsigned short port_1 = 13579;
const unsigned short port_2 = 24680;
const char protocol_1[] = "test_protocol";
const char protocol_2[] = "secondary_proto";

const char socks_host_1[] = "12.34.56.78";
const char socks_host_2[] = "host1.example.com";
const unsigned short socks_port_1 = 54321;
const unsigned short socks_port_2 = 12345;
const char socks_user_1[] = "alice";
const char socks_user_2[] = "anonymous";
const char socks_password_1[] = "p@ssw0rd";
const char socks_password_2[] = "suomynona";
const unsigned char socks_version_1 = 13;
const unsigned char socks_version_2 = 31;

const char http_host_1[] = "98.76.54.32";
const char http_host_2[] = "host2.example.com";
const unsigned short http_port_1 = 17171;
const unsigned short http_port_2 = 20202;
const char http_user_1[] = "bob";
const char http_user_2[] = "username";
const char http_password_1[] = "qwerty123";
const char http_password_2[] = "abcdef";

const unsigned char serialized_bad_chksum[] = {
	0x12, 0x34, 0x56, 0x78, // wrong checksum, valid is: 0x0d, 0x31, 0xfc, 0x24
	0x03, 'f', 'o', 'o', '\0',
	0x50, 0x05,
	0x04, 't', 'e', 's', 't', '\0',
};

const unsigned char serialized_bad_format_1[] = {
	0x0d, 0x31, 0xfc, 0x24,
	0x03, 'f', 'o', 'o', '\0',
	0x00, 0x00, 0x50, 0x05, // 32-bit port
	0x04, 't', 'e', 's', 't', '\0',
};

const unsigned char serialized_bad_format_2[] = {
	0xc1, 0xb9, 0x92, 0x9f,
	0x03, 'f', 'o', 'o', '\0',
	0x00, 0x02,
	0x04, 't', 'e', 's', 't', '\0',
	0x03, 'b', 'a', 'r', '\0',
	0x00, 0x04,
	0x04, 'u', 's', 'e', 'r', '\0',
	0x04, 'p', 'a', 's', 's', '\0',
	// 0x11, // missing socks_version
	0x03, 'b', 'a', 'z', '\0',
	0x00, 0x08,
	0x04, 'u', 's', 'e', 'r', '\0',
	0x04, 'p', 'a', 's', 's', '\0',
};

const unsigned char serialized_accept_empty[] = {
	0x95, 0x15, 0x14, 0x3d,
	0x00, '\0',
	0x00, 0x00,
	0x00, '\0',
};

const unsigned char serialized_accept_1[] = {
	0x27, 0x15, 0x4a, 0x45,
	0x0f, 'v', 'c', 's', '.', 'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', 'c', 'o', 'm', '\0',
	0x0b, 0x35,
	0x0d, 't', 'e', 's', 't', '_', 'p', 'r', 'o', 't', 'o', 'c', 'o', 'l', '\0',
};

const unsigned char serialized_connect_tcp_empty[] = {
	0x9a, 0x15, 0x15, 0x3d,
	0x00, '\0',
	0x01, 0x00,
	0x00, '\0',
	0x00, '\0',
	0x00, 0x00,
	0x00, '\0',
	0x00, '\0',
	0x00,
	0x00, '\0',
	0x00, 0x00,
	0x00, '\0',
	0x00, '\0',
};

const unsigned char serialized_connect_tcp_1[] = {
	0x21, 0x32, 0x33, 0xe2,
	0x0f, 'v', 'c', 's', '.', 'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', 'c', 'o', 'm', '\0',
	0x0b, 0x35,
	0x0d, 't', 'e', 's', 't', '_', 'p', 'r', 'o', 't', 'o', 'c', 'o', 'l', '\0',
	0x0b, '1', '2', '.', '3', '4', '.', '5', '6', '.', '7', '8', '\0',
	0x31, 0xd4,
	0x05, 'a', 'l', 'i', 'c', 'e', '\0',
	0x08, 'p', '@', 's', 's', 'w', '0', 'r', 'd', '\0',
	0x0d,
	0x0b, '9', '8', '.', '7', '6', '.', '5', '4', '.', '3', '2', '\0',
	0x13, 0x43,
	0x03, 'b', 'o', 'b', '\0',
	0x09, 'q', 'w', 'e', 'r', 't', 'y', '1', '2', '3', '\0',
};

const unsigned char serialized_ep_accept[] = {
	0x86, 0x07, 0xcf, 0xc5,
	0x02, 0x00, 0x00, 0x00,
	0x26, 0x00, 0x00, 0x00,
	0x27, 0x15, 0x4a, 0x45,
	0x0f, 'v', 'c', 's', '.', 'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', 'c', 'o', 'm', '\0',
	0x0b, 0x35,
	0x0d, 't', 'e', 's', 't', '_', 'p', 'r', 'o', 't', 'o', 'c', 'o', 'l', '\0',
	0x25, 0x00, 0x00, 0x00,
	0x57, 0x28, 0x55, 0x44,
	0x0c, 's', 'e', 'r', 'v', 'e', 'r', '.', 'l', 'o', 'c', 'a', 'l', '\0',
	0x68, 0x60,
	0x0f, 's', 'e', 'c', 'o', 'n', 'd', 'a', 'r', 'y', '_', 'p', 'r', 'o', 't', 'o', '\0',
};

const unsigned char serialized_ep_connect[] = {
	0x58, 0x8b, 0x2c, 0x1f,
	0x02, 0x00, 0x00, 0x00,
	0x66, 0x00, 0x00, 0x00,
	0x21, 0x32, 0x33, 0xe2,
	0x0f, 'v', 'c', 's', '.', 'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', 'c', 'o', 'm', '\0',
	0x0b, 0x35,
	0x0d, 't', 'e', 's', 't', '_', 'p', 'r', 'o', 't', 'o', 'c', 'o', 'l', '\0',
	0x0b, '1', '2', '.', '3', '4', '.', '5', '6', '.', '7', '8', '\0',
	0x31, 0xd4,
	0x05, 'a', 'l', 'i', 'c', 'e', '\0',
	0x08, 'p', '@', 's', 's', 'w', '0', 'r', 'd', '\0',
	0x0d,
	0x0b, '9', '8', '.', '7', '6', '.', '5', '4', '.', '3', '2', '\0',
	0x13, 0x43,
	0x03, 'b', 'o', 'b', '\0',
	0x09, 'q', 'w', 'e', 'r', 't', 'y', '1', '2', '3', '\0',
	0x78, 0x00, 0x00, 0x00,
	0xfa, 0xb7, 0xd5, 0x31,
	0x0c, 's', 'e', 'r', 'v', 'e', 'r', '.', 'l', 'o', 'c', 'a', 'l', '\0',
	0x68, 0x60,
	0x0f, 's', 'e', 'c', 'o', 'n', 'd', 'a', 'r', 'y', '_', 'p', 'r', 'o', 't', 'o', '\0',
	0x11, 'h', 'o', 's', 't', '1', '.', 'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', 'c', 'o', 'm', '\0',
	0x39, 0x30,
	0x09, 'a', 'n', 'o', 'n', 'y', 'm', 'o', 'u', 's', '\0',
	0x09, 's', 'u', 'o', 'm', 'y', 'n', 'o', 'n', 'a', '\0',
	0x1f,
	0x11, 'h', 'o', 's', 't', '2', '.', 'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', 'c', 'o', 'm', '\0',
	0xea, 0x4e,
	0x08, 'u', 's', 'e', 'r', 'n', 'a', 'm', 'e', '\0',
	0x06, 'a', 'b', 'c', 'd', 'e', 'f',  '\0',
};

inline void CheckRegValue(VS_RegistryKey& key, const char* name, const char* value)
{
	std::string reg_value;
	if (key.GetString(reg_value, name))
		EXPECT_EQ(reg_value, value) << "Value of " << name << " is incorrect";
	else
		ADD_FAILURE() << name << " is missing";
}

inline void CheckRegValue(VS_RegistryKey& key, const char* name, int32_t value)
{
	int32_t buffer;
	if (key.GetValue(&buffer, sizeof(buffer), VS_REG_INTEGER_VT, name) > 0)
		EXPECT_EQ(buffer, value) << "Value of " << name << " is incorrect";
	else
		ADD_FAILURE() << name << " is missing";
}

inline void SetRegValue(VS_RegistryKey& key, const char* name, const char* value)
{
	EXPECT_TRUE(key.SetString(value, name));
}

inline void SetRegValue(VS_RegistryKey& key, const char* name, int32_t value)
{
	EXPECT_TRUE(key.SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, name));
}

TEST(EndpointRegistry, AcceptTCP_Serialize_Empty)
{
	net::endpoint::AcceptTCP x;

	const auto data = x.Serialize();
	ASSERT_EQ(data.size(), array_size(serialized_accept_empty));
	for (size_t i = 0; i < data.size(); ++i)
		EXPECT_EQ(data[i], serialized_accept_empty[i]) << "index=" << i;
}

TEST(EndpointRegistry, AcceptTCP_Deserialize_Empty)
{
	net::endpoint::AcceptTCP x;
	EXPECT_TRUE(x.Deserialize(serialized_accept_empty, array_size(serialized_accept_empty)));

	EXPECT_TRUE(x.host.empty());
	EXPECT_EQ(x.port, 0);
	EXPECT_TRUE(x.protocol_name.empty());
}

TEST(EndpointRegistry, AcceptTCP_Serialize)
{
	net::endpoint::AcceptTCP x{ host_1, port_1, protocol_1 };

	const auto data = x.Serialize();
	ASSERT_EQ(data.size(), array_size(serialized_accept_1));
	for (size_t i = 0; i < data.size(); ++i)
		EXPECT_EQ(data[i], serialized_accept_1[i]) << "index=" << i;
}

TEST(EndpointRegistry, AcceptTCP_Deserialize)
{
	using ::testing::StrEq;

	net::endpoint::AcceptTCP x;
	EXPECT_TRUE(x.Deserialize(serialized_accept_1, array_size(serialized_accept_1)));

	EXPECT_THAT(x.host, StrEq(host_1));
	EXPECT_EQ(x.port, port_1);
	EXPECT_THAT(x.protocol_name, StrEq(protocol_1));
}

TEST(EndpointRegistry, AcceptTCP_Deserialize_BadCheckSum)
{
	net::endpoint::AcceptTCP x;
	ASSERT_FALSE(x.Deserialize(serialized_bad_chksum, array_size(serialized_bad_chksum)));
}

TEST(EndpointRegistry, AcceptTCP_Deserialize_BadFormat)
{
	net::endpoint::AcceptTCP x;
	ASSERT_FALSE(x.Deserialize(serialized_bad_format_1, array_size(serialized_bad_format_1)));
}

TEST(EndpointRegistry, AcceptUDP_Serialize_Empty)
{
	net::endpoint::AcceptUDP x;

	const auto data = x.Serialize();
	ASSERT_EQ(data.size(), array_size(serialized_accept_empty));
	for (size_t i = 0; i < data.size(); ++i)
		EXPECT_EQ(data[i], serialized_accept_empty[i]) << "index=" << i;
}

TEST(EndpointRegistry, AcceptUDP_Deserialize_Empty)
{
	net::endpoint::AcceptUDP x;
	EXPECT_TRUE(x.Deserialize(serialized_accept_empty, array_size(serialized_accept_empty)));

	EXPECT_TRUE(x.host.empty());
	EXPECT_EQ(x.port, 0);
	EXPECT_TRUE(x.protocol_name.empty());
}

TEST(EndpointRegistry, AcceptUDP_Serialize)
{
	net::endpoint::AcceptUDP x{ host_1, port_1, protocol_1 };

	const auto data = x.Serialize();
	ASSERT_EQ(data.size(), array_size(serialized_accept_1));
	for (size_t i = 0; i < data.size(); ++i)
		EXPECT_EQ(data[i], serialized_accept_1[i]) << "index=" << i;
}

TEST(EndpointRegistry, AcceptUDP_Deserialize)
{
	using ::testing::StrEq;

	net::endpoint::AcceptUDP x;
	EXPECT_TRUE(x.Deserialize(serialized_accept_1, array_size(serialized_accept_1)));

	EXPECT_THAT(x.host, StrEq(host_1));
	EXPECT_EQ(x.port, port_1);
	EXPECT_THAT(x.protocol_name, StrEq(protocol_1));
}

TEST(EndpointRegistry, AcceptUDP_Deserialize_BadCheckSum)
{
	net::endpoint::AcceptUDP x;
	ASSERT_FALSE(x.Deserialize(serialized_bad_chksum, array_size(serialized_bad_chksum)));
}

TEST(EndpointRegistry, AcceptUDP_Deserialize_BadFormat)
{
	net::endpoint::AcceptUDP x;
	ASSERT_FALSE(x.Deserialize(serialized_bad_format_1, array_size(serialized_bad_format_1)));
}

TEST(EndpointRegistry, ConnectTCP_Serialize_Empty)
{
	net::endpoint::ConnectTCP x;
	x.port = 1;

	const auto data = x.Serialize();
	ASSERT_EQ(data.size(), array_size(serialized_connect_tcp_empty));
	for (size_t i = 0; i < data.size(); ++i)
		EXPECT_EQ(data[i], serialized_connect_tcp_empty[i]) << "index=" << i;
}

TEST(EndpointRegistry, ConnectTCP_Deserialize_Empty)
{
	net::endpoint::ConnectTCP x;
	EXPECT_TRUE(x.Deserialize(serialized_connect_tcp_empty, array_size(serialized_connect_tcp_empty)));

	EXPECT_TRUE(x.host.empty());
	EXPECT_EQ(x.port, 1);
	EXPECT_TRUE(x.protocol_name.empty());

	EXPECT_TRUE(x.socks_host.empty());
	EXPECT_EQ(x.socks_port, 0);
	EXPECT_TRUE(x.socks_user.empty());
	EXPECT_TRUE(x.socks_password.empty());
	EXPECT_EQ(x.socks_version, 0);

	EXPECT_TRUE(x.http_host.empty());
	EXPECT_EQ(x.http_port, 0);
	EXPECT_TRUE(x.http_user.empty());
	EXPECT_TRUE(x.http_password.empty());
}

TEST(EndpointRegistry, ConnectTCP_Serialize)
{
	net::endpoint::ConnectTCP x{ host_1, port_1, protocol_1, socks_host_1, socks_port_1, socks_user_1, socks_password_1, socks_version_1, http_host_1, http_port_1, http_user_1, http_password_1 };

	const auto data = x.Serialize();
	ASSERT_EQ(data.size(), array_size(serialized_connect_tcp_1));
	for (size_t i = 0; i < data.size(); ++i)
		EXPECT_EQ(data[i], serialized_connect_tcp_1[i]) << "index=" << i;
}

TEST(EndpointRegistry, ConnectTCP_Deserialize)
{
	using ::testing::StrEq;

	net::endpoint::ConnectTCP x;
	EXPECT_TRUE(x.Deserialize(serialized_connect_tcp_1, array_size(serialized_connect_tcp_1)));

	EXPECT_THAT(x.host, StrEq(host_1));
	EXPECT_EQ(x.port, port_1);
	EXPECT_THAT(x.protocol_name, StrEq(protocol_1));

	EXPECT_THAT(x.socks_host, StrEq(socks_host_1));
	EXPECT_EQ(x.socks_port, socks_port_1);
	EXPECT_THAT(x.socks_user, StrEq(socks_user_1));
	EXPECT_THAT(x.socks_password, StrEq(socks_password_1));
	EXPECT_EQ(x.socks_version, socks_version_1);

	EXPECT_THAT(x.http_host, StrEq(http_host_1));
	EXPECT_EQ(x.http_port, http_port_1);
	EXPECT_THAT(x.http_user, StrEq(http_user_1));
	EXPECT_THAT(x.http_password, StrEq(http_password_1));
}

TEST(EndpointRegistry, ConnectTCP_Deserialize_Short)
{
	using ::testing::StrEq;

	net::endpoint::ConnectTCP x;
	EXPECT_TRUE(x.Deserialize(serialized_accept_1, array_size(serialized_accept_1)));

	EXPECT_THAT(x.host, StrEq(host_1));
	EXPECT_EQ(x.port, port_1);
	EXPECT_THAT(x.protocol_name, StrEq(protocol_1));

	EXPECT_TRUE(x.socks_host.empty());
	EXPECT_EQ(x.socks_port, 0);
	EXPECT_TRUE(x.socks_user.empty());
	EXPECT_TRUE(x.socks_password.empty());
	EXPECT_EQ(x.socks_version, 0);

	EXPECT_TRUE(x.http_host.empty());
	EXPECT_EQ(x.http_port, 0);
	EXPECT_TRUE(x.http_user.empty());
	EXPECT_TRUE(x.http_password.empty());
}

TEST(EndpointRegistry, ConnectTCP_Deserialize_BadCheckSum)
{
	net::endpoint::ConnectTCP x;
	ASSERT_FALSE(x.Deserialize(serialized_bad_chksum, array_size(serialized_bad_chksum)));
}

TEST(EndpointRegistry, ConnectTCP_Deserialize_BadFormat_1)
{
	net::endpoint::ConnectTCP x;
	ASSERT_FALSE(x.Deserialize(serialized_bad_format_1, array_size(serialized_bad_format_1)));
}

TEST(EndpointRegistry, ConnectTCP_Deserialize_BadFormat_2)
{
	net::endpoint::ConnectTCP x;
	ASSERT_FALSE(x.Deserialize(serialized_bad_format_2, array_size(serialized_bad_format_2)));
}

TEST(EndpointRegistry, AcceptTCP_Store)
{
	VS_SCOPE_EXIT{ net::endpoint::Remove(ep_1); };
	net::endpoint::Remove(ep_1);
	EXPECT_EQ(1u, net::endpoint::AddAcceptTCP({ host_1, port_1, protocol_1 }, ep_1, false));

	std::string key_name;
	key_name += "Endpoints\\";
	key_name += ep_1;
	key_name += "\\AcceptTCP1";
	VS_RegistryKey key(false, key_name);
	ASSERT_TRUE(key.IsValid()) << "Key " << key_name << " is missing";

	CheckRegValue(key, "Host", host_1);
	CheckRegValue(key, "Port", port_1);
	CheckRegValue(key, "Protocol", protocol_1);
}

TEST(EndpointRegistry, AcceptTCP_Load)
{
	using ::testing::StrEq;

	VS_SCOPE_EXIT{ VS_RegistryKey(false, "Endpoints", false).RemoveKey(ep_1); };
	std::string key_name;
	key_name += "Endpoints\\";
	key_name += ep_1;
	key_name += "\\AcceptTCP1";
	VS_RegistryKey key(false, key_name, false, true);
	ASSERT_TRUE(key.IsValid()) << "Unable to create key " << key_name;

	SetRegValue(key, "Host", host_1);
	SetRegValue(key, "Port", port_1);
	SetRegValue(key, "Protocol", protocol_1);

	auto x = net::endpoint::ReadAcceptTCP(1, ep_1, false);
	ASSERT_TRUE(static_cast<bool>(x));
	EXPECT_THAT(x->host, StrEq(host_1));
	EXPECT_EQ(x->port, port_1);
	EXPECT_THAT(x->protocol_name, StrEq(protocol_1));
}

TEST(EndpointRegistry, AcceptTCP_MakeFirst)
{
	VS_SCOPE_EXIT{ net::endpoint::Remove(ep_1); };
	net::endpoint::Remove(ep_1);
	EXPECT_EQ(1u, net::endpoint::AddAcceptTCP({ host_1, port_1, protocol_1 }, ep_1, false));
	EXPECT_EQ(2u, net::endpoint::AddAcceptTCP({ host_2, port_2, protocol_2 }, ep_1, false));

	net::endpoint::MakeFirstAcceptTCP(2, ep_1, false);

	std::string key_name;
	key_name += "Endpoints\\";
	key_name += ep_1;
	key_name += "\\AcceptTCP1";
	VS_RegistryKey key(false, key_name);
	ASSERT_TRUE(key.IsValid()) << "Key " << key_name << " is missing";

	CheckRegValue(key, "Host", host_2);
	CheckRegValue(key, "Port", port_2);
	CheckRegValue(key, "Protocol", protocol_2);
}

TEST(EndpointRegistry, AcceptUDP_Store)
{
	VS_SCOPE_EXIT{ net::endpoint::Remove(ep_1); };
	net::endpoint::Remove(ep_1);
	EXPECT_EQ(1u, net::endpoint::AddAcceptUDP({ host_1, port_1, protocol_1 }, ep_1, false));

	std::string key_name;
	key_name += "Endpoints\\";
	key_name += ep_1;
	key_name += "\\AcceptUDP1";
	VS_RegistryKey key(false, key_name);
	ASSERT_TRUE(key.IsValid()) << "Key " << key_name << " is missing";

	CheckRegValue(key, "Host", host_1);
	CheckRegValue(key, "Port", port_1);
	CheckRegValue(key, "Protocol", protocol_1);
}

TEST(EndpointRegistry, AcceptUDP_Load)
{
	using ::testing::StrEq;

	VS_SCOPE_EXIT{ VS_RegistryKey(false, "Endpoints", false).RemoveKey(ep_1); };
	std::string key_name;
	key_name += "Endpoints\\";
	key_name += ep_1;
	key_name += "\\AcceptUDP1";
	VS_RegistryKey key(false, key_name, false, true);
	ASSERT_TRUE(key.IsValid()) << "Unable to create key " << key_name;

	SetRegValue(key, "Host", host_1);
	SetRegValue(key, "Port", port_1);
	SetRegValue(key, "Protocol", protocol_1);

	auto x = net::endpoint::ReadAcceptUDP(1, ep_1, false);
	ASSERT_TRUE(static_cast<bool>(x));
	EXPECT_THAT(x->host, StrEq(host_1));
	EXPECT_EQ(x->port, port_1);
	EXPECT_THAT(x->protocol_name, StrEq(protocol_1));
}

TEST(EndpointRegistry, AcceptUDP_MakeFirst)
{
	VS_SCOPE_EXIT{ net::endpoint::Remove(ep_1); };
	net::endpoint::Remove(ep_1);
	EXPECT_EQ(1u, net::endpoint::AddAcceptUDP({ host_1, port_1, protocol_1 }, ep_1, false));
	EXPECT_EQ(2u, net::endpoint::AddAcceptUDP({ host_2, port_2, protocol_2 }, ep_1, false));

	net::endpoint::MakeFirstAcceptUDP(2, ep_1, false);

	std::string key_name;
	key_name += "Endpoints\\";
	key_name += ep_1;
	key_name += "\\AcceptUDP1";
	VS_RegistryKey key(false, key_name);
	ASSERT_TRUE(key.IsValid()) << "Key " << key_name << " is missing";

	CheckRegValue(key, "Host", host_2);
	CheckRegValue(key, "Port", port_2);
	CheckRegValue(key, "Protocol", protocol_2);
}


TEST(EndpointRegistry, ConnectTCP_Store)
{
	VS_SCOPE_EXIT{ net::endpoint::Remove(ep_1); };
	net::endpoint::Remove(ep_1);
	EXPECT_EQ(1u, net::endpoint::AddConnectTCP({ host_1, port_1, protocol_1, socks_host_1, socks_port_1, socks_user_1, socks_password_1, socks_version_1, http_host_1, http_port_1, http_user_1, http_password_1 }, ep_1, false));

	std::string key_name;
	key_name += "Endpoints\\";
	key_name += ep_1;
	key_name += "\\ConnectTCP1";
	VS_RegistryKey key(false, key_name);
	ASSERT_TRUE(key.IsValid()) << "Key " << key_name << " is missing";

	CheckRegValue(key, "Host", host_1);
	CheckRegValue(key, "Port", port_1);
	CheckRegValue(key, "Protocol", protocol_1);

	CheckRegValue(key, "Socks", socks_host_1);
	CheckRegValue(key, "Socks Port", socks_port_1);
	CheckRegValue(key, "Socks Version", socks_version_1);

	CheckRegValue(key, "HTTP Proxy", http_host_1);
	CheckRegValue(key, "HTTP Proxy Port", http_port_1);

	// user/password pairs for socks and http proxies are stored encrypted, this is hard to test.
}

TEST(EndpointRegistry, ConnectTCP_Load)
{
	using ::testing::StrEq;

	VS_SCOPE_EXIT{ VS_RegistryKey(false, "Endpoints", false).RemoveKey(ep_1); };
	std::string key_name;
	key_name += "Endpoints\\";
	key_name += ep_1;
	key_name += "\\ConnectTCP1";
	VS_RegistryKey key(false, key_name, false, true);
	ASSERT_TRUE(key.IsValid()) << "Unable to create key " << key_name;

	SetRegValue(key, "Host", host_1);
	SetRegValue(key, "Port", port_1);
	SetRegValue(key, "Protocol", protocol_1);

	SetRegValue(key, "Socks", socks_host_1);
	SetRegValue(key, "Socks Port", socks_port_1);
	SetRegValue(key, "Socks User", socks_user_1);
	SetRegValue(key, "Socks Password", socks_password_1);
	SetRegValue(key, "Socks Version", socks_version_1);

	SetRegValue(key, "HTTP Proxy", http_host_1);
	SetRegValue(key, "HTTP Proxy Port", http_port_1);
	SetRegValue(key, "HTTP Proxy User", http_user_1);
	SetRegValue(key, "HTTP Proxy Password", http_password_1);

	auto x = net::endpoint::ReadConnectTCP(1, ep_1, false);
	ASSERT_TRUE(static_cast<bool>(x));
	EXPECT_THAT(x->host, StrEq(host_1));
	EXPECT_EQ(x->port, port_1);
	EXPECT_THAT(x->protocol_name, StrEq(protocol_1));

	EXPECT_THAT(x->socks_host, StrEq(socks_host_1));
	EXPECT_EQ(x->socks_port, socks_port_1);
	EXPECT_THAT(x->socks_user, StrEq(socks_user_1));
	EXPECT_THAT(x->socks_password, StrEq(socks_password_1));
	EXPECT_EQ(x->socks_version, socks_version_1);

	EXPECT_THAT(x->http_host, StrEq(http_host_1));
	EXPECT_EQ(x->http_port, http_port_1);
	EXPECT_THAT(x->http_user, StrEq(http_user_1));
	EXPECT_THAT(x->http_password, StrEq(http_password_1));
}

TEST(EndpointRegistry, ConnectTCP_MakeFirst)
{
	VS_SCOPE_EXIT{ net::endpoint::Remove(ep_1); };
	net::endpoint::Remove(ep_1);
	EXPECT_EQ(1u, net::endpoint::AddConnectTCP({ host_1, port_1, protocol_1, socks_host_1, socks_port_1, socks_user_1, socks_password_1, socks_version_1, http_host_1, http_port_1, http_user_1, http_password_1 }, ep_1, false));
	EXPECT_EQ(2u, net::endpoint::AddConnectTCP({ host_2, port_2, protocol_2, socks_host_2, socks_port_2, socks_user_2, socks_password_2, socks_version_2, http_host_2, http_port_2, http_user_2, http_password_2 }, ep_1, false));

	net::endpoint::MakeFirstConnectTCP(2, ep_1, false);

	std::string key_name;
	key_name += "Endpoints\\";
	key_name += ep_1;
	key_name += "\\ConnectTCP1";
	VS_RegistryKey key(false, key_name);
	ASSERT_TRUE(key.IsValid()) << "Key " << key_name << " is missing";

	CheckRegValue(key, "Host", host_2);
	CheckRegValue(key, "Port", port_2);
	CheckRegValue(key, "Protocol", protocol_2);

	CheckRegValue(key, "Socks", socks_host_2);
	CheckRegValue(key, "Socks Port", socks_port_2);
	CheckRegValue(key, "Socks Version", socks_version_2);

	CheckRegValue(key, "HTTP Proxy", http_host_2);
	CheckRegValue(key, "HTTP Proxy Port", http_port_2);

	// user/password pairs for socks and http proxies are stored encrypted, this is hard to test.
}

TEST(EndpointRegistry, Serialize_Accept)
{
	VS_SCOPE_EXIT{ net::endpoint::Remove(ep_1); };
	net::endpoint::Remove(ep_1);
	EXPECT_EQ(1u, net::endpoint::AddAcceptTCP({ host_1, port_1, protocol_1 }, ep_1, false));
	EXPECT_EQ(2u, net::endpoint::AddAcceptTCP({ host_2, port_2, protocol_2 }, ep_1, false));

	const auto data = net::endpoint::Serialize(false, ep_1, false);
	ASSERT_EQ(data.size(), array_size(serialized_ep_accept));
	for (size_t i = 0; i < data.size(); ++i)
		EXPECT_EQ(data[i], serialized_ep_accept[i]) << "index=" << i;
}

TEST(EndpointRegistry, Deserialize_Accept_1)
{
	using ::testing::StrEq;

	net::endpoint::AcceptTCP x;
	EXPECT_TRUE(net::endpoint::GetFromBuffer(0, x, serialized_ep_accept, array_size(serialized_ep_accept)));

	EXPECT_THAT(x.host, StrEq(host_1));
	EXPECT_EQ(x.port, port_1);
	EXPECT_THAT(x.protocol_name, StrEq(protocol_1));
}

TEST(EndpointRegistry, Deserialize_Accept_2)
{
	using ::testing::StrEq;

	net::endpoint::AcceptTCP x;
	EXPECT_TRUE(net::endpoint::GetFromBuffer(1, x, serialized_ep_accept, array_size(serialized_ep_accept)));

	EXPECT_THAT(x.host, StrEq(host_2));
	EXPECT_EQ(x.port, port_2);
	EXPECT_THAT(x.protocol_name, StrEq(protocol_2));
}

TEST(EndpointRegistry, Serialize_Connect)
{
	VS_SCOPE_EXIT{ net::endpoint::Remove(ep_1); };
	net::endpoint::Remove(ep_1);
	EXPECT_EQ(1u, net::endpoint::AddConnectTCP({ host_1, port_1, protocol_1, socks_host_1, socks_port_1, socks_user_1, socks_password_1, socks_version_1, http_host_1, http_port_1, http_user_1, http_password_1 }, ep_1, false));
	EXPECT_EQ(2u, net::endpoint::AddConnectTCP({ host_2, port_2, protocol_2, socks_host_2, socks_port_2, socks_user_2, socks_password_2, socks_version_2, http_host_2, http_port_2, http_user_2, http_password_2 }, ep_1, false));

	const auto data = net::endpoint::Serialize(true, ep_1, false);
	ASSERT_EQ(data.size(), array_size(serialized_ep_connect));
	for (size_t i = 0; i < data.size(); ++i)
		EXPECT_EQ(data[i], serialized_ep_connect[i]) << "index=" << i;
}

TEST(EndpointRegistry, Deserialize_Connect_1)
{
	using ::testing::StrEq;

	net::endpoint::ConnectTCP x;
	EXPECT_TRUE(net::endpoint::GetFromBuffer(0, x, serialized_ep_connect, array_size(serialized_ep_connect)));

	EXPECT_THAT(x.host, StrEq(host_1));
	EXPECT_EQ(x.port, port_1);
	EXPECT_THAT(x.protocol_name, StrEq(protocol_1));

	EXPECT_THAT(x.socks_host, StrEq(socks_host_1));
	EXPECT_EQ(x.socks_port, socks_port_1);
	EXPECT_THAT(x.socks_user, StrEq(socks_user_1));
	EXPECT_THAT(x.socks_password, StrEq(socks_password_1));
	EXPECT_EQ(x.socks_version, socks_version_1);

	EXPECT_THAT(x.http_host, StrEq(http_host_1));
	EXPECT_EQ(x.http_port, http_port_1);
	EXPECT_THAT(x.http_user, StrEq(http_user_1));
	EXPECT_THAT(x.http_password, StrEq(http_password_1));
}

TEST(EndpointRegistry, Deserialize_Connect_2)
{
	using ::testing::StrEq;

	net::endpoint::ConnectTCP x;
	EXPECT_TRUE(net::endpoint::GetFromBuffer(1, x, serialized_ep_connect, array_size(serialized_ep_connect)));

	EXPECT_THAT(x.host, StrEq(host_2));
	EXPECT_EQ(x.port, port_2);
	EXPECT_THAT(x.protocol_name, StrEq(protocol_2));

	EXPECT_THAT(x.socks_host, StrEq(socks_host_2));
	EXPECT_EQ(x.socks_port, socks_port_2);
	EXPECT_THAT(x.socks_user, StrEq(socks_user_2));
	EXPECT_THAT(x.socks_password, StrEq(socks_password_2));
	EXPECT_EQ(x.socks_version, socks_version_2);

	EXPECT_THAT(x.http_host, StrEq(http_host_2));
	EXPECT_EQ(x.http_port, http_port_2);
	EXPECT_THAT(x.http_user, StrEq(http_user_2));
	EXPECT_THAT(x.http_password, StrEq(http_password_2));
}

}
