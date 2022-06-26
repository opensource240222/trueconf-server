#pragma once

#include "VS_LDAPCoreImp_WinLDAP.h"
#include "VS_LDAPCoreImp_OpenLDAP.h"
#include "VS_LDAPConst.h"
#include "VS_GroupManager.h"

#include "std/cpplib/VS_Lock.h"
#include "std/cpplib/VS_ThreadPool.h"
#include "std-generic/cpplib/atomic_shared_ptr.h"
#include "std-generic/cpplib/synchronized.h"
#include "std-generic/compat/condition_variable.h"
#include "ldap_core/liblutil/tc_ldap.h"

#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <map>
#include <set>
#include <vector>

#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace tc
{
	class LDAPCore :
		public VS_IABSink,
		public VS_IABSink_GetRegGroupUsers,
#ifdef _WIN32
		public VS_LDAPCoreImp_WinLDAP
#else
		public VS_LDAPCoreImp_OpenLDAP
#endif
	{
		class TrustedDomain
		{
		public:
			LDAP* m_ldap;
			std::set<std::string> m_baseDN;
			std::string flatName;

			TrustedDomain() : m_ldap(nullptr)
			{}
			~TrustedDomain()
			{
				if (m_ldap)
					ldap_unbind_ext(m_ldap, nullptr, nullptr);
			}
		};

		VS_Locked<bool>						m_params_changed;

		// ldap attributes
		std::string	m_a_ALLOWED_BY_SERVER_MAX_BW;
		std::string	m_a_ALLOWED_BY_SERVER_MAX_FPS;
		std::string	m_a_ALLOWED_BY_SERVER_MAX_WXH;
		std::string	m_a_AddressBook;
		std::string	m_a_ServerName;

		std::string m_ldap_attr_Phone_mobile;
		std::string m_ldap_attr_Phone_work;
		std::string m_ldap_attr_Phone_home;

		// ldap attributes that we modify
		std::string	m_ldap_attr_UserStatus;
		std::string	m_ldap_attr_UserID;

		bool		m_trust_enabled = true;
		std::string	m_a_trustPartner;
		std::string	m_a_flatName;
		std::string	m_filter_trustedDomain;
		std::string	m_filter_foreignSecurityPrincipal;		// for objectSID of user from another trustedDomain
		std::atomic_bool				m_trustedDomains_updated{};
		std::shared_ptr<VS_ThreadPool>	m_trust_threads;

		VS_Lock									m_nested_groups_lock;
		std::chrono::system_clock::time_point	m_last_nested_cache_update;
		std::chrono::system_clock::time_point	m_last_login_group_cache_update;
		std::thread								m_nested_thread;
		vs::condition_variable					m_nested_cv;
		std::mutex								m_nested_thread_mutex;
		bool									m_nested_do_update = false;
		bool									m_nested_is_stopped = false;
		bool									m_nested_is_doing_update = false;
		void UpdateNestedCacheThread();
		ldap_error_code_t UpdateNestedGroupsCacheImp(const std::set<group_dn>& groups);
		void UpdateAllCacheImp();

		std::shared_ptr<VS_GroupManager> m_group_manager;

		void InitTrustedDomains();

		void FetchUsersFromResults(const std::vector<attrs_t>& results, std::vector<ldap_user_info>& users, const std::vector<attr_name_t>* custom_attrs = 0);
		void FetchForeignUsersBySID(const std::set<std::string>& objSIDs, std::vector<ldap_user_info>& u_foreign);
		void FetchForeignUserByFilter(const char* filter, std::vector<ldap_user_info>& u_foreign, const char* desired_trust = 0);
		bool IsObjectSID(const std::string& str) const;

		void GetServerRootInfo(LDAP* ctx, std::string& base_dn, std::set<std::string>& supportedControl);
		bool GetDnsServerName(LDAP* ctx, std::string& foundName);

		int ConnectBySRV(const std::string& domain);
		int ConnectByHost(const std::string& domain);
		int ConnectByIp(const std::string& ipStr);

	protected:
		std::string	m_a_memberOf;
		std::vector<std::string> m_skip_referrals;
		boost::function<void(const ldap_error_code_t)> m_ldap_error_handler_functor;

		std::string		m_our_domain;
		class CtxUnbinder
		{
			LDAP* m_ctx = nullptr;
		public:
			explicit CtxUnbinder(LDAP* ctx) : m_ctx(ctx)
			{}
			CtxUnbinder(const CtxUnbinder&) = delete;
			CtxUnbinder& operator=(const CtxUnbinder&) = delete;
			~CtxUnbinder()
			{
				if (m_ctx)
					ldap_unbind_ext(m_ctx, nullptr, nullptr);
			}
			LDAP* ctx() const
			{
				return m_ctx;
			}
		};
		vs::atomic_shared_ptr<CtxUnbinder> m_ldap;

		ldap_error_code_t LDAPSearch(LDAP* ld, const std::string& dn, const long& scope, const std::string& filter, const char** attrs, std::vector<attrs_t>& out, page_cookie_t& cookie, const long page_size = LDAP_DEFAULT_PAGE_SIZE, const std::pair<std::string, bool>* sort_attr = 0, const bool changed_ctx = false);
		//ldap_error_code_t LDAPSearchImp(LDAP* conn, const std::string& dn, const long& scope, const std::string& filter, const char** attrs, std::vector<attrs_t>& out);

		bool isUselessDN(const std::string &dn) const override;
		
		inline bool IsAvatarsAttr(string_view attr) const noexcept override
		{
			return attr == m_a_avatars;
		}

	public:

		typedef std::function<void(const std::string&/*login*/, const std::multimap<tc::attr_name_t, attr_value_t>&/*attributes*/)> ProcessAvatarsHandler;

		enum class CheckLoginResult : uint8_t
		{
			OK = 0,
			NOT_FOUND_OR_LDAPDISABLED_OR_AMBIGUOUS,
			NOT_IN_LOGIN_GROUP,
			LDAP_ERROR,

			UNDEFINED = 0xff,
		};

		// ldap server configuration
		eLDAPServerType		m_server_type;
		bool				m_ldap_autodetect = false;
		std::string			m_basedn;
		std::string			m_domain;
		std::chrono::seconds	m_ab_cache_timeout{};
		VS_Locked<std::string>	m_login_group;
		bool					m_use_ntlm = false;
		VS_WithLock<std::map<std::string, std::shared_ptr<TrustedDomain>>> m_trustedDomains;
		std::string m_our_flatName;

		std::string	m_ldap_avatars_path;
		bool m_use_avatars;
		bool m_avatar_propagating_allowed;
		std::int32_t m_avatars_size{};
		std::int32_t m_avatars_quality{};

		mutable VS_Locked<unsigned long>	m_params_hash;

		// ldap server attributes
		std::string	m_a_login;
		std::string	m_a_email;
		std::string	m_a_firstname;
		std::string	m_a_middlename;
		std::string	m_a_lastname;
		std::string	m_a_displayname;
		std::string	m_a_company;
		std::string	m_a_groupmember;
		std::string	m_a_primaryGroupId;
		std::string	m_a_primaryGroupToken;
		std::string	m_a_GroupDisplayName;
		std::string m_a_avatars;
		std::string	m_ldap_attr_FullID;
		std::vector<attr_name_t> m_custom_attrs_user_info;
		std::vector<std::string> m_detailed_user_info;
		std::vector<attr_name_t> m_user_aliases;
		char LDAP_FILTER_IS_USER[1024]{};

		// ldap filters
		std::string					m_filter_login;
		std::string					m_filter_callid;
		std::string					m_filter_ab;
		std::string					m_filter_verify;
		std::string					m_filter_group;
		std::string					m_filter_person;
		bool						m_filter_search_by_login_group = true;
		VS_Locked<std::string>		m_filter_disabled;

		// caches
		vs::atomic_shared_ptr<VS_AbCommonMap> m_cache_all_users;
		vs::atomic_shared_ptr<const std::map<std::string, std::shared_ptr<VS_AbCommonMap>>> m_cache_groups_users;
		VS_Lock																	m_cache_user_info_lock;
		std::map<std::string, boost::shared_ptr<VS_StorageUserData>, ci_less>	m_cache_user_info;

		SortControlScheme m_sort_control_scheme = SortControlScheme::INVALID;
		
		LDAPCore(ProcessAvatarsHandler avatarsHandler);
		virtual ~LDAPCore();

		bool Init(const cfg_params_t& params, bool do_not_run_cache_updater = false);
		void InitUpdateNestedCacheThread();
		bool SetGroupManager(const std::shared_ptr<VS_GroupManager> &gr_manager);

		int Connect(bool force = false);		// 0 - no errors
		void Disconnect();

		void StartUpdateNestedCache();
		std::chrono::system_clock::time_point GetLastUpdateNestedCacheTime();
		void ClearCaches();

		ldap_error_code_t Search(const char* filter, std::vector<attrs_t>& out, const std::vector<attr_name_t>* custom_attrs = 0, const std::pair<std::string, bool>* sort_attr = 0, int8_t scope = LDAP_SCOPE_SUBTREE);
		ldap_error_code_t Search(const char* filter, std::vector<attrs_t>& out, page_cookie_t& cookie, const long page_size = 0, const std::vector<attr_name_t>* custom_attrs = 0, const std::pair<std::string, bool>* sort_attr = 0, int8_t scope = LDAP_SCOPE_SUBTREE);

		ldap_error_code_t SearchForUser(const char* filter, std::vector<ldap_user_info>& found_users, const std::vector<attr_name_t>* custom_attrs = 0, const std::pair<std::string, bool>* sort_attr = 0);
		ldap_error_code_t SearchForUser(const char* filter, std::vector<ldap_user_info>& found_users, page_cookie_t& cookie, const long page_size = 0, const std::vector<attr_name_t>* custom_attrs = 0, const std::pair<std::string, bool>* sort_attr = 0);

		bool FetchUser(const ldap_user_info& info, VS_StorageUserData& ude);
		void FetchForeignUserByLogin(const std::string& login, std::vector<ldap_user_info>& u_foreign);

		std::string GetOurDomainFromBaseDN() const;
		std::string PreprocessCallID(string_view call_id) const;
		std::string EscapeForLDAPFilter(string_view sv);

		vs::atomic_shared_ptr<const std::map<std::string, std::string>> m_login_group_expand; // [key=dn,val=primaryGroupToken]
		void ExpandGroup(const std::string& login_group, std::map<std::string, std::string>& result);
		void ExpandGroup_hasMemberOf(const std::string& login_group, std::map<std::string, std::string>& result);
		void ExpandGroup_noMemberOf(const std::string& login_group, std::map<std::string, std::string>& result);
		void UpdateLoginGroupCache(void);

		void SetUserStatus(const VS_SimpleStr& call_id, int status);

		virtual bool GetRegGroups(std::map<std::string, VS_RegGroupInfo>& reg_groups) override;

		std::string GetOurDomain() const;
		// IASink interface
		virtual bool GetDisplayName(const vs_user_id& /*user_id*/, std::string& /*display_name*/) override { return false; }
		virtual int  SearchUsers(VS_Container& /*cnt*/, const std::string& /*query*/, VS_Container* /*in_cnt*/) override { return 0; }
		virtual bool FindUser_Sink(const vs_user_id& user_id, VS_StorageUserData& ude, bool onlyCached = false) override;
		virtual bool GetAllUsers(std::shared_ptr<VS_AbCommonMap>& users) override;
		//virtual bool GetABForUserImp(const vs_user_id& owner, VS_AbCommonMap& m) override { return false; }
		virtual bool IsCacheReady() const override;
		virtual bool IsLDAP_Sink() const override;

		// IASink_GetRegGroupUsers
		virtual bool GetRegGroupUsers(const std::string& gid, std::shared_ptr<VS_AbCommonMap>& users) override;

		ldap_error_code_t GetAllGroups(std::vector<ldap_group_info>& all_groups, const std::pair<std::string, bool>* sort_attr = 0, const std::string* query = 0);
		ldap_error_code_t GetAllGroups(std::vector<ldap_group_info>& all_groups, page_cookie_t& cookie, const long page_size = 0, const std::pair<std::string, bool>* sort_attr = 0, const std::string* query = 0);

		ldap_error_code_t GetAllUsers(std::vector<ldap_user_info>& all_users, const std::vector<attr_name_t>* custom_attrs = 0, const std::pair<std::string, bool>* sort_attr = 0, const std::string* query = 0);
		ldap_error_code_t GetAllUsers(std::vector<ldap_user_info>& all_users, page_cookie_t& cookie, const long page_size = 0, const std::vector<attr_name_t>* custom_attrs = 0, const std::pair<std::string, bool>* sort_attr = 0, const std::string* query = 0);

		ldap_error_code_t GetNoGroupUsers(std::vector<ldap_user_info>& group_users, page_cookie_t& cookie, const long page_size, const std::vector<attr_name_t>* custom_attrs, const std::pair<std::string, bool>* sort_attr, const std::string* query);
		virtual ldap_error_code_t GetGroupUsers(const char* group_dn, std::vector<ldap_user_info>& group_users, page_cookie_t& cookie, const long page_size = 0, const std::vector<attr_name_t>* custom_attrs = 0, const std::pair<std::string, bool>* sort_attr = 0, const std::string* query = 0);
		virtual ldap_error_code_t GetGroupUsers(const char* group_dn, std::vector<ldap_user_info>& group_users, const std::vector<attr_name_t>* custom_attrs = 0, const std::pair<std::string, bool>* sort_attr = 0, const std::string* query = 0);

		ldap_error_code_t GetGroupUsers_noMemberOf(const char* group_dn, std::vector<ldap_user_info>& group_users, page_cookie_t& cookie, const long page_size = 0, const std::vector<attr_name_t>* custom_attrs = 0, const std::pair<std::string, bool>* sort_attr = 0, const std::string* query = 0);
		ldap_error_code_t GetGroupUsersUIDs(const char* group_dn, std::string &uids_filter);

		ldap_error_code_t GetAllAttributes(const char* dn, const char* filter, const std::vector<attr_name_t>* custom_attrs, std::map<attr_name_t, std::vector<attr_value_t>>& attrs);
		ldap_error_code_t GetAllAttributes(const char* dn, const std::vector<attr_name_t>* custom_attrs, std::map<attr_name_t, std::vector<attr_value_t>>& attrs);

		virtual ldap_error_code_t GetAllGroupsOfUser(const ldap_user_info& user, const unsigned long user_primary_group_id, const std::string& objectSid, std::set<group_dn>& ret) = 0;
		ldap_error_code_t GetAllGroupsOfUser_noMemberOf(const ldap_user_info& user, const unsigned long user_primary_group_id, const std::string& objectSid, std::set<group_dn>& ret);

		void SetLDAPErrorHandler(const boost::function<void(const ldap_error_code_t)>& functor);
		static bool IsErrorReconnect(const ldap_error_code_t err);

		LDAPCore::CheckLoginResult LoginUser_CheckLogin(string_view login, tc::ldap_user_info& found, std::map<std::string, VS_RegGroupInfo>& user_reg_groups, std::string& user_at_domain);
		bool LoginUser_CheckPassword(string_view login, string_view password, string_view user_dn);
		LDAP* NewAuthLDAPConn();
		
		int WriteAvatar(const std::string& dn, const std::string& attr, const std::vector<std::uint8_t>& buff);
		int DeleteAvatar(const std::string& dn);

	private:
		ProcessAvatarsHandler m_avatarsHandler;

		void DeInitUpdateNestedCacheThread();
	};

}
