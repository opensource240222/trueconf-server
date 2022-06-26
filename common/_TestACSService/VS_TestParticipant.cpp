#include "_testacsconst.h"
#include "VS_TestParticipant.h"
#include "VS_CoolTimer.h"
#include <windows.h>

VS_Packet::VS_Packet(const signed long size)
: m_data(new char[size]), m_total(0), m_transferred(0), m_sequence(1) {
	memset((void *)m_data, 0, size);
};

VS_Packet::~VS_Packet() {
	delete[] m_data;
};

VS_Socket_Handle::VS_Socket_Handle(HANDLE &hiocp, const sockaddr &_iaddr, void *&iocp_key, const bool onesocket)
: m_IOCP(hiocp), m_lastError(0), m_op_mode(iocp_Nop), m_handshake(0), m_lhandshake(0),
m_Ov(), m_Socket(INVALID_SOCKET), m_wsaBuf(), m_sock_addr(_iaddr), m_lastAccTime(0),
m_pwCrypt(NULL), m_prCrypt(NULL), m_openSSL(AppMode.ssl), m_sslRC(0) {
	if (!onesocket) DoOpenConnection(iocp_key);
	m_packet = new VS_Packet(BUF_SIZE);
	m_wsaBuf.buf = m_packet->m_data;
};

VS_Socket_Handle::~VS_Socket_Handle(){
	DoCloseConnection();
	delete m_packet;
};

extern VS_CoolTimer timer;

