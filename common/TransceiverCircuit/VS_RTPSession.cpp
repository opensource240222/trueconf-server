#include "VS_RTPSession.h"
#include "VS_RTPModuleParameters.h"
#include "VS_FFLSourceCollection.h"
#include "VS_MediaSourceCollection.h"
#include "VS_RelayMediaSource.h"
#include "VS_TransceiverPartsMgr.h"
#include "VS_TransceiverParticipant.h"
#include "FrameFilterLib/Audio/FilterAudioFormatReader.h"
#include "FrameFilterLib/Audio/FilterAudioTranscoder.h"
#include "FrameFilterLib/Audio/FilterDumpAudio.h"
#include "FrameFilterLib/Base/FilterCommand.h"
#include "FrameFilterLib/Base/TraceLog.h"
#include "FrameFilterLib/Endpoints/SinkRTPChannel.h"
#include "FrameFilterLib/Endpoints/SinkTransceiverParticipant.h"
#include "FrameFilterLib/Endpoints/SinkFECCChannel.h"
#include "FrameFilterLib/Endpoints/SourceMediaPeer.h"
#include "FrameFilterLib/Endpoints/SourceRTPChannel.h"
#include "FrameFilterLib/Endpoints/SourceStaticImage.h"
#include "FrameFilterLib/Endpoints/SourceTransceiverParticipant.h"
#include "FrameFilterLib/Endpoints/SourceFECCChannel.h"
#include "FrameFilterLib/Transport/FilterDumpRTP.h"
#include "FrameFilterLib/Transport/FilterRTPSorter.h"
#include "FrameFilterLib/Transport/FilterRTPUnwrapper.h"
#include "FrameFilterLib/Transport/FilterRTPValidator.h"
#include "FrameFilterLib/Transport/FilterRTPWrapper.h"
#include "FrameFilterLib/Transport/FilterVSFrameSlicer.h"
#include "FrameFilterLib/Transport/FilterVSFrameUnwrapper.h"
#include "FrameFilterLib/Transport/FilterVSFrameWrapper.h"
#include "FrameFilterLib/Utility/FilterActivityMonitor.h"
#include "FrameFilterLib/Utility/FilterAudioVideoJoiner.h"
#include "FrameFilterLib/Utility/FilterChangeFormatCommandInjector.h"
#include "FrameFilterLib/Utility/FilterKeyFrameRequestLimiter.h"
#include "FrameFilterLib/Utility/FilterKeyFrameRequester.h"
#include "FrameFilterLib/Utility/FilterStatisticsCalculator.h"
#include "FrameFilterLib/Utility/FilterUniformTransmit.h"
#include "FrameFilterLib/Video/FilterDumpVideo.h"
#include "FrameFilterLib/Video/FilterH264SpsPpsInjector.h"
#include "FrameFilterLib/Video/FilterH264StreamLayoutInjector.h"
#include "FrameFilterLib/Video/FilterVideoFormatReader.h"
#include "FrameFilterLib/Video/FilterSlideEncoder.h"
#include "FrameFilterLib/Video/FilterVideoTranscoder.h"
#include "FrameFilterLib/Video/FilterVideoTranscoderWithResolutionLimits.h"
#include "streams/Protocol.h"
#include "std/cpplib/VS_Singleton.h"
#include "std/cpplib/VS_ThreadPool.h"
#include "MediaParserLib/VS_AACParser.h"
#include "tools/H323Gateway/Lib/h224/OpalH224Handler.h"
#include "tools/SingleGatewayLib/ModeSelection.h"
#include "tools/SingleGatewayLib/VS_RTPMediaChannels.h"
#include "std/cpplib/VS_ClientCaps_io.h"
#include "std/cpplib/VS_MediaFormat.h"
#include "std/cpplib/VS_MediaFormat_io.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/json/reader.h"
#include "std/cpplib/MakeShared.h"
#include "std/debuglog/VS_Debug.h"
#include "tools/Server/VS_MediaChannelInfo.h"

#include <curl/curl.h>

#include <boost/filesystem/operations.hpp>

#include <algorithm>
#include <cassert>
#include "std-generic/compat/memory.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

std::shared_ptr<VS_RTPSessionInterface> VS_RTPSessionInterface::CreateNewSession(
	boost::asio::io_service& ios,
	string_view id,
	string_view part_id,
	string_view sess_key,
	const std::shared_ptr<VS_RTPModuleParameters>& parameters,
	const std::shared_ptr<VS_TransceiverPartsMgr>& partsMgr,
	const std::shared_ptr<VS_FFLSourceCollection>& source_collection
	)
{
	return vs::MakeShared<VS_RTPSession>(
		ios,
		id,
		part_id,
		sess_key,
		parameters,
		partsMgr,
		source_collection);
}

VS_RTPSession::VS_RTPSession(
	boost::asio::io_service& ios,
	string_view id,
	string_view part_id,
	string_view sess_key,
	const std::shared_ptr<VS_RTPModuleParameters>& parameters,
	const std::shared_ptr<VS_TransceiverPartsMgr>& partsMgr,
	const std::shared_ptr<VS_FFLSourceCollection>& source_collection
	)
	: m_id(id)
	, m_our_part_id(part_id)
	, m_conf_subtype(GCST_UNDEF)
	, m_session_key(sess_key)
	, m_recv_content(SDP_CONTENT_MAIN)
	, m_audio_channel(nullptr)
	, m_main_video_channel(nullptr)
	, m_slides_video_channel(nullptr)
	, m_device_status(0)
	, m_stopping(false)
	, m_rtp(vs::make_unique<VS_RTPMediaChannels>(ios))
	, m_parameters(parameters)
	, m_partsMgr(partsMgr)
	, m_source_collection(source_collection)
	, m_stat_log_seq_id(0)
	, m_fake_video_mode(FVM_DISABLED)
{
	dprint4("VS_RTPSession(%s): created\n", m_id.c_str());

	m_parameters->Update();
	m_vs_banner_source = ffl::SourceStaticImage::Create();
}

VS_RTPSession::~VS_RTPSession()
{
	m_trace_log.reset();
	m_trace_file_buf.close();

	dprint4("VS_RTPSession(%s): destroyed\n", m_id.c_str());
}

const std::string& VS_RTPSession::GetConferenceName() const
{
	return m_conf_name;
}

const std::string& VS_RTPSession::GetParticipantName() const
{
	return m_our_part_id;
}

bool VS_RTPSession::IsReadyToDestroy()
{
	return m_rtp->IsDisconnected();
}

void VS_RTPSession::Stop()
{
	dprint4("VS_RTPSession(%s): stopped\n", m_id.c_str());
	m_stopping = true;

	if (auto ut = m_ut_main_video.lock()) {
		dprint4("VS_RTPSession(%s): UniformTransmit for main video stats: %s\n", m_id.c_str(), ut->GetStatistics().c_str());
	}
	if (auto ut = m_ut_slides_video.lock()) {
		dprint4("VS_RTPSession(%s): UniformTransmit for slides stats: %s\n", m_id.c_str(), ut->GetStatistics().c_str());
	}

	for (const auto& x: m_rtp_sinks)
	{
		assert(x.second);
		x.second->Stop();
		x.second->SetRTPChannel(nullptr);
	}
	m_rtp_sinks.clear();

	for (const auto& x: m_rtp_sources)
	{
		assert(x.second);
		x.second->SetRTPChannel(nullptr);
	}
	m_rtp_sources.clear();

	m_rtp->DisconnectAll();

	m_source_collection->SetContentSource(m_conf_name, m_our_part_id.c_str(), nullptr);

	if (m_participant && m_partsMgr)
		m_partsMgr->FreePart(m_participant->GetConfName(), m_participant->GetPartName());

	if (m_participant_sink)
		m_participant_sink->Stop();

	if (m_conference_source)
		m_conference_source->Stop();

	if (m_fecc_sink)
		m_fecc_sink->Stop();

	m_participant_sink.reset();
	m_participant_source.reset();
	m_conference_source.reset();
	m_vs_sink.reset();
	m_vs_slides_source.reset();
	m_vs_banner_source.reset();
	m_fecc_source.reset();
	m_fecc_sink.reset();
	if (auto vs_video_unwrapper = m_vs_video_unwrapper.lock())
		vs_video_unwrapper->Detach();
	m_rtp_slides_report_connection.disconnect();
	if (auto rtp_slides_monitor = m_rtp_slides_monitor.lock())
		rtp_slides_monitor->Detach();
	if (auto rtp_slides_encoder = m_rtp_slides_encoder.lock())
		rtp_slides_encoder->Detach();
	if (auto rtp_in_audio_stat = m_rtp_in_audio_stat.lock())
		rtp_in_audio_stat->Detach();
	if (auto rtp_in_video_stat = m_rtp_in_video_stat.lock())
		rtp_in_video_stat->Detach();
	if (auto rtp_in_slides_stat = m_rtp_in_slides_stat.lock())
		rtp_in_slides_stat->Detach();
}

void VS_RTPSession::SetConference(string_view conf_name, string_view part_id, string_view owner, VS_GroupConf_SubType subtype, const VS_ClientCaps& conference_caps)
{
	dstream4 << "VS_RTPSession(" << m_id << "): SetConference: conf_name=\"" << conf_name << "\", part_id=\"" << part_id << "\", owner=\"" << owner << "\", subtype=" << subtype << ", caps:\n" << conference_caps;

	m_conf_name = std::string(conf_name);
	m_remote_part_id = std::string(part_id);
	m_conf_owner = std::string(owner);
	m_conf_subtype = subtype;
	assert(!m_remote_part_id.empty() || m_conf_subtype != GCST_UNDEF);
	m_conference_caps = conference_caps;
	CreateFilters();
}

