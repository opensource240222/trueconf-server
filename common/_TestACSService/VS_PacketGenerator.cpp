#include "_testacsconst.h"
#include "VS_TestParticipant.h"
#include "VS_PacketGenerator.h"
#include "VS_Scope.h"
#include "VS_CoolTimer.h"

VS_PacketGenerator* VS_PacketGenerator::m_hInstance = NULL;
signed long VS_PacketGenerator::m_requestCount = 0;

VS_PacketGenerator::VS_PacketGenerator(void) {
	InitializeCriticalSection(&m_pgcs);
	InitializeCriticalSection(&m_ccs);
}

VS_PacketGenerator::~VS_PacketGenerator(void) {
	DeleteCriticalSection(&m_ccs);
	DeleteCriticalSection(&m_pgcs);
}

extern VS_CoolTimer timer;

signed long VS_PacketGenerator::TryConnectParticipant(VS_TestParticipant &tp) {
	EnterCriticalSection(&m_pgcs);
	tp.sendSock.m_handshake  = VS_FormTransportHandshake(0, AppMode.endpoint.c_str(), 0, AppMode.ssl);
	tp.sendSock.m_lhandshake = sizeof(net::HandshakeHeader) + ((net::HandshakeHeader *)tp.sendSock.m_handshake)->body_length + 1;

	timer.Start(tm_handshake);
	tp.sendSock.SendSync((char *)tp.sendSock.m_handshake, tp.sendSock.m_lhandshake, tp.sendSock.m_Socket);
	if (tp.sendSock.RecvSync(tp.sendSock.m_packet->m_data, sizeof(net::HandshakeHeader), tp.sendSock.m_Socket) == -1) {
		printf("\nError :: unable to receive answer on FixedZeroHandshake from server :: %d", GetLastError());
		LeaveCriticalSection(&m_pgcs);
		return -1;
	}
	if (tp.sendSock.RecvSync(tp.sendSock.m_packet->m_data + sizeof(net::HandshakeHeader), ((net::HandshakeHeader *)tp.sendSock.m_packet->m_data)->body_length + 1, tp.sendSock.m_Socket) == -1) {
		printf("\nError :: unable to receive answer on ZeroHandshake from server :: %d", GetLastError());
		LeaveCriticalSection(&m_pgcs);
		return -1;
	}
	timer.Shot(tm_handshake);
	timer.cAdd(tm_handshake, 1000);
	{
		unsigned char resultCode(~0); unsigned short maxConnSilenceMs(0); unsigned char fatalSilenceCoef(0); unsigned char hops(0);
		char *sid(0), *cid(0);
		bool isKeepAlive(false);
		if (!VS_TransformTransportReplyHandshake((net::HandshakeHeader *)tp.sendSock.m_packet->m_data, resultCode, maxConnSilenceMs, fatalSilenceCoef, hops, sid, cid,isKeepAlive) ||
		 ((resultCode) || (!((cid) && (*cid))))) {
			printf("\nError :: TransformTransportReplyHandshake fault :: = %d", resultCode);
			LeaveCriticalSection(&m_pgcs);
			return -1;
		}
		tp.sendSock.m_epn = cid;
	}
	tp.recvSock.m_handshake = tp.sendSock.m_handshake;
	tp.recvSock.m_lhandshake = tp.sendSock.m_lhandshake;
	LeaveCriticalSection(&m_pgcs);
	return 0;
}

