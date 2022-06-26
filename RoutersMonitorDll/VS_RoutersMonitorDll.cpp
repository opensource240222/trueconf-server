#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <Windows.h>

#include "VS_RoutersMonitorDll.h"
#include "../common/acs/VS_AcsDefinitions.h"
#include "../common/acs/Lib/VS_AcsLib.h"
#include "acs/connection/VS_ConnectionByte.h"
#include "../common/streams/Router/VS_StreamsMonitor.h"
#include "../common/std/cpplib/VS_Utils.h"
#include "std/cpplib/ThreadUtils.h"

/////////////////////////////////////////////////////////////////////////////////////////

inline bool TestAccessibility( const char computerName[], const char endpointName[], const char postfix[] )
{
	VS_ConnectionByte   pipe;
	char   serverName[256];		memset( (void *)serverName, 0, sizeof(serverName) );
	strncpy( serverName, endpointName, sizeof(serverName) - 1 );
	VS_FilterPath(serverName);
	char   namePipe[512];	memset( (void *)namePipe, 0, sizeof(namePipe) );
	sprintf( namePipe, "\\\\%s\\pipe\\%s%s\\%s", computerName, VS_Servers_PipeDir, serverName, postfix );
	return pipe.Open( namePipe, vs_pipe_type_duplex );
}
// end TestAccessibility

/////////////////////////////////////////////////////////////////////////////////////////

struct Reading
{
	/////////////////////////////////////////////////////////////////////////////////////

	Reading( const char computerName[], const char endpointName[],
				const HWND hwnd, const UINT msg, const char postfix[] )
		: isValid(false), hwnd(hwnd), msg(msg), stateRead(0), readBuffer()
		, readType(0), readBody(0), readBytes(0), event(CreateEvent( 0, TRUE, FALSE, 0 ))
		, pipe(new VS_ConnectionByte), hthr(0)
	{
		if (!pipe || !event)	return;
		char   serverName[256];		memset( (void *)serverName, 0, sizeof(serverName) );
		strncpy( serverName, endpointName, sizeof(serverName) - 1 );
		VS_FilterPath(serverName);
		char   namePipe[512];	memset( (void *)namePipe, 0, sizeof(namePipe) );
		sprintf( namePipe, "\\\\%s\\pipe\\%s%s\\%s", computerName, VS_Servers_PipeDir, serverName, postfix );
		if (!pipe->Open( namePipe, vs_pipe_type_duplex ) || !pipe->CreateOvReadEvent())
			return;
		isValid = true;
	}
	// end Reading::Reading

	~Reading( void )
	{
		if (pipe)	delete pipe;
		if (event)	CloseHandle( event );
	}
	// end Reading::~Reading

	bool   isValid;
	const HWND   hwnd;		const UINT   msg;
	unsigned   stateRead;
	void   *readBuffer, *readType, *readBody;
	unsigned long   readBytes;
	HANDLE   event;
	VS_ConnectionByte   *pipe;
	uintptr_t   hthr;

	/////////////////////////////////////////////////////////////////////////////////////

	inline bool Start( void )
	{
		if (!isValid)	return false;
		hthr = _beginthreadex( 0, 0, Thread, (void *)this, 0, 0 );
		if (!hthr || hthr == -1L) {		hthr = 0;	return false;	}
		return true;
	}
	// end Reading::Start

	inline void Stop( void )
	{
		if (hthr)
		{	if (SetEvent( event ) && WaitForSingleObject( (HANDLE)hthr, INFINITE ) == WAIT_OBJECT_0)
				CloseHandle( (HANDLE)hthr );
			hthr = 0;
	}	}
	// end Reading::Stop

	/////////////////////////////////////////////////////////////////////////////////////

	virtual bool   PostRead( void ) = 0;

	/////////////////////////////////////////////////////////////////////////////////////

	inline bool PostMsg( const void *post_buffer, const unsigned long post_bytes )
	{
		if (!post_buffer || !post_bytes)	return false;
		void   *buffer = malloc( (size_t)post_bytes );
		if (!buffer)	return false;
		memcpy( buffer, post_buffer, (size_t)post_bytes );
		WPARAM   wparam = 0;	LPARAM   lparam = 0;
		VS_Set64Bits( (unsigned __int64)buffer, wparam, lparam );
		const bool   ret = PostMessage( hwnd, msg, wparam, lparam ) == TRUE;
		if (!ret)	free( buffer );
		return ret;
	}
	// end Reading::PostMsg

