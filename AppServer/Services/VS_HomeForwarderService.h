/****************************************************************************
 * (c) 2007 Visicron Systems, Inc.  http://www.visicron.net/
 * $Revision: 8 $
 * $History: VS_HomeForwarderService.h $
 *
 * *****************  Version 8  *****************
 * User: Ktrushnikov  Date: 26.10.11   Time: 13:46
 * Updated in $/VSNA/Servers/AppServer/Services
 * - long time for AddToAddressBook in roaming between service
 *
 * *****************  Version 7  *****************
 * User: Ktrushnikov  Date: 9.08.11    Time: 17:21
 * Updated in $/VSNA/Servers/AppServer/Services
 * - VS_DNSGetASForDomain: AddToAB, PERSON_DETAILS, offline chat
 * - Roaming_GetDN in offline mode (ask AS->BS instead of RS)
 * - AddToAB: try FindUser() to solve alias problem in trueconf.com
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 15.12.10   Time: 16:00
 * Updated in $/VSNA/Servers/AppServer/Services
 * - conf stat to file in AS (was onlyin VCS)
 * - "Save Conf Stat" dword reg key added
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 3.09.09    Time: 12:03
 * Updated in $/VSNA/Servers/AppServer/Services
 * - AS abooks moved to VS_Storage
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 3.09.09    Time: 11:38
 * Updated in $/VSNA/Servers/AppServer/Services
 * Save abooks at AS:
 *   - VS_Container: GetLongValueRef() added
 *   - TRANSPORT_SRCUSER_PARAM: pass user from Transport to Container
 *
 * *****************  Version 3  *****************
 * User: Ktrushnikov  Date: 4.03.09    Time: 21:09
 * Updated in $/VSNA/Servers/AppServer/Services
 * no BS implementation:
 * - AS login user if have previous cached answer from BS
 * - BS add SERVER_PARAM = OurEndpoint() to rCnt
 * - HomeFWDSrv answers by itself if no BS and mess from ABOOK (emulate
 * SearchAB with "no changes" result)
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 12.12.07   Time: 20:40
 * Updated in $/VSNA/Servers/AppServer/Services
 * fixed init
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 12.12.07   Time: 18:11
 * Created in $/VSNA/Servers/AppServer/Services
 * fixes
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 27.11.07   Time: 18:54
 * Created in $/VSNA/Servers/AppServer/Services
 ***********************************************/
#ifndef VS_HOMEFORWARDER_SERVICE_H
#define VS_HOMEFORWARDER_SERVICE_H

#include "../../common/transport/Router/VS_TransportRouterServiceHelper.h"
#include "AppServer/Services/VS_Storage.h"
#include "../../common/std/cpplib/VS_Map.h"
#include "../../ServerServices/VS_FileConfStat.h"


class VS_HomeForwarderService
	  : public VS_TransportRouterServiceReplyHelper
	  , public VS_FileConfStat
{


	void ProcessNoBS(VS_Container& in_cnt, const char* callId);

public:
	VS_HomeForwarderService(void) { }
	virtual ~VS_HomeForwarderService(void) { }
	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	bool TryProcessBoooksForAS(const void* body, unsigned long bodySize, VS_SimpleStr &homeServer);
	void ProcessCntFromBS(VS_Container& cnt);
	void ProcessCntFromUser(const char* from_user, VS_Container& cnt);

	void ProcessConfStat(const void* body, unsigned long bodySize);
	bool TryForwardToBS(const void* body, unsigned long bodySize);
};

#endif /*VS_CHAT_SERVICE_H*/
