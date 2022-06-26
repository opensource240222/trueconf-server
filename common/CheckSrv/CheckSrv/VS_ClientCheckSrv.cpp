#include "VS_ClientCheckSrv.h"
#include "acs/connection/VS_ConnectionTCP.h"
#include "acs/ConnectionManager/VS_ConnectionManager.h"
#include "net/EndpointRegistry.h"
#include "std/cpplib/ThreadUtils.h"

#include "std-generic/compat/memory.h"

VS_ClientCheckSrv::VS_ClientCheckSrv()
	: m_id(0)
	, m_callback(0)
	, m_padding_sz(0)
	, m_timeout(0)
{}
VS_ClientCheckSrv::~VS_ClientCheckSrv()
{
	if(m_thread.joinable())
	{
		m_thread.join();
	}
}
bool VS_ClientCheckSrv::Init(std::unique_ptr<net::endpoint::ConnectTCP> reg_conn, VS_CheckSrvResultCallBack* callback, const unsigned check_id, const unsigned padding_sz, unsigned char* padding)
{
	if (!reg_conn || !callback)
		return false;
	m_callback = callback;
	m_check_reg_conn = std::move(reg_conn);
	m_id = check_id;
	if(padding_sz)
	{
		m_padding_buf = vs::make_unique_default_init<unsigned char[]>(padding_sz);
		memcpy(m_padding_buf.get(),padding,padding_sz);
		m_padding_sz = padding_sz;
	}
	return true;
}
bool VS_ClientCheckSrv::AsyncCheck(const unsigned long mills)
{
	if(m_thread.joinable()|| !m_callback)
		return false;
	m_timeout = mills;
	m_thread = std::thread([this]()
	{
		vs::SetThreadName("ClientCheckSrv");
		Thread();
	}
	);
	return true;
}
unsigned VS_ClientCheckSrv::Thread()
{
	VS_CheckSrvPack	pack;
	VS_ConnectionSock* conn(0);
	unsigned long mills = m_timeout;
	conn = VS_CreateConnection(*m_check_reg_conn, mills, false);
	if(!conn || !conn->CreateOvReadEvent() || !conn->CreateOvWriteEvent())
		m_callback->Result(m_id, e_chksrv_error,0);
	else
	{
		unsigned char	*buf(0);
		unsigned long	sz(0);
		if(m_padding_sz)
		{
			pack.SetPadding(m_padding_sz, m_padding_buf.get());
		}
		pack.GetNetworkBuffer(0, sz);
		if(sz)
		{
			buf = new unsigned char [sz];
			if(!pack.GetNetworkBuffer(buf,sz))
			{
				m_callback->Result(m_id, e_chksrv_error,0);
			}
			else if(!conn->Write(buf, sz)||(conn->GetWriteResult(mills)!=sz))
			{
				m_callback->Result(m_id, e_chksrv_error,0);
			}
			else if(!conn->Read(buf,sz) || (conn->GetReadResult(mills) != sz))
			{
				m_callback->Result(m_id, e_chksrv_error,0);
			}
			else
			{
				VS_CheckSrvPack	result_pack;
				if(result_pack.SetNetworkBuffer(buf,sz))
					m_callback->Result(m_id,e_chksrv_ok, &result_pack);
				else
					m_callback->Result(m_id,e_chksrv_handshake_failed,0);
			}
		}
	}
	if(conn)
	{
		conn->Disconnect();
		delete conn;
	}
	return 0;
}
VS_ClientCheckSrvFast::VS_ClientCheckSrvFast(std::unique_ptr<net::endpoint::ConnectTCP> reg_conn, const unsigned long mills, const unsigned padding_sz, unsigned char *padding)
	: m_reg_conn(std::move(reg_conn))
	, m_timeout(mills)
{
	if(padding_sz)
		m_padding_buf.assign(padding,padding+padding_sz);
}
void VS_ClientCheckSrvFast::Check()
{
	VS_CheckSrvPack	pack;
	boost::shared_ptr<VS_ConnectionSock> conn;
	unsigned long mills = m_timeout;
	conn.reset(VS_CreateConnection(*m_reg_conn, mills, false));
	if(!conn || !conn->CreateOvReadEvent() || !conn->CreateOvWriteEvent())
		m_fireResult(e_chksrv_error,0);
	else
	{
		std::vector<unsigned char> buf;
		unsigned long	sz(0);
		if(m_padding_buf.size())
			pack.SetPadding(m_padding_buf.size(), &m_padding_buf[0]);
		pack.GetNetworkBuffer(0, sz);
		auto start_recv_tick = std::chrono::steady_clock::now();
		if(sz)
		{
			buf.resize(sz);
			if (!pack.GetNetworkBuffer(&buf[0], sz)
				|| !conn->Write(&buf[0], sz)
				|| (conn->GetWriteResult(mills) != sz)
				|| !conn->Read(&buf[0], sz)
				|| (conn->GetReadResult(mills) != sz))
			{
				m_fireResult(e_chksrv_error, 0);
			}
			else
			{
				auto srv_response_time = std::chrono::steady_clock::now() - start_recv_tick;
				VS_CheckSrvPack	result_pack;
				if(result_pack.SetNetworkBuffer(&buf[0],sz))
				{
					unsigned long sz = 0;
					result_pack.GetEPCount(sz);
					auto srv_response_time_ms =
						std::chrono::duration_cast<
							std::chrono::milliseconds>(
								srv_response_time).count()
						+ sz/50;
					m_fireResult(e_chksrv_ok,static_cast<uint32_t>(srv_response_time_ms));
				}
				else
					m_fireResult(e_chksrv_handshake_failed,0);
			}
		}
	}
}

void VS_LocatorCheck::Run(unsigned long checktime)
{
	auto ep = net::endpoint::ReadConnectTCP(1, m_ep.m_str, false);
	if (!ep)
	{
		net::endpoint::CreateFromID(m_ep.m_str, false);
		ep = net::endpoint::ReadConnectTCP(1, m_ep.m_str, false);
		if (!ep)
			return;
	}
	VS_ClientCheckSrvFast check_srv(std::move(ep), checktime, 0);
	VS_ClientCheckSrvFast::ResultSignalType::slot_type slot(&VS_LocatorCheck::Result, this, _1, _2);
	check_srv.ConnectToSignal(slot);
	check_srv.Check();
}