#pragma once

#include "acs_v2/Handler.h"
#include <memory>
#include <set>
#include <queue>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>

#include "net/Endpoint.h"
#include "net/QoSSettings.h"
#include "net/Lib.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/compat/memory.h"
#include "TrueGateway/CallConfig/VS_CallConfig.h"
#include "SIPParserLib/VS_SIPMessage.h"
#include "std-generic/cpplib/macro_utils.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/MakeShared.h"
#include "TrueGateway/sip/SIPChannelEventListener.h"

#include "std/debuglog/VS_LogHelpers.h"
#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_MULTIGATEWAY

class VS_SIPParser;
class VS_TranscoderKeeper;
class VS_CallConfigStorage;
class VS_SIPMessage;
class VS_SIPParserInfo;
namespace net {
	class LoggerInterface;
} //namespace net

namespace sip {
class Channel;

struct ChanelInfo {
	std::set <std::string, vs::str_less>	dialogs;
	net::Endpoint localEp, remoteEp;
	std::deque<net::protocol> protocols;
	unsigned int	incomingMsgCount = 0;
	std::chrono::steady_clock::time_point lastActivity;
	std::chrono::steady_clock::time_point creationTime; // when the channel was created in SipTransportLayer (see: make_channel())
	bool connected = false;
};
enum class OutgoingMsgType {
	Normal,
	Retransmit,
	Pong
};
struct MsgInfo {
	VS_FORWARDING_CTOR3(MsgInfo, ctx, msg, outgoingMessageType) {}
	VS_FORWARDING_CTOR4(MsgInfo, ctx, msg, outgoingMessageType, isRetry) {}
	std::shared_ptr<VS_SIPParserInfo> ctx;
	std::shared_ptr<VS_SIPMessage> msg;
	OutgoingMsgType outgoingMessageType;
	bool isRetry = false;
};
struct TransportChannel {
	std::shared_ptr<Channel> ch;
	ChanelInfo	info;
	std::queue<MsgInfo> queueOut;
};

struct CSInfo {
	bool			isAccepted = false;
	net::Endpoint	bindEp, peerEp;
	std::int32_t	numDialogs = 0;

