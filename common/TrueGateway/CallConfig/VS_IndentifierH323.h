#pragma once

#include "VS_Indentifier.h"
namespace net {
	class LoggerInterface;
} //namespace net

class VS_IndentifierH323 : public VS_Indentifier
{
	bool IsMyCallId_Impl(string_view callId) const override;
	acs::Response Protocol_Impl(const void* buf, std::size_t bufSz) const override;
	bool Resolve_Impl(VS_CallConfig &cfg, string_view callId, VS_UserData *from) override;
	bool CreateDefaultConfiguration_Impl(VS_CallConfig& cfg, const net::Endpoint &ep, VS_CallConfig::eSignalingProtocol protocol, string_view username) override;
	bool PostResolve_Impl(VS_CallConfig &config, string_view callId, VS_UserData *from, bool block) override;
	VS_CallConfig::eSignalingProtocol GetSignalongProtocol_Impl() const override;
	std::shared_ptr<VS_ParserInterface> CreateParser_Impl(boost::asio::io_service::strand& strand, const std::shared_ptr<net::LoggerInterface>& logger) override;
	void LoadConfigurations_Impl(std::vector<VS_CallConfig> &users, std::vector<VS_CallConfig> &hosts, const char *peerId) override;
	bool AsyncResolveImpl(std::function<void()> &/*resolveTask*/) const override
	{
		return false;
	}
public:
	struct H323CallID final
	{
		explicit H323CallID(string_view callId, const bool removeVisualSeparator = true);
		std::string GetCallId() const;
		std::string GetDTMF() const;

		std::string prefix;
		std::string name;
		std::string host;
		std::string dtmf;
	};
	bool IsTelephone(string_view call_id) const;
	explicit VS_IndentifierH323(boost::asio::io_service &io) : VS_Indentifier(io)
	{}
};