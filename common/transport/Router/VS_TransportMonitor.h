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
/// \file VS_TransportMonitor.h
/// \brief
/// \note
///

#ifndef VS_TRANSPORT_MONITOR_H
#define VS_TRANSPORT_MONITOR_H

#include "../../acs/VS_AcsDefinitions.h"
#include "../../transport/VS_TransportDefinitions.h"

#include <time.h>
#include <string.h>

#define   VS_TR_PREFIX_MON_PIPE   "transport"
extern const char   VS_TrPrefixMonPipe[];		// = VS_TR_PREFIX_MON_PIPE

struct VS_TransportMonitor
{
	VS_TransportMonitor( void ) {}
	// end VS_TransportMonitor::VS_TransportMonitor

	virtual ~VS_TransportMonitor( void ) {}
	// end VS_TransportMonitor::~VS_TransportMonitor

	//////////////////////////////////////////////////////////////////////////////////////

#pragma pack( 1 )

	#define   TM_TYPE_UNKNOWN					0
	#define   TM_TYPE_PERIODIC_START			1
	#define   TM_TYPE_PERIODIC_START_ENDPOINTS	2
	#define   TM_TYPE_PERIODIC_ENDPOINT			3
	#define   TM_TYPE_PERIODIC_STOP_ENDPOINTS	4
	#define   TM_TYPE_PERIODIC_START_SERVICES	5
	#define   TM_TYPE_PERIODIC_SERVICE			6
	#define   TM_TYPE_PERIODIC_STOP_SERVICES	7
	#define   TM_TYPE_PERIODIC_STOP				8

	union TmRequest
	{	TmRequest( void ) {	memset( (void *)this, 0, sizeof(TmRequest) );	}
		// end VS_TransportMonitor::TmRequest::TmRequest end
		unsigned char   type;
	}; // end VS_TransportMonitor::TmRequest union

	union TmReply
	{	TmReply( void ) {	memset( (void *)this, 0, sizeof(TmReply) );		}
		// end VS_TransportMonitor::TmReply::TmReply end
		unsigned char   type;
		struct StartEndpoints
		{	unsigned char	type;
			unsigned long	maxEndpoints;
		}; // end VS_TransportMonitor::TmReply::StartEndpoints struct
		struct Endpoint
		{	unsigned char	type;
			unsigned char	ep_type;
			char			cid[VS_ACS_MAX_SIZE_CID + 1];
			char			username[VS_ACS_MAX_SIZE_ENDPOINT_NAME + 1];
			char			protocol[10];
			time_t			connectionDate;
			time_t			lastConnectDateTime, lastDisconnectDateTime;
			unsigned long	minRcvMessSz, aveRcvMessSz, maxRcvMessSz;
			unsigned long	minSndMessSz, aveSndMessSz, maxSndMessSz;
			unsigned long	reconnects;
			char			localHost[16], remoteHost[16];
			unsigned short	localPort, remotePort;
			unsigned __int64	sndMess;
			unsigned __int64	rcvMess;
		}; // end VS_TransportMonitor::TmReply::Endpoint struct
		struct StartServices
		{	unsigned char	type;
			unsigned long	maxServices;
		}; // end VS_TransportMonitor::TmReply::StartServices struct
		struct Service
		{	unsigned char	type;
			char			serviceName[VS_TRANSPORT_MAX_SIZE_SERVICE_NAME + 1];
			char			serviceType[10];
			unsigned long	minRcvMessSz, aveRcvMessSz, maxRcvMessSz;
			unsigned long	minSndMessSz, aveSndMessSz, maxSndMessSz;
			unsigned __int64	rcvMess;
			unsigned __int64	sndMess;
		}; // end VS_TransportMonitor::TmReply::Service struct
		StartEndpoints		startEndpoints;
		Endpoint			endpoint;
		StartServices		startServices;
		Service				service;
	}; // end VS_TransportMonitor::TmReply union

#pragma pack(   )
};
// end VS_TransportMonitor struct

#endif // VS_TRANSPORT_MONITOR_H