signed long VS_Socket_Handle::DoOpenConnection(void *&iocp_key) {
	SetLastError(0);	WSASetLastError(0);
	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_Socket == INVALID_SOCKET) {
		printf("\nError :: unable to create socket :: %d", WSAGetLastError());
		return INVALID_SOCKET;
	}
	m_IOCP = CreateIoCompletionPort((HANDLE)(m_Socket), m_IOCP, (ULONG_PTR)iocp_key, 0);
	if (m_IOCP == NULL) {
		printf("\nError :: IOCP association failed :: %d", GetLastError());
		return INVALID_SOCKET;
	}
	timer.Start(tm_connect);
	if (WSAConnect(m_Socket, (SOCKADDR *)&m_sock_addr, sizeof(sockaddr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
		printf("\nError :: unable to connect socket :: %d", WSAGetLastError());
		return INVALID_SOCKET;
	}
	timer.Shot(tm_connect);
	timer.cAdd(tm_connect);
	return m_Socket;
};

signed long VS_Socket_Handle::DoCloseConnection() {
	CancelIo((HANDLE)m_Socket);
	closesocket(m_Socket);
	WSASetLastError(0); SetLastError(0);
	return 0;
};

unsigned long VS_Socket_Handle::SendSync(const char *buffer, const signed long size, const SOCKET s) {
	return send(s, buffer, size, 0);
}

unsigned long VS_Socket_Handle::RecvSync(char *buffer, const signed long size, const SOCKET r) {
	if (size <= BUF_SIZE) {
		signed long done = 0; signed long loop = 0;
		while (done < size) {
			if  (loop >= 127) break;
			signed long _recvCount = recv(r, &buffer[done], size - done, 0);
			if (_recvCount == SOCKET_ERROR) { loop = 127; continue;	}
			done = done + _recvCount; loop ++;
		} if (loop >= 127) { return -1; } else { return done; }
	} else return recv(r, buffer, BUF_SIZE, 0);
}

VS_TestParticipant::VS_TestParticipant(HANDLE &hiocp, const sockaddr &_iaddr, void *iocp_key)
: sendSock(hiocp, _iaddr, iocp_key, false), recvSock(hiocp, _iaddr, iocp_key, AppMode.onesocket), m_ctrl(0), m_error(0), m_locked(0) {
	char _eptemp[256 + 1] = {};
	EnterCriticalSection(&gcs);
//	VS_GenerateTempEndpoint((char *)&_eptemp);
	LeaveCriticalSection(&gcs);
	m_endpointName = (std::string)((char *)&_eptemp);
	sendSock.m_epn = recvSock.m_epn = m_endpointName;
	sendSock.m_op_mode = iocp_Write; recvSock.m_op_mode = iocp_Read;
	recvSock.m_packet->m_total = BUF_SIZE; recvSock.m_wsaBuf.len = BUF_SIZE;
	if (AppMode.onesocket) {
		recvSock.m_Socket = sendSock.m_Socket;
	}
};

VS_TestParticipant::~VS_TestParticipant() {
};

VS_ParticipantModifier::VS_ParticipantModifier(const VS_TestParticipant* _mod)
: m_vs_part_mod(_mod) {
};

VS_ParticipantModifier::~VS_ParticipantModifier() {
};

volatile LPWSAPROTOCOL_INFO VS_QoS_Modifier::QoSProtocol = NULL;
FLOWSPEC VS_QoS_Modifier::default_spec = {680000, 68000, 1360000, QOS_NOT_SPECIFIED, QOS_NOT_SPECIFIED, SERVICETYPE_GUARANTEED | SERVICE_NO_QOS_SIGNALING, 340, 340};
QOS VS_QoS_Modifier::default_qos = {default_spec, default_spec, 0};
const vs_modifier_type VS_QoS_Modifier::m_modifer_type = mod_Sock;

VS_QoS_Modifier::VS_QoS_Modifier(const VS_TestParticipant* _mod)
: VS_ParticipantModifier(_mod), modName("QoS") {
};

VS_QoS_Modifier::~VS_QoS_Modifier() {
};

LPWSAPROTOCOL_INFO VS_QoS_Modifier::InitQoSProtocol() {
	if (QoSProtocol != NULL) return QoSProtocol;
	DWORD bufferSize = -1; DWORD numProtocols = 0;
    LPWSAPROTOCOL_INFO installedProtocols = NULL;
	LPWSAPROTOCOL_INFO qosProtocol = NULL;
	numProtocols = WSAEnumProtocols(NULL, NULL, &bufferSize);
    if ((numProtocols == SOCKET_ERROR) && (WSAGetLastError() != WSAENOBUFS)) { return NULL; }
	installedProtocols = (LPWSAPROTOCOL_INFO)malloc(bufferSize);
	memset((void*)installedProtocols, 0, bufferSize);
	numProtocols = WSAEnumProtocols(NULL, installedProtocols, &bufferSize);
	if (numProtocols == SOCKET_ERROR) { return NULL; } else {
		qosProtocol = installedProtocols;
		for (DWORD i = 0; i < numProtocols; qosProtocol++, i++) {
			if  (((qosProtocol->dwServiceFlags1 & XP1_QOS_SUPPORTED) == XP1_QOS_SUPPORTED)
					&& (qosProtocol->iSocketType == SOCK_STREAM)
						&& (qosProtocol->iAddressFamily == AF_INET)) { break; }}}
	InterlockedExchangePointer((volatile PVOID*)QoSProtocol, qosProtocol);
	return(qosProtocol);
}

VS_ParticipantModifier * VS_QoS_Modifier::DoCreateNew() {
	return new VS_QoS_Modifier(m_vs_part_mod);
}

signed long VS_QoS_Modifier::DoModify() {
	DWORD dwBytesRet(0);
	return 	WSAIoctl(m_vs_part_mod->sendSock.m_Socket, SIO_SET_QOS, (void *)&default_qos, sizeof(QOS), NULL, 0, &dwBytesRet, NULL, NULL);
}

const vs_modifier_type VS_Crypt_Modifier::m_modifer_type = mod_Data;

VS_Crypt_Modifier::VS_Crypt_Modifier(const VS_TestParticipant* _mod)
: VS_ParticipantModifier(_mod), modName("Crypt") {
};

VS_Crypt_Modifier::~VS_Crypt_Modifier() {
};

VS_ParticipantModifier * VS_Crypt_Modifier::DoCreateNew() {
	return new VS_Crypt_Modifier(m_vs_part_mod);
}

signed long VS_Crypt_Modifier::DoModify() {
	return 0;
}

tcp_keepalive VS_KeepAlive_Modifier::default_alive = {1, 5000, 15000};
const vs_modifier_type VS_KeepAlive_Modifier::m_modifer_type = mod_Sock;

VS_KeepAlive_Modifier::VS_KeepAlive_Modifier(const VS_TestParticipant* _mod)
: VS_ParticipantModifier(_mod), modName("Keep_Alive") {
};

VS_KeepAlive_Modifier::~VS_KeepAlive_Modifier() {
};

VS_ParticipantModifier * VS_KeepAlive_Modifier::DoCreateNew() {
	return new VS_KeepAlive_Modifier(m_vs_part_mod);
};

signed long VS_KeepAlive_Modifier::DoModify() {
	DWORD dwBytesRet(0);
	if (::AppMode.onesocket) {
		return WSAIoctl(m_vs_part_mod->sendSock.m_Socket, SIO_KEEPALIVE_VALS, (void *)&default_alive, sizeof(tcp_keepalive),  NULL, 0, &dwBytesRet, NULL, NULL);
	} else {
		return WSAIoctl(m_vs_part_mod->sendSock.m_Socket, SIO_KEEPALIVE_VALS, (void *)&default_alive, sizeof(tcp_keepalive),  NULL, 0, &dwBytesRet, NULL, NULL)
				|| WSAIoctl(m_vs_part_mod->recvSock.m_Socket, SIO_KEEPALIVE_VALS, (void *)&default_alive, sizeof(tcp_keepalive),  NULL, 0, &dwBytesRet, NULL, NULL);
	}
};

VS_Modifier_Holder::VS_Modifier_Holder() {
};

VS_Modifier_Holder::~VS_Modifier_Holder() {
};

signed long VS_Modifier_Holder::RegisterNewModifier(const vs_modifier_type type, VS_ParticipantModifier *part) {
	mem_holder[type].push_back(part);
	return mem_holder[type].size();
};

signed long VS_Modifier_Holder::GetModifiersCount(const vs_modifier_type type) {
	return mem_holder[type].size();
};

VS_ParticipantModifier *VS_Modifier_Holder::GetModifier(const vs_modifier_type type, const long number) {
	return mem_holder[type].at(number);
};

VS_Dumb_Socket::VS_Dumb_Socket()
: m_op_mode(iocp_Nop), m_Ov() {
};

VS_Dumb_Socket::~VS_Dumb_Socket() {
};