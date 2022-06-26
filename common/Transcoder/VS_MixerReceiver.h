#pragma once

#include "VS_AudioVideoMixer.h"
#include "VS_VS_Buffers.h"
#include "VideoCodec.h"
#include "VS_AudioSource.h"

namespace mixer_receiver
{

	const int32_t max_buffer_lenght = 30;
	const int32_t default_video_queue_limit = 4 * 30 * 10240; // 4 sec for 10240KB 30fps
	const int32_t min_video_queue_bitrate = 256 * 1024 / 8; // 256kbps

	typedef std::unique_ptr<mixer_receiver::PushFrame> PacketPtr;

	struct TrackDescription
	{
		stream::TrackType media_type;
		mutable std::mutex mtx_packets;
		std::deque<PacketPtr> packets;
		VS_VS_InputBuffer vs_input;
		VS_MediaFormat mf;
		std::unique_ptr<VideoCodec> vc;
		VS_TimeDeviation<uint64_t> predictor;
		VS_TimeDeviation<int> audiodurr;
		VS_TimeDeviation<double> frame_interval;
		VS_TimeDeviation<double> frame_size;
		std::shared_ptr<mixer_receiver::AtomicCacheVideo> cache;
		mixer_receiver::CycleBuffer cycle_buffer;
		std::uintptr_t context_device = 0;
		bool wait_key_frame = true;
		uint64_t last_fill_time = 0;
		uint64_t last_frame_time = 0;
		int32_t prev_buffer_duration = 0;
		int32_t buffer_s = 0;
		int64_t	queue_limit_bytes = default_video_queue_limit;
		int64_t	queue_bytes = 0;

	public:

		TrackDescription(stream::TrackType type);
		UniqueVideoBuffer AllocateVideoBuffer(bool& emptyCache);
		void CorrectCacheLenght(uint64_t due, uint64_t ct, double vi, int32_t cycleBufferLenght, bool emptyCache);
		PacketPtr GetPacket(bool &needKey, uint32_t &numPackets);

	};

	typedef std::shared_ptr<mixer_receiver::TrackDescription> TrackDescriprionPtr;

}

class VS_MixerReceiver
{

private:

	std::mutex					m_mtxTracks;
	std::mutex					m_mtxAudioLock;
	std::string					m_CallId;
	std::unique_ptr<VS_AudioSource> m_audioSource;
	unsigned long				m_capsFourcc = 0;
	bool						m_IsInited = false;
	bool						m_IsNeedDecoding = true;
	uint32_t					m_skip_audio_frames_counter = 0;
	VS_MediaFormat				m_new_audio_mf;
	std::map<stream::Track, mixer_receiver::TrackDescriprionPtr> m_tracksDesc;

private:

	int CheckMediaFormat(mixer_receiver::TrackDescription *desc, uint8_t* buffer, bool key, uint32_t size);
	int ChangeVideoFormat(mixer_receiver::TrackDescription *desc, VS_MediaFormat& mf);
	int ChangeAudioFormat(mixer_receiver::TrackDescription *desc, VS_MediaFormat& mf);
	mixer_receiver::TrackDescriprionPtr GetTrackDescription(stream::Track track);
	bool ProcessCompressedData(mixer_receiver::TrackDescription* desc, mixer_receiver::PacketPtr &&frame, bool &needKey);
	bool ProcessUncompressedData(mixer_receiver::TrackDescription* desc, mixer_receiver::PacketPtr&& frame);
	uint64_t GetSynchronizationTime(mixer_receiver::TrackDescription* desc, uint64_t ct, unsigned long videoInterval);
	void CorrectBuffersLenght(mixer_receiver::TrackDescription *desc, uint64_t ct, uint64_t dueTime, unsigned long videoInterval, unsigned long compressedSize, bool emptyCache);
	void StorageVideoBuffer(mixer_receiver::TrackDescription* desc, mixer_receiver::UniqueVideoBuffer &&vb, uint64_t dueTime);

public:

	VS_MixerReceiver();
	~VS_MixerReceiver();
	int Init(VS_MediaFormat *mf, unsigned long capsFourcc, char *callId, const char *StreamName);
	bool AddTrack(stream::Track track, stream::TrackType type);
	bool RemoveTrack(stream::Track track);
	void SetActive(bool set);
	bool Activated() const;
	void Release();
	long PushData(unsigned char* buff, int size, stream::Track track);
	int ReceiveVideo(stream::Track track, uint32_t &packets, bool &needKey);
	int ReceiveAudio();
	void CheckAudio(mixer_receiver::AudioBuffer *ab, uint64_t dt);
	mixer_receiver::UniqueVideoBuffer CheckVideo(stream::Track track, const uint64_t ct, uint64_t& nexttime);
	void SetNewAudioFmt(const VS_MediaFormat &fmt);
	uint32_t GetVol();

};

struct MixerTrackReceiver
{
	std::shared_ptr<VS_MixerReceiver> rcv;
	std::vector<std::weak_ptr<VS_MixerReceiver>> rcvs_queue;
	stream::Track track = stream::Track::undef;
	stream::TrackType type = stream::TrackType::undef;
	MixerTrackReceiver(stream::Track tr, stream::TrackType tp) : track(tr), type(tp) { };
};

typedef std::map<std::string, std::shared_ptr<MixerTrackReceiver>> mixerReseivers;
typedef vs::atomic_shared_ptr<mixerReseivers> atomicReceivers;
