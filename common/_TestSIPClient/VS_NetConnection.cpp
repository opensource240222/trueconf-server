#include "VS_NetConnection.h"
#include "VS_NetEventHandler.h"
#include "VS_NetReactor.h"
#include "VS_NetHandlerFactory.h"
#include "VS_NetEventType.h"

#include "../acs/connection/VS_Connection.h"
#include "../acs/connection/VS_ConnectionTCP.h"
#include "../acs/connection/VS_ConnectionUDP.h"
#include "../SIPParserLib/VS_SIPError.h"

unsigned int VS_NetConnection::GetReadMessage( char *& message, unsigned int& message_sz)
{
	if (m_readQuery.IsEmpty())
		return 0;
	VS_Buffer theBuffer;
	if (m_readQuery.RemoveMessageFromBack(&theBuffer))
	{
		message = static_cast<char*>(theBuffer.buffer);
		message_sz = theBuffer.length;
		return m_readQuery.GetMessagesNumber() + 1 ;
	}
	return 0;
}
int VS_NetConnection::FindHandler(int aHandlerType, VS_NetEventHandler *& aHandler,bool isCreate )
{
	if (!m_handlers.empty())
	{
		m_it = m_handlers.begin();
		for(;m_it != m_handlers.end();++m_it)
		{
			if ((*m_it).handlerType == aHandlerType)
			{
				aHandler = (*m_it).handler;
				return true;
			}
		}
	}
	if (isCreate)
	{
		if (e_ok!=CreateHandler( aHandlerType ))
			return false;
		m_it = m_handlers.begin();
		for(;m_it != m_handlers.end();++m_it)
		{
			if ((*m_it).handlerType == aHandlerType)
			{
				aHandler = (*m_it).handler;
				return true;
			}
		}
	}
	return false;
}
int VS_NetConnection::CreateHandler(int aHandlerType)
{
	int res = 0;

	if (!m_conn) return e_badObjectState;

	if (m_reactor)
	{
		VS_NetHandlerFactory * factory = 0;
		
		if (e_ok !=(res=m_reactor->GetHandlerFactory( factory )))
		{
			return res;
		}
		if (!factory)
		{
			return e_badObjectState;
		}
		VS_NetEventType theType;
		theType.m_type = aHandlerType;

		VS_NetEventHandler * theHandler = 0;
		if (e_ok!=(res=factory->CreateHandler(theType,theHandler)))
		{
			return res;
		}
		if (!theHandler)
		{
			return e_UNKNOWN;
		}
		if (e_ok!=theHandler->SetHandle( m_conn , this ))
			return e_bad;
		VS_NetConnectionHandler handle;
		handle.handler = theHandler;
		handle.handlerType = aHandlerType;

		m_handlers.push_back( handle );
		return e_ok;
	}
	return e_badObjectState;
}
int VS_NetConnection::MakePostCreate()
{
	if (e_ok!=(res=CreateHandler(e_reader )))
		return res;

	if (e_ok!=(res=CreateHandler(e_writer )))
		return res;
	return e_ok;
}
int VS_NetConnection::Init(bool isTcp, VS_NetReactor * aReactor )
{
	if (isTcp)
	{
		if (!m_conn)
			m_conn = new VS_ConnectionTCP;
	}
	else
	{
		if (!m_conn)
			m_conn = new VS_ConnectionUDP;
	}
	m_isTcp = isTcp;

	if (m_conn)
	{
		if (e_ok==SetReactor( aReactor ))
		{
			MakePostCreate();
		}
		return e_ok;
	}
	return e_null;
}

VS_NetConnection::VS_NetConnection():m_reactor(0),m_conn(0),m_state(0)
{}

VS_NetConnection::~VS_NetConnection()
{
}

int VS_NetConnection::SetReactor( VS_NetReactor * aReactor )
{
	if (aReactor)
	{
		m_reactor = aReactor;
		return e_ok;
	}
	return e_null;
}

