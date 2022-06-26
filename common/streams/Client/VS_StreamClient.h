/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: ������� ���������� ������� ��������� ����� �������
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_StreamClient.h
/// \brief ������� ���������� ������� ��������� ����� �������
/// \note
///


#ifndef VS_STREAM_CLIENT_H
#define VS_STREAM_CLIENT_H

#include "../fwd.h"
#include "acs/connection/VS_ConnectionSock.h"

struct _QualityOfService;

enum VS_StreamClient_ConnectType { vs_stream_client_connect_type_unconnected,
									vs_stream_client_connect_type_accept,
									vs_stream_client_connect_type_connect,
									vs_stream_client_connect_type_host_accept,
									vs_stream_client_connect_type_host_connect,
									vs_stream_client_connect_type_set,
									vs_stream_client_connect_not_guaranteed };
#define   VS_CONNECT_TIMEOUT   20000

class VS_StreamClient
{
public:
			VS_StreamClient	(stream::ClientType type);
	virtual	~VS_StreamClient( void );
	struct VS_StreamClient_Implementation   *imp;

	///
	/// \brief �������� VS_ConnectionManager-� � ���, ��� ����� ����������� ��� �������.
	/// ( �� ������ ������ ��� ����� ��� ����, ��� �� VS_ConnectionManager ��� �����
	/// "�����������" Accept(... endpoint_name ...). ���� ����� ��������� �� ������������,
	/// �� ��� ������� ��������� �������������. ). ������ false, ���� VS_ConnectionManager
	/// �� ����� (,� �� �� ������ ������ (�����)).
	///
	static bool		AdhereToConnectionManager( void );

	virtual bool IsValid() const { return imp != 0; }

	//  ����!!! ������ �������� �� ��� ��������� ���������: connectEvent, waitForConnection
	//  ���� �� �� ������, ��� �� ������� �������������, �� ��� ���� ������ ����� �������
	//  ( SetEvent( EVENT ) ) �� ����� ����� ���������� (�����/�������), �� �������� HANDLE
	//  HANDLE EVENT-� � connectEvent. ���� �� �� ������ ����������, �� ������ ������
	//  � CloseConnection � delete clientObjects, � ������ ������������� � ����� �����������
	//  ��� ������ (�������� � wait � EVENT). (� ���������� ������������ ��������� threads,
	//  ������� ��� ����� ������ ��������� ��������� ��������� �������, �� ���� ������� ���
	//  ��� ��� �� CloseConnection(), �� ��� ���� � ����� ������� �������� ��� �� �����.)
	//  ��� ������, default ������ ���������� ��� �������� �� �������.
	///
	/// \brief ������� TCP ���������� �� ��������� ��������� host,port
	/// (default - host: ��� ������, port: 4444/4445).
	///
	bool	Accept				( const char *host = 0,
									const unsigned short port = 0,
									const unsigned long milliseconds = VS_CONNECT_TIMEOUT,
									void *connectEvent = 0,
									const bool waitForConnection = true );

	///
	/// \brief ������������� �� TCP � ���������� ���������� host,port
	/// (default - port: 4444/4445)
	///
	bool	Connect				( const char *host,
									const unsigned short port = 0,
									const unsigned long milliseconds = VS_CONNECT_TIMEOUT,
									void *connectEvent = 0,
									const bool waitForConnection = true );

	///
	/// \brief ������������� �� UDP �� ��������� host,port,flag-���� �� 0 - �� ���������
	///
	/*bool		Reconnect				(	const unsigned long ip,
										const unsigned short port );*/


    bool		Connect				(	 const unsigned short port ,const char *host,
									const long uid, bool isSimpleUdpStream = false,
									unsigned long * ip_out = 0, unsigned short * port_out =0, const char *source_ip = NULL );
	///
	///\breaf ������������� � ������� NHP handshake
	///
	/*bool		ConnectNHPServer(		const char *host,const unsigned short port,
									const long uid, const unsigned long timeout, bool isSimpleUdpStream = false,
									unsigned long * ip_out = 0, unsigned short * port_out = 0);*/
	//bool		ConnectNHPClient( const VS_StreamClient_ConnectType connectType,
	//				const char *conferenceName, const char *participantName,
	//				const char *connectedParticipantName, const char *connectedEndpointName,
	//				const unsigned char *tracks, const unsigned n_tracks,
	//				const unsigned long connectTimeout,
	//				void *connectEvent, const bool waitForConnection);

