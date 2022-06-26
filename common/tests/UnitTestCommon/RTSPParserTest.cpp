#include "SIPParserLib/VS_RTSP_Transport.h"
#include "SIPParserLib/VS_RTSP_ParserInfo.h"
#include "std-generic/cpplib/ignore.h"
#include "std/cpplib/MakeShared.h"
#include "tests/common/ASIOEnvironment.h"
#include "TrueGateway/CallConfig/VS_CallConfigStorage.h"
#include "TrueGateway/CallConfig/VS_IndentifierRTSP.h"
#include "GTestPrinters.h"

#include <gtest/gtest.h>

namespace rtsp_parser_test {

#if defined(_WIN32) //not ported

struct RTSPtest : public ::testing::Test {
	boost::asio::ip::address my_address = boost::asio::ip::address::from_string("1.2.3.4", vs::ignore<boost::system::error_code>());
	net::port port = 123;
	VS_RTSP_ParserInfo i;

	void SetUp() override {
		boost::asio::ip::udp::endpoint ep(my_address, port);
		i.SetEndpoint(ep);
	}

	std::string MakeTransportField() {
		VS_RTSP_Transport transportField;
		transportField.Init(&i);

		VS_SIPBuffer b;
		EXPECT_EQ(transportField.Encode(b), TSIPErrorCodes::e_ok);

		char data[256] = {};
		auto writeSize = b.GetWriteIndex();
		EXPECT_EQ(b.GetData(data, writeSize), TSIPErrorCodes::e_ok);

		return std::string(data, writeSize);
	}
};

TEST_F(RTSPtest, SetDestinationUsingLocalTransceiver) {
	i.UseRemoteTransceiver(false);
	auto transportFieldStr = MakeTransportField();
	EXPECT_EQ(transportFieldStr.find("destination"),string_view::npos);
}

TEST_F(RTSPtest, SetDestinationUsingRemoteTransceiver) {
	i.UseRemoteTransceiver(true);
	auto transportFieldStr = MakeTransportField();
	ASSERT_NE(transportFieldStr.find("destination"), string_view::npos);

	auto ipBegin = transportFieldStr.find("destination=") + sizeof("destination=") - 1;
	auto ipEnd = transportFieldStr.find(';', ipBegin);
	ASSERT_NE(ipEnd, string_view::npos);

	auto ip = transportFieldStr.substr(ipBegin, ipEnd - ipBegin);
	auto addr = boost::asio::ip::address::from_string(ip, vs::ignore<boost::system::error_code>());

	EXPECT_FALSE(addr.is_unspecified());
	EXPECT_EQ(addr, my_address);
}

#endif

static const struct ResolveTestParam
{
	string_view cid_1;
	string_view cid_1_resolved;
	string_view cid_2;
	string_view cid_2_resolved;
} resolve_params[] = {
	{ "#rtsp:%2f/127.0.0.1%2ffirst",  "#rtsp:%2f%2f127.0.0.1%2ffirst",
	  "#rtsp:/%2f127.0.0.1%2fsecond", "#rtsp:%2f%2f127.0.0.1%2fsecond" },
	{ "#rtsp:%2f/127.0.0.1:554%2ffirst",  "#rtsp:%2f%2f127.0.0.1:554%2ffirst",
	  "#rtsp:/%2f127.0.0.1:544%2fsecond", "#rtsp:%2f%2f127.0.0.1:544%2fsecond" },
	{ "#rtsp:%2f/127.0.0.1:1554%2ffirst",  "#rtsp:%2f%2f127.0.0.1:1554%2ffirst",
	  "#rtsp:/%2f127.0.0.1:5440%2fsecond", "#rtsp:%2f%2f127.0.0.1:5440%2fsecond" },
	{ "#rtsp:%2f/user:pass@127.0.0.1%2ffirst",  "#rtsp:%2f%2f127.0.0.1%2ffirst",
	  "#rtsp:/%2fuser:pass@127.0.0.1%2fsecond", "#rtsp:%2f%2f127.0.0.1%2fsecond" },
	{ "#rtsp:%2f/user:pass@127.0.0.1:1554%2ffirst",  "#rtsp:%2f%2f127.0.0.1:1554%2ffirst",
	  "#rtsp:/%2fuser:pass@127.0.0.1:5440%2fsecond", "#rtsp:%2f%2f127.0.0.1:5440%2fsecond" },
};
std::ostream& operator<<(std::ostream& os, const ResolveTestParam& x)
{
	return os << '"' << x.cid_1 << "\" and \"" << x.cid_2 << '"';
}
struct RTSPResolveTest : public ::testing::TestWithParam<ResolveTestParam>
{
	boost::shared_ptr<VS_IndentifierRTSP> identifier;
	std::shared_ptr<VS_CallConfigStorage> call_config;
	VS_CallConfig cfg_1;
	VS_CallConfig cfg_2;
	VS_CallConfig cfg_1_2;