void VS_RTPSession::SetMediaChannels(const std::vector<VS_MediaChannelInfo>& channels)
{
	dprint4("VS_RTPSession(%s): SetMediaChannels\n", m_id.c_str());
	std::set<unsigned int> channel_ids;
	for (const auto& channel: m_media_channels)
		channel_ids.insert(channel.index);
	for (const auto& channel: channels)
		channel_ids.insert(channel.index);
	for (auto id: channel_ids)
	{
		auto channel_it = std::find_if(m_media_channels.begin(), m_media_channels.end(), [id](const VS_MediaChannelInfo& channel) {
			return channel.index == id;
		});
		auto new_channel_it = std::find_if(channels.begin(), channels.end(), [id](const VS_MediaChannelInfo& channel) {
			return channel.index == id;
		});

		if (channel_it == m_media_channels.end() && new_channel_it != channels.end())
		{
			// Add new channel
			channel_it = m_media_channels.emplace(m_media_channels.end(), *new_channel_it);
			if (!OpenRTPChannel(*channel_it))
			{
				dstream1 << "VS_RTPSession(" << m_id << "): open RTP channel fail: id=" << new_channel_it->index;

				m_media_channels.erase(channel_it);
				continue;
			}

			dstream3 << "VS_RTPSession(" << m_id << "): open RTP channel: id=" << channel_it->index
				<< ", local ports={" << channel_it->our_rtp_address.port() << "," << channel_it->our_rtcp_address.port() << "}"
				<< ", remote ports={" << channel_it->remote_rtp_address.port() << "," << channel_it->remote_rtcp_address.port() << "}"
				<< '\n';
		}
		else if (channel_it != m_media_channels.end() && new_channel_it == channels.end())
		{
			dstream3 << "VS_RTPSession(" << m_id << "): close RTP channel: id=" << channel_it->index
				<< ", local ports={" << channel_it->our_rtp_address.port() << "," << channel_it->our_rtcp_address.port() << "}"
				<< ", remote ports={" << channel_it->remote_rtp_address.port() << "," << channel_it->remote_rtcp_address.port() << "}"
				<< '\n';

			// Remove old channel
			CloseRTPChannel(*channel_it);
			m_media_channels.erase(channel_it);
		}
		else if (channel_it != m_media_channels.end() && new_channel_it != channels.end())
		{
			dstream3 << "VS_RTPSession(" << m_id << "): update RTP channel: id=" << channel_it->index
				<< ", local ports={" << channel_it->our_rtp_address.port() << "," << channel_it->our_rtcp_address.port() << "}"
				<< ", old remote ports={" << channel_it->remote_rtp_address.port() << "," << channel_it->remote_rtcp_address.port() << "}"
				<< ", new remote ports={" << new_channel_it->remote_rtp_address.port() << "," << new_channel_it->remote_rtcp_address.port() << "}"
				<< '\n';

			// Update channel
			*channel_it = *new_channel_it;
			if (!UpdateRTPChannel(*channel_it))
			{
				CloseRTPChannel(*channel_it);
				m_media_channels.erase(channel_it);
				continue;
			}
		}
	}

	m_signal_SetMediaChannels(m_id, m_media_channels);
	CreateFilters();
}

void VS_RTPSession::FullIntraframeRequest(bool from_rtp)
{
	dprint4("VS_RTPSession(%s): FullIntraframeRequest from %s\n", m_id.c_str(),from_rtp?"rtp":"tc");
	if(from_rtp)
	{
		if (m_participant_sink)
			m_participant_sink->SendCommandToSources(ffl::FilterCommand::MakeKeyFrameRequest());
	}
	else
	{
		if (m_main_video_channel)
			if (const auto& rtp_sink = GetRTPSink(m_main_video_channel->index))
				rtp_sink->SendCommandToSources(ffl::FilterCommand::MakeKeyFrameRequest());
		if (m_slides_video_channel)
			if (const auto& rtp_sink = GetRTPSink(m_slides_video_channel->index))
				rtp_sink->SendCommandToSources(ffl::FilterCommand::MakeKeyFrameRequest());
	}
}

void VS_RTPSession::RestrictBitrateSVC(uint32_t v_bitrate, uint32_t bitrate, uint32_t old_bitrate)
{
	if (m_participant_sink)
		m_participant_sink->SendCommandToSources(ffl::FilterCommand::MakeSetBitrateRequest(v_bitrate));
}

void VS_RTPSession::SetFakeVideoMode(FakeVideo_Mode mode)
{
	dprint4("VS_RTPSession(%s): SetFakeVideoMode: mode=%u, old_mode=%u\n", m_id.c_str(), (unsigned int)mode, (unsigned int)m_fake_video_mode);
	if (m_fake_video_mode == mode)
		return;

	switch (mode)
	{
	case FVM_GROUPCONF_NOPEOPLE:
	case FVM_BROADCAST_INPROGRESS:
	case FVM_NOSPEAKERS:
	{
		VS_BinBuff image;
		unsigned int w;
		unsigned int h;
		if (m_parameters->background_image_path.empty() || !ReadImageFromFile(image, w, h, m_parameters->background_image_path.c_str()))
		{
			w = 864;
			h = 480;
			DrawTextOnImage(image, w, h, GetFakeVideoMessage(mode, m_parameters->language.c_str()), 32, w*3/5);
		}
		m_vs_banner_source->SetImage(image.Buffer(), image.Size(), w, h);
		m_vs_banner_source->Resume();
	}
		break;
	default:
		m_vs_banner_source->Pause();
		break;
	}

	m_fake_video_mode = mode;
	CreateFilters_VS2RTPVideo();
}

void VS_RTPSession::ShowSlide(const char* url)
{
	auto ds = dstream4;
	ds << "VS_RTPSession(" << m_id << "): ShowSlide: url=\"" << (url ? url : "(null)") << '\"';
	m_source_collection->SetSlide(m_conf_name, url);

	if (url)
	{
		// Set up temporary replacement of main video if needed
		m_parameters->UpdateSlideshow();
		if (!m_slides_video_channel && m_parameters->slide_show_duration_s > 0)
		{
			ds << ", updating chain VS2RTP video for " << m_parameters->slide_show_duration_s << "s";
			SetFakeVideoMode(FVM_SLIDESHOW);
			m_slide_expire_time = std::chrono::steady_clock::now() + std::chrono::seconds(m_parameters->slide_show_duration_s);
			VS_Singleton<VS_ThreadPool>::Instance().Post([this, self_weak = weak_from_this()]() {
				auto self = self_weak.lock();
				if (!self)
					return;

				if (std::chrono::steady_clock::now() < m_slide_expire_time)
					return;
				SetFakeVideoMode(FVM_DISABLED);
			}, m_slide_expire_time);
		}
	}
	else
		SetFakeVideoMode(FVM_DISABLED);

	ds << '\n';
}

void VS_RTPSession::SelectVideo(eSDP_ContentType content)
{
	dprint4("VS_RTPSession(%s): SelectVideo: content=%u\n", m_id.c_str(), (unsigned int)content);
	if (m_recv_content == content)
		return;
	if (content == SDP_CONTENT_SLIDES && !m_slides_video_channel)
		return;

	m_recv_content = content;
	CreateFilters_RTP2VSVideo();
	if (m_recv_content == content)
	{
		// Report only if we changed video content to which was requested
		bool slides_available = false;
		if (auto rtp_slides_monitor = m_rtp_slides_monitor.lock())
			slides_available = rtp_slides_monitor->State();
		m_signal_VideoStatus(m_id, m_recv_content, slides_available);
	}
}

void VS_RTPSession::PauseAudio()
{
	dprint4("VS_RTPSession(%s): PauseAudio\n", m_id.c_str());
	if (m_device_status & DVS_SND_NOTPRESENT || m_device_status & DVS_SND_PAUSED)
		return;

	m_device_status |= DVS_SND_PAUSED;
	CreateFilters_RTP2VSAudio();
	m_signal_DeviceStatus(m_id, m_device_status);
}

void VS_RTPSession::ResumeAudio()
{
	dprint4("VS_RTPSession(%s): ResumeAudio\n", m_id.c_str());
	if (!(m_device_status & DVS_SND_PAUSED))
		return;

	m_device_status &= ~DVS_SND_PAUSED;
	CreateFilters_RTP2VSAudio();
	m_signal_DeviceStatus(m_id, m_device_status);
}

void VS_RTPSession::PauseVideo()
{
	dprint4("VS_RTPSession(%s): PauseVideo\n", m_id.c_str());
	//if (m_device_status & (DVS_SND_NOTPRESENT << 16) || m_device_status & (DVS_SND_PAUSED << 16))
	//	return;
	if (m_device_status & (DVS_SND_NOTPRESENT << 16))
		return;

	m_device_status |= (DVS_SND_PAUSED << 16);
	CreateFilters_RTP2VSVideo();
	m_signal_DeviceStatus(m_id, m_device_status);
}

void VS_RTPSession::ResumeVideo()
{
	dprint4("VS_RTPSession(%s): ResumeVideo\n", m_id.c_str());
	//if (!(m_device_status & (DVS_SND_PAUSED << 16)))
	//	return;

	m_device_status &= ~(DVS_SND_PAUSED << 16);
	CreateFilters_RTP2VSVideo();
	m_signal_DeviceStatus(m_id, m_device_status);
}

void VS_RTPSession::ContentForward_Pull()
{
	// Nothing to do
	// dprint4("VS_RTPSession(%s): ContentForward_Pull\n", m_id.c_str());
}

void VS_RTPSession::ContentForward_Push()
{
	dprint4("VS_RTPSession(%s): ContentForward_Push\n", m_id.c_str());
	if (!m_slides_video_channel || !m_slides_video_channel->IsRecv())
		return;

	m_source_collection->SetContentSource(m_conf_name, m_our_part_id.c_str(), GetUnwrappedRTPVideoSource(m_slides_video_channel));
}

void VS_RTPSession::ContentForward_Stop()
{
	dprint4("VS_RTPSession(%s): ContentForward_Stop\n", m_id.c_str());
	m_source_collection->SetContentSource(m_conf_name, m_our_part_id.c_str(), nullptr);
}

void VS_RTPSession::FarEndCameraControl(eFeccRequestType type, int32_t extra_param)
{
	if (m_fecc_source)
		m_fecc_source->ProcessMessage(type, extra_param);
}

void VS_RTPSession::LogStatistics(unsigned seq_id)
{
	if (seq_id != m_stat_log_seq_id)
		return;

	LogStatistics_Media();
	LogStatistics_RTCP();
	LogStatistics_H235();

	if (!m_stopping)
		ScheduleLogStatistics();
}

