#pragma once

#ifndef _WIN32

#include "VS_LDAPCoreImp_Common.h"

#include <algorithm>

namespace tc {

	class VS_LDAPCoreImp_OpenLDAP: public VS_LDAPCoreImp_Common
	{
	protected:
		virtual bool isUselessDN(const std::string &dn) const = 0;

	public:

		void InitLib();
		int ConnectServer(LDAP*& ctx, const char* host, const unsigned long port);
		ldap_error_code_t LDAPSearchPagedImp(LDAP* ld, const std::string& dn, const long& scope, const std::string& filter, const char** attrs, std::vector<attrs_t>& out, page_cookie_t& cookie, const long page_size = LDAP_DEFAULT_PAGE_SIZE, const std::pair<std::string, bool>* sort_attr = 0, const bool changed_ctx = false);

	private:
		class RAII_SearchHandle
		{
			VS_Locked<LDAP*> m_ctx;			// ldap context
			VS_Locked<std::string>	m_currentBaseDN;
			VS_Locked<berval>	m_h;		// system search handle

			VS_WithLock<std::vector<referral_t>>					m_refs;	// referrals info
			decltype(m_refs)										m_refs_chased;
			vs::Synchronized<std::vector<berval>>					m_clean_up_sys_handles;
			//VS_WithLock<std::vector<std::pair<LDAP*, struct berval>>> m_clean_up_sys_handles;
		public:
			RAII_SearchHandle(LDAP* ctx, berval search_handle, const std::string& new_BaseDN)
			{
				m_ctx.set(ctx);
				m_h.set(search_handle);
				m_currentBaseDN.set(new_BaseDN);
			}
			~RAII_SearchHandle()
			{
#ifdef _WIN32
#else
				auto lock = m_clean_up_sys_handles.lock();
				if (lock)
					for (auto&& i : *lock)
						ldap_memfree(i.bv_val);
#endif
				////#ifdef _WIN32	// no such func at OpenLDAP
				//			{
				//				VS_AutoLock lock(&m_clean_up_sys_handles);
				//				for (const auto& it : m_clean_up_sys_handles)
				//	//				ldap_search_abandon_page(it.first, it.second);
				//			}
				//	//		ldap_search_abandon_page(m_ctx.get(), m_h.get());
				////#endif
			}
			berval handle() const
			{
				return m_h.get();
			}
			void handle(berval new_h)
			{
				m_clean_up_sys_handles->emplace_back(m_h.get());
				m_h.set(new_h);
			}
			LDAP* ctx() const
			{
				return m_ctx.get();
			}
			void new_ctx(LDAP* new_ctx)
			{
				m_ctx.set(new_ctx);
			}
			const std::string base_dn() const
			{
				return m_currentBaseDN.get();
			}
			void base_dn(const std::string& new_baseDN)
			{
				m_currentBaseDN.set(new_baseDN);
			}
			const std::vector<referral_t> refs() const
			{
				VS_AutoLock lock(&m_refs);
				return m_refs;
			}
			bool add_refer(const referral_t& ref)
			{
				VS_AutoLock lock(&m_refs);
				if (std::find(m_refs_chased.begin(), m_refs_chased.end(), ref) != m_refs_chased.end())
					return false;
				m_refs.push_back(ref);
				return true;
			}
			void remove_refer(const referral_t& ref)
			{
				VS_AutoLock lock(&m_refs);
				m_refs.erase(std::remove(m_refs.begin(), m_refs.end(), ref), m_refs.end());
				m_refs_chased.emplace_back(ref);
			}
			unsigned long num_chased_refs()
			{
				VS_AutoLock lock(&m_refs);
				return m_refs_chased.size();
			}
		};
		typedef std::map<page_cookie_t, std::shared_ptr<RAII_SearchHandle>> page_map_t;

		bool MakePageControl(std::shared_ptr<RAII_SearchHandle>& h, const long page_size_to_use, LDAPControl*& pageControl);

	protected:	// temporary protected, make private?
				// todo(kt): use syncronized
		VS_Lock				m_page_map_lock;
		page_map_t			m_page_map;
	
	protected:
		virtual bool IsAvatarsAttr(string_view attr) const = 0;
	};

} // namespace tc

#endif // ifndef _WIN32
