#pragma once

#ifdef _WIN32

#ifndef __wtypes_h__
#include <wtypes.h>
#endif

#ifndef __WINDEF_
#include <windef.h>
#endif

#ifndef LDAP_UNICODE
#define LDAP_UNICODE 1
#endif

#define SECURITY_WIN32 1

#include <windows.h>
#include <windns.h>
#include <winldap.h>

#ifndef UNICODE
#define UNICODE
#include <security.h>
#undef  UNICODE
#endif

#include "VS_LDAPCoreImp_Common.h"

namespace tc {

	class VS_LDAPCoreImp_WinLDAP: public VS_LDAPCoreImp_Common
	{
		void LDAPReferral(LDAP*& ctx, PCHAR HostName, ULONG PortNumber);
		void ProcessLDAPError(ldap_error_code_t& ldap_error);

	protected:
		virtual bool isUselessDN(const std::string &dn) const = 0;
		bool ParseReferral(const std::wstring& ref, referral_t& obj) const;

	public:
		void InitLib() override;
		int ConnectServer(LDAP*& ctx, const char* host, const unsigned long port);
		ldap_error_code_t LDAPSearchPagedImp(LDAP* ld, const std::string& dn, const long& scope, const std::string& filter, const char** attrs, std::vector<attrs_t>& out, page_cookie_t& cookie, const long page_size = LDAP_DEFAULT_PAGE_SIZE, const std::pair<std::string, bool>* sort_attr = 0, const bool changed_ctx = false);

	private:
		class RAII_SearchHandle
		{
			VS_Locked<PLDAP> m_ctx;			// ldap context
			VS_Locked<std::string>	m_currentBaseDN;
			VS_Locked<PLDAPSearch>	m_h;		// system search handle

			VS_WithLock<std::vector<referral_t>>					m_refs;	// referrals info
			decltype(m_refs)										m_refs_chased;
			VS_WithLock<std::vector<std::pair<PLDAP, PLDAPSearch>>> m_clean_up_sys_handles;
		public:
			RAII_SearchHandle(PLDAP ctx, PLDAPSearch search_handle, const std::string& new_BaseDN)
			{
				m_ctx.set(ctx);
				m_h.set(search_handle);
				m_currentBaseDN.set(new_BaseDN);
			}
			~RAII_SearchHandle()
			{
				{
					VS_AutoLock lock(&m_clean_up_sys_handles);
					for (const auto& it : m_clean_up_sys_handles)
						ldap_search_abandon_page(it.first, it.second);
				}
				ldap_search_abandon_page(m_ctx.get(), m_h.get());
			}
			PLDAPSearch handle() const
			{
				return m_h.get();
			}
			void handle(PLDAPSearch new_h)
			{
				{
					VS_AutoLock lock(&m_clean_up_sys_handles);
					m_clean_up_sys_handles.emplace_back(m_ctx.get(), m_h.get());
				}
				m_h.set(new_h);
			}
			PLDAP ctx() const
			{
				return m_ctx.get();
			}
			void new_ctx(PLDAP new_ctx)
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

	protected:	// temporary protected, make private?
		// todo(kt): use syncronized
		VS_Lock				m_page_map_lock;
		page_map_t			m_page_map;

	protected:
		virtual bool IsAvatarsAttr(string_view attr) const = 0;
	};
}	// namespace tc
#endif	// ifdef _WIN32
