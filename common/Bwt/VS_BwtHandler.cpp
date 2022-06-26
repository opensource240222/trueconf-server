
//#define   _MY_DEBUG_
#if _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <Windows.h>

#include "../std/cpplib/VS_MemoryLeak.h"
#include "VS_BwtLib.h"
#include "VS_BwtHandler.h"
#include "VS_BwtHexBuffer.h"
#include "../acs/Lib/VS_AcsLib.h"
#include "../streams/Protocol.h"
#include "std/cpplib/ThreadUtils.h"

extern std::string g_tr_endpoint_name;

bool VS_BwtHandler::IsValid( void ) const
{	return VS_AccessConnectionHandler::IsValid();	}
// end VS_BwtHandler::IsValid

bool VS_BwtHandler::Init( const char *handler_name )
{
#ifdef   _MY_DEBUG_
printf( "VS_BwtHandler::Init( handler_name: %s )\n", handler_name );
#endif
	return true;
}
// end VS_BwtHandler::Create

VS_ACS_Response VS_BwtHandler::Connection( unsigned long *in_len )
{
#ifdef   _MY_DEBUG_
printf( "VS_BwtHandler::Connection( *in_len: %u )\n", *in_len );
#endif
	if (in_len)
		*in_len = sizeof(net::HandshakeHeader);
	return vs_acs_next_step;
}
// end VS_BwtHandler::Connection

VS_ACS_Response VS_BwtHandler::Protocol( const void *in_buffer, unsigned long *in_len,
												void **out_buffer, unsigned long *out_len,
												void **context )
{
#ifdef   _MY_DEBUG_
printf( "VS_BwtHandler::Protocol( ..., *in_len: %u, ... )\n", *in_len );
#endif
	if (*in_len < sizeof(net::HandshakeHeader))
	{
		*in_len = sizeof(net::HandshakeHeader);
		return vs_acs_next_step;
	}
	net::HandshakeHeader   *hs = (net::HandshakeHeader *)in_buffer;
	if (hs->head_cksum != net::GetHandshakeHeaderChecksum(*hs)
			|| strncmp( hs->primary_field, VS_Bwt_PrimaryField, sizeof(hs->primary_field) ))
		return vs_acs_connection_is_not_my;
	const unsigned long   body_length = hs->body_length + 1,
						handshake_length = sizeof(net::HandshakeHeader) + body_length;
	if (*in_len > handshake_length)		return vs_acs_connection_is_not_my;
	if (*in_len < handshake_length)
	{	*in_len = handshake_length;		return vs_acs_my_connections;	}
	if (hs->body_cksum != net::GetHandshakeBodyChecksum(*hs))
		return vs_acs_connection_is_not_my;
	return vs_acs_accept_connections;
}
// end VS_BwtHandler::Protocol

struct BwtArgs
{
	BwtArgs( VS_ConnectionSock *conn, const unsigned type, const unsigned long tm,
								const unsigned long size, const unsigned long period,
								const char *my_endpoint )
		: conn(conn), type(type), tm(tm), size(size), period(period)
		, my_endpoint(_strdup(my_endpoint)) {}
	// end BwtArgs::BwtArgs

	~BwtArgs( void )
	{	delete conn;	if (my_endpoint)	free( (void *)my_endpoint );	}
	// end BwtArgs::~BwtArgs

	VS_ConnectionSock   *conn;
	const unsigned   type;		const unsigned long   tm, size, period;
	const char   *my_endpoint;

	inline void GoSend( void )
	{
		void   *buffer = 0;
		conn->SetFastSocket(true);
		const unsigned long size_frame = size + sizeof(stream::FrameHeader);
		{	unsigned char   *bf = (unsigned char *)malloc( (size_t)size_frame );
			if (!bf)	return;		buffer = (void *)bf;	bf += sizeof(stream::FrameHeader);
			unsigned long   ind = 0, sz = size;
			while (ind < size)
			{	memcpy( &bf[ind], VS_HexBuffer, sz );	ind += sz;		sz = size - ind;
				if (sz > VS_ACS_BWT_HEX_BUFFER_SIZE)	sz = VS_ACS_BWT_HEX_BUFFER_SIZE;
		}	}
		stream::FrameHeader& head = *(stream::FrameHeader *)buffer;
		head.length = (unsigned short)size;
		head.track = stream::Track::old_command;
		head.cksum = stream::GetFrameBodyChecksum(static_cast<uint8_t*>(buffer) + sizeof(stream::FrameHeader), size);
		unsigned long   mills = 0, tick = GetTickCount(), new_tick, cur_bwt;
		const unsigned long   due_bwt = !period ? ~0 : (unsigned long)( (double)( 1000./ period ) * size );
		double   out_bytes = 0;
		while (mills < tm)
		{
			head.tick_count = mills;
			if (conn->Send( buffer, size_frame ) != (int)size_frame)	break;
			new_tick = GetTickCount();		tick = new_tick - tick;
			out_bytes += size_frame;		mills += tick;
			cur_bwt = mills ? (unsigned long)((( out_bytes * 1000 ) + (( mills + 1 ) / 2 )) / mills ) : ~0;
			if (cur_bwt <= due_bwt )	tick = new_tick;
			if (tick > period)		tick = new_tick;
			else
			{	if (tick >= period)		tick = new_tick;
				else
				{	Sleep( period - tick );		tick = new_tick;
					new_tick = GetTickCount();		tick = new_tick - tick;
					mills += tick;		tick = new_tick;
		}	}	}
		if (buffer != (void *)VS_HexBuffer)		free( (void *)buffer );
	}
	// end BwtArgs::GoSend

