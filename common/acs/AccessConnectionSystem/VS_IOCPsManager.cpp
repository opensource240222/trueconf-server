/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: IOCompletionPorts manager
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_IOCPsManager.cpp
/// \brief Реализация IOCompletionPort manager.
/// \note Обеспечивает возможность работать с IO-HANDLEs в разных threads, с разными IOCP.
///

//#define   _MY_DEBUG_

#include <stdio.h>
#include <process.h>
#include <Windows.h>
#include "../../std/cpplib/VS_MemoryLeak.h"
#include "acs/connection/VS_Connection.h"
#include "acs/connection/VS_ConnectionOv.h"
#include "VS_IOCPsManager.h"
#include "std/cpplib/ThreadUtils.h"

extern struct VS_Overlapped   ovZero;
#define   VS_IOCPSM_TIMEOUT_THREAD_SHUTDOWN   60000
#define   VS_IOCPSM_REPEAT_ERROR_NUMBER   10

struct VS_IOCPsManager_Implementation
{
	VS_IOCPsManager_Implementation( void ) :
		isInit(false), hiocp(0), hthr(0)
	{	InitializeCriticalSection( &cs );	}
	// end VS_IOCPsManager_Implementation::VS_IOCPsManager_Implementation

	~VS_IOCPsManager_Implementation( void )
	{	Shutdown();	DeleteCriticalSection( &cs );	}
	// end VS_IOCPsManager_Implementation::~VS_IOCPsManager_Implementation

	CRITICAL_SECTION   cs;
	bool   isInit;
	HANDLE   hiocp, hthr;

	inline void Thread( void )
	{
		DWORD   trans, error, n_repeat_error = 0, repeat_error = 0;
		ULONG_PTR   key;	VS_Overlapped   *pov;
		goto go_continue;
		do
		{
			PostQueuedCompletionStatus( pov->hiocp, trans, key, (LPOVERLAPPED)pov );
go_continue:
			trans = 0;	key = 0;	pov = 0;	error = 0;
			if (!GetQueuedCompletionStatus( hiocp, &trans, &key, (LPOVERLAPPED *)&pov, INFINITE ))
			{
				error = GetLastError();
				if (pov == 0)
				switch (error)
				{
				case WAIT_TIMEOUT :
				case ERROR_SEM_TIMEOUT :	n_repeat_error = 0;		goto go_continue;
				}
				if (!pov)
				{
#ifdef _MY_DEBUG_
printf( "VS_IOCPsManager_Implementation::Thread: if (!pov), error: %u\n", error );
#endif
					if (!n_repeat_error)
					{
						repeat_error = error;	++n_repeat_error;
					}
					else if (repeat_error == error)
					{
						if (++n_repeat_error >= VS_IOCPSM_REPEAT_ERROR_NUMBER)
						{
#ifdef _MY_DEBUG_
printf( "VS_IOCPsManager_Implementation::Thread: if (n_repeat_error >= %u), error: %u\n", VS_IOCPSM_REPEAT_ERROR_NUMBER, error );
#endif
							break;
					}	}
					else	n_repeat_error = 0;
					goto go_continue;
				}
				else	pov->error = error;
				n_repeat_error = 0;
		}	}
		while (pov->hiocp);
#ifdef _MY_DEBUG_
puts( "VS_IOCPsManager_Implementation::Thread: return" );
#endif
	}
	// end Thread

	static unsigned __stdcall Thread( void *arg )
	{
		vs::SetThreadName("IOCPManager");
		if (arg)	((VS_IOCPsManager_Implementation *)arg)->Thread();
		_endthreadex( 0 );		return 0;
	}
	// end Thread

	inline bool InitAct( void )
	{
		if (!(hiocp = CreateIoCompletionPort( INVALID_HANDLE_VALUE, 0, 0, 10 )))
		{	ShutdownAct();	return false;	}
		uintptr_t   th = _beginthreadex( 0, 0, Thread, (void *)this, 0, 0 );
		if ( !th || th == -1L) {	Shutdown();		return false;	}
		hthr = (HANDLE)th;	SetThreadPriority( hthr, THREAD_PRIORITY_TIME_CRITICAL );
		return isInit = true;
	}
	// end VS_IOCPsManager_Implementation::InitAct

	inline bool Init( void )
	{
		EnterCriticalSection( &cs );
		bool   ret = InitAct();
		LeaveCriticalSection( &cs );
		return ret;
	}
	// end VS_IOCPsManager_Implementation::Init

	inline void *HandleIocp( void )
	{
		EnterCriticalSection( &cs );
		void   *ret = !isInit ? 0 : (void *)hiocp;
		LeaveCriticalSection( &cs );
		return ret;
	}
	// end VS_IOCPsManager_Implementation::HandleIocp

	inline void ShutdownAct( void )
	{
		if (hthr)
		{
			VS_Overlapped   ov = ovZero;
			if (hiocp)	PostQueuedCompletionStatus( hiocp, 0, 0, (LPOVERLAPPED)&ov );
			if (WaitForSingleObject( hthr, VS_IOCPSM_TIMEOUT_THREAD_SHUTDOWN ) == WAIT_OBJECT_0)
				CloseHandle( hthr );
			hthr = 0;
		}
		if (hiocp) {	CloseHandle( hiocp );	hiocp = 0;	}
	}
	// end VS_IOCPsManager_Implementation::ShutdownAct

	inline void Shutdown( void )
	{
		EnterCriticalSection( &cs );
		ShutdownAct();
		LeaveCriticalSection( &cs );
	}
	// end VS_IOCPsManager_Implementation::Shutdown
};
// end VS_IOCPsManager_Implementation struct

VS_IOCPsManager::VS_IOCPsManager( void ) :
	imp(new VS_IOCPsManager_Implementation)
{}
// end VS_IOCPsManager::VS_IOCPsManager

VS_IOCPsManager::~VS_IOCPsManager( void )
{	if (imp)	delete imp;		}
// end VS_IOCPsManager::~VS_IOCPsManager

bool VS_IOCPsManager::Init( void )
{	return !imp ? false : imp->Init();	}
// end VS_IOCPsManager::Init

void *VS_IOCPsManager::HandleIocp( void )
{	return !imp ? 0 : imp->HandleIocp();	}
// end VS_IOCPsManager::HandleIocp

void VS_IOCPsManager::Shutdown( void )
{	if (imp)	imp->Shutdown();	}
// end VS_IOCPsManager::Shutdown
