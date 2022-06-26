#if defined(_WIN32) // Not ported yet

#include "VS_WorkThreadEvents.h"
#include "../../acs/connection/VS_Connection.h"
#include "../../acs/connection/VS_ConnectionOv.h"
#include "../../acs/connection/VS_IOHandler.h"
#include "VS_MessageData.h"

#include "VS_MessageHandlerAdapter.h"

namespace
{
	enum
	{
		e_add_conn,
		e_remove_conn
	};
}

VS_WorkThreadEvents::VS_WorkThreadEvents(const bool freeByThreadTerminate) : VS_WorkThread(freeByThreadTerminate),
m_isStop(false)
{
	m_events.push_back(CreateEvent(0,FALSE,FALSE,0));
	m_messHandler = boost::signals2::deconstruct<VS_MessageHandlerAdapter>();
	m_messHandler->ConnectToHandleMessageWithResult(boost::bind(&VS_WorkThreadEvents::HandleMessWithRes,this,_1,_2));
}
VS_WorkThreadEvents::~VS_WorkThreadEvents()
{
	CloseHandle(m_events[0]);
}
void VS_WorkThreadEvents::Thread(boost::shared_ptr<VS_WorkThread>&&/*keepAlive*/)
{
	unsigned long mills(500);
	while(true)
	{
		unsigned long sz = m_events.size();
		DWORD res = WaitForMultipleObjects(sz,&m_events[0],FALSE,mills);
		if(res == WAIT_TIMEOUT)
			TryMakeTimeOut();
		else if(WAIT_OBJECT_0 == res)
		{
			if(m_isStop)
				break;
			else
				ProcessMessages();
		}
		else if(res< WAIT_OBJECT_0 + sz)
		{
			if(m_handlers.size()<res)
				continue;
			const VS_Overlapped *ov(m_handlers[res-1]);
			///Maybe better save connaction and make GetReadResult then Handle with trans = sz;
			ov->io_handler->Handle(0,ov);
			/// handle
		}
		else if(res < WAIT_ABANDONED_0 + sz)
		{

			/**
				unsigned long index = res - WAIT_ABANDONED_0;
				m_events.erase(m_events.begin() + index);
				m_handlers.erase(m_handlers.begin() + index-1);
			*/
		}
		else
			break;

	}
}
void VS_WorkThreadEvents::Notify()
{
	SetEvent(m_events[0]);
}
void VS_WorkThreadEvents::HandleMessWithRes(const boost::shared_ptr<VS_MessageData> &mess, const boost::shared_ptr<VS_MessResult> &res)
{
	if(!&mess)
		return;
	unsigned long sz(0);
	unsigned long type(0);
	void * p = mess->GetMessPointer(type,sz);
	VS_SendMessageResult<bool> * result = static_cast<VS_SendMessageResult<bool>*>(res.get());
	if(!result)
		return;
	switch(type)
	{
	case e_add_conn:
		result->result() = SetHandledConnection((VS_Connection*)p);
		break;
	case e_remove_conn:
		result->result() = UnBindConnection((VS_Connection*)p);
		break;
	}
}
void VS_WorkThreadEvents::Stop()
{
	m_isStop = true;
	SetEvent(m_events[0]);
}

bool VS_WorkThreadEvents::SetHandledConnection(VS_Connection *conn)
{
	if(!IsCurrent())
	{
		boost::shared_ptr<VS_MessageData> mess(new VS_MessageData(e_add_conn,conn,sizeof(conn)));
		boost::shared_ptr<VS_SendMessageResult<bool>> result(new VS_SendMessageResult<bool>(false));
		Send(m_messHandler,mess,result);
		return result->result();
	}
	HANDLE readEvent = conn->OvReadEvent(), writeEvent = conn->OvWriteEvent();
	const VS_Overlapped *ovRead = conn->ReadOv(), *ovWrite = conn->WriteOv();
	if(!readEvent || !writeEvent || !ovRead || !ovWrite ||
		!ovRead->io_handler || !ovWrite->io_handler)
		return false;
	if(m_events.size() + 2 > MAXIMUM_WAIT_OBJECTS)
		return false;
	m_events.push_back(readEvent);
	m_handlers.push_back(ovRead);
	m_events.push_back(writeEvent);
	m_handlers.push_back(ovWrite);
	return true;
}

bool VS_WorkThreadEvents::UnBindConnection(VS_Connection *conn)
{
	if(!IsCurrent())
	{
		boost::shared_ptr<VS_MessageData> mess(new VS_MessageData(e_remove_conn,conn,sizeof(conn)));
		boost::shared_ptr<VS_SendMessageResult<bool>> result(new VS_SendMessageResult<bool>(false));
		Send(m_messHandler,mess,result);
		return result->result();
	}
	HANDLE h[2] = {conn->OvReadEvent(), conn->OvWriteEvent()};
	for(int i =0;i<2;i++)
	{
		if(!h[i])
			continue;
		size_t it = std::find(m_events.begin(),m_events.end(),h[i]) - m_events.begin();
		m_events.erase(m_events.begin() + it);
		m_handlers.erase(m_handlers.begin() + it-1);
	}
	return true;
}

#endif