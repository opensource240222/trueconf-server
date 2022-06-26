#if defined(_WIN32) // Windows only logic

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "tcauthutils.h"
#include <dsgetdc.h>
#include <lm.h>

/* !!! Login Parser !!! */

// empty wide string
#define EMPTY ((wchar_t *)L"")

// we will use it as separator - it is well known illegal character:
#define UTF16_BOM     L'\xFEFF'
#define UTF16_BOM_STR L"\xFEFF"

// characters not allowed in user names
// https://technet.microsoft.com/en-us/library/bb726984.aspx
#define NON_ALLOWED_USERNAME_CHARS   L"/\\[]:;|=,+*?<>"
// we will use this symbols to split login string into tokens
#define DELIMITERS          NON_ALLOWED_USERNAME_CHARS UTF16_BOM_STR
#define LOCAL_USER_DATABASE L"."

tc_auth_utils::LoginParser::LoginParser(const char *login_string, const size_t len)
	: m_parsed(false), m_login_string(NULL), m_login_string_len(0),
	m_type(LOGIN_ILLEGAL), m_subtype(SUBTYPE_NONE),
	m_user(EMPTY), m_user_len(0), m_domain(EMPTY), m_domain_len(0)
{
	// convert login string to UTF16
	m_login_string = u8str_to_wstr(login_string, len, &m_login_string_len);
}

tc_auth_utils::LoginParser::~LoginParser()
{
	delete[] m_login_string;
}

void tc_auth_utils::LoginParser::FindLoginType()
{
	wchar_t *pos;

	// We will replace found login and domain delimiter with known non allowed character (UTF16 BOM)
	// to simplify parsing
	if ( ((pos = wcsstr(m_login_string, L".\\")) != NULL) && pos == m_login_string) // .\user
	{
		m_type = LOGIN_LOCAL;
		pos[0] = UTF16_BOM;
		pos[1] = UTF16_BOM;
	}
	else if ( (pos = wcsstr(m_login_string, L"\\")) != NULL) // domain\user
	{
		m_type = LOGIN_DOMAIN;
		m_subtype = SUBTYPE_DOMAIN_USER;
		pos[0] = UTF16_BOM;
	}
	else if ((pos = wcsstr(m_login_string, L"@")) != NULL) // user@domain
	{
		m_type = LOGIN_DOMAIN;
		m_subtype = SUBTYPE_USER_AT_DOMAIN;
		pos[0] = UTF16_BOM;
	}
	else if (m_login_string_len > 0) // default: user
	{
		m_type = LOGIN_DEFAULT;
	}
	else
	{
		m_type = LOGIN_ILLEGAL;
	}
}

// check if there are no any characters from 'non_allowed' in 'wstr'
static bool wcsnchk(const wchar_t *wstr, const size_t len, const wchar_t *non_allowed)
{
	if (wstr == NULL)
		return false;

	for (size_t i = 0; (i < len) && wstr[i] != L'\0'; i++)
	{
		if (wcschr(non_allowed, wstr[i]))
			return false;
	}

	return true;
}

static bool wcschk(const wchar_t *wstr, const wchar_t *non_allowed)
{
	if (wstr == NULL)
		return false;

	for (size_t i = 0; wstr[i] != L'\0'; i++)
	{
		if (wcschr(non_allowed, wstr[i]))
			return false;
	}

	return true;
}

static bool is_valid_domain_name(const wchar_t *wstr)
{
	wchar_t ch;

	if (wstr == NULL)
		return false;

	for (size_t i = 0; (ch = wstr[i]) != L'\0'; i++)
	{
		if (!iswalnum(ch) && wcschr(L".-", ch) == NULL)
			return false;
	}

	return true;
}

static bool is_valid_username(const wchar_t *wstr)
{
	return wcschk(wstr, NON_ALLOWED_USERNAME_CHARS);
}

