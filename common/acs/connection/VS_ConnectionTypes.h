/**
 **************************************************************************
 * \file VS_ConnectionTypes.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief The most powerfull file.
 *
 * There is a lot of needed implementations.
 *
 *
 *
 * \b Project
 * \author SlavetskyA
 * \date 01.10.02
 *
 * $Revision: 12 $
 *
 * $History: VS_ConnectionTypes.h $
 *
 * *****************  Version 12  *****************
 * User: Mushakov     Date: 7.08.12    Time: 21:42
 * Updated in $/VSNA/acs/Connection
 * - static var isDisableNagle  removed
 *
 * *****************  Version 11  *****************
 * User: Mushakov     Date: 4.06.12    Time: 20:18
 * Updated in $/VSNA/acs/Connection
 * - bind 0.0.0.0 in transcoder
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 8.02.12    Time: 18:09
 * Updated in $/VSNA/acs/Connection
 *  - call by UDP fixed
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 8.09.11    Time: 21:20
 * Updated in $/VSNA/acs/Connection
 * - Gateway with server transport
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 25.01.10   Time: 15:02
 * Updated in $/VSNA/acs/connection
 *  - direct UDP realized
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 16.01.10   Time: 18:57
 * Updated in $/VSNA/acs/connection
 * - SSL without EncryptLSP
 * - code for SSL throu EncryptLSP removed
 *
 * *****************  Version 6  *****************
 * User: Dront78      Date: 29.12.09   Time: 17:07
 * Updated in $/VSNA/acs/connection
 * - socket provider refactor. default socket creation time speedup
 *
 * *****************  Version 5  *****************
 * User: Dront78      Date: 14.12.09   Time: 17:09
 * Updated in $/VSNA/acs/connection
 * - bugfix 6848
 * - bugfix 6849
 *
 * *****************  Version 4  *****************
 * User: Dront78      Date: 9.02.09    Time: 19:43
 * Updated in $/VSNA/acs/connection
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 18.06.08   Time: 17:00
 * Updated in $/VSNA/acs/connection
 * - VS_MemoryLeak included
 * - Logging to smtp service added
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:01
 * Updated in $/VSNA/acs/connection
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 20  *****************
 * User: Dront78      Date: 7.11.07    Time: 17:15
 * Updated in $/VS2005/acs/connection
 * SocketAny rewrited to be more simple and universal.
 *
 * *****************  Version 19  *****************
 * User: Dront78      Date: 6.11.07    Time: 19:00
 * Updated in $/VS2005/acs/connection
 * Typing errors fixed
 *
 * *****************  Version 18  *****************
 * User: Dront78      Date: 6.11.07    Time: 18:47
 * Updated in $/VS2005/acs/connection
 * Added EncryptLSP forced disable.
 *
 * *****************  Version 17  *****************
 * User: Mushakov     Date: 31.10.07   Time: 17:09
 * Updated in $/VS2005/acs/connection
 * set socket mode to non-blocking after connect asynch
 *
 * *****************  Version 16  *****************
 * User: Mushakov     Date: 23.10.07   Time: 20:06
 * Updated in $/VS2005/acs/connection
 *  - bad result of WSAEventSelect handling added
 *
 * *****************  Version 15  *****************
 * User: Mushakov     Date: 22.10.07   Time: 19:46
 * Updated in $/VS2005/acs/connection
 *  - long time connection fixed
 *  - clearing handle added to VS_ConnectionTypes.h
 *
 * *****************  Version 14  *****************
 * User: Dront78      Date: 18.07.07   Time: 18:06
 * Updated in $/VS2005/acs/connection
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 17.07.07   Time: 14:43
 * Updated in $/VS2005/acs/connection
 * - time for sockect create now taking into account
 *
 * *****************  Version 12  *****************
 * User: Dront78      Date: 10.07.07   Time: 18:21
 * Updated in $/VS2005/acs/connection
 * Code updated to work with disabled QoS Service
 *
 * *****************  Version 11  *****************
 * User: Dront78      Date: 25.06.07   Time: 16:43
 * Updated in $/VS2005/acs/connection
 *
 * *****************  Version 10  *****************
 * User: Ktrushnikov  Date: 9.04.07    Time: 19:14
 * Updated in $/VS2005/acs/connection
 * - Disconnect(): don't set isAccept = false, it will be set overlapped
 * by SetAcceptResult()
 *
 * *****************  Version 9  *****************
 * User: Dront78      Date: 6.04.07    Time: 18:06
 * Updated in $/VS2005/acs/connection
 *
 * *****************  Version 8  *****************
 * User: Dront78      Date: 22.03.07   Time: 17:04
 * Updated in $/VS2005/acs/connection
 *
 * *****************  Version 7  *****************
 * User: Dront78      Date: 22.03.07   Time: 11:06
 * Updated in $/VS2005/acs/connection
 *
 * *****************  Version 5  *****************
 * User: Dront78      Date: 20.03.07   Time: 19:44
 * Updated in $/VS2005/acs/connection
 *
 * *****************  Version 4  *****************
 * User: Avlaskin     Date: 15.03.07   Time: 12:55
 * Updated in $/VS2005/acs/connection
 * + tos added
 *
 * *****************  Version 3  *****************
 * User: Avlaskin     Date: 21.02.07   Time: 16:31
 * Updated in $/VS2005/acs/connection
 * + 0.0.0.0 added
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 5.02.07    Time: 21:30
 * Updated in $/VS2005/acs/connection
 * - project configuration
 * - depricated functions warnings suppressed
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/acs/connection
 *
 * *****************  Version 101  *****************
 * User: Mushakov     Date: 27.11.06   Time: 13:01
 * Updated in $/VS/acs/connection
 * delete using CancelIo;
 * delete using TransmitFile;
 *
 *
 * *****************  Version 100  *****************
 * User: Mushakov     Date: 17.11.06   Time: 13:50
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 99  *****************
 * User: Mushakov     Date: 17.11.06   Time: 11:22
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 98  *****************
 * User: Mushakov     Date: 15.11.06   Time: 11:03
 * Updated in $/VS/acs/connection
 *  - asynchronous ClientSecureHandshake added to TransportRouter
 *
 * *****************  Version 97  *****************
 * User: Mushakov     Date: 26.10.06   Time: 14:30
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 96  *****************
 * User: Mushakov     Date: 13.10.06   Time: 10:47
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 95  *****************
 * User: Mushakov     Date: 21.09.06   Time: 15:26
 * Updated in $/VS/acs/connection
 * Test encryptLsp projects were added
 *
 * *****************  Version 94  *****************
 * User: Mushakov     Date: 20.06.06   Time: 14:43
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 93  *****************
 * User: Avlaskin     Date: 9.06.06    Time: 17:38
 * Updated in $/VS/acs/connection
 * SIP add ons
 *
 * *****************  Version 92  *****************
 * User: Avlaskin     Date: 22.05.06   Time: 18:32
 * Updated in $/VS/acs/connection
 * pipe file deleting
 *
 * *****************  Version 91  *****************
 * User: Mushakov     Date: 27.04.06   Time: 13:10
 * Updated in $/VS/acs/connection
 * updated manage of certificate
 *
 * *****************  Version 90  *****************
 * User: Avlaskin     Date: 26.04.06   Time: 16:37
 * Updated in $/VS/acs/connection
 * added accept timing counter
 *
 * *****************  Version 89  *****************
 * User: Mushakov     Date: 18.04.06   Time: 19:26
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 88  *****************
 * User: Mushakov     Date: 13.01.06   Time: 16:42
 * Updated in $/VS/acs/connection
 * Added SecureLib, SecureHandshake
 *
 * *****************  Version 87  *****************
 * User: Avlaskin     Date: 15.11.05   Time: 20:15
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 85  *****************
 * User: Mushakov     Date: 31.10.05   Time: 17:18
 * Updated in $/VS/acs/Connection
 * Добавил механизм Handshake для Nat Hole Panching.
 *
 * *****************  Version 84  *****************
 * User: Mushakov     Date: 15.09.05   Time: 12:38
 * Updated in $/VS/acs/Connection
 * Добавлен метод ReceiveFrom в класс VS_ConnectionSock_Implementation
 *
 * *****************  Version 83  *****************
 * User: Avlaskin     Date: 4.08.05    Time: 10:15
 * Updated in $/VS/acs/connection
 * new overload of RecieveFrom
 *
 * *****************  Version 82  *****************
 * User: Avlaskin     Date: 5.05.05    Time: 16:23
 * Updated in $/VS/acs/connection
 * Release build -  fixed.
 *
 * *****************  Version 81  *****************
 * User: Avlaskin     Date: 5.05.05    Time: 15:35
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 80  *****************
 * User: Avlaskin     Date: 27.04.05   Time: 12:36
 * Updated in $/VS/acs/connection
 * Stream UDP multicast added
 *
 * *****************  Version 79  *****************
 * User: Avlaskin     Date: 24.01.05   Time: 18:28
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 78  *****************
 * User: Avlaskin     Date: 24.01.05   Time: 15:20
 * Updated in $/VS/acs/connection
 *
 * *****************  Version 77  *****************
 * User: Avlaskin     Date: 17.12.04   Time: 20:07
 * Updated in $/VS/acs/connection
 *
 ****************************************************************************/
#ifndef VS_CONNECTION_TYPES_H
#define VS_CONNECTION_TYPES_H
//#include "../../std/cpplib/VS_MemoryLeak.h"

#include "../Lib/VS_IPPortAddress.h"

#ifdef _DEBUG
#include <stdio.h>
#else
//#define printf
#endif // _DEBUG

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#ifdef FD_SETSIZE
#undef FD_SETSIZE
#endif // FD_SETSIZE
#define   FD_SETSIZE   2
#include <Winsock2.h>
#include <iphlpapi.h>
#include <Mswsock.h>
#include <Mstcpip.h>
#include <ws2tcpip.h>
#include <Qossp.h.>
#include "process.h"

#include "VS_Connection.h"
#include "VS_ConnectionSock.h"
#include "VS_ConnectionPipe.h"
#include "VS_ConnectionOv.h"
#include "../Lib/VS_AcsLib.h"
#include "../Lib/VS_QoSEnabledSocket.h"
//#include "../Lib/VS_AcsLog.h"

#include "../../net/Lib.h"
#include "../../std/cpplib/VS_RegistryKey.h"
#include "std-generic/cpplib/hton.h"

#include <vector>

#pragma comment(lib, "IPHLPAPI.lib")

extern struct sockaddr   zero_addr;
extern OVERLAPPED   over_zero;
extern struct VS_Overlapped   ovZero;

static const GUID VS_SECURE_QOS_PROVIDER_GUID =
	{ 0x6918c44d, 0x8b52, 0x4649, { 0x98, 0xd9, 0x81, 0x16, 0xaf, 0xf6, 0xe4, 0x99 } };

// Default QOS
// All digit parameters are ingnored because SERVICE_NO_QOS_SIGNALING set to compability with RSVP in Windows 2000
// This working for default mode only and can be overrided by setting QoSdata structure.
extern signed long __enable_QoS;
extern FLOWSPEC default_g711; // =	{680000, 68000, 1360000, QOS_NOT_SPECIFIED, QOS_NOT_SPECIFIED, __enable_QoS | SERVICE_NO_QOS_SIGNALING, 340, 340};
extern _QualityOfService g711_qos; // = {default_g711, default_g711, 0};
extern void __apply_QoS_changes();
extern signed long __QoS_isOK;
extern signed long GetQoSEnable();

#define VS_NONE						0
#define VS_CLIENT_SECURE_HANDSHAKE	1
#define VS_SERVER_SECURE_HANDSHAKE	2

#define VS_SIGNAL_TO_IOCP			1
#define VS_SIGNAL_TO_EVENT			2
//////////////////////// CONNECTION IMPLEMENTATION ////////////////////////////

struct VS_Connection_Implementation
{
	VS_Connection_Implementation( void )
		: heap(0), heapSect(0), type(vs_connection_type_uninstalled)
		, hio(INVALID_HANDLE_VALUE), hiocp(0)
		, writeSize(-1), readSize(-1), writeEvent(0), readEvent(0),connectEvent(0)
		, isWriteOv(false), isReadOv(false), isWrite(false), isRead(false), fRdOv(false)
		, wB(0), wBM(0), rB(0), rBM(0), writeOv(ovZero), readOv(ovZero) {}
	// end of VS_Connection_Implementation::VS_Connection_Implementation

	~VS_Connection_Implementation( void )
	{	ResetWrite();	ResetRead();	CloseOvWriteEvent();	CloseOvReadEvent();
		if((connectEvent)&&(INVALID_HANDLE_VALUE != connectEvent))
		{
			CloseHandle(connectEvent);
		}
	}
	// end of VS_Connection_Implementation::~VS_Connection_Implementation

	HANDLE   heap;
	CRITICAL_SECTION   *heapSect;
	VS_Connection_Type   type;
	int   writeSize, readSize;
	HANDLE   hio, hiocp;
	HANDLE   writeEvent, readEvent,connectEvent;
	bool   isWriteOv, isReadOv, isWrite, isRead, fRdOv;
	void   *wB, *wBM, *rB, *rBM;
	VS_Overlapped   writeOv, readOv;

	inline void *ConnHeapAlloc( HANDLE heap, const unsigned long n_bytes )
	{
		if (heapSect)	EnterCriticalSection( heapSect );
		void* ret = HeapAlloc(heap, HEAP_NO_SERIALIZE, (SIZE_T)n_bytes);
		if (heapSect)	LeaveCriticalSection( heapSect );
		return ret;
	}
	// end of VS_Connection_Implementation::ConnHeapAlloc

	inline void ConnHeapFree( HANDLE heap, void *buffer )
	{
		if (heapSect)	EnterCriticalSection( heapSect );
		HeapFree( heap, HEAP_NO_SERIALIZE, buffer );
		if (heapSect)	LeaveCriticalSection( heapSect );
	}
	// end of VS_Connection_Implementation::ConnHeapFree

	inline bool Alloc( void *&buffer, const unsigned long n_bytes )
	{
		char   *bff = 0;
		if (heap)	bff = (char *)ConnHeapAlloc( heap, n_bytes );
		else				bff = (char *)malloc( (size_t)n_bytes );
		if (!bff)	return false;
		buffer = (void *)bff;
		return true;
	}
	// end of VS_Connection_Implementation::Alloc

