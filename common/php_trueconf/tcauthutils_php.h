#pragma once

#if defined(_WIN32) // Windows only logic

#include "tcauthutils.h"

/* trueconf_php.dll specific authorization and access check utilities */
/* Author: Artem Boldarev, 28.05.2015 */

namespace tc_auth_utils {
	// Access check status codes
	enum RegistryAccessCodes {
		RACCESS_NONE = -2, // for internal use only
		RACCESS_SYSTEM_ERROR = -1,
		RACCESS_OK = 0,
		RACCESS_AUTH_ERROR = 1,
		RACCESS_REGISTRY_ACCESS_ERROR = 2,
		RACCESS_DENIED = 3,
		RACCESS_PARSE_ERROR = 4,
		RACCESS_AUTH_OBJECT_ERROR = 5,
		RACCESS_AUTH_TIMEOUT = 6,
	};

	// registry acccess checker
	class RegistryAccessChecker {
	public:
		RegistryAccessChecker(HKEY hkey, wchar_t *path);
		virtual ~RegistryAccessChecker();

		void Check(const wchar_t *username, const wchar_t *domain, const wchar_t *password, DWORD access_mask);

		int GetAccessStatus() const;
		const wchar_t *GetAccessErrorMessage() const;
		const char *GetAccessErrorMessageUTF8() const;

		DWORD GetAccessErrorCode() const; // get WinAPI error code
	private:
		void SetStatusCode(int code);
		void SetStatusCode(int code, DWORD ecode);
	private:
		SystemErrorTextualizer m_emsg;
		const wchar_t *m_path;
		HKEY m_hkey;

		int m_status;
	};

	// Check user access to specified registry key
	class UserAccessChecker {
	public:
		UserAccessChecker(const char *utf8_login_string, const size_t login_string_len,
			const char *utf8_password, const size_t pass_len,
			HKEY reg_hive,
			const char *utf8_reg_path, const size_t reg_path_len,
			const DWORD access_mask);

		virtual ~UserAccessChecker();

		void Check();
		bool IsChecked() const;

		int GetAccessStatus() const;
		const wchar_t *GetAccessErrorMessage() const;
		const char *GetAccessErrorMessageUTF8() const;
		DWORD GetAccessErrorCode() const; // get WinAPI error code
		LoginParser &GetLoginParser();
	private:
		bool                  is_checked;
		LoginParser           m_parser;
		RegistryAccessChecker m_reg_checker;
		wchar_t              *m_password;
		wchar_t              *m_reg_path;
		size_t                m_pass_len;
		int                   m_status;
		DWORD                 m_access_mask;
	};

	// Authorize user with only login string and password
	// Returns true on suceess.
	// You can obtain system error code and login parse status via two last parameters.
	extern bool AuthorizeWithLoginString(
		const char *login_string_utf8, const size_t login_len,
		const char *password_utf8, const size_t pass_len,
		int &login_type, DWORD &error_code);

	// Create TC Authorization Registry Object (if it is not already exists)
	// Returns system error code on failure (ERROR_SUCCESS on success)
	extern DWORD CreateAuthRegistryObject(
		HKEY hive,                         // well known registry hive
		const char *registry_path_utf8,    // registry path in UTF8
		const size_t registry_path_len,
		const char *group_name_utf8,       // group name
		const size_t group_name_len
		);
}

#endif
