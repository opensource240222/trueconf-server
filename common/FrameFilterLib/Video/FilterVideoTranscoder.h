#pragma once

#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "FrameFilterLib/Base/AbstractThreadedFilter.h"
#include "std/cpplib/VS_MediaFormat.h"
#include "std/VS_TimeDeviation.h"
#include "Transcoder/LoadBalancing/BalancingModule.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <vector>

class VideoCodec;
class VS_VideoCodecManager;
class VSVideoProcessingIpp;

namespace ffl {

	class FilterVideoTranscoder : public AbstractThreadedFilter<AbstractSingleSourceSink>, public balancing_module::BalancingModule
	{
	public:
		static std::shared_ptr<FilterVideoTranscoder> Create(const std::shared_ptr<AbstractSource>& src, const std::vector<VS_MediaFormat>& formats, bool allow_upscale, const VS_MediaFormat& default_format, uint32_t svc_stream_mode, bool slide_sharing);

		FilterVideoTranscoder(const std::shared_ptr<AbstractSource>& src, const std::vector<VS_MediaFormat>& formats, bool allow_upscale, const VS_MediaFormat& default_format, uint32_t svc_stream_mode, bool slide_sharing);
		~FilterVideoTranscoder();

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;
		bool ProcessCommand(const FilterCommand& cmd) override;

		virtual bool SelectOutputFormat(VS_MediaFormat& new_mf_out);
		bool ChangeOutputFormat(const VS_MediaFormat& new_mf_out);

	private:
		void UpdateTranscoder(bool input_format_changed);
		bool UpdateProcessing();
		bool UpdateEncoder();
		void SetBitrate(unsigned int bitrate);
		bool DecodeVideo(vs::SharedBuffer& buffer);
		void ProcessVideo(vs::SharedBuffer& buffer);
		bool EncodeVideo(vs::SharedBuffer& buffer, bool &key, unsigned long interval);

	protected:
		std::mutex m_mutex;
		std::vector<VS_MediaFormat> m_formats;
		bool m_allow_upscale;
		uint32_t m_svc_stream_mode;
		bool m_slide_sharing;

	protected:
		VS_MediaFormat m_mf_in;
		VS_MediaFormat m_mf_out;
		unsigned int m_last_bitrate;
		std::atomic<unsigned> m_set_bitrate;
		std::chrono::steady_clock::duration m_key_frame_interval;
		std::chrono::steady_clock::time_point m_last_key_frame_time;

	protected:
		std::unique_ptr<VideoCodec> m_decoder;
		std::unique_ptr<VS_VideoCodecManager> m_encoder;
		std::unique_ptr<VSVideoProcessingIpp> m_processing;

	private:
		VS_MediaFormat m_mf_out_requested;
		int	m_rsmpOffsetH, m_rsmpOffsetW;
		double m_kw = 0.0, m_kh = 0.0, m_factor = 1.0;
		bool m_recalcBitrate = false;
		VS_TimeDeviation<double> m_videointeval;
	};
}
