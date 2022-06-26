#pragma once

#include "VS_EnvelopeRelayMessage.h"

struct VS_MediaChannelInfo;
class VS_ClientCaps;
enum FakeVideo_Mode : int;
enum eSDP_ContentType : int;
struct SlideInfo;
enum class eFeccRequestType : int;
enum VS_GroupConf_SubType : int;

class VS_RTPModuleRelayMessage : public VS_EnvelopeRelayMessage
{
public:
	enum MessageType
	{
		e_InvalidMessageType = 0,
		e_CreateSession,
		e_CreateSessionResponse,
		e_DestroySession,
		e_SetConference,
		e_SetMediaChannels,
		e_FullIntraframeRequest,
		e_SetFakeVideoMode,
		e_ShowSlide,
		e_SelectVideo,
		e_PauseAudio,
		e_ResumeAudio,
		e_PauseVideo,
		e_ResumeVideo,
		e_VideoStatus,
		e_DeviceStatus,
		e_ContentForward_Pull,
		e_ContentForward_Push,
		e_ContentForward_Stop,
		e_FarEndCameraControl
	};

	static const char module_name[];

	VS_RTPModuleRelayMessage();
	virtual ~VS_RTPModuleRelayMessage();

	MessageType GetMessageType() const;

	bool MakeCreateSession(string_view id, string_view part_id, string_view sess_key);
	bool MakeCreateSessionResponse(string_view id);
	bool MakeDestroySession(string_view id);
	bool MakeSetConference(string_view id, string_view conf_name, string_view part_id, string_view owner, VS_GroupConf_SubType subtype, const VS_ClientCaps& conference_caps);
	bool MakeSetMediaChannels(string_view id, const std::vector<VS_MediaChannelInfo>& media_channels);
	bool MakeFullIntraframeRequest(string_view id, bool from_rtp); // from_rtp == true => TC requests key frame from RTP (sip/h323), from_rtp == false => SIP/H323 requests key frame from TC;
	bool MakeSetFakeVideoMode(string_view id, FakeVideo_Mode mode);
	bool MakeShowSlide(string_view id, const char* url, const SlideInfo &info);
	bool MakeEndSlideShow(string_view id);
	bool MakeSelectVideo(string_view id, eSDP_ContentType content);
	bool MakePauseAudio(string_view id);
	bool MakeResumeAudio(string_view id);
	bool MakePauseVideo(string_view id);
	bool MakeResumeVideo(string_view id);
	bool MakeVideoStatus(string_view id, eSDP_ContentType content, bool slides_available);
	bool MakeDeviceStatus(string_view id, uint32_t value);
	bool MakeContentForward_Pull(string_view id);
	bool MakeContentForward_Push(string_view id);
	bool MakeContentForward_Stop(string_view id);
	bool MakeFarEndCameraControl(string_view id, eFeccRequestType type, int32_t extra_param);

	string_view GetID() const;
	string_view GetPartID() const;
	string_view GetConfName() const;
	string_view GetSessionKey() const;
	bool GetDirection() const;
	string_view GetConferenceOwner() const;
	VS_GroupConf_SubType GetConferenceSubType() const;
	VS_ClientCaps GetConferenceCaps() const;
	std::vector<VS_MediaChannelInfo> GetMediaChannels() const;
	FakeVideo_Mode GetFakeVideoMode() const;
	const char* GetSlideURL() const;
	eSDP_ContentType GetContent() const;
	bool GetSlidesAvailable() const;
	uint32_t GetDeviceStatus() const;
	SlideInfo GetSlideInfo() const;
	eFeccRequestType GetFECCRequestType() const;
	int32_t GetFECCExtraParam() const;
private:
	using VS_EnvelopeRelayMessage::SetParam;
	bool SetParam(string_view name, const std::vector<VS_MediaChannelInfo>& val);
	bool SetParam(string_view name, const VS_ClientCaps& val);
};