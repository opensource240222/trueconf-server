#pragma once

#include "VS_RTCPStatistics.h"
#include "net/QoS.h"
#include "net/Port.h"
#include "SecureLib/SecureTypes.h"
#include "std-generic/cpplib/SharedBuffer.h"
#include "std-generic/cpplib/VS_ClockWrapper.h"

#include "modules/rtp_rtcp/include/rtp_rtcp_defines.h"
#include "pc/srtp_shared.h"

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/circular_buffer.hpp>

#include "std-generic/compat/memory.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <string>

class VS_H235Session;
struct VS_SRTP;
struct VS_H235SecurityCapability;

class VS_RTP_Channel : public vs::enable_shared_from_this<VS_RTP_Channel>
{
public:

	typedef boost::asio::ip::udp::socket udp_socket_t;
	typedef udp_socket_t::endpoint_type endpoint_t;

	struct EncryptionStatistic
	{
		unsigned packets = 0;
		unsigned encryptedDecrypted = 0;
		unsigned ctsMethod = 0;
		unsigned paddingMethod = 0;
	};

	struct StatisticsInfo
	{
		uint64_t totalSentPackets;	// total number of sent packets
		uint32_t totalLostPackets;	// total number of lost packets
		float    totalLossPercent;  // percent of lost packets
		uint32_t lastJitter;        // last jitter in miliseconds
		uint32_t maxJitter;         // max jitter in miliseconds
	};


	~VS_RTP_Channel();

	void SetReceiveCallBack(std::function<void (vs::SharedBuffer&&)>);
	void SetFIRCallBack(std::function<void()>);
	void SetVSRCallBack(std::function<void(int, int, int, unsigned)>);

	bool Start(bool isIpv6 = false);

	void SetRemoteRTPEndpoint(endpoint_t endpoint);
	void SetRemoteRTCPEndpoint(endpoint_t endpoint);

	void SendRTP(vs::SharedBuffer packet, bool protect = true);
	void SendFIR();
	void SendFakePackets(size_t count = 2);

	bool GetLocalEndpoints(endpoint_t& localRTPEndp, endpoint_t& localRTCPEndp) const;

	void Disconnect() noexcept;
	bool IsDisconnected() const noexcept
	{
		return m_channel_disconnect.load(std::memory_order_acquire);
	}

	void InitSRTP(const std::string &b64OurKey, const std::string &b64RemoteKey);
	void InitH235MediaSession(const VS_H235SecurityCapability *pRecvCap, const VS_H235SecurityCapability *pSendCap, const int recvPayload);

	void SetICEPWD(std::string s);

	void SetVSRPT(int pt);

	void SetSSRCRange(std::pair<uint32_t, uint32_t> p);

	EncryptionStatistic EncryptStatistic() const
	{
		std::lock_guard<decltype(m_mutex)> _{ m_mutex };
		return m_h235_encr_statistic;
	}

	EncryptionStatistic DecryptionStatistic() const
	{
		std::lock_guard<decltype(m_mutex)> _{ m_mutex };
		return m_h235_decr_statistic;
	}

	void SetRemoteRTPClockRates(std::map<uint8_t, uint32_t> clockRatesMap);

	void SetLocalRTPClockRate(uint32_t ourModeClockRate);

	StatisticsInfo IncomingStatistics() const
	{
		std::lock_guard<decltype(m_mutex)> _{ m_mutex };
		return m_incoming_info;
	}
	StatisticsInfo OutgoingStatistics() const
	{
		std::lock_guard<decltype(m_mutex)> _{ m_mutex };
		return m_outgoing_info;
	}

protected:
	VS_RTP_Channel(boost::asio::io_service::strand& strand);
	steady_clock_wrapper &clock() { return m_clock; }

private:

	struct Session final
	{
		std::unique_ptr<char[]> receiveBuffer;

		udp_socket_t sock;

		endpoint_t receiveFromEndpoint;
		endpoint_t sendToEndpoint;
		endpoint_t localEndpoint;

		using queue_type = boost::circular_buffer<vs::SharedBuffer>;
		queue_type outQueue;

		const std::size_t bufferMaxLen;
		const std::size_t queueMaxSize;

		Session(boost::asio::io_service &ios, std::size_t bufferLen, std::size_t queueSize,  std::size_t queueCapacity)
			: receiveBuffer(vs::make_unique_default_init<char[]>(bufferLen)), sock(ios), outQueue(queueCapacity), bufferMaxLen(bufferLen), queueMaxSize(queueSize){}
	};

private:
	void Statistics(const std::vector<webrtc::rtcp::ReportBlock>& reportBlocks, StatisticsInfo& info) const;
	void FillStatistic(const encryption_meta& m, EncryptionStatistic& st) const;

