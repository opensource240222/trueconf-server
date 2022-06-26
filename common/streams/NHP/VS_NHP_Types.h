#pragma once

#include "nhp_HSAPI.h"

#include <stdio.h>

#define SYN_FRAME_SIZE (sizeof(NHP_PACK_TYPE)+4*sizeof(long)+MESSAGE_SIZE)
#define SYN_ACK_FRAME_SIZE (sizeof(NHP_PACK_TYPE) + 4*sizeof(long) + HASH_SIZE)
#define REQ_CLIENT_INFO_SIZE (sizeof(NHP_PACK_TYPE) + END_POINT_SIZE)
#define RESP_CLIENT_INFO_SIZE (sizeof(NHP_PACK_TYPE) + END_POINT_SIZE + 2*sizeof(unsigned long) + 2*sizeof(unsigned short))

struct VS_NHP_ConnectionData
{
	VS_NHP_ConnectionData()
	{
		ip = 0;
		port = 0;
		/*local_ip = 0;
		local_port = 0;*/
	}
	unsigned ip;
	unsigned short	port;
	/*unsigned long	local_ip;
	unsigned short	local_port;*/
	void Show(const char *str=0)
	{
		const unsigned char * ipch =
		reinterpret_cast<const unsigned char *>(&ip);
		printf("\n\t %s Ip: %u.%u.%u.%u Ip: %x Port: %d \n",
			str,
			ipch[3],
			ipch[2],
			ipch[1],
			ipch[0],ip,port);
		/*const unsigned char * lca_ipch =
		reinterpret_cast<const unsigned char *>(&local_ip);
		printf("\n\t %s local Ip: %u.%u.%u.%u local Ip: %x local Port: %d \n",
			str,
			ipch[3],
			ipch[2],
			ipch[1],
			ipch[0],local_ip,local_port);*/
	}
};

struct VS_NHP_Container
{
	VS_NHP_ConnectionData my_local;
	VS_NHP_ConnectionData my_nat;
	VS_NHP_ConnectionData peer_local;
	VS_NHP_ConnectionData peer_nat;
	VS_NHP_ConnectionData server;
	void Show()
	{
		my_local.Show(  "  my_local");
		my_nat.Show(    "    my_nat");
		peer_local.Show("peer_local");
		peer_nat.Show(  "  peer_nat");
		server.Show(    "    server");
	}
};

//struct VS_NHP_ClientConnectionInfo
//{
//	char	EndPoint[10];
//	VS_NHP_ConnectionData	local;
//	VS_NHP_ConnectionData	nat;
//};
#include <list>
//typedef std::list<VS_NHP_ClientConnectionInfo> VS_NHP_ClientConnectionInfoContainer;

struct _EP_INFO
{
	char			EndPoint[END_POINT_SIZE];
	unsigned long	IP;
	unsigned short	port;
};

enum NHP_PACK_TYPE{
	SYN				= 0,
	SYN_ACK			= 1,
	ACK				= 2,
	PUSH			= 3,
	REQCLIENTINFO	= 4,
	RESPCLIENTINFO	= 5,
	UNKNOWN			= 6
};

//struct NHPHandShakePacket
//{
//	NHP_PACK_TYPE	type;
//	union
//	{
//		SYN_TYPE		syndata;
//		SYN_ACK_TYPE	synackdata;
//		_EP_INFO		epdata;
//	};
//};

////////////////////////////////////////////////////
