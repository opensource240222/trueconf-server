#pragma once

#include "VS_Indentifier.h"
#include "std-generic/compat/set.h"
#include "std-generic/cpplib/StrCompare.h"

class VS_IndentifierSIP : public VS_Indentifier
{
public:
	VS_IndentifierSIP(boost::asio::io_service &io,const std::string& serverInfo) : VS_Indentifier(io), m_serverInfo(serverInfo)
	{}

	void InitBindAddrSet(vs::set<std::string, vs::str_less>&& bindAddrSet);
private:
	vs::set<std::string, vs::str_less> m_bind_addr_set;
	struct SIPCallID final
	{
		explicit SIPCallID(std::string callId);
		std::string ToCallId() const;
		bool IsValidDTMF(char ch) const;

		std::string orig_call_id;
		std::string prefix;
		std::string name;
		std::string host;
		net::port port;
		std::string dtmf;
		bool use_tcp;
		bool use_tls;
		bool use_udp;
	};

	bool IsMyCallId_Impl(string_view callId) const override;
	acs::Response Protocol_Impl(const void* buf, std::size_t bufSz) const override;

	bool IsVCSCallID(string_view call_id);
	bool Resolve_Impl(VS_CallConfig &cfg, string_view callId, VS_UserData *from) override;
	bool PostResolve_Impl(VS_CallConfig &config, string_view callId, VS_UserData *from, bool block) override;

	void LoadConfigurations_Impl(std::vector<VS_CallConfig> &users, std::vector<VS_CallConfig> &hosts, const char *peerId) override;
	bool ConvertConfiguration_Impl(VS_CallConfig &cfg, string_view tcId, const VS_ExternalAccount &a) override;

	bool CreateDefaultConfiguration_Impl(VS_CallConfig& cfg, const net::Endpoint &ep, VS_CallConfig::eSignalingProtocol protocol,
	                                     string_view username) override;

	bool ResolveThroughDNS(const std::string& host, net::port port, const std::vector<net::protocol>& desiredProtos, net::address &setAddr, net::port &setPort, bool block) const override;
	bool AsyncResolveImpl(std::function<void()> &resolveTask) const override;

	VS_CallConfig::eSignalingProtocol GetSignalongProtocol_Impl() const override;
	std::shared_ptr<VS_ParserInterface> CreateParser_Impl(boost::asio::io_service::strand& strand, const std::shared_ptr<net::LoggerInterface>& logger) override;

	void CleanTelephoneSeparators(SIPCallID &sipCallId) const;
	void SIPCallID_to_ConnectionTypeSeq(const SIPCallID &sci, std::vector<net::protocol>& ConnectionTypeSeq, net::protocol &proto);

	bool IsTelephone(string_view callId) const;
	const std::string m_serverInfo;
};
