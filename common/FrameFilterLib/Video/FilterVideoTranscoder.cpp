#include "FrameFilterLib/Video/FilterVideoTranscoder.h"
#include "FrameFilterLib/Base/FilterCommand.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "IppLib2/VSVideoProcessingIpp.h"
#include "std-generic/cpplib/AtomicCache.h"
#include "std-generic/cpplib/scope_exit.h"
#include "Transcoder/VS_RetriveVideoCodec.h"
#include "Transcoder/VS_SysBenchmarkBase.h"
#include "Transcoder/VS_VideoCodecManager.h"

#include <algorithm>

namespace ffl {

	vs::AtomicCache<unsigned char*, std::default_delete<unsigned char[]>, 128> s_video_transcoder_raw_cache(0);
	vs::AtomicCache<unsigned char*, std::default_delete<unsigned char[]>, 64> s_video_transcoder_cmp_cache(0);

	std::shared_ptr<FilterVideoTranscoder> FilterVideoTranscoder::Create(const std::shared_ptr<AbstractSource>& src, const std::vector<VS_MediaFormat>& formats, bool allow_upscale, const VS_MediaFormat& default_format, uint32_t svc_stream_mode, bool slide_sharing)
	{
		if (!src)
			return nullptr;
		auto transcoder = std::make_shared<FilterVideoTranscoder>(src, formats, allow_upscale, default_format, svc_stream_mode, slide_sharing);
		load_balancing::RegisterBalancingModule(transcoder);
		return src->RegisterSinkOrGetCompatible(transcoder);
	}

	FilterVideoTranscoder::FilterVideoTranscoder(const std::shared_ptr<AbstractSource>& src, const std::vector<VS_MediaFormat>& formats, bool allow_upscale, const VS_MediaFormat& default_format, uint32_t svc_stream_mode, bool slide_sharing)
		: AbstractThreadedFilter(std::chrono::milliseconds(500))
		, balancing_module::BalancingModule(balancing_module::Type::transcoder)
		, m_formats(formats)
		, m_allow_upscale(allow_upscale)
		, m_svc_stream_mode(svc_stream_mode)
		, m_slide_sharing(slide_sharing)
		, m_last_bitrate(0)
		, m_set_bitrate(0)
		, m_key_frame_interval(std::chrono::seconds(10))
	{
		SetName("video transcoder");
		m_mf_in.SetZero();
		m_mf_out.SetZero();
		m_mf_out.SetVideo(default_format.dwVideoWidht, default_format.dwVideoHeight, default_format.dwVideoCodecFCC, default_format.dwFps, default_format.dwStereo, m_svc_stream_mode);
		m_mf_out_requested.SetZero();
		s_video_transcoder_cmp_cache.IncreaseSize(1);
		s_video_transcoder_raw_cache.IncreaseSize(2);
	}

	FilterVideoTranscoder::~FilterVideoTranscoder()
	{
		s_video_transcoder_raw_cache.DecreaseSize(2);
		s_video_transcoder_cmp_cache.DecreaseSize(1);
	}

