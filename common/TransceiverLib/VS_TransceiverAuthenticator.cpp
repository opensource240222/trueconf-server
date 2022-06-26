#include "VS_TransceiverAuthenticator.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/VS_AuthUtils.h"

std::string MakeTransceiverPath(string_view transceiverName) {
	std::string path = TRANSCEIVERS_KEY; path += '\\'; path += transceiverName;
	return path;
}

std::string auth::Transceiver::ReadSharedKey(string_view transceiverName) {
	std::string path = MakeTransceiverPath(transceiverName);
	VS_RegistryKey trans_key(false, path);

	std::string result;
	trans_key.GetString(result, TRANSCEIVER_SHARED_KEY_TAG);
	return result;
}

bool auth::Transceiver::SaveSharedKey(string_view transceiverName, const char* key)
{
	if (!key) return false;
	std::string path = MakeTransceiverPath(transceiverName);
	VS_RegistryKey cfg_key(false, path, false, true);
	return cfg_key.SetString(key, TRANSCEIVER_SHARED_KEY_TAG);
}

void auth::Transceiver::CleanSharedKey(string_view transceiverName)
{
	std::string path = TRANSCEIVERS_KEY;
	VS_RegistryKey cfg_key(false, path, false);
	cfg_key.RemoveKey(transceiverName);
}

string_view auth::Transceiver::GetLoginFromHandshake(const string_view input) {
	auto timestamp_pos = input.find_first_of('*');
	auto login_pos = input.find_first_of('*', timestamp_pos + 1);
	auto login_end = input.find_first_of('*', login_pos + 1);

	++login_pos;
	if (login_pos == std::string::npos) return {};
	if (login_end == std::string::npos) return {};

	return input.substr(login_pos, login_end - login_pos);
}

bool auth::Transceiver::AuthConnection(const unsigned char * data, const unsigned long data_sz)
{
	if (!data || data_sz == 0) return false;
	auto inputdata_sv = string_view((const char*)data, data_sz);

	auto login = GetLoginFromHandshake(inputdata_sv);
	if (login.empty()) return false;

	auto shared_key = ReadSharedKey(login);
	auto res = !shared_key.empty() && CheckTempPassword(inputdata_sv,shared_key);

	if (res) m_authenticatedName = static_cast<std::string>(login);
	return res;
}

const std::string & auth::Transceiver::GetAuthenticatedName() const
{
	return m_authenticatedName;
}