	bool		ConnectNHP(const char* Cl_Endpoint,const char *Serv_Endpoint,const char *ConferenceName,
							void * connectEvent, const unsigned long timeout, const char *source_ip = NULL );


	///
	/// \brief ���������� ���������� ��� conferenceName �����������, ��� ���� �� ������� ����������.\n
	/// � ������ ����������� �� ����� ��� participantName,���,�� ���� ���� ���������� -
	/// acceptedParticipantName
	///
	bool	Accept				( const char *conferenceName,
									const char *participantName,
									const char *connectedParticipantName,
									const char *connectedEndpointName,
									const stream::Track* tracks,
									const unsigned nTracks,
									const unsigned long connectTimeout = VS_CONNECT_TIMEOUT,
									void *connectEvent = 0,
									const bool waitForConnection = false );

	///
	/// \brief ���������� ���������� ��� conferenceName �����������, ��� ���� �� ������������� ����������.\n
	/// � ������ ����������� �� ����� ��� participantName,���,� ���� ������������� ����������\n
	/// - connectedParticipantName
	///
	bool	Connect				( const char *conferenceName,
									const char *participantName,
									const char *connectedParticipantName,
									const char *connectedEndpointName,
									const stream::Track* tracks,
									const unsigned nTracks,
									const unsigned long connectTimeout = VS_CONNECT_TIMEOUT,
									void *connectEvent = 0,
									const bool waitForConnection = false );
	///
	/// \brief �������� ����� �������� ��� ��������� ����.
	///
	void      SetFastStream     ( bool is_nagle_disable = true );

	///
	/// \brief ������������� ���� ���������� � IP-���������.
	///
	void      SetQOSStream     (bool qos = false, _QualityOfService *qos_params = NULL);

	///
	/// \brief ������������� ���� ���������� � IP-���������. ����� �������� ���� �� ���������� ������.
	///
	void      SetQOSStreamEx   (bool qos = false, _QualityOfService *qos_params = NULL);

	///
	/// \brief �������� ������� ������� ����������.
	///
	bool	SetConnection		( VS_ConnectionSock *conn,
									const unsigned long milliseconds = VS_CONNECT_TIMEOUT );

	///
	/// \brief ������� ����������.
	///
	void	CloseConnection		( void );

	///
	/// \brief ��� ��������� ������� connectTimeout �� ������� ��� ����� ������� ������ �����
	///  ������������� ��������� ���������� (�� ��������� connectTimeout = VS_CONNECT_TIMEOUT).
	///
	bool			SetConnectTimeout	( const unsigned long connectTimeout = VS_CONNECT_TIMEOUT );

	///
	/// \brief ������� Connect Timeout
	///
	unsigned long	ConnectTimeout		( void ) const;

	///
	/// \brief ��� �������: sender or receiver.
	///
	stream::ClientType ClientType() const;

	///
	/// \brief ��� ����������: unconnected, accept, connect, set.
	///
	VS_StreamClient_ConnectType		ConnectType( void ) const;

	//static bool InitUdpListener(const char *host, const unsigned short port);
	static bool InitUdpListener();
	static bool UdpIsBinded();
	static bool GetBindedUdpAddressV4(char *host, unsigned long host_sz, unsigned short &port);
	static bool GetBindedUdpAddressV6(char *host, unsigned long host_sz, unsigned short &port);
	static void FreeUdpListener();
};
// end VS_StreamClient class

#define   _MD_CONNECT_TYPE_(val)   (char *)(val == vs_stream_client_connect_type_unconnected ? "vs_stream_client_connect_type_unconnected" : (val == vs_stream_client_connect_type_accept ? "vs_stream_client_connect_type_accept" : (val == vs_stream_client_connect_type_connect ? "vs_stream_client_connect_type_connect" : (val == vs_stream_client_connect_type_host_accept ? "vs_stream_client_connect_type_host_accept" : (val == vs_stream_client_connect_type_host_connect ? "vs_stream_client_connect_type_host_connect" : (val == vs_stream_client_connect_type_set ? "vs_stream_client_connect_type_set" : "???"))))))

#endif  // VS_STREAM_CLIENT_H
