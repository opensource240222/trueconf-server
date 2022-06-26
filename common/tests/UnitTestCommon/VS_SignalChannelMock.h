#pragma once

#include "tests/common/GMockOverride.h"
#include "TrueGateway/net/VS_SignalChannel.h"

#include <gmock/gmock.h>

#include <memory>

#pragma warning (push)
#pragma warning (disable:4373)
class VS_SignalChannelMock : public VS_SignalChannel
{
public:
	VS_SignalChannelMock()
	{
		using ::testing::AnyNumber;

		EXPECT_CALL(*this, LocalAddress()).Times(AnyNumber());
		EXPECT_CALL(*this, RemoteAddress()).Times(AnyNumber());
	}

	void DelegateTo(VS_SignalChannel* impl, std::shared_ptr<void> track = nullptr)
	{
		using ::testing::_;
		using ::testing::Invoke;

		impl->ConnectToChannelOpened(m_signal_ChannelOpened);
		impl->ConnectToChannelClosed(m_signal_ChannelClosed);
		impl->ConnectToDataReceived(m_signal_DataReceived);

		ON_CALL(*this, Open(_, _, _, _, _, _)).WillByDefault(Invoke(impl, static_cast<bool (VS_SignalChannel::*)(unsigned long flags, const net::address& bind_addr, net::port bind_port, const net::address& connect_addr, net::port connect_port, const net::QoSFlowSharedPtr& flow)>(&VS_SignalChannel::Open)));
		ON_CALL(*this, Open(_)).WillByDefault(Invoke(impl, static_cast<bool (VS_SignalChannel::*)(const net::QoSFlowSharedPtr &flow)>(&VS_SignalChannel::Open)));
		ON_CALL(*this, Close(_)).WillByDefault(Invoke(impl, &VS_SignalChannel::Close));
		ON_CALL(*this, Send_mocked(_)).WillByDefault(Invoke([impl](vs::SharedBuffer& buffer) {
			return impl->Send(std::move(buffer));
		}));
		ON_CALL(*this, LocalAddress()).WillByDefault(Invoke(impl, &VS_SignalChannel::LocalAddress));
		ON_CALL(*this, LocalPort()).WillByDefault(Invoke(impl, &VS_SignalChannel::LocalPort));
		ON_CALL(*this, RemoteAddress()).WillByDefault(Invoke(impl, &VS_SignalChannel::RemoteAddress));
		ON_CALL(*this, RemotePort()).WillByDefault(Invoke(impl, &VS_SignalChannel::RemotePort));

		m_track = track;
	}

	MOCK_METHOD6_OVERRIDE(Open, bool(unsigned long flags, const net::address& bind_addr, net::port bind_port, const net::address& connect_addr, net::port connect_port, const net::QoSFlowSharedPtr& flow));
	MOCK_METHOD1_OVERRIDE(Open, bool(const net::QoSFlowSharedPtr &flow));
	MOCK_METHOD1_OVERRIDE(Close, void(bool wait_for_send));
	void Send(vs::SharedBuffer&& buffer) override
	{
		Send_mocked(buffer);
	}
	MOCK_METHOD1(Send_mocked, void(vs::SharedBuffer& buffer));
	MOCK_CONST_METHOD0_OVERRIDE(LocalAddress, net::address());
	MOCK_CONST_METHOD0_OVERRIDE(LocalPort, net::port());
	MOCK_CONST_METHOD0_OVERRIDE(RemoteAddress, net::address());
	MOCK_CONST_METHOD0_OVERRIDE(RemotePort, net::port());

private:
	std::shared_ptr<void> m_track;
};
#pragma warning (pop)