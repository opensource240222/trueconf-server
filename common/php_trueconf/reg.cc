#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "common.h"

#include "php.h"
#include "php_ini.h"
#include "ext/standard/php_string.h"
#include "ext/standard/info.h"

#include "php_reg.h"
#include "vs_zend_parse_parameters.h"

#include "../std/cpplib/VS_RegistryKey.h"
#include "std-generic/cpplib/string_view.h"

#include <vector>
#include <cstdlib>
#include <wchar.h>

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_PHP

const zend_function_entry reg_functions[] = {
    PHP_FE(reg_read_value, NULL)
	PHP_FE(reg_write_value, NULL)
	PHP_FE(reg_delete_value, NULL)
	PHP_FE(reg_get_value_names, NULL)
	PHP_FE(reg_read_values, NULL)
	PHP_FE(reg_read_values_assoc, NULL)

	PHP_FE(reg_create_key, NULL)
	PHP_FE(reg_delete_key, NULL)
	PHP_FE(reg_get_sub_keys, NULL)
	PHP_FE(reg_key_exists, NULL)
	PHP_FE(reg_value_exists, NULL)
	PHP_FE(reg_get_key_info, NULL)

	PHP_FE_END
};

zend_module_entry reg_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
    "reg",
    reg_functions,
    PHP_MINIT(reg),
    NULL,
    NULL,
    NULL,
    PHP_MINFO(reg),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_TRUECONF_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#define REGISTER_LONG_CONSTANT_MACROS(x) REGISTER_LONG_CONSTANT(#x, x,  CONST_PERSISTENT | CONST_CS)

PHP_MINIT_FUNCTION(reg)
{
	return SUCCESS;
}

char * RegistryVT_ToString[5] =
{
	/* 0		*/		"REG_DWORD",
	/* 1		*/		"REG_QWORD",
	/* 2		*/		"REG_SZ",
	/* 3		*/		"REG_SZ",
	/* 4		*/		"REG_BINARY",
};


void create_RegValue(zval* return_value, char *name, char *val, size_t val_len, char *type)
{
	array_init(return_value);

	add_assoc_string(return_value, "name", name);
	add_assoc_string(return_value, "type", type);
	add_assoc_stringl(return_value, "value", val, val_len);
}


void create_RegKeyInfo(zval* return_value, const std::string& last_write_time, uint32_t sub_keys_count, uint32_t values_count)
{
	array_init(return_value);

	add_assoc_string(return_value, "last_write_time", const_cast<char*>(last_write_time.c_str()));
	add_assoc_long(return_value, "values_count",	values_count);
	add_assoc_long(return_value, "sub_keys_count",	sub_keys_count);
}

static bool ReadRegPath(const char* path, size_t path_len, std::string& short_path)
{
	if (!path)
		return false;

	string_view result { path, path_len };
	const auto& root = VS_RegistryKey::GetDefaultRoot();
	const auto root_pos = result.find(root);
	if (root_pos == string_view::npos)
		return false; // Path is outside of our root
	result.remove_prefix(root_pos + root.size());
	if (!result.empty() && result.front() != '\\')
		return false; // Found substring doesn't correspond to a full key (e.g "TrueConf\\Server" vs "TrueConf\\ServerTwo")

	// Trim extra backslashes on both ends
	while (!result.empty() && result.front() == '\\')
		result.remove_prefix(1);
	while (!result.empty() && result.back() == '\\')
		result.remove_suffix(1);

	short_path = std::string(result);
	return true;
}

RegistryVT StringToRegVT(const string_view szType) {
	if (szType.compare("REG_SZ") == 0) return VS_REG_STRING_VT;			else
	if (szType.compare("REG_BINARY"	) == 0) return VS_REG_BINARY_VT;	else
	if (szType.compare("REG_DWORD"	) == 0) return VS_REG_INTEGER_VT;	else
	if (szType.compare("REG_QWORD"	) == 0) return VS_REG_INT64_VT;		else
	if (szType.compare("REG_EXPAND_SZ") == 0) return VS_REG_STRING_VT;	else
	if (szType.compare("REG_MULTI_SZ") == 0) return VS_REG_STRING_VT;	else
	/*default*/	return VS_REG_STRING_VT;
}

char* RegValToString(const void *val,const size_t in_len, RegistryVT type, std::string& OUT_strDigit, size_t &OUT_len) {
	if (!val) return nullptr;
	switch (type) {
	case VS_REG_INTEGER_VT: {
		OUT_strDigit = std::to_string(*static_cast<const uint32_t*>(val));
		OUT_len = OUT_strDigit.length();
		return (char *)(OUT_strDigit.c_str());
	}
	case VS_REG_INT64_VT: {
		OUT_strDigit = std::to_string(*static_cast<const uint64_t*>(val));
		OUT_len = OUT_strDigit.length();
		return (char *)(OUT_strDigit.c_str());
	}
	case VS_REG_STRING_VT:
		OUT_len = in_len;
		if (OUT_len) --OUT_len;
		return (char*)val;
	case VS_REG_BINARY_VT:
	default:
		OUT_len = in_len;
		return (char*)val;
	}
}

