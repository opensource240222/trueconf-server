#if defined(_WIN32) // Not ported yet

#include "VS_NHP_Handshake_Impl.h"
#include "../../acs/Lib/VS_AcsLib.h"
#include "acs/connection/VS_ConnectionSock.h"
#include "std-generic/cpplib/hton.h"
#include "../../std/debuglog/VS_Debug.h"

#include <Windows.h>

#define DEBUG_CURRENT_MODULE VS_DM_NHP

VS_NHP_Handshake_Impl::VS_NHP_Handshake_Impl(void)
	:cur_conn(0),num_syn(0),num_synack(0),m_pPort(0),m_pIP(0),m_buffSz(0xffff)
{
//NANOBEGIN;
	memset(&m_pushdata,0,sizeof(m_pushdata));
	memset(&m_addr_buf,0,16);
	memset(m_recvBuff,0,0xffff);

	m_pushbuf = 0;
	m_ackdata.ack_buf.length = 0;
	m_ackdata.ack_buf.buf = 0;
	InitializeCriticalSection(&nhp_sect);
//NANOEND;
}
////////////////////////////////////////////////////
VS_NHP_Handshake_Impl::~VS_NHP_Handshake_Impl(void)
{
//NANOBEGIN;
	if(m_ackdata.ack_buf.length)
		NHPBufFree(&m_ackdata.ack_buf);
	if(m_pushdata.length)
		NHPBufFree(&m_pushbuf);
	memset(&m_pushdata,0,sizeof(m_pushdata));
	DeleteCriticalSection(&nhp_sect);
//NANOEND;
}

////////////////////////////////////////////////////
////void VS_NHP_Handshake_Impl::NHPInit(VS_ConnectionSock* newconn)
////{
////	if(!cur_conn)
////		cur_conn = newconn;
////}
void VS_NHP_Handshake_Impl::NHPInit(VS_ConnectionUDP* newconn)
{
	if(!cur_conn)
		cur_conn = newconn;
}
////////////////////////////////////////////////////
void VS_NHP_Handshake_Impl::NHPFinal()
{
	cur_conn = 0;
}
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::IsSynData(const char* data, unsigned long  sz)
{
	NHP_PACK_TYPE	type;
	if(sz<SYN_FRAME_SIZE)
		return false;
	memcpy(&type,data,sizeof(type));
	if(SYN != type)
		return false;
	SYN_TYPE	syn;
	if(!GetSynFromBuf(data,sz,syn))
		return false;
	else
	{
		if(NHPGetCurrentVersion()!=NHPGetSynVersion(&syn))
			return false;
		else
			return true;
	}
}
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::IsSynAckData(const char* data, unsigned long  sz)
{
	NHP_PACK_TYPE	type;
	if(sz<SYN_ACK_FRAME_SIZE)
		return false;
	memcpy(&type,data,sizeof(type));
	if(SYN_ACK != type)
		return false;
	SYN_ACK_TYPE	syn_ack;
	if(!GetSynAckFromBuf(data,sz,syn_ack))
		return false;
	else
	{
		if(NHPGetCurrentVersion()!=NHPGetSynAckVersion(&syn_ack))
			return false;
		else
			return true;
	}
}
//end VS_StreamClient_Implementation::IsSynAckData
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::IsAckData(const char* data, unsigned long sz)
{
	NHP_PACK_TYPE	type;
	unsigned long size(0);
	if(sz<sizeof(size)+sizeof(type))
		return false;
	memcpy(&type,data,sizeof(type));
	if(ACK!=type)
		return false;
	memcpy(&size,data+sizeof(type),sizeof(size));
	if(size>sz)
		return false;
	return true;
}
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::IsPushData(const char* data, unsigned long sz)
{
	NHP_PACK_TYPE	type;
	unsigned long	size(0);
	if(sz <sizeof(type))
		return false;
	memcpy(&type,data,sizeof(type));
	if(PUSH!=type)
		return false;
	memcpy(&size,data + sizeof(type) + sizeof(PUSH_TYPE) - sizeof(unsigned long),sizeof(size));
	if(size!= sz - sizeof(type) - sizeof(PUSH_TYPE))
		return false;
	return true;
}
////////////////////////////////////////////////////
NHP_PACK_TYPE VS_NHP_Handshake_Impl::WhatPacketType(const char* data,unsigned long sz)
{
	NHP_PACK_TYPE	type;
	memcpy(&type,data,sizeof(type));
	switch(type)
	{
	case SYN:
		if(IsSynData(data,sz))
			return SYN;
		break;
	case SYN_ACK:
		if(IsSynAckData(data,sz))
			return SYN_ACK;
		break;
	case ACK:
		if(IsAckData(data,sz))
			return ACK;
		break;
	case PUSH:
		if(IsPushData(data,sz))
			return PUSH;
		break;
	}
	return UNKNOWN;
}
// end VS_StreamClient_Implementation::IsAckData
////////////////////////////////////////////////////
int VS_NHP_Handshake_Impl::GetNHPData(char* buf,unsigned long buf_size,unsigned long mills)
{
	int	res;
	if(!cur_conn)
		return -1;
	if(!cur_conn->IsRead())
	{
		if(!cur_conn->Read(buf,buf_size))
			return -1;
	}
	res = cur_conn->GetReadResult(mills,0,true);
	if(-1 ==res)
	{
		if(!cur_conn->Read(buf,buf_size))
			return -1;
		res = cur_conn->GetReadResult(mills,0,true);
	}
	return res;
}
	// end VS_StreamClient_Implementation::GetNHPData
