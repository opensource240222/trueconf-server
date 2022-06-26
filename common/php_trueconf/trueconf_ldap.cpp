#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "common.h"

#include "php.h"
#include "php_ini.h"
#include "ext/standard/php_string.h"
#include "ext/standard/info.h"

#include "trueconf_ldap.h"
#include "vs_zend_parse_parameters.h"

#include "../ldap_core/VS_LDAPCore.h"
#include "../ldap_core/VS_LDAPFactory.h"
#include "../ldap_core/CfgHelper.h"
#include "../ldap_core/common/VS_ABStorage.h"
#include "../ldap_core/common/VS_RegABStorage.h"
#include "../ldap_core/common/VS_LogABLimit_Web.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "../std/cpplib/VS_RegistryConst.h"
#include "std-generic/cpplib/atomic_shared_ptr.h"
#include "std-generic/cpplib/utf8.h"
#include "std-generic/cpplib/ignore.h"

#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_PHP

vs::atomic_shared_ptr<tc::LDAPCore> g_ldap_core;

void HandleLDAPError(tc::ldap_error_code_t err);

static std::shared_ptr<tc::LDAPCore> get_ldap() noexcept
{
	static std::mutex mtx;

	auto ldap_core = g_ldap_core.load();
	if (ldap_core)
		return ldap_core;

	std::lock_guard<decltype(mtx)> _{ mtx };
	ldap_core = g_ldap_core.load();
	if (ldap_core)
		return ldap_core;

	tc::cfg_params_t params;
	if (!tc::ReadCfg(params)) {
		dstream0 << "trueconf_ldap init: read configuration\n";
		return ldap_core;
	}

	if (!boost::iequals(params[STORAGE_TYPE_KEY_NAME].c_str(), "LDAP")) {
		return ldap_core;		// we are in Registry mode
	}

	tc::eLDAPServerType serv_type;
	if (!VS_LDAPFactory::GetLDAPServerType(params, serv_type)) {
		dstream0 << "trueconf_ldap init: can't get ldap server\n";
		return ldap_core;
	}

	ldap_core = VS_LDAPFactory::CreateInstance(serv_type, params, {});
	if (!ldap_core || !ldap_core->SetGroupManager(std::make_shared<VS_GroupManager>(true)))
	{
		ldap_core = {};
		return ldap_core;
	}
	g_ldap_core.store(ldap_core);

	ldap_core->SetLDAPErrorHandler(&HandleLDAPError);
	ldap_core->Connect(false);
	ldap_core->StartUpdateNestedCache();

	return ldap_core;
}

void HandleLDAPError(tc::ldap_error_code_t err)
{
	if (tc::LDAPCore::IsErrorReconnect(err))
	{
		auto ldap_core = get_ldap();
		if (ldap_core)
			ldap_core->Connect(true);
	}
}

#ifdef COMPILE_DL_TRUECONF_LDAP
ZEND_GET_MODULE(trueconf_ldap)
#endif


// copy custom_attrs from zval to multimap (zval was given by php as input params)
bool GetCustomAttrs(zval *arr, std::vector<std::string>& vector){
	if (Z_TYPE_P(arr) != IS_ARRAY) {
		return false;
	}

	if (zend_hash_num_elements(Z_ARRVAL_P(arr)) == 0) {
		return true;
	}

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(arr));
	zval *entry;
	while ((entry = zend_hash_get_current_data(Z_ARRVAL_P(arr))) != NULL) {
		zend_hash_move_forward(Z_ARRVAL_P(arr));

		if (Z_TYPE_P(entry) != IS_STRING){
			continue;
		}

		convert_to_string_ex(entry);
		vector.emplace_back(Z_STRVAL_P(entry), Z_STRLEN_P(entry));
	}
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(arr));
	return true;
}


void SetCustomAttrs(const std::vector<tc::attrs_t>& entries, zval* output, bool hideDistinguishedName)
{
	auto ldap_core = get_ldap();

	for (const auto& entry : entries){
		std::map<std::string, std::vector<std::string>> attrsList;
		std::string distinguishedName;

		for (const auto& entryData : entry){
			if (entryData.first == ldap_core->m_a_distinguishedName){
				distinguishedName = entryData.second;
				if (hideDistinguishedName){
					continue;
				}
			}

			auto it = attrsList.find(entryData.first);
			if (it == attrsList.end()){
				std::vector<std::string> valuesList;
				valuesList.push_back(entryData.second);
				attrsList[entryData.first] = valuesList;
			}
			else {
				it->second.push_back(entryData.second);
			}
		}

		if (distinguishedName.length() == 0){
			continue;
		}

		zval zEntry;
		array_init(&zEntry);

		for (const auto& attribute : attrsList){
			char* unicodeKey = const_cast<char*>(attribute.first.c_str());

			if (attribute.second.size() == 1){
				std::string value = attribute.second.front();
				add_assoc_string(&zEntry, unicodeKey, const_cast<char*>(value.c_str()));
			}
			else {
				zval zValueList;
				array_init(&zValueList);

				for (const auto& value : attribute.second){
					add_next_index_string(&zValueList, const_cast<char*>(value.c_str()));
				}

				add_assoc_zval(&zEntry, unicodeKey, &zValueList);
			}
		}

		add_assoc_zval(output, const_cast<char*>(distinguishedName.c_str()), &zEntry);
	}
}

