/**
****************************************************************************
*
* Project: server services
*
****************************************************************************/

/**
* \file VS_ConfMemStorage.cpp
* Server Database Storage class implementation functions
*
*/


#include "../../common/std/CallLog/VS_DBCallLogPostgres.h"
#include "VS_ConfMemStorage.h"
#include "VS_VCSConfRestrict.h"
#include "../../ServerServices/VS_ReadLicense.h"
#include "std/cpplib/md5.h"
#include "../../common/std/cpplib/VS_IntConv.h"
#include "../../common/std/cpplib/VS_Utils.h"
#include "../../ServersConfigLib/VS_ServersConfigLib.h"
#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "ProtectionLib/Protection.h"
#include "../../BaseServer/Services/VS_AddressBookService.h"		// for SearchAddressBook_Task and AddToAddressBook_Task
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "../../common/std/cpplib/VS_CallIDUtils.h"
#include "../../common/std/cpplib/VS_Protocol.h"
#include "../../common/std/cpplib/VS_Container_hash.h"
#include "../../common/net/EndpointRegistry.h"
#include "../../common/TrueGateway/h323/VS_H225RASParser.h"
#include "../../common/TrueGateway/clientcontrols/VS_TranscoderLogin.h"
#include "../../common/std/cpplib/netutils.h"
#include "std-generic/sqlite/CppSQLite3.h"
#include "std-generic/clib/vs_time.h"
#include "std/VS_RegistryPasswordEncryption.h"

#include <curl/curl.h>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include "std-generic/compat/memory.h"
#include <string>
#include <vector>
#include "../../common/std/cpplib/curl_deleters.h"
#include "../../common/std/clib/vs_defines.h"
#include "std-generic/cpplib/scope_exit.h"
#include "SecureLib/VS_SymmetricCrypt.h"
#include "std/cpplib/base64.h"
#include <boost/algorithm/string.hpp>
#include <openssl/sha.h>

extern std::string g_tr_endpoint_name;

#define DEBUG_CURRENT_MODULE VS_DM_RSTOR

#define SQL_TIME_PARAM "TimeColumn"

#define GET_TICK std::chrono::steady_clock::now()
#define NULL_TICK std::chrono::steady_clock::time_point()

using namespace netutils;

/// registry keys
const char RIGHTS_TAG[] = "Rights";

const char ALIASES_KEY[] = "Aliases";

const char MANAGE_AB_TAG[] = "Manage Address Book";
const bool MANAGE_AB_INIT = true;

const char LOGIN_NAME_TAG[] = "Login Name";
const char DISPLAY_NAME_TAG[] = "Display Name";


/// user endpoints
const char ENDPOINTS_KEY[] = "UsersEndpoints";

const char LAST_ENDPOINT_TAG[] = "Last Endpoint";
const char LAST_LOGGEDIN_TAG[] = "Last Loggedin";

const char STATUS_TAG[] = "Status";
const char ONLINE_STATUS_TAG[] = "Online Status";
const char LDAP_STATUS_TAG[] = "LDAP Status";
const char AUTOLOGIN_TAG[] = "Auto Login";
const char TYPE_TAG[] = "Type";
const char PROPERTIES_KEY[] = "Properties";
const char REGISTERED_TAG[] = "Registered";
const char EXTENDED_STATUS_KEY[] = "ExtendedStatus";

const char VS_SimpleStorage::PROTOCOLVERSION_TAG[] = "Protocol Version";
const char VS_SimpleStorage::APPLICATION_TAG[] = "Application";
const char WAN_IP_TAG[] = "IP";

const char VS_SimpleStorage::CLIENTVERSION_TAG[] = "Client Version";
const char REGISTRAR_TAG[] = "Registrar";
const char VS_SimpleStorage::LASTCONNECTED_TAG[] = "Last Connected";
const char NETINFO_TAG[] = "Network Info";

const char VS_SimpleStorage::GUEST_PREFIX[] = "*guest*";					// tc_id = @g + random
const char VS_SimpleStorage::GUEST_PREFIX_FIXED_CALLID[] = "*guest2*";		// tc_id = @g$ + requested call_id
const char VS_SimpleStorage::GUEST_DISPLAYNAME[] = "Guest";
const char VS_SimpleStorage::SHARED_KEY_2[] = "Shared Key";
const char VS_SimpleStorage::SHARED_KEY_3[] = "Shared Key With Prefix 3";

const char VS_SimpleStorage::SQLITE_FILE[] = "OfflineChatMessages.sqlite";
const char VS_SimpleStorage::SQLITE_TABLE[] = "OfflineChatMessages";
const char VS_SimpleStorage::SQLITE_TABLE_CALLS[] = "Calls";
const char VS_SimpleStorage::SQLITE_CONF_CONTENT_FILE[] = "slideshow\\%s\\content.sqlite";
const char VS_SimpleStorage::SQLITE_CHAT_TABLE[] = "chat";
const char VS_SimpleStorage::SQLITE_SLIDES_TABLE[] = "slides";

const char VS_SimpleStorage::AVATARS_DIRECTORY[] = "avatars/";

const char HA1_PASSWORD_TAG[] = "HA1 Password";


const char ID_COLUMN[] = "id";
const char TIME_DIFF_COLUMN[] = "time_diff";
const char FROM_COLUMN[] = "from_call_id";
const char TO_COLUMN[] = "to_call_id";
const char TO_CONF_COLUMN[] = "to_conf_stream_id";
const char TEXT_COLUMN[] = "text";
const char COMMAND_COLUMN[] = "command";
const char URL_COLUMN[] = "url";
const char MIME_COLUMN[] = "mime-type";
const char SLIDE_INDEX_COLUMN[] = "slide_index";
const char SLIDES_COUNT_COLUMN[] = "slides_count";
const char SLIDE_ABOUT_COLUMN[] = "slide_about";
const char WIDTH_COLUMN[] = "width";
const char HEIGHT_COLUMN[] = "height";
const char SIZE_COLUMN[] = "size";

/// User registry key
const char VS_SimpleStorage::EMAIL_TAG[] = "E-Mail";

const char VS_SimpleStorage::CUSTOM_GROUPS_KEY[] = "CustomGroups";
const char VS_SimpleStorage::GROUP_NAME_TAG[] = "GroupName";

const char INVITE_MAIL_SUBJ_INIT[] = "Invitation from %caller_display_name";
const char MISSED_CALL_MAIL_SUBJ_INIT[] = "Missed call from %caller_display_name";
const char MULTI_INVITE_MAIL_SUBJECT_INIT[] = "Multi-conference Invite";

#ifdef _SVKS_M_BUILD
const char INVITE_MAIL_BODY_INIT[] = "Hello %caller_display_name!\n\n%caller_display_name (%caller_call_id) attempted to contact you over Server at %missed_call_time.\n\nOur system indicates that you are not a Server user yet.\nPlease contact the server administrator for user account to be able to receive calls.\n\nEven if you do not have a camera, you can still receive calls and see your friends and colleagues live and in person.\n\nYou will get true Business Quality video conferencing over the Internet--crisp, full screen video with high frame speeds and crystal clear synchronous audio even at bandwidths below 128Kbps. With on-line presence notification and dial by e-mail, connecting with other subscribers is a snap.\n\n";
const char MISSED_CALL_MAIL_BODY_INIT[] = "Hello, %caller_display_name!\n\n%caller_display_name (%caller_call_id) attempted to contact you over Server at %missed_call_time.\n\nLogin to the application to return this call or receive and place new calls.\n\nEven if you do not have a camera, you can still receive calls and see your friends and colleagues live and in person.";
const char MULTI_INVITE_MAIL_INIT[] = "Hello %recipient_display_name.\nThere is a multi conference created and I want you to join it!\n\n";
#else
const char INVITE_MAIL_BODY_INIT[] = "Hello %caller_display_name!\n\n%caller_display_name (%caller_call_id) attempted to contact you over TrueConf Server at %missed_call_time.\n\nOur system indicates that you are not a TrueConf Server user yet.\nPlease contact the server administrator for user account to be able to receive calls.\n\nEven if you do not have a camera, you can still receive calls and see your friends and colleagues live and in person.\n\nYou will get true Business Quality video conferencing over the Internet--crisp, full screen video with high frame speeds and crystal clear synchronous audio even at bandwidths below 128Kbps. With on-line presence notification and dial by e-mail, connecting with other subscribers is a snap.\n\nSincerely,\nThe TrueConf Team.";
const char MISSED_CALL_MAIL_BODY_INIT[] = "Hello, %caller_display_name!\n\n%caller_display_name (%caller_call_id) attempted to contact you over TrueConf Server at %missed_call_time.\n\nLogin to the TrueConf Plus application to return this call or receive and place new calls.\n\nEven if you do not have a camera, you can still receive calls and see your friends and colleagues live and in person.";
const char MULTI_INVITE_MAIL_INIT[] = "Hello %recipient_display_name.\nThere is a multi conference created and I want you to join it!\n\nSincerely,\nThe TrueConf Team.";
#endif


const char MISSED_NAMED_CONF_MAIL_SUBJ_INIT[] = "named conf subj: MISSED_NAMED_CONF_MAIL_SUBJ_INIT";
const char MISSED_NAMED_CONF_MAIL_BODY_INIT[] = "body of named conf: MISSED_NAMED_CONF_MAIL_BODY_INIT";

const char OFFLINE_MESS_EXPIRE_DAYS_TAG[] = "Offline Message Expire Days";
const int OFFLINE_MESS_EXPIRE_DAYS_INIT = 30;

const char VS_SimpleStorage::HASH_PHONES_TAG[] = "Hash Phones";
const char VS_SimpleStorage::PHONE_BOOK_TAG[] = "\\Phone Book";
const char VS_SimpleStorage::USER_PHONES_TAG[] = "\\UserPhones";
const std::string VS_SimpleStorage::USER_PROPERTIES_TAG = "\\UserProperties";

std::string VS_SimpleStorage::STRING_SEPARATOR;

/// Property  registry key
const char APPPROPERTIES_KEY[] = "AppProperties";

const char SEARCHURL_PROPNAME[] = "search_url";

const size_t VS_SimpleStorage::OFFLINE_MESSAGES_CHUNK_SIZE = 100;
const size_t VS_SimpleStorage::MAX_OFFLINE_CHAT_MESSAGES = 1000;
const int32_t DEFAULT_VALID_HASH_FOR_NO_PICTURE = 1;

bool VS_ParseHelper(const std::string& query, const char* mask, size_t mask_size, std::string& out)
{
	assert(mask != nullptr); if (!mask) return false;
	out.clear();

	auto pos = query.find(mask);
	if (pos != std::string::npos)
	{
		auto start_pos = pos + mask_size;
		auto end_pos = query.find('%', start_pos);
		if (end_pos != std::string::npos)
		{
			out = query.substr(start_pos, end_pos - start_pos);
			std::transform(out.begin(), out.end(), out.begin(), ::tolower);
			return true;
		}

	}
	return false;
}

const char PARSE_LASTNAME_MASK[] = "p.last_name LIKE N'";
const char PARSE_FIRSTNAME_MASK[] = "p.first_name LIKE N'";
const char PARSE_EMAIL_MASK[] = "p.email LIKE N'";

namespace
{
class store_ext_status_to_reg : public boost::static_visitor<>
{
	VS_RegistryKey &reg_key_;
	const char *status_name_;
	public:
		store_ext_status_to_reg(VS_RegistryKey &key, const char*status_name) : reg_key_(key), status_name_(status_name)
		{}
		void operator()(bool val) const
		{
			int32_t i = val?1:0;
			reg_key_.SetValue(&i, sizeof(i), RegistryVT::VS_REG_INTEGER_VT, status_name_);
		}
		void operator()(int32_t val) const
		{
			reg_key_.SetValue(&val, sizeof(val), RegistryVT::VS_REG_INTEGER_VT, status_name_);
		}
		void operator()(int64_t val) const
		{
			reg_key_.SetValue(&val, sizeof(val), RegistryVT::VS_REG_INT64_VT, status_name_);
		}
		void operator()(const std::string& str) const
		{
			reg_key_.SetString(str.c_str(), status_name_);
		}
		void operator ()(const std::vector<uint8_t> &val) const
		{
			reg_key_.SetValue(val.data(), val.size(), RegistryVT::VS_REG_BINARY_VT, status_name_);
		}

};
}
const char VS_SimpleStorage::LOGGEDUSER_TAG[] = "Logged User";
const char VS_SimpleStorage::LASTUSER_TAG[] = "Last User";
VS_SimpleStorage::VS_SimpleStorage(const std::weak_ptr<VS_TranscoderLogin> &transLogin)
	: m_dbCallLog(vs::make_unique<callLog::Postgres>())
	, m_ab_storage(0)
	, m_license_checker(std::bind(VS_CheckLicense,std::placeholders::_1))
	, m_ab_manage(true)
	, m_money_warn_time(0)
	, m_money_warn_period(0)
	, m_money_warn_send_time(0)
	, m_tick(0)
	, m_skip(0)
	, m_chunk_size(OFFLINE_MESSAGES_CHUNK_SIZE)
	, m_Timer_OfflineChatMessages(0)
	, m_offline_mess_expire_days(0)
	, m_transLogin(transLogin)
{
	m_picture_mime_types.emplace_back(".jpg", "image/jpeg");
	m_picture_mime_types.emplace_back(".jpeg", "image/jpeg");
	m_picture_mime_types.emplace_back(".gif", "image/gif");
	m_picture_mime_types.emplace_back(".png", "image/png");
	m_picture_mime_types.emplace_back(".bmp", "image/bmp");
}
VS_SimpleStorage::~VS_SimpleStorage() = default;

bool VS_SimpleStorage::ParseQuery(const std::string& query, VS_StorageUserData& params)
{
	if (query.empty())
		return false;


	bool result = false;
	result |= VS_ParseHelper(query, PARSE_LASTNAME_MASK, sizeof(PARSE_LASTNAME_MASK) - 1, params.m_LastName);
	result |= VS_ParseHelper(query, PARSE_FIRSTNAME_MASK, sizeof(PARSE_FIRSTNAME_MASK) - 1, params.m_FirstName);

	std::string email;
	result |= VS_ParseHelper(query, PARSE_EMAIL_MASK, sizeof(PARSE_EMAIL_MASK) - 1, email);

	params.m_name = email.c_str();

	return result;

}

///application properties
int  VS_SimpleStorage::GetAppProperties(VS_Container& prop, const VS_SimpleStr& app_name)
{
	VS_AutoLock lock(&m_appProps_lock);
	if (app_name.Length() && m_appPropertiesByName.find(app_name.m_str) != m_appPropertiesByName.end())
	{
		boost::shared_ptr<VS_Container> propsByName = m_appPropertiesByName[app_name.m_str];
		propsByName->Reset();
		while (propsByName->Next())
		{
#ifndef _SVKS_M_BUILD_
			if (strcasecmp(propsByName->GetName(), SEARCHURL_PROPNAME) != 0 || m_license_checker(LE_ENABLE_DIRECTORY))
#endif
				prop.AddValue(propsByName->GetName(), propsByName->GetStrValueRef());
		}

		m_appProperties.Reset();
		while (m_appProperties.Next())
		{
			const char* name = m_appProperties.GetName();
			if (!prop.GetStrValueRef(name)
#ifndef _SVKS_M_BUILD_
				&& (strcasecmp(name, SEARCHURL_PROPNAME) != 0 || m_license_checker(LE_ENABLE_DIRECTORY))
#endif
				)
				prop.AddValue(name, m_appProperties.GetStrValueRef());
		}
	}
	else
	{
		m_appProperties.Reset();
		while (m_appProperties.Next())
		{
#ifndef _SVKS_M_BUILD_
			if (strcasecmp(m_appProperties.GetName(), SEARCHURL_PROPNAME) != 0 || m_license_checker(LE_ENABLE_DIRECTORY))
#endif
				prop.AddValue(m_appProperties.GetName(), m_appProperties.GetStrValueRef());
		}
	}
	return prop.IsValid();
}

bool VS_SimpleStorage::GetAppProperty(const VS_SimpleStr& app_name, const VS_SimpleStr& prop_name, VS_SimpleStr& value)
{
	if (!prop_name
#ifndef _SVKS_M_BUILD_
		|| (app_name == SEARCHURL_PROPNAME && !m_license_checker(LE_ENABLE_DIRECTORY))
#endif
		)
		return false;

	VS_AutoLock lock(&m_appProps_lock);

	std::map<std::string, boost::shared_ptr<VS_Container>>::iterator
		i = m_appPropertiesByName.find(!app_name ? "" : (const char *)app_name);

	const char* val = NULL;
	if (i != m_appPropertiesByName.end())
		val = i->second->GetStrValueRef(prop_name.m_str);

	if (!val)
		val = m_appProperties.GetStrValueRef(prop_name.m_str);

	if (!val)
		return false;
	value = val;
	return true;
}

