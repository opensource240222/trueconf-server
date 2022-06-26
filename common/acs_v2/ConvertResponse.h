#pragma once

#include "acs/AccessConnectionSystem/Responses.h"
#include "acs_v2/Responses.h"

namespace acs {
	Response ConvertResponseCode(const VS_ACS_Response r);
}