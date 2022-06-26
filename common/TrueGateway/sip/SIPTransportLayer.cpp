#include "SIPTransportLayer.h"
#include "SIPTransportChannel.h"
#include "TrueGateway/CallConfig/VS_CallConfigStorage.h"
#include "TrueGateway/sip/VS_SIPParser.h"
#include "TrueGateway/sip/VS_TranscoderKeeper.h"
#include "TrueGateway/clientcontrols/VS_ClientControlInterface.h"
#include "TrueGateway/sip/VS_SIPGetInfoImpl.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/ignore.h"
#include "std/cpplib/event.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "std/VS_TransceiverInfo.h"
#include "std/cpplib/base64.h"
#include "net/Connect.h"
#include "net/VS_MediaAddress.h"
#include "net/DNSUtils/VS_DNSTools.h"
#include "net/tls/Connection.h"
#include "acs/Lib/VS_AcsLibDefinitions.h"
#include "SIPParserLib/VS_SIPMessage.h"
#include "SIPParserLib/VS_SIPField_Via.h"
#include "SIPParserLib/VS_SIPMetaField.h"
#include "SIPParserLib/VS_SIPAuthScheme.h"
#include "SIPParserLib/VS_SIPAuthGSS.h"
#include "SIPParserLib/VS_SIPField_Contact.h"
#include "SIPParserLib/VS_SIPField_From.h"
#include "SIPParserLib/VS_SIPField_To.h"
#include "SIPParserLib/VS_SIPURI.h"
#include "SIPParserLib/VS_SIPField_Expires.h"

#include "net/Logger/PcapLogger.h"

#include <boost/asio/write.hpp>
#include <iomanip>

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_MULTIGATEWAY

extern std::string g_tr_endpoint_name;
static const std::chrono::seconds IDLE_CHANNEL_TIMEOUT = std::chrono::seconds(100);
static const std::chrono::seconds CONNECT_TIMEOUT = std::chrono::seconds(4);
static const std::chrono::seconds ZOMBIE_CHANNEL_TIMEOUT = std::chrono::seconds(5);
static const unsigned int c_maxQueueSize = 100;
const uint32_t sip::TransportLayer::_65KB = 66560;

namespace
{
	void config_remote_endpoint(const std::shared_ptr<VS_SIPParserInfo> &ctx,
		const std::shared_ptr<VS_CallConfigStorage> &callConfigStorage,
		net::Endpoint &enp, bool isRequest)
	{
		assert(ctx != nullptr);
		assert(ctx->GetConfig().Address.protocol != net::protocol::none);

		auto contact = ctx->GetSIPContact();
		const auto orig_enp = enp;

		if (isRequest)
		{
			if (contact == nullptr)
			{
				// First request to user server
				enp = ctx->GetConfig().Address;
				if (!ctx->GetConfig().sip.OutboundProxy.empty())
				{
					auto resolved = net::dns_tools::single_make_a_aaaa_lookup(ctx->GetConfig().sip.OutboundProxy);
					dstream4 << "change addr=" << enp.addr << " to OutboundProxy=" << ctx->GetConfig().sip.OutboundProxy << " (resolved:" << resolved << ")";
					enp.addr = resolved;
					if (enp.addr.is_unspecified())
					{
						enp.addr = ctx->GetConfig().Address.addr;
						dstream4 << "SIP: Wrong outbound proxy format";
					}
				}
			}
			else
			{
				const auto uri = contact->GetLastURI();
				assert(uri);

				enp.protocol = uri->Transport();
				enp.protocol = enp.protocol != net::protocol::none ? enp.protocol : uri->URIType() == SIPURI_SIPS ? net::protocol::TLS : net::protocol::none;
				enp.port = uri->Port();

				boost::system::error_code ec;
				enp.addr = net::address::from_string(uri->Host(), ec);

				if (ec || enp.protocol == net::protocol::none || enp.port == 0)
				{
					std::string call_id;
					uri->GetRequestURI(call_id);
					call_id.insert(0, SIP_CALL_ID_PREFIX, ::strlen(SIP_CALL_ID_PREFIX));

					VS_CallConfig cfg;
					callConfigStorage->Resolve(cfg, call_id, nullptr);

					enp.port = enp.port == 0 ? cfg.Address.port : enp.port;
					enp.protocol = enp.protocol == net::protocol::none ? cfg.Address.protocol : enp.protocol;
					enp.addr = ec ? cfg.Address.addr : enp.addr;
				}
			}
		}
		else
		{
			assert(enp.protocol != net::protocol::none);
			auto &&via_top = ctx->GetViaTop();
			enp.port = via_top == nullptr ? 0 : via_top->Port();
		}

		if (enp.port == 0)
			enp.port = enp.protocol == net::protocol::TLS ? 5061 /*TLS*/ : 5060 /*TCP or UDP*/;

		if (enp.protocol == net::protocol::none)
			enp.protocol = orig_enp.protocol;

		if (enp.addr.is_unspecified())
			enp.addr = orig_enp.addr;
	}

	sip::TransportChannel make_channel(const std::shared_ptr<VS_SIPParserInfo> &ctx,
		const std::shared_ptr<VS_CallConfigStorage> &callConfigStorage,
		net::Endpoint &endpoint, bool isRequest)
	{
		sip::TransportChannel res;

		auto &&cfg = ctx->GetConfig();
		if (!cfg.ConnectionTypeSeq.empty())
		{
			endpoint.protocol = cfg.ConnectionTypeSeq[0];
		}
		else
		{
			assert(endpoint.protocol != net::protocol::none);

			cfg.ConnectionTypeSeq.clear();
			cfg.ConnectionTypeSeq.emplace_back(endpoint.protocol);
		}

		config_remote_endpoint(ctx, callConfigStorage, endpoint, isRequest);

		res.info.protocols.assign(cfg.ConnectionTypeSeq.cbegin(), cfg.ConnectionTypeSeq.cend());
		return res;
	}

	void set_network_info(const std::shared_ptr<VS_SIPParserInfo> &ctx, const net::Endpoint &remoteEp, const net::Endpoint &bindEp, net::port listenPort, boost::asio::io_service &ios) noexcept
	{
		assert(ctx != nullptr);
		assert(listenPort != 0);

		ctx->SetMyCsAddress(bindEp);
		if (ts::UseLocalTransceiver())
			ctx->SetMyMediaAddress(bindEp.addr);
		else
			ctx->SetMyMediaAddress(net::GetRTPAddress(ios));

		ctx->SetListenPort(listenPort);

		if (net::is_private_address(remoteEp.addr))
			ctx->SetMyExternalCsAddress({});
	}


	std::string get_msg_signature(const std::shared_ptr<VS_SIPAuthGSS>& auth,
		const std::shared_ptr<VS_SIPParserInfo>& ctx,
		const std::shared_ptr<VS_SIPMessage>& msg)
	{
		assert(msg != nullptr);
		assert(ctx != nullptr);
		assert(auth != nullptr);

		std::string auth_method = auth->scheme_str();
		if (auth_method.empty())
			return "";
		auto meta = msg->GetSIPMetaField();
		if (!meta || !meta->iFrom || !meta->iFrom->GetURI() || !meta->iTo)
			return "";

		std::string from_url, to_url;
		meta->iFrom->GetURI()->GetRequestURI(from_url);
		meta->iTo->GetURI()->GetRequestURI(to_url);
		from_url = "sip:" + from_url;
		to_url = "sip:" + to_url;

		auto expires = meta->iExpires ? meta->iExpires->Value() : std::chrono::seconds(0);
		int response_code = msg->GetResponseCode();

		std::string res; res.reserve(512);
		res += "<"; res += auth_method; res += ">";
		res += "<"; res += auth->crand(); res += ">";
		res += "<"; res += std::to_string(auth->cnum()); res += ">";
		res += "<"; res += auth->realm(); res += ">";
		res += "<"; res += auth->targetname(); res += ">";
		res += "<"; res += std::string(msg->CallID()); res += ">";
		res += "<"; res += std::to_string(msg->GetCSeq()); res += ">";
		res += "<"; res += VS_SIPObjectFactory::GetMethod(msg->GetMethod()); res += ">";
		res += "<"; res += from_url; res += ">";
		res += "<"; res += meta->iFrom->GetURI()->Tag(); res += ">";
		res += "<"; res += to_url; res += ">";
		res += "<"; res += meta->iTo->GetURI()->Tag(); res += ">";
		res += "<><>";
		res += "<"; res += (expires.count() ? std::to_string(expires.count()) : ""); res += ">";
		if (response_code > 0) {
			res += "<"; res += std::to_string(response_code); res += ">";
		}
		return res;
	}

