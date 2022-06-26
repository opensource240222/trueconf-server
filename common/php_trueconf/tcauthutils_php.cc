#if defined(_WIN32) // Windows only logic

#include "tcauthutils_php.h"

/* !!! Registry Access Checker !!! */

tc_auth_utils::RegistryAccessChecker::RegistryAccessChecker(HKEY hkey, wchar_t *path)
	: m_hkey(hkey), m_path(path), m_status(RACCESS_NONE)
{}

tc_auth_utils::RegistryAccessChecker::~RegistryAccessChecker()
{}

void tc_auth_utils::RegistryAccessChecker::SetStatusCode(int code)
{
	m_status = code;
	m_emsg.UpdateLastErrorCode();
}

void tc_auth_utils::RegistryAccessChecker::SetStatusCode(int code, DWORD ecode)
{
	m_status = code;
	m_emsg.SetErrorCode(ecode);
}

int tc_auth_utils::RegistryAccessChecker::GetAccessStatus() const
{
	return m_status;
}

const wchar_t *tc_auth_utils::RegistryAccessChecker::GetAccessErrorMessage() const
{
	return m_emsg.GetString();
}

const char *tc_auth_utils::RegistryAccessChecker::GetAccessErrorMessageUTF8() const
{
	return m_emsg.GetStringUTF8();
}

DWORD tc_auth_utils::RegistryAccessChecker::GetAccessErrorCode() const
{
	return m_emsg.GetErrorCode();
}

#define SafeCloseHandle(h) if (h != INVALID_HANDLE_VALUE && h != NULL) { CloseHandle(h); }
#define IsValidHandle(h) (h != NULL && h != INVALID_HANDLE_VALUE ? 1 : 0)
#define SafeRegCloseKey(k) (k == NULL ? ERROR_SUCCESS : RegCloseKey(k))

void tc_auth_utils::RegistryAccessChecker::Check(
	const wchar_t *username,
	const wchar_t *domain,
	const wchar_t *password,
	DWORD access_mask)
{
	HANDLE user_token = INVALID_HANDLE_VALUE;
	HKEY reg_key = NULL;
	DWORD ecode = ERROR_SUCCESS;
	DWORD sec_len;
	SECURITY_DESCRIPTOR *sec_desc = NULL;

	try {
		// Get User Handle Value
		user_token = GetUserTokenHandle(username, domain, password);
		if (!IsValidHandle(user_token))
		{
			SetStatusCode(RACCESS_AUTH_ERROR);
			throw std::exception();
		}

		// open registry key
		ecode = RegOpenKeyW(m_hkey, m_path, &reg_key);
		if (ecode != ERROR_SUCCESS)
		{
			SetStatusCode(RACCESS_REGISTRY_ACCESS_ERROR, ecode);
			throw std::exception();
		}

		// We will try to get registry security descriptor

		// get length of security descriptor
		sec_len = 0;
		ecode = RegGetKeySecurity(reg_key,
			OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
			NULL, &sec_len);
		if (ecode != ERROR_INSUFFICIENT_BUFFER)
		{
			SetStatusCode(RACCESS_SYSTEM_ERROR, ecode);
			throw std::exception();
		}

		// allocate memory for security descriptor
		sec_desc = (SECURITY_DESCRIPTOR *)new char[sec_len];

		// get registry key security descriptor
		ecode = RegGetKeySecurity(reg_key,
			OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
			sec_desc, &sec_len);
		if (ecode != ERROR_SUCCESS)
		{
			SetStatusCode(RACCESS_SYSTEM_ERROR, ecode);
			throw std::exception();
		}

		// !!! Perform Access Check !!!
		{
			GENERIC_MAPPING mapping = { 0 };
			PRIVILEGE_SET priv_set = { 0 };
			DWORD priv_set_len = sizeof(priv_set);
			BOOL status = FALSE;
			BOOL ret;
			DWORD granted_access = 0;

			mapping.GenericAll = KEY_ALL_ACCESS;
			mapping.GenericRead = KEY_READ;
			mapping.GenericWrite = KEY_WRITE;
			mapping.GenericExecute = KEY_EXECUTE;

			MapGenericMask(&access_mask, &mapping);

			ret = AccessCheck(sec_desc, user_token, access_mask, &mapping,
				&priv_set, &priv_set_len, &granted_access, &status);
			if (!ret)
			{
				SetStatusCode(RACCESS_SYSTEM_ERROR);
				throw std::exception();
			}

			if (status)
			{
				SetStatusCode(RACCESS_OK);
				throw std::exception();
			}
			else
			{
				SetStatusCode(RACCESS_DENIED);
				throw std::exception();
			}
		}
	}
	catch (...)
	{
	}

	SafeCloseHandle(user_token);
	SafeRegCloseKey(reg_key);
	char *p = (char *)sec_desc;
	delete[] p;
}

