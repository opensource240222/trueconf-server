#include "VS_NetEventHandlerEvents.h"
#include "VS_NetOperationEvents.h"
#include "../SIPParserLib/VS_SIPError.h"
#include "VS_NetMessageQuery.h"

VS_NetEventHandlerEvents::VS_NetEventHandlerEvents()
{	
	m_op = (VS_NetOperation*)new VS_NetOperationEvents;
}
VS_NetEventHandlerEvents::~VS_NetEventHandlerEvents()
{
}
int VS_NetEventHandlerEvents::GetHandle( VS_NetOperation *& op )
{
	if (op)
	{
		op = m_op;
		return e_ok;
	}
	return e_null;
}
int VS_NetEventHandlerEvents::HandleEvent( VS_NetOperation * op)
{
	return e_null;
}

int VS_NetEventHandlerEvents::SetHandle( VS_Connection * aConn , VS_NetConnectionInterface * aIf )
{
	VS_NetEventHandler::m_conn = aConn;
	
	VS_NetEventHandler::m_if = aIf;
	
	return e_ok;
}
int VS_NetEventHandlerEvents::Close(int a,unsigned int b)
{
	if (m_if)	m_if->UpdateState(a,b);
	if (m_conn) m_conn->Close();
	return e_bad;
}
///////////////////////////////////////////////////////////////////////////////
///				Connection Handler											///
///////////////////////////////////////////////////////////////////////////////

int VS_NetConnectionHandlerEvents::GetHandle( VS_NetOperation *& op )
{
	//if (m_conn->Type() == vs_connection_type_uninstalled)
	//	return e_badObjectState;
    //???
	if (m_conn->Type() == vs_connection_type_stream)
	{
		VS_ConnectionTCP * tcp = (VS_ConnectionTCP*)m_conn;
		void * handle = 0;
		unsigned long ip = 0,lport = 0;

		if (e_ok!=GetField( e_ip , ip) || e_ok!=GetField(e_port,lport) || !ip || !lport)
		{
			Close(e_connected | e_connecting ,e_connected*0|e_connecting*0);
			return e_badObjectState;
		}
		unsigned short port = (unsigned short)lport;

		if (false==tcp->ConnectAsynch(ip,port, handle))
			return Close(e_connected | e_connecting ,e_connected*0|e_connecting*0);

		m_op = new VS_NetOperationEvents;
		VS_NetOperationEvents * theOp = (VS_NetOperationEvents*)m_op;
		theOp->m_ov.over.hEvent = handle;
		
		op = m_op;

		return e_ok;
	}
	 else
	{
		///UDP Support connect
		VS_ConnectionUDP * udp = static_cast<VS_ConnectionUDP*>(m_conn);
		unsigned long ip = 0,lport = 0;

		if (e_ok!=GetField( e_ip , ip) || e_ok!=GetField(e_port,lport) || !ip || !lport)
		{
			Close(e_connected | e_connecting ,e_connected*0|e_connecting*0);
			return e_badObjectState;
		}
		unsigned short port = (unsigned short)lport;

		if (!udp->Connect(ip,port))
			return Close(e_connected | e_connecting ,e_connected*0|e_connecting*0);;
		void * handle = (void *)CreateEvent(0,0,TRUE,0);
		m_op = new VS_NetOperationEvents;
		VS_NetOperationEvents * theOp = (VS_NetOperationEvents*)m_op;
		theOp->m_ov.over.hEvent = handle;
		
		op = m_op;

		return e_ok;
	}
}
int VS_NetConnectionHandlerEvents::HandleEvent( VS_NetOperation * op)
{
	if (m_conn->Type() == vs_connection_type_uninstalled)
	{
		Close(e_connected | e_connecting ,e_connected*0|e_connecting*0);
		return e_badObjectState;
	}

	if (m_conn->Type() == vs_connection_type_stream)
	{

		VS_ConnectionTCP * tcp = static_cast<VS_ConnectionTCP*>(m_conn);

		if (tcp->GetConnectResult())
		{
			m_if->UpdateState(e_connected | e_connecting ,
								e_connected*1|e_connecting*0);
		}else
		{
			Close(e_connected | e_connecting ,e_connected*0|e_connecting*0);
		}
		///remove me
		return e_bad;
	} else
	{
		///UDP Support connect
		//VS_ConnectionUDP * udp = static_cast<VS_ConnectionUDP*>(m_conn);
		m_if->UpdateState(e_connected | e_connecting ,
					e_connected*1|e_connecting*0);

		return e_bad;
	}
	return e_null;
}
///////////////////////////////////////////////////////////////////////////////
///				Read Handler											///
///////////////////////////////////////////////////////////////////////////////
int VS_NetReadHandlerEvents::GetHandle( VS_NetOperation *& op )
{
	if (m_conn->Type() == vs_connection_type_uninstalled)
		return MakeClose();

	if ((m_conn->Type() == vs_connection_type_stream) ||
		(m_conn->Type() == vs_connection_type_dgram))

	{
		///TCP & UDP
		void* handle = 0;
		if (!(handle=m_conn->OvReadEvent()))
		{
			if (!m_conn->CreateOvReadEvent())
			{
				return MakeClose();
			}
			if (!(handle=m_conn->OvReadEvent()))
				return MakeClose();
		}
		VS_Buffer *theBuffer = new VS_Buffer;

		theBuffer->buffer = new char[65535];
		theBuffer->length = 65535;

		if (m_conn->Read( theBuffer->buffer , theBuffer->length ))
		{
			m_readBuffer = theBuffer;

			theBuffer->length = 0;
			theBuffer = 0;

			m_op = new VS_NetOperationEvents;
			VS_NetOperationEvents * theOp = (VS_NetOperationEvents*)m_op;
			theOp->m_ov.over.hEvent = handle;
			
			op = m_op;
			return e_ok;
		}
	} 
	return MakeClose();
}

