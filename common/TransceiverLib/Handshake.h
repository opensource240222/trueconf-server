#pragma once

#include "acs/AccessConnectionSystem/Responses.h"

/* transceiver */
namespace ts {
	VS_ACS_Response ProtocolHandshake(const void *in_buffer, unsigned long *in_len);
}