void VS_PacketGenerator::MakeMsg_TransportPing(VS_Socket_Handle &sh) {
	EnterCriticalSection(&m_pgcs);
	/*
	VS_Container rCnt;
    rCnt.AddValue(VS_TypeNameValue, (const long)VS_TRANSPORT_MANAGING_PING);
	void* body = NULL; unsigned long bodySize = 0;
	rCnt.SerializeAlloc(body, bodySize);
//	if (VS_TransportMessage::Set(1, ~0, sh.m_epn.c_str(), NULL, NULL, AppMode.endpoint.c_str(), NULL, VS_TRANSPORT_TIMELIFE_PING, body, bodySize)) {
	if (VS_TransportMessage::Set(1, ~0, sh.m_epn.c_str(), 0, 0, 0, 0, VS_TRANSPORT_TIMELIFE_PING, body, bodySize, 0, 0, AppMode.endpoint.c_str(), 0)) {
		unsigned long size_mess = ((VS_FixedPartMessage *)mess)->head_length +
			((VS_FixedPartMessage *)mess)->body_length + 1;
		memcpy((void *)sh.m_wsaBuf.buf, (void *)mess, size_mess); free(body);
		sh.m_wsaBuf.len = sh.m_packet->m_total = size_mess;
	}
	*/

	const char opcode[] = { VS_TRANSPORT_MANAGING_PING, '\0' };
	if (VS_TransportMessage::Set( 1, ~0, 0, 0, (const char *)&opcode, 0, 0, VS_TRANSPORT_TIMELIFE_PING, "", 1, 0, 0, AppMode.endpoint.c_str(), 0)) {
		unsigned long size_mess = ((VS_FixedPartMessage *)mess)->head_length +
			((VS_FixedPartMessage *)mess)->body_length + 1;
		memcpy((void *)sh.m_wsaBuf.buf, (void *)mess, size_mess);
		sh.m_wsaBuf.len = sh.m_packet->m_total = size_mess;
	}
	LeaveCriticalSection(&m_pgcs);
	return;
}

void VS_PacketGenerator::MakeMsg_ServicePing(VS_Socket_Handle &sh) {
	EnterCriticalSection(&m_pgcs);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, PING_METHOD);
	void* body = NULL;	unsigned long bodySize = 0;
	rCnt.SerializeAlloc(body, bodySize);
	/*
	if (VS_TransportMessage::Set(1, sh.m_packet->m_sequence, sh.m_epn.c_str(), PING_SRV, NULL, AppMode.endpoint.c_str(), PING_SRV, SEND_TIMEOUT, body, bodySize)) {
		unsigned long size_mess = ((VS_FixedPartMessage *)mess)->head_length +
			((VS_FixedPartMessage *)mess)->body_length + 1;
		memcpy((void *)sh.m_wsaBuf.buf, (void *)mess, size_mess); free(body);
		sh.m_wsaBuf.len = sh.m_packet->m_total = size_mess; sh.m_packet->m_sequence++;
	}
	*/
	if (VS_TransportMessage::Set(1, sh.m_packet->m_sequence, 0, PING_SRV, 0, 0, PING_SRV, SEND_TIMEOUT, body, bodySize, 0, 0, AppMode.endpoint.c_str(), 0)) {
		unsigned long size_mess = ((VS_FixedPartMessage *)mess)->head_length +
			((VS_FixedPartMessage *)mess)->body_length + 1;
		memcpy((void *)sh.m_wsaBuf.buf, (void *)mess, size_mess); free(body);
		sh.m_wsaBuf.len = sh.m_packet->m_total = size_mess; sh.m_packet->m_sequence++;
	}
	LeaveCriticalSection(&m_pgcs);
	return;
}

void VS_PacketGenerator::MakeMsg_ServiceChat(VS_Socket_Handle &sh) {
	EnterCriticalSection(&m_pgcs);
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, PING_METHOD);
	rCnt.AddValue(TO_PARAM, sh.m_epn.c_str());
	void* body = NULL;	unsigned long bodySize = 0;
	rCnt.SerializeAlloc(body, bodySize);
	/*
	if (VS_TransportMessage::Set(1, sh.m_packet->m_sequence, sh.m_epn.c_str(), CHAT_SRV, NULL, AppMode.endpoint.c_str(), CHAT_SRV, SEND_TIMEOUT, body, bodySize)) {
		unsigned long size_mess = ((VS_FixedPartMessage *)mess)->head_length +
			((VS_FixedPartMessage *)mess)->body_length + 1;
		memcpy((void *)sh.m_wsaBuf.buf, (void *)mess, size_mess); free(body);
		sh.m_wsaBuf.len = sh.m_packet->m_total = size_mess; sh.m_packet->m_sequence++;
	}
	*/
	if (VS_TransportMessage::Set(1, sh.m_packet->m_sequence, 0, PING_SRV, 0, 0, CHAT_SRV, SEND_TIMEOUT, body, bodySize, 0, 0, AppMode.endpoint.c_str(), 0)) {
		unsigned long size_mess = ((VS_FixedPartMessage *)mess)->head_length +
			((VS_FixedPartMessage *)mess)->body_length + 1;
		memcpy((void *)sh.m_wsaBuf.buf, (void *)mess, size_mess); free(body);
		sh.m_wsaBuf.len = sh.m_packet->m_total = size_mess; sh.m_packet->m_sequence++;
	}
	LeaveCriticalSection(&m_pgcs);
	return;
}

