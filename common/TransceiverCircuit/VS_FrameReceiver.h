#pragma once
#include "streams/Relay/VS_ConfControlInterface.h"
#include "VS_FrameReceiverConnector.h"
#include "streams/Relay/Types.h"
#include "TransceiverLib/VS_NetChannelsRouter.h"

#include "std-generic/compat/memory.h"
#include <chrono>
#include <map>
#include <set>

#include <boost/signals2.hpp>

class VS_NetworkRelayMessageBase;
class VS_TransmitFrameInterface;
class VS_MainRelayMessage;


class VS_FrameReceiver: public VS_ConfControlInterface,
						public VS_FrameReceiverConnector,
						public VS_NetChannelsRouter
{
public:
	virtual ~VS_FrameReceiver();
	VS_FrameReceiver(const std::string &addrs, std::string circuit_name, unsigned char *secretData, const unsigned long sz, boost::asio::io_service &ios);
private:
	bool ConnectToTransmitFrame(const char * conf_name, const std::shared_ptr<VS_TransmitFrameInterface> &trans_fr_cb) override;
	bool ConnectToTransmitFrameImpl(const std::string &conf_name, const std::shared_ptr<VS_TransmitFrameInterface> &trans_fr_cb);
	bool DisconnectFromTransmitFrame(const char *conf_name, const std::shared_ptr<VS_TransmitFrameInterface> &trans_fr_cb) override;
	bool DisconnectFromTransmitFrameImpl(const std::string &conf_name, const std::shared_ptr<VS_TransmitFrameInterface> &trans_fr_cb);

	void StartConference(const char *conf_name) override;
	void StartConference(const char *conf_name, const char *owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type) override;
	void StopConference(const char *conf_name) override;
	void StopConferenceImpl(const std::string &conf_name);
	void ParticipantConnect(const char *conf_name, const char *part_name) override;
	void ParticipantDisconnect(const char *conf_name, const char *part_name) override;
	void SetParticipantCaps(const char *conf_name, const char *part_name,const void *caps_buf,const unsigned long buf_sz) override;
	void RestrictBitrateSVC(const char *conferenceName, const char *participantName, long v_bitrate, long bitrate, long old_bitrate) override {}
	void RequestKeyFrame(const char *conferenceName, const char *participantName) override {}

	void ProcessingRcvMessage(const boost::shared_ptr<VS_MainRelayMessage> &mess) override;

	void TransmitFrame(const std::weak_ptr<VS_TransmitFrameInterface> &circuit, const char *conf_name, const char *part, const stream::FrameHeader *frame_head, const void *frame_data);

	std::string m_serverAddrList;
	std::vector<unsigned char>			m_secretData;

	boost::signals2::signal<void (const char *, const char *, const stream::FrameHeader *, const void *)>			m_fireTransmitFrame;

	std::map<std::string, boost::shared_ptr<stream::FrameReceivedSignalType>> m_transmitFrameSignalsByConf;
	std::map<std::tuple<std::string, VS_TransmitFrameInterface*>, boost::signals2::connection> m_circuitsCollection;

	int64_t m_byteReceived;
	std::chrono::steady_clock::time_point m_timeLastCalc;
};