	inline bool BAlloc( const VS_Buffer *buffers, const unsigned long n_buffers,
								void *&buffer, unsigned long &n_bytes )
	{
		char   *bff = 0;
		unsigned long   bts = VS_BuffersLength( buffers, n_buffers );
		if (!Alloc( (void *&)bff, bts ))	return false;
		VS_BuffersCopy( (void *)bff, buffers, n_buffers );
		buffer = (void *)bff;
		n_bytes = bts;
		return true;
	}
	// end of VS_Connection_Implementation::BAlloc

	inline void BFree( void *&buffer )
	{
		if (heap)	ConnHeapFree( heap, buffer );
		else		free( buffer );
		buffer = 0;
	}
	// end of VS_Connection_Implementation::BFree

	inline bool SetHeap( const void *handleHeap, const void *cSect )
	{
		if (isWrite || isRead)	return false;
		heap = (HANDLE)handleHeap;
		heapSect = (CRITICAL_SECTION *)cSect;
		return true;
	}
	// end of VS_Connection_Implementation::SetHeap

	inline bool CreateOvWriteEvent( void )
	{
		if (writeEvent)		return true;
		if (!(writeEvent = CreateEvent( 0, TRUE, FALSE, 0 )))	return false;
		return true;
	}
	// end of VS_Connection_Implementation::CreateOvWriteEvent
	inline bool CreateConnectEvent()
	{
		if (connectEvent && connectEvent!=INVALID_HANDLE_VALUE)		return true;
		if (!(connectEvent = CreateEvent( 0, TRUE, FALSE, 0 )))	return false;
		return true;
	}
	// end of VS_Connection_Implementation::CreateConnectEvent
	inline bool CreateOvReadEvent( void )
	{
		if (readEvent)		return true;
		if (!(readEvent = CreateEvent( 0, TRUE, FALSE, 0 )))	return false;
		return true;
	}
	// end of VS_Connection_Implementation::CreateOvReadEvent

	inline void CloseOvWriteEvent( void )
	{	if (writeEvent) {	CloseHandle( writeEvent );	writeEvent = 0;		}}
	// end of VS_Connection_Implementation::CloseOvWriteEvent

	inline void CloseOvReadEvent( void )
	{	if (readEvent) {	CloseHandle( readEvent );	readEvent = 0;		}}
	// end of VS_Connection_Implementation::CloseOvReadEvent

	inline void SetOvWriteFields( const VS_ACS_Field field1, const VS_ACS_Field field2,
									const VS_ACS_Field field3 )
	{
		if (field1 != VS_ACS_INVALID_FIELD)		writeOv.field1 = field1;
		if (field2 != VS_ACS_INVALID_FIELD)		writeOv.field2 = field2;
		if (field3 != VS_ACS_INVALID_FIELD)		writeOv.field3 = field3;
	}
	// end of VS_Connection_Implementation::SetOvWriteFields

	inline void SetOvReadFields( const VS_ACS_Field field1, const VS_ACS_Field field2,
									const VS_ACS_Field field3 )
	{
		if (field1 != VS_ACS_INVALID_FIELD)		readOv.field1 = field1;
		if (field2 != VS_ACS_INVALID_FIELD)		readOv.field2 = field2;
		if (field3 != VS_ACS_INVALID_FIELD)		readOv.field3 = field3;
	}
	// end of VS_Connection_Implementation::SetOvReadFields

	inline void SetOvFields( const VS_ACS_Field field1, const VS_ACS_Field field2,
								const VS_ACS_Field field3 )
	{
		SetOvWriteFields( field1, field2, field3 );
		SetOvReadFields( field1, field2, field3 );
	}
	// end of VS_Connection_Implementation::SetOvFields
	inline void SetIOHandler(VS_IOHandler *h)
	{
		SetReadHandler(h);
		SetWriteHandler(h);
	}
	inline void SetReadHandler(VS_IOHandler *h)
	{
		readOv.io_handler = h;
	}
	inline void SetWriteHandler(VS_IOHandler *h)
	{
		writeOv.io_handler = h;
	}
	inline const VS_Overlapped *WriteOv() const
	{
		return &writeOv;
	}
	inline const VS_Overlapped *ReadOv() const
	{
		return &readOv;
	}

	inline void SetOvFildIOCP( const void *handleIOCP )
	{	writeOv.hiocp
	= readOv.hiocp = (HANDLE)handleIOCP;	}
	// end of VS_Connection_Implementation::SetOvFildIOCP

	inline bool SetIOCP( void *handleIOCP )
	{
		if (isWrite || isRead
				|| CreateIoCompletionPort( hio, (HANDLE)handleIOCP, 0, 0 ) != handleIOCP)
			return false;
		hiocp = (HANDLE)handleIOCP;
		return true;
	}
	// end of VS_Connection_Implementation::SetIOCP

	inline int GetResult( VS_Overlapped &ov, unsigned long &mills )
	{
		unsigned long prevMs = GetTickCount();
		switch (WaitForSingleObject( ov.over.hEvent, mills ))
		{
		case WAIT_OBJECT_0 :	break;
		case WAIT_TIMEOUT :		mills = 0;	return -2;
		default :				return -1;
		}
		prevMs = GetTickCount() - prevMs;
		if (mills > prevMs)
			mills -= prevMs;
		else
			mills = 0;
		unsigned long bytes_trans(0);
		BOOL ret = GetOverlappedResult(hio, &ov.over, &bytes_trans, FALSE);
		if(!ret && ERROR_IO_INCOMPLETE == GetLastError())
			return -2;
		ov.b_last = bytes_trans;
		if (!ResetEvent( ov.over.hEvent ) || !ret)	return -1;
		return (int)ov.b_last;
	}
	// end of VS_Connection_Implementation::GetResult

	inline int GetWriteResult( VS_Overlapped &ov, unsigned long &mills )
	{
		int ret = GetResult(ov, mills);
		if (((ret == -1 && GetLastError() == ERROR_OPERATION_ABORTED)
			|| (!ret && ov.over.OffsetHigh == ERROR_OPERATION_ABORTED))
			&& PPWrite())	ret = GetResult( ov, mills );
		return ret;
	}
	// end of VS_Connection_Implementation::GetWriteResult

	inline int GetReadResult( VS_Overlapped &ov, unsigned long &mills )
	{
		int ret = GetResult(ov, mills);
		if (((ret == -1 && GetLastError() == ERROR_OPERATION_ABORTED)
			|| (!ret && ov.over.OffsetHigh == ERROR_OPERATION_ABORTED))
			&& PPRead())	ret = GetResult( ov, mills );
		return ret;
	}
	// end of VS_Connection_Implementation::GetReadResult

	inline bool PPWrite( void )
	{
		writeOv.b_ov = 0;
		writeOv.over = over_zero;
		writeOv.over.hEvent = writeEvent;
		if (!WriteFile( hio, (const void *)&((char *)wB)[writeOv.b_trans],
			writeOv.b_want - writeOv.b_trans, &writeOv.b_ov, &writeOv.over ))
		{
			DWORD error = GetLastError();
			if (error != ERROR_IO_PENDING) {	writeOv.error = error;	return false;	}
			isWriteOv = true;
		}
		return isWrite = true;
	}
	// end of VS_Connection_Implementation::PPWrite

	inline bool PPWriteDgramTo( const unsigned long to_ip, const unsigned short to_port )
	{
		writeOv.b_ov = 0;
		writeOv.over = over_zero;
		writeOv.over.hEvent = writeEvent;
		WSABUF   buf = { writeOv.b_want - writeOv.b_trans, &((char *)wB)[writeOv.b_trans] };
		struct sockaddr_in   addr;		memset( (void *)&addr, 0, sizeof(addr) );
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = vs_htonl(to_ip);
		addr.sin_port = vs_htons(to_port);
		if (WSASendTo( (SOCKET)hio, &buf, 1, &writeOv.b_ov, 0, (sockaddr *)&addr, sizeof(addr), &writeOv.over, 0 ))
		{
			DWORD error = GetLastError();
			if (error != ERROR_IO_PENDING) {	writeOv.error = error;	return false;	}
			isWriteOv = true;
		}
		return isWrite = true;
	}
	// end of VS_Connection_Implementation::PPWriteDgramTo

	inline bool PPWriteDgramToV6( const in6_addr to_ip, const unsigned short to_port )
	{
		writeOv.b_ov = 0;
		writeOv.over = over_zero;
		writeOv.over.hEvent = writeEvent;
		WSABUF   buf = { writeOv.b_want - writeOv.b_trans, &((char *)wB)[writeOv.b_trans] };
		struct sockaddr_in6 addr;
		memset( (void *)&addr, 0, sizeof(addr) );
		addr.sin6_family = AF_INET6;
		addr.sin6_addr = to_ip;
		addr.sin6_port = vs_htons(to_port);
		if (WSASendTo( (SOCKET)hio, &buf, 1, &writeOv.b_ov, 0, (sockaddr *)&addr, sizeof(addr), &writeOv.over, 0 ))
		{
			DWORD error = GetLastError();
			if (error != ERROR_IO_PENDING) {	writeOv.error = error;	return false;	}
			isWriteOv = true;
		}
		return isWrite = true;
	}
	// end of VS_Connection_Implementation::PPWriteDgramTo

	inline bool PWrite( const void *buffer, const unsigned long n_bytes )
	{
		writeOv.b_last = writeOv.b_trans = 0;
		writeOv.b_want = n_bytes;
		wB = (void *)buffer;
		return PPWrite();
	}
	// end of VS_Connection_Implementation::PWrite

	inline bool PWriteDgramTo( const void *buffer, const unsigned long n_bytes, const unsigned long to_ip, const unsigned short to_port )
	{
		writeOv.b_last = writeOv.b_trans = 0;
		writeOv.b_want = n_bytes;
		wB = (void *)buffer;
		return PPWriteDgramTo( to_ip, to_port );
	}
	// end of VS_Connection_Implementation::PWriteDgramTo

	inline bool PWriteDgramToV6( const void *buffer, const unsigned long n_bytes, const in6_addr to_ip, const unsigned short to_port )
	{
		writeOv.b_last = writeOv.b_trans = 0;
		writeOv.b_want = n_bytes;
		wB = (void *)buffer;
		return PPWriteDgramToV6( to_ip, to_port );
	}
	// end of VS_Connection_Implementation::PWriteDgramTo

	inline bool ResetWrite( void )
	{
		if (wBM) {	BFree( wBM );	wBM = 0;	}
		return isWrite = isWriteOv = false;
	}
	// end of VS_Connection_Implementation::ResetWrite

	inline bool PPRead( void )
	{
go_again:
		readOv.b_ov = 0;
		readOv.over = over_zero;
		readOv.over.hEvent = readEvent;
		if (!ReadFile( hio, (void *)&((char *)rB)[readOv.b_trans],
			readOv.b_want - readOv.b_trans, &readOv.b_ov, &readOv.over ))
		{
			DWORD error = GetLastError();
			switch (error)
			{
			case ERROR_IO_PENDING :		break;
			case ERROR_PORT_UNREACHABLE :
				if (type == vs_connection_type_dgram)	goto go_again;
			default :		readOv.error = error;	return false;
			}
			isReadOv = true;
		}
		return isRead = true;
	}
	// end of VS_Connection_Implementation::PPRead

	inline bool PRead( void *buffer, const unsigned long n_bytes )
	{
		readOv.b_last = readOv.b_trans = 0;
		readOv.b_want = n_bytes;
		rB = buffer;
		return PPRead();
	}
	// end of VS_Connection_Implementation::PRead

	inline bool ResetRead( void )
	{
		if (rBM) {	BFree( rBM );	rBM = 0;	}
		return isRead = isReadOv = false;
	}
	// end of VS_Connection_Implementation::ResetRead

	inline bool RWriteStream( const VS_Buffer *buffers, const unsigned long n_buffers )
	{
		if (!isWrite && BAlloc( buffers, n_buffers, wBM, writeOv.b_want )
				&& PWrite( wBM, writeOv.b_want ))	return true;
		return ResetWrite();
	}
	// end of VS_Connection_Implementation::RWriteStream

	inline bool WriteStream( const void *buffer, const unsigned long n_bytes )
	{
		if (!isWrite && PWrite( buffer, n_bytes ))	return true;
		return ResetWrite();
	}
	// end of VS_Connection_Implementation::WriteStream

	inline int WriteStreamResult( void )
	{
		if ((writeOv.b_trans += writeOv.b_last) > writeOv.b_want)
		{	ResetWrite();	return -1;	}
		if (writeOv.b_trans == writeOv.b_want)
		{	ResetWrite();	return (int)writeOv.b_trans;	}
		if (!PPWrite()) {	ResetWrite();	return -1;	}
		return -2;
	}
	// end of VS_Connection_Implementation::WriteStreamResult

	inline int GetWriteStreamResult( unsigned long &mills )
	{
		if (!isWrite)	return -1;
		do
		{
			switch (GetWriteResult( writeOv, mills ))
			{
			case -2 :	return -2;
			case -1 :	ResetWrite();	return -1;
			default :	switch (WriteStreamResult())
						{
						case -2 :	break;
						case -1 :	return -1;
						default :	return (int)writeOv.b_trans;
		}	}			}
		while (mills);
		return -2;
	}
	// end of VS_Connection_Implementation::GetWriteStreamResult

	inline int SetWriteStreamResult( const unsigned long b_trans, const struct VS_Overlapped *ov )
	{
		if (!isWrite || ov != &writeOv)		return -1;
		if (ov->error) {	ResetWrite();	return -1;	}
		writeOv.b_last = b_trans;
		return WriteStreamResult();
	}
	// end of VS_Connection_Implementation::SetWriteStreamResult

	inline bool RWriteDgram( const VS_Buffer *buffers, const unsigned long n_buffers )
	{
		if (!isWrite && BAlloc( buffers, n_buffers, wBM, writeOv.b_want )
				&& PWrite( wBM, writeOv.b_want ))	return true;
		return ResetWrite();
	}
	// end of VS_Connection_Implementation::RWriteDgram

	inline bool RWriteDgramTo( const VS_Buffer *buffers, const unsigned long n_buffers,
							const unsigned long to_ip, const unsigned short to_port )
	{
		if (!isWrite && BAlloc( buffers, n_buffers, wBM, writeOv.b_want )
				&& PWriteDgramTo( wBM, writeOv.b_want, to_ip, to_port ))	return true;
		return ResetWrite();
	}

	inline bool RWriteDgramToV6( const VS_Buffer *buffers, const unsigned long n_buffers,
							const in6_addr to_ip, const unsigned short to_port )
	{
		if (!isWrite && BAlloc( buffers, n_buffers, wBM, writeOv.b_want )
				&& PWriteDgramToV6( wBM, writeOv.b_want, to_ip, to_port ))	return true;
		return ResetWrite();
	}

