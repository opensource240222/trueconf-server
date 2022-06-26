#pragma once

#include "VS_AudioVideoMixer.h"
#include "VS_MixerReceiver.h"
#include "LoadBalancing/BalancingModule.h"

namespace vs
{
	class ASIOThreadPool;
	class ASIOThreadPoolBalancing;
}

class ThreadDecompressor : public balancing_module::BalancingModule
{

private:

	enum Events
	{
		tde_newData = 0,
		tde_size = 1
	};

	atomicReceivers						*m_receivers;
	std::unique_ptr<vs::ASIOThreadPoolBalancing> m_decoderPool;
	uint32_t							m_numThreads = 0;

	std::thread							m_localLooper;
	vs::EventArray						m_localEvents;

private:

	void                            Loop();

public:

	boost::signals2::signal<void(const avmuxer::LoadStatistic &)> fireUpdateLoadStatistic;
	boost::signals2::signal<void(const std::string & /* sender id */)> fireKeyFrameRequest;

public:

	ThreadDecompressor();
	~ThreadDecompressor();
	int Init(atomicReceivers *receivers);
	void Go();
	void Stop();

	inline void SetNewDataEvent() {
		m_localEvents.set(tde_newData);
	}
};