bool VS_SimpleStorage::SetAppProperty(const VS_SimpleStr& prop_name, const VS_SimpleStr &value) {
    VS_AutoLock lock(&m_appProps_lock);
    if (prop_name.m_str && m_appProperties.GetStrValueRef(prop_name.m_str) == NULL) {
        m_appProperties.AddValue(prop_name.m_str, value);
    } else {
        VS_Container new_cnt;
        m_appProperties.Reset();
        while (m_appProperties.Next()) {
            if (prop_name.m_str && strcasecmp(m_appProperties.GetName(), prop_name) != 0) {
                new_cnt.AddValue(prop_name.m_str, value);
            } else {
                m_appProperties.AttachCurrentTo(new_cnt);
            }
        }
        if (new_cnt.IsValid()) {
            new_cnt.CopyTo(m_appProperties);
        } else {
            return false;
        }
    }
    m_additional_app_properties[prop_name] = value;
    return true;
}
/// aliases in registry
void VS_SimpleStorage::UpdateAliasList()
{
	VS_RegistryKey aliasKey(false, ALIASES_KEY, false, true);
	if (!aliasKey.IsValid())
		return;

	auto last_write = aliasKey.GetLastWriteTime();
	if (!last_write.empty() && m_last_write == last_write)
		return;
	m_last_write = last_write;

	auto pLockedAliasList = m_aliasList.lock();
	pLockedAliasList->Clear();

	aliasKey.ResetValues();

	std::string alias, call_id;

	int result2;

	while ((result2 = aliasKey.NextString(call_id, alias)) != 0)
	{
		if (result2<0)
			continue;

		if (!call_id.empty())
		{
			std::transform(call_id.begin(), call_id.end(), call_id.begin(), ::tolower);
			std::transform(alias.begin(), alias.end(), alias.begin(), ::tolower);
			if (VS_IsNotTrueConfCallID(call_id))
				pLockedAliasList->Insert(alias.c_str(), call_id.c_str());
			else {
				VS_RealUserLogin r(call_id);
				pLockedAliasList->Insert(alias.c_str(), r);
			}
		}
	}

	std::set<std::string> postfix;
	GetServerIPAsAliasPostfix(postfix);

	std::multimap<std::string, std::string> m;
	for (VS_StrIStrMap::Iterator it = pLockedAliasList->Begin(); it != pLockedAliasList->End(); ++it)
	{
		m.insert(std::pair<std::string, std::string>(it->data, it->key));				// alias

		for (std::set<std::string>::iterator it2 = postfix.begin(); it2 != postfix.end(); ++it2)
		{
			VS_SimpleStr a = it->key;
			a += it2->c_str();
			m.insert(std::pair<std::string, std::string>(it->data, (const char*)a));		// alias@ip
		}

		VS_RealUserLogin r(it->key);
		m.insert(std::pair<std::string, std::string>(it->data, (const char*)r));		// alias@sid
	}

	m_fireAliasesChanged(m);
}

bool VS_SimpleStorage::ResolveCallIDByAlias(const VS_SimpleStr& alias, vs_user_id& call_id)
{
	std::set<std::string> postfix;
	GetServerIPAsAliasPostfix(postfix);
	postfix.insert("@" + std::string(VS_RemoveServerType(g_tr_endpoint_name)));

	auto pLockedAliasList = m_aliasList.lock();
	for (VS_StrIStrMap::Iterator it = pLockedAliasList->Begin(); it != pLockedAliasList->End(); ++it)
	{
		const char* test_alias = it->key;
		const char* ret_real_id = it->data;

		if (strcasecmp(alias, test_alias) == 0)
		{
			call_id = ret_real_id;
			return true;
		}

		for (std::set<std::string>::iterator it2 = postfix.begin(); it2 != postfix.end(); ++it2)
		{
			VS_SimpleStr test_alias_with_postfix = test_alias;
			test_alias_with_postfix += it2->c_str();
			if (strcasecmp(alias, test_alias_with_postfix) == 0)
			{
				call_id = ret_real_id;
				return true;
			}
		}
	}
	return false;
}

// return only aliases, that are in Registry (without @ip, @sip_ip, @external_ip)
// you have to call GetServerIPAsAliasPostfix() to get those endings of call_id
bool VS_SimpleStorage::ResolveAliasByCallID(const vs_user_id& call_id, std::vector<std::string>& v_alias)
{
	if (!call_id)
		return false;

	auto pLockedAliasList = m_aliasList.lock();
	for (VS_StrIStrMap::Iterator it = pLockedAliasList->Begin(); it != pLockedAliasList->End(); it++)
	{
		if (it->data && call_id == it->data)
		{
			VS_RealUserLogin r(it->key);
			if (r.IsOurSID())
				v_alias.push_back(r.GetUser());
			else
				v_alias.emplace_back((const char*)r);
		}
	}
	return v_alias.size() != 0;
}
///////////////////////////////////////////////////////
///Server Properties

bool VS_SimpleStorage::SetServerProperty(const VS_SimpleStr& name, const VS_SimpleStr& value)
{
	VS_RegistryKey    cfg(false, CONFIGURATION_KEY, false, true);
	if (!cfg.IsValid())
		return false;

	return cfg.SetString(value.m_str, name);
}


bool VS_SimpleStorage::GetServerProperty(const std::string& name, std::string& value)
{
	VS_RegistryKey    cfg(false, CONFIGURATION_KEY, false, true);
	if (!cfg.IsValid())
		return false;

	if (!cfg.GetString(value, name.c_str()))
		value.clear();

	return !value.empty();
}

bool VS_SimpleStorage::GetWebManagerProperty(const VS_SimpleStr& name, std::string& value)
{
	VS_RegistryKey    cfg(false, "WebManager");
	if (!cfg.IsValid())
		return false;

	if (!cfg.GetString(value, name))
		value.clear();

	return !value.empty();
}

bool VS_SimpleStorage::GetWebManagerProperty(const VS_SimpleStr& name, unsigned long& value)
{
	VS_RegistryKey    cfg(false, "WebManager");
	if (!cfg.IsValid())
		return false;
	return cfg.GetValue(&value, sizeof(unsigned long), VS_REG_INTEGER_VT, name)>0;
}

bool VS_SimpleStorage::SetEndpointProperty(const char* ep_id, const char* name, const char* value)
{
	if (!ep_id || !*ep_id || !value)
		return false;

	char buff[256]; *buff = 0;
	sprintf(buff, "%s\\%s\\%s", ENDPOINTS_KEY, ep_id, PROPERTIES_KEY);
	VS_RegistryKey  endpoint(false, buff, false, true);
	if (!endpoint.IsValid())
		return false;
	return endpoint.SetString(value, name);
}

bool VS_SimpleStorage::SetEndpointProperty(const char* ep_id, const char* name, const wchar_t* value)
{
	if (!ep_id || !*ep_id)
		return false;

	char buff[256]; *buff = 0;
	sprintf(buff, "%s\\%s\\%s", ENDPOINTS_KEY, ep_id, PROPERTIES_KEY);
	VS_RegistryKey  endpoint(false, buff, false, true);
	if (!endpoint.IsValid())
		return false;
	return endpoint.SetValue(value, 0, VS_REG_WSTRING_VT, name);
}

bool VS_SimpleStorage::GetEndpointProperty(const VS_SimpleStr& ep_id, const VS_SimpleStr& name, VS_SimpleStr& value)
{
	if (!ep_id)
		return false;

	char buff[256]; *buff = 0;
	sprintf(buff, "%s\\%s\\%s", ENDPOINTS_KEY, (const char*)ep_id, PROPERTIES_KEY);
	VS_RegistryKey  endpoint(false, buff);
	if (!endpoint.IsValid())
		return false;
	char buff2[2048] = { 0 };
	unsigned long sz2 = 2048;
	if (endpoint.GetValue(buff2, sz2, VS_REG_STRING_VT, name) <= 0)
		return false;
	value = buff2;
	return true;
}

bool VS_SimpleStorage::Read(VS_RegistryKey &endpoint, VS_EndpointDescription& ep)
{
	// Reset
	ep.m_name.Empty(); ep.m_status = 0; ep.m_broker_id.Empty();
	ep.m_loggedUser.Empty(); ep.m_lastUser.Empty(); ep.m_autologin = false; ep.m_type = 0;
	ep.m_appName.Empty(); ep.m_clientVersion.Empty(); ep.m_registrar.Empty();
	ep.m_registered = std::chrono::system_clock::time_point();
	ep.m_lastConnected = std::chrono::system_clock::time_point();
	ep.m_systemConfiguration.Empty();

	// Load from registry
	ep.m_name = endpoint.GetName();
	ep.m_status = ep.DISCONNECTED_STATUS;
	char buff[2048];	// Temporary buffer
	unsigned long buffSize;
	if (endpoint.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, LOGGEDUSER_TAG) > 0)
		ep.m_loggedUser = buff;
	if (endpoint.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, LASTUSER_TAG) > 0)
		ep.m_lastUser = buff;
	ep.m_lastUser.ToLower();
	if (endpoint.GetValue(buff, sizeof(buff), VS_REG_INTEGER_VT, AUTOLOGIN_TAG) > 0)
		ep.m_autologin = !!*(int*)buff;
	if (endpoint.GetValue(buff, sizeof(buff), VS_REG_INTEGER_VT, TYPE_TAG) > 0)
		ep.m_type = *reinterpret_cast<const int32_t*>(buff);
	if (endpoint.GetValue(buff, sizeof(buff), VS_REG_INTEGER_VT, PROTOCOLVERSION_TAG) > 0)
		ep.m_protocolVersion = *reinterpret_cast<const int32_t*>(buff);
	///
	if (endpoint.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, APPLICATION_TAG) > 0)
		ep.m_appName = buff;

	if (endpoint.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, CLIENTVERSION_TAG) > 0)
		ep.m_clientVersion = buff;
	if (endpoint.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, REGISTRAR_TAG) > 0)
		ep.m_registrar = buff;
	if ((buffSize = endpoint.GetValue(buff, sizeof(buff), VS_REG_BINARY_VT, REGISTERED_TAG)) == sizeof(int64_t))
		ep.m_registered = tu::WindowsTickToUnixSeconds(*reinterpret_cast<int64_t*>(buff));
	if ((buffSize = endpoint.GetValue(buff, sizeof(buff), VS_REG_BINARY_VT, LASTCONNECTED_TAG)) == sizeof(int64_t))
		ep.m_lastConnected = tu::WindowsTickToUnixSeconds(*reinterpret_cast<int64_t*>(buff));
	if ((buffSize = endpoint.GetValue(buff, sizeof(buff), VS_REG_BINARY_VT, NETINFO_TAG))>0)
		ep.SetConnectionInfo(buff, buffSize, 0);

	return true;
}

bool VS_SimpleStorage::Init(const VS_SimpleStr& broker_id)
{
	m_broker_id = broker_id;

	VS_RegistryKey cfg_root(false, CONFIGURATION_KEY, false, true);
	char buff[1024];

	if (cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, CONF_MONEY_WARN_TAG)> 0) {
		unsigned int val = atou_s(buff);
		unsigned int min = atou_s(CONF_MONEY_WARN_MIN);
		unsigned int max = atou_s(CONF_MONEY_WARN_MAX);
		val = val>max?max:val<min?min:val;
		m_money_warn_time = val;
	}
	else m_money_warn_time = atou_s(CONF_MONEY_WARN_INIT);

	if (cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, CONF_MONEY_WARN_PERIOD_TAG)> 0) {
		unsigned int val = atou_s(buff);
		unsigned int min = atou_s(CONF_MONEY_WARN_PERIOD_MIN);
		unsigned int max = atou_s(CONF_MONEY_WARN_PERIOD_MAX);
		val = val>max?max:val<min?min:val;
		m_money_warn_period = val;
	}
	else m_money_warn_period = atou_s(CONF_MONEY_WARN_PERIOD_INIT);

	char buf[16] = {};
	sprintf(buf, "%u", static_cast<unsigned int>(VS_GenKeyByMD5()));
	m_session_id = buf;
	cfg_root.SetString(m_session_id.m_str, SESSIONID_TAG);

	std::unique_ptr<char, free_deleter> secret;
	if (cfg_root.GetValue(secret, VS_REG_STRING_VT, SHARED_KEY_2) > 0 && secret)
		m_secret2 = secret.get();
	if (cfg_root.GetValue(secret, VS_REG_STRING_VT, SHARED_KEY_3) > 0 && secret)
		m_secret3 = secret.get();

	int temp = 0;
	if (cfg_root.GetValue(&temp, sizeof(temp), VS_REG_INTEGER_VT, MANAGE_AB_TAG) != 0)
		m_ab_manage = temp != 0;
	else
		m_ab_manage = MANAGE_AB_INIT;

	std::unique_ptr<char, free_deleter> cert_buf;
	int cert_sz = cfg_root.GetValue(cert_buf, VS_REG_BINARY_VT, SRV_CERT_KEY);
	if (cert_buf && cert_sz>0)
		m_srvCert.SetCert(cert_buf.get(), cert_sz, store_PEM_BUF);

	temp = 0;
	if (cfg_root.GetValue(&temp, sizeof(temp), VS_REG_INTEGER_VT, OFFLINE_MESS_EXPIRE_DAYS_TAG) != 0)
		m_offline_mess_expire_days = (temp >= 1 && temp <= 365) ? temp : OFFLINE_MESS_EXPIRE_DAYS_INIT;
	else
		m_offline_mess_expire_days = OFFLINE_MESS_EXPIRE_DAYS_INIT;

	const auto db_conn_str = VS_GetDBConnectionString();
	if (!db_conn_str.empty())
	{
		if (!m_dbCallLog->Init(db_conn_str))
			dstream1 << "CallLog init failed, connection string: " << db_conn_str;
	}
	else
		dstream0 << "CallLog not initialized: no connection string";

	// Set OnlineStatus = Offline in each Endpoint
	VS_RegistryKey	endp_root(false, ENDPOINTS_KEY, false, false);
	VS_RegistryKey	endp;
	if (endp_root.IsValid())
	{
		endp_root.ResetKey();
		while (endp_root.NextKey(endp))
		{
			VS_SimpleStr key_name = endp.GetName();
			if (!!key_name)
				this->SetEndpointProperty(key_name, LOGGEDUSER_TAG, "");
		}
	}

	VS_RegistryKey	u_root(false, USERS_KEY, false, false);
	VS_RegistryKey	user;
	if (u_root.IsValid())
	{
		u_root.ResetKey();
		while (u_root.NextKey(user, false))
		{
			user.RemoveValue(ONLINE_STATUS_TAG);
		}
	}

	OnPropertiesChange(m_session_id);

	GetServerProperty("string_separator", STRING_SEPARATOR);
	if (STRING_SEPARATOR.empty())
		STRING_SEPARATOR = "~#";

#ifdef HAVE_CPPDB
	auto chatDB = ChatDBInterface::Create();
	if (!chatDB)
		return false;

	if (!db_conn_str.empty())
	{
		if (!chatDB->Init(db_conn_str))
		{
			dstream1 << "ChatDB init failed, connection string: " << db_conn_str;
			return false;
		}
		m_chatDB = std::move(chatDB);
	}
	else
		dstream0 << "ChatDB not initialized: no connection string";
#endif
	return true;
}

void VS_SimpleStorage::CleanUp()
{
	m_srvCert = {};
}

bool VS_SimpleStorage::OnPropertiesChange(const char* pass)
{
	if (m_session_id != pass) return false;
	VS_RegistryKey	prop_root(false, APPPROPERTIES_KEY);
	prop_root.ResetValues();
	std::unique_ptr<char, free_deleter> data;
	std::string valuName;
	VS_Container tmp1;
	while (prop_root.NextValue(data, VS_REG_STRING_VT, valuName)>0) {
		if (data && !valuName.empty()) {
			tmp1.AddValue(valuName, data.get());
		}
	}

	// AppProps by AppName
	std::map<std::string, boost::shared_ptr<VS_Container>> tmp2;
	VS_SimpleStr root_keyname = APPPROPERTIES_KEY;	root_keyname += "\\ByName";
	VS_RegistryKey root(false, root_keyname.m_str);
	VS_RegistryKey key;
	root.ResetKey();
	while (root.NextKey(key)) {
		if (key.IsValid()) {
			boost::shared_ptr<VS_Container> cnt(new VS_Container);
			GetPropsFromRegByName(key, cnt);
			tmp2[key.GetName()] = cnt;
		}
	}

	VS_AutoLock lock(&m_appProps_lock);
	m_appProperties.Clear();
	tmp1.CopyTo(m_appProperties);
	m_appPropertiesByName.clear();
	m_appPropertiesByName = tmp2;

	if (m_appProperties.GetStrValueRef("editable_profile_fields") == NULL)
		m_appProperties.AddValue("editable_profile_fields", GetDefauldEditableFields());

	if (m_appProperties.GetStrValueRef("upload_avatar") == NULL)
		m_appProperties.AddValue("upload_avatar", "1");

    for (auto &p : m_additional_app_properties) {
        if (p.first.m_str && m_appProperties.GetStrValueRef(p.first.m_str) == NULL)
            m_appProperties.AddValue(p.first.m_str, p.second);
    }

	return true;
}

void VS_SimpleStorage::GetPropsFromRegByName(VS_RegistryKey& key, boost::shared_ptr<VS_Container> cnt)
{
	key.ResetValues();
	std::unique_ptr<char, free_deleter> data;
	std::string valuName;
	cnt->Clear();

	while (key.NextValue(data, VS_REG_STRING_VT, valuName)>0) {
		if (data && !valuName.empty()) {
			cnt->AddValue(valuName, data.get());
		}
	}
}

bool VS_SimpleStorage::GetServerTime(std::chrono::system_clock::time_point& ftime)
{
	ftime = std::chrono::system_clock::now();
	return true;
}

void VS_SimpleStorage::GetMissedCallMailTemplateBase(std::string &subj_templ, std::string &body_templ)
{
	if (!GetServerProperty("missed_call_mail_subj", subj_templ) || subj_templ.empty())
		subj_templ = MISSED_CALL_MAIL_SUBJ_INIT;

	if (!GetServerProperty("missed_call_mail_body", body_templ) || body_templ.empty())
		body_templ = MISSED_CALL_MAIL_BODY_INIT;
}

void VS_SimpleStorage::GetInviteCallMailTemplateBase(std::string &subj_templ, std::string &body_templ)
{
	if (!GetServerProperty("invite_mail_subj", subj_templ) || subj_templ.empty())
		subj_templ = INVITE_MAIL_SUBJ_INIT;

	if (!GetServerProperty("invite_mail_body", body_templ) || body_templ.empty())
		body_templ = INVITE_MAIL_BODY_INIT;
}

void VS_SimpleStorage::GetMultiInviteMailTemplateBase(std::string &subj_templ, std::string &body_templ)
{
	if (!GetServerProperty("multi_invite_mail_subject", subj_templ) || subj_templ.empty())
		subj_templ = MULTI_INVITE_MAIL_SUBJECT_INIT;

	if (!GetServerProperty("multi_invite_mail_server", body_templ) || body_templ.empty())
		body_templ = MULTI_INVITE_MAIL_INIT;
}

void VS_SimpleStorage::GetMissedNamedConfMailTemplateBase(std::string &subj_templ, std::string &body_templ)
{
	if (!GetServerProperty("conf_invite_mail_subj", subj_templ) || subj_templ.empty())
		subj_templ = MISSED_NAMED_CONF_MAIL_SUBJ_INIT;

	if (!GetServerProperty("conf_invite_mail_body", body_templ) || body_templ.empty())
		body_templ = MISSED_NAMED_CONF_MAIL_BODY_INIT;
}