	template<class Function>
	bool set_request_info(
		const std::shared_ptr<VS_SIPParserInfo>& ctx,
		const std::shared_ptr<VS_SIPMessage>& msg,
		Function& insertSignature,
		VS_SIPObjectFactory::SIPHeader& authHeader, const net::address& chBindAddr) {
		assert(ctx != nullptr);
		assert(msg != nullptr);

		const VS_SIPGetInfoImpl getInfo{ *ctx };
		auto& config = ctx->GetConfig();

		// Set Via Host
		assert(!chBindAddr.is_unspecified());
		boost::system::error_code ec;
		auto addrStr = chBindAddr.to_string(ec);
		if (!ec)
			ctx->SetViaHost(addrStr);
		else if (!g_tr_endpoint_name.empty()) {
			auto host = g_tr_endpoint_name;
			size_t pos = host.find('#');
			if (pos != std::string::npos) host.erase(pos);
			ctx->SetViaHost(std::move(host));
		}
		// Set Contact Domain
		if (!config.sip.ContactDomain.empty())
			ctx->SetContactHost(config.sip.ContactDomain);
		else if (msg->GetMethod() != TYPE_REGISTER)
			ctx->SetContactHost(ctx->GetViaHost());

		// Update header fields
		eStartLineType m = msg->GetMethod();
		const bool include_contact = !(m == TYPE_BYE || m == TYPE_CANCEL /*|| m == TYPE_PRACK */ || msg->GetResponseCode() == 100);
		if (!msg->UpdateOrInsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Via, getInfo) ||
			(include_contact && !msg->UpdateOrInsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Contact, getInfo)) ||
			!msg->UpdateOrInsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo) ||
			!msg->UpdateOrInsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_To, getInfo)) {
			return false;
		}

		insertSignature();

		// set OPTION branch
		if (m == TYPE_OPTIONS && ctx->DoINeedUpdateOptionsBranch())
		{
			ctx->NeedUpdateOptionsBranch(false);
			ctx->SetInviteAfterOptions(static_cast<std::string>(msg->Branch()));
		}

		if (!msg->UpdateOrIgnoreSIPField(authHeader, getInfo))
			return false;

		return true;
	}

	bool contact_required(const std::shared_ptr<VS_SIPMessage>& msg) noexcept
	{
		assert(msg != nullptr);

		eStartLineType m = msg->GetMethod();
		int code = msg->GetResponseCode();
		int codeClass = msg->GetResponseCodeClass();
		return (codeClass == 1 && code != 100 && (m == TYPE_INVITE || m == TYPE_SUBSCRIBE || m == TYPE_NOTIFY || m == TYPE_UPDATE)) ||
			(codeClass == 2 && !(m == TYPE_ACK || m == TYPE_BYE || m == TYPE_CANCEL || m == TYPE_INFO || m == TYPE_MESSAGE /*|| m == TYPE_PRACK */)) ||
			(codeClass == 3 && !(m == TYPE_ACK || m == TYPE_CANCEL || m == TYPE_INFO)) ||
			(code == 485 && !(m == TYPE_ACK || m == TYPE_CANCEL || m == TYPE_INFO)) ||
			((codeClass == 4 || codeClass == 5 || codeClass == 6) && false /*m == TYPE_REFER*/);
	}

	template<class Function>
	bool SetResponseInfo(const std::shared_ptr<VS_SIPParserInfo>& ctx,
		const std::shared_ptr<VS_SIPMessage>& msg,
		Function& insertSignature,
		VS_SIPObjectFactory::SIPHeader& authHeader) {
		assert(msg != nullptr);
		assert(ctx != nullptr);

		const VS_SIPGetInfoImpl getInfo{ *ctx };
		const bool includeContact = contact_required(msg);
		if ((includeContact && !msg->UpdateOrInsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_Contact, getInfo)) ||
			!msg->UpdateOrInsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_ContentLength, getInfo) ||
			!msg->UpdateOrInsertSIPField(VS_SIPObjectFactory::SIPHeader::SIPHeader_To, getInfo)) {
			return false;
		}

		insertSignature();

		auto authScheme = ctx->GetAuthScheme();
		if (authScheme && authScheme->scheme() == SIP_AUTHSCHEME_NTLM &&
			!msg->UpdateOrInsertSIPField(authHeader, getInfo)) {
			return false;
		}
		return true;
	}

	void add_dialog(sip::ChanelInfo& info, const std::string& dialog) {
		bool newDialog = info.dialogs.emplace(dialog).second;
		if (newDialog)
			dstream4 << "SIP: Transport add dialog='" << dialog << "',ch=" << info.localEp << " <-> " << info.remoteEp;
	}

} //anonymous namespace

namespace sip{

const size_t c_read_size = 0xffff;

using TcpSocket = boost::asio::ip::tcp::socket;
using SharedStreamBuff = std::shared_ptr<acs::Handler::stream_buffer>;
class TCPChannel : public Channel {
	TcpSocket m_socket;
	acs::Handler::stream_buffer m_buffer;

	void HandleRead(acs::Handler::stream_buffer && buffer);
	void StartRead();
	const std::string& LogID() override;
public:
	~TCPChannel() { Close(); }
	void Close() override;
	template<class Handler>
	void Accept(std::shared_ptr<TcpSocket> && socket, SharedStreamBuff && buffer, Handler&& onAccept);
	void Write(vs::SharedBuffer &&data) override;
	template<class Handler>
	void Connect(const boost::asio::ip::tcp::endpoint &ep, Handler&& onConnect);
	void GetPeerEndpoint(net::Endpoint &ep);
	void GetBindEndpoint(net::Endpoint &ep);
protected:
        template<typename ...Args>
	TCPChannel(boost::asio::io_service::strand& strand, Args&&... args)
		: Channel(strand, std::forward<Args>(args)...)
		, m_socket(strand.get_io_service())
	{}
	static void PostConstruct(std::shared_ptr<TCPChannel> &) { /*stub*/ }
};

using SharedPacketBuff = std::shared_ptr<acs::Handler::packet_buffer>;
class UDPChannel : public Channel {
	net::UDPConnection m_conn;
	acs::Handler::packet_buffer m_buffer;

	void HandleRead(acs::Handler::packet_buffer && buffer);
	void StartRead();
	const std::string& LogID() override;
public:
	~UDPChannel() { Close(); }
	void Close() override;
	template<class Handler>
	void Accept(std::shared_ptr<net::UDPConnection> && conn, SharedPacketBuff && buffer, Handler&& onAccept);
	void Write(vs::SharedBuffer &&data) override;
	template<class Handler>
	void Connect(const net::UDPRouter::endpoint_type &dst, net::port srcPort, Handler &&onConnect);
	void GetPeerEndpoint(net::Endpoint &ep);
	void GetBindEndpoint(net::Endpoint &ep);
protected:
	template<typename ...Args>
	UDPChannel(Args&&... args)
		: Channel(std::forward<Args>(args)...)
	{}
	static void PostConstruct(std::shared_ptr<UDPChannel>&) { /*stub*/ }
};

// TODO: Remove it when certificate issue will be resolved
static int verify_cb(int /*preverifyOk*/, X509_STORE_CTX* /*ctx*/) {
	return 1;	// always ok
}

class TLSChannel : public Channel {
	tls::socket m_socket;
	acs::Handler::stream_buffer m_buffer;

	void HandleRead(acs::Handler::stream_buffer && buffer);
	void StartRead();
	const std::string& LogID() override;
public:
	~TLSChannel() { Close(); }
	void Close() override;
	template<class Handler>
	void Accept(std::shared_ptr<tls::socket> && socket, SharedStreamBuff && buffer, Handler&& onAccept);
	void Write(vs::SharedBuffer &&data) override;
	template<class Handler>
	void Connect(const tls::endpoint &ep, Handler&& onConnect);
	void GetPeerEndpoint(net::Endpoint &ep);
	void GetBindEndpoint(net::Endpoint &ep);
	bool DeriveKey(std::vector<uint8_t>&key, size_t wantedLen, const char *label, size_t label_len, const uint8_t *context, size_t context_len);
protected:
        template<typename ...Args>
	TLSChannel(boost::asio::io_service::strand& strand, Args&& ...args)
		: Channel(strand, std::forward<Args>(args)...)
		, m_socket(strand.get_io_service(), false)
	{
		// TODO: Remove it when certificate issue will be resolved
		m_socket.reset_verify_cb(SSL_VERIFY_NONE, verify_cb);
	}
	static void PostConstruct(std::shared_ptr<TLSChannel> &) { /*stub*/ }
};

void TCPChannel::HandleRead(acs::Handler::stream_buffer && buffer)
{
	assert(m_strand.running_in_this_thread());

	if (m_logger)
		m_logger->Log(buffer.data(), buffer.size(),m_socket.remote_endpoint(vs::ignore<boost::system::error_code>()),
			m_socket.local_endpoint(vs::ignore<boost::system::error_code>()), m_channel_log_info, net::protocol::TCP, false);
	
	m_queueIn.PutMessageWithFilters(buffer.data(), buffer.size(), e_SIP_CS, VS_SIPInputMessageQueue::FLT_NONE);
	ProcessInputMsgs();
	StartRead();
}
void TCPChannel::StartRead()
{
	assert(m_strand.running_in_this_thread());
	m_buffer.resize(c_read_size);
	m_socket.async_read_some(boost::asio::buffer(m_buffer.data(), m_buffer.size()), m_strand.wrap(
		[this, w_self = this->weak_from_this()](const boost::system::error_code& ec, size_t bytesTransferred) mutable
	{
		auto self = w_self.lock();
		if (!self)
			return;

		VS_SCOPE_EXIT{
			if ((ec || !m_socket.is_open())) {
				ProcessInputMsgs();	// push all msgs out before dying
				if (auto listener = m_eventListener.lock())
					listener->OnConnectionDie(m_id);
			}
		};
		if (ec == boost::asio::error::operation_aborted)
			return;
		if (ec){
			dstream2 << "SIP: TCPChannel(" << LogID() << "): read failed: " << ec.message();
			return;
		}
		if (!m_socket.is_open()){
			dstream4 << "SIP: TCPChannel(" << LogID() << "): was closed before dispatch was finished\n";
			return;
		}
		if (bytesTransferred == 0){
			StartRead();
			return;
		}

		m_buffer.resize(bytesTransferred);
		HandleRead(std::move(m_buffer));
	}));
}
const std::string & TCPChannel::LogID()
{
	assert(m_strand.running_in_this_thread());
	if (!m_logId.empty())
		return m_logId;

	return m_logId = logh::GetSocketEndpointsStr(m_socket);
}
void TCPChannel::Close()
{
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };

