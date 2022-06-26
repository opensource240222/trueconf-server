#pragma once

#include <cstdint>

enum VS_ACS_Response : uint32_t
{
	vs_acs_next_step,
	vs_acs_connection_is_not_my,
	vs_acs_my_connections,
	vs_acs_free_connections,
	vs_acs_accept_connections
};
// end VS_ACS_Response enum