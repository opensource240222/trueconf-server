#if defined(_WIN32) // Not ported yet

#include "VS_UdpStreamRouter.h"
#include <process.h>
#include <Windows.h>
#include "../std/cpplib/VS_MemoryLeak.h"
#include "../acs/Lib/VS_AcsLib.h"
#include "acs/connection/VS_ConnectionUDP.h"
#include "../streams/Protocol.h"
#include "../acs/VS_AcsDefinitions.h"
#include "../streams/VS_StreamsDefinitions.h"
#include "../net/EndpointRegistry.h"
#include "std-generic/cpplib/hton.h"
#include "std/cpplib/ThreadUtils.h"
#include "../std/debuglog/VS_Debug.h"
//#include "../Servers/ServersConfigLib/VS_ServersConfigLib.h"

#define DEBUG_CURRENT_MODULE VS_DM_NHP

///////////////////////////////////////////////////////////////////////////////
VS_UdpStreamRouter_TestImp::VS_UdpStreamRouter_TestImp()
:m_port(4567),m_conn(0),handle_server(0),loop_th(0),
 readEvent(0) , writeEvent(0),time_interval(1000)
{
}
// end VS_UdpStreamRouter_TestImp
///////////////////////////////////////////////////////////////////////////////
VS_UdpStreamRouter_TestImp::~VS_UdpStreamRouter_TestImp()
{
}
//end VS_UdpStreamRouter_TestImp::
///////////////////////////////////////////////////////////////////////////////
bool VS_UdpStreamRouter_TestImp::Init()
{
	handle_server =  new VS_USR_HandleServer();
	if(!handle_server) return false;

	m_conn = new VS_ConnectionUDP();
	if(!m_conn) return false;

	const unsigned long sz = 256;
	char host[sz];

	if(!m_conn->CreateOvReadEvent() || !m_conn->CreateOvWriteEvent())
		return false;

	readEvent = m_conn->OvReadEvent();
	writeEvent = m_conn->OvWriteEvent();

	if (!readEvent || !writeEvent)
		return false;

	VS_GetDefaultHostName( host , 256 );
	if (!m_conn->Bind( host, m_port, false )) return false;

	m_flag |=e_s_init;

	return true;
}
//end VS_UdpStreamRouter_TestImp::
///////////////////////////////////////////////////////////////////////////////
HANDLE VS_UdpStreamRouter_TestImp::Start( const char * endpoint_name )
{
	if (m_flag!=e_add_handler) return 0;
	HANDLE th = reinterpret_cast<HANDLE>( _beginthread( Thread , 0 , (void *)this ));
	if (th!=0) m_flag |=e_s_start;
	loop_th = th;
	return th;
}
//end VS_UdpStreamRouter_TestImp::
///////////////////////////////////////////////////////////////////////////////

bool VS_UdpStreamRouter_TestImp::Stop( void )
{
	Sleep( 200 );
	m_flag |= e_s_loop_stop;
	WaitForSingleObject( loop_th , 30000 );
	if (loop_th) CloseHandle( loop_th );
	return true;
}
//end VS_UdpStreamRouter_TestImp::
///////////////////////////////////////////////////////////////////////////////

bool VS_UdpStreamRouter_TestImp::IsValid( void )
{
	return (m_flag >= e_add_handler );
}
//end VS_UdpStreamRouter_TestImp::
///////////////////////////////////////////////////////////////////////////////
void VS_UdpStreamRouter_TestImp::Thread(void * arg)
{
	vs::SetThreadName("UDPSR_Test");
	VS_UdpStreamRouter_TestImp * imp =
	reinterpret_cast< VS_UdpStreamRouter_TestImp* >(arg);
	imp->Loop();
	if (imp->loop_th) imp->loop_th = 0;
}
//end VS_UdpStreamRouter_TestImp::
///////////////////////////////////////////////////////////////////////////////