void SetCustomAttrs(const std::vector<tc::attrs_t>& entries, zval* z){
	return SetCustomAttrs(entries, z, false);
}

// copy custom_attrs from multimap to zval
void SetCustomAttrs(const std::multimap<tc::attr_name_t, tc::attr_value_t>& custom_attrs, zval* z)
{
	zval z2;
	array_init(&z2);
	std::string key;
	for (std::multimap<tc::attr_name_t, tc::attr_value_t>::const_iterator it = custom_attrs.begin();
		it != custom_attrs.end();
		++it)
	{
		std::string new_key = it->first;
		std::string val = it->second;
		if (key.empty())
			key = new_key;
		if (strcasecmp(key.c_str(), new_key.c_str()) != 0)
		{
			add_assoc_zval(z, key.c_str(), &z2);
			key = new_key;

			array_init(&z2);
		}
		add_next_index_string(&z2, (char*)val.c_str());
	}
	if (custom_attrs.size())
		add_assoc_zval(z, key.c_str(), &z2);		// last value
}

void AddLong(const std::string& name, const long value, zval* z){
	add_assoc_long(z, const_cast<char*>(name.c_str()), value);
}

void AddString(const std::string& name, const std::string& value, zval* z)
{
	add_assoc_stringl(z, const_cast<char*>(name.c_str()), const_cast<char*>(value.c_str()), value.length());
}

void AddUserInfo(const tc::ldap_user_info& u, zval* z)
{
	auto ldap_core = g_ldap_core.load();

	AddString(ldap_core->m_a_distinguishedName, u.dn, z);
	AddString(ldap_core->m_a_displayname, u.displayName, z);
	AddString(ldap_core->m_a_login, u.login, z);
	SetCustomAttrs(u.custom_attrs, z);
}

void AddUserGroups(zval* z, const std::vector<std::string>& groups, const std::map<std::string, VS_RegGroupInfo>& reg_groups)
{
	zval z2;
	array_init(&z2);
	for (const auto& g : groups)
	{
		auto it = reg_groups.find(g);
		if (it != reg_groups.end())
			AddString(g, it->second.group_name, &z2);
		else
			AddString(g, "", &z2);
	}
	add_assoc_zval(z, "Groups", &z2);

}

void AddUsersInfo(zval* z, const std::vector<tc::ldap_user_info>& group_users)
{
	auto ldap_core = get_ldap();
	std::map<std::string, VS_RegGroupInfo> reg_groups;
	ldap_core->GetRegGroups(reg_groups);

	for (const auto& u : group_users)
	{
		zval z2;
		array_init(&z2);
		AddUserInfo(u, &z2);
		std::string user_utf8 = u.login;
		VS_StorageUserData ude;
		// add groups for CustomContacts
		for (const auto& r : reg_groups)
		{
			if (r.second.contacts.find(user_utf8) != r.second.contacts.end())
			{
				if (std::find(ude.m_groups.begin(), ude.m_groups.end(), r.first.c_str()) == ude.m_groups.end())
				{
					ude.m_groups.push_back(r.first);
				}
			}
		}
		if (ldap_core->FindUser_Sink(user_utf8.c_str(), ude) && !ude.m_groups.empty())
			AddUserGroups(&z2, ude.m_groups, reg_groups);
		add_next_index_zval(z, &z2);
	}
}

void AddStorageUserData(zval* z, const VS_StorageUserData& ude, const std::map<std::string, VS_RegGroupInfo>& reg_groups)
{
	if (ude.m_realLogin.IsOurSID())
		add_assoc_string(z, "login", (char*)ude.m_realLogin.GetUser().c_str());
	else
		add_assoc_string(z, "login", (char*)(const char*)ude.m_realLogin);
	AddString("displayName", ude.m_displayName, z);
	AddUserGroups(z, ude.m_groups, reg_groups);
}


void AddUsersFromMap(zval* z, const VS_AbCommonMap& users, const std::map<std::string, VS_RegGroupInfo>& reg_groups)
{
	auto ldap_core = get_ldap();
	for (const auto& u : users)
	{
		zval z2;
		array_init(&z2);
		VS_StorageUserData ude;

		bool isRealUser = true;
		if (!ldap_core->FindUser_Sink(u.first.c_str(), ude, true))
		{
			isRealUser = false;
			ude.m_realLogin = VS_RealUserLogin(u.first);
			ude.m_displayName = u.second.displayName;
		}

		// bug#57363: custom displayName should have more priority, than from ldap
		if (!u.second.displayName.empty() && (u.second.IsCustomContactOfGroup || u.second.IsCustomContactOfUser))
		{
			ude.m_displayName = u.second.displayName;
		}

		// add groups for CustomContacts
		for (const auto& r : reg_groups)
		{
			if (r.second.contacts.find(u.first) != r.second.contacts.end())
			{
				if (std::find(ude.m_groups.begin(), ude.m_groups.end(), r.first.c_str()) == ude.m_groups.end())
				{
					ude.m_groups.push_back(r.first);
				}
			}
		}

		AddStorageUserData(&z2, ude, reg_groups);
		add_assoc_bool(&z2, "IsCustomContactOfGroup", u.second.IsCustomContactOfGroup);
		add_assoc_bool(&z2, "IsCustomContactOfUser", u.second.IsCustomContactOfUser);
		add_assoc_bool(&z2, "IsRealUser", isRealUser);

		add_next_index_zval(z, &z2);
	}
}

