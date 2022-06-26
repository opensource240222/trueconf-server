#include "FrameFilterLib/Video/FilterVideoDecoder.h"
#include "FrameFilterLib/Base/FilterCommand.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "Transcoder/VideoCodec.h"
#include "Transcoder/VS_RetriveVideoCodec.h"
#include "std-generic/cpplib/AtomicCache.h"

namespace ffl {

	vs::AtomicCache<unsigned char*, std::default_delete<unsigned char[]>, 128> s_video_dec_memory_cache(0); // 128 buffers = 64 simultaneous SIP * (1 decoder buffer + 1 adjuster buffer)

	std::shared_ptr<FilterVideoDecoder> FilterVideoDecoder::Create(const std::shared_ptr<AbstractSource>& src)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterVideoDecoder>());
	}

	FilterVideoDecoder::FilterVideoDecoder()
		: AbstractThreadedFilter(std::chrono::milliseconds(500))
	{
		SetName("video decoder");
		m_mf_in.SetZero();

		s_video_dec_memory_cache.IncreaseSize(1);
	}

	FilterVideoDecoder::~FilterVideoDecoder()
	{
		s_video_dec_memory_cache.DecreaseSize(1);
	}

	auto FilterVideoDecoder::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (md.track == stream::Track::command)
			return e_lastBuffer; // forward commands
		if (md.track != stream::Track::video)
			return e_noResult; // drop non-video data
		if (m_mf_in.IsVideoValid_WithoutMultiplicity8() && m_mf_in.dwVideoCodecFCC == FOURCC_I420)
			return e_lastBuffer; // forward raw video
		if (!m_codec)
			return e_noResult; // we are not initialized yet

		struct return_to_cache { void operator()(unsigned char* x) const { s_video_dec_memory_cache.Put(x); }};
		std::unique_ptr<unsigned char, return_to_cache> memory_block(s_video_dec_memory_cache.Get());
		if (!memory_block)
			memory_block.reset(new unsigned char[1920 * 1080 * 3 / 2]);
		vs::SharedBuffer out_buffer(std::move(memory_block), m_mf_in.dwVideoWidht * m_mf_in.dwVideoHeight * 3 / 2);

		VS_VideoCodecParam prm;
		prm.dec.Flags = 0;
		prm.dec.FrameSize = buffer.size();
		int ret = m_codec->Convert(const_cast<uint8_t*>(buffer.data<const uint8_t>()), out_buffer.data<uint8_t>(), &prm);
		if (ret <= 0)
		{
			if (auto log = m_log.lock())
				log->TraceMessage(this, "decode error");
			// Don't log the command since we just logged the reason of the key frame request.
			SendCommandToSources(FilterCommand::MakeKeyFrameRequest(), false);
			return e_noResult;
		}
		out_buffer.shrink(0, ret);
		buffer = std::move(out_buffer);
		md.keyframe = true;
		return e_lastBuffer;
	}

	bool FilterVideoDecoder::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterVideoDecoder*>(sink);
		if (!p)
			return false;
		return true;
	}

	bool FilterVideoDecoder::ProcessFormat(const std::shared_ptr<AbstractSource>& /*src*/, const FilterFormat& format)
	{
		if (format.type != FilterFormat::e_mf)
			return false; // can't work with non-media format

		if (m_mf_in.VideoEq(format.mf))
			return true; // format didn't change, nothing to do
		m_mf_in.SetVideo(format.mf.dwVideoWidht, format.mf.dwVideoHeight, format.mf.dwVideoCodecFCC);
		if (m_mf_in.dwVideoCodecFCC != FOURCC_I420)
			UpdateCodec();
		else
			SetFormat(FilterFormat::MakeMF(m_mf_in));
		return true;
	}

	void FilterVideoDecoder::UpdateCodec()
	{
		m_codec.reset();
		m_codec = std::unique_ptr<VideoCodec>(VS_RetriveVideoCodec(m_mf_in, false));
		if (!m_codec)
		{
			m_mf_in.SetZero();
			return;
		}
		if (0 != m_codec->Init(m_mf_in.dwVideoWidht, m_mf_in.dwVideoHeight, FOURCC_I420))
		{
			m_mf_in.SetZero();
			return;
		}
		SetFormat(FilterFormat::MakeVideo(FOURCC_I420, m_mf_in.dwVideoWidht, m_mf_in.dwVideoHeight));
	}
}