#include "_testacsconst.h"
#include "VS_Flooder.h"
#include <algorithm>

HANDLE VS_Flooder::m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0); // main completion port
HANDLE VS_Flooder::m_hCCCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0); // control code completion port
HANDLE VS_Flooder::m_hTIMER = CreateEvent(NULL, true, false, NULL);
volatile signed long VS_Flooder::m_clients = 0;		// total online participants counter
volatile signed long VS_Flooder::m_terminate = 0;	// treminate threads flag

VS_Flooder::VS_Flooder (const char *tAddress, const signed long nConn, const signed long flood_sleep, const signed long flood_mult, const signed long flood_inc, const signed long after_time)
: m_pg(VS_PacketGenerator::GetInstance()), m_flood_sleep(flood_sleep), m_flood_mult(flood_mult), m_flood_inc(flood_inc), m_after_time(after_time),
  m_msg_counter(0), m_inc_timer(GetTickCount()), m_npacket(0), m_startytime(GetTickCount()), m_msgcount(0) {
	if (m_flood_sleep == INFINITE) { return; }
	___wbuf.buf = (char *)___buffer;
	___wbuf.len = BUF_SIZE * 2;
	m_flood_mult_div = flood_mult / 50;
	m_addr = _strdup(tAddress); char *_s = strchr(m_addr, ':'); m_ip = m_addr;
	m_port = _s + 1; memset(_s, 0, 1); addrinfo _ai = {}; m_addrinfo = NULL; m_size = 1; // currently one flood client
	_ai.ai_family   = AF_INET; _ai.ai_socktype = SOCK_STREAM; _ai.ai_protocol = IPPROTO_TCP;
	if (getaddrinfo(m_ip, m_port, &_ai, &m_addrinfo)) {
		printf("\nError : getaddrinfo failed");
		return;
	}
	if (!nConn) {
		printf("\nError : participant numbers = %d", nConn);
		return;
	}
	m_listPeers.clear(); m_listPeers.reserve(nConn);
	InitializeCriticalSection(&m_vcs);
	m_hSYNCEVENT = CreateEvent(NULL, true, true, NULL);
	if (!m_hSYNCEVENT) {
		printf("\nError : m_heCEVENT creation failed");
		return;
	}
	m_hCThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)connThread, this, 0, &m_idCThread);
	if (!m_hCThread) {
		printf("\nError : m_hCThread creation failed");
		return;
	}
	m_hDThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)processThread, this, 0, &m_idDThread);
	if (!m_hDThread) {
		printf("\nError : m_hDThread creation failed");
		return;
	}
	m_hZThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)controlThread, this, 0, &m_idZThread);
	if (!m_hZThread) {
		printf("\nError : m_hZThread creation failed");
		return;
	}
	m_hBThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)rebootThread, this, 0, &m_idBThread);
	if (!m_hBThread) {
		printf("\nError : m_hZThread creation failed");
		return;
	}
	return;
}

VS_Flooder::~VS_Flooder(void) {
	DeleteCriticalSection(&m_vcs);
}

DWORD WINAPI VS_Flooder::connThread(void *arg) {
	return ((VS_Flooder *)arg)->doConnect();
}

DWORD VS_Flooder::doConnect() {
	WaitForSingleObject(m_hSYNCEVENT, INFINITE);
	while (true) {
		if (InterlockedExchange(&m_terminate, m_terminate)) {
			::printf("\nExitting thread %d", m_hCThread);
			break;
		}
		while (m_listPeers.size() < m_size) {
			WaitForSingleObject(m_hTIMER, AppMode.sleep_connect);
			if (InterlockedExchange(&m_terminate, m_terminate)) {
				::printf("\nExitting thread %d", m_hCThread);
				break; break;
			}
			void **iocp_key = new (void *); *iocp_key = NULL;
			VS_TestParticipant *newpart = new VS_TestParticipant(m_hIOCP, *(m_addrinfo->ai_addr), &*iocp_key);
			*iocp_key = newpart;
			if (m_pg->TryConnectParticipant(*newpart)) { delete newpart; continue; } // != 0 means error
			if (AppMode.ssl) {
				AppMode.ssl = m_pg->__recvClientSecureHandshake(newpart->recvSock);
				if (AppMode.onesocket) {
					newpart->sendSock.m_prCrypt = newpart->recvSock.m_prCrypt;
					newpart->sendSock.m_pwCrypt = newpart->recvSock.m_pwCrypt;
				} else {
					AppMode.ssl &= m_pg->__sendClientSecureHandshake(newpart->sendSock);
				}
			}
			InterlockedIncrement(&m_clients);
			EnterCriticalSection(&m_vcs);
			m_listPeers.push_back(&(*iocp_key));
			LeaveCriticalSection(&m_vcs);
			PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)&(*iocp_key), (LPOVERLAPPED)(&(*newpart).dumbSock));
		} WaitForSingleObject(m_hTIMER, 1 * (60/2) * 1000); // once per 30 seconds
	}
	ExitThread(0);
	return 0;
}