////////////////////////////////////////////////////
int VS_NHP_Handshake_Impl::GetNHPData(char* buf,unsigned long buf_size,unsigned long mills, unsigned long &ip_from, unsigned short &port_from)
{
	int res(0);
	if(!cur_conn)
		return -1;

	unsigned long tmpTick(GetTickCount());
	do
	{
		if(cur_conn->IsRead() || cur_conn->AsynchReceiveFrom(m_recvBuff,m_buffSz,m_addr_buf,m_pIP,m_pPort))
		{
			res = cur_conn->GetReadResult(mills,0,true);
			if(res>0)
			{
				ip_from = vs_htonl(*m_pIP);
				port_from = vs_htons(*m_pPort);
				memcpy(buf,m_recvBuff,buf_size<m_buffSz?buf_size:m_buffSz);
				res = res < (int)buf_size ? res : buf_size;
				unsigned char * c = (unsigned char*)m_pIP;
				dprint3("NHPData Was received from with IP = %d.%d.%d.%d. port = %d\n",c[0],c[1],c[2],c[3],vs_htons(*m_pPort));
			}
			else
			{
				switch(GetLastError())
				{
				case ERROR_PORT_UNREACHABLE:
					dprint3("ERROR_PORT_UNREACHABLE GetReadResult\n");
					continue;

					break;
				case WSAECONNRESET:
					dprint3("WSAECONNRESET GetReadResult\n");
					continue;

					break;
				}
			}
			memset(m_addr_buf,0,16);
			return res;
		}
		else
		{
			switch(GetLastError())
			{
			case ERROR_PORT_UNREACHABLE:
				dprint3("ERROR_PORT_UNREACHABLE\n");
				if((GetTickCount() - tmpTick)<mills)
				{
					continue;
				}
				break;
			case WSAECONNRESET:
				dprint3("WSAECONNRESET\n");
				if((GetTickCount() - tmpTick)<mills)
				{
					continue;
				}
				break;
			}
			memset(m_addr_buf,0,16);
			return -1;
		}
	}while(true);
	memset(m_addr_buf,0,16);
	return res;
}

