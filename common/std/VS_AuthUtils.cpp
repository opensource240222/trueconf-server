#include "VS_AuthUtils.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/md5.h"

#include <chrono>

const unsigned int c_MD5_HASH_SIZE = 32;

std::string auth::MakeTempPassword(const string_view secret, const string_view login)
{
	std::string res; res.reserve(c_MD5_HASH_SIZE * 3);

	char rand[c_MD5_HASH_SIZE + 1] = {};
	VS_GenKeyByMD5(rand);

	auto timestamp = std::to_string(std::chrono::system_clock::to_time_t((std::chrono::system_clock::now() + std::chrono::seconds(10))));

	MD5 md5;
	md5.Update(rand, c_MD5_HASH_SIZE);
	md5.Update(timestamp);
	md5.Update(login);
	md5.Update(secret);
	md5.Final();
	char sign[c_MD5_HASH_SIZE + 1];
	md5.GetString(sign);

	// [RAND]*[TIMESTAMP]*[LOGIN]*md5([RAND]+[TIMESTAMP]+[LOGIN]+[SECRET])
	res += rand; res += '*'; res += timestamp; res += '*'; res += login; res += '*'; res += sign;
	return res;
}

// Password format:[RAND]*[TIMESTAMP]*[LOGIN]*md5([RAND]+[TIMESTAMP]+[LOGIN]+[SECRET])
bool auth::CheckTempPassword(const string_view password, const string_view secret)
{
	auto rand_end = password.find_first_of('*');
	if (rand_end == string_view::npos) return false;

	auto timestamp_end = password.find_first_of('*', rand_end + 1);
	if (timestamp_end == string_view::npos) return false;

	auto login_end = password.find_first_of('*', timestamp_end + 1);
	if (login_end == string_view::npos) return false;

	std::string timestamp(password.substr(rand_end + 1, timestamp_end - rand_end - 1));
	auto t = std::chrono::system_clock::from_time_t(strtoll(timestamp.c_str(), nullptr, 10));
	if(t < std::chrono::system_clock::now()) return false; // password expired

	auto rand = password.substr(0, rand_end);
	auto login = password.substr(timestamp_end + 1, login_end - timestamp_end - 1);
	auto arived_sign = password.substr(login_end + 1, password.length() - login_end - 1);

	MD5 md5;
	md5.Update(rand);
	md5.Update(timestamp);
	md5.Update(login);
	md5.Update(secret);
	md5.Final();
	char sign[c_MD5_HASH_SIZE + 1];
	md5.GetString(sign);

	return sign == arived_sign.substr(0, c_MD5_HASH_SIZE);
}
