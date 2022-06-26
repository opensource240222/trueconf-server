
#include "webrtc_api/trueconf_media_engine.h"
#include "webrtc_api/trueconf_audio_device_module.h"
#include "webrtc_api/trueconf_video_encoder_factory.h"
#include "webrtc_api/trueconf_video_decoder_factory.h"
#include "api/peerconnectioninterface.h"
#include "examples/peerconnection/client/defaults.h"
#include "api/test/fakeconstraints.h"
#include "modules/audio_processing/include/audio_processing.h"

#include "VS_WebRTCConfBroadCast.h"
#include "VS_RelayMediaSource.h"
#include "VS_PeerConnectionPoolItem.h"
#include "TransceiverLib/VS_RelayMessageSenderInterface.h"
#include "TransceiverLib/VS_WebrtcRelayMsg.h"
#include "VS_ConfConditionConnector.h"
#include "VS_FrameReceiverConnector.h"
#include "VS_WebRtcMediaPeer.h"
#include "VS_TransceiverParticipant.h"
#include "VS_TransceiverPartsMgr.h"
#include "std/debuglog/VS_Debug.h"
#include "std/statistics/TConferenceStatistics.h"
#include "std/cpplib/base64.h"
#include "std/cpplib/json/writer.h"
#include "std-generic/cpplib/scope_exit.h"


#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

VS_WebRTCConfBroadCast::VS_WebRTCConfBroadCast(const char *conf_name, const std::shared_ptr<VS_RelayMediaSource> &media_source,
	const std::shared_ptr<VS_FrameReceiverConnector> &frame_connector,
	const std::shared_ptr<VS_ConfConditionConnector> &cond_connector,
	const std::shared_ptr<VS_RelayMessageSenderInterface> &sender,
	std::shared_ptr<VS_TransceiverPartsMgr> parts_mgr,
	boost::asio::io_service &ios)
	: m_conf_name(conf_name)
	, m_media_source(media_source)
	, m_frameConnector(frame_connector)
	, m_condConnector(cond_connector)
	, m_message_sender(sender)
	, m_partsMgr(std::move(parts_mgr))
	, m_lastGetStatTime(std::chrono::steady_clock::now())
	, m_timer(ios)
	, m_strand(ios)
{

	media_source->ConnectToDataForWebPeer(boost::bind(&VS_WebRTCConfBroadCast::OnMediaSourceData, this, _1, _2, _3));
}

VS_WebRTCConfBroadCast::~VS_WebRTCConfBroadCast()
{
}

void VS_WebRTCConfBroadCast::Init()
{
	SheduleTimer();
}

void VS_WebRTCConfBroadCast::Timeout()
{
	const auto now = std::chrono::steady_clock::now();
	if (now - m_lastGetStatTime >= std::chrono::seconds(5))
	{
		for (const auto& pi_kv : m_peer_pool) {
			pi_kv.second.peer_connection->GetVideoStats(shared_from_this());
			SetPeerStat(pi_kv.first.c_str(), pi_kv.second.media_peer);
		}
		m_lastGetStatTime = now;
	}
}

bool VS_WebRTCConfBroadCast::ConnectPeerConnection(const char *peer_id, const char *remote_sdp, const char* trueconf_id,
	rtc::Thread* worker, rtc::Thread* signaling, rtc::Thread* network, unsigned long limit_n_peers, bool receive_from_peer, int min_port, int max_port, const char* external_ip)
{
	if (!trueconf_id)		trueconf_id = "";
	if (!external_ip)		external_ip = "";
	if (!peer_id || !remote_sdp) {
		dstream4 << "ConnectPeerConnection Error: empty input params peer_id=" << peer_id;
		return false;
	}

	bool res(false);
	vs::event done(true);
	m_strand.dispatch([&]()
	{
		VS_SCOPE_EXIT{ done.set(); };
		res = ConnectPeerConnectionImpl(peer_id, remote_sdp, trueconf_id, worker, signaling, network, limit_n_peers, receive_from_peer, min_port, max_port, external_ip);
	});
	done.wait();
	return res;
}

