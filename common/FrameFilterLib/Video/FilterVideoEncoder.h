#pragma once

#include "FrameFilterLib/Base/AbstractThreadedFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "std/cpplib/VS_MediaFormat.h"

#include <atomic>
#include <chrono>
#include <memory>

class VS_VideoCodecManager;

namespace ffl {
	class FilterVideoEncoder : public AbstractThreadedFilter<AbstractSingleSourceSink>
	{
	public:
		static std::shared_ptr<FilterVideoEncoder> Create(const std::shared_ptr<AbstractSource>& src, unsigned long fourcc, uint32_t svc_stream_mode, bool slide_sharing, std::chrono::steady_clock::duration key_frame_interval = std::chrono::seconds(10));

		FilterVideoEncoder(unsigned long fourcc, uint32_t svc_stream_mode, bool slide_sharing, std::chrono::steady_clock::duration key_frame_interval);
		~FilterVideoEncoder();
		unsigned long GetFourCC() const { return m_codec_fourcc; }
		bool GetSvcStream() const { return m_svc_stream_mode != 0; }
		bool GetSlideSharing() const { return m_slide_sharing; }

		void SetBitrate(unsigned int bitrate);

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;
		bool ProcessCommand(const FilterCommand& cmd) override;

	private:
		void UpdateCodec();

		const unsigned long m_codec_fourcc;
		const uint32_t m_svc_stream_mode;
		const bool m_slide_sharing;
		std::chrono::steady_clock::duration m_key_frame_interval;
		VS_MediaFormat m_mf_in;

		std::atomic<unsigned> m_set_bitrate;
		unsigned int m_last_bitrate;
		std::unique_ptr<VS_VideoCodecManager> m_codec;
		std::chrono::steady_clock::time_point m_last_key_frame_time;
	};
}