//int VS_NetConnection::AddHandler(VS_NetEventHandler * aHandler)
//{
//	return e_bad;
//}
//int VS_NetConnection::GetHandlers(const VS_NetEventHandler **aHandler,unsigned int &aSize)
//{
//	return e_bad;
//}
int VS_NetConnection::Connect(unsigned long ip,unsigned short port)
{
	if (!m_conn)
		return e_badObjectState;

	if (m_state & e_connected)
	{
		printf("\n\t MEGAERRROR!");
		return e_badObjectState;
	}

	if (m_state & e_connecting)
	{
		printf("\n\t MEGAERRROR!");
		return e_badObjectState;
	}
	else
	{
		VS_NetEventHandler * theHandler = 0;
		if (e_ok!=FindHandler( e_connector,theHandler ))
			return false;

		theHandler->SetField(e_ip, ip);
		theHandler->SetField(e_port, port);

		if (e_ok!=(res=m_reactor->RegisterHandler(e_connector,theHandler)))
			return res;
		m_state |= e_connecting;
	}
	return e_ok;
}
int VS_NetConnection::Read()
{
	if (!m_conn)
		return e_badObjectState;
	if (m_state & e_reading)
		return e_ok;
	else
	{
		if (m_isTcp)
			if (( m_state & e_connected)==0)
				return e_badObjectState;
		
		VS_NetEventHandler * theHandler = 0;
		if (e_ok!=FindHandler( e_reader,theHandler ))
			return false;

		theHandler->SetField(e_readQueryPtr, reinterpret_cast<unsigned long>(&m_readQuery));
	
		if (e_ok!=(res=m_reactor->RegisterHandler(e_reader,theHandler)))
			return res;
		m_state |= e_reading;
		return e_ok;

	}
	return e_bad;
}
int VS_NetConnection::Write()
{
	if (!m_conn)
		return e_badObjectState;
	if (m_state & e_writing)
		return e_ok;
	else
	{
		if (m_isTcp)
			if (( m_state & e_connected)==0)
				return e_badObjectState;

		VS_NetEventHandler * theHandler = 0;
		if (e_ok!=FindHandler( e_writer,theHandler ))
			return false;

		theHandler->SetField(e_writeQueryPtr, (unsigned long)&m_writeQuery);

		if (e_ok!=(res=m_reactor->RegisterHandler(e_writer,theHandler)))
			return res;

		m_state |= e_writing;
		return e_ok;
	}
	return e_bad;
}
int VS_NetConnection::AddWriteMessage(char * message, unsigned int message_sz)
{
	VS_Buffer theBuffer;

	theBuffer.buffer = message;
	theBuffer.length = message_sz;

	if (!m_writeQuery.AddMessageToHeader( theBuffer ))
		return e_bad;
	return e_ok;
}
void VS_NetConnection::UpdateState(int aField,unsigned int bField, void * buffer)
{
	///A = m_state;
	///C = aField
	///D = bField
	/// Change C bits on D value.
	/// Tmp = (A&C) | C; ///make C bits - in 1.
	/// Tmp = Tmp & D;   ///make C bits in D value
	/// A = Tmp | (A&~C);/// make result

	unsigned int Tmp = (m_state & aField) | aField;
	Tmp = Tmp & bField;
	m_state = Tmp | (m_state & (~aField));
}
int VS_NetConnection::Bind(unsigned short port)
{
	if (m_conn)
	{
		if ((m_conn->Type() == vs_connection_type_stream))
		{
			VS_ConnectionTCP * tcp = static_cast<VS_ConnectionTCP*>(m_conn);
			if (tcp->Bind("VP41",port))
				return e_ok;
		}
		else
		{
			VS_ConnectionUDP * udp = static_cast<VS_ConnectionUDP*>(m_conn);
			if (udp->Bind("VP41",port))
				return e_ok;
		}
	}
	return e_bad;
}