void VS_RTPSession::LogStatistics_Media()
{
	auto ds = dstream3;
	auto log_audio = [&ds](const char* name, const std::weak_ptr<ffl::FilterStatisticsCalculator>& w_stat) {
		if (const auto stat = w_stat.lock())
		{
			float br_avg; unsigned br_min; unsigned br_max;
			stat->GetAudioBitrate(&br_avg, &br_min, &br_max);
			ds << '\t' << name << ": mf=\"" << stream_aformat(stat->GetMediaFormat()) << "\", bitrate=" << br_avg << " (min=" << br_min << ",max=" << br_max << ")\n";
		}
	};
	auto log_video = [&ds](const char* name, const std::weak_ptr<ffl::FilterStatisticsCalculator>& w_stat) {
		if (const auto stat = w_stat.lock())
		{
			float br_avg; unsigned br_min; unsigned br_max;
			stat->GetVideoBitrate(&br_avg, &br_min, &br_max);
			float fps_avg; unsigned fps_min; unsigned fps_max;
			stat->GetVideoFPS(&fps_avg, &fps_min, &fps_max);
			ds << '\t' << name << ": mf=\"" << stream_vformat(stat->GetMediaFormat()) << "\", bitrate=" << br_avg << " (min=" << br_min << ",max=" << br_max << "), fps=" << fps_avg << " (min=" << fps_min << ",max=" << fps_max << ")\n";
		}
	};

	ds << "VS_RTPSession(" << m_id << "): statistics for the last " << static_cast<unsigned>(ffl::FilterStatisticsCalculator::c_average_seconds) << " seconds:\n";
	log_audio("vs  in  audio", m_vs_in_audio_stat);
	log_audio("rtp out audio", m_rtp_out_audio_stat);
	log_video("vs  in  video", m_vs_in_video_stat);
	log_video("rtp out video", m_rtp_out_video_stat);
	log_video("rtp out slide", m_rtp_out_slides_stat);

	log_audio("rtp in  audio", m_rtp_in_audio_stat);
	log_audio("vs  out audio", m_vs_out_stat);
	log_video("rtp in  video", m_rtp_in_video_stat);
	log_video("vs  out video", m_vs_out_stat);
	log_video("rtp in  slide", m_rtp_in_slides_stat);
}

void VS_RTPSession::LogStatistics_RTCP()
{
	auto ds = dstream3;
	auto log = [&ds](const char* name, const std::weak_ptr<VS_RTP_Channel> w_channel, bool is_out) {
		if (const auto channel = w_channel.lock())
		{
			const auto st = is_out ? channel->OutgoingStatistics() : channel->IncomingStatistics();
			ds << '\t' << name << ": jitter=" << std::setw(6) << st.lastJitter << ", max_jitter=" << std::setw(6) << st.maxJitter << ", lost= " << std::setw(3) << st.totalLossPercent << "%, total_lost=" << st.totalLostPackets << '\n';
		}
	};

	ds << "VS_RTPSession(" << m_id << "): RTCP statistics\n";
	log("rtp out audio", m_rtp_audio_channel,  true);
	log("rtp out video", m_rtp_video_channel,  true);
	log("rtp out slide", m_rtp_slides_channel, true);
	log("rtp out data ", m_rtp_data_channel,   true);
	log("rtp in  audio", m_rtp_audio_channel,  false);
	log("rtp in  video", m_rtp_video_channel,  false);
	log("rtp in  slide", m_rtp_slides_channel, false);
	log("rtp in  data ", m_rtp_data_channel,   false);
}

void VS_RTPSession::LogStatistics_H235()
{
	auto ds = dstream3;
	auto log = [&ds](const char* name, const std::weak_ptr<VS_RTP_Channel> w_channel, bool is_enc) {
		if (const auto channel = w_channel.lock())
		{
			const auto st = is_enc ? channel->EncryptStatistic() : channel->DecryptionStatistic();
			ds << '\t' << name << ": packets=" << std::setw(4) << st.packets << ", encrypted=" << std::setw(4) << st.encryptedDecrypted << " (CTS=" << std::setw(4) << st.ctsMethod << ", padding=" << std::setw(4) << st.paddingMethod << "), errors=" << (st.packets - st.encryptedDecrypted) << '\n';
		}
	};

	ds << "VS_RTPSession(" << m_id << "): H235 statistics\n";
	log("rtp out audio", m_rtp_audio_channel,  true);
	log("rtp out video", m_rtp_video_channel,  true);
	log("rtp out slide", m_rtp_slides_channel, true);
	log("rtp out data ", m_rtp_data_channel,   true);
	log("rtp in  audio", m_rtp_audio_channel,  false);
	log("rtp in  video", m_rtp_video_channel,  false);
	log("rtp in  slide", m_rtp_slides_channel, false);
	log("rtp in  data ", m_rtp_data_channel,   false);
}

void VS_RTPSession::ScheduleLogStatistics()
{
	VS_Singleton<VS_ThreadPool>::Instance().Post([this, self_weak = weak_from_this(), seq_id = ++m_stat_log_seq_id]() {
		auto self = self_weak.lock();
		if (!self)
			return;
		LogStatistics(seq_id);
	}, std::chrono::seconds(static_cast<unsigned>(ffl::FilterStatisticsCalculator::c_average_seconds)));
}

void VS_RTPSession::SendSlide(const std::vector<unsigned char> &buf, const SlideInfo &info)
{
	if (m_parameters->slide_upload_url.empty()) return;

	size_t (*write_callback)(char *, size_t, size_t, void *) = [](char *ptr, size_t size, size_t nmemb, void *userdata) -> size_t {
		std::stringstream *ss = (std::stringstream *)userdata;
		ss->write(ptr, size * nmemb);
		return size * nmemb;
	};

	CURL* curl = curl_easy_init();

	std::string upload_url = m_parameters->slide_upload_url;
	upload_url += "?k=";
	upload_url += m_session_key;
	upload_url += "&call_id=";

	char *call_id = curl_easy_escape(curl, m_our_part_id.c_str(), m_our_part_id.length());
	upload_url += call_id;
	curl_free(call_id);

	upload_url += "&conf_id=";
	upload_url += m_conf_name;

	std::stringstream ss;

	curl_easy_setopt(curl, CURLOPT_URL, upload_url.c_str());

	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "upload_file",
		CURLFORM_CONTENTTYPE, info.img_type.c_str(),
		CURLFORM_BUFFER, "slide.jpg",
		CURLFORM_BUFFERPTR, &buf[0],
		CURLFORM_BUFFERLENGTH, buf.size(),
		CURLFORM_END);
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ss);

	CURLcode res = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	curl_formfree(formpost);

	std::string download_url;

	try {
		json::Object js_root;
		json::Reader::Read(js_root, ss);
		const json::Object &meta = js_root["meta"];
		if (meta["result"] == json::String("success")) {
			const json::Object &data = js_root["data"];
			const json::String &url = data["url"];
			download_url = url;
		}
	} catch (...) {}

	if (!download_url.empty()) {
		m_signal_ShowSlide(m_id, download_url.c_str(), info);
	}
}

bool VS_RTPSession::OpenRTPChannel(VS_MediaChannelInfo& channel)
{
	auto rtp_channel = m_rtp->Get(channel.index, true);
	if (!rtp_channel)
		return false;

	// if one of the sides uses IPv6 then guess that we expect media data to come over IPv6
	bool ipv6 = channel.our_rtp_address.address().is_v6() || channel.remote_rtp_address.address().is_v6();

	if (!rtp_channel->Start(ipv6))
	{
		m_rtp->Disconnect(channel.index);
		return false;
	}

	UpdateRTPChannel(channel);
	return true;
}

bool VS_RTPSession::CloseRTPChannel(VS_MediaChannelInfo& channel)
{
	auto rtp_channel = m_rtp->Get(channel.index);
	if (!rtp_channel)
		return true;

	auto sink_it = m_rtp_sinks.find(channel.index);
	if (sink_it != m_rtp_sinks.end())
	{
		sink_it->second->SetRTPChannel(nullptr);
		m_rtp_sinks.erase(sink_it);
	}
	auto source_it = m_rtp_sources.find(channel.index);
	if (source_it != m_rtp_sources.end())
	{
		source_it->second->SetRTPChannel(nullptr);
		m_rtp_sources.erase(source_it);
	}
	m_rtp->Disconnect(channel.index);
	return true;
}

template <class MediaMode>
const VS_H235SecurityCapability* GetH235SecurityCapability_Recv(const std::vector<MediaMode>& modes, int& pt)
{
	auto it = std::find_if(modes.begin(), modes.end(), [](const MediaMode& m) {
		return !m.sec_cap.h235_sessionKey.empty() && m.sec_cap.m != encryption_mode::no_encryption;
	});
	if (it == modes.end())
		return nullptr;

	pt = it->PayloadType;
	return &it->sec_cap;
}

template <class MediaMode>
const VS_H235SecurityCapability* GetH235SecurityCapability_Send(const MediaMode& mode)
{
	if (mode.sec_cap.h235_sessionKey.empty() || mode.sec_cap.m == encryption_mode::no_encryption)
		return nullptr;

	return &mode.sec_cap;
}

