
#include "rtc_base/thread.h"
#include "p2p/base/portallocator.h"

#include "VS_WebRTCBroadcaster.h"
#include "VS_WebRTCConfBroadCast.h"
#include "VS_MediaSourceCollection.h"
#include "VS_ConfConditionConnector.h"
#include "VS_RelayMediaSource.h"
#include "TransceiverLib/VS_WebrtcRelayMsg.h"
#include "std/debuglog/VS_Debug.h"

#include <boost/signals2.hpp>
#include <boost/make_shared.hpp>

#include <thread>
#include "std/cpplib/MakeShared.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

VS_WebRTCBroadcaster::VS_WebRTCBroadcaster(const std::shared_ptr<VS_FrameReceiverConnector> &frame_connector,
										   const std::shared_ptr<VS_ConfConditionConnector> &cond_connector,
										   boost::asio::io_service& ios, const boost::shared_ptr<VS_MediaSourceCollection> & collection,
										   const std::shared_ptr<VS_TransceiverPartsMgr>& partsMgr)
	: VS_RelayModule(VS_WebrtcRelayMsg::module_name)
	, m_frameConnector(frame_connector)
	, m_condConnector(cond_connector)
	, m_partsMgr(partsMgr)
	, m_media_source_collection(collection)
	, m_webrtc_signaling_thread(new rtc::Thread)
	, m_webrtc_worker_thread(new rtc::Thread)
	, m_counterPeers(0)
	, m_strand(ios)
{
	m_webrtc_signaling_thread->SetName("Signaling", nullptr);
	m_webrtc_worker_thread->SetName("Worker", nullptr);
	m_webrtc_signaling_thread->Start();
	m_webrtc_worker_thread->Start();
	uint32_t nth = std::min<uint32_t>(8, std::thread::hardware_concurrency());
	for (uint32_t i = 0; i < nth; i++) {
		auto thread = std::make_unique<rtc::Thread>();
		thread->SetName("Network", nullptr);
		thread->Start();
		m_webrtc_network_thread_pool.push_back(std::move(thread));
	}
}

VS_WebRTCBroadcaster::~VS_WebRTCBroadcaster()
{
	m_conf_broadcaster.clear();
	delete m_webrtc_signaling_thread;
	delete m_webrtc_worker_thread;
}
void VS_WebRTCBroadcaster::Close()
{
	m_strand.dispatch([this, w_self = this->weak_from_this()]() {
		auto self = w_self.lock();
		if (!self)
			return;

		while (!m_conf_broadcaster.empty())
		{
			std::string conf_name = m_conf_broadcaster.begin()->first;
			StopConference(conf_name.c_str());
		}
	});
}

