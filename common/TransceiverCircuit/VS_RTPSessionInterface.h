#pragma once

#include "std-generic/cpplib/string_view.h"
#include "std-generic/asio_fwd.h"

#include <boost/signals2/signal.hpp>
#include <boost/signals2/connection.hpp>

class VS_RTPModuleParameters;
class VS_FFLSourceCollection;
class VS_TransceiverPartsMgr;

struct VS_MediaChannelInfo;
class VS_ClientCaps;
enum FakeVideo_Mode : int;
enum eSDP_ContentType : int;
enum class eFeccRequestType : int;
enum VS_GroupConf_SubType : int;

struct SlideInfo;

class VS_RTPSessionInterface
{
public:
	static std::shared_ptr<VS_RTPSessionInterface> CreateNewSession(
		boost::asio::io_service& ios,
		string_view id,
		string_view our_part_id,
		string_view sess_key,
		const std::shared_ptr<VS_RTPModuleParameters>& parameters,
		const std::shared_ptr<VS_TransceiverPartsMgr>& partsMgr,
		const std::shared_ptr<VS_FFLSourceCollection>& source_collection
	);
	virtual ~VS_RTPSessionInterface() {}

	typedef boost::signals2::signal<void(const std::string& id, const std::vector<VS_MediaChannelInfo>& channels)> SetMediaChannelsSignalType;
	boost::signals2::connection ConnectToSetMediaChannels(const SetMediaChannelsSignalType::slot_type& slot)
	{
		return m_signal_SetMediaChannels.connect(slot);
	}

	typedef boost::signals2::signal<void(const std::string& id)> FullIntraframeRequestSignalType;
	boost::signals2::connection ConnectToFullIntraframeRequest(const FullIntraframeRequestSignalType::slot_type& slot)
	{
		return m_signal_FullIntraframeRequest.connect(slot);
	}

	typedef boost::signals2::signal<void(const std::string& id, eSDP_ContentType content, bool slides_available)> VideoStatusSignalType;
	boost::signals2::connection ConnectToVideoStatus(const VideoStatusSignalType::slot_type& slot)
	{
		return m_signal_VideoStatus.connect(slot);
	}

	typedef boost::signals2::signal<void(const std::string& id, uint32_t value)> DeviceStatusSignalType;
	boost::signals2::connection ConnectToDeviceStatus(const DeviceStatusSignalType::slot_type& slot)
	{
		return m_signal_DeviceStatus.connect(slot);
	}

	typedef boost::signals2::signal<void(const std::string& id, const char* url, const SlideInfo &info)> ShowSlideSignalType;
	boost::signals2::connection ConnectToShowSlide(const ShowSlideSignalType::slot_type& slot)
	{
		return m_signal_ShowSlide.connect(slot);
	}

	typedef boost::signals2::signal<void(const std::string& id)> EndSlideShowSignalType;
	boost::signals2::connection ConnectToEndSlideShow(const EndSlideShowSignalType::slot_type& slot)
	{
		return m_signal_EndSlideShow.connect(slot);
	}

	typedef boost::signals2::signal<void(const std::string& id, eFeccRequestType type, int32_t extra_param)> FarEndCameraControlSignalType;
	boost::signals2::connection ConnectToFarEndCameraControl(const FarEndCameraControlSignalType::slot_type& slot)
	{
		return m_signal_FarEndCameraControl.connect(slot);
	}

	virtual const std::string& GetConferenceName() const = 0;
	virtual const std::string& GetParticipantName() const = 0;

	virtual bool IsReadyToDestroy() = 0;
	virtual void Stop() = 0;
	virtual void SetConference(string_view conf_name, string_view part_id, string_view owner, VS_GroupConf_SubType subtype, const VS_ClientCaps& conference_caps) = 0;
	virtual void SetMediaChannels(const std::vector<VS_MediaChannelInfo>& channels) = 0;
	virtual void FullIntraframeRequest(bool from_rtp) = 0;// from_rtp == true => TC requests key frame from RTP (sip/h323), from_rtp == false => SIP/H323 requests key frame from TC;
	virtual void RestrictBitrateSVC(uint32_t v_bitrate, uint32_t bitrate, uint32_t old_bitrate) = 0;
	virtual void SetFakeVideoMode(FakeVideo_Mode mode) = 0;
	virtual void ShowSlide(const char* url) = 0;
	virtual void SelectVideo(eSDP_ContentType content) = 0;
	virtual void PauseAudio() = 0;
	virtual void ResumeAudio() = 0;
	virtual void PauseVideo() = 0;
	virtual void ResumeVideo() = 0;
	virtual void ContentForward_Pull() = 0;
	virtual void ContentForward_Push() = 0;
	virtual void ContentForward_Stop() = 0;
	virtual void FarEndCameraControl(eFeccRequestType type, int32_t extra_param) = 0;

protected:
	SetMediaChannelsSignalType m_signal_SetMediaChannels;
	FullIntraframeRequestSignalType m_signal_FullIntraframeRequest;
	VideoStatusSignalType m_signal_VideoStatus;
	DeviceStatusSignalType m_signal_DeviceStatus;
	ShowSlideSignalType m_signal_ShowSlide;
	EndSlideShowSignalType m_signal_EndSlideShow;
	FarEndCameraControlSignalType m_signal_FarEndCameraControl;
};