VS_PacketGenerator *VS_PacketGenerator::GetInstance() {
	if (!m_requestCount) { m_hInstance = new VS_PacketGenerator(); }
	m_requestCount ++;
    return m_hInstance;
}

void VS_PacketGenerator::FreeInstance() {
	m_requestCount --;
	if (!m_requestCount) { m_hInstance = NULL; delete this; }
} 

signed long VS_PacketGenerator::TryReconnectParticipant(VS_TestParticipant &tp, void *iocp_key) {
	/*
	EnterCriticalSection(&m_ccs); // ccs - Control Critical Section
	if (!((tp.sendSock.m_lastError) || (tp.recvSock.m_lastError))) {
		return 0;
	}
	if ((AppMode.onesocket) && (tp.recvSock.m_lastError)) {
		tp.sendSock.m_lastError = tp.recvSock.m_lastError;
		tp.recvSock.m_lastError = 0;
	}
	if (tp.sendSock.m_lastError) {
		SOCKET __oldone = tp.sendSock.m_Socket;
		tp.sendSock.m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		CancelIo((HANDLE)__oldone); closesocket(__oldone);
		if (tp.sendSock.m_handshake) {
			free(tp.sendSock.m_handshake);
		}
		if (AppMode.onesocket) {
			tp.sendSock.m_handshake = tp.recvSock.m_handshake = NULL;
			tp.sendSock.m_lhandshake = tp.recvSock.m_lhandshake = 0;
		} else {
			tp.sendSock.m_handshake = NULL; tp.sendSock.m_lhandshake = 0;
		}
		if (tp.sendSock.m_Socket == INVALID_SOCKET) {
			printf("\nError :: _RE_ unable create send socket :: %d", WSAGetLastError());
			LeaveCriticalSection(&m_ccs);
			return -1;
		}
		tp.sendSock.m_IOCP = CreateIoCompletionPort((HANDLE)(tp.sendSock.m_Socket), tp.sendSock.m_IOCP, (ULONG_PTR)iocp_key, 0);
		if (tp.sendSock.m_IOCP == NULL) {
			printf("\nError :: _RE_ unable to link recv socket on IOCP :: %d", GetLastError());
			LeaveCriticalSection(&m_ccs);
			return -1;
		}
		if (WSAConnect(tp.sendSock.m_Socket, (SOCKADDR *)&tp.sendSock.m_sock_addr, sizeof(sockaddr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
			printf("\nError :: _RE_ unable to connect socket :: %d", WSAGetLastError());
			LeaveCriticalSection(&m_ccs);
			return -1;
		}
		if (AppMode.onesocket) {
			tp.recvSock.m_Socket = tp.sendSock.m_Socket;
			tp.sendSock.m_handshake = tp.recvSock.m_handshake = VS_FormTransportHandshake(tp.m_endpointName.c_str(), hkey, VS_ACS_LIB_BOTH, AppMode.endpoint.c_str(), 0, AppMode.ssl);
			tp.sendSock.m_lhandshake = tp.recvSock.m_lhandshake = sizeof(net::HandshakeHeader) + ((net::HandshakeHeader *)tp.sendSock.m_handshake)->body_length + 1;
		} else {
			tp.sendSock.m_handshake = VS_FormTransportHandshake(tp.m_endpointName.c_str(), hkey, VS_ACS_LIB_SENDER, AppMode.endpoint.c_str(), 0, AppMode.ssl);
			tp.sendSock.m_lhandshake = sizeof(net::HandshakeHeader) + ((net::HandshakeHeader *)tp.sendSock.m_handshake)->body_length + 1;
		}
		tp.sendSock.SendSync((char *)tp.sendSock.m_handshake, tp.sendSock.m_lhandshake, tp.sendSock.m_Socket);
		if (tp.sendSock.RecvSync(tp.sendSock.m_packet->m_data, sizeof(net::HandshakeHeader), tp.sendSock.m_Socket) == -1) {
			printf("\nError :: _RE_ unable to receive answer on FixedZeroHandshake from server :: %d", GetLastError());
			LeaveCriticalSection(&m_ccs);
			return -1;
		}
		if (tp.sendSock.RecvSync(tp.sendSock.m_packet->m_data + sizeof(net::HandshakeHeader), ((net::HandshakeHeader *)tp.sendSock.m_packet->m_data)->body_length + 1, tp.sendSock.m_Socket) == -1) {
			printf("\nError :: _RE_ unable to receive answer on ZeroHandshake from server :: %d", GetLastError());
			LeaveCriticalSection(&m_ccs);
			return -1;
		}
		{
			char *key = NULL; unsigned char type = 0; unsigned char resultCode = 0;
			unsigned short maxConnSilenceMs = 0; unsigned char fatalSilenceCoef = 0; unsigned char hops = 0;
			VS_TransformTransportReplyHandshake((net::HandshakeHeader *)tp.sendSock.m_packet->m_data, key, type, resultCode, maxConnSilenceMs, fatalSilenceCoef, hops);
			if (resultCode) {
				printf("\nError :: _RE_ TransformTransportReplyHandshake fault :: = %d", resultCode);
				LeaveCriticalSection(&m_ccs);
				return -1;
			}
			if ((AppMode.onesocket) && (type != VS_ACS_LIB_BOTH)) {
				AppMode.onesocket = false;
				printf("\nWarning : _RE_ Target server not supported 1-socket transport. Swtching to 2-socket mode.");
				return -1;
			}
		}
		tp.sendSock.m_lastError = 0;
	}
	if (tp.recvSock.m_lastError) {
		SOCKET __oldone = tp.sendSock.m_Socket;
		tp.recvSock.m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		CancelIo((HANDLE)__oldone); closesocket(__oldone);
		if (tp.recvSock.m_handshake) {
			free(tp.recvSock.m_handshake);
		}
		tp.recvSock.m_handshake = NULL; tp.recvSock.m_lhandshake = 0;
		tp.recvSock.m_handshake = VS_FormTransportHandshake(tp.m_endpointName.c_str(), hkey, VS_ACS_LIB_RECEIVER, AppMode.endpoint.c_str(), 0, AppMode.ssl);
		tp.recvSock.m_lhandshake = sizeof(net::HandshakeHeader) + ((net::HandshakeHeader *)tp.recvSock.m_handshake)->body_length + 1;
		tp.recvSock.SendSync((char *)tp.recvSock.m_handshake, tp.recvSock.m_lhandshake, tp.recvSock.m_Socket);
		if (tp.recvSock.RecvSync(tp.recvSock.m_packet->m_data, sizeof(net::HandshakeHeader), tp.recvSock.m_Socket) == -1) {
			printf("\nError :: _RE_ unable to receive answer on FixedZeroHandshake from server :: %d", GetLastError());
			LeaveCriticalSection(&m_ccs);
			return -1;
		}
		if (tp.recvSock.RecvSync(tp.recvSock.m_packet->m_data + sizeof(net::HandshakeHeader), ((net::HandshakeHeader *)tp.recvSock.m_packet->m_data)->body_length + 1, tp.recvSock.m_Socket) == -1) {
			printf("\nError :: _RE_ recvSock unable to receive answer on ZeroHandshake from server :: %d", GetLastError());
			LeaveCriticalSection(&m_ccs);
			return -1;
		}
		{
			char *key = NULL; unsigned char type = 0; unsigned char resultCode = 0;
			unsigned short maxConnSilenceMs = 0; unsigned char fatalSilenceCoef = 0; unsigned char hops = 0;
			VS_TransformTransportReplyHandshake((net::HandshakeHeader *)tp.recvSock.m_packet->m_data, key, type, resultCode, maxConnSilenceMs, fatalSilenceCoef, hops);
			if (resultCode) {
				printf("\nError :: _RE_ TransformTransportReplyHandshake fault :: = %d", resultCode);
				LeaveCriticalSection(&m_ccs);
				return -1;
			}
		}
	}
	LeaveCriticalSection(&m_ccs);
	*/
	return 0;
};

