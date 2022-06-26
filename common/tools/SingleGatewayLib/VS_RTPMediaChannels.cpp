#include "VS_RTPMediaChannels.h"
#include "VS_H235Session.h"
#include "rtcp_utils.h"
#include "stun.h"
#include "net/Address.h"
#include "net/Lib.h"
#include "tools/Server/VS_ApplicationInfo.h"
#include "Transcoder/RTPPacket.h"
#include "std/cpplib/base64.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/MakeShared.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/hton.h"
#include "std/debuglog/VS_Debug.h"
#include "net/QoSSettings.h"

#include "modules/rtp_rtcp/include/rtp_rtcp.h"
#include "modules/rtp_rtcp/source/rtp_rtcp_impl.h"
#include "modules/rtp_rtcp/source/rtcp_packet/receiver_report.h"
#include "modules/rtp_rtcp/source/rtcp_packet/sender_report.h"
#include "modules/rtp_rtcp/source/rtcp_packet/bye.h"
#include "modules/rtp_rtcp/source/rtcp_packet/sdes.h"
#include "modules/rtp_rtcp/source/rtcp_packet/common_header.h"
#include "modules/rtp_rtcp/source/rtcp_packet/fir.h"
#include "third_party/libsrtp/include/srtp.h"
#include "modules/rtp_rtcp/source/rtcp_packet/report_block.h"

#include <algorithm>

#define DEBUG_CURRENT_MODULE VS_DM_MULTIGATEWAY

#define DEFAULT_RTP_OFFSET 42

namespace
{
	const auto MAX_RTP_QUEUE_SIZE = 1024;
	const auto MAX_RTCP_QUEUE_SIZE = 128;

	const auto BUFFER_MAX_LEN = 65535;

	bool add_extra_flow(const net::QoSFlowSharedPtr &flow, VS_RTP_Channel::udp_socket_t &sock, const VS_RTP_Channel::endpoint_t &endpoint)
	{
		auto addr = endpoint.address();
		auto port = endpoint.port();

		void *sockaddr = nullptr;
		sockaddr_in sin;
		sockaddr_in6 sin6;

		if (addr.is_v4())
		{
			memset(&sin, 0, sizeof(sin));
			sin.sin_family = AF_INET;
			sin.sin_addr.s_addr = vs_htonl(addr.to_v4().to_ulong());
			sin.sin_port = vs_htons(port);

			sockaddr = &sin;
		}
		else if (addr.is_v6())
		{
			auto v6_addr = addr.to_v6().to_bytes();
			memset(&sin6, 0, sizeof(sin6));
			sin6.sin6_family = AF_INET6;
			assert(v6_addr.size() == 16);
			memcpy(sin6.sin6_addr.s6_addr, v6_addr.data(), v6_addr.size());
			sin6.sin6_port = vs_htons(port);

			sockaddr = &sin6;
		}
		else
		{
			return false;
		}

		if (!flow->AddSocket(sock.native_handle(), sockaddr))
		{
			return false;
		}

		return true;
	}


	void remove_extra_flow(net::QoSFlowSharedPtr &flow, VS_RTP_Channel::udp_socket_t &sock) //not ported this method
	{
		if (flow) flow->RemoveSocket(sock.native_handle());
		flow = nullptr;
	}


	void enable_qos_for_connection(VS_RTP_Channel::udp_socket_t &sock, net::QoSFlowSharedPtr &flow, const VS_RTP_Channel::endpoint_t &remote)
	{
		if (remote.port() != 0)
		{
			auto port = sock.local_endpoint().port();
			auto new_flow = net::QoSSettings::GetInstance().GetRTPQoSFlow();
			if (new_flow != nullptr && new_flow.get() != flow.get())
			{
				if (flow != nullptr)
				{
					remove_extra_flow(flow, sock);
				}

				if (add_extra_flow(new_flow, sock, remote))
				{
					flow = new_flow;
				}
			}
		}
	}
}

struct VS_SRTP
{
	srtp_policy_t policy = {};
	srtp_ctx_t *ctx = nullptr;
	int rtp_offset = DEFAULT_RTP_OFFSET;

	~VS_SRTP()
	{
		free(policy.key);
		if (ctx) {
			const auto res = srtp_dealloc(ctx);
			assert(res == srtp_err_status_ok);
		}
	}
};


std::uint32_t CreateSSRC(std::pair<uint32_t, uint32_t> ssrcRange)
{
	// https://msdn.microsoft.com/en-us/library/hh641667(v=office.12).aspx
	// recommendation for a=x-ssrc-range
	uint32_t generated_ssrc = rand() % 4294967040 + 1;
	if (ssrcRange.first) {
		generated_ssrc = ssrcRange.first + (ssrcRange.second == ssrcRange.first ? 0 : (rand() % (ssrcRange.second - ssrcRange.first)));
	}
	return generated_ssrc;
}


VS_RTP_Channel::VS_RTP_Channel(boost::asio::io_service::strand& strand)
	  :  m_strand(strand)
	  , m_rtp { strand.get_io_service(), std::size_t(BUFFER_MAX_LEN), 32, std::size_t(MAX_RTP_QUEUE_SIZE) }
	  , m_rtcp{ strand.get_io_service(), std::size_t(BUFFER_MAX_LEN), 4, std::size_t(MAX_RTCP_QUEUE_SIZE) }
	  , m_RecieveCallBack(nullptr)
	  , m_FIRCallBack(nullptr)
	  , m_incoming_info{}
	  , m_outgoing_info{}
	  , m_tmp_receiver_only_ssrc(0)
	  , m_our_mode_clock_rate(0)
	  , m_can_send_fake_packets(true)
	  , m_vsr_pt(0)
	  , m_fir_seqno_counter(0)
	  , m_channel_disconnect(true)
	  , m_not_empty_SR_was_received(false)
	  , m_rtp_was_sent_last_time_interval(false)
