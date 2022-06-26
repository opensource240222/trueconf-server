#include "VS_WebRtcMediaPeer.h"
#include "streams/Protocol.h"

#include "webrtc_api/trueconf_media_engine.h"
#include "rtc_base/criticalsection.h"

#include "streams/VS_SendFrameQueueBase.h"
#include "std/debuglog/VS_Debug.h"
#include "std/statistics/TConferenceStatistics.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

VS_WebRtcMediaPeer::VS_WebRtcMediaPeer(const char *peer_id, const char *part_id, rtc::Thread *thread)
	: VS_MediaPeerBase(peer_id, part_id)
	, m_queueLock(new rtc::CriticalSection())
	, m_snd_queue(VS_SendFrameQueueBase::Factory(false, false))
{
	m_type = vs_media_peer_type_webrtc;
	m_videoBuffer = new unsigned char[65536 * 10 * 2];
	m_audioBuffer = new unsigned char[65536 * 2];
	m_rcv_samplerate = 16000;
	m_audio_timestamp = 0;
	m_last_rtp_timestamp = 0;
}

VS_WebRtcMediaPeer::~VS_WebRtcMediaPeer()
{
	delete[] m_videoBuffer;
	delete[] m_audioBuffer;
	delete m_queueLock;
}

tc_webrtc_api::TrueConfMediaEngine* VS_WebRtcMediaPeer::GetMediaEngine()
{
	if (m_isClosed)
		return 0;
	if (!m_media_engine)
	{
		m_media_engine.reset(new tc_webrtc_api::TrueConfMediaEngine(m_peer_id.c_str()));
		m_media_engine->fireSendRequestKeyFrame.connect(this, &VS_WebRtcMediaPeer::RequestKeyFrame);
		m_media_engine->fireSetRates.connect(this, &VS_WebRtcMediaPeer::SetRates);
		m_media_engine->fireUpdateSendVideoPayload.connect(this, &VS_WebRtcMediaPeer::SetSndFrameResolution);
		m_media_engine->fireUpdateReceiveVideoPayload.connect(this, &VS_WebRtcMediaPeer::ChangeRcvFrameResolution);
		m_media_engine->fireUpdateSendAudioPayload.connect(this, &VS_WebRtcMediaPeer::ChangeAudioSendPayload);
		m_media_engine->fireUpdateReceiveAudioPayload.connect(this, &VS_WebRtcMediaPeer::ChangeAudioRcvPayload);
		m_media_engine->fireReceiveVideoFrame.connect(this, &VS_WebRtcMediaPeer::ReceiveVideo);
		m_media_engine->fireReceiveAudioFrame.connect(this, &VS_WebRtcMediaPeer::ReceiveAudio);
		m_media_engine->firePeerConnectionDead.connect(this, &VS_WebRtcMediaPeer::OnPeerDead);
	}
	return m_media_engine.get();
}

void VS_WebRtcMediaPeer::AddTrack(const std::string &trackid)
{
	auto it = m_streams.find(trackid);
	if (it == m_streams.end()) {
		StatePeerStream st;
		st.snd_queue = boost::shared_ptr<VS_SendFrameQueueBase>(VS_SendFrameQueueBase::Factory(false, true));
		m_streams.emplace(trackid, std::move(st));
		m_media_engine->KeyFrameRequest();
	}
}

void VS_WebRtcMediaPeer::RemoveTrack(const std::string & trackid)
{
	m_streams.erase(trackid);
}

void VS_WebRtcMediaPeer::RemoveTracks()
{
	m_streams.clear();
}

void VS_WebRtcMediaPeer::GetStat(TConferenceStatistics * cst)
{
	if (!cst)
		return;
	uint64_t rbr, sbr;
	uint32_t st, rt;
	uint32_t w, h;
	double rfps, sfps;
	m_sstat.GetStat(sbr, sfps, st);
	m_rstat.GetStat(rbr, rfps, rt);
	m_sstat.GetResolution(w, h);

	cst->size_of_stat = sizeof(TConferenceStatistics);

	cst->participant_time = rt;
	cst->broadcast_time = st;
	cst->video_w = w;
	cst->video_h = h;
	cst->loss_rcv_packets = 0;
	cst->avg_send_bitrate = (int)((sbr >> 7) / st);
	cst->avg_rcv_bitrate = (int)((rbr >> 7) / rt);
	cst->avg_cpu_load = 0;
	cst->avg_jitter = 0;
	cst->avg_send_fps = sfps;
	cst->start_part_gmt = m_rstat.m_stime;
	cst->end_part_gmt = std::chrono::system_clock::now();
}