	inline void GoReceive( void )
	{
		void   *buffer = malloc( (size_t)size );
		if (!buffer)	return;			int   st;
		stream::FrameHeader head;		unsigned state = 0, ind = 0, sz = sizeof(head);
		unsigned long   mills = 0, tm = BwtArgs::tm * 4, tick = GetTickCount(), new_tick;
		while (mills < tm)
		{	switch (state)
			{
			case 0 :
				new_tick = tm - mills;
				if (conn->SelectIn( new_tick ) != 1) {		mills = tm;		break;		}
				st = conn->VS_ConnectionSock::Receive( (void *)&((unsigned char *)&head)[ind], sz );
				if (st <= 0) {		mills = tm;		break;		}
				ind += st;		sz -= st;
				if (!sz) {		state = 1;	ind = 0;	sz = (unsigned long)head.length;
								if (sz != size)		mills = tm;
								break;		}
				else if (ind < sizeof(head))	break;
				mills = tm;		break;
			case 1 :
				new_tick = tm - mills;
				if (conn->SelectIn( new_tick ) != 1) {		mills = tm;		break;		}
				st = conn->VS_ConnectionSock::Receive( (void *)&((unsigned char *)buffer)[ind], sz );
				if (st <= 0) {		mills = tm;		break;		}
				ind += st;		sz -= st;
				if (!sz) {		state = 0;	ind = 0;	sz = sizeof(head);	break;		}
				else if (ind < (unsigned long)head.length)	break;
				mills = tm;		break;
			default :	break;
			}
			new_tick = GetTickCount();		tick = new_tick - tick;
			mills += tick;		tick = new_tick;
		}
		free( (void *)buffer );
	}
	// end BwtArgs::GoReceive

	inline void Thread( void )
	{
		{
			const char   *our_name = g_tr_endpoint_name.c_str();
			VS_BwtHandshakeReply   hr;		memset( (void *)&hr, 0, sizeof(hr) );
			hr.hs.body_length = sizeof(hr.resultCode) - 1;
			if (!our_name || strcmp( our_name, my_endpoint ))	hr.resultCode = 1;
			net::UpdateHandshakeChecksums(hr.hs);
			conn->VS_ConnectionSock::Send((void *)&hr, sizeof(hr));
			if (hr.resultCode)		return;		}
		SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );
		if (type == VS_ACS_LIB_SENDER)	GoSend();
		else							GoReceive();
	}
	// end BwtArgs::Thread

	static void __cdecl Thread( void *args )
	{
		vs::SetThreadName("BwtHandler");
		static_cast<BwtArgs*>(args)->Thread();
		delete static_cast<BwtArgs*>(args);
		_endthread();
	}
	// end BwtArgs::Thread
};
// end BwtArgs struct

void VS_BwtHandler::Accept( VS_ConnectionTCP *conn, const void *in_buffer,
								const unsigned long in_len, const void *context )
{
#ifdef   _MY_DEBUG_
printf( "VS_BwtHandler::Accept( conn: %08X, ..., in_len: %u, ... )\n", _MD_POINT_(conn), in_len );
#endif

	VS_BwtHandshake   *hs = (VS_BwtHandshake *)in_buffer;
	const unsigned long handshake_length = sizeof(net::HandshakeHeader) + hs->hs.body_length + 1;
	char   *my_endpoint = &((char *)in_buffer)[sizeof(VS_BwtHandshake) + 1];
	((char *)hs)[handshake_length - 1] = 0;
	BwtArgs   *args = new BwtArgs( conn, hs->type, hs->tm, hs->size, hs->period, my_endpoint );
	if (!args) {	delete conn;	return;		}
	uintptr_t   ptr = _beginthread( BwtArgs::Thread, 0, (void *)args );
	if (!ptr || ptr == -1L)		delete args;
}
// end VS_BwtHandler::Accept

void VS_BwtHandler::Destructor( const void *context )
{
#ifdef   _MY_DEBUG_
printf( "VS_BwtHandler::Destructor( context: %08X )\n", _MD_POINT_(context) );
#endif
}
// end VS_BwtHandler::Destructor

void VS_BwtHandler::Destroy( const char *handler_name )
{
#ifdef   _MY_DEBUG_
printf( "VS_BwtHandler::Destroy( handler_name: %s )\n", handler_name );
#endif
}
// end VS_BwtHandler::Destroy
#endif