void tc_auth_utils::LoginParser::Parse()
{
	wchar_t *ctx; // wcstok_s context
	wchar_t *tok;
	wchar_t *tokens[2]; // user and domain (or domain and user - it depends)
	int pnum = 0;

	if (m_parsed)
		return;

	m_parsed = true;

	if (m_login_string_len == 0)
		return;

	FindLoginType();

	if (!wcsnchk(m_login_string, m_login_string_len, NON_ALLOWED_USERNAME_CHARS))
	{
		m_type = LOGIN_ILLEGAL;
		return;
	}

	if (m_type == LOGIN_ILLEGAL)
		return;

	// parse tokens
	for (ctx = tok = NULL, pnum = 0; (tok = wcstok_s((pnum == 0 ? m_login_string : NULL), DELIMITERS, &ctx)) != NULL; pnum++)
	{
		if (pnum >= (sizeof(tokens) / sizeof(tokens[0])))
			break;
		else
		{
			tokens[pnum] = tok;
		}
	}

	if (m_type == LOGIN_DOMAIN && pnum == 2)
	{
		if (m_subtype == SUBTYPE_DOMAIN_USER)
		{
			m_domain = tokens[0];
			m_domain_len = wcslen(tokens[0]);
			m_user = tokens[1];
			m_user_len = wcslen(tokens[1]);
		}
		else if (m_subtype == SUBTYPE_USER_AT_DOMAIN)
		{
			m_user = tokens[0];
			m_user_len = wcslen(tokens[0]);
			m_domain = tokens[1];
			m_domain_len = wcslen(tokens[1]);
		}
		else
		{
			m_type = LOGIN_ILLEGAL;
			m_subtype = SUBTYPE_NONE;
		}

	}
	else if ((m_type == LOGIN_DEFAULT || m_type == LOGIN_LOCAL) && pnum == 1)
	{
		m_user = tokens[0];
		m_user_len = wcslen(tokens[0]);

		if (m_type == LOGIN_LOCAL)
		{
			m_domain = (wchar_t *)LOCAL_USER_DATABASE;
			m_domain_len = wcslen(LOCAL_USER_DATABASE);
		}
	}
	else
	{
		m_type = LOGIN_ILLEGAL;
	}


	// validate user name
	if (!is_valid_username(m_user))
	{
		m_type = LOGIN_ILLEGAL;
	}

	if ((m_domain != NULL && m_domain[0] != L'\0') && !is_valid_domain_name(m_domain))
	{
		m_type = LOGIN_ILLEGAL;

	}

	if (m_type == LOGIN_ILLEGAL)
	{
		m_user = EMPTY;
		m_user_len = 0;
		m_domain = EMPTY;
		m_domain_len = 0;
	}
}

bool tc_auth_utils::LoginParser::IsParsed() const
{
	return m_parsed;
}

// getters
int tc_auth_utils::LoginParser::GetLoginType() const
{
	return m_type;
}

const wchar_t *tc_auth_utils::LoginParser::GetDomain(size_t *out_len) const
{
	if (out_len != NULL)
		*out_len = m_domain_len;

	return m_domain;
}

const wchar_t *tc_auth_utils::LoginParser::GetUser(size_t *out_len) const
{
	if (out_len != NULL)
		*out_len = m_user_len;

	return m_user;
}

/* System Error Textualizer */

tc_auth_utils::SystemErrorTextualizer::SystemErrorTextualizer()
	: m_wmsg(DEFAULT_BUFSZ), m_msg_utf8(DEFAULT_BUFSZ), m_ecode(ERROR_SUCCESS)
{
	m_wmsg[0] = L'\0';
	m_msg_utf8[0] = '\0';
}

tc_auth_utils::SystemErrorTextualizer::~SystemErrorTextualizer()
{}

void tc_auth_utils::SystemErrorTextualizer::UpdateMessageStrings()
{
	wchar_t *str;

	m_wmsg.resize(DEFAULT_BUFSZ);
	m_msg_utf8.resize(DEFAULT_BUFSZ);

	// get error message
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, m_ecode,
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		(wchar_t *)&str, 0, NULL);

	if (str == NULL)
	{
		m_wmsg[0] = L'\0';
		m_msg_utf8[0] = '\0';
	}
	else
	{
		size_t len = wcslen(str);

		if (len > m_wmsg.size())
		{
			m_wmsg.resize(len + 1);
		}

		// delete \r\n
		if (len > 2)
		{
			str[len] = L'\0';
			str[len - 1] = L'\0';
		}

		memcpy((void *)&m_wmsg[0], (void *)str, (len + 1) * sizeof(wchar_t));
		LocalFree(str);

		size_t utf8_len_needed = WideCharToMultiByte(CP_UTF8, 0, &m_wmsg[0], len + 1, NULL, 0, NULL, NULL);

		if (utf8_len_needed > 0)
		{
			if (utf8_len_needed >= m_msg_utf8.size())
			{
				m_msg_utf8.resize(utf8_len_needed);
			}

			WideCharToMultiByte(CP_UTF8, 0, &m_wmsg[0], len + 1, &m_msg_utf8[0], m_msg_utf8.size(), NULL, NULL);
		}
	}
}