void VS_WebRtcMediaPeer::PutVideo(unsigned char *pFrame, unsigned long size, bool isKey, unsigned timestamp)
{
	if (m_isClosed)
		return;
	if (!m_media_engine)
		return;
	m_rstat.CountData(size, 0);
	m_media_engine->DeliverVideoFrame(pFrame, size, 640, 480, isKey, timestamp);
}

void VS_WebRtcMediaPeer::PutCompressedAudio(unsigned char *buf, unsigned long sz, unsigned int samples, unsigned int timestamp)
{
	if (m_isClosed)
		return;
	if (!m_media_engine)
		return;
	m_rstat.CountData(sz, 1);
	m_media_engine->DeliverAudioFrame(buf + sizeof(int32_t), sz, samples);
}

void VS_WebRtcMediaPeer::PutUncompressedAudio(unsigned char *buf, unsigned long sz, unsigned int timestamp)
{

}

void VS_WebRtcMediaPeer::RequestKeyFrame()
{
	if (m_isClosed)
		return;
	if (m_observer)
		m_observer->KeyFrameRequestForPeer(m_part_id.c_str(), m_peer_id.c_str());
}
void VS_WebRtcMediaPeer::SetPeerBitrate(int bitrate)
{
	if (m_isClosed || !m_media_engine)
		return;
	m_media_engine->BitrateRequest(bitrate);
}

void VS_WebRtcMediaPeer::RequestKeyFrameFromPeer()
{
	if (m_isClosed || !m_media_engine)
		return;
	m_media_engine->KeyFrameRequest();
}

void VS_WebRtcMediaPeer::SetRates(const char *peer_name, unsigned int bitrate, unsigned int frame_rate)
{
	if (m_isClosed)
		return;
	if (m_observer)
		m_observer->SetRates(m_part_id.c_str(), peer_name, bitrate, frame_rate);
}

void VS_WebRtcMediaPeer::SetSndFrameResolution(const char *peer_name, const char *plname, uint8_t pltype, unsigned short width, unsigned short height)
{
	if (m_isClosed)
		return;
	if (m_observer)
		m_observer->SetSndFrameResolution(m_part_id.c_str(), peer_name, plname, pltype, width, height);
}

void VS_WebRtcMediaPeer::ChangeAudioSendPayload(const char *plname, int plfreq)
{
	if (m_isClosed)
		return;
	if (m_observer)
		m_observer->ChangeAudioSendPayload(m_part_id.c_str(), m_peer_id.c_str(), plname, plfreq);
}

void VS_WebRtcMediaPeer::ChangeVideoSendPayload(const VS_MediaFormat& mf)
{
	if (m_isClosed)
		return;
	if (m_observer)
		m_observer->ChangeVideoSendPayload(m_part_id.c_str(), m_peer_id.c_str(), m_type, mf);
}

void VS_WebRtcMediaPeer::ChangeAudioRcvPayload(const char *plname, const int plfreq)
{
	if (m_isClosed)
		return;
	dprint4("ChangeAudioRcvPayload peer = %s, plname = %s, plfreq = %d\n", m_peer_id.c_str(), plname, plfreq);
	m_rcv_samplerate = plfreq;
	m_fireAudioChangeAudioRcvPayload(m_peer_id.c_str(), plname, plfreq);
}

void VS_WebRtcMediaPeer::ChangeRcvFrameResolution(const char *peer_name, const char *stream_id, const char *plname, uint8_t pltype, unsigned short width, unsigned short height)
{
	if (m_isClosed)
		return;
	auto it = m_streams.find(stream_id);
	if (it == m_streams.end()) {
		return;
	}
	int mb = width * height / 256;
	it->second.sl = 0;
	for (auto stream = m_streams.begin(); stream != m_streams.end(); ++stream) {
		if (stream->second.mb != 0) {
			if (stream->second.mb >= mb) {
				it->second.sl++;
			}
			else {
				stream->second.sl++;
			}
		}
	}
	it->second.w = width;
	it->second.h = height;
	it->second.mb = mb;
	dprint4("ChangeRcvFrameResolution peer = %s [%s], w = %d, h = %d, sl = %d\n", m_peer_id.c_str(), stream_id, width, height, it->second.sl);
	m_sstat.SetResolution(width, height);
}

