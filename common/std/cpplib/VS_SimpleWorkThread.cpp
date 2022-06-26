#if defined(_WIN32) // Not ported yet

#include "VS_SimpleWorkThread.h"

VS_SimpleWorkThread::VS_SimpleWorkThread(const bool freeByThreadTerminate)
	: VS_WorkThread(freeByThreadTerminate)
	, m_event(false)
	, m_isStopped(false)
{}
VS_SimpleWorkThread::~VS_SimpleWorkThread()
{
	Stop();
	WaitStopped();
}
void VS_SimpleWorkThread::Thread(boost::shared_ptr<VS_WorkThread>&&/*keepAlive*/)
{
	while(!m_isStopped)
	{
		m_event.wait();
		ProcessMessages();
	}
}
void VS_SimpleWorkThread::Notify()
{
	m_event.set();
}
void VS_SimpleWorkThread::Stop()
{
	m_isStopped = true;
	Notify();
}

#endif