void VS_UdpStreamRouter_TestImp::Loop()
{
	//////////////////////////////////////////////////////////
	Sleep(300);
	if (m_flag!=e_start) return;
	m_flag |= e_s_loop_start;
	//////////////////////////////////////////////////////////
	char * rdata = new char[ 65536 ];
	int sz = 0;
	char *pAddrFrom = new char[16];
	unsigned long tm = 0;
	unsigned long start,finish = 0;
	HANDLE hs[] = { readEvent , writeEvent };
	tm = time_interval;

//	m_conn->Read( rdata , 65536 );

	while(m_flag<=e_loop_start)
	{
		if (tm==0 || tm > 0xfff)
		{
			if(!PeriodicTick())
			{
				m_flag |= e_s_loop_stop;
				return;
			}
			tm = time_interval;
		}

		start = GetTickCount();
		unsigned short port = 0;
		unsigned long ip = 0;

		unsigned short* pPort = NULL;
		unsigned long* pIP = NULL;

		if(m_conn->AsynchReceiveFrom(rdata,65536,pAddrFrom,pIP,pPort))
		{
			unsigned long to = 10000;
			int				Transfer(0);
			DWORD			dwRes;
			do
			{
				dwRes = WaitForSingleObject(readEvent,10000);
				switch(dwRes)
				{
				case WAIT_OBJECT_0:
					to = 10000;
					Transfer = m_conn->GetReadResult(to,NULL,true);
					if(ProcNHPData(rdata, Transfer, vs_htonl(*pIP), vs_htons(*pPort)))
						printf("NHP data is ok");
					else
						printf("NHP data is bad");
						//printf("\n\t Data received: %s",rdata);
						//printf("\n\t Ip: %d.%d.%d.%d\n\t Port: %d",
						//					((*pIP)&0xff),
						//					((*pIP)&0xff00)>>8,
						//					((*pIP)&0xff0000)>>16,
						//					((*pIP)&0xFF000000)>>24,
						//					 *pPort);
					break;
				case WAIT_ABANDONED:
					printf("Abandoned");
					break;
				case WAIT_TIMEOUT:
					printf("\n\tTimeout");
					break;
				default:
					printf("\n\tWaitForSingleObject Error! ErrNum %ld", GetLastError());
				}
			}while((dwRes==WAIT_TIMEOUT)&&(m_flag<=e_loop_start));
		}

		finish = GetTickCount();
	}
	delete []pAddrFrom;
	//////////////////////////////////////////////////////////
}
//end VS_UdpStreamRouter_TestImp::
///////////////////////////////////////////////////////////////////////////////
bool VS_UdpStreamRouter_TestImp::ProcNHPData(char* data,int sz,unsigned long IP,unsigned short port)
{
	char			chIP[9];
	char			chPort[11];
	NHP_PACK_TYPE	type;
	char			*Pos = data;
	unsigned long	mills;
	int counter(0);

	_itoa(IP,chIP,16);
	_itoa(port,chPort,10);
	std::string	str = (std::string)chIP + (std::string)chPort;
	ClientStateContainer::iterator iter = m_CurrClients.find(str);
	if(ProcReqClientPack(data,sz,IP,port))
		return true;
	if(iter==m_CurrClients.end())
	{
		if(SYN_FRAME_SIZE!=sz)
			return false;
		memcpy(&type,Pos,sizeof(type));
		Pos+=sizeof(type);
		if(SYN!=type)
			return false;
		StateClientInfoType tmp;

		memcpy(&tmp.syndata.RND,Pos,sizeof(tmp.syndata.RND));
		Pos+=sizeof(tmp.syndata.RND);

		memcpy(tmp.syndata.Message,Pos,MESSAGE_SIZE);
		Pos+=MESSAGE_SIZE;

		memcpy(&tmp.syndata.field1,Pos,sizeof(tmp.syndata.field1));
		Pos+=sizeof(tmp.syndata.field1);

		memcpy(&tmp.syndata.field2,Pos,sizeof(tmp.syndata.field2));
		Pos+=sizeof(tmp.syndata.field2);

		memcpy(&tmp.syndata.field3,Pos,sizeof(tmp.syndata.field3));

		int err;
		if(!NHPTransformSYN(&tmp.syndata,&tmp.synackdata,&err))
			return false;
		tmp.curstate = e_SYN_ACK;
		tmp.nat.ip = IP;
		tmp.nat.port = port;
		char* senddatabuf = new char[SYN_ACK_FRAME_SIZE];
		Pos = senddatabuf;
		type = SYN_ACK;

		memcpy(Pos,&type,sizeof(type));
		Pos+=sizeof(type);
		counter+=sizeof(type);

		memcpy(Pos,&tmp.synackdata.SEQ,sizeof(tmp.synackdata.SEQ));
		Pos+=sizeof(tmp.synackdata.SEQ);
		counter+=sizeof(tmp.synackdata.SEQ);

		memcpy(Pos,tmp.synackdata.synhash,HASH_SIZE);
		Pos+=HASH_SIZE;
		counter+=HASH_SIZE;

		memcpy(Pos,&tmp.synackdata.field1,sizeof(tmp.synackdata.field1));
		Pos+=sizeof(tmp.synackdata.field1);
		counter += sizeof(tmp.synackdata.field1);

		memcpy(Pos,&tmp.synackdata.field2,sizeof(tmp.synackdata.field2));
		Pos+=sizeof(tmp.synackdata.field2);
		counter+=sizeof(tmp.synackdata.field2);

		memcpy(Pos,&tmp.synackdata.field3,sizeof(tmp.synackdata.field3));
		counter+=sizeof(tmp.synackdata.field3);
		if(counter!=SYN_ACK_FRAME_SIZE)
			printf("\n\nHEAP IS FAILED!!!!");

		//unsigned long IP1 =

		//m_conn->WriteTo(senddatabuf, SYN_ACK_FRAME_SIZE, vs_htonl(IP), vs_htons(port));
		//bool b = m_conn->Write(senddatabuf,SYN_ACK_FRAME_SIZE);
		bool b = m_conn->WriteTo(senddatabuf,SYN_ACK_FRAME_SIZE,IP,port);
		//Pos = data;
		//Pos+=5;
		//memcpy(&track,Pos,sizeof(track));
		//Pos+=4;
		//memcpy(&UID,Pos,sizeof(UID));
		//int n = SendUdpFrameAct(senddatabuf,SYN_ACK_FRAME_SIZE,track,UID,IP,port);

		int trans;
		switch(WaitForSingleObject(writeEvent,10000))
		{
		case WAIT_OBJECT_0:
			mills = 10000;
			trans = m_conn->GetWriteResult(mills);
			if(trans!=SYN_ACK_FRAME_SIZE)
			{
				delete [] senddatabuf;
				return false;
			}
			m_CurrClients[str] = tmp;
			delete [] senddatabuf;
			return true;
			break;
		default:
			delete [] senddatabuf;
			return false;
		}
	}
	else
	{
		unsigned long size(0);
		memcpy(&type,Pos,sizeof(type));	Pos+=sizeof(type); counter+=sizeof(type);
		if(type!=ACK)
			return false;
		memcpy(&size,Pos,sizeof(size));	Pos+=sizeof(size); counter+=sizeof(size);
		if(size!=sz)
			return false;
		Pos +=2* sizeof(char); //Номер Ack'а и кол-во посланных ack'ов
		StateClientInfoType tmp = m_CurrClients[str];
		if(tmp.curstate!=e_SYN_ACK)
			return false;
		VS_NHP_Container cont;
//		memcpy(tmp.ackdata.EndPoint,Pos,END_POINT_SIZE);
//		Pos+=END_POINT_SIZE;
//		counter+=END_POINT_SIZE;

		memcpy(&tmp.ackdata.IP,Pos,sizeof(tmp.ackdata.IP));
		Pos+=sizeof(tmp.ackdata.IP);
		counter+=sizeof(tmp.ackdata.IP);

		memcpy(&tmp.ackdata.port,Pos,sizeof(tmp.ackdata.port));
		Pos+=sizeof(tmp.ackdata.port);
		counter+=sizeof(tmp.ackdata.port);

		memcpy(&tmp.ackdata.ack_buf.length,Pos,sizeof(tmp.ackdata.ack_buf.length));
		Pos+=sizeof(tmp.ackdata.ack_buf.length);
		counter+=sizeof(tmp.ackdata.ack_buf.length);

		tmp.ackdata.ack_buf.buf = new unsigned char[tmp.ackdata.ack_buf.length];
		memcpy(tmp.ackdata.ack_buf.buf,Pos,tmp.ackdata.ack_buf.length);
		counter+=tmp.ackdata.ack_buf.length;

		cont.my_nat.ip = tmp.nat.ip;
		cont.my_nat.port = tmp.nat.port;
		cont.my_local.ip = tmp.ackdata.IP;
		cont.my_local.port = tmp.ackdata.port;
		delete [] tmp.ackdata.ack_buf.buf;
	//	std::string	strEndPoint = m_CurrClients[str].ackdata.EndPoint;
		m_ClientTable[str] = cont;

		m_CurrClients.erase(str);
		return ProcessingMessage((char*)&cont,sizeof(cont));
	}
}