	inline bool WriteDgram( const void *buffer, const unsigned long n_bytes )
	{
		if (!isWrite && PWrite( buffer, n_bytes ))	return true;
		return ResetWrite();
	}
	// end of VS_Connection_Implementation::WriteDgram

	inline bool WriteDgramTo( const void *buffer, const unsigned long n_bytes, const unsigned long to_ip, const unsigned short to_port )
	{
		if (!isWrite && PWriteDgramTo( buffer, n_bytes, to_ip, to_port ))	return true;
		return ResetWrite();
	}
	// end of VS_Connection_Implementation::WriteDgramTo

	inline bool WriteDgramToV6( const void *buffer, const unsigned long n_bytes, const in6_addr to_ip, const unsigned short to_port )
	{
		if (!isWrite && PWriteDgramToV6( buffer, n_bytes, to_ip, to_port ))	return true;
		return ResetWrite();
	}
	// end of VS_Connection_Implementation::WriteDgramTo

	inline int WriteDgramResult( void )
	{
		if ((writeOv.b_trans += writeOv.b_last) != writeOv.b_want)
		{	ResetWrite();	return -1;	}
		ResetWrite();	return (int)writeOv.b_trans;
	}
	// end of VS_Connection_Implementation::WriteDgramResult

	inline int GetWriteDgramResult( unsigned long &mills )
	{
		if (!isWrite)	return -1;
		int ret = GetWriteResult(writeOv, mills);
		switch (ret)
		{
		case -2 :	return ret;
		case -1 :	ResetWrite();	return -1;
		default :	return WriteDgramResult();
		}
	}
	// end of VS_Connection_Implementation::GetWriteDgramResult

	inline int SetWriteDgramResult( const unsigned long b_trans, const struct VS_Overlapped *ov )
	{
		if (!isWrite || ov != &writeOv)		return -1;
		if (ov->error) {	ResetWrite();	return -1;	}
		writeOv.b_last = b_trans;
		return WriteDgramResult();
	}
	// end of VS_Connection_Implementation::SetWriteDgramResult

	inline bool IsWrite( void ) const {	return isWrite;	}
	// end of VS_Connection_Implementation::IsWrite

	inline bool RReadStream( const unsigned long n_bytes )
	{
		if (!isRead && Alloc( rBM, n_bytes ) && PRead( rBM, n_bytes ))	return true;
		return ResetRead();
	}
	// end of VS_Connection_Implementation::RReadStream

	inline bool ReadStream( void *buffer, const unsigned long n_bytes )
	{
		if (!isRead && PRead( buffer, n_bytes ))	return true;
		return ResetRead();
	}
	// end of VS_Connection_Implementation::ReadStream

	inline int ReadStreamResult( void **buffer, const bool portion )
	{
		if (!readOv.b_last || (readOv.b_trans += readOv.b_last) > readOv.b_want)
		{	ResetRead();
			return -1;	}
		if (readOv.b_trans == readOv.b_want || portion)
		{	if (rBM)
			{	if (!buffer) {	ResetRead();
				return -1;	}
				*buffer = rBM;	rBM = 0;		}
			else if (buffer)	*buffer = 0;
			ResetRead();	return (int)readOv.b_trans;		}
		if (!PPRead()) {	ResetRead();
			return -1;	}
		return -2;
	}
	// end of VS_Connection_Implementation::ReadStreamResult

	inline int GetReadStreamResult( unsigned long &mills, void **buffer = 0,
										const bool portion = false )
	{
		if (!isRead)
			return -1;
		do
		{
			int ret = GetReadResult(readOv, mills);
			switch (ret)
			{
			case -2 :	return -2;
			case -1 :
				return -1;
			default :	switch (ReadStreamResult( buffer, portion ))
						{
						case -2 :	break;
						case -1 :
							return -1;
						default :	return (int)readOv.b_trans;
		}	}			}
		while (mills);
		return -2;
	}
	// end of VS_Connection_Implementation::GetReadStreamResult

	inline int SetReadStreamResult( const unsigned long b_trans,
										const struct VS_Overlapped *ov,
										void **buffer, const bool portion )
	{
		if (!isRead || ov != &readOv)
			return -1;
		if (ov->error) {	ResetRead();	return -1;	}
		readOv.b_last = b_trans;
		return ReadStreamResult( buffer, portion );
	}
	// end of VS_Connection_Implementation::SetReadStreamResult

	inline bool RReadDgram( const unsigned long n_bytes )
	{
		if (!isRead)
		{
			if (Alloc( rBM, 65536 ) && PRead( rBM, 65536 ))
			{	rB = 0;	return fRdOv = true;	}
			return ResetRead();
		}
		if (!fRdOv) {	rB = 0;	readOv.b_want = n_bytes;	return fRdOv = true;	}
		return ResetRead();
	}
	// end of VS_Connection_Implementation::RReadDgram

	inline bool ReadDgram( void *buffer, const unsigned long n_bytes )
	{
		if (!isRead)
		{
			if (Alloc( rBM, 65536 ) && PRead( rBM, 65536 ))
			{	rB = buffer;	readOv.b_want = n_bytes;	return fRdOv = true;	}
			return ResetRead();
		}
		if (!fRdOv) {	rB = buffer;	readOv.b_want = n_bytes;	return fRdOv = true;	}
		return ResetRead();
	}
	// end of VS_Connection_Implementation::ReadDgram

	inline int ReadDgramResult( void **buffer, const bool portion )
	{
		readOv.b_last = readOv.b_trans - readOv.b_ov;
		if (readOv.b_want < readOv.b_last)	readOv.b_last = readOv.b_want;
		else if (readOv.b_want > readOv.b_last && !portion)
		{	ResetRead();	return -1;	}
		if (rB)
		{	memcpy( rB, (const void *)&((char *)rBM)[readOv.b_ov], readOv.b_last );
			if (buffer)		*buffer = 0;		}
		else
		{	if (!buffer || !Alloc( *buffer, readOv.b_last ))
			{	ResetRead();	return -1;	}
			memcpy( *buffer, (const void *)&((char *)rBM)[readOv.b_ov], readOv.b_last );	}
		if ((readOv.b_ov += readOv.b_last) < readOv.b_trans)	fRdOv = false;
		else	ResetRead();
		return (int)readOv.b_last;
	}
	// end of VS_Connection_Implementation::ReadDgramResult

	inline int GetReadDgramResult( unsigned long &mills, void **buffer = 0,
									const bool portion = false )
	{
		if (!isRead || !fRdOv)	return -1;
		if (!readOv.b_last)
		{
			int ret = GetReadResult(readOv, mills);
			switch (ret)
			{
			case -2 :	return ret;
			case -1 :	ResetRead();	return -1;
			}
			readOv.b_trans = readOv.b_last;
			readOv.b_ov = 0;
		}
		return ReadDgramResult( buffer, portion );
	}
	// end of VS_Connection_Implementation::GetReadDgramResult

	inline int SetReadDgramResult( const unsigned long b_trans,
										const struct VS_Overlapped *ov,
										void **buffer, const bool portion )
	{
		if (!isRead || !fRdOv || ov != &readOv)		return -1;
		if (ov->error) {	ResetRead();	return -1;	}
		if (!readOv.b_last)
		{
			readOv.b_trans = b_trans;
			readOv.b_ov = 0;
		}
		return ReadDgramResult( buffer, portion );
	}
	// end of VS_Connection_Implementation::SetReadDgramResult

	inline void Free( void *buffer ) {	BFree( buffer );	}
	// end of VS_Connection_Implementation::Free
};
// end of VS_Connection_Implementation

//////////////////////// CONNECTION SOCK IMPLEMENTATION ///////////////////////

struct VS_ConnectionSock_Implementation : VS_Connection_Implementation, public VS_QoSEnabledSocket
{
	VS_ConnectionSock_Implementation( void )
		: state(vs_sock_state_not_created), connectType(vs_sock_type_not_connected)
		, isAccept(false), bindIp(0)
		, bindPort(0), peerIp(0), peerPort(0)
		, connectHost(0), connectPort(0), acceptHost(0), acceptPort(0), connectTime(0), qosMode(0)
		, fastMode(6),eventTime(0),m_addr_len(0),m_flag_tmp(0)
		, multicastTTL(GetMulticastTTL()), TOSValue(GetTOSValue()), currentAddressFamily(VS_IPPortAddress::ADDR_IPV4),isFastEnabled(false)
	{
		m_wsaRecvBuf.buf = 0;
		m_wsaRecvBuf.len = 0;
		m_wsaSendBuf.buf = 0;
		m_wsaSendBuf.len = 0;
	}
	// end of VS_ConnectionSock_Implementation::VS_ConnectionSock_Implementation

	~VS_ConnectionSock_Implementation( void )
	{	Close();	ResetHostIpPort();	}
	// end of VS_ConnectionSock_Implementation::~VS_ConnectionSock_Implementation

	VS_Sock_State   state;
	VS_ConnectDirection   connectType;
	bool   isAccept;
	char   *bindIp, *bindPort, *peerIp, *peerPort,
			 *acceptHost, *acceptPort, *connectHost, *connectPort;
	time_t   connectTime;
	int			m_addr_len;
	unsigned long	m_flag_tmp;
	WSABUF		m_wsaRecvBuf;
	WSABUF		m_wsaSendBuf;

	int fastMode;
	bool isFastEnabled;
	int qosMode;

	unsigned long eventTime;

	signed long multicastTTL;
	signed long TOSValue;

	// Bind and peer addresses in VS_IPPortAddress format.
	VS_IPPortAddress bindAddr, peerAddr;

	SOCKET GetSocketHandle(void)
	{
		if (hio != INVALID_HANDLE_VALUE)
			return (SOCKET)hio;
		return 0;
	}

	bool IsConnected(void)
	{
		return state == vs_sock_state_connected;
	}

public:

	// Поле, которое показывает, какой протокол используется для создания сокета:
	// AF_INET (для IPv4) или AF_INET6 (для IPV6). Для обратной совместимости, по
	// умолчанию стоит AF_INET.
	int currentAddressFamily;

	// Метод для переключения объекта в режим IPv4.
	// Должен быть вызван до создания сокета.
	void SwitchToIPv4() { currentAddressFamily = VS_IPPortAddress::ADDR_IPV4; }

	// Метод для переключения объекта в режим IPv6.
	// Должен быть вызван до создания сокета.
	void SwitchToIPv6() { currentAddressFamily = VS_IPPortAddress::ADDR_IPV6; }

	// Returns address type of current socket.
	// Returns value even if socket is not created or connection is not established.
	// This address type sets during the creation of object.
	unsigned GetSocketAddressFamily() const { return currentAddressFamily; }

	// Returns address type of current connection.
	// If object is not connected, returns VS_IPPortAddress::ADDR_UNDEF.
	unsigned GetConnectionAddressFamily() const
	{
		if(bindAddr.getAddressType() != VS_IPPortAddress::ADDR_UNDEF)
			return bindAddr.getAddressType();
		return peerAddr.getAddressType();
	}

	inline void ResetBindIpPort( void )
	{
		if (bindIp) {		free( (void *)bindIp );			bindIp = 0;		}
		if (bindPort) {		free( (void *)bindPort );		bindPort = 0;	}

		bindAddr = VS_IPPortAddress();
	}
	// end of VS_ConnectionSock_Implementation::ResetIpPort

	inline void ResetPeerIpPort( void )
	{
		if (peerIp) {		free( (void *)peerIp );			peerIp = 0;		}
		if (peerPort) {		free( (void *)peerPort );		peerPort = 0;	}

		peerAddr = VS_IPPortAddress();
	}
	// end of VS_ConnectionSock_Implementation::ResetPeerIpPort

	inline void ResetAcceptHostPort( void )
	{
		if (acceptHost) {	free( (void *)acceptHost );		acceptHost = 0;		}
		if (acceptPort) {	free( (void *)acceptPort );		acceptPort = 0;		}
	}
	// end of VS_ConnectionSock_Implementation::ResetAcceptHostPort

	inline void ResetConnectHostPort( void )
	{
		if (connectHost) {		free( (void *)connectHost );	connectHost = 0;	}
		if (connectPort) {		free( (void *)connectPort );	connectPort = 0;	}
	}
	// end of VS_ConnectionSock_Implementation::ResetConnectHostPort

	inline void ResetHostIpPort( void )
	{
		ResetBindIpPort();
		ResetPeerIpPort();
		ResetConnectHostPort();
		ResetAcceptHostPort();
	}
	// end of VS_ConnectionSock_Implementation::ResetHostIpPort

	inline bool SetBindIpPort( struct sockaddr_storage *addr_in = 0 )
	{
		ResetBindIpPort();
		char   sIp[50] = { 0 }, sPort[12] = { 0 };
		struct sockaddr_storage   loc_addr_in = { AF_INET, 0 };
		if (!addr_in)
		{	int   len = sizeof(loc_addr_in);
			if (getsockname( (SOCKET)hio, (sockaddr *)&loc_addr_in, &len ) == SOCKET_ERROR)	return false;

			addr_in = &loc_addr_in;
		}
		if(addr_in->ss_family == AF_INET6)
		{
			// Для адресов ipv6.
			sockaddr_in6* addr6 = (sockaddr_in6*) addr_in;
			// Извлечение ip и порта из струткуры для IPV6.
			net::inet_ntop(AF_INET6, &addr6->sin6_addr, sIp, sizeof(sIp));
			::free(bindIp);
			bindIp = _strdup(sIp);
			if(addr6->sin6_port)
			{
				_itoa((int)vs_ntohs(addr6->sin6_port), sPort, 10);
				::free(bindPort);
				bindPort = _strdup(sPort);

				bindAddr.ipv6(addr6->sin6_addr);
				bindAddr.port_netorder(addr6->sin6_port);
			}
		}
		else if(addr_in->ss_family == AF_INET)
		{
			// Для адресов ipv4.
			sockaddr_in* addr4 = (sockaddr_in*) addr_in;
			// Извелечение ip и порта из структуры для IPV4.
			///if (addr_in->sin_addr.S_un.S_addr)
			{
				strncpy( sIp, inet_ntoa( addr4->sin_addr ), sizeof(sIp) - 1 );
				::free(bindIp);
				bindIp = _strdup( sIp );
			}
			if (addr4->sin_port)
			{
				_itoa((int)vs_ntohs(addr4->sin_port), sPort, 10);
				::free(bindPort);
				bindPort = _strdup( sPort );

				bindAddr.ipv4_netorder(addr4->sin_addr.S_un.S_addr);
				bindAddr.port_netorder(addr4->sin_port);
			}
		}
		return bindIp && bindPort;
	}
	// end of VS_ConnectionSock_Implementation::SetBindIpPort