void VS_WebRTCBroadcaster::StartConference(const char *conf_name)
{
	auto media_source = m_media_source_collection->GetMediaSource(conf_name);
}
void VS_WebRTCBroadcaster::StartConference(const char *conf_name, const char *, VS_Conference_Type, VS_GroupConf_SubType)
{
	StartConference(conf_name);
}
bool VS_WebRTCBroadcaster::ConnectPeerConnection(const char* peer_id, const char *remote_sdp, const char *conf_name, const char* trueconf_id, unsigned long limit_n_peers, bool receive_from_peer, int min_port, int max_port, const char* external_ip)
{
	auto iter = m_conf_broadcaster.find(string_view(conf_name));
	auto networkThread = m_webrtc_network_thread_pool[m_counterPeers % m_webrtc_network_thread_pool.size()].get();
	m_counterPeers++;
	if(iter==m_conf_broadcaster.end())
	{
		auto media_source = m_media_source_collection->GetMediaSource(conf_name);
		if(!media_source)
			return false;

		auto frame_connector = m_frameConnector.lock();
		auto cond_connector = m_condConnector.lock();
		if(!frame_connector || !cond_connector)
			return false;

		auto defaultMsgSenderIt = m_message_senders.find({});
		assert(defaultMsgSenderIt != m_message_senders.end());
		if (defaultMsgSenderIt == m_message_senders.end()) return false;

		auto conf_bc = vs::MakeShared<VS_WebRTCConfBroadCast>(conf_name,media_source,frame_connector,
																			cond_connector, defaultMsgSenderIt->second.lock(),
																			m_partsMgr, m_strand.get_io_service());
		if(conf_bc->ConnectPeerConnection(peer_id,remote_sdp,trueconf_id,m_webrtc_worker_thread, m_webrtc_signaling_thread, networkThread, limit_n_peers, receive_from_peer, min_port, max_port, external_ip))
		{
			m_conf_broadcaster[conf_name] = conf_bc;
			return true;
		}
		else
		{
			conf_bc->Close(true);
			{
				auto conf_cond_interface = shared_from_this();
				cond_connector->DisconnectFromConfControl(conf_name, conf_cond_interface);
			}
			cond_connector->DisconnectFromConfControl(conf_name, media_source);
			frame_connector->DisconnectFromTransmitFrame(conf_name, media_source);
			return false;
		}
	}
	else
		return iter->second->ConnectPeerConnection(peer_id,remote_sdp,trueconf_id,m_webrtc_worker_thread, m_webrtc_signaling_thread, networkThread, limit_n_peers, receive_from_peer, min_port, max_port, external_ip);
}
void VS_WebRTCBroadcaster::DisconnectPeerConnection(const char *peer_id, const char *conf_name)
{
	auto iter = m_conf_broadcaster.find(string_view(conf_name));
	if(iter != m_conf_broadcaster.end())
	{
		iter->second->DisconnectPeerConnection(peer_id);
	}
}
void VS_WebRTCBroadcaster::DisconnectPeerConnectionFromAllConf(const char *peer_id)
{
	for(auto iter=m_conf_broadcaster.begin(); iter!=m_conf_broadcaster.end(); ++iter)
	{
		iter->second->DisconnectPeerConnection(peer_id);
	}
}

void VS_WebRTCBroadcaster::ManageAnyData(const char*conf_name, const char *peer_id, const char *command)
{
	assert(m_strand.running_in_this_thread());
	if (!m_strand.running_in_this_thread())
		return;

	if (!conf_name || !command)
		return;

	json::Object data;
	try
	{
		std::string s(command);
		std::stringstream ss(s);
		json::Reader::Read(data, ss);

		std::string method;
		json::Object::const_iterator it = data.Find("method");
		if (it != data.End()) method = (const json::String&) it->element;

		if (method.empty())
			throw false;

		if (method == "ManageLayout") {
			auto ms = m_media_source_collection->GetMediaSource(conf_name, false);
			if (!ms)
				throw true;

			ms->ManageLayout(data);
		}
		else if (method == "SendCommand") {
			if (peer_id && *peer_id) {
				auto it = m_conf_broadcaster.find(string_view(conf_name));
				if (it != m_conf_broadcaster.end()) {
					json::Object::const_iterator jit = data.Find("RequestKeyFrame");
					if (jit != data.End())
						it->second->GenKeyFrame(peer_id);
				}
			}
		}
		else
			throw false;
	}
	catch (json::Exception &) {
		dprint4("Sintax error in any data");
	}
	catch (bool e) {
		if (!e)
			dprint4("Unknown any data");
		else
			dprint4("any data not applied");
	}
}

void VS_WebRTCBroadcaster::StopConference(const char *conf_name) {
	if (!conf_name)
		return;
	m_strand.dispatch([w_self = this->weak_from_this(), conf_name = std::string(conf_name)]() {
		if (auto self = w_self.lock())
			self->StopConferenceImpl(conf_name);
	});
}

void VS_WebRTCBroadcaster::StopConferenceImpl(const std::string& conf_name)
{
	auto iter = m_conf_broadcaster.find(conf_name);
	if(iter != m_conf_broadcaster.end())
	{
		iter->second->Close(true);
		auto cond_connector = m_condConnector.lock();
		m_conf_broadcaster.erase(iter);
	}
	m_media_source_collection->RemoveMediaSource(conf_name.c_str());
}

void VS_WebRTCBroadcaster::ParticipantDisconnect(const char *conf_name, const char *part_name) {
	if (!conf_name || !part_name)
		return;
	m_strand.dispatch([w_self = this->weak_from_this(), conf_name = std::string(conf_name), part_name = std::string(part_name)]() {
		if (auto self = w_self.lock())
			self->ParticipantDisconnectImpl(conf_name, part_name);
	});
}