bool VS_UdpStreamRouter_TestImp::ProcReqClientPack(char* data,int sz, unsigned long IP, unsigned short port)
{
	char			*Pos = data;
	NHP_PACK_TYPE	type;
	char			EndPoint[END_POINT_SIZE];
	char			senddatabuf[RESP_CLIENT_INFO_SIZE];
	if(REQ_CLIENT_INFO_SIZE!=sz)
		return false;
    memcpy(&type,Pos,sizeof(type));
	Pos+= sizeof(type);
	if(REQCLIENTINFO!=type)
		return false;
	memcpy(EndPoint,Pos,END_POINT_SIZE);
	std::string str = EndPoint;
	if(m_ClientTable.end() == m_ClientTable.find(str))
		return true;
	Pos = senddatabuf;
	type = RESPCLIENTINFO;
	memcpy(Pos,&type,sizeof(type));
	Pos += sizeof(type);

	memcpy(Pos,&m_ClientTable[str].my_nat.ip,sizeof(unsigned long));
	Pos+= sizeof(unsigned long);

	memcpy(Pos,&m_ClientTable[str].my_nat.port,sizeof(unsigned short));
	Pos+= sizeof(unsigned short);

	memcpy(Pos,&m_ClientTable[str].my_local.ip,sizeof(unsigned long));
	Pos+= sizeof(unsigned long);

	memcpy(Pos,&m_ClientTable[str].my_local.port,sizeof(unsigned short));
	bool b = m_conn->WriteTo(senddatabuf,RESP_CLIENT_INFO_SIZE,IP,port);
	int trans;
	unsigned long mills(10000);
	if(!b)
		return true;
	switch(WaitForSingleObject(writeEvent,mills))
	{
	case WAIT_OBJECT_0:
		trans = m_conn->GetWriteResult(mills);
		break;
	}
	return true;
}


int VS_UdpStreamRouter_TestImp::SendUdpFrameAct( const void *buffer, const int n_bytes,
							stream::Track track, unsigned long UID,unsigned long ip, unsigned short port)
{
	stream::UDPFrameHeader head;
	char* Pos;
	char* senddatabuf;
	int	ret(0);

	head.head_length = sizeof(head);
	head.version = 1;
	head.UID= UID;
	head.length = n_bytes;
	head.tick_count = GetTickCount();
	head.track = track;
	head.cksum = stream::GetFrameBodyChecksum(buffer, n_bytes);
	Pos = senddatabuf = new char[sizeof(stream::UDPFrameHeader)];
	memcpy(Pos,&head.length,sizeof(head.length));
	Pos += head.length;

	memcpy(Pos, &head.tick_count, sizeof(head.tick_count));
	Pos += sizeof(head.tick_count);

	memcpy(Pos,&head.track,sizeof(head.track));
	Pos+= sizeof(head.track);

	memcpy(Pos, &head.cksum,sizeof(head.cksum));
	Pos+= sizeof(head.cksum);

	memcpy(Pos, &head.head_length, sizeof(head.head_length));
	Pos += sizeof(head.head_length);

	memcpy(Pos,&head.version,sizeof(head.version));
	Pos+= sizeof(head.version);

	memcpy(Pos,&head.UID,sizeof(head.UID));
	Pos+= sizeof(head.UID);

	memcpy(Pos,buffer,n_bytes);

	if(!m_conn->WriteTo(senddatabuf, n_bytes + sizeof(stream::UDPFrameHeader), ip, port))
		ret = -1;
	else
		ret = n_bytes;
	delete [] senddatabuf;
	return ret;
	//VS_Buffer buffers[] = {	{head.length, (void*)&head},
	//					{(unsigned long)n_bytes,(void*)buffer}};
 //   if(!m_conn->RWrite(buffers,2)) return -1;

	return 0;

}

//bool VS_UdpStreamRouter_TestImp::SendSynAck(StateClientInfoType*  cl)
//{
//	int err;
//	if(e_SYN!=cl->curstate)
//		return false;
//	NHPHandShakePacket senddata;
//	SYN_ACK_TYPE synackdata;
//
//	if(!NHPTransformSYN(&cl->syndata,&synackdata,&err))
//		return false;
//	senddata.type = SYN_ACK;
//	memcpy(&senddata.synackdata,&synackdata,sizeof(synackdata));
//	int res = m_conn->SendTo(&senddata,sizeof(senddata),cl->nat.ip,cl->nat.port);
//	switch(WaitForSingleObject(writeEvent,10000))
//	{
//	case WAIT_OBJECT_0:
//		memcpy(&cl->synackdata,&synackdata,sizeof(synackdata));
//		cl->curstate = e_SYN_ACK;
//		return true;
//		break;
//	default:
//		return false;
//	}
//}
bool VS_UdpStreamRouter_TestImp::AddHandler( VS_USR_SomeHandler  * handler )
{
	if (!handle_server || m_flag < e_init) return false;
	if (!handle_server->AddHandler( handler ))
		return false;
	if (m_flag!=e_add_handler)
		m_flag |= e_s_add_handler;
	return true;
}
//end VS_UdpStreamRouter_TestImp::
///////////////////////////////////////////////////////////////////////////////
bool VS_UdpStreamRouter_TestImp::ProcessingMessage( char * data,int sz)
{
	if (sz<sizeof(VS_NHP_Container)) return true;
	int offset = sz - sizeof(VS_NHP_Container);
	VS_NHP_Container * res =
		reinterpret_cast<VS_NHP_Container*>( (char *)(data + offset) );
	res->Show();

	return true;
}
//end VS_UdpStreamRouter_TestImp::
///////////////////////////////////////////////////////////////////////////////

