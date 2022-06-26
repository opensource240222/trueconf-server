#pragma once

#include <boost/weak_ptr.hpp>

class VS_PresenceService;

namespace utils
{
	void init_gateway_ext_funcs_for_TCS(boost::weak_ptr<VS_PresenceService> presence) noexcept;
} // namespace utils