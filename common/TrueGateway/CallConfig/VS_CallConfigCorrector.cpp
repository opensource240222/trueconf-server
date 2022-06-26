#include "VS_CallConfigCorrector.h"
#include "../../SIPParserBase/VS_Const.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std-generic/cpplib/utf8.h"
#include "std/cpplib/md5.h"
#include "std/debuglog/VS_Debug.h"

#include <cstdint>
#include <cstdio>
#include <cassert>

#include <string>
#include <vector>
#include <fstream>
#include <streambuf>
#include <chrono>

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

typedef std::lock_guard<std::recursive_mutex> lock_guard_t;

// VS_CallConfigCorrector is a global object
std::recursive_mutex VS_CallConfigCorrector::g_lock;

// function get_call_config_data(protocol, terminal_id) ... end
//
// Parameters description:
// protocol - numeric protocol identifier: SIGNAL_PROTOCOL_SIP, SIGNAL_PROTOCOL_RTSP, SIGNAL_PROTOCOL_H323;
// terminal - terminal identifier as string.
// Return values:
// nil   - can't find correction data for specified terminal;
// table - table with call config prarameters where key is the parameter name string.
#define LUA_ENTRY_FUNCTION "get_call_config_data"
// 1 Mb
#define REG_BUF_SZ (1 * 1024 * 1024)

VS_CallConfigCorrector &VS_CallConfigCorrector::GetInstance(void)
{
	static VS_CallConfigCorrector corrector;
	return corrector;
}

bool VS_CallConfigCorrector::IsValidData(const char *data)
{
	VS_CallConfigCorrector checker;

	return checker.UpdateCorrectorDataNoLock(data);
}

bool VS_CallConfigCorrector::LoadDataFromRegistry(void)
{
	lock_guard_t lock(g_lock);
	auto &corrector = VS_CallConfigCorrector::GetInstance();
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	if (!key.IsValid())
		return false;

	// Read data from specified file (for debugging purposes only)
	std::unique_ptr<char, free_deleter> data;
	if (key.GetValue(data, VS_REG_STRING_VT, CFG_CORRECTOR_FILENAME) > 0 && data != nullptr)
	{
		std::string str;
		// read dumb terminals database
#if defined(_WIN32)
		auto fileName_wstring = vs::UTF8ToWideCharConvert(data.get());
		std::ifstream ifs(fileName_wstring);
#else
		std::ifstream ifs(data.get());
#endif
		if (!ifs)
			return false;

		str = std::string((std::istreambuf_iterator<char>(ifs)),
				std::istreambuf_iterator<char>());

		if (str.empty())
			return false;

		return (IsValidData(&str[0]) && corrector.UpdateCorrectorData(&str[0]));
	}

	if (key.GetValue(data, VS_REG_STRING_VT, CFG_CORRECTOR_DATA) > 0 && data != nullptr)
	{
		return IsValidData(data.get()) && corrector.UpdateCorrectorData(data.get());
	}

	return false;
}

uint64_t VS_CallConfigCorrector::GetSecondsCountSinceEpoch(void)
{
	auto current_time = std::chrono::system_clock::now();
	auto current_time_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(current_time.time_since_epoch()).count();

	return current_time_since_epoch;
}

bool VS_CallConfigCorrector::UpdateDataInRegistry(const char *data)
{
	lock_guard_t lock(g_lock);
	if (!VS_CallConfigCorrector::IsValidData(data))
		return false;

	VS_RegistryKey key(false, CONFIGURATION_KEY, false, false);

	return (key.IsValid() &&
		key.SetString(data, CFG_CORRECTOR_DATA)) && // set data
		UpdateTimestamp();
}

bool VS_CallConfigCorrector::IsObsolete(const uint64_t update_timeout_sec)
{
	lock_guard_t lock(g_lock);
	uint64_t timestamp = 0;
	VS_RegistryKey key(false, CONFIGURATION_KEY, true, false);

	if (key.GetValue(&timestamp, sizeof(timestamp), VS_REG_INT64_VT, CFG_CORRECTOR_TIMESTAMP) <= 0)
	{
		timestamp = 0;
		return true;
	}

	return (timestamp + update_timeout_sec) < GetSecondsCountSinceEpoch();
}

bool VS_CallConfigCorrector::UpdateTimestamp(void)
{
	lock_guard_t lock(g_lock);
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, false);
	auto timestamp = GetSecondsCountSinceEpoch();

	return key.IsValid() && key.SetValue(&timestamp, sizeof(timestamp), VS_REG_INT64_VT, CFG_CORRECTOR_TIMESTAMP); // set timestamp
}

bool VS_CallConfigCorrector::GetDataHash(std::string &result)
{
	lock_guard_t lock(g_lock);
	VS_RegistryKey key(false, CONFIGURATION_KEY, true, false);

	std::string data;
	if (!key.GetString(data, CFG_CORRECTOR_DATA))
		return false;

	char md5_buf[32 + 1] = {};
	VS_ConvertToMD5(data, md5_buf);

	result = md5_buf;
	return true;
}