void tc_auth_utils::SystemErrorTextualizer::UpdateLastErrorCode()
{
	m_ecode = GetLastError();
	UpdateMessageStrings();
}

void tc_auth_utils::SystemErrorTextualizer::SetErrorCode(const unsigned long ecode)
{
	m_ecode = ecode;
	UpdateMessageStrings();
}

const wchar_t *tc_auth_utils::SystemErrorTextualizer::GetString() const
{
	return &m_wmsg[0];
}

const char *tc_auth_utils::SystemErrorTextualizer::GetStringUTF8() const
{
	return &m_msg_utf8[0];
}

DWORD tc_auth_utils::SystemErrorTextualizer::GetErrorCode() const
{
	return m_ecode;
}


/* !!! System Common Class !!! */

tc_auth_utils::SystemCommon::SystemCommon(BOOL status, bool is_initialized, DWORD error_code)
	: m_status(status), m_is_initialized(is_initialized), m_error_code(error_code)
{}

tc_auth_utils::SystemCommon::~SystemCommon()
{}

BOOL tc_auth_utils::SystemCommon::GetStatus(void) const
{
	return m_status;
}

bool tc_auth_utils::SystemCommon::IsInitialized(void) const
{
	return m_is_initialized;
}

DWORD tc_auth_utils::SystemCommon::GetErrorCode(void) const
{
	return m_error_code;
}

void tc_auth_utils::SystemCommon::SetStatus(const BOOL status)
{
	m_status = status;
}

void tc_auth_utils::SystemCommon::SetInitialized(const bool is_initialized)
{
	m_is_initialized = is_initialized;
}

void tc_auth_utils::SystemCommon::SetErrorCode(const DWORD error_code)
{
	m_error_code = error_code;
}

void tc_auth_utils::SystemCommon::UpdateErrorCode(void)
{
	SetErrorCode(GetLastError());
}

/* !!! System SID class !!! */

tc_auth_utils::SystemSID::SystemSID(SID_IDENTIFIER_AUTHORITY *id_authority)
	: SystemCommon(FALSE, false), m_sid(NULL), m_free_sid(true), m_use_local_free(false)
{
	SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;

	if (id_authority == NULL)
	{
		m_id_authority = nt_authority;
	}
	else
	{
		m_id_authority = *id_authority;
	}
}

tc_auth_utils::SystemSID::SystemSID(PSID sid, bool free_sid, bool use_LocalFree)
	: SystemCommon(FALSE, false), m_sid(sid), m_free_sid(free_sid), m_use_local_free(use_LocalFree)
{
	if (sid == NULL)
	{
		SetInitialized(false);
		return;
	}
	SetStatus(TRUE);
	SetInitialized(true);
}

tc_auth_utils::SystemSID::SystemSID(const wchar_t *user_or_group_name, const wchar_t *system_name)
	: SystemCommon(FALSE, false), m_free_sid(true), m_use_local_free(true)
{
	m_sid = GetSidByName(system_name, user_or_group_name, NULL);
	SetInitialized(true);
	if (m_sid == NULL)
	{
		UpdateErrorCode();
		return;
	}

	SetStatus(TRUE);
}

tc_auth_utils::SystemSID::~SystemSID()
{
	if (m_sid != NULL && m_free_sid)
	{
		if (m_use_local_free)
			LocalFree(m_sid);
		else
			FreeSid(m_sid);
	}
}

PSID tc_auth_utils::SystemSID::GetSID(void)
{
	if (!IsInitialized() || !GetStatus())
		return NULL;

	return m_sid;
}

