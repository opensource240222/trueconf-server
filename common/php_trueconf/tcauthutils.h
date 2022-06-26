#pragma once

#if defined(_WIN32) // Windows only logic

#include <windows.h>
#include <aclapi.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>

/* Authorization and access check utilities (portable? reusable part) */
/* Author: Artem Boldarev, 18.05.2015 */
/* 27.05.2015 - SystemSID and SystemACL classes */
/* 28.05.2015 - split module. trueconf_php.dll specific utilities are in "tcauthutils_php.cc". */
/* 13.07.2015 - some C bindings for string conversion functiionas has been added. */

#ifdef __cplusplus

#include <vector>

namespace tc_auth_utils {

	const size_t MAX_DNS_NAME_LEN = 255;

	// !!! utility functions !!!
	extern HANDLE GetUserTokenHandle(const wchar_t *user, const wchar_t *domain, const wchar_t *password);
	extern bool   CheckUserLogin(const wchar_t *user, const wchar_t *domain, const wchar_t *password);

	// returns false if computer is not in domain
	extern bool   GetDomainName(wchar_t *domain_name_buf = NULL, size_t buf_len = 0);
	extern bool   IsInDomain(void);
	// SID manipulation

	// you should free SID with LocalFree
	// system name can be NULL
	extern PSID     GetSidByName(const wchar_t *system_name, const wchar_t *account_name, SID_NAME_USE *ptype = NULL);
	//extern wchar_t* GetNameBySid(const wchar_t *system_name, PSID *sid, SID_NAME_USE *ptype);

	// Yous have to use delete [] to free result.
	extern wchar_t *u8str_to_wstr(const char *str, const size_t len, size_t *res_len = NULL); // UTF8 string to wide string
	extern char    *wstr_to_u8str(const wchar_t *wstr, const size_t len, size_t *res_len = NULL); // Wide string to UTF8 string

	// !!! Utility Classes !!!
	// Parse user login string
	class LoginParser {
	public:
		enum {
			LOGIN_ILLEGAL,
			LOGIN_DOMAIN, //  domain\user, user@domain
			LOGIN_LOCAL,  // .\user
			LOGIN_DEFAULT //  user
		};

		LoginParser(const char *login_string, const size_t len);
		virtual ~LoginParser();

		void     Parse();
		bool     IsParsed() const;

		const wchar_t *GetUser(size_t *out_len = NULL) const;
		const wchar_t *GetDomain(size_t *out_len = NULL) const;
		int      GetLoginType() const;
	private:
		// for internal use only
		enum {
			SUBTYPE_NONE,
			SUBTYPE_DOMAIN_USER,   // domain\user
			SUBTYPE_USER_AT_DOMAIN // user@domain
		};
		void FindLoginType();
	private:
		// all length fileds will store string lengths in UTF16 code points
		bool     m_parsed;
		wchar_t *m_login_string;
		size_t   m_login_string_len;

		int      m_type;
		int      m_subtype; // domain\user or user@domain

		wchar_t *m_user;
		size_t   m_user_len;

		wchar_t *m_domain;
		size_t   m_domain_len;
	};

	// convert system error code to text message in English
	class SystemErrorTextualizer {
	public:
		static const size_t DEFAULT_BUFSZ = 2048;

		SystemErrorTextualizer();
		virtual ~SystemErrorTextualizer();


		void UpdateLastErrorCode();
		void SetErrorCode(const DWORD ecode);

		const wchar_t *GetString() const;
		const char *GetStringUTF8() const;
		DWORD GetErrorCode() const;
	private:
		void UpdateMessageStrings();
	private:
		DWORD m_ecode;
		std::vector<wchar_t> m_wmsg;
		std::vector<char> m_msg_utf8;
	};


	// Common System Object class - mostly to simplify error handling
	class SystemCommon {
	public:
		SystemCommon(BOOL status = FALSE, bool is_initialized = false, DWORD error_code = ERROR_SUCCESS);

		virtual ~SystemCommon();

		virtual BOOL GetStatus(void) const;
		virtual bool IsInitialized(void) const;
		virtual DWORD GetErrorCode() const;
	protected:
		virtual void SetStatus(const BOOL status);
		virtual void SetInitialized(const bool is_initialized);
		virtual void SetErrorCode(const DWORD error_code);
		virtual void UpdateErrorCode(void); // set error  code to GetLastErrorCode()
	private:
		BOOL  m_status; // operation status
		bool  m_is_initialized; // the class was initialized
		DWORD m_error_code; // error code
	};

	// permit copying of object
	class SystemNonCopyable {
	public:
		SystemNonCopyable() {};
	private:
		SystemNonCopyable(const SystemNonCopyable &obj);
		SystemNonCopyable & operator = (const SystemNonCopyable &obj);
	};

	// System SID
	class SystemSID : public SystemCommon, SystemNonCopyable {
	public:
		// we will use NT Authority by default
		SystemSID(SID_IDENTIFIER_AUTHORITY *id_authority = NULL);
		// accquire existing SID
		SystemSID(PSID sid, bool free_sid = true, bool use_LocalFree = true);
		SystemSID(const wchar_t *user_or_group_name, const wchar_t *system_name = NULL);

		// dtor
		virtual ~SystemSID();

		PSID                      GetSID(void);
		const SID_IDENTIFIER_AUTHORITY *GetIdentifierAuthority(void);
		// you can add up to 8 sub authorities
		bool                      AddSubAuthority(DWORD sub_authority);
		bool                      Initialize(void);
	private:
		bool                       m_free_sid;
		bool                       m_use_local_free; // Free SID using local free
		std::vector<DWORD>         m_sub_authorities;
		SID_IDENTIFIER_AUTHORITY   m_id_authority;
		PSID                       m_sid;
	};

	class SystemACL : public SystemCommon, SystemNonCopyable {
	public:
		// Merge two ACLs and create new one.
		static bool Merge(SystemACL &result,
			const SystemACL &acl1, const SystemACL &acl2,
			const bool change_inheritance = false,     // change grfInheritance in every EXPLICIT_ACCESSW
			const DWORD inheritance = NO_INHERITANCE); // replace grfInheritance with this value
	public:
		SystemACL(void);
		// parse existing ACL
		SystemACL(ACL *acl, bool free_acl = true);
		virtual ~SystemACL();

		void                    AddExplicitAccess(const EXPLICIT_ACCESSW &ea);
		void                    ClearExplicitAccessSlots(void);
		size_t                  GetExplicitAccessSlotsCount(void) const;
		const EXPLICIT_ACCESSW *GetExplicitAccessSlots(void) const;
		ACL                    *GetACL(void);
		bool                    Initialize(void);
	private:
		bool                           m_free_acl;
		std::vector<EXPLICIT_ACCESSW>  m_acccess_slots;
		ACL                           *m_acl;
	};
}

#endif /* __cplusplus */

	/*!!! C bindings !!!*/
#ifdef __cplusplus
extern "C" {
#endif

/*
UTF8->WideChar
WideChar->UTF8
We will use it in ldap.c

Main implementation is in tcauthutils.cpp
*/

extern wchar_t *tc_u8str_to_wstr(const char *str, const size_t len, size_t *res_len);
extern void tc_free_wstr(wchar_t *wstr);

extern char *tc_wstr_to_u8str(const wchar_t *wstr, const size_t len, size_t *res_len);
extern void tc_free_u8str(char *wstr);

#ifdef __cplusplus
}
#endif

#endif
