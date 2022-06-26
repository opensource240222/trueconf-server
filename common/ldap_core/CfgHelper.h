#pragma once

#include "VS_LDAPConst.h"
#include "VS_LDAPCore.h"

#include <set>

class VS_RegistryKey;

namespace tc {

	bool ReadCfg(VS_RegistryKey& cfg, tc::cfg_params_t& params);
	bool ReadCfg(tc::cfg_params_t& params);

}  // namespace tc