/* !!! User Access Checker !!! */

tc_auth_utils::UserAccessChecker::UserAccessChecker(
	const char *utf8_login_string, const size_t login_string_len,
	const char *utf8_password, const size_t pass_len,
	HKEY reg_hive,
	const char *utf8_reg_path, const size_t reg_path_len,
	const DWORD access_mask)

	: m_parser(utf8_login_string, login_string_len),
	m_password(u8str_to_wstr(utf8_password, pass_len, &m_pass_len)),
	is_checked(false), m_reg_checker(reg_hive, (m_reg_path = u8str_to_wstr(utf8_reg_path, reg_path_len))),
	m_status(RACCESS_DENIED), m_access_mask(access_mask)
{}

tc_auth_utils::UserAccessChecker::~UserAccessChecker()
{
	if (m_password != NULL)
		SecureZeroMemory(m_password, m_pass_len * sizeof(wchar_t));
	delete[] m_password;
	delete[] m_reg_path;
}

void tc_auth_utils::UserAccessChecker::Check()
{
	if (is_checked)
		return;

	is_checked = true;

	m_parser.Parse();
	if (m_parser.GetLoginType() == LoginParser::LOGIN_ILLEGAL)
	{
		m_status = RACCESS_PARSE_ERROR;
		return;
	}

	m_reg_checker.Check(m_parser.GetUser(), m_parser.GetDomain(), m_password, m_access_mask);

	m_status = m_reg_checker.GetAccessStatus();
}

int tc_auth_utils::UserAccessChecker::GetAccessStatus() const
{
	return m_status;
}

const wchar_t *tc_auth_utils::UserAccessChecker::GetAccessErrorMessage() const
{
	return m_reg_checker.GetAccessErrorMessage();
}

const char *tc_auth_utils::UserAccessChecker::GetAccessErrorMessageUTF8() const
{
	return m_reg_checker.GetAccessErrorMessageUTF8();
}

DWORD tc_auth_utils::UserAccessChecker::GetAccessErrorCode() const
{
	return m_reg_checker.GetAccessErrorCode();
}

tc_auth_utils::LoginParser &tc_auth_utils::UserAccessChecker::GetLoginParser()
{
	return m_parser;
}

bool tc_auth_utils::AuthorizeWithLoginString(
	const char *login_string_utf8, const size_t login_len,
	const char *password_utf8, const size_t pass_len,
	int &login_type, DWORD &error_code)
{
	LoginParser parser(login_string_utf8, login_len);
	wchar_t *password = NULL;
	bool status = false;

	// parse login
	parser.Parse();
	login_type = parser.GetLoginType();

	// check if login is valid
	if (parser.GetLoginType() == LoginParser::LOGIN_ILLEGAL)
	{
		return false;
	}

	// convert password to wide string
	password = u8str_to_wstr(password_utf8, pass_len);
	status = CheckUserLogin(parser.GetUser(), parser.GetDomain(), password);

	// save error code
	if (!status)
	{
		error_code = GetLastError();
	}
	else
	{
		error_code = ERROR_SUCCESS;
	}

	delete[] password;

	return status;
}