bool VS_NHP_Handshake_Impl::GetAck(const unsigned long mills)
{
	unsigned long	size, timeout(mills),tick(GetTickCount()),tmptick;
	char			*Pos;
	char			*recvbuf(0);
	NHP_PACK_TYPE	type;
	int TransByte = GetNHPData((char*)&type,sizeof(type),/*10000*/timeout);
	if(ACK!=type)
		return false;
	tmptick = GetTickCount();
	timeout -= (tmptick-tick)>timeout?timeout:tmptick - tick;
	TransByte = GetNHPData((char*)&size,sizeof(size),timeout);
	if(size<sizeof(size))
		return false;
	if(TransByte!=sizeof(size))
		return false;
	if(size>65536)
		return false;
	Pos = recvbuf = (char*)malloc(size);//new char[size];
	memcpy(recvbuf,&type,sizeof(type));	Pos += sizeof(type);
	memcpy(recvbuf,&size,sizeof(size));	Pos += sizeof(size);
	tmptick = GetTickCount();
	timeout -= (tmptick-tick)>timeout?timeout:tmptick - tick;
	tick = tmptick;
	TransByte = GetNHPData(Pos,size-TransByte,timeout);
	if(0>TransByte)
		return false;
	if(!IsAckData(recvbuf,TransByte + sizeof(size)))
		return false;
	if(!GetAckFromBuf(recvbuf,TransByte + sizeof(size),m_ackdata))
		return false;
	return true;
}
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::GetPush(const unsigned long mills)
{
	unsigned long	timeout(mills),
					tick(GetTickCount()),
					tmptick;

	NHP_PACK_TYPE	type;
	if(m_pushbuf)
		NHPBufFree(&m_pushbuf);
	int TransByte = GetNHPData((char*)&type,sizeof(type),timeout);
	if(PUSH!= type)
		return false;
	if(TransByte != sizeof(type))
		return false;
	tmptick = GetTickCount();
	timeout -= (tmptick-tick)>timeout?timeout:tmptick - tick;
	tick = tmptick;
	TransByte = GetNHPData((char*)&m_pushdata,sizeof(m_pushdata),timeout);
	if(TransByte != sizeof(m_pushdata))
		return false;
	NHPPrepareBuf(&m_pushbuf,m_pushdata.length);
	tmptick = GetTickCount();
	timeout -= (tmptick-tick)>timeout?timeout:tmptick - tick;
	tick = tmptick;
	TransByte = GetNHPData(m_pushbuf,m_pushdata.length,timeout);
	if(TransByte!=m_pushdata.length)
	{
		NHPBufFree(&m_pushbuf);
		memset(&m_pushdata,0,sizeof(m_pushdata));
		return false;
	}
	if(!NHPPushIsValid(&m_pushdata,&m_ackdata))
	{
		NHPBufFree(&m_pushbuf);
		memset(&m_pushdata,0,sizeof(m_pushdata));
		return false;
	}
	return true;
}
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::GetPush(const char* host_from, const unsigned short port_from, const unsigned long mills)
{
	/**
		получить пакет от конкретного адреса,
	*/
	unsigned long	timeout(mills),
					tmptick(0);
	int				TransByte(0);

	NHP_PACK_TYPE	type;
	if(m_pushbuf)
		NHPBufFree(&m_pushbuf);
	if(m_pushbuf)
		NHPBufFree(&m_pushbuf);

	unsigned long	ip_from(0);
	unsigned long	ip(0);
	unsigned short	port(0);
	char ch_ip[256]={0};

	VS_GetHostByName(host_from,ch_ip,255);
	VS_GetIpByHost(ch_ip,&ip_from);

	char rcv_buf[0xffff] = {0};
	char *pPos = rcv_buf;
	do
	{
		unsigned long tick_first = GetTickCount();
		TransByte = GetNHPData(rcv_buf,0xffff,timeout,ip,port);
		unsigned long tick_last = GetTickCount();
		timeout -=  (tick_last - tick_first)<timeout?tick_last - tick_first : timeout;
		if(TransByte<(int)(sizeof(type) + sizeof(m_pushdata)))
			return false;
	}while((ip!=ip_from) || (port != port_from));
	type = *((NHP_PACK_TYPE*)pPos);	pPos+= sizeof(type);
	if(PUSH!= type)
		return false;
	memcpy((void*)&m_pushdata,pPos,sizeof(m_pushdata));
	pPos += sizeof(m_pushdata);
	if((unsigned)TransByte < m_pushdata.length + sizeof(type) + sizeof(m_pushdata))
		return false;

	NHPPrepareBuf(&m_pushbuf,m_pushdata.length);

	memcpy(m_pushbuf,pPos,m_pushdata.length);

	if(!NHPPushIsValid(&m_pushdata,&m_ackdata))
	{
		NHPBufFree(&m_pushbuf);
		memset(&m_pushdata,0,sizeof(m_pushdata));
		return false;
	}
	return true;
}
bool VS_NHP_Handshake_Impl::SendSYN(const unsigned long mills,bool GenSYN)
{
	int err;
	NHP_PACK_TYPE	type(SYN);
	char			senddatabuf[SYN_FRAME_SIZE];
	unsigned long	timeout(mills);

	if(!cur_conn)
		return false;
	if(GenSYN)
		if(!NHPSyn(timeout,&m_syndata,&err))
			return false;

	PrepareSYNBuf(m_syndata,senddatabuf);
	num_syn++;
	if(!cur_conn->Write(senddatabuf,SYN_FRAME_SIZE))
		return false;
	dprint3("Syn n = %ld was sent\n", m_syndata.field2);
	int  res  = cur_conn->GetWriteResult(timeout);
	dprint3("cur_conn->GetWriteResult(%lu); return %d\n", timeout, res);
	return(SYN_FRAME_SIZE==res);
}
// end VS_StreamClient_Implementation::SendSYN
bool VS_NHP_Handshake_Impl::SendSYN(const unsigned long mills, const char* host_to, const unsigned short port_to, bool GenSYN)
{
	int err;
	NHP_PACK_TYPE	type(SYN);
	char			senddatabuf[SYN_FRAME_SIZE];
	unsigned long	timeout(mills);

	if(!cur_conn)
		return false;
	if(GenSYN)
		if(!NHPSyn(timeout,&m_syndata,&err))
			return false;

	PrepareSYNBuf(m_syndata,senddatabuf);
	num_syn++;
	unsigned long ip(0);
	char ip_my[256]={0};

	if((!VS_GetHostByName(host_to , ip_my , 255 )) || (!VS_GetIpByHost(ip_my,&ip)))
		return false;
	if(!cur_conn->WriteTo(senddatabuf,SYN_FRAME_SIZE,ip,port_to))
		return false;
	dprint3("Syn n = %ld was sent host=%s(%s), port = %d\n", m_syndata.field2, host_to, ip_my, port_to);
	int  res  = cur_conn->GetWriteResult(timeout);
	dprint3("cur_conn->GetWriteResult(%lu); return %d\n", timeout, res);
	return(SYN_FRAME_SIZE==res);
}
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::SendSYN(const unsigned long mills, const unsigned long ip_to, const unsigned short port_to, bool GenSYN)
{
	int err;
	NHP_PACK_TYPE	type(SYN);
	char			senddatabuf[SYN_FRAME_SIZE];
	unsigned long	timeout(mills);

	if(!cur_conn)
		return false;
	if(GenSYN)
		if(!NHPSyn(timeout,&m_syndata,&err))
			return false;

	PrepareSYNBuf(m_syndata,senddatabuf);
	num_syn++;
	if(!cur_conn->WriteTo(senddatabuf,SYN_FRAME_SIZE,ip_to,port_to))
		return false;
	dprint3("Syn n = %ld was sent\n", m_syndata.field2);
	int  res  = cur_conn->GetWriteResult(timeout);
	dprint3("cur_conn->GetWriteResult(%lu); return %d\n", timeout, res);
	return(SYN_FRAME_SIZE==res);
}

