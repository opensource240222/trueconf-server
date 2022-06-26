#include "VS_TransceiverInfo.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std-generic/clib/strcasecmp.h"

namespace ts {
	const char NAME_OPTION_TAG[] = "Name";
	const char REGISTRY_BACKEND_OPTION_TAG[] = "RegistryBackend";
	const char ROOT_OPTION_TAG[] = "Root";
	const char SERVER_ADDRESS_OPTION_TAG[] = "ServerAddress";
	const char SERVER_ENDPOINT_OPTION_TAG[] = "ServerEndpoint";

	const char DEBUG_OPTION_TAG[] = "Debug";
	const char DEBUG_MODULES_OPTION_TAG[] = "DebugModules";
	const char LOG_DIRECTORY_OPTION_TAG[] = "LogDirectory";

	const char DEFAULT_CONFIG_FILE[] = "tc_transceiver.cfg";
	const char DEFAULT_REG_KEY_ROOT[] = "TrueConf\\Server";
#ifdef _WIN32
	const char PROCESS_EXE_NAME[] = "tc_transceiver.exe";
#else
	const char PROCESS_EXE_NAME[] = "tc_transceiver";
#endif
	const char LOG_DIRECTORY_NAME[] = "transceiver_logs";

}

bool ts::UseLocalTransceiver()
{
	std::string value;
	VS_RegistryKey(false, CONFIGURATION_KEY).GetString(value, TRANSCEIVER_LOCATION);
	return strcasecmp(value.c_str(), "remote") != 0;
}