int VS_SimpleStorage::GetAllAppProperties(VS_AppPropertiesMap &/*prop_map*/)
{
	return 0;
}

bool VS_SimpleStorage::SetAllEpProperties(const char* /*app_id*/, const int /*prot_version*/, const short int /*type*/, const wchar_t* /*version*/,
	const wchar_t* /*app_name*/, const wchar_t* /*sys_conf*/, const wchar_t* /*processor*/, const wchar_t* /*directX*/,
	const wchar_t* /*hardwareConfig*/, const wchar_t* /*AudioCapture*/, const wchar_t* /*VideoCapture*/, const wchar_t* /*AudioRender*/, const char* /*call_id*/)
{
	return false;
}


//logging
bool VS_SimpleStorage::LogConferenceStart(const VS_ConferenceDescription& conf, bool remote)
{
	m_dbCallLog->LogConferenceStart(conf, remote);
	if (!!conf.m_name)
		m_conf_started->emplace(conf.m_name, std::chrono::steady_clock::now());
	return false;
}

bool VS_SimpleStorage::LogConferenceEnd(const VS_ConferenceDescription& conf)
{
	m_dbCallLog->LogConferenceEnd(conf);
	if (!!conf.m_name)
		m_conf_started->erase(conf.m_name);
	return false;
}

bool VS_SimpleStorage::LogParticipantInvite(const vs_conf_id& conf_id, const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,
	const std::chrono::system_clock::time_point time, VS_ParticipantDescription::Type type)
{
	m_dbCallLog->LogParticipantInvite(conf_id, call_id1, app_id, call_id2, time, type);
	return false;
}
bool VS_SimpleStorage::LogParticipantJoin(const VS_ParticipantDescription& pd, const VS_SimpleStr& callid2)
{
	m_dbCallLog->LogParticipantJoin(pd, callid2);
	return false;
}
bool VS_SimpleStorage::LogParticipantReject(const vs_conf_id& conf_id, const vs_user_id& user, const vs_user_id& invited_from, const VS_Reject_Cause cause)
{
	m_dbCallLog->LogParticipantReject(conf_id, user, invited_from, cause);
	return false;
}
bool VS_SimpleStorage::LogParticipantLeave(const VS_ParticipantDescription& pd)
{
	m_dbCallLog->LogParticipantLeave(pd);
	return false;
}

bool VS_SimpleStorage::LogParticipantStatistics(const VS_ParticipantDescription& pd)
{
	m_dbCallLog->LogParticipantStatistics(pd);
	return false;
}

bool VS_SimpleStorage::LogConferenceRecordStart(const vs_conf_id& conf_id, const char* filename,
	std::chrono::system_clock::time_point started_at)
{
	return m_dbCallLog->LogConferenceRecordStart(conf_id, filename, started_at);
}

bool VS_SimpleStorage::LogConferenceRecordStop(const vs_conf_id& conf_id,
	std::chrono::system_clock::time_point stopped_at, uint64_t file_size)
{
	return m_dbCallLog->LogConferenceRecordStop(conf_id, stopped_at, file_size);
}

bool VS_SimpleStorage::LogSystemParams(std::vector<int> &params)
{
	return 	m_dbCallLog->LogSystemParams(params);
}

bool VS_SimpleStorage::LogParticipantDevices(VS_ParticipantDeviceParams& params)
{
	return m_dbCallLog->LogParticipantDevices(params);
}

bool VS_SimpleStorage::LogParticipantRole(VS_ParticipantDescription& params)
{
	return m_dbCallLog->LogParticipantRole(params);
}

long VS_SimpleStorage::SetRegID(const char * call_id, const char * reg_id, VS_RegID_Register_Type type)
{
	return m_dbCallLog->SetRegID(call_id, reg_id, type);
}


bool VS_SimpleStorage::LogSlideShowCmd(const char *confId, const char *from, const char *url, const char *mimeType, size_t slideIndex, size_t slidesCount,
	const char *about, size_t width, size_t height, size_t size)
{
	dprint3("$LogSlideShowCmd\n");
	if (!confId || !*confId)
		return false;

	std::string confId_no_suffix(confId);
	size_t pos = confId_no_suffix.find('#');
	if (pos != std::string::npos)
		confId_no_suffix.erase(pos);

	CppSQLite3DB db;
	if (!ConfContent_InitSQLiteDB(db, confId_no_suffix.c_str()))
		return false;

	bool res = false;
	try
	{

		CppSQLite3Buffer bufSQL;
		bufSQL.format("INSERT INTO %s (%s, %s, %s, %s, \"%s\", %s, %s, %s, %s, %s, %s) VALUES (%d, %Q, %Q, %Q, %Q, %d, %d, %Q, %d, %d, %d);",
			SQLITE_SLIDES_TABLE, TIME_DIFF_COLUMN, FROM_COLUMN, COMMAND_COLUMN, URL_COLUMN, MIME_COLUMN, SLIDE_INDEX_COLUMN, SLIDES_COUNT_COLUMN, SLIDE_ABOUT_COLUMN,
			WIDTH_COLUMN, HEIGHT_COLUMN, SIZE_COLUMN,
			GetConfStartTimeDiffOrDefault(confId), from, "SLIDE_SHOW_COMMAND", url, mimeType, slideIndex, slidesCount, about, width, height, size);

		db.execDML(bufSQL);
		res = true;
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
	}

	return res;
}

bool VS_SimpleStorage::LogSlideShowEnd(const char *confId, const char *from)
{
	dprint3("$LogSlideShowEnd\n");
	if (!confId || !*confId)
		return false;

	std::string confId_no_suffix(confId);
	size_t pos = confId_no_suffix.find('#');
	if (pos != std::string::npos)
		confId_no_suffix.erase(pos);

	CppSQLite3DB db;
	if (!ConfContent_InitSQLiteDB(db, confId_no_suffix.c_str()))
		return false;

	bool res = false;
	try
	{
		CppSQLite3Buffer bufSQL;
		bufSQL.format("INSERT INTO %s (%s, %s, %s) VALUES (%d, %Q, %Q);",
			SQLITE_SLIDES_TABLE, TIME_DIFF_COLUMN, FROM_COLUMN, COMMAND_COLUMN, GetConfStartTimeDiffOrDefault(confId), from, END_SLIDESHOW_COMMAND);

		db.execDML(bufSQL);
		res = true;
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
	}

	return res;
}

bool VS_SimpleStorage::LogGroupChat(const char *confId, const char *from, const char *text)
{
	dprint3("$LogGroupChat\n");
	if (!confId || !*confId)
		return false;

	std::string confId_no_suffix(confId);
	size_t pos = confId_no_suffix.find('#');
	if (pos != std::string::npos)
		confId_no_suffix.erase(pos);

	CppSQLite3DB db;
	if (!ConfContent_InitSQLiteDB(db, confId_no_suffix.c_str()))
		return false;

	bool res = false;
	try
	{
		CppSQLite3Buffer bufSQL;
		bufSQL.format("INSERT INTO %s (%s, %s, %s, %s) VALUES (%d, %Q, %Q, %Q);",
			SQLITE_CHAT_TABLE, TIME_DIFF_COLUMN, FROM_COLUMN, TO_CONF_COLUMN, TEXT_COLUMN, GetConfStartTimeDiffOrDefault(confId), from, confId_no_suffix.c_str(), text);

		db.execDML(bufSQL);
		res = true;
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
	}

	return res;
}

bool VS_SimpleStorage::UpdateConferencePic(VS_Container &/*cnt*/)
{
	return false;
}

void VS_SimpleStorage::GetBSEvents(std::vector<BSEvent> &/*vec*/)
{

}

int VS_SimpleStorage::LoginUser(const VS_SimpleStr& login, const VS_SimpleStr& password, const VS_SimpleStr& appID, VS_SimpleStr& autoKey, const VS_SimpleStr& /*appServer*/, VS_UserData& user, VS_Container& prop_cnt, const VS_ClientType& client_type)
{
	if (client_type==CT_TRANSCODER || client_type==CT_GATEWAY || client_type==CT_TRANSCODER_CLIENT) {
		if (!!autoKey)	// no AutoLogin from Gateway or transcoder
			return SILENT_REJECT_LOGIN;
		if (!m_license_checker(LE_GATEWAYLOGIN))
			return LICENSE_USER_LIMIT;
	}

	if (client_type == CT_SDK)
	{
		if (!m_license_checker(LE_SDK_CLIENTS))
			return INVALID_CLIENT_TYPE;
	}

	VS_SimpleStr guest_login;
	VS_SimpleStr guest_displayName_utf8;
	int result = INVALID_CLIENT_TYPE;
	VS_SimpleStr appname = std::move(user.m_appName);

	if (client_type == CT_TRANSCODER)
	{
		result = LoginAsTranscoder(login, password, appID, user, client_type);
	}
	else if (GetGuestParams(login, guest_login, guest_displayName_utf8))
	{
		result = LoginAsGuest(guest_login, guest_displayName_utf8, password, appID, user, client_type);
	}
	else
	{
		VS_StorageUserData ude;
		result = LoginAsUser(login, password, appID, autoKey, ude, prop_cnt, client_type);
		user = *(VS_UserData*)&ude;

		if (result == USER_LOGGEDIN_OK)
		{
			std::set<std::string> postfix;
			GetServerIPAsAliasPostfix(postfix);				// all postfix (@ip, @sip_ip, @external_sip_ip

			std::vector<std::string> v;
			ResolveAliasByCallID(user.m_name, v);			// only registry aliases of this user
			for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it)
			{
				user.m_aliases.Assign(it->c_str(), 0);		// UserAliases=0 (SystemAliases=1)

				for (std::set<std::string>::iterator it2 = postfix.begin(); it2 != postfix.end(); ++it2)
				{
					VS_SimpleStr a = it->c_str();
					a += it2->c_str();
					user.m_aliases.Assign((const char*)a, 0);		// alias@ip
				}
			}

			if (!m_license_checker(LE_PAID_SERVER))
				user.m_tarif_name = "Free";

#ifdef _SVKS_M_BUILD_
			user.m_tarif_name.Empty();
#endif
		}
	}
	if (result == USER_LOGGEDIN_OK)
	{
		auto pLockedUsers = m_users.lock();
		auto u = pLockedUsers->Find(user.m_name);
		if (!!u) {
			VS_StorageUserData *pud = u->data;
			pud->m_appName = appname;
		}
		user.m_appName = std::move(appname);
	}
	return result;
}

#include "ProtectionLib/OptimizeDisable.h"
int  VS_SimpleStorage::LoginAsGuest(const vs_user_id& login_requested, const VS_SimpleStr& display_name, const VS_SimpleStr& password, const VS_SimpleStr& appID, VS_UserData& user, const VS_ClientType &client_type)
{
	int res = LICENSE_USER_LIMIT;
	SECUREBEGIN_B_GUEST;
	if (m_license_checker(LE_GUEST_LOGIN))
		res = CheckGuestPassword(login_requested,password, client_type) ? USER_LOGGEDIN_OK : ACCESS_DENIED;

	if (USER_LOGGEDIN_OK == res) {
		bool IsTerminalLoginAllowed = false;
		bool IsTerminalPro = false;
		if (client_type == CT_TERMINAL)
		{
			CheckLic_TerminalPro(IsTerminalLoginAllowed, IsTerminalPro);
			if (!IsTerminalLoginAllowed)
				return INVALID_CLIENT_TYPE;
		}

		auto pLockedUsers = m_users.lock();
		UserMap::ConstIterator it = pLockedUsers->Begin();
		do {
			VS_SimpleStr login;
			std::string gname;
			if (!!login_requested) {
				login = "#guest2:";
				login += login_requested;
				gname = login_requested.m_str;
			} else {
				login = "#guest:";
				unsigned long r = VS_GenKeyByMD5();
				char tmp[256] = { 0 };
				sprintf(tmp, "%lx", r);
				login += tmp;
				login.ToLower();
				gname.assign(tmp, 4);
			}
			VS_RealUserLogin realLogin(SimpleStrToStringView(login));
			UserMap::ConstIterator it = (*pLockedUsers)[realLogin];
			if (!!it && !!login_requested) {		// guest with requested fixed call exists, delete old and auth new
				pLockedUsers->Erase(realLogin);
				it = (*pLockedUsers)[realLogin];
			}
			if (!it)
			{
				VS_StorageUserData ud;
				ud.m_realLogin = realLogin;
				ud.m_login = ud.m_name = realLogin;
				ud.m_appID = appID;
				ud.m_aliases.Assign(login, 0);
				if (!display_name) {
					ud.m_displayName = VS_SimpleStorage::GUEST_DISPLAYNAME;
					ud.m_displayName += " ";
					ud.m_displayName += gname;
				}
				else {
					ud.m_displayName = display_name;
				}
				ud.m_type = VS_UserData::UT_GUEST;
				ud.m_status = VS_UserData::US_LOGIN;
				ud.m_client_type = client_type;
				std::string buff;
				if (m_srvCert.GetSubjectEntry("organizationName", buff))
					ud.m_Company = buff;


				FetchRights(ud, (VS_UserData::UserRights &) ud.m_rights);
				if (client_type == CT_TERMINAL && IsTerminalPro)
					ud.m_rights |= VS_UserData::UR_COMM_PROACCOUNT;
				FetchTarifOpt(ud);

				pLockedUsers->Insert(realLogin, ud);
				user = ud;
				OnUserLoggedInAtEndpoint(&user);
				break;
			}
		} while (it);
	}
	SECUREEND_B_GUEST;
	return res;
}
#include "ProtectionLib/OptimizeEnable.h"

int VS_SimpleStorage::LoginAsTranscoder(const VS_SimpleStr& login, const VS_SimpleStr& password, const VS_SimpleStr& appID, VS_UserData& user, const VS_ClientType &client_type)
{
	VS_RealUserLogin realLogin(SimpleStrToStringView(login));

	auto pLockedUsers = m_users.lock();
	UserMap::ConstIterator it = (*pLockedUsers)[realLogin];
	if (!!it)
		return USER_ALREADY_LOGGEDIN;
	if (!CheckTranscoderPassword(login, password))
		return ACCESS_DENIED;

	VS_StorageUserData ud;
	ud.m_realLogin = realLogin;
	ud.m_login = ud.m_name = login;
	ud.m_appID = appID;
	//	ud.m_displayName.AssignUTF8( (!!display_name)? display_name: VS_SimpleStorage::GUEST_DISPLAYNAME );
	//	ud.m_type = VS_UserData::UT_GUEST;
	ud.m_status = VS_UserData::US_LOGIN;
	ud.m_client_type = client_type;

	FetchRights(ud, (VS_UserData::UserRights &) ud.m_rights);
	FetchTarifOpt(ud);

	pLockedUsers->Insert(realLogin, ud);
	user = ud;
	return USER_LOGGEDIN_OK;
}

bool VS_SimpleStorage::CheckTranscoderPassword(const VS_SimpleStr &call_id, const VS_SimpleStr &pass)
{
	auto transcoder_login = m_transLogin.lock();
	if (transcoder_login == nullptr)
		return false;

	return transcoder_login->Login(string_view( call_id.m_str, call_id.Length() ), string_view( pass.m_str, pass.Length()));
}

// Password format: $2[RAND]*[TIMESTAMP]*md5([RAND]+[TIMESTAMP]+[SECRET])
bool VS_SimpleStorage::CheckGuestPassword(const char* fixed_id, const VS_SimpleStr& password, const VS_ClientType &type)
{
	return (type != VS_ClientType::CT_WEB_CLIENT) || m_license_checker(LE_WEBINARS) ? CheckTempPassword(password, "$2", 0, fixed_id, m_secret2) : false;
}

// Password format: $3[RAND]*[TIMESTAMP]*md5([LOGIN]+[RAND]+[TIMESTAMP]+[SECRET])
bool VS_SimpleStorage::CheckGuestUserPassword(const char* password, const char* user_id)
{
	return CheckTempPassword(password, "$3", user_id,0, m_secret3);
}

bool VS_SimpleStorage::CheckTempPassword(const char* password, const char* prefix, const char* login, const char *fixed_id, const char* secret)
{
	if (!password || strlen(password) <= 2 || !prefix || !*prefix || strlen(password)<strlen(prefix))
		return false;

	// make a copy of password and work with it (check)
	VS_SimpleStr str = password;
	char* p = str.m_str;

	// check prefix
	if (strncasecmp(p, prefix, strlen(prefix)) != 0)
		return false;
	p += 2;

	char* rnd = p;

	// skip [RAND]
	p = strchr(p, '*');
	if (!p)
		return false;

	p[0] = 0;
	VS_SimpleStr str_rnd = rnd;

	p++;

	char* p_md5 = strchr(p, '*');
	if (!p_md5)
		return false;

	// set null temporary
	p_md5[0] = 0;
	VS_SimpleStr str_time = p;
	auto t = strtoll(p,nullptr,10);

	if (t < time(0))	// password expired
		return false;

	p_md5++;
	if (!*p_md5)
		return false;

	MD5 md5;
	if ((!fixed_id || !*fixed_id)&&login&&*login)
		md5.Update(login);
	md5.Update(SimpleStrToStringView(str_rnd));
	if (fixed_id && *fixed_id&&(!login || !*login))
		md5.Update(fixed_id);
	md5.Update(SimpleStrToStringView(str_time));
	if (secret&&*secret)
		md5.Update(secret);
	md5.Final();
	char md5_hash[33];
	md5.GetString(md5_hash);

	if (strcasecmp(md5_hash, p_md5) != 0)
		return false;

	return true;
}

void VS_SimpleStorage::FetchTarifOpt(VS_UserData& user)
{
	int32_t tarif_opts[4] = {0};
	VS_GetLicence_TarifRestrictions(tarif_opts);
	user.m_tarif_restrictions = 0;
	for (int i = 0; i < 4; i++)
		user.m_tarif_restrictions += (tarif_opts[i] & 0xff) << i * 8;
}

