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
/// \file VS_StreamsMonitor.h
/// \brief
/// \note
///

#ifndef VS_STREAMS_MONITOR_H
#define VS_STREAMS_MONITOR_H

#include "../../streams/VS_StreamsDefinitions.h"

#include <cstdint>
#include <cstring>
#include <ctime>

#define   VS_SR_PREFIX_MON_PIPE   "streams"
extern const char   VS_SrPrefixMonPipe[];		// = VS_SR_PREFIX_MON_PIPE

struct VS_StreamsMonitor
{
	VS_StreamsMonitor( void ) {}
	// end VS_StreamsMonitor::VS_StreamsMonitor

	~VS_StreamsMonitor( void ) {}
	// end VS_StreamsMonitor::~VS_StreamsMonitor

	//////////////////////////////////////////////////////////////////////////////////////

#pragma pack( 1 )

	#define   SM_TYPE_UNKNOW						0
	#define   SM_TYPE_PERIODIC_START				1
	#define   SM_TYPE_PERIODIC_START_CONFERENCES	2
	#define   SM_TYPE_PERIODIC_CONFERENCE			3
	#define   SM_TYPE_PERIODIC_START_PARTICIPANTS	4
	#define   SM_TYPE_PERIODIC_PARTICIPANT			5
	#define   SM_TYPE_PERIODIC_STOP_PARTICIPANTS	6
	#define   SM_TYPE_PERIODIC_STOP_CONFERENCES		7
	#define   SM_TYPE_PERIODIC_STOP					8

	union SmRequest
	{	SmRequest( void ) {		memset( (void *)this, 0, sizeof(SmRequest) );	}
		// end VS_StreamsMonitor::SmRequest::SmRequest end
		unsigned char   type;
	}; // end VS_StreamsMonitor::SmRequest union

	union SmReply
	{	SmReply( void ) {	memset( (void *)this, 0, sizeof(SmReply) );		}
		// end VS_StreamsMonitor::SmReply::SmReply end
		unsigned char   type;
		struct StartConferences
		{	unsigned char   type;
			unsigned long   maxConferences;
		}; // end VS_StreamsMonitor::SmReply::StartConferences struct
		struct Conference
		{	unsigned char   type;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			time_t   createdDate;
			unsigned long   maxParticipants, nParticipants, absenceMs;
		}; // end VS_StreamsMonitor::SmReply::Conference struct
		struct Participant
		{	unsigned char   type;
			char   conferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
			char   participantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
			char   attachedParticipantName[VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME + 1];
			time_t   createdDate;
			char   receiver[10];
			unsigned long   rcvRecTms;
			time_t   rcvLastConDt, rcvLastDisDt;
			char   rcvLocalHost[16], rcvLocalPort[6], rcvTargetHost[16], rcvTargetPort[6];
			uint64_t rcvBytes, rcvFrames;
			unsigned short   minRcvFramesSz, aveRcvFramesSz, maxRcvFramesSz;
			unsigned long   rcvBytesBandwidth, rcvFramesBandwidth;
			char   sender[10];
			unsigned long   sndRecTms;
			time_t   sndLastConDt, sndLastDisDt;
			char   sndLocalHost[16], sndLocalPort[6], sndTargetHost[16], sndTargetPort[6];
			uint64_t sndBytes, sndFrames;
			unsigned short   minSndFramesSz, aveSndFramesSz, maxSndFramesSz;
			unsigned long   sndBytesBandwidth, sndFramesBandwidth;
		}; // end VS_StreamsMonitor::SmReply::Participant struct
		StartConferences   startConferences;
		Conference   conference;
		Participant   participant;
	}; // end VS_StreamsMonitor::SmReply union

#pragma pack(   )
};
// end VS_StreamsMonitor struct

#endif // VS_STREAMS_MONITOR_H