	uint32_t GetClockRateBySSRC(uint32_t ssrc) const;
	uint32_t GetLocalRTPClockRate() const;

	void Timeout();

	void ParseRTCP(const uint8_t *buffer, const size_t &length);
	int SendRTCP(webrtc::RTCPPacketType additionalType = {});
	int SendSR(uint32_t ssrc);
	int SendRR(uint32_t ssrc, webrtc::RTCPPacketType additionalType);


	bool ParseSTUNRequest(const unsigned char *data, unsigned size, uint32_t transactionId[3], std::string &outReqUser);
	void WriteSTUNResponse(const uint32_t transactionId[3], const std::string &reqUser, unsigned long ip, net::port port, vs::SharedBuffer& buffer);

	void ReceiveFromRTP();
	void ReceiveFromRTCP();

	void SendRTCP(vs::SharedBuffer packet, bool protect = true);
	bool SendVSR();

	uint32_t GetReceiverOnlySSRC();

	void Write(bool rtp);
	void Send(vs::SharedBuffer packet, bool rtp);
	Session &GetSession(bool rtp);

private:
	boost::asio::io_service::strand m_strand; // This is a copy of the strand from the parent VS_RTPMediaChannels object.
	Session m_rtp;
	Session m_rtcp;
	std::shared_ptr<VS_SRTP> m_srtp_cod;
	std::shared_ptr<VS_SRTP> m_srtp_dec;
	SrtpSharedHandle m_srtp_shared;
	std::shared_ptr<VS_H235Session> m_h235_session;
	std::map<uint8_t, uint32_t> m_remote_clock_rates;
	std::map<uint32_t, VS_RTCPOutgoingStatistics> m_outgoing_statistics;
	std::map<uint32_t, VS_RTCPIncomingStatistics> m_incoming_statistics;
	std::string m_ice_pwd;
	std::function<void(vs::SharedBuffer&&)> m_RecieveCallBack;
	std::function<void()>  m_FIRCallBack;
	std::function<void(int, int, int, unsigned)> m_VSRCallBack;
	StatisticsInfo m_incoming_info;
	StatisticsInfo m_outgoing_info;
	EncryptionStatistic m_h235_encr_statistic;
	EncryptionStatistic m_h235_decr_statistic;

	typedef std::mutex mutex_t;
	mutable mutex_t m_mutex;

	steady_clock_wrapper m_clock;

	std::pair<uint32_t, uint32_t> m_ssrc_range;
	uint32_t m_tmp_receiver_only_ssrc;
	uint32_t m_our_mode_clock_rate;
	unsigned m_can_send_fake_packets;
	int m_vsr_pt;
	uint8_t m_fir_seqno_counter; // Technically there should unique counter for each (local SSRC, remote SSRC) pair, but who cares? (WebRTC doesn't)
	std::chrono::steady_clock::time_point m_last_sendRTCP;
	std::chrono::steady_clock::time_point m_last_sendVSR;
	std::atomic_bool m_channel_disconnect;

	bool m_not_empty_SR_was_received;
	bool m_rtp_was_sent_last_time_interval;


	net::QoSFlowSharedPtr m_rtp_flow;
	net::QoSFlowSharedPtr m_rtcp_flow;

#ifndef NDEBUG
	std::atomic<int> m_version; // 0 - not executed Start; 1 - start execute Start; 2 - done success execute Start
#endif
};

class VS_RTPMediaChannels
{
public:
	VS_RTPMediaChannels(boost::asio::io_service& ios);
	~VS_RTPMediaChannels();

	std::shared_ptr<VS_RTP_Channel> Get(std::size_t id, bool create = false);

	bool Disconnect(std::size_t id) noexcept;
	void DisconnectAll() noexcept;
	bool IsDisconnected() const noexcept
	{
		return m_channels.empty();
	}

	template <class Handler>
	void ForEach(Handler fn)
	{
		for (auto& x: m_channels)
			fn(x.first, x.second.get());
	}

	template <class Handler>
	void ForEach(Handler fn) const
	{
		for (const auto& x: m_channels)
			fn(x.first, x.second.get());
	}

private:
	boost::asio::io_service::strand m_strand;
	std::map<std::size_t /*id*/, std::shared_ptr<VS_RTP_Channel>> m_channels;
};