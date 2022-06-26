#if defined(_WIN32) // Not ported yet

#include <iostream>
#include <sstream>
#include "VS_NHP_HandshakeHelper.h"

#include "VSClient/VS_Dmodule.h"

#define VS_COUNT_OF_FIRST_SYN_MESS		10
#define VS_COUNT_OF_SYN_ACK_MESS		5
#define VS_COUNT_OF_ACK_MESS			5

//#define NHP_HELPER_DEBUG_LOG_ON

VS_NHP_HandshakeHelper::VS_NHP_HandshakeHelper(const unsigned long servIp, const unsigned short servPort, const unsigned long bind_ip,
											   const unsigned short bind_port, const unsigned long timeout,
											   const char *ourEP, const char *connectedEP,const char *confName,
											   const boost::function<void ()> & srvFoundcb,
											   const boost::function<void (const unsigned long, const unsigned short, const unsigned long /*UIDR*/, const unsigned long /*UIDS*/) >  &connReadycb)

: m_state(e_send_syn),m_isSrvHandshake(true),m_timeout(timeout), m_our_ep(ourEP),m_connected_ep(connectedEP), m_conf_name(confName),
m_fireServerFound(srvFoundcb), m_fireConnectionReady(connReadycb),m_num_syn(0),m_num_synack(0),m_ip(servIp),m_port(servPort),m_bind_ip(bind_ip),m_bind_port(bind_port),
m_ext_ip(0),m_ext_port(0),m_local_ip(0),m_local_port(0),m_master(0),m_found_ip(0),m_found_port(0),m_counter(0),m_UIDR(0),m_UIDS(0)
{
	memset((void*)&m_syndata,0,sizeof(m_syndata));
	memset((void*)&m_peer_syn,0,sizeof(m_peer_syn));
	memset((void*)&m_ack,0,sizeof(m_ack));
}

VS_NHP_HandshakeHelper::~VS_NHP_HandshakeHelper()
{
	if(m_ack.ack_buf.length)
		NHPBufFree(&m_ack.ack_buf);
}