	auto FilterVideoTranscoder::ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_mf_out_requested.IsVideoValid_WithoutMultiplicity8())
		{
			if (ChangeOutputFormat(m_mf_out_requested))
				UpdateTranscoder(false);
			m_mf_out_requested.SetZero();
		}
		if (md.track == stream::Track::command)
			return e_lastBuffer; // forward commands
		if (md.track != stream::Track::video)
			return e_noResult; // drop non-video data
		if (!m_mf_out.IsVideoValid_WithoutMultiplicity8() || !m_mf_in.IsVideoValid_WithoutMultiplicity8())
			return e_noResult; // we are not initialized yet

		if (m_decoder) {
			BeginWorkThread(balancing_module::Thread::decoder);
			VS_SCOPE_EXIT{ EndWorkThread(balancing_module::Thread::decoder); };
			if (!DecodeVideo(buffer))
				return e_noResult;
		}

		if (m_processing) {
			BeginWorkThread(balancing_module::Thread::resampler);
			VS_SCOPE_EXIT{ EndWorkThread(balancing_module::Thread::resampler); };
			ProcessVideo(buffer);
		}

		if (m_encoder) {
			BeginWorkThread(balancing_module::Thread::encoder);
			VS_SCOPE_EXIT{ EndWorkThread(balancing_module::Thread::encoder); };
			if (!EncodeVideo(buffer, md.keyframe, md.interval))
				return e_noResult;
		}

		return e_lastBuffer;
	}

	bool FilterVideoTranscoder::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterVideoTranscoder*>(sink);
		if (!p)
			return false;
		if (m_formats != p->m_formats)
			return false;
		if (m_allow_upscale != p->m_allow_upscale)
			return false;
		if (m_svc_stream_mode != p->m_svc_stream_mode)
			return false;
		if (m_slide_sharing != p->m_slide_sharing)
			return false;
		return true;
	}

	bool FilterVideoTranscoder::ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (format.type != FilterFormat::e_mf)
			return false; // can't work with non-media format

		if (m_mf_in.VideoEq(format.mf))
			return true; // video input format didn't change, nothing to do
		m_mf_in.SetVideo(format.mf.dwVideoWidht, format.mf.dwVideoHeight, format.mf.dwVideoCodecFCC);

		VS_MediaFormat new_mf_out;
		new_mf_out.SetZero();
		if (SelectOutputFormat(new_mf_out))
			ChangeOutputFormat(new_mf_out);
		UpdateTranscoder(true);
		return true;
	}

	bool FilterVideoTranscoder::SelectOutputFormat(VS_MediaFormat& new_mf_out)
	{
		auto mf_it(m_formats.end());
		// Now select most suitable output format from m_formats:
		// 1. Format with same codec as input format and same resolution (equal or wildcard (==0))
		if (mf_it == m_formats.end())
			mf_it = std::find_if(m_formats.begin(), m_formats.end(), [&](const VS_MediaFormat& x) {
				return x.dwVideoCodecFCC == m_mf_in.dwVideoCodecFCC
					&& (x.dwVideoWidht == m_mf_in.dwVideoWidht || x.dwVideoWidht == 0)
					&& (x.dwVideoHeight == m_mf_in.dwVideoHeight || x.dwVideoHeight == 0);
			});
		// 2. Format with same resolution as input format (to avoid resampling)
		if (mf_it == m_formats.end())
			mf_it = std::find_if(m_formats.begin(), m_formats.end(), [&](const VS_MediaFormat& x) {
				return x.dwVideoWidht == m_mf_in.dwVideoWidht
				    && x.dwVideoHeight == m_mf_in.dwVideoHeight;
			});
		// 3. Same format as current output format (to minimize changes on receiver)
		if (mf_it == m_formats.end())
			mf_it = std::find_if(m_formats.begin(), m_formats.end(), [&](const VS_MediaFormat& x) {
				return x.VideoEq(m_mf_out);
			});
		// 4. Format with same codec as input format
		if (mf_it == m_formats.end())
			mf_it = std::find_if(m_formats.begin(), m_formats.end(), [&](const VS_MediaFormat& x) {
				return x.dwVideoCodecFCC == m_mf_in.dwVideoCodecFCC;
		});
		// 5. Any format
		if (mf_it == m_formats.end())
			mf_it = m_formats.begin();

		if (mf_it == m_formats.end())
			return false; // no suitable output format found
		new_mf_out.SetVideo(
			mf_it->dwVideoWidht != 0 ? mf_it->dwVideoWidht : m_mf_in.dwVideoWidht,
			mf_it->dwVideoHeight != 0 ? mf_it->dwVideoHeight : m_mf_in.dwVideoHeight,
			mf_it->dwVideoCodecFCC,
			mf_it->dwFps != 0 ? mf_it->dwFps : m_mf_in.dwFps
		);
		new_mf_out.dwSVCMode = m_svc_stream_mode;
		return true;
	}

	bool FilterVideoTranscoder::ChangeOutputFormat(const VS_MediaFormat& new_mf_out)
	{
		// If we are receiving different format, or desired FPS has changed then request desired format from source.
		if (!m_mf_in.VideoEq(new_mf_out) || m_mf_out.dwFps != new_mf_out.dwFps || m_mf_out.dwSVCMode != new_mf_out.dwSVCMode)
		{
			assert(new_mf_out.dwAudioCodecTag == 0);
			SendCommandToSources(FilterCommand::MakeChangeFormatRequest(new_mf_out));
		}

		if (m_mf_out.VideoEq(new_mf_out) && new_mf_out.dwSVCMode == m_mf_out.dwSVCMode)
		{
			m_mf_out.dwFps = new_mf_out.dwFps;
			return false;
		}

		m_mf_out.SetVideo(new_mf_out.dwVideoWidht, new_mf_out.dwVideoHeight, new_mf_out.dwVideoCodecFCC, new_mf_out.dwFps, 0, new_mf_out.dwSVCMode);
		return true;
	}

	void FilterVideoTranscoder::UpdateTranscoder(bool input_format_changed)
	{
		if (!m_mf_in.IsVideoValid_WithoutMultiplicity8() || !m_mf_out.IsVideoValid_WithoutMultiplicity8())
			return;

		if (!m_allow_upscale)
		{
			m_mf_out.dwVideoWidht = std::min(m_mf_out.dwVideoWidht, m_mf_in.dwVideoWidht);
			m_mf_out.dwVideoHeight = std::min(m_mf_out.dwVideoHeight, m_mf_in.dwVideoHeight);
		}
		{
			load_balancing::BalancingDevice device(load_balancing::BalancingDevice::software);
			/// decoder
			UnregisterPseudoThreads(balancing_module::Thread::decoder);
			if (m_mf_in.dwVideoCodecFCC != FOURCC_I420 && (!m_mf_in.VideoEq(m_mf_out) || m_mf_out.dwSVCMode != 0x0)) {
				// Recreate the decoder only if the input format was changed, otherwise use the existing decoder to not lose its internal state.
				const bool reuse_decoder = m_decoder && !input_format_changed;
				if (!reuse_decoder) {
					m_decoder = std::unique_ptr<VideoCodec>(VS_RetriveVideoCodec(m_mf_in, false));
					if (!m_decoder) {
						m_mf_in.SetZero();
						return;
					}
				}
				assert(m_decoder);
				device = VS_GetTypeDevice(m_decoder.get());
				// Update the decoder if it was just created or when it depends on the output format (e.g. nvidia)
				if (!reuse_decoder || device == load_balancing::BalancingDevice::nvidia) {
					base_Param settings;
					settings.color_space = FOURCC_I420;
					settings.device_memory = false;
					settings.width = m_mf_in.dwVideoWidht;
					settings.height = m_mf_in.dwVideoHeight;
					settings.out_width = m_mf_out.dwVideoWidht;
					settings.out_height = m_mf_out.dwVideoHeight;
					if (m_decoder->InitExtended(settings) != 0) {
						m_mf_in.SetZero();
						return;
					}
				}
				RegisterPseudoThreads(balancing_module::Thread::decoder, 1);
			}
			else {
				m_decoder.reset();
			}
			/// encoder
			UnregisterPseudoThreads(balancing_module::Thread::encoder);
			if (m_mf_out.dwSVCMode != 0x0 || (m_mf_out.dwVideoCodecFCC != FOURCC_I420 && !m_mf_out.VideoEq(m_mf_in))) {
				m_encoder = std::make_unique<VS_VideoCodecManager>();
				m_videointeval.Init(120);
			}
			else {
				m_encoder.reset();
			}
			if (m_encoder) {
				if (!UpdateEncoder()) {
					m_mf_out.SetZero();
					return;
				}
				RegisterPseudoThreads(balancing_module::Thread::encoder, m_encoder->GetNumThreads());
			}
			/// processing
			UnregisterPseudoThreads(balancing_module::Thread::resampler);
			if (device != load_balancing::BalancingDevice::nvidia && (m_mf_in.dwVideoWidht != m_mf_out.dwVideoWidht || m_mf_in.dwVideoHeight != m_mf_out.dwVideoHeight)) {
				m_processing = std::make_unique<VSVideoProcessingIpp>();
				RegisterPseudoThreads(balancing_module::Thread::resampler, 1);
			}
			else {
				m_processing.reset();
			}
			if (m_processing) {
				UpdateProcessing();
			}

			if (m_decoder || m_processing || m_encoder)
			{
				assert(m_decoder || m_mf_in.dwVideoCodecFCC == FOURCC_I420);
				assert(m_encoder || m_mf_out.dwVideoCodecFCC == FOURCC_I420);
			}
		}
		SetFormat(FilterFormat::MakeMF(m_mf_out));
	}

	bool FilterVideoTranscoder::UpdateProcessing()
	{
		m_rsmpOffsetH = m_rsmpOffsetW = 0;
		m_kw = m_kh = 0.0;
		m_factor = 1.0;
		int drawW = m_mf_out.dwVideoWidht;
		int drawH = m_mf_out.dwVideoHeight;
		int rsmpW = drawW;
		int rsmpH = drawH;
		m_kw = (double) m_mf_in.dwVideoWidht / (double) drawW;
		m_kh = (double) m_mf_in.dwVideoHeight / (double) drawH;
		if (m_kw > m_kh) {
			int nh = (int) (((drawH * m_kh) / m_kw + 0.5));
			int d0 = drawH - nh;
			int d1 = nh - (nh &~1);
			if (d1 < d0) {
				rsmpH = nh - d1;
			}
			m_rsmpOffsetH = ((drawH - rsmpH)&~3) / 2;
			m_factor = 1.0 / m_kw;
		}
		else if (m_kh > m_kw) {
			int nw = (int) (((drawW * m_kw) / m_kh + 0.5));
			int d0 = drawW - nw;
			int d1 = nw - (nw &~1);
			if (d1 < d0) {
				rsmpW = nw - d1;
			}
			m_rsmpOffsetW = ((drawW - rsmpW)&~3) / 2;
			m_factor = 1.0 / m_kh;
		}
		else {
			m_rsmpOffsetH = m_rsmpOffsetW = 0;
			m_factor = 1.0 / m_kw;
		}
		return true;
	}

	bool FilterVideoTranscoder::UpdateEncoder()
	{
		unsigned char snd_lvl = VS_SysBenchmarkBase().GetSndLevel(ENCODER_SOFTWARE, m_mf_out.dwVideoCodecFCC);
		VS_MediaFormat mf = m_mf_out;
		mf.dwHWCodec = m_slide_sharing ? ENCODER_SLIDES : ENCODER_SOFTWARE;
		if (!m_encoder->Init(&mf, snd_lvl)) {
			return false;
		}
		if (m_last_bitrate > 0) {
			SetBitrate(m_last_bitrate);
		}
		m_last_key_frame_time -= m_key_frame_interval * 2;
		return true;
	}

	void FilterVideoTranscoder::SetBitrate(unsigned int bitrate)
	{
		m_set_bitrate = bitrate;
	}

	bool FilterVideoTranscoder::DecodeVideo(vs::SharedBuffer& buffer)
	{
		struct return_to_cache { void operator()(unsigned char* x) const { s_video_transcoder_raw_cache.Put(x); }};
		std::unique_ptr<unsigned char, return_to_cache> memory_block(s_video_transcoder_raw_cache.Get());
		if (!memory_block) {
			memory_block.reset(new unsigned char[1920 * 1080 * 3 / 2]);
		}
		uint32_t w = (m_processing) ? m_mf_in.dwVideoWidht : m_mf_out.dwVideoWidht;
		uint32_t h = (m_processing) ? m_mf_in.dwVideoHeight : m_mf_out.dwVideoHeight;
		vs::SharedBuffer out_buffer(std::move(memory_block), w * h * 3 / 2);
		VS_VideoCodecParam prm;
		prm.dec.Flags = 0;
		prm.dec.FrameSize = buffer.size();
		int ret = m_decoder->Convert(const_cast<uint8_t*>(buffer.data<const uint8_t>()), out_buffer.data<uint8_t>(), &prm);
		if (ret <= 0) {
			if (auto log = m_log.lock()) {
				log->TraceMessage(this, "decode error");
			}
			// Don't log the command since we just logged the reason of the key frame request.
			SendCommandToSources(FilterCommand::MakeKeyFrameRequest(), false);
			return false;
		}
		out_buffer.shrink(0, ret);
		buffer = std::move(out_buffer);
		return true;
	}

	void FilterVideoTranscoder::ProcessVideo(vs::SharedBuffer& buffer)
	{
		struct return_to_cache { void operator()(unsigned char* x) const { s_video_transcoder_raw_cache.Put(x); }};
		std::unique_ptr<unsigned char, return_to_cache> memory_block(s_video_transcoder_raw_cache.Get());
		if (!memory_block) {
			memory_block.reset(new unsigned char[1920 * 1080 * 3 / 2]);
		}
		auto out_buffer = vs::SharedBuffer(std::move(memory_block), m_mf_out.dwVideoWidht * m_mf_out.dwVideoHeight * 3 / 2);

		uint8_t* srcPlanes[3];
		uint8_t* dstPlanes[3];

		srcPlanes[0] = const_cast<uint8_t*>(buffer.data<const uint8_t>());
		srcPlanes[1] = srcPlanes[0] + m_mf_in.dwVideoWidht * m_mf_in.dwVideoHeight;
		srcPlanes[2] = srcPlanes[0] + m_mf_in.dwVideoWidht * m_mf_in.dwVideoHeight * 5 / 4;

		dstPlanes[0] = out_buffer.data<uint8_t>();
		dstPlanes[1] = dstPlanes[0] + m_mf_out.dwVideoWidht * m_mf_out.dwVideoHeight;
		dstPlanes[2] = dstPlanes[0] + m_mf_out.dwVideoWidht * m_mf_out.dwVideoHeight * 5 / 4;

		if (m_kw == m_kh) {
			m_processing->ResampleCropI420(srcPlanes, dstPlanes,
											m_mf_in.dwVideoWidht, m_mf_in.dwVideoHeight, m_mf_in.dwVideoWidht,
										    m_mf_out.dwVideoWidht, m_mf_out.dwVideoHeight, m_mf_out.dwVideoWidht,
											m_mf_in.dwVideoWidht, m_mf_in.dwVideoHeight,
											0, 0, m_factor, m_factor, IPPI_INTER_LINEAR); // linear
		}
		else {
			m_processing->ResampleInscribedI420(srcPlanes, dstPlanes,
												m_mf_in.dwVideoWidht, m_mf_in.dwVideoHeight, m_mf_in.dwVideoWidht,
												m_mf_out.dwVideoWidht, m_mf_out.dwVideoHeight, m_mf_out.dwVideoWidht,
												m_rsmpOffsetW, m_rsmpOffsetH, m_factor, m_factor, IPPI_INTER_LINEAR); // linear
		}

		buffer = std::move(out_buffer);
	}

	bool FilterVideoTranscoder::EncodeVideo(vs::SharedBuffer& buffer, bool &key, unsigned long interval)
	{
		double averageInterval(1000.0 / 30.0);
		m_videointeval.Snap(static_cast<double>(std::min<unsigned long>(interval, 1000)));
		m_videointeval.GetAverage(averageInterval);
		unsigned int set_bitrate = 0;
		set_bitrate = m_set_bitrate.exchange(set_bitrate);
		if (set_bitrate == 0 && m_recalcBitrate) {
			set_bitrate = m_last_bitrate;
		}
		if (set_bitrate != 0) {
			double fps(1000.0 / averageInterval);
			if (auto log = m_log.lock()) {
				std::string msg;
				msg.reserve(64);
				msg.append("set bitrate: ").append(std::to_string(set_bitrate)).append(", fps = ").append(std::to_string(static_cast<int32_t>(fps)));
				log->TraceMessage(this, msg);
			}
			m_last_bitrate = set_bitrate;
			m_encoder->SetBitrate(set_bitrate, 10000, static_cast<int32_t>(fps * 100));
		}

		constexpr size_t out_size = 500 * 1024;
		struct return_to_cache { void operator()(unsigned char* x) const { s_video_transcoder_cmp_cache.Put(x); }};
		std::unique_ptr<unsigned char, return_to_cache> memory_block(s_video_transcoder_cmp_cache.Get());
		if (!memory_block) {
			memory_block.reset(new unsigned char[out_size]);
		}
		vs::SharedBuffer out_buffer(std::move(memory_block), out_size);

		bool key_frame(std::chrono::steady_clock::now() - m_last_key_frame_time > m_key_frame_interval);
		m_recalcBitrate = key_frame;
		key = key_frame;
		int ret = m_encoder->Convert(const_cast<uint8_t*>(buffer.data<const uint8_t>()), out_buffer.data<uint8_t>(), &key);
		if (ret < 0) {
			if (auto log = m_log.lock()) {
				log->TraceMessage(this, "encode error");
			}
			return false;
		}
		else if (ret == 0) {
			return false;
		}
		out_buffer.shrink(0, ret);
		buffer = std::move(out_buffer);
		if (key_frame) {
			m_last_key_frame_time = std::chrono::steady_clock::now();
		}
		return true;
	}

	bool FilterVideoTranscoder::ProcessCommand(const FilterCommand& cmd)
	{
		if (AbstractSource::ProcessCommand(cmd))
			return true;

		std::lock_guard<std::mutex> lock(m_mutex);
		if (cmd.type == FilterCommand::e_changeFormatRequest)
		{
			auto mf = cmd.mf;
			if (!mf.dwVideoCodecFCC) mf.dwVideoCodecFCC = m_mf_out_requested.dwVideoCodecFCC;
			if (!mf.dwVideoCodecFCC) mf.dwVideoCodecFCC = m_mf_out.dwVideoCodecFCC;

			if (!mf.IsVideoValid_WithoutMultiplicity8())
				return false; // invalid format, can't change to it
			m_mf_out_requested.SetVideo(mf.dwVideoWidht, mf.dwVideoHeight, mf.dwVideoCodecFCC, mf.dwFps, 0, m_svc_stream_mode);
			return true;
		}
		else if (cmd.type == FilterCommand::e_setBitrateRequest)
		{
			// Consume command only when encoder is present.
			if (m_encoder)
			{
				SetBitrate(cmd.bitrate);
				return true;
			}
			else
			{
				// Save the last bitrate value to use it in case we need to create the encoder.
				m_last_bitrate = cmd.bitrate;
			}
		}
		else if (cmd.type == FilterCommand::e_keyFrameRequest)
		{
			// Consume command only when encoder is present.
			if (m_encoder)
			{
				m_last_key_frame_time -= m_key_frame_interval * 2;
				return true;
			}
		}

		return SendCommandToSources(cmd, false);
	}
}