	inline void Thread( void )
	{
		HANDLE   hs[] = { pipe->OvReadEvent(), event };
		while (1)
		{	if (!pipe->Read( readBuffer, readBytes ))	break;
			if (WaitForMultipleObjects( 2, hs, FALSE, INFINITE ) == WAIT_OBJECT_0)
			{	unsigned long   mills = 0;
				if (pipe->GetReadResult( mills = INFINITE ) != (int)readBytes
					|| !PostRead())		break;
			} else	break;
	}	}
	// end Reading::Thread

	static unsigned __stdcall Thread( void *arg )
	{
		vs::SetThreadName("RM_PipeReader");
		static_cast<Reading*>(arg)->Thread();
		_endthreadex( 0 );
		return 0;
	}
	// end Reading::Thread

	/////////////////////////////////////////////////////////////////////////////////////
};
// end Reading struct

struct ReadingAcs : public Reading
{
	/////////////////////////////////////////////////////////////////////////////////////

	ReadingAcs( const char computerName[], const char endpointName[],
					const HWND hwnd, const UINT msg )
		: Reading( computerName, endpointName, hwnd, msg, VS_AcsPrefixMonPipe )
	{
		memset( (void *)&request, 0, sizeof(request) );
		memset( (void *)&reply, 0, sizeof(reply) );
		readType = (void *)&reply;
		readBody = (void *)((char *)&reply + sizeof(reply.type));
		readBuffer = readType;		readBytes = sizeof(reply.type);
	}
	// end ReadingAcs::ReadingAcs

	~ReadingAcs( void ) {}
	// end ReadingAcs::~ReadingAcs

	VS_AccessConnectionMonitor::AcmRequest   request;
	VS_AccessConnectionMonitor::AcmReply   reply;

	/////////////////////////////////////////////////////////////////////////////////////

	bool PostRead( void )
	{
		unsigned long   post_bytes = sizeof(reply.type);
		switch (stateRead)
		{
		case 0 :
			switch (reply.type)
			{
			case ACM_TYPE_PERIODIC_START :				break;
			case ACM_TYPE_PERIODIC_START_LISTENERS :
				stateRead = 1;	readBuffer = readBody;
				readBytes = sizeof(reply.startListeners) - sizeof(reply.type);
				return true;
			case ACM_TYPE_PERIODIC_LISTENER :
				stateRead = 1;	readBuffer = readBody;
				readBytes = sizeof(reply.listener) - sizeof(reply.type);
				return true;
			case ACM_TYPE_PERIODIC_STOP_LISTENERS :		break;
			case ACM_TYPE_PERIODIC_START_CONNECTIONS :
				stateRead = 1;	readBuffer = readBody;
				readBytes = sizeof(reply.startConnections) - sizeof(reply.type);
				return true;
			case ACM_TYPE_PERIODIC_CONNECTION :
				stateRead = 1;	readBuffer = readBody;
				readBytes = sizeof(reply.connection) - sizeof(reply.type);
				return true;
			case ACM_TYPE_PERIODIC_STOP_CONNECTIONS :	break;
			case ACM_TYPE_PERIODIC_START_HANDLERS :
				stateRead = 1;	readBuffer = readBody;
				readBytes = sizeof(reply.startHandlers) - sizeof(reply.type);
				return true;
			case ACM_TYPE_PERIODIC_HANDLER :
				stateRead = 1;	readBuffer = readBody;
				readBytes = sizeof(reply.handler) - sizeof(reply.type);
				return true;
			case ACM_TYPE_PERIODIC_STOP_HANDLERS :		break;
			case ACM_TYPE_PERIODIC_STOP :
				break;
			default :	return false;
			}
			break;
		case 1 :
			switch (reply.type)
			{
			case ACM_TYPE_PERIODIC_START_LISTENERS :
				post_bytes = sizeof(reply.startListeners);		break;
			case ACM_TYPE_PERIODIC_LISTENER :
				post_bytes = sizeof(reply.listener);			break;
			case ACM_TYPE_PERIODIC_START_CONNECTIONS :
				post_bytes = sizeof(reply.startConnections);	break;
			case ACM_TYPE_PERIODIC_CONNECTION :
				post_bytes = sizeof(reply.connection);			break;
			case ACM_TYPE_PERIODIC_START_HANDLERS :
				post_bytes = sizeof(reply.startHandlers);		break;
			case ACM_TYPE_PERIODIC_HANDLER :
				post_bytes = sizeof(reply.handler);				break;
			default :	return false;
			}
			stateRead = 0;	readBuffer = readType;
			readBytes = sizeof(reply.type);
			break;
		default :	return false;
		}
		return PostMsg( (void *)&reply, post_bytes );
	}
	// end ReadingAcs::PostRead

