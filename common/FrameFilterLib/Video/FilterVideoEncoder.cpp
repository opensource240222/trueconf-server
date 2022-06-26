#include "FrameFilterLib/Video/FilterVideoEncoder.h"
#include "FrameFilterLib/Base/FilterCommand.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "Transcoder/VS_SysBenchmarkBase.h"
#include "Transcoder/VS_VideoCodecManager.h"
#include "std-generic/cpplib/AtomicCache.h"

namespace ffl {

	vs::AtomicCache<unsigned char*, std::default_delete<unsigned char[]>, 32> s_video_enc_memory_cache(0);

	std::shared_ptr<FilterVideoEncoder> FilterVideoEncoder::Create(const std::shared_ptr<AbstractSource>& src, unsigned long fourcc, uint32_t svc_stream_mode, bool slide_sharing, std::chrono::steady_clock::duration key_frame_interval)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterVideoEncoder>(fourcc, svc_stream_mode, slide_sharing, key_frame_interval));
	}

	FilterVideoEncoder::FilterVideoEncoder(unsigned long fourcc, uint32_t svc_stream_mode, bool slide_sharing, std::chrono::steady_clock::duration key_frame_interval)
		: AbstractThreadedFilter(std::chrono::milliseconds(500))
		, m_codec_fourcc(fourcc)
		, m_svc_stream_mode(svc_stream_mode)
		, m_slide_sharing(slide_sharing)
		, m_key_frame_interval(key_frame_interval)
		, m_set_bitrate(0)
		, m_last_bitrate(0)
	{
		SetName("video encoder");
		m_mf_in.SetZero();

		s_video_enc_memory_cache.IncreaseSize(1);
	}

	FilterVideoEncoder::~FilterVideoEncoder()
	{
		s_video_enc_memory_cache.DecreaseSize(1);
	}

	void FilterVideoEncoder::SetBitrate(unsigned int bitrate)
	{
		m_set_bitrate = bitrate;
	}

	auto FilterVideoEncoder::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (md.track == stream::Track::command)
			return e_lastBuffer; // forward commands
		if (md.track != stream::Track::video)
			return e_noResult; // drop non-video data

		if (!m_codec)
			return e_noResult; // we are not initialized yet

		unsigned int set_bitrate = 0;
		set_bitrate = m_set_bitrate.exchange(set_bitrate);
		if (set_bitrate != 0)
		{
			if (auto log = m_log.lock())
			{
				std::string msg;
				msg.reserve(64);
				msg.append("set bitrate: ").append(std::to_string(set_bitrate));
				log->TraceMessage(this, msg);
			}
			m_last_bitrate = set_bitrate;
			float fps(30.0f); // FIXME: recalc real source framerate
			m_codec->SetBitrate(set_bitrate, 10000, static_cast<int32_t>(fps * 100));
		}

		constexpr size_t out_size = 500 * 1024;
		struct return_to_cache { void operator()(unsigned char* x) const { s_video_enc_memory_cache.Put(x); }};
		std::unique_ptr<unsigned char, return_to_cache> memory_block(s_video_enc_memory_cache.Get());
		if (!memory_block)
			memory_block.reset(new unsigned char[out_size]);
		vs::SharedBuffer out_buffer(std::move(memory_block), out_size);

		bool key_frame(std::chrono::steady_clock::now() - m_last_key_frame_time > m_key_frame_interval);
		md.keyframe = key_frame;
		int ret = m_codec->Convert(const_cast<uint8_t*>(buffer.data<const uint8_t>()), out_buffer.data<uint8_t>(), &md.keyframe);
		if (ret < 0)
		{
			if (auto log = m_log.lock())
				log->TraceMessage(this, "encode error");
			return e_noResult;
		}
		else if (ret == 0)
			return e_noResult;
		out_buffer.shrink(0, ret);
		buffer = std::move(out_buffer);
		if (key_frame)
			m_last_key_frame_time = std::chrono::steady_clock::now();
		return e_lastBuffer;
	}

	bool FilterVideoEncoder::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterVideoEncoder*>(sink);
		if (!p)
			return false;
		if (m_codec_fourcc != p->m_codec_fourcc)
			return false;
		if (GetSvcStream() != p->GetSvcStream())
			return false;
		return true;
	}

	bool FilterVideoEncoder::ProcessFormat(const std::shared_ptr<AbstractSource>& /*src*/, const FilterFormat& format)
	{
		if (format.type != FilterFormat::e_mf)
			return false; // can't work with non-media format
		if (format.mf.dwVideoCodecFCC != FOURCC_I420)
			return false; // can only work with raw video

		if (m_mf_in.VideoEq(format.mf))
			return true; // format didn't change, nothing to do
		m_mf_in.SetVideo(format.mf.dwVideoWidht, format.mf.dwVideoHeight, format.mf.dwVideoCodecFCC);
		UpdateCodec();
		return true;
	}

	void FilterVideoEncoder::UpdateCodec()
	{
		unsigned char snd_lvl = VS_SysBenchmarkBase().GetSndLevel(ENCODER_SOFTWARE, m_codec_fourcc);

		m_codec = std::make_unique<VS_VideoCodecManager>();
		if (!m_codec)
		{
			m_mf_in.SetZero();
			return;
		}
		VS_MediaFormat mf = m_mf_in;
		mf.dwSVCMode = m_svc_stream_mode;
		mf.dwVideoCodecFCC = m_codec_fourcc;
		mf.dwHWCodec = m_slide_sharing ? ENCODER_SLIDES : ENCODER_SOFTWARE;
		if (!m_codec->Init(&mf, snd_lvl)) {
			m_mf_in.SetZero();
			return;
		}
		if(m_last_bitrate > 0)
			SetBitrate(m_last_bitrate);
		m_last_key_frame_time -= m_key_frame_interval*2;
		SetFormat(FilterFormat::MakeVideo(m_codec_fourcc, m_mf_in.dwVideoWidht, m_mf_in.dwVideoHeight));
	}

	bool FilterVideoEncoder::ProcessCommand(const FilterCommand& cmd)
	{
		if (AbstractSource::ProcessCommand(cmd))
			return true;

		if      (cmd.type == FilterCommand::e_keyFrameRequest)
		{
			m_last_key_frame_time -= m_key_frame_interval*2;
			return true;
		}
		else if (cmd.type == FilterCommand::e_setBitrateRequest)
		{
			SetBitrate(cmd.bitrate);
			return true;
		}
		return SendCommandToSources(cmd, false);
	}
}