void VS_WebRtcMediaPeer::ProcessFrameQueue(VS_SendFrameQueueBase *queue)
{
	unsigned char *buf(0);
	int sz(0);
	unsigned char track(0), slayer(0);
	while (queue->GetFrame(buf, sz, track, slayer) > 0) {
		{
			rtc::CritScope lock(m_queueLock);
			m_fireReceiveFrame(buf, sz, static_cast<stream::Track>(track));
		}
		queue->MarkFirstAsSend();
	}
}

void VS_WebRtcMediaPeer::ReceiveVideo(const char *peer_name, const char *stream_id, const unsigned char *pFrame, int size, bool isKey, unsigned int timestamp)
{
	if (m_isClosed)
		return;
	auto it = m_streams.find(stream_id);
	if (it == m_streams.end()) {
		return;
	}
	m_sstat.CountData(size, 0);
	//TODO: fix extra copying for fast
	m_videoBuffer[0] = it->second.frame_seq;
	m_videoBuffer[1] = !isKey;
	*(unsigned int*)(m_videoBuffer + 2) = timestamp / 90;
	*(int*)(m_videoBuffer + 6) = size + sizeof(stream::SVCHeader);
	stream::SVCHeader* h = reinterpret_cast<stream::SVCHeader*>(m_videoBuffer + 6 + sizeof(int));
	h->quality = 0;
	h->temporal = 0;
	h->spatial = it->second.sl;
	h->maxspatial = m_streams.size() - 1;
	memcpy(m_videoBuffer + 6 + sizeof(int) + sizeof(stream::SVCHeader), pFrame, size);
	it->second.snd_queue->AddFrame(2, size + 6 + sizeof(int) + sizeof(stream::SVCHeader), m_videoBuffer, FRAME_PRIORITY_VIDEO);
	ProcessFrameQueue(it->second.snd_queue.get());
	it->second.frame_seq++;
}

void VS_WebRtcMediaPeer::ReceiveAudio(const char *peer_name, const unsigned char *buf, int sz, unsigned int timestamp)
{
	if (m_isClosed)
		return;
	m_sstat.CountData(sz, 1);
	//TODO: fix extra copying for fast
	if (m_last_rtp_timestamp == 0) {
		m_audio_timestamp = timestamp;
		m_last_rtp_timestamp = timestamp;
	}
	m_audio_timestamp += ((timestamp - m_last_rtp_timestamp) * 1000 / m_rcv_samplerate);
	m_last_rtp_timestamp = timestamp;
	*(unsigned int*)(m_audioBuffer) = m_audio_timestamp;
	memcpy(m_audioBuffer + 4, buf, sz);
	m_snd_queue->AddFrame(1, sz + 4, m_audioBuffer, FRAME_PRIORITY_AUDIO);
	ProcessFrameQueue(m_snd_queue.get());
}

void VS_WebRtcMediaPeer::OnPeerDead()
{
	if (m_observer)
		m_observer->ReadyToDie(m_part_id.c_str(), m_peer_id.c_str());
}

MediaStat::MediaStat()
{
	Reset();
}

MediaStat::~MediaStat()
{
}

void MediaStat::Reset()
{
	m_vbytes = m_abytes = 0;
	m_vframes = m_aframes = 0;
	m_w = m_h = 0;
	m_stime = std::chrono::system_clock::now();
}

void MediaStat::CountData(uint32_t size, uint32_t type)
{
	if (type == 0) {
		m_vbytes += size;
		m_vframes++;
	}
	else {
		m_abytes += size;
		m_aframes++;
	}
}

void MediaStat::GetStat(uint64_t & bytes, double & fps, uint32_t& time)
{
	auto now = std::chrono::system_clock::now();
	time = std::chrono::duration_cast<std::chrono::seconds>(now - m_stime).count();
	if (!time) time++;
	bytes = (m_abytes + m_vbytes);
	fps = (double)m_vframes / time;
}

void MediaStat::SetResolution(uint32_t w, uint32_t h)
{
	m_w = w; m_h = h;
}

void MediaStat::GetResolution(uint32_t & w, uint32_t & h)
{
	w = m_w; h = m_h;
}