	/////////////////////////////////////////////////////////////////////////////////////
};
// end ReadingAcs struct

struct ReadingTransportRouter : public Reading
{
	/////////////////////////////////////////////////////////////////////////////////////

	ReadingTransportRouter( const char computerName[], const char endpointName[],
								const HWND hwnd, const UINT msg )
		: Reading( computerName, endpointName, hwnd, msg, /*VS_TrPrefixMonPipe*/VS_TR_PREFIX_MON_PIPE )
	{
		memset( (void *)&request, 0, sizeof(request) );
		memset( (void *)&reply, 0, sizeof(reply) );
		readType = (void *)&reply;
		readBody = (void *)((char *)&reply + sizeof(reply.type));
		readBuffer = readType;		readBytes = sizeof(reply.type);
	}
	// end ReadingTransportRouter::ReadingTransportRouter

	~ReadingTransportRouter( void ) {}
	// end ReadingTransportRouter::~ReadingTransportRouter

	VS_TransportMonitor::TmRequest   request;
	VS_TransportMonitor::TmReply   reply;

	/////////////////////////////////////////////////////////////////////////////////////

	bool PostRead( void )
	{
		unsigned long   post_bytes = sizeof(reply.type);
		switch (stateRead)
		{
		case 0 :
			switch (reply.type)
			{
			case TM_TYPE_PERIODIC_START :				break;
			case TM_TYPE_PERIODIC_START_ENDPOINTS :
				stateRead = 1;	readBuffer = readBody;
				readBytes = sizeof(reply.startEndpoints) - sizeof(reply.type);
				return true;
			case TM_TYPE_PERIODIC_ENDPOINT :
				stateRead = 1;	readBuffer = readBody;
				readBytes = sizeof(reply.endpoint) - sizeof(reply.type);
				return true;
			case TM_TYPE_PERIODIC_STOP_ENDPOINTS :		break;
			case TM_TYPE_PERIODIC_START_SERVICES :
				stateRead = 1;	readBuffer = readBody;
				readBytes = sizeof(reply.startServices) - sizeof(reply.type);
				return true;
			case TM_TYPE_PERIODIC_SERVICE :
				stateRead = 1;	readBuffer = readBody;
				readBytes = sizeof(reply.service) - sizeof(reply.type);
				return true;
			case TM_TYPE_PERIODIC_STOP_SERVICES :		break;
			case TM_TYPE_PERIODIC_STOP :				break;
			default :	return false;
			}
			break;
		case 1 :
			switch (reply.type)
			{
			case TM_TYPE_PERIODIC_START_ENDPOINTS :
				post_bytes = sizeof(reply.startEndpoints);	break;
			case TM_TYPE_PERIODIC_ENDPOINT :
				post_bytes = sizeof(reply.endpoint);		break;
			case TM_TYPE_PERIODIC_START_SERVICES :
				post_bytes = sizeof(reply.startServices);	break;
			case TM_TYPE_PERIODIC_SERVICE :
				post_bytes = sizeof(reply.service);			break;
			default :	return false;
			}
			stateRead = 0;	readBuffer = readType;
			readBytes = sizeof(reply.type);
			break;
		default :	return false;
		}
		return PostMsg( (void *)&reply, post_bytes );
	}
	// end ReadingTransportRouter::PostRead

	/////////////////////////////////////////////////////////////////////////////////////
};
// end ReadingTransportRouter struct

struct ReadingStreamsRouter : public Reading
{
	/////////////////////////////////////////////////////////////////////////////////////

	ReadingStreamsRouter( const char computerName[], const char endpointName[],
							const HWND hwnd, const UINT msg )
		: Reading( computerName, endpointName, hwnd, msg, VS_SrPrefixMonPipe )
	{
		memset( (void *)&request, 0, sizeof(request) );
		memset( (void *)&reply, 0, sizeof(reply) );
		readType = (void *)&reply;
		readBody = (void *)((char *)&reply + sizeof(reply.type));
		readBuffer = readType;		readBytes = sizeof(reply.type);
	}
	// end ReadingStreamsRouter::ReadingStreamsRouter

	~ReadingStreamsRouter( void ) {}
	// end ReadingStreamsRouter::~ReadingStreamsRouter

	VS_StreamsMonitor::SmRequest   request;
	VS_StreamsMonitor::SmReply   reply;

	/////////////////////////////////////////////////////////////////////////////////////

