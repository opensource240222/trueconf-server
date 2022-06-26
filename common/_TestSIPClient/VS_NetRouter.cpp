#include "VS_NetRouter.h"
#include "../SIPParserLib/VS_SIPError.h"

#include <process.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>

//##ModelId=44742BA2001B
VS_NetRouter::VS_NetRouter():m_stateDone(0),m_thread(0),m_reactor(0)
{
}
VS_NetRouter::~VS_NetRouter()
{
}

int VS_NetRouter::Start(VS_NetReactor * ptr)
{
	if (e_ok!=Init())
		return e_bad;

	m_reactor = ptr;

	if (!m_thread)
	{
		m_thread = (unsigned int *)_beginthreadex(0,0,MainLoopThread,this,0,0);
		if (m_thread)
		{
			long val = 1;
			InterlockedExchange( &m_stateDone , val );
		}
		return e_ok;
	}
	return e_bad;
}
int VS_NetRouter::Stop()
{
	long val = 0;
	InterlockedExchange( &m_stateDone , val );
	long res = WaitForSingleObject((HANDLE)m_thread,1000);
	if (res==WAIT_OBJECT_0)
		return e_ok;
	return false;
}
unsigned int _stdcall MainLoopThread(void *args)
{
	VS_NetRouter * router = (VS_NetRouter*)args;
	int res =  router->MainLoop();
	_endthreadex(res);
	return (unsigned int)res;
}

int VS_NetRouter::MainLoop()
{
	return e_bad;
}
