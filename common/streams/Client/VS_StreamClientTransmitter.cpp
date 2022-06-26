#if defined(_WIN32) // Not ported yet

#include "VS_StreamClientTransmitter.h"
#include "VS_StreamClientTypes.h"

VS_StreamClientTransmitter::VS_StreamClientTransmitter(stream::ClientType type) :
VS_StreamClient(type == stream::ClientType::transmitter ? stream::ClientType::transmitter : stream::ClientType::rtp) {}
// end VS_StreamClientTransmitter::VS_StreamClientTransmitter

VS_StreamClientTransmitter::~VS_StreamClientTransmitter( void ) {}
// end VS_StreamClientTransmitter::~VS_StreamClientTransmitter

bool VS_StreamClientTransmitter::ConnectUDP(unsigned addressFamily, const char *ip, const unsigned short port, const char* conf_name, const char *our_username, const char*peer_username, void * connectEvent, const unsigned long timeout)
{
	return !imp ? 0 : imp->ConnectDirectUDP(addressFamily, ip,port,conf_name, our_username, peer_username, connectEvent,timeout);
}

#endif