	bool PostRead( void )
	{
		unsigned long   post_bytes = sizeof(reply.type);
		switch (stateRead)
		{
		case 0 :
			switch (reply.type)
			{
			case SM_TYPE_PERIODIC_START :				break;
			case SM_TYPE_PERIODIC_START_CONFERENCES :
				stateRead = 1;	readBuffer = readBody;
				readBytes = sizeof(reply.startConferences) - sizeof(reply.type);
				return true;
			case SM_TYPE_PERIODIC_CONFERENCE :
				stateRead = 1;	readBuffer = readBody;
				readBytes = sizeof(reply.conference) - sizeof(reply.type);
				return true;
			case SM_TYPE_PERIODIC_START_PARTICIPANTS :	break;
			case SM_TYPE_PERIODIC_PARTICIPANT :
				stateRead = 1;	readBuffer = readBody;
				readBytes = sizeof(reply.participant) - sizeof(reply.type);
				return true;
			case SM_TYPE_PERIODIC_STOP_PARTICIPANTS :	break;
			case SM_TYPE_PERIODIC_STOP_CONFERENCES :	break;
			case SM_TYPE_PERIODIC_STOP :				break;
			default :	return false;
			}
			break;
		case 1 :
			switch (reply.type)
			{
			case SM_TYPE_PERIODIC_START_CONFERENCES :
				post_bytes = sizeof(reply.startConferences);	break;
			case SM_TYPE_PERIODIC_CONFERENCE :
				post_bytes = sizeof(reply.conference);			break;
			case SM_TYPE_PERIODIC_PARTICIPANT :
				post_bytes = sizeof(reply.participant);			break;
			default :	return false;
			}
			stateRead = 0;	readBuffer = readType;
			readBytes = sizeof(reply.type);
			break;
		default :	return false;
		}
		return PostMsg( (void *)&reply, post_bytes );
	}
	// end ReadingStreamsRouter::PostRead

	/////////////////////////////////////////////////////////////////////////////////////
};
// end ReadingStreamsRouter

/////////////////////////////////////////////////////////////////////////////////////////

static CRITICAL_SECTION   cs;
static const unsigned long   start_sequence = 0x0ABCDEF0;
inline unsigned long StartFunction( void )
{	memset( (void *)&cs, 0, sizeof(cs) );	InitializeCriticalSection( &cs );	return start_sequence;	}
// end StartFunction
static const unsigned long   start_flag = StartFunction();

static ReadingAcs   *rd_acs = 0;
static ReadingTransportRouter   *rd_tr = 0;
static ReadingStreamsRouter   *rd_sr = 0;

/////////////////////////////////////////////////////////////////////////////////////////

inline bool WhatAtPresentAct( const char *computerName, const char *endpointName,
								bool *acs, bool *transportRouter, bool *streamsRouter )
{
	if (acs)				*acs = rd_acs ? true : TestAccessibility( computerName, endpointName, VS_AcsPrefixMonPipe );
	if (transportRouter)	*transportRouter = rd_tr ? true : TestAccessibility( computerName, endpointName, /*VS_TrPrefixMonPipe*/VS_TR_PREFIX_MON_PIPE );
	if (streamsRouter)		*streamsRouter = rd_sr ? true : TestAccessibility( computerName, endpointName, VS_SrPrefixMonPipe );
	return true;
}
// end WhatAtPresentAct

bool VS_WhatAtPresent( const char *computerName, const char *endpointName,
							bool *acs, bool *transportRouter, bool *streamsRouter )
{
	if (!computerName || !*computerName || !endpointName || !*endpointName
		|| strlen(endpointName) > VS_ACS_MAX_SIZE_ENDPOINT_NAME)	return false;
	EnterCriticalSection( &cs );
	const bool   ret = WhatAtPresentAct( computerName, endpointName, acs, transportRouter, streamsRouter );
	LeaveCriticalSection( &cs );
	return ret;
}
// end VS_WhatAtPresent

/////////////////////////////////////////////////////////////////////////////////////////

void VS_FreeMessage( void *wlparams ) {		if (wlparams)	free( wlparams );	}
// end VS_FreeMessage

/////////////////////////////////////////////////////////////////////////////////////////

char *VS_CTime( const time_t time )
{
	static char empty_str[] = "";

	if (time == 0)
		return empty_str;

	char* result = ctime(&time);
	if (!result)
		return empty_str;

	char* p = strchr(result, '\n');
	if (p)
		*p = '\0';

	return result;
}
// end VS_FreeMessage

/////////////////////////////////////////////////////////////////////////////////////////

