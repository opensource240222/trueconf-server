#ifndef VS_LDAP_CORE_389
#define VS_LDAP_CORE_389

#include "VS_LDAPCore.h"

namespace tc
{

	class LDAPCore_389dir : public LDAPCore
	{
	public:
		template<typename... Args>
		LDAPCore_389dir(Args&& ...args) : LDAPCore(std::forward<Args>(args)...){}
		virtual ~LDAPCore_389dir(){}
		virtual ldap_error_code_t GetGroupUsers(const char* group_dn, std::vector<ldap_user_info>& group_users, page_cookie_t& cookie,
			const long page_size = 0, const std::vector<tc::attr_name_t>* custom_attrs = 0, const std::pair<std::string, bool>* sort_attr = 0,
			const std::string* query = 0) override;
		virtual ldap_error_code_t GetAllGroupsOfUser(const tc::ldap_user_info& user, const unsigned long user_primary_group_id, const std::string& objectSid,
			std::set<tc::group_dn>& ret) override;
	};

}

#endif /*VS_LDAP_CORE_389*/