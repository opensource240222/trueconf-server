#pragma once
#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "transport/Router/VS_PoolThreadsService.h"
#include "ServerServices/Common.h" /// for id
#include "transport/VS_SystemMessage.h"
#include "AppServer/Services/VS_AppServerData.h"
#include "AppServer/Services/VS_PresenceService.h"
#include "std/cpplib/VS_Policy.h"
#include "std/cpplib/VS_UserData.h"
#include "std-generic/compat/memory.h"
#include "VS_LicenseShareService.h"

#include "VS_InvitesStorage.h"

#include <vector>
#include <algorithm>

#include "boost/asio/io_service.hpp"
#include <boost/asio/strand.hpp>
#include <boost/signals2.hpp>

const unsigned long nAuthLogin		= 10;
const unsigned long LoginBarrier	= 10*1000;	// 10 sec
const unsigned long noBSbarrier		= 60*1000;	// 60 sec

class VS_VCSAuthService :
	virtual public VS_TransportRouterServiceReplyHelper,
	public VS_SystemMessage,
	public lic::ShareService,
	public vs::enable_shared_from_this<VS_VCSAuthService>
	, public VS_PresenceServiceMember
{
	boost::asio::io_service::strand		m_strand;
	std::atomic<bool>		m_is_stopping{ false };
	VS_SimpleStr			m_login_session_secret;

	// login stat params
	std::vector<int>	m_loginTime;
	long				m_login_stat[nAuthLogin];
	long				m_login_count;
	long				m_avetime;
	VS_InvitesStorage	m_invites_storage;
	VS_Policy m_policy;

	void OnUserLoginEnd_Event(const VS_UserData& ud, const std::string &cid);
	void OnUserLogoff_Event(const VS_UserData& ud, const std::string &cid);
private:
	VS_UserLoggedin_Result LoginUser(bool is_allowed, VS_Container &cnt, VS_SimpleStr &cid, const char* user, const char* addString);
	void TrySendInvites();
public:
	virtual ~VS_VCSAuthService(void) {
		m_OnUserLoggedIn.disconnect_all_slots();
		m_OnUserLoggedOut.disconnect_all_slots();
	}

	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	void AsyncDestroy() override;
	bool Timer(unsigned long ticks) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	bool AddSessionKey(const char* call_id, VS_Container& cnt);

	void LoginUser_Method (VS_Container& cnt);
	void LogoutUser_Method (const vs_user_id& user, OnUserChangeType type);

	void CheckUserLoginStatus_Method();

	void OnPointConnected_Method(const VS_Container &cnt);
	void OnPointDisconnected_Method(const VS_Container &cnt);
	bool OnPointConnected_Event(const VS_PointParams* prm) override;
	bool OnPointDisconnected_Event(const VS_PointParams* prm) override;

	void Authorize_Method(const char* cid, VS_Container* in_cnt);

	VS_UserLoggedin_Result AuthorizedLogin(const char *cid, VS_UserData &ud, const VS_SimpleStr& autoKey, VS_Container& prop_cnt, const VS_ClientType client_type, const std::string& addString);
	void OnUserChange_Method(const char* user_, const OnUserChangeType type, const char* pass);

	void OnAddressBookChange_Method( VS_Container& cnt );
	void ReqUpdateAccount_Method(VS_Container& cnt);

	void AutoInvite_Method(VS_Container &cnt);
	void SetRegID_Method(VS_Container& cnt);
	void InviteUsers_Method(VS_Container & cnt);

#ifdef _SVKS_M_BUILD_
	void AuthByECP_Method(const char *cid, VS_Container& cnt);
#endif

	boost::signals2::signal< void (const VS_UserData &ud, const std::string &cid) > m_OnUserLoggedIn;
	boost::signals2::signal< void (const VS_UserData &ud, const std::string &cid) > m_OnUserLoggedOut;

protected:
	VS_VCSAuthService(boost::asio::io_service &io_service) :
		m_strand(io_service), m_login_count(0), m_avetime(0), m_policy("TC")
	{
		m_TimeInterval = std::chrono::seconds(10);
		memset(m_login_stat, 0, sizeof(m_login_stat)); m_loginTime.reserve(1024);
	}
};