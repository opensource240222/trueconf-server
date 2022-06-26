#include "TransceiverConfiguration.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"

const unsigned c_defaultMinFreeTransceivers = 2;
const unsigned c_defaultMaxConfsByTransceiver = 1;
const std::chrono::minutes c_defaultMaxFreeTimeForTransceiver(30);

unsigned ts::GetMinAvailableTransceivers()
{
	VS_RegistryKey transceivers(false, TRANSCEIVERS_KEY);
	if (!transceivers.IsValid()) return c_defaultMinFreeTransceivers;

	int32_t val(0);
	transceivers.GetValue(&val, sizeof(val), VS_REG_INTEGER_VT, MIN_FREE_TRANSCEIVERS_TAG);
	return val > 0 ? static_cast<unsigned>(val) : c_defaultMinFreeTransceivers;
}

unsigned ts::GetMaxConferencesByOneTransceiver()
{
	VS_RegistryKey transceivers(false, TRANSCEIVERS_KEY);
	if (!transceivers.IsValid()) return c_defaultMaxConfsByTransceiver;

	int32_t val(0);
	transceivers.GetValue(&val, sizeof(val), VS_REG_INTEGER_VT, MAX_CONFS_BY_TRANSCEIVER_TAG);
	return val > 0 ? static_cast<unsigned>(val) : c_defaultMaxConfsByTransceiver;
}

std::chrono::minutes ts::GetMaxFreeTimeForTransceiver()
{
	VS_RegistryKey transceivers(false, TRANSCEIVERS_KEY);
	if (!transceivers.IsValid()) return c_defaultMaxFreeTimeForTransceiver;

	int32_t val(0);
	transceivers.GetValue(&val, sizeof(val), VS_REG_INTEGER_VT, MAX_TRANSCEIVER_FREE_TIME_MINUTES);
	return  val > 0 ? std::chrono::minutes(val) : c_defaultMaxFreeTimeForTransceiver;
}

void ts::DeleteTransceiversKeys()
{
	VS_RegistryKey transceivers(false, TRANSCEIVERS_KEY,false);
	if (!transceivers.IsValid()) return;

	VS_RegistryKey subkey;
	transceivers.ResetKey();
	while (transceivers.NextKey(subkey, false)) {
		auto name = subkey.GetName();
		if (!name) continue;

		transceivers.RemoveKey(name);
	}
}

uint32_t ts::GetMaxTransceivers() {
	VS_RegistryKey transceivers(false, TRANSCEIVERS_KEY);
	if (!transceivers.IsValid()) return true;

	uint32_t val(~0);
	transceivers.GetValue(&val, sizeof(val), VS_REG_INTEGER_VT, MAX_TRANSCEIVERS);
	return  val;
}
