#pragma once

#include <gmock/gmock.h>

#include "tests/common/GMockOverride.h"
#include "TransceiverLib/VS_RTPModuleControlInterface.h"
#include "tools/SingleGatewayLib/FakeVideo.h"
#include "SIPParserBase/VS_Const.h"
#include "tools/Server/VS_MediaChannelInfo.h"

struct RTPModuleControlMock : public VS_RTPModuleControlInterface {
	RTPModuleControlMock()
	{
		using ::testing::_;
		using ::testing::Return;

		ON_CALL(*this, CreateSession(_,_,_)).WillByDefault(Return(true));
		ON_CALL(*this, DestroySession(_)).WillByDefault(Return(true));
		ON_CALL(*this, SetConference(_,_,_,_,_,_)).WillByDefault(Return(true));
		ON_CALL(*this, SetMediaChannels(_,_)).WillByDefault(Return(true));
		ON_CALL(*this, FullIntraframeRequest(_,_)).WillByDefault(Return(true));
		ON_CALL(*this, SetFakeVideoMode(_,_)).WillByDefault(Return(true));
		ON_CALL(*this, ShowSlide(_,_)).WillByDefault(Return(true));
		ON_CALL(*this, SelectVideo(_,_)).WillByDefault(Return(true));
		ON_CALL(*this, PauseAudio(_)).WillByDefault(Return(true));
		ON_CALL(*this, ResumeAudio(_)).WillByDefault(Return(true));
		ON_CALL(*this, PauseVideo(_)).WillByDefault(Return(true));
		ON_CALL(*this, ResumeVideo(_)).WillByDefault(Return(true));
		ON_CALL(*this, ContentForward_Pull(_)).WillByDefault(Return(true));
		ON_CALL(*this, ContentForward_Push(_)).WillByDefault(Return(true));
		ON_CALL(*this, ContentForward_Stop(_)).WillByDefault(Return(true));
		ON_CALL(*this, FarEndCameraControl(_,_,_)).WillByDefault(Return(true));
	}

	MOCK_METHOD3_OVERRIDE(CreateSession,bool (string_view id, string_view part_id, string_view sess_key));
	MOCK_METHOD1_OVERRIDE(DestroySession,bool (string_view id));
	MOCK_METHOD6_OVERRIDE(SetConference, bool (string_view id, string_view conf_name, string_view part_id, string_view owner, VS_GroupConf_SubType subtype, const VS_ClientCaps& conference_caps));
	MOCK_METHOD2_OVERRIDE(SetMediaChannels, bool (string_view id, const std::vector<VS_MediaChannelInfo>& media_channels));
	MOCK_METHOD2_OVERRIDE(FullIntraframeRequest, bool (string_view id, bool from_rtp));
	MOCK_METHOD2_OVERRIDE(SetFakeVideoMode, bool (string_view id, FakeVideo_Mode mode));
	MOCK_METHOD2_OVERRIDE(ShowSlide, bool (string_view id, const char* url));
	MOCK_METHOD2_OVERRIDE(SelectVideo, bool (string_view id, eSDP_ContentType content));
	MOCK_METHOD1_OVERRIDE(PauseAudio, bool (string_view id));
	MOCK_METHOD1_OVERRIDE(ResumeAudio, bool (string_view id));
	MOCK_METHOD1_OVERRIDE(PauseVideo, bool (string_view id));
	MOCK_METHOD1_OVERRIDE(ResumeVideo, bool (string_view id));
	MOCK_METHOD1_OVERRIDE(ContentForward_Pull, bool (string_view id));
	MOCK_METHOD1_OVERRIDE(ContentForward_Push, bool (string_view id));
	MOCK_METHOD1_OVERRIDE(ContentForward_Stop, bool (string_view id));
	MOCK_METHOD3_OVERRIDE(FarEndCameraControl, bool (string_view id, eFeccRequestType type, int32_t extra_param));

	MOCK_METHOD4_OVERRIDE(CreateSession_sync, bool (string_view id, string_view part_id, string_view sess_key, std::chrono::steady_clock::duration timeout));
	MOCK_METHOD4_OVERRIDE(CreateSession_sync, bool (string_view id, string_view part_id, string_view sess_key, std::chrono::steady_clock::time_point expire_time));
};