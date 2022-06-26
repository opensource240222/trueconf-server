#if defined(_WIN32) // Not ported yet

#include "VS_ConnectNHPExecutor.h"
#include "../NHP/VS_NHP_HandshakeHelper.h"
#include "../../net/EndpointRegistry.h"
#include "acs/Lib/VS_AcsLib.h"
#include "../../std/cpplib/VS_WorkThreadEvents.h"
#include "../../std/cpplib/VS_MessageHandlerAdapter.h"
#include "std-generic/cpplib/hton.h"
#include "transport/Client/VS_TransportClient.h"
#include "../../acs/connection/VS_ConnectionUDP.h"
#include "../../acs/connection/VS_IO_Operation_Type.h"
#include "../../acs/connection/VS_ConnectionOv.h"
#include <vector>

#include "VSClient/VS_Dmodule.h"

namespace
{
	enum
	{
		e_start,
		e_stop
	};

}

VS_ConnectNHPExecutor::VS_ConnectNHPExecutor(const char *ourEp, const char *ClEp, const char *SrvEp, const char *ConferenceName, const unsigned long timeout, const char *source_ip)
:m_ourEp(ourEp), m_clientEp(ClEp),m_srvEp(SrvEp), m_conferenceName(ConferenceName),m_source_ip(source_ip),m_timeout(timeout),m_finishBefore(),m_workerThread(new VS_WorkThreadEvents(true)),
m_readyConn(0),m_connected_ip(0),m_connected_port(0),m_UIDR(0),m_UIDS(0),m_connectionIsReady(false)
{
	m_messHandler = boost::signals2::deconstruct<VS_MessageHandlerAdapter>();
	m_messHandler->ConnectToHandleMessage(boost::bind(&VS_ConnectNHPExecutor::HandleMess,this,_1));
}
VS_ConnectNHPExecutor::~VS_ConnectNHPExecutor()
{}

void VS_ConnectNHPExecutor::MakeNHP()
{
	m_this = shared_from_this();
	m_workerThread->Start("NHPExecutor");
	m_workerThread->RegisterTimeout(m_this);
	m_ThreadTerminatedConn = m_workerThread->ConnectToThreadTerminated(boost::bind(&VS_ConnectNHPExecutor::ThreadTerminated,this));
	m_workerThread->Post(m_messHandler,boost::shared_ptr<VS_MessageData>(new VS_MessageData(e_start,0,0)));
}

void VS_ConnectNHPExecutor::ThreadTerminated()
{
	m_ThreadTerminatedConn.disconnect();
	m_this.reset();
}

void VS_ConnectNHPExecutor::HandleMess(const boost::shared_ptr<VS_MessageData> &mess)
{
	if(!mess)
		return;
	unsigned long type(0);
	unsigned long sz(0);
	mess->GetMessPointer(type,sz);
	switch(type)
	{
	case e_start:
		StartNHP();
		break;
	case e_stop:
		{
			if (m_nhp_conns.empty()) {
				DTRACE(VSTM_NETWORK, "HandleMess(e_stop); Stop Work Thread by m_nhp_conns.empty()");

				m_workerThread->Stop();
			}
			std::map<VS_ConnectionUDP*, ConnectionData> temp = m_nhp_conns;
			for(std::map<VS_ConnectionUDP*, ConnectionData>::iterator i = temp.begin();i!=temp.end();i++)
			{
				i->second.get<0>()->Finish();
				DeleteConn(i->first);
			}
		}
		break;

	}
}
void VS_ConnectNHPExecutor::Stop()
{
	m_workerThread->Post(m_messHandler,boost::shared_ptr<VS_MessageData>(new VS_MessageData(e_stop,0,0)));
	m_workerThread->WaitStopped();

}