		dprint4("Closing SIP TCP channel");
		if (m_flow)
			m_flow->RemoveSocket(m_socket.native_handle());
		m_socket.close(vs::ignore<boost::system::error_code>());

		ProcessInputMsgs();	// push all msgs out before dying
		if (auto listener = m_eventListener.lock())
			listener->OnConnectionDie(m_id);
	});
	done.wait();
}

template<class Handler>
void TCPChannel::Accept(std::shared_ptr<TcpSocket> && socket, SharedStreamBuff && buffer, Handler&& onAccept)
{
	m_strand.dispatch(
		[this, w_this = this->weak_from_this(), s = std::move(socket), b = std::move(buffer), onAccept= std::forward<Handler>(onAccept)]() mutable
	{
		auto self = w_this.lock();
		if (!self)
			return;
		m_socket = std::move(*s);
		if (m_flow)
			m_flow->AddSocket(m_socket.native_handle(), m_socket.remote_endpoint(vs::ignore<boost::system::error_code>()).data());

		net::Endpoint local, remote;
		GetBindEndpoint(local);
		GetPeerEndpoint(remote);
		onAccept(m_id, std::move(local), std::move(remote));

		HandleRead(std::move(*b));
	});

}
void sip::TCPChannel::Write(vs::SharedBuffer&& data)
{	
	m_strand.dispatch([this, w_this = weak_from_this(), data = std::move(data)]() mutable {
		auto self = w_this.lock();
		if (!self)
			return;
		const auto buffer = boost::asio::buffer(data.data<const char>(), data.size());
		boost::asio::async_write(m_socket, buffer, m_strand.wrap(
				[this, w_this = std::move(w_this), data = std::move(data)](const boost::system::error_code& ec, size_t bytes_transferred) {
					auto self = w_this.lock();
					if (!self)
						return;
					if (ec)
						dstream2 << "SIP: TCPChannel(" << LogID() << "): write failed: " << ec.message();
					else if (m_logger)
						m_logger->Log(data.data(), data.size(), m_socket.local_endpoint(vs::ignore<boost::system::error_code>()),
							m_socket.remote_endpoint(vs::ignore<boost::system::error_code>()), m_channel_log_info, net::protocol::TCP, true);
					if (auto listener = m_eventListener.lock())
						listener->OnWriteEnd(m_id, ec);
				}));
	});
}

template<class Handler>
void sip::TCPChannel::Connect(const boost::asio::ip::tcp::endpoint & ep, Handler&& onConnect)
{
	using Protocol = boost::asio::ip::tcp;

	net::Connect<Protocol>(m_strand, ep,
		[this, w_this = this->weak_from_this(), onConnect=std::forward<Handler>(onConnect), ep]
		(const boost::system::error_code& ec, typename Protocol::socket&& socket)
	{
		auto self = w_this.lock();
		if (!self)
			return;
		if (ec) {
			dstream2 << "SIP: TCPChannel(" << ep << "): connect failed: " << ec.message();
			return;
		}
		m_socket = std::move(socket);

		net::Endpoint local, peer;
		GetBindEndpoint(local);
		GetPeerEndpoint(peer);
		onConnect(m_id, local, peer);

		StartRead();
	});
}

template<class Socket>
void GetRemoteEp(const Socket& s, net::address & addr, net::port & p) {
	auto remoteEp = s.remote_endpoint(vs::ignore<boost::system::error_code>());
	addr = remoteEp.address();
	p = remoteEp.port();
}
template<class Socket>
void GetLocalEp(const Socket& s, net::address & addr, net::port & p) {
	auto localEp = s.local_endpoint(vs::ignore<boost::system::error_code>());
	addr = localEp.address();
	p = localEp.port();
}

void sip::TCPChannel::GetPeerEndpoint(net::Endpoint &ep)
{
	assert(m_strand.running_in_this_thread());
	GetRemoteEp(m_socket, ep.addr, ep.port);
	ep.protocol = net::protocol::TCP;
}

void sip::TCPChannel::GetBindEndpoint(net::Endpoint &ep)
{
	assert(m_strand.running_in_this_thread());
	GetLocalEp(m_socket, ep.addr, ep.port);
	ep.protocol = net::protocol::TCP;
}

void sip::UDPChannel::HandleRead(acs::Handler::packet_buffer && buffer)
{
	assert(m_strand.running_in_this_thread());
	while (!buffer.Empty()) {
		if (m_logger)
			m_logger->Log(buffer.Front().Data(), buffer.Front().Size(), m_conn.remote_endpoint(vs::ignore<boost::system::error_code>()),
				m_conn.local_endpoint(vs::ignore<boost::system::error_code>()), m_channel_log_info, net::protocol::UDP);
		m_queueIn.PutMessageWithFilters(reinterpret_cast<unsigned char*>(buffer.Front().Data()), buffer.Front().Size(), e_SIP_CS, VS_SIPInputMessageQueue::DEFALUT_FILTERS);
		buffer.PopFront();
	}
	ProcessInputMsgs();
	StartRead();
}

void sip::UDPChannel::StartRead()
{
	assert(m_strand.running_in_this_thread());
	m_buffer.PushBack(nullptr, m_buffer.Empty() ? m_buffer.GetMaxBlockSize() : m_buffer.GetChunkFreeSpace());
	m_conn.async_receive(boost::asio::buffer(m_buffer.Back().Data(), m_buffer.Back().Size()), m_strand.wrap(
		[this, w_self = this->weak_from_this()](const boost::system::error_code& ec, size_t bytesTransferred) mutable
	{
		auto self = w_self.lock();
		if (!self)
			return;

		VS_SCOPE_EXIT{
			if ((ec || !m_conn.is_open())) {
				ProcessInputMsgs();	// push all msgs out before dying
				if (auto listener = m_eventListener.lock())
					listener->OnConnectionDie(m_id);
			}
		};
		if (ec == boost::asio::error::operation_aborted)
			return;
		if (ec){
			dstream2 << "SIP: UDPChannel(" << LogID() << "): read failed: " << ec.message();
			return;
		}
		if (!m_conn.is_open()){
			dstream4 << "SIP: UDPChannel(" << LogID() << "): was closed before dispatch was finished\n";
			return;
		}

		assert(bytesTransferred <= m_conn.GetRouter().GetMaxPacketSize());
		m_buffer.ResizeBack(bytesTransferred);
		HandleRead(std::move(m_buffer));
	}));

}

const std::string & sip::UDPChannel::LogID()
{
	assert(m_strand.running_in_this_thread());
	if (!m_logId.empty())
		return m_logId;

	return m_logId = logh::GetSocketEndpointsStr(m_conn);
}

void UDPChannel::Close() {
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };

		dprint4 ("Closing SIP UDP channel");
		if (m_flow && m_conn.is_open())
			m_flow->RemoveSocket(m_conn.native_handle());
		m_conn = {};
		ProcessInputMsgs();	// push all msgs out before dying
		if (auto listener = m_eventListener.lock())
			listener->OnConnectionDie(m_id);
	});
	done.wait();
}

template<class Handler>
void sip::UDPChannel::Accept(std::shared_ptr<net::UDPConnection> && conn, SharedPacketBuff && buffer, Handler&& onAccept)
{
	m_strand.dispatch(
		[this, w_this = this->weak_from_this(), c = std::move(conn), b = std::move(buffer), onAccept = std::forward<Handler>(onAccept)]() mutable
	{
		auto self = w_this.lock();
		if (!self)
			return;

		m_conn = std::move(*c);
		if (m_flow)
			m_flow->AddSocket(m_conn.native_handle(), m_conn.remote_endpoint(vs::ignore<boost::system::error_code>()).data());

		net::Endpoint local, remote;
		GetBindEndpoint(local);
		GetPeerEndpoint(remote);
		onAccept(m_id, std::move(local), std::move(remote));
		HandleRead(std::move(*b));
	});
}

