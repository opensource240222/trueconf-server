#pragma once

#include "std-generic/cpplib/string_view.h"

#include <boost/signals2/signal.hpp>
#include <boost/signals2/connection.hpp>

#include <chrono>
#include <vector>

struct VS_MediaChannelInfo;
class VS_ClientCaps;
enum FakeVideo_Mode : int;
enum eSDP_ContentType : int;
enum class eFeccRequestType : int;
enum VS_GroupConf_SubType : int;

class VS_RTPModuleControlInterface
{
public:
	virtual ~VS_RTPModuleControlInterface() {};

	virtual bool CreateSession(string_view id, string_view part_id, string_view sess_key) = 0;
	virtual bool DestroySession(string_view id) = 0;
	virtual bool SetConference(string_view id, string_view conf_name, string_view part_id, string_view owner, VS_GroupConf_SubType subtype, const VS_ClientCaps& conference_caps) = 0;
	virtual bool SetMediaChannels(string_view id, const std::vector<VS_MediaChannelInfo>& media_channels) = 0;
	virtual bool FullIntraframeRequest(string_view id, bool from_rtp) = 0;
	virtual bool SetFakeVideoMode(string_view id, FakeVideo_Mode mode) = 0;
	virtual bool ShowSlide(string_view id, const char* url) = 0;
	virtual bool SelectVideo(string_view id, eSDP_ContentType content) = 0;
	virtual bool PauseAudio(string_view id) = 0;
	virtual bool ResumeAudio(string_view id) = 0;
	virtual bool PauseVideo(string_view id) = 0;
	virtual bool ResumeVideo(string_view id) = 0;
	virtual bool ContentForward_Pull(string_view id) = 0;
	virtual bool ContentForward_Push(string_view id) = 0;
	virtual bool ContentForward_Stop(string_view id) = 0;
	virtual bool FarEndCameraControl(string_view id, eFeccRequestType type, int32_t extra_param) = 0;

	virtual bool CreateSession_sync(string_view id, string_view part_id, string_view sess_key, std::chrono::steady_clock::duration timeout) = 0;
	virtual bool CreateSession_sync(string_view id, string_view part_id, string_view sess_key, std::chrono::steady_clock::time_point expire_time) = 0;

	typedef boost::signals2::signal<void(string_view id)> CreateSessionResponseSignalType;
	boost::signals2::connection ConnectToCreateSessionResponse(const CreateSessionResponseSignalType::slot_type& slot)
	{
		return m_signal_CreateSessionResponse.connect(slot);
	}

	typedef boost::signals2::signal<void(string_view id, const std::vector<VS_MediaChannelInfo>& media_channels)> SetMediaChannelsSignalType;
	boost::signals2::connection ConnectToSetMediaChannels(const SetMediaChannelsSignalType::slot_type& slot)
	{
		return m_signal_SetMediaChannels.connect(slot);
	}

	typedef boost::signals2::signal<void(string_view id)> FullIntraframeRequestSignalType;
	boost::signals2::connection ConnectToFullIntraframeRequest(const FullIntraframeRequestSignalType::slot_type& slot)
	{
		return m_signal_FullIntraframeRequest.connect(slot);
	}

	typedef boost::signals2::signal<void(string_view id, eSDP_ContentType content, bool slides_available)> VideoStatusSignalType;
	boost::signals2::connection ConnectToVideoStatus(const VideoStatusSignalType::slot_type& slot)
	{
		return m_signal_VideoStatus.connect(slot);
	}

	typedef boost::signals2::signal<void(string_view id, uint32_t value)> DeviceStatusSignalType;
	boost::signals2::connection ConnectToDeviceStatus(const DeviceStatusSignalType::slot_type& slot)
	{
		return m_signal_DeviceStatus.connect(slot);
	}

	typedef boost::signals2::signal<void(string_view id, const char* url, const struct SlideInfo &info)> ShowSlideSignalType;
	boost::signals2::connection ConnectToShowSlide(const ShowSlideSignalType::slot_type &slot)
	{
		return m_signal_ShowSlide.connect(slot);
	}

	typedef boost::signals2::signal<void(string_view id)> EndSlideshowSignalType;
	boost::signals2::connection ConnectToEndSlideShow(const EndSlideshowSignalType::slot_type &slot)
	{
		return m_signal_EndSlideShow.connect(slot);
	}

	typedef boost::signals2::signal<void(string_view id, eFeccRequestType type, int32_t extra_param)> FECCSignalType;
	boost::signals2::connection ConnectToFECC(const FECCSignalType::slot_type &slot)
	{
		return m_signal_FECC.connect(slot);
	}

protected:
	CreateSessionResponseSignalType m_signal_CreateSessionResponse;
	SetMediaChannelsSignalType m_signal_SetMediaChannels;
	FullIntraframeRequestSignalType m_signal_FullIntraframeRequest;
	VideoStatusSignalType m_signal_VideoStatus;
	DeviceStatusSignalType m_signal_DeviceStatus;
	ShowSlideSignalType m_signal_ShowSlide;
	EndSlideshowSignalType m_signal_EndSlideShow;
	FECCSignalType m_signal_FECC;
};