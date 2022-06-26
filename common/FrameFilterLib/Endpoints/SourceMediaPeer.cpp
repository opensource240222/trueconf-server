#include "FrameFilterLib/Endpoints/SourceMediaPeer.h"
#include "FrameFilterLib/Base/FilterCommand.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "std/cpplib/VS_Singleton.h"
#include "std/cpplib/VS_ThreadPool.h"
#include "TransceiverCircuit/VS_RelayMediaSource.h"

#include <cassert>

namespace ffl {
	const char* ACodecToPayloadType(unsigned codec)
	{
		switch (codec)
		{
		case VS_ACODEC_PCM: return "L16";
		case VS_ACODEC_G711a: return "PCMA";
		case VS_ACODEC_G711mu: return "PCMU";
		case VS_ACODEC_GSM610: return "GSM";
		case VS_ACODEC_G723: return "G723";
		case VS_ACODEC_G728: return "G728";
		case VS_ACODEC_G729A: return "G729";
		case VS_ACODEC_G722: return "G722";
		case VS_ACODEC_G7221_24: return "G7221_24";
		case VS_ACODEC_G7221_32: return "G7221_32";
		case VS_ACODEC_SPEEX: return "speex";
		case VS_ACODEC_ISAC: return "ISAC";
		case VS_ACODEC_G7221C_24: return "G7221C_24";
		case VS_ACODEC_G7221C_32: return "G7221C_32";
		case VS_ACODEC_G7221C_48: return "G7221C_48";
		case VS_ACODEC_OPUS_B0914: return "opus";
		case VS_ACODEC_MP3: return "MP3";
		default: return nullptr;
		}
	}

	std::shared_ptr<SourceMediaPeer> SourceMediaPeer::Create(const char* peer_id, const char* part_id, const std::shared_ptr<VS_RelayMediaSource>& media_source)
	{
		auto peer = std::make_shared<SourceMediaPeer>(peer_id, part_id, media_source);
		media_source->PeerConnect(peer);
		return peer;
	}

	SourceMediaPeer::SourceMediaPeer(const char* peer_id, const char* part_id, const std::shared_ptr<VS_RelayMediaSource>& media_source)
		: VS_MediaPeerBase(peer_id, part_id)
		, m_media_source(media_source)
		, m_force_video_format(false)
		, m_last_timestamp(0)
	{
		m_type = vs_media_peer_type_mcu;
		SetName("media peer source");
		VS_MediaFormat mf;
		mf.SetZero();
		media_source->GetPeerMediaFormat(&mf, m_type);
		assert(mf.IsAudioValid());
		assert(mf.IsVideoValid_WithoutMultiplicity8());
		SetFormat(FilterFormat::MakeMF(mf));
	}

	SourceMediaPeer::~SourceMediaPeer()
	{
	}

	bool SourceMediaPeer::ProcessCommand(const FilterCommand& cmd)
	{
		if (AbstractSource::ProcessCommand(cmd))
			return true;

		// Check if our media source is alive.
		const auto media_source = m_media_source.lock();
		if (!media_source)
			return false;

		assert(GetFormat().type == FilterFormat::e_mf);
		switch (cmd.type)
		{
		case FilterCommand::e_keyFrameRequest:
			if (!m_observer)
				return false;
			if (GetFormat().mf.dwVideoCodecFCC == 0)
				return false;
			m_observer->KeyFrameRequestForPeer(m_part_id.c_str(), m_peer_id.c_str());
			return true;
		case FilterCommand::e_changeFormatRequest:
		{
			VS_MediaFormat mf(GetFormat().mf);
			if (mf.AudioEq(cmd.mf) && mf.VideoEq(cmd.mf))
				return true; // same format requested, nothing to do

			bool changed(false);
			if (cmd.mf.IsAudioValid() && !mf.AudioEq(cmd.mf) && m_observer)
			{
				const char* payload(ACodecToPayloadType(cmd.mf.dwAudioCodecTag));
				if (payload)
				{
					m_observer->ChangeAudioSendPayload(m_part_id.c_str(), m_peer_id.c_str(), payload, cmd.mf.dwAudioSampleRate);
					mf.SetAudio(cmd.mf.dwAudioSampleRate, cmd.mf.dwAudioCodecTag);
					changed = true;
				}
			}
			if (cmd.mf.IsVideoValid_WithoutMultiplicity8() && !mf.VideoEq(cmd.mf) && m_observer)
			{
				if ((cmd.mf.dwVideoCodecFCC == VS_VCODEC_VPX || (cmd.mf.dwVideoCodecFCC == VS_VCODEC_H264 && m_part_id.empty())) // Mixer is providing video for specific participant only in format it arrived, for now this means VP8.
				)
				{
					// Changing video format from thread owned by VS_VideoCodecThreadManager may result in a deadlock.
					// To avoid this we move request to other thread.
					VS_Singleton<VS_ThreadPool>::Instance().Post([this, mf_v = cmd.mf, self = shared_from_this()]()
					{
						if (!m_observer)
							return;
						VS_MediaFormat out_mf;
						m_observer->ChangeVideoSendPayload(m_part_id.c_str(), m_peer_id.c_str(), mf_v, out_mf, m_force_video_format);
					});
					mf.SetVideo(cmd.mf.dwVideoWidht, cmd.mf.dwVideoHeight, cmd.mf.dwVideoCodecFCC);
					changed = true;
				}
				else
				{
					if (auto log = m_log.lock())
						log->TraceMessage(this, "video format change request ignored");
				}
			}
			if (!changed)
				return false;
			SetFormat(FilterFormat::MakeMF(mf));
			return true;
		}
		case FilterCommand::e_setBitrateRequest:
			if (!m_observer)
				return false;
			if (GetFormat().mf.dwVideoCodecFCC == 0)
				return false;

			m_observer->SetRates(m_part_id.c_str(), m_peer_id.c_str(), cmd.bitrate, GetFormat().mf.dwFps);

			return true;
		default:
			return false;
		}
	}