void AddAddressBookLimits(zval * z, const std::vector<VS_LogABLimit_Web::WebABLimit>& limits){
	for (const VS_LogABLimit_Web::WebABLimit& limit : limits)
	{
		zval z2;
		array_init(&z2);

		add_assoc_long(&z2, "type", limit.type);

		AddString("owner", limit.owner, &z2);

		add_assoc_long(&z2, "current_size", limit.before_size);
		add_assoc_long(&z2, "invalid_group_size", limit.add_items);
		add_assoc_long(&z2, "max_size", limit.limit_value);

		AddString("parent_group_id", limit.gid_from_group, &z2);
		AddString("invalid_group_id", limit.gid_add_group, &z2);

		add_next_index_zval (z, &z2);
	}
}

/* {{{ proto resource trueconf_ldap_init
todo(kt): test func from ktrushnikov */
PHP_FUNCTION(trueconf_ldap_init)
{
	tc::cfg_params_t params;
	if (!tc::ReadCfg(params) || !boost::iequals(params[STORAGE_TYPE_KEY_NAME].c_str(), "LDAP")) {
		dstream0 << "trueconf_ldap_init: not ldap mode\n";
		RETURN_FALSE;
	}

	auto ldap_core = g_ldap_core.load();

	unsigned long new_hash = params.CaclHash();
	if (ldap_core && new_hash && (new_hash == ldap_core->m_params_hash.get())) {
		RETURN_TRUE;
	}
	tc::eLDAPServerType serv_type;
	if (!VS_LDAPFactory::GetLDAPServerType(params, serv_type)) RETURN_FALSE;

	auto new_core = VS_LDAPFactory::CreateInstance(serv_type, params, {});
	if (!new_core || !new_core->SetGroupManager(std::make_shared<VS_GroupManager>(true)))
		RETURN_FALSE;

	g_ldap_core.store(new_core);

	new_core->Connect(false);
	new_core->StartUpdateNestedCache();
	RETURN_TRUE;

}

PHP_FUNCTION(trueconf_ldap_deinit){
	auto ldap_core = get_ldap();
	if (!ldap_core){
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "ldap_core does not exist!");
		RETURN_FALSE;
	}

	// clear caches
	ldap_core->ClearCaches();
	g_ldap_core.store(nullptr);
	dstream2 << "trueconf_ldap_deinit(): done\n";
	RETURN_TRUE
}

/* {{{ proto resource trueconf_ldap_connect
todo(kt): test func from ktrushnikov */
PHP_FUNCTION(trueconf_ldap_connect)
{
	auto ldap_core = get_ldap();
	if (!ldap_core){
		dprint0("ldap_core does not exist!");
		RETURN_FALSE;
	}
	int res = ldap_core->Connect();
	if (res)
	{
		dprint1("tc::LDAPCore::Connect() failed\n");
		RETURN_FALSE
	}

	dstream2 << "trueconf_ldap_connect(): done to " << ldap_core->m_ldap_server << ":" << ldap_core->m_ldap_port;
	RETURN_TRUE
}

/* {{{ proto resource trueconf_ldap_update_nested_groups_cache
todo(kt): test func from ktrushnikov */
PHP_FUNCTION(trueconf_ldap_start_cache_update)
{
	auto ldap_core = get_ldap();
	if (!ldap_core){
		dprint0("ldap_core does not exist!");
		RETURN_FALSE;
	}
	ldap_core->StartUpdateNestedCache();

	dstream2 << "trueconf_ldap_start_cache_update(): done\n";
	RETURN_TRUE
}

/* {{{ proto resource trueconf_ldap_get_all_groups
todo(kt): test func from ktrushnikov */
PHP_FUNCTION(trueconf_ldap_get_all_groups)
{
	zval* zcookie = nullptr;
	zend_long page_size = 0;
	_char(query_str);
	_char(z_sort_attr);
	zend_bool z_sort_order = 0;
	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zlssb", &zcookie, &page_size, &query_str, &query_str_len, &z_sort_attr, &z_sort_attr_len, &z_sort_order) != SUCCESS) {
		dstream0 << "trueconf_ldap_get_all_groups(): failed to parse args!";
		RETURN_FALSE;
	}
	tc::page_cookie_t cookie;
	if (zcookie)
	{
		if (Z_TYPE_P(Z_REFVAL_P(zcookie)) == IS_LONG)
		{
			cookie = Z_LVAL_P(Z_REFVAL_P(zcookie));
		}
		else
		{
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid cookie type");
			RETURN_FALSE;
		}
	}

	std::string query{ query_str, query_str_len };

	auto sort_pair = (z_sort_attr && z_sort_attr_len) ?
		std::make_unique<std::pair<std::string, bool>>(std::string(z_sort_attr, z_sort_attr_len), z_sort_order ? true : false)
		: nullptr;

	auto ldap_core = get_ldap();
	if (!ldap_core) {
		dstream0 << "trueconf_ldap_get_all_groups(): ldap core does not exist!";
		RETURN_FALSE;
	}

	tc::ldap_error_code_t err(LDAP_SUCCESS);
	std::vector<tc::ldap_group_info> all_groups;

	err = ldap_core->GetAllGroups(all_groups, cookie, page_size, sort_pair.get(), &query);

	if (err != LDAP_SUCCESS){
		RETURN_LONG(err);
	}

	if (zcookie)
	{
		if (Z_TYPE_P(Z_REFVAL_P(zcookie)) == IS_LONG) {
			Z_LVAL_P(Z_REFVAL_P(zcookie)) = (long)cookie;
		}
	}

	array_init(return_value);
	for (std::vector<tc::ldap_group_info>::const_iterator it = all_groups.begin(); it != all_groups.end(); ++it)
	{
		zval z;
		array_init(&z);
		AddString(ldap_core->m_a_distinguishedName, it->dn, &z);
		AddString(ldap_core->m_a_GroupDisplayName, it->displayName, &z);
		if (it->primaryGroupToken != 0)
			AddLong(ldap_core->m_a_primaryGroupToken, it->primaryGroupToken, &z);

		add_next_index_zval(return_value, &z);
	}

	dstream2 << "trueconf_ldap_get_all_groups(): done. Fetched " << all_groups.size() << " groups\n";
}

