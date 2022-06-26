#include "std-generic/compat/memory.h"
#include "VS_VideoCodecManager.h"
#include "VS_SysBenchmarkBase.h"
#include "VS_VideoCodecState.h"
#include "VideoCodec.h"
#include "IppLib2/VSVideoProcessingIpp.h"
#include "streams/Protocol.h"
#include "std/cpplib/VS_MediaFormat.h"

#include <algorithm>
#include <thread>

const int VS_VideoCodecManager::slBitrateLimit[VS_VideoCodecManager::BTR_LIMIT_MAX][VS_VideoCodecManager::MAX_SPATIAL_LAYER] =
{
	{  0,  100,  512,  512,  512},
	{  0,  100,  410,  410,  410},
	{  0,  128,  512, 1024, 1024}
	//{128,  512, 1024, 1024, 1024}
};

const double VS_VideoCodecManager::slBitrateCoefs[VS_VideoCodecManager::MAX_SPATIAL_LAYER] =
{
	1.0, 0.4, 0.16, 0.0, 0.0
};

VS_VideoCodecManager::VS_VideoCodecManager()
{
	m_bValid = false;
	m_supportMode = 0x00070100;
}

VS_VideoCodecManager::~VS_VideoCodecManager()
{
	Release();
}

bool VS_VideoCodecManager::Init(VS_MediaFormat *mf, unsigned char senderLvl)
{
	Release();

	m_maxWidth = mf->dwVideoWidht;
	m_maxHeight = mf->dwVideoHeight;
	m_baseWidth = m_maxWidth;
	m_baseHeight = m_maxHeight;
	m_maxBitrateK = VS_VideoCodecManager::slBitrateCoefs[0];
	m_currentMode = mf->dwSVCMode;
	if (m_currentMode != 0xffffffff) {
		m_currentMode &= m_supportMode;
		if (m_currentMode & 0x00ff0000) {
			if (m_currentMode & 0x00020000) {
				m_limitSLayers = 3;
			}
			else if (m_currentMode & 0x00010000) {
				m_limitSLayers = 2;
			}
		}
	}
	m_maxSLayers = 1;
	int thrMB = (m_currentMode & 0x00040000) ? 1600 : 3600;
	if (mf->GetFrameSizeMB() >= 900 && m_maxSLayers < m_limitSLayers) {
		m_maxSLayers++;
		m_maxBitrateK += VS_VideoCodecManager::slBitrateCoefs[1];
		if (mf->GetFrameSizeMB() >= thrMB && m_maxSLayers < m_limitSLayers) {
			m_maxSLayers++;
			m_maxBitrateK += VS_VideoCodecManager::slBitrateCoefs[2];
		}
	}
	m_numSLayers = m_maxSLayers;
	int mulWidth = (mf->dwVideoCodecFCC == VS_VCODEC_VPXSTEREO) ? 2 : 1;
	unsigned int currMode = m_currentMode;
	if (senderLvl == 0) {
		VS_SysBenchmarkBase benchmark;
		senderLvl = benchmark.GetSndLevel();
	}
	for (int i = 0; i < m_maxSLayers; i++) {
		int ret(-1);
		bool main(i == 0);
		VS_MediaFormat fmt_main = *mf;
		VS_MediaFormat fmt = *mf;
		{
			fmt.dwVideoWidht = (m_maxWidth >> i) &~1;
			fmt.dwVideoHeight = (m_maxHeight >> i) &~1;
			if (fmt.dwHWCodec == ENCODER_H264_NVIDIA && !main) {
				fmt.dwHWCodec = ENCODER_SOFTWARE;
			}
		}
		auto desc = std::make_unique<VS_VideoCodecState>(fmt);
		auto codec = desc.get();
		uint32_t w(fmt.dwVideoWidht), h(fmt.dwVideoHeight);
		uint32_t wm(fmt_main.dwVideoWidht), hm(fmt_main.dwVideoHeight);
		if (!main) {
			codec->thread = std::thread(
				[codec, w, h, wm, hm]() -> int32_t
				{
					double k = (double)w / (double)wm;
					auto proc = std::make_unique<VSVideoProcessingIpp>();
					auto resampleFrame = vs::make_unique_default_init<uint8_t[]>(w * h * 3 / 2);
					uint8_t *rsmpl[3] = { resampleFrame.get(), resampleFrame.get() + w * h, resampleFrame.get() + 5 * w * h / 4 };
					while (true) {
						if (codec->shutdown) {
							break;
						}
						codec->eventStartEncode.wait();
						if (codec->shutdown || codec->cdc == 0) {
							break;
						}
						uint8_t *src[3] = { codec->srcFrame, codec->srcFrame + wm * hm, codec->srcFrame + 5 * wm * hm / 4 };
						proc->ResampleCropI420(src, rsmpl, wm, hm, wm, w, h, w, wm, hm, 0, 0, k, k, IPPI_INTER_LINEAR);
						int32_t idx(codec->shift % codec->encodeSize.size());
						codec->EncodeRoutine(resampleFrame.get(), codec->shift % codec->encodeSize.size());
						if (codec->encodeSize[idx] > 0) {
							codec->shift++;
						}
						codec->eventEndEncode.set();
					}
					return 0;
				}
			);
		}
		uint32_t svcMode(m_currentMode);
		if (codec && codec->cdc) {
			unsigned char sndLvl = (i == (m_maxSLayers - 1)) ? senderLvl : ((i > 0) ? 10 : 0);
			ret = codec->cdc->Init(w, h * mulWidth, FOURCC_I420, sndLvl);
			if (!codec->cdc->SetSVCMode(svcMode)) {
				svcMode = 0x0;
			}
			if (main && currMode != 0xffffffff) {
				currMode &= 0x00ff0000;
				currMode |= (0x0000ffff & svcMode);
				currMode |= (0xff000000 & (((unsigned int)(m_maxSLayers - 1)) << 24));
			}
			m_numThreads += codec->cdc->GetNumThreads();
		}
		if (ret != 0) {
			Release();
			break;
		}
		m_codecDesc.push_back(std::move(desc));
		m_bValid = true;
	}
	m_currentMode = currMode;
	mf->dwSVCMode = m_currentMode;
	return m_bValid;
}

