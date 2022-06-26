#include "VS_RTPModuleRelayMessage.h"

#include "tools/Server/CommonTypes.h"
#include "tools/SingleGatewayLib/FakeVideo.h"
#include "std/cpplib/VS_ClientCaps.h"
#include "tools/Server/VS_MediaChannelInfo.h"

const char VS_RTPModuleRelayMessage::module_name[] = "RTPModule";

VS_RTPModuleRelayMessage::VS_RTPModuleRelayMessage() : VS_EnvelopeRelayMessage(module_name)
{
}

VS_RTPModuleRelayMessage::~VS_RTPModuleRelayMessage()
{
}

bool VS_RTPModuleRelayMessage::SetParam(string_view name, const std::vector<VS_MediaChannelInfo>& val)
{
	VS_Container cnt;
	for (const auto& channel: val)
		channel.Serialize(cnt, "MediaChannel");
	return SetParam(name, cnt);
}

bool VS_RTPModuleRelayMessage::SetParam(string_view name, const VS_ClientCaps& val)
{
	void* data(nullptr);
	size_t size(0);
	val.Get(data, size);
	bool res = SetParam(name, data, size);
	free(data);
	return res;
}

bool VS_RTPModuleRelayMessage::MakeCreateSession(string_view id, string_view part_id, string_view sess_key)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_CreateSession))
		|| !SetParam("ID", id)
		|| !SetParam("PartId", part_id)
		|| !SetParam("SessionKey", sess_key)
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakeCreateSessionResponse(string_view id)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_CreateSessionResponse))
		|| !SetParam("ID", id)
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakeDestroySession(string_view id)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_DestroySession))
		|| !SetParam("ID", id)
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakeSetConference(string_view id, string_view conf_name, string_view part_id, string_view owner, VS_GroupConf_SubType subtype, const VS_ClientCaps& conference_caps)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_SetConference))
		|| !SetParam("ID", id)
		|| !SetParam("ConfName", conf_name)
		|| !SetParam("PartID", part_id)
		|| !SetParam("ConferenceOwner", owner)
		|| !SetParam("ConferenceSubType", subtype)
		|| !SetParam("ConferenceCaps", conference_caps)
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakeSetMediaChannels(string_view id, const std::vector<VS_MediaChannelInfo>& media_channels)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_SetMediaChannels))
		|| !SetParam("ID", id)
		|| !SetParam("MediaChannels", media_channels)
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakeFullIntraframeRequest(string_view id, bool from_rtp)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_FullIntraframeRequest))
		|| !SetParam("Direction",from_rtp)
		|| !SetParam("ID", id)
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakeSetFakeVideoMode(string_view id, FakeVideo_Mode mode)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_SetFakeVideoMode))
		|| !SetParam("ID", id)
		|| !SetParam("FakeVideoMode", static_cast<int32_t>(mode))
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakeShowSlide(string_view id, const char* url, const SlideInfo &info)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_ShowSlide))
		|| !SetParam("ID", id)
		|| (url && !SetParam("SlideURL", url))
		)
		return false;

	VS_Container cnt;
	if (!info.Serialize(cnt)) {
		return false;
	}
	SetParam("SlideInfo", cnt);

	return Make();
}

bool VS_RTPModuleRelayMessage::MakeEndSlideShow(string_view id)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_ShowSlide))
		|| !SetParam("ID", id)
		)
		return false;

	return Make();
}

bool VS_RTPModuleRelayMessage::MakeSelectVideo(string_view id, eSDP_ContentType content)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_SelectVideo))
		|| !SetParam("ID", id)
		|| !SetParam("Content", static_cast<int32_t>(content))
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakePauseAudio(string_view id)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_PauseAudio))
		|| !SetParam("ID", id)
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakeResumeAudio(string_view id)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_ResumeAudio))
		|| !SetParam("ID", id)
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakePauseVideo(string_view id)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_PauseVideo))
		|| !SetParam("ID", id)
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakeResumeVideo(string_view id)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_ResumeVideo))
		|| !SetParam("ID", id)
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakeVideoStatus(string_view id, eSDP_ContentType content, bool slides_available)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_VideoStatus))
		|| !SetParam("ID", id)
		|| !SetParam("Content", static_cast<int32_t>(content))
		|| !SetParam("SlidesAvailable", slides_available)
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakeDeviceStatus(string_view id, uint32_t value)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_DeviceStatus))
		|| !SetParam("ID", id)
		|| !SetParam("DeviceStatus", static_cast<int32_t>(value))
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakeContentForward_Pull(string_view id)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_ContentForward_Pull))
		|| !SetParam("ID", id)
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakeContentForward_Push(string_view id)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_ContentForward_Push))
		|| !SetParam("ID", id)
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakeContentForward_Stop(string_view id)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_ContentForward_Stop))
		|| !SetParam("ID", id)
		)
		return false;
	return Make();
}

