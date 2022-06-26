#include "rtc_base/refcountedobject.h"
#include "VS_PeerConnectionPoolItem.h"
#include "VS_PeerConnectionPoolItemObserver.h"
#include "../std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

using namespace rtc;
using namespace webrtc;

const char kAudioLabel[] = "audio_label";
const char kVideoLabel[] = "video_label";
const char kStreamLabel[] = "stream_label";


VS_GetStatsObserver::VS_GetStatsObserver(const std::shared_ptr<VS_PoolItemStatsObserver> &observer, const char *peer_id): m_observer(observer),m_peer_id(peer_id)
{}

void VS_GetStatsObserver::OnComplete(const webrtc::StatsReports& reports)
{
	m_observer->OnComplete(m_peer_id.c_str(),reports);
}

VS_PeerConnectionPoolItem::VS_PeerConnectionPoolItem(const char* id,VS_PeerConnectionPoolItemObserver *observer) :
	peer_id_(id), observer_(observer),
	set_sdp_observer_local(new rtc::RefCountedObject<VS_SetSDPObserver>(this, true)),
	set_sdp_observer_remote(new rtc::RefCountedObject<VS_SetSDPObserver>(this, false)),
	m_bye_sent(false)
{
}
VS_PeerConnectionPoolItem::~VS_PeerConnectionPoolItem()
{
}

bool VS_PeerConnectionPoolItem::InitPeerConnection(const char *trueconf_id, const char *peer_id, /**TODO: remove*/
												   const rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> &factory,
												   cricket::VideoCapturer* capturer,
												   const webrtc::MediaConstraintsInterface* fc)
{
	/**
		Create peer connection and init streams
	*/
	if(!factory.get() || peer_connection_.get())
		return false;
	peer_connection_factory_ = factory;


	webrtc::PeerConnectionInterface::RTCConfiguration config;
	webrtc::PeerConnectionInterface::IceServer server;
	server.uri = "stun:stun.l.google.com:19302";
	config.servers.push_back(server);
	config.set_prerenderer_smoothing(false);
	config.set_cpu_adaptation(false);

	peer_connection_ = peer_connection_factory_ ->CreatePeerConnection(config,fc,0,0,this);
	if (!peer_connection_.get()) {
		return false;
	}
	if(!AddStreams(trueconf_id,peer_id, capturer)) {
		peer_connection_ = NULL;
		return false;
	}
	if (!!trueconf_id) {
		m_trueconf_id = trueconf_id;
	}
	return true;
}

bool VS_PeerConnectionPoolItem::AddStreams(const char *trueconf_id, const char *peer_id, cricket::VideoCapturer* capturer)
{
	dprint4("VS_PeerConnectionPoolItem::AddStreams id = %s, tc_id = %s\n",peer_id_.c_str(), trueconf_id);
	my_audio_track_ = peer_connection_factory_->CreateAudioTrack(
		kAudioLabel, peer_connection_factory_->CreateAudioSource(NULL));

	my_video_track_ = peer_connection_factory_->CreateVideoTrack(
		kVideoLabel,
		peer_connection_factory_->CreateVideoSource(capturer,
		NULL));

	rtc::scoped_refptr<webrtc::MediaStreamInterface> stream = peer_connection_factory_->CreateLocalMediaStream(kStreamLabel);

	stream->AddTrack(my_audio_track_);
	stream->AddTrack(my_video_track_);
	return AddStream(stream,NULL);
}

void VS_PeerConnectionPoolItem::CreateOffer(const webrtc::MediaConstraintsInterface *constraints)
{
	dprint4("VS_PeerConnectionPoolItem::CreateOffer id=%s\n", peer_id_.c_str());
	assert(!!peer_connection_.get());
	peer_connection_->CreateOffer(this,constraints);
}
void VS_PeerConnectionPoolItem::CreateAnswer(const webrtc::MediaConstraintsInterface* constraints)
{
	dprint4("VS_PeerConnectionPoolItem::CreateAnswer id = %s\n", peer_id_.c_str());
	assert(!!peer_connection_.get());

	webrtc::PeerConnectionInterface::RTCOfferAnswerOptions opt;
	peer_connection_->CreateAnswer(this, opt);
}

bool VS_PeerConnectionPoolItem::GetVideoStats(const std::shared_ptr<VS_PoolItemStatsObserver> &observer)
{
	rtc::scoped_refptr<VS_GetStatsObserver> obs(new rtc::RefCountedObject<VS_GetStatsObserver>(observer, peer_id_.c_str()));
	return GetStats(obs,/*remote_v_tracks_[0]*/my_video_track_);
}
bool VS_PeerConnectionPoolItem::GetAudioStats(const std::shared_ptr<VS_PoolItemStatsObserver> &observer)
{
	rtc::scoped_refptr<VS_GetStatsObserver> obs(new rtc::RefCountedObject<VS_GetStatsObserver>(observer, peer_id_.c_str()));
	return GetStats(obs,/*remote_v_tracks_[0]*/my_audio_track_);
}
rtc::scoped_refptr<webrtc::StreamCollectionInterface> VS_PeerConnectionPoolItem::local_streams()
{
	assert(!!peer_connection_.get());
	return peer_connection_->local_streams();
}
rtc::scoped_refptr<webrtc::StreamCollectionInterface> VS_PeerConnectionPoolItem::remote_streams()
{
	assert(!!peer_connection_.get());
	return peer_connection_->remote_streams();
}


