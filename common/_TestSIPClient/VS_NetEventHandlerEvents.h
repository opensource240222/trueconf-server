#ifndef VS_NETEVENTHANDLEREVENTS_H_HEADER_INCLUDED_BB87B5F3
#define VS_NETEVENTHANDLEREVENTS_H_HEADER_INCLUDED_BB87B5F3
#include "VS_NetEventHandler.h"


//##ModelId=4478374701F8
class VS_NetEventHandlerEvents : public VS_NetEventHandler
{
public:
	VS_NetEventHandlerEvents();
	virtual ~VS_NetEventHandlerEvents();
	virtual int GetHandle( VS_NetOperation *& op );
	virtual int HandleEvent( VS_NetOperation * op);
	virtual int SetHandle( VS_Connection * aConn , VS_NetConnectionInterface * aIf = 0 ); 
protected:
	virtual int Close(int a,unsigned int b);
private:
};

class VS_NetConnectionHandlerEvents : public  VS_NetEventHandlerEvents
{
public:
	virtual int GetHandle( VS_NetOperation *& op );
	virtual int HandleEvent( VS_NetOperation * op);
protected:
private:
};

class VS_NetReadHandlerEvents : public  VS_NetEventHandlerEvents
{
public:
	virtual int GetHandle( VS_NetOperation *& op );
	virtual int HandleEvent( VS_NetOperation * op);
protected:
	virtual int MakeRead();
	int MakeClose();
private:
	void * m_readBuffer;
};

class VS_NetWriteHandlerEvents : public  VS_NetEventHandlerEvents
{
public:
	virtual int GetHandle( VS_NetOperation *& op );
	virtual int HandleEvent( VS_NetOperation * op);
protected:
	int MakeWrite();
	int MakeClose();
private:
};


#endif /* VS_NETEVENTHANDLEREVENTS_H_HEADER_INCLUDED_BB87B5F3 */