bool VS_WebRTCConfBroadCast::ConnectPeerConnectionImpl(const std::string& peer_id, const std::string& remote_sdp, const std::string& trueconf_id,
	rtc::Thread* worker, rtc::Thread* signaling, rtc::Thread* network, unsigned long limit_n_peers, bool receive_from_peer, int min_port, int max_port, const std::string& external_ip)
{
	auto fi = m_peer_pool.find(peer_id);
	if (fi == m_peer_pool.end()) {
		// create new
		if (m_peer_pool.size() >= limit_n_peers) // license max_guests limit reached
		{
			char reason[32] = { 0 };
			snprintf(reason, sizeof(reason), "lic_limit=%ld", limit_n_peers);
			boost::shared_ptr<VS_WebrtcRelayMsg> msg(new VS_WebrtcRelayMsg);
			msg->MakeWebrtcMsg(e_bye, peer_id.c_str(), m_conf_name.c_str(), reason, 0);
			auto sender = m_message_sender.lock();
			sender->SendMsg(msg);
			return false;
		}
		///Connect stream participant
		auto peer_item = std::make_shared<VS_WebRtcMediaPeer>(peer_id.c_str(), trueconf_id.c_str());
		peer_item->ConnectToAudioPayLoadChange(boost::bind(&VS_WebRTCConfBroadCast::OnAudioPLChanged, this, _1, _2, _3));

		if (receive_from_peer)
		{
			std::shared_ptr<VS_TransceiverParticipant> part = m_partsMgr->GetPart(m_conf_name.c_str(), peer_id.c_str());
			std::weak_ptr<VS_TransceiverParticipant> w_part(part);
			peer_item->ConnectToReceiveFrame([w_part](const unsigned char *buf, const unsigned long size, stream::Track track) {
				if (auto part = w_part.lock()) {
					part->SendFrame(buf, size, track);
				}
			});
			part->ConnectToKeyFrameRequest([peer_item]() {peer_item->RequestKeyFrameFromPeer();});
			part->ConnectToSetBitrateRequest([peer_item](int bitrate) {peer_item->SetPeerBitrate(bitrate); });
			part->ConnectStream();
		}

		tc_webrtc_api::TrueConfMediaEngine *meng = peer_item->GetMediaEngine();
		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory = webrtc::CreatePeerConnectionFactory(network,
																												worker,
																												signaling,
																												meng->CreateAudioDeviceModule(),
																												meng->CreateAudioEncoderFactory(),
																												meng->CreateAudioDecoderFactory(),
																												meng->CreateVideoEncoderFactory(),
																												meng->CreateVideoDecoderFactory(),
																												nullptr, nullptr);
		{
			auto media_source = m_media_source.lock();
			if (media_source == nullptr || !media_source->PeerConnect(peer_item)) {
				dstream3 << "WebRTC: PeerConnect error = " << peer_id;
			}
		}

		dstream3 << "WebRTC: RemoteSDP from peer = " << peer_id << "\n" << remote_sdp;

		// todo(kt): i add this for Firefox use DTLS-SRTP
		webrtc::FakeConstraints fc;
		fc.AddOptional("DtlsSrtpKeyAgreement", true);
		if (min_port || max_port)
		{
			fc.AddOptional("X-TrueConf-MinPort", min_port);
			fc.AddOptional("X-TrueConf-MaxPort", max_port);
		}
		if (!external_ip.empty())
		{
			fc.AddOptional("X-TrueConf-ExternalIP-candidate", external_ip);
		}

		rtc::scoped_refptr<VS_PeerConnectionPoolItem> item(new rtc::RefCountedObject<VS_PeerConnectionPoolItem>(peer_id.c_str(), this));
		if (item->InitPeerConnection(trueconf_id.c_str(), peer_id.c_str(), factory, (cricket::VideoCapturer*)meng->CreateVideoCapturer(false), (const webrtc::FakeConstraints*)&fc))
		{
			m_peer_pool.emplace(peer_id, peer_info{ peer_item, item });
			/**
			for broadcast it is kOffer type allways
			*/
			webrtc::SdpParseError error;
			webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(webrtc::SessionDescriptionInterface::kOffer, remote_sdp, &error));
			item->SetRemoteDescription(session_description);
			item->CreateAnswer((const webrtc::MediaConstraintsInterface*)&fc);
			return true;
		}
		else
			return false;
	}
	else {
		// old perrconnection
		webrtc::FakeConstraints fc;

		rtc::scoped_refptr<VS_PeerConnectionPoolItem> item = fi->second.peer_connection;
		webrtc::SdpParseError error;
		webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(webrtc::SessionDescriptionInterface::kOffer, remote_sdp, &error));
		item->SetRemoteDescription(session_description);
		item->CreateAnswer((const webrtc::MediaConstraintsInterface*)&fc);
		return true;
	}
}