DWORD WINAPI VS_Flooder::processThread(void *arg) {
	return ((VS_Flooder *)arg)->doProcess();
};

DWORD VS_Flooder::doProcess() {
	WaitForSingleObject(m_hSYNCEVENT, INFINITE);
	VS_TestParticipant **__prevpart = NULL;
	while (m_hIOCP) {
        DWORD __bytesTransferred = 0; VS_TestParticipant **__part = NULL; LPOVERLAPPED __lp = NULL;
		if (GetQueuedCompletionStatus(m_hIOCP , &__bytesTransferred, (PULONG_PTR)&__part, &__lp, INFINITE) == 0) {
			if ((__lp) && (__part) && (*__part) && (!(*__part)->m_ctrl)) {
				InterlockedIncrement(&(*__part)->m_ctrl);
				(*__part)->recvSock.m_lastError ++;
				(*__part)->sendSock.m_lastError ++;
				(*__part)->ctrlSock.m_op_mode = iocp_Delete;
				PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)__part, (LPOVERLAPPED)(&(*__part)->ctrlSock));
				continue;
			} else {
				::printf("\nError :: IOCP failed :: %d", GetLastError());
				PostQueuedCompletionStatus(m_hIOCP, 0, NULL, NULL);
				break;
			}
		}
		if (InterlockedExchange(&m_terminate, m_terminate)) {
			::printf("\nExitting thread %d", m_hDThread);
			PostQueuedCompletionStatus(m_hIOCP, 0, NULL, NULL);
			break;
		}
		if (!((__bytesTransferred) || (__part) || (__lp))) {
			::printf("\nInformation :: IOCP closed :: %d", GetLastError());
			PostQueuedCompletionStatus(m_hIOCP, 0, NULL, NULL);
			break;
		}
		if (!__part) {
			continue;
		}
		if (!(((*__part)->recvSock.m_wsaBuf.buf) || ((*__part)->sendSock.m_wsaBuf.buf))) {
			continue;
		}