bool VS_UdpStreamRouter_TestImp::PeriodicTick()
{
	printf(".");
	return true;
}
//end VS_UdpStreamRouter_TestImp::
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
VS_UdpStreamRouter::VS_UdpStreamRouter()
:imp(0)
{
}
//end VS_UdpStreamRouter::
///////////////////////////////////////////////////////////////////////////////
VS_UdpStreamRouter::~VS_UdpStreamRouter()
{
	if(imp)
		delete imp;
	//handle_server->RemoveAllHandlers();
}
//end VS_UdpStreamRouter::
///////////////////////////////////////////////////////////////////////////////
HANDLE VS_UdpStreamRouter::Start( const char * endpoint_name )
{
	if (imp) return imp->Start( endpoint_name );
	return false;
}
//end VS_UdpStreamRouter::Start
///////////////////////////////////////////////////////////////////////////////
bool VS_UdpStreamRouter::Init(VS_UdpStreamRouter_implementation * imp_, char * bind_host, unsigned short port)
{
	if (imp || !imp_) return false;
	imp = imp_;
	if (imp) return imp->Init(port,bind_host);
	return false;
}
//end VS_UdpStreamRouter::
///////////////////////////////////////////////////////////////////////////////
bool VS_UdpStreamRouter::Init(VS_UdpStreamRouter_implementation *imp_,char *Endpoint, bool try_all_ip)
{
	bool res(false);
	if(imp || !imp_)
		return false;
	imp = imp_;
	const unsigned number = net::endpoint::GetCountAcceptTCP(Endpoint, false);
	if (number) {		// IPs from Registry
		for(unsigned int i = 1;i<=number;i++)
		{
			auto rat = net::endpoint::ReadAcceptTCP(i, Endpoint, false);
			if(!rat)
				continue;
			if(imp->IsValid())
				res =  OpenPort(rat->port, rat->host.c_str()) || res;
			else
				res = res || imp->Init(rat->port, rat->host.c_str());
		}
	} else if (try_all_ip) {		// Automatically detect ANY_IP
		char host[512] = {0};
		if (!VS_GetDefaultHostName(host, 512))
			return false;

		const int net_interface = 5;
		char *ips_my[net_interface] = {0};
		unsigned ips_count = 0;
		for(int i =0;i<net_interface;i++)
			ips_my[i] = new char[64];

		if(!(ips_count = VS_GetHostsByName(host,ips_my,net_interface,64)))
		{
			for(int i =0;i<net_interface;i++)
				delete [] ips_my[i];
			return false;
		}

		for(unsigned int i=0; i < ips_count; i++)
			if(imp->IsValid())
				res =  OpenPort(4307,ips_my[i])||res;
			else
				res = res || imp->Init(4307,ips_my[i]);

		for(int i =0;i<net_interface;i++)
			delete [] ips_my[i];
	}
	return res;
}
bool VS_UdpStreamRouter::OpenPort(const unsigned short port,const char *bind_port)
{
	return imp? imp->OpenPort(port,bind_port) : false;
}
bool VS_UdpStreamRouter::Stop( void )
{
	if (imp) return imp->Stop();
	return false;
}
//end VS_UdpStreamRouter::Stop
///////////////////////////////////////////////////////////////////////////////

//end VS_UdpStreamRouter::
///////////////////////////////////////////////////////////////////////////////

//end VS_UdpStreamRouter::
///////////////////////////////////////////////////////////////////////////////
VS_USR_SomeHandler::Names_it & VS_USR_SomeHandler::FindName(
	const char * name , unsigned int &index_out)
{
	index_out = ~0; nit = names.end();
	if (names.empty()) return nit;

	index_out = 0;

	for(nit = names.begin();
		nit!= names.end();
		nit++,index_out++)
	{
		if( _stricmp((*nit).c_str(), name)==0 )
		{
			return nit;
		}
	}
	index_out = ~0;
	return nit;

}
//end VS_USR_SomeHandler::FindName
///////////////////////////////////////////////////////////////////////////////
bool VS_USR_SomeHandler::RunCallBack(const char * name ,
									 const VS_SomeData * data_in )
{
	if (m_ptr_container.empty() ||
		!name					||
		names.empty())
			return false;
	unsigned int index = ~0;

	nit = FindName( name , index );

	if( nit== names.end()) return false;

	it = m_ptr_container.find( index );

	if(it==m_ptr_container.end())
	{
		return false;
	}
	(this->*(it->second))( data_in );
	return true;
}
VS_USR_SomeHandler::VS_USR_SomeHandler()
{
	AddCallBack( "EventOne" , &VS_USR_SomeHandler::SetDataEvent );
}
//end VS_USR_SomeHandler::VS_USR_SomeHandler
///////////////////////////////////////////////////////////////////////////////
VS_USR_SomeHandler::~VS_USR_SomeHandler()
{
	if(!m_ptr_container.empty())
		m_ptr_container.clear();
}
//end VS_USR_SomeHandler::~VS_USR_SomeHandler
///////////////////////////////////////////////////////////////////////////////
bool VS_USR_SomeHandler::GetHandlerEventsName
	( std::vector<std::string> *& names_out )
{
	names_out = &names;
	return true;
}
//end VS_USR_SomeHandler::
///////////////////////////////////////////////////////////////////////////////
void VS_USR_SomeHandler::AddCallBack( const char * name , TYPE_CallBackMember ptr )
{
	std::string str( name );
	m_ptr_container[ static_cast<unsigned long>(names.size()) ] = ptr;
	names.insert( names.end() , str );
}
//end VS_USR_SomeHandler::AddCallBack
///////////////////////////////////////////////////////////////////////////////
VS_USR_DetermineIpHandler::VS_USR_DetermineIpHandler()
{
}
// end VS_USR_DetermineIpHandler::VS_USR_DetermineIpHandler
///////////////////////////////////////////////////////////////////////////////
VS_USR_DetermineIpHandler::~VS_USR_DetermineIpHandler()
{
}
// end VS_USR_DetermineIpHandler::~VS_USR_DetermineIpHandler
///////////////////////////////////////////////////////////////////////////////
void VS_USR_DetermineIpHandler::SetDataEvent(const VS_SomeData * data_in )
{
	const VS_EndpointInformationData * edata =
			static_cast< const VS_EndpointInformationData* >( data_in );
	ReturnEndpointInformation_Event( edata->endpoint_name.c_str(),
									edata->ip_nat,
									edata->port_nat,
									edata->ip_local,
									edata->port_local);
}
//end VS_USR_DetermineIpHandler::SetDataEvent
///////////////////////////////////////////////////////////////////////////////
VS_USR_SomeHandlersServer::VS_USR_SomeHandlersServer()
:current_index(0)
{
}
//end VS_USR_SomeHandlersServer::
///////////////////////////////////////////////////////////////////////////////
VS_USR_SomeHandlersServer::~VS_USR_SomeHandlersServer()
{
}
//end VS_USR_SomeHandlersServer::
///////////////////////////////////////////////////////////////////////////////

bool VS_USR_SomeHandlersServer::AddHandler( VS_USR_SomeHandler  * handler,
										   unsigned int * my_index_out)
{
	handles_container[ current_index ] = handler;

	if (!my_index_out)
		return PlusOneHandle();
	else
		*my_index_out = current_index;

	return PlusOneHandle();
}
//end VS_USR_SomeHandlersServer::
///////////////////////////////////////////////////////////////////////////////

bool VS_USR_SomeHandlersServer::RemoveHandler( VS_USR_SomeHandler * handler )
{
	if (handles_container.empty()) return true;
	it = handles_container.begin();
	do
	{
		if( it->second == handler )
		{
			handles_container.erase( it );
			if (handles_container.empty()) return true;
			it = handles_container.begin();
		} else it++;

	}while(it!= handles_container.end());
	return true;
}
//end VS_USR_SomeHandlersServer::
///////////////////////////////////////////////////////////////////////////////

