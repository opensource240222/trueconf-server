#include "VS_NetHandlerFactoryEvents.h"
#include "../SIPParserLib/VS_SIPError.h"
#include "VS_NetEventHandlerEvents.h"
#include "VS_NetRouterEvents.h"

int VS_NetHandlerFactoryEvents::CreateHandler(VS_NetEventType aType, VS_NetEventHandler*& aPointer)
{
	if (aType.m_type == e_connector)
	{
		aPointer = new VS_NetConnectionHandlerEvents;
	}
	if (aType.m_type == e_reader)
	{
	 	aPointer = new VS_NetReadHandlerEvents;
	}
	if (aType.m_type == e_writer)
	{
		aPointer = new VS_NetWriteHandlerEvents;
	}
	if (!aPointer)
        return e_bad;
	return e_ok;
}
int VS_NetHandlerFactoryEvents::CreateRouter(VS_NetRouter*& aPtr)
{
	aPtr = new VS_NetRouterEvents;
	if (!aPtr)
        return e_bad;
	return e_ok;
}
