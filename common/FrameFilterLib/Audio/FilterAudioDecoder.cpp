#include "FrameFilterLib/Audio/FilterAudioDecoder.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "Transcoder/AudioCodec.h"

namespace ffl {
	std::shared_ptr<FilterAudioDecoder> FilterAudioDecoder::Create(const std::shared_ptr<AbstractSource>& src)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterAudioDecoder>());
	}

	FilterAudioDecoder::FilterAudioDecoder() :AbstractThreadedFilter(std::chrono::milliseconds(500))
	{
		SetName("audio decoder");
		m_mf_in.SetZero();
	}

	auto FilterAudioDecoder::ProcessFrame(const std::shared_ptr<AbstractSource>& /*src*/, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (md.track == stream::Track::command)
			return e_lastBuffer; // forward commands
		if (md.track != stream::Track::audio)
			return e_noResult; // drop non-audio data
		if (m_mf_in.IsAudioValid() && m_mf_in.dwAudioCodecTag == VS_ACODEC_PCM)
			return e_lastBuffer; // forward raw audio
		if (!m_codec)
			return e_noResult; // we are not initialized yet

		vs::SharedBuffer out_buffer(m_mf_in.dwAudioSampleRate / 2);
		int ret = m_codec->Convert(const_cast<uint8_t*>(buffer.data<const uint8_t>()), out_buffer.data<uint8_t>(), buffer.size());
		if (ret < 0)
		{
			if (auto log = m_log.lock())
				log->TraceMessage(this, "decode error");
			return e_noResult;
		}
		else if (ret == 0)
			return e_noResult;
		out_buffer.shrink(0, ret);
		buffer = std::move(out_buffer);
		return e_lastBuffer;
	}

	void FilterAudioDecoder::UpdateCodec()
	{
		m_codec.reset(VS_RetriveAudioCodec(m_mf_in.dwAudioCodecTag, false));
		if (!m_codec)
		{
			m_mf_in.SetZero();
			return;
		}
		WAVEFORMATEX wf;
		wf.nChannels = 1;
		wf.nSamplesPerSec = m_mf_in.dwAudioSampleRate;
		wf.wBitsPerSample = 16;
		if (0 != m_codec->Init(&wf))
		{
			m_mf_in.SetZero();
			return;
		}
		SetFormat(FilterFormat::MakeAudio(VS_ACODEC_PCM, m_mf_in.dwAudioSampleRate));
	}

	bool FilterAudioDecoder::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterAudioDecoder*>(sink);
		if (!p)
			return false;
		return true;
	}

	bool FilterAudioDecoder::ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format)
	{
		if (format.type != FilterFormat::e_mf)
			return false; // can't work with non-media format

		if (m_mf_in.AudioEq(format.mf))
			return true; // format didn't change, nothing to do
		m_mf_in.SetAudio(format.mf.dwAudioSampleRate, format.mf.dwAudioCodecTag);
		if (m_mf_in.dwAudioCodecTag != VS_ACODEC_PCM)
			UpdateCodec();
		else
			SetFormat(FilterFormat::MakeMF(m_mf_in));
		return true;
	}

}