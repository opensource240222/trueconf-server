#include "VS_CallConfig.h"
#include "std/cpplib/lua_deleters.h"

#include <memory>
#include <mutex>
#include <cstdint>
#include <string>

#include <lua.hpp>

class VS_CallConfigCorrector {
public:
	typedef std::unique_ptr<lua_State, lua_State_deleter> lua_State_ptr;

public:
	static VS_CallConfigCorrector &GetInstance(void);

	static bool IsValidData(const char *data);
	static bool IsObsolete(const uint64_t update_timeout_sec);

	static bool LoadDataFromRegistry(void);
	static bool UpdateDataInRegistry(const char *data);
	static bool UpdateTimestamp(void);
	static bool GetDataHash(std::string &result);

	bool UpdateCorrectorData(const char *data);
	bool CorrectCallConfig(VS_CallConfig &res, const VS_CallConfig::eSignalingProtocol protocol, const char *terminal_id);
	// non copyable
	VS_CallConfigCorrector(VS_CallConfigCorrector const &) = delete;
	void operator=(VS_CallConfigCorrector const &) = delete;
private:
	// Do not create new instances of this object directly - this is a singleton object.
	// Use VS_CallConfigCorrector::GetInstance() to access object.
	VS_CallConfigCorrector(void);

	static uint64_t GetSecondsCountSinceEpoch(void);

	bool CreateNewLuaState(void);
	bool UpdateCorrectorDataNoLock(const char *data); // for data validation
	bool LoadData(const char *data);

private:
	static std::recursive_mutex g_lock;

private:
	lua_State_ptr m_lua;
};
