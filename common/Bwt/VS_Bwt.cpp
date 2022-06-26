
//////////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <Windows.h>

#include "../std/cpplib/VS_MemoryLeak.h"
#include "VS_Bwt.h"
#include "VS_BwtLib.h"
#include "VS_BwtHexBuffer.h"
#include "../acs/Lib/VS_AcsLib.h"
#include "../acs/Lib/VS_AcsLibDefinitions.h"
#include "../acs/ConnectionManager/VS_ConnectionManager.h"
#include "../acs/connection/VS_ConnectionSock.h"
#include "../net/EndpointRegistry.h"
#include "../streams/Protocol.h"
#include "std/cpplib/ThreadUtils.h"

//////////////////////////////////////////////////////////////////////////////////////////

struct VS_BwtArgs
{
	VS_BwtArgs( VS_BwtResult *result, const char *endpoint, const unsigned seconds, const unsigned long frames_size, const unsigned long period, VS_BwtIntermediate *callback )
		: result(result), out_tcs(0), in_tcs(0), out_flag(false), in_flag(false)
		, endpoint(_strdup(endpoint)), seconds(seconds), size(frames_size), period(period)
		, callback(callback)
	{	memset( (void *)result, 0, sizeof(VS_BwtResult) );	}
	// end VS_BwtArgs::VS_BwtArgs

	~VS_BwtArgs( void ) {	if (endpoint)	free( (void *)endpoint );	}
	// end VS_BwtArgs::~VS_BwtArgs

	bool IsValid( void ) {		return endpoint != 0;	}
	// end VS_BwtArgs::IsValid

	VS_BwtResult   *result;
	volatile unsigned long   out_tcs, in_tcs;
	volatile bool   out_flag, in_flag;
	const char   *endpoint;
	const unsigned   seconds;
	const unsigned long   size, period;
	VS_BwtIntermediate   *callback;

