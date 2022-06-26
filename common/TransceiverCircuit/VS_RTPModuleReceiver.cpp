#include "VS_RTPModuleReceiver.h"
#include "VS_RTPSessionInterface.h"
#include "VS_RTPModuleParameters.h"
#include "VS_FFLSourceCollection.h"
#include "TransceiverLib/VS_RTPModuleRelayMessage.h"
#include "std/cpplib/VS_ThreadPool.h"
#include "std/cpplib/VS_Singleton.h"
#include "std/cpplib/MakeShared.h"
#include "tools/SingleGatewayLib/FakeVideo.h"
#include "std/cpplib/VS_ClientCaps.h"
#include "std/debuglog/VS_Debug.h"
#include "tools/Server/VS_MediaChannelInfo.h"

#include <boost/make_shared.hpp>

#include <chrono>
#include <thread>
#include "std-generic/cpplib/ThreadUtils.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

VS_RTPModuleReceiver::VS_RTPModuleReceiver(boost::asio::io_service& ios, const boost::shared_ptr<VS_MediaSourceCollection>& collection, const std::shared_ptr<VS_TransceiverPartsMgr>& partsMgr)
	: VS_RelayModule(VS_RTPModuleRelayMessage::module_name)
	, m_ios(ios)
	, m_parameters(std::make_shared<VS_RTPModuleParameters>())
	, m_partsMgr(partsMgr)
	, m_source_collection(vs::MakeShared<VS_FFLSourceCollection>(ios, collection))
	, m_deleting(false)
{
	m_parameters->Update();
}

VS_RTPModuleReceiver::~VS_RTPModuleReceiver()
{
	m_deleting = true;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (auto& x: m_sessions)
		{
			x.second->Stop();
			m_to_remove.insert(x.second);
		}
		m_sessions.clear();
	}

	while (ClearOldSessions())
		vs::SleepFor(std::chrono::milliseconds(500));
	VS_Singleton<VS_ThreadPool>::Instance().Stop();
}

bool VS_RTPModuleReceiver::ClearOldSessions()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	for (auto it = m_to_remove.begin(); it != m_to_remove.end();)
	{
		if ((*it)->IsReadyToDestroy())
			it = m_to_remove.erase(it);
		else
			++it;
	}

	return !m_to_remove.empty();
}

bool VS_RTPModuleReceiver::ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase>& msg_in)
{
	ClearOldSessions();

	std::unique_ptr<VS_RTPModuleRelayMessage> msg(new VS_RTPModuleRelayMessage);
	if (msg->SetMessage(msg_in->GetMess()))
	{
		switch (msg->GetMessageType())
		{
		case VS_RTPModuleRelayMessage::e_CreateSession:
			CreateSession(msg->GetID(), msg->GetPartID(), msg->GetSessionKey());
			break;
		case VS_RTPModuleRelayMessage::e_DestroySession:
			DestroySession(msg->GetID());
			break;
		case VS_RTPModuleRelayMessage::e_SetConference:
			SetConference(msg->GetID(), msg->GetConfName(), msg->GetPartID(), msg->GetConferenceOwner(), msg->GetConferenceSubType(), msg->GetConferenceCaps());
			break;
		case VS_RTPModuleRelayMessage::e_SetMediaChannels:
			SetMediaChannels(msg->GetID(), msg->GetMediaChannels());
			break;
		case VS_RTPModuleRelayMessage::e_FullIntraframeRequest:
			FullIntraframeRequest(msg->GetID(),msg->GetDirection());
			break;
		case VS_RTPModuleRelayMessage::e_SetFakeVideoMode:
			SetFakeVideoMode(msg->GetID(), msg->GetFakeVideoMode());
			break;
		case VS_RTPModuleRelayMessage::e_ShowSlide:
			ShowSlide(msg->GetID(), msg->GetSlideURL());
			break;
		case VS_RTPModuleRelayMessage::e_SelectVideo:
			SelectVideo(msg->GetID(), msg->GetContent());
			break;
		case VS_RTPModuleRelayMessage::e_PauseAudio:
			PauseAudio(msg->GetID());
			break;
		case VS_RTPModuleRelayMessage::e_ResumeAudio:
			ResumeAudio(msg->GetID());
			break;
		case VS_RTPModuleRelayMessage::e_PauseVideo:
			PauseVideo(msg->GetID());
			break;
		case VS_RTPModuleRelayMessage::e_ResumeVideo:
			ResumeVideo(msg->GetID());
			break;
		case VS_RTPModuleRelayMessage::e_ContentForward_Pull:
			ContentForward_Pull(msg->GetID());
			break;
		case VS_RTPModuleRelayMessage::e_ContentForward_Push:
			ContentForward_Push(msg->GetID());
			break;
		case VS_RTPModuleRelayMessage::e_ContentForward_Stop:
			ContentForward_Stop(msg->GetID());
			break;
		case VS_RTPModuleRelayMessage::e_FarEndCameraControl:
			FarEndCameraControl(msg->GetID(), msg->GetFECCRequestType(), msg->GetFECCExtraParam());
			break;
		default:
			dprint2("VS_RTPModuleReceiver: unknown relay message: type=%u\n", msg->GetMessageType());
		}
	}
	return true;
}