/*
		if ((__bytesTransferred == 0)) {
			__bytesTransferred == 0 means Socket error but
			we use this for PostQueuedCompletionStatus exchange,
			because any error detected via WSASend è WSARecv
		}
*/
		if (((*__part)->m_ctrl) && ((*__part)->ctrlSock.m_op_mode <= 0xFF)) { continue; } // socket in control thread. drop event and and continue
		while (InterlockedExchange(&(*__part)->m_locked, 1) || InterlockedExchange(&m_terminate, m_terminate)) {}; // lock participant. this takes less resources then critical section
		VS_Socket_Handle *socket = (VS_Socket_Handle *)__lp; DWORD flags(0); DWORD bytes_transferred(0);
		if ((!socket) || ((socket->m_op_mode != iocp_Nop) && (!socket->m_packet))) { InterlockedExchange(&(*__part)->m_locked, 0); continue; } // strange behavior (parallel exec?) detected. unlock, drop event and continue
		switch (socket->m_op_mode) {
			case iocp_Read :	{ // read answer empty for flooders
				break;
								}
			case iocp_Nop :		{ // prepare new packet
				++m_msg_counter;
				if (m_flood_mult_div) {
					--m_flood_mult_div;
				} else {
					WaitForSingleObject(m_hTIMER, 20);
					m_flood_mult_div = m_flood_mult / 50;
				}
				if (m_msg_counter == m_flood_mult) {
					m_msg_counter = 0; m_npacket = 0;
					signed long delta = m_flood_sleep - (GetTickCount() - m_startytime);
					if (delta < 20) delta = 20;
					WaitForSingleObject(m_hTIMER, delta);
					m_startytime = GetTickCount();
				}
				if ((GetTickCount() - m_inc_timer) >= m_after_time) {
					m_inc_timer = GetTickCount();
					m_flood_mult += m_flood_inc;
					printf("\nInformation : floders count increased to %d, msgs send %d", m_flood_mult, m_msgcount);
					m_msgcount = 0;
					m_msg_counter = 0; m_npacket = 0;
					WaitForSingleObject(m_hTIMER, 20);
					m_startytime = GetTickCount();
				}
				(*__part)->sendSock.m_Ov = WSAOVERLAPPED();
				(*__part)->sendSock.m_wsaBuf.buf = (*__part)->sendSock.m_packet->m_data;
				(*__part)->sendSock.m_wsaBuf.len = (*__part)->sendSock.m_packet->m_transferred = (*__part)->sendSock.m_packet->m_total = 0;
				switch (AppMode.type_service) {
					case 0 :	{ // TRANSPORT_PING
						m_pg->MakeMsg_TransportPing((*__part)->sendSock);
						break;
								}
					case 1 :	{ // SERVICE_PING
						m_pg->MakeMsg_ServicePing((*__part)->sendSock);
						break;
								}
					case 2 :	{ // SERVICE_CHAT
						m_pg->MakeMsg_ServiceChat((*__part)->sendSock);
						break;
								}
					default :	{ // WRONG_SERVICE?
								}
				}
				if ((*__part)->sendSock.m_openSSL) {
					unsigned char *memcrypt = new unsigned char[BUF_SIZE]; unsigned long __len = BUF_SIZE;
					if (memcrypt) {
						if ((*__part)->sendSock.m_pwCrypt->Encrypt((const unsigned char *)(*__part)->sendSock.m_wsaBuf.buf, (*__part)->sendSock.m_wsaBuf.len, memcrypt, &__len)) {
							memcpy((void *)((*__part)->sendSock.m_wsaBuf.buf), memcrypt, __len);
							(*__part)->sendSock.m_wsaBuf.len = __len;
						}
						delete [] memcrypt;
					}
				}
				PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)__part, (LPOVERLAPPED)(&(*__part)->sendSock));
				break;
								}
			case iocp_Write :	{ // write packet
				socket->m_packet->m_transferred += __bytesTransferred;
				if (socket->m_packet->m_transferred < socket->m_packet->m_total) {
					socket->m_wsaBuf.buf += __bytesTransferred; socket->m_wsaBuf.len -= __bytesTransferred;
					int _delta = GetTickCount() - socket->m_lastAccTime;
					if ((0) && (_delta < AppMode.sleep_time)) {
						PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)__part, (LPOVERLAPPED)(&(*__part)->sendSock));
					} else {
						socket->m_lastAccTime = GetTickCount();
						if (WSASend(socket->m_Socket, &socket->m_wsaBuf, 1, &bytes_transferred, flags, (LPOVERLAPPED)(&(*__part)->sendSock), NULL) == SOCKET_ERROR) {
							signed long _err = WSAGetLastError();
							if (!((_err == WSA_IO_PENDING) || (_err == WSAENOBUFS))) {
								InterlockedIncrement(&(*__part)->m_ctrl);
								socket->m_lastError ++;
								(*__part)->ctrlSock.m_op_mode = iocp_Reconnect;
								PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)__part, (LPOVERLAPPED)(&(*__part)->ctrlSock));
								::printf("\nError :: WSASend error :: %d", _err);
								CancelIo((HANDLE)socket->m_Socket);
							}
						}
					}
				} else {
					// do not read answers for flooders
					// printf("\nInformation : sending flood packet %d", m_npacket);
					++m_npacket;
					++m_msgcount;
					PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)__part, (LPOVERLAPPED)(&(*__part)->dumbSock));
					DWORD _rez = 0; DWORD _flags = 0;
					WSARecv(socket->m_Socket, &___wbuf, 1, &_rez, &_flags, (LPOVERLAPPED)(&(*__part)->recvSock), 0); 
				}
				break;
								}
			case iocp_Reconnect : {
				InterlockedDecrement(&m_clients);
				PostQueuedCompletionStatus(m_hCCCP, 0, (ULONG_PTR)__part, (LPOVERLAPPED)__lp);
				break;
								  }
			case iocp_Delete :	{
				InterlockedDecrement(&m_clients);
				PostQueuedCompletionStatus(m_hCCCP, 0, (ULONG_PTR)__part, (LPOVERLAPPED)__lp);
				break;
								}
			case iocp_Flood :	{
				break;
								}
			default :			{ // wrong packet?
								}
		} InterlockedExchange(&(*__part)->m_locked, 0); // unlock participant
	}
	ExitThread(0);
	return 0;
}