const SID_IDENTIFIER_AUTHORITY *tc_auth_utils::SystemSID::GetIdentifierAuthority(void)
{
	if (!IsInitialized() || !GetStatus())
	{
		return NULL;
	}

	return GetSidIdentifierAuthority(m_sid);
}

bool tc_auth_utils::SystemSID::AddSubAuthority(DWORD sub_authority)
{
	if (m_sub_authorities.size() >= 8 || IsInitialized())
		return false;

	m_sub_authorities.push_back(sub_authority);
	return true;
}

bool tc_auth_utils::SystemSID::Initialize(void)
{
	DWORD sa[8]; // sub_authorities
	BYTE sub_auth_num = (BYTE)m_sub_authorities.size();
	BOOL status;

	if (IsInitialized())
		return (GetStatus() == TRUE);

	SetInitialized(true);

	ZeroMemory(sa, sizeof(sa));
	for (size_t i = 0; i < m_sub_authorities.size(); i++)
	{
		sa[i] = m_sub_authorities[i];
	}

	status = AllocateAndInitializeSid(&m_id_authority,
		sub_auth_num,
		sa[0], sa[1], sa[2], sa[3], sa[4], sa[5], sa[6], sa[7],
		&m_sid);

	SetStatus(status);


	if (!GetStatus())
	{
		UpdateErrorCode();
		return false;
	}

	return true;
}

/* !!! End System SID class !!!*/

/* !!! SystemACL class !!! */

tc_auth_utils::SystemACL::SystemACL(void)
	: SystemCommon(FALSE, false), m_acl(NULL), m_free_acl(true)
{}

tc_auth_utils::SystemACL::SystemACL(ACL *acl, bool free_acl)
{
	SystemACL();

	DWORD ecode;
	ULONG count;
	EXPLICIT_ACCESSW *entries = NULL;
	m_free_acl = free_acl;
	m_acl = acl;

	if (acl == NULL)
	{
		SetErrorCode(ERROR_INVALID_PARAMETER);
		SetStatus(FALSE);
		SetInitialized(true);
		return;
	}

	// "parse" existing acl
	ecode = GetExplicitEntriesFromAclW(acl, &count, &entries);

	if (ecode != ERROR_SUCCESS)
	{
		SetErrorCode(ecode);
		SetStatus(FALSE);
		SetInitialized(true);
		return;
	}

	// save all EXPLICIT_ACCESS structures
	for (size_t i = 0; i < count; i++)
	{
		m_acccess_slots.push_back(entries[i]);
	}

	// free entries
	LocalFree(entries);

	SetErrorCode(ERROR_SUCCESS);
	SetStatus(TRUE);
	SetInitialized(true);
}

tc_auth_utils::SystemACL::~SystemACL()
{
	if (m_acl != NULL && m_free_acl)
		LocalFree(m_acl);
}

void tc_auth_utils::SystemACL::AddExplicitAccess(const EXPLICIT_ACCESSW &ea)
{
	m_acccess_slots.push_back(ea);
}

void tc_auth_utils::SystemACL::ClearExplicitAccessSlots(void)
{
	if (!IsInitialized())
		m_acccess_slots.clear();
}

size_t tc_auth_utils::SystemACL::GetExplicitAccessSlotsCount(void) const
{
	return m_acccess_slots.size();
}

const EXPLICIT_ACCESSW *tc_auth_utils::SystemACL::GetExplicitAccessSlots(void) const
{
	if (GetExplicitAccessSlotsCount() == 0)
		return NULL;

	return &m_acccess_slots[0];
}

ACL *tc_auth_utils::SystemACL::GetACL(void)
{
	return m_acl;
}

bool tc_auth_utils::SystemACL::Initialize(void)
{
	DWORD ecode;

	if (IsInitialized())
		return (GetStatus() == TRUE);
	SetInitialized(true);

	ecode = SetEntriesInAclW(
		(ULONG) GetExplicitAccessSlotsCount(),
		(EXPLICIT_ACCESSW *) GetExplicitAccessSlots(),
		NULL,
		&m_acl);

	SetErrorCode(ecode);
	SetStatus(ecode == ERROR_SUCCESS ? TRUE : FALSE);

	return (GetStatus() == TRUE);
}