void EnableRTPSecurity(const VS_MediaChannelInfo& ch_info, std::shared_ptr<VS_RTP_Channel>& rtp_channel, const std::string& rtp_session_id)
{
	if (!rtp_channel) return;

	if (!ch_info.our_srtp_key.empty() && !ch_info.remote_srtp_key.empty()) {
		rtp_channel->InitSRTP(ch_info.our_srtp_key, ch_info.remote_srtp_key);
		if (ch_info.type == SDPMediaType::video && ch_info.IsRecv()) {
			int pt = 0;
			for (const auto &m : ch_info.rcv_modes_video) {
				if (m.CodecType == e_videoXH264UC) {
					pt = m.PayloadType;
				}
			}
			rtp_channel->SetVSRPT(pt);
		}
	}
	else if (ch_info.type != SDPMediaType::invalid) {
		const VS_H235SecurityCapability* recv_cap = nullptr;
		const VS_H235SecurityCapability* send_cap = nullptr;
		int recv_pt(0);
		switch (ch_info.type)
		{
		case SDPMediaType::audio:
			recv_cap = GetH235SecurityCapability_Recv(ch_info.rcv_modes_audio, recv_pt);
			send_cap = GetH235SecurityCapability_Send(ch_info.snd_mode_audio);

			break;
		case SDPMediaType::video:
			recv_cap = GetH235SecurityCapability_Recv(ch_info.rcv_modes_video, recv_pt);
			send_cap = GetH235SecurityCapability_Send(ch_info.snd_mode_video);
			break;
		case SDPMediaType::application_fecc:
			recv_cap = GetH235SecurityCapability_Recv(ch_info.rcv_modes_data, recv_pt);
			send_cap = GetH235SecurityCapability_Send(ch_info.snd_mode_data);
			break;
		}
		if (recv_cap || send_cap)
		{
			auto ds = dstream4;
			ds << "VS_RTPSession(" << rtp_session_id << "): H235 (channel=" << ch_info.index << ") init:";
			if (recv_cap)
				ds << " recv_cap found (syncFlag=" << recv_cap->syncFlag << ", pt=" << recv_pt << ')';
			else
				ds << " recv_cap not found";
			if (send_cap)
				ds << ", send_cap found (syncFlag=" << send_cap->syncFlag << ')';
			else
				ds << ", send_cap not found";
			rtp_channel->InitH235MediaSession(recv_cap, send_cap, recv_pt);
		}
	}
}

bool VS_RTPSession::UpdateRTPChannel(VS_MediaChannelInfo& channel)
{
	auto rtp_channel = m_rtp->Get(channel.index);
	if (!rtp_channel)
		return false;

	rtp_channel->SetRemoteRTPEndpoint(channel.remote_rtp_address);
	rtp_channel->SetRemoteRTCPEndpoint(channel.remote_rtcp_address);

	VS_RTP_Channel::endpoint_t local_rtp_endpoint;
	VS_RTP_Channel::endpoint_t local_rtcp_endpoint;

	const auto res = rtp_channel->GetLocalEndpoints(local_rtp_endpoint, local_rtcp_endpoint);
	assert(res);

	channel.our_rtp_address = local_rtp_endpoint;
	channel.our_rtcp_address = local_rtcp_endpoint;

	rtp_channel->SetICEPWD(channel.our_ice_pwd);
	EnableRTPSecurity(channel, rtp_channel, m_id);

	std::map<uint8_t, uint32_t> clock_rates_map;//map of <PayloadType, ClockRate>
	switch (channel.type)
	{
	case SDPMediaType::audio:
		for (auto&& a_mode : channel.rcv_modes_audio)
		{
			clock_rates_map.emplace(a_mode.PayloadType, a_mode.ClockRate);
		}
		channel.snd_mode_audio.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::audio, channel.snd_mode_audio.CodecType);
		rtp_channel->SetLocalRTPClockRate(channel.snd_mode_audio.ClockRate);

		break;
	case SDPMediaType::video:
		for (auto&& v_mode : channel.rcv_modes_video)
		{
			clock_rates_map.emplace(v_mode.PayloadType, v_mode.ClockRate);
		}
		channel.snd_mode_video.ClockRate = vs::GetCodecClockRateByCodecType(SDPMediaType::video, channel.snd_mode_video.CodecType);
		rtp_channel->SetLocalRTPClockRate(channel.snd_mode_video.ClockRate);

		break;
	case SDPMediaType::invalid:
		default:
		break;
	}

	if (channel.ssrc_range.first) {
		rtp_channel->SetSSRCRange(std::make_pair(channel.ssrc_range.first, channel.ssrc_range.first));
	}

	if (!channel.IsSend() && channel.remote_rtp_address.port() != 0)
		rtp_channel->SendFakePackets();

	rtp_channel->SetRemoteRTPClockRates(std::move(clock_rates_map));
	return true;
}

std::shared_ptr<ffl::SinkRTPChannel> VS_RTPSession::GetRTPSink(unsigned int id, bool create)
{
	auto it = m_rtp_sinks.find(id);
	if (it == m_rtp_sinks.end())
	{
		if (create)
		{
			it = m_rtp_sinks.emplace(id, ffl::SinkRTPChannel::Create()).first;
			it->second->SetRTPChannel(m_rtp->Get(id));
		}
		else
			return nullptr;
	}
	return it->second;
}

std::shared_ptr<ffl::SourceRTPChannel> VS_RTPSession::GetRTPSource(unsigned int id, bool create)
{
	auto it = m_rtp_sources.find(id);
	if (it == m_rtp_sources.end())
	{
		if (create)
		{
			stream::Track track = {};
			auto c_it = std::find_if(m_media_channels.begin(), m_media_channels.end(), [id](const VS_MediaChannelInfo& x) {
				return x.index == id;
			});
			if (c_it != m_media_channels.end())
			{
				switch (c_it->type)
				{
				case SDPMediaType::audio: track = stream::Track::audio; break;
				case SDPMediaType::video: track = stream::Track::video; break;
				}
			}

			it = m_rtp_sources.emplace(id, ffl::SourceRTPChannel::Create()).first;
			it->second->SetRTPChannel(m_rtp->Get(id), track);
		}
		else
			return nullptr;
	}
	return it->second;
}

void VS_RTPSession::SelectChannels()
{
	{
		auto it = std::find_if(m_media_channels.begin(), m_media_channels.end(), [](const VS_MediaChannelInfo& x) {
			return x.type == SDPMediaType::audio && x.content == SDP_CONTENT_MAIN;
		});
		m_audio_channel = it != m_media_channels.end() ? &*it : nullptr;
	}

	{
		auto it = std::find_if(m_media_channels.begin(), m_media_channels.end(), [](const VS_MediaChannelInfo& x) {
			return x.type == SDPMediaType::video && x.content == SDP_CONTENT_MAIN;
		});
		m_main_video_channel = it != m_media_channels.end() ? &*it : nullptr;
	}

	{
		auto it = std::find_if(m_media_channels.begin(), m_media_channels.end(), [](const VS_MediaChannelInfo& x) {
			return x.type == SDPMediaType::video && x.content == SDP_CONTENT_SLIDES;
		});
		m_slides_video_channel = it != m_media_channels.end() ? &*it : nullptr;
	}

	{
		auto it = std::find_if(m_media_channels.begin(), m_media_channels.end(), [](const VS_MediaChannelInfo& x) {
			return x.type == SDPMediaType::application_fecc;
		});
		m_data_channel = it != m_media_channels.end() ? &*it : nullptr;
	}
}

void VS_RTPSession::CreateFilters()
{
	if (m_conf_name.empty() && m_remote_part_id.empty())
		return;

	SelectChannels();
	if (!m_audio_channel && !m_main_video_channel && !m_slides_video_channel && !m_data_channel)
		return;
	// If there are some channels present then we need to create all common FFL objects,
	// because all other code assumes that they exist when a channel is present.

	InitTraceLog();

	const bool is_groupconf = m_remote_part_id.empty();

	if (!m_participant)
	{
		m_participant = m_partsMgr->GetPart(m_conf_name.c_str(), m_our_part_id.c_str());
		m_participant->ConnectStream();
	}

	if (!m_participant_sink)
	{
		m_participant_sink = ffl::SinkTransceiverParticipant::Create(m_participant);

		std::shared_ptr<ffl::AbstractSource> head;
		m_vs_sink = ffl::FilterAudioVideoJoiner::Create();
		m_vs_sink->SetChainID();
		m_vs_sink->EnableTrace(m_trace_log);
		head = m_vs_sink;
		auto vs_out_stat = ffl::FilterStatisticsCalculator::Create(head);
		m_vs_out_stat = vs_out_stat;
		head = vs_out_stat;
		head = ffl::FilterChangeFormatCommandInjector::Create(head);
		head = ffl::FilterVSFrameSlicer::Create(head, is_groupconf);
		head = ffl::FilterVSFrameWrapper::Create(head);
		head->RegisterSinkOrGetCompatible(m_participant_sink);
	}

	if (!m_participant_source)
	{
		m_participant_source = ffl::SourceTransceiverParticipant::Create(m_participant, m_conference_caps);
		m_participant_source->SetChainID();
		m_participant_source->EnableTrace(m_trace_log);
	}

	if (is_groupconf && !m_conference_source)
	{
		auto media_source = m_source_collection->MediaSourceCollection().GetMediaSource(m_conf_name.c_str());
		assert(media_source);

		std::string part_id;
		if ((m_conf_subtype == GCST_ALL_TO_OWNER || m_conf_subtype == GCST_PENDING_ALL_TO_OWNER) && m_our_part_id != m_conf_owner)
			part_id = m_conf_owner;

		m_conference_source = ffl::SourceMediaPeer::Create(m_our_part_id.c_str(), part_id.c_str(), media_source);
		m_conference_source->SetChainID();
		m_conference_source->EnableTrace(m_trace_log);
	}

	if (!m_vs_slides_source)
	{
		m_vs_slides_source = m_source_collection->GetSlideshowSource(m_conf_name, m_our_part_id.c_str());
		m_vs_slides_source->SetChainID();
		m_vs_slides_source->EnableTrace(m_trace_log);
	}
	m_vs_banner_source->SetChainID();
	m_vs_banner_source->EnableTrace(m_trace_log);
	if (m_audio_channel)
	{
		auto rtp_source = GetRTPSource(m_audio_channel->index, true);
		if (rtp_source)
		{
			rtp_source->SetChainID();
			rtp_source->EnableTrace(m_trace_log);
		}
	}
	if (m_main_video_channel)
	{
		auto rtp_source = GetRTPSource(m_main_video_channel->index, true);
		if (rtp_source)
		{
			rtp_source->SetChainID();
			rtp_source->EnableTrace(m_trace_log);
		}
	}
	if (m_slides_video_channel)
	{
		auto rtp_source = GetRTPSource(m_slides_video_channel->index, true);
		if (rtp_source)
		{
			rtp_source->SetChainID();
			rtp_source->EnableTrace(m_trace_log);
		}
	}
	if (m_data_channel)
	{
		auto rtp_source = GetRTPSource(m_data_channel->index, true);
		if (rtp_source)
		{
			rtp_source->SetChainID();
			rtp_source->EnableTrace(m_trace_log);
		}
	}

	CreateFilters_RTPSlidesMonitor();
	if (m_parameters->content_as_slides) {
		CreateFilters_RTPSlidesEncoder();
	}
	auto old_device_status = m_device_status;
	CreateFilters_RTP2VSAudio();
	CreateFilters_RTP2VSVideo();
	CreateFilters_VS2RTPAudio();
	CreateFilters_VS2RTPVideo();
	CreateFilters_VS2RTPSlides();
	CreateFilters_VS2RTPFECC();
	CreateFilters_RTP2VSFECC();
	if (m_device_status != old_device_status)
		m_signal_DeviceStatus(m_id, m_device_status);
	ScheduleLogStatistics();
	LogTraceLogInfo();
}

