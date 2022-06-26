#pragma once

#include "tests/common/GMockOverride.h"
#include "FakeClient/VS_FakeEndpoint.h"

#include <gmock/gmock.h>

class VS_FakeEndpointMock : public VS_FakeEndpoint
{
public:
	void DelegateTo(VS_FakeEndpoint* impl)
	{
		using ::testing::_;
		using ::testing::Invoke;

		ON_CALL(*this, Stop()).WillByDefault(Invoke(impl, &VS_FakeEndpoint::Stop));
		ON_CALL(*this, SetReceiver(_)).WillByDefault(Invoke(impl, &VS_FakeEndpoint::SetReceiver));
		ON_CALL(*this, CID()).WillByDefault(Invoke(impl, &VS_FakeEndpoint::CID));
		ON_CALL(*this, Send_mocked(_)).WillByDefault(Invoke([impl](transport::Message& message) {
			return impl->Send(std::move(message));
		}));
	}

	MOCK_METHOD0_OVERRIDE(Stop, void ());
	MOCK_METHOD1_OVERRIDE(SetReceiver, void (std::weak_ptr<Receiver> receiver));
	MOCK_CONST_METHOD0_OVERRIDE(CID, const std::string& ());
	MOCK_METHOD1(Send_mocked, bool (transport::Message& message));
	bool Send(transport::Message&& message) override
	{
		return Send_mocked(message);
	}
};

class VS_FakeEndpointReceiverMock : public VS_FakeEndpoint::Receiver
{
public:
	void DelegateTo(VS_FakeEndpoint::Receiver* impl)
	{
		using ::testing::_;
		using ::testing::Invoke;

		ON_CALL(*this, OnReceive(_)).WillByDefault(Invoke(impl, &VS_FakeEndpoint::Receiver::OnReceive));
		ON_CALL(*this, OnError(_)).WillByDefault(Invoke(impl, &VS_FakeEndpoint::Receiver::OnError));
		ON_CALL(*this, Timeout()).WillByDefault(Invoke(impl, &VS_FakeEndpoint::Receiver::Timeout));
	}

	MOCK_METHOD1_OVERRIDE(OnReceive, void (const transport::Message& message));
	MOCK_METHOD1_OVERRIDE(OnError, void (unsigned error));
	MOCK_METHOD0_OVERRIDE(Timeout, void ());
};
