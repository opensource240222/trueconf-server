#include "../../FakeClient/VS_ConferenceInfo.h"
#include "VS_ConferenceProtocolMock.h"
#include "VS_ConferenceProtocolFake.h"
#include "VS_IdentifierMock.h"
#include "VS_SignalChannelMock.h"
#include "VS_SignalChannelFake.h"
#include "GTestPrinters.h"
#include "SIPParserLib/VS_SIPMessage.h"
#include "SIPParserLib/VS_SIPRequest.h"
#include "TrueGateway/sip/VS_SIPParserInfo.h"
#include "SIPParserLib/VS_SIPAuthDigest.h"
#include "SIPParserLib/VS_SIPAuthGSS.h"
#include "SIPParserLib/VS_SIPMetaField.h"
#include "SIPParserLib/VS_SIPField_Via.h"
#include "SIPParserLib/VS_SIPField_CSeq.h"
#include "SIPParserLib/VS_SIPField_From.h"
#include "SIPParserLib/VS_SIPField_To.h"
#include "SIPParserLib/VS_SIPField_Contact.h"
#include "SIPParserLib/VS_SIPField_CallID.h"
#include "SIPParserLib/VS_SIPField_ContentType.h"
#include "SIPParserLib/VS_SIPField_ContentLength.h"
#include "SIPParserLib/VS_SIPField_RecordRoute.h"
#include "SIPParserLib/VS_SIPURI.h"
#include "SIPParserLib/VS_SIPField_StartLine.h"
#include "SIPParserLib/VS_SIPField_Auth.h"
#include "SIPParserLib/VS_SIPField_Expires.h"
#include "SIPParserLib/VS_SIPField_SessionExpires.h"
#include "SIPParserLib/VS_SIPField_Event.h"
#include "SIPParserLib/VS_SIPResponse.h"
#include "SIPParserLib/VS_SDPMetaField.h"
#include "SIPParserLib/VS_SDPField_MediaStream.h"
#include "TrueGateway/VS_GatewayStarter.h"
#include "TrueGateway/net/VS_SignalChannel.h"
#include "TrueGateway/sip/VS_SIPParser.h"
#include "TrueGateway/sip/SIPTransportLayer.h"
#include "TrueGateway/sip/SIPTransportChannel.h"
#include "TrueGateway/CallConfig/VS_CallConfig.h"
#include "TrueGateway/CallConfig/VS_CallConfigStorage.h"
#include "TrueGateway/CallConfig/VS_IndentifierSIP.h"
#include "FakeSIPChannel.h"
#include "tools/SingleGatewayLib/VS_H264ResolutionCalc.h"
#include "std/cpplib/VS_Policy.h"
#include "std/cpplib/VS_UserData.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/string_view.h"
#include "VS_IdentifierSIPTestImpl.h"
#include "tests/common/Utils.h"
#include "TestSIPTransportLayer.h"

#include <gtest/gtest.h>

#include <boost/asio/io_service.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "std/cpplib/MakeShared.h"

extern void strreplace(std::string &str, string_view _old, string_view _new);
extern std::string CombineInviteAndSDP(string_view inv, string_view sdp);
extern void ConstructResponse(std::string &response, string_view orig, string_view callId, string_view branch);

class SIPParserExtends : public VS_SIPParser
{
public:
	using VS_SIPParser::clock;
protected:
	template<typename ...Args>
	SIPParserExtends(Args&& ... args) : VS_SIPParser(std::forward<Args>(args)...)
	{}

	static void PostConstruct(std::shared_ptr<SIPParserExtends>&) { /*stub*/ }
};

template <typename T>
class SIPParserTestBase : public T
{
public:
	SIPParserTestBase()
		: strand(g_asio_environment->IOService())
		, vcs_addr{ net::address_v4(1), 1, net::protocol::UDP }
		, terminal_addr(vcs_addr)
		, confProtocol(std::make_shared<VS_ConferenceProtocolMock>())
	{
		InitOurEndpointName();
	}

	SIPParserTestBase(
		net::Endpoint vcsEp,
		net::Endpoint terminalEp)
		: strand(g_asio_environment->IOService())
		, vcs_addr{ vcsEp }
		, terminal_addr{ terminalEp }
		, confProtocol(std::make_shared<VS_ConferenceProtocolMock>())
	{
		InitOurEndpointName();
	}

	void InitOurEndpointName() {
		our_endpoint = g_tr_endpoint_name;
		if (our_endpoint.find('#') != std::string::npos) our_endpoint.erase(our_endpoint.find('#'));
	}

