#include "VS_VideoCodecThreaded.h"
#include "std-generic/compat/memory.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/VS_MediaFormat.h"
#include "std/cpplib/VS_VideoLevelCaps.h"
#include "Transcoder/VS_RetriveVideoCodec.h"
#include "streams/Protocol.h"
#include "VideoBuffer.h"

VS_VideoCodecThreaded::VS_VideoCodecThreaded() : balancing_module::BalancingModule(balancing_module::Type::compressor)
{

}

VS_VideoCodecThreaded::~VS_VideoCodecThreaded()
{
	Release();
}

uint32_t VS_VideoCodecThreaded::GetScalableMode() const
{
	return m_scalableMode.load();
}

bool VS_VideoCodecThreaded::Init(const VS_MediaFormat &mf)
{
	Release();
	do {
		std::lock_guard<std::mutex> lock(m_mtxEncode);
		m_codec.reset(VS_RetriveVideoCodec(mf, true));
		if (!m_codec) {
			break;
		}
		if (m_codec->Init(mf.dwVideoWidht, mf.dwVideoHeight) != 0) {
			break;
		}
		uint32_t svcMode(mf.dwSVCMode);
		if (svcMode != 0xffffffff) {
			svcMode &= 0x0000ffff;
		}
		if (!m_codec->SetSVCMode(svcMode)) {
			svcMode = 0;
		}
		m_scalableMode.store(svcMode);
		m_init.store(true);
		m_encodeThread = std::thread(
			[this, w = mf.dwVideoWidht, h = mf.dwVideoHeight, fourcc = mf.dwVideoCodecFCC, svc = (svcMode != 0 && svcMode != 0xffffffff)]() -> void
			{
				char type[5];
				type[0] = fourcc; type[1] = fourcc >> 8; type[2] = fourcc >> 16; type[3] = fourcc >> 24; type[4] = '\0';
				std::string name = "Enc" + std::to_string(h) + "p" + std::string(type);
				vs::SetThreadName(name.c_str());
				bool pseudo(VS_GetTypeDevice(m_codec.get()) == load_balancing::BalancingDevice::software);
				RegisterThread(balancing_module::Thread::encoder);
				if (pseudo) {
					RegisterPseudoThreads(balancing_module::Thread::encoder, m_codec->GetNumThreads());
				}
				EncodeRoutine(w, h, fourcc, svc);
				UnregisterPseudoThreads(balancing_module::Thread::encoder);
				UnregisterThread();
			}
		);
	} while (false);
	if (!m_init.load()) {
		Release();
	}
	return m_init.load();
}

void VS_VideoCodecThreaded::EncodeRoutine(uint32_t w, uint32_t h, uint32_t fourcc, bool svc)
{
	std::shared_ptr<media_synch::VideoBuffer> raw;
	auto encoded = vs::make_unique_default_init<uint8_t[]>(w * h * 3 / 2);
	while (true) {
		m_checkData.wait();
		if (!m_init.load()) {
			return;
		}
		bool key(false);
		{
			BeginWorkThread(balancing_module::Thread::encoder);
			VS_SCOPE_EXIT{ EndWorkThread(balancing_module::Thread::encoder); };
			uint32_t tm(0);
			{
				std::lock_guard<std::mutex> lock(m_mtxSwap);
				if (!m_srcFrame) {
					continue;
				}
				tm = m_timestamp;
				key = m_key;
				m_key = false;
				raw.swap(m_srcFrame);
				m_srcFrame.reset();
			}
			uint8_t tl(0);
			int32_t size = Encode(raw->buffer, encoded.get() + (svc ? sizeof(stream::SVCHeader) + sizeof(int32_t) : 0), key, tl);
			raw.reset();
			if (size > 0) {
				SendData(encoded.get(), size, svc, key, tl, tm, fourcc);
			}
		}
	}
}

int32_t VS_VideoCodecThreaded::Encode(uint8_t *raw, uint8_t *encoded, bool &key, uint8_t &tl)
{
	VS_VideoCodecParam prm;
	prm.cmp.Quality = 0;
	prm.cmp.KeyFrame = key ? 1 : 0;
	int size = m_codec->Convert(raw, encoded, &prm);
	key = (prm.cmp.IsKeyFrame > 0);
	tl = static_cast<uint8_t>(prm.cmp.Quality & 0x000000ff);
	return size;
}

void VS_VideoCodecThreaded::SendData(uint8_t *encoded, int32_t size, bool svc, bool key, uint8_t tl, uint64_t tm, uint32_t fourcc)
{
	if (size > 0) {
		if (svc) {
			*(int32_t*) encoded = size + sizeof(stream::SVCHeader);
			auto h = reinterpret_cast<stream::SVCHeader*>(encoded + sizeof(int32_t));
			h->spatial = 0;
			h->quality = 0;
			h->temporal = tl;
			h->maxspatial = 0;
			size += sizeof(int32_t) + sizeof(stream::SVCHeader);
		}
		m_firePushFrameQueue(encoded, size, key, tm, fourcc);
	}
}

void VS_VideoCodecThreaded::Release()
{
	if (m_encodeThread.joinable()) {
		m_init.store(false);
		m_checkData.set();
		m_encodeThread.join();
	}
	{
		std::lock_guard<std::mutex> lock(m_mtxEncode);
		m_codec.reset();
		m_scalableMode.store(0);
	}
}

bool VS_VideoCodecThreaded::ConfigAsync(int32_t bitrate, int32_t framerate)
{
	std::lock_guard<std::mutex> lock(m_mtxEncode);
	if (!m_init.load()) {
		return false;
	}
	return m_codec->SetBitrate(bitrate * 10 * 100 / framerate);
}

void VS_VideoCodecThreaded::ConvertAsync(std::shared_ptr<media_synch::VideoBuffer> in, bool key, uint32_t timestamp)
{
	std::lock_guard<std::mutex> lock(m_mtxEncode);
	if (m_init.load() && in) {
		{
			std::lock_guard<std::mutex> lock(m_mtxSwap);
			m_srcFrame = in;
			if (key) {
				m_key = true;
			}
			m_timestamp = timestamp;
		}
		m_checkData.set();
	}
}
