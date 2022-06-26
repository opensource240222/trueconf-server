#pragma once

#include "FrameFilterLib/Base/AbstractFilter.h"
#include "FrameFilterLib/Base/AbstractSingleSourceSink.h"

#include <boost/circular_buffer.hpp>

#include <chrono>
#include <mutex>

namespace ffl {
	class FilterStatisticsCalculator : public AbstractFilter<AbstractSingleSourceSink>
	{
	public:
		static const unsigned c_average_seconds = 8;

		static std::shared_ptr<FilterStatisticsCalculator> Create(const std::shared_ptr<AbstractSource>& src);

		FilterStatisticsCalculator();

		void ResetData();
		void GetAudioBitrate(float* avg, unsigned* min = nullptr, unsigned* max = nullptr, unsigned seconds = c_average_seconds);
		void GetVideoBitrate(float* avg, unsigned* min = nullptr, unsigned* max = nullptr, unsigned seconds = c_average_seconds);
		void GetVideoFPS(float* avg, unsigned* min = nullptr, unsigned* max = nullptr, unsigned seconds = c_average_seconds);
		VS_MediaFormat GetMediaFormat() const;

		bool IsCompatibleWith(const AbstractSink* sink) override;

	protected:
		e_processingResult ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) override;
		bool ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format) override;

	private:
		void ExpireSlices();

		mutable std::mutex m_mutex;
		std::chrono::steady_clock::time_point m_last_slice_start;
		struct slice_stat
		{
			unsigned audio_size;
			unsigned video_size;
			unsigned video_frames;
		};
		boost::circular_buffer<slice_stat> m_slices;
		VS_MediaFormat m_mf;
	};
}
