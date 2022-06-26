#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "common.h"

#include "php.h"
#include "php_ini.h"
#include "ext/standard/php_string.h"
#include "ext/standard/info.h"

#include "php_tccache.h"
#include "vs_zend_parse_parameters.h"

#include <map>
#include <string>
#include <vector>
#include "std/cpplib/fast_mutex.h"
#include "std-generic/compat/map.h"
#include "std-generic/cpplib/synchronized.h"

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_PHP

static time_t tc_testvar_GetTime = ~0;
static int tc_testvar_TimeToCallClear = ~0;

time_t GetTime(time_t *)
{
	if (tc_testvar_GetTime != ~0)
		return tc_testvar_GetTime;
	else return time(NULL);
}

class Cache
{
public:

	Cache()
	{
		m_defaultTimeout = 60 * 60 * 24 * 365; // 1 year
	}

	~Cache()
	{
	}

	void SetDefaultTimeout(time_t timeout)
	{
		m_defaultTimeout = timeout;
	}

	bool SetData(const std::string &key, std::string &val, time_t timeout = ~0/*uses default*/)
	{
		return UpdateDataInternal(key, val, val, timeout, false);
	}

	void Delete(std::string &key)
	{
		auto pLockedDB = m_db.lock();
		std::map<std::string, dbRecord>::iterator i = pLockedDB->find( key );
		if (i != pLockedDB->end()) pLockedDB->erase( i );
	}

	bool GetData(const std::string &key, std::string &val)
	{
		if (TimeToCallClear()) ClearOutdated();

		{auto pLockedDB = m_db.lock();
		std::map<std::string, dbRecord>::iterator i = pLockedDB->find(key);
		if (i != pLockedDB->end())
		{
			if (i->second.last_update_time + i->second.timeout <= GetTime(NULL))
			{
				pLockedDB->erase(i);
				return false;
			}
			i->second.last_update_time = GetTime(NULL);
			val = i->second.val;
			return true;
		}
		}

		return false;
	}

	bool AtomicCompareExchange(const std::string &key, std::string &old_val, std::string &new_val, time_t timeout = ~0/*uses default*/)
	{
		return UpdateDataInternal(key, old_val, new_val, timeout, true);
	}

	bool IsExists(const std::string &key)
	{
		{auto pLockedDB = m_db.lock();
		std::map<std::string, dbRecord>::iterator i = pLockedDB->find(key);

		// key not found;
		if (i == pLockedDB->end())
			return false;

		// entry is outdated
		if (i->second.last_update_time + i->second.timeout <= GetTime(NULL))
		{
			// remove it from cache
			pLockedDB->erase(i);
			return false;
		}
		}	// end of lock

		return true;
	}

	void Clear()
	{
		m_db->clear();
	}

	struct Info
	{
		std::vector<std::string> keys;
		size_t total_used;
	};

	void GetUsageInfo(Info &res)
	{
		res.keys.clear();
		res.total_used = 0;

		{auto pLockedDB = m_db.lock();
		for (auto i = pLockedDB->begin(); i != pLockedDB->end(); ++i)
		{
			if (i->second.last_update_time + i->second.timeout > GetTime(NULL))
			{
				res.keys.push_back(i->first);
			}
			res.total_used += i->first.size() + i->second.GetSize();
		}
		}	// end of lock
	}

private:

	bool UpdateDataInternal(const std::string &key, std::string &old_val, std::string &new_val, time_t timeout, bool compare)
	{
		if (TimeToCallClear()) ClearOutdated();

		if (timeout == ~0) timeout = m_defaultTimeout;

		{auto pLockedDB = m_db.lock();
		auto i = pLockedDB->find( key );

		if (compare)
		{
			std::string *real_val = NULL;
			if (i != pLockedDB->end() && i->second.last_update_time + i->second.timeout > GetTime(NULL)) real_val = &i->second.val;
			if (real_val == NULL && old_val.empty() || real_val != NULL && *real_val == old_val)
			{

			} else return false;
		}

		if (i == pLockedDB->end())
		{
			i = pLockedDB->emplace(key, dbRecord()).first;
		}

		i->second.val = new_val;
		i->second.timeout = timeout;
		i->second.last_update_time = GetTime(NULL);
		}	// end of lock
		return true;
	}

	void ClearOutdated()
	{
		{auto pLockedDB = m_db.lock();

		for (auto i = pLockedDB->begin(); i != pLockedDB->end(); )
		{
			if (i->second.last_update_time + i->second.timeout <= GetTime(NULL))
			{
				i = pLockedDB->erase( i );
			} else ++i;
		}
		}	// end of lock
	}

	bool TimeToCallClear()
	{
		if (tc_testvar_TimeToCallClear != ~0) return  !!tc_testvar_TimeToCallClear;
		return rand() < RAND_MAX / 100;
	}

	struct dbRecord
	{
		std::string val;
		time_t last_update_time;
		time_t timeout;

		size_t GetSize()
		{
			return val.size() + sizeof(dbRecord);
		}
	};

	vs::Synchronized<vs::map<std::string, dbRecord>, vs::fast_mutex> m_db;
	time_t m_defaultTimeout;
};


const zend_function_entry tccache_functions[] = {
	PHP_FE(tccache_set_timeout, NULL)
	PHP_FE(tccache_info, NULL)
	PHP_FE(tccache_set, NULL)
	PHP_FE(tccache_get, NULL)
	PHP_FE(tccache_cas, NULL)
	PHP_FE(tccache_exists, NULL)
	PHP_FE(tccache_delete, NULL)
	PHP_FE(tccache_clear, NULL)
	PHP_FE(tccache_test_SetTime, NULL)
	PHP_FE(tccache_test_setTimeToClear, NULL)
	PHP_FE_END
};

