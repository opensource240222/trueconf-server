#include "VS_LDAPCore_ActiveDirectory.h"

#ifdef _WIN32
#include <Sddl.h>
#endif

namespace tc
{
	ldap_error_code_t LDAPCore_ActiveDirectory::GetGroupUsers(const char* group_dn, std::vector<ldap_user_info>& group_users, page_cookie_t& cookie,
		const long page_size, const std::vector<tc::attr_name_t>* custom_attrs, const std::pair<std::string, bool>* sort_attr,
		const std::string* query){
		return LDAPCore::GetGroupUsers(group_dn, group_users, cookie, page_size, custom_attrs, sort_attr, query);
	}

	ldap_error_code_t LDAPCore_ActiveDirectory::GetAllGroupsOfUser(const tc::ldap_user_info& user, const unsigned long user_primary_group_id, const std::string& objectSid, std::set<tc::group_dn>& ret)
	{
		tc::ldap_error_code_t err(LDAP_SUCCESS);
		std::set<tc::group_dn> memberOf;
		for (const auto& p : user.custom_attrs)
			if (p.first == m_a_memberOf)
				memberOf.insert(p.second);

		if (user_primary_group_id)
		{
			// replace "-last" part of SID with primaryGroupID
			std::string objSid_with_group = objectSid;

			auto last = objSid_with_group.find_last_of('-');
			if (last != std::string::npos && (++last < objSid_with_group.length()))
				objSid_with_group.replace(last, objSid_with_group.length() - last, std::to_string(user_primary_group_id));

			page_cookie_t fake_cookie;
			std::vector<attrs_t> out;
			std::string filter;
			filter.reserve(128);
			filter += "(";
			filter += m_a_objectSid;
			filter += "=";
			filter += objSid_with_group;
			filter += ")";
			err = Search(filter.c_str(), out, fake_cookie);		// get DN of group by primaryGroupId(encoded as objectSID)
			if (err != LDAP_SUCCESS)
				return err;
			for (const auto& entry : out)
			{
				for (const auto& a : entry)
				{
					if (a.first == m_a_distinguishedName)
						memberOf.insert(a.second);
				}
			}
		}

		ret = memberOf;

		std::set<tc::group_dn> parent_groups;
		while (!memberOf.empty())
		{
			err = GetParentGroupsOfGroups(memberOf, parent_groups);
			if (err != LDAP_SUCCESS)
				return err;

			auto it = parent_groups.begin();
			while (it != parent_groups.end())
			{
				if (ret.find(*it) != ret.end())
					it = parent_groups.erase(it);
				else
					++it;
			}

			ret.insert(parent_groups.cbegin(), parent_groups.cend());
			memberOf = std::move(parent_groups);
		}

		return err;
	}

	tc::ldap_error_code_t LDAPCore_ActiveDirectory::GetParentGroupsOfGroups(const std::set<tc::group_dn>& in_groups, std::set<tc::group_dn>& out_groups)
	{
		if (in_groups.empty())
			return LDAP_SUCCESS;
		tc::ldap_error_code_t err(LDAP_SUCCESS);

		auto batch = [&](const std::string& members) -> tc::ldap_error_code_t {
			std::string filter;
			filter.reserve(m_filter_group.length() + members.length() + 10);
			filter += "(&";
			filter += m_filter_group;
			filter += "(|";
			filter += members;
			filter += "))";

			page_cookie_t fake_cookie;
			std::vector<attrs_t> out;
			err = Search(filter.c_str(), out, fake_cookie);
			if (err != LDAP_SUCCESS)
				return err;
			for (const auto& entry : out)
			{
				for (const auto& a : entry)
				{
					if (a.first == m_a_distinguishedName)
						out_groups.insert(a.second);
				}
			}
			return err;
		};

		std::string members;
		members.reserve(100 * in_groups.size());
		for (const auto& g : in_groups)
		{
			members += "(";
			members += m_a_groupmember;
			members += "=";
			members += EscapeForLDAPFilter(g);
			members += ")";
			if (members.length() >= 2000)	// just to split into batches, not to make a too big filter to get filter error from underlying lib
			{
				err = batch(members);
				if (err != LDAP_SUCCESS)
					return err;
				members.clear();
			}
		}
		if (!members.empty())
			err = batch(members);
		return err;
	}

} // namespace tc
