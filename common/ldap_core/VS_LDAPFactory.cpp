#include "VS_LDAPFactory.h"
#include "VS_LDAPConst.h"

#include "VS_LDAPCore_ActiveDirectory.h"
#include "VS_LDAPCore_389dir.h"
#include "VS_LDAPCore_OpenLDAP.h"
#include "VS_LDAPCore_Custom.h"

std::shared_ptr<tc::LDAPCore> VS_LDAPFactory::CreateInstance(tc::eLDAPServerType serverType, const tc::cfg_params_t &params, tc::LDAPCore::ProcessAvatarsHandler avatarHandler, bool doNotRunCacheUpdater)
{
	std::shared_ptr<tc::LDAPCore> created{};
	switch (serverType)
	{
	case tc::LDAP_SERVER_TYPE_AD:
		created = std::make_shared<tc::LDAPCore_ActiveDirectory>(std::move(avatarHandler));
		break;
	case tc::LDAP_SERVER_TYPE_OPENLDAP:
		created = std::make_shared<tc::LDAPCore_OpenLDAP>(std::move(avatarHandler));
		break;
	case tc::LDAP_SERVER_TYPE_389DIR:
		created = std::make_shared<tc::LDAPCore_389dir>(std::move(avatarHandler));
		break;
	case tc::LDAP_SERVER_TYPE_CUSTOM:
		created = std::make_shared<tc::LDAPCore_Custom>(std::move(avatarHandler));
		break;
	}

	if(created)
	{
		if (!created->Init(params, doNotRunCacheUpdater))
			created.reset();
	}

	return created;
}

bool VS_LDAPFactory::GetLDAPServerType(const tc::cfg_params_t &params, tc::eLDAPServerType &OUT_type){
	auto it = params.find(tc::LDAP_SERVER_TYPE_TAG);
	if (it == params.end())
		OUT_type = tc::LDAP_SERVER_TYPE_AD;
	else
		OUT_type = (tc::eLDAPServerType)::atoi(it->second.c_str());

	return true;
}
