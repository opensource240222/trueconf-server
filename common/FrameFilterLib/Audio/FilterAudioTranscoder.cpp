#include "FrameFilterLib/Audio/FilterAudioTranscoder.h"
#include "FrameFilterLib/Base/FilterCommand.h"
#include "FrameFilterLib/Audio/FilterAudioAdjuster.h"
#include "FrameFilterLib/Audio/FilterAudioDecoder.h"
#include "FrameFilterLib/Audio/FilterAudioEncoder.h"
#include "FrameFilterLib/Audio/FilterDumpAudio.h"
#include "streams/Command.h"

#include <algorithm>

namespace ffl {
	const std::chrono::steady_clock::duration c_cmd_send_interval = std::chrono::seconds(1);

	std::shared_ptr<FilterAudioTranscoder> FilterAudioTranscoder::Create(const std::shared_ptr<AbstractSource>& src, const std::vector<VS_MediaFormat>& formats, const VS_MediaFormat& default_format)
	{
		if (!src)
			return nullptr;
		return src->RegisterSinkOrGetCompatible(std::make_shared<FilterAudioTranscoder>(src, formats, default_format));
	}

	FilterAudioTranscoder::FilterAudioTranscoder(const std::shared_ptr<AbstractSource>& src, const std::vector<VS_MediaFormat>& formats, const VS_MediaFormat& default_format)
		: m_src(src)
		, m_formats(formats)
	{
		SetName("audio transcoder");
		m_mf_in.SetZero();
		m_mf_out.SetZero();
		m_mf_out.SetAudio(default_format.dwAudioSampleRate, default_format.dwAudioCodecTag);
		m_mf_out_requested.SetZero();
		m_mf_out_pending.SetZero();
	}

	auto FilterAudioTranscoder::ProcessFrame(const std::shared_ptr<AbstractSource>& src, vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_mf_out_requested.IsAudioValid())
		{
			if (ChangeOutputFormat(m_mf_out_requested, true))
				UpdateChain();
			m_mf_out_requested.SetZero();
		}
		if (m_transcoded_src && src != m_transcoded_src)
			return e_noResult; // ignore input from anything other than current transcoding chain if we are transcoding
		if (!m_transcoded_src && src != m_src)
			return e_noResult; // ignore input from anything other than original source if we are not transcoding
		if (md.track == stream::Track::command)
			return e_moreBuffers; // forward commands
		if (md.track != stream::Track::audio)
			return GetNextBuffer(buffer, md); // drop non-audio data
		if (!m_mf_out.IsAudioValid())
			return GetNextBuffer(buffer, md); // we are not initialized yet

