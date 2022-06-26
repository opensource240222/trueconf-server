#ifndef VS_NETCONNECTION_H_HEADER_INCLUDED_BB87B94B
#define VS_NETCONNECTION_H_HEADER_INCLUDED_BB87B94B

#include <vector>
#include "VS_NetMessageQuery.h"


class VS_NetEventHandler;
class VS_Connection;
class VS_NetReactor;

struct VS_NetConnectionHandler
{
	VS_NetEventHandler* handler;
	int handlerType;
};



//##ModelId=44784855019E
class VS_NetConnection : public VS_NetConnectionInterface
{
    //##ModelId=447848C4013F
public:
	VS_NetConnection();
	virtual ~VS_NetConnection();
	virtual int SetReactor( VS_NetReactor * aReactor );
	virtual int Init(bool isTcp=true, VS_NetReactor * aReactor=0 );
	virtual int Connect(unsigned long ip,unsigned short port);
	virtual int Bind(unsigned short port);
	virtual int Read();
	virtual int Write();
	virtual int AddWriteMessage(char * message, unsigned int message_sz);
	virtual void UpdateState(int aField=0,unsigned int bField=0, void * buffer=0);
	virtual unsigned int GetReadMessage( char *& message, unsigned int& message_sz);
	//virtual int AddHandler(VS_NetEventHandler * aHandler);
	//virtual int GetHandlers(const VS_NetEventHandler **aHandler,unsigned int &aSize);
protected:
	virtual int MakePostCreate();
	virtual int FindHandler(int aHandlerType, VS_NetEventHandler *& aHandler,bool isCreate = true);
	virtual int CreateHandler(int aHandlerType);
	typedef std::vector<VS_NetConnectionHandler> TYPE_HandlersArray;
	typedef TYPE_HandlersArray::iterator TYPE_HandlersArray_it;
    TYPE_HandlersArray m_handlers;
	VS_Connection * m_conn;
	VS_Buffer m_readBuffer;
	VS_Buffer m_writeBuffer;
	VS_NetMessageQuery m_readQuery;
	VS_NetMessageQuery m_writeQuery;
private:
	unsigned long m_state;
	bool m_isTcp;
	TYPE_HandlersArray_it m_it;
	VS_NetReactor * m_reactor;
	int res;
};



#endif /* VS_NETCONNECTION_H_HEADER_INCLUDED_BB87B94B */