	inline void GoSend( VS_ConnectionSock *conn )
	{
		if (!conn) {	out_flag = false;	return;		}
		SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );
		void   *buffer = 0;
		const unsigned long size_frame = size + sizeof(stream::FrameHeader);
		{	unsigned char   *bf = (unsigned char *)malloc( (size_t)size_frame );
			if (!bf)	return;		buffer = (void *)bf;	bf += sizeof(stream::FrameHeader);
			unsigned long   ind = 0, sz = size;
			while (ind < size)
			{	memcpy( &bf[ind], VS_HexBuffer, sz );	ind += sz;		sz = size - ind;
				if (sz > VS_ACS_BWT_HEX_BUFFER_SIZE)	sz = VS_ACS_BWT_HEX_BUFFER_SIZE;
		}	}
		stream::FrameHeader& head = *(stream::FrameHeader *)buffer;		long jitter;
		head.length = (unsigned short)size;
		head.track = stream::Track::old_command;
		head.cksum = stream::GetFrameBodyChecksum(static_cast<uint8_t*>(buffer) + sizeof(stream::FrameHeader), size);
		unsigned long   mills = 0, tm = seconds * 1000, tick = GetTickCount(), new_tick,
						jitter_diff, cur_bwt;		bool   jitter_diff_flag = false;
		const unsigned long   due_bwt = !period ? ~0 : (unsigned long)( (double)( 1000./ period ) * size );
		double   out_bytes = 0;
		while (mills < tm)
		{
			head.tick_count = mills;
			if (conn->Send( buffer, size_frame ) != (int)size_frame)	break;
			new_tick = GetTickCount();		tick = new_tick - tick;
			out_bytes += size_frame;		mills += tick;
			out_tcs += tick;	result->out_bytes = out_bytes;
			if (jitter_diff_flag)
			{	jitter = mills - (head.tick_count + jitter_diff);
				if (result->out_jitter_min_ms > jitter)		result->out_jitter_min_ms = jitter;
				if (result->out_jitter_max_ms < jitter)		result->out_jitter_max_ms = jitter;
				if (callback)	callback->Out( mills, jitter );		}
			else {		jitter_diff_flag = true;	jitter_diff = mills;	}
			cur_bwt = mills ? (unsigned long)((( out_bytes * 1000 ) + (( mills + 1 ) / 2 )) / mills ) : ~0;
			if (cur_bwt <= due_bwt )	tick = new_tick;
			else
			{	if (tick >= period)		tick = new_tick;
				else
				{	Sleep( period - tick );		tick = new_tick;
					new_tick = GetTickCount();		tick = new_tick - tick;
					out_tcs += tick;	mills += tick;		tick = new_tick;
		}	}	}
		if (buffer != (void *)VS_HexBuffer)		free( (void *)buffer );
		out_flag = false;
	}
	// end VS_BwtArgs::GoSend

	inline void GoReceive( VS_ConnectionSock *conn )
	{
		SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );
		if (!conn) {	in_flag = false;	return;		}
		void   *buffer = malloc( (size_t)size );
		if (!buffer)	return;			int   st;		long   jitter;
		stream::FrameHeader head;		unsigned state = 0, ind = 0, sz = sizeof(head);
		unsigned long   mills = 0, tm = seconds * 4000, tick, new_tick = tm, jitter_diff;
		bool   jitter_diff_flag = false;	volatile double   &in_bytes = result->in_bytes;
		if (conn->SelectIn( new_tick ) == 1)
		{	tick = GetTickCount();
			while (mills < tm)
			{	switch (state)
				{
				case 0 :
					new_tick = tm - mills;
					if (conn->SelectIn( new_tick ) != 1) {		mills = tm;		break;		}
					st = conn->Receive( (void *)&((unsigned char *)&head)[ind], sz );
					if (st <= 0) {		mills = tm;		break;		}
					new_tick = GetTickCount();	tick = new_tick - tick;
					in_tcs += tick;		in_bytes += st;
					mills += tick;		tick = new_tick;
					ind += st;		sz -= st;
					if (!sz) {		state = 1;	ind = 0;	sz = (unsigned long)head.length;
									if (sz != size) 	mills = tm;
									break;		}
					else if (ind < sizeof(head))	break;
					mills = tm;		break;
				case 1 :
					new_tick = tm - mills;
					if (conn->SelectIn( new_tick ) != 1) {		mills = tm;		break;		}
					st = conn->Receive( (void *)&((unsigned char *)buffer)[ind], sz );
					if (st <= 0) {		mills = tm;		break;		}
					new_tick = GetTickCount();	tick = new_tick - tick;
					in_tcs += tick;		in_bytes += st;
					mills += tick;		tick = new_tick;
					ind += st;		sz -= st;
					if (!sz)
					{	if (jitter_diff_flag)
						{	jitter = mills - (head.tick_count + jitter_diff);
							if (result->in_jitter_min_ms > jitter)		result->in_jitter_min_ms = jitter;
							if (result->in_jitter_max_ms < jitter)		result->in_jitter_max_ms = jitter;
							if (callback)	callback->In( mills, jitter );		}
						else {		jitter_diff_flag = true;	jitter_diff = mills - 10;	}
						state = 0;		ind = 0;	sz = sizeof(head);		break;		}
					else if (ind < (unsigned long)head.length)		break;
					mills = tm;		break;
				default :	break;
		}	}	}
		free( (void *)buffer );		in_flag = false;
	}
	// end VS_BwtArgs::GoReceive

	inline unsigned CalcIntermediate( void )
	{
		unsigned long   mills = seconds * 1400, tick = GetTickCount(), new_tick;
		const volatile double   &out_bytes = result->out_bytes, &in_bytes = result->in_bytes;
		volatile float   &out_bps = result->out_bps, &in_bps = result->in_bps;
		while (mills && (out_flag || in_flag))
		{
			Sleep( 250 );
			new_tick = GetTickCount();	tick = new_tick - tick;
			mills = tick > mills ? 0 : mills - tick;		tick = new_tick;
			if (out_tcs)	out_bps = (float)((double)( out_bytes * 1000 ) / out_tcs );
			if (in_tcs)		in_bps = (float)((double)( in_bytes * 1000 ) / in_tcs );
			if (callback && !callback->Result( VS_BWT_ST_INTER_RESULT, result, 0 ))
			{	return VS_BWT_RET_TEST_STOP;	}
		}
		return 0;
	}
	// end VS_BwtArgs::CalcIntermediate
};
// end VS_BwtArgs

