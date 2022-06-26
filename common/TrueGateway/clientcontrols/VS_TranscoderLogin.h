#pragma once

#include <cstdint>

#include <map>
#include <string>
#include <memory>
#include <chrono>

#include "std/cpplib/VS_Lock.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/attributes.h"
#include "std-generic/compat/map.h"
#include "std-generic/compat/functional.h"

class VS_TranscoderLogin
{
public:
	~VS_TranscoderLogin();

	std::string GenerateTranscoderPass(string_view login, std::chrono::seconds timeoutSec = std::chrono::seconds(60));

	bool Login(string_view login, string_view md5pass);

private:
	const size_t DEFAULT_PASS_LEN = 64;

	typedef struct _LoginData {
		std::string password;
		uint64_t    login_exceeding_time;
	} LoginData;

protected:
	VS_TranscoderLogin();

private:
	typedef vs::fast_mutex mutex_t;

	bool IsValidLogin(string_view login);
	uint64_t GetSecondsCountSinceEpoch(void);
	void CleanExpiredLogins(void);

	static std::string GeneratePassword(const size_t len);
private:
	mutex_t m_mutex;
	vs::map<std::string, LoginData, vs::str_less> m_logins;
};