int VS_NHP_HandshakeHelper::GetBufForSend(unsigned long &ip, unsigned short &port, void *buf, unsigned long &sz)
{
	int res(0);
	if(m_isSrvHandshake)
	{
		switch(m_state)
		{
		case e_send_syn:
			res = MakeServerSyn(buf,sz);
			ip = m_ip;
			port = m_port;
			m_state = e_get_synack;
			break;
		case e_send_ack:
			res = MakeServerAck(buf,sz);
			m_state = e_get_push;
			ip = m_ip;
			port = m_port;
			break;
		}
	}
	else
	{
		switch(m_state)
		{
		case e_send_syn_to_internal:
			//send syns to local and external ports n times, but before getting first syn, then send final syn
			if(m_counter<VS_COUNT_OF_FIRST_SYN_MESS)
			{
				//first send to internal then external and then increment counter
				ip = m_local_ip;
				port = m_local_port;
				m_state = e_send_syn_to_ext;
				if((m_local_ip == m_ext_ip) && (m_local_port == m_ext_port))
					m_counter++;
				res = MakeClientSyn(buf,sz);
				{
					unsigned char *chip = (unsigned char*)&ip;
					std::stringstream str;
					str << m_our_ep << "->" << m_connected_ep << " " << "Send Syn to ip = " << (int)chip[3] << '.' << (int)chip[2] << '.' << (int)chip[1] << '.' << (int)chip[0] << " port = " << port << "; e_send_syn_to_internal" << std::endl;
					DTRACE(VSTM_NETWORK, str.str().c_str());
				}
			}
			break;
		case e_send_syn_to_ext:
			if(m_counter<VS_COUNT_OF_FIRST_SYN_MESS)
			{
				//first send to internal then external and then increment counter
				m_counter++;
				ip = m_ext_ip;
				port = m_ext_port;
				m_state = e_send_syn_to_internal;
				res = MakeClientSyn(buf,sz);
				{
					unsigned char *chip = (unsigned char*)&ip;
					std::stringstream str;
					str << m_our_ep << "->" << m_connected_ep << " " << "Syn num = " << (int)m_counter << " to ip = " << (int)chip[3] << '.' << (int)chip[2] << '.' << (int)chip[1] << '.' << (int)chip[0] << " port = " << port << "; e_send_syn_to_ext"<< std::endl;
					DTRACE(VSTM_NETWORK, str.str().c_str());
				}
			}
			break;
		case e_send_syn:
			if(!m_found_ip || !m_found_port)
				return 0;
			ip = m_found_ip;
			port = m_found_port;
			res = MakeClientSyn(buf,sz);
			m_state = m_master ? e_send_synack : e_get_synack;
			m_counter = 0;
			{
				unsigned char *chip = (unsigned char*)&ip;
				std::stringstream str;
				str << m_our_ep << "->" << m_connected_ep << " " << "Last Syn to ip = " << (int)chip[3] << '.' << (int)chip[2] << '.' << (int)chip[1] << '.' << (int)chip[0] << " port = " << port << "; e_send_syn"<<std::endl;
				DTRACE(VSTM_NETWORK, str.str().c_str());
			}
			break;
		case e_send_synack:
			if(m_counter>=VS_COUNT_OF_SYN_ACK_MESS)
				return 0;
			if(sz<SYN_ACK_FRAME_SIZE)
				return 0;
			PrepareSynAckBuf(m_peer_syn,buf);
			m_num_synack++;
			res = sz = SYN_ACK_FRAME_SIZE;
			ip = m_found_ip;
			port = m_found_port;
			m_counter++;
			{
				unsigned char *chip = (unsigned char*)&ip;
				std::stringstream str;
				str << m_our_ep << "->" << m_connected_ep << " " << "SynAck num = " << (int)m_counter << " to ip = " << (int)chip[3] << '.' << (int)chip[2] << '.' << (int)chip[1] << '.' << (int)chip[0] << " port = " << port << "; e_send_synack"<< std::endl;
				DTRACE(VSTM_NETWORK, str.str().c_str());
			}
			break;
		case e_send_ack:
			if(m_counter>=VS_COUNT_OF_ACK_MESS)
				HandshakeOk();
			else
			{
				res = MakeClientAck(buf,sz);
				ip = m_found_ip;
				port = m_found_port;
				m_counter++;
				{
					unsigned char *chip = (unsigned char*)&ip;
					std::stringstream str;
					str << m_our_ep << "->" << m_connected_ep << " " << "Ack num = " << (int)m_counter << " to ip = " << (int)chip[3] << '.' << (int)chip[2] << '.' << (int)chip[1] << '.' << (int)chip[0] << " port = " << port << "; e_send_ack"<< std::endl;
					DTRACE(VSTM_NETWORK, str.str().c_str());
				}
			}
			break;
		}
	}

	return res;
}
bool VS_NHP_HandshakeHelper::SetRecvBuf(const unsigned long from_ip, const unsigned short from_port, const void *buf, const unsigned long sz)
{
	{
		NHP_PACK_TYPE *t = (NHP_PACK_TYPE*)buf;
		unsigned char *chip = (unsigned char*)&from_ip;
		std::stringstream str;
		str<<std::endl<<"Pack received. our ep = "<<m_our_ep.c_str()<<" type = "<<*t<<" sz = "<<sz<<" ip from = "<<(int)chip[3]<<'.'<<(int)chip[2]<<'.'<<(int)chip[1]<<'.'<<(int)chip[0]<<" port from = "<<from_port<<std::endl;
		DTRACE(VSTM_NETWORK, str.str().c_str());
	}
	if(m_isSrvHandshake)
	{
		if(from_ip!=m_ip || from_port != m_port)
			return true; //waiting for next packet
		switch(m_state)
		{
		case e_get_synack:
			if(!CheckSynAck(buf,sz))
				return false;
			m_state = e_send_ack;
			m_fireServerFound();
			return true;
		case e_get_push:
			if(!ParsePushData(buf,sz))
				return false;
			m_isSrvHandshake = false;
			m_state = e_send_syn_to_internal;
			return true;
		}
	}
	else
	{
		if((from_ip != m_local_ip || from_port != m_local_port)&&(from_ip != m_ext_ip   || from_port != m_ext_port)	)
			return true;
		switch(m_state)
		{
			case e_send_syn_to_internal:
			case e_send_syn_to_ext:
				{
					const SYN_TYPE * syn(0);
					if(!IsSyn(buf,sz,syn))
						break;
					m_found_ip = from_ip;
					m_found_port = from_port;
					m_UIDR = syn->RND;
					m_UIDS = m_syndata.RND;
					memcpy_s(&m_peer_syn,sizeof(m_peer_syn),syn,sizeof(SYN_TYPE));
					{
						unsigned char *chip = (unsigned char*)&from_ip;
						std::stringstream str;
						str << m_our_ep << "<-" << m_connected_ep << " Syn ip = " << (int)chip[3] << '.' << (int)chip[2] << '.' << (int)chip[1] << '.' << (int)chip[0] << " port = " << from_port << "; state = " << (m_state == e_send_syn_to_internal ? "e_send_syn_to_internal" :"e_send_syn_to_ext") << std::endl;
						DTRACE(VSTM_NETWORK, str.str().c_str());
					}
					m_state = e_send_syn;
					break;
				}

			case e_send_synack:
				// waiting ack from found address
				if(from_ip != m_found_ip || from_port != m_found_port)
					break;
				if(!IsAck(buf,sz))
					break;
				{
					unsigned char *chip = (unsigned char*)&from_ip;
					std::stringstream str;
					str << m_our_ep << "<-" << m_connected_ep << " Ack ip = " << (int)chip[3] << '.' << (int)chip[2] << '.' << (int)chip[1] << '.' << (int)chip[0] << " port = " << from_port << "; e_send_synack"<<std::endl;
					DTRACE(VSTM_NETWORK, str.str().c_str());
				}
				HandshakeOk();
				break;
			case e_send_syn:
				if(m_master)
					break;
			case e_get_synack:
				if(from_ip != m_found_ip || from_port != m_found_port)
					break;
				if(!CheckSynAck(buf,sz))
					break;
				m_counter = 0;
				m_state = e_send_ack;
				{
					unsigned char *chip = (unsigned char*)&from_ip;
					std::stringstream str;
					str << m_our_ep << "<-" << m_connected_ep << " SynAck ip = " << (int)chip[3] << '.' << (int)chip[2] << '.' << (int)chip[1] << '.' << (int)chip[0] << " port = " << from_port << "; e_get_synack"<< std::endl;
					DTRACE(VSTM_NETWORK, str.str().c_str());
				}
				break;
		}
		return true;
	}

	return false;
}
bool VS_NHP_HandshakeHelper::IsSyn(const void *buf, const unsigned long sz, const SYN_TYPE * &syn)
{
	if(sz!=SYN_FRAME_SIZE)
		return false;
	const NHP_PACK_TYPE	*type = (const NHP_PACK_TYPE*)buf;
	if(SYN != *type)
		return false;
	syn = (const SYN_TYPE*)((const char*)buf + sizeof(NHP_PACK_TYPE));
	if(NHPGetCurrentVersion()!=NHPGetSynVersion(syn))
		return false;
	else
		return true;

}
bool VS_NHP_HandshakeHelper::IsAck(const void *buf, const unsigned long sz)
{
	if(sz<sizeof(NHP_PACK_TYPE))
		return false;
	const NHP_PACK_TYPE	*type = (const NHP_PACK_TYPE*) buf;
	if(*type!=ACK)
		return false;
	return true;
}
int VS_NHP_HandshakeHelper::MakeClientSyn(void *buf,unsigned long &sz)
{
	int err(0);
	if(sz<SYN_FRAME_SIZE)
		return 0;
	PrepareSYNBuf(m_syndata,buf);
	m_num_syn++;
	sz = SYN_FRAME_SIZE;
	return sz;
}
int VS_NHP_HandshakeHelper::MakeServerSyn(void *buf, unsigned long &sz)
{
	int err(0);
	if(sz<SYN_FRAME_SIZE)
		return 0;
	if(!NHPSyn(m_timeout,&m_syndata,&err))
		return 0;
	PrepareSYNBuf(m_syndata,buf);
	m_num_syn++;
	sz = SYN_FRAME_SIZE;
	return sz;
}
int VS_NHP_HandshakeHelper::MakeServerAck(void *buf, unsigned long &sz)
{
	return PrepareAckBuf(buf,sz);
}
int VS_NHP_HandshakeHelper::MakeClientAck(void *buf, unsigned long &sz)
{
	return PrepareAckBuf(buf,sz,m_counter,VS_COUNT_OF_ACK_MESS);
}

