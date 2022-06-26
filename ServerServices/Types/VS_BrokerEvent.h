/*************************************************
 * $Revision: 1 $
 * $History: VS_BrokerEvent.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:50
 * Created in $/VSNA/Servers/ServerServices/types
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/ServerServices/types
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 1.03.06    Time: 20:06
 * Created in $/VS/Servers/ServerServices/Types
 * added broker events
 *************************************************/

#ifndef VS_BROKER_EVENT_H
#define VS_BROKER_EVENT_H

#include "transport/Router/VS_TransportRouterServiceBase.h"

class VS_BrokerEvent
{
public:

    VS_BrokerEvent()
    {};
    virtual ~VS_BrokerEvent(void)
    {};
    virtual bool Run(VS_TransportRouterServiceBase* caller)
    {return false;}
    virtual bool IsValid()
    {return false;};
};

#endif // VS_BROKER_EVENT_H