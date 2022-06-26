#include "ConvertResponse.h"

acs::Response acs::ConvertResponseCode(const VS_ACS_Response r)
{
	switch (r)
	{
	case	vs_acs_next_step: return acs::Response::next_step;
	case	vs_acs_my_connections: return acs::Response::my_connection;
	case	vs_acs_accept_connections: return acs::Response::accept_connection;
	case	vs_acs_free_connections:
	case	vs_acs_connection_is_not_my:
	default:
		return acs::Response::not_my_connection;
	}
}
