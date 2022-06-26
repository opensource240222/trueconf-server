#if defined(_WIN32) // Not ported yet

#include "windows.h"
#include "VS_WorkThread.h"
#include "VS_MessageHandler.h"
#include "VS_TimeoutHandler.h"
#include "../../acs/connection/VS_IOHandler.h"
#include "../../acs/connection/VS_Connection.h"
#include "process.h"
#include "winbase.h"
#include "../../acs/connection/VS_ConnectionOv.h"
#include "std/cpplib/ThreadUtils.h"

namespace
{
	unsigned long __stdcall ThreadExec(void *argv)
	{
		auto f = static_cast<std::function<void()>*>(argv);
		(*f)();
		delete f;
		return 0;
	}
}

VS_WorkThread::VS_WorkThread(const bool freeByThreadTerminate) :m_freeByThreadTerminate(freeByThreadTerminate), m_hThread(0), m_ThreadID(0), m_stop_message_processing(false)
{
	m_mess_handler = boost::signals2::deconstruct<VS_MessageHandlerAdapter>();
	m_mess_handler_connection = m_mess_handler->ConnectToHandleMessage(boost::bind(&VS_WorkThread::HandleMessage,this,_1));
}

VS_WorkThread::~VS_WorkThread()
{
	m_mess_handler_connection.disconnect();
}

bool VS_WorkThread::StartImpl(const char* name)
{
	assert(name);

	if(IsStarted())
		return false;
	if(m_freeByThreadTerminate)	m_this = shared_from_this();
	m_hThread = CreateThread(NULL, 0, &ThreadExec, new std::function<void()>(
		[this, name = std::string(name), keepAlive = shared_from_this()] () mutable
	{
		vs::SetThreadName(name.c_str());
		Thread(std::move(keepAlive));
	}), 0, &m_ThreadID);
	return !!m_hThread;
}

void VS_WorkThread::Post(const boost::shared_ptr<VS_MessageHandler> &h, const boost::shared_ptr<VS_MessageData> &data)
{
	/**
		Add to queue
		Notify
	*/
	if (!h)
		return;
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_tasks.emplace([h, data]() { h->HandleMessage(data); });
	}
	Notify();
}
void VS_WorkThread::Send(const boost::shared_ptr<VS_MessageHandler> &h, const boost::shared_ptr<VS_MessageData> &data,const boost::shared_ptr<VS_MessResult> &outRes)
{
	if (!h)
		return;
	if(IsCurrent())
	{
		h->HandleMessageWithResult(data,outRes);
		return;
	}
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_tasks.emplace([h, data, outRes]() {
			h->HandleMessageWithResult(data, outRes);
			if (outRes)
				outRes->set();
		});
	}
	Notify();
	if(outRes)
		outRes->wait();
}
void VS_WorkThread::TryMakeTimeOut()
{
	// todo(kt): make dynamic timeout mills

	static const auto interval = std::chrono::seconds(1);
	auto dur = std::chrono::steady_clock::now() - m_last_timeout;
	if(dur>=interval)
	{
		m_last_timeout = std::chrono::steady_clock::now();
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_tasks.emplace([this]() { Timeout(); });
		}
		Notify();
	}
}
void VS_WorkThread::WaitStopped()
{
	if (m_hThread)
	{
		if(!IsCurrent())
			WaitForSingleObject(m_hThread, INFINITE);
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_hThread == 0)
				return;
			CloseHandle(m_hThread);
			m_hThread = 0;
			m_ThreadID = 0;
		}
		m_fireThreadTerminated();
		m_this.reset();
	}
}

void VS_WorkThread::ProcessMessages()
{
	bool isEmpty(true);
	do
	{
		task_type task;
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			if(m_tasks.empty())
				return;
			task = std::move(m_tasks.front());
			m_tasks.pop();
			isEmpty = m_tasks.empty();
		}
		task();
	} while (!isEmpty && !m_stop_message_processing);

	if(!m_stop_message_processing)
		TryMakeTimeOut();
	else
	{
		while (m_tasks.size())
			m_tasks.pop();
	}

}
void VS_WorkThread::HardStop()
{
	m_stop_message_processing = true;
	Stop();
}
bool VS_WorkThread::IsCurrent() const
{
	return m_ThreadID == GetCurrentThreadId();
}
std::string VS_WorkThread::GetThreadId() const
{
	return std::to_string(m_ThreadID);
}
bool VS_WorkThread::IsStarted() const
{
	//return m_thread.joinable();
	return !m_hThread ? false : WAIT_TIMEOUT==WaitForSingleObject(m_hThread,0);
}
bool VS_WorkThread::RegisterTimeout(const boost::shared_ptr<VS_TimeoutHandler> &h)
{
	if(!h)
		return false;
	std::unique_lock<std::mutex> lock(m_mutex);
	auto result = m_timeoutConns.emplace(h.get(), boost::signals2::connection());
	if (!result.second) // New element wasn't added
		return false;
	result.first->second = m_fireTimeout.connect(boost::bind(&VS_WorkThread::Timeout, this, boost::weak_ptr<VS_TimeoutHandler>(h)));
	return true;
}
void VS_WorkThread::UnregisterTimeout(const boost::shared_ptr<VS_TimeoutHandler> &h)
{
	if(!h)
		return;
	std::unique_lock<std::mutex> lock(m_mutex);
	m_timeoutConns.erase(h.get());
}

void VS_WorkThread::Timeout()
{
	m_fireTimeout();
}
void VS_WorkThread::Timeout(boost::weak_ptr<VS_TimeoutHandler> h)
{
	boost::shared_ptr<VS_TimeoutHandler> ptr = h.lock();
	if(!ptr)
		return;
	ptr->Timeout();
}


void VS_WorkThread::Handle(const unsigned long sz, const struct VS_Overlapped *ov)
{
	if(!IsCurrent())
	{
		unsigned long len = sizeof(sz) + sizeof(ov);
		char* data = (char*)malloc(len);
		memcpy(data,&sz,sizeof(sz));
		memcpy(data+sizeof(sz),&ov,sizeof(ov));
		Post(m_mess_handler, boost::shared_ptr<VS_MessageData>(new VS_MessageData(MESSAGE_HANDLE,data,len)));
		return;
	}
	ov->io_handler->Handle(sz,ov);
}

void VS_WorkThread::HandleError(const unsigned long err, const struct VS_Overlapped *ov)
{
	if(!IsCurrent())
	{
		unsigned long len = sizeof(err) + sizeof(ov);
		char* data = (char*)malloc(len);
		memcpy(data,&err,sizeof(err));
		memcpy(data+sizeof(err),&ov,sizeof(ov));
		Post(m_mess_handler, boost::shared_ptr<VS_MessageData>(new VS_MessageData(MESSAGE_ERROR,data,len)));
		return;
	}
	ov->io_handler->HandleError(err,ov);
}

void VS_WorkThread::HandleMessage(const boost::shared_ptr<VS_MessageData> &message)
{
	unsigned long type, size;
	char *data = (char*)message->GetMessPointer(type, size);
	const unsigned long first = *((unsigned long*)data);
	struct VS_Overlapped *ov = *(VS_Overlapped**)(data + sizeof(first));
	switch ( type )
	{
	case MESSAGE_HANDLE:
		Handle(first,ov);
		break;
	case MESSAGE_ERROR:
		ov->error = first;
		HandleError(first,ov);
		break;
	default:
		break;
	}
	free(data);
}

#endif