void sip::UDPChannel::Write(vs::SharedBuffer && data)
{	
	m_strand.dispatch([this, w_this = weak_from_this(), data = std::move(data)]() mutable {
		auto self = w_this.lock();
		if (!self)
			return;
		const auto buffer = boost::asio::buffer(data.data<const char>(), data.size());
		m_conn.async_send(buffer, m_strand.wrap(
			[this, w_this = std::move(w_this), data = std::move(data)](const boost::system::error_code& ec, size_t bytesTransferred) {
			auto self = w_this.lock();
			if (!self)
				return;
			if (ec)
				dstream2 << "SIP: UDPChannel(" << LogID() << "): write failed: " << ec.message();
			else if (m_logger)
				m_logger->Log(data.data(), bytesTransferred, m_conn.local_endpoint(vs::ignore<boost::system::error_code>()),
					m_conn.remote_endpoint(vs::ignore<boost::system::error_code>()), m_channel_log_info, net::protocol::UDP);
			if (auto listener = m_eventListener.lock())
				listener->OnWriteEnd(m_id, ec);
		}));
	});
}

template<class Handler>
void sip::UDPChannel::Connect(const net::UDPRouter::endpoint_type & dst, net::port srcPort, Handler && onConnect)
{
	m_strand.post([this, w_this = this->weak_from_this(), onConnect=std::forward<Handler>(onConnect), dst, srcPort]() {
		boost::system::error_code ec;
		auto conn = net::UDPRouter::Connect(m_strand.get_io_service(), srcPort,	dst, ec);
		if (ec) {
			dstream2 << "SIP: UDPChannel(" << dst << "): connect failed: " << ec.message();
			return;
		}
		m_conn = std::move(conn);
		assert(m_conn.is_open());

		net::Endpoint local, peer;
		GetBindEndpoint(local);
		GetPeerEndpoint(peer);
		onConnect(m_id, local, peer);

		StartRead();
	});
}


void sip::UDPChannel::GetPeerEndpoint(net::Endpoint &ep)
{
	assert(m_strand.running_in_this_thread());
	GetRemoteEp(m_conn, ep.addr, ep.port);
	ep.protocol = net::protocol::UDP;
}

void sip::UDPChannel::GetBindEndpoint(net::Endpoint &ep)
{
	assert(m_strand.running_in_this_thread());
	GetLocalEp(m_conn, ep.addr, ep.port);
	ep.protocol = net::protocol::UDP;
}

void sip::TLSChannel::HandleRead(acs::Handler::stream_buffer && buffer)
{
	assert(m_strand.running_in_this_thread());

	if (m_logger)
		m_logger->Log(buffer.data(), buffer.size(), m_socket.remote_endpoint(vs::ignore<boost::system::error_code>()),
			m_socket.local_endpoint(vs::ignore<boost::system::error_code>()), m_channel_log_info, net::protocol::TLS, false);

	m_queueIn.PutMessageWithFilters(buffer.data(), buffer.size(), e_SIP_CS, VS_SIPInputMessageQueue::FLT_NONE);
	ProcessInputMsgs();
	StartRead();
}

void sip::TLSChannel::StartRead()
{
	assert(m_strand.running_in_this_thread());
	m_buffer.resize(c_read_size);
	m_socket.async_receive(boost::asio::buffer(m_buffer.data(), m_buffer.size()), m_strand.wrap(
		[this, w_self = this->weak_from_this()](const boost::system::error_code& ec, size_t bytesTransferred) mutable
	{
		auto self = w_self.lock();
		if (!self)
			return;

		VS_SCOPE_EXIT{
			if ((ec || !m_socket.is_open())) {
				ProcessInputMsgs();	// push all msgs out before dying
				if (auto listener = m_eventListener.lock())
					listener->OnConnectionDie(m_id);
			}
		};
		if (ec == boost::asio::error::operation_aborted)
			return;
		if (ec) {
			dstream2 << "SIP: TLSChannel(" << LogID() << "): read failed: " << ec.message();
			return;
		}
		if (!m_socket.is_open()) {
			dstream4 << "SIP: TLSChannel(" << LogID() << "): was closed before dispatch was finished\n";
			return;
		}
		if (bytesTransferred == 0) {
			StartRead();
			return;
		}

		m_buffer.resize(bytesTransferred);
		HandleRead(std::move(m_buffer));
	}));
}

const std::string & sip::TLSChannel::LogID()
{
	assert(m_strand.running_in_this_thread());
	if (!m_logId.empty())
		return m_logId;

	return m_logId = logh::GetSocketEndpointsStr(m_socket);
}

void TLSChannel::Close()
{
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		dprint4("Closing SIP TLS channel");

		if (m_flow)
			m_flow->RemoveSocket(m_socket.native_handle());
		m_socket.close(vs::ignore<boost::system::error_code>());

		ProcessInputMsgs();	// push all msgs out before dying
		if (auto listener = m_eventListener.lock())
			listener->OnConnectionDie(m_id);
	});
	done.wait();
}

template<class Handler>
void TLSChannel::Accept(std::shared_ptr<tls::socket>&& socket, SharedStreamBuff && buffer, Handler && onAccept)
{
	m_strand.dispatch(
		[this, w_this = this->weak_from_this(), s = std::move(socket), b = std::move(buffer), onAccept = std::forward<Handler>(onAccept)]() mutable
	{
		auto self = w_this.lock();
		if (!self)
			return;
		m_socket = std::move(*s);
		if (m_flow)
			m_flow->AddSocket(m_socket.native_handle(), m_socket.remote_endpoint(vs::ignore<boost::system::error_code>()).data());

		net::Endpoint local, remote;
		GetBindEndpoint(local);
		GetPeerEndpoint(remote);
		onAccept(m_id, std::move(local), std::move(remote));

		HandleRead(std::move(*b));
	});
}

template<class Handler>
void sip::TLSChannel::Connect(const tls::endpoint & ep, Handler && onConnect)
{
	m_strand.dispatch([this, w_this = this->weak_from_this(), onConnect = std::forward<Handler>(onConnect), ep]() {
		auto self = w_this.lock();
		if (!self)
			return;

		m_socket.async_connect(ep, m_strand.wrap([this, w_this, onConnect = std::move(onConnect), ep](const boost::system::error_code& ec) {
			auto self = w_this.lock();
			if (!self)
				return;
			if (ec) {
				dstream2 << "SIP: TLSChannel(" << ep << "): connect failed: " << ec.message();
				return;
			}

			net::Endpoint local, peer;
			GetBindEndpoint(local);
			GetPeerEndpoint(peer);
			onConnect(m_id, local, peer);

			StartRead();
		}));
	});
}

void sip::TLSChannel::Write(vs::SharedBuffer && data)
{
	m_strand.dispatch([this, w_this = weak_from_this(), data = std::move(data)]() mutable {
		auto self = w_this.lock();
		if (!self)
			return;
		const auto buffer = boost::asio::buffer(data.data<const char>(), data.size());
		m_socket.async_send(buffer, m_strand.wrap(
			[this, w_this = std::move(w_this), data = std::move(data)](const boost::system::error_code& ec, size_t bytes_transferred) {
			auto self = w_this.lock();
			if (!self)
				return;
			if (ec)
				dstream2 << "SIP: TLSChannel(" << LogID() << "): write failed: " << ec.message();
			else if (m_logger)
				m_logger->Log(data.data(), bytes_transferred, m_socket.local_endpoint(vs::ignore<boost::system::error_code>()),
					m_socket.remote_endpoint(vs::ignore<boost::system::error_code>()), m_channel_log_info, net::protocol::TLS, true);
			if (auto listener = m_eventListener.lock())
				listener->OnWriteEnd(m_id, ec);
		}));
	});
}

void sip::TLSChannel::GetPeerEndpoint(net::Endpoint & ep)
{
	assert(m_strand.running_in_this_thread());
	GetRemoteEp(m_socket, ep.addr, ep.port);
	ep.protocol = net::protocol::TLS;
}

void sip::TLSChannel::GetBindEndpoint(net::Endpoint & ep)
{
	assert(m_strand.running_in_this_thread());
	GetLocalEp(m_socket, ep.addr, ep.port);
	ep.protocol = net::protocol::TLS;
}

bool sip::TLSChannel::DeriveKey(std::vector<uint8_t>& key, size_t wantedLen, const char * label, size_t label_len, const uint8_t * context, size_t context_len)
{
	bool res(false);
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		res = m_socket.derive_key(key, wantedLen, label, label_len, context, context_len);
	});
	done.wait();
	return res;
}

}	// namespace sip

sip::TransportLayer::TransportLayer(boost::asio::io_service::strand& strand,
	const std::shared_ptr<VS_SIPParser>& parser,
	const std::shared_ptr<VS_TranscoderKeeper>& trKeeper,
	const std::shared_ptr<VS_CallConfigStorage>& callConfigStorage,
	net::port listenTCP, net::port listenUDP,
	const std::shared_ptr<net::LoggerInterface>& logger)
	: m_strand(strand)
	, m_parser(parser)
	, m_trKeeper(trKeeper)
	, m_callConfigStorage(callConfigStorage)
	, m_timer(strand.get_io_service())
	, m_listenTCP(listenTCP)
	, m_listenUDP(listenUDP)
	, m_logger(logger)
{
	assert(m_parser != nullptr);
}


acs::Response sip::TransportLayer::Protocol(const stream_buffer & buffer, unsigned /*channel_token*/)
{
	return m_parser->Protocol(buffer.data(), buffer.size());
}