#ifndef NDEBUG
	  , m_version(0)
#endif
{
}


void VS_RTP_Channel::Write(bool rtp)
{
	Session &session = GetSession(rtp);

	if (!session.sock.is_open())
		return;

	const auto &message = session.outQueue.front();

	session.sock.async_send_to(boost::asio::buffer(message.data(), message.size()), session.sendToEndpoint,
		m_strand.wrap(
			[self = shared_from_this(), this, &session, rtp](const boost::system::error_code& error,
				std::size_t bytes_transferred)
	{

		if (error == boost::asio::error::operation_aborted)
			return;

		if (error)
		{
			dstream4 << "VS_RTP_Channel error write " << (rtp ? "RTP" : "RTCP") << session.localEndpoint << " -> : " << error.message();
			return;
		}

		Timeout();

		auto &packet = session.outQueue.front();

		if (rtp)
		{
			RTPPacket rtp_packet(packet.data<const void>(), packet.size());
			if (rtp_packet.IsValid() && rtp_packet.Version() == 2 && rtp_packet.SSRC() != 0)
			{
				m_outgoing_statistics[rtp_packet.SSRC()].AddPacket(
					packet.size(), rtp_packet.Timestamp(), GetLocalRTPClockRate());
				m_rtp_was_sent_last_time_interval = true;
			}
		}

		session.outQueue.pop_front();

		if (!session.outQueue.empty())
		{
			// more messages to send
			Write(rtp);
		}
	}));
}

inline VS_RTP_Channel::Session& VS_RTP_Channel::GetSession(bool rtp)
{
	return rtp ? m_rtp : m_rtcp;
}


void VS_RTP_Channel::Send(vs::SharedBuffer packet, bool rtp)
{
	Session &session = GetSession(rtp);

	if (session.outQueue.full() && session.outQueue.capacity() < session.queueMaxSize)
	{
		session.outQueue.set_capacity(std::min(session.outQueue.capacity() * 3 / 2, session.queueMaxSize));
		dstream4 << "VS_RTP_Channel(port=" << session.localEndpoint.port() << "): " << (rtp ? "RTP" : "RTCP") << " out buffer grows to " << session.outQueue.capacity();
	}

	if (session.outQueue.full()) dstream1 << "VS_RTP_Channel(port=" << session.localEndpoint.port() << "): " << (rtp ? "RTP" : "RTCP") << " out buffer oveflow";

	session.outQueue.push_back(std::move(packet));

	if (session.outQueue.size() > 1) return; //outstanding async_write

	Write(rtp);
}


VS_RTP_Channel::~VS_RTP_Channel()
{
	boost::system::error_code ec;
	m_rtp.sock.close(ec);
	m_rtcp.sock.close(ec);

	remove_extra_flow(m_rtp_flow, m_rtp.sock);
	remove_extra_flow(m_rtcp_flow, m_rtcp.sock);
}


void VS_RTP_Channel::SetReceiveCallBack(std::function<void (vs::SharedBuffer&&)> f)
{
	m_strand.post([self = shared_from_this(), f = std::move(f)]() mutable noexcept
	{
		self->m_RecieveCallBack = std::move(f);
	});
}

bool VS_RTP_Channel::Start(bool isIpv6)
{
	assert(m_version.exchange(1, std::memory_order_acq_rel) == 0);
	int32_t minPort(0), maxPort(0);

	VS_RegistryKey key(false, CONFIGURATION_KEY);
	key.GetValue(&minPort, sizeof(minPort), VS_REG_INTEGER_VT, "RTP MinPort");
	key.GetValue(&maxPort, sizeof(maxPort), VS_REG_INTEGER_VT, "RTP MaxPort");
	if (minPort % 2 != 0)
		++minPort; // RFC3550 says that RTP ports SHOULD be even, so we should start searching from an even port.
	if (!minPort || !maxPort || minPort > maxPort)
	{
		minPort = DEFAULT_RTP_MINPORT;
		maxPort = DEFAULT_RTP_MAXPORT;
	}

	net::address addr;

	if (!isIpv6)
	{
		m_rtp.sock.open(boost::asio::ip::udp::v4());
		m_rtcp.sock.open(boost::asio::ip::udp::v4());
		addr = boost::asio::ip::address_v4::any();
	}
	else
	{
		m_rtp.sock.open(boost::asio::ip::udp::v6());
		m_rtcp.sock.open(boost::asio::ip::udp::v6());
		addr = boost::asio::ip::address_v6::any();
	}

	bool res = false;

	//TODO:FIXME : !!!! if somewhere to use this, then it is necessary to implement in the function
#if defined(_WIN32) /* Windows*/
	typedef boost::asio::detail::socket_option::boolean<BOOST_ASIO_OS_DEF(SOL_SOCKET), SO_EXCLUSIVEADDRUSE> exclusive_address_use_t;
	m_rtp.sock.set_option(exclusive_address_use_t(true));
	m_rtcp.sock.set_option(exclusive_address_use_t(true));
#endif
	static std::atomic<uint16_t> lastUsedPort{ 0 };
	uint16_t rtp_port = 0;
	int32_t attempt;
	for (attempt = 0; attempt < (maxPort - minPort + 1) / 2; ++attempt)
	{
//		Obtain a free port
		auto prevPort = lastUsedPort.load(std::memory_order_relaxed);// load last
		do
		{
			rtp_port = prevPort + 2;// set next
			if (rtp_port < minPort || rtp_port > maxPort - 1) // check borders
				rtp_port = minPort;
			assert(rtp_port % 2 == 0);
		} while (!lastUsedPort.compare_exchange_weak(prevPort, rtp_port, std::memory_order_relaxed)); // check if someone else hasn't changed the value

		boost::system::error_code ec;
		m_rtp.sock.bind(endpoint_t(addr, rtp_port), ec);
		if (!ec)
		{
			m_rtcp.sock.bind(endpoint_t(addr, 1 + rtp_port), ec);
			if (!ec)
			{
				res = true;
				break;
			}
		}
	}

	if (!res)
	{
		assert(m_version.exchange(0, std::memory_order_acq_rel) == 1);

		boost::system::error_code ec; //for no-throw
		m_rtp.sock.close(ec);
		m_rtcp.sock.close(ec);

		return false;
	}

	VS_RegistryKey reg_key(VS_TRUECONF_WS_ROOT_KEY_NAME, false, CONFIGURATION_KEY);
	reg_key.GetValue(&m_can_send_fake_packets, sizeof(m_can_send_fake_packets), VS_REG_INTEGER_VT, "rtp send fake packets");

	boost::system::error_code ec; //for no-throw
	m_rtp.localEndpoint = m_rtp.sock.local_endpoint(ec);
	m_rtcp.localEndpoint = m_rtcp.sock.local_endpoint(ec);
	assert(!ec);

	m_channel_disconnect.store(false, std::memory_order_release);

	ReceiveFromRTP();
	ReceiveFromRTCP();

	assert(m_version.exchange(2, std::memory_order_acq_rel) == 1);

	return true;
}

