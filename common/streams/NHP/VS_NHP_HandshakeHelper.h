#pragma once

#include "VS_NHP_Types.h"

#include <boost/function.hpp>

class VS_NHP_HandshakeHelper
{
	boost::function<void ()> m_fireServerFound;
	boost::function<void (const unsigned long, const unsigned short, const unsigned long /*UIDR*/, const unsigned long /*UIDS*/) > m_fireConnectionReady;
	enum
	{
		e_send_syn,
		e_get_synack,
		e_send_synack,
		e_send_ack,
		e_get_push,
		e_send_syn_to_internal,
		e_send_syn_to_ext,

		e_finish
	}
	m_state;
	unsigned char m_counter;

	bool m_isSrvHandshake;
	unsigned long m_timeout;
	unsigned long m_ip;
	unsigned short m_port;

	//candidate address
	unsigned long m_ext_ip;
	unsigned short m_ext_port;
	unsigned long m_local_ip;
	unsigned short m_local_port;

	//real addresss
	unsigned long m_found_ip;
	unsigned short m_found_port;
	unsigned long m_UIDR,m_UIDS;

	unsigned char m_master;

	unsigned long	m_bind_ip;
	unsigned short	m_bind_port;
	std::string	m_our_ep;
	std::string m_connected_ep;
	std::string	m_conf_name;

	SYN_TYPE	m_syndata;
	SYN_TYPE	m_peer_syn;
	ACK_TYPE	m_ack;
	long		m_num_syn;
	long		m_num_synack;

	/// make message
	int		MakeServerSyn(void *buf, unsigned long &sz);
	int		MakeClientSyn(void *buf, unsigned long &sz);
	int		MakeServerAck(void *buf, unsigned long &sz);
	int		MakeClientAck(void *buf, unsigned long &sz);
	bool	CheckSynAck(const void *buf,const unsigned long sz);
	bool	ParsePushData(const void *buf, const unsigned long sz);
	void	PrepareSYNBuf(const SYN_TYPE &syndata,void *outbuf);
	void	PrepareSynAckBuf(const SYN_TYPE &syndata, void *outbuf);
	int		PrepareAckBuf(void *outbuf, unsigned long &sz,const unsigned char acknum = 0, const unsigned char ackcount = 1);
	bool	IsSyn(const void *buf, const unsigned long sz, const SYN_TYPE* &syn);
	bool	IsAck(const void *buf, const unsigned long sz);

	void HandshakeOk();

public:
	VS_NHP_HandshakeHelper(const unsigned long servIp, const unsigned short servPort,
							const unsigned long bind_ip,const unsigned short bind_port,
							const unsigned long timeout,
							const char *ourEP, const char *connectedEP,const char *confName,
							const boost::function<void ()> & srvFoundcb, const boost::function<void (const unsigned long, const unsigned short, const unsigned long /*UIDR*/, const unsigned long /*UIDS*/) >  &connReadycb);
	virtual ~VS_NHP_HandshakeHelper();
	bool SetRecvBuf(const unsigned long from_ip, const unsigned short from_port, const void *buf, const unsigned long sz);
	int GetBufForSend(unsigned long &ip, unsigned short &pot, void *buf, unsigned long &sz);
	void Finish();
	bool IsFinished() const
	{
		return m_state == e_finish || (!m_isSrvHandshake && m_master==0 && m_state == e_send_ack);
	}
};