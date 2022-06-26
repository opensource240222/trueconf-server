#include "VS_IndentifierH225RAS.h"
#include "../h323/VS_H225RASParser.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/MakeShared.h"

#include "net/DNSUtils/VS_DNSTools.h"
#include <boost/asio/io_service.hpp>

acs::Response VS_IndentifierH225RAS::Protocol_Impl(const void* buf, std::size_t bufSz) const
{
	if(!buf || !bufSz)
		return acs::Response::not_my_connection;
	VS_PerBuffer buff(buf, bufSz * 8);
	VS_RasMessage msg;
	return msg.Decode(buff) ? acs::Response::accept_connection : acs::Response::not_my_connection;
}

bool VS_IndentifierH225RAS::PostResolve_Impl(VS_CallConfig &config, string_view callId, VS_UserData *from, bool block)
{
	if (config.SignalingProtocol != VS_CallConfig::H225RAS) return false;

	net::address tmp_addr = net::dns_tools::single_make_a_aaaa_lookup(config.HostName);

	if (tmp_addr.is_unspecified())
	{
		create_call_config_manager(config).SetVerificationResult(VS_CallConfig::e_ServerUnreachable, true);
		return false;
	}

	auto &address = config.Address;
	address.protocol = net::protocol::UDP;

	if (address.addr.is_unspecified())
		address.addr = std::move(tmp_addr);
	if (address.port == 0)
		address.port = 1719; //default port

	return true;
}

VS_CallConfig::eSignalingProtocol VS_IndentifierH225RAS::GetSignalongProtocol_Impl() const
{
	return VS_CallConfig::H225RAS;
}

std::shared_ptr<VS_ParserInterface> VS_IndentifierH225RAS::CreateParser_Impl(boost::asio::io_service::strand& strand, const std::shared_ptr<net::LoggerInterface>& logger)
{
	return vs::MakeShared<VS_H225RASParser>(strand, logger);
}

void VS_IndentifierH225RAS::LoadConfigurations_Impl(std::vector<VS_CallConfig> &users, std::vector<VS_CallConfig> &hosts, const char *peerId)
{
	LoadConfigurations_Impl_Base(H323_PEERS_KEY, users, hosts, peerId);
}

bool VS_IndentifierH225RAS::AsyncResolveImpl(std::function<void()>& resolveTask) const
{
	m_io.post(std::move(resolveTask));
	return true;
}