bool tc_auth_utils::SystemACL::Merge(SystemACL &result,
	const SystemACL &acl1, const SystemACL &acl2,
	const bool change_inheritance,
	const DWORD inheritance)
{
	size_t i;
	EXPLICIT_ACCESSW ea;
	const EXPLICIT_ACCESSW *arr;

	if (!acl1.GetStatus()
		|| !acl2.GetStatus()
		|| result.GetExplicitAccessSlotsCount() > 0)
		return false;

	arr = acl1.GetExplicitAccessSlots();
	for (i = 0; i < acl1.GetExplicitAccessSlotsCount(); i++)
	{
		ea = arr[i];

		if (change_inheritance)
			ea.grfInheritance = inheritance;

		result.AddExplicitAccess(ea);
	}

	arr = acl2.GetExplicitAccessSlots();
	for (i = 0; i < acl2.GetExplicitAccessSlotsCount(); i++)
	{
		ea = arr[i];

		if (change_inheritance)
			ea.grfInheritance = inheritance;

		result.AddExplicitAccess(ea);
	}

	return result.Initialize();
}

/* !!! End SystemACL class !!! */

/*!!! utilities !!!*/
static const wchar_t *sanitize_wstring_value(const wchar_t *str)
{
	if (str == NULL)
		return NULL;

	if (str[0] == L'\0')
	{
		return NULL;
	}

	return str;
}

static BOOL TryLogonUser(const wchar_t *user, const wchar_t *domain, const wchar_t *password, HANDLE *token)
{
	BOOL res;
	res = LogonUserW(user, sanitize_wstring_value(domain), password,
		LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT, token);

	if (!res)
		res = LogonUserW(user, sanitize_wstring_value(domain), password,
		LOGON32_LOGON_BATCH, LOGON32_PROVIDER_DEFAULT, token);

	return res;
}

HANDLE tc_auth_utils::GetUserTokenHandle(const wchar_t *user, const wchar_t *domain, const wchar_t *password)
{
	HANDLE token = INVALID_HANDLE_VALUE;
	const wchar_t *pdomain = sanitize_wstring_value(domain);
	wchar_t domain_buf[MAX_DNS_NAME_LEN + 1] = {L'\0'};

	// Try to log-in without domain.
	if (pdomain == NULL && TryLogonUser(user, NULL, password, &token) == TRUE)
	{
		return token;
	}
	else if (pdomain != NULL) // Try to log-in with domain name, if it was specified.
	{
		if (TryLogonUser(user, pdomain, password, &token) == TRUE)
			return token;
	}
	else if (GetDomainName(domain_buf, MAX_DNS_NAME_LEN + 1)) // Domain name was not specified. Try to get it and log-in.
	{
		if (TryLogonUser(user, domain_buf, password, &token) == TRUE)
			return token;
	}

	return token;
}

bool tc_auth_utils::CheckUserLogin(const wchar_t *user, const wchar_t *domain, const wchar_t *password)
{
	HANDLE token;
	bool ret;

	token = GetUserTokenHandle(user, domain, password);

	if (token == INVALID_HANDLE_VALUE)
		ret = false;
	else
		ret = true;

	if (token != INVALID_HANDLE_VALUE)
		CloseHandle(token);

	return ret;
}


wchar_t *tc_auth_utils::u8str_to_wstr(const char *str, const size_t len, size_t *res_len)
{
	wchar_t *res = NULL;
	size_t wbuf_len;

	if (str == NULL)
		return NULL;

	// Get length of the string buffer in UTF16 code points
	wbuf_len = MultiByteToWideChar(CP_UTF8, 0, str, len + 1, NULL, 0);
	if (len != 0 && wbuf_len == 0)
		return NULL;
	else if (len == 0) // empty string
	{
		res = new wchar_t[1];
		res[0] = L'\0';

		if (res_len != NULL)
		{
			*res_len = 0;
		}

		return res;
	}

	// allocate buffer for new string
	res = new wchar_t[wbuf_len];

	// convertion
	if (MultiByteToWideChar(CP_UTF8, 0, str, len + 1, res, wbuf_len) == 0)
	{
		delete[] res;
		res = NULL;
		wbuf_len = 0;
	}

	if (res_len != NULL)
	{
		if (wbuf_len == 0)
			*res_len = 0;
		else
			*res_len = wbuf_len - 1;
	}

	return res;
}