VS_CallConfigCorrector::VS_CallConfigCorrector(void)
{
}

bool VS_CallConfigCorrector::UpdateCorrectorDataNoLock(const char *data)
{
	if (!data) // illegal input data
		return false;

	if (!CreateNewLuaState())
	{
		m_lua = nullptr;
		return false;
	}

	return LoadData(data);
}

bool VS_CallConfigCorrector::UpdateCorrectorData(const char *data)
{
	lock_guard_t lock(g_lock);
	return UpdateCorrectorDataNoLock(data);
}

class LuaCallConfigValueReader :
	public VS_CallConfig::ValueReaderInterface
{
private:
	void GetField(const char *pname)
	{
		auto L = m_lua.get();
		assert(lua_gettop(L) == 1); // table
		assert(lua_checkstack(L, 1));
		lua_getfield(L, -1, pname);
		assert(lua_gettop(L) == 2); // table + value
	}

	void Cleanup(void)
	{
		auto L = m_lua.get();
		// pop value from the stack
		lua_pop(L, 1); // pop value
		assert(lua_gettop(L) == 1); // table
	}

public:
	explicit LuaCallConfigValueReader(VS_CallConfigCorrector::lua_State_ptr &lua)
		: m_lua(lua)
	{
	}

	bool ReadBool(const char *pname, bool &val)
	{
		if (!pname || (!*pname))
		{
			return false;
		}

		auto L = m_lua.get();
		bool res = false;
		GetField(pname);

		if (lua_isboolean(L, -1))
		{
			// handle booleans
			val = lua_toboolean(L, -1) != 0;
			res = true;
		}

		Cleanup();
		return res;
	}

	bool ReadInteger(const char *pname, int32_t &val)
	{
		if (!pname || (!*pname))
		{
			return false;
		}

		auto L = m_lua.get();
		bool res = false;
		GetField(pname);

		if (lua_isinteger(L, -1))
		{
			// handle integer numbers
			val = (int32_t)lua_tointeger(L, -1);
			res = true;
		}
		else if (lua_isnumber(L, -1))
		{
			// handle float numbers
			val = (int32_t)lua_tonumber(L, -1);
			res = true;
		}

		Cleanup();
		return res;
	}

	bool ReadString(const char *pname, std::string &val, bool canBeEmpty = false)
	{
		if (!pname || (!*pname))
		{
			return false;
		}

		auto L = m_lua.get();
		bool res = false;
		GetField(pname);

		if (lua_isstring(L, -1))
		{
			// handle strings
			val = lua_tostring(L, -1);
			res = true;
		}

		Cleanup();
		return res;
	}

	bool ReadProtocolSeq(const char *name, std::vector<net::protocol> &seq)
	{
		std::string str_val;
		if (!name || !*name)
			return false;

		int32_t val = 0;
		if (ReadInteger(name, val))
		{
			switch (val)
			{
			case 4:
				seq.push_back(net::protocol::TLS);
				return true;
			case 3:
				seq.push_back(net::protocol::any);
				return true;
			case 2:
				seq.push_back(net::protocol::UDP);
				return true;
			case 1:
				seq.push_back(net::protocol::TCP);
				return true;
			case 0:
				seq.push_back(net::protocol::none);
				return true;
			default:
				break;
			}
		}
		else if (ReadString(name, str_val))
		{
			return DefaultCallManager::ParseProtocolSeqString(str_val.c_str(), seq);
		}
		return false;
	}

private:
	VS_CallConfigCorrector::lua_State_ptr &m_lua;
};
bool VS_CallConfigCorrector::CorrectCallConfig(VS_CallConfig &res, const VS_CallConfig::eSignalingProtocol protocol, const char *terminal_id)
{
	lock_guard_t lock(g_lock);
	VS_CallConfig correction_data;

	if (!m_lua)
		return false;

	if (protocol != VS_CallConfig::SIP &&
		protocol != VS_CallConfig::RTSP &&
		protocol != VS_CallConfig::H323)
		return false;

	auto L = m_lua.get();

	assert(lua_gettop(L) == 0);

	// Can't find script entry point.
	if (lua_getglobal(L, LUA_ENTRY_FUNCTION) != LUA_TFUNCTION)
	{
		dprint3("Can't find Lua entry function: %s, stack size: %d\n", LUA_ENTRY_FUNCTION, lua_gettop(L));
		lua_pop(L, 1); // pop function name
		assert(lua_gettop(L) == 0);
		return false;
	}

	assert(lua_checkstack(L, 3)); // 2 arguments + result
	// push function arguments
	lua_pushinteger(L, protocol);
	lua_pushstring(L, terminal_id);

	if (lua_pcall(L, 2, 1, 0) != LUA_OK)
	{
		dprint3("Can't call Lua entry function: %s\n", lua_tostring(L, -1));
		lua_pop(L, 1); // pop error data
		assert(lua_gettop(L) == 0);
		return false;
	}

	assert(lua_istable(L, -1) || lua_isnil(L, -1));
	// There is no correction data.
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1); // pop value
		assert(lua_gettop(L) == 0);
		return true;
	}

	// get values from Lua table
	LuaCallConfigValueReader reader(m_lua);
	create_call_config_manager(correction_data).LoadValues(reader);
	//correction_data.LoadValues([this](const char *pname, const VS_CallConfig::eCallConfigValueType type, VS_CallConfig::CallConfigValue &val) -> bool {
	//	return LoadValue(pname, type, val);
	//});

	lua_pop(L, 1); // pop table
	create_call_config_manager(res).MergeWith(correction_data);

	assert(lua_gettop(L) == 0);
	return true;
}
bool VS_CallConfigCorrector::LoadData(const char *data)
{
	if (!data) // illegal input data
	{
		return false;
	}

	if (luaL_dostring(m_lua.get(), data) != LUA_OK) // error - delete Lua state
	{
		dprint3("Can't load Lua script: %s\n", lua_tostring(m_lua.get(), -1));
		m_lua = nullptr;
		return false;
	}

	return true;
}


