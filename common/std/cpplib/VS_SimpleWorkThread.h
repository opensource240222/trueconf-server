#pragma once
#include "VS_WorkThread.h"
#include "event.h"

/**
	TODO: implement
*/
class VS_SimpleWorkThread : public VS_WorkThread
{
public:
	VS_SimpleWorkThread(const bool freeByThreadTerminate = false);
	~VS_SimpleWorkThread();

	void Stop() override;

	bool SetHandledConnection(VS_Connection* /*conn*/) override { return false; } ///not implemented
	bool IsStopping() {
		return m_isStopped;
	};
protected:
	void Thread(boost::shared_ptr<VS_WorkThread>&&/*keepAlive*/) override;
	void Notify() override;
private:
	vs::event			m_event;
	bool				m_isStopped;
};