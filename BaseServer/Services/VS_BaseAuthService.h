/*************************************************
 * $Revision: 2 $
 * $History: VS_BaseAuthService.h $
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 10.08.10   Time: 21:28
 * Updated in $/VSNA/Servers/BaseServer/Services
 * [#7561]: GenerateSessionKey() at LoginUser and at ReqUpdateAccount()
 * [#7562]: update login session key at timer timeout in Visicron.dll
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 6.12.07    Time: 18:51
 * Created in $/VSNA/Servers/BaseServer/Services
 * base services done
 *
 ************************************************/

#ifndef VS_BASE_AUTH_SERVICE_H
#define VS_BASE_AUTH_SERVICE_H

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "../../ServerServices/Common.h" /// for id
#include "storage/VS_DBStorage.h"

class VS_BaseAuthService :
	public VS_TransportRouterServiceReplyHelper
{
	VS_SimpleStr m_login_session_secret;
	VS_SimpleStr GenerateSessionKey(const char* call_id);
	bool SerializeExternalAccounts(const std::vector<VS_ExternalAccount>& v, VS_Container& cnt);

public:
	VS_BaseAuthService(void) { }
	virtual ~VS_BaseAuthService(void) { }
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	void LoginUser_Method (VS_Container& cnt);
	void ReqUpdateAccount_Method(VS_Container& cnt);
	void SetRegID_Method(VS_Container& cnt);
	void UpdatePeerCfg_Method(VS_Container& cnt);
};

#endif // VS_BASE_AUTH_SERVICE_H