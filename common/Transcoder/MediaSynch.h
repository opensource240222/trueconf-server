#pragma once

#include "VS_AudioVideoMixer.h"
#include "VS_MixerReceiver.h"
#include "LoadBalancing/BalancingModule.h"

namespace vs
{
	class ASIOThreadPool;
	class ASIOThreadPoolBalancing;
}

class MediaSynch : public balancing_module::BalancingModule
{

public:

	MediaSynch();
	~MediaSynch();
	bool Init(atomicReceivers *receivers);
	void Release();
	void UpdateInfoStreams(const std::uintptr_t mixerHandle, const infoSenderStreams &infoStreams);
	void Run();
	void Stop();

public:

	boost::signals2::signal<void(const std::map <std::string, mixerAudioSynchBuffer> &)> firePutAudioSignal;
	boost::signals2::signal<void(const std::string &, const mixerVideoSynchBuffer &)> firePutVideoSignal;
	boost::signals2::signal<void(const std::map<std::string, int32_t> &)> fireMbParticipantsUpdateSignal;

private:

	typedef std::map<std::string /* stream id */, std::vector<media_synch::AudioProcessingItem>> synchAudioPool;
	typedef std::map<std::string /* stream id */, std::vector<media_synch::VideoProcessingItem>> synchVideoPool;

	void SynchThreadFunc();
	void PrepareBuffers();
	void PrepareAudioBuffers(synchAudioPool & pool, const std::string &id, uint32_t sr, std::uintptr_t handle);
	void PrepareVideoBuffers(synchVideoPool & pool, const std::string &id, uint32_t w, uint32_t h, std::uintptr_t handle);
	mixerAudioSynchBuffer ProcessAudioStream(const std::string &id, mixer_receiver::AudioBuffer *ab);
	mixerVideoSynchBuffer ProcessVideoStream(const std::string &id, mixer_receiver::VideoBuffer *vb);
	std::shared_ptr<media_synch::VideoBuffer> AllocateVideoBuffer(media_synch::VideoProcessingItem *vpi, std::uintptr_t context);
	std::shared_ptr<media_synch::VideoBuffer> ProcessingSoftware(mixer_receiver::VideoBuffer *vb, media_synch::VideoProcessingItem *vpi, uint8_t **src);
	std::shared_ptr<media_synch::VideoBuffer> ProcessingHardware(mixer_receiver::VideoBuffer *vb, media_synch::VideoProcessingItem *vpi, uint8_t **src);
	void ProcessNonMultipleFrame(mixer_receiver::VideoBuffer *vb);

private:

	std::thread m_synchThread;
	bool m_releaseThread;
	vs::event m_checkData{ false };
	atomicReceivers* m_receivers = nullptr;
	atomicQueueRequestInfoStreams m_queueInfoStreams;
	mixerInfoStreams m_infoStreams;
	synchVideoPool m_videoProcessingPool;
	synchAudioPool m_audioProcessingPool;
	std::unique_ptr<vs::ASIOThreadPoolBalancing> m_synchPool;

};
