#include "VS_LDAPCore_389dir.h"
#include <boost/algorithm/string.hpp>

namespace tc
{
	ldap_error_code_t LDAPCore_389dir::GetGroupUsers(const char* group_dn, std::vector<ldap_user_info>& group_users, page_cookie_t& cookie,
		const long page_size, const std::vector<tc::attr_name_t>* custom_attrs, const std::pair<std::string, bool>* sort_attr,
		const std::string* query){
		if (!m_a_memberOf.empty())
		{
			return LDAPCore::GetGroupUsers(group_dn, group_users, cookie, page_size, custom_attrs, sort_attr, query);
		}
		return LDAPCore::GetGroupUsers_noMemberOf(group_dn, group_users, cookie, page_size, custom_attrs, sort_attr, query);
	}
	ldap_error_code_t LDAPCore_389dir::GetAllGroupsOfUser(const tc::ldap_user_info& user, const unsigned long /*user_primary_group_id*/, const std::string& /*objectSid*/, std::set<tc::group_dn>& ret) {
		tc::ldap_error_code_t err(LDAP_SUCCESS);

		tc::page_cookie_t fake_cookie;
		std::vector<attrs_t> results;
		if (m_a_memberOf.empty())
		{
			char buff[4096] = { 0 };
			snprintf(buff, sizeof(buff), "(&%s(%s:=%s))", m_filter_group.c_str(), m_a_groupmember.c_str(), user.dn.c_str());
			err = LDAPSearch(m_ldap.load()->ctx(), m_basedn, LDAP_SCOPE_SUBTREE, buff, nullptr, results, fake_cookie/*, page_size, sort_attr*/);

			if (err == LDAP_SUCCESS) {
				for (const auto &group : results) {
					for (const auto& atr : group)
						if (boost::iequals(atr.first, m_a_distinguishedName)) ret.emplace(atr.second);
				}
			}
		}
		else
		{
			const char* attrs[2] = {
				m_a_memberOf.c_str(),
				0
			};

			std::string filter = "(&";
			filter += m_filter_person;
			filter += "(";
			filter += m_a_login;
			filter += "=";
			filter += user.login;
			filter += "))";

			err = LDAPSearch(m_ldap.load()->ctx(), m_basedn, LDAP_SCOPE_SUBTREE, filter.c_str(), attrs , results, fake_cookie/*, page_size, sort_attr*/);
			if (err == LDAP_SUCCESS) {
				if (!results.empty())
				{
					assert(results.size() == 1); // there should be only one person
					for (const auto& atr : results[0])
					{
						if (boost::iequals(atr.first, m_a_memberOf)) ret.emplace(atr.second);
					}
				}
			}
			
		}

		if (err != LDAP_SUCCESS && m_ldap_error_handler_functor) {
				m_ldap_error_handler_functor(err);
		}

		return err;
	}
}
