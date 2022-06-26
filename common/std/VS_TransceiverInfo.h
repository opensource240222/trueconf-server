#pragma once

#include <string>

/* transceiver */
namespace ts {
	extern const char NAME_OPTION_TAG[];
	extern const char REGISTRY_BACKEND_OPTION_TAG[];
	extern const char ROOT_OPTION_TAG[];
	extern const char SERVER_ADDRESS_OPTION_TAG[];
	extern const char SERVER_ENDPOINT_OPTION_TAG[];

	extern const char DEBUG_OPTION_TAG[];
	extern const char DEBUG_MODULES_OPTION_TAG[];
	extern const char LOG_DIRECTORY_OPTION_TAG[];

	extern const char DEFAULT_CONFIG_FILE[];
	extern const char DEFAULT_REG_KEY_ROOT[];
	extern const char PROCESS_EXE_NAME[];
	extern const char LOG_DIRECTORY_NAME[];

	const char RTSP_SECRET_HEADER[] = "X-TrueConf-Secret";

	bool UseLocalTransceiver();
}