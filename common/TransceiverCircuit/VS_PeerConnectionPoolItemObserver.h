#pragma once
///#include "talk/app/webrtc/peerconnectioninterface.h"

class VS_PoolItemStatsObserver
{
public:
	virtual void OnComplete(const char* peer_id, const webrtc::StatsReports &reports) = 0;
};

class VS_PeerConnectionPoolItemObserver
{
public:
	virtual ~VS_PeerConnectionPoolItemObserver(){}
///PeerConnectionObserver

  virtual void OnError(const char *id) = 0;

  // Triggered when the SignalingState changed.
  virtual void OnSignalingChange(const char *id,
	  webrtc::PeerConnectionInterface::SignalingState new_state) = 0;

  // Triggered when SignalingState or IceState have changed.
  // TODO(bemasc): Remove once callers transition to OnSignalingChange.
  virtual void OnStateChange(const char *id/*,webrtc::PeerConnectionObserver::StateType state_changed*/) = 0;

  // Triggered when media is received on a new stream from remote peer.
  virtual void OnAddStream(const char *id, webrtc::MediaStreamInterface* stream) = 0;

  // Triggered when new track
  virtual void OnAddTrack(const char *id, webrtc::RtpReceiverInterface *receiver) = 0;

  // Triggered when a remote peer close a stream.
  virtual void OnRemoveStream(const char *id,webrtc::MediaStreamInterface* stream) = 0;

  // Triggered when a remote peer open a data channel.
  // TODO(perkj): Make pure virtual.
  virtual void OnDataChannel(const char *id,webrtc::DataChannelInterface* data_channel) = 0;

  // Triggered when renegotation is needed, for example the ICE has restarted.
  virtual void OnRenegotiationNeeded(const char *id) = 0;

  // Called any time the IceConnectionState changes
  virtual void OnIceConnectionChange(const char *id,webrtc::PeerConnectionInterface::IceConnectionState new_state) = 0;

  // Called any time the IceGatheringState changes
  virtual void OnIceGatheringChange(const char *id,
	  webrtc::PeerConnectionInterface::IceGatheringState new_state) = 0;

  // New Ice candidate have been found.
  virtual void OnIceCandidate(const char *id, const webrtc::IceCandidateInterface* candidate) = 0;

  // TODO(bemasc): Remove this once callers transition to OnIceGatheringChange.
  // All Ice candidates have been found.
  virtual void OnIceComplete(const char *id) = 0;

  ///sdp
  virtual void OnSuccess(const char *id, webrtc::SessionDescriptionInterface* desc) = 0;
  virtual void OnFailure(const char * id, const std::string& error) = 0;



};