bool VS_PeerConnectionPoolItem::AddStream(MediaStreamInterface *stream, const  MediaConstraintsInterface* constraints)
{
	assert(!!peer_connection_.get());
	if (!peer_connection_.get())
		return false;
	dprint4("VS_PeerConnectionPoolItem::AddStream id = %s\n", peer_id_.c_str());
	return peer_connection_->AddStream(stream);
}

void VS_PeerConnectionPoolItem::RemoveStream(webrtc::MediaStreamInterface* stream)
{
	dprint4("VS_PeerConnectionPoolItem::RemoveStream id = %s\n", peer_id_.c_str());
	assert(!!peer_connection_.get());
	peer_connection_->RemoveStream(stream);
}

rtc::scoped_refptr<webrtc::DtmfSenderInterface> VS_PeerConnectionPoolItem::CreateDtmfSender(webrtc::AudioTrackInterface* track)
{
	dprint4("VS_PeerConnectionPoolItem::CreateDtmfSender id = %s \n", peer_id_.c_str());
	assert(!!peer_connection_.get());
	return peer_connection_->CreateDtmfSender(track);
}

bool VS_PeerConnectionPoolItem::GetStats(webrtc::StatsObserver* observer,webrtc::MediaStreamTrackInterface* track)
{
	assert(!!peer_connection_.get());
	//dprint4("VS_PeerConnectionPoolItem::GetStats id=%s\n",peer_id_.c_str());
	return peer_connection_->GetStats(observer, track, webrtc::PeerConnectionInterface::kStatsOutputLevelStandard);
}

rtc::scoped_refptr<webrtc::DataChannelInterface> VS_PeerConnectionPoolItem::CreateDataChannel(const std::string& label,
	const webrtc::DataChannelInit* config)
{
	assert(!!peer_connection_.get());
	dprint4("VS_PeerConnectionPoolItem::CreateDataChannel id = %s\n", peer_id_.c_str());
	return peer_connection_->CreateDataChannel(label, config);
}

const webrtc::SessionDescriptionInterface* VS_PeerConnectionPoolItem::local_description() const
{
	assert(!!peer_connection_.get());
	return peer_connection_->local_description();
}
const webrtc::SessionDescriptionInterface* VS_PeerConnectionPoolItem::remote_description() const
{
	assert(!!peer_connection_.get());
	return peer_connection_->remote_description();
}

void VS_PeerConnectionPoolItem::SetRemoteDescription(webrtc::SessionDescriptionInterface* desc)
{
	assert(!!peer_connection_.get());
	if (!peer_connection_.get())
		return;
	dprint4("VS_PeerConnectionPoolItem::SetRemoteDescription id=%s\n", peer_id_.c_str());
	peer_connection_->SetRemoteDescription(set_sdp_observer_remote, desc);
}

void VS_PeerConnectionPoolItem::SetLocalDescription(SessionDescriptionInterface* desc)
{
	assert(!!peer_connection_.get());
	if (!peer_connection_.get())
		return;
	dprint4("VS_PeerConnectionPoolItem::SetLocalDescription id = %s\n", peer_id_.c_str());
	peer_connection_->SetLocalDescription(set_sdp_observer_local, desc);
}

bool VS_PeerConnectionPoolItem::UpdateIce(const webrtc::PeerConnectionInterface::IceServers &configuration, const webrtc::MediaConstraintsInterface *constraints)
{
	assert(!!peer_connection_.get());
	dprint4("VS_PeerConnectionPoolItem::UpdateIce id = %s\n", peer_id_.c_str());
	//return peer_connection_->UpdateIce(configuration,constraints);
	return true;
}

bool VS_PeerConnectionPoolItem::AddIceCandidate(const IceCandidateInterface *candidate)
{
	assert(!!peer_connection_.get());
	if(!peer_connection_.get())
		return false;
	dprint4("VS_PeerConnectionPoolItem::AddIceCandidate id = %s\n", peer_id_.c_str());
	return peer_connection_->AddIceCandidate(candidate);
}

webrtc::PeerConnectionInterface::SignalingState VS_PeerConnectionPoolItem::signaling_state()
{
	assert(!!peer_connection_.get());
	return peer_connection_->signaling_state();
}

