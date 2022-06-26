#include "VS_MultiMixerTransceiverPass.h"
#include "VS_SingleRayPass.h"

VS_MultiMixerTransceiverPass::VS_MultiMixerTransceiverPass() : VS_MultiMixerVideo({ 0, 0, 0, 0, 0, 0 })
{
}

VS_MultiMixerTransceiverPass::~VS_MultiMixerTransceiverPass()
{
}

bool VS_MultiMixerTransceiverPass::Get(uint8_t *buf, int32_t &size, uint32_t &mb, uint8_t &tl, bool &key, uint32_t &tm, uint32_t &fourcc)
{
	unsigned long vi(0);
	bool ret = m_mr.begin()->second->GetVideo(buf, size, vi, mb, tl, key, fourcc);
	if (ret) {
		auto it = m_layersTm.find(mb);
		if (it == m_layersTm.end()) {
			m_layersTm[mb] = vi;
			tm = vi;
		}
		else {
			it->second += vi;
			tm = it->second;
		}
	}
	return ret;
}

VS_VideoRay* VS_MultiMixerTransceiverPass::CreateRay(const std::string& dname, const VS_MediaFormat& out, int backgndColor)
{
	return new VS_SingleRayPass(dname);
}
