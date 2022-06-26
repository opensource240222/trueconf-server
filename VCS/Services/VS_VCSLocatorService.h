/****************************************************************************
 * (c) 2007 Visicron Systems, Inc.  http://www.visicron.net/
 * $Revision: 1 $
 * $History: VS_VCSLocatorService.h $
 *
 * *****************  Version 1  *****************
 * User: Ktrushnikov  Date: 23.02.10   Time: 14:40
 * Created in $/VSNA/Servers/VCS/Services
 * VCS:
 * - Services added: SMTP_MAILER, LOCATOR, LOG (two last to make work
 * smtp_mailer)
 * - OnUserChange(): changes come from configurator vs_bc.dll
 * vs_bc.dll:
 * - fix params (broker) where to send updates
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
#ifndef VS_VCS_LOCATOR_SERVICE_H
#define VS_VCS_LOCATOR_SERVICE_H

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"

class VS_VCSLocatorService :
	public VS_TransportRouterServiceHelper
{
public:
	VS_VCSLocatorService(void) { };
	virtual ~VS_VCSLocatorService(void) { }
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;
};

#endif /*VS_VCS_LOCATOR_SERVICE_H*/