//////////////////////////////////////////////////////////////////////////////////////////

struct BwtAsyncArgs
{
	BwtAsyncArgs( unsigned *ret_code, const char *endpoint, VS_BwtResult *result,
					VS_BwtIntermediate *callbacks, const unsigned mode, const unsigned seconds,
					const unsigned long frames_size, const unsigned long period )
	: isValid(false), ret_code(ret_code), endpoint(0), result(result), callbacks(callbacks)
	, mode(mode), seconds(seconds), frames_size(frames_size), period(period)
	{	if (!result || !endpoint
				|| seconds < VS_BWT_MIN_TEST_SECONDS || seconds > VS_BWT_MAX_TEST_SECONDS
				|| frames_size < VS_ACS_BWT_MIN_HEX_BUFFER_SIZE || frames_size > VS_ACS_BWT_MAX_HEX_BUFFER_SIZE
				|| period > VS_ACS_BWT_MAX_PERIOD)
		{	if (ret_code)	*ret_code = VS_BWT_RET_ERR_ARGS;	return;		}
		BwtAsyncArgs::endpoint = _strdup( endpoint );
		if (!BwtAsyncArgs::endpoint)	return;
		isValid = true;		}
	// end BwtAsyncArgs::BwtAsyncArgs
	~BwtAsyncArgs( void ) {		if (endpoint)	free( (void *)endpoint );	}
	// end BwtAsyncArgs::~BwtAsyncArgs
	bool   isValid;		unsigned   *ret_code;	const char   *endpoint;
	VS_BwtResult   *result;						VS_BwtIntermediate   *callbacks;
	const unsigned   mode, seconds;				const unsigned long   frames_size, period;
	void Bwt( void )
	{	const unsigned   res = VS_Bwt( endpoint, result, callbacks, mode, seconds, frames_size, period );
		if (ret_code)		*ret_code = res;	}
	// end BwtAsyncArgs::Bwt
	static void __cdecl Bwt( void *_args )
	{
		vs::SetThreadName("BwtAsync");
		if (_args) {	BwtAsyncArgs   *args = (BwtAsyncArgs *)_args;
						args->Bwt();	delete args;	}		_endthread();	}
	// end BwtAsyncArgs::Bwt
};
// end BwtAsyncArgs struct

bool VS_BwtAsync( unsigned *ret_code, const char *server_endpoint_name, VS_BwtResult *result,
					VS_BwtIntermediate *callbacks, const unsigned mode, const unsigned seconds,
					const unsigned long frames_size, const unsigned long period )
{
	bool   ret = false;
	BwtAsyncArgs   *args = new BwtAsyncArgs( ret_code, server_endpoint_name, result, callbacks, mode, seconds, frames_size, period );
	if (args) {		if (args->isValid)
					{	uintptr_t   thr = _beginthread( BwtAsyncArgs::Bwt, 0, (void *)args );
						ret = thr && thr != -1L;	}
					if (!ret)	delete args;	}
	if (!ret && ret_code)	*ret_code = VS_BWT_RET_ERR_START_ASYNC;
	return ret;
}
// end VS_BwtAsync

//////////////////////////////////////////////////////////////////////////////////////////

static inline unsigned TestSend( VS_BwtArgs & );
static inline unsigned TestReceive( VS_BwtArgs & );
static inline unsigned TestDuplex( VS_BwtArgs & );
static inline unsigned TestHalfDuplex( VS_BwtArgs & );

unsigned VS_Bwt( const char *endpoint, VS_BwtResult *result, VS_BwtIntermediate *callback,
							const unsigned mode, const unsigned seconds,
							const unsigned long frames_size, const unsigned long period )
{	if (!result || !endpoint
			|| seconds < VS_BWT_MIN_TEST_SECONDS || seconds > VS_BWT_MAX_TEST_SECONDS
			|| frames_size < VS_ACS_BWT_MIN_HEX_BUFFER_SIZE || frames_size > VS_ACS_BWT_MAX_HEX_BUFFER_SIZE
			|| period > VS_ACS_BWT_MAX_PERIOD)
		return VS_BWT_RET_ERR_ARGS;
	VS_BwtArgs   args( result, endpoint, seconds, frames_size, period, callback );
	if (!args.IsValid())	return VS_BWT_RET_ERR_INIT;
	switch (mode)
	{
	case VS_BWT_MODE_OUT :			return TestSend( args );
	case VS_BWT_MODE_IN :			return TestReceive( args );
	case VS_BWT_MODE_DUPLEX :		return TestDuplex( args );
	case VS_BWT_MODE_HALFDUPLEX :	return TestHalfDuplex( args );
	default :						return VS_BWT_RET_ERR_ARGS;
}	}
// end VS_Bwt