bool VS_WebRTCConfBroadCast::DisconnectPeerConnection(const char *peer_id) {
	if (!peer_id) {
		dstream4 << "DisconnectPeerConnection error: empty peer ID\n";
		return false;
	}

	bool res(false);
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		res = DisconnectPeerConnectionImpl(peer_id);
	});
	done.wait();
	return res;
}

bool VS_WebRTCConfBroadCast::DisconnectPeerConnectionImpl(const std::string& peer_id)
{
	{
		auto media_source = m_media_source.lock();
		media_source->PeerDisconnect(peer_id.c_str());
	}
	auto iter = m_peer_pool.find(peer_id);
	if (iter != m_peer_pool.end())
	{
		m_partsMgr->FreePart(m_conf_name.c_str(), iter->first.c_str());
		iter->second.peer_connection->Close();
		m_peer_pool.erase(iter);
	}
	return true;
}

bool VS_WebRTCConfBroadCast::DisconnectPart(const char *part_id) {
	if (!part_id) {
		dstream4 << "DisconnectPeerConnection error: empty participant ID\n";
		return false;
	}

	bool res(false);
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		res = DisconnectPartImpl(part_id);
	});
	done.wait();
	return res;
}

bool VS_WebRTCConfBroadCast::DisconnectPartImpl(const std::string& part_id)
{
	for (auto iter = m_peer_pool.begin(); iter != m_peer_pool.end(); )
	{
		if (iter->second.peer_connection->trueconf_id() == part_id)
		{
			if (!iter->second.peer_connection->bye_sent())
			{
				boost::shared_ptr<VS_WebrtcRelayMsg> msg(new VS_WebrtcRelayMsg);
				msg->MakeWebrtcMsg(e_bye, iter->first.c_str(), m_conf_name.c_str(), 0, 0);
				auto sender = m_message_sender.lock();
				sender->SendMsg(msg);
				iter->second.peer_connection->bye_sent(true);
			}
			m_partsMgr->FreePart(m_conf_name.c_str(), iter->first.c_str());
			iter->second.peer_connection->Close();
			iter = m_peer_pool.erase(iter);
		}
		else
			++iter;
	}
	return true;
}

bool VS_WebRTCConfBroadCast::IsEmpty() const
{
	/**
	TODO: remove this method;
	*/
	bool res(false);
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		res = m_peer_pool.empty();
	});
	done.wait();
	return res;
}

void VS_WebRTCConfBroadCast::Close(bool IsFromTrueConf) {
	m_strand.dispatch([w_self = this->weak_from_this(), IsFromTrueConf]() {
		if (auto self = w_self.lock())
			self->CloseImpl(IsFromTrueConf);
	});
}

