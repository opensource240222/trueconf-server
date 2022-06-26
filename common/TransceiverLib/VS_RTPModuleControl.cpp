#include "VS_RTPModuleControl.h"
#include "VS_RTPModuleRelayMessage.h"
#include "tools/Server/VS_MediaChannelInfo.h"
#include "std/cpplib/VS_Protocol.h"

#include <boost/make_shared.hpp>

#include <memory>

VS_RTPModuleControl::VS_RTPModuleControl()
	: VS_RelayModule(VS_RTPModuleRelayMessage::module_name)
{
}

VS_RTPModuleControl::~VS_RTPModuleControl()
{
}

bool VS_RTPModuleControl::CreateSession(string_view id, string_view part_id, string_view sess_key)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakeCreateSession(id, part_id, sess_key);
	return SendMsg(msg);
}

bool VS_RTPModuleControl::DestroySession(string_view id)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakeDestroySession(id);
	return SendMsg(msg);
}

bool VS_RTPModuleControl::SetConference(string_view id, string_view conf_name, string_view part_id, string_view owner, VS_GroupConf_SubType subtype, const VS_ClientCaps& conference_caps)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakeSetConference(id, conf_name, part_id, owner, subtype, conference_caps);
	return SendMsg(msg);
}

bool VS_RTPModuleControl::SetMediaChannels(string_view id, const std::vector<VS_MediaChannelInfo>& media_channels)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakeSetMediaChannels(id, media_channels);
	return SendMsg(msg);
}

bool VS_RTPModuleControl::FullIntraframeRequest(string_view id, bool from_rtp)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakeFullIntraframeRequest(id,from_rtp);
	return SendMsg(msg);
}
bool VS_RTPModuleControl::SetFakeVideoMode(string_view id, FakeVideo_Mode mode)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakeSetFakeVideoMode(id, mode);
	return SendMsg(msg);
}

bool VS_RTPModuleControl::ShowSlide(string_view id, const char* url)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakeShowSlide(id, url, {});
	return SendMsg(msg);
}

bool VS_RTPModuleControl::SelectVideo(string_view id, eSDP_ContentType content)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakeSelectVideo(id, content);
	return SendMsg(msg);
}

bool VS_RTPModuleControl::PauseAudio(string_view id)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakePauseAudio(id);
	return SendMsg(msg);
}

bool VS_RTPModuleControl::ResumeAudio(string_view id)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakeResumeAudio(id);
	return SendMsg(msg);
}

bool VS_RTPModuleControl::PauseVideo(string_view id)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakePauseVideo(id);
	return SendMsg(msg);
}

bool VS_RTPModuleControl::ResumeVideo(string_view id)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakeResumeVideo(id);
	return SendMsg(msg);
}

bool VS_RTPModuleControl::ContentForward_Pull(string_view id)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakeContentForward_Pull(id);
	return SendMsg(msg);
}

bool VS_RTPModuleControl::ContentForward_Push(string_view id)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakeContentForward_Push(id);
	return SendMsg(msg);
}

bool VS_RTPModuleControl::ContentForward_Stop(string_view id)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakeContentForward_Stop(id);
	return SendMsg(msg);
}

bool VS_RTPModuleControl::FarEndCameraControl(string_view id, eFeccRequestType type, int32_t extra_param)
{
	boost::shared_ptr<VS_RTPModuleRelayMessage> msg(boost::make_shared<VS_RTPModuleRelayMessage>());
	msg->MakeFarEndCameraControl(id, type, extra_param);
	return SendMsg(msg);
}

bool VS_RTPModuleControl::CreateSession_sync(string_view id, string_view part_id, string_view sess_key, std::chrono::steady_clock::duration timeout)
{
	return CreateSession_sync(id, part_id, sess_key, std::chrono::steady_clock::now() + timeout);
}

bool VS_RTPModuleControl::CreateSession_sync(string_view id, string_view part_id, string_view sess_key, std::chrono::steady_clock::time_point expire_time)
{
	if (!CreateSession(id, part_id, sess_key))
		return false;

	std::unique_lock<std::mutex> lock(m_create_session_responses_mutex);
	while (true)
	{
		auto it = m_create_session_responses.find(id);
		if (it != m_create_session_responses.end())
		{
			m_create_session_responses.erase(it);
			return true;
		}
		if (m_create_session_responses_cv.wait_until(lock, expire_time) == std::cv_status::timeout)
			return false;
	}
}

bool VS_RTPModuleControl::ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase>& msg_in)
{
	std::unique_ptr<VS_RTPModuleRelayMessage> msg(new VS_RTPModuleRelayMessage);
	if (msg->SetMessage(msg_in->GetMess()))
	{
		switch (msg->GetMessageType())
		{
		case VS_RTPModuleRelayMessage::e_CreateSessionResponse:
			{
				std::lock_guard<std::mutex> lock(m_create_session_responses_mutex);
				m_create_session_responses.emplace(msg->GetID());
			}
			m_create_session_responses_cv.notify_all();
			m_signal_CreateSessionResponse(msg->GetID());
			break;

		case VS_RTPModuleRelayMessage::e_SetMediaChannels:
			m_signal_SetMediaChannels(msg->GetID(), msg->GetMediaChannels());
			break;

		case VS_RTPModuleRelayMessage::e_FullIntraframeRequest:
			m_signal_FullIntraframeRequest(msg->GetID());
			break;

		case VS_RTPModuleRelayMessage::e_VideoStatus:
			m_signal_VideoStatus(msg->GetID(), msg->GetContent(), msg->GetSlidesAvailable());
			break;

		case VS_RTPModuleRelayMessage::e_DeviceStatus:
			m_signal_DeviceStatus(msg->GetID(), msg->GetDeviceStatus());
			break;

		case VS_RTPModuleRelayMessage::e_ShowSlide: {
			const char *url = msg->GetSlideURL();
			if (url) {
				m_signal_ShowSlide(msg->GetID(), url, msg->GetSlideInfo());
			} else {
				m_signal_EndSlideShow(msg->GetID());
			}
			} break;
		case VS_RTPModuleRelayMessage::e_FarEndCameraControl:
//			TODO: Check if works (may be a bit transparent)
			m_signal_FECC(msg->GetID(), msg->GetFECCRequestType(), msg->GetFECCExtraParam());
			break;

		}
	}
	return true;
}
