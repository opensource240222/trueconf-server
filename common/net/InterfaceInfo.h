#pragma once

#include "Types.h"
#include "std-generic/attributes.h"

#include <memory>
#include <vector>
#include <string>

namespace net {

struct interface_info
{
	std::string name;
	std::vector<address_v4> addr_local_v4;
	std::vector<address_v6> addr_local_v6;
	uint32_t index;
};
typedef std::shared_ptr<const std::vector<interface_info>> interface_info_list;
VS_NODISCARD interface_info_list GetInterfaceInfo(bool update = false);

}