PHP_FUNCTION(trueconf_ldap_get_last_cache_update)
{
	auto ldap_core = get_ldap();
	if (!ldap_core){
		dprint0( "trueconf_ldap_get_last_cache_update(): ldap_core does not exist!");
		RETURN_FALSE;
	}

	auto lastUpdate = ldap_core->GetLastUpdateNestedCacheTime();
	double t = (std::chrono::duration<double>(lastUpdate.time_since_epoch())).count();

	char buff[512] = {};
	auto strSize = tu::TimeToGStr(lastUpdate, buff, 512);
	if(strSize > 0)
		dstream2 << "trueconf_ldap_get_last_cache_update(): done. Last cache update=" << string_view(buff, strSize);

	RETURN_DOUBLE(t);
}

/* {{{ proto resource trueconf_ldap_get_group_users
todo(kt): test func from ktrushnikov */
PHP_FUNCTION(trueconf_ldap_get_group_users)
{
	zval* arr = nullptr;
	_char(gdn);
	zval* zcookie = nullptr;
	zend_long page_size = 0;
	_char(query_str);
	_char(z_sort_attr);
	zend_bool z_sort_order = 0;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|zlassb", &gdn, &gdn_len, &zcookie, &page_size, &arr, &query_str, &query_str_len, &z_sort_attr, &z_sort_attr_len, &z_sort_order) != SUCCESS) {
		dstream0 << "trueconf_ldap_get_group_users(): failed to parse arguments\n";
		return;
	}

	if (!*gdn){
		dprint0("trueconf_ldap_get_group_users(): Invalid group distinguishedName!\n");
		RETURN_FALSE;
	}

	tc::page_cookie_t cookie;
	if (zcookie)
	{
		if (Z_TYPE_P(Z_REFVAL_P(zcookie)) == IS_LONG){
			cookie = Z_LVAL_P(Z_REFVAL_P(zcookie));
		}
		else
		{
			dprint0("trueconf_ldap_get_group_users():Invalid cookie type\n");
			RETURN_FALSE;
		}
	}

	// get array of input custom attributes
	std::vector<tc::attr_name_t> custom_attrs;
	if (arr)
	{
		if (!GetCustomAttrs(arr, custom_attrs))
		{
			dprint0("trueconf_ldap_get_group_users():Array initialization wrong\n");
			RETURN_FALSE;
		}
	}

	std::string query{ query_str, query_str_len };

	auto sort_pair = (z_sort_attr && z_sort_attr_len) ?
		std::make_unique<std::pair<std::string, bool>>(std::string(z_sort_attr, z_sort_attr_len), z_sort_order ? true : false)
		: nullptr;

	tc::ldap_error_code_t err(LDAP_SUCCESS);

	auto ldap_core = get_ldap();
	if (!ldap_core){
		dprint0("trueconf_ldap_get_group_users():ldap_core does not exist!\n");
		RETURN_FALSE;
	}
	std::vector<tc::ldap_user_info> group_users;
	if (strcasecmp(gdn, "@no_group") != 0){
		err = ldap_core->GetGroupUsers(gdn, group_users, cookie, page_size, &custom_attrs, sort_pair.get(), &query);
	}
	else {
		err = ldap_core->GetNoGroupUsers(group_users, cookie, page_size, &custom_attrs, sort_pair.get(), &query);
	}

	if (err != LDAP_SUCCESS) {
		dstream0 << "trueconf_ldap_get_group_users(): failed, err=" << err;
		RETURN_LONG(err);
	}

	if (zcookie)
	{
		if (Z_TYPE_P(Z_REFVAL_P(zcookie)) == IS_LONG) {
			Z_LVAL_P(Z_REFVAL_P(zcookie)) = (long)cookie;
		}
	}

	array_init(return_value);
	AddUsersInfo(return_value, group_users);
	dstream2 << "trueconf_ldap_get_group_users(): done. Fetched " << group_users.size() << " users\n";
}

