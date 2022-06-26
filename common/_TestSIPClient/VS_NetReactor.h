#ifndef VS_NETREACTOR_H_HEADER_INCLUDED_BB932164
#define VS_NETREACTOR_H_HEADER_INCLUDED_BB932164
#include "VS_NetEventHandler.h"
#include "VS_NetEventType.h"
#include <map>

class VS_NetRouter;
class VS_NetHandlerFactory;

//##ModelId=446C852C013F
class VS_NetReactor
{
public:
	VS_NetReactor();
	virtual ~VS_NetReactor();
    //##ModelId=446C85FA0070
	int Init();
    //##ModelId=447836B70259
	int RegisterHandler(int type, VS_NetEventHandler *handler);
    //##ModelId=447836B7025A
	int RemoveHandler(VS_NetOperation * aOperation);
    //##ModelId=447836B70268
	int HandleEvent(VS_NetOperation * aOperation);
    //##ModelId=44800EB1025F
    int GetHandlerFactory(VS_NetHandlerFactory*& aPtr);

protected:
    //##ModelId=447836B7024A
	typedef std::map<unsigned int , VS_NetEventHandler *>  TYPE_ReactorHandlers;
	typedef TYPE_ReactorHandlers::iterator TYPE_ReactorHandlers_it;
	TYPE_ReactorHandlers   m_handlers;
	TYPE_ReactorHandlers_it m_it;
	VS_NetRouter * m_router;
	VS_NetHandlerFactory * m_factory;
	int res;
private:
};



#endif /* VS_NETREACTOR_H_HEADER_INCLUDED_BB932164 */