void VS_WebRTCConfBroadCast::CloseImpl(bool IsFromTrueConf)
{
	while (!m_peer_pool.empty())
	{
		auto iter = m_peer_pool.begin();
		if (IsFromTrueConf && !iter->second.peer_connection->bye_sent())
		{
			boost::shared_ptr<VS_WebrtcRelayMsg> msg(new VS_WebrtcRelayMsg);
			msg->MakeWebrtcMsg(e_bye, iter->first.c_str(), m_conf_name.c_str(), 0, 0);
			auto sender = m_message_sender.lock();
			sender->SendMsg(msg);
			iter->second.peer_connection->bye_sent(true);
		}
		m_partsMgr->FreePart(m_conf_name.c_str(), iter->first.c_str());
		iter->second.peer_connection->Close();
		m_peer_pool.erase(iter);
	}
}

void VS_WebRTCConfBroadCast::GenKeyFrame(const char * part_id) {
	if (!part_id)
		return;
	m_strand.dispatch([w_self = this->weak_from_this(), part_id = std::string(part_id)]() {
		if (auto self = w_self.lock())
			self->GenKeyFrameImpl(part_id);
	});
}

void VS_WebRTCConfBroadCast::GenKeyFrameImpl(const std::string& part_id)
{
	auto iter = m_peer_pool.find(part_id);
	if (iter == m_peer_pool.end())
		return;
	iter->second.media_peer->RequestKeyFrameFromPeer();
}

void VS_WebRTCConfBroadCast::AddIceCandidate(const char* id, const char* sdp, const char* sdp_mid, int sdp_mlineindex) {
	if (!id || !sdp || !sdp_mid) {
		dstream4 << "AddIceCandidate error: empty input params\n";
		return;
	}
	m_strand.dispatch([w_self = this->weak_from_this(), id = std::string(id), sdp = std::string(sdp), sdp_mid = std::string(sdp_mid), sdp_mlineindex]() {
		if (auto self = w_self.lock())
			self->AddIceCandidateImpl(id, sdp, sdp_mid, sdp_mlineindex);
	});
}

void VS_WebRTCConfBroadCast::AddIceCandidateImpl(const std::string& id, const std::string& sdp, const std::string& sdp_mid, int sdp_mlineindex)
{
	webrtc::SdpParseError error;
	std::unique_ptr<webrtc::IceCandidateInterface> candidate(webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp, &error));

	dstream4 << "VS_WebRTCConfBroadCast::AddIceCandidate id = " << id;

	auto iter = m_peer_pool.find(id);
	if (iter == m_peer_pool.end())
		return;
	iter->second.peer_connection->AddIceCandidate(candidate.get());
}

void VS_WebRTCConfBroadCast::OnError(const char *id)
{
	dprint4("VS_WebRTCConfBroadCast::OnError id = %s\n", id);
	DisconnectPeerConnection(id);
}

void VS_WebRTCConfBroadCast::OnSignalingChange(const char *id, webrtc::PeerConnectionInterface::SignalingState new_state)
{
	dprint4("VS_WebRTCConfBroadCast::OnSignalingChange id = %s state = %d\n", id, new_state);
	/**
	TODO: implement
	*/
}
void VS_WebRTCConfBroadCast::OnStateChange(const char *id/*, webrtc::PeerConnectionObserver::StateType state_changed*/)
{
	dprint4("VS_WebRTCConfBroadCast::OnStateChange id = %s\n"/*, state = %d\n*/, id/*, state_changed*/);

	/**
	TODO: implement
	*/
}

void VS_WebRTCConfBroadCast::OnAddStream(const char *id, webrtc::MediaStreamInterface* stream)
{
	// todo: put in own thread
	//auto iter = m_peer_pool.find(id);
	//if (iter != m_peer_pool.end()) {
	//	auto vt = stream->GetVideoTracks();
	//	if (vt.size() > 0) {
	//		iter->second.media_peer->RemoveTracks();
	//		for (auto& st : vt) {
	//			dprint4("VS_WebRTCConfBroadCast::OnAddStream id = %s, trackid = %s\n", id, st->id().c_str());
	//			iter->second.media_peer->AddTrack(st->id());
	//		}
	//	}
	//}
}

