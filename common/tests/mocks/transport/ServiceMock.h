#pragma once

#include "tests/common/GMockOverride.h"
#include "newtransport/Router/IService.h"

#include <gmock/gmock.h>

#include <string>

namespace transport_test {

class ServiceMock : public transport::IService
{
public:
	ServiceMock(string_view name)
		: m_name(name)
	{
		using ::testing::_;
		using ::testing::Return;

		ON_CALL(*this, GetName()).WillByDefault(Return(m_name));
	}

	void DelegateTo(IService* impl)
	{
		using ::testing::_;
		using ::testing::Invoke;

		ON_CALL(*this, ProcessMessage_mocked(_)).WillByDefault(Invoke([impl](transport::Message& message) {
			return impl->ProcessMessage(std::move(message));
		}));
		ON_CALL(*this, GetName()).WillByDefault(Invoke(impl, &transport::IService::GetName));
		ON_CALL(*this, FillMonitorStruct(_)).WillByDefault(Invoke(impl, &transport::IService::FillMonitorStruct));
		ON_CALL(*this, OnEndpointConnect(_,_,_,_)).WillByDefault(Invoke(impl, &transport::IService::OnEndpointConnect));
		ON_CALL(*this, OnEndpointDisconnect(_,_,_,_)).WillByDefault(Invoke(impl, &transport::IService::OnEndpointDisconnect));
		ON_CALL(*this, OnEndpointIP(_,_)).WillByDefault(Invoke(impl, &transport::IService::OnEndpointIP));
	}

	MOCK_METHOD1(ProcessMessage_mocked, bool(transport::Message& message));
	bool ProcessMessage(transport::Message&& message) override
	{
		return ProcessMessage_mocked(message);
	}
	MOCK_METHOD0_OVERRIDE(GetName, string_view());
	MOCK_METHOD1_OVERRIDE(FillMonitorStruct, void(transport::Monitor::TmReply::Service&));
	MOCK_METHOD4_OVERRIDE(OnEndpointConnect, void(bool, transport::EndpointConnectReason, string_view, string_view));
	MOCK_METHOD4_OVERRIDE(OnEndpointDisconnect, void(bool, transport::EndpointConnectReason, string_view, string_view));
	MOCK_METHOD2_OVERRIDE(OnEndpointIP, void(string_view, string_view));

private:
	std::string m_name;
};

}
