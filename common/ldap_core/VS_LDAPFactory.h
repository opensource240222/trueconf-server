#ifndef VS_LDAP_FACTORY
#define VS_LDAP_FACTORY

#include "VS_LDAPCore.h"

#include <memory>

class VS_LDAPFactory
{
public:
	static std::shared_ptr<tc::LDAPCore> CreateInstance(tc::eLDAPServerType serverType, const tc::cfg_params_t &params, tc::LDAPCore::ProcessAvatarsHandler avatarHandler, bool doNotRunCacheUpdater = false);
	static bool GetLDAPServerType(const tc::cfg_params_t &params, tc::eLDAPServerType &OUT_type);
};

#endif /* VS_LDAP_FACTORY */