bool VS_PacketGenerator::__sendClientSecureHandshake(VS_Socket_Handle &sh) {
	if (&sh == NULL) {
		return false;
	}
	if (!sh.m_openSSL) {
		return true;
	}
	if (sh.m_pwCrypt) {
		return true;
	}
	VS_SecureHandshake SecureHandshakeMgr;
	if (!SecureHandshakeMgr.Init(1, handshake_type_Client)) {
		printf("\nError : error send SecureHandshakeMgr.Init");
		return false;
	}
	VS_SecureHandshakeState state = SecureHandshakeMgr.Next();
	char *buf = NULL;
	unsigned long buf_size = 0;
	bool bRes = false;
	
	while ((secure_st_Error != state) && (secure_st_Finish != state)) {
		switch (state) {
				case secure_st_GetPacket:	{
					if (!SecureHandshakeMgr.PreparePacket((void **)&buf, &buf_size)) {
						bRes = false;
					} else {
						if (!sh.RecvSync(buf, buf_size, sh.m_Socket)) {
							SecureHandshakeMgr.FreePacket((void **)&buf);
							buf_size = 0;
							bRes = false;
						} else {
							bRes = SecureHandshakeMgr.ProcessPacket(buf, buf_size);
							SecureHandshakeMgr.FreePacket((void **)&buf);
							buf_size = 0;
						}
					} break;
											}
				case secure_st_SendPacket:	{
					if (!SecureHandshakeMgr.PreparePacket((void **)&buf, &buf_size)) {
						bRes = false;
					} else {
						if (!sh.SendSync((const char *)buf, buf_size, sh.m_Socket)) {
							SecureHandshakeMgr.FreePacket((void **)&buf);
							buf_size = 0;
							bRes = false;
						} else {
							SecureHandshakeMgr.FreePacket((void **)&buf);
							buf_size = 0;
							bRes = true;
						}
					} break;
											}
				case secure_st_SendCert:	{
					bRes = false;
					break;
											}
				case secure_st_Error:		{
					bRes = false;
					break;
											}
				case secure_st_Finish:		{
					bRes = true;
					break;
											}
				default:					{
                        bRes = false;
                }
		} /* end case */
		if (!bRes) {
			if ((e_err_VerificationFailed == SecureHandshakeMgr.GetHandshakeErrorCode()) ||
				(e_err_DataAreNotCertificate == SecureHandshakeMgr.GetHandshakeErrorCode())) {
					sh.m_openSSL = false;
			} else {
				if (sh.m_sslRC < MAX_SSL_RECONNECT) {
					sh.m_sslRC ++;
				} else {
					sh.m_openSSL = false;
				}
			}
			SecureHandshakeMgr.StopHandshake();
			return false;
		}
		if (secure_st_Finish != state) {
			state = SecureHandshakeMgr.Next();
		}
	}
	if (secure_st_Finish == state) {
		sh.m_pwCrypt = SecureHandshakeMgr.GetWriteSymmetricCrypt();
		SecureHandshakeMgr.StopHandshake();
		if (!sh.m_pwCrypt) {
			return false;
		} else {
			return true;
		}
	} else {
		if ((e_err_VerificationFailed == SecureHandshakeMgr.GetHandshakeErrorCode()) ||
			(e_err_DataAreNotCertificate == SecureHandshakeMgr.GetHandshakeErrorCode())) {
				sh.m_openSSL = false;
		} else {
			if (sh.m_sslRC < MAX_SSL_RECONNECT) {
				sh.m_sslRC ++;
			} else {
				sh.m_openSSL = false;
			}
		}
		SecureHandshakeMgr.StopHandshake();
		return false;
	}
};