// PHP extention implementaion  //

PHP_FUNCTION(reg_read_value)
{
	char *type;
	_char(reg_path);
	_char(name);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"ss", &reg_path, &reg_path_len, &name, &name_len) != SUCCESS) {
		dprint1("reg_read_value: can't parse reg path\n");
		RETURN_FALSE;
	}

	std::string short_path;
	if (!ReadRegPath(reg_path, reg_path_len,  short_path)) {
		dprint1("reg_read_value: can't get short path from '%s'\n", reg_path);
		RETURN_FALSE;
	}

	VS_RegistryKey key( false, short_path, true, false);

	std::unique_ptr<void, free_deleter> val;
	RegistryVT reg_type = VS_REG_STRING_VT;
	size_t val_len = key.GetValueAndType(val, reg_type, name);
	type = RegistryVT_ToString[reg_type];

	if ((int32_t)val_len <= 0) {
		dprint1("reg_read_value: failed for path=%s, res=%d\n", short_path.c_str(), (int32_t)val_len);
		RETVAL_FALSE;
	}
	else
	{
		std::string buffer;
		auto res_val = RegValToString(val.get(), val_len, reg_type, buffer, val_len);
		create_RegValue(return_value, name, res_val, val_len, type);
		dprint2("reg_read_value(%s): success\n", short_path.c_str());
	}
}


PHP_FUNCTION(reg_value_exists)
{
	_char(reg_path);
	_char(name);
	bool result = false;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"ss", &reg_path, &reg_path_len, &name, &name_len) != SUCCESS) {
		dprint1("reg_value_exists: can't parse reg path\n");
		RETURN_FALSE;
	}

	std::string short_path;
	if (!ReadRegPath(reg_path, reg_path_len,  short_path)) {
		dprint1("reg_value_exists: can't get short path from '%s'\n", reg_path);
		RETURN_FALSE;
	}

	VS_RegistryKey key( false, short_path, true, false);
	result = key.HasValue(name);

	dprint2("reg_value_exists(%s): %d\n", short_path.c_str(), result);
	RETURN_BOOL(result);
}


PHP_FUNCTION(reg_write_value)
{
	_char( reg_path);
	_char( name);
	_char( val);
	_char( type);
	bool res ;


	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss|s",
		&reg_path, &reg_path_len, &name, &name_len, &val, &val_len, &type, &type_len) != SUCCESS) {
		dprint1("reg_write_value: can't parse reg path\n");
		RETURN_FALSE;
	}

	std::string short_path;
	if (!ReadRegPath(reg_path, reg_path_len,  short_path)) {
		dprint1("reg_write_value: can't get short path from '%s'\n", reg_path);
		RETURN_FALSE;
	}

	VS_RegistryKey key( false, short_path, false, true);
	auto regType = StringToRegVT(type);
	switch (regType) {
	case VS_REG_INTEGER_VT: {
		int32_t iVal = static_cast<int32_t>(strtol(val, nullptr, 10));
		res = key.SetValue(&iVal, sizeof(iVal), regType, name);
		break;
	}
	case VS_REG_INT64_VT: {
		int64_t iVal = static_cast<int64_t>(strtoll(val, nullptr, 10));
		res = key.SetValue(&iVal, sizeof(iVal), regType, name);
		break;
	}
	case VS_REG_STRING_VT:
	case VS_REG_BINARY_VT:
	default:
		res = key.SetValue(val, val_len, regType, name);
		break;
	}

	dprint2("reg_write_value(%s, %s): %d\n", short_path.c_str(), val, res);
	RETURN_BOOL( res );
}


PHP_FUNCTION(reg_delete_value)
{
	_char(reg_path);
	_char(name);
	bool res;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
		&reg_path, &reg_path_len, &name, &name_len) != SUCCESS) {
		dprint1("reg_delete_value: can't parse reg path\n");
		RETURN_FALSE;
	}

	std::string short_path;
	if (!ReadRegPath(reg_path, reg_path_len,  short_path)) {
		dprint1("reg_delete_value: can't get short path from '%s'\n", reg_path);
		RETURN_FALSE;
	}

	VS_RegistryKey key( false, short_path, false, true);
	res = key.RemoveValue(name);

	dprint2("reg_delete_value(%s): %d\n", short_path.c_str() ,res);
	RETURN_BOOL( res );
}