static const unsigned char H264_start_code[] = { 0, 0, 0, 1 };

std::shared_ptr<ffl::AbstractSource> VS_RTPSession::GetUnwrappedRTPAudioSource(VS_MediaChannelInfo* audio_channel)
{
	std::shared_ptr<ffl::AbstractSource> audio_head;

	auto ds = dstream3;
	ds << "VS_RTPSession(" << m_id << "): RTP audio input (channel=" << audio_channel->index << ") init";

	ffl::FilterRTPValidator::pt_set_t valid_pt;
	ffl::FilterRTPUnwrapper::format_map_t rtp_in_payloads_audio;
	ffl::FilterRTPUnwrapper::mpeg4es_config_map_t mpeg4es_conf;
	ffl::FilterRTPUnwrapper::swap_bytes_map_t swap_bytes;
	for (const auto& x: audio_channel->rcv_modes_audio)
	{
		VS_MediaFormat mf;
		FillMediaFormat(mf, x);
		mf.SetVideo(0, 0, 0, 0, 0, 0, 0);
		if (!mf.IsAudioValid())
			continue;
		if (x.CodecType == e_rcvAAC)
		{
			unsigned sfi;
			unsigned sample_rate;
			ParseMPEG4AudioConfig(x.mpeg4es.config.data(), x.mpeg4es.config.size(), nullptr, &sfi, nullptr, &sample_rate);
			mf.dwAudioSampleRate = sfi == 15 ? sample_rate : SamplingRateFromMPEG4FrequencyIndex(sfi);
			mpeg4es_conf[x.PayloadType] = x.mpeg4es;
		}
		swap_bytes[x.PayloadType] = x.SwapBytes;
		rtp_in_payloads_audio[x.PayloadType] = mf;
		valid_pt.insert(x.PayloadType);
	}
	if (rtp_in_payloads_audio.empty())
	{
		ds << " failed: can't select media format for any rcv mode";
		return audio_head;
	}

	audio_head = GetRTPSource(audio_channel->index, true);
	audio_head = ffl::FilterDumpRTP::Create(audio_head, "rtp-in");
	audio_head = ffl::FilterRTPValidator::Create(audio_head, valid_pt, false);
	audio_head = ffl::FilterRTPSorter::Create(audio_head);
	audio_head = ffl::FilterDumpRTP::Create(audio_head, "rtpsort-out");
	audio_head = ffl::FilterRTPUnwrapper::Create(audio_head, rtp_in_payloads_audio, mpeg4es_conf, swap_bytes);
	audio_head = ffl::FilterDumpAudio::Create(audio_head, "rtpunwrap-out");
	audio_head = ffl::FilterAudioFormatReader::Create(audio_head);
	auto rtp_in_stat = ffl::FilterStatisticsCalculator::Create(audio_head);
	// When devices's audio becomes paused its transcoding chain gets deleted.
	// Make statistics calculator persistent to ensure that:
	// 1. We are always logging RTP statistics, even when audio is paused.
	rtp_in_stat->Persistent(true);
	// If wnwrapping chain have changed, we need to manually detach the old chain to ensure it will be deleted.
	if (auto rtp_in_audio_stat = m_rtp_in_audio_stat.lock())
	{
		if (rtp_in_audio_stat != rtp_in_stat)
			rtp_in_audio_stat->Detach();
	}
	m_rtp_in_audio_stat = rtp_in_stat;
	audio_head = rtp_in_stat;

	ds << ": in formats:\n";
	for (const auto& x: rtp_in_payloads_audio)
		ds << "\t{mf=\"" << x.second << "\" <- pt=" << static_cast<int>(x.first) << "}\n";

	return audio_head;
}

std::shared_ptr<ffl::AbstractSource> VS_RTPSession::GetUnwrappedRTPVideoSource(VS_MediaChannelInfo* video_channel)
{
	std::shared_ptr<ffl::AbstractSource> video_head;

	const bool is_groupconf = m_remote_part_id.empty();
	const bool is_slides_channel = video_channel->content == SDP_CONTENT_SLIDES;

	auto ds = dstream3;
	ds << "VS_RTPSession(" << m_id << "): RTP " << (is_slides_channel ? "slides" : "main video") << " input (channel=" << video_channel->index << ") init";

	ffl::FilterRTPValidator::pt_set_t valid_pt;
	ffl::FilterRTPUnwrapper::format_map_t rtp_in_payloads_video;
	std::vector<unsigned char> sps;
	std::vector<unsigned char> pps;
	for (const auto& x: video_channel->rcv_modes_video)
	{
		VS_MediaFormat mf;
		FillMediaFormat(mf, x, false, is_groupconf);
		mf.SetAudio(0, 0, 0);
		if (!mf.IsVideoValid())
			continue;
		rtp_in_payloads_video[x.PayloadType] = mf;
		valid_pt.insert(x.PayloadType);

		if (x.sizeOfSPS > 12)
		{
			sps.assign(std::begin(H264_start_code), std::end(H264_start_code));
			sps.insert(sps.end(), x.SequenceParameterSet + 12, x.SequenceParameterSet + x.sizeOfSPS);
		}
		if (x.sizeOfPPS > 12)
		{
			pps.assign(std::begin(H264_start_code), std::end(H264_start_code));
			pps.insert(pps.end(), x.PictureParameterSet + 12, x.PictureParameterSet + x.sizeOfPPS);
		}
	}
	if (rtp_in_payloads_video.empty())
	{
		ds << " failed: can't select media format for any rcv mode";
		return video_head;
	}

	auto rtp_source = GetRTPSource(video_channel->index, true);
	if (std::none_of(video_channel->rcv_modes_video.begin(), video_channel->rcv_modes_video.end(), [](const VS_GatewayVideoMode& x) { return x.IsFIRSupported; }))
		rtp_source->SetAlternativeFIRHandler([this]() {
			dprint3("VS_RTPSession(%s): requesting key frame from RTP source via signaling\n", m_id.c_str());
			m_signal_FullIntraframeRequest(m_id);
		});
	video_head = rtp_source;
	video_head = ffl::FilterDumpRTP::Create(video_head, "rtp-in");
	video_head = ffl::FilterKeyFrameRequestLimiter::Create(video_head, std::chrono::seconds(m_parameters->in_min_keyframe_interval_s));
	video_head = ffl::FilterRTPValidator::Create(video_head, valid_pt, false);
	video_head = ffl::FilterRTPSorter::Create(video_head);
	video_head = ffl::FilterDumpRTP::Create(video_head, "rtpsort-out");
	video_head = ffl::FilterRTPUnwrapper::Create(video_head, rtp_in_payloads_video, {}, {}, video_channel->snd_mode_video.CodecType == e_videoXH264UC);
	video_head = ffl::FilterDumpVideo::Create(video_head, "rtpunwrap-out");
	auto sps_pps_injector = ffl::FilterH264SpsPpsInjector::Create(video_head);
	sps_pps_injector->SetSPS(std::move(sps));
	sps_pps_injector->SetPPS(std::move(pps));
	video_head = sps_pps_injector;
	video_head = ffl::FilterVideoFormatReader::Create(video_head);
	auto rtp_in_stat = ffl::FilterStatisticsCalculator::Create(video_head);
	// When devices's video becomes paused its transcoding chain gets deleted.
	// Make statistics calculator persistent to ensure that:
	// 1. We are always logging RTP statistics, even when video is paused.
	// 2. SPS/PPS injector state is preserved when video is paused.
	rtp_in_stat->Persistent(true);
	// If unwrapping chain have changed, we need to manually detach the old chain to ensure it will be deleted.
	auto& rtp_in_video_stat_ref = is_slides_channel ? m_rtp_in_slides_stat : m_rtp_in_video_stat;
	if (auto rtp_in_video_stat = rtp_in_video_stat_ref.lock())
	{
		if (rtp_in_video_stat != rtp_in_stat)
			rtp_in_video_stat->Detach();
	}
	rtp_in_video_stat_ref = rtp_in_stat;
	video_head = rtp_in_stat;

	ds << ": in formats:\n";
	for (const auto& x: rtp_in_payloads_video)
		ds << "\t{mf=\"" << x.second << "\" <- pt=" << static_cast<int>(x.first) << "}\n";

	return video_head;
}

void VS_RTPSession::CreateFilters_RTPSlidesMonitor()
{
	if (!m_slides_video_channel)
		return;

	std::shared_ptr<ffl::AbstractSource> video_head = GetUnwrappedRTPVideoSource(m_slides_video_channel);
	if (!video_head)
		return;
	auto rtp_slides_monitor = ffl::FilterActivityMonitor::Create(
		video_head,
		std::chrono::milliseconds(1500),
		std::chrono::seconds(4),
		std::chrono::seconds(10)
	);
	m_rtp_slides_report_connection = rtp_slides_monitor->ConnectToStateReport([this](bool state) {
		if (!state) {
			if (m_recv_content == SDP_CONTENT_SLIDES) {
				m_recv_content = SDP_CONTENT_MAIN;
				CreateFilters_RTP2VSVideo();
			}
			m_signal_EndSlideShow(m_id);
		}
		m_signal_VideoStatus(m_id, m_recv_content, state);
	});
	m_rtp_slides_monitor = rtp_slides_monitor;
}

