#include "VS_NetworkRelayMessage.h"
#include "VS_ProtocolConst.h"
#include "../streams/VS_StreamsDefinitions.h"
#include "../net/Handshake.h"

#include <cstring>

#define VS_REMOTE_CIRCUIT_AUTH_BUF_LEN 128
namespace
{
#pragma pack (1)
	struct RemoteCircuitStartPack
	{
		net::HandshakeHeader hs;
		unsigned char				auth_data[VS_REMOTE_CIRCUIT_AUTH_BUF_LEN];
	};
	struct RemoteFrameTransmitStartPack : public RemoteCircuitStartPack
	{
		char conference_name[VS_STREAMS_MAX_SIZE_CONFERENCE_NAME + 1];
	};

#pragma pack ( )

}

VS_NetworkRelayMessageBase::VS_NetworkRelayMessageBase() : m_mess(new std::vector<unsigned char>),m_mess_size(0),m_isComplete(false)
{}
bool VS_NetworkRelayMessageBase::Empty() const
{
	return m_mess->empty();
}

bool VS_NetworkRelayMessageBase::SetMessage(const boost::shared_ptr<std::vector<unsigned char> > &mess)
{
	m_mess = mess;
	m_mess_size = m_mess->size();
	m_isComplete = true;		// kt: set temporary, beause IsValid() need it to set correct value
	return IsValid();
}
bool VS_NetworkRelayMessageBase::SetMessage(const unsigned char *buf, const unsigned long sz)
{
	m_mess->insert(m_mess->begin(),buf,buf+sz);
	m_mess_size = m_mess->size();
	return true;
}

boost::shared_ptr<std::vector<unsigned char>> VS_NetworkRelayMessageBase::GetMess() const
{
	return m_mess;
}
const unsigned char *VS_NetworkRelayMessageBase::GetMess(unsigned long &sz) const
{
	sz = m_mess_size;//m_mess->size();
	return &(*m_mess)[0];
}
bool VS_NetworkRelayMessageBase::IsComplete() const
{
	return m_isComplete;
}
void VS_NetworkRelayMessageBase::SetReadBytes(const unsigned long received_bytes)
{
	m_mess_size += received_bytes;
	m_mess->resize(m_mess_size);
}

void VS_NetworkRelayMessageBase::Clear()
{
	m_mess_size = 0;
	m_mess->resize(0);
}


VS_StartControlMess::VS_StartControlMess(const unsigned char *auth_buf, const unsigned long sz)
{
	if(sz>VS_REMOTE_CIRCUIT_AUTH_BUF_LEN)
		return;
	m_mess->resize(sizeof(RemoteCircuitStartPack));

	RemoteCircuitStartPack *start_pack = reinterpret_cast<RemoteCircuitStartPack *>(&(*m_mess)[0]);
	memcpy(start_pack->auth_data,auth_buf,sz);
	strncpy(start_pack->hs.primary_field, VS_Circuit_PrimaryField, sizeof(start_pack->hs.primary_field));
	start_pack->hs.version = 1;
	start_pack->hs.body_length = sizeof(RemoteCircuitStartPack) - sizeof(net::HandshakeHeader);
	// net::UpdateHandshakeChecksums thinks that body is 1 byte longer than what is specified in net::HandshakeHeader.
	// This can't be fixed without breaking backward compatibility, so to avoid accessing unallocated memory
	// we have to decrease body length in header by 1, receivers must take that into account.
	start_pack->hs.body_length -= 1;
	net::UpdateHandshakeChecksums(start_pack->hs);
	m_mess_size = m_mess->size();
}

VS_StartFrameTransmitterMess::VS_StartFrameTransmitterMess(const unsigned char *auth_buf, const unsigned long sz, const char *conf_name)
{
	if(sz>VS_REMOTE_CIRCUIT_AUTH_BUF_LEN || !conf_name || !*conf_name || strlen( conf_name ) > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME)
		return;
	m_mess->resize(sizeof(RemoteFrameTransmitStartPack));
	RemoteFrameTransmitStartPack *start_pack = reinterpret_cast<RemoteFrameTransmitStartPack *>(&(*m_mess)[0]);
	memset(start_pack,0,sizeof(RemoteFrameTransmitStartPack));
	memcpy(start_pack->auth_data,auth_buf,sz);
	strncpy(start_pack->conference_name, conf_name, sizeof(start_pack->conference_name));
	strncpy(start_pack->hs.primary_field, VS_FrameTransmit_PrimaryField, sizeof(start_pack->hs.primary_field));
	start_pack->hs.version = 1;
	start_pack->hs.body_length = sizeof(RemoteFrameTransmitStartPack) - sizeof(net::HandshakeHeader);
	// net::UpdateHandshakeChecksums thinks that body is 1 byte longer than what is specified in net::HandshakeHeader.
	// This can't be fixed without breaking backward compatibility, so to avoid accessing unallocated memory
	// we have to decrease body length in header by 1, receivers must take that into account.
	start_pack->hs.body_length -= 1;
	net::UpdateHandshakeChecksums(start_pack->hs);
	m_mess_size = m_mess->size();
}
const char *VS_StartFrameTransmitterMess::GetConferenceName() const
{
	if(m_mess->size() < sizeof(RemoteFrameTransmitStartPack))
		return 0;
	RemoteFrameTransmitStartPack *start_pack = reinterpret_cast<RemoteFrameTransmitStartPack *>(&(*m_mess)[0]);
	if (start_pack->hs.head_cksum != net::GetHandshakeHeaderChecksum(start_pack->hs))
		return 0;
	return start_pack->conference_name;
}
const unsigned char *VS_StartFrameTransmitterMess::GetAuthData(unsigned long &sz) const
{
	if(m_mess->size() < sizeof(RemoteFrameTransmitStartPack))
		return 0;
	RemoteFrameTransmitStartPack *start_pack = reinterpret_cast<RemoteFrameTransmitStartPack *>(&(*m_mess)[0]);
	if (start_pack->hs.head_cksum != net::GetHandshakeHeaderChecksum(start_pack->hs))
		return 0;
	sz = VS_REMOTE_CIRCUIT_AUTH_BUF_LEN;
	return start_pack->auth_data;
}