PHP_FUNCTION(reg_get_value_names)
{
	_char(reg_path);
	_char(r_type);

	r_type = 0;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s",
		&reg_path, &reg_path_len, &r_type, &r_type_len) != SUCCESS) {
		dprint1("reg_get_value_names: can't parse reg path\n");
		RETURN_FALSE;
	}

	std::string short_path;
	if (!ReadRegPath(reg_path, reg_path_len,  short_path)) {
		dprint1("reg_get_value_names: can't get short path from '%s'\n", reg_path);
		RETURN_FALSE;
	}

	VS_RegistryKey key( false, short_path, true, false);
	if (!key.IsValid()) {
		RETURN_FALSE;
	}

	array_init(return_value);

	std::unique_ptr<void, free_deleter> buff;
	RegistryVT vt = VS_REG_STRING_VT;
	std::string sname;
	key.ResetValues();
	while (key.NextValueAndType(buff, vt, sname) > 0) {
		if (r_type == NULL)
		{
			add_next_index_string(return_value, sname.c_str());
		}
		else {
			auto pType = RegistryVT_ToString[vt];
			if (strcmp(r_type, pType) == 0)
			{
				add_next_index_string(return_value, sname.c_str());
			}
		}
	}

	dprint2("reg_get_value_names(%s):done\n", short_path.c_str());
}

PHP_FUNCTION(reg_read_values)
{
	_char(reg_path);
	_char(val);
	_char(r_type);
	r_type = NULL;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s",
		&reg_path, &reg_path_len, &r_type, &r_type_len) != SUCCESS) {
		dprint1("reg_read_values: can't parse reg path\n");
		RETURN_FALSE;
	}

	std::string short_path;
	if (!ReadRegPath(reg_path, reg_path_len,  short_path)) {
		dprint1("reg_read_values: can't get short path from '%s'\n", reg_path);
		RETURN_FALSE;
	}

	VS_RegistryKey key( false, short_path, true, false);
	array_init(return_value);

	std::unique_ptr<void, free_deleter> val_buff;
	RegistryVT vt = VS_REG_STRING_VT;
	std::string sname;
	key.ResetValues();
	while ((val_len = key.NextValueAndType(val_buff, vt, sname)) > 0) {
		std::string sbuff;
		val = RegValToString(val_buff.get(), val_len, vt, sbuff, val_len);

		auto pType = RegistryVT_ToString[vt];
		if (r_type == NULL || strcmp(r_type, pType) == 0)
		{
			zval tmp;
			create_RegValue(&tmp, (char*)sname.c_str(), val, val_len, pType);
			add_next_index_zval(return_value, &tmp);
		}
	}

	dprint2("reg_read_values(%s): done\n", short_path.c_str());
}

PHP_FUNCTION(reg_read_values_assoc)
{
	_char(reg_path);

	zval *keys(0), *entry(0);
	HashPosition pos;
	zend_bool useKeys = 0;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ab",
		&reg_path, &reg_path_len, &keys, &useKeys) != SUCCESS) {
		dprint1("reg_read_values_assoc: can't parse reg path\n");
		RETURN_FALSE;
	}

	std::string short_path;
	if (!ReadRegPath(reg_path, reg_path_len,  short_path)) {
		dprint1("reg_read_values_assoc: can't get short path from '%s'\n", reg_path);
		RETURN_FALSE;
	}

	VS_RegistryKey key( false, short_path, true, false);
	array_init(return_value);

	if (keys){
		zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(keys), &pos);
		while ((entry = zend_hash_get_current_data_ex(Z_ARRVAL_P(keys), &pos)) != NULL) {
			if (Z_TYPE_P(entry) != IS_STRING){
				zend_hash_move_forward_ex(Z_ARRVAL_P(keys), &pos);
				continue;
			}

			std::unique_ptr<void, free_deleter> val;
			RegistryVT reg_type = VS_REG_STRING_VT;
			size_t val_len = key.GetValueAndType(val, reg_type, Z_STRVAL_P(entry));

			std::string buff;
			auto res_val = RegValToString(val.get(), val_len, reg_type, buff, val_len);

			if ((int32_t)val_len <= 0)
			{
				zend_hash_move_forward_ex(Z_ARRVAL_P(keys), &pos);
				continue;
			}

			if (useKeys){
				zend_string *strKey(0);
				zend_ulong keyIndex = 0;
				int keyType = zend_hash_get_current_key_ex(Z_ARRVAL_P(keys), &strKey, &keyIndex, &pos);

				switch (keyType){
				case HASH_KEY_IS_LONG:
					add_index_stringl(return_value, keyIndex, res_val, val_len);
					break;
				case HASH_KEY_IS_STRING:
					add_assoc_stringl(return_value, strKey->val, res_val, val_len);
					break;
				}
			}
			else {
				add_assoc_stringl(return_value, Z_STRVAL_P(entry), res_val, val_len);
			}

			zend_hash_move_forward_ex(Z_ARRVAL_P(keys), &pos);
		}
	}
	else {

		std::unique_ptr<void, free_deleter> val;
		std::string sname;
		RegistryVT vt = VS_REG_STRING_VT;
		int32_t val_len(0);
		key.ResetValues();
		while ((val_len = key.NextValueAndType(val, vt, sname)) > 0) {
			std::string sbuff;
			size_t len = val_len;
			auto res_val = RegValToString(val.get(), len, vt, sbuff, len);
			add_assoc_stringl(return_value, sname.c_str(), res_val, len);
		}
	}

	dprint2("reg_read_values_assoc(%s): done\n", short_path.c_str());
}