int VS_SimpleStorage::GetOfflineChatMessages(const char* call_id, std::vector<VS_Container> &vec)
{
	if (!call_id || !*call_id)
		return 0;

	dprint3("$GetOfflineChatMessages for %s\n", call_id);
#ifdef HAVE_CPPDB
	if (m_chatDB == nullptr)
		return 0;

	int n = 0;
	auto func = [this, &vec, &n](const int64_t &mID,
		const VS_Container *cnt,
		const char *message,
		const std::time_t *timestamp,
		const bool *is_offline,
		const char *from_call_id,
		const char *from_display_name,
		const char *to_call_id,
		const char *conference_id) -> bool {
		std::chrono::system_clock::time_point			time(std::chrono::system_clock::now());
		if (to_call_id == nullptr || *to_call_id == '\0')
		{
			return true;
		}

		if (cnt == nullptr)
		{
			dprint3("$GetOfflineChatMessages(to = %s): there is no message container!\n", to_call_id);
			return true;
		}

		VS_Container c(*cnt);

		c.AddValue(FILETIME_PARAM, time);

		vec.push_back(std::move(c));
		n++;

		return true;
	};

	auto res = m_chatDB->ProcessUserOfflineMessages(func, call_id, m_chunk_size);
	if (!res)
	{
		dprint4("$GetOfflineChatMessages has failed!\n");
	}

	return n;
#else
	CppSQLite3DB db;
	int n = 0;
	try
	{
		CppSQLite3Buffer bufSQL;
		CppSQLite3Query q;
		VS_SimpleStr	from;
		const char*		from_display_name;
		VS_SimpleStr	to;
		VS_SimpleStr	body;
		std::chrono::system_clock::time_point			time;
		CppSQLite3Binary blob;

		db.open(SQLITE_FILE);
		if (!db.tableExists(SQLITE_TABLE))
			return 0;

		bufSQL.format("select * from %s where \"%s\"=%Q order by %Q limit %lu", SQLITE_TABLE, TO_PARAM, call_id, SQL_TIME_PARAM, MAX_OFFLINE_CHAT_MESSAGES);

		q = db.execQuery(bufSQL);

		while (!q.eof())
		{
			from = q.fieldValue(FROM_PARAM);
			to = q.fieldValue(TO_PARAM);
			from_display_name = q.fieldValue(DISPLAYNAME_PARAM);
			body = q.fieldValue(MESSAGE_PARAM);

			try
			{
				auto ptr = q.fieldValue(MESSAGE_CONTAINER_PARAM);
				if (ptr)
					blob.setEncoded((unsigned char*)ptr);
				time = std::chrono::system_clock::from_time_t(q.getInt64Field(SQL_TIME_PARAM));
			}
			catch (CppSQLite3Exception&)
			{
			}

			VS_Container cont;
			if (cont.Deserialize(blob.getBinary(), blob.getBinaryLength())){
				vec.push_back(std::move(cont));
				++n;
				q.nextRow();
				continue;
			}

			VS_Container c;
			c.AddValue(METHOD_PARAM, SENDMESSAGE_METHOD);
			c.AddValue(FROM_PARAM, from);
			c.AddValue(DISPLAYNAME_PARAM, from_display_name);
			c.AddValue(TO_PARAM, to);
			c.AddValue(MESSAGE_PARAM, body);
			c.AddValue(FILETIME_PARAM, time);

			vec.push_back(std::move(c));
			n++;
			q.nextRow();
		}

		if (n > 0)
		{
			m_skip += n;
		}
		else
		{
			m_skip = 0;
		}
		if (n > 0)
		{
			bufSQL.format("delete from %s where \"%s\"=%Q;", SQLITE_TABLE, TO_PARAM, call_id);
			db.execQuery(bufSQL);
		}
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
	}
	return n;
#endif
}

bool VS_SimpleStorage::TryAddColumn(CppSQLite3DB &db, const char *table, const char* column_name, const char *column_type){
	if (!table || !column_name || !column_type)
		return false;

	try // table will not be altered if column 'column_name' already exists
	{
		CppSQLite3Buffer bufSQL;
		bufSQL.format("ALTER TABLE %Q ADD COLUMN %Q %s;", table, column_name, column_type);
		db.execQuery(bufSQL);
	}
	catch (CppSQLite3Exception&){ return false; }
	return true;
}

bool VS_SimpleStorage::SetOfflineChatMessage(const VS_SimpleStr& from_call_id, const VS_SimpleStr& to_call_id, const VS_SimpleStr& body_utf8, const std::string& from_dn, const VS_Container &cont)
{
	if (!from_call_id || !to_call_id)
		return false;

	if (!IsValidUserSID(SimpleStrToStringView(to_call_id)))
		return false;

	dprint3("$SetOfflineChatMessage from %s to %s\n", from_call_id.m_str, to_call_id.m_str);

#ifdef HAVE_CPPDB
	if (m_chatDB == nullptr)
		return false;

	int64_t mID = 0;
	bool is_offline = true;

	if (cont.GetValueI64(DB_MESSAGE_ID_PARAM, mID) && mID > 0) // the message is already in DB
	{
		auto res = m_chatDB->ChangeMessage(mID, 0, 0, 0, &is_offline);
		if (!res)
		{
			dprint3("SetOfflineChatMessage(to = %s, from = %s): cannot change data in DB!\n", to_call_id.m_str, from_call_id.m_str);
		}
		return res;
	}
	else // new message
	{
		const char* from = cont.GetStrValueRef(FROM_PARAM);
		//const char* display_name = cont.GetStrValueRef(DISPLAYNAME_PARAM);
		const char* message = cont.GetStrValueRef(MESSAGE_PARAM);
		const char *to = cont.GetStrValueRef(TO_PARAM);
		std::time_t timestamp = 0;
		std::time_t *ptimestamp = nullptr;
		std::chrono::system_clock::time_point ts;

		if (!to && *to == '\0')
		{
			return false;
		}

		if (cont.GetValue(FILETIME_PARAM, ts))
		{
			timestamp = std::chrono::system_clock::to_time_t(ts);
			ptimestamp = &timestamp;
		}

		auto dname = from_dn;
		if (dname.empty()) {
			VS_UserData ud;
			if (FindUser(from_call_id, ud) && !ud.m_displayName.empty())
				dname = ud.m_displayName;
			else
				dname = from_call_id;
		}

		if (m_chatDB->AddNewMessage(mID, message, ptimestamp, from, dname.c_str(), to))
		{
			VS_Container cnt(cont);
			cnt.AddValueI64(DB_MESSAGE_ID_PARAM, mID);
			auto res = m_chatDB->ChangeMessage(mID, &cnt, 0, 0, &is_offline);
			if (!res)
			{
				dprint3("SetOfflineChatMessage(to = %s, from = %s, dn = %s): cannot change data in DB!\n", to, from, dname.c_str());
			}
			return res;
		}
		else
		{
			dprint3("SetOfflineChatMessage(to = %s, from = %s, dn = %s): cannot create a new message in DB!\n", to, from, dname.c_str());
			return false;
		}
	}
#else
	try
	{
		CppSQLite3DB db;
		db.open(SQLITE_FILE);

		TryAddColumn(db, SQLITE_TABLE, SQL_TIME_PARAM, "varchar(32)");
		TryAddColumn(db, SQLITE_TABLE, MESSAGE_CONTAINER_PARAM, "blob");

		if (!db.tableExists(SQLITE_TABLE))
		{
			CppSQLite3Buffer bufSQL;
			bufSQL.format("create table %s(%Q varchar(256), %Q varchar(256), %Q varchar(256), %Q varchar(256), expires varchar(32), %Q varchar(32), %Q blob);",
				SQLITE_TABLE, FROM_PARAM, DISPLAYNAME_PARAM, TO_PARAM, MESSAGE_PARAM, SQL_TIME_PARAM, MESSAGE_CONTAINER_PARAM);

			db.execDML(bufSQL);
		}

		auto dname = from_dn;
		if (dname.empty()) {
			VS_UserData ud;
			if (FindUser(from_call_id, ud) && !ud.m_displayName.empty())
				dname = ud.m_displayName;
			else
				dname = from_call_id;
		}

		void *raw_cont;
		size_t raw_cont_size;
		if (!cont.SerializeAlloc(raw_cont, raw_cont_size)) return false;
		std::unique_ptr<void, free_deleter> buff(raw_cont);

		CppSQLite3Binary blob;
		blob.setBinary((const unsigned char*)raw_cont, raw_cont_size);

		CppSQLite3Buffer bufSQL;
		bufSQL.format("insert into %s values(%Q, %Q, %Q, %Q, datetime('now', 'localtime', '%d days'), strftime('%%s','now'), %Q);",
			SQLITE_TABLE, from_call_id.m_str, dname.c_str(), to_call_id.m_str, body_utf8.m_str, m_offline_mess_expire_days, blob.getEncoded());

		db.execDML(bufSQL);
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
		return false;
	}
#endif
	return true;
}

void VS_SimpleStorage::GetRoamingOfflineMessages(const char* our_sid, VS_ChatMsgs& v)
{
	if (!our_sid)
		return;
	dprint4("$GetRoamingOfflineMessages\n");
#ifdef HAVE_CPPDB
	if (m_chatDB == nullptr)
		return;

	auto func = [this, &v](const int64_t &mID,
		const VS_Container *cnt,
		const char *message,
		const std::time_t *timestamp,
		const bool *is_offline,
		const char *from_call_id,
		const char *from_display_name,
		const char *to_call_id,
		const char *conference_id) -> bool {
		if (to_call_id == nullptr || *to_call_id == '\0')
		{
			return true;
		}

		VS_RealUserLogin r(to_call_id);
		if (r.IsOurSID())
		{
			return true;
		}

		if (cnt == nullptr)
		{
			return true;
		}

		VS_Container* c = new VS_Container(*cnt);

		VS_RoamingChatMsg msg;

		msg.to_callId = r;
		msg.cnt = c;

		v.push_back(msg);

		return true;
	};

	auto res = m_chatDB->ProcessRoamingOfflineMessages(func, our_sid, m_chunk_size);
	if (!res)
	{
		dprint4("$GetRoamingOfflineMessages has failed!\n");
	}
#else
	CppSQLite3DB db;

	int n = 0;
	try
	{
		CppSQLite3Buffer bufSQL;
		CppSQLite3Query q;
		VS_SimpleStr	from;
		VS_SimpleStr	to;
		VS_SimpleStr	body;
		CppSQLite3Binary blob;

		db.open(SQLITE_FILE);
		if (!db.tableExists(SQLITE_TABLE))
			return;

		bufSQL.format("select * from %s where \"%s\" not like \"%%@%s\" order by %Q limit %lu offset %lu;", SQLITE_TABLE, TO_PARAM, our_sid, TO_PARAM, m_chunk_size, m_skip);
		q = db.execQuery(bufSQL);

		while (!q.eof())
		{
			from = q.fieldValue(FROM_PARAM);
			to = q.fieldValue(TO_PARAM);
			body = q.fieldValue(MESSAGE_PARAM);

			VS_Container* c = new VS_Container;

			c->AddValue(METHOD_PARAM, SENDMESSAGE_METHOD);
			c->AddValue(FROM_PARAM, from);
			c->AddValue(DISPLAYNAME_PARAM, q.fieldValue(DISPLAYNAME_PARAM));
			c->AddValue(TO_PARAM, to);
			c->AddValue(MESSAGE_PARAM, body);

			VS_RealUserLogin r(SimpleStrToStringView(to));
			if (r.IsOurSID())
			{
				delete c;
				q.nextRow();
				continue;
			}

			VS_RoamingChatMsg msg;
			msg.to_callId = r;
			msg.cnt = c;

			v.push_back(msg);

			n++;
			q.nextRow();
		}

		if (n > 0)
		{
			m_skip += n;
		}
		else
		{
			m_skip = 0;
		}
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
	}
#endif
}

void VS_SimpleStorage::DeleteOfflineChatMessage(VS_Container& cnt)
{
	const char* to = cnt.GetStrValueRef(TO_PARAM);
	const char* from = cnt.GetStrValueRef(FROM_PARAM);
	const char* dn = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
	const char* msg = cnt.GetStrValueRef(MESSAGE_PARAM);

	if (!to || !*to || !from || !*from || !dn || !*dn || !msg || !*msg)
		return;

	dprint3("$DeleteOfflineChatMessage(to = %s, from = %s, dn = %s)\n", to, from, dn);
#if defined(HAVE_CPPDB)
	if (m_chatDB == nullptr)
		return;

	int64_t mID = 0;
	bool is_offline = false;
	if (cnt.GetValueI64(DB_MESSAGE_ID_PARAM, mID) && mID != 0) // fast path - new servers
	{
		if (!m_chatDB->ChangeMessage(mID, 0, 0, 0, &is_offline))
		{
			dprint3("DeleteOfflineChatMessage(to = %s, from = %s, dn = %s): cannot change data in DB!\n", to, from, dn);
		}
	}
	else // slow path - for compatibility reasons
	{
		if (!m_chatDB->MarkRoamingMessageAsDelivered(from, dn, to, msg))
		{
			dprint3("DeleteOfflineChatMessage(to = %s, from = %s, dn = %s): cannot change data in DB!\n", to, from, dn);
		}
	}
#else
	try
	{
		CppSQLite3DB db;
		db.open(SQLITE_FILE);
		if (!db.tableExists(SQLITE_TABLE))
			return;

		CppSQLite3Buffer bufSQL;
		bufSQL.format("delete from %s where \"%s\"=%Q and \"%s\"=%Q and \"%s\"=%Q and \"%s\"=%Q;",
			SQLITE_TABLE, TO_PARAM, to, FROM_PARAM, from, DISPLAYNAME_PARAM, dn, MESSAGE_PARAM, msg);
		db.execQuery(bufSQL);
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
	}
#endif
}

void VS_SimpleStorage::Timer(unsigned long ticks, VS_TransportRouterServiceHelper* caller)
{
	if (ticks - m_Timer_OfflineChatMessages > 24 * 60 * 60 * 1000) {		// once a day
		m_Timer_OfflineChatMessages = ticks;
		Timer_OfflineChatCleanup();
	}
}
void VS_SimpleStorage::Timer_OfflineChatCleanup()
{
	dprint3("$Timer_OfflineChatCleanup\n");
	try
	{
		CppSQLite3DB db;
		db.open(SQLITE_FILE);
		if (!db.tableExists(SQLITE_TABLE))
			return;

		CppSQLite3Buffer bufSQL;
		bufSQL.format("delete from %s where expires <= datetime('now','localtime');",
			SQLITE_TABLE);

		db.execDML(bufSQL);
	}
	catch (CppSQLite3Exception& e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
	}
}

void VS_SimpleStorage::ProcessConfStat(VS_Container& cnt)
{
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	if (!key.IsValid())
		return;
	int val = 0;
	if (key.GetValue(&val, sizeof(val), VS_REG_INTEGER_VT, SAVE_CONF_STAT_TAG)>0 && !val)
		return;

	VS_UserData ud;
	FindUser(cnt.GetStrValueRef(CALLID_PARAM), ud);
	size_t size = 0;
	const void* buff = cnt.GetBinValueRef(CONF_BASE_STAT_PARAM, size);

	if (buff && size && (size == sizeof(TConferenceStatistics)))
			m_dbCallLog->LogParticipantStatistics(cnt,(const TConferenceStatistics*)buff, ud.m_displayName);
}

bool VS_SimpleStorage::ConfContent_InitSQLiteDB(CppSQLite3DB& db, const char *conf_id)
{
	char fileName[MAX_PATH];
	std::string dir("slideshow");
	bool res = false;
	sprintf(fileName, SQLITE_CONF_CONTENT_FILE, conf_id);
	dir += "/";
	dir += conf_id;
	boost::filesystem::create_directories(dir);

	try
	{
		db.open(fileName);
		if (!db.tableExists(SQLITE_CHAT_TABLE))
		{
			CppSQLite3Buffer bufSQL;
			bufSQL.format("CREATE TABLE %s (%Q INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
				"%Q INTEGER,"
				"%Q VARCHAR(255),"
				"%Q VARCHAR(255),"
				"%Q VARCHAR(255),"
				"%Q TEXT);",
				SQLITE_CHAT_TABLE, ID_COLUMN, TIME_DIFF_COLUMN, FROM_COLUMN, TO_COLUMN, TO_CONF_COLUMN, TEXT_COLUMN);

			db.execDML(bufSQL);
		}
		if (!db.tableExists(SQLITE_SLIDES_TABLE))
		{
			CppSQLite3Buffer bufSQL;
			bufSQL.format("CREATE TABLE %s (%Q INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
				"%Q INTEGER,"
				"%Q VARCHAR(255),"
				"%Q VARCHAR(255),"
				"%Q VARCHAR(255),"
				"%Q VARCHAR(255),"
				"%Q INTEGER,"
				"%Q INTEGER,"
				"%Q TEXT,"
				"%Q INTEGER,"
				"%Q INTEGER,"
				"%Q INTEGER);",
				SQLITE_SLIDES_TABLE, ID_COLUMN, TIME_DIFF_COLUMN, FROM_COLUMN, COMMAND_COLUMN, URL_COLUMN, MIME_COLUMN,
				SLIDE_INDEX_COLUMN, SLIDES_COUNT_COLUMN, SLIDE_ABOUT_COLUMN, WIDTH_COLUMN, HEIGHT_COLUMN, SIZE_COLUMN);

			db.execDML(bufSQL);
		}

		res = true;
	}
	catch (CppSQLite3Exception &e)
	{
		dprint3("SQLITE Error: %d:%s\n", e.errorCode(), e.errorMessage());
	}

	return res;
}