	RTSPResolveTest()
		: identifier(boost::make_shared<VS_IndentifierRTSP>(g_asio_environment->IOService()))
		, call_config(vs::MakeShared<VS_CallConfigStorage>())
	{
		call_config->RegisterProtocol(identifier);
	}
};
INSTANTIATE_TEST_CASE_P(RTSPResolve, RTSPResolveTest, ::testing::ValuesIn(resolve_params));

TEST_P(RTSPResolveTest, SimilarURL)
{
	EXPECT_TRUE(call_config->Resolve(cfg_1, GetParam().cid_1, nullptr));
	EXPECT_EQ(cfg_1.resolveResult.NewCallId, GetParam().cid_1_resolved);
	EXPECT_TRUE(call_config->Resolve(cfg_2, GetParam().cid_2, nullptr));
	EXPECT_EQ(cfg_2.resolveResult.NewCallId, GetParam().cid_2_resolved);
	EXPECT_TRUE(call_config->Resolve(cfg_1_2, GetParam().cid_1, nullptr));
	EXPECT_EQ(cfg_1_2.resolveResult.NewCallId, GetParam().cid_1_resolved);
}

static const struct CallIDTestParam
{
	string_view call_id;
	string_view login;
	string_view password;
	string_view hostname;
	string_view port;
	string_view path;
} call_id_params[] = {
	{ "#rtsp://127.0.0.1", "", "", "127.0.0.1", "", "" },
	{ "#rtsp://[fe80::1234:5678:abcde]", "", "", "[fe80::1234:5678:abcde]", "", "" },
	{ "#rtsp://domain.tld", "", "", "domain.tld", "", "" },
	{ "#RtsP://127.0.0.1", "", "", "127.0.0.1", "", "" },
	{ "#rtsp:%2f%2f127.0.0.1", "", "", "127.0.0.1", "", "" },
	{ "#rtsp://127.0.0.1:1554", "", "", "127.0.0.1", "1554", "" },
	{ "#rtsp://[fe80::1234:5678:abcde]:2554", "", "", "[fe80::1234:5678:abcde]", "2554", "" },
	{ "#rtsp://127.0.0.1/some/path?opt=123", "", "", "127.0.0.1", "", "/some/path?opt=123" },
	{ "#rtsp://user@127.0.0.1", "user", "", "127.0.0.1", "", "" },
	{ "#rtsp://user:@127.0.0.1", "user", "", "127.0.0.1", "", "" },
	{ "#rtsp://user:pass@127.0.0.1", "user", "pass", "127.0.0.1", "", "" },
	{ "#rTSp:/%2fuser:pass@[fe80::1234:5678:abcde]:1554/some%2fpath?opt=123", "user", "pass", "[fe80::1234:5678:abcde]", "1554", "/some/path?opt=123" },
	{ "#rtsp:127.0.0.1", "", "", "127.0.0.1", "", "" },
	// Failure cases
	{ "#rtsp://", "", "", "", "", "" },
	{ "#rtsp:/127.0.0.1", "", "", "", "", "" },
	{ "#rtsp://127.0.0.1:string", "", "", "", "", "" },
	{ "#sip:user@127.0.0.1", "", "", "", "", "" },
};
std::ostream& operator<<(std::ostream& os, const CallIDTestParam& x)
{
	return os << '"' << x.call_id << '"';
}
struct CallIDTest : public ::testing::TestWithParam<CallIDTestParam> {};
INSTANTIATE_TEST_CASE_P(RTSP, CallIDTest, ::testing::ValuesIn(call_id_params));

TEST_P(CallIDTest, Parse)
{
	VS_IndentifierRTSP::RTSPCallID call_id(GetParam().call_id);
	EXPECT_EQ(call_id.login,    GetParam().login);
	EXPECT_EQ(call_id.password, GetParam().password);
	EXPECT_EQ(call_id.hostname, GetParam().hostname);
	EXPECT_EQ(call_id.port,     GetParam().port);
	EXPECT_EQ(call_id.path,     GetParam().path);
}

}
