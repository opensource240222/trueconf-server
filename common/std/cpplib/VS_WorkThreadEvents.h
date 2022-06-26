#pragma once
#include "VS_WorkThread.h"
#include "windows.h"
#include <vector>

class VS_MessageHandlerAdapter;
class VS_MessageData;
class VS_MessResult;

class VS_WorkThreadEvents : public VS_WorkThread
{

	std::vector<HANDLE> m_events;
	std::vector<const struct VS_Overlapped*> m_handlers;
	bool m_isStop;
	boost::shared_ptr<VS_MessageHandlerAdapter>	m_messHandler;
public:
	~VS_WorkThreadEvents();
	VS_WorkThreadEvents(const bool freeByThreadTerminate = false);

	bool UnBindConnection(VS_Connection *conn);
	bool SetHandledConnection(VS_Connection *conn) override;
	void Stop() override;
private:

	void Thread(boost::shared_ptr<VS_WorkThread>&&/*keepAlive*/) override;

	void Notify() override;

	void HandleMessWithRes(const boost::shared_ptr<VS_MessageData> &mess,const boost::shared_ptr<VS_MessResult> &res);

};