DWORD tc_auth_utils::CreateAuthRegistryObject(
	HKEY hive,
	const char *registry_path_utf8, const size_t registry_path_len,
	const char *group_name_utf8, const size_t group_name_len)
{
	DWORD error_code = ERROR_SUCCESS;
	HKEY reg_key = NULL;
	const wchar_t *reg_path = NULL;

	// check if registry path is specified
	if (hive == NULL || registry_path_utf8 == NULL || registry_path_len == 0)
	{
		return ERROR_INVALID_PARAMETER;
	}

	reg_path = u8str_to_wstr(registry_path_utf8, registry_path_len);

	// check if object is already exists
	if ((error_code = RegOpenKeyExW(hive, reg_path, 0, KEY_READ, &reg_key)) == ERROR_SUCCESS)
	{
		// authorization object already exists
		delete[] reg_path;
		RegCloseKey(reg_key);

		return ERROR_SUCCESS;
	}
	else if (error_code != ERROR_FILE_NOT_FOUND)
	{
		// unexpected error
		delete[] reg_path;

		return error_code;
	}

	// We will use throw instead of goto ;-)
	//
	// You may go, when you will go,
	// And I will stay, behind.
	//
	// https://github.com/FoOTOo/inftik/raw/master/lec1/paper/p261-knuth.pdf
	//
	try
	{
		// !!! build ACL !!!
		wchar_t *group_name = NULL;
		SystemSID admins_sid;
		SystemSID local_system_sid;
		SystemACL acl;
		EXPLICIT_ACCESSW ea;
		PSECURITY_DESCRIPTOR sd = NULL;
		SECURITY_ATTRIBUTES sa;

		// initialize admins SID
		admins_sid.AddSubAuthority(SECURITY_BUILTIN_DOMAIN_RID);
		admins_sid.AddSubAuthority(DOMAIN_ALIAS_RID_ADMINS);
		// initialize admins SID
		if (!admins_sid.Initialize())
		{
			// free resources on error
			error_code = admins_sid.GetErrorCode();

			throw std::exception(); // goto end; // ;-)
		}

		// add Administrators group to ACL
		ZeroMemory(&ea, sizeof(ea));
		ea.grfAccessPermissions = KEY_ALL_ACCESS;
		ea.grfAccessMode = SET_ACCESS;
		ea.grfInheritance = NO_INHERITANCE;
		ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
		ea.Trustee.ptstrName = (LPWSTR)admins_sid.GetSID();

		// add Administrators ACE to ACL
		acl.AddExplicitAccess(ea);

		// initialize Local System SID
		local_system_sid.AddSubAuthority(SECURITY_LOCAL_SYSTEM_RID);
		if (!local_system_sid.Initialize())
		{
			// free resources on error
			error_code = local_system_sid.GetErrorCode();

			throw std::exception(); // goto end; // ;-)
		}

		// add Local System to the ACL
		ZeroMemory(&ea, sizeof(ea));
		ea.grfAccessPermissions = KEY_ALL_ACCESS;
		ea.grfAccessMode = SET_ACCESS;
		ea.grfInheritance = NO_INHERITANCE;
		ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
		ea.Trustee.ptstrName = (LPWSTR)local_system_sid.GetSID();

		// add Local System to the ACL
		acl.AddExplicitAccess(ea);

		// convert group name to UTF16
		if (group_name_len > 0 && group_name_utf8 != NULL)
		{
			group_name = u8str_to_wstr(group_name_utf8, group_name_len);
		}

		// try to add group sid to ACL
		SystemSID group_sid(group_name);

		if (group_name != NULL)
		{
			delete[] group_name;
		}

		// add group to ACL
		if (group_name_len > 0)
		{
			if (group_sid.GetStatus() == TRUE) // OK
			{
				ZeroMemory(&ea, sizeof(ea));
				ea.grfAccessPermissions = KEY_READ;
				ea.grfAccessMode = SET_ACCESS;
				ea.grfInheritance = NO_INHERITANCE;
				ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
				ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
				ea.Trustee.ptstrName = (LPWSTR)group_sid.GetSID();

				// add group ACE to ACL
				acl.AddExplicitAccess(ea);
			}
			else // Bad group name or system error
			{
				if (group_sid.GetErrorCode() != ERROR_NONE_MAPPED) // OK if group is not exists
				{
					// probably, system error
					error_code = group_sid.GetErrorCode();
					throw std::exception(); // goto end; // ;-)
				}
			}
		}

		// build ACL
		if (!acl.Initialize())
		{
			error_code = acl.GetErrorCode();

			throw std::exception(); // goto end; // ;-)
		}

		// !!! create authorization object in registry !!!

		// Create security descriptor
		sd = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,
			SECURITY_DESCRIPTOR_MIN_LENGTH);

		// memory allocation error
		if (sd == NULL)
		{
			error_code = GetLastError();
			throw std::exception(); // goto end; // ;-)
		}

		// initialize Security Descriptor
		if (!InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION))
		{
			LocalFree(sd);

			error_code = GetLastError();
			throw std::exception(); // goto end; // ;-)
		}

		// Set ACL in Security Descriptor
		if (!SetSecurityDescriptorDacl(sd, TRUE, acl.GetACL(), FALSE))
		{
			LocalFree(sd);
			error_code = GetLastError();

			throw std::exception(); // goto end; // ;-)
		}

		// Initialize Security Attributes
		ZeroMemory(&sa, sizeof(sa));
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = sd;
		sa.bInheritHandle = FALSE;

		DWORD pos = 0;
		// create auth object itself
		error_code = RegCreateKeyExW(hive, reg_path, 0, NULL, 0, KEY_READ, &sa, &reg_key, &pos);

		// free security descriptor
		LocalFree(sd);
	}
// ;-)
//end:
	catch (...)
	{}

	if (reg_path != NULL)
	{
		delete[] reg_path;
	}

	if (reg_key != NULL && reg_key != hive)
	{
		RegCloseKey(reg_key);
	}

	return error_code;
}

#endif