void VS_RTP_Channel::SetRemoteRTPEndpoint(endpoint_t endpoint)
{
	m_strand.post([self = shared_from_this(), this, endpoint = std::move(endpoint)]() mutable noexcept
	{
		if (m_rtp.sendToEndpoint != endpoint)
		{
			enable_qos_for_connection(m_rtp.sock, m_rtp_flow, endpoint);
		}
		m_rtp.sendToEndpoint = std::move(endpoint);
	});
}

void VS_RTP_Channel::SetRemoteRTCPEndpoint(endpoint_t endpoint)
{
	m_strand.post([self = shared_from_this(), this, endpoint = std::move(endpoint)]() mutable noexcept
	{
		if (m_rtcp.sendToEndpoint != endpoint)
		{
			enable_qos_for_connection(m_rtcp.sock, m_rtcp_flow, endpoint);
		}
		m_rtcp.sendToEndpoint = std::move(endpoint);
	});
}

void VS_RTP_Channel::SendRTP(vs::SharedBuffer packet, bool protect)
{
	assert(m_version.load(std::memory_order_acquire) == 2);

	if (packet.empty()) return;

	m_strand.post([self = shared_from_this(), this, packet = std::move(packet), protect]() mutable
	{
		if (!self->m_rtp.sock.is_open())
			return;

		if (protect && m_srtp_cod)
		{
			vs::SharedBuffer protected_packet(packet.size() + SRTP_MAX_TRAILER_LEN + 4);
			std::memcpy(protected_packet.data<void>(), packet.data<const void>(), packet.size());
			int len = static_cast<int>(packet.size());
			auto r = srtp_protect(m_srtp_cod->ctx, protected_packet.data<void>(), &len);
			if (r == srtp_err_status_ok) {
				protected_packet.shrink(0, len);
				packet = std::move(protected_packet);
			}
		}
		else if (m_h235_session)
		{
			encryption_meta em;
			vs::SharedBuffer protected_packet = m_h235_session->EncryptPacket(packet, em);
			{
				FillStatistic(em, m_h235_encr_statistic);
			}
			if (!protected_packet.empty())
			{
				packet = std::move(protected_packet);
			}
		}

		Send(std::move(packet), true);
	});
}


void VS_RTP_Channel::Disconnect() noexcept
{
	if (!m_channel_disconnect.exchange(true, std::memory_order_acq_rel))
	{
		m_strand.post([self = shared_from_this(), this]()
		{
			SendRTCP(webrtc::kRtcpBye);

			remove_extra_flow(m_rtp_flow, m_rtp.sock);
			remove_extra_flow(m_rtcp_flow, m_rtcp.sock);

			boost::system::error_code ec;
			m_rtp.sock.close(ec);
			m_rtcp.sock.close(ec);
		});
	}
}

void VS_RTP_Channel::SendFakePackets(size_t count)
{
	assert(m_version.load(std::memory_order_acquire) == 2);

	m_strand.post([self = shared_from_this(), count]()
	{
		if (!self->m_rtp.sock.is_open())
			return;

		if (!self->m_can_send_fake_packets)
			return;

		const unsigned char RTP_v3_packet[] = { 0xce, 0xfa, 0xed, 0xfe };
		vs::SharedBuffer packet(4);
		std::memcpy(packet.data<void>(), RTP_v3_packet, sizeof(RTP_v3_packet));

		for( size_t i = 0; i < count; i++)
		{
			self->Send(packet, true);
		}
	});
}

bool VS_RTP_Channel::GetLocalEndpoints(endpoint_t& localRTPEndp, endpoint_t& localRTCPEndp) const
{
	if (IsDisconnected()) return false;
	localRTPEndp = m_rtp.localEndpoint;
	localRTCPEndp = m_rtcp.localEndpoint;
	return true;
}

inline uint32_t VS_RTP_Channel::GetLocalRTPClockRate()const
{
	return m_our_mode_clock_rate;
}


void VS_RTP_Channel::Timeout()
{
	const auto time_now = m_clock.now();
	if (m_last_sendRTCP + std::chrono::milliseconds(5000) < time_now)
	{
		m_last_sendRTCP = time_now;
		SendRTCP();
	}

	if (m_vsr_pt && !m_not_empty_SR_was_received && m_last_sendVSR + std::chrono::milliseconds(100) <  time_now)
	{
		if (SendVSR())
		{
			m_last_sendVSR = time_now;
		}
	}
}

