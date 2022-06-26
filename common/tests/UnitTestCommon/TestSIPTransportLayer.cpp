#include "TestSIPTransportLayer.h"
#include "TrueGateway/sip/VS_SIPParser.h"
#include "std/cpplib/MakeShared.h"

test::SIPTransportLayer::SIPTransportLayer(boost::asio::io_service::strand& strand,
	const std::shared_ptr<VS_SIPParser>& parser,
	const std::shared_ptr<VS_CallConfigStorage>& callConfigStorage,
	const net::Endpoint& bind_addr, const net::Endpoint& peer_addr)
	: sip::TransportLayer(strand, parser, nullptr, callConfigStorage, 5060, 5060, nullptr)
	, eventListener(std::make_shared<test::ChannelEventListenerStub>())
	, fakeChannel(vs::MakeShared<test::FakeChannel>(strand, m_lastChannelId++, 0, nullptr, eventListener, nullptr))
{
	assert(!bind_addr.addr.is_unspecified() && bind_addr.port != 0 && bind_addr.protocol != net::protocol::none);
	assert(!peer_addr.addr.is_unspecified() && peer_addr.port != 0 && peer_addr.protocol != net::protocol::none);

	fakeChannel->localEp = bind_addr;
	fakeChannel->remoteEp = peer_addr;
	sip::TransportChannel ch;
	ch.ch = fakeChannel;
	ch.info.remoteEp = peer_addr;
	ch.info.localEp = bind_addr;
	m_channels.emplace(fakeChannel->GetID(), std::move(ch));
}

test::SIPTransportLayer::~SIPTransportLayer()
{
}

void test::SIPTransportLayer::ResetParser(const std::shared_ptr<VS_SIPParser>& parser)
{
	m_parser = parser;
}

bool test::SIPTransportLayer::ProcessMessage(const std::shared_ptr<VS_SIPParserInfo>& ctx, const std::shared_ptr<VS_SIPMessage>& msg)
{
	return FillMsgOutgoing(ctx, msg) && m_parser->ProcessTransaction(ctx, msg);
}

bool test::SIPTransportLayer::PutRawMessage(const char * buf, size_t size)
{
	fakeChannel->AcceptMessage((unsigned char*)buf, size);
	auto msg = fakeChannel->GetMsg();
	return ProcessMsgIncomingTest(msg, fakeChannel);
}

bool test::SIPTransportLayer::CloseChannel(const net::Endpoint &peer) {
	auto chIt = std::find_if(m_channels.begin(), m_channels.end(),
		[&peer](const std::pair<unsigned int, sip::TransportChannel> &p)
	{
		return peer == p.second.info.remoteEp;
	});

	if (chIt == m_channels.end())
		return false;

	chIt->second.ch->Close();
	return true;
}

void test::SIPTransportLayer::AddDialogToFakeCh(const std::string& dialog) {
	auto it = m_channels.find(fakeChannel->GetID());
	assert(it != m_channels.end());

	it->second.info.dialogs.emplace(dialog);
}

bool test::SIPTransportLayer::FindChannel(const net::Endpoint& peer, sip::TransportChannel& OUT_info) {
	auto chIt = std::find_if(m_channels.begin(), m_channels.end(),
		[&peer](const std::pair<unsigned int, sip::TransportChannel> &p)
	{
		return peer == p.second.info.remoteEp;
	});
	if (chIt == m_channels.end())
		return false;

	OUT_info = chIt->second;
	return true;
}