bool VS_USR_SomeHandlersServer::RemoveAllHandlers( void )
{
	if (handles_container.empty()) return true;
	handles_container.clear();
	return true;
}
//end VS_USR_SomeHandlersServer::
///////////////////////////////////////////////////////////////////////////////
unsigned int VS_USR_SomeHandlersServer::
	GetIndex( VS_USR_SomeHandler * handler )
{
	if (handles_container.empty()) return ~0;

	for (it = handles_container.begin();
		it!= handles_container.end();
		it++)
	{
		if ((*it).second==handler)
			return (*it).first;
	}
	return false;
}
//bool VS_USR_SomeHandlersServer::SetCustomEvent( const char * name , const VS_SomeData * data)
//{
//	return false;
//}
////end VS_USR_SomeHandlersServer::
///////////////////////////////////////////////////////////////////////////////
bool VS_USR_SomeHandlersServer::PlusOneHandle()
{
	if( current_index>= ~0 ) current_index =0;
	else current_index++;
	return true;
}
//end VS_USR_SomeHandlersServer::
///////////////////////////////////////////////////////////////////////////////
VS_USR_SomeName::VS_USR_SomeName()
{
}
//end VS_USR_SomeName::
///////////////////////////////////////////////////////////////////////////////
VS_USR_SomeName::VS_USR_SomeName(const char * name_in)
:name( name_in )
{

}
//end VS_USR_SomeName::
///////////////////////////////////////////////////////////////////////////////
VS_USR_SomeName::~VS_USR_SomeName()
{
	name.clear();
}
//end VS_USR_SomeName::
///////////////////////////////////////////////////////////////////////////////
bool VS_USR_SomeName::SetName(const char * name_in)
{
	if(!name_in) return false;
	name.clear();
	name = std::string( name_in );
	return true;
}
//end VS_USR_SomeName::
///////////////////////////////////////////////////////////////////////////////
bool VS_USR_SomeName::GetAllIndexes( const std::vector<unsigned int> *& list ) const
{
	list = &indexes;
	return true;
}
//end VS_USR_SomeName::
///////////////////////////////////////////////////////////////////////////////

bool VS_USR_SomeName::RemoveIndex( unsigned int index )
{
	if(!FindIndex( index , it )) return false;
	indexes.erase( it );
	return true;
}
//end VS_USR_SomeName::
///////////////////////////////////////////////////////////////////////////////

bool VS_USR_SomeName::RemoveAllIndexes( void )
{
	if (indexes.empty()) return true;
	indexes.clear();
	return true;
}
//end VS_USR_SomeName::
///////////////////////////////////////////////////////////////////////////////
bool VS_USR_SomeName::AddIndex( unsigned int index )
{
	if(FindIndex( index , it )) return true;///No duplicates!

	indexes.insert( indexes.end() , index );
	return true;
}
//end VS_USR_SomeName::
///////////////////////////////////////////////////////////////////////////////
bool VS_USR_SomeName::FindIndex( unsigned int index,
								TYPE_Iterator & it_out)
{
	if (indexes.empty())
	{
		it_out = indexes.end();
		return false;
	}

	for(	it = indexes.begin();
			it!= indexes.end();
			it++)
	{
		if (*it == index)
		{
			it_out = it;
			return true;
		}
	}
	it_out = indexes.end();
	return false;
}
//end VS_USR_SomeName::
///////////////////////////////////////////////////////////////////////////////
bool VS_USR_SomeNameServer::AddName( const char * name ,
						unsigned int * my_index_in_out )
{
	if (!my_index_in_out) return false;

	unsigned int index = ~0;

	if (IsGenerateCondition())
	{
		unsigned int index = GenerateIndex();
	}
	else
	{
		index = *my_index_in_out;
	}

	if (!FindName( name , it ))
	{

		VS_USR_SomeName * tmp = 0;

		tmp = new VS_USR_SomeName( name );

		it = names_container.insert( names_container.end(), tmp );
	}

	return (*it)->AddIndex( index );;
}
// end VS_USR_SomeNameServer
///////////////////////////////////////////////////////////////////////////////
bool VS_USR_SomeNameServer::FindName( const char * name ,
						 TYPE_Names_Iterator & it_out)
{
	it_out = names_container.end();

	if (names_container.empty()) return false;

	for (it=names_container.begin();
		 it!=names_container.end();
		 ++it)
	{
		if (_stricmp( (*it)->GetName(), name)==0)
		{
			it_out = it;
			return true;
		}
	}
	return false;
}
// end VS_USR_SomeNameServer::FindName
bool VS_USR_SomeNameServer::IsNameExist( const char * name )
{
	if (FindName( name , it )) return true;
	return false;
}
// end VS_USR_SomeNameServer
///////////////////////////////////////////////////////////////////////////////
unsigned int VS_USR_SomeNameServer::GenerateIndex( void )
{	return ~0;
}
// end VS_USR_SomeNameServer
bool VS_USR_SomeNameServer::IsGenerateCondition()
{	return false;
}
// end VS_USR_SomeNameServer
///////////////////////////////////////////////////////////////////////////////

bool VS_USR_SomeNameServer::RemoveName( const char * name )
{
	if (!FindName( name , it )) return false;
	VS_USR_SomeName * sname = (*it);
	delete ( sname );
	names_container.erase( it );
	return true;
}
// end VS_USR_SomeNameServer
///////////////////////////////////////////////////////////////////////////////

