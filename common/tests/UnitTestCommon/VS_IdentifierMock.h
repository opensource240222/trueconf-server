#pragma once

#include "TrueGateway/CallConfig/VS_Indentifier.h"
#include "std/cpplib/VS_ExternalAccount.h"

#include <gmock/gmock.h>

#pragma warning (push)
#pragma warning (disable:4373)
namespace net { class LoggerInterface; }
class VS_IndentifierMock : public VS_Indentifier
{
public:

	explicit VS_IndentifierMock(boost::asio::io_service& io)
		: VS_Indentifier(io)
	{
	}

	void DelegateTo(VS_Indentifier* impl)
	{
		using ::testing::_;
		using ::testing::Invoke;

		ON_CALL(*this, IsMyCallId_Impl(_)).WillByDefault(Invoke(impl, &VS_Indentifier::IsMyCallId));
		ON_CALL(*this, Protocol_Impl(_, _)).WillByDefault(Invoke(impl, &VS_Indentifier::Protocol));
		ON_CALL(*this, Resolve_Impl(_, _, _)).WillByDefault(Invoke(impl, &VS_Indentifier::Resolve));
		ON_CALL(*this, PostResolve_Impl(_, _, _, _)).WillByDefault(Invoke(impl, &VS_Indentifier::PostResolve));
		ON_CALL(*this, LoadConfigurations_Impl(_, _, _)).WillByDefault(Invoke(impl, &VS_Indentifier::LoadConfigurations));
		ON_CALL(*this, ConvertConfiguration_Impl(_, _, _)).WillByDefault(Invoke(impl, &VS_Indentifier::ConvertConfiguration));
		ON_CALL(*this, CreateDefaultConfiguration_Impl(_, _, _, _)).WillByDefault(Invoke(impl, &VS_Indentifier::CreateDefaultConfiguration));
	}

	MOCK_CONST_METHOD1(IsMyCallId, bool (string_view));
	MOCK_CONST_METHOD2(Protocol, acs::Response (const void*, std::size_t));
	MOCK_CONST_METHOD0_OVERRIDE(GetSignalongProtocol_Impl, VS_CallConfig::eSignalingProtocol ());
	MOCK_CONST_METHOD6_OVERRIDE(ResolveThroughDNS, bool (const std::string&, net::port, const std::vector<net::protocol>&, net::address&, net::port&, bool));
	MOCK_CONST_METHOD1_OVERRIDE(AsyncResolveImpl, bool (std::function<void()>&));
	MOCK_METHOD0_OVERRIDE(LoadVoipProtocolConfiguration, void ());
	MOCK_CONST_METHOD1_OVERRIDE(IsMyCallId_Impl, bool (string_view));
	MOCK_CONST_METHOD2_OVERRIDE(Protocol_Impl, acs::Response (const void*, std::size_t));
	MOCK_METHOD3_OVERRIDE(Resolve_Impl, bool (VS_CallConfig&, string_view, VS_UserData*));
	MOCK_METHOD2_OVERRIDE(CreateParser_Impl, std::shared_ptr<VS_ParserInterface> (boost::asio::io_service::strand& strand, const std::shared_ptr<net::LoggerInterface>& logger));
	MOCK_METHOD3_OVERRIDE(LoadConfigurations_Impl, void (std::vector<VS_CallConfig>&, std::vector<VS_CallConfig>&, const char*));
	MOCK_METHOD3_OVERRIDE(ConvertConfiguration_Impl, bool (VS_CallConfig&, string_view, const VS_ExternalAccount&));
	MOCK_METHOD4_OVERRIDE(CreateDefaultConfiguration_Impl, bool (VS_CallConfig&, const net::Endpoint&, VS_CallConfig::eSignalingProtocol, string_view));
	MOCK_METHOD4_OVERRIDE(PostResolve_Impl, bool (VS_CallConfig&, string_view, VS_UserData*, bool));
};
#pragma warning (pop)