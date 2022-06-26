#pragma once

#include "tests/common/GMockOverride.h"
#include "../../FakeClient/VS_FakeClient.h"

#include <gmock/gmock.h>

class FakeClientMock : public VS_FakeClient {
protected:
	FakeClientMock(std::unique_ptr<VS_FakeEndpoint> endpoint)
		: VS_FakeClient(std::move(endpoint))
	{
	}
	static void PostConstruct(const std::shared_ptr<VS_FakeClient>& p)
	{
		VS_FakeClient::PostConstruct(p);
	}

public:
	MOCK_METHOD8_OVERRIDE(CreateConference, bool(long /*maxPart*/, VS_Conference_Type /*confType*/, long /*subType*/, const char* /*name*/, const std::string &/*topic*/, const char* /*passwd*/, bool /*is_public*/, const std::string &/*token*/));
	MOCK_METHOD3_OVERRIDE(JoinAsync, bool(const std::string &/*to*/, const VS_ConferenceInfo& /*info*/, const std::string &/*token*/));
};