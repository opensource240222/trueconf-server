/****************************************************************************
 * (c) 2007 Visicron Systems, Inc.  http://www.visicron.net/
 * $Revision: 5 $
 * $History: VS_LocatorService.h $
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 21.12.11   Time: 18:43
 * Updated in $/VSNA/Servers/BaseServer/Services
 * LocatorSRV: reconnect to PostgreSQL
 * - ProcessDBCall() added
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 11.06.08   Time: 18:45
 * Updated in $/VSNA/Servers/BaseServer/Services
 * additional logging ProcessCOMError added
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 19.02.08   Time: 21:04
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - auto connect to sm
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 12.12.07   Time: 20:40
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fixed init
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 5.12.07    Time: 11:09
 * Created in $/VSNA/Servers/BaseServer/Services
 * BS - new iteration
 *
 ***********************************************/
#ifndef VS_LOCATOR_SERVICE_H
#define VS_LOCATOR_SERVICE_H

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"

class VS_LocatorService :
	public VS_TransportRouterServiceHelper
{

	VS_SimpleStr	m_SM;
	bool			m_IsSmOnline;
	unsigned long	m_SmLastCheckTime;

public:
	VS_LocatorService(void);
	virtual ~VS_LocatorService(void) { }
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll) override;
	bool Timer(unsigned long tickcount) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

    // VS_EndpointConditions implementation
	bool OnPointDisconnected_Event(const VS_PointParams* prm) override;
	bool OnPointConnected_Event(const VS_PointParams* prm) override;
};

#endif /*VS_LOCATOR_SERVICE_H*/
