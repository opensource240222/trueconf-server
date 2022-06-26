#pragma once
#include <boost/weak_ptr.hpp>
#include <boost/signals2.hpp>
#include <mutex>
#include <queue>
#include "VS_MessageData.h"
#include "event.h"
#include "std/cpplib/function.h"
#include "std-generic/cpplib/ptr_arg.h"
#include "std-generic/compat/boost/enable_shared_from_this.hpp"
#include "VS_MessageHandlerAdapter.h"
#include <chrono>

#define MESSAGE_HANDLE	1
#define MESSAGE_ERROR	2

/**
	thread for handling tasks in context
	it can work with VS_Connection and call VS_IOHandler::Handle when io operation is conpleted
*/

/**
	TODO:
	make default implementation using boost, without handling VS_Connection
*/
struct VS_Overlapped;
class VS_MessageHandler;
class VS_Connection;
class VS_TimeoutHandler;

class VS_MessResult : public vs::event
{
protected:
	VS_MessResult() : vs::event(false) {}
	~VS_MessResult(){}
};

template <class T>
class VS_SendMessageResult:public VS_MessResult
{
public:
	VS_SendMessageResult(const T &init_value = T()) : m_result(init_value)
	{}
	~VS_SendMessageResult()
	{}
	T & result()
	{
		return m_result;
	}
private:
	T m_result;
};
class VS_WorkThread : public vs_boost::enable_shared_from_this<VS_WorkThread>
{
public:
	typedef vs::function<void()> task_type;
private:
	bool m_freeByThreadTerminate;

	std::queue<task_type> m_tasks;
	std::mutex				m_mutex;
	///boost::thread				m_thread;

	void*	m_hThread;
	unsigned long	m_ThreadID;


	//boost::posix_time::ptime	m_last_timeout;
	std::chrono::steady_clock::time_point	m_last_timeout;

	boost::signals2::signal<void (void)> m_fireTimeout;
	boost::signals2::signal<void (void)> m_fireThreadTerminated;
	boost::shared_ptr<VS_WorkThread>	m_this;

	std::map<VS_TimeoutHandler*,boost::signals2::scoped_connection> m_timeoutConns;

	void Timeout(boost::weak_ptr<VS_TimeoutHandler> h);

	boost::signals2::connection					m_mess_handler_connection;

	bool StartImpl(const char* name);

protected:
	// keepAlive is needed for guarantee that instance will not destroyed until work loop started.
	virtual void Thread(boost::shared_ptr<VS_WorkThread>&& keepAlive) = 0;
	virtual void Notify() = 0;

	void ProcessMessages();
	void Timeout();
	void TryMakeTimeOut();

	boost::shared_ptr<VS_MessageHandlerAdapter> m_mess_handler;
	bool m_stop_message_processing;
public:
	VS_WorkThread(const bool freeByThreadTerminate = false);
	virtual ~VS_WorkThread();

	bool Start(vs::ptr_arg<const char> name)
	{
		return StartImpl(name);
	}
	template <unsigned N>
	bool Start(const char (&name)[N])
	{
		static_assert(N <= 15 + 1, "Thread name is too long");
		return StartImpl(name);
	}
	virtual void Stop() = 0;
	bool IsStarted() const;
	void HardStop(); //process only current message and skip other

	void WaitStopped();

	virtual void Post(task_type task)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_tasks.emplace(std::move(task));
		}
		Notify();
	}

	virtual void Dispatch(task_type task)
	{
		if (IsCurrent())
			task();
		else
		{
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_tasks.emplace(std::move(task));
			}
			Notify();
		}
	}
	virtual void Send(const boost::shared_ptr<VS_MessageHandler> &h, const boost::shared_ptr<VS_MessageData> &data,const boost::shared_ptr<VS_MessResult> &outRes);
 	virtual void Post(const boost::shared_ptr<VS_MessageHandler> &h, const boost::shared_ptr<VS_MessageData> &data);
	virtual bool IsCurrent() const;
	std::string GetThreadId() const;

	virtual bool SetHandledConnection(VS_Connection *conn) = 0; //???
	virtual bool RegisterTimeout(const boost::shared_ptr<VS_TimeoutHandler> &h);
	virtual void UnregisterTimeout(const boost::shared_ptr<VS_TimeoutHandler> &h);

	boost::signals2::connection	ConnectToThreadTerminated(const boost::signals2::signal<void (void)>::slot_type &slot)
	{
		return m_fireThreadTerminated.connect(slot);
	}

	virtual void Handle(const unsigned long sz, const struct VS_Overlapped *ov);
	virtual void HandleError(const unsigned long err, const struct VS_Overlapped *ov);
	void HandleMessage(const boost::shared_ptr<VS_MessageData> &message);
};