//-------------------RTCP--------------


int VS_RTP_Channel::SendRR(uint32_t ssrc, webrtc::RTCPPacketType additionalType)
{
	vs::SharedBuffer packet(1024);
	size_t pos = 0;
	bool res = true;

	webrtc::rtcp::ReceiverReport rr;

	rr.SetSenderSsrc(ssrc);

	for (auto&& incom_stat : m_incoming_statistics)
	{
		if (!incom_stat.second.IsEmpty())
		{
			res = rr.AddReportBlock(incom_stat.second.GetReportBlock(incom_stat.first)) && res;
		}
	}

	Statistics(rr.report_blocks(), m_incoming_info);

	res = static_cast<webrtc::rtcp::RtcpPacket&>(rr).Create(packet.data<uint8_t>(), &pos, packet.size() - pos, nullptr) && res;

	webrtc::rtcp::Sdes sdes;
	res = sdes.AddCName(ssrc, "CNAME") && res;
	res = static_cast<webrtc::rtcp::RtcpPacket&>(sdes).Create(packet.data<uint8_t>(), &pos, packet.size() - pos, nullptr) && res;

	switch (additionalType)
	{
	case 0:
		// No additional data
		break;
	case webrtc::kRtcpFir:
	{
		webrtc::rtcp::Fir fir;

		fir.SetSenderSsrc(ssrc);
		fir.AddRequestTo(ssrc, m_fir_seqno_counter++);
		res = static_cast<webrtc::rtcp::RtcpPacket&>(fir).Create(packet.data<uint8_t>(), &pos, packet.size() - pos, nullptr) && res;

		break;
	}
	case webrtc::kRtcpBye:
	{
		webrtc::rtcp::Bye bye;

		bye.SetSenderSsrc(ssrc);
		res = static_cast<webrtc::rtcp::RtcpPacket&>(bye).Create(packet.data<uint8_t>(), &pos, packet.size() - pos, nullptr) && res;
		break;
	}
	default:
		res = false;
		assert(false);
	}

	if (!res)
	{
		assert(m_rtcp.sock.is_open());
		dstream2 << "VS_RTP_Channel(port=" << m_rtcp.localEndpoint.port() << "): failed to create RTCP RR packet for type=" << static_cast<int>(additionalType);
		pos = 0;
	}

	assert(pos < 1024);
	if (pos) {
		packet.shrink(0, pos);
		SendRTCP(std::move(packet));
	}

	return 0;
}


int VS_RTP_Channel::SendSR(uint32_t ssrc)
{
	vs::SharedBuffer packet(1024);
	size_t pos = 0;
	bool res = true;

	auto& stat = m_outgoing_statistics[ssrc];

	webrtc::rtcp::SenderReport sr;

	for (auto&& incom_stat : m_incoming_statistics)
	{
		if (!incom_stat.second.IsEmpty())
		{
			res = sr.AddReportBlock(incom_stat.second.GetReportBlock(incom_stat.first)) && res;
		}
	}

	Statistics(sr.report_blocks(), m_incoming_info);

	sr.SetSenderSsrc(ssrc);
	const auto ntp_now = rtcp_utils::ntp_time();
	sr.SetNtp(webrtc::NtpTime(ntp_now));
	sr.SetRtpTimestamp(stat.EstimatedRTPTimestamp(ntp_now, m_our_mode_clock_rate));
	sr.SetPacketCount(stat.GetPacketCount());
	sr.SetOctetCount(stat.GetOctetCount());
	res = static_cast<webrtc::rtcp::RtcpPacket&>(sr).Create(packet.data<uint8_t>(), &pos, packet.size() - pos, nullptr) && res;

	// Skype for Buisness (Link) support
	rtcp_utils::PeerInfoExchange pei = {};
	pei.type = 0x000C;
	pei.length = sizeof(rtcp_utils::PeerInfoExchange);
	pei.ssrc = ssrc;
	pei.inbound_link_bandwidth = 2147483647;
	pei.outbound_link_bandwidth = 2147483647;
	pei.NC = 0;
	const auto pei_size = rtcp_utils::WritePeerInfoExchange(pei, packet.data<uint8_t>() + pos, packet.size() - pos);
	pos += pei_size;
	res = (pei_size != 0) && res;

	/// hack for SR len
	*reinterpret_cast<uint16_t*>(packet.data<uint8_t>() + 2) = vs_htons((pos / 4) - 1);

	webrtc::rtcp::Sdes sdes;
	res = sdes.AddCName(ssrc, "CNAME") && res;
	res = static_cast<webrtc::rtcp::RtcpPacket&>(sdes).Create(packet.data<uint8_t>(), &pos, packet.size() - pos, nullptr) && res;

	if (!res)
	{
		assert(m_rtcp.sock.is_open());
		dstream2 << "VS_RTP_Channel(port=" << m_rtp.localEndpoint.port() << "): failed to create RTCP SR packet";
		pos = 0;
	}

	assert(pos < 1024);
	if (pos) {
		packet.shrink(0, pos);
		SendRTCP(std::move(packet));
	}

	return 0;
}



int VS_RTP_Channel::SendRTCP(webrtc::RTCPPacketType additionalType)
{

	if (m_outgoing_statistics.empty())
	{
		if (m_rtp_was_sent_last_time_interval) 	return -1;//error: smth wrong occured
		SendRR(GetReceiverOnlySSRC(), additionalType);
	}
	else
	{
		if (additionalType != 0 || !m_rtp_was_sent_last_time_interval)
		{
			for (auto& kv : m_outgoing_statistics)
			{
				SendRR(kv.first, additionalType);
			}
		}
		else
		{
			m_rtp_was_sent_last_time_interval = false;
			for (auto& kv : m_outgoing_statistics)
			{
				SendSR(kv.first);
			}
		}
	}
	return 0;
}