	virtual void SetUp() {
		using ::testing::_;
		using ::testing::Invoke;
		using ::testing::NiceMock;

		sip = vs::MakeShared<SIPParserExtends>(strand, "serverVedor", nullptr);
		sip->UseACL(false);

		sip->SetMyCsAddress(vcs_addr);
		sip->SetPeerCSAddress({}, terminal_addr);

		//identifier_impl = VS_Indentifier::GetCommonIndentifier();
		identifier_impl = boost::make_shared<VS_IndentifierSIP>(g_asio_environment->IOService(), "serverVendor");
		identifier = boost::make_shared<NiceMock<VS_IndentifierMock>>(g_asio_environment->IOService());
		identifier->DelegateTo(identifier_impl.get());
		call_config = vs::MakeShared<VS_CallConfigStorage>();
		call_config->RegisterProtocol(identifier);

		sip->SetCallConfigStorage(call_config);
		sip->SetConfCallBack(confProtocol);
		sip->SetPolicy(boost::make_shared<VS_Policy>("SIP"));

		confProtocol_fake.parser = sip.get();
		confProtocol->DelegateTo(&confProtocol_fake);

		sip_transport = std::make_shared<test::SIPTransportLayer>(strand, sip, call_config, vcs_addr, terminal_addr);
		sip->Connect_GetSRTPKey([this](string_view dialog, const net::Endpoint &bindEp, const net::Endpoint &peerEp)->std::string
		{
			return sip_transport->GetSRTPKey(dialog, bindEp, peerEp);
		});
	}

	bool SetRecvBuf(const char *buf, size_t size) {
		return sip_transport->PutRawMessage(buf, size);
	}

	std::shared_ptr<VS_SIPMessage> GetMessageFromParser(const std::shared_ptr<VS_SIPParser> &parser, bool should_be_decodable = true)
	{
		auto m = parser->GetMsgForSend_SIP();
		if (m.first && m.second) {
			sip_transport->ProcessMessage(m.first, m.second);
		}
		return m.second;
	}

	virtual void TearDown() {
		test::ClearRegistry();
	}

	void TestMediaPart(bool isAudio, int audioCodec, bool isVideo, int videoCodec)
	{
		if (isAudio)
		{
			auto audio_channel_it = std::find_if(confProtocol_fake.last_media_channels.begin(), confProtocol_fake.last_media_channels.end(), [](const VS_MediaChannelInfo& x) {
				return x.type == SDPMediaType::audio && x.content == SDP_CONTENT_MAIN;
			});
			ASSERT_TRUE(audio_channel_it != confProtocol_fake.last_media_channels.end());
			EXPECT_EQ(audio_channel_it->snd_mode_audio.CodecType, audioCodec);
		}

		if (isVideo)
		{
			auto video_channel_it = std::find_if(confProtocol_fake.last_media_channels.cbegin(), confProtocol_fake.last_media_channels.cend(), [](const VS_MediaChannelInfo& x) {
				return x.type == SDPMediaType::video && x.content == SDP_CONTENT_MAIN;
			});
			ASSERT_TRUE(video_channel_it != confProtocol_fake.last_media_channels.end());
			EXPECT_EQ(video_channel_it->snd_mode_video.CodecType, videoCodec);
		}
	}

	const vs::map<std::string, std::shared_ptr<VS_SIPParserInfo>, vs::str_less>& GetInternalContextsTest() const {
		assert(sip != nullptr);
		return sip->m_ctx;
	}

	bool CallOnResponse_Code200(const std::shared_ptr<VS_SIPResponse>& rsp) {
		return sip->OnResponse_Code200(rsp);
	}

	std::string GetFromHost() const
	{
		return sip->GetFromHost();
	}

	boost::asio::io_service::strand strand;
	std::shared_ptr<VS_ConferenceProtocolMock> confProtocol;
	VS_ConferenceProtocolFake confProtocol_fake;
	boost::shared_ptr<VS_IndentifierMock> identifier;
	boost::shared_ptr<VS_IndentifierSIP> identifier_impl;
	std::shared_ptr<VS_CallConfigStorage> call_config;
	std::shared_ptr<SIPParserExtends> sip;
	std::shared_ptr<test::SIPTransportLayer> sip_transport;

	net::Endpoint vcs_addr, terminal_addr;

	std::string our_endpoint;
	boost::asio::io_service m_ios;
};