	void SourceMediaPeer::SetAudioFormat(const VS_MediaFormat& mf)
	{
		if (!m_observer)
			return;
		VS_MediaFormat cur_mf = GetFormat().mf;
		m_observer->ChangeAudioSendPayload(m_part_id.c_str(), m_peer_id.c_str(), ACodecToPayloadType(mf.dwAudioCodecTag), mf.dwAudioSampleRate);
		cur_mf.SetAudio(mf.dwAudioSampleRate, mf.dwAudioCodecTag);
		SetFormat(FilterFormat::MakeMF(cur_mf));
	}

	void SourceMediaPeer::SetVideoFormat(const VS_MediaFormat& mf, bool force)
	{
		if (!m_observer)
			return;
		m_force_video_format = force;
		VS_MediaFormat cur_mf = GetFormat().mf;
		m_observer->ChangeVideoSendPayload(m_part_id.c_str(), m_peer_id.c_str(), mf, cur_mf, m_force_video_format);
		SetFormat(FilterFormat::MakeMF(cur_mf));
	}

	void SourceMediaPeer::SetBitrate(unsigned int bitrate)
	{
		assert(GetFormat().type == FilterFormat::e_mf);
		if (GetFormat().mf.dwVideoCodecFCC == 0)
			return;
		m_observer->SetRates(m_part_id.c_str(), m_peer_id.c_str(), bitrate, GetFormat().mf.dwFps);
	}

	void SourceMediaPeer::Stop()
	{
		auto media_source = m_media_source.lock();
		if (!media_source)
			return;
		media_source->PeerDisconnect(m_peer_id.c_str());
		m_observer->ReadyToDie(m_part_id.c_str(), m_peer_id.c_str());
	}

	void SourceMediaPeer::PutVideo(unsigned char *pFrame, unsigned long size, bool isKey, unsigned timestamp)
	{
		assert(GetFormat().type == FilterFormat::e_mf);
		if (GetFormat().mf.dwVideoCodecFCC == 0)
			return;
		vs::SharedBuffer buffer(size);
		std::memcpy(buffer.data(), pFrame, buffer.size());
		uint32_t dt = timestamp >= m_last_timestamp
			? timestamp - m_last_timestamp
			: (std::numeric_limits<uint32_t>::max() - m_last_timestamp + 1) + timestamp;
		if (dt > 1000)
			dt = 1000;
		else if (dt == 0)
			dt = 1;
		m_last_timestamp = timestamp;
		SendFrameToSubscribers(std::move(buffer), FrameMetadata::MakeVideo(dt, isKey));
	}

	void SourceMediaPeer::PutCompressedAudio(unsigned char *buf, unsigned long sz, unsigned int /*samples*/, unsigned int /*timestamp*/)
	{
		assert(GetFormat().type == FilterFormat::e_mf);
		if (GetFormat().mf.dwAudioCodecTag == 0)
			return;
		if (GetFormat().mf.dwAudioCodecTag == VS_ACODEC_PCM)
			return;

		// Apparently there are 4 bytes of garbage(?) at the beginning of the buffer
		vs::SharedBuffer buffer(sz);
		std::memcpy(buffer.data(), buf + sizeof(unsigned int), buffer.size());
		SendFrameToSubscribers(std::move(buffer), FrameMetadata::MakeAudio());
	}

	void SourceMediaPeer::PutUncompressedAudio(unsigned char *buf, unsigned long sz, unsigned int /*timestamp*/)
	{
		assert(GetFormat().type == FilterFormat::e_mf);
		if (GetFormat().mf.dwAudioCodecTag == 0)
			return;
		if (GetFormat().mf.dwAudioCodecTag != VS_ACODEC_PCM)
			return;

		vs::SharedBuffer buffer(sz);
		std::memcpy(buffer.data(), buf, buffer.size());
		SendFrameToSubscribers(std::move(buffer), FrameMetadata::MakeAudio());
	}

	void SourceMediaPeer::ReceiveAudio(const char */*peer_name*/, const unsigned char */*buf*/, int /*sz*/, unsigned int /*timestamp*/)
	{
	}

	void SourceMediaPeer::ChangeAudioRcvPayload(const char */*plname*/, const int /*plfreq*/)
	{
	}

	void SourceMediaPeer::ReceiveVideo(const char */*peer_name*/, const char */*stream_id*/, const unsigned char */*pFrame*/, int /*size*/, bool /*isKey*/, unsigned int /*timestamp*/)
	{
	}
	void SourceMediaPeer::ChangeRcvFrameResolution(const char */*peer_name*/, const char */*stream_id*/, const char */*plname*/, uint8_t /*pltype*/, unsigned short /*width*/, unsigned short /*height*/)
	{
	}
}
