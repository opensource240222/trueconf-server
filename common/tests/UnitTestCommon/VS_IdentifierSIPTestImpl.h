#pragma once

#include "../TrueGateway/CallConfig/VS_IndentifierSIP.h"
#include <tests/common/ASIOEnvironment.h>

class VS_IndentifierSIP_TestImpl : public VS_IndentifierSIP {
public:
	void LoadConfigurations_Impl(std::vector<VS_CallConfig> &users, std::vector<VS_CallConfig> &hosts, const char *peerId) override;
	void AddRootConfiguration(VS_CallConfig test_host);
	void AddHostConfiguration(VS_CallConfig test_host);
	void AddUserConfiguration(VS_CallConfig test_user);
	void ClearConfigurations();

	explicit VS_IndentifierSIP_TestImpl() : VS_IndentifierSIP(g_asio_environment->IOService(), "ServerVendor") {}

protected:
	std::vector<VS_CallConfig> m_users;
	std::vector<VS_CallConfig> m_hosts;
};