void VS_NHP_HandshakeHelper::PrepareSYNBuf(const SYN_TYPE &syndata,void *outbuf)
{
	NHP_PACK_TYPE	type = SYN;
	char* Pos = (char*)outbuf;
	m_syndata.field2 = m_num_syn;
	memcpy(Pos,&type,sizeof(type));							Pos += sizeof(type);
	memcpy(Pos,&syndata.RND,sizeof(syndata.RND));			Pos += sizeof(syndata.RND);
	memcpy(Pos,syndata.Message,MESSAGE_SIZE);				Pos += MESSAGE_SIZE;
	memcpy(Pos,&syndata.field1,sizeof(syndata.field1));		Pos +=sizeof(syndata.field1);
	memcpy(Pos,&syndata.field2,sizeof(syndata.field2));		Pos += sizeof(syndata.field2);
	memcpy(Pos,&syndata.field3,sizeof(syndata.field3));		Pos += sizeof(syndata.field3);
}
void VS_NHP_HandshakeHelper::PrepareSynAckBuf(const SYN_TYPE &syndata, void* outbuf)
{
	int				err;
	SYN_ACK_TYPE	synack;
	NHP_PACK_TYPE	type = SYN_ACK;

	memset(outbuf,0,SYN_ACK_FRAME_SIZE);
	if(!NHPTransformSYN(&syndata,&synack,&err))	return;
	synack.field2 = m_num_synack;
	char *Pos = (char*)outbuf;
	memcpy(Pos,&type,sizeof(type));						Pos += sizeof(type);
	memcpy(Pos,&synack.SEQ,sizeof(synack.SEQ));			Pos += sizeof(synack.SEQ);
	memcpy(Pos,synack.synhash,HASH_SIZE);				Pos += HASH_SIZE;
	memcpy(Pos,&synack.field1,sizeof(synack.field1));	Pos += sizeof(synack.field1);
	memcpy(Pos,&synack.field2,sizeof(synack.field2));	Pos += sizeof(synack.field2);
	memcpy(Pos,&synack.field3,sizeof(synack.field3));	Pos += sizeof(synack.field3);
}

