#pragma once
#include <string>
#include <memory>
#include <mutex>

#include "VS_FrameReceiverConnector.h"
#include "TransceiverLib/VS_RelayModule.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "streams/Relay/VS_ConfControlInterface.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/compat/memory.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include "std-generic/compat/map.h"
#include "std-generic/compat/functional.h"

namespace cricket
{
	class PortAllocator;
}
namespace rtc
{
	class Thread;
}

class VS_RelayMediaSource;
class VS_WebRTCConfBroadCast;
class VS_WebrtcRelayMsg;
class VS_MessageHandlerAdapter;
class VS_MediaSourceCollection;
class VS_TransceiverPartsMgr;
class VS_ConfConditionConnector;

class VS_WebRTCBroadcaster
	: public VS_RelayModule
	, public VS_ConfControlInterface
	, public vs::enable_shared_from_this<VS_WebRTCBroadcaster>
{
	class StopConferenceMessage;
	class ParticipantDisconnectMessage;

protected:
	VS_WebRTCBroadcaster(	const std::shared_ptr<VS_FrameReceiverConnector> &frame_connector,
							const std::shared_ptr<VS_ConfConditionConnector> &cond_connector,
							boost::asio::io_service& ios,
							const boost::shared_ptr<VS_MediaSourceCollection> &coll,
							const std::shared_ptr<VS_TransceiverPartsMgr>& partsMgr);

public:
	virtual ~VS_WebRTCBroadcaster();

	void Close();

private:
	bool ConnectPeerConnection(const char* peer_id, const char *remote_sdp, const char *conf_name, const char* trueconf_id,
		unsigned long limit_n_peers = ULONG_MAX, bool receive_from_peer = false, int min_port = 0, int max_port = 0, const char* external_ip = 0);
	void DisconnectPeerConnection(const char *peer_id, const char *conf_name);
	void DisconnectPeerConnectionFromAllConf(const char *peer_id);
	void ManageAnyData(const char*conf_name, const char *peer_id, const char* command);

	bool ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess) override;
	void ProcessMessageFromServer();

	void StartConference(const char *conf_name) override;
	void StartConference(const char *conf_name, const char *owner_name, VS_Conference_Type, VS_GroupConf_SubType) override;
	void StopConference(const char *conf_name) override;
	void ParticipantConnect(const char *conf_name, const char *part_name) override {};
	void ParticipantDisconnect(const char *conf_name, const char *part_name) override;
	void SetParticipantCaps(const char *conf_name, const char *part_name, const void *caps_buf, const unsigned long buf_sz) override {}
	void RestrictBitrateSVC(const char *conferenceName, const char *participantName, long v_bitrate, long bitrate, long old_bitrate) override{}
	void RequestKeyFrame(const char *conferenceName, const char *participantName) override{};

	bool ProcessingOffer(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess);
	void AddIceCandidate(boost::shared_ptr<VS_WebrtcRelayMsg>& wrtc_mess);
	void StopConferenceImpl(const std::string& conf_name);
	void ParticipantDisconnectImpl(const std::string& conf_name, const std::string& part_name);

	///bool ProcesingAnswer

	vs::map<std::string,std::shared_ptr<VS_WebRTCConfBroadCast>, vs::str_less> m_conf_broadcaster;
	std::weak_ptr<VS_FrameReceiverConnector>	m_frameConnector;
	std::weak_ptr<VS_ConfConditionConnector>	m_condConnector;
	std::shared_ptr<VS_TransceiverPartsMgr>	m_partsMgr;

	boost::shared_ptr<VS_MediaSourceCollection>	m_media_source_collection;

	std::multimap<std::string, boost::shared_ptr<VS_WebrtcRelayMsg>>	m_msg_from_server;		// [key=peer_id,value=his_messages]
	std::string															m_msg_from_server_last_peer_id;

	////init data for peerconection factory
	rtc::Thread* m_webrtc_signaling_thread;
	rtc::Thread* m_webrtc_worker_thread;
	std::vector<std::unique_ptr<rtc::Thread>> m_webrtc_network_thread_pool;
	uint64_t m_counterPeers;

	boost::asio::io_service::strand m_strand;
};