void VS_VideoCodecManager::Release()
{
	m_bValid = false;
	m_codecDesc.clear();
	m_currentMode = 0x0;
	m_maxWidth = m_maxHeight = 0;
	m_baseWidth = m_baseHeight = 0;
	m_setFramerate = m_setFramerateSet = 0;
	m_baseBitrate = m_baseBitrateSet = 0;
	m_maxBitrate = m_maxBitrateSet = 0;
	m_numSLayers = m_limitSLayers = m_maxSLayers = 1;
	m_maxBitrateK = 1.0;
	m_numThreads = 1;
	m_layerFrameSizeMBs.clear();
}

bool VS_VideoCodecManager::SetBitrate(int baseBitrate, int maxBitrate, int framerate)
{
	std::lock_guard<std::mutex> lock(m_mtxEncode);
	m_setFramerateSet = framerate;
	m_baseBitrateSet = baseBitrate;
	m_maxBitrateSet = maxBitrate;
	return true;
}

void VS_VideoCodecManager::UpdateBitrate()
{
	int baseBitrate(0), maxBitrate(0), framerate(0);
	{
		std::lock_guard<std::mutex> lock(m_mtxEncode);
		baseBitrate = m_baseBitrateSet;
		maxBitrate = m_maxBitrateSet;
		framerate = m_setFramerateSet;
	}
	if (baseBitrate != m_baseBitrate ||
		maxBitrate != m_maxBitrate ||
		framerate != m_setFramerate)
	{
		int sid = 0;
		double kBitrate = m_maxBitrateK;
		double bitrate = baseBitrate * kBitrate;
		bitrate = std::min<decltype(bitrate)>(bitrate, maxBitrate);
		baseBitrate = (int)(bitrate / kBitrate + 0.5);
		if (m_currentMode != 0x0 && m_currentMode != 0xffffffff) {
			for (int i = 0; i < m_maxSLayers; i++) {
				int id = m_maxSLayers - 1 - i;
				int set_bitrate = (int)(baseBitrate * VS_VideoCodecManager::slBitrateCoefs[sid] + 0.5);
				if (set_bitrate < VS_VideoCodecManager::slBitrateLimit[VS_VideoCodecManager::BTR_LIMIT_LOW][id]) {
					kBitrate -= VS_VideoCodecManager::slBitrateCoefs[id];
					baseBitrate = (int)(bitrate / kBitrate + 0.5);
					sid = 0;
					if (m_codecDesc[i]->state == VS_VideoCodecState::STATE_WAIT_IDLE) m_codecDesc[i]->state = VS_VideoCodecState::STATE_SKIP;
					if (m_codecDesc[i]->state != VS_VideoCodecState::STATE_SKIP) m_codecDesc[i]->state = VS_VideoCodecState::STATE_WAIT_SKIP;
				}
				else if (set_bitrate > VS_VideoCodecManager::slBitrateLimit[VS_VideoCodecManager::BTR_LIMIT_HIGH][id]) {
					sid++;
					if (m_codecDesc[i]->state == VS_VideoCodecState::STATE_SKIP) m_codecDesc[i]->state = VS_VideoCodecState::STATE_WAIT_IDLE;
					else if (m_codecDesc[i]->state == VS_VideoCodecState::STATE_WAIT_SKIP) m_codecDesc[i]->state = VS_VideoCodecState::STATE_IDLE;
					m_codecDesc[i]->cdc->SetBitrate(set_bitrate * 10 * 100 / framerate);
				}
			}
		}
		else {
			if (m_codecDesc[0]->format.dwHWCodec != ENCODER_H264_LOGITECH) {
				m_codecDesc[0]->cdc->SetBitrate(baseBitrate * 10 * 100 / framerate);
			}
			else {
				m_codecDesc[0]->cdc->SetBitrate(baseBitrate);
			}
		}
		m_baseBitrate = baseBitrate;
		m_maxBitrate = maxBitrate;
		m_setFramerate = framerate;
	}
}