int VS_NHP_HandshakeHelper::PrepareAckBuf(void *outbuf,unsigned long &sz, const unsigned char acknum, const unsigned char ackcount)
{
	char*			Pos(0);
	const NHP_PACK_TYPE	type = ACK;
	int				err(0);
	unsigned long	size(0);
	if(!m_ack.ack_buf.length)
		if(!NHPAck(m_our_ep.c_str(),m_connected_ep.c_str(),m_conf_name.c_str(),m_bind_ip,m_bind_port,&m_ack,&err))
			return 0;


	size = sizeof(type) + 4/*sizeof(size)*/ + 1/*sizeof(ackcount)*/ + 1/*sizeof(acknum)*/ + 4/*sizeof(ack_buf.IP)*/ + 2/*sizeof(ack_buf.port)*/ + 4/*sizeof(ack_buf.ack_buf.length)*/ +
		m_ack.ack_buf.length;
	if(size>sz)
	{
		sz = size;
		NHPBufFree(&m_ack.ack_buf);
		memset(&m_ack,0,sizeof(m_ack));
		return 0;
	}
	Pos = (char*)outbuf;
	memcpy(Pos,&type,sizeof(type));								Pos += sizeof(type);
	memcpy(Pos,&size,4);										Pos += 4;
	memcpy(Pos,&acknum,1);										Pos += 1;
	memcpy(Pos,&ackcount,1);									Pos += 1;
	memcpy(Pos,&m_ack.IP,4);									Pos += 4;
	memcpy(Pos,&m_ack.port,2);									Pos += 2;
	memcpy(Pos,&m_ack.ack_buf.length,4);						Pos += 4;
	memcpy(Pos,m_ack.ack_buf.buf,m_ack.ack_buf.length);			Pos += m_ack.ack_buf.length;
	sz = size;
	return size;
}
bool VS_NHP_HandshakeHelper::CheckSynAck(const void *buf, const unsigned long sz)
{
	/**
		GetSynAck
		Check Version
		Validate synack;
	*/
	if(sz<SYN_ACK_FRAME_SIZE)
		return false;
	const NHP_PACK_TYPE	*type = (const NHP_PACK_TYPE*)buf;
	if(*type!=SYN_ACK)
		return false;
	SYN_ACK_TYPE *synackdata = (SYN_ACK_TYPE*)((const char*)buf + sizeof(NHP_PACK_TYPE));
	if(NHPGetCurrentVersion()!=synackdata->field3)
		return false;
	return NHPSynAckIsValid(synackdata,&m_syndata);
}
bool VS_NHP_HandshakeHelper::ParsePushData(const void *buf, const unsigned long sz)
{
	/**
		check pack_type

	*/
	unsigned long limit_sz(sizeof(NHP_PACK_TYPE)+sizeof(PUSH_TYPE));
	if(sz<limit_sz)
		return false;
	const NHP_PACK_TYPE	*type = (const NHP_PACK_TYPE*)buf;
	if(*type!=PUSH)
		return false;
	const char *pos = (const char*)buf + sizeof(NHP_PACK_TYPE);
	PUSH_TYPE* push = (PUSH_TYPE*)pos;	pos+=sizeof(PUSH_TYPE);
	limit_sz+= push->length;
	if(sz!=limit_sz)
		return false;
	if(!NHPPushIsValid(push,&m_ack))
		return false;

	char *push_buf(0);
	NHPPrepareBuf(&push_buf,push->length);
	memcpy(push_buf,pos,push->length);
	/**
		Get ip from push
	*/
	int err(0);
	bool res = NHPGetPushInfo(push,push_buf,&m_ext_ip,&m_ext_port,&m_local_ip,&m_local_port,&err);
	m_master = push->master;
	DTRACE(VSTM_NETWORK, "we are %s", m_master ? "master" : "slave");
	NHPBufFree(&push_buf);
	return res;
}
void VS_NHP_HandshakeHelper::HandshakeOk()
{
	Finish();
	m_fireConnectionReady(m_found_ip,m_found_port,m_UIDR,m_UIDS);
}
void VS_NHP_HandshakeHelper::Finish()
{
	m_state = e_finish;
}

#endif