PHP_FUNCTION(trueconf_ldap_get_address_book)
{
	_char(owner);
	zval * abLimits(0);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &owner, &owner_len, &abLimits) != SUCCESS) {
		dstream0 << "trueconf_ldap_get_address_book(): failed to parse args\n";
		return;
	}

	if (!owner || !*owner){
		dprint0("Invalid call_id!\n");
		RETURN_FALSE;
	}

	std::string owner_low = vs::UTF8ToLower(string_view(owner, owner_len));
	auto ldap_core = get_ldap();
	if (!ldap_core){
		dprint0( "trueconf_ldap_get_address_book(): ldap_core does not exist!");
		RETURN_FALSE;
	}
	VS_AbCommonMap m;
	long server_hash(0);
	VS_LogABLimit_Web limits;
	VS_RegABStorage reg_ab;
	reg_ab.SetSink(ldap_core.get(), ldap_core.get());
	if (!reg_ab.GetABForUser(owner_low.c_str(), m, server_hash, &limits))
	{
		dprint0("trueconf_ldap_get_address_book(): Cache not ready for get_address_book");
		RETURN_FALSE;
	}

	std::map<std::string, VS_RegGroupInfo> reg_groups;
	ldap_core->GetRegGroups(reg_groups);
	dstream2<< "trueconf_ldap_get_address_book(): fetched " << reg_groups.size() << " groups\n";

	array_init(return_value);
	AddUsersFromMap(return_value, m, reg_groups);

	std::vector<VS_LogABLimit_Web::WebABLimit> errors = limits.GetLimits();

	if (errors.empty() || abLimits == 0){
		dstream2 << "trueconf_ldap_get_address_book(): fail to get limits\n";
		return;
	}

	zval_dtor(abLimits);
	array_init(abLimits);

	AddAddressBookLimits(abLimits, errors);
}

PHP_FUNCTION(trueconf_ldap_get_address_book_of_group)
{
	_char(gid);
	zval* abLimits(0);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &gid, &gid_len, &abLimits) != SUCCESS) {
		dstream0 << "trueconf_ldap_get_address_book_of_group(): failed to parse args\n";
		return;
	}

	if (!gid || !*gid){
		dprint0("trueconf_ldap_get_address_book_of_group(): Invalid Group GID!");
		RETURN_FALSE;
	}

	auto ldap_core = get_ldap();
	if (!ldap_core){
		dprint0("trueconf_ldap_get_address_book_of_group(): ldap_core does not exist!");
		RETURN_FALSE;
	}
	std::map<std::string, VS_RegGroupInfo> reg_groups;
	ldap_core->GetRegGroups(reg_groups);

	auto it = reg_groups.find(gid);
	if (it == reg_groups.end())
	{
		dprint0("trueconf_ldap_get_address_book_of_group(): Group GID not found at Registry");
		RETURN_FALSE;
	}

	if (!ldap_core->IsCacheReady())
	{
		dprint0("trueconf_ldap_get_address_book_of_group(): Cache not ready to get Group AB");
		RETURN_FALSE;
	}

	VS_LogABLimit_Web limits;
	std::string fake_owner;
	std::shared_ptr<VS_AbCommonMap> tmp_users;
	auto users = boost::make_shared<VS_AbCommonMap>();
	switch (it->second.scope)
	{
	case e_ab_scope_all_users:
		if (!ldap_core->GetAllUsers(tmp_users))
		{
			dprint0("trueconf_ldap_get_address_book_of_group(): Can not get all users for scope=all");
			RETURN_FALSE;
		}
		if (tmp_users)
		{
			if (users->size() + tmp_users->size() <= AB_LIMIT(AB_MAXSIZE_BY_GROUPS))
				users->insert(tmp_users->begin(), tmp_users->end());
			else
				limits.LimitByGroupAllUsers(fake_owner, users->size(), tmp_users->size(), AB_LIMIT(AB_MAXSIZE_BY_GROUPS), it->first);
		}
		tmp_users.reset();
		break;
	case e_ab_scope_groups:
		for (const auto& g : it->second.groups)
		{
			if (ldap_core->GetRegGroupUsers(g, tmp_users) && tmp_users)
			{
				if (users->size() + tmp_users->size() <= AB_LIMIT(AB_MAXSIZE_BY_GROUPS))
					users->insert(tmp_users->begin(), tmp_users->end());
				else
					limits.LimitByGroup(fake_owner, users->size(), tmp_users->size(), AB_LIMIT(AB_MAXSIZE_BY_GROUPS), it->first, g);
			}
			tmp_users.reset();
		}
		break;
	case e_ab_scope_nobody:
		break;
	default:
		dprint0("trueconf_ldap_get_address_book_of_group(): Unknown group scope");
		RETURN_FALSE;
		break;
	}

	if (users->size() + it->second.contacts.size() <= AB_LIMIT(AB_MAXSIZE_BY_GROUPS)) {
		for (const auto& c : it->second.contacts)
		{
			auto& ref = (*users)[c.first];
			ref.displayName = c.second;
			ref.IsCustomContactOfGroup = true;
		}
	} else
		limits.LimitByGroupCustomContacts(fake_owner, users->size(), it->second.contacts.size(), AB_LIMIT(AB_MAXSIZE_BY_GROUPS), it->first);

	if (!limits.IsErrorShown() && users->size() > AB_LIMIT(WARNING_AB_MAXSIZE_BY_GROUPS))
		limits.WarnByGroup(fake_owner, users->size(), AB_LIMIT(WARNING_AB_MAXSIZE_BY_GROUPS));

	array_init(return_value);
	AddUsersFromMap(return_value, *users, reg_groups);

	std::vector<VS_LogABLimit_Web::WebABLimit> errors = limits.GetLimits();

	if (errors.empty() || abLimits == 0){
		return;
	}

	zval_dtor(abLimits);
	array_init(abLimits);

	AddAddressBookLimits(abLimits, errors);
}