int VS_SimpleStorage::FindUsers(VS_Container& cnt, int& entries, VS_AddressBook ab, const vs_user_id& owner, const std::string& query, long client_hash, VS_Container& in_cnt)
{
	if (ab == AB_USER_PICTURE)
	{
		auto res = FindUserPicture(cnt, entries, ab, query, client_hash);
		auto server_hash = cnt.GetLongValueRef(HASH_PARAM);
		if (server_hash && (*server_hash == DEFAULT_VALID_HASH_FOR_NO_PICTURE))
		{
			VS_StorageUserData ude;
			auto q = query;
			VS_NormalizeCallID(q);
			VS_SkipSIPPrefix(q);
			if (FindUserByAlias(q, ude))
			{
				std::string new_query = ude.m_realLogin.GetID();
				int new_entries = 0;
				VS_Container new_cnt;
				new_cnt.AddValue(USERNAME_PARAM, query);	// enh#50722: pass original query: hack for ClientLib v27 (ClientLib v28 uses QUERY_PARAM)
				auto res2 = FindUserPicture(new_cnt, new_entries, ab, new_query, client_hash);
				if (res2 == SEARCH_DONE || res2 == SEARCH_NOT_MODIFIED)
				{
					cnt = std::move(new_cnt);
					entries = new_entries;
					return res2;
				}
			}
		}
		return res;
	}
	else if (ab == AB_COMMON || ab == AB_BAN_LIST || ab == AB_PERSONS) {
		VS_RealUserLogin r(SimpleStrToStringView(owner));
		return m_ab_storage->ABFind(cnt, entries, ab, r.GetUser().c_str(), query, client_hash, &in_cnt);
	}
	else if (ab == AB_MISSED_CALLS || ab == AB_PLACED_CALLS || ab == AB_RECEIVED_CALLS) {
		int64_t last_deleted_call = -1;
		if (GetUserProperty(owner, LAST_DELETED_CALL_PARAM, last_deleted_call))
			in_cnt.AddValueI64(LAST_DELETED_CALL_PARAM, last_deleted_call);
		in_cnt.GetValue(LAST_DELETED_CALL_PARAM, last_deleted_call);
		return m_dbCallLog->GetCalls(cnt, entries, ab, owner, last_deleted_call);
	} else if (ab == AB_GROUPS) {
		GetABGroupsOfUser(owner.m_str, cnt, entries);
		auto server_hash = cnt.GetLongValueRef(HASH_PARAM);
		if (server_hash && VS_CompareHash(client_hash, *server_hash))
		{
			entries = -1;
			return SEARCH_NOT_MODIFIED;
		}
		return SEARCH_DONE;
	} else if (ab == AB_DIRECTORY) {
		entries = 0;
		return SEARCH_DONE;
	} else {
		return SEARCH_FAILED;
	}
}

int VS_SimpleStorage::FindUserPicture(VS_Container& cnt, int& entries, VS_AddressBook ab, const std::string& query, long client_hash)
{
	assert(ab == AB_USER_PICTURE);

	return FindUserPictureImpl(AVATARS_DIRECTORY, entries, query, client_hash, cnt);
}

int VS_SimpleStorage::SetUserPicture(VS_Container &cnt, VS_AddressBook ab, const char *callId, long &hash)
{
	if (ab != AB_USER_PICTURE)
		return -1;
	if (!callId || !*callId)
		return VSS_USER_NOT_VALID;

	const char *buf = cnt.GetStrValueRef("avatar_type");
	if (!buf || !*buf)
		return -1;

	std::string mimeType(buf);
	std::transform(mimeType.begin(), mimeType.end(), mimeType.begin(), tolower);
	std::string extension;

	for (auto it = m_picture_mime_types.begin(); it != m_picture_mime_types.end(); ++it)
	{
		if (it->second == mimeType)
		{
			extension = it->first;
			break;
		}
	}
	if (extension.empty())
		return -1;

	size_t fileSize = 0;
	buf = static_cast<const char*>(cnt.GetBinValueRef("avatar", fileSize));
	if (!buf || !fileSize || (fileSize >= MAX_AVATAR_SIZE))
		return -1;

	std::string fileName(AVATARS_DIRECTORY);

	boost::system::error_code ec;
	boost::filesystem::create_directories(AVATARS_DIRECTORY, ec);
	if (ec) {
		dstream0 << "SetUserPicture: Error creating '" << fileName << "': " << ec.message();
		return VSS_FILE_WRITE_ERROR;
	}

	fileName += VS_GetUserPartEscaped(callId);
	fileName += extension;

	std::ofstream file;
#if defined(_WIN32)
	auto fileName_wstr = vs::UTF8ToWideCharConvert(fileName);
	if (fileName_wstr.empty())
		return VSS_FILE_WRITE_ERROR;

	file.open(fileName_wstr, std::ios::out | std::ios::binary);
#else
	file.open(fileName, std::ios::out | std::ios::binary);
#endif

	if (!file.is_open())
		return VSS_FILE_WRITE_ERROR;

	if (!file.write(buf, fileSize)) {
		file.close();
		return VSS_FILE_WRITE_ERROR;
	}

	file.close();
	std::time_t t = boost::filesystem::last_write_time(fileName);
	if(t != time_t(-1)) hash = VS_MakeHash(std::chrono::system_clock::from_time_t(t));	// hash must be calculated after closing of file

	return 0;
}

int VS_SimpleStorage::DeleteUserPicture(VS_AddressBook ab, const char *callId, long &hash)
{
	if (ab != AB_USER_PICTURE)
		return -1;
	if (!callId || !*callId)
		return VSS_USER_NOT_VALID;

	std::string fileName(AVATARS_DIRECTORY);

	boost::system::error_code ec;
	boost::filesystem::create_directories(AVATARS_DIRECTORY, ec);
	if (ec) {
		dstream0 << "DeleteUserPicture: Error creating '" << fileName << "': " << ec.message();
		return VSS_FILE_WRITE_ERROR;
	}

	fileName += VS_GetUserPartEscaped(callId);
	for (auto it = m_picture_mime_types.begin(); it != m_picture_mime_types.end(); ++it)
	{
		fileName += it->first;
		boost::system::error_code ec;
		if (boost::filesystem::exists(fileName, ec)) {
			boost::filesystem::remove(fileName, ec);
			if (ec)
				return VSS_FILE_WRITE_ERROR;
		}

		fileName.erase(fileName.end() - it->first.length(), fileName.end());
	}

	hash = DEFAULT_VALID_HASH_FOR_NO_PICTURE;

	return 0;
}

void VS_SimpleStorage::UpdateUsersGroups(const std::function<bool(void)>& is_stopping)
{
	if (!m_group_manager) return;

	m_group_manager->UpdateGroupList();

	// clean groups of all users
	auto pLockedUsers = m_users.lock();
	for (UserMap::Iterator it = pLockedUsers->Begin(); it != pLockedUsers->End(); it++)
	{
		VS_StorageUserData* ude = it->data;
		ude->m_groups.clear();
	}

	std::map<std::string, VS_RegGroupInfo> reg_groups;
	m_group_manager->GetRegGroups(reg_groups);

	for (const auto& g : reg_groups)
	{
		auto m = std::make_shared<VS_AbCommonMap>();
		GetRegGroupUsers(g.first, m);
		for (const auto& u : *m)
		{
			auto it = pLockedUsers->Find(u.first.c_str());
			if (it != pLockedUsers->End())
			{
				VS_StorageUserData* ude = it->data;
				ude->m_groups.push_back(g.first);	// add gid of reg_group
			}
		}
	}

	for (UserMap::Iterator it = pLockedUsers->Begin(); it != pLockedUsers->End(); it++)
	{
		VS_StorageUserData* ude = it->data;
		FetchRights(*ude, (VS_UserData::UserRights &) ude->m_rights);
		FetchTarifOpt(*((VS_UserData*)ude));
		ReadApplicationSettings(*ude);

		if (!(ude->m_rights & VS_UserData::UR_COMM_CREATEMULTI))
		{
			ude->m_rights |= VS_UserData::UR_COMM_CREATEMULTI;
			ude->m_tarif_restrictions = 0;
		}
	}
}

int VS_SimpleStorage::GetNamedConfInfo(const char* conf_id, VS_ConferenceDescription& cd, ConferenceInvitation& ci, VS_SimpleStr& /*as_server*/, long& scope)
{
	std::string keyName = MULTI_CONFERENCES_KEY;
	keyName += "\\";
	auto confId = VS_RealUserLogin(conf_id).GetUser();

	keyName += confId;

	VS_RegistryKey confKey(false, keyName);
	if (!confKey.IsValid())
		return VSS_CONF_NOT_FOUND;

	int res = 0;

	// invitation params
	confKey.GetValue(&ci.m_invitation_type, sizeof(int), VS_REG_INTEGER_VT, MMC_INVITATION_TYPE);
	confKey.GetValue(&ci.m_invitation_time, sizeof(int), VS_REG_INTEGER_VT, MMC_INVITATION_TIME);
	confKey.GetValue(&ci.m_auto_invite, sizeof(int), VS_REG_INTEGER_VT, MMC_AUTO_INVITE);

	unsigned long duration = 0xd2d2d; // 10 days
	confKey.GetValue(&duration, sizeof(unsigned long), VS_REG_INTEGER_VT, MMC_DURATION_TAG);
	cd.SetTimeExp(duration);

	{
		unsigned long planned = 0;
		confKey.GetValue(&planned, sizeof(unsigned long), VS_REG_INTEGER_VT, MMC_PLANNEDPARTICIPANTSONLY_TAG);
		cd.m_PlannedPartsOnly = planned != 0;
	}

	if (ci.m_invitation_type == 0) // regular
	{
		confKey.GetValue(&ci.m_invitation_day, sizeof(int), VS_REG_INTEGER_VT, MMC_INVITATION_DAY);
		std::string conf_id;
		confKey.GetString(conf_id, MMC_CONFID_TAG);
		if (ci.m_invitation_day && conf_id.empty())
		{
			auto t = time(nullptr);
			std::tm local_tm;
			localtime_r(&t, &local_tm);
			int days = ci.m_invitation_day;
			int curDay = 1 << local_tm.tm_wday;
			if (!(curDay & days))
				res = VSS_CONF_NOT_STARTED;
			else
			{
				auto hour = ci.m_invitation_time / 60;
				auto minute = ci.m_invitation_time % 60;
				if (hour >= 24)
					hour -= 24;

				// also check for the end time should be provided later
				if (local_tm.tm_hour < hour || (local_tm.tm_hour == hour && local_tm.tm_min < minute))
					res = VSS_CONF_NOT_STARTED;
			}
		}
	}
	else if (ci.m_invitation_type == 1) // one-time
	{
		std::string invitation_date;
		if (confKey.GetString(invitation_date, MMC_INVITATION_DATE) && !invitation_date.empty())
		{
			ci.m_invitation_start_time = ConferenceInvitation::CalculateInvitationStartTime(ci, invitation_date.c_str());
			if (std::chrono::system_clock::now() < ci.m_invitation_start_time)
				res = VSS_CONF_NOT_STARTED;
			cd.m_timeExp = ci.m_invitation_start_time + std::chrono::seconds(duration);
			if (cd.m_timeExp < std::chrono::system_clock::now())
				res = VSS_CONF_EXPIRED;
		}
	}


	// other params
	char buf[256] = { 0 };
	cd.m_name.Empty();
	cd.m_call_id = confId.c_str();

	std::string owner;
	confKey.GetString(owner, MMC_OWNER_TAG);
	owner = vs::UTF8ToLower(owner);
	VS_RealUserLogin r(owner);
	cd.m_owner = r;

	cd.m_type = CT_MULTISTREAM;
	confKey.GetValue(&cd.m_type, sizeof(int), VS_REG_INTEGER_VT, MMC_TYPE_TAG);

	confKey.GetValue(&cd.m_SubType, sizeof(int), VS_REG_INTEGER_VT, MMC_SUBTYPE_TAG);
	if (cd.m_SubType < GCST_FULL || cd.m_SubType > GCST_ROLE)
		cd.m_SubType = GCST_FULL;

	confKey.GetString(cd.m_topic, MMC_TOPIC_TAG);

	*buf = 0;
	confKey.GetValue(buf, sizeof(buf), VS_REG_STRING_VT, MMC_CONFID_TAG);
	cd.m_name = buf;

	*buf = 0;
	confKey.GetValue(buf, sizeof(buf), VS_REG_STRING_VT, MMC_PASS_TAG);
	cd.m_password = buf;

	*buf = 0;
	confKey.GetValue(buf, sizeof(buf), VS_REG_STRING_VT, MMC_MULTICAST_IP_TAG);
	cd.m_multicast_ip = buf;

	cd.m_MaxCast = 4;
	if (cd.m_SubType == GCST_ROLE)
		confKey.GetValue(&cd.m_MaxCast, sizeof(int), VS_REG_INTEGER_VT, MMC_PODIUMS_TAG);

	cd.m_MaxParticipants = -1;
	confKey.GetValue(&cd.m_MaxParticipants, sizeof(int), VS_REG_INTEGER_VT, MMC_MAXPARTS_TAG);

	int broadcast_enabled_value = 0;
	confKey.GetValue(&broadcast_enabled_value, sizeof(broadcast_enabled_value), VS_REG_INTEGER_VT, MMC_BROADCAST_TAG);
	cd.m_isBroadcastEnabled = broadcast_enabled_value != 0;

	int recording = 0;
	confKey.GetValue(&recording, sizeof(recording), VS_REG_INTEGER_VT, MMC_CONFERENCE_RECORDING_TAG);
	cd.m_need_record = recording != 0;

	confKey.GetString(cd.m_rtspEnabledCodecs, MMC_CODECS_TAG);

	confKey.GetString(cd.m_rtspHelperProgram, MMC_RTSP_HELPER_PROGRAM_TAG);

	auto announcesListKeyName(keyName);
	announcesListKeyName += "\\";
	announcesListKeyName += MMC_RTSP_ANNOUNCE_KEY;
	VS_RegistryKey announcesListKey(false, announcesListKeyName);
	if (announcesListKey.IsValid())
	{
		announcesListKey.ResetKey();
		VS_RegistryKey announceKey;
		while (announcesListKey.NextKey(announceKey))
		{
			std::string url;
			if (!announceKey.GetString(url, MMC_RTSP_ANNOUNCE_URL_TAG))
				continue;
			auto announce_it = cd.m_rtspAnnounces.emplace(announceKey.GetName(), VS_ConferenceDescription::RTSPAnnounce()).first;
			announce_it->second.url = std::move(url);
			announceKey.GetString(announce_it->second.username, MMC_RTSP_ANNOUNCE_USERNAME_TAG);
			announceKey.GetString(announce_it->second.password, MMC_RTSP_ANNOUNCE_PASSWORD_TAG);
			announceKey.GetString(announce_it->second.enabled_codecs, MMC_RTSP_ANNOUNCE_CODECS_TAG);
			int rtp_over_tcp_value = 0;
			announceKey.GetValue(&rtp_over_tcp_value, sizeof(rtp_over_tcp_value), VS_REG_INTEGER_VT, MMC_RTSP_ANNOUNCE_RTP_OVER_TCP_TAG);
			announce_it->second.rtp_over_tcp = rtp_over_tcp_value != 0;
			announce_it->second.keepalive_timeout = announce_it->second.rtp_over_tcp ? 0 : 60;
			announceKey.GetValue(&announce_it->second.keepalive_timeout, sizeof(announce_it->second.keepalive_timeout), VS_REG_INTEGER_VT, MMC_RTSP_ANNOUNCE_KEEPALIVE_TIMEOUT_TAG);
			announce_it->second.retries = 0;
			announceKey.GetValue(&announce_it->second.retries, sizeof(announce_it->second.retries), VS_REG_INTEGER_VT, MMC_RTSP_ANNOUNCE_RETRIES_TAG);
			announce_it->second.retry_delay = 10;
			announceKey.GetValue(&announce_it->second.retry_delay, sizeof(announce_it->second.retry_delay), VS_REG_INTEGER_VT, MMC_RTSP_ANNOUNCE_RETRY_DELAY_TAG);
			int active_value = 0;
			announceKey.GetValue(&active_value, sizeof(active_value), VS_REG_INTEGER_VT, MMC_RTSP_ANNOUNCE_ACTIVE_TAG);
			announce_it->second.active = active_value != 0;
			announceKey.GetString(announce_it->second.reason, MMC_RTSP_ANNOUNCE_REASON_TAG);
		}
	}

	{
		auto webpkey(keyName); webpkey += "\\"; webpkey += MMC_WEBPARAMS_KEY;
		VS_RegistryKey key(false, webpkey);
		cd.m_PrivacyType = cd.e_PT_Private;
		key.GetValue(&cd.m_PrivacyType, 4, VS_REG_INTEGER_VT, MMC_WEBPARAMS_TYPE);
		cd.m_public = cd.m_PrivacyType == cd.e_PT_Public;
	}

	{
		auto ClientRights(keyName); ClientRights += "\\"; ClientRights += MMC_CLIENTRIGHTS_KEY;
		VS_RegistryKey key(false, ClientRights);
		cd.m_CMR_guest = cd.m_CMR_user = CMRFlags::ALL;
		key.GetValue(&cd.m_CMR_guest, 4, VS_REG_INTEGER_VT, MMC_CLIENTRIGHTS_GUEST);
		key.GetValue(&cd.m_CMR_user, 4, VS_REG_INTEGER_VT, MMC_CLIENTRIGHTS_USER);
	}

	auto ModeratorsKeyName(keyName); ModeratorsKeyName += "\\";	ModeratorsKeyName += "Moderators";

	VS_RegistryKey parts_root(false, ModeratorsKeyName);
	VS_RegistryKey part;
	parts_root.ResetKey();

	while (parts_root.NextKey(part))
	{
		const char* call_id = part.GetName();
		if (call_id && *call_id)
			cd.m_moderators.insert((const char*)(VS_RealUserLogin)call_id);
	}

	return res;
}

static void SaveRTSPAnnounce(const VS_ConferenceDescription::RTSPAnnounce& data, string_view announce_id, const VS_SimpleStr& call_id)
{
	std::string announceKeyName;
	announceKeyName.reserve(128);
	announceKeyName += MULTI_CONFERENCES_KEY;
	announceKeyName += '\\';
	announceKeyName += call_id;
	announceKeyName += '\\';
	announceKeyName += MMC_RTSP_ANNOUNCE_KEY;
	announceKeyName += '\\';
	announceKeyName += announce_id;

	VS_RegistryKey announceKey(false, announceKeyName, false);
	if (!announceKey.IsValid())
		return;

	int active_value = data.active ? 1 : 0;
	announceKey.SetValue(&active_value, sizeof(active_value), VS_REG_INTEGER_VT, MMC_RTSP_ANNOUNCE_ACTIVE_TAG);
	if (!data.reason.empty())
		announceKey.SetString(data.reason.c_str(), MMC_RTSP_ANNOUNCE_REASON_TAG);
	else
		announceKey.RemoveValue(MMC_RTSP_ANNOUNCE_REASON_TAG);
}

