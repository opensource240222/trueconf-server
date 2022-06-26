#pragma once

#ifndef _VS_SCOPE_H
#define _VS_SCOPE_H

#include "VS_TestParticipant.h"
#include "VS_PacketGenerator.h"
#include "VS_CoolTimer.h"
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

class VS_TestParticipant;

class VS_Scope
{
public:
	char *m_addr; char *m_ip; char *m_port; unsigned long m_size; addrinfo *m_addrinfo;
	HANDLE m_hSYNCEVENT;// used as sleep replace for thread;
    HANDLE m_hCThread;  // connection thread handle
    DWORD  m_idCThread; // connection thread id
	HANDLE m_hDThread;	// receiver thread handle
	DWORD  m_idDThread;	// receiver thread id
	HANDLE m_hZThread;	// control thread handle
	DWORD  m_idZThread;	// receiver thread id
	HANDLE m_hBThread;	// control thread handle
	DWORD  m_idBThread;	// receiver thread id
	static HANDLE m_hIOCP;	// IOCP handle
	static HANDLE m_hCCCP;	// IOCP Control Code Handle
    static DWORD WINAPI connThread(void *arg);
    DWORD doConnect();
	static DWORD WINAPI processThread(void *arg);
	DWORD doProcess();
	static DWORD WINAPI controlThread(void *arg);
	DWORD doControl();
	static DWORD WINAPI rebootThread(void *arg);
	DWORD doReboot();
	VS_PacketGenerator *m_pg;
	CRITICAL_SECTION m_vcs; // critical section for vector operations
public:
	VS_CoolTimer timer;
	std::vector<void **> m_listPeers;
	static volatile signed long m_clients;
	static volatile signed long m_terminate;
	static HANDLE m_hTIMER;	// used as sleep replace
	VS_Scope(const char *tAddress, const signed long nConn, const signed long tSleep);
	virtual ~VS_Scope(void);
};

#endif