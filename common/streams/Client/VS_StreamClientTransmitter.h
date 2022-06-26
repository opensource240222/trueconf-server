
#pragma once

#include "VS_StreamClientReceiver.h"
#include "VS_StreamClientSender.h"
#include "../Handshake.h"

class VS_StreamClientTransmitter : public VS_StreamClientReceiver, public VS_StreamClientSender
{
public:
			VS_StreamClientTransmitter		(stream::ClientType type = stream::ClientType::transmitter);
	virtual	~VS_StreamClientTransmitter		( );
	// addressFamily - one of VS_IPPortAddress::ADDR_* constants.
	bool ConnectUDP(unsigned addressFamily, const char *ip, const unsigned short port, const char* conf_name, const char *our_username, const char*peer_username, void * connectEvent, const unsigned long timeout);
};

class VS_StreamClientRtp : public VS_StreamClientTransmitter
{
public:
	VS_StreamClientRtp() : VS_StreamClient(stream::ClientType::rtp) {}
};

// VS_StreamClientTransmitter class


