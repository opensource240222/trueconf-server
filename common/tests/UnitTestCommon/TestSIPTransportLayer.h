#pragma once

#include "FakeSIPChannel.h"
#include "TrueGateway/sip/SIPTransportLayer.h"
#include "std/cpplib/event.h"

namespace test {
// we could simply implement the interface on test::SIPTransportLayer,
// but we can't call weak_from_this() from the constructor to pass it to the channel
class ChannelEventListenerStub
	: public sip::ChannelEventListener {
public:
	void OnConnectionDie(unsigned channelID) override {}
	void OnProcessMsg(const std::shared_ptr<sip::Channel>& ch, const std::shared_ptr<VS_SIPMessage>& msg) override {}
	void OnWriteEnd(unsigned channelID, const boost::system::error_code& ec) override {}
};

class SIPTransportLayer : public sip::TransportLayer {
	std::shared_ptr<ChannelEventListenerStub> eventListener;
	std::shared_ptr<test::FakeChannel> fakeChannel;
public:
	SIPTransportLayer(boost::asio::io_service::strand& strand,
		const std::shared_ptr<VS_SIPParser>& parser,
		const std::shared_ptr<VS_CallConfigStorage>& callConfigStorage,
		const net::Endpoint &bind_addr, const net::Endpoint &peer_addr);
	~SIPTransportLayer();

	void ResetParser(const std::shared_ptr<VS_SIPParser> &parser);
	bool ProcessMessage(const std::shared_ptr<VS_SIPParserInfo> &ctx,
		const std::shared_ptr<VS_SIPMessage> &msg);
	bool PutRawMessage(const char *buf, size_t size);
	bool FillMsgOutgoing(const std::shared_ptr<VS_SIPParserInfo> &ctx,
		const std::shared_ptr<VS_SIPMessage> &msg) {
		vs::event done(true);
		bool res(false);
		m_strand.dispatch([&]() {
			VS_SCOPE_EXIT{ done.set(); };
			res = sip::TransportLayer::FillMsgOutgoing(ctx, msg, fakeChannel);
		});
		done.wait();
		return res;
	}
	bool Write(const std::shared_ptr<VS_SIPParserInfo> &ctx, const std::shared_ptr<VS_SIPMessage> &msg) {
		vs::event done(true);
		bool res(false);
		m_strand.dispatch([&]() {
			VS_SCOPE_EXIT{ done.set(); };
			res = sip::TransportLayer::Write(ctx, msg);
		});
		done.wait();
		return res;
	}
	bool CloseChannel(const net::Endpoint &peer);
	void AddDialogToFakeCh(const std::string& dialog);
	bool FindChannel(const net::Endpoint& peer, sip::TransportChannel& OUT_info);
};
}