acs::Response sip::TransportLayer::Protocol(const packet_buffer & buffer, unsigned /*channel_token*/)
{
	// https://tools.ietf.org/html/rfc3261#section-7.5 for udp is not allowed fragmented messages
	if(m_parser->Protocol(buffer.Front().Data(), buffer.Front().Size()) == acs::Response::accept_connection)
		return acs::Response::accept_connection;
	return acs::Response::not_my_connection;
}

bool sip::TransportLayer::Write(const std::shared_ptr<VS_SIPParserInfo>& ctx, const std::shared_ptr<VS_SIPMessage>& msg, bool retransmit, bool isRetry)
{
	assert(ctx != nullptr);
	assert(msg != nullptr);
	assert(m_strand.running_in_this_thread());

	auto peerEp = ctx->GetConfig().Address;
	const bool isRequest = msg->GetMessageType() == MESSAGE_TYPE_REQUEST;
	if (!isRequest) {
		auto meta = msg->GetSIPMetaField();
		VS_SIPField_Via *via = nullptr;

		if (meta && !meta->iVia.empty() && (via = meta->iVia[0]) && !via->Received().empty()) {
			peerEp.addr = net::address::from_string(via->Received(), vs::ignore<boost::system::error_code>());
			peerEp.port = via->Port();
		}
	}
	if (peerEp.addr.is_unspecified()) {
		dstream2 << "SIP: TransportLayer::Write: Wrong CS address for dialog=" <<  msg->CallID();
		return false;
	}

	std::string dialog = ctx->GetRegCtxDialogID();
	if (dialog.empty())
		dialog = static_cast<std::string>(msg->CallID());
	auto ch = FindChannel(dialog);
	if (!ch) {
		ch = FindChannel(peerEp);
		if (!ch) {
			// create new ch
			auto &&onConnectHandler = [w_this = this->weak_from_this()](unsigned int id, const net::Endpoint & local, const net::Endpoint & remote)
			{
				auto self = w_this.lock();
				if (!self)
					return;
				self->OnConnect(id, local, remote);
			};

			auto transportChannel = make_channel(ctx, m_callConfigStorage, peerEp, isRequest);
			transportChannel.info.lastActivity = std::chrono::steady_clock::now();
			transportChannel.info.creationTime = std::chrono::steady_clock::now();
			if (!TryConnect(transportChannel, peerEp, std::move(onConnectHandler))) {
				dstream4 << "SIP: Transport: failed to connect to " << peerEp << "; dialog=" << dialog;
				return false;
			}

			ch = transportChannel.ch;
			auto chIt = m_channels.emplace(ch->GetID(), std::move(transportChannel)).first;
			if (chIt == m_channels.end())
				return false;

			chIt->second.queueOut.emplace(ctx, msg, retransmit ? OutgoingMsgType::Retransmit : OutgoingMsgType::Normal, isRetry);
			add_dialog(chIt->second.info, dialog);
			return true;
		}

		auto chIt = m_channels.find(ch->GetID());
		if (chIt == m_channels.end())
			return false;
		add_dialog(chIt->second.info, dialog);
	}
	EnqueueMsgOutgoing(ctx, msg, ch, retransmit ? OutgoingMsgType::Retransmit : OutgoingMsgType::Normal, isRetry);
	return true;
}

bool sip::TransportLayer::Write(TransportChannel & ch)
{
	assert(m_strand.running_in_this_thread());

	auto m = m_parser->GetMsgForSend_SIP(ch.info.dialogs);
	if(m.first != nullptr && m.second != nullptr) {
		EnqueueMsgOutgoing(m.first, m.second, ch.ch, OutgoingMsgType::Normal);
		return true;
	}
	return false;
}

template<class Callback>
std::shared_ptr<sip::Channel> sip::TransportLayer::Connect(const net::Endpoint & dst, Callback && onConnect)
{
	assert(m_strand.running_in_this_thread());
	if (dst.addr.is_unspecified() || dst.port == 0)
		return nullptr;

	std::shared_ptr<Channel> channel(nullptr);
	switch (dst.protocol)
	{
	case net::protocol::TCP:
	{
		auto flow = net::QoSSettings::GetInstance().GetSIPQoSFlow(false, dst.addr.is_v6());
		auto tcpChannel = vs::MakeShared<TCPChannel>(m_strand, ++m_lastChannelId, _65KB, flow, weak_from_this(), m_logger);
		boost::asio::ip::tcp::endpoint ep(dst.addr, dst.port);
		tcpChannel->Connect(ep, std::forward<Callback>(onConnect));
		channel = tcpChannel;
		break;
	}
	case net::protocol::UDP:
	{
		auto flow = net::QoSSettings::GetInstance().GetSIPQoSFlow(true, dst.addr.is_v6());
		auto udpChannel = vs::MakeShared<UDPChannel>(m_strand, ++m_lastChannelId, _65KB, flow, weak_from_this(), m_logger);
		net::UDPRouter::endpoint_type ep(dst.addr, dst.port);
		udpChannel->Connect(ep, 5060, std::forward<Callback>(onConnect));
		channel = udpChannel;
		break;
	}
	case net::protocol::TLS:
	{
		auto flow = net::QoSSettings::GetInstance().GetSIPQoSFlow(false, dst.addr.is_v6());
		auto tlsChannel = vs::MakeShared<TLSChannel>(m_strand, ++m_lastChannelId, _65KB, flow, weak_from_this(), m_logger);
		tls::endpoint ep(dst.addr, dst.port);
		tlsChannel->Connect(ep, std::forward<Callback>(onConnect));
		channel = tlsChannel;
		break;
	}
	default:
		return nullptr;
	}

	assert(channel != nullptr);
	return channel;
}

template<class Callback>
bool sip::TransportLayer::TryConnect(TransportChannel & channel, const net::Endpoint & dst, Callback&& onConnect)
{
	auto& protocols = channel.info.protocols;
	assert(m_strand.running_in_this_thread());
	assert(!protocols.empty());
	if (channel.ch)
		channel.ch->Close();
	if (protocols.empty())
		return false;

	auto newDst = dst;
	newDst.protocol = protocols.front();
	protocols.pop_front();
	channel.ch = Connect(newDst, std::forward<Callback>(onConnect));
	if (channel.ch == nullptr)
		return false;
	channel.info.remoteEp = dst;
	return true;
}

void sip::TransportLayer::OnConnect(unsigned int id, const net::Endpoint & local, const net::Endpoint & remote)
{
	m_strand.dispatch([this, w_this = this->weak_from_this(), id, local, remote]() {
		auto self = w_this.lock();
		if (!self)
			return;
		auto it = m_channels.find(id);
		if (it == m_channels.end()) {
			dstream4 << "SIP: Transport: failed to find channel after connect:" << local << " <-> " << remote;
			return;
		}

		auto &info = it->second.info;
		info.localEp = local;
		info.remoteEp = remote;
		info.connected = true;

		SendNextQueuedMessage(it->second.ch);
	});
}

void sip::TransportLayer::CancelDialogOperations(TransportChannel & ch, const std::string & dialog)
{
	assert(m_strand.running_in_this_thread());
	auto &chInfo = ch.info;
	if (chInfo.dialogs.count(dialog) == 0)
		return;

	auto m = m_parser->GetMsgForSend_SIP(dialog);
	while (m.second) {
		if (m.first)
			EnqueueMsgOutgoing(m.first, m.second, ch.ch);

		m = m_parser->GetMsgForSend_SIP(dialog);
	}

	chInfo.dialogs.erase(dialog);
}

void sip::TransportLayer::OnChannelDie(unsigned int chId)
{
	assert(m_strand.running_in_this_thread());
	m_strand.dispatch([this, w_this = this->weak_from_this(), chId]() {
		auto self = w_this.lock();
		if (!self)
			return;

		auto chIt = m_channels.find(chId);
		if (chIt == m_channels.end())
			return;

		if (chIt->second.info.incomingMsgCount == 0)
		{
			auto &chInfo = chIt->second.info;
			for (auto it = chInfo.dialogs.begin(); it != chInfo.dialogs.end(); ++it) {
				auto t = m_trKeeper->GetTranscoder(*it);
				if (t && t->trans)
					t->trans->Hangup();
				m_parser->CleanParserContext(*it, VS_ParserInterface::SourceClean::TRANSPORT);
			}
		}
	});
}

void sip::TransportLayer::ScheduleTimer(const std::chrono::milliseconds period)
{
	m_timer.expires_from_now(period);
	m_timer.async_wait([w_this = this->weak_from_this(), period](const boost::system::error_code& ec) {
		if (ec == boost::asio::error::operation_aborted)
			return;

		if (auto self = w_this.lock()) {
			self->Timeout();
			self->ScheduleTimer(period);
		}
	});
}

void sip::TransportLayer::Init() {
	m_parser->Connect_DialogFinished([w_this = this->weak_from_this()](string_view dialog) {
		if (auto self = w_this.lock())
			self->OnDialogFinished(dialog);
	});
	m_parser->Connect_FreeDialogFromChannel([w_this = this->weak_from_this()](string_view dialog) {
		if (auto self = w_this.lock())
			self->FreeDialogFromChannel(dialog);
	});
	m_parser->Connect_GetSRTPKey([w_this = this->weak_from_this()](string_view dialog, const net::Endpoint &bindEp, const net::Endpoint &peerEp) ->std::string
	{
		if (auto self = w_this.lock())
			return self->GetSRTPKey(dialog, bindEp, peerEp);
		return std::string();
	});

	ScheduleTimer();
}