//////////////////////////////////////////////////////////////////////////////////////////

static inline unsigned GoConnect( VS_ConnectionSock *&sock, VS_BwtArgs &args, const unsigned type )
{
	if (args.callback && !args.callback->Result( VS_BWT_ST_START_CONNECT, 0, type ))
	{	return VS_BWT_RET_TEST_STOP;	}
	class CreationAttempts : public VS_CreationAttempts
	{	public:	CreationAttempts( VS_BwtArgs &args ) : isValid(true),
			status(VS_BWT_ST_CONNECT_ERROR), args(args), con_att(0) {}
		// end CreationAttempts::CreationAttempts
		bool   isValid;		unsigned   status;
		VS_BwtArgs   &args;		unsigned   con_att;
		bool Attempt(unsigned nTCP, const net::endpoint::ConnectTCP* rc)
		{	if (args.callback)	isValid = args.callback->Result( VS_BWT_ST_CONNECT_ATTEMPT, rc, ++con_att );
			return isValid;
	}	};	// end CreationAttempts class
	unsigned long   mills = 30000;		unsigned   nTCP = 0, againCount = 5;
	CreationAttempts   attempts( args );
go_again:		--againCount;
	VS_ConnectionSock   *loc_sock = VS_CreateConnection( args.endpoint, mills, &nTCP, false, &attempts );
	if (!attempts.isValid)		return VS_BWT_RET_TEST_STOP;
	if (!loc_sock)
	{	if (args.callback && !args.callback->Result( VS_BWT_ST_CONNECT_ERROR, 0, type ))
		{	return VS_BWT_RET_TEST_STOP;	}
		return VS_BWT_RET_ERR_CONNECT;
	}
	if (args.callback && !args.callback->Result( VS_BWT_ST_CONNECT_OK, 0, type ))
	{	delete loc_sock;	return VS_BWT_RET_TEST_STOP;	}
	if (nTCP > 1)	mills = 20000;
	if (args.callback && !args.callback->Result( VS_BWT_ST_START_HANDSHAKE, 0, type ))
	{	delete loc_sock;	return VS_BWT_RET_TEST_STOP;	}
	bool   hsRes = false;	unsigned   mark = 0;
	{	const unsigned long   sz_endpoint = (unsigned long)strlen( args.endpoint ),
								sz = sizeof(VS_BwtHandshake) + 1 + sz_endpoint + 1;
		VS_BwtHandshake   *hs = (VS_BwtHandshake *)malloc( (size_t)sz );
		if (hs)
		{	memset( (void *)hs, 0, sizeof(hs) );
			strncpy(hs->hs.primary_field, VS_Bwt_PrimaryField, sizeof(hs->hs.primary_field));
			hs->hs.body_length = sizeof(VS_BwtHandshake) - sizeof(net::HandshakeHeader)
									+ 1 + sz_endpoint + 1 - 1;
			hs->type = (unsigned char)!type;	hs->tm = args.seconds * 1000;
			hs->size = args.size;		hs->period = args.period;
			unsigned char   *p = &((unsigned char *)hs)[sizeof(VS_BwtHandshake)];
			*p = (unsigned char)sz_endpoint;	++p;	strcpy( (char *)p, args.endpoint );
			net::UpdateHandshakeChecksums(hs->hs);
			unsigned long   latency = GetTickCount();
			if (VS_WriteZeroHandshake(loc_sock, (net::HandshakeHeader *)hs, mills))
			{	VS_BwtHandshakeReply   *hr = 0;
				if (VS_ReadZeroHandshake(loc_sock, (net::HandshakeHeader **)&hr, mills))
				{	latency = GetTickCount() - latency;
					if (type == VS_ACS_LIB_SENDER)	args.result->out_response_ms = latency;
					else							args.result->in_response_ms = latency;
					if (!hr->resultCode)	hsRes = true;
					else	mark = hr->resultCode;
					free( (void *)hr );
			}	}
			free( (void *)hs );
	}	}
	if (!hsRes)
	{	if (args.callback && !args.callback->Result( VS_BWT_ST_HANDSHAKE_ERROR, 0, mark ))
		{	delete loc_sock;	return VS_BWT_RET_TEST_STOP;	}	delete loc_sock;
		if ((!mark || mark == 1) && againCount)		goto go_again;
		return VS_BWT_RET_ERR_HANDSHAKE;
	}
	if (nTCP > 1)
		net::endpoint::MakeFirstConnectTCP(nTCP, args.endpoint);
	if (args.callback && !args.callback->Result( VS_BWT_ST_HANDSHAKE_OK, 0, type ))
	{	delete loc_sock;	return VS_BWT_RET_TEST_STOP;	}
	sock = loc_sock;	return 0;
}
// end GoConnect

