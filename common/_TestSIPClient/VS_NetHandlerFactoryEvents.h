#ifndef VS_NETHANDLERFACTORYEVENTS_H_HEADER_INCLUDED_BB7F97AC
#define VS_NETHANDLERFACTORYEVENTS_H_HEADER_INCLUDED_BB7F97AC
#include "VS_NetHandlerFactory.h"

//##ModelId=448009210353
class VS_NetHandlerFactoryEvents : public VS_NetHandlerFactory
{
public:
    virtual int CreateHandler(VS_NetEventType aType, VS_NetEventHandler*& aPointer);
    virtual  int CreateRouter(VS_NetRouter*& aPtr) ;

};



#endif /* VS_NETHANDLERFACTORYEVENTS_H_HEADER_INCLUDED_BB7F97AC */