void VS_RTP_Channel::ParseRTCP(const uint8_t *buffer, const size_t &length)
{
	{	using namespace rtcp_utils;

		FeedbackMsgHeader fb_hdr;
		size_t off = 0;
		if (ParseFeedbackMsgHeader(buffer, length, fb_hdr)) {
			off += sizeof(FeedbackMsgHeader);

			VSRHeader vsr_hdr;
			if (fb_hdr.FMT == 15 && fb_hdr.PT == PSFB &&
				ParseVSRHeader(buffer + off, length - off, vsr_hdr)) {
				off += sizeof(VSRHeader);

				VSREntry vsr_ent;
				if (ParseVSREntry(buffer + off, length - off, vsr_ent)) {
					int max_fps = 7;
					if (vsr_ent.framerate_bitmask & _15) max_fps = 15;
					if (vsr_ent.framerate_bitmask & _25) max_fps = 25;
					if (vsr_ent.framerate_bitmask & _30) max_fps = 30;

					unsigned max_bitrate = vsr_ent.min_bitrate;
					for (int i = 9; i >= 0; i--) {
						if (vsr_ent.bitrate_histogram[i]) {
							max_bitrate += i * vsr_ent.bitrate_per_level;
							break;
						}
					}

					float preferred_aspect = 1;
					if (vsr_ent.aspect & _16x9) preferred_aspect = 9.0f / 16;
					else if (vsr_ent.aspect & _4x3) preferred_aspect = 3.0f / 4;

					int max_w = static_cast<int>(vsr_ent.max_w);
					int max_h = max_w * preferred_aspect;
					if (max_w * max_h > vsr_ent.max_num_pixels) {
						float k = float(vsr_ent.max_num_pixels) / (max_w * max_h);
						max_w *= k; max_h *= k;
						assert(max_w * max_h <= vsr_ent.max_num_pixels);
					}

					if (m_VSRCallBack) {
						m_VSRCallBack(max_w, max_h, max_fps, max_bitrate);
					}
					if (vsr_hdr.A && m_FIRCallBack) { // keyframe request
						m_FIRCallBack();
					}
					return;
				}
			}
		}
	}

	const uint8_t* const data = static_cast<const uint8_t*>(buffer);
	const uint8_t* const data_end = buffer + length;
	webrtc::rtcp::CommonHeader header;
	for (const uint8_t* next_packet = data; next_packet != data_end; next_packet = header.NextPacket()) {
		if (!header.Parse(next_packet, data_end - next_packet)) {
			return;
		}
		switch (header.type()) {
			case webrtc::rtcp::Psfb::kPacketType:
				switch (header.fmt()) {
					case webrtc::rtcp::Fir::kFeedbackMessageType:
					{
						if (m_FIRCallBack)
							m_FIRCallBack();
						break;
					}
				}
				break;

			case webrtc::rtcp::SenderReport::kPacketType:
			{
				webrtc::rtcp::SenderReport sender_report;
				sender_report.Parse(header);
				if (sender_report.ntp().Valid() && sender_report.sender_ssrc() > 0)
				{
					m_incoming_statistics[sender_report.sender_ssrc()].SetLastSrTS(static_cast<uint32_t>(static_cast<uint64_t>(sender_report.ntp()) >> 16));
					m_incoming_statistics[sender_report.sender_ssrc()].SetLastSrReceive(m_clock.now());
					if (!sender_report.report_blocks().empty()) m_not_empty_SR_was_received = true;
				}
				Statistics(sender_report.report_blocks(), m_outgoing_info);

				break;
			}
			case webrtc::rtcp::ReceiverReport::kPacketType:
			{
				webrtc::rtcp::ReceiverReport receiver_report;
				receiver_report.Parse(header);

				Statistics(receiver_report.report_blocks(), m_outgoing_info);

				break;
			}
			default:
				break;
		}
	}
}

void VS_RTP_Channel::SetFIRCallBack(std::function<void()> f)
{
	m_strand.post([self = shared_from_this(), f = std::move(f)] () mutable noexcept
	{
		self->m_FIRCallBack = std::move(f);
	});
}

void VS_RTP_Channel::SetVSRCallBack(std::function<void(int, int, int, unsigned)> f)
{
	m_strand.post([self = shared_from_this(), f = std::move(f)] () mutable noexcept
	{
		self->m_VSRCallBack = std::move(f);
	});
}

void VS_RTP_Channel::SendFIR()
{
	assert(m_version.load(std::memory_order_acquire) == 2);

	m_strand.post([self = shared_from_this(), this]()
	{
		if (!m_rtcp.sock.is_open())
			return;

		if (m_vsr_pt)
		{
			SendVSR();
		}
		else
		{
			SendRTCP(webrtc::kRtcpFir);
		}
	});
}

bool VS_RTP_Channel::ParseSTUNRequest(const unsigned char *data, unsigned size, uint32_t transactionId[3], std::string &outReqUser)
{
	stun::Header req_hdr;
	if (stun::ParseHeader(data, size, req_hdr)) {
		memcpy(&transactionId[0], &req_hdr.transaction_id[0], 4 * 3);

		unsigned data_len = req_hdr.msg_len;
		if (data_len > size) return false;
		const unsigned char *p_data = data + sizeof(stun::Header);
		unsigned off = 0;
		while (off < data_len) {
			uint16_t attrib_type = vs_ntohs(*(uint16_t *)&p_data[off]);
			uint16_t attrib_len = vs_ntohs(*(uint16_t *)&p_data[off + 2]);
			off += 4;

			if (attrib_type == stun::UserName) {
				return stun::ParseUsernameAttr(&p_data[off], attrib_len, outReqUser);
			}

			off += attrib_len;

			// skip padding
			while ((off & 0x03) != 0 && off < data_len) {
				off++;
			}
		}
	}

	return false;
}