void VS_ConnectNHPExecutor::Timeout()
{
	if(!CheckTime())
	{
		if (m_nhp_conns.empty()) {
			DTRACE(VSTM_NETWORK, "Timeout(); Stop Work Thread by m_nhp_conns.empty() and CheckTime()");
			m_workerThread->Stop();
		}
		std::map<VS_ConnectionUDP*, ConnectionData> temp = m_nhp_conns;
		for(std::map<VS_ConnectionUDP*, ConnectionData>::iterator i = temp.begin();i!=temp.end();i++)
		{
			i->second.get<0>()->Finish();
			DeleteConn(i->first);
		}
	}
}

void VS_ConnectNHPExecutor::StartNHP()
{
	m_finishBefore = std::chrono::steady_clock::now() + std::chrono::milliseconds(m_timeout);
	if(m_source_ip.empty())
	{
		char defaultGate[265] = {0};
		char my_host[256] = {0};
		long host_len(255);
		if(VS_GetTransportClientDefaultGate(defaultGate,255))
		{
			VS_GetEndpointSourceIP(defaultGate,my_host,host_len);
			m_source_ip = my_host;
		}
	}
	if (VS_GetIpv6ByHost(m_source_ip.c_str(), NULL)) {
		// Don't do NHP for ipv6
		return;
	}
	unsigned long bind_ip(0);
	if(!VS_GetIpByHostName(m_source_ip.c_str(),&bind_ip))
		return;


	DTRACE(VSTM_NETWORK, "StartNHP...");

	std::vector<std::pair<unsigned long,unsigned short>>	internal_addrs, external_addr;
	const unsigned n_ctcp = net::endpoint::GetCountConnectTCP(m_srvEp);
	for (unsigned i = 1; i <= n_ctcp; ++i)
	{
		auto reg_conn = net::endpoint::ReadConnectTCP(i, m_srvEp);
		if(!reg_conn)
			continue;
		unsigned long	host_ip(0);
		//check IP, we need external ip address
		//ip&255	== 10*256^3;				10.0.0.0/8
		//ip&4095	== 172*256^3 + 16*256^2;	172.16.0.0/12
		//ip&65535	== 192*256^3 + 168*256^2	192.168.0.0/16
		if (!VS_GetIpByHostName(reg_conn->host.c_str(), &host_ip))
			continue;
		if(((host_ip & VS_SUBNET_MASK_1) != VS_ADDRESS_SPACE_1)&&
		   ((host_ip & VS_SUBNET_MASK_2) != VS_ADDRESS_SPACE_2)&&
		   ((host_ip & VS_SUBNET_MASK_3) != VS_ADDRESS_SPACE_3))
			// external address
			external_addr.emplace_back(host_ip, reg_conn->port);
		else if(external_addr.empty())
			// save internal address
			internal_addrs.emplace_back(host_ip, reg_conn->port);
	}
	std::vector<std::pair<unsigned long,unsigned short>> &curr_vector = !external_addr.empty() ? external_addr : internal_addrs;
	for(std::vector<std::pair<unsigned long,unsigned short>>::iterator i = curr_vector.begin();i!=curr_vector.end();i++)
	{
		VS_ConnectionUDP * conn = new VS_ConnectionUDP;
		conn->Socket();
		unsigned short bind_port = 49152;//start dynamic port
		while(!conn->Bind(bind_ip,bind_port,false) && bind_port<0xffff)
			bind_port++;
		if(!conn->CreateOvReadEvent()||!conn->CreateOvWriteEvent())
		{
			delete conn;
			continue;
		}

		boost::shared_ptr<VS_NHP_HandshakeHelper> helper(new VS_NHP_HandshakeHelper(i->first,i->second,bind_ip,bind_port, m_timeout, m_ourEp.c_str(),m_clientEp.c_str(),m_conferenceName.c_str(),boost::bind(&VS_ConnectNHPExecutor::ServerFound,this,conn),
				boost::bind(&VS_ConnectNHPExecutor::ConnectionReady,this,conn,_1,_2,_3,_4)));
		boost::shared_ptr<std::vector<unsigned char>> sndBuf(new std::vector<unsigned char>);
		boost::shared_ptr<std::vector<unsigned char>> recvBuf(new std::vector<unsigned char>);
		boost::shared_ptr<boost::array<char,16>>	addrFrom(new boost::array<char,16>);
		sndBuf->resize(0xffff);
		recvBuf->resize(0xffff);
		m_nhp_conns[conn] = ConnectionData(helper,sndBuf,recvBuf,addrFrom);
		conn->SetIOHandler( this );
		conn->SetOvReadFields( e_operation_read, reinterpret_cast<VS_ACS_Field>(conn));
		conn->SetOvWriteFields( e_operation_write,reinterpret_cast<VS_ACS_Field>(conn));
		m_workerThread->SetHandledConnection(conn);
		unsigned long sz = sndBuf->size();
		unsigned long ip_to(0);
		unsigned short port_to(0);
		if(helper->GetBufForSend(ip_to, port_to, &(*sndBuf)[0],sz))
		{
			if(!conn->WriteTo(&(*sndBuf)[0],sz,ip_to,port_to))
			{
				DeleteConn(conn);
				continue;
			}
		}
		if(!conn->AsynchReceiveFrom(&(*recvBuf)[0],recvBuf->size(),&(*addrFrom)[0],m_nhp_conns[conn].get<4>(),m_nhp_conns[conn].get<5>()))
			DeleteConn(conn);
	}
}