DWORD WINAPI VS_Flooder::controlThread(void *arg) {
	return ((VS_Flooder *)arg)->doControl();
};

DWORD VS_Flooder::doControl() {
	WaitForSingleObject(m_hSYNCEVENT, INFINITE);
	while (m_hIOCP) {
        DWORD __bytesTransferred = 0; VS_TestParticipant **__part = NULL; LPOVERLAPPED __lp = NULL;
		if (GetQueuedCompletionStatus(m_hCCCP , &__bytesTransferred, (PULONG_PTR)&__part, &__lp, INFINITE) == 0) {
			if (!((__lp) || (__part) || (*__part))) {
				if (__part) {
					(*__part)->ctrlSock.m_op_mode = iocp_Delete;
					(*__part)->recvSock.m_lastError ++;
					(*__part)->sendSock.m_lastError ++;
				}
			} else {
				::printf("\nError :: recv IOCP failed :: %d", GetLastError());
				PostQueuedCompletionStatus(m_hCCCP, 0, NULL, NULL);
				break;
			}
		}
		if (InterlockedExchange(&m_terminate, m_terminate)) {
			::printf("\nExitting thread %d", m_hZThread);
			PostQueuedCompletionStatus(m_hCCCP, 0, NULL, NULL);
			break;
		}
		if (!((__bytesTransferred) || (__part) || (__lp))) {
			::printf("\nInformation :: recv IOCP closed :: %d", GetLastError());
			PostQueuedCompletionStatus(m_hCCCP, 0, NULL, NULL);
			break;
		}
		if (!__part) {
			continue;
		}
		if (!(*__part)->m_ctrl) { continue; } // socket not in control thread. drop event and continue
		while (InterlockedExchange(&(*__part)->m_locked, 1) || InterlockedExchange(&m_terminate, m_terminate)) {}; // lock participant. this takes less resources then critical section
		VS_Socket_Handle *socket = (VS_Socket_Handle *)__lp; DWORD flags(0); DWORD bytes_transferred(0);
		switch (socket->m_op_mode) {
			case iocp_Reconnect : {
				WaitForSingleObject(m_hTIMER, AppMode.sleep_connect);
				(*__part)->m_error ++;
				if ((*__part)->m_error > AppMode.reconnect) {
					(*__part)->ctrlSock.m_op_mode = iocp_Delete;
					PostQueuedCompletionStatus(m_hCCCP, 0, (ULONG_PTR)__part, (LPOVERLAPPED)(&(*__part)->ctrlSock));
				} else if (m_pg->TryReconnectParticipant((**__part), __part)) {
					InterlockedExchange(&(*__part)->m_locked, 0);
					PostQueuedCompletionStatus(m_hCCCP, 0, (ULONG_PTR)__part, (LPOVERLAPPED)__lp);
				} else {
					InterlockedIncrement(&m_clients);
					InterlockedDecrement(&(*__part)->m_ctrl);
					InterlockedExchange(&(*__part)->m_locked, 0); // unlock participant
					PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)__part, (LPOVERLAPPED)(&(*__part)->dumbSock));
				}
				break;
								  }
			case iocp_Delete :	{
				EnterCriticalSection(&m_vcs);
				std::vector<void **>::iterator _peer = std::find(m_listPeers.begin(), m_listPeers.end(), (void **)__part);
				if (_peer != m_listPeers.end()) {
					std::swap(*_peer, m_listPeers.back());
					m_listPeers.pop_back();
					m_pg->DeleteParticipant(__part);
				}
				LeaveCriticalSection(&m_vcs);
				break;
								}
			default :			{
								}
		}
	}
	ExitThread(0);
	return 0;
};

extern void CreateNewInstance();

DWORD WINAPI VS_Flooder::rebootThread(void *arg) {
	return ((VS_Flooder *)arg)->doReboot();
}

DWORD VS_Flooder::doReboot() {
	WaitForSingleObject(m_hTIMER, AppMode.reboot_time * 60 * 1000); // 30 minutes
	CreateNewInstance();
	return 0;
}