bool VS_USR_SomeNameServer::RemoveAllName( void )
{
	if (names_container.empty()) return false;

	for( it = names_container.begin();
		 it!= names_container.end();
		 it++)
	{
		VS_USR_SomeName * sname = (*it);
		delete ( sname );
	}
	names_container.clear();
	return true;
}
// end VS_USR_SomeNameServer
///////////////////////////////////////////////////////////////////////////////
bool VS_USR_SomeNameServer::GetIndexesByName(
						const char * name  ,
						const TYPE_Index *& indexes )
{
	if (!FindName( name , it )) return false;
	if(!((*it)->GetAllIndexes(indexes))) return false;
	return true;
}
// end VS_USR_SomeNameServer::
///////////////////////////////////////////////////////////////////////////////
bool VS_USR_SomeNameServer::RemoveIndex( unsigned int index )
{
	for (it=names_container.begin();
		 it!=names_container.end();
		 ++it)
	{
		(*it)->RemoveIndex( index );
	}
	return true;
}
// end VS_USR_SomeNameServer
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
VS_USR_HandleServer::VS_USR_HandleServer()
{
}
// end VS_USR_HandleServer::
///////////////////////////////////////////////////////////////////////////////
VS_USR_HandleServer::~VS_USR_HandleServer()
{
}
// end VS_USR_HandleServer::
///////////////////////////////////////////////////////////////////////////////
bool VS_USR_HandleServer::AddHandler( VS_USR_SomeHandler  * handler )
{
	unsigned int my_index=0;
	std::vector< std::string > * names;

	if (!VS_USR_SomeHandlersServer::AddHandler(
		handler ,
		&my_index ))
		return false;

	if (!handler->GetHandlerEventsName(names ))
		return false;
	for(nnit = names->begin();
		nnit!= names->end();
		nnit++)
	{
		if (!VS_USR_SomeNameServer::AddName(
			nnit->c_str() ,
			&my_index ))
		{
			///Totall remove this handle
			RemoveHandler( handler );
			return false;
		}
	}
	return true;
}
// end VS_USR_HandleServer::
///////////////////////////////////////////////////////////////////////////////
bool VS_USR_HandleServer::RemoveHandler( VS_USR_SomeHandler  * handler )
{
	unsigned int index = VS_USR_SomeHandlersServer::GetIndex( handler );

	if (index == ~0) return false;

	VS_USR_SomeNameServer::RemoveIndex( index );

	VS_USR_SomeHandlersServer::RemoveHandler( handler );

	return true;
}
// end VS_USR_HandleServer::
///////////////////////////////////////////////////////////////////////////////
bool VS_USR_HandleServer::SetCustomEvent(
	const char * name ,
	const VS_SomeData * data)
{
	TYPE_Index * indexes = 0;
	TYPE_Index::iterator iter;
	if (!VS_USR_SomeNameServer::GetIndexesByName( name , (const TYPE_Index *&)indexes ))
		return false;

	if (indexes->empty()) return false;
	bool ret = true;
	for( iter = indexes->begin();
		iter!= indexes->end();
		iter++)
	{
		ret &= RunByIndex( name , data , *iter );
	}
	return ret;
}
// end VS_USR_HandleServer::
///////////////////////////////////////////////////////////////////////////////
bool VS_USR_HandleServer::RunByIndex(	const char * name ,
										const VS_SomeData * data ,
										unsigned int index )
{
	if (handles_container.empty()) return false;
	VS_USR_SomeHandlersServer::TYPE_Handle_iterator iter;
	iter = handles_container.find( index );
	if (iter==handles_container.end()) return false;
	return iter->second->RunCallBack( name , data );
}
// end VS_USR_HandleServer::
///////////////////////////////////////////////////////////////////////////////

VS_UdpStreamRouter_Impl::VS_UdpStreamRouter_Impl():
	m_loop_th(0),stopEvent(0),m_count_wait_objects(0)
	{
		memset(m_conns,0,VS_UDP_ROUTER_MAX_PORTS*sizeof(m_conns[0]));
		memset(m_NHPHandshakeObjs,0,VS_UDP_ROUTER_MAX_PORTS*sizeof(m_NHPHandshakeObjs[0]));
		memset(m_hEventArr,0,MAXIMUM_WAIT_OBJECTS*sizeof(m_hEventArr[0]));
	}

VS_UdpStreamRouter_Impl::~VS_UdpStreamRouter_Impl()
{
	TerminateAll();
}

bool VS_UdpStreamRouter_Impl::Init(unsigned short port, const char* bind_host)
{
	char host[256];
	if(m_flag&e_s_init)
	{
		//TerminateAll();
		//return true;//?
		return false;
	}
	m_conns[0] = new VS_ConnectionUDP;
	m_NHPHandshakeObjs[0] = new VS_NHP_Handshake_Impl;
	m_NHPHandshakeObjs[0]->NHPInit(m_conns[0]);

	if(!m_conns[0]->CreateOvReadEvent() || !m_conns[0]->CreateOvWriteEvent())
		return false;

	stopEvent = CreateEvent(0,FALSE,FALSE,0);
	m_hEventArr[0] = stopEvent;
	m_hEventArr[1] = m_conns[0]->OvReadEvent();
	m_hEventArr[2] = m_conns[0]->OvWriteEvent();
	m_count_wait_objects = 3;

	if(bind_host)
	{
		strcpy(host,bind_host);
	}
	else
	{
		if(!VS_GetDefaultHostName( host , 256 ))
		{
			TerminateAll();
			return false;
		}
	}
	if(!m_conns[0]->Bind(host,port,true))
	{
		TerminateAll();
		return false;
	}
	dprint3("Bind host = %s, bind port = %d\n",host,port);
	m_port = port;
	m_flag |=e_s_init;
	return true;
}

bool VS_UdpStreamRouter_Impl::Init()
{
	return Init(4567,"0.0.0.0");
}
bool VS_UdpStreamRouter_Impl::OpenPort(const unsigned short port, const char *bind_host)
{
	const char *host = 0;
	if((!port) || !(m_flag & e_s_init) || (m_flag & e_s_loop_start))
		return false;
	int i = 0;
	for(i = 0;i<VS_UDP_ROUTER_MAX_PORTS;i++)
	{
		if(!m_conns[i])
			break;
	}
	if(i >= VS_UDP_ROUTER_MAX_PORTS)
		return false;
	m_conns[i] = new VS_ConnectionUDP;
	if(!m_conns[i]->CreateOvReadEvent() || !m_conns[i]->CreateOvWriteEvent())
	{
		delete m_conns[i];
		m_conns[i] = 0;
		return false;
	}
	if(!bind_host)
		host = m_conns[0]->GetBindIp();
	else
		host = bind_host;
	if(!m_conns[i]->Bind(host,port,true))
	{
		delete m_conns[i];
		m_conns[i] = 0;
		dprint3("Bind host = %s, bind port = %d FAILED!!!\n",host,port);
		return false;
	}
	dprint3("Bind host = %s, bind port = %d\n",host,port);

	m_hEventArr[i*2 + 1] = m_conns[i]->OvReadEvent();
	m_hEventArr[i*2 + 2] = m_conns[i]->OvWriteEvent();
	m_count_wait_objects += 2;
	m_NHPHandshakeObjs[i] = new VS_NHP_Handshake_Impl;
	m_NHPHandshakeObjs[i]->NHPInit(m_conns[i]);

	return true;
}

bool VS_UdpStreamRouter_Impl::IsValid()
{
	return (m_flag & e_s_init);
}

HANDLE VS_UdpStreamRouter_Impl::Start(const char* endpoint_name)
{
	if(!(m_flag&e_s_init))	return 0;
	HANDLE th = (HANDLE)_beginthread( Thread , 0 , (void *)this );
	m_loop_th = th;
	return th;
}

void VS_UdpStreamRouter_Impl::Thread(void * arg)
{
	vs::SetThreadName("UDPSR");
	VS_UdpStreamRouter_Impl* pThis = (VS_UdpStreamRouter_Impl*)arg;
	pThis->Loop();
}

