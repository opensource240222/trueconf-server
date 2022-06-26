/*
* $Revision: 4 $
* $History: VS_OfflineChatService.h $
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 8/10/11    Time: 10:56p
 * Updated in $/VSNA/Servers/VCS/Services
 * - send offline chat though AS (not by Resolve)
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 13.07.11   Time: 17:35
 * Updated in $/VSNA/Servers/VCS/Services
 * - Roaming
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 9.03.11    Time: 20:05
 * Updated in $/VSNA/Servers/VCS/Services
 * offline chat checks license LE_ROAMING_ON
 *
 * *****************  Version 1  *****************
 * User: Ktrushnikov  Date: 9.03.11    Time: 18:28
 * Created in $/VSNA/Servers/VCS/Services
 * added new service OFFLINECHAT_SRV in VCS & BS
*
***********************************************/
#ifndef VS_OFFLINECHAT_SERVICE_H
#define VS_OFFLINECHAT_SERVICE_H

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "../../common/transport/Router/VS_PoolThreadsService.h"
#include "AppServer/Services/VS_Storage.h"
#include "../../common/std/debuglog/VS_Debug.h"

class VS_OfflineChatService
	: public VS_TransportRouterServiceReplyHelper
{
	boost::shared_ptr<VS_ConfRestrictInterface> m_confRestriction;

	unsigned long ROAMING_MSG_PERIOD;							// время в мин, через которое пробовать посылать оффлайн мессаги другому VCS в роуминге
	static const long ROAMING_MSG_PERIOD_MINIMUM = 3;	// Время должно быть больше времени жизни транспортной мессаги, чтобы не было дупов (>2 минут)
	unsigned long m_last_roaming_msg_tick;
public:
	VS_OfflineChatService()
		: ROAMING_MSG_PERIOD(ROAMING_MSG_PERIOD_MINIMUM)
		, m_last_roaming_msg_tick(0)
	{
		m_TimeInterval = std::chrono::seconds(30);
	}
	virtual ~VS_OfflineChatService(void) { }
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	bool Timer(unsigned long ticks) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	void SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface>& confRestrict);

	void ProcessChatLocal(VS_Container& cnt);
	void ProcessChatInRoaming(VS_Container& cnt);
	void SendRoamingMessages();
};

#endif /*VS_OFFLINECHAT_SERVICE_H*/