/* {{{ proto resource trueconf_ldap_get_all_attributes
todo(kt): test func from ktrushnikov */
PHP_FUNCTION(trueconf_ldap_get_all_attributes)
{
	_char(dn);
	_char(filter);
	zval * supportedAttrs(0);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sa", &dn, &dn_len, &filter, &filter_len, &supportedAttrs) != SUCCESS) {
		dstream0 << "trueconf_ldap_get_all_attributes(): failed to parse arguments\n";
		return;
	}

	if (!dn || !*dn) {
		dstream0 << "trueconf_ldap_get_all_attributes(): DN is empty\n";
		RETURN_FALSE;
	}

	tc::ldap_error_code_t err(LDAP_SUCCESS);
	auto ldap_core = get_ldap();
	if (!ldap_core){
		dprint0("trueconf_ldap_get_all_attributes(): ldap_core does not exist!");
		RETURN_FALSE;
	}
	std::map<tc::attr_name_t, std::vector<tc::attr_value_t>> attrs;

	std::vector<tc::attr_name_t> custom_attrs;
	if (supportedAttrs)
	{
		if (!GetCustomAttrs(supportedAttrs, custom_attrs))
		{
			dprint0("trueconf_ldap_get_all_attributes(): Array initialization wrong");
			RETURN_FALSE;
		}
	}

	if (filter_len != 0){
		err = ldap_core->GetAllAttributes(dn, filter, &custom_attrs, attrs);
	}
	else {
		err = ldap_core->GetAllAttributes(dn, &custom_attrs, attrs);
	}


	if (err != LDAP_SUCCESS) {
		dstream0 << "trueconf_ldap_get_all_attributes(): error=" << err;
		RETURN_LONG(err);
	}

	array_init(return_value);
	for (std::map<tc::attr_name_t, std::vector<tc::attr_value_t>>::const_iterator it = attrs.begin(); it != attrs.end(); ++it)
	{
		std::string key = it->first;

		zval z;
		array_init(&z);
		for (std::vector<tc::attr_value_t>::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			std::string val = *it2;
			add_next_index_string(&z, val.c_str());
		}
		add_assoc_zval(return_value, key.c_str(), &z);
	}

	dstream0 << "trueconf_ldap_get_all_attributes(" << string_view(dn, dn_len) <<"): done. Fetched " << attrs.size() << " attributes\n";
}

static void php_trueconf_ldap_search(INTERNAL_FUNCTION_PARAMETERS, bool searchUsers){
	_char(filter);
	zval *zcookie = nullptr;
	zend_long page_size = 0;
	zval *attrs = nullptr;
	_char(z_sort_attr);
	zend_bool z_sort_order = 0;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|zlasb", &filter, &filter_len, &zcookie, &page_size, &attrs, &z_sort_attr, &z_sort_attr_len, &z_sort_order) != SUCCESS) {
		dstream0 << "php_trueconf_ldap_search(): failed to parse arguments\n";
		return;
	}

	tc::page_cookie_t cookie;
	if (zcookie)
	{
		if (Z_TYPE_P(Z_REFVAL_P(zcookie)) == IS_LONG){
			cookie = Z_LVAL_P(Z_REFVAL_P(zcookie));
		}
		else
		{
			dprint0("php_trueconf_ldap_search(): Invalid cookie type\n");
			RETURN_FALSE;
		}
	}

	if (filter_len == 0)
	{
		dprint0("php_trueconf_ldap_search(): Ldap filter is empty!");
		RETURN_FALSE;
	}
	// get array of input custom attributes
	std::vector<tc::attr_name_t> custom_attrs;
	if (attrs)
	{
		if (!GetCustomAttrs(attrs, custom_attrs))
		{
			dprint0("php_trueconf_ldap_search(): Array initialization wrong");
			RETURN_FALSE;
		}
	}

	auto sort_pair = (z_sort_attr && z_sort_attr_len) ?
		std::make_unique<std::pair<std::string, bool>>(std::string(z_sort_attr, z_sort_attr_len), z_sort_order ? true : false)
		: nullptr;

	tc::ldap_error_code_t err(LDAP_SUCCESS);
	auto ldap_core = get_ldap();
	if (!ldap_core){
		dprint0("php_trueconf_ldap_search(): ldap_core does not exist!");
		RETURN_LONG(LDAP_PARAM_ERROR);
	}
	if (searchUsers){
		std::vector<tc::ldap_user_info> found_users;
		err = ldap_core->SearchForUser(filter, found_users, cookie, page_size, &custom_attrs, sort_pair.get());
		if (err != LDAP_SUCCESS) {
			dstream0 << "php_trueconf_ldap_search(): error=" << err;
			RETURN_LONG(err);
		}

		array_init(return_value);
		AddUsersInfo(return_value, found_users);
	}
	else {
		std::vector<tc::attrs_t> out;

		bool removeDistinguishedName = std::find(custom_attrs.begin(), custom_attrs.end(), ldap_core->m_a_distinguishedName) == custom_attrs.end();
		if (!removeDistinguishedName){
			custom_attrs.erase(std::remove(custom_attrs.begin(), custom_attrs.end(), ldap_core->m_a_distinguishedName), custom_attrs.end());
		}

		err = ldap_core->Search(filter, out, cookie, page_size, &custom_attrs, sort_pair.get());
		if (err != LDAP_SUCCESS) {
			dstream0 << "php_trueconf_ldap_search(): error=" << err;
			RETURN_LONG(err);
		}

		array_init(return_value);
		SetCustomAttrs(out, return_value, removeDistinguishedName);
	}

	dstream2 << "php_trueconf_ldap_search(" << string_view(filter, filter_len) << "): done\n";
	if (zcookie)
	{
		if (Z_TYPE_P(Z_REFVAL_P(zcookie)) == IS_LONG) {
			Z_LVAL_P(Z_REFVAL_P(zcookie)) = (long)cookie;
		}
	}
}


