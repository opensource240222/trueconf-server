#pragma once

#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "LicenseLib/VS_LicenseSharer.h"
#include "std/cpplib/VS_UserData.h"

#include <queue>
#include<boost/variant.hpp>

enum class ExpandLicenseOnLoginStep : int32_t {
	RequestLicense,
	ReceiveLicense
};

enum VS_UserLoggedin_Result : int32_t;
enum VS_ClientType : int32_t;
enum OnUserChangeType : int32_t;

using UserCtx = std::map<std::string, boost::variant<std::string, VS_Container, int32_t, LoginType, VS_ClientType, VS_UserData, uint64_t>>;

extern const char LOGIN_TYPE_TAG[];
extern const char ADD_STRING_TAG[];
extern const char ARRIVED_INFO_TAG[];
extern const char SRC_CID_TAG[];
extern const char SRC_USER_TAG[];
extern const char USER_DATA_TAG[];

/* license */
namespace lic {
	class ShareService : public virtual VS_TransportRouterServiceHelper
	{
	public:
		bool Processing(std::unique_ptr<VS_RouterMessage> &&recvMess) override;
		bool Timer(unsigned long tickcount) override;
		bool Init();
		void ExpandLicenseOnLogin(UserCtx && userCtx, const VS_Container *license_response, const ExpandLicenseOnLoginStep step);
		virtual VS_UserLoggedin_Result LoginUser(bool is_allowed, VS_Container &cnt, VS_SimpleStr &cid, const char* user, const char* addString) = 0;
		virtual VS_UserLoggedin_Result AuthorizedLogin(const char *cid, VS_UserData &ud, const VS_SimpleStr& autoKey, VS_Container& prop_cnt, const VS_ClientType client_type, const std::string& addString) = 0;
		virtual void LogoutUser_Method(const vs_user_id& user, OnUserChangeType type) = 0;

		void OnPointConnected_Method(const VS_Container &cnt);
		void OnPointDisconnected_Method(const VS_Container &cnt);
		void OnReturnLicenseByForce_Method(const VS_Container &cnt, const string_view from);
		void OnMasterChanged_Event();

	private:
		void ClearDelayedLogins();
		void ClearDelayedLogin(UserCtx && userCtx);
		void ClearDelayedLogin(uint64_t licenseID);
		bool IsLicenseMasterActive();
		void ClearOverhead(VS_License &to_clear);

		Sharer	m_license_sharer;
		std::deque<UserCtx> m_delayed_logins;
		unsigned int	m_licReqId = 1;
		bool m_lic_share_req_answered = true;
		std::chrono::steady_clock::time_point m_last_lic_share_req_time;
		std::chrono::steady_clock::time_point m_last_lic_check_query;
	};
}