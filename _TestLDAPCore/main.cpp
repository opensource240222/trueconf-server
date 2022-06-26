#include "../common/ldap_core/VS_LDAPCore.h"
#include "../common/std/cpplib/VS_RegistryKey.h"
#include "../common/ldap_core/CfgHelper.h"

void main()
{
	VS_RegistryKey::SetRoot("TrueConf\\Server");
	VS_RegistryKey cfg(false, CONFIGURATION_KEY);
	tc::cfg_params_t params;
	tc::ReadCfg(cfg, params);
	params[L"LDAP Version"] = L"3";
	tc::LDAPCore ldap_core;
	bool init_result = ldap_core.Init(params);
	if (!init_result)
	{
		printf("tc::LDAPCore::Init() failed\n");
		return;
	}

	int connect_result = ldap_core.Connect();
	if (connect_result)
	{
		printf("tc::LDAPCore::Connect() failed\n");
		return;
	}
	//tc::nested_groups nested_groups1;
	//ldap_core.ExpandGroup(L"CN=fathergroup,CN=Users,DC=tc71,DC=loc", nested_groups1);

	std::vector<tc::group_dn> g;
//	g.push_back(L"CN=group1,OU=Groups,DC=kt,DC=ru");
//	g.push_back(L"CN=fathergroup,CN=Users,DC=tc71,DC=loc");
//	g.push_back(L"CN=grandson group,CN=Users,DC=tc71,DC=loc");
	ldap_core.UpdateNestedGroupsCache(g);

	//tc::nested_groups nested_groups2;
	//ldap_core.ExpandGroup(L"CN=fathergroup,CN=Users,DC=tc71,DC=loc", nested_groups2);


	//std::vector<tc::ldap_group_info> all_groups;
	//ldap_core.GetAllGroups(all_groups);

	//std::vector<tc::ldap_user_info> all_users;
	//ldap_core.GetAllUsers(all_users);

	std::vector<tc::attr_name_t> custom_attrs = { L"otherHomePhone", L"mail", L"telephoneNumber" };
	//std::vector<tc::ldap_user_info> group_users;
	//ldap_core.GetGroupUsers(L"CN=fathergroup,CN=Users,DC=tc71,DC=loc", group_users, &custom_attrs);

	std::vector<tc::ldap_user_info> found_users;
	ldap_core.SearchForUser(L"(userPrincipalName=*)", found_users, &custom_attrs);

	tc::page_cookie_t cookie;
	long page_size = 3;
	std::vector<tc::ldap_user_info> found_users2;
	ldap_core.SearchForUser(L"(userPrincipalName=*)", found_users2, cookie, page_size, &custom_attrs);
	//std::map<tc::attr_name_t, std::vector<tc::attr_value_t>> attrs;
	//ldap_core.GetAllAttributes(L"CN=Grandson User,CN=Users,DC=tc71,DC=loc", attrs);
	//std::wstring pg = ldap_core.GetParentGroup(L"CN=grandson group,CN=Users,DC=tc71,DC=loc", 0);

	ldap_core.Disconnect();


	/*
	bool IsLastPage(false);
	do{
	std::vector<attrs_t> o;
	err = LDAPSearchPagedImp(m_ldap, m_basedn.c_str(), LDAP_SCOPE_SUBTREE, filter, (const wchar_t**)attrs_arr, o, cookie, page_size);
	if (err != LDAP_SUCCESS) {
	// todo(kt): need abandon search
	return err;
	}
	if (o.size() != page_size)
	IsLastPage = true;
	if (page_size == 0)
	out = std::move(o);
	else{
	for (const auto& x : o)
	out.push_back(x);
	}
	} while (!IsLastPage);
	*/

}