bool VS_NHP_Handshake_Impl::GetSynAck(const unsigned long mills)
{
	unsigned long	timeout(mills);

	char			recvdatabuf[SYN_ACK_FRAME_SIZE + 1];
	char*			Pos = recvdatabuf;
	int				TransBytes;
	if(!cur_conn)
		return false;
	TransBytes = GetNHPData(recvdatabuf,SYN_ACK_FRAME_SIZE+1,timeout);
	if(TransBytes<=0)
		return false;
	if(!IsSynAckData(recvdatabuf,TransBytes))
		return false;
	if(!GetSynAckFromBuf(recvdatabuf,TransBytes,m_synackdata))
		return false;
	if(!NHPSynAckIsValid(&m_synackdata,&m_syndata))
		return false;
	else
		return true;
}
//end VS_StreamClient_Implementation::GetSynAck
bool VS_NHP_Handshake_Impl::GetSynAck(const unsigned int mills, const char *host_from, const unsigned short port_from)
{
	/**
		Получить данные из сокета, и проверить адрес,
		если адрес нужный, то обрабатываем, если левый, то повторяем, пока время не выйдет
	*/
	unsigned long	timeout(mills);

	char			recvdatabuf[SYN_ACK_FRAME_SIZE + 1];
	char*			Pos = recvdatabuf;
	int				TransBytes;

	if(!cur_conn)
		return false;
	unsigned long	ip(0);
	unsigned short	port(0);
	unsigned long ip_from(0);
	char ch_ip[256]={0};

	VS_GetHostByName(host_from,ch_ip,255);
	VS_GetIpByHost(ch_ip,&ip_from);

	do
	{
		unsigned long tick = GetTickCount();
		if(!timeout)
			return false;
		TransBytes = GetNHPData(recvdatabuf,SYN_ACK_FRAME_SIZE + 1, timeout,ip,port);
		if(TransBytes<=0)
			return false;
		unsigned long tick_last = GetTickCount();
		timeout -=  (tick_last - tick)<=timeout?tick_last - tick : timeout;
	}while((ip!=ip_from)||(port!=port_from));
	if(!IsSynAckData(recvdatabuf,TransBytes))
		return false;
	if(!GetSynAckFromBuf(recvdatabuf,TransBytes,m_synackdata))
		return false;
	if(!NHPSynAckIsValid(&m_synackdata,&m_syndata))
		return false;
	else
		return true;

}
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::SendAck(char EndpointName[],char ConnectedEndpointName[],char ConferenceName[],const unsigned long mills,SYN_ACK_TYPE	*synakc,unsigned char acknum,unsigned char ackcount)
{
	unsigned long	timeout(mills);
	unsigned long	size(0);
	char*			senddatabuf;
	unsigned long	IP;
	unsigned short	port;

	if(!cur_conn)
		return false;
	const char* chIP = cur_conn->GetBindIp();
	const char* chPort = cur_conn->GetBindPort();
	if(!chIP || !*chIP || !chPort || !*chPort)
		return false;
	port = atoi(chPort);
	if(!VS_GetIpByHost(chIP,&IP))
		return false;

	if(!synakc)
		senddatabuf = PrepareAckBuf(EndpointName,ConnectedEndpointName,ConferenceName,IP,port,&m_synackdata,acknum,ackcount,size);
	else
		senddatabuf = PrepareAckBuf(EndpointName,ConnectedEndpointName,ConferenceName,IP,port,synakc,acknum,ackcount,size);
	if(!GetAckFromBuf(senddatabuf,size,m_ackdata))
	{
		free(senddatabuf);
		return false;
	}
	if(!cur_conn->Write(senddatabuf,size))
	{
		free(senddatabuf);
		return false;
	}
	bool res = (size==cur_conn->GetWriteResult(timeout));
	free(senddatabuf);
	return res;
}
//end VS_StreamClient_Implementation::SendAck
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::SendAck(char EndpointName[], char ConnectedEndpointName[],char ConferenceName[],
						const char *host_to, const unsigned short port_to, const unsigned long mills,
						SYN_ACK_TYPE	*synakc,unsigned char acknum,unsigned char ackcount)
{
	unsigned long	timeout(mills);
	unsigned long	size(0);
	char*			senddatabuf;
	unsigned long	IP;
	unsigned short	port;

	if(!cur_conn)
		return false;
	const char* chIP = cur_conn->GetBindIp();
	const char * chPort = cur_conn->GetBindPort();
	if(!chIP || !*chIP || !chPort || !*chPort)
		return false;
	port = atoi(chPort);
	if(!VS_GetIpByHost(chIP,&IP))
		return false;

	if(!synakc)
		senddatabuf = PrepareAckBuf(EndpointName,ConnectedEndpointName,ConferenceName,IP,port,&m_synackdata,acknum,ackcount,size);
	else
		senddatabuf = PrepareAckBuf(EndpointName,ConnectedEndpointName,ConferenceName,IP,port,synakc,acknum,ackcount,size);
	if(!GetAckFromBuf(senddatabuf,size,m_ackdata))
	{
		free(senddatabuf);
		return false;
	}
	unsigned long ip_to(0);
	char ch_ip[256] = {0};
	VS_GetHostByName(host_to , ch_ip , 255 );
	VS_GetIpByHost(ch_ip,&ip_to);

	if(!cur_conn->WriteTo(senddatabuf,size,ip_to,port_to))
	{
		free(senddatabuf);
		return false;
	}
	bool res = (size==cur_conn->GetWriteResult(timeout));
	free(senddatabuf);
	return res;
}
bool VS_NHP_Handshake_Impl::SendPush(const ACK_TYPE &ackdata, const unsigned char master,unsigned long ip, unsigned short port,
										unsigned long local_IP, unsigned short local_port,unsigned long mills)
{
	unsigned long	size(0);
	char			*senddatabuf;

	if(!cur_conn)
		return false;
	senddatabuf = PreparePUSHBuf(ackdata,ip,port,local_IP,local_port,master,size);
	if(m_pushbuf)
		NHPBufFree(&m_pushbuf);
	if(!GetPushFromBuf(senddatabuf,size,m_pushdata,&m_pushbuf))
	{
		free(senddatabuf);
		return false;
	}
	if(!cur_conn->Write(senddatabuf,size))
	{
		free(senddatabuf);
		return false;
	}
	bool bRes = (size == cur_conn->GetWriteResult(mills));
	free(senddatabuf);
	return bRes;
}
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::GetSynFromBuf(const char* buf,unsigned long sz,SYN_TYPE& syndata)
{
	char	*Pos = (char*)buf + sizeof(NHP_PACK_TYPE);
	NHP_PACK_TYPE	type;
	if(sz<SYN_FRAME_SIZE)
		return false;
	memcpy(&type,buf,sizeof(type));
	if(SYN != type)
		return false;
	memcpy(&syndata.RND,Pos,sizeof(syndata.RND));		Pos	+= sizeof(syndata.RND);
	memcpy(&syndata.Message,Pos,MESSAGE_SIZE);			Pos	+= MESSAGE_SIZE;
	memcpy(&syndata.field1,Pos,sizeof(syndata.field1));	Pos	+= sizeof(syndata.field1);
	memcpy(&syndata.field2,Pos,sizeof(syndata.field2));	Pos	+= sizeof(syndata.field2);
	memcpy(&syndata.field3,Pos,sizeof(syndata.field3));	Pos	+= sizeof(syndata.field3);
	return true;
}
//end VS_StreamClient_Implementation::GetSynFromBuf
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::GetSynAckFromBuf(const char* buf, unsigned long sz, SYN_ACK_TYPE& synackdata)
{
	char	*Pos = (char*)buf + sizeof(NHP_PACK_TYPE);
	NHP_PACK_TYPE	type;
	if(sz<SYN_ACK_FRAME_SIZE)
		return false;
	memcpy(&type,buf,sizeof(type));
	if(SYN_ACK != type)
		return false;
	memcpy(&synackdata.SEQ,Pos,sizeof(synackdata.SEQ));			Pos += sizeof(synackdata.SEQ);
	memcpy(synackdata.synhash,Pos,HASH_SIZE);					Pos += HASH_SIZE;
	memcpy(&synackdata.field1,Pos,sizeof(synackdata.field1));	Pos += sizeof(synackdata.field1);
	memcpy(&synackdata.field2,Pos,sizeof(synackdata.field2));	Pos += sizeof(synackdata.field2);
	memcpy(&synackdata.field3,Pos,sizeof(synackdata.field3));	Pos += sizeof(synackdata.field3);
	return true;
}
//end VS_StreamClient_Implementation::GetSynAckFromBuf
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::GetAckFromBuf(const char* buf,unsigned long sz, ACK_TYPE& ackdata)
{
	char	*Pos = (char*)buf + sizeof(NHP_PACK_TYPE) + sizeof(unsigned long) + 2*sizeof(char);

	NHP_PACK_TYPE	type;
	unsigned long size(0);
	if(sz<sizeof(size)+sizeof(type))
		return false;
	memcpy(&type,buf,sizeof(type));
	if(ACK!=type)
		return false;
	memcpy(&size,buf+sizeof(type),sizeof(size));
	if(size>sz)
		return false;
	memcpy(&ackdata.IP,Pos,sizeof(ackdata.IP));							Pos += sizeof(ackdata.IP);
	memcpy(&ackdata.port,Pos,sizeof(ackdata.port));						Pos += sizeof(ackdata.port);
	memcpy(&ackdata.ack_buf.length,Pos,sizeof(ackdata.ack_buf.length));	Pos += sizeof(ackdata.ack_buf.length);
	ackdata.ack_buf.buf = (unsigned char*)malloc(ackdata.ack_buf.length);
	memcpy(ackdata.ack_buf.buf,Pos,ackdata.ack_buf.length);
	return true;
}
// end VS_StreamClient_Implementation::GetAckFromBuf
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::GetPushFromBuf(const char* buf,unsigned long sz, PUSH_TYPE & pushdata,char **pushbuf)
{
	char *Pos = (char*)buf + sizeof(NHP_PACK_TYPE);
	if(!IsPushData(buf,sz))
		return false;
	memcpy(&pushdata,Pos,sizeof(pushdata));	Pos += sizeof(pushdata);
	*pushbuf = (char*)malloc(pushdata.length);// new char[pushdata.length];
	memcpy(*pushbuf,Pos,pushdata.length);
	return true;
}
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::SendSYNAck(const SYN_TYPE& syndata,const unsigned long mills)
{
	unsigned long	timeout(mills);
	char			senddata[SYN_ACK_FRAME_SIZE];
	char			emptydata[SYN_ACK_FRAME_SIZE];

	memset(emptydata,0,SYN_ACK_FRAME_SIZE);
	if(!cur_conn)	return false;
	PrepareSynAckBuf(syndata,senddata);
	num_synack++;
	if(!memcmp(senddata,emptydata,SYN_ACK_FRAME_SIZE))
		return false;
	if(!cur_conn->Write(senddata,SYN_ACK_FRAME_SIZE))	return false;
	if(SYN_ACK_FRAME_SIZE!=cur_conn->GetWriteResult(timeout))	return false;
	dprint3("SynAck N = %ld was sent\n", num_synack - 1);
	return true;
}
//end VS_StreamClient_Implementation::SendSYNAck
////////////////////////////////////////////////////
void VS_NHP_Handshake_Impl::GenNewSyn(const unsigned long mills)
{
	int err;
	NHPSyn(mills,&m_syndata,&err);
}
////////////////////////////////////////////////////
void VS_NHP_Handshake_Impl::PrepareSYNBuf(const SYN_TYPE &syndata,char outbuf[])
{

	NHP_PACK_TYPE	type = SYN;
	char* Pos = outbuf;
	m_syndata.field2 = num_syn;
	memcpy(Pos,&type,sizeof(type));							Pos += sizeof(type);
	memcpy(Pos,&syndata.RND,sizeof(syndata.RND));			Pos += sizeof(syndata.RND);
	memcpy(Pos,syndata.Message,MESSAGE_SIZE);				Pos += MESSAGE_SIZE;
	memcpy(Pos,&syndata.field1,sizeof(syndata.field1));		Pos +=sizeof(syndata.field1);
	memcpy(Pos,&syndata.field2,sizeof(syndata.field2));		Pos += sizeof(syndata.field2);
	memcpy(Pos,&syndata.field3,sizeof(syndata.field3));		Pos += sizeof(syndata.field3);

}
////////////////////////////////////////////////////
void VS_NHP_Handshake_Impl::PrepareSynAckBuf(const SYN_TYPE &syndata, char outbuf[SYN_ACK_FRAME_SIZE])
{
	int				err;
	SYN_ACK_TYPE	synack;
	NHP_PACK_TYPE	type = SYN_ACK;

	memset(outbuf,0,SYN_ACK_FRAME_SIZE);
	if(!NHPTransformSYN(&syndata,&synack,&err))	return;
	synack.field2 = num_synack;
	char *Pos = outbuf;
	memcpy(Pos,&type,sizeof(type));						Pos += sizeof(type);
	memcpy(Pos,&synack.SEQ,sizeof(synack.SEQ));			Pos += sizeof(synack.SEQ);
	memcpy(Pos,synack.synhash,HASH_SIZE);				Pos += HASH_SIZE;
	memcpy(Pos,&synack.field1,sizeof(synack.field1));	Pos += sizeof(synack.field1);
	memcpy(Pos,&synack.field2,sizeof(synack.field2));	Pos += sizeof(synack.field2);
	memcpy(Pos,&synack.field3,sizeof(synack.field3));	Pos += sizeof(synack.field3);
}
////////////////////////////////////////////////////
char* VS_NHP_Handshake_Impl::PrepareAckBuf(char EndpointName[],char ConnectedEndpointName[],char ConferenceName[],unsigned long IP, unsigned short port,
										   SYN_ACK_TYPE* synack,unsigned char acknum,
										   unsigned char ackcount,unsigned long &length)
{
	ACK_TYPE		ack_buf;
	char*			outbuf(0);
	char*			Pos;
	const NHP_PACK_TYPE	type = ACK;
	int				err;
	unsigned long	size;

	if(!NHPAck(/*synack,*/EndpointName,ConnectedEndpointName,ConferenceName,IP,port,&ack_buf,&err))
		return 0;
	size = sizeof(type) + sizeof(size) + sizeof(ackcount) + sizeof(acknum) + sizeof(ack_buf.IP) + sizeof(ack_buf.port) + sizeof(ack_buf.ack_buf.length) +
		ack_buf.ack_buf.length;

	Pos = outbuf = (char*)malloc(size);
	memcpy(Pos,&type,sizeof(type));										Pos += sizeof(type);
	memcpy(Pos,&size,sizeof(size));										Pos += sizeof(size);
	memcpy(Pos,&acknum,sizeof(acknum));									Pos += sizeof(acknum);
	memcpy(Pos,&ackcount,sizeof(ackcount));								Pos += sizeof(ackcount);
	memcpy(Pos,&ack_buf.IP,sizeof(ack_buf.IP));							Pos += sizeof(ack_buf.IP);
	memcpy(Pos,&ack_buf.port,sizeof(ack_buf.port));						Pos += sizeof(ack_buf.port);
	memcpy(Pos,&ack_buf.ack_buf.length,sizeof(ack_buf.ack_buf.length));	Pos += sizeof(ack_buf.ack_buf.length);
	memcpy(Pos,ack_buf.ack_buf.buf,ack_buf.ack_buf.length);				Pos += ack_buf.ack_buf.length;
	NHPBufFree(&ack_buf.ack_buf);
	length = size;
	return outbuf;
}
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::TransformACK(const ACK_TYPE			&ackdata,
										char					*EndpointName,
										char					*ConnectedEndpointName,
										char					*ConferenceName,
										unsigned long			&IP,
										unsigned short			&port)
{
	int err;
	return NHPGetAckInfo(&ackdata,EndpointName,ConnectedEndpointName,ConferenceName,&IP,&port,&err);
}
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::TransformPUSH(const PUSH_TYPE	&pushdata,
										  const char		*pushbuf,
										  unsigned long		&ip,
										  unsigned short	&port,
										  unsigned long		&local_ip,
										  unsigned short	&local_port)
{
	int err;
	return NHPGetPushInfo(&pushdata,pushbuf,&ip,&port,&local_ip,&local_port,&err);
}
////////////////////////////////////////////////////

