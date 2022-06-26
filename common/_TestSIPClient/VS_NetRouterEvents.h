#ifndef VS_NETROUTEREVENTS_H_HEADER_INCLUDED_BB8BAD49
#define VS_NETROUTEREVENTS_H_HEADER_INCLUDED_BB8BAD49
#include "VS_NetRouter.h"
#include "..\SIPParserLib\VS_SipError.h"

class VS_NetOperationEvents;

enum TRouterError
{
	e_none = 0,
	e_timeOut = -1,
	e_noEnoughMemory = -2,
	e_noEnoughHandles = -3,
	e_errorInHandle =   -4
};

//##ModelId=44744DA502BB
class VS_NetRouterEvents : public VS_NetRouter, public VS_SIPError
{
  public:
    //##ModelId=44745D860357
    VS_NetRouterEvents();
	~VS_NetRouterEvents();
	virtual int Init(VS_NetReactor * ptr = 0);
	virtual int AddOperation(VS_NetOperation * op);
	virtual int RemoveOperation(VS_NetOperation * op);
	virtual int GetLastEvent(unsigned long mills, VS_NetOperationEvents *& op,unsigned int &res);
	//virtual int Start();
	virtual int MainLoop();
	//virtual int Stop();
protected:
//	static void MainLoopThread(void * args);
	void ShiftHandles(unsigned int index);
private:
	
	unsigned int i;
	static const unsigned int MAX_EVENTS = 63;
	void* m_handles[MAX_EVENTS];
	VS_NetOperationEvents* m_operations[MAX_EVENTS]; 
	unsigned int m_maxEvents;
	unsigned long timeOutPerOperations;
	unsigned long isWait;

};
/// Можно даже сделать произвольное заполнение, как сделано у Саши.
/// r r r r r r s s s s s s c c c c c c 
///
/// rECIVE, sEND, cONNECT, 



#endif /* VS_NETROUTEREVENTS_H_HEADER_INCLUDED_BB8BAD49 */
