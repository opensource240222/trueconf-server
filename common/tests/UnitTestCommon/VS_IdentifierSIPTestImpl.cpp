#include "VS_IdentifierSIPTestImpl.h"
#include "std/cpplib/VS_RegistryConst.h"

void VS_IndentifierSIP_TestImpl::LoadConfigurations_Impl(std::vector<VS_CallConfig> &users, std::vector<VS_CallConfig> &hosts, const char *peerId)
{
	if (!m_users.empty() || !m_hosts.empty()) {
		for (auto& pConfig : m_users) { users.emplace_back(pConfig); }
		for (auto& pConfig : m_hosts) { hosts.emplace_back(pConfig); }
	}
	else {
		VS_IndentifierSIP::LoadConfigurations(users, hosts, peerId);
	}
}

void VS_IndentifierSIP_TestImpl::AddRootConfiguration(VS_CallConfig test_host)
{
	test_host.HostName = BASE_PEERS_CONFIGURATION_TAG;
	m_hosts.emplace_back(std::move(test_host));
}

void VS_IndentifierSIP_TestImpl::AddHostConfiguration(VS_CallConfig test_host)
{
	 m_hosts.emplace_back(std::move(test_host));
}

void VS_IndentifierSIP_TestImpl::AddUserConfiguration(VS_CallConfig test_user)
{
	m_users.emplace_back(std::move(test_user));
}

void VS_IndentifierSIP_TestImpl::ClearConfigurations()
{
	m_hosts.clear();
	m_users.clear();
}