void VS_UdpStreamRouter_Impl::Loop()
{
	int i = 0;
	if(!m_conns[0])
		return;
	unsigned long mills(5000);
	char *rbuf[VS_UDP_ROUTER_MAX_PORTS];
	char *AddrFrom[VS_UDP_ROUTER_MAX_PORTS];
	unsigned long *pIPs[VS_UDP_ROUTER_MAX_PORTS];
	unsigned short *pPorts[VS_UDP_ROUTER_MAX_PORTS];

	memset(rbuf,0,VS_UDP_ROUTER_MAX_PORTS*sizeof(rbuf[0]));
	memset(AddrFrom,0,VS_UDP_ROUTER_MAX_PORTS*sizeof(AddrFrom[0]));
	memset(pIPs,0,VS_UDP_ROUTER_MAX_PORTS*sizeof(pIPs[0]));
	memset(pPorts,0,VS_UDP_ROUTER_MAX_PORTS*sizeof(pPorts[0]));

	for(i = 0;m_conns[i];i++)
	{
		rbuf[i] = new char[65536];
		AddrFrom[i] = new char[16];
	}
	m_flag |= e_s_loop_start;
	bool IsLoop = true;
	int Trans;
	DWORD dwRes;
	for(i=0;m_conns[i];i++)
	{
		if(!m_conns[i]->AsynchReceiveFrom(rbuf[i],65536,AddrFrom[i],pIPs[i],pPorts[i]))
			if(GetLastError() == WSAECONNRESET )
				i--;
	}
	while(IsLoop)
	{
		dwRes = WaitForMultipleObjects(m_count_wait_objects,m_hEventArr,FALSE,mills);
		VerifyClients();
		switch(dwRes)
		{
		case WAIT_OBJECT_0:
			IsLoop = false;
			break;
		case WAIT_ABANDONED:
			break;
		case WAIT_TIMEOUT:
			break;
		default:
			if(dwRes<MAXIMUM_WAIT_OBJECTS + WAIT_OBJECT_0)
			{
				unsigned int event_index = dwRes - WAIT_OBJECT_0;
				unsigned int conn_index = (event_index - 1)/2;
				if(m_hEventArr[event_index] == m_conns[conn_index]->OvReadEvent())
				{
					Trans = m_conns[conn_index]->GetReadResult(mills,0,true);
					mills = 5000;
					if(Trans>0)
						Respond(rbuf[conn_index], Trans, vs_htonl(*pIPs[conn_index]), vs_htons(*pPorts[conn_index]), conn_index);
					unsigned long err(0);
					bool		res(true);
					do
					{
						if(!(res = m_conns[conn_index]->AsynchReceiveFrom(rbuf[conn_index],65536,AddrFrom[conn_index],pIPs[conn_index],pPorts[conn_index])))
							err = GetLastError();
					}while(!res && err == WSAECONNRESET );
				}
				else if(m_hEventArr[event_index] == m_conns[conn_index]->OvWriteEvent())
				{
					Trans = m_conns[conn_index]->GetWriteResult(mills);
					mills = 5000;
				}
			}
		}
	}
	for(i=0;m_conns[i];i++)
	{
		m_conns[i]->Close();
		delete[] rbuf[i];
		delete[] AddrFrom[i];
	}
	m_flag &= ~e_s_loop_start;
}
void VS_UdpStreamRouter_Impl::Respond(const char* ReceiveBuf,const unsigned long sz, const unsigned long ip, const unsigned short port, const unsigned long conn_index)
{
	char* respond(0);
	unsigned long	length(0);
	length = PrepareRespond(ReceiveBuf,sz,ip,port,conn_index,respond);
	if(!length)
		return;
	SendRespond(respond,length,ip,port,conn_index);
	delete [] respond;
}
unsigned long VS_UdpStreamRouter_Impl::PrepareRespond(const char* ReceivedBuf,const unsigned long sz,
													  const unsigned long ip,const unsigned short port,const unsigned long conn_index,char *&RespondData)
{
	ClientInfoType* pClInfo = GetClient(ip,port);
	SYN_TYPE			syndata;
	ACK_TYPE			ackdata;
	unsigned char *c = (unsigned char*)&ip;
	if(!pClInfo)
	{
		if(!m_NHPHandshakeObjs[conn_index]->IsSynData(ReceivedBuf,sz))
			return 0;
		else
		{
			RespondData = new char[SYN_ACK_FRAME_SIZE];

			if(!m_NHPHandshakeObjs[conn_index]->GetSynFromBuf(ReceivedBuf,sz,syndata))
			{
				delete [] RespondData;
				return 0;
			}

			dprint3("SYN was received from %d.%d.%d.%d:%d\n",c[3],c[2],c[1],c[0],port);
			m_NHPHandshakeObjs[conn_index]->PrepareSynAckBuf(syndata,RespondData);
			NewClient(ip,port,syndata,conn_index);
			return SYN_ACK_FRAME_SIZE;
		}
	}
	else
	{
		if(m_NHPHandshakeObjs[conn_index]->IsSynData(ReceivedBuf,sz))
		{
			dprint3("SYN was received from %d.%d.%d.%d:%d\n",c[3],c[2],c[1],c[0],port);
			RespondData = new char[SYN_ACK_FRAME_SIZE];

			if(!m_NHPHandshakeObjs[conn_index]->GetSynFromBuf(ReceivedBuf,sz,syndata))
			{
				delete [] RespondData;
				return 0;
			}
			m_NHPHandshakeObjs[conn_index]->PrepareSynAckBuf(syndata,RespondData);
			dprint3("SYN_ACK was sent to %d.%d.%d.%d:%d\n",c[3],c[2],c[1],c[0],port);
			return SYN_ACK_FRAME_SIZE;
		}
		else if(m_NHPHandshakeObjs[conn_index]->IsAckData(ReceivedBuf,sz))
		{
			if(!m_NHPHandshakeObjs[conn_index]->GetAckFromBuf(ReceivedBuf,sz,ackdata))
				return 0;
			dprint3("Ack was received from %d.%d.%d.%d:%d\n",c[3],c[2],c[1],c[0],port);
			pClInfo->local_IP = ackdata.IP;
			pClInfo->local_port = ackdata.port;
			NotifyClients(pClInfo,ackdata);
			return 0;
		}
		else
			return 0;
	}
}
void VS_UdpStreamRouter_Impl::VerifyClients()
{
	WaitingClientsTableType::iterator iter;
	std::list<std::string>	keylst;
	std::list<std::string>::iterator lstiter;
	for(iter = m_ClientsTable.begin();iter!=m_ClientsTable.end();iter++)
	{
		if(!ClientIsActual(&iter->second))
			keylst.push_back(iter->first);
	}
	for(lstiter = keylst.begin();lstiter!=keylst.end();lstiter++)
		m_ClientsTable.erase(*lstiter);
}

