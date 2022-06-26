#include "_testacsconst.h"
#include "VS_Scope.h"
#include <algorithm>

HANDLE VS_Scope::m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0); // main completion port
HANDLE VS_Scope::m_hCCCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0); // control code completion port
HANDLE VS_Scope::m_hTIMER = CreateEvent(NULL, true, false, NULL);
volatile signed long VS_Scope::m_clients = 0; // total online participants counter
volatile signed long VS_Scope::m_terminate = 0; // treminate threads flag

VS_Scope::VS_Scope(const char *tAddress, const signed long nConn, const signed long tSleep)
: m_pg(VS_PacketGenerator::GetInstance()) {
	m_addr = _strdup(tAddress); char *_s = strchr(m_addr, ':'); m_ip = m_addr;
	m_port = _s + 1; memset(_s, 0, 1); addrinfo _ai = {}; m_addrinfo = NULL; m_size = nConn;
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

VS_Scope::~VS_Scope(void) {
	DeleteCriticalSection(&m_vcs);
}

DWORD WINAPI VS_Scope::connThread(void *arg) {
	return ((VS_Scope *)arg)->doConnect();
}

DWORD VS_Scope::doConnect() {
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

DWORD WINAPI VS_Scope::processThread(void *arg) {
	return ((VS_Scope *)arg)->doProcess();
};

DWORD VS_Scope::doProcess() {
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
		if (!((__bytesTransferred) || (__part) || (__lp))) { //FixMe! WSARecv may return OK when __bytesTransferred == 0, and WSAGetlastError will OK! o_O -> so error will not be detected
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
		if ((__bytesTransferred == 0)) { // look up at comment higher! ^-^
			__bytesTransferred == 0 means Socket error but
			we use this for PostQueuedCompletionStatus exchange,
			because any error detected via WSASend è WSARecv
		}
*/
		if (((*__part)->m_ctrl) && ((*__part)->ctrlSock.m_op_mode <= 0xFF)) { continue; } // socket in control thread. drop event and and continue
		while (InterlockedExchange(&(*__part)->m_locked, 1) && !InterlockedExchange(&m_terminate, m_terminate)) {}; // lock participant. this takes less resources then critical section
		VS_Socket_Handle *socket = (VS_Socket_Handle *)__lp; DWORD flags(0); DWORD bytes_transferred(0);
		if ((!socket) || ((socket->m_op_mode != iocp_Nop) && (!socket->m_packet))) { InterlockedExchange(&(*__part)->m_locked, 0); continue; } // strange behavior (parallel exec?) detected. unlock, drop event and continue
		switch (socket->m_op_mode) {
			case iocp_Read :	{ // read answer
				socket->m_packet->m_transferred += __bytesTransferred;
				if (AppMode.ssl) {
					unsigned long _crbsize = socket->m_prCrypt->GetBlockSize();
					unsigned long _zrbsize = _crbsize + 1;
					signed long _needsize = (sizeof(VS_FixedPartMessage) / _crbsize + 1) * _crbsize;
					if ((socket->m_packet->m_total == BUF_SIZE) && (socket->m_packet->m_transferred > _needsize)) {
						unsigned char *buf = new unsigned char[_zrbsize];
						if (socket->m_prCrypt->Decrypt((const unsigned char *)socket->m_packet->m_data, _zrbsize, buf, &_zrbsize)) {
							socket->m_packet->m_total = ((VS_FixedPartMessage *)buf)->head_length + ((VS_FixedPartMessage *)buf)->body_length + 1;
							socket->m_packet->m_total = (socket->m_packet->m_total / _crbsize + 1) * _crbsize;
						}
						delete [] buf;
					}
					if (socket->m_packet->m_total == socket->m_packet->m_transferred) {
						unsigned long _vrxsize = socket->m_packet->m_total;
						unsigned char *buf = new unsigned char[_vrxsize];
						if (!socket->m_prCrypt->Decrypt((const unsigned char *)(socket->m_packet->m_data + _zrbsize + 1), socket->m_packet->m_total - _zrbsize - 1, buf, &_vrxsize)) {
							printf("\nError :: decrypt failed.");
						}
						delete [] buf;
					}
				} else if ((socket->m_packet->m_total == BUF_SIZE) && (socket->m_packet->m_transferred >= sizeof(VS_FixedPartMessage))) {
					socket->m_packet->m_total = ((VS_FixedPartMessage *)socket->m_packet->m_data)->head_length + ((VS_FixedPartMessage *)socket->m_packet->m_data)->body_length + 1;
				}
				if (socket->m_packet->m_transferred < socket->m_packet->m_total) {
/*					int a = GetTickCount() - socket->m_lastAccTime;
					if (a < 2000) {
						printf("\nread time error %d", a);
					};*/
					socket->m_lastAccTime = GetTickCount();
					socket->m_wsaBuf.buf += __bytesTransferred; socket->m_wsaBuf.len -= __bytesTransferred;
					if (WSARecv(socket->m_Socket, &socket->m_wsaBuf, 1, &bytes_transferred, &flags, __lp, NULL) == SOCKET_ERROR) {
						signed long _err = WSAGetLastError();
						if (!((_err == WSA_IO_PENDING) || (_err == WSAENOBUFS))) {
							InterlockedIncrement(&(*__part)->m_ctrl);
							(*__part)->ctrlSock.m_op_mode = iocp_Reconnect;
							socket->m_lastError ++;
							PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)__part, (LPOVERLAPPED)(&(*__part)->ctrlSock));
							::printf("\nError :: WSARecv error :: %d", _err);
							CancelIo((HANDLE)socket->m_Socket);
						}
					} 
				} else {
					PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)__part, (LPOVERLAPPED)(&(*__part)->dumbSock));
				}
				break;
								}
			case iocp_Nop :		{ // prepare new packet
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
					if (_delta < AppMode.sleep_time) {
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
					(*__part)->recvSock.m_Ov = WSAOVERLAPPED();
					(*__part)->recvSock.m_wsaBuf.buf = (*__part)->recvSock.m_packet->m_data;
					(*__part)->recvSock.m_wsaBuf.len = (*__part)->recvSock.m_packet->m_total = BUF_SIZE;
					(*__part)->recvSock.m_packet->m_transferred = 0;
					PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)__part, (LPOVERLAPPED)(&(*__part)->recvSock));
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

DWORD WINAPI VS_Scope::controlThread(void *arg) {
	return ((VS_Scope *)arg)->doControl();
};

DWORD VS_Scope::doControl() {
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
		while (InterlockedExchange(&(*__part)->m_locked, 1) && !InterlockedExchange(&m_terminate, m_terminate)) {}; // lock participant. this takes less resources then critical section
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

DWORD WINAPI VS_Scope::rebootThread(void *arg) {
	return ((VS_Scope *)arg)->doReboot();
}

DWORD VS_Scope::doReboot() {
	WaitForSingleObject(m_hTIMER, AppMode.reboot_time * 60 * 1000); // 30 minutes
	CreateNewInstance();
	return 0;
}
