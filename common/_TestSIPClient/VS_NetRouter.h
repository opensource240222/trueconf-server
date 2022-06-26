#ifndef VS_NETROUTER_H_HEADER_INCLUDED_BB8B99BA
#define VS_NETROUTER_H_HEADER_INCLUDED_BB8B99BA

#include "VS_NetOperation.h"

class VS_NetReactor;

//##ModelId=446C914E008D
class VS_NetRouter
{
  public:
    //##ModelId=44742BA2001B
	VS_NetRouter();
	virtual ~VS_NetRouter();
	virtual int AddOperation(VS_NetOperation * op) = 0;
	virtual int RemoveOperation(VS_NetOperation * op) = 0;
	virtual int Start(VS_NetReactor * ptr = 0);
	virtual int MainLoop();
	virtual int Stop();
	virtual int Init(VS_NetReactor * ptr = 0) = 0;
protected:
	friend unsigned int _stdcall MainLoopThread(void * args);
	unsigned int *m_thread;
	long m_stateDone;
	VS_NetReactor * m_reactor;
};



#endif /* VS_NETROUTER_H_HEADER_INCLUDED_BB8B99BA */