webrtc::PeerConnectionInterface::IceConnectionState VS_PeerConnectionPoolItem::ice_connection_state()
{
	assert(!!peer_connection_.get());
	return peer_connection_->ice_connection_state();
}
webrtc::PeerConnectionInterface::IceGatheringState VS_PeerConnectionPoolItem::ice_gathering_state()
{
	assert(!!peer_connection_.get());
	return peer_connection_->ice_gathering_state();
}
void VS_PeerConnectionPoolItem::Close()
{
	assert(!!peer_connection_.get());
	dprint4("VS_PeerConnectionPoolItem::Close id = %s\n", peer_id_.c_str());
	peer_connection_->Close();
}


////observer
void VS_PeerConnectionPoolItem::OnError()
{
	if(observer_)
		observer_->OnError(peer_id_.c_str());
}
void VS_PeerConnectionPoolItem::OnSignalingChange(PeerConnectionInterface::SignalingState new_state)
{
	if(observer_)
		observer_->OnSignalingChange(peer_id_.c_str(),new_state);
}

void VS_PeerConnectionPoolItem::OnStateChange(/*StateType state_changed*/)
{
	if(observer_)
		observer_->OnStateChange(peer_id_.c_str()/*,state_changed*/);
}

void VS_PeerConnectionPoolItem::OnAddStream(rtc::scoped_refptr<MediaStreamInterface> stream)
{

	remote_a_tracks_ = stream->GetAudioTracks();
	remote_v_tracks_ = stream->GetVideoTracks();
	/*remote_video_track_ = stream->GetVideoTracks();
	remote_audio_track_ = stream->GetAudioTracks();*/
	if(observer_)
		observer_->OnAddStream(peer_id_.c_str(),stream);
}

void VS_PeerConnectionPoolItem::OnRemoveStream(rtc::scoped_refptr<MediaStreamInterface> stream)
{
	remote_a_tracks_.clear();
	remote_v_tracks_.clear();
	if(observer_)
		observer_->OnRemoveStream(peer_id_.c_str(), stream);
}

void VS_PeerConnectionPoolItem::OnDataChannel(rtc::scoped_refptr<DataChannelInterface> data_channel)
{
	if(observer_)
		observer_->OnDataChannel(peer_id_.c_str(),data_channel);
}

void VS_PeerConnectionPoolItem::OnRenegotiationNeeded()
{
	if(observer_)
		observer_->OnRenegotiationNeeded(peer_id_.c_str());
}

void VS_PeerConnectionPoolItem::OnIceConnectionChange(
      PeerConnectionInterface::IceConnectionState new_state)
{
	if(observer_)
		observer_->OnIceConnectionChange(peer_id_.c_str(),new_state);
}

void VS_PeerConnectionPoolItem::OnIceGatheringChange(
      PeerConnectionInterface::IceGatheringState new_state)
{
	if(observer_)
		observer_->OnIceGatheringChange(peer_id_.c_str(),new_state);
}

void VS_PeerConnectionPoolItem::OnIceCandidate(const IceCandidateInterface* candidate)
{
	if(observer_)
		observer_->OnIceCandidate(peer_id_.c_str(),candidate);
}

void VS_PeerConnectionPoolItem::OnAddTrack(rtc::scoped_refptr<RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<MediaStreamInterface>>& streams)
{
	if (observer_)
		observer_->OnAddTrack(peer_id_.c_str(), receiver);

	for (auto &st : streams) {
		for (auto &tr : st->GetVideoTracks()) {
			//dprint4("VS_PeerConnectionPoolItem::OnAddTrack %s vlabel = %s\n", tr->id().c_str(), st->label().c_str());
			dprint4("VS_PeerConnectionPoolItem::OnAddTrack %s\n", tr->id().c_str());
		}
		for (auto &tr : st->GetAudioTracks()) {
			//dprint4("VS_PeerConnectionPoolItem::OnAddTrack %s alabel = %s\n", tr->id().c_str(), st->label().c_str());
			dprint4("VS_PeerConnectionPoolItem::OnAddTrack %s\n", tr->id().c_str());
		}
	}
}

//sdp
void VS_PeerConnectionPoolItem::OnSuccess(SessionDescriptionInterface* desc)
{
	if(observer_)
		observer_->OnSuccess(peer_id_.c_str(),desc);
}

void VS_PeerConnectionPoolItem::OnFailure(const std::string& error)
{
	if(observer_)
		observer_->OnFailure(peer_id_.c_str(),error);
}

void VS_PeerConnectionPoolItem::OnSetSDPSuccess(bool local)
{
	dprint4("VS_PeerConnectionPoolItem::OnSetSDPSuccess %s, dir = %d\n", peer_id_.c_str(), local);
}

void VS_PeerConnectionPoolItem::OnSetSDPFailure(bool local, const std::string & err)
{
	dprint3("VS_PeerConnectionPoolItem::OnSetSDPFailure %s, dir = %d, err |%s|\n", peer_id_.c_str(), local, err.c_str());
}
