#include "VS_SingleRayPass.h"
#include "VS_VS_Buffers.h"
#include "MediaParserLib/VS_VPXParser.h"
#include "MediaParserLib/VS_H264Parser.h"

VS_SingleRayPass::VS_SingleRayPass(const std::string& dname)
	: VS_AudioRay({})
	, VS_VideoRay(dname, {}, 0x0)
{
}

VS_SingleRayPass::~VS_SingleRayPass()
{

}

/*bool VS_SingleRayPass::AddVideo(uint8_t *video, int32_t procWidth, int32_t procHeight, int32_t baseWidth, int32_t baseHeight)
{
	// need SIZE!
	//auto packet = boost::shared_ptr<uint8_t>(new uint8_t[size]);
	//memcpy(packet.get(), video, size);
	//m_videoQueue.push(std::make_pair(packet, size));
	return true;
}*/

bool VS_SingleRayPass::AddAudio(uint8_t *audio, int32_t size)
{
	auto packet = boost::shared_ptr<uint8_t>(new uint8_t[size]);
	memcpy(packet.get(), audio, size);
	m_audioQueue.push(std::make_pair(packet, size));
	return true;
}

bool VS_SingleRayPass::GetVideo(uint8_t *video, int32_t &size, unsigned long &vi, uint32_t &mb, uint8_t &tl, bool &key, uint32_t &fourcc)
{
	mb = 0;
	while (!m_videoQueue.empty()) {
		uint8_t *buffer = m_videoQueue.front().first.get();
		unsigned long bs = m_videoQueue.front().second;
		int max_sl(0), spatial(0), temporal(0), quality(0);
		/*if (VS_VideoRay::m_infmt.dwSVCMode != 0) {
			quality = (buffer[bs - 1] & 0xc0) >> 6;
			spatial = (buffer[bs - 1] & 0x30) >> 4;
			temporal = (buffer[bs - 1] & 0xc) >> 2;
			max_sl = (buffer[bs - 1] & 0x3);
			bs--;
		}*/
		auto in = &m_videoInput[spatial];
		if (in->second.Add(buffer, bs, stream::Track::video)) {
			uint8_t *p(nullptr);
			unsigned long ns(0);
			if (in->second.GetRef(p, ns, vi, key) == stream::Track::video) {
				if (key) {
					int w(0), h(0), nth(0);
					if (ResolutionFromBitstream_VPX(p, ns, w, h, nth) == 0) {
						in->first = w * h / 256;
					}
					else if (ResolutionFromBitstream_H264(p, ns, w, h) == 0) {
						in->first = w * h / 256;
					}
					else {
						in->first = 0;
					}
				}
				//fourcc = VS_VideoRay::m_infmt.dwVideoCodecFCC;
				size = ns;
				mb = in->first;
				tl = temporal;
				memcpy(video, p, size);
				in->second.Reset(stream::Track::video);
			}
		}
		m_videoQueue.pop();
		if (mb > 0) {
			return true;
		}
	}
	return false;
}

uint8_t* VS_SingleRayPass::GetAudio(int32_t &size)
{
	uint8_t *audio(nullptr);
	size = 0;
	if (!m_audioQueue.empty()) {
		size = m_audioQueue.front().second;
		audio = m_audioQueue.front().first.get();
	}
	return audio;
}

void VS_SingleRayPass::DropAudio(int32_t size)
{
	if (!m_audioQueue.empty()) {
		m_audioQueue.pop();
	}
}
