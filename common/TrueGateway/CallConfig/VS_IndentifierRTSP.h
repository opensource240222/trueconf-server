#include "VS_Indentifier.h"

namespace net {
	class LoggerInterface;
} //namespace net

class VS_IndentifierRTSP : public VS_Indentifier
{
public:
	struct RTSPCallID final
	{
		explicit RTSPCallID(string_view call_id);
		std::string GetCallId() const;

		string_view login;
		string_view password;
		string_view hostname;
		string_view port;
		string_view path;
	private:
		std::string url;
	};

private:
	bool IsMyCallId_Impl(string_view callId) const override;
	acs::Response Protocol_Impl(const void* buf, std::size_t bufSz) const override;
	bool Resolve_Impl(VS_CallConfig &cfg, string_view callId, VS_UserData *from) override;
	bool CreateDefaultConfiguration_Impl(VS_CallConfig& cfg, const net::Endpoint &ep, VS_CallConfig::eSignalingProtocol protocol, string_view username) override;
	bool PostResolve_Impl(VS_CallConfig &config, string_view callId, VS_UserData *from,bool block) override;

	std::shared_ptr<VS_ParserInterface> CreateParser_Impl(boost::asio::io_service::strand& strand, const std::shared_ptr<net::LoggerInterface>& logger) override;

	bool AsyncResolveImpl(std::function<void()> &/*resolveTask*/) const override
	{
		return false;
	}

public:
	VS_IndentifierRTSP(boost::asio::io_service &io) : VS_Indentifier(io) {}
};