int VS_VideoCodecManager::Convert(unsigned char *in, unsigned char *out, bool *key)
{
	int ret(0), i(0);
	if (!IsValid()) {
		return 0;
	}
	UpdateBitrate();
	for (i = 0; i < m_maxSLayers; i++) {
		m_codecDesc[i]->PushEncode(in, *key);
	}
	auto st = m_codecDesc[0].get();
	st->EncodeRoutine(in, 0);
	for (i = 1; i < m_maxSLayers; i++) {
		m_codecDesc[i]->eventEndEncode.wait();
	}
	if (st->encodeSize[0] <= 0) {
		for (i = 1; i < m_maxSLayers; i++) {
			if (m_codecDesc[i]->encodeSize[0] > 0) {
				m_codecDesc[i]->encodeSize.push_back(0);
				auto encframe = vs::make_unique_default_init<uint8_t[]>(m_codecDesc[i]->format.dwVideoWidht * m_codecDesc[i]->format.dwVideoHeight * 4);
				m_codecDesc[i]->encodeFrame.push_back(std::move(encframe));
			}
		}
	}
	else {
		st->shift++;
	}
	m_layerFrameSizeMBs.clear();
	*key = false;
	m_numSLayers = 0;
	for (i = 0; i < m_maxSLayers; i++) {
		st = m_codecDesc[i].get();
		if (st->state == VS_VideoCodecState::STATE_SKIP || st->state == VS_VideoCodecState::STATE_WAIT_IDLE) continue;
		if (m_numSLayers == 0 && st->keyFrame) {
			m_baseWidth = st->format.dwVideoWidht;
			m_baseHeight = st->format.dwVideoHeight;
			*key = st->keyFrame;
		}
		m_layerFrameSizeMBs.push_back(st->format.GetFrameSizeMB());
		m_numSLayers++;
	}

	ret = 0;
	unsigned char *cmpFrame = out;
	int sl = 0;
	for (i = 0; i < m_maxSLayers; i++) {
		st = m_codecDesc[i].get();
		if (st->state == VS_VideoCodecState::STATE_SKIP || st->state == VS_VideoCodecState::STATE_WAIT_IDLE || st->shift < st->encodeSize.size()) continue;
		unsigned int shiftIdx = st->shift % st->encodeSize.size();
		if (st->encodeSize[shiftIdx] < 0) {
			ret = -1;
			break;
		}
		ret += st->encodeSize[shiftIdx];
		if (m_currentMode != 0x0 && m_currentMode != 0xffffffff) {
			*(int*)cmpFrame = st->encodeSize[shiftIdx] + sizeof(stream::SVCHeader);
			cmpFrame += sizeof(int);
			ret += sizeof(int);
			auto h = reinterpret_cast<stream::SVCHeader*>(cmpFrame);
			h->spatial = sl++;
			h->quality = (unsigned char)((st->svc & 0x0000ff00) >> 8);
			h->temporal = (unsigned char)(st->svc & 0x000000ff);
			h->maxspatial = m_numSLayers - 1;
			cmpFrame += sizeof(stream::SVCHeader);
			ret += sizeof(stream::SVCHeader);
		}
		memcpy(cmpFrame, st->encodeFrame[shiftIdx].get(), st->encodeSize[shiftIdx]);
		cmpFrame += st->encodeSize[shiftIdx];
	}

	return ret;
}

void VS_VideoCodecManager::GetResolutionBaseLayer(int *width, int *height)
{
	*width = m_baseWidth;
	*height = m_baseHeight;
	m_baseWidth = -1;
	m_baseHeight = -1;
}

std::vector<int> VS_VideoCodecManager::GetLayerFrameSizeMBs()
{
	return m_layerFrameSizeMBs;
}

uint32_t VS_VideoCodecManager::GetNumThreads() const
{
	return std::max<uint32_t>(m_numThreads - 1, 1);
}
