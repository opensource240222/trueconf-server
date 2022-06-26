#pragma once
#include <boost/signals2.hpp>
#include <chrono>
#include <string>

#include "api/peerconnectioninterface.h"

#include "VS_PeerConnectionPoolItemObserver.h"
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/compat/memory.h"
#include "std-generic/compat/map.h"
#include "std-generic/compat/functional.h"


class VS_WebRtcMediaPeer;
class VS_RelayMediaSource;
class VS_FrameReceiverConnector;
class VS_ConfConditionConnector;
class VS_PeerConnectionPoolItem;
class VS_RelayMessageSenderInterface;
class VS_TransceiverParticipant;
class VS_TransceiverPartsMgr;

class VS_WebRTCConfBroadCast
	: public VS_PeerConnectionPoolItemObserver
	, public VS_PoolItemStatsObserver
	, public vs::enable_shared_from_this<VS_WebRTCConfBroadCast>
{
public:
	virtual ~VS_WebRTCConfBroadCast();
	bool ConnectPeerConnection(const char* peer_id, const char *remote_sdp, const char* trueconf_id,
		rtc::Thread* worker, rtc::Thread* signaling, rtc::Thread* network, unsigned long limit_n_peers, bool receive_from_peer, int min_port, int max_port, const char* external_ip);

	bool SetCallBackForCPLChange(const char *peer, boost::signals2::signal<void(const char*, const char*, const int)>::slot_type slot);

	bool DisconnectPeerConnection(const char *peer_id);
	bool DisconnectPart(const char *part_id);
	bool IsEmpty() const;
	void Close(bool IsFromTrueConf);
	void GenKeyFrame(const char *part_id);

	void AddIceCandidate(const char* id, const char* sdp, const char* sdp_mid, int sdp_mlineindex);


protected:
	VS_WebRTCConfBroadCast(const char* conf_name, const std::shared_ptr<VS_RelayMediaSource>& media_source,
		const std::shared_ptr<VS_FrameReceiverConnector>& frame_connector, const std::shared_ptr<VS_ConfConditionConnector>& cond_connector,
		const std::shared_ptr<VS_RelayMessageSenderInterface>& sender,
		std::shared_ptr<VS_TransceiverPartsMgr> parts_mgr,
		boost::asio::io_service& ios);

	static void PostConstruct(std::shared_ptr<VS_WebRTCConfBroadCast> &p)
	{
		p->Init();
	}

	virtual void Timeout();

	virtual void OnComplete(const char* peer_id, const webrtc::StatsReports &reports);

	/////VS_PeerConnectionPoolItemObserver

	virtual void OnError(const char *id);

	// Triggered when the SignalingState changed.
	virtual void OnSignalingChange(const char *id,
		webrtc::PeerConnectionInterface::SignalingState new_state);

	// Triggered when SignalingState or IceState have changed.
	// TODO(bemasc): Remove once callers transition to OnSignalingChange.
	virtual void OnStateChange(const char *id/*, webrtc::PeerConnectionObserver::StateType state_changed*/);

	// Triggered when media is received on a new stream from remote peer.
	virtual void OnAddStream(const char *id, webrtc::MediaStreamInterface* stream);

	// Triggered when track
	virtual void OnAddTrack(const char *id, webrtc::RtpReceiverInterface *receiver);

	// Triggered when a remote peer close a stream.
	virtual void OnRemoveStream(const char *id, webrtc::MediaStreamInterface* stream);

	// Triggered when a remote peer open a data channel.
	// TODO(perkj): Make pure virtual.
	virtual void OnDataChannel(const char *id, webrtc::DataChannelInterface* data_channel);

	// Triggered when renegotation is needed, for example the ICE has restarted.
	virtual void OnRenegotiationNeeded(const char *id);

	// Called any time the IceConnectionState changes
	virtual void OnIceConnectionChange(const char *id, webrtc::PeerConnectionInterface::IceConnectionState new_state);

	// Called any time the IceGatheringState changes
	virtual void OnIceGatheringChange(const char *id,
		webrtc::PeerConnectionInterface::IceGatheringState new_state);

	// New Ice candidate have been found.
	virtual void OnIceCandidate(const char *id, const webrtc::IceCandidateInterface* candidate);

	// TODO(bemasc): Remove this once callers transition to OnIceGatheringChange.
	// All Ice candidates have been found.
	virtual void OnIceComplete(const char *id);

	///sdp
	virtual void OnSuccess(const char *id, webrtc::SessionDescriptionInterface* desc);
	virtual void OnFailure(const char * id, const std::string& error);
private:
	void SetLocalDecs(const std::string& id, const std::string& sdp);
	void OnSetLocalDesc(const char* id, const char* conf, const char* sdp);
	void SheduleTimer(const std::chrono::milliseconds period = std::chrono::seconds(1));

private:
	void Init();

	void OnAudioPLChanged(const char *id, const char *plname, const int plfreq);
	void OnMediaSourceData(const char *id, const char *conf_name, const char *data);
	bool SetPeerStat(const char *peer, const std::shared_ptr<VS_WebRtcMediaPeer> &media_peer);
	///////////////////////////////////////

	bool ConnectPeerConnectionImpl(const std::string& peer_id, const std::string& remote_sdp, const std::string& trueconf_id,
		rtc::Thread* worker, rtc::Thread* signaling, rtc::Thread* network, unsigned long limit_n_peers, bool receive_from_peer, int min_port, int max_port, const std::string& external_ip);
	bool DisconnectPeerConnectionImpl(const std::string& peer_id);
	bool DisconnectPartImpl(const std::string& part_id);
	void CloseImpl(bool IsFromTrueConf);
	void GenKeyFrameImpl(const std::string& part_id);
	void AddIceCandidateImpl(const std::string& id, const std::string& sdp, const std::string& sdp_mid, int sdp_mlineindex);

	std::string		m_conf_name;
	std::weak_ptr<VS_RelayMediaSource>			m_media_source;
	std::weak_ptr<VS_FrameReceiverConnector>		m_frameConnector;
	std::weak_ptr<VS_ConfConditionConnector>		m_condConnector;
	std::weak_ptr<VS_RelayMessageSenderInterface> m_message_sender;
	std::shared_ptr<VS_TransceiverPartsMgr>		m_partsMgr;
	///webrtc
	/**
	portallocator
	webrtc threads
	*/

	struct peer_info
	{
		// We need to prevent VS_WebRtcMediaPeer from destruction, because it holds instance of
		// tc_TrueconfMediaEngine which is used by VS_PeerConnectionPoolItem::peer_connection_
		// and VS_PeerConnectionPoolItem::peer_connection_factory_.
		// Field order is important as it ensures correct destruction order.
		std::shared_ptr<VS_WebRtcMediaPeer> media_peer;
		rtc::scoped_refptr<VS_PeerConnectionPoolItem> peer_connection;
	};
	vs::map<std::string, peer_info, vs::str_less> m_peer_pool;

	std::chrono::steady_clock::time_point m_lastGetStatTime;
	boost::asio::steady_timer	m_timer;
	mutable boost::asio::io_service::strand	m_strand;
};