bool VS_PacketGenerator::__recvClientSecureHandshake(VS_Socket_Handle &sh) {
	if (&sh == NULL) {
		return false;
	}
	if (!sh.m_openSSL) {
		return true;
	}
	if (sh.m_prCrypt) {
		return true;
	}
	VS_SecureHandshake SecureHandshakeMgr;
	if (!SecureHandshakeMgr.Init(1, handshake_type_Client)) {
		printf("\nError : error recv SecureHandshakeMgr.Init");
		return false;
	}
	VS_SecureHandshakeState state = SecureHandshakeMgr.Next();
	char *buf = NULL;
	unsigned long buf_size = 0;
	bool bRes = false;

	while ((secure_st_Error != state) && (secure_st_Finish != state)) {
		switch (state) {
				case secure_st_GetPacket:	{
					if (!SecureHandshakeMgr.PreparePacket((void **) &buf, &buf_size)) {
						bRes = false;
					} else {
						if (!sh.RecvSync(buf, buf_size, sh.m_Socket)) {
							SecureHandshakeMgr.FreePacket((void **)&buf);
							buf_size = 0;
							bRes = false;
						} else {
							bRes = SecureHandshakeMgr.ProcessPacket(buf, buf_size);
							SecureHandshakeMgr.FreePacket((void **)&buf);
							buf_size = 0;
						}
					} break;
											}
                case secure_st_SendPacket:	{
					if (!SecureHandshakeMgr.PreparePacket((void **)&buf, &buf_size)) {
						bRes = false;
					} else {
						if (!sh.SendSync((const char *)buf, buf_size, sh.m_Socket)) {
							SecureHandshakeMgr.FreePacket((void **)&buf);
							buf_size = 0;
							bRes = false;
						} else {
							SecureHandshakeMgr.FreePacket((void **)&buf);
							buf_size = 0;
							bRes = true;
						}
					}
					break;
											}
                case secure_st_SendCert:	{
					bRes = false;
					break;
											}
				case secure_st_Error:		{
					bRes = false;
					break;
											}
				case secure_st_Finish:		{
					bRes = true;
					break;
											}
				default:					{
					bRes = false;
											}
		}
		if (!bRes) {
			if ((e_err_VerificationFailed == SecureHandshakeMgr.GetHandshakeErrorCode())||
				(e_err_DataAreNotCertificate == SecureHandshakeMgr.GetHandshakeErrorCode())) {
					sh.m_openSSL = false;
			} else {
				if (sh.m_sslRC < MAX_SSL_RECONNECT) {
					sh.m_sslRC ++;
				} else {
					sh.m_openSSL = false;
				}
			}
			SecureHandshakeMgr.StopHandshake();
			return false;
		}
		if (secure_st_Finish != state) {
			state = SecureHandshakeMgr.Next();
		}
	}
	if (secure_st_Finish == state) {
		unsigned long sz(0);
		char *pem_buf(0);
		sh.m_prCrypt = SecureHandshakeMgr.GetReadSymmetricCrypt();
		if (AppMode.ssl) {
			sh.m_pwCrypt = SecureHandshakeMgr.GetWriteSymmetricCrypt();
		}
		SecureHandshakeMgr.GetCertificate(pem_buf,sz);
		if (sz) {
			pem_buf = new char[sz];
			if (SecureHandshakeMgr.GetCertificate(pem_buf,sz)) {
				serverCertificate.SetCert(pem_buf,sz,store_PEM_BUF);
			}
			delete [] pem_buf;
			sz = 0;
		}
		SecureHandshakeMgr.StopHandshake();
		if (!sh.m_prCrypt) {
			return false;
		} else {
			return true;
		}
	} else {
		if ((e_err_VerificationFailed == SecureHandshakeMgr.GetHandshakeErrorCode())||
			(e_err_DataAreNotCertificate == SecureHandshakeMgr.GetHandshakeErrorCode())) {
				sh.m_openSSL = false;
		} else {
			if (sh.m_sslRC < MAX_SSL_RECONNECT) {
				sh.m_sslRC ++;
			} else {
				sh.m_openSSL = false;
			}
		}
		SecureHandshakeMgr.StopHandshake();
		return false;
	}
};

