/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: Базовая реализация клиента протокола медиа потоков
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_StreamClient.h
/// \brief Базовая реализация клиента протокола медиа потоков
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
	/// \brief Сообщить VS_ConnectionManager-у о том, что будут создаваться эти объекты.
	/// ( На данный момент это нужно для того, что бы VS_ConnectionManager мог ждать
	/// "запоздавший" Accept(... endpoint_name ...). Если такой ситтуации не предвидеться,
	/// то эту функцию запускать необязятельно. ). Вернет false, если VS_ConnectionManager
	/// не готов (,а он не всегда пионер (шутка)).
	///
	static bool		AdhereToConnectionManager( void );

	virtual bool IsValid() const { return imp != 0; }

	//  Ваня!!! Обрати внимание на два последних аргумента: connectEvent, waitForConnection
	//  Если ты не хочешь, что бы функция блокировалась, но при этом хочешь иметь событие
	//  ( SetEvent( EVENT ) ) на любой исход соединения (успех/неудача), то подставь HANDLE
	//  HANDLE EVENT-а в connectEvent. Если ты не хочешь дожидаться, то можешь делать
	//  и CloseConnection и delete clientObjects, а можешь комбинировать с этими аргументами
	//  как хочешь (например и wait и EVENT). (В реализации запускаються отдельные threads,
	//  которые при любом исходе пытаються доставить результат объекту, но если объекта уже
	//  нет или он CloseConnection(), то они тихо и мирно умирают вычистив все за собой.)
	//  При данном, default наборе аргументов все работает по старому.
	///
	/// \brief Создать TCP соединение на указанном локальном host,port
	/// (default - host: имя машины, port: 4444/4445).
	///
	bool	Accept				( const char *host = 0,
									const unsigned short port = 0,
									const unsigned long milliseconds = VS_CONNECT_TIMEOUT,
									void *connectEvent = 0,
									const bool waitForConnection = true );

	///
	/// \brief Присоединится по TCP к указанному удаленному host,port
	/// (default - port: 4444/4445)
	///
	bool	Connect				( const char *host,
									const unsigned short port = 0,
									const unsigned long milliseconds = VS_CONNECT_TIMEOUT,
									void *connectEvent = 0,
									const bool waitForConnection = true );

	///
	/// \brief Присоединится по UDP на указанном host,port,flag-если не 0 - то мультикас
	///
	/*bool		Reconnect				(	const unsigned long ip,
										const unsigned short port );*/


    bool		Connect				(	 const unsigned short port ,const char *host,
									const long uid, bool isSimpleUdpStream = false,
									unsigned long * ip_out = 0, unsigned short * port_out =0, const char *source_ip = NULL );
	///
	///\breaf Присоединится и сделать NHP handshake
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
	/// \brief Установить соединение для conferenceName конференции, при этом мы ожидаем соединение.\n
	/// В данной конференции мы имеем имя participantName,тот,от кого ждем соединение -
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
	/// \brief Установить соединение для conferenceName конференции, при этом мы устанавливаем соединение.\n
	/// В данной конференции мы имеем имя participantName,тот,к кому устанавливаем соединение\n
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
	/// \brief Ускоряет обмен пакетами для локальной сети.
	///
	void      SetFastStream     ( bool is_nagle_disable = true );

	///
	/// \brief Устанавливает биты приоритета в IP-заголовке.
	///
	void      SetQOSStream     (bool qos = false, _QualityOfService *qos_params = NULL);

	///
	/// \brief Устанавливает биты приоритета в IP-заголовке. Может изменять биты на работающем сокете.
	///
	void      SetQOSStreamEx   (bool qos = false, _QualityOfService *qos_params = NULL);

	///
	/// \brief Передать объекту готовое соединение.
	///
	bool	SetConnection		( VS_ConnectionSock *conn,
									const unsigned long milliseconds = VS_CONNECT_TIMEOUT );

	///
	/// \brief Закрыть соединение.
	///
	void	CloseConnection		( void );

	///
	/// \brief При истечении данного connectTimeout на отсылку или прием фреймов клиент будет
	///  автоматически закрывать соединение (по умолчанию connectTimeout = VS_CONNECT_TIMEOUT).
	///
	bool			SetConnectTimeout	( const unsigned long connectTimeout = VS_CONNECT_TIMEOUT );

	///
	/// \brief Текущий Connect Timeout
	///
	unsigned long	ConnectTimeout		( void ) const;

	///
	/// \brief Тип клиента: sender or receiver.
	///
	stream::ClientType ClientType() const;

	///
	/// \brief Тип соединения: unconnected, accept, connect, set.
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
