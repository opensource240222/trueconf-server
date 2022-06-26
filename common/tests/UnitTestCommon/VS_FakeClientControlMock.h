#pragma once

#include "tests/common/GMockOverride.h"
#include "TrueGateway/clientcontrols/VS_FakeClientControl.h"
#include "TrueGateway/clientcontrols/VS_TranscoderLogin.h"
#include "TransceiversPoolFake.h"
#include "std/cpplib/MakeShared.h"

#include <gmock/gmock.h>

struct FakeClientControlMock : public VS_FakeClientControl {
	FakeClientControlMock()
		: VS_FakeClientControl(std::make_shared<test::TransceiversPoolFake>(), vs::MakeShared<VS_TranscoderLogin>())
	{}
	MOCK_METHOD6_OVERRIDE(InviteMethod, VS_CallInviteStatus(string_view fromId, string_view toId_, const VS_ConferenceInfo & info, bool isIpv4, bool newSession, bool forceCreate));
};