	inline bool SetPeerIpPort( struct sockaddr_storage *addr_in = 0 )
	{
		ResetPeerIpPort();
		char   sIp[50] = { 0 }, sPort[12] = { 0 };
		struct sockaddr_storage   loc_addr_in = { AF_INET, 0 };
		if (!addr_in)
		{	int   len = sizeof(loc_addr_in);
			if (getpeername( (SOCKET)hio, (sockaddr *)&loc_addr_in, &len ) == SOCKET_ERROR)	return false;
			// Проверка на 0.0.0.0
			if (loc_addr_in.ss_family == AF_INET && !((sockaddr_in*)&loc_addr_in)->sin_addr.S_un.S_addr) return false;
			// Проверка на ::
			if (loc_addr_in.ss_family == AF_INET6)
			{
				if(VS_TestIPv6_IsZero(&((sockaddr_in6*) &loc_addr_in)->sin6_addr)) return false;
			}
			addr_in = &loc_addr_in;
		}
		if(addr_in->ss_family == AF_INET6)
		{
			sockaddr_in6* addr6 = (sockaddr_in6*) addr_in;
			// Извлечение ip и порта из струткуры для IPV6.
			net::inet_ntop(AF_INET6, &addr6->sin6_addr, sIp, sizeof(sIp));
			::free(peerIp);
			peerIp = _strdup(sIp);
			if(addr6->sin6_port)
			{
				_itoa((int)vs_ntohs(addr6->sin6_port), sPort, 10);
				::free(peerPort);
				peerPort = _strdup(sPort);

				peerAddr.ipv6(addr6->sin6_addr);
				peerAddr.port_netorder(addr6->sin6_port);
			}
		}
		else
		{
			sockaddr_in* addr4 = (sockaddr_in*) addr_in;
			if (addr4->sin_addr.S_un.S_addr)
			{
				strncpy( sIp, inet_ntoa( addr4->sin_addr ), sizeof(sIp) - 1 );
				::free(peerIp);
				peerIp = _strdup( sIp );

				peerAddr.ipv4_netorder(addr4->sin_addr.S_un.S_addr);
			}
			if (addr4->sin_port)
			{
				_itoa((int)vs_ntohs(addr4->sin_port), sPort, 10);
				::free(peerPort);
				peerPort = _strdup( sPort );

				peerAddr.port_netorder(addr4->sin_port);
			}
		}
		return peerIp && peerPort;
	}
	// end of VS_ConnectionSock_Implementation::SetPeerIpPort

	inline bool SetConnectHostPortAct( const char *host, const unsigned short port )
	{
		::free(connectHost);
		connectHost = _strdup( host );
		if (!connectHost)	return false;
		char   sPort[12];	ZeroMemory( (void *)sPort, sizeof(sPort) );
		_itoa( port, sPort, 10 );
		::free(connectPort);
		connectPort = _strdup( sPort );
		return connectPort != nullptr;
	}
	// end of VS_ConnectionSock_Implementation::SetConnectHostPort

	inline bool SetConnectHostPort( const char *host, const unsigned short port )
	{
		ResetConnectHostPort();
		if (!host || !*host || !port)	return false;
		return SetConnectHostPortAct( host, port );
	}
	// end of VS_ConnectionSock_Implementation::SetConnectHostPort

	inline bool SetConnectHostPort( const unsigned long ip, const unsigned short port )
	{
		ResetConnectHostPort();
		char   host[16];	ZeroMemory( (void *)host, sizeof(host) );
		if (!port || !VS_GetHostByIp( ip, host, sizeof(host) ))		return false;
		return SetConnectHostPortAct( host, port );
	}
	// end of VS_ConnectionSock_Implementation::SetConnectHostPort

	inline bool SetAcceptHostPortAct( const char host[], const unsigned short port )
	{
		::free(acceptHost);
		acceptHost = _strdup( host );
		if (!acceptHost)	return false;
		char   sPort[12];	ZeroMemory( (void *)sPort, sizeof(sPort) );
		_itoa( port, sPort, 10 );
		::free(acceptPort);
		acceptPort = _strdup( sPort );
		return acceptPort != nullptr;
	}
	// end of VS_ConnectionSock_Implementation::SetAcceptHostPortAct

	inline bool SetAcceptHostPort( const char *host, const unsigned short port )
	{
		ResetAcceptHostPort();
		if (!host || !*host || !port)	return false;
		return SetAcceptHostPortAct( host, port );
	}
	// end of VS_ConnectionSock_Implementation::SetAcceptHostPort

	inline bool SetAcceptHostPort( const unsigned long ip, const unsigned short port )
	{
		ResetAcceptHostPort();
		char   host[16];	ZeroMemory( (void *)host, sizeof(host) );
		if (!port || !VS_GetHostByIp( ip, host, sizeof(host) ))		return false;
		return SetAcceptHostPortAct( host, port );
	}
	// end of VS_ConnectionSock_Implementation::SetAcceptHostPort

	inline const char* GetBindIp()
	{
		if (!bindIp)	SetBindIpPort();
		return bindIp;
	}
	// end of VS_ConnectionSock_Implementation::GetBindIp

	inline const char* GetBindPort()
	{
		if (!bindPort)	SetBindIpPort();
		return bindPort;
	}
	// end of VS_ConnectionSock_Implementation::GetBindPort

	inline const char* GetPeerIp()
	{
		if (!peerIp)	SetPeerIpPort();
		return peerIp;
	}
	// end of VS_ConnectionSock_Implementation::GetPeerIp

	inline const char* GetPeerPort()
	{
		if (!peerPort)	SetPeerIpPort();
		return peerPort;
	}
	// end of VS_ConnectionSock_Implementation::GetPeerPort


	const VS_IPPortAddress& GetBindAddress()
	{
		if (bindAddr.getAddressType() == VS_IPPortAddress::ADDR_UNDEF)
			SetBindIpPort();
		return bindAddr;
	}

	const VS_IPPortAddress& GetPeerAddress()
	{
		if (peerAddr.getAddressType() == VS_IPPortAddress::ADDR_UNDEF)
			SetPeerIpPort();
		return peerAddr;
	}

	inline char *GetAcceptHost( void ) const {		return acceptHost;		}
	// end of VS_ConnectionSock_Implementation::GetAcceptHost

	inline char *GetAcceptPort( void ) const {		return acceptPort;		}
	// end of VS_ConnectionSock_Implementation::GetAcceptPort

	inline char *GetConnectHost( void ) const {		return connectHost;		}
	// end of VS_ConnectionSock_Implementation::GetConnectHost

	inline char *GetConnectPort( void ) const {		return connectPort;		}
	// end of VS_ConnectionSock_Implementation::GetConnectPort

	inline bool SetSizeBuffers( const int writeSize, const int readSize )
	{
		if (writeSize < -1 || writeSize > 1048576 || readSize < -1 || readSize > 1048576)
			return false;
		bool ret = true;

		if (VS_Connection_Implementation::writeSize != writeSize)
		{
			if (writeSize != -1)
			{
				VS_Connection_Implementation::writeSize = writeSize;
				if (setsockopt( (SOCKET)hio, SOL_SOCKET, SO_SNDBUF, (char FAR *)&(VS_Connection_Implementation::writeSize), sizeof(VS_Connection_Implementation::writeSize) ) == SOCKET_ERROR)
					ret = false;
		}	}
		if (VS_Connection_Implementation::readSize != readSize)
		{
			if (readSize != -1)
			{
				VS_Connection_Implementation::readSize = readSize;
				if (setsockopt( (SOCKET)hio, SOL_SOCKET, SO_RCVBUF, (char FAR *)&(VS_Connection_Implementation::readSize), sizeof(VS_Connection_Implementation::readSize) ) == SOCKET_ERROR)
					ret = false;
		}	}
		return ret || (state <= vs_sock_state_listen);
	}
	// end of VS_ConnectionSock_Implementation::SetSizeBuffers

	bool SetAsWriter( void ) {		return SetSizeBuffers( -1, 0 );		}
	// end of VS_ConnectionSock_Implementation::SetAsWriter

	bool SetAsReader( void ) {		return SetSizeBuffers( 0, -1 );		}
	// end of VS_ConnectionSock_Implementation::SetAsReader

	inline bool Socket( VS_Connection_Type type, const bool QoS = false ) {
		if (__QoS_isOK < 0) { __QoS_isOK = GetQoSEnable(); }
		if (QoS) {
			return SocketQoS( type );
		} else {
			return SocketAny( type );
		}
	}

	inline bool SocketAny( VS_Connection_Type type )
	{
		if (state > vs_sock_state_not_created)	return VS_ConnectionSock_Implementation::type == type;
		int   tp = -1, pr = -1;
		switch (type)
		{
		case vs_connection_type_stream :	tp = SOCK_STREAM;	pr = IPPROTO_TCP;	break;
		case vs_connection_type_dgram :		tp = SOCK_DGRAM;	pr = IPPROTO_UDP;	break;
		}
		SOCKET	sock = INVALID_SOCKET;
		if ( SOCK_DGRAM == tp ) {
			sock = socket( currentAddressFamily, tp, pr );
		} else {
			if (SocketQoS(type)) { sock = (SOCKET)hio; }

		}
		if (sock == INVALID_SOCKET)		return false;
		hio = (HANDLE)sock;
		VS_ConnectionSock_Implementation::type = type;
		state = vs_sock_state_created;
		return true;
	}
	// end of VS_ConnectionSock_Implementation::SocketAny
	inline bool SocketQoS( VS_Connection_Type type ) {
		if (state > vs_sock_state_not_created)	return VS_ConnectionSock_Implementation::type == type;
		int   tp = -1, pr = -1;
		switch (type) {
		case vs_connection_type_stream :	tp = SOCK_STREAM;	pr = IPPROTO_TCP;	break;
		case vs_connection_type_dgram :		tp = SOCK_DGRAM;	pr = IPPROTO_UDP;	break;
		}

		WSAPROTOCOL_INFO *ProviderProtocol(0), *lpProtocolsBuf(0);
		if (__QoS_isOK) {
			DWORD bufferSize(0); int ProtocolCount(0);
			ProtocolCount = WSAEnumProtocols(0, 0, &bufferSize);
			if ((SOCKET_ERROR == ProtocolCount) && (WSAENOBUFS == WSAGetLastError())) {
				lpProtocolsBuf	= reinterpret_cast<WSAPROTOCOL_INFO*>(new char[bufferSize]);
				ProtocolCount	= WSAEnumProtocols(0, lpProtocolsBuf, &bufferSize);
				if (SOCKET_ERROR != ProtocolCount) {
					for (int i = 0; i < ProtocolCount; ++i) {
						if (((lpProtocolsBuf[i].dwServiceFlags1 & XP1_QOS_SUPPORTED) == XP1_QOS_SUPPORTED) &&
							(lpProtocolsBuf[i].iSocketType == tp) &&
							// Отсеиваем по IPv4 или IPv6, в зависимости от того, что было выбрано.
							(lpProtocolsBuf[i].iAddressFamily == currentAddressFamily)) {
								ProviderProtocol = &lpProtocolsBuf[i];
								break;
						}
					}
				}
			}
		}

		SOCKET	sock = INVALID_SOCKET;
		if(SOCK_DGRAM==tp) {
			sock = WSASocket(currentAddressFamily, tp, pr, ProviderProtocol, 0, 0);
			if (sock == INVALID_SOCKET) {
				sock = WSASocket(currentAddressFamily, tp, pr, 0, 0, 0);
			}
		} else {
			sock = WSASocket(currentAddressFamily, tp, pr, ProviderProtocol, 0, WSA_FLAG_OVERLAPPED);
			if (sock == INVALID_SOCKET) {
				sock = WSASocket(currentAddressFamily, tp, pr, 0, 0, WSA_FLAG_OVERLAPPED);
			}
		}
		delete [] lpProtocolsBuf;
		if (sock == INVALID_SOCKET)	return false;
		hio = (HANDLE)sock;
		VS_ConnectionSock_Implementation::type = type;
		state = vs_sock_state_created;
		return true;
	}
	// end of VS_ConnectionSock_Implementation::SocketQoS

	signed long GetMulticastTTL() {
		signed long value(5);
		VS_RegistryKey key_client(true, "Current configuration");
		key_client.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "MulticastTTL");
		return value;
	}
	// end of VS_ConnectionSock_Implementation::GetMulticastTTL

	signed long GetTOSValue() {
		signed long value(0);
		VS_RegistryKey key_client(true, "Current configuration");
		key_client.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "ToS");
		return value;
	}
	// end of VS_ConnectionSock_Implementation::GetTOSValue

	inline bool SetMulticastTTL(unsigned long ttl)
	{
		if ((ttl >= 1) && (ttl <= 255)) {
			unsigned long _ttl(ttl); DWORD ret(0);
			return (WSAIoctl((SOCKET)hio, SIO_MULTICAST_SCOPE, &_ttl, sizeof(_ttl), 0, 0, &ret, 0, 0) == 0);
		} else return true;
	}
	// end of VS_ConnectionSock_Implementation::SetMulticastTTL

	inline bool SetQOSSocket(_QualityOfService *qos_params) {
		_QualityOfService * m_qos = qos_params;
		if (m_qos == NULL) { __apply_QoS_changes(); m_qos = &g711_qos; }
		DWORD dwBytesRet = 0;
		return (WSAIoctl((SOCKET)hio, SIO_SET_QOS, (void *)m_qos, sizeof(_QualityOfService), NULL, 0, &dwBytesRet, NULL, NULL) == 0);
	}
	// end of VS_ConnectionSock_Implementation::SetQOSSocket

