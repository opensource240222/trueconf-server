#pragma once

#include "std-generic/cpplib/string_view.h"
#include "std-generic/asio_fwd.h"
#include "acs_v2/Responses.h"
#include "VS_CallConfig.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>

class VS_ParserInterface;
class VS_UserData;
class VS_RegistryKey;
namespace net {
	class LoggerInterface;
} //namespace net

struct VS_ExternalAccount;

class VS_Indentifier
{
public:
	VS_Indentifier(boost::asio::io_service &io) : m_voipProtocol(VS_CallConfig::UNDEFINED), m_voipGatewayParamName("#tel"), m_io(io){}
	virtual ~VS_Indentifier(){ /*stub*/ }
	bool IsMyCallId(string_view callId) const;
	acs::Response Protocol(const void* buf, std::size_t bufSz) const;

	bool Resolve(VS_CallConfig &cfg, string_view callId, VS_UserData *from);
	bool PostResolve(VS_CallConfig &config, string_view callId, VS_UserData *from, bool block);

	void LoadConfigurations(std::vector<VS_CallConfig> &users, std::vector<VS_CallConfig> &hosts, const char *peerId);
	bool ConvertConfiguration(VS_CallConfig &cfg, string_view tcId, const VS_ExternalAccount &a);

	bool CreateDefaultConfiguration(VS_CallConfig& cfg, const net::Endpoint &ep,
	                                VS_CallConfig::eSignalingProtocol protocol, string_view username = {});

	std::shared_ptr<VS_ParserInterface> CreateParser(boost::asio::io_service::strand& strand, const void* buf, std::size_t bufSz, const std::shared_ptr<net::LoggerInterface>& logger);
	std::shared_ptr<VS_ParserInterface> CreateParser(boost::asio::io_service::strand& strand, string_view callId, const std::shared_ptr<net::LoggerInterface>& logger);
	std::shared_ptr<VS_ParserInterface> CreateParser(boost::asio::io_service::strand& strand, const VS_CallConfig& config, const std::shared_ptr<net::LoggerInterface>& logger);

	static boost::shared_ptr<VS_Indentifier> MakeIndentifierChain(const std::vector<VS_CallConfig::eSignalingProtocol> &protocols, boost::asio::io_service &io, const std::string& serverInfo);
	static boost::shared_ptr<VS_Indentifier> GetCommonIndentifierChain(boost::asio::io_service &io, const std::string& serverInfo);
	boost::shared_ptr<VS_Indentifier> GetNext() const;

	virtual VS_CallConfig::eSignalingProtocol GetSignalongProtocol_Impl() const
	{
		return VS_CallConfig::UNDEFINED;
	}

	VS_CallConfig::eSignalingProtocol GetVoipProtocol() const;
	void SetVoipProtocol(const VS_CallConfig::eSignalingProtocol voip_proto); // It was added mostly for being used in the unit tests.

	virtual bool ResolveThroughDNS(const std::string & /*host*/, net::port /*port*/,
		const std::vector<net::protocol> & /*desiredProtos*/, net::address &/*setAddr*/,
		net::port &/*setPort*/, bool /*block*/) const;
	bool AsyncResolve(std::function<void()> &&resolveTask) const;
	virtual bool AsyncResolveImpl(std::function<void()> &resolveTask) const = 0;
protected:
	void LoadConfigurations_Impl_Base(const char* keyName, std::vector<VS_CallConfig> &users, std::vector<VS_CallConfig> &hosts, const char *peerId);
	void ReplaceTelephonePrefix(VS_CallConfig &config);
	static bool IsVisualSeparator(int ch) noexcept;

	virtual void LoadVoipProtocolConfiguration();
	VS_CallConfig::eSignalingProtocol m_voipProtocol;
	std::string						  m_voipGatewayParamName;

	boost::asio::io_service &m_io;
private:

	void SetNext(const boost::shared_ptr< VS_Indentifier > &i);
	boost::shared_ptr< VS_Indentifier > m_next;

	virtual bool IsMyCallId_Impl(string_view /*callId*/) const
	{
		return false;
	}

	virtual acs::Response Protocol_Impl(const void* /*buf*/, std::size_t /*bufSz*/) const;

	virtual bool Resolve_Impl(VS_CallConfig &/*cfg*/, string_view /*callId*/, VS_UserData */*from*/)
	{
		return false;
	}

	virtual bool PostResolve_Impl(VS_CallConfig &/*config*/, string_view /*callId*/, VS_UserData */*from*/, bool /*block*/)
	{
		return false;
	}

	virtual void LoadConfigurations_Impl(std::vector<VS_CallConfig> &/*users*/, std::vector<VS_CallConfig> &/*hosts*/, const char */*peerId*/)
	{
	}

	bool LoadConfigForomKey(VS_CallConfig &cfg, const VS_RegistryKey &key, const char *configName, const char *hostname,
		const char *defConfigName = nullptr);

	virtual bool ConvertConfiguration_Impl(VS_CallConfig &/*cfg*/, string_view /*tcId*/, const VS_ExternalAccount &/*a*/)
	{
		return false;
	}

	virtual bool CreateDefaultConfiguration_Impl(VS_CallConfig& /*cfg*/, const net::Endpoint &/*ep*/,
		VS_CallConfig::eSignalingProtocol /*protocol*/, string_view /*username*/)
	{
		return false;
	}

	virtual std::shared_ptr<VS_ParserInterface> CreateParser_Impl(boost::asio::io_service::strand& /*strand*/, const std::shared_ptr<net::LoggerInterface>& /*logger*/)
	{
		return {};
	}
};

