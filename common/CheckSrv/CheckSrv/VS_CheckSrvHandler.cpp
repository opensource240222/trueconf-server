#include "process.h"

#include "VS_CheckSrvHandler.h"
#include "net/Handshake.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/VS_MemoryLeak.h"
#include "std-generic/compat/memory.h"
#include "transport/Router/VS_TransportRouter.h"
#include "transport/Router/VS_TransportRouterServiceBase.h"

VS_CheckSrvHandler::VS_CheckSrvHandler():m_tr(0)
{
}
VS_CheckSrvHandler::~VS_CheckSrvHandler()
{
}
bool VS_CheckSrvHandler::IsValid() const
{
	return VS_AccessConnectionHandler::IsValid();
}
bool VS_CheckSrvHandler::Init(const char *handler_name)
{
	return true;
}
VS_ACS_Response VS_CheckSrvHandler::Connection(unsigned long *in_len)
{
	if (in_len)
		*in_len = sizeof(net::HandshakeHeader);
	return vs_acs_next_step;
}
VS_ACS_Response VS_CheckSrvHandler::Protocol(const void *in_buffer, unsigned long *in_len,
											void ** out_buffer, unsigned long *out_len, void **context)
{
	if(*in_len < sizeof(net::HandshakeHeader))
	{
		*in_len = sizeof(net::HandshakeHeader);
		return vs_acs_next_step;
	}
	const auto& hs = *static_cast<const net::HandshakeHeader*>(in_buffer);
	if (hs.head_cksum != net::GetHandshakeHeaderChecksum(hs) ||
		!m_check_pack.CheckPrimaryField(hs.primary_field))
			return vs_acs_connection_is_not_my;
	*in_len = hs.body_length;
	return vs_acs_accept_connections;
}
struct CheckSrvArg
{
	VS_CheckSrvPack	m_pack;
	std::unique_ptr<VS_ConnectionTCP> m_conn;
	VS_TransportRouter	*m_tr;
	std::vector<unsigned char> m_buffer;
	bool m_isinit;
	CheckSrvArg(
		std::unique_ptr<VS_ConnectionTCP>&& conn,
		std::vector<unsigned char> &&buffer,
		VS_TransportRouter* tr)
		: m_conn(std::move(conn))
		, m_buffer(std::move(buffer))
		, m_tr(tr)
		, m_isinit(false)
	{
		m_isinit = m_buffer.size() >= sizeof(net::HandshakeHeader);
	}
	bool FormPack()
	{
		if(!m_conn)
			return false;
		auto head = reinterpret_cast<net::HandshakeHeader*>(m_buffer.data());
		auto sz = sizeof(net::HandshakeHeader) + head->body_length + 1 - m_buffer.size();
		if (sz)
		{
			m_buffer.resize(m_buffer.size() + sz);
			unsigned long mills(3000);
			if (m_conn->SelectIn(mills) != 1)
				return false;
			int res = m_conn->Receive(m_buffer.data() + m_buffer.size() - sz, sz);
			if (res != sz)
				return false;
			head = reinterpret_cast<net::HandshakeHeader*>(m_buffer.data());
		}
		if (head->body_cksum != net::GetHandshakeBodyChecksum(*head))
			return false;
		else
		{
			if (!m_pack.SetNetworkBuffer(m_buffer.data(), m_buffer.size()))
				return false;
			else
				return true;
		}
	}
	inline void Thread()
	{
		if(!m_isinit || !FormPack())
			return;
		std::unique_ptr<unsigned char[]> padding;
		unsigned long sz(0);
		if(!m_pack.GetPadding(sz, nullptr)&&sz)
		{
			padding = vs::make_unique<unsigned char[]>(sz);
			m_pack.GetPadding(sz,padding.get());
		}
		unsigned ep(0);
		unsigned cpu(0);
		VS_TransportRouterStatistics	stat;
		if(m_tr->GetStatistics(stat))
			ep = stat.endpoints;
		VS_CheckSrvPack	new_pack;
		new_pack.SetEPCount(ep);
		new_pack.SetCPU(cpu);
		new_pack.SetPadding(sz, padding.get());
		std::unique_ptr<unsigned char[]> network_buf;
		sz = 0;
		new_pack.GetNetworkBuffer(nullptr,sz);
		if(sz)
		{
			network_buf = vs::make_unique<unsigned char[]>(sz);
			new_pack.GetNetworkBuffer(network_buf.get(),sz);
			m_conn->Send(network_buf.get(),sz);
			unsigned long mills(3000);
			m_conn->SelectOut(mills);
		}
	}
};
void VS_CheckSrvHandler::Accept(VS_ConnectionTCP *conn, const void *in_buffer, const unsigned long in_len, const void *context)
{
	if(!conn||!m_tr)
		return;
	auto arg = vs::make_unique<CheckSrvArg>(
		std::unique_ptr<VS_ConnectionTCP>(conn),
		std::vector<unsigned char>(
			static_cast<const unsigned char*>(in_buffer),
			static_cast<const unsigned char*>(in_buffer) + in_len),
		m_tr);
	std::thread th(
		[arg = std::move(arg)]() {
			vs::SetThreadName("ChkSrvHandler");
			arg->Thread();
		}
	);
	th.detach();
}
void VS_CheckSrvHandler::Destructor(const void *context)
{
}
void VS_CheckSrvHandler::Destroy(const char *handler_name)
{
}
void VS_CheckSrvHandler::SetTransportRouter(VS_TransportRouter	*tr)
{
	m_tr = tr;
}