public:
	// Связывает сокет с адресом IPv6 (для IPv4 используется метод Bind()).
	inline bool BindV6( in6_addr ip, const unsigned short port,
		const VS_Connection_Type type, const bool exclusiveAddrUse)
	{
		SwitchToIPv6();
		if ((state <= vs_sock_state_not_created && !Socket( type ))
			|| VS_ConnectionSock_Implementation::type != type)	return false;

		SetExclusiveAddrUse(exclusiveAddrUse);
		sockaddr_in6 addr_in6;
		ZeroMemory((void*)&addr_in6, sizeof(addr_in6));

		addr_in6.sin6_family = AF_INET6;
		// Проверяем на multicast адрес.
		if(ip.u.Byte[0] == 0xFF)
			addr_in6.sin6_addr = in6addr_any;
		else
			addr_in6.sin6_addr = ip;
		addr_in6.sin6_port = vs_htons(port);

		if (bind( (SOCKET)hio, (sockaddr *)&addr_in6, sizeof(addr_in6) ) != SOCKET_ERROR)
		{
			// Получаем текстовое представление IPv6 адреса.
			char ipv6_text[256];
			net::inet_ntop(AF_INET6, &ip, ipv6_text, sizeof(ipv6_text));

			SetBindIpPort();
			SetAcceptHostPort( ipv6_text, port );
			state = vs_sock_state_bind;
			return true;
		}
		return false;
	}

	inline void SetExclusiveAddrUse(const bool exclusiveAddrUse)
	{
		unsigned long val = exclusiveAddrUse ? 1 : 0;
		setsockopt((SOCKET)hio, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&val, sizeof(val));
	}

	inline bool Bind( const unsigned long ip, const unsigned short port,
						const VS_Connection_Type type, const bool exclusiveAddrUse )
	{
		SwitchToIPv4();
		if ((state <= vs_sock_state_not_created && !Socket( type ))
			|| VS_ConnectionSock_Implementation::type != type)	return false;

		SetExclusiveAddrUse(exclusiveAddrUse);

		sockaddr_storage   addr_in_stor;
		ZeroMemory( (void *)&addr_in_stor, sizeof(addr_in_stor) );
		sockaddr_in* addr_in = (sockaddr_in*) &addr_in_stor;
		addr_in->sin_family = AF_INET;
		int mask = (ip&0xff000000)>>24;
		bool isMulticast = mask >= 224 && mask <= 239;
		addr_in->sin_addr.S_un.S_addr = isMulticast ? INADDR_ANY : vs_htonl(ip);

		addr_in->sin_port = vs_htons(port);



		if (bind( (SOCKET)hio, (sockaddr *)&addr_in_stor, sizeof(addr_in_stor) ) != SOCKET_ERROR)
		{
			SetBindIpPort();
			SetAcceptHostPort( ip, port );
			state = vs_sock_state_bind;
			return true;
		}
		return false;
	}
	// end of VS_ConnectionSock_Implementation::Bind

	inline bool Bind( const char *host, const unsigned short port,
						const VS_Connection_Type type, const bool exclusiveUseAddr )
	{
		unsigned long   ip = 0;
		in6_addr ip6;
		// True - если используется ipv4 адрес, false - если ipv6.
		bool ipv4flag;
		if(VS_GetIPAddressByHostName(host, currentAddressFamily, &ip, &ip6, &ipv4flag))
		{
			if(ipv4flag) // IPv4
			{
				return Bind(vs_ntohl(ip), port, type, exclusiveUseAddr);
			}
			else // IPv6
			{
				return BindV6(ip6, port, type, exclusiveUseAddr);
			}
		}
		else return false;
	}
	// end of VS_ConnectionSock_Implementation::Bind

	inline bool Listen(const int backlog /*= 0*/, const bool waitAccept /*= false*/)
	{
		if (state < vs_sock_state_bind)
			return false;

		int bcklg = backlog;
		if (!bcklg)		bcklg = 200;//SOMAXCONN;
		if (waitAccept)
		{
			BOOL   condAcc = TRUE;
			if (setsockopt( (SOCKET)hio, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (const char *)&condAcc, sizeof(condAcc) ) == SOCKET_ERROR)
				return false;
		}
		if (listen( (SOCKET)hio, bcklg ) == SOCKET_ERROR)	return false;
		state = vs_sock_state_listen;
		//SetFastSocket(fastMode,"Listen");
		return true;
	}
	// end of VS_ConnectionSock_Implementation::Listen

	inline bool Listen(const unsigned short port,
		const int backlog, const bool waitAccept, const bool exclusiveUseAddr )
	{
		// Вызываем правильный Bind в зависимости от заданного AddressFamily.
		if(currentAddressFamily == AF_INET)
		{
			if (!Bind(ADDR_ANY, port, vs_connection_type_stream, exclusiveUseAddr))		return false;
		}
		else if(currentAddressFamily == AF_INET6)
		{
			if (!BindV6(in6addr_any, port, vs_connection_type_stream, exclusiveUseAddr))		return false;
		}
		else return false;
		return Listen(backlog, waitAccept);
	}

	inline bool ConnectAsynch( const unsigned long ip, const unsigned short port,
							const VS_Connection_Type type ,void *& event)
	{
		SwitchToIPv4();
		if ((state <= vs_sock_state_not_created && !Socket( type ))
			|| VS_ConnectionSock_Implementation::type != type)		return false;

		if (!CreateConnectEvent())
			return false;

		event = connectEvent;

		sockaddr_in   addr_in;		ZeroMemory( (void *)&addr_in, sizeof(addr_in) );
		addr_in.sin_family = AF_INET;
		addr_in.sin_addr.S_un.S_addr = vs_htonl(ip);
		addr_in.sin_port = vs_htons(port);

		AddSocketToFlow(&addr_in);

		WSAEventSelect((SOCKET)hio,connectEvent,FD_CONNECT);

		int res = connect((SOCKET)hio,(sockaddr *)&addr_in, sizeof(addr_in));
		if (res==SOCKET_ERROR)
		{
			long error = GetLastError();
			switch(error)
			{
			case WSAEWOULDBLOCK:break;
			default:  return false;
			}
		}
		SetConnectHostPort( ip, port );
		return true;
	}

	inline bool ConnectAsynchV6( in_addr6 ip6, const unsigned short port,
							const VS_Connection_Type type ,void *& event)
	{
		SwitchToIPv6();
		if ((state <= vs_sock_state_not_created && !Socket( type ))
			|| VS_ConnectionSock_Implementation::type != type)		return false;

		if (!CreateConnectEvent())
			return false;

		event = connectEvent;

		sockaddr_in6   addr_in6;
		ZeroMemory( (void *)&addr_in6, sizeof(addr_in6) );
		addr_in6.sin6_family = AF_INET6;
		addr_in6.sin6_addr = ip6;
		addr_in6.sin6_port = vs_htons(port);

		AddSocketToFlow(&addr_in6);

		WSAEventSelect((SOCKET)hio,connectEvent,FD_CONNECT);

		int res = connect((SOCKET)hio,(sockaddr *)&addr_in6, sizeof(addr_in6));
		if (res==SOCKET_ERROR)
		{
			long error = GetLastError();
			switch(error)
			{
			case WSAEWOULDBLOCK:break;
			default:  return false;
			}
		}
		char ipv6_text[256];
		net::inet_ntop(AF_INET6, &ip6, ipv6_text, sizeof(ipv6_text));
		SetConnectHostPort(ipv6_text, port);
		return true;
	}

	inline bool ConnectAsynch( const char* host, const unsigned short port,
							const VS_Connection_Type type ,void *& event)
	{
		unsigned long   ip = 0;
		in6_addr ip6;
		// True - если используется ipv4 адрес, false - если ipv6.
		bool ipv4flag;
		if(VS_GetIPAddressByHostName(host, currentAddressFamily, &ip, &ip6, &ipv4flag))
		{
			if(ipv4flag) // IPv4
			{
				return ConnectAsynch(vs_ntohl(ip), port, type, event);
			}
			else // IPv6
			{
				return ConnectAsynchV6(ip6, port, type, event);
			}
		}
		else return false;
	}

	// end of VS_ConnectionSock_Implementation::ConnectAsynch
	inline bool GetConnectResult(unsigned long mills = 0,bool noWait = true,bool isFastSocket = false, bool qos = false, _QualityOfService * qos_params = NULL)
	{
		int res = 0;
		if (!noWait)
		{
			res = WaitForSingleObject(connectEvent,mills);
			switch(res)
			{
			case WAIT_OBJECT_0: break;
			case WAIT_ABANDONED_0:
			case WAIT_TIMEOUT:
			default:
				{
					return false;
				} break;
			}
		}
		WSANETWORKEVENTS NetworkEvents;

		WSAEnumNetworkEvents((SOCKET)hio,connectEvent,&NetworkEvents);
		if (NetworkEvents.lNetworkEvents & FD_CONNECT )
		{
			if (0!=NetworkEvents.iErrorCode[ FD_CONNECT_BIT ])
				return false;
			state = vs_sock_state_connected;
			SetBindIpPort();
			SetPeerIpPort();
			if (isFastSocket) SetFastSocket(fastMode,"Connect");
			if (qos) { SetQOSSocket(qos_params); }
			connectType = vs_sock_type_connect;
			if(SOCKET_ERROR == WSAEventSelect((SOCKET) hio,connectEvent,0))
				return false;
			unsigned long mode = 0;
			if (ioctlsocket( (SOCKET)hio, FIONBIO, &mode ))
				return false;
			return true;
		}
		return false;
	}
	// end of VS_ConnectionSock_Implementation::GetConnectResult
	inline bool Connect( const unsigned long ip, const unsigned short port,
							const VS_Connection_Type type , bool isFastSocket = false, const bool qos = false , _QualityOfService * qos_params = NULL)
	{
		SwitchToIPv4();
		if ((state <= vs_sock_state_not_created && !Socket( type, qos ))
			|| VS_ConnectionSock_Implementation::type != type)		return false;
		sockaddr_in   addr_in;		ZeroMemory( (void *)&addr_in, sizeof(addr_in) );
		addr_in.sin_family = AF_INET;
		addr_in.sin_addr.S_un.S_addr = vs_htonl(ip);
		addr_in.sin_port = vs_htons(port);

		AddSocketToFlow(&addr_in);

		if (connect( (SOCKET)hio, (sockaddr *)&addr_in, sizeof(addr_in) ) != SOCKET_ERROR)
		{
			state = vs_sock_state_connected;
			SetBindIpPort();
			SetPeerIpPort();
			if (isFastSocket) SetFastSocket(fastMode,"Connect");
			if (qos) { SetQOSSocket(qos_params); }
			SetConnectHostPort( ip, port );
			connectType = vs_sock_type_connect;
			return true;
		}
		return false;
	}
	// end of VS_ConnectionSock_Implementation::Connect

	inline bool ConnectV6( in6_addr ip, const unsigned short port,
							const VS_Connection_Type type , bool isFastSocket = false, const bool qos = false , _QualityOfService * qos_params = NULL)
	{
		SwitchToIPv6();
		if ((state <= vs_sock_state_not_created && !Socket( type, qos ))
			|| VS_ConnectionSock_Implementation::type != type)		return false;
		sockaddr_in6   addr_in;
		ZeroMemory( (void *)&addr_in, sizeof(addr_in) );
		addr_in.sin6_family = AF_INET6;
		addr_in.sin6_addr = ip;
		addr_in.sin6_port = vs_htons(port);

		AddSocketToFlow(&addr_in);

		if (connect( (SOCKET)hio, (sockaddr *)&addr_in, sizeof(addr_in) ) != SOCKET_ERROR)
		{
			state = vs_sock_state_connected;
			SetBindIpPort();
			SetPeerIpPort();
			if (isFastSocket) SetFastSocket(fastMode,"Connect");
			if (qos) { SetQOSSocket(qos_params); }

			char ipv6_text[256];
			net::inet_ntop(AF_INET6, &ip, ipv6_text, sizeof(ipv6_text));

			SetConnectHostPort( ipv6_text, port );
			connectType = vs_sock_type_connect;
			return true;
		}
		return false;
	}

	inline bool Connect( const char *host, const unsigned short port,
							const VS_Connection_Type type , bool isFastSocket = false, const bool qos = false , _QualityOfService * qos_params = NULL)
	{
		unsigned long ip4;
		in6_addr ip6;
		bool isIPv4;
		// Выполняем преобразование строки в IP адрес.
		if(VS_GetIPAddressByHostName(host, currentAddressFamily, &ip4, &ip6, &isIPv4))
		{
			if(isIPv4) // IPv4
			{
				char ipv4_text[256];
				net::inet_ntop(AF_INET, &ip4, ipv4_text, sizeof(ipv4_text));
				return Connect(vs_ntohl(ip4), port, type , isFastSocket , qos, qos_params);
			}
			else // IPv6
			{
				char ipv6_text[256];
				net::inet_ntop(AF_INET6, &ip6, ipv6_text, sizeof(ipv6_text));
				return ConnectV6(ip6, port, type, isFastSocket, qos, qos_params);
			}
		}
		else return false;
	}
	// end of VS_ConnectionSock_Implementation::Connect

	inline bool Connect( const unsigned long ip, const unsigned short port,
							const VS_Connection_Type type, unsigned long &mills ,
							bool isFastSocket = false , const bool qos = false , _QualityOfService * qos_params = NULL)
	{
		SwitchToIPv4();
		unsigned long   allMs = 0, prevMs = GetTickCount(), currMs;
		if ((state <= vs_sock_state_not_created && !Socket( type, qos ))
				|| VS_ConnectionSock_Implementation::type != type)
			return false;

		while (1)
		{
			bool   ret = Connect( ip, port, type , isFastSocket , qos, qos_params);
			DWORD   err = GetLastError();
			currMs = GetTickCount();
			allMs = currMs - prevMs;
			if (allMs >= mills)		mills = 0;
			else					mills -= allMs;
			if (ret)	return true;
			else	switch (err)
					{
					case WSAETIMEDOUT :
					case WSAEHOSTUNREACH :
					case WSAECONNREFUSED :
					case WSAHOST_NOT_FOUND :	break;
					case WSAENOTSOCK :		hio = INVALID_HANDLE_VALUE;
						state = vs_sock_state_not_created;		Close();
					default :	return false;
					}
			if (!mills)		break;
			prevMs = currMs;
			Sleep( 250 );
		}
		return false;
	}
	// end of VS_ConnectionSock_Implementation::Connect

	inline bool ConnectV6( const in6_addr ip, const unsigned short port,
							const VS_Connection_Type type, unsigned long &mills ,
							bool isFastSocket = false , const bool qos = false , _QualityOfService * qos_params = NULL)
	{
		SwitchToIPv6();
		unsigned long   allMs = 0, prevMs = GetTickCount(), currMs;
		if ((state <= vs_sock_state_not_created && !Socket( type, qos ))
				|| VS_ConnectionSock_Implementation::type != type)
			return false;

		while (1)
		{
			bool   ret = ConnectV6( ip, port, type , isFastSocket , qos, qos_params);
			DWORD   err = GetLastError();
			currMs = GetTickCount();
			allMs = currMs - prevMs;
			if (allMs >= mills)		mills = 0;
			else					mills -= allMs;
			if (ret)	return true;
			else	switch (err)
					{
					case WSAETIMEDOUT :
					case WSAEHOSTUNREACH :
					case WSAECONNREFUSED :
					case WSAHOST_NOT_FOUND :	break;
					case WSAENOTSOCK :		hio = INVALID_HANDLE_VALUE;
						state = vs_sock_state_not_created;		Close();
					default :	return false;
					}
			if (!mills)		break;
			prevMs = currMs;
			Sleep( 250 );
		}
		return false;
	}

	inline bool Connect( const char *host, const unsigned short port,
					const VS_Connection_Type type, unsigned long &mills,
					bool isFastSocket = false , const bool qos = false , _QualityOfService * qos_params = NULL)
	{
		unsigned long ip4;
		in6_addr ip6;
		bool isIPv4;
		// Выполняем преобразование строки в IP адрес.
		if(VS_GetIPAddressByHostName(host, currentAddressFamily, &ip4, &ip6, &isIPv4))
		{
			if(isIPv4) // IPv4
			{
				return Connect(vs_ntohl(ip4), port, type, mills , isFastSocket , qos, qos_params);
			}
			else // IPv6
			{
				return ConnectV6(ip6, port, type, mills, isFastSocket, qos, qos_params);
			}
		}
		else return false;
	}
	// end of VS_ConnectionSock_Implementation::Connect

	inline int SelectIn( unsigned long &mills )
	{
		unsigned long tc = GetTickCount();
		const timeval   tmval = { mills >> 10, (mills & 0x3FF) << 10 };
		fd_set   ibits;
		FD_ZERO(&ibits);
		FD_SET((SOCKET)hio,&ibits);
		int ret = select(0, &ibits, 0, 0, &tmval);
		tc = GetTickCount() - tc;
		mills = (tc >= mills) ? 0 : mills - tc;
		return ret;
	}
	// end of VS_ConnectionSock_Implementation::SelectIn

	inline int SelectOut( unsigned long &mills )
	{
		unsigned long tc = GetTickCount();
		const timeval   tmval = { mills >> 10, (mills & 0x3FF) << 10 };
		fd_set   obits;
		FD_ZERO(&obits);
		FD_SET((SOCKET)hio,&obits);
		int ret = select(0, 0, &obits, 0, &tmval);
		tc = GetTickCount() - tc;
		mills = (tc >= mills) ? 0 : mills - tc;
		return ret;
	}
	// end of VS_ConnectionSock_Implementation::SelectOut
	void SetEventTime()
	{
		eventTime = GetTickCount();
	}
	unsigned long GetEventTime()
	{
		return eventTime;
	}
	inline bool Accept( VS_ConnectionSock_Implementation *list_imp ,
						bool isFastSocket = false, bool qos = false, _QualityOfService * qos_params = NULL )
	{
		if ((state <= vs_sock_state_not_created && !Socket( vs_connection_type_stream, qos ))
				|| state != vs_sock_state_created || type != vs_connection_type_stream)
			return false;
		if (!Alloc( rBM, 512 ))		return false;
		memset( rBM, 0, 512 );
		readOv.b_ov = 0;	readOv.over = over_zero;	readOv.over.hEvent = readEvent;
		readOv.over.Offset = (DWORD)(__int64)hio;
		if (isFastSocket) SetFastSocket( (SOCKET)list_imp->hio , fastMode);
		if (isFastSocket) SetFastSocket( fastMode , "Accept" );
		/*if (!AcceptEx( (SOCKET)list_imp->hio, (SOCKET)hio, rBM, 0,
							sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
							&readOv.b_ov, &readOv.over )
			&& GetLastError() != ERROR_IO_PENDING)	*/
		if (!AcceptEx( (SOCKET)list_imp->hio, (SOCKET)hio, rBM, 0,
							sizeof(sockaddr_storage) + 16, sizeof(sockaddr_storage) + 16,
							&readOv.b_ov, &readOv.over )
			&& GetLastError() != ERROR_IO_PENDING)
		{
			//printf("\n\t AcceptEx fail!");
			return false;
		}
		//printf("\n\t AcceptEx well done");
		if (isFastSocket) SetFastSocket( (SOCKET)list_imp->hio , fastMode);
		if (isFastSocket) SetFastSocket( fastMode , "Accept" );
		if (qos) { SetQOSSocket(qos_params); }
		SetEventTime();
		return isAccept = true;
	}
	// end of VS_ConnectionSock_Implementation::Accept

	inline bool SetAcceptResult( const unsigned long b_trans,
			const struct VS_Overlapped *ov, VS_ConnectionSock_Implementation *list_imp,
			bool isFastSocket = false, bool qos = false, _QualityOfService * qos_params = NULL )
	{
		sockaddr   *bind_addr = 0, *peer_addr = 0;
		if (!isAccept || ov != &readOv)
			return false;
		isAccept = false;
		if (ov->error || !list_imp || list_imp->hio == INVALID_HANDLE_VALUE)
		{
			BFree(rBM);
			return false;
		}
		{	int   bind_len = 0, peer_len = 0;
			//GetAcceptExSockaddrs( rBM, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, (sockaddr **)&bind_addr, &bind_len, (sockaddr **)&peer_addr, &peer_len );
			GetAcceptExSockaddrs( rBM, 0, sizeof(sockaddr_storage) + 16, sizeof(sockaddr_storage) + 16, (sockaddr **)&bind_addr, &bind_len, (sockaddr **)&peer_addr, &peer_len );
			SetBindIpPort( (sockaddr_storage *)bind_addr );	SetPeerIpPort( (sockaddr_storage *)peer_addr );	}
		SOCKET   sock = (SOCKET)list_imp->hio;
		int   res = setsockopt( (SOCKET)hio, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&sock, sizeof(sock) );
		if (res == SOCKET_ERROR)
		{
			BFree( rBM );	//printf("setsockopt is failed!");
			return false;
		}
		else
		{
			//printf("setsockopt is done!");
		}
		int   seconds = 0, bytes = sizeof(seconds);
		res = getsockopt( (SOCKET)hio, SOL_SOCKET, SO_CONNECT_TIME, (char *)&seconds, (PINT)&bytes );
		if (res != NO_ERROR) {		BFree( rBM );	return false;	}
		connectTime = time( 0 ) - seconds;
		state = vs_sock_state_connected;	connectType = vs_sock_type_accept;
		if (isFastSocket) SetFastSocket( (SOCKET)list_imp->hio, fastMode );
		if (isFastSocket) SetFastSocket( fastMode,"SetAcceptResult" );
		if (qos) { SetQOSSocket(qos_params); }
		AddSocketToFlow(peer_addr);
		BFree( rBM );	return true;
	}
	// end of VS_ConnectionSock_Implementation::SetAcceptResult

	inline bool Accept( const char *host, const unsigned short port,
					const VS_Connection_Type type, unsigned long &mills,
					const bool exclusiveUseAddr,
					bool isFastSocket /*= false*/, bool qos /*= false*/, _QualityOfService * qos_params /*= NULL*/ )
	{
		if (type != vs_connection_type_stream
			|| (state <= vs_sock_state_bind && !Bind(host, port, vs_connection_type_stream, exclusiveUseAddr) && !Listen(1, false)))
			return false;
		int ret = SelectIn(mills);
		SOCKET   s = INVALID_SOCKET;
		sockaddr_storage   addr;
		if (ret == 1)
		{
			ZeroMemory((void*)&addr, sizeof(addr));
			BOOL   optval = 0;
			int   optlen = sizeof(BOOL), len = sizeof(addr);
			if (getsockopt( (SOCKET)hio, SOL_SOCKET, SO_ACCEPTCONN, (char *)&optval, &optlen )
				|| !optval)
			{
				if (GetLastError() == WSAENOTSOCK) {	hio = INVALID_HANDLE_VALUE;
					state = vs_sock_state_not_created;	Close();	}}
			else	s = accept( (SOCKET)hio, (sockaddr*)&addr, &len );
		}
		if (hio != INVALID_HANDLE_VALUE)
		{	closesocket( (SOCKET)hio );		hio = INVALID_HANDLE_VALUE;		}
		if (s != INVALID_SOCKET)
		{
			state = vs_sock_state_connected;
			hio = (HANDLE)s;
			SetBindIpPort();
			SetPeerIpPort();
			if (isFastSocket) SetFastSocket( fastMode,"Accept#2" );
			if (qos) { SetQOSSocket(qos_params); }
			SetAcceptHostPort( host, port );
			connectType = vs_sock_type_accept;
			AddSocketToFlow(&addr);
			return true;
		}
		state = vs_sock_state_not_created;
		return false;
	}
	// end of VS_ConnectionSock_Implementation::Accept

	inline int Accept( VS_ConnectionSock_Implementation *list_imp, unsigned long &mills ,
						bool isFastSocket = false, bool qos = false, _QualityOfService * qos_params = NULL )
	{
		if (!list_imp || state != vs_sock_state_not_created)	return -1;
		switch (list_imp->SelectIn( mills ))
		{
		case 1 :	break;
		case 0 :	return -2;
		default :	return -1;
		}
		sockaddr_storage   addr;
		ZeroMemory(&addr, sizeof(addr));
		int   len = sizeof(addr);
		hio = (HANDLE)accept( (SOCKET)list_imp->hio, (sockaddr*)&addr, &len );
		if ((SOCKET)hio == INVALID_SOCKET)	return -1;
		state = vs_sock_state_connected;
		SetBindIpPort();
		SetPeerIpPort();
		if (isFastSocket) SetFastSocket( fastMode,"Accept#3" );
		if (qos) { SetQOSSocket(qos_params); }
		SetAcceptHostPort( list_imp->acceptHost, (const unsigned short)atoi( list_imp->acceptPort ));
		connectType = vs_sock_type_accept;
		AddSocketToFlow(&addr);
		return 1;
	}
	// end of VS_ConnectionSock_Implementation::Accept

	inline int Send( const void *buffer, const unsigned long n_bytes )
	{
		return send( (SOCKET)hio, (char *)buffer, (int)n_bytes, 0 );
	}
	// end of VS_ConnectionSock_Implementation::Send

	inline int Send( const void *buffer, const unsigned long n_bytes, unsigned long &mills, const bool keep_blocked )
	{
		unsigned long   bytes = 0;		int   st;		unsigned long   mode = 1;
		if (ioctlsocket( (SOCKET)hio, FIONBIO, &mode ))		return -1;
		while (1) {		st = SelectOut( mills );
						if (st != 1) {		st = !st ? (int)bytes : -1;		break;		}
						st = Send( (void *)&((char *)buffer)[bytes], n_bytes - bytes );
						if (st <= 0) {		st = -1;	break;		}		bytes += st;
						if (bytes >= n_bytes) {		st = (int)bytes;	break;		}}
		if (keep_blocked)
		{
			mode = 0;
			if (ioctlsocket( (SOCKET)hio, FIONBIO, &mode ))
				return -1;
		}
		return st;
	}
	// end of VS_ConnectionSock_Implementation::Send

	inline int Receive( void *buffer, const unsigned long n_bytes )
	{
		return recv( (SOCKET)hio, (char *)buffer, (int)n_bytes, 0 );
	}
	// end of VS_ConnectionSock_Implementation::Receive

	inline int Receive( void *buffer, const unsigned long n_bytes, unsigned long &mills, bool portion = false)
	{
		unsigned long bytes = 0;
		int st;
		while (1)
		{
			st = SelectIn(mills);
			if (st != 1)
			{
				st = !st ? static_cast<int>(bytes) : -1;
				break;
			}
			st = Receive(reinterpret_cast<char*>(buffer) + bytes, n_bytes - bytes);
			if (st <= 0)
			{
				st = -1;
				break;
			}
			bytes += st;
			if (bytes >= n_bytes || portion)
			{
				st = static_cast<int>(bytes);
				break;
			}
		}
		return st;
	}
	// end of VS_ConnectionSock_Implementation::Receive

	inline int ReceiveFrom( void *buffer, const unsigned long n_bytes,
								char *from_host, const unsigned long from_host_size,
								unsigned short *port, unsigned long &milliseconds )
	{
		if (state != vs_sock_state_bind)	return -1;
		int ret = SelectIn(milliseconds);
		switch (ret)
		{
		case 1 :	break;
		case 0 :	return -2;
		default :	return -1;
		}
		struct sockaddr_storage   addr = { 0 };
		int   addr_len = sizeof(addr);
		ret = recvfrom( (SOCKET)hio, (char *)buffer, (int)n_bytes, 0, (sockaddr *)&addr, &addr_len );
		if (ret == SOCKET_ERROR)	return -1;
		hostent   *hst = 0;
		void* paddr = 0;
		if(addr.ss_family == AF_INET)
		{
			sockaddr_in* addr4 = (sockaddr_in*) &addr;
			if(port) *port = vs_ntohs(addr4->sin_port);
			hst = gethostbyaddr( (const char *)&addr4->sin_addr, sizeof(addr4->sin_addr), AF_INET );
		}
		else
		{
			sockaddr_in6* addr6 = (sockaddr_in6*) &addr;
			if(port) *port = vs_ntohs(addr6->sin6_port);
			hst = gethostbyaddr( (const char *)&addr6->sin6_addr, sizeof(addr6->sin6_addr), AF_INET6 );
		}
		if(hst)
		{
			strncpy(from_host, hst->h_name, (size_t)(from_host_size - 1));
			from_host[from_host_size - 1] = 0;
		}
		else
		{
			net::inet_ntop(addr.ss_family, paddr, from_host, from_host_size);
		}
		return ret;
	}
	// end of VS_ConnectionSock_Implementation::ReceiveFrom
	inline bool ReceiveFrom(void* buf,const unsigned long nSizeBuf,char  lpFrom[16],
									unsigned long* &puIPFrom, unsigned short* &puPortFrom)
	{
		if((!isRead)&&(Alloc( rBM, 65536 )))
		{

			readOv.b_last = readOv.b_trans = 0;
			readOv.b_want = nSizeBuf;
			//rB = buf;
			rB = rBM;
			readOv.over = over_zero;
			readOv.over.hEvent = readEvent;
			puIPFrom = &((sockaddr_in*)lpFrom)->sin_addr.s_addr;
			puPortFrom = &((sockaddr_in*)lpFrom)->sin_port;
			m_flag_tmp = 0;
			//WSABUF   wbuf = { readOv.b_want, (char *)rB};
			m_wsaRecvBuf.len = readOv.b_want;
			m_wsaRecvBuf.buf = (char *)rB;
			//int		nLenAddr = sizeof(sockaddr_in);
			m_addr_len = sizeof(sockaddr_in);
			if(SOCKET_ERROR == WSARecvFrom((SOCKET)hio,&m_wsaRecvBuf,1,&readOv.b_ov,&m_flag_tmp,
				(sockaddr*)lpFrom,&m_addr_len,&readOv.over,0))
			{
				DWORD dwErr = GetLastError();
				switch(dwErr)
				{
				case ERROR_IO_PENDING:
					rB = buf;
					break;
				case WSAECONNRESET:
					return ResetRead();
				default:
					readOv.error = dwErr;
					return ResetRead();
				}
				isReadOv = true;
				fRdOv = true;
			}
			else
			{
				rB = buf;
				isReadOv = true;
				fRdOv = true;
			}
			return isRead = true;
		}
		return ResetRead();
	}
	// end of VS_ConnectionSock_Implementation::ReceiveFrom

	inline bool ReceiveFromV6(void* buf,const unsigned long nSizeBuf,char*  lpFrom,
									in6_addr* &puIPFrom, unsigned short* &puPortFrom)
	{
		if((!isRead)&&(Alloc( rBM, 65536 )))
		{

			readOv.b_last = readOv.b_trans = 0;
			readOv.b_want = nSizeBuf;
			//rB = buf;
			rB = rBM;
			readOv.over = over_zero;
			readOv.over.hEvent = readEvent;
			puIPFrom = &((sockaddr_in6*)lpFrom)->sin6_addr;
			puPortFrom = &((sockaddr_in6*)lpFrom)->sin6_port;
			m_flag_tmp = 0;
			//WSABUF   wbuf = { readOv.b_want, (char *)rB};
			m_wsaRecvBuf.len = readOv.b_want;
			m_wsaRecvBuf.buf = (char *)rB;
			//int		nLenAddr = sizeof(sockaddr_in);
			m_addr_len = sizeof(sockaddr_in6);
			if(SOCKET_ERROR == WSARecvFrom((SOCKET)hio,&m_wsaRecvBuf,1,&readOv.b_ov,&m_flag_tmp,
				(sockaddr*)lpFrom,&m_addr_len,&readOv.over,0))
			{
				DWORD dwErr = GetLastError();
				switch(dwErr)
				{
				case ERROR_IO_PENDING:
					rB = buf;
					break;
				case WSAECONNRESET:
					return ResetRead();
				default:
					readOv.error = dwErr;
					return ResetRead();
				}
				isReadOv = true;
				fRdOv = true;
			}
			else
			{
				rB = buf;
				isReadOv = true;
				fRdOv = true;
			}
			return isRead = true;
		}
		return ResetRead();
	}

	// end of VS_ConnectionSock_Implementation::ReceiveFromV6
	inline int ReceiveFrom( void *buffer, const unsigned long n_bytes,
								unsigned long * from_ip,
								unsigned short *port, unsigned long &milliseconds )
	{
		if (state != vs_sock_state_bind)	return -1;
		int ret = SelectIn(milliseconds);
		switch (ret)
		{
		case 1 :	break;
		case 0 :	return -2;
		default :	return -1;
		}
		struct sockaddr_in   addr = { 0 };
		int   addr_len = sizeof(addr);
		ret = recvfrom( (SOCKET)hio, (char *)buffer, (int)n_bytes, 0, (sockaddr *)&addr, &addr_len );
		if (ret == SOCKET_ERROR)	return -1;

		/*hostent   *hst = gethostbyaddr( (const char *)&addr.sin_addr, sizeof(addr.sin_addr), AF_INET );
		const char   *host = !hst ? inet_ntoa( addr.sin_addr ) : hst->h_name;
		if (!host)	return -1;
		strncpy( from_host, host, (size_t)( from_host_size - 1 ));
		from_host[from_host_size - 1] = 0;*/

		if (from_ip)
			*from_ip = vs_ntohl(addr.sin_addr.S_un.S_addr);
		if (port)
			*port = vs_ntohs(addr.sin_port);
		return ret;
	}
	// end of VS_ConnectionSock_Implementation::ReceiveFrom



	inline int SendTo( void *buffer, const unsigned long n_bytes,
							const unsigned long to_ip, const unsigned short to_port )
	{
		struct sockaddr_in   addr;		memset( (void *)&addr, 0, sizeof(addr) );
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = vs_htonl(to_ip);
		addr.sin_port = vs_htons(to_port);
		return sendto( (SOCKET)hio, (const char *)buffer, (int)n_bytes, 0, (sockaddr *)&addr, sizeof(addr) );
	}

	inline int SendToV6( void *buffer, const unsigned long n_bytes,
							in6_addr to_ip, const unsigned short to_port )
	{
		struct sockaddr_in6   addr;
		memset((void *)&addr, 0, sizeof(addr));
		addr.sin6_family = AF_INET6;
		addr.sin6_addr = to_ip;
		addr.sin6_port = vs_htons(to_port);
		return sendto( (SOCKET)hio, (const char *)buffer, (int)n_bytes, 0, (sockaddr *)&addr, sizeof(addr) );
	}

	inline int SendTo( void *buffer, const unsigned long n_bytes,
							const char* to_host, const unsigned short to_port )
	{
		unsigned long ip4;
		in6_addr ip6;
		bool isIPv4;
		// Выполняем преобразование строки в IP адрес.
		if(VS_GetIPAddressByHostName(to_host, currentAddressFamily, &ip4, &ip6, &isIPv4))
		{
			if(isIPv4) // IPv4
			{
				return SendTo(buffer, n_bytes, vs_ntohl(ip4), to_port);
			}
			else // IPv6
			{
				return SendToV6(buffer, n_bytes, ip6, to_port);
			}
		}
		else return false;
	}

	// end of VS_ConnectionSock_Implementation::SendTo

	inline void Disconnect( void )
	{
		RemoveSocketFromFlow();
		ClearExtraFlows();
		if (state >= vs_sock_state_bind)
			Break( (void *)hio );
		else if (state >= vs_sock_state_created)
			closesocket( (SOCKET)hio );
		hio = INVALID_HANDLE_VALUE;
		state = vs_sock_state_not_created;
		//isAccept = false;
	}
	// end of VS_ConnectionSock_Implementation::Disconnect

	inline void Terminate( void )
	{
		RemoveSocketFromFlow();
		ClearExtraFlows();
		if (state >= vs_sock_state_bind)
			Cancel( (void *)hio );
		else if (state >= vs_sock_state_created)
			closesocket( (SOCKET)hio );
		hio = INVALID_HANDLE_VALUE;
		state = vs_sock_state_not_created;
	}
	// end of VS_ConnectionSock_Implementation::Terminate

	inline void Close( void ) {		Disconnect();	}
	// end of VS_ConnectionSock_Implementation::Close

	static inline void Break( void *handle )
	{

		if (!handle || handle == INVALID_HANDLE_VALUE)	return;
		//BOOL b = CancelIo( handle );
		//TransmitFile( (SOCKET)handle, 0, 0, 0, 0, 0, TF_DISCONNECT );
		shutdown( (SOCKET)handle, SD_BOTH );
		closesocket( (SOCKET)handle );
	}
	// end of VS_ConnectionSock_Implementation::Break

	static inline void Cancel( void *handle )
	{
		if (!handle || handle == INVALID_HANDLE_VALUE)	return;
		SOCKET _s = (SOCKET)handle;
		struct linger _l = { 1, 0 };
		setsockopt(_s, SOL_SOCKET, SO_LINGER, (const char *)&_l, sizeof(struct linger));
		closesocket(_s);
	}
	// end of VS_ConnectionSock_Implementation::Cancel

	static inline int SelectIn( const void **handles,
									const unsigned n_handles, unsigned long &mills )
	{
		int SelectIn( const void **handles, const unsigned n_handles, unsigned long &mills );
		return SelectIn( handles, n_handles, mills );
	}
	// end of VS_ConnectionSock_Implementation::SelectIn
	inline void SetTOSSocket( unsigned int tos_value )
	{
		unsigned char buf[4]={0};
		unsigned size = 1;
		///MAX DELAY | MAX THROUGPUT | MAX RELIABILITY | MAX COST
		///By RFC 1340,1349 - only one bit can be equal 1.
		///000(TOS=4bit)0 ///E.G. 000 0001 0
		buf[0] = (unsigned char)(tos_value & 0x0000001E);
		if (setsockopt( (SOCKET)hio , IPPROTO_IP, IP_TOS, (const char*)buf,size)==SOCKET_ERROR)
		{
			return ;
		}
	}
	inline void SetTOS()
	{
		///MAX DELAY | MAX THROUGPUT | MAX RELIABILITY | MAX COST | 0
		SetTOSSocket( TOSValue );
	}
	inline void SetFastSocket( SOCKET hio1 , int mode = 2 )
	{
		char buf[4]={0};
		unsigned size = 1;
		switch(mode)
		{
		case 0: break;
		case 2:
			{
				int * ptr = (int *)buf;
				*ptr = 16384;
				size = 4;
				if (!setsockopt( (SOCKET)hio1 , SOL_SOCKET, SO_RCVBUF, buf,size))
				{
					return ;
				}
			}
		case 1:
			{
				int * ptr = (int *)buf;
				*ptr = 16384;
				size = 4;
				if (!setsockopt( (SOCKET)hio1 , SOL_SOCKET, SO_SNDBUF, buf,size))
				{
					return ;
				}
			}
		default :
			{
				buf[0] = 1;
				size = 4;
				if (setsockopt( (SOCKET)hio , IPPROTO_TCP, TCP_NODELAY, buf,size)==SOCKET_ERROR)
				{
					return ;
				}
				int sizes=4;
				if (getsockopt( (SOCKET)hio , IPPROTO_TCP, TCP_NODELAY, buf,&sizes)==SOCKET_ERROR)
				{
					return ;
				}
			}	break;
		}
	}