int VS_SimpleStorage::UpdateNamedConfInfo_RTSPAnnounce(const VS_ConferenceDescription& cd, string_view announce_id)
{
	std::string key_name;
	key_name.reserve(128);
	key_name += MULTI_CONFERENCES_KEY;
	key_name += '\\';
	key_name += cd.m_call_id.m_str;

	VS_RegistryKey confKey(false, key_name);
	if (!confKey.IsValid())
		return VSS_CONF_NOT_FOUND;

	if (!announce_id.empty())
	{
		const auto announce_it = cd.m_rtspAnnounces.find(announce_id);
		if (announce_it == cd.m_rtspAnnounces.end())
			return 0;
		SaveRTSPAnnounce(announce_it->second, announce_id, cd.m_call_id);
	}
	else
	{
		for (const auto& announce: cd.m_rtspAnnounces)
			SaveRTSPAnnounce(announce.second, announce.first, cd.m_call_id);
	}
	return 0;
}

bool VS_SimpleStorage::ManageGroups_CreateGroup(const vs_user_id& owner, const VS_WideStr& gname, long& gid, long& hash)
{
	if (!owner)
		return false;
	unsigned long N_retries = 0xFFFF;		// max amount of CustomGroups
	bool exist(true);
	long new_gid(0x0FFFFFFF);
	std::string key_name_base = USERS_KEY;
	key_name_base += "\\";
	key_name_base += VS_GetUserPartEscaped(owner);
	key_name_base += "\\";
	key_name_base += CUSTOM_GROUPS_KEY;
	key_name_base += "\\";
	std::string key_name;
	while (exist && N_retries > 0) {
		key_name = key_name_base;
		key_name += std::to_string(new_gid);
		VS_RegistryKey key(false, key_name);
		if (!key.IsValid())
			break;
		--new_gid;
		--N_retries;
	}
	if (N_retries <= 0)
		return false;

	VS_RegistryKey key(false, key_name, false, true);
	bool ret = key.SetValue(gname.m_str, 0, VS_REG_WSTRING_VT, GROUP_NAME_TAG);
	if (ret)
		gid = new_gid;

	hash = CalcABGroupsHash(owner.m_str);
	return ret;
}

bool VS_SimpleStorage::ManageGroups_DeleteGroup(const vs_user_id& owner, const long gid, long& hash)
{
	if (!owner)
		return false;
	std::string key_name = USERS_KEY;
	key_name += "\\";
	key_name += VS_GetUserPartEscaped(owner);
	key_name += "\\";
	key_name += CUSTOM_GROUPS_KEY;
	VS_RegistryKey key(false, key_name, false);
	bool ret = key.RemoveKey(std::to_string(gid));

	hash = CalcABGroupsHash(owner.m_str);
	return ret;
}

bool VS_SimpleStorage::ManageGroups_RenameGroup(const vs_user_id& owner, const long gid, const VS_WideStr& gname, long& hash)
{
	if (!owner)
		return false;
	std::string key_name = USERS_KEY;
	key_name += "\\";
	key_name += VS_GetUserPartEscaped(owner);
	key_name += "\\";
	key_name += CUSTOM_GROUPS_KEY;
	key_name += "\\";
	key_name += std::to_string(gid);
	VS_RegistryKey key(false, key_name, false);
	auto ret = key.SetValue(gname, 0, VS_REG_WSTRING_VT, GROUP_NAME_TAG);
	hash = CalcABGroupsHash(owner.m_str);
	return ret;
}

bool VS_SimpleStorage::ManageGroups_AddUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash)
{
	if (!owner || !call_id)
		return false;
	{	// check it is not a SystemGroup
		std::map<std::string, VS_RegGroupInfo> reg_groups;
		m_group_manager->GetRegGroups(reg_groups);
		for (const auto& g : reg_groups)
		{
			if (::atol(g.first.c_str()) == gid)
				return false;
		}
		if (reg_groups.find(std::to_string(gid)) != reg_groups.end())
			return false;
	}
	std::string key_name = USERS_KEY;
	key_name += "\\";
	key_name += VS_GetUserPartEscaped(owner.m_str);
	key_name += "\\";
	key_name += CUSTOM_GROUPS_KEY;
	key_name += "\\";
	key_name += std::to_string(gid);
	key_name += "\\";
	key_name += "Users";
	VS_RegistryKey key(false, key_name, false, true);
	VS_RealUserLogin r2(SimpleStrToStringView(call_id));
	auto ret = key.SetString("", r2.IsOurSID() ? r2.GetUser().c_str() : call_id.m_str);
	hash = CalcABGroupsHash(owner.m_str);
	return ret;
}

bool VS_SimpleStorage::ManageGroups_DeleteUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash)
{
	if (!owner)
		return false;
	std::string key_name = USERS_KEY;
	key_name += "\\";
	key_name += VS_GetUserPartEscaped(owner.m_str);
	key_name += "\\";
	key_name += CUSTOM_GROUPS_KEY;
	key_name += "\\";
	key_name += std::to_string(gid);
	key_name += "\\";
	key_name += "Users";
	VS_RegistryKey key(false, key_name, false);
	VS_RealUserLogin r2(SimpleStrToStringView(call_id));
	auto ret = (key.RemoveValue(call_id.m_str) || key.RemoveValue(r2.GetUser()));
	hash = CalcABGroupsHash(owner.m_str);
	return ret;
}

bool VS_SimpleStorage::FindGuest(const vs_user_id& id, VS_UserData& user)
{
	if (!id)
	{
		error_code = VSS_USER_NOT_VALID;
		return false;
	};
	VS_RealUserLogin	realLogin(SimpleStrToStringView(id));

	auto pLockedUsers = m_users.lock();
	UserMap::ConstIterator i = pLockedUsers->Find(realLogin);
	if (!i)
	{
		error_code = VSS_USER_NOT_FOUND;
		return false;
	}
	user = i;
	return true;
}

void VS_SimpleStorage::SetUserStatus(const VS_SimpleStr& call_id, int status, const VS_ExtendedStatusStorage &extStatus, bool /*set_server*/, const VS_SimpleStr& /*server*/)
{
	if (!call_id)
		return;
	std::string root = USERS_KEY;
	root += '\\';
	root += VS_GetUserPartEscaped(call_id);
	VS_RegistryKey	key(false, root, false, false);
	key.SetValue(&status, sizeof(status), VS_REG_INTEGER_VT, ONLINE_STATUS_TAG);
	if (IsLDAP() && status == USER_AVAIL)
	{
		long one = 1;
		key.SetValue(&one, sizeof(one), VS_REG_INTEGER_VT, LDAP_STATUS_TAG);
	}

	/**
	Write ext status
	name - value (or set value) - isSticky
	*/
	auto changed_statuses = extStatus.GetChangedStatuses();
	auto reset = changed_statuses.second;
	auto &extStMap = changed_statuses.first;
	auto deletedStatuses = extStatus.GetDeletedStatuses();
	if (reset)
	{
		auto allowed_ext_st = VS_ExtendedStatusStorage::GetStickyStatusesNames();
		for (const auto &i : allowed_ext_st)
		{
			if (!extStatus.IsStatusExist(i))
				deletedStatuses.insert(i);
		}
	}
	if (status>USER_LOGOFF&&(!extStMap.empty()||!deletedStatuses.empty()))
	{
		root += '\\'; root += EXTENDED_STATUS_KEY;
		key = VS_RegistryKey(false, root, false, true);
		for (const auto&i : extStMap)
			boost::apply_visitor(store_ext_status_to_reg(key,i.first.c_str()),i.second);
		for (const auto &i : deletedStatuses)
			key.RemoveValue(i);
	}
	else
		CleanExtStatuses(call_id);
}

void VS_SimpleStorage::CleanExtStatuses(const char *call_id)
{
	if (!call_id)
		return;
	VS_RealUserLogin r(call_id);
	std::string key_name = USERS_KEY; key_name += "\\"; key_name += r.GetUser();
	key_name += "\\"; key_name += EXTENDED_STATUS_KEY;
	VS_RegistryKey	key(false, key_name, false);
	key.ResetValues();
	std::string name;
	std::string val;
	int res(0);
	std::vector<std::string> to_delete;
	while((res = key.NextString(val,name)) != 0)
	{
		if(res<0)
			continue;
		if(!VS_ExtendedStatusStorage::IsStatusSticky(name.c_str()))
			to_delete.emplace_back(name);
	}
	for (const auto& x: to_delete)
		key.RemoveKey(x);
}
bool VS_SimpleStorage::GetExtendedStatus(const char *call_id, VS_ExtendedStatusStorage &extStatus)
{
	if (!call_id || !*call_id)
		return false;
	VS_RealUserLogin r(call_id);
	std::string key_name = USERS_KEY; key_name += "\\"; key_name += r.GetUser();
	key_name += "\\"; key_name += EXTENDED_STATUS_KEY;
	VS_RegistryKey	key(false, key_name, false);

	int32_t int_val(0);
	if (key.GetValue(&int_val, sizeof(int_val), RegistryVT::VS_REG_INTEGER_VT, EXTSTATUS_NAME_EXT_STATUS))
		extStatus.UpdateStatus(EXTSTATUS_NAME_EXT_STATUS, int_val);
	else
		extStatus.DeleteStatus(EXTSTATUS_NAME_EXT_STATUS);
	std::string description;
	if (key.GetString(description, EXTSTATUS_NAME_DESCRIPTION))
		extStatus.UpdateStatus(EXTSTATUS_NAME_DESCRIPTION, std::move(description));
	else
		extStatus.DeleteStatus(EXTSTATUS_NAME_DESCRIPTION);
	int64_t int64_val(0);
	if (key.GetValue(&int64_val, sizeof(int64_val), RegistryVT::VS_REG_INT64_VT, EXTSTATUS_NAME_LAST_ONLINE_TIME))
		extStatus.UpdateStatus(EXTSTATUS_NAME_LAST_ONLINE_TIME, int64_val);
	else
		extStatus.DeleteStatus(EXTSTATUS_NAME_LAST_ONLINE_TIME);
	if (key.GetValue(&int_val, sizeof(int_val), RegistryVT::VS_REG_INTEGER_VT, EXTSTATUS_NAME_DEVICE_TYPE))
		extStatus.UpdateStatus(EXTSTATUS_NAME_DEVICE_TYPE, int_val);
	else
		extStatus.DeleteStatus(EXTSTATUS_NAME_DEVICE_TYPE);
	if (key.GetValue(&int_val, sizeof(int_val), RegistryVT::VS_REG_INTEGER_VT, EXTSTATUS_NAME_CAMERA))
		extStatus.UpdateStatus(EXTSTATUS_NAME_CAMERA, int_val);
	else
		extStatus.DeleteStatus(EXTSTATUS_NAME_CAMERA);
	if (key.GetValue(&int_val, sizeof(int_val), RegistryVT::VS_REG_INTEGER_VT, EXTSTATUS_NAME_IN_CLOUD))
		extStatus.UpdateStatus(EXTSTATUS_NAME_IN_CLOUD, int_val);
	else
		extStatus.DeleteStatus(EXTSTATUS_NAME_IN_CLOUD);
	if (key.GetValue(&int_val, sizeof(int_val), RegistryVT::VS_REG_INTEGER_VT, EXTSTATUS_NAME_FWD_TYPE))
		extStatus.UpdateStatus(EXTSTATUS_NAME_FWD_TYPE, int_val);
	else
		extStatus.DeleteStatus(EXTSTATUS_NAME_FWD_TYPE);
	std::string fwd_call_id;
	if (key.GetString(fwd_call_id, EXTSTATUS_NAME_FWD_CALL_ID))
		extStatus.UpdateStatus(EXTSTATUS_NAME_FWD_CALL_ID, std::move(fwd_call_id));
	else
		extStatus.DeleteStatus(EXTSTATUS_NAME_FWD_CALL_ID);
	std::string fwd_timeout_call_id;
	if (key.GetString(fwd_timeout_call_id, EXTSTATUS_NAME_FWD_TIMEOUT_CALL_ID))
		extStatus.UpdateStatus(EXTSTATUS_NAME_FWD_TIMEOUT_CALL_ID, std::move(fwd_timeout_call_id));
	else
		extStatus.DeleteStatus(EXTSTATUS_NAME_FWD_TIMEOUT_CALL_ID);
	if (key.GetValue(&int_val, sizeof(int_val), RegistryVT::VS_REG_INTEGER_VT, EXTSTATUS_NAME_FWD_TIMEOUT))
		extStatus.UpdateStatus(EXTSTATUS_NAME_FWD_TIMEOUT, int_val);
	else
		extStatus.DeleteStatus(EXTSTATUS_NAME_FWD_TIMEOUT);
	return true;
}
void VS_SimpleStorage::GetUpdatedExtStatuses(std::map<std::string, VS_ExtendedStatusStorage> &)
{
	//TODO: implement
	assert(false);
}

void VS_SimpleStorage::SaveAutoLoginKey(const std::string& user_id, const char* appID, const char* autoKey)
{
	SaveAutoLoginKey(user_id.c_str(), appID, autoKey);
}

void VS_SimpleStorage::SaveAutoLoginKey(const char* user_id, const char* appID, const char* autoKey)
{
	if (!user_id || !appID || !*appID || !strlen(appID) || !autoKey || !*autoKey || !strlen(autoKey))
		return;

	std::string root = USERS_KEY;	root += "\\";	root += user_id;	root += "\\AutoLogins";
	VS_RegistryKey logins(false, root, false, true);
	logins.SetString(autoKey, appID);
}

void VS_SimpleStorage::OnUserLoggedInAtEndpoint(const VS_UserData* ud)
{
	if (!ud)
		return;
	VS_SimpleStr str_key = ENDPOINTS_KEY;		str_key += "\\";		str_key += ud->m_appID;

	bool IsRegistered = VS_RegistryKey(false, str_key.m_str).HasValue(REGISTERED_TAG);

	VS_RegistryKey reg_key(false, str_key.m_str, false, true);
	if (!reg_key.IsValid())
		return;

	reg_key.SetValue(&ud->m_type, sizeof(int), VS_REG_INTEGER_VT, TYPE_TAG);
	//len = ud.m_key.ByteLength();
	//reg_key.SetString(len == 0 ? "" : (const char*)ud.m_key, KEY_TAG);

	if (!IsRegistered)
		reg_key.SetValue(std::chrono::system_clock::now(), REGISTERED_TAG);

	reg_key.SetValue(std::chrono::system_clock::now(), VS_SimpleStorage::LASTCONNECTED_TAG);

	int online_status = USER_AVAIL;
	reg_key.SetValue(&online_status, sizeof(online_status), VS_REG_INTEGER_VT, ONLINE_STATUS_TAG);

	auto user_escaped = VS_GetUserPartEscaped(ud->m_name.m_str);
	reg_key.SetString(user_escaped.c_str(), LASTUSER_TAG);
	SetEndpointProperty(ud->m_appID, VS_SimpleStorage::LOGGEDUSER_TAG, user_escaped.c_str());

	// Update user Registry
	if (!user_escaped.empty() && ud->m_type != VS_UserData::UT_GUEST)
	{
		std::string root= USERS_KEY;
		root += "\\";
		root += user_escaped;

		VS_RegistryKey reg_user(false, root, false, true);

		reg_user.SetValue(std::chrono::system_clock::now(), LAST_LOGGEDIN_TAG);
		reg_user.SetString(ud->m_appID, LAST_ENDPOINT_TAG);
	}
}

bool VS_SimpleStorage::CheckSessionID(const char *password)
{
	return password == nullptr
		? false
		: m_session_id == password;
}
const char* VS_SimpleStorage::GetSessionID() const
{
	return m_session_id.m_str;
}

void VS_SimpleStorage::SearchUsersByAlias(const char* email, const char* name, std::map<VS_SimpleStr, VS_StorageUserData> &result)
{
	if ((email&&*email) || (name&&*name))
	{
		vs_user_id user_id;
		if ((ResolveCallIDByAlias(email, user_id) && !!user_id) ||
			(ResolveCallIDByAlias(name, user_id) && !!user_id))
		{
			auto u = SimpleStrToStringView(user_id);
			VS_StorageUserData ud;
			if (FindUser(user_id.m_str, ud))
			{
				result[ud.m_name] = ud;
			}
			else if (VS_IsRTPCallID(u)) {
				ud.m_name = user_id;
				ud.m_displayName = vs::PrettyRTPName(u);
				ud.m_online_status = VS_CheckLicense(LE_MULTIGATEWAY)? USER_AVAIL: USER_LOGOFF;
				result[user_id] = ud;
			}
			else if (!VS_RealUserLogin(u).IsOurSID()) {
				ud.m_name = user_id;
				result[user_id] = ud;
			}
		}
	}

	// check for phone
	if (name && *name)
	{
		std::string aterm = name;
		boost::trim(aterm);

		// todo: if match regexp [+-().#]
		const static boost::regex e(" *(?:[0-9*#+\\-() \\.]*) *", boost::regex::optimize);
		bool IsPhone = boost::regex_match(aterm, e);

		if (IsPhone)
		{
			// todo: normalize as telephone
			const std::string allowed_symbols = "0123456789*#";
			std::string tel_normilized = "+";
			for (unsigned int i = 0; i<aterm.length(); i++)
			{
				if ((i == 0) && aterm[i] == '+')
					continue;
				if (allowed_symbols.find(aterm[i]) != std::string::npos)
					tel_normilized += aterm[i];
			}
			if (tel_normilized.length()>1)
			{
				VS_SimpleStr user_id;
				if (ResolveCallIDByAlias(tel_normilized.c_str(), user_id) && !!user_id)
				{
					VS_StorageUserData ud;
					if (FindUser(user_id.m_str, ud))
					{
						result[ud.m_name] = ud;
						//j++;
						//cnt.AddValue(USERNAME_PARAM, ud.m_name);
						//cnt.AddValue(CALLID_PARAM,   ud.m_name);
						//cnt.AddValue(DISPLAYNAME_PARAM, ud.m_displayName);
						//cnt.AddValue(USERPRESSTATUS_PARAM, (long)ud.m_online_status);
					}
				}
			}
		}
	}
}

