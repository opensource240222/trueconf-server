#pragma once
#include "../../net/Handshake.h"

#define VS_CHECK_SRV_PRIMARY_FIELD	"CHECK_SRV"

class VS_CheckSrvPack
{
private:
	unsigned long	m_padding_size;
	unsigned char	*m_padding_buffer;
	unsigned		m_ep_count;
	unsigned		m_cpu;

	unsigned char	*m_network_buffer;
	unsigned		m_network_buffer_sz;
	char			m_CheckSrv_PrimaryField[net::HandshakeHeader::primary_field_size];

#pragma pack(1)
	struct CheckPack
	{
		net::HandshakeHeader hs;
		unsigned ep_count, cpu, padding_sz;
		unsigned char	byte;
		/*
			Дальше буфер набивки
		*/
	};
#pragma pack ( )


public:
	VS_CheckSrvPack();
	virtual ~VS_CheckSrvPack();

	void	SetPadding(const unsigned long sz, const unsigned char *padding = 0);
	bool	GetPadding(unsigned long &sz, unsigned char *padding);

	void	SetEPCount(const unsigned long count);
	bool	GetEPCount(unsigned long &count);

	void	SetCPU(const unsigned cpu);
	bool	GetCPU(unsigned &cpu);

	bool	SetNetworkBuffer(unsigned char *buf, const unsigned long sz );
	bool	GetNetworkBuffer(unsigned char *buf, unsigned long &sz);
	bool	CheckPrimaryField(const char field[]);
	void	Reset();
};