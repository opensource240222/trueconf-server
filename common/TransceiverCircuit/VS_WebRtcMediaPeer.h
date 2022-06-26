#pragma once
#include "VS_MediaPeerBase.h"
#include <boost/signals2.hpp>
#include <chrono>

#include "rtc_base/sigslot.h"

#include "streams/fwd.h"
#include "streams/Relay/VS_ConfControlInterface.h"

namespace tc_webrtc_api
{
	class TrueConfMediaEngine;
}

namespace rtc
{
	class Thread;
	class CriticalSection;
}

class VS_SendFrameQueueBase;
class AudioCodec;
class VS_FifoBuffer;
struct TConferenceStatistics;

class MediaStat
{
	uint64_t		m_vbytes;
	uint64_t		m_abytes;
	uint32_t		m_vframes;
	uint32_t		m_aframes;
	uint32_t		m_w;
	uint32_t		m_h;
public:
	std::chrono::system_clock::time_point	m_stime;

	MediaStat();
	~MediaStat();
	void Reset();
	void CountData(uint32_t size, uint32_t type);
	void GetStat(uint64_t& bytes, double& fps, uint32_t& time);
	void SetResolution(uint32_t w, uint32_t h);
	void GetResolution(uint32_t &w, uint32_t &h);
};

class VS_WebRtcMediaPeer :	public VS_MediaPeerBase,
							public sigslot::has_slots<>


{
	MediaStat		m_rstat;
	MediaStat		m_sstat;

	struct StatePeerStream
	{
		int w;
		int h;
		int mb;
		int sl;
		int frame_seq;
		boost::shared_ptr<VS_SendFrameQueueBase> snd_queue;
		StatePeerStream() : w(0), h(0), mb(0), sl(0), frame_seq(0) {}
	};
	std::map <std::string, StatePeerStream> m_streams;

	unsigned char *m_videoBuffer;
	unsigned char *m_audioBuffer;
	int			   m_rcv_samplerate;
	unsigned int   m_audio_timestamp;
	unsigned int   m_last_rtp_timestamp;

public:

	typedef boost::signals2::signal<void (const unsigned char *, const unsigned long, stream::Track)>	RecvFrameSignal;
	typedef RecvFrameSignal::slot_type																		RecvFrameSignalSlot;


	typedef boost::signals2::signal<void (const char*, const char *, const int)>	AudioPayLoadChangeSignal;
	typedef AudioPayLoadChangeSignal::slot_type										AudioPayLoadChangeSignalSlot;

	VS_WebRtcMediaPeer(const char *peer_id, const char *part_id, rtc::Thread* capturer_thread = 0);

	virtual ~VS_WebRtcMediaPeer();

	tc_webrtc_api::TrueConfMediaEngine*	GetMediaEngine();
	void AddTrack(const std::string &trackid);
	void RemoveTrack(const std::string &trackid);
	void RemoveTracks();
	// stat
	void GetStat(TConferenceStatistics *cst);

////VS_MediaPeerBase
	void PutVideo(unsigned char *pFrame, unsigned long size, bool isKey, unsigned timestamp) override;
	void PutCompressedAudio(unsigned char *buf, unsigned long sz, unsigned int samples, unsigned int timestamp) override;
	void PutUncompressedAudio(unsigned char *buf, unsigned long sz, unsigned int timestamp) override;
	void ReceiveAudio(const char *peer_name, const unsigned char *buf, int sz, unsigned int timestamp) override;

	void ReceiveVideo(const char *peer_name, const char *stream_id, const unsigned char *pFrame, int size, bool isKey, unsigned int timestamp) override;
	void ChangeAudioRcvPayload(const char *plname, const int plfreq) override;
	void ChangeRcvFrameResolution(const char *peer_name, const char *stream_id, const char *plname, uint8_t pltype, unsigned short width, unsigned short height) override;

	void RequestKeyFrame();
	void SetPeerBitrate(int bitrate);
	void RequestKeyFrameFromPeer();

	void SetRates(const char *peer_name, unsigned int bitrate, unsigned int frame_rate);
	void SetSndFrameResolution(const char *peer_name, const char *plname, uint8_t pltype, unsigned short width, unsigned short height);

	void ChangeAudioSendPayload(const char *plname, int plfreq);
	void ChangeVideoSendPayload(const VS_MediaFormat& mf);

	boost::signals2::connection	ConnectToReceiveFrame(const RecvFrameSignalSlot &slot){
		return m_fireReceiveFrame.connect(slot);}
	boost::signals2::connection ConnectToAudioPayLoadChange(const AudioPayLoadChangeSignalSlot &slot){
		return m_fireAudioChangeAudioRcvPayload.connect(slot);
	}

private:
	void OnPeerDead();
	void ProcessFrameQueue(VS_SendFrameQueueBase *queue);

	rtc::CriticalSection *m_queueLock;
	boost::shared_ptr<tc_webrtc_api::TrueConfMediaEngine>	m_media_engine;

	boost::shared_ptr<VS_SendFrameQueueBase>			m_snd_queue;

	RecvFrameSignal		m_fireReceiveFrame;
	AudioPayLoadChangeSignal	m_fireAudioChangeAudioRcvPayload;
};
