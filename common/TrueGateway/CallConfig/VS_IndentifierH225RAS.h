#pragma once

#include "VS_Indentifier.h"
namespace net {
	class LoggerInterface;
} //namespace net

class VS_IndentifierH225RAS : public VS_Indentifier
{
	bool IsMyCallId_Impl(string_view /*callId*/) const override
	{
		return false;
	}
	acs::Response Protocol_Impl(const void* buf, std::size_t bufSz) const override;
	bool Resolve_Impl(VS_CallConfig& /*cfg*/,  string_view /*callId*/, VS_UserData* /*from*/) override
	{
		return false;
	}
	bool CreateDefaultConfiguration_Impl(VS_CallConfig &/*cfg*/, const net::Endpoint& /*ep*/,
		VS_CallConfig::eSignalingProtocol /*protocol*/, string_view /*username*/ = {}) override
	{
		return false;
	}
	bool PostResolve_Impl(VS_CallConfig &config, string_view callId, VS_UserData *from,bool block) override;
	VS_CallConfig::eSignalingProtocol GetSignalongProtocol_Impl() const override;
	std::shared_ptr<VS_ParserInterface> CreateParser_Impl(boost::asio::io_service::strand& strand, const std::shared_ptr<net::LoggerInterface>& logger) override;
	void LoadConfigurations_Impl(std::vector<VS_CallConfig> &users, std::vector<VS_CallConfig> &hosts, const char *peerId) override;
	bool AsyncResolveImpl(std::function<void()> &resolve_task) const override;
public:
	explicit VS_IndentifierH225RAS(boost::asio::io_service &io) : VS_Indentifier(io)
	{}
};