void VS_WebRTCConfBroadCast::OnAddTrack(const char * id, webrtc::RtpReceiverInterface * receiver)
{
	if (!id || !receiver)
		return;

	auto iter = m_peer_pool.find(string_view(id));
	if (iter != m_peer_pool.end()) {
		auto track = receiver->track();
		if (track) {
			dprint4("VS_WebRTCConfBroadCast::OnAddTrack id = %s, trackid = %s\n", id, track->id().c_str());
			if (receiver->media_type() == cricket::MEDIA_TYPE_VIDEO) {
				iter->second.media_peer->RemoveTracks();
				iter->second.media_peer->AddTrack(track->id());
			}
		}
	}
}

void VS_WebRTCConfBroadCast::OnRemoveStream(const char *id, webrtc::MediaStreamInterface* stream)
{
	//dprint4("VS_WebRTCConfBroadCast::OnRemoveStream id = %s, label = %s\n", id, stream->label().c_str());
	dprint4("VS_WebRTCConfBroadCast::OnRemoveStream id = %s\n", id/*, stream->label().c_str()*/);
}

void VS_WebRTCConfBroadCast::OnDataChannel(const char *id, webrtc::DataChannelInterface* data_channel)
{
	dprint4("VS_WebRTCConfBroadCast::OnDataChannel id = %s\n", id);
}

void VS_WebRTCConfBroadCast::OnRenegotiationNeeded(const char *id)
{
	dprint4("VS_WebRTCConfBroadCast::OnRenegotiationNeeded id = %s\n", id);
}


void VS_WebRTCConfBroadCast::OnIceConnectionChange(const char *id, webrtc::PeerConnectionInterface::IceConnectionState new_state)
{
	dprint4("VS_WebRTCConfBroadCast::OnIceConnectionChange id=%s, state = %d\n", id, new_state);
}


void VS_WebRTCConfBroadCast::OnIceGatheringChange(const char *id,
	webrtc::PeerConnectionInterface::IceGatheringState new_state)
{
	dprint4("VS_WebRTCConfBroadCast::OnIceGatheringChange id = %s, state = %d\n", id, new_state);
}

void VS_WebRTCConfBroadCast::OnIceCandidate(const char *id, const webrtc::IceCandidateInterface* candidate)
{
	boost::shared_ptr<VS_WebrtcRelayMsg> msg(new VS_WebrtcRelayMsg);
	std::string sdp;
	candidate->ToString(&sdp);
	msg->MakeCandidate(id, m_conf_name.c_str(), sdp.c_str(), candidate->sdp_mid().c_str(), candidate->sdp_mline_index());

	dprint4("VS_WebRTCConfBroadCast::OnIceCandidate id=%s \n", id);
	auto sender = m_message_sender.lock();
	sender->SendMsg(msg);
}
void VS_WebRTCConfBroadCast::OnAudioPLChanged(const char *id, const char *plname, const int plfreq)
{
	////TODO: send message to server
}

void VS_WebRTCConfBroadCast::OnMediaSourceData(const char *id, const char *conf_name, const char *data)
{
	boost::shared_ptr<VS_WebrtcRelayMsg> msg(new VS_WebrtcRelayMsg);
	msg->MakeAnyData(id, m_conf_name.c_str(), data);

	auto sender = m_message_sender.lock();
	sender->SendMsg(msg);
}

bool VS_WebRTCConfBroadCast::SetPeerStat(const char *peer, const std::shared_ptr<VS_WebRtcMediaPeer> &media_peer)
{
	TConferenceStatistics cst;
	media_peer->GetStat(&cst);
	const unsigned char *in = (const unsigned char *)&cst;
	size_t outsize;
	base64_encode(in, cst.size_of_stat, nullptr, outsize);
	auto bs = std::make_unique<char[]>(outsize);
	if (base64_encode(in, cst.size_of_stat, bs.get(), outsize)) {
		json::Object d;
		d["method"] = json::String("LogPartStat");
		d["data"] = json::String(bs.get());
		std::stringstream ss;
		json::Writer::Write(d, ss);
		if (ss.str().length() > 0)
			OnMediaSourceData(peer, m_conf_name.c_str(), ss.str().c_str());
	}
	return false;
}