void VS_ConnectNHPExecutor::Handle(const unsigned long sz, const struct VS_Overlapped *ov)
{
	if(!ov)
		return;
	VS_ConnectionUDP * c = reinterpret_cast<VS_ConnectionUDP*>(ov->field2);
	unsigned long mills(100);
	int res(-1);
	if(!c)
		return;
	std::map<VS_ConnectionUDP*, ConnectionData>::iterator iter = m_nhp_conns.find(c);
	DTRACE(VSTM_NETWORK, "%s for conn = 0x%x", ov->field1 == e_operation_read ? "Handle Read" : ov->field1 == e_operation_write ? " Handle Write" : "Handle unexpected code", c);
	if(iter==m_nhp_conns.end())
	{
		DTRACE(VSTM_NETWORK, "VS_ConnectNHPExecutor::Handle(...); connection is not found in map. Delete it. operation = %d", ov->field1);
		DeleteConn(c);
		return;
	}
	switch(ov->field1)
	{
	case e_operation_read:
		res = c->GetReadResult(mills,0,true);
		if(res<0 || !iter->second.get<0>()->SetRecvBuf(vs_htonl(*iter->second.get<4>()), vs_htons(*iter->second.get<5>()), &(*iter->second.get<2>())[0], res))
		{
			if(res==-1&&GetLastError()!=ERROR_PORT_UNREACHABLE)
			{
				DTRACE(VSTM_NETWORK, "VS_ConnectNHPExecutor::Handle(...) GetReadResult failed! err = %d. Delete conn", GetLastError());
				DeleteConn(c);
				return;
			}
		}
		break;
	case e_operation_write:
		res = c->GetWriteResult(mills);
		if(res<0)
		{
			DTRACE(VSTM_NETWORK, "VS_ConnectNHPExecutor::Handle(...) c->GetWriteResult(mills); return %d; GetLastErr = %d", res,GetLastError());
			DeleteConn(c);
			return;
		}
		break;
	}
	if (m_nhp_conns.find(c) == m_nhp_conns.end())
		return;
	bool isValid(c->State() != vs_sock_state_not_created);
	if(isValid && !c->IsRead()&&!iter->second.get<0>()->IsFinished())
	{
		boost::shared_ptr<std::vector<unsigned char>> rcvBuf = iter->second.get<2>();
		boost::shared_ptr<boost::array<char,16>> addrFrom = iter->second.get<3>();
		do{
			isValid = c->AsynchReceiveFrom(&(*rcvBuf)[0],rcvBuf->size(),&(*addrFrom)[0],iter->second.get<4>(),iter->second.get<5>());
		}while(!isValid&&GetLastError() == WSAECONNRESET);
		if (!isValid)
			DTRACE(VSTM_NETWORK, "c->AsynchReceiveFrom(...) failed; GetLastError = %d", GetLastError());
	}
	unsigned long buf_sz(0xffff);
	boost::shared_ptr<std::vector<unsigned char>> sndBuf = iter->second.get<1>();
	unsigned long ip_to(0);
	unsigned short port_to(0);
	if (isValid && !c->IsWrite() && iter->second.get<0>()->GetBufForSend(ip_to, port_to, &(*sndBuf)[0], buf_sz))
	{
		isValid = c->WriteTo(&(*sndBuf)[0], buf_sz, ip_to, port_to);
		if (!isValid)
			DTRACE(VSTM_NETWORK, "c->WriteTo failed; GetLastError = %d", GetLastError());
	}
	if(!isValid)
		DeleteConn(c);
	else if(c == m_readyConn)
		ConnectionReady(c,m_connected_ip,m_connected_port,m_UIDR,m_UIDS);


}