std::shared_ptr<sip::Channel> sip::TransportLayer::FindChannel(string_view dialog) const noexcept
{
	auto chIt = std::find_if(m_channels.cbegin(), m_channels.cend(),
		[dialog](const std::pair<const unsigned int, sip::TransportChannel> &p) noexcept
		{
			auto &dialogs = p.second.info.dialogs;
			return dialogs.find(dialog) != dialogs.end();
		});

	if (chIt != m_channels.cend()
		&& std::find(m_dyingChannels.cbegin(), m_dyingChannels.cend(), chIt->first) == m_dyingChannels.cend())
	{
		return chIt->second.ch;
	}
	return nullptr;
}

std::shared_ptr<sip::Channel> sip::TransportLayer::FindChannel(const net::Endpoint& peerEp) const noexcept
{
	auto chIt = std::find_if(m_channels.cbegin(), m_channels.cend(),
		[&peerEp](const std::pair<unsigned int, sip::TransportChannel> &p) noexcept
		{
			return peerEp == p.second.info.remoteEp ||
				   peerEp.protocol == net::protocol::any && p.second.info.remoteEp.protocol != net::protocol::none &&
				   peerEp.addr == p.second.info.remoteEp.addr && peerEp.port == p.second.info.remoteEp.port;
		});

	if (chIt != m_channels.cend()
		&& std::find(m_dyingChannels.cbegin(), m_dyingChannels.cend(), chIt->first) == m_dyingChannels.cend())
	{
		return chIt->second.ch;
	}
	return nullptr;
}

void sip::TransportLayer::Accept(boost::asio::ip::tcp::socket && socket, stream_buffer && buffer)
{
	Accept<boost::asio::ip::tcp::socket, stream_buffer, TCPChannel>(std::move(socket), std::move(buffer), net::protocol::TCP);
}

void sip::TransportLayer::Accept(net::UDPConnection && connection, packet_buffer && buffer)
{
	Accept<net::UDPConnection, packet_buffer, UDPChannel>(std::move(connection), std::move(buffer), net::protocol::UDP);
}

bool sip::TransportLayer::Write(string_view dialog)
{
	bool res(false);
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };

		auto &&m = m_parser->GetMsgForSend_SIP(dialog);
		if(m.second)
		{
			auto ctx = std::move(m.first);
			if (ctx)
			{
				auto msg = std::move(m.second);
				res = Write(ctx, msg, ctx->GetConfig().Address.protocol != net::protocol::UDP);
			}
		}

	});
	done.wait();
	return res;
}

void sip::TransportLayer::Timeout()
{
	m_strand.dispatch([w_this = this->weak_from_this(), this](){
		auto self = w_this.lock();
		if (!self)
			return;

		std::vector<TransportChannel> newChannels;
		for(auto it = m_channels.begin();it != m_channels.end(); ++it)
		{
			auto &info = it->second.info;
			auto&ch = it->second;

			auto now = std::chrono::steady_clock::now();
			if (!info.connected && (now - info.lastActivity >= CONNECT_TIMEOUT)) {
				if (!it->second.info.protocols.empty()) 
				{
					auto onConnectHandler =
						[w_this](unsigned int id, const net::Endpoint & local, const net::Endpoint & remote)
					{
						if (auto self = w_this.lock())
							self->OnConnect(id, local, remote);
					};

					TransportChannel newCh;
					newCh.info = std::move(it->second.info);
					newCh.queueOut = std::move(it->second.queueOut);
					newCh.info.creationTime = std::chrono::steady_clock::now();
					m_dyingChannels.emplace_back(it->first);
					if (!TryConnect(newCh, newCh.info.remoteEp, std::move(onConnectHandler)))
						continue;

					assert(m_parser != nullptr);
					for (const auto& dialog : newCh.info.dialogs){
						m_parser->RetryCall(dialog, std::chrono::seconds(1));
					}

					newChannels.emplace_back(std::move(newCh));
				}
				else // we ran out of possible protocols for the connection
				{
					m_dyingChannels.emplace_back(it->first);
				}
				continue;
			}

			if (now - info.lastActivity >= IDLE_CHANNEL_TIMEOUT && info.dialogs.empty() ||
				now - info.creationTime >= ZOMBIE_CHANNEL_TIMEOUT && info.incomingMsgCount == 0)
			{                                            // second case added for UDP - when whe created 
				m_dyingChannels.emplace_back(it->first); // channel more than ZOMBIE_CHANNEL_TIMEOUT secs. ago and haven`t received any messages
				continue;
			}

			Write(ch);
		}

		for (auto&& trCh : newChannels)	{
			auto id = trCh.ch->GetID();
			m_channels.emplace(id, std::move(trCh));
		}

		for (const auto& chId : m_dyingChannels){
			m_channels[chId].ch.reset();/* (to work properly independent of fields layout in TransportChannel)
			                                to firstly call ~Channel() -> Channel.Close() -> onChannelDie in which we need TransportChannel.info */
			m_channels.erase(chId);
		}
		m_dyingChannels.clear();

		m_parser->Timeout();

		auto m = m_parser->GetMsgForSend_SIP();
		while (m.second) {
			auto ctx = std::move(m.first);
			auto msg = std::move(m.second);

			if (ctx && msg)
				Write(ctx, msg);

			m = m_parser->GetMsgForSend_SIP();
		}

		m = m_parser->GetMsgForRetransmit_SIP();
		while (m.second) {
			auto ctx = std::move(m.first);
			auto msg = std::move(m.second);

			if (ctx && msg)
				Write(ctx, msg, true);

			m = m_parser->GetMsgForRetransmit_SIP();
		}

		static int ii = 0;
		if (ii++ % 10 == 0 && !m_channels.empty()) {
			dprint2("\t======CURRENT CHANNELS======\n");
			for (auto &ch : m_channels) {
				if (!ch.second.ch) continue;

				auto &info = ch.second.info;
				dstream2 << "\t" << info.localEp << " <-> " << info.remoteEp << ", dialogs=" << info.dialogs.size();
			}
			dprint2("\t============================\n");
		}
	});
}

void sip::TransportLayer::Shutdown()
{
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		m_parser->Shutdown();
		// Deletion of m_channels entries might trigger deletion of sip::Channel objects.
		// That in turn will result in calls to OnChannelDie() which will try to access elements of m_channels while it is being cleared.
		// To avoid that we reset all shared pointers to channels before removing m_channels entries.
		for (auto& x : m_channels)
			x.second.ch.reset();
		m_channels.clear();
	});
	done.wait();
}

void sip::TransportLayer::OnDialogFinished(string_view dialog)
{
	m_strand.dispatch([this, w_this = this->weak_from_this(), dialog = std::string(dialog)]() {
		auto self = w_this.lock();
		if (!self)
			return;

		auto it = std::find_if(m_channels.begin(), m_channels.end(),
			[&](const std::pair<unsigned int, TransportChannel> &p)
		{
			return p.second.info.dialogs.count(dialog) > 0;
		});
		if(it != m_channels.end())
		{
			CancelDialogOperations(it->second, dialog);
			Write(it->second);

			auto &chInfo = it->second.info;
		}

		if (m_trKeeper)
			m_trKeeper->FreeTranscoder(dialog);
	});
}

void sip::TransportLayer::FreeDialogFromChannel(string_view dialog)
{
	m_strand.dispatch([this, w_this = this->weak_from_this(), dialog = std::string(dialog)]() {
		auto self = w_this.lock();
		if (!self)
			return;

		auto it = std::find_if(m_channels.begin(), m_channels.end(),
			[&](const std::pair<unsigned int, TransportChannel> &p)
		{
			return p.second.info.dialogs.count(dialog) > 0;
		});
		if (it != m_channels.end())
		{
			CancelDialogOperations(it->second, dialog);

			auto &chInfo = it->second.info;
			if (chInfo.dialogs.empty()) // no dialogs left => delete channel
				m_dyingChannels.emplace_back(it->first);
		}
	});
}

sip::CSInfo sip::TransportLayer::GetCSInfo(string_view dialog)
{
	CSInfo res;
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		auto chIt = std::find_if(m_channels.begin(), m_channels.end(),
			[&](const std::pair<const unsigned int, sip::TransportChannel> &p)
		{
			auto &dialogs = p.second.info.dialogs;
			return dialogs.find(dialog) != dialogs.end();
		});
		if (chIt == m_channels.end())
			return;

		auto &chInfo = chIt->second.info;
		res.isAccepted = chInfo.connected;
		res.bindEp = chInfo.localEp;
		res.peerEp = chInfo.remoteEp;
		res.numDialogs = chInfo.dialogs.size();
	});
	done.wait();
	return res;
}

