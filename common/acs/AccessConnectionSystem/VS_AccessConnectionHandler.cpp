
#include "VS_AccessConnectionHandler.h"
#include "VS_AccessConnectionHandlerTypes.h"

VS_AccessConnectionHandler::VS_AccessConnectionHandler( void )
{	imp = new VS_AccessConnectionHandler_Implementation;	}
// end VS_AccessConnectionHandler::VS_AccessConnectionHandler

VS_AccessConnectionHandler::~VS_AccessConnectionHandler( void )
{	if (imp)	delete imp;		}
// end VS_AccessConnectionHandler::~VS_AccessConnectionHandler

inline char *VS_AccessConnectionHandler::HandlerName( void ) const
{	return imp->handlerName;	}
// end VS_AccessConnectionHandler::HandlerName