PHP_FUNCTION(trueconf_ldap_search_for_user){
	php_trueconf_ldap_search(INTERNAL_FUNCTION_PARAM_PASSTHRU, true);
}

PHP_FUNCTION(trueconf_ldap_search){
	php_trueconf_ldap_search(INTERNAL_FUNCTION_PARAM_PASSTHRU, false);
}


PHP_FUNCTION(trueconf_ldap_search_for_trust_user)
{
	_char(owner);
	zval * abLimits(0);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &owner, &owner_len, &abLimits) != SUCCESS) {
		dstream0 << "trueconf_ldap_search_for_trust_user(): failed to parse arguments\n";
		return;
	}

	if (!owner || !*owner){
		dprint0("trueconf_ldap_search_for_trust_user(): Invalid call_id!");
		RETURN_FALSE;
	}

	auto ldap_core = g_ldap_core.load();
	if (!ldap_core){
		dprint0("trueconf_ldap_search_for_trust_user(): ldap_core does not exist!");
		RETURN_FALSE;
	}
	std::vector<tc::ldap_user_info> users;
	ldap_core->FetchForeignUserByLogin(vs::UTF8ToLower(owner), users);
	array_init(return_value);
	AddUsersInfo(return_value, users);

	dstream2 << "trueconf_ldap_search_for_trust_user(" << string_view(owner, owner_len) << "):done\n";
}

/* {{{ proto resource trueconf_ldap_get_all_users
todo(kt): test func from ktrushnikov */
PHP_FUNCTION(trueconf_ldap_get_all_users)
{
	zval* arr = nullptr;
	zval* zcookie = nullptr;
	zend_long page_size = 0;
	_char(query_str);
	_char(z_sort_attr);
	zend_bool z_sort_order = 0;
	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zlassb", &zcookie, &page_size, &arr, &query_str, &query_str_len, &z_sort_attr, &z_sort_attr_len, &z_sort_order) != SUCCESS) {
		dstream0 << "trueconf_ldap_get_all_users(): failed to parse arguments\n";
		RETURN_FALSE;
	}

	dstream2 << "trueconf_ldap_get_all_users(): begin.\n";
	tc::page_cookie_t cookie;
	if (zcookie)
	{
		if (Z_TYPE_P(Z_REFVAL_P(zcookie)) == IS_LONG){
			cookie = Z_LVAL_P(Z_REFVAL_P(zcookie));
		}
		else
		{
			dprint0("trueconf_ldap_get_all_users(): Invalid cookie type");
			RETURN_FALSE;
		}
	}

	// get array of input custom attributes
	std::vector<tc::attr_name_t> custom_attrs;
	if (arr)
	{
		if (!GetCustomAttrs(arr, custom_attrs))
		{
			dprint0("trueconf_ldap_get_all_users(): Array initialization wrong");
			RETURN_FALSE;
		}
	}

	std::string query{ query_str, query_str_len };

	auto sort_pair = (z_sort_attr && z_sort_attr_len) ?
		std::make_unique<std::pair<std::string, bool>>(std::string(z_sort_attr, z_sort_attr_len), z_sort_order ? true : false)
		: nullptr;

	auto ldap_core = get_ldap();
	if (!ldap_core){
		dprint0("trueconf_ldap_get_all_users(): ldap_core does not exist!");
		RETURN_LONG(LDAP_PARAM_ERROR);
	}
	tc::ldap_error_code_t err(LDAP_SUCCESS);
	std::vector<tc::ldap_user_info> all_users;
	if (ldap_core->m_login_group.get().empty())
		err = ldap_core->GetAllUsers(all_users, cookie, page_size, &custom_attrs, sort_pair.get(), &query);
	else
		err = ldap_core->GetGroupUsers(ldap_core->m_login_group.get().c_str(), all_users, cookie, page_size, &custom_attrs, sort_pair.get(), &query);
	if (err != LDAP_SUCCESS)
		RETURN_LONG(err);

	if (zcookie)
	{
		if (Z_TYPE_P(Z_REFVAL_P(zcookie)) == IS_LONG) {
			Z_LVAL_P(Z_REFVAL_P(zcookie)) = (long)cookie;
		}
	}
	array_init(return_value);
	AddUsersInfo(return_value, all_users);
	dstream2 << "trueconf_ldap_get_all_users(): done. Fetched " << all_users.size() << " users\n";
}