/*
		__participant.SendSync((char *)__participant.m_packetHandshake, __participant.m_lenHandshake);
		if (__participant.RecvSync(__participant.m_recvBuf, sizeof(net::HandshakeHeader), __participant.m_sendSocket) == -1) {
			eReturn = -1;
			printf("\nError : _RE_sendSock unable to receive answer on FixedZeroHandshake from server");
			LeaveCriticalSection(&m_hCS4CNN);
			return;
		}
		if (__participant.RecvSync(__participant.m_recvBuf + sizeof(net::HandshakeHeader), ((net::HandshakeHeader *)__participant.m_recvBuf)->body_length + 1, __participant.m_sendSocket) == -1) {
			eReturn = -1;
			printf("\nError : _RE_sendSock unable to receive answer on ZeroHandshake from server");
			LeaveCriticalSection(&m_hCS4CNN);
			return;
		}
		{
			char *key = NULL;
			unsigned char type = 0;
			unsigned char resultCode = 0;
			unsigned short maxConnSilenceMs = 0;
			unsigned char fatalSilenceCoef = 0;
			unsigned char hops = 0;
			VS_TransformTransportReplyHandshake((net::HandshakeHeader *)__participant.m_recvBuf, key, type, resultCode, maxConnSilenceMs, fatalSilenceCoef, hops);
			if (resultCode) {
				eReturn = -1;
				printf("\nError : _RE_sendSock TransformTransportReplyHandshake fault. eCode = %d", resultCode);
				LeaveCriticalSection(&m_hCS4CNN);
				return;
			}
		}
		if (__participant.m_openSSL) {
			__participant.m_openSSL = __sendClientSecureHandshake(__participant);
		}
		__participant._sErrorConnection = false;
	}
	if (__participant._rErrorConnection) {
		SOCKET __exrSocket = __participant.m_recvSocket;
		__participant.m_recvSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (__participant.m_recvSocket == INVALID_SOCKET) {
			printf("\nError : unable create _RE_recv socket. code %d", WSAGetLastError());
			eReturn = -1;
			LeaveCriticalSection(&m_hCS4CNN);
			return;
		}
		CancelIo((HANDLE)__exrSocket);
		closesocket(__exrSocket);
		DWORD dwBytesRet = 0;
		WSAIoctl(__participant.m_recvSocket, SIO_KEEPALIVE_VALS, (void *)&alive, sizeof(alive),  NULL, 0, &dwBytesRet, NULL, NULL);
		if (WSAConnect(__participant.m_recvSocket, (SOCKADDR *)&_iaddr, sizeof(sockaddr),
			NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
				eReturn = WSAGetLastError();
				printf("\nError : unable to _RE_connect rsocket. eCode = %d", eReturn);
				LeaveCriticalSection(&m_hCS4CNN);
				return;
		}
		__participant.m_hRIOCP = CreateIoCompletionPort((HANDLE)(__participant.m_recvSocket), __participant.m_hRIOCP, (ULONG_PTR)&__participant, 0);
		if (__participant.m_hRIOCP == NULL) {
			eReturn = GetLastError();
			printf("\nError : unable to _RE_link recv socket on IOCP. eCode = %d", eReturn);
			LeaveCriticalSection(&m_hCS4CNN);
			return;
		};
		if (__participant.m_packetHandshake) {
			free(__participant.m_packetHandshake);
		}
		__participant.m_packetHandshake = NULL;
		__participant.m_packetHandshake = VS_FormTransportHandshake(__participant.m_endpointName.c_str(), hkey, VS_ACS_LIB_RECEIVER, AppMode.endpoint.c_str(), 0, AppMode.ssl);
		__participant.m_lenHandshake = sizeof(net::HandshakeHeader) + ((net::HandshakeHeader *)__participant.m_packetHandshake)->body_length + 1;
		__participant.SendSync((char *)__participant.m_packetHandshake, __participant.m_lenHandshake, __participant.m_recvSocket);
		if (__participant.RecvSync(__participant.m_recvBuf, sizeof(net::HandshakeHeader)) == -1) {
			eReturn = -1;
			printf("\nError : _RE_recvSock unable to receive answer on FixedZeroHandshake from server");
			LeaveCriticalSection(&m_hCS4CNN);
			return;
		}
		if (__participant.RecvSync(__participant.m_recvBuf + sizeof(net::HandshakeHeader), ((net::HandshakeHeader *)__participant.m_recvBuf)->body_length + 1) == -1) {
			eReturn = -1;
			printf("\nError : _RE_recvSock unable to receive answer on ZeroHandshake from server");
			LeaveCriticalSection(&m_hCS4CNN);
			return;
		}
		{
			char *key = NULL;
			unsigned char type = 0;
			unsigned char resultCode = 0;
			unsigned short maxConnSilenceMs = 0;
			unsigned char fatalSilenceCoef = 0;
			unsigned char hops = 0;
			VS_TransformTransportReplyHandshake((net::HandshakeHeader *)__participant.m_recvBuf, key, type, resultCode, maxConnSilenceMs, fatalSilenceCoef, hops);
			if (resultCode) {
				eReturn = -1;
				printf("\nError : _RE_recvSock TransformTransportReplyHandshake fault. eCode = %d", resultCode);
				LeaveCriticalSection(&m_hCS4CNN);
				return;
			}
		}
		if (__participant.m_openSSL) {
			__participant.m_openSSL = __recvClientSecureHandshake(__participant);
		}
		__participant._rErrorConnection = false;
	}
	LeaveCriticalSection(&m_hCS4CNN);
	DoProductNextPacket(__participant);
	__participant.sock_alive = true;
	__participant.SendASync();
	__participant.RecvASync();
	return;
}
*/

signed long VS_PacketGenerator::DeleteParticipant(VS_TestParticipant **&tp) {
	EnterCriticalSection(&m_ccs); // ccs - Control Critical Section
	VS_TestParticipant **_temp = tp; *tp = NULL;
	delete *_temp;
	LeaveCriticalSection(&m_ccs);
	return 0;
};