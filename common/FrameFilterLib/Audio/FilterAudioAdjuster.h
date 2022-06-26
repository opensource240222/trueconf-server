#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"
#include "std/cpplib/VS_MediaFormat.h"
#include "Transcoder/VS_FfmpegResampler.h"

#include <memory>

namespace ffl {
	class FilterAudioAdjuster : public AbstractFilter<AbstractSingleSourceSink>
	{
	public:
		static std::shared_ptr<FilterAudioAdjuster> Create(const std::shared_ptr<AbstractSource>& src, unsigned sample_rate);

		explicit FilterAudioAdjuster(unsigned sample_rate);
		unsigned GetSampleRate() const { return m_sample_rate; }

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

	private:
		void UpdateAdjuster();

		const unsigned m_sample_rate;
		VS_MediaFormat m_mf_in;
		VS_FfmpegResampler m_aresmp;
		std::unique_ptr<unsigned char[]> m_tmp_buffer;
	};
}