////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::GetAddressFromPush(unsigned long &ip, unsigned short &port, unsigned long &local_ip, unsigned short &local_port)
{
	return TransformPUSH(m_pushdata,m_pushbuf,ip,port,local_ip,local_port);
}
////////////////////////////////////////////////////
char* VS_NHP_Handshake_Impl::PreparePUSHBuf(const ACK_TYPE& ackdata,
											const unsigned long IP, const unsigned short port,
											const unsigned long	local_IP,const unsigned short local_port,
											const unsigned char master,
											unsigned long &length)
{
	PUSH_TYPE		pushdata;
	char*			pushbuf(0);
	unsigned long	len(0);
	int				err;
	unsigned long	size;
	char*			outbuf(0),*Pos;

	const NHP_PACK_TYPE	type = PUSH;

	if(!NHPPush(&ackdata,IP,port,local_IP,local_port,master,&pushdata,&pushbuf,&len,&err))
		return 0;
	size = sizeof(type) + sizeof(pushdata) + pushdata.length;
	Pos = outbuf = (char*)malloc(size);
	memcpy(Pos,&type,sizeof(type));			Pos += sizeof(type);
	memcpy(Pos,&pushdata,sizeof(pushdata));	Pos += sizeof(pushdata);
	memcpy(Pos, pushbuf,pushdata.length);
	NHPBufFree(&pushbuf);
	length = size;
	return outbuf;
}
bool VS_NHP_Handshake_Impl::GetIsMaster(unsigned char &res)
{
	if(!m_pushdata.length)
		return false;
	else
	{
		res = m_pushdata.master;
		return true;
	}
}
////////////////////////////////////////////////////
bool VS_NHP_Handshake_Impl::GetRndFromSyn(unsigned long &rnd)
{
	rnd = m_syndata.RND;
	return true;
}
void VS_NHP_Handshake_Impl::Lock()
{
	EnterCriticalSection(&nhp_sect);
}
void VS_NHP_Handshake_Impl::Unlock()
{
	LeaveCriticalSection(&nhp_sect);
}
#endif
