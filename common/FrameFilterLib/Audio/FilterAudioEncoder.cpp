#include "FrameFilterLib/Audio/FilterAudioEncoder.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "Transcoder/AudioCodec.h"

namespace ffl {
	std::shared_ptr<FilterAudioEncoder> FilterAudioEncoder::Create(const std::shared_ptr<AbstractSource>& src, unsigned codec_tag)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterAudioEncoder>(codec_tag));
	}

	FilterAudioEncoder::FilterAudioEncoder(unsigned codec_tag) :AbstractThreadedFilter(std::chrono::milliseconds(500)), m_codec_tag(codec_tag)
	{
		SetName("audio encoder");
		m_mf_in.SetZero();
	}

	auto FilterAudioEncoder::ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (md.track == stream::Track::command)
			return e_lastBuffer; // forward commands
		if (md.track != stream::Track::audio)
			return e_noResult; // drop non-audio data

		if (!m_codec)
			return e_noResult; // we are not initialized yet

		m_in_buffer = std::move(buffer);
		return GetNextBuffer(buffer, md);
	}

	auto FilterAudioEncoder::GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& /*md*/) -> e_processingResult
	{
		if (m_in_buffer.empty())
			return e_noResult;

		const size_t chunk_size = std::min<size_t>(m_in_buffer.size(), GetFormat().mf.dwAudioBufferLen);
		assert(chunk_size > 0);
		buffer = vs::SharedBuffer(4096);
		int ret = m_codec->Convert(const_cast<uint8_t*>(m_in_buffer.data<const uint8_t>()), buffer.data<uint8_t>(), chunk_size);
		if (ret < 0)
		{
			if (auto log = m_log.lock())
				log->TraceMessage(this, "encode error");
			m_in_buffer = vs::SharedBuffer();
			return e_noResult;
		}
		else if (ret == 0)
		{
			m_in_buffer = vs::SharedBuffer();
			return e_noResult;
		}
		buffer.shrink(0, ret);
		m_in_buffer.shrink(chunk_size, m_in_buffer.size() - chunk_size);
		return m_in_buffer.empty() ? e_lastBuffer : e_moreBuffers;
	}

	bool FilterAudioEncoder::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterAudioEncoder*>(sink);
		if (!p)
			return false;
		if (m_codec_tag != p->m_codec_tag)
			return false;
		return true;
	}

	bool FilterAudioEncoder::ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format)
	{
		if (format.type != FilterFormat::e_mf)
			return false; // can't work with non-media format
		if (format.mf.dwAudioCodecTag != VS_ACODEC_PCM)
			return false; // can only work with raw audio

		if (m_mf_in.AudioEq(format.mf))
			return true; // format didn't change, nothing to do
		m_mf_in.SetAudio(format.mf.dwAudioSampleRate, format.mf.dwAudioCodecTag);
		UpdateCodec();
		return true;
	}

	void FilterAudioEncoder::UpdateCodec()
	{
		m_codec.reset(VS_RetriveAudioCodec(m_codec_tag, true));
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
		SetFormat(FilterFormat::MakeAudio(m_codec_tag, m_mf_in.dwAudioSampleRate));
	}
}