// Load all Lua 5.3 standard libraries except I/O, OS, and Debug.
// Do not load some Lua libraries into release builds to create safe sandboxed environment.
// See linit.c in the Lua distribution for details.
static const luaL_Reg loadedlibs[] = {
	{ "_G", luaopen_base },
	{ LUA_LOADLIBNAME, luaopen_package },
	{ LUA_COLIBNAME,   luaopen_coroutine },
	{ LUA_STRLIBNAME,  luaopen_string },
	{ LUA_MATHLIBNAME, luaopen_math },
	{ LUA_UTF8LIBNAME, luaopen_utf8 },
#if defined(LUA_COMPAT_BITLIB)
	{ LUA_BITLIBNAME, luaopen_bit32 },
#endif
#ifndef NDEBUG // enable unsafe libraries for debug builds
	{ LUA_DBLIBNAME, luaopen_debug },
	{ LUA_IOLIBNAME, luaopen_io },
	{ LUA_OSLIBNAME, luaopen_os },
#endif
	{ NULL, NULL }
};

typedef struct _Lua_constant
{
	const char *name;
	int value;
} Lua_constant;

static const Lua_constant lua_constants[] = {
	// connection types
	{ "CONNECTIONTYPE_INVALID", CONNECTIONTYPE_INVALID },
	{ "CONNECTIONTYPE_TCP",     CONNECTIONTYPE_TCP     },
	{ "CONNECTIONTYPE_UDP",     CONNECTIONTYPE_UDP     },
	{ "CONNECTIONTYPE_BOTH",    CONNECTIONTYPE_BOTH    },
	{ "CONNECTIONTYPE_TLS",     CONNECTIONTYPE_TLS     },

	{ "SDP_FLOORCTRL_ROLE_INVALID", SDP_FLOORCTRL_ROLE_INVALID },
	{ "SDP_FLOORCTRL_ROLE_C_ONLY",  SDP_FLOORCTRL_ROLE_C_ONLY  },
	{ "SDP_FLOORCTRL_ROLE_S_ONLY",  SDP_FLOORCTRL_ROLE_S_ONLY  },
	{ "SDP_FLOORCTRL_ROLE_C_S",     SDP_FLOORCTRL_ROLE_C_S     },

	// signalling protocols
	{ "SIGNAL_PROTOCOL_SIP",  VS_CallConfig::SIP  },
	{ "SIGNAL_PROTOCOL_RTSP", VS_CallConfig::RTSP },
	{ "SIGNAL_PROTOCOL_H323", VS_CallConfig::H323 },
	// debug build constants
#ifndef NDEBUG
	{ "DEBUG_BUILD", 1}, // we could check for debug builds inside Lua script checking for this variable: if DEBUG_BUILD then ... end
#endif
	{ nullptr, 0 }
};

bool VS_CallConfigCorrector::CreateNewLuaState(void)
{
	m_lua = lua_State_ptr(luaL_newstate());
	auto L = m_lua.get();

	if (!L) // Lua state creation failed
		return false;

	// Load Lua standard libraries.
	// see linit.c for details.
	for (auto lib = loadedlibs; lib->func; lib++) {
		luaL_requiref(L, lib->name, lib->func, 1);
		lua_pop(L, 1);  /* remove library from stack */
	}

	std::string init_script;
	// define some constants in the Lua global environment
	for (auto constant = lua_constants; constant->name; constant++)
	{
		init_script += std::string(constant->name) + " = " + std::to_string(constant->value) + ";\n";
	}

	// define DEFAULT_ENABLED_CODECS
	init_script += "DEFAULT_ENABLED_CODECS = \"" + std::string(DEFAULT_ENABLED_CODECS) + "\";\n";
	// define dummy entry point function. It should be redefined after script loading.
	init_script += std::string("function " LUA_ENTRY_FUNCTION "(protocol, terminal_id) return nil; end\n");

	//puts(init_script.c_str());
	if (luaL_dostring(L, init_script.c_str()) != LUA_OK)
	{
		dprint3("Can't load Lua script: %s\n", lua_tostring(m_lua.get(), -1));
		m_lua = nullptr;
		return false;
	}

	return true;
}
