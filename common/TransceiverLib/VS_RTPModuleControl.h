#pragma once

#include "VS_RTPModuleControlInterface.h"
#include "VS_RelayModule.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/compat/condition_variable.h"

#include "std-generic/compat/set.h"
#include <mutex>
#include <string>

class VS_RTPModuleControl : public VS_RTPModuleControlInterface, public VS_RelayModule
{
	static const char module_name[];
public:
	VS_RTPModuleControl();
	virtual ~VS_RTPModuleControl();

	bool CreateSession(string_view id, string_view part_id, string_view sess_key) override;
	bool DestroySession(string_view id) override;
	bool SetConference(string_view id, string_view conf_name, string_view part_id, string_view owner, VS_GroupConf_SubType subtype, const VS_ClientCaps& conference_caps) override;
	bool SetMediaChannels(string_view id, const std::vector<VS_MediaChannelInfo>& media_channels) override;
	bool FullIntraframeRequest(string_view id, bool from_rtp) override;
	bool SetFakeVideoMode(string_view id, FakeVideo_Mode mode) override;
	bool ShowSlide(string_view id, const char* url) override;
	bool SelectVideo(string_view id, eSDP_ContentType content) override;
	bool PauseAudio(string_view id) override;
	bool ResumeAudio(string_view id) override;
	bool PauseVideo(string_view id) override;
	bool ResumeVideo(string_view id) override;
	bool ContentForward_Pull(string_view id) override;
	bool ContentForward_Push(string_view id) override;
	bool ContentForward_Stop(string_view id) override;
	bool FarEndCameraControl(string_view id, eFeccRequestType type, int32_t extra_param) override;

	bool CreateSession_sync(string_view id, string_view part_id, string_view sess_key, std::chrono::steady_clock::duration timeout) override;
	bool CreateSession_sync(string_view id, string_view part_id, string_view sess_key, std::chrono::steady_clock::time_point expire_time) override;

private:
	virtual bool ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase>& msg) override;

private:
	std::mutex m_create_session_responses_mutex;
	vs::condition_variable m_create_session_responses_cv;
	vs::set<std::string, vs::str_less> m_create_session_responses;
};