		return e_moreBuffers;
	}

	auto FilterAudioTranscoder::GetNextBuffer(vs::SharedBuffer& buffer, FrameMetadata& md) -> e_processingResult
	{
		if (m_mf_out_pending.dwAudioCodecTag != 0 && std::chrono::steady_clock::now() - m_last_cmd_time > c_cmd_send_interval)
		{
			auto cmd = std::make_shared<stream::Command>();
			cmd->ChangeRcvMFormat(m_mf_out_pending);
			buffer = vs::SharedBuffer(cmd, cmd->Size());
			md = FrameMetadata::MakeCommand();
			m_last_cmd_time = std::chrono::steady_clock::now();
			return e_lastBuffer;
		}
		return e_noResult;
	}

	bool FilterAudioTranscoder::IsCompatibleWith(const AbstractSink* sink)
	{
		auto p = dynamic_cast<const FilterAudioTranscoder*>(sink);
		if (!p)
			return false;
		if (m_formats != p->m_formats)
			return false;
		return true;
	}

	bool FilterAudioTranscoder::ProcessFormat(const std::shared_ptr<AbstractSource>& src, const FilterFormat& format)
	{
		if (src != m_src)
			return true; // ignore format changes from transcoding chain
		std::lock_guard<std::mutex> lock(m_mutex);
		if (format.type != FilterFormat::e_mf)
			return false; // can't work with non-media format

		if (m_mf_in.AudioEq(format.mf))
			return true; // audio input format didn't change, nothing to do
		m_mf_in.SetAudio(format.mf.dwAudioSampleRate, format.mf.dwAudioCodecTag);

		VS_MediaFormat new_mf_out;
		new_mf_out.SetZero();
		if (SelectOutputFormat(new_mf_out))
			ChangeOutputFormat(new_mf_out);
		UpdateChain();
		return true;
	}

	bool FilterAudioTranscoder::SelectOutputFormat(VS_MediaFormat& new_mf_out)
	{
		auto mf_it(m_formats.end());
		// Now select most suitable output format from m_formats:
		// 1. Format with same codec as input format and same sample rate (equal or wildcard (==0))
		if (mf_it == m_formats.end())
			mf_it = std::find_if(m_formats.begin(), m_formats.end(), [&](const VS_MediaFormat& x) {
				return x.dwAudioCodecTag == m_mf_in.dwAudioCodecTag
					&& (x.dwAudioSampleRate == m_mf_in.dwAudioSampleRate || x.dwAudioSampleRate == 0);
			});
		// 2. Format with same sample rate as input format (to avoid resampling)
		if (mf_it == m_formats.end())
			mf_it = std::find_if(m_formats.begin(), m_formats.end(), [&](const VS_MediaFormat& x) {
				return x.dwAudioSampleRate == m_mf_in.dwAudioSampleRate;
			});
		// 3. Same format as current output format (to minimize changes on receiver)
		if (mf_it == m_formats.end())
			mf_it = std::find_if(m_formats.begin(), m_formats.end(), [&](const VS_MediaFormat& x) {
				return x.AudioEq(m_mf_out);
			});
		// 4. Format with same codec as input format
		if (mf_it == m_formats.end())
			mf_it = std::find_if(m_formats.begin(), m_formats.end(), [&](const VS_MediaFormat& x) {
				return x.dwAudioCodecTag == m_mf_in.dwAudioCodecTag;
		});
		// 5. Any format
		if (mf_it == m_formats.end())
			mf_it = m_formats.begin();

		if (mf_it == m_formats.end())
			return false; // no suitable output format found
		new_mf_out.SetAudio(mf_it->dwAudioSampleRate != 0 ? mf_it->dwAudioSampleRate : m_mf_in.dwAudioSampleRate, mf_it->dwAudioCodecTag);
		return true;
	}

	bool FilterAudioTranscoder::ChangeOutputFormat(const VS_MediaFormat& new_mf_out, bool force)
	{
		if (m_mf_out.AudioEq(new_mf_out))
			return false;

		if (force)
		{
			m_mf_out.SetAudio(m_mf_out_requested.dwAudioSampleRate, m_mf_out_requested.dwAudioCodecTag);
			return true;
		}
		else
		{
			m_mf_out_pending.SetAudio(new_mf_out.dwAudioSampleRate, new_mf_out.dwAudioCodecTag);
			m_last_cmd_time = std::chrono::steady_clock::now() - c_cmd_send_interval*2;
			return false;
		}
	}

	void FilterAudioTranscoder::UpdateChain()
	{
		if (!m_mf_in.IsAudioValid() || !m_mf_out.IsAudioValid())
			return;

		if (m_mf_in.AudioEq(m_mf_out))
		{
			if (m_transcoded_src)
			{
				m_transcoded_src->UnregisterSink(shared_from_this());
				m_transcoded_src.reset();
			}
		}
		else
		{
			assert(m_mf_out.dwVideoCodecFCC == 0);
			m_src->ProcessCommand(FilterCommand::MakeChangeFormatRequest(m_mf_out));

			std::shared_ptr<AbstractSource> head = m_src;

			auto decoder = m_decoder.lock();
			if (decoder)
				decoder = head->RegisterSinkOrGetCompatible(decoder);
			else
				decoder = FilterAudioDecoder::Create(head);
			m_decoder = decoder;
			head = decoder;
			head = FilterDumpAudio::Create(head, "adec-out");

			if (m_mf_in.dwAudioSampleRate != m_mf_out.dwAudioSampleRate)
			{
				auto adjuster = m_adjuster.lock();
				if (adjuster && adjuster->GetSampleRate() == m_mf_out.dwAudioSampleRate)
					adjuster = head->RegisterSinkOrGetCompatible(adjuster);
				else
					adjuster = FilterAudioAdjuster::Create(head, m_mf_out.dwAudioSampleRate);
				m_adjuster = adjuster;
				head = adjuster;
				head = FilterDumpAudio::Create(head, "aadj-out");
			}

			auto encoder = m_encoder.lock();
			if (encoder && encoder->GetCodec() == m_mf_out.dwAudioCodecTag)
				encoder = head->RegisterSinkOrGetCompatible(encoder);
			else
				encoder = FilterAudioEncoder::Create(head, m_mf_out.dwAudioCodecTag);
			m_encoder = encoder;
			head = encoder;
			head = FilterDumpAudio::Create(head, "aenc-out");

			head->RegisterSinkOrGetCompatible(shared_from_this());
			m_transcoded_src = head;
		}
		SetFormat(FilterFormat::MakeMF(m_mf_out));
	}

	void FilterAudioTranscoder::Detach()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_transcoded_src.reset();
		AbstractFilter::Detach();
	}

	bool FilterAudioTranscoder::ProcessCommand(const FilterCommand& cmd)
	{
		if (AbstractSource::ProcessCommand(cmd))
			return true;

		std::lock_guard<std::mutex> lock(m_mutex);
		if (cmd.type == FilterCommand::e_changeFormatRequest)
		{
			if (!cmd.mf.IsAudioValid())
				return false; // invalid format, can't change to it
			m_mf_out_pending.SetZero();
			m_mf_out_requested.SetAudio(cmd.mf.dwAudioSampleRate, cmd.mf.dwAudioCodecTag);
			return true;
		}

		if (m_transcoded_src)
			return m_transcoded_src->ProcessCommand(cmd);
		return m_src->ProcessCommand(cmd);
	}
}
