#pragma once

#ifndef _VS_TESTPARTICIPANT_H_
#define _VS_TESTPARTICIPANT_H_

#include <winsock2.h>
#include <ws2tcpip.h>
#include <ras.h>
#include <iphlpapi.h>
#include <iprtrmib.h>
#include <MSTcpIP.h>
#include <windows.h>
#include <string>

#include "_testacstypes.h"

#include "../SecureLib/VS_SymmetricCrypt.h"

class VS_CoolTimer;

class VS_Packet {
public  :
	char *m_data;
	signed long m_transferred;
	signed long m_total;
	signed long m_sequence;
	VS_Packet(const signed long size);
	virtual ~VS_Packet();
};

class VS_Dumb_Socket {				// used only in nop operations
public  :
	WSAOVERLAPPED	m_Ov;			// overlapped classure
	vs_iocp_op		m_op_mode;		// operation mode
	VS_Dumb_Socket();
	virtual ~VS_Dumb_Socket();
};

class VS_Socket_Handle {
public  :
	WSAOVERLAPPED	m_Ov;			// overlapped classure
	vs_iocp_op		m_op_mode;		// operation mode
	SOCKET			m_Socket;		// socket handle
	HANDLE			&m_IOCP;		// iocp handle
	WSABUF			m_wsaBuf;		// wsa buffer
	signed long		m_lastError;	// last error
	signed long		m_lastAccTime;	// last access time
	sockaddr		m_sock_addr;	// target address
	VS_Packet		*m_packet;		// packet data
	VS_SymmetricCrypt *m_pwCrypt;	// used for data crypting
	VS_SymmetricCrypt *m_prCrypt;	// used for data crypting
	bool			m_openSSL;		// checked when OpenSSL presents
	signed long		m_sslRC;		// OpenSSL Reconnects Counter
	std::string m_epn;				// endpoint name for packet generator
	void *m_handshake;				// stored handshake data
	signed long m_lhandshake;		// stored handshake length
	VS_Socket_Handle(HANDLE &hiocp, const sockaddr &_iaddr, void *&iocp_key, const bool onesocket);	// default conclassor
	signed long DoOpenConnection(void *&iocp_key);	// initialize and connect socket
	signed long DoCloseConnection();// finalize and disconnect socket
	unsigned long SendSync(const char *buffer, const signed long _packetSize, SOCKET s);
	unsigned long RecvSync(char *buffer, const signed long _recvSize, SOCKET r);
	virtual ~VS_Socket_Handle();	// declassor
};

class VS_TestParticipant {
public  :
	std::string m_endpointName;		// endpoint name
	VS_Dumb_Socket   dumbSock;		// dumb nop Socket
	VS_Dumb_Socket   ctrlSock;		// dumb control Socket
	VS_Socket_Handle recvSock;		// recv Socket
	VS_Socket_Handle sendSock;		// send Socket
	volatile signed long m_ctrl;	// socket in control thread. must be 0 or 1 else thread sync error
	volatile signed long m_locked;	// participant lock via InterlockedExchange
	signed long m_error;			// participant errors count.
	VS_TestParticipant(HANDLE &hiocp, const sockaddr &_iaddr, void *iocp_key);	// default conclassor
	virtual ~VS_TestParticipant();	// declassor
};

class VS_ParticipantModifier {
public  :
	const VS_TestParticipant *m_vs_part_mod;
	VS_ParticipantModifier(const VS_TestParticipant *_mod);
	virtual VS_ParticipantModifier *DoCreateNew() = 0;
	virtual signed long DoModify() = 0;
	virtual ~VS_ParticipantModifier();
};

class VS_QoS_Modifier : public VS_ParticipantModifier {
private :
	static volatile LPWSAPROTOCOL_INFO QoSProtocol;
	static FLOWSPEC default_spec;
	static QOS default_qos;
	const char *modName;
	LPWSAPROTOCOL_INFO InitQoSProtocol();
	virtual VS_ParticipantModifier *DoCreateNew();
public  :
	const static vs_modifier_type m_modifer_type;
	VS_QoS_Modifier(const VS_TestParticipant *_mod);
	virtual signed long DoModify();
	virtual ~VS_QoS_Modifier();
};

class VS_Crypt_Modifier : public VS_ParticipantModifier {
private :
	const char *modName;
	virtual VS_ParticipantModifier *DoCreateNew();
public  :
	const static vs_modifier_type m_modifer_type;
	VS_Crypt_Modifier(const VS_TestParticipant *_mod);
	virtual signed long DoModify();
	virtual ~VS_Crypt_Modifier();
};

class VS_KeepAlive_Modifier : public VS_ParticipantModifier {
private :
	const char *modName;
	static tcp_keepalive default_alive;
	virtual VS_ParticipantModifier *DoCreateNew();
public  :
	const static vs_modifier_type m_modifer_type;
	VS_KeepAlive_Modifier(const VS_TestParticipant *_mod);
	virtual signed long DoModify();
	virtual ~VS_KeepAlive_Modifier();
};

class VS_Modifier_Holder { // must be singletone
private :
	std::map<signed long, std::vector<VS_ParticipantModifier *> > mem_holder;
public  :
	VS_Modifier_Holder();
	virtual ~VS_Modifier_Holder();
	signed long GetModifiersCount(const vs_modifier_type type);
	VS_ParticipantModifier *GetModifier(const vs_modifier_type type, const signed long number);
	signed long RegisterNewModifier(const vs_modifier_type type, VS_ParticipantModifier *);
};

#endif
