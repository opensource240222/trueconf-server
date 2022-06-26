#pragma once

#include "std-generic/compat/memory.h"
#include "VS_AudioVideoMixer.h"
#include "LoadBalancing/BalancingModule.h"

#include <mutex>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

namespace vs
{
	class ASIOThreadPool;
	class ASIOThreadPoolBalancing;
}

class VS_ConferenceMixer : public vs::enable_shared_from_this<VS_ConferenceMixer>, public balancing_module::BalancingModule
{
	enum Events
	{
		cme_checkVideoQueue = 0,
		cme_checkAudio = 1,
		cme_size = 2// technically you don't need to assign anything, according to cppreference
	};

	static const std::uintptr_t m_defaultHandle = 0;

	std::unique_ptr<vs::ASIOThreadPoolBalancing> m_mixerTasksPool;
	std::unique_ptr<vs::ASIOThreadPoolBalancing> m_workerTasksPool;
	std::unique_ptr<vs::ASIOThreadPool>		m_commandTasksPool;
	std::unique_ptr<boost::asio::io_service::strand> m_strandCommand;
	std::unique_ptr<boost::asio::io_service::strand> m_strandProcessLayout;
	std::recursive_mutex					m_mtxVideoPool;
	std::recursive_mutex					m_mtxAudioPool;
	std::recursive_mutex					m_mtxCodecFramePool;
	std::thread								m_localLooper;
	vs::EventArray							m_localEvents;
	atomicMixerReceivers					m_receivers;
	atomicMixerPool							m_mixerPool;
	videoFramePool							m_codecFramePool;
	avmuxer::ScalableProperty				m_scalableProperty;
	std::shared_ptr<VS_MixerGrid>			m_mixerGrid;
	VS_MediaFormat							m_mf;
	unsigned int							m_key_frame_interval;
	unsigned int							m_key_frame_interval_min;
	std::atomic_bool						m_is_inited;
	bool									m_isNameOn;

private:

	std::shared_ptr<avmuxer::MixerItem<avmuxer::VideoItem>> GetVideoMixerItem(const std::uintptr_t handle, uint32_t fourcc);
	std::shared_ptr<avmuxer::LayoutItem>		GetLayoutMixer(const std::uintptr_t handle);
	std::shared_ptr<avmuxer::VideoPool>			GetVideoPool(const std::uintptr_t handle);
	std::shared_ptr<avmuxer::AudioPool>			GetAudioPool(const std::uintptr_t handle);
	std::shared_ptr<avmuxer::AudioPool>			GetAudioPoolNotify(const std::uintptr_t handle);
	std::shared_ptr<VS_MultiMixerVideo>			GetVideoMixer(const std::uintptr_t handle);
	std::shared_ptr<avmuxer::LayoutItem>		CreateMixer(const avmuxer::LayoutFormat &lf);
	void										CreateVideoMixer(const std::uintptr_t handle, avmuxer::LayoutItem *layoutItem);
	void										CreateAudioMixer(const std::uintptr_t handle, avmuxer::LayoutItem *layoutItem);
	std::shared_ptr<avmuxer::AudioPool>			CreateAudioMixer();
	void										DestroyVideoMixer(avmuxer::LayoutItem *layoutItem);
	void										DestroyAudioMixer(avmuxer::LayoutItem *layoutItem);
	avmuxer::VideoResampleItem					CreateVideoResampleItem(int32_t w, int32_t h);
	avmuxer::AudioResampleItem					CreateAudioResampleItem(int32_t sr);
	avmuxer::VideoItem*							CreateVideoItem(VS_MediaFormat *mf, uint32_t bitrate, const avmuxer::LayoutFormat &lf);
	avmuxer::AudioItem*							CreateAudioItem(VS_MediaFormat *mf, const std::uintptr_t handle);
	void										CorrectMediaFormatStream(VS_MediaFormat *mf);
	void										UpdateMixer(const std::uintptr_t handle, const avmuxer::LayoutFormat &lf);
	void										ProcessMixerLayout(const std::shared_ptr<avmuxer::VideoPool> &vp, uint32_t ct);
	void										ProcessMixerAsymmetricLayout(const std::shared_ptr<avmuxer::VideoPool> &vp, const avmuxer::LayoutFormat &lf, uint32_t ct);
	void										CheckMixerVideo(uint32_t ct);
	void										CheckMixerAudio(uint32_t ct);
	void										CheckVideoQueue(bool timeout);
	void										CallbackPushVideo(const avmuxer::LayoutFormat & lf, uint8_t *buffer, uint32_t svcMode, int size, bool keyframe, uint32_t timestamp, uint32_t mb, uint32_t fourcc);
	void										FormLayoutData(const std::uintptr_t & handle, const std::string & to);
	void										SendNotifyLayoutData(const std::uintptr_t & handle, const std::string & toPeer = "");
	std::shared_ptr<media_synch::VideoBuffer>	AllocateVideoBuffer(const std::shared_ptr<media_synch::AtomicCacheVideo> &cache, int32_t width, int32_t height, std::uintptr_t context);

public:

	~VS_ConferenceMixer();
	bool Init(VS_MediaFormat *mf, VS_MixerGrid* MixGrid, const avmuxer::ScalableProperty &scalableProperty, uint32_t numThreads = 0);
	bool CreateMixer(const std::uintptr_t handle, const avmuxer::LayoutFormat &lf);
	bool DestroyMixer(const std::uintptr_t handle);
	bool CreateVideoPool(const std::uintptr_t handle, uint32_t fourcc, int32_t *tl);
	bool DestroyVideoPool(const std::uintptr_t handle, uint32_t fourcc);
	bool CreateAudioPool(const std::uintptr_t handle, uint32_t twocc);
	bool DestroyAudioPool(const std::uintptr_t handle, uint32_t twocc);
	bool AddVideoPeerStream(const std::uintptr_t handle, const VS_MediaFormat &mf, uint32_t *mb);
	bool AddAudioPeerStream(const std::uintptr_t handle, const VS_MediaFormat &mf);
	bool RemoveVideoPeerStream(const std::uintptr_t handle, uint32_t fourcc, uint32_t mb);
	bool RemoveAudioPeerStream(const std::uintptr_t handle, uint32_t twocc, uint32_t samplerate);
	void EnableVideoLayer(bool enable, const std::uintptr_t handle, uint32_t fourcc, uint32_t mb);
	void EnableAudioLayer(bool enable, const std::uintptr_t handle, uint32_t twocc, uint32_t samplerate);
	int  RequestKeyFrame(const std::uintptr_t handle, uint32_t fourcc);
	void SetBitrate(const std::uintptr_t handle, uint32_t fourcc, uint32_t bitrate, int32_t framerate);
	void ManageLayout(const std::uintptr_t handle, const avmuxer::LayoutControl & layoutControl);
	void SetFlags(int flags);
	/////////////////////////////////////////////////////////////
	void StreamCreate(const std::string & handle, VS_MediaFormat *mf, const std::string & name);
	void StreamRelaese(const std::string & handle);
	bool StreamChangeName(const std::string & handle, const std::string & name);
	bool StreamSetActive(const std::string & handle, bool active);
	/////////////////////////////////////////////////////////////
	void Stop();
	bool Go();
	void Release();
	//////////////////////////////////////////////////////////
	void PutAudio(const std::map <std::string /* callid */, mixerAudioSynchBuffer> &frames);
	void PutVideo(const std::string &callId, const mixerVideoSynchBuffer &frames);
	void PutPassData(const std::string &callId, uint8_t *data, int32_t size, stream::Track type);
	//////////////////////////////////////////////////////////
	void Loop();

public:

	boost::signals2::signal<void(const std::uintptr_t, const infoSenderStreams &)> fireUpdateInfoStreams;
	boost::signals2::signal<void(const avmuxer::LayoutPayload &, uint8_t*, int, uint8_t, bool, uint32_t)> fireSendVideo;
	boost::signals2::signal<void(const avmuxer::LayoutFormat &, uint32_t, uint32_t, uint8_t*, int, uint32_t, uint32_t)> fireSendAudio;
	boost::signals2::signal<void(const std::uintptr_t, const avmuxer::LayoutControl &)> fireNewLayoutList;

protected:

	VS_ConferenceMixer();

};