///
	inline void SetFastSocket( int mode = 2 )
	{
		SetTOS();
		char buf[4]={0};
		unsigned size = 1;
		switch(mode)
		{
		case 0: break;
		case 5:
			{
				int * ptr = (int *)buf;
				*ptr = 0;
				size = 4;
				if (setsockopt( (SOCKET)hio , SOL_SOCKET, SO_RCVBUF, buf,size)==SOCKET_ERROR)
				{
					return ;
				}
			}
		case 4:
			{
				int * ptr = (int *)buf;
				*ptr = 0;
				size = 4;
				if (setsockopt( (SOCKET)hio , SOL_SOCKET, SO_SNDBUF, buf,size)==SOCKET_ERROR)
				{
					return ;
				}
			}
		case 3:
			{
				int sizes=4;
				if (getsockopt( (SOCKET)hio , IPPROTO_TCP, TCP_NODELAY, buf,&sizes)==SOCKET_ERROR)
				{
					return ;
				}
				buf[0] = 1;
				size = 1;
				if (setsockopt( (SOCKET)hio , IPPROTO_TCP, TCP_NODELAY, buf,size)==SOCKET_ERROR)
				{
					return ;
				}
			}	break;
		case 2:
			{
				int * ptr = (int *)buf;
				*ptr = 16384;
				size = 4;
				if (setsockopt( (SOCKET)hio , SOL_SOCKET, SO_RCVBUF, buf,size)==SOCKET_ERROR)
				{
					return ;
				}
			}
		case 1:
			{
				int * ptr = (int *)buf;
				*ptr = 16384;
				size = 4;
				if (setsockopt( (SOCKET)hio , SOL_SOCKET, SO_SNDBUF, buf,size)==SOCKET_ERROR)
				{
					return ;
				}
			}
		default :
			{
				buf[0] = 1;
				size = 4;
				if (setsockopt( (SOCKET)hio , IPPROTO_TCP, TCP_NODELAY, buf,size)==SOCKET_ERROR)
				{
					return ;
				}
				int sizes=4;
				if (getsockopt( (SOCKET)hio , IPPROTO_TCP, TCP_NODELAY, buf,&sizes)==SOCKET_ERROR)
				{
					return ;
				}
				// fix low speed sending issue on long latecy tcp channels
				sizes = 4;
				int *sb = (int*)buf;
				if (getsockopt((SOCKET)hio, SOL_SOCKET, SO_SNDBUF, buf, &sizes) != SOCKET_ERROR) {

					if (*sb != 0x10000 && *sb != 0) { // 0x10000 - default value on OS without issue, 0 - probally disabled by SetSizeBuffers()
						*sb = 58400;// 32768;
						setsockopt((SOCKET)hio, SOL_SOCKET, SO_SNDBUF, buf, size);
					}
				}

			}	break;
		}
		isFastEnabled = true;
	}
	inline void SetFastSocket(int mode, const char* str)
	{
		//printf("\n\t Run SetFastSocket in : %s",str);
		SetFastSocket(mode);
	}
	inline void SetFastSocket( bool isFast )
	{
		if(isFast)
			SetFastSocket(fastMode);
		/**
		else
			TODO: remove fastMode from SockOpt
		*/
	}

	inline bool SetKeepAliveMode(bool isKeepAlive, unsigned long keepaliveTime, unsigned long keepalliveInterval)
	{
		if(INVALID_HANDLE_VALUE == hio || vs_connection_type_stream!=type)
			return false;
		int optval = isKeepAlive ? 1 : 0;
		int optlen = sizeof(optval);
		tcp_keepalive  alive;
		alive.onoff = true;
		alive.keepalivetime = keepaliveTime;
		alive.keepaliveinterval = keepalliveInterval;
		DWORD bytesResieved(0);
		return(0 == setsockopt((SOCKET)hio,SOL_SOCKET,SO_KEEPALIVE,(const char*)&optval,optlen) && (!isKeepAlive || (0 == WSAIoctl((SOCKET)hio,SIO_KEEPALIVE_VALS,&alive,sizeof(alive),0,0,&bytesResieved,0,0))));
	}
	inline void TestSetFastSocket()
	{
		char buf[4]={0};
		int sizes = 1;
			if (getsockopt( (SOCKET)hio , IPPROTO_TCP, TCP_NODELAY, buf,&sizes)==SOCKET_ERROR)
			{
//				printf("\n\t Error in SetFastSocket -> getsockopt #0");
				LPVOID lpMsgBuf;
				FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL
				);
//				printf("\n\t Error in SetFastSocket -> setsockopt #0 : %s",(char *)lpMsgBuf);
				return ;
			}