	operator bool() const { return !(bindEp.addr.is_unspecified() || peerEp.addr.is_unspecified()); }
};

class TransportLayer
	: public acs::Handler
	, public ChannelEventListener
	, public vs::enable_shared_from_this<TransportLayer>
{
	static const uint32_t _65KB; // 65KB in bytes
protected:
	TransportLayer(boost::asio::io_service::strand& strand,
		const std::shared_ptr<VS_SIPParser> &parser,
		const std::shared_ptr<VS_TranscoderKeeper> &trKeeper,
		const std::shared_ptr<VS_CallConfigStorage> &callConfigStorage,
		net::port listenTCP, net::port listenUDP,
		const std::shared_ptr<net::LoggerInterface>& logger);
	static void PostConstruct(std::shared_ptr<TransportLayer>& p )
	{
		p->Init();
	}

public:
	acs::Response Protocol(const stream_buffer& buffer, unsigned channel_token = 0) override;
	acs::Response Protocol(const packet_buffer& buffer, unsigned channel_token = 0) override;
	void Accept(boost::asio::ip::tcp::socket&& socket, stream_buffer&& buffer) override;
	void Accept(net::UDPConnection&& connection, packet_buffer&& buffer) override;
	template<class Socket, class Buffer, typename ChannelType>
	void Accept(Socket&& socket, Buffer&& buffer, net::protocol p);
	bool Write(string_view dialog);
	void Timeout();
	void Shutdown();
	void OnDialogFinished(string_view dialog);
	void FreeDialogFromChannel(string_view dialog);
	CSInfo GetCSInfo(string_view dialog);
	CSInfo GetCSInfo(const net::Endpoint &peer);
	std::string GetSRTPKey(string_view dialog, const net::Endpoint &bindEp, const net::Endpoint& peerEp);

	void OnConnectionDie(unsigned channelID) override;
	void OnProcessMsg(const std::shared_ptr<sip::Channel>& ch, const std::shared_ptr<VS_SIPMessage>& msg) override;
	void OnWriteEnd(unsigned channelID, const boost::system::error_code& ec) override;
protected:
	bool ProcessMsgIncomingTest(const std::shared_ptr<VS_SIPMessage> &msg, const std::shared_ptr<Channel> &ch);
private:
	void ProcessMsgIncoming(const std::shared_ptr<VS_SIPMessage> &msg, const std::shared_ptr<Channel> &ch);
	bool ProcessMsgIncomingImpl(const std::shared_ptr<VS_SIPMessage> &msg, const std::shared_ptr<Channel> &ch);
	bool FillMsgIncoming(const std::shared_ptr<VS_SIPMessage>& msg, const TransportChannel& ch);
	bool ProcessMsgOutgoing(
		const std::shared_ptr<VS_SIPParserInfo> &ctx,
		const std::shared_ptr<VS_SIPMessage> &msg,
		const std::shared_ptr<Channel> &ch,
		OutgoingMsgType outMessageType);
protected:
	bool FillMsgOutgoing(
		const std::shared_ptr<VS_SIPParserInfo> &ctx,
		const std::shared_ptr<VS_SIPMessage> &msg,
		const std::shared_ptr<Channel> &ch);
	bool Write(const std::shared_ptr<VS_SIPParserInfo> &ctx, const std::shared_ptr<VS_SIPMessage> &msg, bool retransmit = false, bool isRetry = false);
private:
	void EnqueueMsgOutgoing(
		const std::shared_ptr<VS_SIPParserInfo> &ctx,
		const std::shared_ptr<VS_SIPMessage> &msg,
		const std::shared_ptr<Channel> &ch,
		OutgoingMsgType outMessageType = OutgoingMsgType::Normal,
		bool isRetry = false);
	void SendNextQueuedMessage(const std::shared_ptr<Channel> &ch);
	void HandleWrite(unsigned int ch, const boost::system::error_code & ec);
	void RetrySendMessage(const sip::MsgInfo & msgInfo);

	bool Write(TransportChannel& ch);
	template<class Callback>
	std::shared_ptr<Channel> Connect(const net::Endpoint &dst, Callback&& onConnect);
	template<class Callback>
	bool TryConnect(TransportChannel& channel, const net::Endpoint &dst, Callback&& onConnect);
	void OnConnect(unsigned int id, const net::Endpoint& local, const net::Endpoint& remote);
	void CancelDialogOperations(TransportChannel& ch, const std::string& dialog);
	void OnChannelDie(unsigned int chId);
	void ScheduleTimer(const std::chrono::milliseconds period = std::chrono::milliseconds(500));
	void Init();
	std::shared_ptr<sip::Channel> FindChannel(string_view dialog) const noexcept;
	std::shared_ptr<sip::Channel> FindChannel(const net::Endpoint& peerEp) const noexcept;
protected:
	boost::asio::io_service::strand m_strand;
	std::shared_ptr<VS_SIPParser> m_parser;
private:
	std::shared_ptr<VS_TranscoderKeeper> m_trKeeper;
	std::shared_ptr<VS_CallConfigStorage> m_callConfigStorage;
protected:
	std::map<unsigned int, TransportChannel>	m_channels;
	unsigned int m_lastChannelId = 0;
private:
	std::vector<unsigned int>	m_dyingChannels;
	boost::asio::steady_timer	m_timer;
	const net::port m_listenTCP;
	const net::port m_listenUDP;

	std::shared_ptr<net::LoggerInterface> m_logger;
};

template<class Socket, class Buffer, typename ChannelType>
void sip::TransportLayer::Accept(Socket&& socket, Buffer&& buffer, net::protocol p) {
	m_strand.dispatch([w_this = this->weak_from_this(), this, socket = std::make_shared<Socket>(std::move(socket)), buffer = std::make_shared<Buffer>(std::move(buffer)), p]() mutable {
		auto self = w_this.lock();
		if (!self)
			return;

		boost::system::error_code ec;
		auto peerEp = socket->remote_endpoint(ec);
		if (ec) {
			if (p == net::protocol::TCP)
				dstream4 << "SIP: TransportTCP: can't get remote endpoint\n";
			else if (p == net::protocol::UDP)
				dstream4 << "SIP: TransportUDP: can't get remote endpoint\n";
			else
				dstream4 << "SIP: TransportLayer::Accept: unknown protocol\n";

			return;
		}

		auto &&onAccept = [w_this, this](unsigned int chId, const net::Endpoint &local, const net::Endpoint &remote)
		{
			auto self = w_this.lock();
			if (!self)
				return;
			auto it = m_channels.find(chId);
			if (it == m_channels.end())
				return;
			auto &chInfo = it->second.info;
			chInfo.localEp = std::move(local);
			chInfo.remoteEp = std::move(remote);
			chInfo.connected = true;
		};

		auto flow = net::QoSSettings::GetInstance().GetSIPQoSFlow(p == net::protocol::UDP, peerEp.address().is_v6());
		if (p == net::protocol::TCP) {
			auto channel = vs::MakeShared<ChannelType>(m_strand, ++m_lastChannelId, _65KB, flow, weak_from_this(), m_logger);

			VS_RegistryKey key(false, CONFIGURATION_KEY);
			uint32_t is_enabled(1);
			key.GetValue(&is_enabled, sizeof(is_enabled), VS_REG_INTEGER_VT, "TCP KeepAlive");
			if (is_enabled != 0)
				net::EnableTCPKeepAlive(socket->native_handle(), 20, 2);

			TransportChannel ch;
			ch.ch = channel;
			m_channels.emplace(channel->GetID(), std::move(ch));
			dstream3 << "SIP: TransportTCP: Accept connection " << logh::GetSocketEndpointsStr(*socket);
			channel->Accept(std::move(socket), std::move(buffer), m_strand.wrap(std::move(onAccept)));
		}
		else if (p == net::protocol::UDP) {
			auto channel = vs::MakeShared<ChannelType>(m_strand, ++m_lastChannelId, _65KB, flow, weak_from_this(), m_logger);

			TransportChannel ch;
			ch.ch = channel;
			m_channels.emplace(channel->GetID(), std::move(ch));
			dstream3 << "SIP: TransportUDP: Accept connection " << logh::GetSocketEndpointsStr(*socket);
			channel->Accept(std::move(socket), std::move(buffer), m_strand.wrap(std::move(onAccept)));
		}
	});
}
}

#undef DEBUG_CURRENT_MODULE