void VS_RTPModuleReceiver::CreateSession(string_view id, string_view part_id, string_view sess_key)
{
	if (m_deleting)
		return;
	if (id.empty())
		return;

	if (GetSession(id))
	{
		auto msg = boost::make_shared<VS_RTPModuleRelayMessage>();
		msg->MakeCreateSessionResponse(id);
		SendMsg(msg);
	}

	auto session = VS_RTPSessionInterface::CreateNewSession(m_ios, id, part_id, sess_key, m_parameters, m_partsMgr, m_source_collection);
	session->ConnectToSetMediaChannels([this](const std::string& id, const std::vector<VS_MediaChannelInfo>& channels) {
		const auto msg = boost::make_shared<VS_RTPModuleRelayMessage>();
		msg->MakeSetMediaChannels(id, channels);
		SendMsg(msg);
	});
	session->ConnectToFullIntraframeRequest([this](const std::string& id) {
		const auto msg = boost::make_shared<VS_RTPModuleRelayMessage>();
		msg->MakeFullIntraframeRequest(id, true);
		SendMsg(msg);
	});
	session->ConnectToVideoStatus([this](const std::string& id, eSDP_ContentType content, bool slides_available) {
		const auto msg = boost::make_shared<VS_RTPModuleRelayMessage>();
		msg->MakeVideoStatus(id, content, slides_available);
		SendMsg(msg);
	});
	session->ConnectToDeviceStatus([this](const std::string& id, uint32_t value) {
		const auto msg = boost::make_shared<VS_RTPModuleRelayMessage>();
		msg->MakeDeviceStatus(id, value);
		SendMsg(msg);
	});
	session->ConnectToShowSlide([this](const std::string& id, const char *url, const SlideInfo &info) {
		const auto msg = boost::make_shared<VS_RTPModuleRelayMessage>();
		msg->MakeShowSlide(id, url, info);
		SendMsg(msg);
	});
	session->ConnectToEndSlideShow([this](const std::string& id) {
		const auto msg = boost::make_shared<VS_RTPModuleRelayMessage>();
		msg->MakeEndSlideShow(id);
		SendMsg(msg);
	});
	session->ConnectToFarEndCameraControl([this](const std::string& id, eFeccRequestType type, int32_t extra_param) {
		const auto msg = boost::make_shared<VS_RTPModuleRelayMessage>();
		msg->MakeFarEndCameraControl(id, type, extra_param);
		SendMsg(msg);
	});

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_sessions.emplace(id, session);
	}

	auto msg = boost::make_shared<VS_RTPModuleRelayMessage>();
	msg->MakeCreateSessionResponse(id);
	SendMsg(msg);
}

void VS_RTPModuleReceiver::DestroySession(string_view id)
{
	if (m_deleting)
		return;
	if (id.empty())
		return;

	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_sessions.find(id);
	if (it == m_sessions.end())
		return;
	m_sessions_by_part.erase(std::make_tuple(it->second->GetConferenceName(), it->second->GetParticipantName()));
	it->second->Stop();
	if (!it->second->IsReadyToDestroy())
		m_to_remove.insert(it->second);
	m_sessions.erase(it);
}

std::shared_ptr<VS_RTPSessionInterface> VS_RTPModuleReceiver::GetSession(string_view id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_sessions.find(id);
	if (it == m_sessions.end())
		return nullptr;
	return it->second;
}

std::shared_ptr<VS_RTPSessionInterface> VS_RTPModuleReceiver::GetSession(const char* conf_name, const char* part_id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_sessions_by_part.find(std::make_tuple(conf_name, part_id));
	if (it == m_sessions_by_part.end())
		return nullptr;
	return it->second.lock();
}