void VS_RTPSession::CreateFilters_RTPSlidesEncoder()
{
	if (m_slides_video_channel && m_slides_video_channel->IsRecv())
	{
		std::shared_ptr<ffl::AbstractSource> video_head = GetUnwrappedRTPVideoSource(m_slides_video_channel);
		if (!video_head)
			return;
		video_head = ffl::FilterKeyFrameRequester::Create(video_head, std::chrono::seconds(m_parameters->in_max_keyframe_interval_s));
		std::vector<VS_MediaFormat> out_formats_video;
		out_formats_video.emplace_back();
		out_formats_video.back().SetAudio(0, 0, 0);
		out_formats_video.back().SetVideo(0, 0, FOURCC_I420);
		video_head = ffl::FilterVideoTranscoder::Create(video_head, out_formats_video, false, out_formats_video.front(), 0, false);
		auto rtp_slides_encoder = ffl::FilterSlideEncoder::Create(video_head, std::bind(&VS_RTPSession::SendSlide, this, std::placeholders::_1, std::placeholders::_2),
													 std::chrono::microseconds(m_parameters->slide_interval_us),
													 m_parameters->slide_dimension, m_parameters->slide_quality);
		m_rtp_slides_encoder = rtp_slides_encoder;
	}
}

void VS_RTPSession::CreateFilters_RTP2VSAudio()
{
	const bool is_groupconf = m_remote_part_id.empty();

	if (!m_audio_channel || !m_audio_channel->IsRecv())
	{
		m_device_status |= DVS_SND_NOTPRESENT;
		// We might not have a channel because CreateFilters() wasn't called yet.
		// In this case we can't assume that filters/sources already exist.
		if (m_vs_sink)
			m_vs_sink->SetAudio(nullptr);
		return;
	}

	if (m_device_status & DVS_SND_PAUSED)
	{
		dstream3 << "VS_RTPSession(" << m_id << "): VS audio output (channel=" << m_audio_channel->index << ") release";
		m_vs_sink->SetAudio(nullptr);
		return;
	}

	m_device_status &= ~DVS_SND_NOTPRESENT;

	std::vector<VS_MediaFormat> out_formats_audio;
	VS_MediaFormat out_format_default;
	if (is_groupconf || m_parameters->force_transcoding)
	{
		out_format_default.SetAudio(16000, VS_ACODEC_OPUS_B0914);
		out_format_default.SetVideo(0, 0, 0, 0, 0, 0, 0);
		out_formats_audio.push_back(out_format_default);
	}
	else
	{
		if (m_conference_caps.FindAudioCodec(VS_ACODEC_OPUS_B0914)) // Keep in sync with media format set in our caps (see VS_FakeClientControl::GetMyClientCaps)
			out_format_default.SetAudio(16000, VS_ACODEC_OPUS_B0914);
		else
			m_conference_caps.GetMediaFormat(out_format_default);
		out_format_default.SetVideo(0, 0, 0, 0, 0, 0, 0);

		if (m_conference_caps.GetStreamsDC()&VSCC_STREAM_CAN_CHANGE_MF_RCV)
		{
			uint16_t acodecs[100];
			size_t realCount = 100;
			m_conference_caps.GetAudioCodecs(acodecs, realCount);
			for (unsigned i = 0; i < realCount; i++)
			{
				VS_MediaFormat mf;
				mf.SetAudio(0, acodecs[i]);
				mf.SetVideo(0, 0, 0, 0, 0, 0, 0);
				out_formats_audio.push_back(mf);
			}
		}
		else
			out_formats_audio.push_back(out_format_default);
	}

	if (m_audio_channel)
		m_rtp_audio_channel = m_rtp->Get(m_audio_channel->index);

	std::shared_ptr<ffl::AbstractSource> audio_head;
	audio_head = GetUnwrappedRTPAudioSource(m_audio_channel);
	if (!audio_head)
		return;
	audio_head = ffl::FilterAudioTranscoder::Create(audio_head, out_formats_audio, out_format_default);
	m_vs_sink->SetAudio(audio_head);

	auto ds = dstream3;
	ds << "VS_RTPSession(" << m_id << "): VS audio output (channel=" << m_audio_channel->index << ") init";
	ds << ": out formats:\n";
	for (const auto& x: out_formats_audio)
		ds << "\t{mf=\"" << x << "\"}\n";
}

void VS_RTPSession::CreateFilters_RTP2VSVideo()
{
	const bool is_groupconf = m_remote_part_id.empty();

	// Select what to send to VS based on what we asked to send (m_recv_content) and channels available
	VS_MediaChannelInfo* recv_video_channel = nullptr;
	switch (m_recv_content)
	{
	case SDP_CONTENT_SLIDES:
		if (m_slides_video_channel)
			recv_video_channel = m_slides_video_channel;
		else if (m_main_video_channel)
		{
			recv_video_channel = m_main_video_channel;
			// Notify receivers that we are changing video source
			m_recv_content = SDP_CONTENT_MAIN;
			m_signal_VideoStatus(m_id, m_recv_content, false);
		}
		break;
	case SDP_CONTENT_MAIN:
		if (m_main_video_channel)
			recv_video_channel = m_main_video_channel;
		break;
	}

	if (!recv_video_channel || !recv_video_channel->IsRecv())
	{
		m_device_status |= (DVS_SND_NOTPRESENT << 16);
		// We might not have a channel because CreateFilters() wasn't called yet.
		// In this case we can't assume that filters/sources already exist.
		if (m_vs_sink)
			m_vs_sink->SetVideo(nullptr);
		return;
	}

	if (m_device_status & (DVS_SND_PAUSED << 16))
	{
		dstream3 << "VS_RTPSession(" << m_id << "): VS video output (channel=" << recv_video_channel->index << ") release";
		m_vs_sink->SetVideo(nullptr);
		return;
	}

	m_device_status &= ~(DVS_SND_NOTPRESENT << 16);

	std::vector<VS_MediaFormat> out_formats_video;
	VS_MediaFormat out_format_default;
	if (is_groupconf || m_parameters->force_transcoding)
	{
		out_format_default.SetAudio(0, 0, 0);
		out_format_default.SetVideo(m_parameters->gconf_width, m_parameters->gconf_height, VS_VCODEC_VPX);
		out_formats_video.push_back(out_format_default);
	}
	else
	{
		if (m_conference_caps.FindVideoCodec(VS_VCODEC_VPX))
			out_format_default.SetVideo(640, 360, VS_VCODEC_VPX);
		else
			m_conference_caps.GetMediaFormat(out_format_default);
		out_format_default.SetAudio(0, 0, 0);

		if (m_conference_caps.GetStreamsDC()&VSCC_STREAM_CAN_CHANGE_MF_RCV)
		{
			uint32_t vcodecs[100];
			size_t realCount = 100;
			m_conference_caps.GetVideoCodecs(vcodecs, realCount);
			for (unsigned i = 0; i < realCount; i++)
			{
				VS_MediaFormat mf;
				mf.SetAudio(0, 0, 0);
				mf.SetVideo(0, 0, vcodecs[i]);
				out_formats_video.push_back(mf);
			}
		}
		else
			out_formats_video.push_back(out_format_default);
	}

	unsigned int max_bitrate = is_groupconf ? video_presets::max_bitrate_groupconf : video_presets::max_bitrate;
	if (!is_groupconf && m_conference_caps.GetBandWRcv() > 0)
		max_bitrate = std::min(max_bitrate, (unsigned int)m_conference_caps.GetBandWRcv());

	if (m_main_video_channel)
		m_rtp_video_channel = m_rtp->Get(m_main_video_channel->index);
	if (m_slides_video_channel)
		m_rtp_slides_channel = m_rtp->Get(m_slides_video_channel->index);

	std::shared_ptr<ffl::AbstractSource> video_head;
	video_head = GetUnwrappedRTPVideoSource(recv_video_channel);
	if (!video_head)
		return;
	video_head = ffl::FilterKeyFrameRequester::Create(video_head, std::chrono::seconds(m_parameters->in_max_keyframe_interval_s));
	if (is_groupconf)
	{
		if (m_parameters->in_video_transcoding)
			video_head = ffl::FilterVideoTranscoder::Create(video_head, out_formats_video, true, out_format_default, 0x00070100, false);
	}
	else
		video_head = ffl::FilterVideoTranscoderWithResolutionLimits::Create(video_head, out_formats_video, true, out_format_default, m_conference_caps);
	video_head->ProcessCommand(ffl::FilterCommand::MakeSetBitrateRequest(max_bitrate));
	video_head->ProcessCommand(ffl::FilterCommand::MakeKeyFrameRequest());	// make sure key frame will be requested at start, it is for Delphi -> lync2013 call
	m_vs_sink->SetVideo(video_head);

	auto ds = dstream3;
	ds << "VS_RTPSession(" << m_id << "): VS video output (channel=" << recv_video_channel->index << ") init";
	ds << ": bitrate=" << max_bitrate;
	if (!is_groupconf || m_parameters->in_video_transcoding)
	{
		ds << ", out formats:\n";
		for (const auto& x: out_formats_video)
			ds << "\t{mf=\"" << x << "\"}\n";
	}
	else
		ds << ", transcoding disabled";
}