sip::CSInfo sip::TransportLayer::GetCSInfo(const net::Endpoint & peer)
{
	CSInfo res;
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		auto chIt = std::find_if(m_channels.begin(), m_channels.end(),
			[&](std::pair<const unsigned int, sip::TransportChannel> &p)
		{
			return p.second.info.remoteEp == peer;
		});
		if (chIt == m_channels.end())
			return;

		auto &chInfo = chIt->second.info;
		res.isAccepted = chInfo.connected;
		res.bindEp = chInfo.localEp;
		res.peerEp = chInfo.remoteEp;
		res.numDialogs = chInfo.dialogs.size();
	});
	done.wait();
	return res;
}


std::string sip::TransportLayer::GetSRTPKey(string_view dialog, const net::Endpoint & /*bindEp*/, const net::Endpoint & peerEp)
{
	static const std::string default_key = "hU2fLb9u2B+y9Z8URuA1be9Ony5vl1DD5sN7V1ynqK5qK3+iniRBELO8yKpdzDnk9pEGgkSVXrUf4AQx";
	assert(peerEp.protocol == net::protocol::TLS);
	if (peerEp.protocol != net::protocol::TLS)
		return "";

	vs::event done(true);
	std::string res(default_key);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		auto ch = FindChannel(dialog);
		if (!ch)
			ch = FindChannel(peerEp);
		if (!ch) {
			dstream3 << "Can't find TLS channel. peer='" << peerEp << "', dialog='" << dialog << "'\n";
			return;
		}
		auto tlsCh = std::dynamic_pointer_cast<TLSChannel>(ch);
		if (!tlsCh) {
			dstream3 << "Can't find TLS channel. peer='" << peerEp << "', dialog='" << dialog << "'. Cast wasn't done.\n";
			return;
		}

		// use method from rfc5764 for dtls,
		// treat client_write/server_write keys as keys for audio/video channels
		// use dialog id as context, its not right, but ok for now
		// better than hardcoded key anyway

		const int SRTP_MASTER_KEY_KEY_LEN = 16;
		const int SRTP_MASTER_KEY_SALT_LEN = 14;

		unsigned char audio_key[SRTP_MASTER_KEY_KEY_LEN + SRTP_MASTER_KEY_SALT_LEN],
					  video_key[SRTP_MASTER_KEY_KEY_LEN + SRTP_MASTER_KEY_SALT_LEN];

		const char label[] = "EXTRACTOR-dtls_srtp";
		const size_t wantedLen = SRTP_MASTER_KEY_KEY_LEN * 2 + SRTP_MASTER_KEY_SALT_LEN * 2;

		std::vector<uint8_t> derivedKey;
		if(!tlsCh->DeriveKey(derivedKey, wantedLen, label, strlen(label), reinterpret_cast<const uint8_t*>(dialog.data()), dialog.length()))
			return;

		assert(derivedKey.size() == wantedLen);
		if (derivedKey.size() != wantedLen)
			return;

		size_t off = 0;
		memcpy(&audio_key[0], derivedKey.data(), SRTP_MASTER_KEY_KEY_LEN);
		off += SRTP_MASTER_KEY_KEY_LEN;
		memcpy(&video_key[0], derivedKey.data() + off, SRTP_MASTER_KEY_KEY_LEN);
		off += SRTP_MASTER_KEY_KEY_LEN;
		memcpy(&audio_key[SRTP_MASTER_KEY_KEY_LEN], derivedKey.data() + off, SRTP_MASTER_KEY_SALT_LEN);
		off += SRTP_MASTER_KEY_SALT_LEN;
		memcpy(&video_key[SRTP_MASTER_KEY_KEY_LEN], derivedKey.data() + off, SRTP_MASTER_KEY_SALT_LEN);

		size_t b64_audio_key_len = 0;
		base64_encode(&audio_key[0], sizeof(audio_key), nullptr, b64_audio_key_len);
		auto b64_audio_key = vs::make_unique_default_init<char[]>(b64_audio_key_len);
		base64_encode(&audio_key[0], sizeof(audio_key), b64_audio_key.get(), b64_audio_key_len);

		size_t b64_video_key_len = 0;
		base64_encode(&video_key[0], sizeof(video_key), nullptr, b64_video_key_len);
		auto b64_video_key = vs::make_unique_default_init<char[]>(b64_video_key_len);
		base64_encode(&video_key[0], sizeof(video_key), b64_video_key.get(), b64_video_key_len);

		res = std::string(b64_audio_key.get(), b64_audio_key_len) + std::string(b64_video_key.get(), b64_video_key_len);
	});
	done.wait();
	return res;
}


bool sip::TransportLayer::ProcessMsgIncomingTest(const std::shared_ptr<VS_SIPMessage>& msg, const std::shared_ptr<Channel>& ch)
{
	vs::event done(true);
	bool res(false);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		res = ProcessMsgIncomingImpl(msg, ch);
	});
	done.wait();
	return res;
}

void sip::TransportLayer::ProcessMsgIncoming(const std::shared_ptr<VS_SIPMessage>& msg, const std::shared_ptr<Channel>& ch)
{
	m_strand.dispatch([this, w_this = weak_from_this(), msg, ch]() {
		auto self = w_this.lock();
		if (!self)
			return;
		ProcessMsgIncomingImpl(msg, ch);
	});
}

bool sip::TransportLayer::ProcessMsgIncomingImpl(const std::shared_ptr<VS_SIPMessage>& msg, const std::shared_ptr<Channel>& ch)
{
	assert(m_strand.running_in_this_thread());

	if (!msg || !ch)
		return false;

	auto chIt = m_channels.find(ch->GetID());
	if (chIt == m_channels.end())
		return false;

	bool res(false);
	auto &chInfo = chIt->second.info;
	VS_SCOPE_EXIT{
		if (!res)
			return;
		chInfo.lastActivity = std::chrono::steady_clock::now();
		++chInfo.incomingMsgCount;
	};

	if (msg->GetMessageType() == MESSAGE_TYPE_INVALID) {
		EnqueueMsgOutgoing(nullptr, nullptr, ch, OutgoingMsgType::Pong);
		return res = true;
	}
	if (msg->CallID().empty() || !FillMsgIncoming(msg, chIt->second)) {
		return res = false;
	}

	auto dialog = static_cast<std::string>(msg->CallID());
	bool newDialog = chInfo.dialogs.emplace(dialog).second;
	if (newDialog)
		dstream4 << "SIP: Transport: add dialog '" << dialog << "'; channel=" << ch->GetLogID();

	auto sameDialogChIt = std::find_if(m_channels.begin(), m_channels.end(), [&](std::pair<const unsigned int, sip::TransportChannel> &p) -> bool {
		if (p.first == ch->GetID())
			return false;
		auto &dialogs = p.second.info.dialogs;
		return dialogs.find(dialog) != dialogs.end();
	});
	if (sameDialogChIt != m_channels.end()) {
		dstream4 << "SIP: Transport: found dialog '" << dialog << "' in channel=" << sameDialogChIt->second.ch->GetLogID() << "; Clearing it!";

		auto& chInfo = sameDialogChIt->second.info;
		chInfo.dialogs.erase(dialog);
		if (chInfo.dialogs.empty()) {
			dstream4 << "SIP: Transport: channel=" << sameDialogChIt->second.ch->GetLogID() << " has no dialogs after clearing. Closing it!";
			m_dyingChannels.emplace_back(sameDialogChIt->second.ch->GetID());
		}
	}

	assert(m_parser);
	if (!m_parser->SetRecvMsg_SIP(msg, chInfo.remoteEp)) {
		dstream4 << "SIP: Fail to parse incoming message. dialog='" << dialog << "'; channel=" << ch->GetLogID();
		chInfo.dialogs.erase(dialog);
		return res = false;
	}

	return res = true;
}

bool sip::TransportLayer::FillMsgIncoming(const std::shared_ptr<VS_SIPMessage>& msg, const TransportChannel& ch)
{
	assert(msg != nullptr);
	assert(m_strand.running_in_this_thread());

	const auto pMeta = msg->GetSIPMetaField();
	VS_SIPField_Via *via = nullptr;
	if (!pMeta || !(via = pMeta->iVia[0]))
		return false;

	const auto & remoteAddr = ch.info.remoteEp.addr;
	auto addr = net::dns_tools::single_make_a_aaaa_lookup(via->Host());

	if (addr.is_unspecified() && remoteAddr.is_unspecified())
		return false;

	// https://tools.ietf.org/html/rfc3261#section-18.2.1, 2 paragraph
	if (!remoteAddr.is_unspecified() && (remoteAddr != addr || net::is_domain_name(via->Host())))
		via->Received(remoteAddr.to_string(vs::ignore<boost::system::error_code>()));

	return true;
}

