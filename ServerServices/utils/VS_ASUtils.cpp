#include "VS_ASUtils.h"

#include "TrueGateway/VS_GatewayStarter.h"
#include "statuslib/VS_CallIDInfo.h"
#include "AppServer/Services/VS_PresenceService.h"

namespace utils
{
	void init_gateway_ext_funcs_for_AS(boost::weak_ptr<VS_PresenceService> presence) noexcept
	{
		VS_GatewayStarter::SetGetUserStatusFunction([presence_weak = presence](string_view call_id, bool use_cache, bool do_ext_resolve)
		{
			auto presense = presence_weak.lock();
			UserStatusInfo res;
			if (!presense)
				return res;
			VS_SimpleStr call_id_ret(call_id.size(), call_id.data());
			VS_CallIDInfo ci;
			presense->Resolve(call_id_ret, ci, use_cache, nullptr, do_ext_resolve);
			UserStatusInfo::User *user = boost::get<UserStatusInfo::User>(&res.info);
			assert(user);
			user->status = ci.m_status;
			user->extanded_status = ci.m_extStatusStorage;
			user->home_server = ci.m_homeServer;
			user->server = ci.m_serverID;
			return res;
		}
		);
	}
} //namespace utils