void VS_WebRTCBroadcaster::ParticipantDisconnectImpl(const std::string& conf_name, const std::string& part_name)
{
	// todo(kt): find peer_id to disconnect
	auto iter = m_conf_broadcaster.find(conf_name);
	if(iter==m_conf_broadcaster.end())
		return;
	iter->second->DisconnectPart(part_name.c_str());
}

bool VS_WebRTCBroadcaster::ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess)
{
	auto wrtc_mess = boost::make_shared<VS_WebrtcRelayMsg>();
	if (!wrtc_mess->SetMessage(mess->GetMess()))
		return false;

	std::string peer_id;
	if (wrtc_mess->GetPeerID())
		peer_id = wrtc_mess->GetPeerID();
	if (peer_id.empty())
		return false;

	m_strand.post([this, w_self = this->weak_from_this(), wrtc_mess, peer_id]() {
		auto self = w_self.lock();
		if (!self)
			return;

		m_msg_from_server.emplace(peer_id, wrtc_mess);
		self->ProcessMessageFromServer();
	});

	return true;
}

void VS_WebRTCBroadcaster::ProcessMessageFromServer()
{
	assert(m_strand.running_in_this_thread());

	boost::shared_ptr<VS_WebrtcRelayMsg> wrtc_mess;
	// On each call try to get one msg from next peer_id (shuffle messages in queue)
	if (m_msg_from_server.empty())
		return;
	auto get_mess = [&](const std::multimap<std::string, boost::shared_ptr<VS_WebrtcRelayMsg>>::iterator& where) -> void {
		m_msg_from_server_last_peer_id = where->first;
		wrtc_mess = where->second;
		m_msg_from_server.erase(where);
	};
	if (m_msg_from_server_last_peer_id.empty())
	{
		get_mess(m_msg_from_server.begin());
	}
	else {
		auto it = m_msg_from_server.upper_bound(m_msg_from_server_last_peer_id);
		if (it == m_msg_from_server.end())
			get_mess(m_msg_from_server.begin());
		else {
			if (++it == m_msg_from_server.end())
				--it;
			get_mess(it);
		}
	}

	/**
	1. ConnectToConference
	a. PeerID
	b. ConferenceName
	c. SDP
	**/
	switch (wrtc_mess->GetWebrtcMsgType())
	{
	case e_offer:
		dprint3("ConnectPeerConnection: trueconf_id=%s\n", wrtc_mess->GetTrueConfID());
		ConnectPeerConnection(wrtc_mess->GetPeerID(), wrtc_mess->GetSDP(), wrtc_mess->GetConferenceName(),
			wrtc_mess->GetTrueConfID(), atol(wrtc_mess->GetLimitNPeers()), wrtc_mess->GetReceiveFromPeer(), wrtc_mess->GetMinPort(), wrtc_mess->GetMaxPort(), wrtc_mess->GetStrValRef("external_ip"));
		break;
	case e_answer:
		dprint3("answer from %s\n", wrtc_mess->GetPeerID());
		break;
	case e_candidate:
		dprint3("candidate from %s\n", wrtc_mess->GetPeerID());
		AddIceCandidate(wrtc_mess);
		break;
	case e_bye:
		if (wrtc_mess->GetConferenceName() && *wrtc_mess->GetConferenceName())
			DisconnectPeerConnection(wrtc_mess->GetPeerID(), wrtc_mess->GetConferenceName());
		else
			DisconnectPeerConnectionFromAllConf(wrtc_mess->GetPeerID());
		break;
	case e_anydata:
		ManageAnyData(wrtc_mess->GetConferenceName(), wrtc_mess->GetPeerID(), wrtc_mess->GetSDP());
		break;
	}
}

void VS_WebRTCBroadcaster::AddIceCandidate(boost::shared_ptr<VS_WebrtcRelayMsg>& wrtc_mess)
{
	auto iter = m_conf_broadcaster.find(string_view(wrtc_mess->GetConferenceName()));
	if(iter==m_conf_broadcaster.end())
		return;

	iter->second->AddIceCandidate(wrtc_mess->GetPeerID(), wrtc_mess->GetSDP(), wrtc_mess->GetCandidate_Mid(), wrtc_mess->GetCandidate_MLineIndex());
}