//////////////////////////////////////////////////////////////////////////////////////////

struct ThreadArgs
{
	ThreadArgs( VS_BwtArgs &args, VS_ConnectionSock *sock ) : args(args), sock(sock) {}
	// end ThreadArgs::ThreadArgs
	~ThreadArgs( void ) {	delete sock;	}
	// end ThreadArgs::~ThreadArgs
	VS_BwtArgs   &args;		VS_ConnectionSock   *sock;
	inline void ThreadSend( void ) {	args.GoSend( sock );	}
	// end ThreadArgs::ThreadSend
	inline void ThreadReceive( void ) {		args.GoReceive( sock );		}
	// end ThreadArgs::ThreadReceive
};	// end ThreadArgs struct

static unsigned __stdcall ThreadSend( void *args )
{
	vs::SetThreadName("BwtSender");
	static_cast<ThreadArgs*>(args)->ThreadSend();
	_endthreadex(0);
	return 0;
}
// end ThreadSend

static unsigned __stdcall ThreadReceive( void *args )
{
	vs::SetThreadName("BwtReceiver");
	static_cast<ThreadArgs*>(args)->ThreadReceive();
	_endthreadex(0);
	return 0;
}
// end ThreadReceive

static inline unsigned CheckThread( VS_BwtArgs &args, uintptr_t thread, const unsigned type )
{
	if (!thread || thread == -1L)
	{	if (args.callback && !args.callback->Result( VS_BWT_ST_CONNECTION_ERROR, 0, type ))
		{	return VS_BWT_RET_TEST_STOP;	}
		return VS_BWT_RET_ERR_CONNECTION;	}
	return 0;
}
// end CheckThread

//////////////////////////////////////////////////////////////////////////////////////////

static inline unsigned TestSend( VS_BwtArgs &args )
{
	VS_ConnectionSock   *sock = 0;
	unsigned   res = GoConnect( sock, args, VS_ACS_LIB_SENDER );
	if (res) {		return res;		}
	void   *sock_handle = sock->GetHandle();
	ThreadArgs   th_args( args, sock );
	HANDLE   thread = (HANDLE)_beginthreadex( 0, 0, ThreadSend, (void *)&th_args, 0, 0 );
	res = CheckThread( args, (uintptr_t)thread, VS_ACS_LIB_SENDER );
	if (res) {		return res;		}
	if (args.callback && !args.callback->Result( VS_BWT_ST_START_TEST, 0, 0 ))
	{	res = VS_BWT_RET_TEST_STOP;		}
	else {	args.out_flag = true;	res = args.CalcIntermediate();	}
	VS_ConnectionSock::Break( sock_handle );
	if (WaitForSingleObject( thread, 10000 ) == WAIT_TIMEOUT)
		TerminateThread( thread, 0 );
	CloseHandle( thread );
	if (res) {		return res;		}
	if (args.callback && !args.callback->Result( VS_BWT_ST_FINISH_TEST, 0, 0 ))
	{	return VS_BWT_RET_TEST_STOP;	}
	return 0;
}
// end TestSend

