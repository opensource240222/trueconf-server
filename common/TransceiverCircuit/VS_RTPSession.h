#pragma once

#include "FrameFilterLib/fwd.h"
#include "VS_RTPSessionInterface.h"
#include "tools/SingleGatewayLib/FakeVideo.h"
#include "std/cpplib/VS_ClientCaps.h"

#include <boost/signals2/connection.hpp>

#include "std-generic/compat/memory.h"
#include <atomic>
#include <chrono>
#include <string>
#include <fstream>

class VS_TransceiverParticipant;
class VS_RTPMediaChannels;
class VS_RTP_Channel;
class OpalH224Handler;

class VS_RTPSession
	: public VS_RTPSessionInterface
	, public vs::enable_shared_from_this<VS_RTPSession>
{
public:
	virtual ~VS_RTPSession();

	const std::string& GetConferenceName() const override;
	const std::string& GetParticipantName() const override;

	bool IsReadyToDestroy() override;
	void Stop() override;
	void SetConference(string_view conf_name, string_view part_id, string_view owner, VS_GroupConf_SubType subtype, const VS_ClientCaps& conference_caps) override;
	void SetMediaChannels(const std::vector<VS_MediaChannelInfo>& channels) override;
	void FullIntraframeRequest(bool from_rtp) override; // from_rtp == true => TC requests key frame from RTP (SIP/H323), from_rtp == false => SIP/H323 requests key frame from TC.
	void RestrictBitrateSVC(uint32_t v_bitrate, uint32_t bitrate, uint32_t old_bitrate) override;
	void SetFakeVideoMode(FakeVideo_Mode mode) override;
	void ShowSlide(const char* url) override;
	void SelectVideo(eSDP_ContentType content) override;
	void PauseAudio() override;
	void ResumeAudio() override;
	void PauseVideo() override;
	void ResumeVideo() override;
	void ContentForward_Pull() override;
	void ContentForward_Push() override;
	void ContentForward_Stop() override;
	void FarEndCameraControl(eFeccRequestType type, int32_t extra_param) override;

protected:
	VS_RTPSession(
		boost::asio::io_service &ios,
		string_view id,
		string_view our_part_id,
		string_view sess_key,
		const std::shared_ptr<VS_RTPModuleParameters> &parameters,
		const std::shared_ptr<VS_TransceiverPartsMgr> &partsMgr,
		const std::shared_ptr<VS_FFLSourceCollection> &source_collection
	);

private:
	bool OpenRTPChannel(VS_MediaChannelInfo& channel);
	bool CloseRTPChannel(VS_MediaChannelInfo& channel);
	bool UpdateRTPChannel(VS_MediaChannelInfo& channel);
	std::shared_ptr<ffl::SinkRTPChannel> GetRTPSink(unsigned int id, bool create = false);
	std::shared_ptr<ffl::SourceRTPChannel> GetRTPSource(unsigned int id, bool create = false);
	std::shared_ptr<ffl::AbstractSource> GetUnwrappedRTPAudioSource(VS_MediaChannelInfo* audio_channel);
	std::shared_ptr<ffl::AbstractSource> GetUnwrappedRTPVideoSource(VS_MediaChannelInfo* video_channel);
	void SelectChannels();
	void CreateFilters();
	void CreateFilters_RTPSlidesMonitor();
	void CreateFilters_RTPSlidesEncoder();
	void CreateFilters_RTP2VSAudio();
	void CreateFilters_RTP2VSVideo();
	void CreateFilters_VS2RTPAudio();
	void CreateFilters_VS2RTPVideo();
	void CreateFilters_VS2RTPSlides();
	void CreateFilters_VS2RTPFECC();
	void CreateFilters_RTP2VSFECC();
	void InitTraceLog();
	void LogTraceLogInfo();
	void LogStatistics(unsigned seq_id);
	void LogStatistics_Media();
	void LogStatistics_RTCP();
	void LogStatistics_H235();
	void ScheduleLogStatistics();
	void SendSlide(const std::vector<unsigned char> &buf, const SlideInfo &info);

	std::string m_id;
	std::string m_our_part_id;
	std::string m_conf_name;
	std::string m_remote_part_id;
	std::string m_conf_owner;
	VS_GroupConf_SubType m_conf_subtype;
	std::string m_session_key;
	VS_ClientCaps m_conference_caps;
	std::vector<VS_MediaChannelInfo> m_media_channels;
	eSDP_ContentType m_recv_content;
	VS_MediaChannelInfo* m_audio_channel;
	VS_MediaChannelInfo* m_main_video_channel;
	VS_MediaChannelInfo* m_slides_video_channel;
	VS_MediaChannelInfo* m_data_channel;
	uint32_t m_device_status;
	bool m_stopping;

protected:
	std::unique_ptr<VS_RTPMediaChannels> m_rtp;
	std::shared_ptr<VS_TransceiverParticipant> m_participant;

	std::shared_ptr<VS_RTPModuleParameters> m_parameters;
	std::shared_ptr<VS_TransceiverPartsMgr> m_partsMgr;
	std::shared_ptr<VS_FFLSourceCollection> m_source_collection;

	std::shared_ptr<ffl::SourceFECCChannel> m_fecc_source;
	std::shared_ptr<ffl::SinkFECCChannel> m_fecc_sink;
	std::shared_ptr<ffl::SinkTransceiverParticipant> m_participant_sink;
	std::shared_ptr<ffl::SourceTransceiverParticipant> m_participant_source;
	std::shared_ptr<ffl::SourceMediaPeer> m_conference_source;
	std::map<unsigned int /*id*/, std::shared_ptr<ffl::SinkRTPChannel>> m_rtp_sinks;
	std::map<unsigned int /*id*/, std::shared_ptr<ffl::SourceRTPChannel>> m_rtp_sources;
	std::shared_ptr<ffl::FilterAudioVideoJoiner> m_vs_sink;
	std::shared_ptr<ffl::AbstractSource> m_vs_slides_source;
	std::shared_ptr<ffl::SourceStaticImage> m_vs_banner_source;
	std::weak_ptr<ffl::FilterRTPWrapper> m_rtp_main_video_sink;
	std::weak_ptr<ffl::FilterVSFrameUnwrapper> m_vs_video_unwrapper;
	std::weak_ptr<ffl::FilterUniformTransmit> m_ut_main_video;
	std::weak_ptr<ffl::FilterUniformTransmit> m_ut_slides_video;
	std::weak_ptr<ffl::FilterActivityMonitor> m_rtp_slides_monitor;
	std::weak_ptr<ffl::FilterSlideEncoder> m_rtp_slides_encoder;
	boost::signals2::scoped_connection m_rtp_slides_report_connection;

	std::weak_ptr<ffl::FilterStatisticsCalculator> m_vs_in_audio_stat;
	std::weak_ptr<ffl::FilterStatisticsCalculator> m_vs_in_video_stat;
	std::weak_ptr<ffl::FilterStatisticsCalculator> m_rtp_out_audio_stat;
	std::weak_ptr<ffl::FilterStatisticsCalculator> m_rtp_out_video_stat;
	std::weak_ptr<ffl::FilterStatisticsCalculator> m_rtp_out_slides_stat;
	std::weak_ptr<ffl::FilterStatisticsCalculator> m_rtp_in_audio_stat;
	std::weak_ptr<ffl::FilterStatisticsCalculator> m_rtp_in_video_stat;
	std::weak_ptr<ffl::FilterStatisticsCalculator> m_rtp_in_slides_stat;
	std::weak_ptr<ffl::FilterStatisticsCalculator> m_vs_out_stat;
	std::atomic<unsigned> m_stat_log_seq_id;

	std::weak_ptr<VS_RTP_Channel>					 m_rtp_audio_channel;
	std::weak_ptr<VS_RTP_Channel>					 m_rtp_video_channel;
	std::weak_ptr<VS_RTP_Channel>					 m_rtp_slides_channel;
	std::weak_ptr<VS_RTP_Channel>					 m_rtp_data_channel;

	std::shared_ptr<OpalH224Handler>                 m_opal_h224_handler;

	std::filebuf m_trace_file_buf;
	std::shared_ptr<ffl::TraceLog> m_trace_log;

	FakeVideo_Mode m_fake_video_mode;
	std::chrono::steady_clock::time_point m_slide_expire_time;

};
