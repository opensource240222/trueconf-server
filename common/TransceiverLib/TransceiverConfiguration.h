#pragma once

#include <chrono>

/* transceiver */
namespace ts {
	unsigned GetMinAvailableTransceivers();
	unsigned GetMaxConferencesByOneTransceiver();
	std::chrono::minutes GetMaxFreeTimeForTransceiver();
	void DeleteTransceiversKeys();
	uint32_t GetMaxTransceivers();
}