bool sip::TransportLayer::ProcessMsgOutgoing(
	const std::shared_ptr<VS_SIPParserInfo>& ctx,
	const std::shared_ptr<VS_SIPMessage>& msg,
	const std::shared_ptr<Channel>& ch,
	OutgoingMsgType outMessageType)
{
	assert(ch != nullptr);
	assert(m_strand.running_in_this_thread());

	auto chIt = m_channels.find(ch->GetID());
	if (chIt == m_channels.end())
		return false;
	if (!chIt->second.info.connected) {
		return true;
	}

	bool res(false);
	VS_SCOPE_EXIT{
		if (!res)
			return;
		auto it = m_channels.find(ch->GetID());
		if (it == m_channels.end())
			return;
		it->second.info.lastActivity = std::chrono::steady_clock::now();
	};

	if (outMessageType == OutgoingMsgType::Pong) {
		auto pong = vs::SharedBuffer(3);
		strcpy(pong.data<char>(), "\r\n");
		ch->Write(std::move(pong));
		return true;
	}

	if (msg->GetMessageType() == MESSAGE_TYPE_INVALID) {
		auto ping = vs::SharedBuffer(5);
		strcpy(ping.data<char>(), "\r\n\r\n");
		ch->Write(std::move(ping));
		return res = true;
	}

	assert(msg != nullptr);
	assert(ctx != nullptr);
	assert(m_parser != nullptr);

	if (!FillMsgOutgoing(ctx, msg, ch) ||
		(outMessageType != OutgoingMsgType::Retransmit && !m_parser->ProcessTransaction(ctx, msg))) {
		return res = false;
	}

	size_t msgSize(0);
	if ((TSIPErrorCodes::e_buffer != msg->Encode(nullptr, msgSize)) || (msgSize == 0))
		return res = false;
	auto msgBuff = vs::SharedBuffer(msgSize + 1);	// allocate msgSize + '\0' for VS_SIPMessage::Encode
	if (TSIPErrorCodes::e_ok != msg->Encode(msgBuff.data<char>(), msgSize))
		return res = false;

	msgBuff.shrink(0, msgSize);	// do not send last '\0'
	ch->Write(std::move(msgBuff));
	return res = true;
}

bool sip::TransportLayer::FillMsgOutgoing(const std::shared_ptr<VS_SIPParserInfo>& ctx, const std::shared_ptr<VS_SIPMessage>& msg, const std::shared_ptr<Channel>& ch)
{
	assert(msg != nullptr);
	assert(ctx != nullptr);
	assert(ch != nullptr);
	assert(m_strand.running_in_this_thread());

	auto chIt = m_channels.find(ch->GetID());
	if (chIt == m_channels.end())
		return false;

	auto &bindEp = chIt->second.info.localEp;
	if (msg->GetMessageType() != MESSAGE_TYPE_RESPONSE && !msg->Branch().empty() && ctx->GetMyCsAddress() != bindEp)
		return true;

	auto &remoteEp = chIt->second.info.remoteEp;

	assert(bindEp.protocol != net::protocol::none && bindEp.protocol != net::protocol::any);
	set_network_info(ctx, remoteEp, bindEp, bindEp.protocol == net::protocol::TCP ? m_listenTCP :m_listenUDP, m_strand.get_io_service());

	ctx->IsRequest(msg->GetMessageType() == MESSAGE_TYPE_REQUEST);

	const VS_SIPGetInfoImpl getInfo{ *ctx };
	auto authScheme = ctx->GetAuthScheme();
	auto authType = authScheme ? authScheme->auth_type() : VS_SIPAuthInfo::TYPE_AUTH_INVALID;
	auto authHeader = VS_SIPObjectFactory::SIPHeader::SIPHeader_Authorization;
	if (authType == VS_SIPAuthInfo::TYPE_AUTH_PROXY_TO_USER)
		authHeader = VS_SIPObjectFactory::SIPHeader::SIPHeader_ProxyAuthorization;

	auto insertSignature = [&]() {
		if (!authScheme || authScheme->scheme() != SIP_AUTHSCHEME_NTLM)
			return;
		auto scheme = std::dynamic_pointer_cast<VS_SIPAuthGSS>(authScheme);
		if (!scheme || ctx->GetCnum() == 0)
			return;

		auto new_scheme = std::make_shared<VS_SIPAuthGSS>(authScheme->scheme());
		*new_scheme = *scheme;
		new_scheme->cnum(ctx->GetIncrCnum());
		std::string signature = get_msg_signature(new_scheme, ctx, msg);

		std::array<unsigned char, 16> signature_buf;
		if (ctx->secureCtx &&
			ctx->secureCtx->sspi.MakeSignature(signature.c_str(), signature.length(), signature_buf)) {

			dstream2 << "Signed buffer: " <<  signature;
			std::stringstream ss; ss << std::hex;
			for (int i = 0; i < 16; i++) {
				ss << std::setfill('0') << std::setw(2) << (int)signature_buf[i];
			}

			new_scheme->response(ss.str());
		}
		ctx->SetAuthScheme(new_scheme);
	};


	if (msg->GetMessageType() == MESSAGE_TYPE_REQUEST) {
		if (!set_request_info(ctx, msg, insertSignature, authHeader, bindEp.addr))
			return false;
	}
	else if (msg->GetMessageType() == MESSAGE_TYPE_RESPONSE) {
		if (!SetResponseInfo(ctx, msg, insertSignature, authHeader))
			return false;
	}

	if (msg->GetSDPMetaField()) {
		if (!msg->UpdateOrInsertSDPField(VS_SDPObjectFactory::SDPHeader::SDPHeader_Origin, getInfo) ||
			!msg->UpdateOrInsertSDPField(VS_SDPObjectFactory::SDPHeader::SDPHeader_Connection, getInfo)) {
			return false;
		}
	}
	return true;
}

void sip::TransportLayer::EnqueueMsgOutgoing(const std::shared_ptr<VS_SIPParserInfo>& ctx,
	const std::shared_ptr<VS_SIPMessage>& msg,
	const std::shared_ptr<Channel>& ch,
	OutgoingMsgType outMessageType,
	bool isRetry)
{
	assert(m_strand.running_in_this_thread());

	auto chIt = m_channels.find(ch->GetID());
	if (chIt == m_channels.end())
		return;

	const bool alreadySending = !chIt->second.queueOut.empty();

	chIt->second.queueOut.emplace(ctx, msg, outMessageType, isRetry);

	if (!alreadySending)
		SendNextQueuedMessage(ch);
}

void sip::TransportLayer::SendNextQueuedMessage(const std::shared_ptr<Channel>& ch)
{
	assert(m_strand.running_in_this_thread());

	const auto chIt = m_channels.find(ch->GetID());
	if (chIt == m_channels.end() or chIt->second.queueOut.empty())
		return;

	auto &transport_ch = chIt->second;
	const auto &msgInfo = transport_ch.queueOut.front();

	ProcessMsgOutgoing(msgInfo.ctx, msgInfo.msg, transport_ch.ch, msgInfo.outgoingMessageType);
}

void sip::TransportLayer::RetrySendMessage(const sip::MsgInfo& msgInfo)
{
	assert(m_strand.running_in_this_thread());

	if (msgInfo.outgoingMessageType == OutgoingMsgType::Pong)
		return; // ignoring pong responses because of lack of information to resend

	if (msgInfo.isRetry)
		return; // failed to send twice already, ignoring

	const bool isRetransmit = msgInfo.outgoingMessageType == OutgoingMsgType::Retransmit;
	Write(msgInfo.ctx, msgInfo.msg, isRetransmit, true);
}

void sip::TransportLayer::HandleWrite(unsigned chID, const boost::system::error_code& ec) {
	m_strand.dispatch([this, w_this = weak_from_this(), chID, ec]() {
		auto self = w_this.lock();
		if (!self)
			return;

		auto chIt = m_channels.find(chID);
		if (chIt == m_channels.end())
			return;

		auto &queueOut = chIt->second.queueOut;
		if (!ec) {
			if (!queueOut.empty()) {
				queueOut.pop();
				SendNextQueuedMessage(chIt->second.ch);
			}
			return;
		}

		if (
			//for TCP
			ec == boost::asio::error::eof
			|| ec == boost::asio::error::shut_down
			//for UDP
			|| ec == boost::asio::error::connection_refused
			|| ec == boost::asio::error::connection_aborted
			|| ec == boost::asio::error::host_unreachable
#ifdef _WIN32
			|| ec == boost::system::error_code(ERROR_HOST_UNREACHABLE, boost::system::system_category())
			|| ec == boost::system::error_code(ERROR_PORT_UNREACHABLE, boost::system::system_category())
			|| ec == boost::system::error_code(ERROR_CONNECTION_REFUSED, boost::system::system_category())
			|| ec == boost::system::error_code(ERROR_CONNECTION_ABORTED, boost::system::system_category())
#endif
			//for both
			|| ec == boost::asio::error::connection_reset
			) { // broken connection
			m_dyingChannels.push_back(chIt->second.ch->GetID());
			while (!queueOut.empty()) {
				MsgInfo msgInfo = std::move(queueOut.front());
				queueOut.pop();
				RetrySendMessage(msgInfo);
			}
			chIt->second.ch->Close();
		} else {
			OnChannelDie(chID);
		}
	});
}

void sip::TransportLayer::OnConnectionDie(unsigned chID) {
	OnChannelDie(chID);
}

void sip::TransportLayer::OnProcessMsg(const std::shared_ptr<Channel>& ch, const std::shared_ptr<VS_SIPMessage>& msg) {
	ProcessMsgIncoming(msg, ch);
}

void sip::TransportLayer::OnWriteEnd(unsigned chID, const boost::system::error_code& ec) {
	HandleWrite(chID, ec);
}

#undef DEBUG_CURRENT_MODULE