char *tc_auth_utils::wstr_to_u8str(const wchar_t *wstr, const size_t len, size_t *res_len)
{
	char *res = NULL;
	size_t buf_len;

	if (wstr == NULL)
		return NULL;

	// Get length of the string buffer in UTF16 code points
	buf_len = WideCharToMultiByte(CP_UTF8, 0, wstr, len + 1, NULL, 0, NULL, NULL);

	if (len != 0 && buf_len == 0)
		return NULL;
	else if (len == 0) // empty string
	{
		res = new char[1];
		res[0] = '\0';

		if (res_len != NULL)
		{
			*res_len = 0;
		}

		return res;
	}

	// allocate buffer for new string
	res = new char[buf_len];

	// convertion
	if (WideCharToMultiByte(CP_UTF8, 0, wstr, len + 1, res, buf_len, NULL, NULL) == 0)
	{
		delete[] res;
		res = NULL;
		buf_len = 0;
	}

	if (res_len != NULL)
	{
		if (buf_len == 0)
			*res_len = 0;
		else
			*res_len = buf_len - 1;
	}

	return res;
}

bool tc_auth_utils::GetDomainName(wchar_t *domain_name_buf, size_t buf_len)
{
	DOMAIN_CONTROLLER_INFOW *dc_info = NULL;
	bool ret = false;
	DWORD ecode;

	ecode = DsGetDcNameW(NULL, NULL, NULL, NULL, DS_IS_DNS_NAME | DS_RETURN_DNS_NAME, &dc_info);
	if (ecode == ERROR_SUCCESS)
	{
		if (domain_name_buf != NULL && buf_len > 0)
			wcsncpy(domain_name_buf, dc_info->DomainName, buf_len);
		ret = true;
	}

	NetApiBufferFree(dc_info);

	return ret;
}

bool tc_auth_utils::IsInDomain(void)
{
	return GetDomainName();
}

PSID tc_auth_utils::GetSidByName(const wchar_t *system_name, const wchar_t *account_name, SID_NAME_USE *ptype)
{
	PSID sid = NULL;
	DWORD sid_size = 0;
    wchar_t *domain_name = NULL;
	DWORD domain_name_len = 0;
	SID_NAME_USE name_use = SidTypeInvalid;
	BOOL res;

	LookupAccountNameW(system_name, account_name, NULL, &sid_size, NULL, &domain_name_len, &name_use);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return NULL;

	// allocate buffer for SID
	sid = (PSID)LocalAlloc(LMEM_FIXED, sid_size);
	if (!sid)
		return NULL;

	domain_name = (wchar_t *)LocalAlloc(LMEM_FIXED, domain_name_len * sizeof(wchar_t));
	if (!domain_name)
	{
		LocalFree(sid);

		return NULL;
	}

	// Get SID
	res = LookupAccountNameW(system_name, account_name,
		sid, &sid_size,
		domain_name, &domain_name_len,
		&name_use);
	if (!res)
	{
		LocalFree(sid);
		LocalFree(domain_name);
		return NULL;
	}

	LocalFree(domain_name);
	SetLastError(ERROR_SUCCESS);

	if (ptype != NULL)
	{
		*ptype = name_use;
	}

	return sid;
}

// wrappers for pure C
extern "C" {
	wchar_t *tc_u8str_to_wstr(const char *str, const size_t len, size_t *res_len)
	{
		return tc_auth_utils::u8str_to_wstr(str, len, res_len);
	}

	void tc_free_wstr(wchar_t *wstr)
	{
		delete [] wstr;
	}

	char *tc_wstr_to_u8str(const wchar_t *wstr, const size_t len, size_t *res_len)
	{
		return tc_auth_utils::wstr_to_u8str(wstr, len, res_len);
	}

	void tc_free_u8str(char *u8str)
	{
		delete [] u8str;
	}
}

#endif