int VS_SimpleStorage::AddToAddressBook(VS_AddressBook ab, const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt, VS_SimpleStr& add_call_id, std::string& add_display_name, VS_TransportRouterServiceHelper* srv)
{
	if (ab == AB_PHONES)
		return AddToAddressBook_Phones(VS_GetUserPartEscaped(user_id1.m_str), cnt, hash, rCnt);

	const char* call_id2_ = cnt.GetStrValueRef(CALLID_PARAM);
	if (!call_id2_ || !*call_id2_)
		call_id2_ = cnt.GetStrValueRef(NAME_PARAM);
	if (!call_id2_ || !*call_id2_)
		return VSS_USER_NOT_VALID;
	auto call_id2 = std::string(VS_RemoveTranscoderID_sv(call_id2_));
	const char* u_dn = cnt.GetStrValueRef(DISPLAYNAME_PARAM);

	VS_SimpleStr resolved;
	if (ResolveCallIDByAlias(call_id2.c_str(), resolved) && !!resolved)
		call_id2 = resolved.m_str;

	int ret = m_ab_storage->ABAdd(ab, user_id1, call_id2.c_str(), u_dn, hash, add_call_id, add_display_name);
	if (ret == 0 && srv != nullptr)		// no error, than update AB_GROUPS
	{
		VS_Container update_ab_cnt;
		update_ab_cnt.AddValue(METHOD_PARAM, ONADDRESSBOOKCHANGE_METHOD);
		update_ab_cnt.AddValue(USERNAME_PARAM, user_id1);
		update_ab_cnt.AddValueI32(ADDRESSBOOK_PARAM, AB_GROUPS);
		srv->PostRequest(srv->OurEndpoint(), 0, update_ab_cnt, 0, ADDRESSBOOK_SRV);
	}

	return ret;
}

int VS_SimpleStorage::RemoveFromAddressBook(VS_AddressBook ab, const vs_user_id& user_id1, const vs_user_id& user_id2, VS_Container &cnt, long& hash, VS_Container &rCnt)
{
	if (ab == AB_PHONES)
		return RemoveFromAddressBook_Phones(VS_GetUserPartEscaped(user_id1.m_str), cnt, hash, rCnt);
	else if (ab == AB_USER_PICTURE)
		return DeleteUserPicture(ab, user_id1, hash);
	return m_ab_storage->ABRemove(ab, user_id1, user_id2, hash);
}

int VS_SimpleStorage::UpdateAddressBook(VS_AddressBook ab, const vs_user_id& user_id1, const char* call_id2, VS_Container& cnt, long& hash, VS_Container& rCnt)
{
	if (ab == AB_PHONES)
		return UpdateAddressBook_Phones(VS_GetUserPartEscaped(user_id1.m_str), cnt, hash, rCnt);
	else if (ab == AB_USER_PICTURE)
		return SetUserPicture(cnt, ab, call_id2, hash);
	else
		return m_ab_storage->ABUpdate(ab, user_id1, call_id2, cnt, hash);
}

template<class Func>
void ParseConnectString(const std::string& str, Func &&action) {
	if (str.empty()) return;

	std::string token;
	std::istringstream tokens(str);
	while (std::getline(tokens, token, ',')) action(token);
}

void VS_SimpleStorage::AddServerIPAsAlias(VS_UserData& ud) const
{
	std::set<std::string> v;
	GetServerIPAsAliasPostfix(v);

	VS_RealUserLogin r(SimpleStrToStringView(ud.m_name));
	for (auto const& i : v)
	{
		auto s = r.GetUser();
		s += i;
		ud.m_aliases.Assign(s.c_str(), 0);
	}
}

void VS_SimpleStorage::GetServerIPAsAliasPostfix(std::set<std::string>& v) const
{
	// Current Connect (server)
	std::string value;
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	if (key.GetString(value, CURRENT_CONNECT_TAG) && !value.empty())
	{
		ParseConnectString(value, [&v](std::string &token) {	// expect string: "ip:port,ip:port..."
			size_t pos(std::string::npos);
			if ((pos = token.find_first_of(':')) != std::string::npos) {
				token.erase(pos);

				std::string call_id = "@";
				call_id += token;
				v.insert(call_id);
			}
		});
	}

	// Current Connect (gateway)
	value.clear();
	if (key.GetString(value, CURRENT_GATEWAY_CONNECT_TAG) && !value.empty())
	{
		ParseConnectString(value, [&v](std::string &token) {	// expect string: "TCP:ip:port,UDP:ip:port..."
			size_t start(std::string::npos), end(std::string::npos);
			if ((start = token.find_first_of(':')) == std::string::npos) return;
			if ((end = token.find_first_of(':', start + 1)) == std::string::npos) return;
			++start;
			v.emplace(std::string("@") + token.substr(start, end - start));
		});
	}

	std::string hname = boost::asio::ip::host_name();
	if (!hname.empty())
		v.emplace(std::string("@") + hname);

	// endpoint connects
	std::string serverName = g_tr_endpoint_name;
	const unsigned connCount = net::endpoint::GetCountConnectTCP(serverName);
	for (unsigned i = 0; i < connCount; ++i)
	{
		auto conn = net::endpoint::ReadConnectTCP(i + 1, serverName);
		v.emplace(std::string("@") + conn->host);
	}

	value.clear();
	if (key.GetString(value, "SIP External Host") > 0 && !value.empty())
		v.emplace(std::string("@") + value);
}

bool VS_SimpleStorage::DeleteUser(const vs_user_id& id)
{
	if (!id)
		return false;
	VS_RealUserLogin r(SimpleStrToStringView(id));

	auto pLockedUsers = m_users.lock();
	UserMap::Iterator it = pLockedUsers->Find((const char*)r);
	if (!!it && it->data->m_client_type == CT_TRANSCODER)
		return pLockedUsers->Erase(r) > 0;
	return true;

}

boost::signals2::connection VS_SimpleStorage::Connect_AliasesChanged(const AliasesChangedSlot &slot)
{
	return m_fireAliasesChanged.connect(slot);
}

void VS_SimpleStorage::FindUsersPhones(const vs_userpart_escaped& owner, std::vector<VS_UserPhoneItem> &v)
{
	if (owner.empty())
		return;
	std::string root = USERS_KEY;
	root += "\\";
	root += owner;
	root += PHONE_BOOK_TAG;
	VS_RegistryKey phones_root(false, root, false, true);
	phones_root.ResetKey();
	VS_RegistryKey  phone_item;
	while (phones_root.NextKey(phone_item))
	{
		VS_UserPhoneItem item;
		if (phone_item.IsValid())
		{
			long type = 0;
			char buff[1024];

			if (phone_item.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, USERPHONE_PARAM) <= 0) continue;
			item.phone = buff;

			if (phone_item.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, CALLID_PARAM) <= 0) continue;
			item.call_id = buff;

			if (phone_item.GetValue(&type, sizeof(type), VS_REG_INTEGER_VT, TYPE_PARAM) <= 0) continue;

			item.id = root.c_str();				// "Users\a\Phone Book"
			item.id += "\\";
			item.id += phone_item.GetName();

			item.type = (VS_UserPhoneType)type;
			item.editable = true;

			v.push_back(item);
		}
	}
}

int32_t VS_SimpleStorage::GetPhoneBookHash(const vs_userpart_escaped& call_id)
{
	if (call_id.empty())
		return 0;
	long UsersPhonesHash_Global = GetUsersPhonesHash_Global();
	long UsersPhonesHash_User = GetUsersPhonesHash_User(call_id);

	if (UsersPhonesHash_Global != UsersPhonesHash_User)
	{
		SetUsersPhonesHash_User(call_id, UsersPhonesHash_Global);
		UpdatePhoneBookHash(call_id);
	}

	std::string root = GetUserPhoneRegPath(call_id);
	VS_RegistryKey phones_root(false, root, false, true);
	int32_t hash_value(0);
	if (phones_root.IsValid())
		phones_root.GetValue(&hash_value, sizeof(hash_value), VS_REG_INTEGER_VT, HASH_PARAM);
	if (hash_value == 0)
	{
		UpdatePhoneBookHash(call_id);
		hash_value = GetPhoneBookHash(call_id);
	}
	return hash_value;
}

std::string VS_SimpleStorage::GetUserPhoneRegPath(const vs_userpart_escaped& call_id) {
	if (call_id.empty())
		return {};
	std::string root = USERS_KEY;
	root += "\\";
	root += call_id;
	root += PHONE_BOOK_TAG;
	return root;
}

void UpdateHash(const std::string& reg_path) {
	VS_RegistryKey phones_root(false, reg_path, false, true);
	long hash(0);
	if (phones_root.IsValid())
		phones_root.GetValue(&hash, sizeof(hash), VS_REG_INTEGER_VT, HASH_PARAM);
	++hash;
	if (hash == 0)
		++hash;
	phones_root.SetValue(&hash, sizeof(hash), VS_REG_INTEGER_VT, HASH_PARAM);
}

void VS_SimpleStorage::UpdatePhoneBookHash(const vs_userpart_escaped& call_id)
{
	if (call_id.empty())
		return;
	std::string root = GetUserPhoneRegPath(call_id);
	UpdateHash(root);
}

long VS_SimpleStorage::GetUsersPhonesHash_Global()
{
	long hash(0);
	VS_RegistryKey	phones_root(false, USERS_KEY);
	if (phones_root.IsValid())
		phones_root.GetValue(&hash, sizeof(hash), VS_REG_INTEGER_VT, HASH_PHONES_TAG);
	if (hash == 0)
	{
		UpdateUsersPhonesHash_Global();
		hash = GetUsersPhonesHash_Global();
	}
	return hash;
}

void VS_SimpleStorage::UpdateUsersPhonesHash_Global()
{
	long hash(0);
	VS_RegistryKey	phones_root(false, USERS_KEY, false, true);
	if (phones_root.IsValid())
		phones_root.GetValue(&hash, sizeof(hash), VS_REG_INTEGER_VT, HASH_PHONES_TAG);
	hash++;
	if (hash == 0)
		hash++;
	phones_root.SetValue(&hash, sizeof(hash), VS_REG_INTEGER_VT, HASH_PHONES_TAG);
}

long GetHash(const std::string &reg_path) {
	VS_RegistryKey phones_root(false, reg_path, false, true);
	long hash(0);
	if (phones_root.IsValid())
		phones_root.GetValue(&hash, sizeof(hash), VS_REG_INTEGER_VT, HASH_PARAM);
	return hash;
}

void SetHash(const std::string &reg_path, long global_hash) {
	VS_RegistryKey phones_root(false, reg_path, false, true);
	if (phones_root.IsValid())
		phones_root.SetValue(&global_hash, sizeof(global_hash), VS_REG_INTEGER_VT, HASH_PARAM);
}

long VS_SimpleStorage::GetUsersPhonesHash_User(const vs_userpart_escaped& call_id)
{
	if (call_id.empty())
		return 0;
	std::string root = GetUserPhoneRegPath(call_id);
	return GetHash(root);
}

void VS_SimpleStorage::SetUsersPhonesHash_User(const vs_userpart_escaped& call_id, long global_hash)
{
	if (call_id.empty())
		return;
	std::string root = GetUserPhoneRegPath(call_id);
	SetHash(root, global_hash);
}

int VS_SimpleStorage::AddToAddressBook_Phones(const vs_userpart_escaped& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt)
{
	const char* call_id2 = cnt.GetStrValueRef(CALLID_PARAM);
	if (!call_id2 || !*call_id2)
		return VSS_USER_NOT_VALID;

	const char* user_phone = cnt.GetStrValueRef(USERPHONE_PARAM);
	if (!user_phone || !*user_phone)
		return VSS_USER_NOT_FOUND;

	int32_t type;
	if (!cnt.GetValue(TYPE_PARAM, type))
		return VSS_USER_NOT_FOUND;

	std::string phones_keyname = GetUserPhoneRegPath(user_id1);
	VS_RegistryKey	phones_root(false, phones_keyname, false, true);
	if (!phones_root.IsValid())
		return VSS_USER_NOT_FOUND;

	// find last id
	long num(0);
	bool found(false);

	do{
		++num;
		phones_keyname += "\\";
		char key_md5[32 + 1] = { 0 };
		VS_GenKeyByMD5(key_md5);
		phones_keyname += key_md5;
		VS_RegistryKey tmp(false, phones_keyname);
		if (!tmp.IsValid())
			found = true;
	} while (!found && num < 100);

	if (!found)
		return VSS_REGISTRY_ERROR;

	rCnt.AddValue(ID_PARAM, phones_keyname);
	VS_RegistryKey key(false, phones_keyname, false, true);
	key.SetValue(&type, sizeof(int32_t), VS_REG_INTEGER_VT, TYPE_PARAM);
	rCnt.AddValue(TYPE_PARAM, type);

	key.SetString(user_phone, USERPHONE_PARAM);
	rCnt.AddValue(USERPHONE_PARAM, user_phone);

	key.SetString(call_id2, CALLID_PARAM);
	rCnt.AddValue(CALLID_PARAM, call_id2);

	rCnt.AddValue(EDITABLE_PARAM, true);

	UpdatePhoneBookHash(user_id1);
	hash = GetPhoneBookHash(user_id1);
	return 0;
}

int VS_SimpleStorage::UpdateAddressBook_Phones(const vs_userpart_escaped& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt)
{
	const char* id = cnt.GetStrValueRef(ID_PARAM);
	if (!id || !*id)
		return VSS_USER_NOT_VALID;

	const char* call_id2 = cnt.GetStrValueRef(CALLID_PARAM);
	if (!call_id2 || !*call_id2)
		return VSS_USER_NOT_VALID;

	const char* user_phone = cnt.GetStrValueRef(USERPHONE_PARAM);
	if (!user_phone || !*user_phone)
		return VSS_USER_NOT_FOUND;

	int32_t type;
	if (!cnt.GetValue(TYPE_PARAM, type))
		return VSS_USER_NOT_FOUND;

	std::string keyname = GetUserPhoneRegPath(user_id1);
	const char* p = strstr(id, keyname.c_str());
	if (!p)
		return VSS_USER_NOT_VALID;
	p += keyname.length();
	p++;			// skip slash

	VS_RegistryKey key_check(false, id);
	if (!key_check.IsValid())
		return VSS_USER_NOT_FOUND;

	rCnt.AddValue(ID_PARAM, id);
	VS_RegistryKey key(false, id, false, false);
	key.SetValue(&type, sizeof(int32_t), VS_REG_INTEGER_VT, TYPE_PARAM);
	rCnt.AddValue(TYPE_PARAM, type);

	key.SetString(user_phone, USERPHONE_PARAM);
	rCnt.AddValue(USERPHONE_PARAM, user_phone);

	key.SetString(call_id2, CALLID_PARAM);
	rCnt.AddValue(CALLID_PARAM, call_id2);

	rCnt.AddValue(EDITABLE_PARAM, true);

	UpdatePhoneBookHash(user_id1);
	hash = GetPhoneBookHash(user_id1);
	return 0;
}

int VS_SimpleStorage::RemoveFromAddressBook_Phones(const vs_userpart_escaped& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt)
{
	const char* id = cnt.GetStrValueRef(ID_PARAM);
	if (!id || !*id)
		return VSS_USER_NOT_VALID;

	std::string keyname = GetUserPhoneRegPath(user_id1);
	const char* p = strstr(id, keyname.c_str());
	if (!p)
		return VSS_USER_NOT_VALID;
	p += keyname.length();
	p++;			// skip slash

	VS_RegistryKey key_check(false, id);
	if (!key_check.IsValid())
		return VSS_USER_NOT_FOUND;

	rCnt.AddValue(ID_PARAM, id);
	VS_RegistryKey key(false, keyname, false, false);
	key.RemoveKey(p);

	UpdatePhoneBookHash(user_id1);
	hash = GetPhoneBookHash(user_id1);
	return 0;
}

// Digest authentication.
// password format: $4*nonce*ha2*response
bool VS_SimpleStorage::TryDigestLogin(const VS_TempUserData &user, const VS_SimpleStr &password)
{
	if (!password || !user.m_HA1_password) return false;

	// validate password format
	if (password.Length() < 3 || password.m_str[0] != '$' || password.m_str[1] != '4') return false;

	{
		int count = 0;
		char *pos;
		for (pos = password.m_str + 2; *pos; pos++)
		{
			count += *pos == '*';
			if (*pos == '*' && *(pos - 1) == '*') return false;
		}
		if (count != 3 || *(pos - 1) == '*') return false;
	}

	std::vector<std::string> ha1;
	VS_SimpleStr tmp = user.m_HA1_password;
	ha1.emplace_back(tmp.m_str);
	tmp.ToLower();
	ha1.emplace_back(tmp.m_str);
	tmp.ToUpper();
	ha1.emplace_back(tmp.m_str);

	for (std::vector<std::string>::iterator it = ha1.begin(); it != ha1.end(); ++it)
	{
		MD5 md5;
		md5.Update(*it);
		md5.Update(":");
		char *pos = password.m_str + 3;
		char *pos2 = strchr(pos, '*');
		md5.Update(pos, pos2 - pos);
		pos = pos2 + 1;
		pos2 = strchr(pos, '*');
		md5.Update(":");
		md5.Update(pos, pos2 - pos);
		md5.Final();
		pos = pos2 + 1; // points to response now

		char correct_response[33];
		md5.GetString(correct_response);

		if (strcasecmp(correct_response, pos) == 0)
			return true;
	}

	return false;
}

