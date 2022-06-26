#include "FrameFilterLib/Utility/FilterStatisticsCalculator.h"

namespace ffl {
	std::shared_ptr<FilterStatisticsCalculator> FilterStatisticsCalculator::Create(const std::shared_ptr<AbstractSource>& src)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterStatisticsCalculator>());
	}

	FilterStatisticsCalculator::FilterStatisticsCalculator()
		: m_slices(1 + c_average_seconds)
	{
		SetName("stat calc");
		m_mf.SetZero();
	}

	void FilterStatisticsCalculator::ResetData()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_last_slice_start = std::chrono::steady_clock::time_point();
		m_slices.clear();
	}

	void FilterStatisticsCalculator::GetAudioBitrate(float* avg, unsigned* min, unsigned* max, unsigned seconds)
	{
		if (seconds == 0 || seconds > c_average_seconds)
			seconds = c_average_seconds;

		std::lock_guard<std::mutex> lock(m_mutex);
		ExpireSlices();
		if (m_slices.size() <= 1)
		{
			// We don't have any completed slices (first slice is the current slice and isn't completed yet).
			if (avg) *avg = 0.0;
			if (min) *min = 0;
			if (max) *max = 0;
			return;
		}

		const size_t last_i = std::min(seconds, static_cast<unsigned>(m_slices.size() - 1));
		if (avg)
		{
			*avg = 0.0;
			for (size_t i = 1; i <= last_i; ++i)
				*avg += m_slices[i].audio_size; // size / duration, duration==1
			*avg /= last_i; // last - first + 1, first==1
			*avg /= 128; // byte/s => Kbit/s
		}
		if (min)
		{
			*min = m_slices[1].audio_size;
			for (size_t i = 2; i <= last_i; ++i)
				*min = std::min(*min, m_slices[i].audio_size);
			*min /= 128; // byte/s => Kbit/s
		}
		if (max)
		{
			*max = m_slices[1].audio_size;
			for (size_t i = 2; i <= last_i; ++i)
				*max = std::max(*max, m_slices[i].audio_size);
			*max /= 128; // byte/s => Kbit/s
		}
	}

	void FilterStatisticsCalculator::GetVideoBitrate(float* avg, unsigned* min, unsigned* max, unsigned seconds)
	{
		if (seconds == 0 || seconds > c_average_seconds)
			seconds = c_average_seconds;

		std::lock_guard<std::mutex> lock(m_mutex);
		ExpireSlices();
		if (m_slices.size() <= 1)
		{
			// We don't have any completed slices (first slice is the current slice and isn't completed yet).
			if (avg) *avg = 0.0;
			if (min) *min = 0;
			if (max) *max = 0;
			return;
		}

		const size_t last_i = std::min(seconds, static_cast<unsigned>(m_slices.size() - 1));
		if (avg)
		{
			*avg = 0.0;
			for (size_t i = 1; i <= last_i; ++i)
				*avg += m_slices[i].video_size; // size / duration, duration==1
			*avg /= last_i; // last - first + 1, first==1
			*avg /= 128; // byte/s => Kbit/s
		}
		if (min)
		{
			*min = m_slices[1].video_size;
			for (size_t i = 2; i <= last_i; ++i)
				*min = std::min(*min, m_slices[i].video_size);
			*min /= 128; // byte/s => Kbit/s
		}
		if (max)
		{
			*max = m_slices[1].video_size;
			for (size_t i = 2; i <= last_i; ++i)
				*max = std::max(*max, m_slices[i].video_size);
			*max /= 128; // byte/s => Kbit/s
		}
	}

	void FilterStatisticsCalculator::GetVideoFPS(float* avg, unsigned* min, unsigned* max, unsigned seconds)
	{
		if (seconds == 0 || seconds > c_average_seconds)
			seconds = c_average_seconds;

		std::lock_guard<std::mutex> lock(m_mutex);
		ExpireSlices();
		if (m_slices.size() <= 1)
		{
			// We don't have any completed slices (first slice is the current slice and isn't completed yet).
			if (avg) *avg = 0.0;
			if (min) *min = 0;
			if (max) *max = 0;
			return;
		}

		const size_t last_i = std::min(seconds, static_cast<unsigned>(m_slices.size() - 1));
		if (avg)
		{
			*avg = 0.0;
			for (size_t i = 1; i <= last_i; ++i)
				*avg += m_slices[i].video_frames; // frames / duration, duration==1
			*avg /= last_i; // last - first + 1, first==1
		}
		if (min)
		{
			*min = m_slices[1].video_frames;
			for (size_t i = 2; i <= last_i; ++i)
				*min = std::min(*min, m_slices[i].video_frames);
		}
		if (max)
		{
			*max = m_slices[1].video_frames;
			for (size_t i = 2; i <= last_i; ++i)
				*max = std::max(*max, m_slices[i].video_frames);
		}
	}

	VS_MediaFormat FilterStatisticsCalculator::GetMediaFormat() const
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_mf;
	}

	auto FilterStatisticsCalculator::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		ExpireSlices();

		assert(!m_slices.empty());
		switch (md.track)
		{
		case stream::Track::audio:
			m_slices[0].audio_size += buffer.size();
			break;
		case stream::Track::video:
			m_slices[0].video_size += buffer.size();
			m_slices[0].video_frames++;
			break;

		}

		return e_lastBuffer;
	}

	bool FilterStatisticsCalculator::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterStatisticsCalculator*>(sink);
		if (!p)
			return false;
		return true;
	}

	bool FilterStatisticsCalculator::ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (format.type == FilterFormat::e_mf)
				m_mf = format.mf;
			else
				m_mf.SetZero();
		}
		return AbstractFilter::ProcessFormat(src, format);
	}

	void FilterStatisticsCalculator::ExpireSlices()
	{
		auto now = std::chrono::steady_clock::now();
		if (now - m_last_slice_start > std::chrono::seconds(static_cast<unsigned>(ffl::FilterStatisticsCalculator::c_average_seconds)))
		{
			// More than c_average_seconds have passed since last slice, which means all slice have expired.
			m_last_slice_start = now;
			m_slices.clear();
			m_slices.push_front(slice_stat{ 0, 0, 0 });
		}
		else
		{
			// Insert missing (empty) statistics for missing slices.
			while (now - m_last_slice_start > std::chrono::seconds(1))
			{
				m_last_slice_start += std::chrono::seconds(1);
				m_slices.push_front(slice_stat{ 0, 0, 0 });
			}
		}
	}
}
