//////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 12.11.02     by  A.Slavetsky
//
//////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_AccessConnectionMonitor.h
/// \brief
/// \note
///

#ifndef VS_ACCESS_CONNECTION_MONITOR_H
#define VS_ACCESS_CONNECTION_MONITOR_H

#include "../../acs/VS_AcsDefinitions.h"

#define   VS_ACS_PREFIX_MON_PIPE   "acs"
extern const char   VS_AcsPrefixMonPipe[];		// = VS_ACS_PREFIX_MON_PIPE

struct VS_AccessConnectionMonitor
{
	VS_AccessConnectionMonitor( void ) {}
	// end VS_AccessConnectionMonitor::VS_AccessConnectionMonitor

	~VS_AccessConnectionMonitor( void ) {}
	// end VS_AccessConnectionMonitor::~VS_AccessConnectionMonitor

	//////////////////////////////////////////////////////////////////////////////////////

#pragma pack( 1 )

	#define   ACM_TYPE_UNKNOWN						0
	#define   ACM_TYPE_PERIODIC_START				1
	#define   ACM_TYPE_PERIODIC_START_LISTENERS		2
	#define   ACM_TYPE_PERIODIC_LISTENER			3
	#define   ACM_TYPE_PERIODIC_STOP_LISTENERS		4
	#define   ACM_TYPE_PERIODIC_START_CONNECTIONS	5
	#define   ACM_TYPE_PERIODIC_CONNECTION			6
	#define   ACM_TYPE_PERIODIC_STOP_CONNECTIONS	7
	#define   ACM_TYPE_PERIODIC_START_HANDLERS		8
	#define   ACM_TYPE_PERIODIC_HANDLER				9
	#define   ACM_TYPE_PERIODIC_STOP_HANDLERS		10
	#define   ACM_TYPE_PERIODIC_STOP				11

	union AcmRequest
	{	AcmRequest( void ) {	memset( (void *)this, 0, sizeof(AcmRequest) );	}
		// end VS_AccessConnectionMonitor::AcmRequest::AcmRequest end
		unsigned char   type;
	}; // end VS_AccessConnectionMonitor::AcmRequest union

	union AcmReply
	{	AcmReply( void ) {	memset( (void *)this, 0, sizeof(AcmReply) );	}
		// end VS_AccessConnectionMonitor::AcmReply::AcmReply end
		unsigned char   type;
		struct StartListeners
		{	unsigned char	type;
			unsigned long	maxListeners;
		}; // end VS_AccessConnectionMonitor::AcmReply::StartListeners struct
		struct Listener
		{	unsigned char	type;
			unsigned long	maxConns, nConns, nRecvConns, nUnsucAttempts;
			char			host[16];
			unsigned short	port;
		}; // end VS_AccessConnectionMonitor::AcmReply::Listener struct
		struct StartConnections
		{	unsigned char	type;
			unsigned long	maxConnections;
		}; // end VS_AccessConnectionMonitor::AcmReply::StartConnections struct
		struct Connection
		{	unsigned char	type;
			time_t			creationDateTime;
			char			localHost[16], remoteHost[16];
			unsigned short	localPort, remotePort;
			unsigned long	nIterations, readedBytes, writtenBytes;
		}; // end VS_AccessConnectionMonitor::AcmReply::Connection struct
		struct StartHandlers
		{	unsigned char	type;
			unsigned long	maxHandlers;
		}; // end VS_AccessConnectionMonitor::AcmReply::StartHandlers struct
		struct Handler
		{	unsigned char	type;
			unsigned long	nProcConns, nAccConns;
			char			handlerName[VS_ACS_MAX_SIZE_HANDLER_NAME + 1];
		}; // end VS_AccessConnectionMonitor::AcmReply::Handler struct
		StartListeners		startListeners;
		Listener			listener;
		StartConnections	startConnections;
		Connection			connection;
		StartHandlers		startHandlers;
		Handler				handler;
	}; // end VS_AccessConnectionMonitor::AcmReply union

#pragma pack(   )
};
// end VS_AccessConnectionMonitor struct

#endif // VS_ACCESS_CONNECTION_MONITOR_H