inline bool StartReadingAcsAct( const char *computerName, const char *endpointName,
									HWND hwnd, UINT msg )
{
	if (rd_acs)		return false;
	rd_acs = new ReadingAcs( computerName, endpointName, hwnd, msg );
	if (!rd_acs)	return false;
	if (!rd_acs->Start()) {		delete rd_acs;	rd_acs = 0;		return false;	}
	return true;
}
// end StartReadingAcsAct

bool VS_StartReadingAcs( const char *computerName, const char *endpointName,
							HWND hwnd, UINT msg )
{
	if (!computerName || !*computerName || !endpointName || !*endpointName
		|| strlen(endpointName) > VS_ACS_MAX_SIZE_ENDPOINT_NAME)	return false;
	EnterCriticalSection( &cs );
	const bool   ret = StartReadingAcsAct( computerName, endpointName, hwnd, msg );
	LeaveCriticalSection( &cs );
	return ret;
}
// end VS_StartReadingAcs

inline void StopReadingAcsAct( void )
{
	if (rd_acs)
	{	rd_acs->Stop();		delete rd_acs;	rd_acs = 0;
}	}
// end StopReadingAcsAct

void VS_StopReadingAcs( void )
{
	EnterCriticalSection( &cs );
	StopReadingAcsAct();
	LeaveCriticalSection( &cs );
}
// end VS_StopReadingAcs

/////////////////////////////////////////////////////////////////////////////////////////

inline bool StartReadingTransportRouterAct( const char *computerName, const char *endpointName,
												HWND hwnd, UINT msg )
{
	if (rd_tr)		return false;
	rd_tr = new ReadingTransportRouter( computerName, endpointName, hwnd, msg );
	if (!rd_tr)	return false;
	if (!rd_tr->Start()) {		delete rd_tr;	rd_tr = 0;		return false;	}
	return true;
}
// end StartReadingTransportRouterAct

bool VS_StartReadingTransportRouter( const char *computerName, const char *endpointName,
										HWND hwnd, UINT msg )
{
	if (!computerName || !*computerName || !endpointName || !*endpointName
		|| strlen(endpointName) > VS_ACS_MAX_SIZE_ENDPOINT_NAME)	return false;
	EnterCriticalSection( &cs );
	const bool   ret = StartReadingTransportRouterAct( computerName, endpointName, hwnd, msg );
	LeaveCriticalSection( &cs );
	return ret;
}
// end VS_StartReadingTransportRouter

inline void StopReadingTransportRouterAct( void )
{
	if (rd_tr)
	{	rd_tr->Stop();	delete rd_tr;	rd_tr = 0;
}	}
// end StopReadingTransportRouterAct

void VS_StopReadingTransportRouter( void )
{
	EnterCriticalSection( &cs );
	StopReadingTransportRouterAct();
	LeaveCriticalSection( &cs );
}
// end VS_StopReadingTransportRouter

/////////////////////////////////////////////////////////////////////////////////////////

inline bool StartReadingStreamsRouterAct( const char *computerName, const char *endpointName,
											HWND hwnd, UINT msg )
{
	if (rd_sr)		return false;
	rd_sr = new ReadingStreamsRouter( computerName, endpointName, hwnd, msg );
	if (!rd_sr)		return false;
	if (!rd_sr->Start()) {		delete rd_sr;	rd_sr = 0;	return false;	}
	return true;
}
// end StartReadingStreamsRouterAct

bool VS_StartReadingStreamsRouter( const char *computerName, const char *endpointName,
										HWND hwnd, UINT msg )
{
	if (!computerName || !*computerName || !endpointName || !*endpointName
		|| strlen(endpointName) > VS_ACS_MAX_SIZE_ENDPOINT_NAME)	return false;
	EnterCriticalSection( &cs );
	const bool   ret = StartReadingStreamsRouterAct( computerName, endpointName, hwnd, msg );
	LeaveCriticalSection( &cs );
	return ret;
}
// end VS_StartReadingStreamsRouter

inline void StopReadingStreamsRouterAct( void )
{
	if (rd_sr)
	{	rd_sr->Stop();	delete rd_sr;	rd_sr = 0;
}	}
// end StopReadingStreamsRouterAct

void VS_StopReadingStreamsRouter( void )
{
	EnterCriticalSection( &cs );
	StopReadingStreamsRouterAct();
	LeaveCriticalSection( &cs );
}
// end VS_StopReadingStreamsRouter

/////////////////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{	return TRUE;	}
// end DllMain

/////////////////////////////////////////////////////////////////////////////////////////