void VS_WebRTCConfBroadCast::OnIceComplete(const char *id)
{
	boost::shared_ptr<VS_WebrtcRelayMsg> msg(new VS_WebrtcRelayMsg);
	msg->MakeCandidateComplete(id, m_conf_name.c_str());

	dprint4("VS_WebRTCConfBroadCast::OnIceComplete id=%s \n", id);
	auto sender = m_message_sender.lock();
	sender->SendMsg(msg);
}

void VS_WebRTCConfBroadCast::OnSuccess(const char *id, webrtc::SessionDescriptionInterface* desc)
{
	dprint4("VS_WebRTCConfBroadCast::OnSuccess id=%s\n", id);
	if (!id)
		return;

	std::string sdp;
	desc->ToString(&sdp);
	m_strand.post([w_self = this->weak_from_this(), id = std::string(id), sdp]() {
		if (auto self = w_self.lock())
			self->SetLocalDecs(id, sdp);
	});
}

void VS_WebRTCConfBroadCast::SetLocalDecs(const std::string& id, const std::string & sdp)
{
	rtc::scoped_refptr<VS_PeerConnectionPoolItem> pc_wrap;
	auto iter = m_peer_pool.find(id);
	if (iter == m_peer_pool.end())
		return;
	pc_wrap = iter->second.peer_connection;
	webrtc::SessionDescriptionInterface* decs = webrtc::CreateSessionDescription(webrtc::SessionDescriptionInterface::kAnswer, sdp, nullptr);

	if (decs != nullptr)
		pc_wrap->SetLocalDescription(decs);
	// don't wait callback
	OnSetLocalDesc(id.c_str(), m_conf_name.c_str(), sdp.c_str());
}

void VS_WebRTCConfBroadCast::OnSetLocalDesc(const char * id, const char * conf, const char * sdp)
{
	assert(m_strand.running_in_this_thread());

	boost::shared_ptr<VS_WebrtcRelayMsg> msg(new VS_WebrtcRelayMsg);
	msg->MakeWebrtcMsg(e_answer, id, m_conf_name.c_str(), sdp);

	auto sender = m_message_sender.lock();
	sender->SendMsg(msg);
}

void VS_WebRTCConfBroadCast::SheduleTimer(const std::chrono::milliseconds period)
{
	m_timer.expires_from_now(period);
	m_timer.async_wait([self = this->shared_from_this(), period](const boost::system::error_code& ec) {
		if (ec == boost::asio::error::operation_aborted)
			return;

		self->Timeout();
		self->SheduleTimer(period);
	});
}


void VS_WebRTCConfBroadCast::OnComplete(const char *peer_id, const webrtc::StatsReports &reports)
{
	//dprint3("****Statisitc for peer = %s****\n",peer_id);
	//for(std::vector<webrtc::StatsReport>::const_iterator i = reports.begin();i!=reports.end();i++)
	//{
	//	dprint3("Type Statistic = %s, id = %s\n\n",i->type.c_str(),i->id.c_str());
	//	for(webrtc::StatsReport::Values::const_iterator j = i->values.begin();j!=i->values.end();j++)
	//	{
	//		dprint3("ValueName = %s; Value = %s\n",j->name.c_str(),j->value.c_str());
	//	}
	//}
	//dprint3("\n******End***\n");
}

void VS_WebRTCConfBroadCast::OnFailure(const char * id, const std::string& error)
{
	dprint3("VS_WebRTCConfBroadCast::OnFailure id = %s err = %s\n", id, error.c_str());
}

#undef DEBUG_CURRENT_MODULE