zend_module_entry tccache_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"tccache",
	tccache_functions,
	PHP_MINIT(tccache),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(tccache),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_TRUECONF_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_TCCACHE
ZEND_GET_MODULE(tccache)
#endif

PHP_MINIT_FUNCTION(tccache)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(tccache)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Trueconf Cache", "enabled");
	php_info_print_table_end();
}

Cache tc_cache_instance;


PHP_FUNCTION(tccache_set_timeout)
{
	zend_long timeout(0);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
		&timeout) != SUCCESS) {
		dprint1("tccache_set_timeout: fail to parse arguments\n");
		RETURN_FALSE;
	}

	if (timeout == 0) RETURN_FALSE;
	tc_cache_instance.SetDefaultTimeout( timeout );
	dstream2 << "tccache_set_timeout: done. timeout=" << timeout;
	RETURN_TRUE;
}

PHP_FUNCTION(tccache_info)
{
	Cache::Info res;
	tc_cache_instance.GetUsageInfo( res );

	array_init(return_value);

	add_assoc_long(return_value, "total_bytes_used",	res.total_used);

	zval arr;
	array_init(&arr);

	for (unsigned i = 0; i < res.keys.size(); ++i)
	{
		add_next_index_string(&arr, res.keys[i].c_str());
	}
	add_assoc_zval(return_value, "keys", &arr);
	dprint2("tccache_info: done. total_bytes_used=%u\n", (uint32_t)res.total_used);
}

PHP_FUNCTION(tccache_set)
{
	_char(key);
	_char(val);

	zend_long timeout(~0);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|l",
		&key, &key_len, &val, &val_len, &timeout) != SUCCESS) {
		dprint1("tccache_set: fail to parse arguments\n");
		RETURN_FALSE;
	}

	std::string _key(key, key + key_len);
	std::string _val(val, val + val_len);

	bool res = tc_cache_instance.SetData(_key, _val, timeout);
	dprint2("tccache_set: done. key=%s, val=%s\n", _key.c_str() ,_val.c_str());
	RETURN_BOOL( res );
}

PHP_FUNCTION(tccache_get)
{
	_char(key);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
		&key, &key_len) != SUCCESS) {
		dprint1("tccache_get: fail to parse arguments\n");
		RETURN_FALSE;
	}

	std::string _key(key, key + key_len);
	std::string val;

	bool res = tc_cache_instance.GetData(_key, val);

	if (!res) {
		dprint1("tccache_get: failed\n");
		RETURN_FALSE;
	}
	else {
		dprint2("tccache_get: done. key=%s, val=%s\n", _key.c_str(), val.c_str());
		RETURN_STRINGL(val.c_str(), val.size());
	}
}

PHP_FUNCTION(tccache_cas)
{
	_char(key);
	_char(old_val);
	_char(val);

	zend_long timeout(~0);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss|l",
		&key, &key_len, &old_val, &old_val_len, &val, &val_len, &timeout) != SUCCESS) {
		dprint1("tccache_cas: fail to parse arguments\n");
		RETURN_FALSE;
	}

	std::string _key(key, key + key_len);
	std::string _val(val, val + val_len);
	std::string _old_val(old_val, old_val + old_val_len);

	bool res = tc_cache_instance.AtomicCompareExchange(_key, _old_val, _val, timeout);
	dprint2("tccache_cas: done. key=%s, val=%s, res=%d\n",_key.c_str(), _val.c_str() ,res);
	RETURN_BOOL( res );
}

PHP_FUNCTION(tccache_exists)
{
	_char(key);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
		&key, &key_len) != SUCCESS) {
		dprint1("tccache_exists: fail to parse arguments\n");
		RETURN_FALSE;
	}

	std::string _key(key, key + key_len);

	bool res = tc_cache_instance.IsExists(_key);
	dprint2("tccache_exists: done. key=%s, res=%d\n", _key.c_str() ,res);
	RETURN_BOOL(res);
}

PHP_FUNCTION(tccache_delete)
{
	_char(key);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
		&key, &key_len) != SUCCESS) {
		dprint1("tccache_delete: fail to parse arguments\n");
		RETURN_FALSE;
	}

	std::string _key(key, key + key_len);

	tc_cache_instance.Delete(_key);
	dprint2("tccache_exists: done. key=%s\n", _key.c_str());
	RETURN_TRUE;
}

PHP_FUNCTION(tccache_clear)
{
	tc_cache_instance.Clear();
	RETURN_TRUE;
}

PHP_FUNCTION(tccache_test_SetTime)
{
	zend_long timeout(0);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
		&timeout) != SUCCESS) {
		dprint1("tccache_test_SetTime: fail to parse arguments\n");
			RETURN_FALSE;
	}

	tc_testvar_GetTime = timeout;
	dstream2 << "tccache_test_SetTime: done. timeout=" << timeout;
	RETURN_TRUE;
}

PHP_FUNCTION(tccache_test_setTimeToClear)
{
	zend_long timeout(0);

	if (vs_zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
		&timeout) != SUCCESS) {
		dprint1("tccache_test_setTimeToClear: fail to parse arguments\n");
			RETURN_FALSE;
	}

	tc_testvar_TimeToCallClear = timeout;
	dstream2 << "tccache_test_setTimeToClear: done. timeout=" << timeout;
	RETURN_TRUE;
}