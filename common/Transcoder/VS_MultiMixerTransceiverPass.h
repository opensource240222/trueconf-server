#pragma once

#include "mixer/VS_MultiMixerVideo.h"

class VS_MultiMixerTransceiverPass : public VS_MultiMixerVideo
{
private:
	std::map<uint32_t /*mb*/, uint64_t /* tm */> m_layersTm;
public:
	VS_MultiMixerTransceiverPass();
	~VS_MultiMixerTransceiverPass();
	bool Get(uint8_t *buf, int32_t &size, uint32_t &mb, uint8_t &tl, bool &key, uint32_t &tm, uint32_t &fourcc) override;

protected:
	VS_VideoRay* CreateRay(const std::string& dname, const VS_MediaFormat& out, int backgndColor) override;
};