void VS_RTP_Channel::WriteSTUNResponse(const uint32_t transactionId[3], const std::string &reqUser, unsigned long ip, net::port port, vs::SharedBuffer& packet)
{
	if (m_ice_pwd.empty()) return;

	stun::Header resp_hdr;
	resp_hdr.msg_type = stun::BindingResponse;
	resp_hdr.msg_len = 0;
	resp_hdr.magic = stun::magic;
	memcpy(&resp_hdr.transaction_id[0], &transactionId[0], 4 * 3);

	//unsigned long ip = m_rtp_ipv4_from ? vs_ntohl(*m_rtp_ipv4_from) : 0;
	//unsigned short port = m_rtp_port_from ? vs_ntohs(*m_rtp_port_from) : 0;

	packet = vs::SharedBuffer(512);
	unsigned char* buf_out = packet.data<unsigned char>();
	int buf_out_size = static_cast<int>(packet.size());

	unsigned off = 20;
	off += stun::WriteXorMappedAddress(ip, port, stun::magic, &buf_out[off], buf_out_size - off);
	off += stun::WriteUsernameAttr(reqUser, &buf_out[off], buf_out_size - off);
	off += stun::WriteMsImplementationVersion(2, &buf_out[off], buf_out_size - off);

	resp_hdr.msg_len = off - 20;	// minus header
	resp_hdr.msg_len += 24;			// make space for integrity attribute
	resp_hdr.msg_len += 8;			// make space for fingerprint attribute
	stun::WriteHeader(&resp_hdr, &buf_out[0], 20);

	unsigned char hmac[20];
	stun::CalculateIntegrity((const unsigned char *)&buf_out[0], off, m_ice_pwd, hmac);

	off += stun::WriteIntegrity(hmac, &buf_out[off], buf_out_size - off);

	uint32_t crc;
	stun::CalculateFingerprint(buf_out, off, &crc);

	off += stun::WriteFingerprint(crc, &buf_out[off], buf_out_size - off);

	packet.shrink(0, off);
}

void VS_RTP_Channel::ReceiveFromRTP()
{
	m_rtp.sock.async_receive_from(boost::asio::buffer(m_rtp.receiveBuffer.get(), m_rtp.bufferMaxLen), m_rtp.receiveFromEndpoint,
		m_strand.wrap([self = shared_from_this(), this](const boost::system::error_code & err,
		std::size_t size) mutable
	{
		if (err == boost::asio::error::operation_aborted)
			return;
		if (err)
		{
			dstream4 << "VS_RTP_Channel error receive RTP " << m_rtp.localEndpoint << " <- : " << err.message()
				<< ", err=" << err;
			if (net::IsRecoverableUDPReadError(err))
			{
				ReceiveFromRTP();
				return;
			}
			return;
		}

		Timeout();

		VS_SCOPE_EXIT{ ReceiveFromRTP(); };

		std::string req_user;
		uint32_t transaction_id[3];

		if (ParseSTUNRequest((const unsigned char *)m_rtp.receiveBuffer.get(), static_cast<unsigned>(size), transaction_id, req_user)) {
			if (m_rtp.receiveFromEndpoint.address().is_v4())
			{
				vs::SharedBuffer packet;
				WriteSTUNResponse(transaction_id, req_user, m_rtp.receiveFromEndpoint.address().to_v4().to_ulong(), m_rtp.receiveFromEndpoint.port(), packet);

				m_rtp.sendToEndpoint = std::move(m_rtp.receiveFromEndpoint);
				Send(std::move(packet), true);
			}

			return;
		}

		vs::SharedBuffer packet(size);
		std::memcpy(packet.data<void>(), m_rtp.receiveBuffer.get(), size);

		if (m_srtp_dec) {
			int len = static_cast<int>(packet.size());
			auto r = srtp_unprotect(m_srtp_dec->ctx, packet.data<void>(), &len);

			if (r != srtp_err_status_ok)
				return;

			packet.shrink(0, len);
		}
		else if (m_h235_session)
		{
			const auto encr_meta = m_h235_session->DecryptPacket(packet);

			FillStatistic(encr_meta, m_h235_decr_statistic);
			if (!encr_meta.succsess)
				return;
		}

		RTPPacket rtp_packet(packet.data<const void>(), packet.size());
		if (rtp_packet.IsValid() && rtp_packet.Version() == 2 && rtp_packet.SSRC() != 0)
		{
			const auto required_pair = m_remote_clock_rates.find(rtp_packet.PayloadType());
			if (required_pair != m_remote_clock_rates.cend())
			{
				m_incoming_statistics[rtp_packet.SSRC()].AddPacket(rtp_packet.SeqNo(), rtp_packet.Timestamp(), m_clock.now(), required_pair->second);
			}
		}

		if (m_RecieveCallBack)
			m_RecieveCallBack(std::move(packet));
	}));
}