void VS_ConnectNHPExecutor::HandleError(const unsigned long err, const VS_Overlapped *ov)
{}
void VS_ConnectNHPExecutor::ServerFound(VS_ConnectionUDP *c)
{
	/**
		continue only one connection and stop nhp for other
	*/
	std::map<VS_ConnectionUDP*, ConnectionData> temp = m_nhp_conns;
	for(std::map<VS_ConnectionUDP*, ConnectionData>::iterator i = temp.begin();i!=temp.end();i++)
		if(i->first!=c)
		{
			i->second.get<0>()->Finish();
			DeleteConn(i->first);
		}
}

void VS_ConnectNHPExecutor::ConnectionReady(VS_ConnectionUDP *c,const unsigned long ip, const unsigned short port, const unsigned long UIDR, const unsigned long UIDS)
{
	/**
		1. release c from event loop
		2. m_fireSetNHPConn
		3. finish (stop thread)
	*/
	DTRACE(VSTM_NETWORK, "ConnectionReady c = 0x%x; ip = %x; port = %d; UIDR = %d; UIDS = %d", c,ip,port,UIDR,UIDS);
	m_connectionIsReady = true;
	assert(!c->IsRead());
	ServerFound(c); //delete connections except current
	if (c->IsWrite() || c->IsRead())
	{
		//wait for write complete
		DTRACE(VSTM_NETWORK,"IO is not compleated. %s return true", c->IsWrite() ? "c->IsWrite()" : "c->IsRead()");
		m_readyConn = c;
		m_connected_ip = ip;
		m_connected_port = port;
		m_UIDR = UIDR;
		m_UIDS = UIDS;
		return;
	}
	m_workerThread->UnBindConnection(c);
	c->Connect(ip,port);
	DTRACE(VSTM_NETWORK, "m_fireSetNHPConn for c = %x", c);
	m_fireSetNHPConn(c,UIDR,UIDS);
	m_nhp_conns.erase(c);
	if (m_nhp_conns.empty())
	{
		DTRACE(VSTM_NETWORK, "ConnectionReady() Stop Work Thread by m_nhp_conns.empty()");
		m_workerThread->Stop();
	}
}
void VS_ConnectNHPExecutor::DeleteConn(VS_ConnectionUDP *c)
{
	if(!c)
		return;
	c->Close();
	if(c->IsRW())
		return;
	m_workerThread->UnBindConnection(c);
	m_nhp_conns.erase(c);
	delete c;
	if(m_nhp_conns.empty())
	{
		DTRACE(VSTM_NETWORK, "DeleteConn() Stop Work Thread by m_nhp_conns.empty()");
		m_workerThread->Stop();
		assert(!CheckTime() || m_connectionIsReady);
	}
}

#endif
