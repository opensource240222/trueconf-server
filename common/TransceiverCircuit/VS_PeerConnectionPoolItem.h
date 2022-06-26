#pragma once
#include "boost/shared_ptr.hpp"

#include "api/peerconnectioninterface.h"
#include "p2p/base/portallocator.h"

class VS_PeerConnectionPoolItemObserver;
class VS_PoolItemStatsObserver;

class VS_GetStatsObserver : public webrtc::StatsObserver
{
public:
	VS_GetStatsObserver(const std::shared_ptr<VS_PoolItemStatsObserver> &observer, const char *peer_id);
protected:
	virtual ~VS_GetStatsObserver()
	{
	}

	void OnComplete(const webrtc::StatsReports& reports) override;
private:
	std::shared_ptr<VS_PoolItemStatsObserver> m_observer;
	std::string m_peer_id;
};


class VS_PeerConnectionPoolItem :		/*public webrtc::PeerConnectionInterface,*/
	public webrtc::PeerConnectionObserver,
	public webrtc::CreateSessionDescriptionObserver
{
private:
	class VS_SetSDPObserver : public webrtc::SetSessionDescriptionObserver
	{
	public:
		virtual ~VS_SetSDPObserver(){}
		VS_SetSDPObserver(VS_PeerConnectionPoolItem *owner, bool local) : m_owner(owner), m_local(local)
		{}
	virtual void OnSuccess() {
		if(m_owner)
			m_owner->OnSetSDPSuccess(m_local);
	}
	virtual void OnFailure(const std::string& error) {
		if(m_owner)
			m_owner->OnSetSDPFailure(m_local, error);
	}
	private:
		VS_PeerConnectionPoolItem *m_owner;
		bool						m_local;
	};

public:
	bool InitPeerConnection(const char *trueconf_id, const char *peer_id, /**TODO: remove*/
							const rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> &factory,
							cricket::VideoCapturer* capturer,
							const webrtc::MediaConstraintsInterface* fc);
	///void SetPeerConnection(const talk_base::scoped_refptr<webrtc::PeerConnectionInterface> &conn);
	void CreateOffer(const webrtc::MediaConstraintsInterface* constraints);
	void CreateAnswer(const webrtc::MediaConstraintsInterface* constraints);
	bool GetVideoStats(const std::shared_ptr<VS_PoolItemStatsObserver> &observer);
	bool GetAudioStats(const std::shared_ptr<VS_PoolItemStatsObserver> &observer);

	const char* get_peer_id() const {return peer_id_.c_str();}


	// Accessor methods to active local streams.
	virtual rtc::scoped_refptr<webrtc::StreamCollectionInterface>
		local_streams();

	// Accessor methods to remote streams.
	virtual rtc::scoped_refptr<webrtc::StreamCollectionInterface>
		remote_streams();

	// Add a new MediaStream to be sent on this PeerConnection.
	// Note that a SessionDescription negotiation is needed before the
	// remote peer can receive the stream.
	virtual bool AddStream(webrtc::MediaStreamInterface* stream,
		const webrtc::MediaConstraintsInterface* constraints);

	// Remove a MediaStream from this PeerConnection.
	// Note that a SessionDescription negotiation is need before the
	// remote peer is notified.
	virtual void RemoveStream(webrtc::MediaStreamInterface* stream);

	// Returns pointer to the created DtmfSender on success.
	// Otherwise returns NULL.
	virtual rtc::scoped_refptr<webrtc::DtmfSenderInterface> CreateDtmfSender(
		webrtc::AudioTrackInterface* track);

	virtual rtc::scoped_refptr<webrtc::DataChannelInterface> CreateDataChannel(
		const std::string& label,
		const webrtc::DataChannelInit* config);

	virtual const webrtc::SessionDescriptionInterface* local_description() const;
	virtual const webrtc::SessionDescriptionInterface* remote_description() const;
	// Sets the local session description.
	// JsepInterface takes the ownership of |desc| even if it fails.
	// The |observer| callback will be called when done.
	virtual void SetLocalDescription(webrtc::SessionDescriptionInterface* desc);
	// Sets the remote session description.
	// JsepInterface takes the ownership of |desc| even if it fails.
	// The |observer| callback will be called when done.
	virtual void SetRemoteDescription(webrtc::SessionDescriptionInterface* desc);
	// Restarts or updates the ICE Agent process of gathering local candidates
	// and pinging remote candidates.
	virtual bool UpdateIce(const webrtc::PeerConnectionInterface::IceServers& configuration,
		const webrtc::MediaConstraintsInterface* constraints);
	// Provides a remote candidate to the ICE Agent.
	// A copy of the |candidate| will be created and added to the remote
	// description. So the caller of this method still has the ownership of the
	// |candidate|.
	// TODO(ronghuawu): Consider to change this so that the AddIceCandidate will
	// take the ownership of the |candidate|.
	virtual bool AddIceCandidate(const webrtc::IceCandidateInterface* candidate);

	// Returns the current SignalingState.
	virtual webrtc::PeerConnectionInterface::SignalingState signaling_state();

	// TODO(bemasc): Remove ice_state when callers are changed to
	// IceConnection/GatheringState.
	// Returns the current IceState.
	virtual webrtc::PeerConnectionInterface::IceConnectionState ice_connection_state();
	virtual webrtc::PeerConnectionInterface::IceGatheringState ice_gathering_state();

	// Terminates all media and closes the transport.
	virtual void Close();

	///SetSessionDescriptionObserver

	const std::string& trueconf_id() const
	{
		return m_trueconf_id;
	}

	void bye_sent(const bool b)
	{
		m_bye_sent = b;
	}
	bool bye_sent() const
	{
		return m_bye_sent;
	}

protected:

	///webrtc::PeerConnectionObserver
	virtual void OnError();

	// Triggered when the SignalingState changed.
	virtual void OnSignalingChange(
		webrtc::PeerConnectionInterface::SignalingState new_state) override;

	// Triggered when SignalingState or IceState have changed.
	// TODO(bemasc): Remove once callers transition to OnSignalingChange.
	virtual void OnStateChange(/*StateType state_changed*/);

	// Triggered when media is received on a new stream from remote peer.
	virtual void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

	// Triggered when a remote peer close a stream.
	virtual void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

	// Triggered when a remote peer open a data channel.
	// TODO(perkj): Make pure virtual.
	virtual void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;

	// Triggered when renegotation is needed, for example the ICE has restarted.
	virtual void OnRenegotiationNeeded() override;

	// Called any time the IceConnectionState changes
	virtual void OnIceConnectionChange(
		webrtc::PeerConnectionInterface::IceConnectionState new_state) override;

	// Called any time the IceGatheringState changes
	virtual void OnIceGatheringChange(
		webrtc::PeerConnectionInterface::IceGatheringState new_state) override;

	// New Ice candidate have been found.
	virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;

	virtual void OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams);

	// The implementation of the CreateSessionDescriptionObserver takes
	// the ownership of the |desc|.
	virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
	virtual void OnFailure(const std::string& error);

	///SetSessionDescr
	void OnSetSDPSuccess(bool local);
	void OnSetSDPFailure(bool local, const std::string &err);

	VS_PeerConnectionPoolItem(const char* id, VS_PeerConnectionPoolItemObserver *observer);
	virtual ~VS_PeerConnectionPoolItem();

private:
	bool AddStreams(const char *trueconf_id, const char *peer_id, cricket::VideoCapturer* capturer);
	bool GetStats(webrtc::StatsObserver* observer,
		webrtc::MediaStreamTrackInterface* track);

	std::string peer_id_;
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
	VS_PeerConnectionPoolItemObserver *observer_;
	rtc::scoped_refptr<VS_SetSDPObserver> set_sdp_observer_local;
	rtc::scoped_refptr<VS_SetSDPObserver> set_sdp_observer_remote;

	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;

	rtc::scoped_refptr<webrtc::VideoTrackInterface> my_video_track_;
	rtc::scoped_refptr<webrtc::AudioTrackInterface> my_audio_track_;

	webrtc::AudioTrackVector remote_a_tracks_;
	webrtc::VideoTrackVector remote_v_tracks_;

	rtc::scoped_refptr<webrtc::VideoTrackInterface> remote_video_track_;
	rtc::scoped_refptr<webrtc::AudioTrackInterface> remote_audio_track_;

	std::string m_trueconf_id;
	bool		m_bye_sent;
};