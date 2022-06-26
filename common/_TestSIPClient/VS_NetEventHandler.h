#ifndef VS_NETEVENTHANDLER_H_HEADER_INCLUDED_BB933CCA
#define VS_NETEVENTHANDLER_H_HEADER_INCLUDED_BB933CCA
#include <map>
#include "VS_NetOperation.h"


class VS_Connection;
class VS_NetConnectionInterface;

enum TFields
{
	e_ip,
	e_port,
	e_readQueryPtr,
	e_writeQueryPtr
};

//##ModelId=446C863C02A2
class VS_NetEventHandler
{
public:
	VS_NetEventHandler();
	virtual int GetHandle( VS_NetOperation *& op ) = 0;
	virtual int HandleEvent( VS_NetOperation * op) = 0;
	virtual int SetHandle( VS_Connection * aConn , VS_NetConnectionInterface * aIf = 0 ) = 0; 
	virtual int SetField(  unsigned long field,
							unsigned long fieldValue);
	virtual int GetField( const unsigned long field,
							unsigned long &fieldValue);

protected:
	typedef std::map<unsigned long ,unsigned long>	TYPE_Map;
	typedef TYPE_Map::iterator						TYPE_Map_iterator;
	TYPE_Map		  m_map;
	TYPE_Map_iterator m_it;
	VS_NetOperation * m_op;
	VS_Connection * m_conn;
	VS_NetConnectionInterface * m_if;
private:
};



#endif /* VS_NETEVENTHANDLER_H_HEADER_INCLUDED_BB933CCA */
