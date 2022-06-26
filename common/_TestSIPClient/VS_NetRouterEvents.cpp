#include "VS_NetRouterEvents.h"
#include <memory.h>
#include "../acs/Lib/vs_acslib.h"
#include "VS_NetOperationEvents.h"
#include "VS_NetReactor.h"



VS_NetRouterEvents::VS_NetRouterEvents():
 timeOutPerOperations(100),
 isWait(0)
{
	VS_NetRouter::m_reactor = 0;
	VS_NetRouter::m_stateDone = 0;
	VS_NetRouter::m_thread = 0;
}
VS_NetRouterEvents::~VS_NetRouterEvents()
{
}
int VS_NetRouterEvents::Init(VS_NetReactor * reactor)
{
	if (!VS_AcsLibInitial())
		return e_UNKNOWN;
	memset(m_handles,0,MAX_EVENTS*sizeof(void*));
	if (!m_reactor)
		m_reactor = reactor;
	return e_ok;
}

int VS_NetRouterEvents::GetLastEvent(unsigned long mills, VS_NetOperationEvents *& op,unsigned int &res)
{
	res = WaitForMultipleObjects(m_maxEvents,m_handles,false,mills);
	if (res == WAIT_TIMEOUT)
	{
		/// time out
		return e_timeOut;
	}
	if (res >= WAIT_ABANDONED && res <= WAIT_ABANDONED+m_maxEvents)
	{
		/// error in handle
		/// ѕока ничего не делаем, это случаетс€ крайне редко.
		return e_errorInHandle;		
	}
	if (res >= WAIT_OBJECT_0 && res <= WAIT_OBJECT_0+m_maxEvents)
	{
		///handling
		i = res - WAIT_OBJECT_0;
		op = m_operations[ i ];
		return e_ok;
	}
	return e_none;
}
void VS_NetRouterEvents::ShiftHandles(unsigned int index)
{
	if (!m_maxEvents) 
		return;
	for(i=index;i<m_maxEvents-1;++i)
	{
		m_handles[ i ] = m_handles[i+1];
		m_operations[ i ] = m_operations[i+1];
	}
	--m_maxEvents;
}
int VS_NetRouterEvents::RemoveOperation(VS_NetOperation * op)
{
	VS_NetOperationEvents * ev = (VS_NetOperationEvents*)op;
	m_handles[ev->m_index] = 0;
	m_operations[ev->m_index] = 0;
	ShiftHandles( ev->m_index );
	if (!m_maxEvents)
		isWait = false;
	return e_ok;
}
int VS_NetRouterEvents::AddOperation(VS_NetOperation * op)
{
	if (m_reactor==0)
		return e_badObjectState;

	VS_NetOperationEvents * ev = (VS_NetOperationEvents*)op;
	ev->m_index = ~0;

	for(i=0;i<MAX_EVENTS && ev->m_index==~0;++i)
	{
		if (m_handles[i]==0)
		{
			m_handles[i] = ev->m_ov.over.hEvent;
			m_operations[i] = ev;
			ev->m_index = i;
			m_maxEvents = i+1;
			isWait = true;
		}		
	}
	if (ev->m_index < ~0)
	{
		return e_ok;
	}
	return e_noEnoughHandles;
}
int VS_NetRouterEvents::MainLoop()
{
	VS_NetOperationEvents * ev = 0; 
	int result = 0;
	bool trueValue = true;
	unsigned int counter = ~0;
	unsigned long s,e;
	while(trueValue)
	{
		s = GetTickCount();
		if (isWait)
		{
			result = GetLastEvent(timeOutPerOperations,ev,counter);
			if (result==e_ok)
			{
				m_reactor->HandleEvent(ev);
				//if(e_ok!=m_reactor->HandleEvent(ev))
				//{
				//	m_reactor->RemoveHandler(ev);
				//}
			}
		}
		if (InterlockedCompareExchange(&m_stateDone,0,0)==0)
			trueValue = false;
		e = GetTickCount();
		if ((e-s)<25)
			Sleep(25);
	}
	return 0;
}