#pragma once

#include "tests/common/GMockOverride.h"
#include "newtransport/Router/IEndpoint.h"

#include <gmock/gmock.h>

namespace transport_test {

class EndpointMock : public transport::IEndpoint
{
public:
	EndpointMock(string_view id)
		: m_id(id)
	{
		using ::testing::_;
		using ::testing::Return;

		ON_CALL(*this, GetId()).WillByDefault(Return(m_id));
	}

	void DelegateTo(IEndpoint* impl)
	{
		using ::testing::_;
		using ::testing::Invoke;

		ON_CALL(*this, Close()).WillByDefault(Invoke(impl, &transport::IEndpoint::Close));
		ON_CALL(*this, Shutdown()).WillByDefault(Invoke(impl, &transport::IEndpoint::Shutdown));

		ON_CALL(*this, ProcessMessage(_)).WillByDefault(Invoke(impl, &transport::IEndpoint::ProcessMessage));
		ON_CALL(*this, SendToPeer(_)).WillByDefault(Invoke(impl, &transport::IEndpoint::SendToPeer));
		ON_CALL(*this, SendPing()).WillByDefault(Invoke(impl, &transport::IEndpoint::SendPing));
		ON_CALL(*this, SendDisconnect()).WillByDefault(Invoke(impl, &transport::IEndpoint::SendDisconnect));

		ON_CALL(*this, GetId()).WillByDefault(Invoke(impl, &transport::IEndpoint::GetId));
		ON_CALL(*this, GetUserId()).WillByDefault(Invoke(impl, &transport::IEndpoint::GetUserId));
		ON_CALL(*this, SetUserId(_)).WillByDefault(Invoke(impl, &transport::IEndpoint::SetUserId));
		ON_CALL(*this, Authorize(_)).WillByDefault(Invoke(impl, &transport::IEndpoint::Authorize));
		ON_CALL(*this, Unauthorize()).WillByDefault(Invoke(impl, &transport::IEndpoint::Unauthorize));
		ON_CALL(*this, IsAuthorized()).WillByDefault(Invoke(impl, &transport::IEndpoint::IsAuthorized));
		ON_CALL(*this, GetHops()).WillByDefault(Invoke(impl, &transport::IEndpoint::GetHops));
		ON_CALL(*this, GetRemoteIp()).WillByDefault(Invoke(impl, &transport::IEndpoint::GetRemoteIp));

		ON_CALL(*this, FillMonitorStruct(_)).WillByDefault(Invoke(impl, &transport::IEndpoint::FillMonitorStruct));
	}

	MOCK_METHOD0_OVERRIDE(Close, void());
	MOCK_METHOD0_OVERRIDE(Shutdown, void());

	MOCK_METHOD1_OVERRIDE(ProcessMessage, void(const transport::Message&));
	MOCK_METHOD1_OVERRIDE(SendToPeer, void(const transport::Message&));
	MOCK_METHOD0_OVERRIDE(SendPing, void());
	MOCK_METHOD0_OVERRIDE(SendDisconnect, void());

	MOCK_METHOD0_OVERRIDE(GetId, string_view());
	MOCK_METHOD0_OVERRIDE(GetUserId, string_view());
	MOCK_METHOD1_OVERRIDE(SetUserId, void(string_view));
	MOCK_METHOD1_OVERRIDE(Authorize, void(string_view));
	MOCK_METHOD0_OVERRIDE(Unauthorize, void());
	MOCK_METHOD0_OVERRIDE(IsAuthorized, bool());
	MOCK_METHOD0_OVERRIDE(GetHops, uint8_t());
	MOCK_METHOD0_OVERRIDE(GetRemoteIp, std::string());

	MOCK_METHOD1_OVERRIDE(FillMonitorStruct, void(transport::Monitor::TmReply::Endpoint&));

private:
	std::string m_id;
};

}