void VS_UdpStreamRouter_Impl::NotifyClients(ClientInfoType* pClient, const ACK_TYPE &ackdata)
{
	unsigned long	timeout(5000);
	WaitingClientsTableType::iterator iter;
	unsigned long	local_ip, peer_local_ip;
	unsigned short	local_port, peer_local_port;
	char	ClientEP[VS_ACS_MAX_SIZE_ENDPOINT_NAME+1],
			peerClientEP[VS_ACS_MAX_SIZE_ENDPOINT_NAME+1],
			ClientEP1[VS_ACS_MAX_SIZE_ENDPOINT_NAME+1],
			peerClientEP1[VS_ACS_MAX_SIZE_ENDPOINT_NAME+1],
			ConferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME+1],
			peerConferenceName[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME+1];
	char	*sendbuf4peer,* sendbuf4client;

	unsigned long len1(0),len2(0);
	unsigned char *c;

/*unsigned long tmp_ip = 0xC0A83DA4;


unsigned short port1 = 4545;
unsigned short port2 = 4646;*/



	if(pClient->ackdata.ack_buf.length)
		delete [] pClient->ackdata.ack_buf.buf;
	memset(&pClient->ackdata,0,sizeof(ACK_TYPE));
	pClient->ackdata = ackdata;
	if(!m_NHPHandshakeObjs[pClient->conn_index]->TransformACK(pClient->ackdata,ClientEP,peerClientEP,
		ConferenceName,local_ip,local_port))
		return;
	c = (unsigned char*)&pClient->IP;
	dprint3("Client was Handshaked Ip=%d.%d.%d.%d port = %d\n",c[3],c[2],c[1],c[0],pClient->port);
	for(iter = m_ClientsTable.begin();iter != m_ClientsTable.end();iter++)
	{
		if(!m_NHPHandshakeObjs[iter->second.conn_index]->TransformACK(iter->second.ackdata,ClientEP1,
							peerClientEP1,peerConferenceName,peer_local_ip,peer_local_port))
			continue;
		if(( !strcmp(ClientEP,peerClientEP1))	&&
			(!strcmp(peerClientEP,ClientEP1))	&&
			(!strcmp(ConferenceName,peerConferenceName)))
		{
			if(!(sendbuf4peer = m_NHPHandshakeObjs[pClient->conn_index]->PreparePUSHBuf(iter->second.ackdata,pClient->IP,pClient->port,pClient->local_IP,pClient->local_port,0,len1)))
				return;
			if(!(sendbuf4client = m_NHPHandshakeObjs[pClient->conn_index]->PreparePUSHBuf(pClient->ackdata,iter->second.IP,iter->second.port,iter->second.local_IP,iter->second.local_port,1,len2)))
				return;
			SendRespond(sendbuf4peer,len1,iter->second.IP,iter->second.port,iter->second.conn_index);
			delete [] sendbuf4peer;
			c = (unsigned char*)&iter->second.IP;
			dprint3("Push was sent to %d.%d.%d.%d:%d\n",c[3],c[2],c[1],c[0],iter->second.port);
			SendRespond(sendbuf4client,len2,pClient->IP,pClient->port,pClient->conn_index);
			delete [] sendbuf4client;
			c = (unsigned char*)&pClient->IP;
			dprint3("Push was sent to %d.%d.%d.%d:%d\n",c[3],c[2],c[1],c[0],pClient->port);
			DeactiveClient(pClient);
			DeactiveClient(iter->second.IP,iter->second.port);
			return;
		}
	}
}
void VS_UdpStreamRouter_Impl::DeactiveClient(ClientInfoType *pClient)
{
	pClient->getting_syn_time = 0;
}
void VS_UdpStreamRouter_Impl::DeactiveClient(unsigned long ip, unsigned short port)
{
	DeactiveClient(GetClient(ip,port));
}
bool VS_UdpStreamRouter_Impl::ClientIsActual(const ClientInfoType *pClient)
{
	unsigned long tick = GetTickCount();
	return tick - pClient->getting_syn_time < (unsigned long)pClient->syndata.field1;
}
bool VS_UdpStreamRouter_Impl::ClientIsActual(unsigned long ip,unsigned short port)
{
	return ClientIsActual(GetClient(ip,port));
}
void VS_UdpStreamRouter_Impl::NewClient(const unsigned long ip,const unsigned short port, const SYN_TYPE& syndata,const unsigned long conn_index)
{
	ClientInfoType	tmp;
	memset(&tmp,0,sizeof(tmp));
	tmp.IP = ip;//NAT's IP
	tmp.port = port;//NAT's port
	tmp.local_IP = 0;
	tmp.local_port = 0;
	tmp.syndata = syndata;
	tmp.getting_syn_time = GetTickCount();
	tmp.conn_index = conn_index;
	char chip[9];
	char chport[10];
	_itoa(ip,chip,16);
	_itoa(port,chport,10);
	std::string str = (std::string)chip + (std::string)chport;
	m_ClientsTable[str] = tmp;
}
void VS_UdpStreamRouter_Impl::SendRespond(const char* senddata, unsigned long sz,
										  unsigned long ip, unsigned short port,const unsigned long conn_index)
{
	if(!IsValid())
		return;
	if(!m_conns[conn_index]->WriteTo(senddata,sz,ip,port))
		return;
	unsigned long mills = 10000;
	int res = m_conns[conn_index]->GetWriteResult(mills);
}

ClientInfoType* VS_UdpStreamRouter_Impl::GetClient(unsigned long ip,unsigned short port)
{
	char chip[9];
	char chport[10];
	_itoa(ip,chip,16);
	_itoa(port,chport,10);
	std::string	str = (std::string)chip + (std::string)chport;
	if(m_ClientsTable.end() == m_ClientsTable.find(str))
		return 0;
	return &m_ClientsTable[str];
}
void VS_UdpStreamRouter_Impl::DelClient(unsigned long ip,unsigned short port)
{
	char chip[9];
	char chport[10];
	_itoa(ip,chip,16);
	_itoa(port,chport,10);
	std::string str = (std::string)chip + (std::string)chport;
	m_ClientsTable.erase(str);
}
void VS_UdpStreamRouter_Impl::ProcAck(unsigned long ip,unsigned short port, ACK_TYPE ackdata)
{
	VS_EndpointInformationData mess;
	mess.ip_nat = ip;
	mess.port_nat = port;
	mess.ip_local= ackdata.IP;
	mess.port_local = ackdata.port;
}

bool VS_UdpStreamRouter_Impl::Stop()
{
	if(!stopEvent)	return false;
	SetEvent(stopEvent);
	while(m_flag & e_s_loop_start)
		Sleep(1);
	return true;
}

void VS_UdpStreamRouter_Impl::TerminateAll()
{
	unsigned long i= 0;
	while(m_conns[i])
	{
		delete m_conns[i];
		m_conns[i] = 0;
		delete m_NHPHandshakeObjs[i];
		m_NHPHandshakeObjs[i] = 0;
		i++;
	}
	if(stopEvent)
		CloseHandle(stopEvent);
	stopEvent = 0;
	m_count_wait_objects = 0;
}

#endif