void VS_RTPSession::CreateFilters_VS2RTPAudio()
{
	const bool is_groupconf = m_remote_part_id.empty();

	if (!m_audio_channel || !m_audio_channel->IsSend())
	{
		// We might not have a channel because CreateFilters() wasn't called yet.
		// In this case we can't assume that filters/sources already exist.
		if (m_participant_source)
			m_participant_source->DisableAudio();
		return;
	}

	ffl::FilterRTPWrapper::codec_pt_map_t rtp_out_payloads_audio;
	ffl::FilterRTPWrapper::swap_bytes_map_t swap_bytes;
	VS_MediaFormat out_format_audio;
	FillMediaFormat(out_format_audio, m_audio_channel->snd_mode_audio);
	out_format_audio.SetVideo(0, 0, 0, 0, 0, 0, 0);
	if (!out_format_audio.IsAudioValid())
	{
		dstream1 << "VS_RTPSession(" << m_id << "): VS2RTP audio (channel=" << m_audio_channel->index << ") init failed: can't select media format for snd mode: {CodecType=" << m_audio_channel->snd_mode_audio.CodecType << "}\n";
		return;
	}
	swap_bytes[m_audio_channel->snd_mode_audio.PayloadType] = m_audio_channel->snd_mode_audio.SwapBytes;
	rtp_out_payloads_audio[out_format_audio.dwAudioCodecTag] = m_audio_channel->snd_mode_audio.PayloadType;

	std::vector<VS_MediaFormat> out_formats_audio;
	out_formats_audio.push_back(out_format_audio);

	std::shared_ptr<ffl::AbstractSource> audio_head;
	if (is_groupconf)
	{
		m_conference_source->SetAudioFormat(out_format_audio);
		audio_head = m_conference_source;
	}
	else
	{
		audio_head = m_participant_source;
		audio_head = ffl::FilterVSFrameUnwrapper::Create(audio_head, stream::Track::audio);
	}
	audio_head = ffl::FilterDumpAudio::Create(audio_head, "in");
	auto vs_in_audio_stat = ffl::FilterStatisticsCalculator::Create(audio_head);
	m_vs_in_audio_stat = vs_in_audio_stat;
	audio_head = vs_in_audio_stat;

	audio_head = ffl::FilterAudioTranscoder::Create(audio_head, out_formats_audio, out_format_audio);
	auto rtp_out_audio_stat = ffl::FilterStatisticsCalculator::Create(audio_head);
	m_rtp_out_audio_stat = rtp_out_audio_stat;
	audio_head = rtp_out_audio_stat;

	auto rtp_wrapper = ffl::FilterRTPWrapper::Create(audio_head, rtp_out_payloads_audio, swap_bytes);
	if (auto ssrc = m_audio_channel->ssrc_range.first) {
		rtp_wrapper->set_ssrc_override(ssrc);
	}
	audio_head = rtp_wrapper;
	audio_head = ffl::FilterDumpRTP::Create(audio_head, "rtpwrap-out");

	//audio_head = ffl::FilterRTPWrapper::Create(audio_head, rtp_out_payloads_audio, swap_bytes);
	audio_head->RegisterSinkOrGetCompatible(GetRTPSink(m_audio_channel->index, true));

	auto ds = dstream3;
	ds << "VS_RTPSession(" << m_id << "): VS2RTP audio (channel=" << m_audio_channel->index << ") init";
	ds << ": out formats:\n";
	for (const auto& x: rtp_out_payloads_audio)
		ds << "\t{codec=\"" << stream_acodec(x.first) << "\" -> pt=" << static_cast<int>(x.second) << "}\n";
}

void VS_RTPSession::CreateFilters_VS2RTPVideo()
{
	const bool is_groupconf = m_remote_part_id.empty();

	if (!m_main_video_channel || !m_main_video_channel->IsSend())
	{
		// We might not have a channel because CreateFilters() wasn't called yet.
		// In this case we can't assume that filters/sources already exist.
		if (m_participant_source)
			m_participant_source->DisableVideo();
		return;
	}

	ffl::FilterRTPWrapper::codec_pt_map_t rtp_out_payloads_video;
	VS_MediaFormat out_format_video;
	bool force_video_format = false;

	auto &snd_mode = m_main_video_channel->snd_mode_video;
	if (snd_mode.CodecType == e_videoH264 && snd_mode.preferred_width > 0 && snd_mode.preferred_height > 0)	// when user wants own height and width prefer his choice
	{
		out_format_video.SetVideo(snd_mode.preferred_width, snd_mode.preferred_height, VS_VCODEC_H264, 30);
		force_video_format = true;
	}
	else
		FillMediaFormat(out_format_video, snd_mode, true, is_groupconf);											// else make calcucations

	out_format_video.SetAudio(0, 0, 0);
	if (!out_format_video.IsVideoValid())
	{
		dstream1 << "VS_RTPSession(" << m_id << "): VS2RTP main video (channel=" << m_main_video_channel->index << ") init failed: can't select media format for snd mode: {CodecType=" << m_main_video_channel->snd_mode_video.CodecType << "}\n";
		return;
	}

	unsigned int max_bitrate = snd_mode.Bitrate / 1024;

	std::vector<VS_MediaFormat> out_formats_video;
	if (snd_mode.CodecType == e_videoH264 && snd_mode.IsMixerCIFMode)
	{
		static const struct { unsigned w, h; } modes[] = {
			//{ 1408,1152 }   // 16cif
			{ 704,576 },    // 4cif - will be default
			{ 352,288 },    // cif
			{ 176,144 },    // qcif
			{ 128,96 },     // sqcif
		};

		const unsigned maxW = max_bitrate >= 768 ? 1280 : (max_bitrate >= 256 ? 704 : 352);
		const unsigned maxH = max_bitrate >= 768 ? 720	: (max_bitrate >= 256 ? 576 : 288);

		for (auto& mode : modes)
		{
			if (mode.w > out_format_video.dwVideoWidht || mode.h > out_format_video.dwVideoHeight)
				continue;
			if (mode.w > maxW || mode.h > maxH)
				continue;

			out_format_video.dwVideoWidht = mode.w;
			out_format_video.dwVideoHeight = mode.h;
			out_formats_video.push_back(out_format_video);
		}
		out_format_video = out_formats_video.front();
		force_video_format = true;
	}
	else if (snd_mode.CodecType == e_videoXH264UC)
	{
		static const struct { unsigned w, h; } modes[] = {
			{ 1920, 1080 }, { 1280, 720 }, { 864, 480 }, { 640, 360 }, { 320, 176 },
			{ 1920, 1440 }, { 1280, 960 }, { 864, 648 }, { 640, 480 }, { 320, 240 },
		};

		auto tempFmt = out_format_video;
		for (auto& mode : modes)
		{
			tempFmt.dwVideoWidht = mode.w;
			tempFmt.dwVideoHeight = mode.h;
			out_formats_video.push_back(tempFmt);
		}
		for (auto& mode : modes)
		{
			if (mode.w > out_format_video.dwVideoWidht || mode.h > out_format_video.dwVideoHeight)
				continue;
			out_format_video.dwVideoWidht = mode.w;
			out_format_video.dwVideoHeight = mode.h;
			break;
		}
		force_video_format = true;
	}
	else if (snd_mode.CodecType == e_videoH263 || snd_mode.CodecType == e_videoH263plus || snd_mode.CodecType == e_videoH263plus2)
	{
		// Some terminals can't display H263 video in non-standard resolution (at least Polycom HDX8000, Huawei HD 9030 and Polycom RealPresence)
		out_formats_video.push_back(out_format_video);
		force_video_format = true;
	}
	else
	{
		out_formats_video.push_back(out_format_video);
	}

	std::shared_ptr<ffl::AbstractSource> video_head;
	switch (m_fake_video_mode)
	{
	case FVM_DISABLED:
	{
		if (is_groupconf)
		{
			m_conference_source->SetVideoFormat(out_format_video, force_video_format);
			m_conference_source->SetBitrate(max_bitrate);
			video_head = m_conference_source;
		}
		else
		{
			video_head = m_participant_source;
			auto vs_video_unwrapper = ffl::FilterVSFrameUnwrapper::Create(video_head, stream::Track::video);
			vs_video_unwrapper->Persistent(true);
			m_vs_video_unwrapper = vs_video_unwrapper;
			video_head = vs_video_unwrapper;
		}
		video_head = ffl::FilterVideoFormatReader::Create(video_head);
		video_head = ffl::FilterDumpVideo::Create(video_head, "in");
		auto vs_in_video_stat = ffl::FilterStatisticsCalculator::Create(video_head);
		m_vs_in_video_stat = vs_in_video_stat;
		video_head = vs_in_video_stat;
	}
		break;
	case FVM_GROUPCONF_NOPEOPLE:
	case FVM_BROADCAST_INPROGRESS:
	case FVM_NOSPEAKERS:
		video_head = m_vs_banner_source;
		break;
	case FVM_SLIDESHOW:
		video_head = m_vs_slides_source;
		break;
	default:
		dstream1 << "VS_RTPSession(" << m_id << "): unknown fake video mode: " << m_fake_video_mode;
		return;
	}

	rtp_out_payloads_video[out_format_video.dwVideoCodecFCC] = snd_mode.PayloadType;

	// select first format as default
	assert(!out_formats_video.empty());
	const bool allow_upscale = !is_groupconf && force_video_format;
	video_head = ffl::FilterVideoTranscoder::Create(video_head, out_formats_video, allow_upscale, out_formats_video.front(), 0, false);
	video_head->ProcessCommand(ffl::FilterCommand::MakeSetBitrateRequest(max_bitrate));
	if (snd_mode.CodecType == e_videoXH264UC) {
		video_head = ffl::FilterH264SLInjector::Create(video_head);
	}
	auto rtp_out_video_stat = ffl::FilterStatisticsCalculator::Create(video_head);
	m_rtp_out_video_stat = rtp_out_video_stat;
	video_head = rtp_out_video_stat;
	auto rtp_main_video_sink = m_rtp_main_video_sink.lock();
	if (!rtp_main_video_sink)
	{
		//rtp_main_video_sink = ffl::FilterRTPWrapper::Create(video_head, rtp_out_payloads_video);

		auto rtp_wrapper = ffl::FilterRTPWrapper::Create(video_head, rtp_out_payloads_video);
		if (auto ssrc = m_main_video_channel->ssrc_range.first) {
			rtp_wrapper->set_ssrc_override(ssrc);
		}
		rtp_main_video_sink = rtp_wrapper;
	}
	else
	{
		rtp_main_video_sink = video_head->RegisterSinkOrGetCompatible(rtp_main_video_sink);
	}
	m_rtp_main_video_sink = rtp_main_video_sink;
	video_head = rtp_main_video_sink;
	video_head = ffl::FilterDumpRTP::Create(video_head, "rtpwrap-out");
	if (m_parameters->use_uniform_transmit && max_bitrate) {
		auto ut = ffl::FilterUniformTransmit::Create(video_head, (max_bitrate * 1024) / 8);
			m_ut_main_video = ut; video_head = ut;
		video_head = ffl::FilterDumpRTP::Create(video_head, "ut-out");
	}
	video_head->RegisterSinkOrGetCompatible(GetRTPSink(m_main_video_channel->index, true));

	auto ds = dstream3;
	ds << "VS_RTPSession(" << m_id << "): VS2RTP main video (channel=" << m_main_video_channel->index << ") init";
	ds << ": bitrate=" << max_bitrate;
	ds << ", out formats:\n";
	for (const auto& x: rtp_out_payloads_video)
		ds << "\t{codec=\"" << stream_vcodec(x.first) << "\" -> pt=" << static_cast<int>(x.second) << "}\n";
}