//			printf("\n\t GetNoDelay: %d size : %d",*((int *)buf), sizes);
	}
	//end inline void TestSetFastSocket
	inline bool SocketBecomeMulticastReciever(const char *host )
	{
		unsigned long ip4;
		in6_addr ip6;
		bool isIPv4;
		if(VS_GetIPAddressByHostName(host, currentAddressFamily, &ip4, &ip6, &isIPv4))
		{
			if(isIPv4)
			{
				return SocketBecomeMulticastRecieverV4(vs_ntohl(ip4));
			}
			else
			{
				return SocketBecomeMulticastRecieverV6(ip6);
			}
		}
		return false;
	}

	inline bool SocketBecomeMulticastRecieverV4(unsigned long ip )
	{
		if((SOCKET)hio==0) return false;
		ip_mreq         mreq;
		mreq.imr_interface.s_addr = bindAddr.ipv4() ? vs_htonl(bindAddr.ipv4()) : INADDR_ANY;
        mreq.imr_multiaddr.s_addr = vs_htonl(ip);
        int dwRet = setsockopt ((SOCKET)hio, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                reinterpret_cast<char*>(&mreq), sizeof(mreq));
		if(SOCKET_ERROR==dwRet)
		{
			return false;
		}
		return true;
	}

	inline bool SocketBecomeMulticastRecieverV6(in6_addr ip6)
	{
		if((SOCKET)hio == 0) return false;

		ipv6_mreq multicastRequest;

		multicastRequest.ipv6mr_multiaddr = ip6;
        multicastRequest.ipv6mr_interface = GetIPv6InterfaceIndex(ip6);

		return setsockopt((SOCKET)hio, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char*)&multicastRequest, sizeof(multicastRequest)) != SOCKET_ERROR;
	}

	//end inline bool SocketBecomeMulticastReciever
	inline bool SocketBecomeMulticastSender()
	{
		SetTOS();
		return SetMulticastTTL(multicastTTL);
		// the code below is not working due to
		// http://support.microsoft.com/kb/257460
		// do not remove this comment and code below for a support reason
		if((SOCKET)hio==0) return false;
		char ttl[4]={0};
		ttl[0]=100;
		int length = 1;
		if(setsockopt( (SOCKET)hio , IPPROTO_IP , IP_MULTICAST_TTL , ttl , length )==SOCKET_ERROR)
			return false;
		return true;
	}
	// end 	inline bool SocketBecomeMulticastSender()

	// Set socket's option IP_MULTICAST_IF to let socket send all multicast data to this
	// network interface (which use this ip address).
	bool SetMulticastDefaultGateway(const char* myip)
	{
		// supports only ipv4.
		unsigned long ip4;
		in6_addr ip6;
		bool isIPv4;
		if (VS_GetIPAddressByHostName(myip, currentAddressFamily, &ip4, &ip6, &isIPv4))
			{}

		if (isIPv4)
			return setsockopt((SOCKET)hio, IPPROTO_IP, IP_MULTICAST_IF, (CHAR*)&ip4, sizeof(ip4)) != SOCKET_ERROR;
		else {
			int index = GetIPv6InterfaceIndex(ip6);

			return setsockopt((SOCKET)hio, IPPROTO_IPV6, IPV6_MULTICAST_IF, (CHAR*)&index, sizeof(index)) != SOCKET_ERROR;
		}
	}

	int GetIPv6InterfaceIndex(in6_addr ip6)
	{
		VS_IPPortAddress ipv6addr;
		ipv6addr.ipv6(ip6);
		DWORD dwRetVal = 0;

		ULONG family = AF_INET6;

		PIP_ADAPTER_ADDRESSES pAddresses = NULL;
		ULONG outBufLen = 0;

		PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
		PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;

		GetAdaptersAddresses(family, 0, NULL, nullptr, &outBufLen);

		if (outBufLen <= 0)
			return false;

		std::vector<char> _pAddress(outBufLen);
		pAddresses = (PIP_ADAPTER_ADDRESSES)_pAddress.data();
		dwRetVal = GetAdaptersAddresses(family, 0, NULL, pAddresses, &outBufLen);

		if (dwRetVal == NO_ERROR) {
			pCurrAddresses = pAddresses;

			while (pCurrAddresses) {
				pUnicast = pCurrAddresses->FirstUnicastAddress;
				if (pUnicast != NULL) {
					for (; pUnicast != NULL; pUnicast = pUnicast->Next) {
						VS_IPPortAddress addr(*pUnicast->Address.lpSockaddr);
						if (ipv6addr == addr) {
							return pCurrAddresses->Ipv6IfIndex;
						}
					}
				}
				pCurrAddresses = pCurrAddresses->Next;
			}
		}

		return 0;
	}
};
// end of VS_ConnectionSock_Implementation struct

