#include "tests/common/GMockOverride.h"
#include "streams_v2/Router/RouterInternalInterface.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace streams_test {

class RouterInternalMock : public stream::RouterInternalInterface
{
public:
	RouterInternalMock()
	{
	}

	void DelegateTo(RouterInternalInterface* impl)
	{
		using ::testing::_;
		using ::testing::Invoke;

		ON_CALL(*this, GetCCS()).WillByDefault(Invoke(impl, &stream::RouterInternalInterface::GetCCS));
		ON_CALL(*this, GetEndpointName()).WillByDefault(Invoke(impl, &stream::RouterInternalInterface::GetEndpointName));
		ON_CALL(*this, GetLogDirectory()).WillByDefault(Invoke(impl, &stream::RouterInternalInterface::GetLogDirectory));
		ON_CALL(*this, GetConference(_)).WillByDefault(Invoke(impl, &stream::RouterInternalInterface::GetConference));
		ON_CALL(*this, DeregisterConference(_)).WillByDefault(Invoke(impl, &stream::RouterInternalInterface::DeregisterConference));
		ON_CALL(*this, NotifyRead(_)).WillByDefault(Invoke(impl, &stream::RouterInternalInterface::NotifyRead));
		ON_CALL(*this, NotifyWrite(_)).WillByDefault(Invoke(impl, &stream::RouterInternalInterface::NotifyWrite));
		ON_CALL(*this, Timer(_)).WillByDefault(Invoke(impl, &stream::RouterInternalInterface::Timer));
	}

	MOCK_METHOD0_OVERRIDE(GetCCS, stream::ConferencesConditions*());
	MOCK_CONST_METHOD0_OVERRIDE(GetEndpointName, const std::string&());
	MOCK_CONST_METHOD0_OVERRIDE(GetLogDirectory, const std::string&());
	MOCK_METHOD1_OVERRIDE(GetConference, conference_ptr(string_view conference_name));
	MOCK_METHOD1_OVERRIDE(DeregisterConference, void(string_view conference_name));
	MOCK_METHOD1_OVERRIDE(NotifyRead, void(size_t bytes));
	MOCK_METHOD1_OVERRIDE(NotifyWrite, void(size_t bytes));
	MOCK_METHOD1_OVERRIDE(Timer, void(std::chrono::steady_clock::time_point now));
};

}