PHP_FUNCTION(reg_create_key)
{
	_char(reg_path);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
		&reg_path, &reg_path_len) != SUCCESS) {
		dprint1("reg_create_key: can't parse reg path\n");
		RETURN_FALSE;
	}

	std::string short_path;
	if (!ReadRegPath(reg_path, reg_path_len,  short_path)) {
		dprint1("reg_create_key: can't get short path from '%s'\n", reg_path);
		RETURN_FALSE;
	}

	VS_RegistryKey key( false, short_path, false, true);

	dprint2("reg_create_key(%s):%d\n", short_path.c_str(), key.IsValid());
	RETURN_BOOL(key.IsValid());
}


PHP_FUNCTION(reg_delete_key)
{
	_char(reg_path);
	bool h;
	zend_bool delete_all = false;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b",
		&reg_path, &reg_path_len, &delete_all) != SUCCESS) {
		dprint1("reg_delete_key: can't parse reg path\n");
		RETURN_FALSE;
	}

	std::string short_path;
	if (!ReadRegPath(reg_path, reg_path_len,  short_path)) {
		dprint1("reg_delete_key: can't get short path from '%s'\n", reg_path);
		RETURN_FALSE;
	}

	VS_RegistryKey key(false, "", false, true);
	h = key.RemoveKey(short_path);

	dprint2("reg_delete_key(%s): %d\n", short_path.c_str(), h);
	RETURN_BOOL( h );
}

PHP_FUNCTION(reg_get_sub_keys)
{
	_char(reg_path);

	int i = 0;

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
		&reg_path, &reg_path_len) != SUCCESS) {
		dprint1("reg_get_sub_keys: can't parse reg path\n");
		RETURN_FALSE;
	}

	std::string short_path;
	if (!ReadRegPath(reg_path, reg_path_len,  short_path)) {
		dprint1("reg_get_sub_keys: can't get short path from '%s'\n", reg_path);
		RETURN_FALSE;
	}

	array_init(return_value);

	VS_RegistryKey key( false, short_path, true, false);

	VS_RegistryKey subKey;
	key.ResetKey();
	while (key.NextKey(subKey)) {
		add_next_index_string(return_value, subKey.GetName());
	}

	dprint2("reg_get_sub_keys(%s): done\n", short_path.c_str());
}


PHP_FUNCTION(reg_key_exists)
{
	_char(reg_path);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
		&reg_path, &reg_path_len) != SUCCESS) {
		dprint1("reg_key_exists: can't parse reg path\n");
		RETURN_FALSE;
	}

	std::string short_path;
	if (!ReadRegPath(reg_path, reg_path_len,  short_path)) {
		dprint1("reg_key_exists: can't get short path from '%s'\n", reg_path);
		RETURN_FALSE;
	}

	VS_RegistryKey key( false, short_path, true, false);
	dprint2("reg_key_exists(%s): %d\n", short_path.c_str(), key.IsValid());
	RETURN_BOOL(key.IsValid()) ;
}

PHP_FUNCTION(reg_get_key_info)
{
	_char(reg_path);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
		&reg_path, &reg_path_len) != SUCCESS) {
		dprint1("reg_get_key_info: can't parse reg path\n");
		RETURN_FALSE;
	}

	std::string short_path;
	if (!ReadRegPath(reg_path, reg_path_len,  short_path)) {
		dprint1("reg_get_key_info: can't get short path from '%s'\n", reg_path);
		RETURN_FALSE;
	}

	VS_RegistryKey key( false, short_path, true, false);
	if (!key.IsValid()) {
		RETURN_FALSE;
	}

	auto last_write_time = key.GetLastWriteTime();
	int32_t sub_keys_count = key.GetKeyCount();
	int32_t values_count = key.GetValueCount();

	create_RegKeyInfo(return_value, last_write_time, sub_keys_count, values_count);
	dprint2("reg_get_key_info(%s): done\n", short_path.c_str());
}

PHP_MINFO_FUNCTION(reg)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Registry Lib", "enabled");
	php_info_print_table_end();
}
