/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 03.03.03     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_AcsDefinition.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef VS_ACS_DEFINITIONS_H
#define VS_ACS_DEFINITIONS_H

#include <string.h>

#define   VS_ACS_MAX_SIZE_ENDPOINT_NAME   255
#define   VS_ACS_MAX_SIZE_SERVER_NAME   64
#define   VS_ACS_MAX_SIZE_HANDLER_NAME   511
#define	  VS_MAX_LISTENERS	10

#define	  VS_ACS_MAX_SIZE_CID	255

/////////////////////////////////////////////////////////////////////////////////////////

inline bool VS_CheckEndpointName( const char *endpointName )
{	return endpointName && *endpointName && strlen( endpointName ) < ( VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1 );		}
// end of VS_CheckEndpointName

inline bool VS_CheckServerName( const char *serverName )
{	return serverName && *serverName && strlen( serverName ) < ( VS_ACS_MAX_SIZE_SERVER_NAME + 1 );		}
// end of VS_CheckServerName

inline bool VS_CheckHandlerName( const char *handlerName )
{	return handlerName && *handlerName && strlen( handlerName ) < ( VS_ACS_MAX_SIZE_HANDLER_NAME + 1 );		}
// end of VS_CheckHandlerName

/////////////////////////////////////////////////////////////////////////////////////////
#endif
