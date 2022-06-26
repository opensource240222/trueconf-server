#ifndef VS_NETHANDLERFACTORY_H_HEADER_INCLUDED_BB7FE9A0
#define VS_NETHANDLERFACTORY_H_HEADER_INCLUDED_BB7FE9A0
#include "VS_NetEventHandler.h"
#include "VS_NetEventType.h"
#include "VS_NetRouter.h"

//##ModelId=4480090E00F2
class VS_NetHandlerFactory
{
  public:
    //##ModelId=44800BF603CB
    virtual int CreateHandler(VS_NetEventType aType, VS_NetEventHandler*& aPointer) = 0;

    //##ModelId=4480127D0363
    virtual  int CreateRouter(VS_NetRouter*& aPtr) = 0;

};



#endif /* VS_NETHANDLERFACTORY_H_HEADER_INCLUDED_BB7FE9A0 */
