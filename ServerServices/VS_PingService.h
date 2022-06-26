/*
 * $Revision: 2 $
 * $History: VS_PingService.h $
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 28.12.07   Time: 20:08
 * Updated in $/VSNA/Servers/ServerServices
 * bool Init( const char *our_endpoint, const char *our_service, const
 * bool permittedAll = false ); added.
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:50
 * Created in $/VSNA/Servers/ServerServices
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/ServerServices
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 29.09.04   Time: 14:24
 * Updated in $/VS/Servers/ServerServices
 * pragma_once removed
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 8.09.03    Time: 16:43
 * Created in $/VS/Servers/ServerServices
 * moved PingService to global services
 *
 * *****************  Version 1  *****************
 * User: Slavetsky    Date: 9/04/03    Time: 4:38p
 * Created in $/VS/Servers/MediaBroker/BrokerServices
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 20.05.03   Time: 15:38
 * Updated in $/VS/Servers/ServerServices
 * Serisies iterfaces rewrited
 *
 ***********************************************/

#ifndef VS_PING_SERVICE_H
#define VS_PING_SERVICE_H

#include "transport/Router/VS_TransportRouterServiceBase.h"

class VS_PingService :
	public VS_TransportRouterServiceBase
{
public:
	VS_PingService(void) : m_recvMess(0) { }
	virtual ~VS_PingService(void) { }

	bool Init(const char* our_endpoint, const char* our_service, const bool permittedAll = false) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

	// Service implementation
	VS_RouterMessage *m_recvMess;
	// PING_METHOD()
	void Ping_Method();
};
#endif