void VS_RTPModuleReceiver::SetConference(string_view id, string_view conf_name, string_view part_id, string_view owner, VS_GroupConf_SubType subtype, const VS_ClientCaps& conference_caps)
{
	const auto session = GetSession(id);
	if (!session)
		return;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_sessions_by_part.erase(std::make_tuple(session->GetConferenceName(), session->GetParticipantName()));
		m_sessions_by_part[std::make_tuple(std::string(conf_name), session->GetParticipantName())] = session;
	}
	session->SetConference(conf_name, part_id, owner, subtype, conference_caps);
}

void VS_RTPModuleReceiver::SetMediaChannels(string_view id, const std::vector<VS_MediaChannelInfo>& media_channels)
{
	const auto session = GetSession(id);
	if (!session)
		return;
	session->SetMediaChannels(media_channels);
}

void VS_RTPModuleReceiver::FullIntraframeRequest(string_view id, bool from_rtp)
{
	const auto session = GetSession(id);
	if (!session)
		return;
	session->FullIntraframeRequest(from_rtp);
}

void VS_RTPModuleReceiver::SetFakeVideoMode(string_view id, FakeVideo_Mode mode)
{
	const auto session = GetSession(id);
	if (!session)
		return;
	session->SetFakeVideoMode(mode);
}

void VS_RTPModuleReceiver::ShowSlide(string_view id, const char* url)
{
	const auto session = GetSession(id);
	if (!session)
		return;
	session->ShowSlide(url);
}

void VS_RTPModuleReceiver::SelectVideo(string_view id, eSDP_ContentType content)
{
	const auto session = GetSession(id);
	if (!session)
		return;
	session->SelectVideo(content);
}

void VS_RTPModuleReceiver::PauseAudio(string_view id)
{
	const auto session = GetSession(id);
	if (!session)
		return;
	session->PauseAudio();
}

void VS_RTPModuleReceiver::ResumeAudio(string_view id)
{
	const auto session = GetSession(id);
	if (!session)
		return;
	session->ResumeAudio();
}

void VS_RTPModuleReceiver::PauseVideo(string_view id)
{
	const auto session = GetSession(id);
	if (!session)
		return;
	session->PauseVideo();
}

void VS_RTPModuleReceiver::ResumeVideo(string_view id)
{
	const auto session = GetSession(id);
	if (!session)
		return;
	session->ResumeVideo();
}

void VS_RTPModuleReceiver::ContentForward_Pull(string_view id)
{
	const auto session = GetSession(id);
	if (!session)
		return;
	session->ContentForward_Pull();
}

void VS_RTPModuleReceiver::ContentForward_Push(string_view id)
{
	const auto session = GetSession(id);
	if (!session)
		return;
	session->ContentForward_Push();
}

void VS_RTPModuleReceiver::ContentForward_Stop(string_view id)
{
	const auto session = GetSession(id);
	if (!session)
		return;
	session->ContentForward_Stop();
}

void VS_RTPModuleReceiver::FarEndCameraControl(string_view id, eFeccRequestType type, int32_t extra_param)
{
	const auto session = GetSession(id);
	if (!session)
		return;
	session->FarEndCameraControl(type, extra_param);
}

void VS_RTPModuleReceiver::StartConference(const char* conf_name)
{
}

void VS_RTPModuleReceiver::StartConference(const char* conf_name, const char* owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type)
{
}

void VS_RTPModuleReceiver::StopConference(const char* conf_name)
{
	m_source_collection->CleanConference(conf_name);
}

void VS_RTPModuleReceiver::ParticipantConnect(const char* conf_name, const char* part_name)
{
}

void VS_RTPModuleReceiver::ParticipantDisconnect(const char* conf_name, const char* part_name)
{
}

void VS_RTPModuleReceiver::SetParticipantCaps(const char* conf_name, const char* part_name, const void *caps_buf, const unsigned long buf_sz)
{
}

void VS_RTPModuleReceiver::RestrictBitrateSVC(const char* conferenceName, const char* participantName, long v_bitrate, long bitrate, long old_bitrate)
{
	const auto session = GetSession(conferenceName, participantName);
	if (!session)
		return;
	session->RestrictBitrateSVC(v_bitrate, bitrate, old_bitrate);
}

void VS_RTPModuleReceiver::RequestKeyFrame(const char *conferenceName, const char * participantName)
{
	const auto session = GetSession(conferenceName, participantName);
	if (session)
		session->FullIntraframeRequest(true);
}