void VS_RTPSession::CreateFilters_VS2RTPSlides()
{
	const bool is_groupconf = m_remote_part_id.empty();

	if (!m_slides_video_channel || !m_slides_video_channel->IsSend())
	{
		return;
	}

	ffl::FilterRTPWrapper::codec_pt_map_t rtp_out_payloads_video;
	VS_MediaFormat out_format_video;
	FillMediaFormat(out_format_video, m_slides_video_channel->snd_mode_video, true, is_groupconf);
	if (is_groupconf)
	{
		// Reduce resolution to the first one from following list that is lower than calculated
		static const struct { unsigned w, h; } valid_resolutions[] = { {1920, 1080}, {1280, 720}, {640, 360} };
		auto res_it = std::find_if(std::begin(valid_resolutions), std::end(valid_resolutions), [&](decltype(valid_resolutions[0]) x) {
			return x.w <= out_format_video.dwVideoWidht && x.h <= out_format_video.dwVideoHeight;
		});
		if (res_it == std::end(valid_resolutions))
			--res_it;
		out_format_video.dwVideoWidht = res_it->w;
		out_format_video.dwVideoHeight = res_it->h;
	}
	out_format_video.SetAudio(0, 0, 0);
	if (!out_format_video.IsVideoValid())
	{
		dstream1 << "VS_RTPSession(" << m_id << "): VS2RTP slides (channel=" << m_slides_video_channel->index << ") init failed: can't select media format for snd mode: {CodecType=" << m_slides_video_channel->snd_mode_video.CodecType << "}\n";
		return;
	}
	rtp_out_payloads_video[out_format_video.dwVideoCodecFCC] = m_slides_video_channel->snd_mode_video.PayloadType;

	std::vector<VS_MediaFormat> out_formats_video;
	out_formats_video.push_back(out_format_video);

	unsigned int max_bitrate = m_slides_video_channel->snd_mode_video.Bitrate / 1024;

	std::shared_ptr<ffl::AbstractSource> video_head;
	video_head = m_vs_slides_source;
	video_head = ffl::FilterVideoTranscoder::Create(video_head, out_formats_video, true, out_format_video, 0, true);
	video_head->ProcessCommand(ffl::FilterCommand::MakeSetBitrateRequest(max_bitrate));
	auto rtp_out_slides_stat = ffl::FilterStatisticsCalculator::Create(video_head);
	m_rtp_out_slides_stat = rtp_out_slides_stat;
	video_head = rtp_out_slides_stat;
	video_head = ffl::FilterRTPWrapper::Create(video_head, rtp_out_payloads_video);
	video_head = ffl::FilterDumpRTP::Create(video_head, "rtpwrap-out");
	if (m_parameters->use_uniform_transmit && max_bitrate) {
		auto ut = ffl::FilterUniformTransmit::Create(video_head, (max_bitrate * 1024) / 8);
		m_ut_slides_video = ut; video_head = ut;
		video_head = ffl::FilterDumpRTP::Create(video_head, "ut-out");
	}
	video_head->RegisterSinkOrGetCompatible(GetRTPSink(m_slides_video_channel->index, true));

	auto ds = dstream3;
	ds << "VS_RTPSession(" << m_id << "): VS2RTP slides (channel=" << m_slides_video_channel->index << ") init";
	ds << ": bitrate=" << max_bitrate;
	ds << ", out formats:\n";
	for (const auto& x: rtp_out_payloads_video)
		ds << "\t{codec=\"" << stream_vcodec(x.first) << "\" -> pt=" << static_cast<int>(x.second) << "}\n";
}

void VS_RTPSession::CreateFilters_VS2RTPFECC()
{
	if (!m_data_channel)
		return;
	if (!m_opal_h224_handler)
	{
		m_opal_h224_handler = vs::MakeShared<OpalH224Handler>(
			m_data_channel->snd_mode_data.PayloadType,
			m_data_channel->snd_mode_data.ExtendedCodec
				? VS_Q922Frame::ExtendedCodec
				: VS_Q922Frame::SimpleCodec);
		m_opal_h224_handler->CreateHandlers();
	}
	m_fecc_source = ffl::SourceFECCChannel::Create(m_opal_h224_handler);
	m_fecc_source->RegisterSinkOrGetCompatible(GetRTPSink(m_data_channel->index, true));
}

void VS_RTPSession::CreateFilters_RTP2VSFECC()
{
	if (!m_data_channel)
		return;

	if (!m_opal_h224_handler)
	{
		m_opal_h224_handler = vs::MakeShared<OpalH224Handler>(
			m_data_channel->snd_mode_data.PayloadType,
			m_data_channel->snd_mode_data.ExtendedCodec
				? VS_Q922Frame::ExtendedCodec
				: VS_Q922Frame::SimpleCodec);
		m_opal_h224_handler->CreateHandlers();
	}

	ffl::FilterRTPValidator::pt_set_t payloadTypeSet;
	auto payloadType = m_data_channel->rcv_modes_data.empty()
		? m_data_channel->snd_mode_data.PayloadType
		: m_data_channel->rcv_modes_data.front().PayloadType;
	payloadTypeSet.insert(payloadType);

	if (m_data_channel)
		m_rtp_data_channel = m_rtp->Get(m_data_channel->index);

	auto rtp_source = GetRTPSource(m_data_channel->index, true);
	if (!rtp_source)
		return;

	std::shared_ptr<ffl::AbstractSource> data_head = rtp_source;
	data_head = ffl::FilterDumpRTP::Create(data_head, "rtp-in");
	data_head = ffl::FilterRTPValidator::Create(data_head, payloadTypeSet, true);
	data_head = ffl::FilterRTPSorter::Create(data_head);
	data_head = ffl::FilterDumpRTP::Create(data_head, "rtp-sort-out");

	m_fecc_sink = ffl::SinkFECCChannel::Create(
		[this, myself = weak_from_this()](eFeccRequestType type, int32_t extra_param)
		{
			if (auto locked_myself = myself.lock())
				m_signal_FarEndCameraControl(m_id, type, extra_param);
		},
		[this, myself = weak_from_this()](std::shared_ptr<RTPPacket>&& packet)
		{
			if (auto locked_myself = myself.lock() && m_fecc_source)
				m_fecc_source->ForwardMessage(std::move(packet));
		},
		m_opal_h224_handler
	);
	data_head->RegisterSinkOrGetCompatible(m_fecc_sink);
}

void VS_RTPSession::InitTraceLog()
{
	const auto ffl_trace_flags = m_parameters->ffl_trace_flags;
	if (ffl_trace_flags == 0)
		return;

	if (!m_trace_file_buf.is_open())
	{
		boost::system::error_code ec;
		boost::filesystem::create_directories(m_parameters->log_dir, ec);
		if (ec)
		{
			dstream0 << "VS_RTPSession(" << m_id << "): Can't create directory '" << m_parameters->log_dir << "': " << ec.message();
			return;
		}

		auto log_path = m_parameters->log_dir;
		log_path += "/ffl_trace_";
		log_path += m_id;
		log_path += ".log";
		if (!m_trace_file_buf.open(log_path.c_str(), std::ios_base::out | std::ios_base::trunc))
			return;
	}

	if (!m_trace_log)
		m_trace_log = std::make_shared<ffl::TraceLog>(&m_trace_file_buf, ffl_trace_flags);
	else
		m_trace_log->SetFlags(ffl_trace_flags);

	dstream4 << "VS_RTPSession(" << m_id << "): Trace log enabled: flags=0x" << std::hex << ffl_trace_flags << std::dec;
}

void VS_RTPSession::LogTraceLogInfo()
{
	if (m_parameters->ffl_trace_flags == 0)
		return;

	auto ds = dstream4;
	ds << "VS_RTPSession(" << m_id << "): Trace log chains:\n";

	// VS to RTP
	if (m_participant_source)
		ds << '\t' << m_participant_source->ChainID() << ": \"VS2RTP participant\"\n";
	if (m_conference_source)
		ds << '\t' << m_conference_source->ChainID() << ": \"VS2RTP conference\"\n";
	if (m_vs_slides_source)
		ds << '\t' << m_vs_slides_source->ChainID() << ": \"VS2RTP slides\"\n";
	if (m_vs_banner_source)
		ds << '\t' << m_vs_banner_source->ChainID() << ": \"VS2RTP banner\"\n";

	// RTP to VS
	if (m_vs_sink)
		ds << '\t' << m_vs_sink->ChainID() << ": \"RTP2VS common\"\n";
	if (m_audio_channel)
	{
		auto rtp_source = GetRTPSource(m_audio_channel->index, true);
		if (rtp_source)
			ds << '\t' << rtp_source->ChainID() << ": \"RTP2VS audio\"\n";
	}
	if (m_main_video_channel)
	{
		auto rtp_source = GetRTPSource(m_main_video_channel->index, true);
		if (rtp_source)
			ds << '\t' << rtp_source->ChainID() << ": \"RTP2VS main video\"\n";
	}
	if (m_slides_video_channel)
	{
		auto rtp_source = GetRTPSource(m_slides_video_channel->index, true);
		if (rtp_source)
			ds << '\t' << rtp_source->ChainID() << ": \"RTP2VS slides\"\n";
	}
}
