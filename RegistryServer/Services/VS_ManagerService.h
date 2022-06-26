/**
 ****************************************************************************
 * $Revision: 1 $
 * $History: VS_ManagerService.h $
 *
 * *****************  Version 1  *****************
 * User: Ktrushnikov  Date: 30.09.10   Time: 15:33
 * Created in $/VSNA/Servers/RegistryServer/Services
 * RegServer
 * - MANAGER_SRV added
 * - Post LogStats_Method to REGISTRATION_SRV
 *
 * *****************  Version 1  *****************
 * User: ktrushnikov        Date: 30.09.10   Time: 14:50
 * Created in $/VS/Servers/DirectoryServer/Services
 *
 ****************************************************************************/


#ifndef VS_MANAGER_SERVICE_H
#define VS_MANAGER_SERVICE_H

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"

class VS_ManagerService :
	public VS_TransportRouterServiceReplyHelper
{
public:
    VS_ManagerService(void);
    ~VS_ManagerService(void);

    bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll) override;
    bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;
};

#endif
