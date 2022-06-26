#include "VS_LDAPCore_Custom.h"

namespace tc
{
	ldap_error_code_t LDAPCore_Custom::GetGroupUsers(const char* group_dn, std::vector<ldap_user_info>& group_users, page_cookie_t& cookie,
		const long page_size, const std::vector<tc::attr_name_t>* custom_attrs, const std::pair<std::string, bool>* sort_attr,
		const std::string* query){
		return LDAPCore::GetGroupUsers_noMemberOf(group_dn, group_users, cookie, page_size, custom_attrs, sort_attr, query);
	}
	ldap_error_code_t LDAPCore_Custom::GetAllGroupsOfUser(const tc::ldap_user_info& user, const unsigned long user_primary_group_id, const std::string& objectSid, std::set<tc::group_dn>& ret) {
		return LDAPCore::GetAllGroupsOfUser_noMemberOf(user, user_primary_group_id, objectSid, ret);
	}
}
