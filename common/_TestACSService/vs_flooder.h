#pragma once

#ifndef _VS_Flooder_H
#define _VS_Flooder_H

#include "VS_TestParticipant.h"
#include "VS_PacketGenerator.h"
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

class VS_TestParticipant;

class VS_Flooder
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
	CRITICAL_SECTION m_vcs;		// critical section for vector operations
	signed long m_msg_counter;	// flood message counter
	unsigned long m_inc_timer;	// timer 4 flood increment
	signed long m_flood_sleep;	// flood sleep parameter
	signed long m_flood_mult;	// flood mult parameter
	signed long m_flood_mult_div;	// flood mult parameter
	signed long m_flood_mult_mult;	// flood mult parameter
	signed long m_flood_inc;	// flood_inc parameter
	unsigned long m_after_time; // flood inc after time parameter
	unsigned long m_npacket;
	char ___buffer[BUF_SIZE * 2];
	unsigned long m_startytime;
	unsigned long m_msgcount;
	WSABUF ___wbuf;
	WSAOVERLAPPED ___wsaov;

public:
	std::vector<void **> m_listPeers;
	static volatile signed long m_clients;
	static volatile signed long m_terminate;
	static HANDLE m_hTIMER;	// used as sleep replace
	VS_Flooder(const char *tAddress, const signed long nConn, const signed long flood_sleep, const signed long flood_mult, const signed long flood_inc = 1000, const signed long after_time = 10000);
	virtual ~VS_Flooder(void);
};

#endif