static inline unsigned TestReceive( VS_BwtArgs &args )
{
	VS_ConnectionSock   *sock = 0;
	unsigned   res = GoConnect( sock, args, VS_ACS_LIB_RECEIVER );
	if (res) {		return res;		}
	void   *sock_handle = sock->GetHandle();
	ThreadArgs   th_args( args, sock );
	HANDLE   thread = (HANDLE)_beginthreadex( 0, 0, ThreadReceive, (void *)&th_args, 0, 0 );
	res = CheckThread( args, (uintptr_t)thread, VS_ACS_LIB_RECEIVER );
	if (res) {		return res;		}
	if (args.callback && !args.callback->Result( VS_BWT_ST_START_TEST, 0, 0 ))
	{	res = VS_BWT_RET_TEST_STOP;		}
	else {	args.in_flag = true;	res = args.CalcIntermediate();	}
	VS_ConnectionSock::Break( sock_handle );
	if (WaitForSingleObject( thread, 10000 ) == WAIT_TIMEOUT)
		TerminateThread( thread, 0 );
	CloseHandle( thread );
	if (res) {		return res;		}
	if (args.callback && !args.callback->Result( VS_BWT_ST_FINISH_TEST, 0, 0 ))
	{	return VS_BWT_RET_TEST_STOP;	}
	return 0;
}
// end TestReceive

static inline unsigned TestDuplex( VS_BwtArgs &args )
{
	VS_ConnectionSock   *sock = 0;
	unsigned   res = GoConnect( sock, args, VS_ACS_LIB_SENDER );
	if (res)	return res;
	void   *out_sock_handle = sock->GetHandle();
	ThreadArgs   out_th_args( args, sock );		sock = 0;
	res = GoConnect( sock, args, VS_ACS_LIB_RECEIVER );
	if (res)	return res;
	void   *in_sock_handle = sock->GetHandle();
	ThreadArgs   in_th_args( args, sock );
	HANDLE   thread[2] = { (HANDLE)_beginthreadex( 0, 0, ThreadSend, (void *)&out_th_args, 0, 0 ),
							(HANDLE)_beginthreadex( 0, 0, ThreadReceive, (void *)&in_th_args, 0, 0 )	};
	{	const unsigned   res1 = CheckThread( args, (uintptr_t)thread[0], VS_ACS_LIB_SENDER ),
						res2 = CheckThread( args, (uintptr_t)thread[1], VS_ACS_LIB_RECEIVER );
		if (res1)
		{	if (!res2)
			{	VS_ConnectionSock::Break( in_sock_handle );
				if (WaitForSingleObject( thread[1], 10000 ) == WAIT_TIMEOUT)
					TerminateThread( thread[1], 0 );
				CloseHandle( thread[1] );
			}
			return res1;
		}
		if (res2)
		{	VS_ConnectionSock::Break( out_sock_handle );
			if (WaitForSingleObject( thread[0], 10000 ) == WAIT_TIMEOUT)
				TerminateThread( thread[0], 0 );
			CloseHandle( thread[0] );		return res2;
	}	}
	if (args.callback && !args.callback->Result( VS_BWT_ST_START_TEST, 0, 0 ))
	{	res = VS_BWT_RET_TEST_STOP;		}
	else {	args.in_flag = args.out_flag = true;	res = args.CalcIntermediate();	}
	VS_ConnectionSock::Break( out_sock_handle );	VS_ConnectionSock::Break( in_sock_handle );
	if (WaitForMultipleObjects( 2, thread, TRUE, 10000 ) == WAIT_TIMEOUT)
	{	TerminateThread( thread[0], 0 );	TerminateThread( thread[1], 0 );	}
	CloseHandle( thread[0] );	CloseHandle( thread[1] );
	if (res)	return res;
	if (args.callback && !args.callback->Result( VS_BWT_ST_FINISH_TEST, 0, 0 ))
	{	return VS_BWT_RET_TEST_STOP;	}
	return 0;
}
// end TestDuplex

static inline unsigned TestHalfDuplex( VS_BwtArgs &args )
{
	unsigned   res = TestSend( args );
	if (res)	return res;
	return TestReceive( args );
}
// end TestHalfDuplex

#endif