bool VS_SimpleStorage::TryH323Login(const VS_TempUserData &user, const VS_SimpleStr &password)
{
	// Check format ($5*...)
	// If password does not starts with "$5*" string.
	if (strstr(password.m_str, "$5*") != password) return false;

	// Skip "$5*"
	const char* pass = password.m_str + 3;
	const char* pass_end = pass + strlen(pass); // position of '\0' sumbol.

	// For each token [alias_size*alias*timestamp1*md5_token]
	char buff[1024] = { 0 };
	while (*pass)
	{
		// Parameters we need to extract.
		unsigned int alias_sz;
		VS_SimpleStr alias("");
		unsigned int timestamp;
		VS_SimpleStr md5("");

		// First bracket.
		if (*pass++ != '[') return false;
		// Alias size.
		const char* pos = strchr(pass, '*');
		if (pos == 0 || pos == pass) return false;
		assert(sizeof(buff) >= pos - pass);
		memcpy(buff, pass, pos - pass);
		buff[pos - pass] = '\0';
		// Alias.
		pass = pos + 1; // step over alias_size and '*' symbol
		alias_sz = atoi(buff);
		if (alias_sz <= 0) return false;
		if (pass + alias_sz >= pass_end) return false;
		assert(sizeof(buff) >= alias_sz);
		memcpy(buff, pass, alias_sz);
		buff[alias_sz] = '\0';
		alias = buff;
		pass += alias_sz + 1;
		// Timestamp.
		pos = strchr(pass, '*');
		if (pos == 0 || pos == pass) return false;
		assert(sizeof(buff) >= pos - pass);
		memcpy(buff, pass, pos - pass);
		buff[pos - pass] = '\0';
		timestamp = atoi(buff);
		pass = pos + 1; // step over timestamp and '*' symbol.
		// MD5
		pos = strchr(pass, ']');
		if (pos == 0 || pos == pass) return false;
		assert(sizeof(buff) >= pos - pass);
		memcpy(buff, pass, pos - pass);
		buff[pos - pass] = '\0';
		md5 = buff;
		pass = pos; // step over md5.
		// Last bracket.
		if (*pass++ != ']') return false;

		// Verify authorization.
		auto md5_to_compare = VS_H225RASParser::MakeEncryptedToken_MD5_String(string_view{ alias.m_str, (size_t)alias.Length() }, string_view{ user.m_h323_password.m_str, (size_t)user.m_h323_password.Length() }, timestamp);
		if (string_view{ md5.m_str, (size_t)md5.Length() } == md5_to_compare) return true;
	}
	return false;
}

bool VS_SimpleStorage::TryTerminalLogin(const VS_TempUserData &user, const char *user_id, const VS_SimpleStr &password)
{
	return TryDigestLogin(user, password) || TryH323Login(user, password) || CheckGuestUserPassword(password, user_id);
}

bool VS_SimpleStorage::FindUser_Sink(const vs_user_id& user_id, VS_StorageUserData& ude, bool)
{
	return FindUser(user_id.m_str, ude);
}

bool VS_SimpleStorage::IsLDAP_Sink() const
{
	return IsLDAP();
}

std::string VS_SimpleStorage::GetDefauldEditableFields()
{
	return std::string();
}

size_t VS_SimpleStorage::GetOfflineMessagesChunkSize(void) const
{
	return m_chunk_size;
}

void VS_SimpleStorage::SetOfflineMessagesChunkSize(const size_t chunk_size)
{
	m_chunk_size = chunk_size;
}

int VS_SimpleStorage::FindUserPictureImpl(const char* path, int& entries, const std::string& query, long clientHash, VS_Container& cnt)
{
	assert(path);

	if (query.empty())
		return SEARCH_FAILED;

	std::string q = query;
	std::transform(q.begin(), q.end(), q.begin(), ::tolower);

	VS_RealUserLogin r(q);
	if (!r.IsOurSID())
		return SEARCH_FAILED;

	auto&& callId_escaped = r.GetUser();

	auto escaped = EscapeCallId(callId_escaped);
	if(!escaped.empty())
	{
		callId_escaped = std::move(escaped);
	}

	std::string file_name(path);
	file_name.append(callId_escaped);

	const char* mime_type(nullptr);
	std::ifstream file;

	auto server_hash(DEFAULT_VALID_HASH_FOR_NO_PICTURE);

	for (auto it = m_picture_mime_types.cbegin(); it != m_picture_mime_types.cend(); ++it)
	{
		file_name += it->first;
		file.open(file_name, std::ios::in | std::ios::binary | std::ios::ate);

		mime_type = it->second.c_str();
		if (file.is_open())
		{
			boost::system::error_code ec;
			std::time_t t = boost::filesystem::last_write_time(file_name, ec);
			if (!ec) server_hash = VS_MakeHash(std::chrono::system_clock::from_time_t(t));
			break;
		}

		file_name.erase(file_name.cend() - it->first.length(), file_name.cend());
		file.close();
	}

	if (VS_CompareHash(server_hash, clientHash))
	{
		entries = -1;
		cnt.AddValue(USERNAME_PARAM, query);
		cnt.AddValueI32(HASH_PARAM, server_hash);
		return SEARCH_NOT_MODIFIED;
	}

	if (!file.is_open())
	{
		entries = -1;
		cnt.AddValue(USERNAME_PARAM, query);
		cnt.AddValueI32(HASH_PARAM, server_hash);
		return SEARCH_DONE;
	}

	file.seekg(0, std::ios::end);
	const auto size = file.tellg();

	if (size >= MAX_AVATAR_SIZE)
	{
		dstream1 << "file ( " << file_name << " ) more or equal " << MAX_AVATAR_SIZE;
		return SEARCH_FAILED;
	}

	std::unique_ptr<char[]> memblock = vs::make_unique<char[]>(size);
	file.seekg(0, std::ios::beg);
	file.read(memblock.get(), size);

	cnt.AddValue(USERNAME_PARAM, query);		// was VS_WideStr
	cnt.AddValueI32(HASH_PARAM, server_hash);
	cnt.AddValue("picture", memblock.get(), size);
	cnt.AddValue("picturetype", mime_type);
	entries++;

	return SEARCH_DONE;
}

bool VS_SimpleStorage::CheckDigestByRegistry(const VS_SimpleStr& login, const VS_SimpleStr& password)
{
	if (!login || !password)
		return false;
	VS_SimpleStr key_name = USERS_KEY; key_name += "\\"; key_name += login;
	VS_RegistryKey key(false, key_name.m_str);
	if (!key.IsValid())
		return false;
	char ha1[33] = { 0 };
	if (key.GetValue(ha1, 33, VS_REG_STRING_VT, HA1_PASSWORD_TAG) <= 0)
		return false;
	VS_TempUserData user;
	user.m_HA1_password = ha1;
	return TryDigestLogin(user, password);
}

bool VS_SimpleStorage::GetGuestParams(const char* guest_login, VS_SimpleStr& fixed_login, VS_SimpleStr& displayName_utf8)
{
	if (!guest_login || !*guest_login)
		return false;
	bool guest_random = !strncasecmp(guest_login, VS_SimpleStorage::GUEST_PREFIX, strlen(VS_SimpleStorage::GUEST_PREFIX));
	bool guest_fixed = !strncasecmp(guest_login, VS_SimpleStorage::GUEST_PREFIX_FIXED_CALLID, strlen(VS_SimpleStorage::GUEST_PREFIX_FIXED_CALLID));
	static const char* DN_DEPARATOR = "*";
	bool res(false);
	if (guest_random) {
		unsigned long pos_dn = strlen(GUEST_PREFIX);
		if (pos_dn)
			displayName_utf8 = (guest_login + pos_dn);
		res = true;
	}
	else if (guest_fixed) {
		unsigned long pos_login = strlen(GUEST_PREFIX_FIXED_CALLID);
		if (pos_login)
		{
			const char* login_start = guest_login + pos_login;
			const char* login_end = strstr(login_start, DN_DEPARATOR);
			if (login_start && !login_end)
				login_end = login_start + strlen(login_start);
			const char* dn_ptr = 0;
			if (login_start && login_end && (login_end>login_start))
			{
				fixed_login.Append(login_start, (int)(login_end - login_start));
				dn_ptr = login_end + strlen(DN_DEPARATOR);
				if (dn_ptr && *dn_ptr)
				{
					displayName_utf8 = dn_ptr;
				}
				res = true;
			}
		}
	}
	return res;
}

std::string VS_SimpleStorage::EscapeCallId(string_view callId)
{
	std::unique_ptr<CURL, CURL_deleter> curl(::curl_easy_init());

	if (!curl)
		return {};

	std::unique_ptr<char, curl_free_deleter> escaped(::curl_easy_escape(curl.get(), callId.data(), callId.length()));
	if (escaped)
		return escaped.get();

	return {};
}

bool VS_SimpleStorage::IsOperator(const vs_user_id& user)
{
	if (!user || !m_group_manager)
		return false;
	if (VS_RealUserLogin::IsGuest(SimpleStrToStringView(user)))
		return false;

	VS_StorageUserData ude;
	if (!FindUser(user.m_str, ude))		// to get group_ids of user, like ude.m_groups["0002"]
		return false;

	std::map<std::string, VS_RegGroupInfo> reg_groups;
	m_group_manager->GetRegGroups(reg_groups);

	if (!ude.m_groups.empty()) {
		for (const auto& g : ude.m_groups)
		{
			auto it = reg_groups.find(g);
			if (it != reg_groups.end() && it->second.IsOperators)
				return true;
		}
	} else {
		auto it = reg_groups.find("@no_group");
		if (it != reg_groups.end() && it->second.IsOperators)
			return true;
	}
	return false;
}

void VS_SimpleStorage::GetSystemGroups(const char* owner, VS_Container& rCnt, int& entries)
{
	if (!m_group_manager || !owner)
		return ;

	std::map<std::string, VS_RegGroupInfo> process_reg_groups;
	std::map<std::string, VS_RegGroupInfo> reg_groups;
	m_group_manager->GetRegGroups(reg_groups);

	auto fill_process_groups = [&process_reg_groups, &reg_groups](const VS_RegGroupInfo& info){
		if (info.scope == e_ab_scope_all_users) {
			process_reg_groups.clear();
			process_reg_groups = reg_groups;
		}
		else if (info.scope == e_ab_scope_groups) {
			for (const auto& group_gid : info.groups)
			{
				auto it = reg_groups.find(group_gid);
				if (it != reg_groups.end())
					process_reg_groups.insert(*it);
			}
		}
		else if (info.scope == e_ab_scope_nobody) {

		}
	};
	VS_RealUserLogin r(owner);
	bool IsNoGroup(true);
	for (const auto& g : reg_groups)
	{
		auto m = std::make_shared<VS_AbCommonMap>();
		GetRegGroupUsers(g.first, m);
		if (m->find((const char*)r) == m->end())
			continue;
		IsNoGroup = false;
		fill_process_groups(g.second);
	}
	if (IsNoGroup)
	{
		auto it = reg_groups.find("@no_group");
		if (it != reg_groups.end())
			fill_process_groups(it->second);
	}

	process_reg_groups.erase("@no_group");
	for (const auto& g : process_reg_groups)
	{
		rCnt.AddValue(GID_PARAM, g.first);
		rCnt.AddValue(GNAME_PARAM, g.second.group_name);
		rCnt.AddValueI32(GTYPE_PARAM, eGroupType::SYSTEM_GROUP);

		for (const auto& c : g.second.contacts)
			rCnt.AddValue(CALLID_PARAM, c.first);

		auto m = std::make_shared<VS_AbCommonMap>();
		GetRegGroupUsers(g.first, m);
		for (const auto& u : *m)
			if (g.second.contacts.find(u.first) == g.second.contacts.end())		// exclude duplicate with contacts
				rCnt.AddValue(CALLID_PARAM, u.first);

		++entries;
	}
}

void VS_SimpleStorage::CheckLic_TerminalPro(bool &IsLoginAllowed, bool &IsTerminalPro)
{
	IsLoginAllowed = true;
	IsTerminalPro = true;
	if (!m_license_checker(LE_TERMINAL_LOGIN))
	{
		IsTerminalPro = false;
		VS_RegistryKey key(false, CONFIGURATION_KEY);
		unsigned long deny_free_terminals(0);
		key.GetValue(&deny_free_terminals, sizeof(unsigned long), VS_REG_INTEGER_VT, "deny_free_terminals");
		if (deny_free_terminals != 0)
			IsLoginAllowed = false;
	}
}

void VS_SimpleStorage::ResetLicenseChecker(LicenseCheckFunctionT && f)
{
	m_license_checker = f;
}

long VS_SimpleStorage::CalcABGroupsHash(const std::string& owner)
{
	VS_Container cnt;
	int entries(0);
	GetABGroupsOfUser(owner, cnt, entries);
	auto ptr = cnt.GetLongValueRef(HASH_PARAM);
	return ptr ? *ptr : 0;
}

void VS_SimpleStorage::GetABGroupsOfUser(const std::string& owner, VS_Container& cnt, int& entries)
{
	GetSystemGroups(owner.c_str(), cnt, entries);
	GetCustomGroups(owner.c_str(), cnt, entries);
	auto server_hash = CalcHashOnMemory(cnt);
	cnt.AddValueI32(HASH_PARAM, server_hash);
}

void VS_SimpleStorage::GetCustomGroups(const char* owner, VS_Container& cnt, int& entries)
{
	if (!owner)
		return ;
	auto user_id = VS_GetUserPartEscaped(owner);
	if (user_id.empty())
		return ;
	std::string key_name = USERS_KEY;
	key_name += "\\";
	key_name += user_id;
	key_name += "\\";
	key_name += CUSTOM_GROUPS_KEY;
	VS_RegistryKey key_root(false, key_name);
	if (!key_root.IsValid())
		return ;
	VS_RegistryKey key;
	key_root.ResetKey();
	while (key_root.NextKey(key))
	{
		VS_SimpleStr gid = key.GetName();
		if (!!gid)
		{
			++entries;
			cnt.AddValue(GID_PARAM, gid);
			std::string group_name;
			if (key.GetString(group_name, GROUP_NAME_TAG))
				cnt.AddValue(GNAME_PARAM, group_name);
			cnt.AddValueI32(GTYPE_PARAM, eGroupType::CUSTOM_GROUP);
			auto key_name2 = key_name;
			key_name2 += "\\";
			key_name2 += gid;
			key_name2 += "\\";
			key_name2 += "Users";
			VS_RegistryKey users_root(false, key_name2);
			if (users_root.IsValid())
			{
				VS_RegistryKey u;
				users_root.ResetValues();
				std::string valueName;
				std::string data;
				while (users_root.NextString(data, valueName) && !valueName.empty()) {
					if (VS_IsNotTrueConfCallID(valueName))
						cnt.AddValue(CALLID_PARAM, valueName);
					else {
						VS_RealUserLogin r2(valueName);
						cnt.AddValue(CALLID_PARAM, (const char*)r2);
					}
				}
			}
		}
	}
}

std::string VS_SimpleStorage::GetUserPropertyPath(const char* call_id) {
	if (!call_id) return "";
	std::string user_prop_key = USERS_KEY;
	user_prop_key += '\\';
	user_prop_key += VS_GetUserPartEscaped(call_id);
	user_prop_key += USER_PROPERTIES_TAG;
	return user_prop_key;
}

bool VS_SimpleStorage::GetUserProperty(const vs_user_id& user_id, const VS_SimpleStr& name, int64_t &value) {
	std::string user_prop_key = GetUserPropertyPath(user_id);
	if (user_prop_key.empty()) return false;

	VS_RegistryKey	u_prop_key(false, user_prop_key, false, true);
	return u_prop_key.GetValue(&value, sizeof(value), VS_REG_INT64_VT, name) != 0;
}

void VS_SimpleStorage::ReadApplicationSettings(VS_StorageUserData &user)
{
	user.m_appSettings.clear();

	std::string app_settings_key_name = USERS_KEY;
	app_settings_key_name += "\\";
	app_settings_key_name += VS_GetUserPartEscaped(user.m_login.m_str);
	app_settings_key_name += '\\';
	app_settings_key_name += APPLICATION_SETTINGS_KEY;
	VS_RegistryKey app_settings_key(false, app_settings_key_name);

	std::unique_ptr<void, free_deleter> buffer;
	RegistryVT type;
	std::string valueName;

	if (app_settings_key.IsValid()) {
		app_settings_key.ResetKey();
		VS_RegistryKey current;
		while (app_settings_key.NextKey(current)) {
			const char *keyName = current.GetName();

			while (current.NextValueAndType(buffer, type, valueName)) {
				switch (type)
				{
				case VS_REG_INTEGER_VT:
					if (valueName == "Value") {
						user.m_appSettings[keyName].integer = *static_cast<const uint32_t*>(buffer.get());
					} else if (valueName == "IsLocked") {
						user.m_appSettings[keyName].IsLocked = *static_cast<const uint32_t*>(buffer.get());
					}
					break;
				default:
					// no support
					break;
				}
			}
		}
	}


	std::map<std::string, VS_RegGroupInfo> reg_groups;
	m_group_manager->GetRegGroups(reg_groups);

	decltype(user.m_groups) g;
	if (user.m_groups.empty())
		g.emplace_back("@no_group");
	else
		g = user.m_groups;
	std::map<std::string, VS_UserData::SettingsValue> mergedAppSettings;
	for (const auto &group_name : g) {
		auto group_info = reg_groups[group_name];

		for (const auto &param : group_info.ApplicationSettings) {
			auto& name = param.first;
			auto& value = param.second;

			VS_UserData::SettingsValue setting;

			if (mergedAppSettings.find(name) != end(mergedAppSettings)) {
				setting.integer = std::min(mergedAppSettings[name].integer, value.integer);
				setting.IsLocked = std::max(mergedAppSettings[name].IsLocked, value.IsLocked);
			} else {
				setting = value;
			}

			mergedAppSettings[name] = setting;
		}
	}

	for (const auto &param : user.m_appSettings) {
		auto& name = param.first;
		auto& value = param.second;

		mergedAppSettings[name] = value;
	}

	user.m_appSettings = mergedAppSettings;
}

unsigned long VS_SimpleStorage::GetConfStartTimeDiffOrDefault(const char* conf_id) const
{
	uint64_t time_diff = 0;
	if (!conf_id || !*conf_id)
		return 0;
	const auto l = m_conf_started.lock();
	const auto it = l->find(conf_id);
	if (it != l->end())
		time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - it->second).count();
	return (unsigned long) time_diff;
}

void VS_SimpleStorage::ResetDBCallLog(std::unique_ptr<callLog::Postgres>&& new_calllog)
{
	m_dbCallLog = std::move(new_calllog);
}

callLog::Postgres * VS_SimpleStorage::GetCallLog() const
{
	return m_dbCallLog.get();
}

vs_conf_id VS_SimpleStorage::NewConfID()
{
	return m_dbCallLog->CreateNewConfID();
}
