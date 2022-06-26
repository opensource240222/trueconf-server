
#ifndef VS_ACCESS_CONNECTION_HANDLER_TYPES_H
#define VS_ACCESS_CONNECTION_HANDLER_TYPES_H

#include <Windows.h>

#include "../Lib/VS_AcsLog.h"

struct VS_AccessConnectionSystem_ImplementationCalls
{
	VS_AccessConnectionSystem_ImplementationCalls( void ) {}
	// end of VS_AccessConnectionSystem_ImplementationCalls constructor
	virtual void SetConnection( VS_ConnectionTCP *conn ) = 0;
};
// end VS_AccessConnectionSystem_ImplementationCalls

struct VS_AccessConnectionHandler_Implementation
{
	VS_AccessConnectionHandler_Implementation( void )
		: imp_calls(0) {	memset( handlerName, 0, sizeof(handlerName) );	}
	// end of VS_AccessConnectionHandler_Implementation constructor
	VS_AccessConnectionSystem_ImplementationCalls   *imp_calls;
	char   handlerName[VS_ACS_MAX_SIZE_HANDLER_NAME + 1];
};
// end VS_AccessConnectionHandler_Implementation struct

#endif // VS_ACCESS_CONNECTION_HANDLER_TYPES_H
