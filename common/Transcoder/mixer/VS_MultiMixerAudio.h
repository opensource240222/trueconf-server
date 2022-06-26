#pragma once

#pragma once

#include <map>
#include <set>
#include <memory>
#include <boost/functional/hash.hpp>
#include "VS_AudioRay.h"
#include "Transcoder/VS_VideoWindow.h"
#include "Transcoder/VS_AudioMixer.h"

#define MAX_NUM_STREAMS		(64)
#define LIMIT_CUR_STREAMS	(64)

class VS_MultiMixerAudio
{
public:
	VS_MultiMixerAudio();
	~VS_MultiMixerAudio();

	bool Add(unsigned char* buff, int size, const std::string &handle);

	// audio
	uint32_t GetAudio(uint8_t *buff, int32_t lenght);
	uint32_t GetAudio(const std::string &handle, uint8_t *buff, int32_t lenght);
	uint32_t PrepareAudio(int32_t lenght);
	bool ChangeSampleRate(int SampleRate);
	int32_t GetSampleRate() const;
	std::set<std::string> GetHandleRays() const;

	// common
	virtual bool AddRay(const std::string &handle, const VS_MediaFormat &in);
	bool DeleteRay(const std::string &handle);

protected:
	std::map<std::string, std::shared_ptr<VS_AudioRay>> m_mr;
	std::set<std::string> m_handleRays;
	std::unique_ptr<int16_t[]> m_mixBuffer;

	VS_MediaFormat	m_outFormat;
	VS_AudioMixer   m_amixer;

	virtual VS_AudioRay* CreateRay(const VS_MediaFormat& out);
};
