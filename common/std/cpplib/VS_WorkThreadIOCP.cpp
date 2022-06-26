#include <boost/make_shared.hpp>
#if defined(_WIN32) && !defined(_TRY_PORTED_) // Not ported yet

#include "VS_WorkThreadIOCP.h"
#include "process.h"
#include "../../acs/connection/VS_Connection.h"
#include "../../acs/connection/VS_ConnectionOv.h"
#include "../../acs/connection/VS_IOHandler.h"

VS_WorkThreadIOCP::VS_WorkThreadIOCP(const bool freeByThreadTerminate) : VS_WorkThread(freeByThreadTerminate),
m_iocp(0)
{
	m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);
	m_processing_messageOv = new VS_Overlapped;
	memset(m_processing_messageOv,0,sizeof(VS_Overlapped));
	m_stopOv = new VS_Overlapped;
	memset(m_stopOv,0,sizeof(VS_Overlapped));
}
VS_WorkThreadIOCP::~VS_WorkThreadIOCP()
{
	Stop();
	WaitStopped();
	delete m_processing_messageOv;
	delete m_stopOv;
	CloseHandle(m_iocp);
}


void VS_WorkThreadIOCP::Thread(boost::shared_ptr<VS_WorkThread>&& keepAlive)
{
	if(!m_iocp)
		return;
	VS_Overlapped	*pov(0);
	ULONG_PTR		key(0);
	DWORD			ByteTransfer(0);
	/**
		TODO: make dynamic
	*/
	auto self_w = weak_from_this();
	// Extend life for safety weak_from_this() call, after that keepAlive isn't needed
	keepAlive.reset();
	unsigned long mills(500);
	while(true)
	{
		auto self = self_w.lock();
		if (!self)
			return;
		BOOL bRes = GetQueuedCompletionStatus(m_iocp,&ByteTransfer,&key,(OVERLAPPED**)&pov,mills);
		if(!bRes)
		{
			DWORD dwErr = GetLastError();
			if(dwErr == WAIT_TIMEOUT)
				TryMakeTimeOut();
			else
			{
				if (pov && pov->io_handler)
					HandleError(pov->io_handler, dwErr, pov);
			}
		}
		else
		{
			if(pov == m_processing_messageOv)
				ProcessMessages();
			else if(pov == m_stopOv)
				break;
			else if(pov && pov->io_handler){
				if (key)
					HandleError(pov->io_handler, key, pov);
				else
					Handle(pov->io_handler, ByteTransfer, pov);
				TryMakeTimeOut();
			}
		}
	}
	auto self = self_w.lock();
	if (!self)
		return;
	ProcessMessages();
}

void VS_WorkThreadIOCP::Notify()
{
	if(!PostQueuedCompletionStatus(m_iocp,0,0,(OVERLAPPED*)m_processing_messageOv))
		printf("PostQueuedCompletionStatus failed! error = %ld\n", GetLastError());
}

bool VS_WorkThreadIOCP::SetHandledConnection(VS_Connection *conn)
{
	if(conn->SetIOCP(m_iocp))
		return true;
	if(ERROR_INVALID_PARAMETER == GetLastError())
	{
		conn->SetOvFildIOCP(m_iocp);
		return true;
	}
	return false;
}
void VS_WorkThreadIOCP::Stop()
{
	if(!PostQueuedCompletionStatus(m_iocp,0,0,(OVERLAPPED*)m_stopOv))
		printf("PostQueuedCompletionStatus failed! error = %ld\n", GetLastError());
}

void VS_WorkThreadIOCP::Handle(const unsigned long sz, const struct VS_Overlapped *ov)
{
	PostQueuedCompletionStatus(m_iocp,sz,0,(OVERLAPPED*)ov);
}

void VS_WorkThreadIOCP::HandleError(const unsigned long err, const struct VS_Overlapped *ov)
{

	PostQueuedCompletionStatus(m_iocp,0,err,(OVERLAPPED*)ov);
}

void VS_WorkThreadIOCP::Handle(VS_IOHandler* handle, const unsigned long sz, const VS_Overlapped* ov)
{
	handle->Handle(sz, ov);
}

void VS_WorkThreadIOCP::HandleError(VS_IOHandler* handle, const unsigned long err, const VS_Overlapped* ov)
{
	handle->HandleError(err, ov);
}


static std::function<boost::shared_ptr<VS_WorkThread>(const char *)> creatorWorkThread;
static std::mutex  creatorMutex;


void SetCreatorWorkThread(std::function<boost::shared_ptr<VS_WorkThread>(const char *)> creator)
{
	std::lock_guard<decltype(creatorMutex)> _{ creatorMutex };
	creatorWorkThread = std::move(creator);
}


boost::shared_ptr<VS_WorkThread> MakeWorkThread(const char *nameThread)
{
	std::lock_guard<decltype(creatorMutex)> _{ creatorMutex };
	if (creatorWorkThread)
	{
		return creatorWorkThread(nameThread);
	}
	//default work thread;
	return boost::make_shared<VS_WorkThreadIOCP>();
}

#endif
