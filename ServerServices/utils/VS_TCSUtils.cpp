#include "VS_TCSUtils.h"

#include "BaseServer/Services/storage/VS_DBStorageInterface.h"
#include "TrueGateway/VS_GatewayStarter.h"
#include "AppServer/Services/VS_PresenceService.h"

#include "std/cpplib/VS_CallIDUtils.h"
#include "std/cpplib/VS_RegistryKey.h"

namespace utils
{
	void init_gateway_ext_funcs_for_TCS(boost::weak_ptr<VS_PresenceService> presence) noexcept
	{
		VS_GatewayStarter::SetGetUserStatusFunction([presence_weak = std::move(presence)](string_view call_id, bool use_cache, bool do_ext_resolve) {

			UserStatusInfo res;
			bool is_group_conf = false;
			bool is_default_dest = false;
			auto name = VS_GetConfNameByCallID(call_id, is_group_conf, is_default_dest);

			auto presense = presence_weak.lock();
			if (!presense)
				return res;

			auto conf_resctrict = presense->GetConfRestrict();

			assert(conf_resctrict);

			if (!is_group_conf)
			{
				if (name.empty())
					return res;

				UserStatusInfo::User info;

				const auto presense_func = [&presense, &res, do_ext_resolve, use_cache](string_view call_id, UserStatusInfo::User &info)
				{
					std::string call_id_ret(call_id);
					VS_CallIDInfo ci;
					VS_SimpleStr callIDRet(call_id_ret.c_str());
					presense->Resolve(callIDRet, ci, use_cache, nullptr, do_ext_resolve);
					res.real_id = !ci.m_realID.empty() ? ci.m_realID : call_id_ret;
					info.status = ci.m_status;
					info.extanded_status = ci.m_extStatusStorage;
					info.home_server = ci.m_homeServer;
					info.server = ci.m_serverID;
				};

				if (!is_default_dest)
				{
					presense_func(call_id, info);

					if (info.status < USER_AVAIL)
					{
						info.status = USER_STATUS_UNDEF;

						VS_StorageUserData user;
						if (conf_resctrict->FindUser(name.c_str(), user))
							info.status = USER_LOGOFF;
					}
				}
				else
				{
					VS_StorageUserData user;
					if (conf_resctrict->FindUser(name.c_str(), user))
					{
						call_id = user.m_realLogin.GetID();

						presense_func(call_id, info);

						if (info.status < USER_AVAIL)
							info.status = USER_LOGOFF;
					}
				}

				VS_ParticipantDescription pd;
				if (g_storage->FindParticipant(res.real_id, pd) && !pd.m_conf_id.IsEmpty())
					res.confStreamID = pd.m_conf_id;

				res.info = std::move(info);
			}
			else
			{
				bool conf_exist{ false };
				if (!name.empty())
				{
					VS_ConferenceDescription cd;
					VS_Container cnt;
					if (conf_resctrict->FindMultiConference(name.c_str(), cd, cnt, {}) == 0)
					{
						assert(cd.m_name.m_str != nullptr);
						conf_exist = true;
						res.real_id = std::string(call_id);
						res.confStreamID = cd.m_name.m_str;
					}
				}
				res.info = UserStatusInfo::Conf{ conf_exist };
			}

			return res;
		});

		VS_GatewayStarter::SetGetAppPropertyFunciton([](string_view prop_name) -> std::string {
			std::string result;
			VS_RegistryKey(false, "AppProperties").GetString(result, std::string(prop_name).c_str());
			return result;
		});

		VS_GatewayStarter::SetGetWebManagerPropertyFunciton([](string_view prop_name) -> std::string {
			auto dbStorage = g_dbStorage;
			if (!dbStorage)
				return std::string();
			std::string val;
			VS_SimpleStr name(std::string(prop_name).c_str());
			dbStorage->GetWebManagerProperty(name, val);
			return val;
		});
	}
} // namespace utils