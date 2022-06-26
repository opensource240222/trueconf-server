/**
 *************************************************
 * $Revision: 7 $
 * $History: VS_ConfigurationService.h $
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 5.05.09    Time: 18:33
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - cache for ep props
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 17.02.08   Time: 10:58
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - HardwareTest: fix output cause of funcs have other names
 * - Stored Procedure: call SetAllEpProperties() for known params
 * - split Capabilities into: is_audio, is_mic, is_camera
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 13.02.08   Time: 13:43
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - properties corrected
 * - sturtup sequence of server connects rewrited
 * - code redused
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 12.02.08   Time: 19:13
 * Updated in $/VSNA/Servers/BaseServer/Services
 * GetAppProperties method realized
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 12.12.07   Time: 20:40
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fixed init
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 5.12.07    Time: 11:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 * BS - new iteration
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 28.11.07   Time: 18:33
 * Created in $/VSNA/Servers/BaseServer/Services
 * first bs ver
 *
 *************************************************/

#ifndef VS_CONFIGURATION_SERVICE_H
#define VS_CONFIGURATION_SERVICE_H

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../../common/std/cpplib/VS_Map.h"
#include "transport/VS_SystemMessage.h"
#include "../../ServerServices/Common.h"

class VS_ConfigurationService :
	public VS_TransportRouterServiceReplyHelper,
	public VS_SystemMessage
{
	VS_Map	m_propcache;
public:
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	// Service implementation
	void GetAllAppProperties_Method();
	void SetEpProperties_Method(VS_Container &cnt);
};

#endif
