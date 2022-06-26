#include "VS_CheckSrvPack.h"
#include "memory.h"
#include "string.h"
#include "../../std/cpplib/VS_MemoryLeak.h"

VS_CheckSrvPack::VS_CheckSrvPack() : m_padding_size(0), m_padding_buffer(0),m_ep_count(0),m_cpu(0),
									m_network_buffer(0),m_network_buffer_sz(0)
{
	memset(m_CheckSrv_PrimaryField, 0, net::HandshakeHeader::primary_field_size);
	strncpy(m_CheckSrv_PrimaryField, VS_CHECK_SRV_PRIMARY_FIELD, net::HandshakeHeader::primary_field_size - 1);
}
VS_CheckSrvPack::~VS_CheckSrvPack()
{
	if(m_padding_buffer)
	{
		delete [] m_padding_buffer;
		m_padding_buffer = 0;
		m_padding_size = 0;
	}
	if(m_network_buffer)
	{
		delete [] m_network_buffer;
		m_network_buffer = 0;
		m_network_buffer_sz = 0;
	}
}
void VS_CheckSrvPack::SetPadding(const unsigned long sz, const unsigned char *padding)
{
	if(!sz)
		return;
	m_padding_buffer = new unsigned char [sz];
	m_padding_size = sz;
	if(padding)
		memcpy(m_padding_buffer, padding, sz);
	else
		memset(m_padding_buffer,0,sz);
}
bool VS_CheckSrvPack::GetPadding(unsigned long &sz, unsigned char *padding)
{
	if(!m_padding_size)
		return false;
	else if(sz < m_padding_size)
	{
		sz = m_padding_size;
		return false;
	}
	else
	{
		memcpy(padding, m_padding_buffer,m_padding_size);
		sz = m_padding_size;
		return true;
	}
}
void VS_CheckSrvPack::SetEPCount(const unsigned long count)
{
	m_ep_count = count;
}
bool VS_CheckSrvPack::GetEPCount(unsigned long &sz)
{
	sz = m_ep_count;
	return true;
}
void VS_CheckSrvPack::SetCPU(const unsigned cpu)
{
	m_cpu = cpu;
}
bool VS_CheckSrvPack::GetCPU(unsigned &cpu)
{
	cpu = m_cpu;
	return true;
}
bool VS_CheckSrvPack::SetNetworkBuffer(unsigned char *buf, const unsigned long sz)
{
	/*
	Получит из буфера все поля
	*/
	if(!sz||!buf)
		return false;
	CheckPack *pack = (CheckPack*)buf;
	if (pack->hs.head_cksum != net::GetHandshakeHeaderChecksum(pack->hs))
		return false;
	if (pack->hs.body_cksum != net::GetHandshakeBodyChecksum(pack->hs))
		return false;
	if(pack->hs.body_length != (sz - sizeof(net::HandshakeHeader) - 1))
		return false;
	m_padding_size = pack->padding_sz;
	unsigned general_sz = m_padding_size + sizeof(CheckPack);
	if(general_sz != sz)
		return false;
	m_cpu = pack->cpu;
	m_ep_count = pack->ep_count;
	if(m_padding_size)
		memcpy(m_padding_buffer,buf + sizeof(CheckPack),m_padding_size);
	else
		m_padding_buffer = 0;
	return true;
}
bool VS_CheckSrvPack::GetNetworkBuffer(unsigned char *buf, unsigned long &sz)
{
	/*
		сформировать буфер, где кол-во EP, загрузка CPU и padding
		Сформировать VS_FixedPart заголовок
	*/
	CheckPack *pack(0);
	unsigned sz_buf = sizeof(CheckPack) + m_padding_size;
	if(sz_buf>sz)
	{
		sz = sz_buf;
		return false;
	}
	unsigned char *net_buf = new unsigned char[sz_buf];
	memset(net_buf, 0, sz_buf);
	pack = (CheckPack*)net_buf;
	pack->ep_count = m_ep_count;
	pack->cpu = m_cpu;
	pack->padding_sz = m_padding_size;
	if(m_padding_size)
	{
		memcpy(net_buf,m_padding_buffer,m_padding_size);
	}
	strcpy(pack->hs.primary_field, VS_CHECK_SRV_PRIMARY_FIELD);
	pack->hs.body_length = sz_buf - sizeof(net::HandshakeHeader) - 1;
	pack->hs.version = 1;
	net::UpdateHandshakeChecksums(pack->hs);
	if(m_network_buffer)
	{
		delete [] m_network_buffer;
		m_network_buffer = 0;
		m_network_buffer_sz = 0;
	}
	m_network_buffer = net_buf;
	m_network_buffer_sz = sz_buf;
	memcpy(buf,m_network_buffer,sz_buf);
	sz = sz_buf;
	return true;
}
bool VS_CheckSrvPack::CheckPrimaryField(const char field[])
{
	return !strncmp(field,m_CheckSrv_PrimaryField,sizeof(m_CheckSrv_PrimaryField));
}
void VS_CheckSrvPack::Reset()
{
	if(m_padding_buffer)
	{
		delete [] m_padding_buffer;
		m_padding_buffer = 0;
		m_padding_size = 0;
	}
	m_ep_count = 0;
	m_cpu = 0;
	if(m_network_buffer)
	{
		delete [] m_network_buffer;
		m_network_buffer = 0;
		m_network_buffer_sz = 0;
	}
}