bool VS_RTPModuleRelayMessage::MakeFarEndCameraControl(string_view id, eFeccRequestType type, int32_t extra_param)
{
	ClearContainer();
	if (!SetParam("Type", static_cast<int32_t>(e_FarEndCameraControl)) ||
		!SetParam("ID", id) ||
		!SetParam("FECC request type", static_cast<int32_t>(type)) ||
		!SetParam("FECC extra param", extra_param))
		return false;
	return Make();
}

VS_RTPModuleRelayMessage::MessageType VS_RTPModuleRelayMessage::GetMessageType() const
{
	int32_t type;
	if (GetParam("Type", type))
		return static_cast<MessageType>(type);
	return e_InvalidMessageType;
}

string_view VS_RTPModuleRelayMessage::GetID() const
{
	return GetStrValueView("ID");
}

string_view VS_RTPModuleRelayMessage::GetPartID() const
{
	return GetStrValueView("PartID");
}

string_view VS_RTPModuleRelayMessage::GetConfName() const
{
	return GetStrValueView("ConfName");
}

string_view VS_RTPModuleRelayMessage::GetSessionKey() const
{
	return GetStrValueView("SessionKey");
}

string_view VS_RTPModuleRelayMessage::GetConferenceOwner() const
{
	return GetStrValueView("ConferenceOwner");
}

VS_GroupConf_SubType VS_RTPModuleRelayMessage::GetConferenceSubType() const
{
	int32_t val;
	if (!GetParam("ConferenceSubType", val))
		return GCST_UNDEF;
	return static_cast<VS_GroupConf_SubType>(val);
}

VS_ClientCaps VS_RTPModuleRelayMessage::GetConferenceCaps() const
{
	VS_ClientCaps val;
	size_t size;
	const void* data = GetBinValRef("ConferenceCaps", size);
	if (!data)
		return val;
	val.Set(data, size);
	return val;
}


std::vector<VS_MediaChannelInfo> VS_RTPModuleRelayMessage::GetMediaChannels() const
{
	std::vector<VS_MediaChannelInfo> val;

	VS_Container cnt;
	if (!GetParam("MediaChannels", cnt))
		return val;

	while (cnt.Next())
	{
		if (strcmp(cnt.GetName(), "MediaChannel") == 0)
		{
			val.emplace_back(0);
			if (!val.back().Deserialize(cnt, nullptr))
				return val;
		}
	}
	return val;
}

FakeVideo_Mode VS_RTPModuleRelayMessage::GetFakeVideoMode() const
{
	int32_t val;
	if (!GetParam("FakeVideoMode", val))
		return FVM_DISABLED;
	return static_cast<FakeVideo_Mode>(val);
}

const char* VS_RTPModuleRelayMessage::GetSlideURL() const
{
	return GetStrValRef("SlideURL");
}

eSDP_ContentType VS_RTPModuleRelayMessage::GetContent() const
{
	int32_t val;
	if (!GetParam("Content", val))
		return SDP_CONTENT_MAIN;
	return static_cast<eSDP_ContentType>(val);
}

bool VS_RTPModuleRelayMessage::GetSlidesAvailable() const
{
	bool val(false);
	if (!GetParam("SlidesAvailable", val))
		return false;
	return val;
}

uint32_t VS_RTPModuleRelayMessage::GetDeviceStatus() const
{
	int32_t val(0);
	if (!GetParam("DeviceStatus", val))
		return 0;
	return static_cast<uint32_t>(val);
}

SlideInfo VS_RTPModuleRelayMessage::GetSlideInfo() const
{
	SlideInfo ret;

	VS_Container cnt;
	if (GetParam("SlideInfo", cnt)) {
		ret.Deserialize(cnt);
	}

	return ret;
}

eFeccRequestType VS_RTPModuleRelayMessage::GetFECCRequestType() const
{
	int32_t ret;
	if (!GetParam("FECC request type", ret))
		return eFeccRequestType::NONE;
	return static_cast<eFeccRequestType>(ret);
}

int32_t VS_RTPModuleRelayMessage::GetFECCExtraParam() const
{
	int32_t ret;
	if (!GetParam("FECC extra param", ret))
		return 0;
	return ret;
}

bool VS_RTPModuleRelayMessage::GetDirection() const
{
	bool direction(false);
	assert(GetParam("Direction", direction));
	return direction;
}