void VS_RTP_Channel::ReceiveFromRTCP()
{
	m_rtcp.sock.async_receive_from(boost::asio::buffer(m_rtcp.receiveBuffer.get(), m_rtcp.bufferMaxLen), m_rtcp.receiveFromEndpoint,
		m_strand.wrap([self = shared_from_this(), this](const boost::system::error_code &err,
			std::size_t bytes) mutable
	{
		if (err == boost::asio::error::operation_aborted)
			return;

		if (err)
		{
			dstream4 << "VS_RTP_Channel error receive RTCP " << m_rtcp.localEndpoint << " <- : " << err.message()
				<< ", err=" << err;
			if (net::IsRecoverableUDPReadError(err))
			{
				ReceiveFromRTCP();
				return;
			}
			return;
		}

		Timeout();

		std::string req_user;
		uint32_t transaction_id[3];

		if (ParseSTUNRequest(reinterpret_cast<const unsigned char *>(m_rtcp.receiveBuffer.get()), static_cast<unsigned>(bytes), transaction_id, req_user)) {
			if (m_rtcp.receiveFromEndpoint.address().is_v4()) {
				vs::SharedBuffer packet;
				WriteSTUNResponse(transaction_id, req_user, m_rtcp.receiveFromEndpoint.address().to_v4().to_ulong(), m_rtcp.receiveFromEndpoint.port(), packet);

				m_rtcp.sendToEndpoint = std::move(m_rtcp.receiveFromEndpoint);
				SendRTCP(std::move(packet), false);
			}

			ReceiveFromRTCP();
			return;
		}

		if (m_srtp_dec) {
			int32_t size = static_cast<int32_t>(bytes);
			const auto r = srtp_unprotect_rtcp(m_srtp_dec->ctx, m_rtcp.receiveBuffer.get(), &size);
			if (r == srtp_err_status_ok)
				ParseRTCP(reinterpret_cast<const uint8_t *>(m_rtcp.receiveBuffer.get()), size);
		}
		else
			ParseRTCP(reinterpret_cast<const uint8_t *>(m_rtcp.receiveBuffer.get()), bytes);

		ReceiveFromRTCP();

	}));
}

void VS_RTP_Channel::SendRTCP(vs::SharedBuffer packet, bool protect)
{
	if (packet.empty()) return;

	if (protect && m_srtp_cod)
	{
		vs::SharedBuffer protected_packet(packet.size() + SRTP_MAX_TRAILER_LEN + 4);
		std::memcpy(protected_packet.data<void>(), packet.data<const void>(), packet.size());
		int len = static_cast<int>(packet.size());
		auto r = srtp_protect_rtcp(m_srtp_cod->ctx, protected_packet.data<void>(), &len);
		if (r == srtp_err_status_ok) {
			protected_packet.shrink(0, len);
			packet = std::move(protected_packet);
		}
	}

	Send(std::move(packet), false);
}

bool VS_RTP_Channel::SendVSR()
{
	assert(m_vsr_pt);

	using namespace rtcp_utils;

	for (auto& kv : m_outgoing_statistics)
	{
		vs::SharedBuffer packet(sizeof(FeedbackMsgHeader) + sizeof(VSRHeader) + sizeof(VSREntry));
		const size_t sz = make_video_source_request(kv.first ? kv.first : GetReceiverOnlySSRC()/*CreateSSRC()*/, 0, m_vsr_pt, packet.data<unsigned char>(), packet.size());
		packet.shrink(0, sz);
		SendRTCP(std::move(packet));
	}

	return !m_outgoing_statistics.empty();
}

void VS_RTP_Channel::InitSRTP(const std::string &b64OurKey, const std::string &b64RemoteKey) {
	assert(!b64OurKey.empty());
	assert(!b64RemoteKey.empty());

	m_strand.post([self = shared_from_this(), this, b64OurKey, b64RemoteKey]()
	{
		m_srtp_shared = GetSrtpSharedHandle();
		{
			auto srtp_dec = std::make_shared<VS_SRTP>();

			srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&srtp_dec->policy.rtp);
			srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&srtp_dec->policy.rtcp);

			size_t key_len = 0;
			base64_decode(b64RemoteKey.c_str(), b64RemoteKey.length(), nullptr, key_len);
			srtp_dec->policy.key = static_cast<unsigned char*>(malloc(key_len)); // will be freed in SRTP::~SRTP
			base64_decode(b64RemoteKey.c_str(), b64RemoteKey.length(), srtp_dec->policy.key, key_len);
			assert(key_len == static_cast<size_t>(srtp_dec->policy.rtp.cipher_key_len));

			srtp_dec->policy.ekt = nullptr;
			srtp_dec->policy.next = nullptr;
			srtp_dec->policy.window_size = 128;
			srtp_dec->policy.allow_repeat_tx = 0;
			srtp_dec->policy.rtp.sec_serv = sec_serv_conf_and_auth;
			srtp_dec->policy.rtcp.sec_serv = sec_serv_conf_and_auth;
			srtp_dec->policy.ssrc.type = ssrc_any_outbound;

			const auto res = srtp_create(&srtp_dec->ctx, &srtp_dec->policy);
			assert(res == srtp_err_status_ok);

			m_srtp_dec = std::move(srtp_dec);
		}
		{
			auto srtp_cod = std::make_shared<VS_SRTP>();

			srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&srtp_cod->policy.rtp);
			srtp_crypto_policy_set_aes_cm_128_hmac_sha1_80(&srtp_cod->policy.rtcp);

			size_t key_len = 0;
			base64_decode(b64OurKey.c_str(), b64OurKey.length(), nullptr, key_len);
			srtp_cod->policy.key = static_cast<unsigned char*>(malloc(key_len)); // will be freed in SRTP::~SRTP
			base64_decode(b64OurKey.c_str(), b64OurKey.length(), srtp_cod->policy.key, key_len);
			assert(key_len == static_cast<size_t>(srtp_cod->policy.rtp.cipher_key_len));

			srtp_cod->policy.ekt = nullptr;
			srtp_cod->policy.next = nullptr;
			srtp_cod->policy.window_size = 128;
			srtp_cod->policy.allow_repeat_tx = 0;
			srtp_cod->policy.rtp.sec_serv = sec_serv_conf_and_auth;
			srtp_cod->policy.rtcp.sec_serv = sec_serv_conf_and_auth;
			srtp_cod->policy.ssrc.type = ssrc_any_inbound;

			const auto res = srtp_create(&srtp_cod->ctx, &srtp_cod->policy);
			assert(res == srtp_err_status_ok);

			m_srtp_cod = std::move(srtp_cod);
		}
	});
}

