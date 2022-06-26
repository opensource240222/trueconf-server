#pragma once

#ifndef _VS_PACKETGENERATOR_H_
#define _VS_PACKETGENERATOR_H_

#include <windows.h>
#include "_testacsinclude.h"

class VS_Socket_Handle;
class VS_PacketGenerator : public VS_TransportMessage {
private :
	static VS_PacketGenerator *m_hInstance;
	static signed long m_requestCount;
	CRITICAL_SECTION m_pgcs; // packetgenerator critical section
	CRITICAL_SECTION m_ccs;  // control critical section
	VS_PacketGenerator();
	virtual ~VS_PacketGenerator	();
public  :
	void MakeMsg_TransportPing	(VS_Socket_Handle &sh);
	void MakeMsg_ServicePing	(VS_Socket_Handle &sh);
	void MakeMsg_ServiceChat	(VS_Socket_Handle &sh);
	signed long TryConnectParticipant (VS_TestParticipant &tp);
	signed long TryReconnectParticipant (VS_TestParticipant &tp, void *iocp_key);
	signed long DeleteParticipant (VS_TestParticipant **&tp);
	static VS_PacketGenerator *GetInstance();
	void FreeInstance();
	bool __sendClientSecureHandshake(VS_Socket_Handle &sh);
	bool __recvClientSecureHandshake(VS_Socket_Handle &sh);
};

#endif