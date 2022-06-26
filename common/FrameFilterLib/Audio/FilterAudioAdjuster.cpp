#include "FrameFilterLib/Audio/FilterAudioAdjuster.h"
#include "FrameFilterLib/Base/TraceLog.h"

#include "std-generic/compat/memory.h"

namespace ffl {
	std::shared_ptr<FilterAudioAdjuster> FilterAudioAdjuster::Create(const std::shared_ptr<AbstractSource>& src, unsigned sample_rate)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterAudioAdjuster>(sample_rate));
	}

	FilterAudioAdjuster::FilterAudioAdjuster(unsigned sample_rate)
		: m_sample_rate(sample_rate)
		, m_tmp_buffer(vs::make_unique_default_init<unsigned char[]>(1024*1024))
	{
		SetName("audio adjuster");
		m_mf_in.SetZero();
	}

	auto FilterAudioAdjuster::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (md.track == stream::Track::command)
			return e_lastBuffer; // forward commands
		if (md.track != stream::Track::audio)
			return e_noResult; // drop non-audio data

		if (m_sample_rate == m_mf_in.dwAudioSampleRate)
			return e_lastBuffer;

		int res = m_aresmp.Process(buffer.data<const void>(), m_tmp_buffer.get(), buffer.size(), m_mf_in.dwAudioSampleRate, m_sample_rate);
		if (res <= 0)
			return e_noResult;
		if (buffer.size() >= static_cast<size_t>(res) && buffer.exclusive())
			buffer.shrink(0, res);
		else
			buffer = vs::SharedBuffer(res);
		std::memcpy(buffer.data(), m_tmp_buffer.get(), res);
		return e_lastBuffer;
	}

	bool FilterAudioAdjuster::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterAudioAdjuster*>(sink);
		if (!p)
			return false;
		if (m_sample_rate != p->m_sample_rate)
			return false;
		return true;
	}

	bool FilterAudioAdjuster::ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format)
	{
		if (format.type != FilterFormat::e_mf)
			return false; // can't work with non-media format
		if (format.mf.dwAudioCodecTag != VS_ACODEC_PCM)
			return false; // can only work with raw audio

		if (m_mf_in.AudioEq(format.mf))
			return true; // format didn't change, nothing to do
		m_mf_in.SetAudio(format.mf.dwAudioSampleRate, format.mf.dwAudioCodecTag);
		UpdateAdjuster();
		return true;
	}

	void FilterAudioAdjuster::UpdateAdjuster()
	{
		SetFormat(FilterFormat::MakeAudio(VS_ACODEC_PCM, m_sample_rate));
	}
}