void VS_RTP_Channel::InitH235MediaSession(const VS_H235SecurityCapability *pRecvCap, const VS_H235SecurityCapability *pSendCap, const int recvPayload) {
	m_strand.post([self = shared_from_this(), this, pRecvCap, pSendCap, recvPayload]()
	{
		auto h235_session = std::make_shared<VS_H235Session>();
		if (m_h235_session) m_h235_session->CloneTo(*h235_session);
		VS_SCOPE_EXIT{ if (h235_session->Valid()) m_h235_session = std::move(h235_session); };

		if (!h235_session->Valid())
			h235_session = std::make_shared<VS_H235Session>(pRecvCap, pSendCap, recvPayload);
		else {
			// if we need to update keys
			h235_session->InitGenericContext(pSendCap);
			h235_session->InitRecvContext(pRecvCap, recvPayload);
		}
	});
}

void VS_RTP_Channel::SetICEPWD(std::string s)
{
	m_strand.post([self = shared_from_this(), s = std::move(s)]() mutable noexcept
	{
		self->m_ice_pwd = std::move(s);
	});
}

void VS_RTP_Channel::SetVSRPT(int pt)
{
	m_strand.post([self = shared_from_this(), pt]()
	{
		self->m_vsr_pt = pt;
	});
}

void VS_RTP_Channel::SetSSRCRange(std::pair<uint32_t, uint32_t> p)
{
	m_strand.post([self = shared_from_this(), p]()
	{
		self->m_ssrc_range = p;
	});
}

VS_RTPMediaChannels::VS_RTPMediaChannels(boost::asio::io_service& ios)
	: m_strand(ios)
{
}

VS_RTPMediaChannels::~VS_RTPMediaChannels()
{
}


std::shared_ptr<VS_RTP_Channel> VS_RTPMediaChannels::Get(std::size_t id, bool create)
{
	auto it = m_channels.find(id);
	if (it == m_channels.cend())
	{
		if (create)
			it = m_channels.emplace(id, vs::MakeShared<VS_RTP_Channel>(m_strand)).first;
		else
			return nullptr;
	}
	return it->second;
}


bool VS_RTPMediaChannels::Disconnect(size_t id) noexcept
{
	auto it = m_channels.find(id);
	if (it == m_channels.cend())
		return false;
	it->second->Disconnect();
	m_channels.erase(it);
	return true;
}

void VS_RTPMediaChannels::DisconnectAll() noexcept
{
	for (auto &item : m_channels)
	{
		item.second->Disconnect();
	}
	m_channels.clear();
}

void VS_RTP_Channel::SetRemoteRTPClockRates(std::map<uint8_t, uint32_t> clockRatesMap)
{
	m_strand.post([self = shared_from_this(), clockRatesMap = std::move(clockRatesMap)]() mutable noexcept
	{
		self->m_remote_clock_rates = std::move(clockRatesMap);
	});
}

void VS_RTP_Channel::SetLocalRTPClockRate(uint32_t ourModeClockRate)
{
	m_strand.post([self = shared_from_this(), ourModeClockRate]()
	{
		self->m_our_mode_clock_rate = ourModeClockRate;
	});
}

uint32_t VS_RTP_Channel::GetReceiverOnlySSRC()
{
	if (!m_tmp_receiver_only_ssrc)
		return (m_tmp_receiver_only_ssrc = CreateSSRC(m_ssrc_range));
	return m_tmp_receiver_only_ssrc;
}

void VS_RTP_Channel::Statistics(const std::vector<webrtc::rtcp::ReportBlock>& reportBlocks, StatisticsInfo& info) const
{
	std::lock_guard<decltype(m_mutex)> _{ m_mutex };
	for (auto&& report_block : reportBlocks)
	{
		const auto clock_rate = GetClockRateBySSRC(report_block.source_ssrc());
		assert(clock_rate != 0);

		//need convert jitter from TS to miliseconds
		info.lastJitter = report_block.jitter() * 1000 / (clock_rate == 0 ? vs::default_video_clock_rate : clock_rate);
		// "* 1000" - because miliseconds, "/ rtp_rate" - because for jitter calculations uses clock rate
		info.maxJitter = info.lastJitter > info.maxJitter ? info.lastJitter : info.maxJitter;

		info.totalLossPercent = report_block.fraction_lost() * 100 / 255;
		// "*100" - because percents; "/ 255" - because loss_fraction in RTCP calculations with *255

		info.totalLostPackets = report_block.cumulative_lost();
	}
}

void VS_RTP_Channel::FillStatistic(const encryption_meta& m, EncryptionStatistic& st) const
{
	std::lock_guard<decltype(m_mutex)> _{m_mutex};
	++st.packets;
	if (m.succsess)
	{
		++st.encryptedDecrypted;
		if (m.encr_m == encryption_method::cts) ++st.ctsMethod;
		else if (m.encr_m == encryption_method::padding) ++st.paddingMethod;
	}
}

uint32_t VS_RTP_Channel::GetClockRateBySSRC(uint32_t ssrc) const
{
	for (auto&& stat : m_incoming_statistics)
	{
		if (stat.first == ssrc)
		{
			return stat.second.GetClockRate();
		}
	}
	for (auto&& stat : m_outgoing_statistics)
	{
		if (stat.first == ssrc)
		{
			return stat.second.GetClockRate();
		}
	}
	return vs::default_video_clock_rate;
}