PHP_FUNCTION(trueconf_ldap_check_login)
{
	_char(login);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &login, &login_len) != SUCCESS) {
		dstream0 << "trueconf_ldap_check_login(): failed to parse args!";
		RETURN_FALSE;
	}
	if (!login)
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid args");
		RETURN_FALSE;
	}

	auto ldap_core = g_ldap_core.load();
	if (!ldap_core) {
		dstream0 << "trueconf_ldap_check_login(): ldap core does not exist!";
		RETURN_FALSE;
	}

	tc::ldap_user_info found;
	string_view sv(login, login_len);
	std::string user_at_domain;
	auto err = ldap_core->LoginUser_CheckLogin(sv, found, vs::ignore<std::map<std::string, VS_RegGroupInfo>>(), user_at_domain);
	dstream3 << "trueconf_ldap_check_login(" << sv << ")=" << (int)err;

	array_init(return_value);
	AddLong("result", (int)err, return_value);
	if (err == tc::LDAPCore::CheckLoginResult::OK) {
		AddString("distinguishedName", found.dn, return_value);
		AddString("user_at_domain", user_at_domain, return_value);
	}
}

PHP_FUNCTION(trueconf_ldap_check_password)
{
	_char(l);
	_char(p);
	_char(d);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &l, &l_len, &p, &p_len, &d, &d_len) != SUCCESS) {
		dstream0 << "trueconf_ldap_check_login(): failed to parse args!";
		RETURN_FALSE;
	}
	if (!l || !p || !d)
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid args");
		RETURN_FALSE;
	}

	auto ldap_core = g_ldap_core.load();
	if (!ldap_core) {
		dstream0 << "trueconf_ldap_check_login(): ldap core does not exist!";
		RETURN_FALSE;
	}

	string_view login(l, l_len);
	string_view pass(p, p_len);
	string_view dn(d, d_len);

	auto err = ldap_core->LoginUser_CheckPassword(login, pass, dn);
	dstream3 << "trueconf_ldap_check_password(" << login << ")=" << err;

	RETVAL_BOOL(err);
}

PHP_MINIT_FUNCTION(trueconf_ldap)
{
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(trueconf_ldap)
{
	g_ldap_core.store(nullptr);
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

static std::atomic<bool> inited_trueconf_ldap{ false };
static std::mutex init_trueconf_ldap_mtx;

PHP_MINFO_FUNCTION(trueconf_ldap)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "TrueConf LDAP Extension", "enabled");
	php_info_print_table_end();
}

const zend_function_entry trueconf_ldap_functions[] = {
	PHP_FE(trueconf_ldap_init, arginfo_trueconf_ldap_void)
	PHP_FE(trueconf_ldap_deinit, arginfo_trueconf_ldap_void)
	PHP_FE(trueconf_ldap_connect, arginfo_trueconf_ldap_void)
	PHP_FE(trueconf_ldap_start_cache_update, arginfo_trueconf_ldap_void)
	PHP_FE(trueconf_ldap_get_last_cache_update, arginfo_trueconf_ldap_void)
	PHP_FE(trueconf_ldap_get_all_groups, arginfo_trueconf_ldap_get_all_groups)
	PHP_FE(trueconf_ldap_get_group_users, arginfo_trueconf_ldap_get_group_users)
	PHP_FE(trueconf_ldap_get_address_book, arginfo_trueconf_ldap_get_address_book)
	PHP_FE(trueconf_ldap_get_address_book_of_group, arginfo_trueconf_ldap_get_address_book_of_group)
	PHP_FE(trueconf_ldap_get_all_attributes, arginfo_trueconf_ldap_get_all_attributes)
	PHP_FE(trueconf_ldap_search_for_user, arginfo_trueconf_ldap_search)
	PHP_FE(trueconf_ldap_search, arginfo_trueconf_ldap_search)
	PHP_FE(trueconf_ldap_search_for_trust_user, arginfo_trueconf_ldap_search_for_trust_user)
	PHP_FE(trueconf_ldap_get_all_users, arginfo_trueconf_ldap_get_all_users)
	PHP_FE(trueconf_ldap_check_login, arginfo_trueconf_ldap_check_login)
	PHP_FE(trueconf_ldap_check_password, arginfo_trueconf_ldap_check_password)
	PHP_FE_END
};

zend_module_entry trueconf_ldap_module_entry = { /* {{{ */
	STANDARD_MODULE_HEADER,
	"PHP TRUECONF LDAP EXTENSION",
	trueconf_ldap_functions,
	PHP_MINIT(trueconf_ldap),
	PHP_MSHUTDOWN(trueconf_ldap),
	NULL,
	NULL,
	PHP_MINFO(trueconf_ldap),
	NO_VERSION_YET,
	STANDARD_MODULE_PROPERTIES
};
