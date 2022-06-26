#pragma once

#include "nhp_HSAPI.h"
#include "VS_NHP_Types.h"
#include "../../acs/Lib/VS_AcsLog.h"
#include "acs/connection/VS_ConnectionUDP.h"

#include "Windows.h"

class VS_ConnectionSock;

class VS_NHP_Handshake_Impl
{
protected:
	/////VS_ConnectionSock	*cur_conn;
	VS_ConnectionUDP	*cur_conn;
	char		m_addr_buf[16];
	unsigned short *m_pPort;
	unsigned long	*m_pIP;
	unsigned char	m_recvBuff[0xffff];
	unsigned long	m_buffSz;

	/**
	Надо сделать несколько коннекшенов, которые можно выбирать, чтобы паралельно соединяться к нескольким
	*/
	SYN_TYPE			m_syndata;
	long				num_syn;
	SYN_ACK_TYPE		m_synackdata;
	long				num_synack;
	ACK_TYPE			m_ackdata;
	PUSH_TYPE			m_pushdata;
	char				*m_pushbuf;
	CRITICAL_SECTION	nhp_sect;
public:
//protected:

	void			PrepareSYNBuf(const SYN_TYPE &syndata,char outbuf[SYN_FRAME_SIZE]);
	void			PrepareSynAckBuf(const SYN_TYPE &syndata, char outbuf[]);
	char			*PrepareAckBuf(char  EndpointName[],
									char ConnectedEndpointName[],
									char ConferenceName[],
									unsigned long	IP,
									unsigned short	port,
									SYN_ACK_TYPE	*synakc,
									unsigned char	acknum,
									unsigned char	ackcount,
									unsigned long&	length);

	char			*PreparePUSHBuf(const ACK_TYPE			&ackdata,
									const unsigned long		IP,
									const unsigned short	port,
									const unsigned long		local_IP,
									const unsigned short	local_port,
									const unsigned char		master,
									unsigned long			&length);
	/*bool			PreparePUSH(const ACK_TYPE			&ackdata,
								const unsigned long		IP,
								const unsigned short	port,
								const unsigned char		master,
								PUSH_TYPE				&pushdata,
								char					**pushbuf);*/

	bool			TransformACK(const ACK_TYPE		&ackdata,
								 char				*EndpointName,
								 char				*ConnectedEndpointName,
								 char				*ConferenceName,
								 unsigned long		&IP,
                                 unsigned short		&port);

	bool			TransformPUSH(const PUSH_TYPE	&pushdata,
								const char			*pushbuf,
								unsigned long		&ip,
								unsigned short		&port,
								unsigned long		&local_ip,
								unsigned short		&local_port);
	bool			GetAddressFromPush(unsigned long &ip, unsigned short &port,
										unsigned long &local_ip, unsigned short &local_port);
	bool			GetIsMaster(unsigned char &res);
	bool			GetRndFromSyn(unsigned long &rnd);
	void			Lock();
	void			Unlock();
//public:
	VS_NHP_Handshake_Impl(void);
	virtual ~VS_NHP_Handshake_Impl(void);

	////virtual void NHPInit(VS_ConnectionSock* newconn);
	virtual void NHPInit(VS_ConnectionUDP* newconn);

	virtual void NHPFinal();
	virtual bool IsSynData(const char* data, unsigned long  sz);
	static bool IsSynAckData(const char* data, unsigned long  sz);
	virtual bool IsAckData(const char* data, unsigned long sz);
	virtual bool IsPushData(const char* data, unsigned long sz);

	virtual NHP_PACK_TYPE WhatPacketType(const char* data, unsigned long sz);

	virtual int GetNHPData(char* buf,unsigned long buf_size,unsigned long mills);
	virtual int GetNHPData(char* buf,unsigned long buf_size,unsigned long mills, unsigned long &ip_from, unsigned short &port_from);

	virtual bool GetAck(const unsigned long mills);

	virtual bool SendSYN(const unsigned long mills,bool GenSYN = true);
	virtual bool SendSYN(const unsigned long mills, const char* host_to, const unsigned short port_to, bool GenSYN = true);
	virtual bool SendSYN(const unsigned long mills, const unsigned long ip_to, const unsigned short port_to, bool GenSYN = true);


	virtual bool GetSynAck(const unsigned long mills);

	virtual bool GetSynAck(const unsigned mills, const char *host_from, const unsigned short port_from);


	virtual bool SendAck(char EndpointName[], char ConnectedEndpointName[],char ConferenceName[],const unsigned long mills,SYN_ACK_TYPE	*synakc = 0,unsigned char acknum = 0,unsigned char ackcount = 1);

	virtual bool SendAck(char EndpointName[], char ConnectedEndpointName[],char ConferenceName[],
						const char *host_to, const unsigned short port_to, const unsigned long mills,
						SYN_ACK_TYPE	*synakc = 0,unsigned char acknum = 0,unsigned char ackcount = 1);

	virtual bool GetSynFromBuf(const char* buf,unsigned long sz,SYN_TYPE& syndata);
	static bool GetSynAckFromBuf(const char* buf, unsigned long sz, SYN_ACK_TYPE& synackdata);
	virtual bool GetAckFromBuf(const char* buf,unsigned long sz, ACK_TYPE& ackdata);
	virtual bool GetPushFromBuf(const char* buf, unsigned long sz, PUSH_TYPE& pushdata,char** pushbuf);
	virtual bool SendSYNAck(const SYN_TYPE &syndata, const unsigned long mills);
	virtual bool SendPush(const ACK_TYPE &ackdata,const unsigned char master,unsigned long ip,unsigned short port,
						unsigned long local_IP, unsigned short local_port, unsigned long mills);
	//virtual bool SendPush(const PUSH_TYPE &pushdata, const char* pushbuf, const unsigned long bufsize, const unsigned long mills);

	virtual bool GetPush(const unsigned long mills);
	virtual bool GetPush(const char* host_from, const unsigned short port, const unsigned long mills);

	virtual void GenNewSyn(const unsigned long mills = 0);

};
