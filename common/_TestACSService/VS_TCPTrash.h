#pragma once

#ifndef _VS_TCPTRASH_H
#define _VS_TCPTRASH_H

#include "VS_Scope.h"

#include <windows.h>
#include <iphlpapi.h>
#include <iprtrmib.h>

#define TRASH_TIMEOUT	5000
#define TRASH_SIZE		0x20000

class VS_TCPTrash
{
private:
	static VS_TCPTrash *m_hInstance;
	static signed long m_requestCount;
	static HANDLE m_hWTRASH;	// trash wait timer
	HANDLE m_hTTRASH;			// trash thread
	_MIB_TCPTABLE *m_pTcpTable;
	VS_TCPTrash(void);
	virtual ~VS_TCPTrash(void);
	static DWORD WINAPI garbageThread(void *arg);
	DWORD doGarbage();
public:
	static VS_TCPTrash *GetInstance();
	volatile static signed long m_terminate;
	void FreeInstance();
};

#endif