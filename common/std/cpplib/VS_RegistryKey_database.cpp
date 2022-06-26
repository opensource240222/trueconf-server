#if defined(HAVE_CPPDB)

#include "VS_RegistryKey_database.h"
#include "VS_RegistryKey_internal.h"
#include "base64.h"
#include "std-generic/cpplib/utf8.h"
#include "../debuglog/VS_Debug.h"

#include <cppdb/errors.h>

#include "std-generic/compat/memory.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cinttypes>
#include <cstring>
#include <exception>
#include <limits>

#include "std-generic/cpplib/VS_CppDBLibs.h"

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

#define REGKEY_VERBOSE_LOGS 0

namespace regkey {

namespace {

static const std::string c_init_session_query_pg = "SET client_min_messages = error;";

// In all queries key path must not include trailing slash: "foo\bar"
static const std::string c_load_value_query = "SELECT * FROM registry.get_value(?,?);"; // $1 - key path, $2 - value name
static const std::string c_store_value_query = "SELECT registry.add_value(?,?,?,?);"; // $1 - key path, $2 - value name, $3 - type, $4 - value
static const std::string c_check_value_exists_query = "SELECT registry.has_value(?,?);"; // $1 - key path, $2 - value name
static const std::string c_remove_value_query = "SELECT registry.delete_value(?,?);"; // $1 - key path, $2 - value name
static const std::string c_remove_key_query = "SELECT registry.delete_key(?);"; // $1 - key path
static const std::string c_rename_key_query = "SELECT registry.rename_key(?,?);"; // $1 - old key path, $2 - new key path
static const std::string c_copy_key_query = "SELECT registry.copy_values(?,?);"; // $1 - old key path, $2 - new key path
static const std::string c_count_values_query = "SELECT registry.get_values_cnt(?);"; // $1 - key path
static const std::string c_load_values_query = "SELECT * FROM registry.get_values(?);"; // $1 - key path
static const std::string c_count_subkeys_query = "SELECT count(*) FROM registry.get_subkeys(?);"; // $1 - key path
static const std::string c_list_subkeys_query = "SELECT registry.get_subkeys(?);"; // $1 - key path
static const std::string c_check_key_exists_query = "SELECT registry.has_key(?);"; // $1 - key path

enum class sql_type : unsigned short
{
	// Names are in the form: logical_type "_" storage_type
	string_string = 1,
	binary_base64 = 2,
	int32_string = 3,
	int64_string = 4,
};

constexpr bool IsStoredAsString(sql_type db_type)
{
	return db_type == sql_type::string_string || db_type == sql_type::int32_string || db_type == sql_type::int64_string;
}

constexpr bool IsStoredAsBase64(sql_type db_type)
{
	return db_type == sql_type::binary_base64;
}

std::string MakeSubKeyPath(string_view key, string_view name)
{
	std::string result;
	result.reserve(key.size() + 1 + name.size());
	if (!key.empty())
	{
		result += key;
		result += '\\';
	}
	result += name;
	return result;
}

bool GetBestType_DB(sql_type db_type, RegistryVT& type)
{
	switch (db_type)
	{
	case sql_type::string_string: type = VS_REG_STRING_VT; return true;
	case sql_type::binary_base64: type = VS_REG_BINARY_VT; return true;
	case sql_type::int32_string:  type = VS_REG_INTEGER_VT; return true;
	case sql_type::int64_string:  type = VS_REG_INT64_VT; return true;
	}
	return false;
}

// Parses DB value (db_type, db_value) to type 'type' and stores it into 'buffer',
// 'size' specifies maximum size of the result that can be stored into 'buffer'.
// If size of the result is greater that 'size' returns -(size of the result).
// If buffer isn't provided (buffer == null) it will be allocated via malloc.
// If parsing fails returns 0, othewise returns size of the result.
int32_t ParseValue_DB(sql_type db_type, const std::string& db_value, RegistryVT type, void*& buffer, size_t size)
{
	switch (type)
	{
	case VS_REG_INTEGER_VT:
		if (IsStoredAsString(db_type))
		{
			const size_t result_size = sizeof(int32_t);
			if (size < result_size)
				return -static_cast<int32_t>(result_size);
			int32_t value_int;
			if (!ParseToInteger(db_value.c_str(), value_int))
				return 0;
			if (buffer == nullptr)
				buffer = ::malloc(result_size);
			*static_cast<int32_t*>(buffer) = value_int;
			return result_size;
		}
		break;
	case VS_REG_INT64_VT:
		if (IsStoredAsString(db_type))
		{
			const size_t result_size = sizeof(int64_t);
			if (size < result_size)
				return -static_cast<int32_t>(result_size);
			int64_t value_int;
			if (!ParseToInteger(db_value.c_str(), value_int))
				return 0;
			if (buffer == nullptr)
				buffer = ::malloc(result_size);
			*static_cast<int64_t*>(buffer) = value_int;
			return result_size;
		}
		break;
	case VS_REG_STRING_VT:
		if (IsStoredAsString(db_type))
		{
			const size_t result_size = db_value.size() + 1;
			if (size < result_size)
				return -static_cast<int32_t>(result_size);
			if (buffer == nullptr)
				buffer = ::malloc(result_size);
			std::memcpy(buffer, db_value.data(), result_size);
			return result_size;
		}
		break;
	case VS_REG_WSTRING_VT:
#if defined(_WIN32) // Not ported yet
		if (IsStoredAsString(db_type))
		{
			auto value_utf16 = vs::UTF8toUTF16Convert(db_value);
			if (db_value[0] && value_utf16.empty())
			{
				dstream4 << "ReqistryKeyDB: UTF8->UTF16 failed";
				return 0;
			}
			const size_t result_size = sizeof(char16_t) * (value_utf16.size() + 1);
			if (size < result_size)
				return -static_cast<int32_t>(result_size);
			if (buffer == nullptr)
				buffer = ::malloc(result_size);
			std::memcpy(buffer, value_utf16.data(), result_size);
			return result_size;
		}
#endif
		break;
	case VS_REG_BINARY_VT:
		if (IsStoredAsString(db_type))
		{
			const size_t result_size = db_value.size();
			if (size < result_size)
				return -static_cast<int32_t>(result_size);
			if (buffer == nullptr)
				buffer = ::malloc(result_size);
			std::memcpy(buffer, db_value.data(), result_size);
			return result_size;
		}
		else if (IsStoredAsBase64(db_type))
		{
			size_t result_size;
			base64_decode(db_value.data(), db_value.size(), nullptr, result_size);
			if (result_size == 0)
				return 0;
			if (size < result_size)
				return -static_cast<int32_t>(result_size);
			if (buffer == nullptr)
				buffer = ::malloc(result_size);
			base64_decode(db_value.data(), db_value.size(), buffer, result_size);
			return result_size;
		}
		break;
	}

	return 0;
}

bool ParseValue_DB(sql_type db_type, std::string&& db_value, std::string& result)
{
	if (IsStoredAsString(db_type))
	{
		result = std::move(db_value);
		return true;
	}
	return false;
}

// Formats value of type 'type' stored in (buffer, size) into DB value
// representation (sql_type db_type, std::string db_value).
// If formating is successful calls 'cb' with (db_type, db_value) as arguments
// and returns its return value, othewise return false.
template <class Callback>
bool FormatValue_DB(RegistryVT type, const void* buffer, size_t size, Callback&& cb)
{
	switch (type)
	{
	case VS_REG_INTEGER_VT:
	{
		if (size < sizeof(int32_t))
			return false;
		char db_value[1/*sign*/ + std::numeric_limits<int32_t>::digits10 + 1 + 1/*'\0'*/] = {};
		sprintf(db_value, "%" PRIi32, *static_cast<const int32_t*>(buffer));
		return cb(sql_type::int32_string, db_value);
	}
	case VS_REG_INT64_VT:
	{
		if (size < sizeof(int64_t))
			return false;
		char db_value[1/*sign*/ + std::numeric_limits<int64_t>::digits10 + 1 + 1/*'\0'*/] = {};
		sprintf(db_value, "%" PRIi64, *static_cast<const int64_t*>(buffer));
		return cb(sql_type::int64_string, db_value);
	}
	case VS_REG_STRING_VT:
		// String in the buffer should already be null-terminated, so we can pass it to cppdb drectly.
		// (At least libpq requires strings that are statement arguments to be null-terminated.)
		return cb(sql_type::string_string, static_cast<const char*>(buffer));
	case VS_REG_WSTRING_VT:
#if defined(_WIN32) // Not ported yet
	{
		auto value_utf8 = vs::UTF16toUTF8Convert(static_cast<const wchar_t*>(buffer));
		if (static_cast<const wchar_t*>(buffer)[0] && value_utf8.empty())
		{
			dstream4 << "ReqistryKeyDB: UTF16->UTF8 failed";
			return false;
		}
		return cb(sql_type::string_string, value_utf8.c_str());
	}
#else
	return false;
#endif
	case VS_REG_BINARY_VT:
	{
		size_t encoded_size;
		base64_encode(buffer, size, nullptr, encoded_size);
		auto encoded_data = vs::make_unique_default_init<char[]>(encoded_size + 1);
		base64_encode(buffer, size, encoded_data.get(), encoded_size);
		encoded_data[encoded_size] = '\0';
		return cb(sql_type::binary_base64, encoded_data.get());
	}
	}
	return false;
}

// Creates tables.
bool SQL_CheckSchema(cppdb::session& session) try
{
	// Prepare all queries, this will fail if schema isn't set up properly.
	session.create_prepared_statement(c_load_value_query);
	session.create_prepared_statement(c_store_value_query);
	session.create_prepared_statement(c_check_value_exists_query);
	session.create_prepared_statement(c_remove_value_query);
	session.create_prepared_statement(c_remove_key_query);
	session.create_prepared_statement(c_rename_key_query);
	session.create_prepared_statement(c_count_values_query);
	session.create_prepared_statement(c_load_values_query);
	session.create_prepared_statement(c_count_subkeys_query);
	session.create_prepared_statement(c_list_subkeys_query);
	session.create_prepared_statement(c_check_key_exists_query);

	return true;
}
catch (const cppdb::cppdb_error& e)
{
	dstream0 << "ReqistryKeyDB: CheckSchema failed: " << e.what();
	return false;
}

// Loads value 'value_name' from key with path 'key_path' into (db_value, db_type).
bool SQL_LoadValue(cppdb::session& session, const std::string& key_path, const char* value_name, sql_type& db_type, std::string& db_value) try
{
#if REGKEY_VERBOSE_LOGS
	auto ds = dstream4;
	ds << "SQL_LoadValue('" << key_path << "', '" << value_name << "')";
#endif

	auto result = session.create_prepared_statement(c_load_value_query)
		.bind(key_path)
		.bind(value_name)
		.query();
	if (!result.next())
		return false;

	assert(result.find_column("type") == 0);
	unsigned short db_type_raw;
	if (!result.fetch(0, static_cast<unsigned short&>(db_type_raw)))
		return false;
	db_type = static_cast<sql_type>(db_type_raw);

	assert(result.find_column("value") == 1);
	if (!result.fetch(1, db_value))
		return false;

#if REGKEY_VERBOSE_LOGS
	ds << ": success, type=" << static_cast<unsigned short>(db_type) << ", value=" << db_value;
#endif
	return true;
}
catch (const cppdb::cppdb_error& e)
{
	dstream4 << "ReqistryKeyDB: LoadValue failed: " << e.what();
	return false;
}

// Stores value (db_value, db_type) in key with path 'key_path' under name 'value_name'.
bool SQL_StoreValue(cppdb::session& session, const std::string& key_path, const char* value_name, sql_type db_type, const char* db_value) try
{
#if REGKEY_VERBOSE_LOGS
	auto ds = dstream4;
	ds << "SQL_StoreValue('" << key_path << "', '" << value_name << "')";
#endif
	// Even though cppdb supports binding string via [begin, end) range libpq
	// doesn't. It requires string arguments to be null-terminated and ignores
	// supplied argument length (yes, this is dumb), cppdb seems to be
	// oblivious of this calculates size of the range and passes begin as
	// argument. Because of that this function accepts value as null-terminated
	// string instead of string view.
	session.create_prepared_statement(c_store_value_query)
		.bind(key_path)
		.bind(value_name)
		.bind(static_cast<unsigned short>(db_type))
		.bind(db_value)
		.query();
#if REGKEY_VERBOSE_LOGS
	ds << ": success, type=" << static_cast<unsigned short>(db_type) << ", value=" << db_value;
#endif
	return true; // Currently the stored procedure always returns 0 as value.
}
catch (const cppdb::cppdb_error& e)
{
	dstream4 << "ReqistryKeyDB: StoreValue failed: " << e.what();
	return false;
}

// Checks if value 'value_name' exists in key with path 'key_path'.
bool SQL_CheckValueExists(cppdb::session& session, const std::string& key_path, const char* value_name) try
{
#if REGKEY_VERBOSE_LOGS
	auto ds = dstream4;
	ds << "SQL_CheckValueExists('" << key_path << "', '" << value_name << "')";
#endif
	auto result = session.create_prepared_statement(c_check_value_exists_query)
		.bind(key_path)
		.bind(value_name)
		.query();
	if (!result.next())
		return false;
	const bool found = result.get<std::string>(0) == "t";
#if REGKEY_VERBOSE_LOGS
	ds << (found ? ": found" : ": not found");
#endif
	return found;
}
catch (const cppdb::cppdb_error& e)
{
	dstream4 << "ReqistryKeyDB: CheckValueExists failed: " << e.what();
	return false;
}

// Removes value 'value_name' from key with path 'key_path'.
bool SQL_RemoveValue(cppdb::session& session, const std::string& key_path, const char* value_name) try
{
#if REGKEY_VERBOSE_LOGS
	auto ds = dstream4;
	ds << "SQL_RemoveValue('" << key_path << "', '" << value_name << "')";
#endif
	auto result = session.create_prepared_statement(c_remove_value_query)
		.bind(key_path)
		.bind(value_name)
		.query();
	if (!result.next())
		return false;
	const bool removed = result.get<int>(0) == 0;
#if REGKEY_VERBOSE_LOGS
	ds << (removed ? ": removed" : ": not found");
#endif
	return removed;
}
catch (const cppdb::cppdb_error& e)
{
	dstream4 << "ReqistryKeyDB: RemoveValue failed: " << e.what();
	return false;
}

// Removes key with path 'path'.
bool SQL_RemoveKey(cppdb::session& session, const std::string& path) try
{
#if REGKEY_VERBOSE_LOGS
	auto ds = dstream4;
	ds << "SQL_RemoveKey('" << path << "')";
#endif
	session.create_prepared_statement(c_remove_key_query)
		.bind(path)
		.query();
#if REGKEY_VERBOSE_LOGS
	ds << ": success";
#endif
	// There is no way to check if we really deleted what we wanted.
	// Trying to remove an empty key should be a success.
	return true;
}
catch (const cppdb::cppdb_error& e)
{
	dstream4 << "ReqistryKeyDB: RemoveKey failed: " << e.what();
	return false;
}

// Changes key path from 'from' to 'to'.
bool SQL_RenameKey(cppdb::session& session, const std::string& from, const std::string& to) try
{
#if REGKEY_VERBOSE_LOGS
	auto ds = dstream4;
	ds << "SQL_RenameKey('" << from << "', '" << to << "')";
#endif
	auto result = session.create_prepared_statement(c_rename_key_query)
		.bind(from)
		.bind(to)
		.query();
	if (!result.next())
		return false;
	const bool renamed = result.get<int>(0) == 0;
#if REGKEY_VERBOSE_LOGS
	ds << (renamed ? ": renamed" : ": not found");
#endif
	return renamed;
}
catch (const cppdb::cppdb_error& e)
{
	dstream4 << "ReqistryKeyDB: RenameKey failed: " << e.what();
	return false;
}

// Copies contents of key 'from' into key 'to'.
bool SQL_CopyKey(cppdb::session& session, const std::string& from, const std::string& to) try
{
#if REGKEY_VERBOSE_LOGS
	auto ds = dstream4;
	ds << "SQL_CopyKey('" << from << "', '" << to << "')";
#endif
	auto result = session.create_prepared_statement(c_copy_key_query)
		.bind(from)
		.bind(to)
		.query();
	if (!result.next())
		return false;
	const bool copied = result.get<int>(0) == 0;
#if REGKEY_VERBOSE_LOGS
	ds << (copied ? ": copied" : ": not found");
#endif
	return copied;
}
catch (const cppdb::cppdb_error& e)
{
	dstream4 << "ReqistryKeyDB: CopyKey failed: " << e.what();
	return false;
}

// Counts number of values in key with path 'path'.
unsigned SQL_CountValues(cppdb::session& session, const std::string& path) try
{
#if REGKEY_VERBOSE_LOGS
	auto ds = dstream4;
	ds << "SQL_CountValues('" << path << "')";
#endif
	auto result = session.create_prepared_statement(c_count_values_query)
		.bind(path)
		.query();
	if (!result.next())
		return 0;
	const auto n_values = result.get<unsigned>(0);
#if REGKEY_VERBOSE_LOGS
	ds << ": success, n_values=" << n_values;
#endif
	return n_values;
}
catch (const cppdb::cppdb_error& e)
{
	dstream4 << "ReqistryKeyDB: CountValues failed: " << e.what();
	return 0;
}

// Loads all values of key with path 'path' as raw data.
cppdb::result SQL_LoadValues(cppdb::session& session, const std::string& path) try
{
#if REGKEY_VERBOSE_LOGS
	auto ds = dstream4;
	ds << "SQL_LoadValues('" << path << "')";
#endif
	auto result = session.create_prepared_statement(c_load_values_query)
		.bind(path)
		.query();
#if REGKEY_VERBOSE_LOGS
	ds << ": success";
#endif
	return result;
}
catch (const cppdb::cppdb_error& e)
{
	dstream4 << "ReqistryKeyDB: LoadValues failed: " << e.what();
	return {};
}

// Loads next value (db_name,db_type,db_value) from raw data 'result'.
bool SQL_LoadValues_ParseNext(cppdb::result& result, std::string& db_name, sql_type& db_type, std::string& db_value)
{
	if (!result.next())
		return false;

	assert(result.find_column("name") == 0);
	if (!result.fetch(0, db_name))
		return false;

	assert(result.find_column("type") == 1);
	unsigned short db_type_raw;
	if (!result.fetch(1, static_cast<unsigned short&>(db_type_raw)))
		return false;
	db_type = static_cast<sql_type>(db_type_raw);

	assert(result.find_column("value") == 2);
	if (!result.fetch(2, db_value))
		return false;

#if REGKEY_VERBOSE_LOGS
	dstream4 << "SQL_LoadValues_ParseNext: name='" << db_name << "', type=" << static_cast<unsigned short>(db_type) << ", value=" << db_value;
#endif
	return true;
}

// Counts number of subkeys of key with path 'path'.
unsigned SQL_CountSubKeys(cppdb::session& session, const std::string& path) try
{
#if REGKEY_VERBOSE_LOGS
	auto ds = dstream4;
	ds << "SQL_CountSubKeys('" << path << "')";
#endif
	auto result = session.create_prepared_statement(c_count_subkeys_query)
		.bind(path)
		.query();
	if (!result.next())
		return 0;
	const auto n_subkeys = result.get<unsigned>(0);
#if REGKEY_VERBOSE_LOGS
	ds << ": success, n_subkeys=" << n_subkeys;
#endif
	return n_subkeys;
}
catch (const cppdb::cppdb_error& e)
{
	dstream4 << "ReqistryKeyDB: CountSubKeys failed: " << e.what();
	return 0;
}

// Gets a list of all subkeys of key with path 'path' as raw data.
cppdb::result SQL_ListSubkeys(cppdb::session& session, const std::string& path) try
{
#if REGKEY_VERBOSE_LOGS
	auto ds = dstream4;
	ds << "SQL_ListSubkeys('" << path << "')";
#endif
	auto result = session.create_prepared_statement(c_list_subkeys_query)
		.bind(path)
		.query();
#if REGKEY_VERBOSE_LOGS
	ds << ": success";
#endif
	return result;
}
catch (const cppdb::cppdb_error& e)
{
	dstream4 << "ReqistryKeyDB: ListSubkeys failed: " << e.what();
	return {};
}

// Load name of the next subkey (db_key) from raw data 'result'.
bool SQL_ListSubkeys_ParseNext(cppdb::result& result, std::string& db_key)
{
	if (!result.next())
		return false;

	assert(result.find_column("get_subkeys") == 0);
	if (!result.fetch(0, db_key))
		return false;

#if REGKEY_VERBOSE_LOGS
	dstream4 << "SQL_ListSubkeys_ParseNext: " << db_key;
#endif
	return true;
}

// Checks if key with path 'path' exists.
bool SQL_CheckKeyExists(cppdb::session& session, const std::string& path) try
{
#if REGKEY_VERBOSE_LOGS
	auto ds = dstream4;
	ds << "SQL_CheckKeyExists('" << path << "')";
#endif
	auto result = session.create_prepared_statement(c_check_key_exists_query)
		.bind(path)
		.query();
	if (!result.next())
		return false;
	const bool found = result.get<std::string>(0) == "t";
#if REGKEY_VERBOSE_LOGS
	ds << (found ? ": found" : ": not found");
#endif
	return found;
}
catch (const cppdb::cppdb_error& e)
{
	dstream4 << "ReqistryKeyDB: CheckKeyExists failed: " << e.what();
	return false;
}

}

DBKey::DBKey(std::shared_ptr<DBBackend> backend, string_view root, string_view name, bool read_only, bool create)
	: m_backend(std::move(backend))
	, m_read_only(read_only)
{
	// Workaround for bug in cppdb: call to next() on default-constructed cppdb::result results in exception.
	m_values.clear();
	m_subkeys.clear();

	if (!IsKeyNameValid(root) || !IsKeyNameValid(name))
		return;

	auto session = m_backend->OpenSession();
	if (!session.is_open())
		return;

	m_key_name.reserve(root.size() + 1 + name.size());
	auto append = [&](string_view x) {
		while (true)
		{
			const auto slash_pos = x.find('\\');
			m_key_name += x.substr(0, slash_pos);
			if (slash_pos == x.npos)
				break;
			m_key_name += '\\';
			x.remove_prefix(slash_pos + 1);

			// Remove duplicate slashes
			const auto non_slash_pos = x.find_first_not_of('\\');
			assert(non_slash_pos != x.npos); // This can fail only if the argument ends with '\' (and we check for that before).
			x.remove_prefix(non_slash_pos);
		}
	};
	append(root);
	if (!root.empty() && !name.empty())
		m_key_name += '\\';
	append(name);

	if ((m_read_only || !create) && !SQL_CheckKeyExists(session, m_key_name))
		return;

	m_session = std::move(session);
}

DBKey::DBKey(std::shared_ptr<DBBackend> backend, std::string&& key_name, bool read_only)
	: m_backend(std::move(backend))
	, m_key_name(std::move(key_name))
	, m_read_only(read_only)
{
	// Special constructor used by NextKey() that omits key existence check because:
	// 1. We don't want to interrupt iteration if one of the keys disappear in the process.
	// 2. We don't want to waste time calling SQL_CheckKeyExists because we already know that the key exists (or existed just a moment ago).

	// Workaround for bug in cppdb: call to next() on default-constructed cppdb::result results in exception.
	m_values.clear();
	m_subkeys.clear();

	assert(IsKeyNameValid(m_key_name));

	auto session = m_backend->OpenSession();
	if (!session.is_open())
		return;

	m_session = std::move(session);
}

DBKey::~DBKey()
{
}

DBKey::DBKey(const DBKey& x)
	: m_backend(x.m_backend)
	, m_session(m_backend->OpenSession())
	, m_key_name(x.m_key_name)
	, m_read_only(x.m_read_only)
{
	// Workaround for bug in cppdb: call to next() on default-constructed cppdb::result results in exception.
	m_values.clear();
	m_subkeys.clear();

	// m_values and m_subkeys aren't copied because copies of cppdb::result
	// refer to the same internal object and influence each other.
}

key_ptr DBKey::Clone() const
{
	return vs::make_unique<DBKey>(*this);
}

bool DBKey::IsValid() const
{
	return m_session.is_open();
}

int32_t DBKey::GetValue(void* buffer, size_t size, RegistryVT type, const char* name) const
{
	assert(!(buffer == nullptr && size != 0)); // This combination doesn't make sense, we also use it internally in allocating GetValue.

	if (!m_session.is_open())
		return 0;

	sql_type db_type;
	std::string db_value;
	if (!SQL_LoadValue(m_session, m_key_name, name, db_type, db_value))
		return 0;
	return ParseValue_DB(db_type, db_value, type, buffer, size);
}

int32_t DBKey::GetValue(std::unique_ptr<void, free_deleter>& buffer, RegistryVT type, const char* name) const
{
	if (!m_session.is_open())
		return 0;

	sql_type db_type;
	std::string db_value;
	if (!SQL_LoadValue(m_session, m_key_name, name, db_type, db_value))
		return 0;
	void* p_data = nullptr;
	const auto res = ParseValue_DB(db_type, db_value, type, p_data, std::numeric_limits<int32_t>::max());
	if (p_data)
		buffer.reset(p_data);
	return res;
}

int32_t DBKey::GetValueAndType(void* buffer, size_t size, RegistryVT& type, const char* name) const
{
	assert(!(buffer == nullptr && size != 0)); // This combination doesn't make sense, we also use it internally in allocating GetValue.

	if (!m_session.is_open())
		return 0;

	sql_type db_type;
	std::string db_value;
	if (!SQL_LoadValue(m_session, m_key_name, name, db_type, db_value))
		return 0;
	if (!GetBestType_DB(db_type, type))
		return 0;
	return ParseValue_DB(db_type, db_value, type, buffer, size);
}

int32_t DBKey::GetValueAndType(std::unique_ptr<void, free_deleter>& buffer, RegistryVT& type, const char* name) const
{
	if (!m_session.is_open())
		return 0;

	sql_type db_type;
	std::string db_value;
	if (!SQL_LoadValue(m_session, m_key_name, name, db_type, db_value))
		return 0;
	if (!GetBestType_DB(db_type, type))
		return 0;
	void* p_data = nullptr;
	const auto res = ParseValue_DB(db_type, db_value, type, p_data, std::numeric_limits<int32_t>::max());
	if (p_data)
		buffer.reset(p_data);
	return res;
}

bool DBKey::GetString(std::string& data, const char* name) const
{
	if (!m_session.is_open())
		return false;

	sql_type db_type;
	std::string db_value;
	if (!SQL_LoadValue(m_session, m_key_name, name, db_type, db_value))
		return false;
	return ParseValue_DB(db_type, std::move(db_value), data);
}

bool DBKey::SetValue(const void* buffer, size_t size, RegistryVT type, const char* name)
{
	if (!m_session.is_open())
		return false;
	if (m_read_only)
		return false;

	return FormatValue_DB(type, buffer, size, [&](sql_type db_type, const char* db_value)
	{
		return SQL_StoreValue(m_session, m_key_name, name, db_type, db_value);
	});
}

bool DBKey::HasValue(string_view name) const
{
	if (!m_session.is_open())
		return false;

	return SQL_CheckValueExists(m_session, m_key_name, std::string(name).c_str());
}

bool DBKey::RemoveValue(string_view name)
{
	if (!m_session.is_open())
		return false;
	if (m_read_only)
		return false;

	return SQL_RemoveValue(m_session, m_key_name, std::string(name).c_str());
}

bool DBKey::RemoveKey(string_view name)
{
	if (!m_session.is_open())
		return false;
	if (m_read_only)
		return false;

	return SQL_RemoveKey(m_session, MakeSubKeyPath(m_key_name, name));
}

bool DBKey::RenameKey(string_view from, string_view to)
{
	if (!m_session.is_open())
		return false;
	if (m_read_only)
		return false;
	switch (IsSubKey(from, to))
	{
	case 1: case 2:
		return false; // Moving to a or from a subkey, registry backend doesn't support this so we shouldn't also.
	case 3:
		return true; // Moving to itself, nothing to do.
	}

	return SQL_RenameKey(m_session, MakeSubKeyPath(m_key_name, from), MakeSubKeyPath(m_key_name, to));
}

bool DBKey::CopyKey(string_view from, string_view to)
{
	if (!m_session.is_open())
		return false;
	if (m_read_only)
		return false;
	switch (IsSubKey(from, to))
	{
	case 1: case 2:
		return false; // Copying to a or from a subkey, registry backend doesn't support this so we shouldn't also.
	case 3:
		return true; // Copying to itself, nothing to do.
	}

	return SQL_CopyKey(m_session, MakeSubKeyPath(m_key_name, from), MakeSubKeyPath(m_key_name, to));
}

unsigned DBKey::GetValueCount() const
{
	if (!m_session.is_open())
		return false;

	return SQL_CountValues(m_session, m_key_name);
}

unsigned DBKey::GetKeyCount() const
{
	if (!m_session.is_open())
		return false;

	return SQL_CountSubKeys(m_session, m_key_name);
}

void DBKey::ResetValues()
{
	if (!m_session.is_open())
		return;

	m_values = SQL_LoadValues(m_session, m_key_name);
}

int32_t DBKey::NextValue(std::unique_ptr<void, free_deleter>& buffer, RegistryVT type, std::string& name)
{
	sql_type db_type;
	std::string db_value;
	if (!SQL_LoadValues_ParseNext(m_values, name, db_type, db_value))
		return 0;

	void* p_data = nullptr;
	const auto res = ParseValue_DB(db_type, db_value, type, p_data, std::numeric_limits<int32_t>::max());
	if (p_data)
		buffer.reset(p_data);
	return res;
}

int32_t DBKey::NextValueAndType(std::unique_ptr<void, free_deleter>& buffer, RegistryVT& type, std::string& name)
{
	sql_type db_type;
	std::string db_value;
	if (!SQL_LoadValues_ParseNext(m_values, name, db_type, db_value))
		return 0;
	if (!GetBestType_DB(db_type, type))
		return 0;

	void* p_data = nullptr;
	const auto res = ParseValue_DB(db_type, db_value, type, p_data, std::numeric_limits<int32_t>::max());
	if (p_data)
		buffer.reset(p_data);
	return res;
}

bool DBKey::NextString(std::string& data, std::string& name)
{
	sql_type db_type;
	std::string db_value;
	if (!SQL_LoadValues_ParseNext(m_values, name, db_type, db_value))
		return 0;

	return ParseValue_DB(db_type, std::move(db_value), data);
}

void DBKey::ResetKey()
{
	if (!m_session.is_open())
		return;

	m_subkeys = SQL_ListSubkeys(m_session, m_key_name);
}

key_ptr DBKey::NextKey(bool read_only)
{
	std::string db_key;
	if (!SQL_ListSubkeys_ParseNext(m_subkeys, db_key))
		return nullptr;

	key_ptr key = std::unique_ptr<DBKey>(new DBKey(m_backend, std::move(db_key), read_only));
	if (!key->IsValid())
		return nullptr;
	return key;
}

const char* DBKey::GetName() const
{
	if (!m_session.is_open())
		return nullptr;

	const auto last_sep_pos = m_key_name.find_last_of('\\');
	if (last_sep_pos == m_key_name.npos)
		return nullptr;
	const auto name_pos = last_sep_pos + 1;
	return m_key_name.data() + name_pos;
}

std::string DBKey::GetLastWriteTime() const
{
	std::string result;
	GetString(result, ""); // value with default empty name
	return result;
}

bool DBBackend::Init(string_view configuration) try
{
	m_pool = cppdb::pool::create(std::string(configuration));
	auto session = OpenSession_internal();
	if (!session.is_open())
		return false;
	if (!SQL_CheckSchema(session))
		return false;
	return true;
}
catch (const cppdb::cppdb_error& e)
{
	dstream0 << "ReqistryKeyDB: Init failed: " << e.what();
	return false;
}

key_ptr DBBackend::Create(string_view root, bool, string_view name, bool read_only, bool create)
{
	return vs::make_unique<DBKey>(shared_from_this(), root, name, read_only, create);
}

cppdb::session DBBackend::OpenSession_internal()
{
	return cppdb::session(m_pool->open(), [](cppdb::session& session) {
		session.create_statement(c_init_session_query_pg).exec();
	});
}

cppdb::session DBBackend::OpenSession() try
{
	return OpenSession_internal();
}
catch (const cppdb::cppdb_error& e)
{
	dstream4 << "ReqistryKeyDB: OpenSession failed: " << e.what();
	return {};
}

}

#endif