int VS_NetReadHandlerEvents::HandleEvent( VS_NetOperation * op)
{
	if (m_conn->Type() == vs_connection_type_uninstalled)
		return e_badObjectState;
	unsigned long mills = 0;
	VS_Buffer *buf = (VS_Buffer*)m_readBuffer;
	void * buffer = 0;
	int length = 0;
	if ((length=m_conn->GetReadResult(mills,&buffer,true))>0)
	{
		printf("rOK");
		buf->length = length;
		if (MakeRead()==e_ok) 
		{
			VS_Buffer *theBuffer = new VS_Buffer;

			if (!theBuffer) 
				return MakeClose();
			theBuffer->buffer = new char[65535];

			if (!theBuffer->buffer)
				return MakeClose();

			theBuffer->length = 65535;

			if (m_conn->Read( theBuffer->buffer , theBuffer->length ))
			{
				m_readBuffer = theBuffer;

				theBuffer->length = 0;
				theBuffer = 0;
				return e_ok;
			}
		}
	}
	
	return MakeClose();
}

int VS_NetReadHandlerEvents::MakeRead()
{
	VS_Buffer *buf = (VS_Buffer*)m_readBuffer;

	void * ptr = 0;
	if (e_ok!=GetField(e_readQueryPtr, (unsigned long &)ptr ))
		return e_bad;

	VS_NetMessageQuery * query = (VS_NetMessageQuery*)ptr;
	query->AddMessageToHeader( *buf );
	///WE CAN SKIP RECIVED MESSAGE
	m_readBuffer = 0;

	return e_ok;
}
int VS_NetReadHandlerEvents::MakeClose()
{
	printf("rBAD");
	return Close(e_reading,e_reading*0);
}
///////////////////////////////////////////////////////////////////////////////
///				Write Handler											///
///////////////////////////////////////////////////////////////////////////////

int VS_NetWriteHandlerEvents::GetHandle( VS_NetOperation *& op )
{
	if (m_conn->Type() != vs_connection_type_uninstalled)
	{
		if ((m_conn->Type() == vs_connection_type_stream) ||
			(m_conn->Type() == vs_connection_type_dgram))

		{
			///TCP & UDP
			void* handle = 0;
			if (!(handle=m_conn->OvWriteEvent()))
			{
				if (!m_conn->CreateOvWriteEvent())
				{
					return MakeClose();
				}
				if (!(handle=m_conn->OvWriteEvent()))
					return MakeClose();
			}

			if (e_ok!=MakeWrite())
				return MakeClose();

			m_op = new VS_NetOperationEvents;
			VS_NetOperationEvents * theOp = (VS_NetOperationEvents*)m_op;
			theOp->m_ov.over.hEvent = handle;
			
			op = m_op;
			printf("wOK");
			return e_ok;
		}
	}
	return MakeClose();
}
int VS_NetWriteHandlerEvents::HandleEvent( VS_NetOperation * op)
{
	if (m_conn->Type() != vs_connection_type_uninstalled)
	{	
		unsigned long mills = 0;
		if (m_conn->GetWriteResult( mills )>0)
		{
			if (e_ok==MakeWrite())
				return e_ok;		
		}
	} else	return MakeClose();
	if (m_if) m_if->UpdateState(e_writing,e_writing*0);
	return e_ok;
}

int VS_NetWriteHandlerEvents::MakeWrite()
{
	void * ptr = 0;
	if (e_ok!=GetField(e_writeQueryPtr, (unsigned long &)ptr ))
		return e_bad;

	VS_NetMessageQuery * query = (VS_NetMessageQuery*)ptr;

	VS_Buffer theBuffer;
	theBuffer.buffer = 0;
	theBuffer.length = 0;

	if (query->IsEmpty())
		return e_bad;

	if (!query->RemoveMessageFromBack( &theBuffer ))
		return e_bad;

	if (!m_conn->Write( theBuffer.buffer , theBuffer.length ))
		return e_bad;
	return e_ok;	
}
int VS_NetWriteHandlerEvents::MakeClose()
{
	printf("wBAD");
	return Close(e_writing,e_writing*0);
}
