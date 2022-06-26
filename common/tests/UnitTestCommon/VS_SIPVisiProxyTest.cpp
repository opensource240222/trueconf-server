#include "tests/mocks/VS_FakeEndpointMock.h"
#include "TrueGateway/sip/VS_SIPParser.h"
#include "TrueGateway/sip/VS_SIPVisiProxy.h"
#include "TrueGateway/sip/VS_VisiSIPProxy.h"
#include "TrueGateway/sip/VS_TranscoderKeeper.h"
#include "FakeClient/VS_ConferenceInfo.h"
#include "TrueGateway/sip/VS_SIPCallResolver.h"
#include "TrueGateway/CallConfig/VS_CallConfigStorage.h"
#include "TrueGateway/clientcontrols/VS_FakeClientControl.h"
#include "TrueGateway/VS_GatewayParticipantInfo.h"
#include "VS_FakeClientMock.h"
#include "VS_FakeClientControlMock.h"
#include "SIPParserTestBase.h"
#include "TransceiversPoolFake.h"
#include "std/cpplib/MakeShared.h"

#include <boost/asio/io_service.hpp>
#include <boost/make_shared.hpp>
#include <gmock/gmock.h>

struct VS_TranscoderKeeperFake : public VS_TranscoderKeeper {
	boost::shared_ptr<Transcoder_Descr> NewTranscoder(string_view dialogId) override {
		auto transcoder_descr = boost::make_shared<Transcoder_Descr>();
		fake_clientControl->SetFakeClientInterface(fake_clientMock);
		transcoder_descr->trans = fake_clientControl;

		return transcoder_descr;
	}

	VS_TranscoderKeeperFake(const std::shared_ptr< test::TransceiversPoolFake> &pool, const std::weak_ptr<VS_TranscoderLogin>& transLogin)
		: VS_TranscoderKeeper()
		, fake_clientMock(vs::MakeShared<FakeClientMock>(std::make_unique<VS_FakeEndpointMock>()))
		, fake_clientControl(boost::make_shared<VS_FakeClientControl>(pool, transLogin))
	{
	}

	std::shared_ptr<FakeClientMock> fake_clientMock;
	boost::shared_ptr<VS_FakeClientControl> fake_clientControl;
};

struct SIPVisiProxyTest : public ::testing::Test {
	boost::asio::io_service::strand strand;
	std::shared_ptr<VS_TranscoderKeeperFake> tr_keeper_fake;
	std::shared_ptr<VS_SIPParser>			   parser;
	std::shared_ptr<VS_SIPCallResolver>	   sip_call_resolver;
	std::shared_ptr<test::SIPTransportLayer>  sip_transport;
	std::shared_ptr<VS_VisiSIPProxy>		   visi_sip_proxy;
	std::shared_ptr<VS_SIPVisiProxy>		   sip_visi_proxy;
	std::shared_ptr<VS_CallConfigStorage>	   call_config_storage;
	std::shared_ptr<test::TransceiversPoolFake> m_fakeTransceiversPool;
	std::shared_ptr<VS_TranscoderLogin>	m_transLogin;
	net::Endpoint myEp;
	net::Endpoint remoteEp;
	std::string server_name = "my.server.name#vcs";
	boost::asio::io_service m_ios;

	UserStatusFunction get_user_status = [this](string_view callId, bool/**use_cache*/, bool/**do_ext_resolve*/) -> UserStatusInfo {
		if (callId.empty())
			return UserStatusInfo();
		UserStatusInfo user;
		boost::get<UserStatusInfo::User>(user.info).status = USER_AVAIL;
		user.real_id = std::string(callId) + '@' + server_name;
		return user;
	};

	SIPVisiProxyTest()
		: strand(g_asio_environment->IOService())
		, myEp{net::address::from_string("127.0.0.1"), 5060, net::protocol::UDP}
		, remoteEp{ net::address::from_string("1.2.3.4"), 5060, net::protocol::UDP }
	{
	}

	void SetUp() override
	{
		m_fakeTransceiversPool = std::make_shared<test::TransceiversPoolFake>();
		m_transLogin = vs::MakeShared<VS_TranscoderLogin>();
		tr_keeper_fake = std::make_shared<VS_TranscoderKeeperFake>(m_fakeTransceiversPool, m_transLogin);
		parser = vs::MakeShared<VS_SIPParser>(strand, "serverVendor", nullptr);
		{
			VS_SIPCallResolver::InitInfo init;
			init.trKeeper = tr_keeper_fake;
			init.parser = parser;
			init.peerConfig = nullptr; //stub

			sip_call_resolver = vs::MakeShared<VS_SIPCallResolver>(strand, std::move(init));
		}
		call_config_storage = vs::MakeShared<VS_CallConfigStorage>();
		sip_transport = std::make_shared<test::SIPTransportLayer>(strand, parser, call_config_storage, myEp, remoteEp);
		{
			VS_VisiSIPProxy::InitInfo init;
			init.parser = parser;
			init.trKeeper = tr_keeper_fake;
			init.peerConfig = call_config_storage;
			init.sipTransport = sip_transport;
			visi_sip_proxy = vs::MakeShared<VS_VisiSIPProxy>(strand, std::move(init));
		}
		tr_keeper_fake->SetVisiToSip(visi_sip_proxy);

		{
			VS_SIPVisiProxy::InitInfo init;
			init.trKeeper = tr_keeper_fake;
			init.sipTransport = sip_transport;
			init.getUserStatus = get_user_status;
			init.sipCallResolver = sip_call_resolver;
			init.transcPool = m_fakeTransceiversPool;
			sip_visi_proxy = vs::MakeShared<VS_SIPVisiProxy>(strand, std::move(init));
		}
	}
};

TEST_F(SIPVisiProxyTest, EmptyConferenceCreation) {
	using ::testing::EndsWith;
	using ::testing::StrEq;
	using ::testing::_;

	std::string topic = "Topic";
	// make call AsyncInvite and expect that call will reach fake_clientMock with conference params like we exepect
	EXPECT_CALL(*(tr_keeper_fake->fake_clientMock), JoinAsync(_, _, _));
	sip_visi_proxy->AsyncInvite("dialog_id", { "from", false }, EMPTY_CONFERENCE_TAG, VS_ConferenceInfo(true, true, topic), [](bool redirect, VS_ConferenceProtocolInterface::ConferenceStatus status, const std::string &ip) {}, "fromDN", true);
}

TEST_F(SIPVisiProxyTest, InvitetoSIP) {
	using ::testing::_;

	tr_keeper_fake->fake_clientControl = boost::make_shared<FakeClientControlMock>();

	EXPECT_CALL(*(boost::static_pointer_cast<FakeClientControlMock>(tr_keeper_fake->fake_clientControl)), InviteMethod(_, _, _, _, true, _));
	sip_visi_proxy->AsyncInvite("dialog_id", { "from", false }, "#sip:@1.2.3.4", VS_ConferenceInfo(false, false), [](bool redirect, VS_ConferenceProtocolInterface::ConferenceStatus status, const std::string &ip) {}, "fromDN", true);
}