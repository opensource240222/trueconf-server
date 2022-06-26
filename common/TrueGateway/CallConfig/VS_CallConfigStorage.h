#pragma once

#include <string>
#include <vector>
#include <future>

#include <boost/make_shared.hpp>

#include "std/cpplib/fast_mutex.h"
#include "std-generic/compat/memory.h"

#include "VS_CallConfig.h"

class VS_Indentifier;
class VS_UserData;

struct VS_ExternalAccount;

class VS_CallConfigStorage: public vs::enable_shared_from_this<VS_CallConfigStorage>
{
public:
	typedef std::future<void> update_result_t;
	typedef vs::fast_mutex mutex_t;

	~VS_CallConfigStorage(void);
	update_result_t UpdateSettings();
	update_result_t UpdateSettingByPeerId(const char *peerId);

	void AddUser(string_view tcId, const std::vector<VS_ExternalAccount> &accounts);
	void RemoveUser(string_view tcId);

	void RegisterProtocol(const boost::shared_ptr<VS_Indentifier> &ident);

	vs::Synchronized<std::vector<VS_CallConfig>, mutex_t> &GetRegistrationSettings()
	{
		return m_registration;
	}

	bool CheckUserName(const VS_CallConfig::eSignalingProtocol protocol, string_view remoteUsername);

	bool Resolve(VS_CallConfig &cfg, string_view callId, VS_UserData *froUser);


	VS_CallConfig GetConfiguration(const net::Endpoint &ep,
	                               VS_CallConfig::eSignalingProtocol protocol, string_view remoteUsername = {},
	                               const std::string &userAgent = {});

	boost::shared_ptr<VS_Indentifier> GetCommonIdentifier() const
	{
		return m_indetifier;
	}

	boost::shared_ptr<VS_Indentifier> GetIdentifier(VS_CallConfig::eSignalingProtocol protocol) const;

protected:
	VS_CallConfigStorage(void) {}
private:
	update_result_t UpdateSettings(const char* peerId);

	struct RegConfigs final
	{
		std::vector<VS_CallConfig> m_hosts;
		std::vector<VS_CallConfig> m_users;
	};
	vs::Synchronized<RegConfigs, mutex_t> m_reg_configs;
	vs::Synchronized<std::vector<VS_CallConfig>, mutex_t> m_cache;
	vs::Synchronized<std::vector<VS_CallConfig>, mutex_t> m_registration;
	boost::shared_ptr<VS_Indentifier> m_indetifier;

	VS_CallConfig& MergeWithHostAndUserConfiguration(VS_CallConfig &baseConf, RegConfigs& configs, bool cache = true, const std::string &userAgent = {});

	std::size_t GetRootConfigIndex(const std::vector<VS_CallConfig> &data, const VS_CallConfig &conf) const;
	template <class T>
	std::size_t GetConfigIndex(std::vector<T> &data, VS_CallConfig &baseConf, bool ignoreUsername);

	void Clear(bool removeAll);
	void ResolveHostnames();
	void UdpateRegistrationSettings();
};
