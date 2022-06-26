#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractMultiSourceSink.h"
#include "std/cpplib/VS_MediaFormat.h"

#include <chrono>
#include <mutex>
#include <vector>

namespace ffl {
	class FilterAudioDecoder;
	class FilterAudioAdjuster;
	class FilterAudioEncoder;

	class FilterAudioTranscoder : public AbstractFilter<AbstractMultiSourceSink>
	{
	public:
		static std::shared_ptr<FilterAudioTranscoder> Create(const std::shared_ptr<AbstractSource>& src, const std::vector<VS_MediaFormat>& formats, const VS_MediaFormat& default_format);

		FilterAudioTranscoder(const std::shared_ptr<AbstractSource>& src, const std::vector<VS_MediaFormat>& formats, const VS_MediaFormat& default_format);

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		e_processingResult GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;
		void Detach() override;
		bool ProcessCommand(const FilterCommand& cmd) override;

		virtual bool SelectOutputFormat(VS_MediaFormat& new_mf_out);
		bool ChangeOutputFormat(const VS_MediaFormat& new_mf_out, bool force = false);

	private:
		void UpdateChain();

	protected:
		std::mutex m_mutex;
		std::shared_ptr<AbstractSource> m_src;
		std::shared_ptr<AbstractSource> m_transcoded_src;
		std::weak_ptr<FilterAudioDecoder> m_decoder;
		std::weak_ptr<FilterAudioAdjuster> m_adjuster;
		std::weak_ptr<FilterAudioEncoder> m_encoder;

		std::vector<VS_MediaFormat> m_formats;
		VS_MediaFormat m_mf_in;
		VS_MediaFormat m_mf_out;

	private:
		VS_MediaFormat m_mf_out_requested;

		VS_MediaFormat m_mf_out_pending;
		std::chrono::steady_clock::time_point m_last_cmd_time;
	};
}