//////////////////////// CONNECTION PIPE IMPLEMENTATION ///////////////////////

struct VS_ConnectionPipe_Implementation : VS_Connection_Implementation
{
	VS_ConnectionPipe_Implementation( void )
		: state(vs_pipe_state_not_created), pipeType(vs_pipe_type_uninstalled)
		, pipeName(0), isConnect(false), tmp_file_fd(-1),tmpFileName(0)
	{}
	// end of VS_ConnectionPipe_Implementation::VS_ConnectionPipe_Implementation

	virtual ~VS_ConnectionPipe_Implementation( void ) {		Close();	}
	// end of VS_ConnectionPipe_Implementation::~VS_ConnectionPipe_Implementation

	VS_Pipe_State   state;
	VS_Pipe_Type   pipeType;
	char   *pipeName;
	char   *tmpFileName;
	bool   isConnect;
	int   tmp_file_fd;

	inline bool SetSizeBuffers( const int writeSize, const int readSize )
	{
		if (writeSize < -1 || writeSize > 1048576 || readSize < -1 || readSize > 1048576)
			return false;
		VS_Connection_Implementation::writeSize = writeSize;
		VS_Connection_Implementation::readSize = readSize;
		return true;
	}
	// end of VS_ConnectionPipe_Implementation::SetSizeBuffers

	inline bool Create( const char *pipeName, const VS_Connection_Type type,
							const VS_Pipe_Type pipeType )
	{
		if (state > vs_pipe_state_not_created || !pipeName || !*pipeName)	return false;
		::free(VS_ConnectionPipe_Implementation::pipeName);
		VS_ConnectionPipe_Implementation::pipeName = _strdup( pipeName );
		if (!VS_ConnectionPipe_Implementation::pipeName)	return false;
		int   writeSize = VS_Connection_Implementation::writeSize,
				readSize = VS_Connection_Implementation::readSize;
		if (writeSize < 0)		writeSize = 0;
		if (readSize < 0)		readSize = 0;
		DWORD   pipeMode = PIPE_WAIT, openMode = FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED;
		switch (VS_Connection_Implementation::type = type)
		{
		case vs_connection_type_stream :	pipeMode |= PIPE_TYPE_BYTE | PIPE_READMODE_BYTE;		break;
		case vs_connection_type_dgram :		pipeMode |= PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE;	break;
		default :							goto go_error;
		}
		switch (VS_ConnectionPipe_Implementation::pipeType = pipeType)
		{
		case vs_pipe_type_duplex :		openMode |= PIPE_ACCESS_DUPLEX;		break;
		case vs_pipe_type_inbound :		openMode |= PIPE_ACCESS_INBOUND;	break;
		case vs_pipe_type_outbound :	openMode |= PIPE_ACCESS_OUTBOUND;	break;
		default :						goto go_error;
		}
		hio = CreateNamedPipe( VS_ConnectionPipe_Implementation::pipeName,
								openMode, pipeMode, 2, writeSize, readSize,
								NMPWAIT_USE_DEFAULT_WAIT, 0 );
		if (hio == INVALID_HANDLE_VALUE)
		{
go_error:	free( (void *)VS_ConnectionPipe_Implementation::pipeName);
			VS_ConnectionPipe_Implementation::pipeName = 0;		return false;
		}
		state = vs_pipe_state_created;	return true;
	}
	// end of VS_ConnectionPipe_Implementation::Create

	inline bool Open( const char *pipeName, const VS_Connection_Type type,
						const VS_Pipe_Type pipeType )
	{
		if (state > vs_pipe_state_not_created || !pipeName || !*pipeName)	return false;
		::free(VS_ConnectionPipe_Implementation::pipeName);
		VS_ConnectionPipe_Implementation::pipeName = _strdup( pipeName );
		if (!VS_ConnectionPipe_Implementation::pipeName)	return false;
		DWORD   desiredAccess = 0,	shareMode = 0,
				creationDisposition = OPEN_EXISTING,
				flagsAndAttributes = FILE_FLAG_OVERLAPPED,
				setMode = 0;
		switch (VS_Connection_Implementation::type = type)
		{
		case vs_connection_type_stream :	setMode |= PIPE_READMODE_BYTE;		break;
		case vs_connection_type_dgram :		setMode |= PIPE_READMODE_MESSAGE;	break;
		default :							goto go_error;
		}
		switch (VS_ConnectionPipe_Implementation::pipeType = pipeType)
		{
		case vs_pipe_type_duplex :		desiredAccess |= GENERIC_WRITE | GENERIC_READ;	break;
		case vs_pipe_type_inbound :		desiredAccess |= GENERIC_READ;		break;
		case vs_pipe_type_outbound :	desiredAccess |= GENERIC_WRITE;		break;
		default :						goto go_error;
		}
		hio = CreateFile( VS_ConnectionPipe_Implementation::pipeName,
							desiredAccess, shareMode, 0, creationDisposition,
							flagsAndAttributes, 0 );
		if (hio == INVALID_HANDLE_VALUE && !SetNamedPipeHandleState( hio, &setMode, 0, 0 ))
		{
go_error:	free( (void *)VS_ConnectionPipe_Implementation::pipeName);
			VS_ConnectionPipe_Implementation::pipeName = 0;		return false;
		}
		state = vs_pipe_state_connected;	return true;
	}
	// end of VS_ConnectionPipe_Implementation::Open
	inline bool DeletePipeFile()
	{
		if (tmpFileName!=0 && (*tmpFileName)!=0)
		{
			bool res = (1==DeleteFile( tmpFileName ));
			return res;
		}
		return false;
	}
	inline bool Create( const VS_Connection_Type type, const VS_Pipe_Type pipeType )
	{
		if (tmpFileName)
		{
			DeletePipeFile();
			free( (void *)tmpFileName );	tmpFileName = 0;
		}
		tmpFileName = (char *)malloc( MAX_PATH );
		if (!tmpFileName)	return false;
		memset( (void*)tmpFileName, 0, MAX_PATH);
		char   *tempPath = (char *)malloc( MAX_PATH );
		if (!tempPath) {
			free( (void *)tmpFileName );	tmpFileName = 0; return false;	}
		memset( (void*)tempPath, 0, MAX_PATH );
		DWORD   res = GetTempPath( MAX_PATH - 1, tempPath );
		if (!res || res >= MAX_PATH - 1 )	strcpy( tempPath, "\\.\\" );
		res = GetTempFileName( tempPath, "vs_", 0, tmpFileName );
		free( (void *)tempPath );
		if (!res) {
			free( (void *)tmpFileName );	tmpFileName = 0; return false;	}
		tmp_file_fd = _open( tmpFileName, _O_CREAT | _O_SHORT_LIVED | _O_TEMPORARY, _S_IREAD | _S_IWRITE );
		char   *realName = strrchr( tmpFileName, '\\' );
		realName = !realName ? tmpFileName : ++realName;
		unsigned long   sz = (unsigned long)strlen( VS_PipeDir ) + (unsigned long)strlen( realName ) + 4;
		char   *tempName = (char *)malloc( sz );
		if (!tempName) {	free( (void *)tempName );	return false;	}
		memset( (void*)tempName, 0, sz );	strcpy( tempName, VS_PipeDir );
		strcat( tempName, realName );	//free( (void *)tmpName );
		const bool   ret = Create( tempName, type, pipeType );
		free( (void *)tempName );	return ret;
	}
	// end of VS_ConnectionPipe_Implementation::Create

	inline bool Open( VS_ConnectionPipe_Implementation *pipe_imp,
						const VS_Connection_Type type, const VS_Pipe_Type pipeType )
	{
		if (pipe_imp->state <= vs_pipe_state_not_created)	return false;
		return Open( pipe_imp->pipeName, type, pipeType );
	}
	// end of VS_ConnectionPipe_Implementation::Open

	inline bool Connect( void )
	{
		if (state <= vs_pipe_state_not_created)		return false;
		readOv.b_ov = 0;
		readOv.over = over_zero;
		readOv.over.hEvent = readEvent;
		if (!ConnectNamedPipe( hio, &readOv.over ))
		{
			DWORD   err = GetLastError();
			switch (err)
			{
			case ERROR_IO_PENDING :			break;
			case ERROR_PIPE_CONNECTED :		state = vs_pipe_state_connected;
											return false;
			case ERROR_PIPE_LISTENING :		return false;
			default :						return false;
		}	}
		return isConnect = true;
	}
	// end of VS_ConnectionPipe_Implementation::Connect

	inline int GetConnectResult( unsigned long &mills )
	{
		if (!isConnect)		return false;
		const int   st = GetResult( readOv, mills );
		if (st == -1 || st == -2)	return st;
		state = vs_pipe_state_connected;
		isConnect = false;		return 0;
	}
	// end of VS_ConnectionPipe_Implementation::GetConnectResult

	inline bool SetConnectResult( const unsigned long b_trans,
										const struct VS_Overlapped *ov )
	{
		if (!isConnect || ov != &readOv)	return false;
		state = vs_pipe_state_connected;
		isConnect = false;		return true;
	}
	// end of VS_ConnectionPipe_Implementation::SetConnectResult

	inline void Disconnect( void )
	{
		if (state >= vs_pipe_state_connected)	DisconnectNamedPipe( hio );
		if (state >= vs_pipe_state_created)
		{
			CloseHandle( hio );
			DeletePipeFile();
			free(pipeName);
		}
		if (tmpFileName)
		{
			free(tmpFileName);
			tmpFileName = 0;
		}
		if (tmp_file_fd != -1)		_close( tmp_file_fd );
		hio = INVALID_HANDLE_VALUE;		pipeName = 0;		tmp_file_fd = -1;
		state = vs_pipe_state_not_created;
	}
	// end of VS_ConnectionPipe_Implementation::Disconnect

	inline void Close( void ) {		Disconnect();	}
	// end of VS_ConnectionPipe_Implementation::Close
};
// end of VS_ConnectionPipe_Implementation struct

//////////////////////// CONNECTION PIPE IMPLEMENTATION ///////////////////////

#endif // VS_CONNECTION_TYPES_H

