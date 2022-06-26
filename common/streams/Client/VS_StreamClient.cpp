#if defined(_WIN32) // Not ported yet

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
/// \file VS_StreamClient.cpp
/// \brief Базовая реализация клиента протокола медиа потоков
/// \note
///

#include "VS_StreamClient.h"
#include "VS_StreamClientTypes.h"

unsigned   VS_StreamClient_Implementation::nClients = VS_StreamClient_Implementation::InitStreamClients();

bool VS_StreamClient_Implementation::directUdpConnEstablished = false;
VS_ConnectionUDP *VS_StreamClient_Implementation::udpConn   = 0;
VS_ConnectionUDP *VS_StreamClient_Implementation::udpConnV4 = 0;
VS_ConnectionUDP *VS_StreamClient_Implementation::udpConnV6 = 0;
VS_AcsLog		*VS_StreamClient_Implementation::m_udpConnLogger = 0;

VS_IPPortAddress VS_StreamClient_Implementation::bindUDPv4 = VS_IPPortAddress();
VS_IPPortAddress VS_StreamClient_Implementation::bindUDPv6 = VS_IPPortAddress();
HANDLE VS_StreamClient_Implementation::hThreadHandshakeDirectUDP = 0;


VS_StreamClient_Implementation   *VS_StreamClient_Implementation::headClient = 0,
									*VS_StreamClient_Implementation::endClient = 0;
CRITICAL_SECTION   VS_StreamClient_Implementation::sect;
unsigned long   VS_StreamClient_Implementation::acceptTimeout = 0;
const unsigned char   VS_StreamClient_Implementation::allTracks[] = { 127,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
unsigned long   VS_StreamClient_Implementation::clientIdSequence = 0;

VS_StreamClient::VS_StreamClient(stream::ClientType type) :
		imp(new VS_StreamClient_Implementation( type ))
{
	if (!imp)	return;
	if (imp->type != type) {	delete imp;	imp = 0;	}
}
// end VS_StreamClient::VS_StreamClient

VS_StreamClient::~VS_StreamClient( void )
{	if (imp)	delete imp;		}
// end VS_StreamClient::~VS_StreamClient

bool VS_StreamClient::AdhereToConnectionManager( void )
{	return VS_StreamClient_Implementation::AdhereToConnectionManager();		}
// end VS_StreamClient::AdhereToConnectionManager

///
/// \brief Ожидать TCP соединение на указанном локальном host,port
/// (default - host: имя машины, port: 9876/9874).
///
bool VS_StreamClient::Accept( const char *host, const unsigned short port, const unsigned long milliseconds, void *connectEvent, const bool waitForConnection )
{	return !imp ? false : imp->Accept( host, port, milliseconds, connectEvent, waitForConnection );		}
// end VS_StreamClient::Accept

///
/// \brief Присоединится по TCP к указанному удаленному host,port
/// (default - port: 9876/9874)
///
bool VS_StreamClient::Connect( const char *host, const unsigned short port, const unsigned long milliseconds, void *connectEvent, const bool waitForConnection )
{	return !imp ? false : imp->Connect( host, port, milliseconds, connectEvent, waitForConnection );	}
// end VS_StreamClient::Connect

///
/// \brief Присоединится по UDP на указанном local_host, к удаленному target_host
/// на ка-а-анкретном port-е(у)
///
bool VS_StreamClient::Connect( const unsigned short port, const char *host ,const long uid ,const bool isSimpleUdpStream, unsigned long * ip_out , unsigned short * port_out , const char *source_ip )
{	return !imp ? false : imp->Connect( host, port, uid, isSimpleUdpStream, ip_out, port_out, source_ip );	}
// end VS_StreamClient::Connect

///
/// \brief Установить соединение для conferenceName конференции, при этом мы ожидаем соединение.\n
/// В данной конференции мы имеем имя participantName,тот,от кого ждем соединение -
/// acceptedParticipantName
///
bool VS_StreamClient::Accept(const char *conferenceName, const char *participantName, const char *connectedParticipantName, const char *connectedEndpointName, const stream::Track* tracks, const unsigned nTracks, const unsigned long connectTimeout, void *connectEvent, const bool waitForConnection)
{	return !imp ? false : imp->Accept( conferenceName, participantName, connectedParticipantName, connectedEndpointName, tracks, nTracks, connectTimeout, connectEvent, waitForConnection );	}
// end VS_StreamClient::Accept

///
/// \brief Установить соединение для conferenceName конференции, при этом мы устанавливаем соединение.\n
/// В данной конференции мы имеем имя participantName,тот,к кому устанавливаем соединение\n
/// - connectedParticipantName
///
bool VS_StreamClient::Connect(const char *conferenceName, const char *participantName, const char *connectedParticipantName, const char *connectedEndpointName, const stream::Track* tracks, const unsigned nTracks, const unsigned long connectTimeout, void *connectEvent, const bool waitForConnection)
{	return !imp ? false : imp->Connect( conferenceName, participantName, connectedParticipantName, connectedEndpointName, tracks, nTracks, connectTimeout, connectEvent, waitForConnection );	}
// end VS_StreamClient::Connect

///
/// \brief Передать объекту готовое соединение.
///
bool VS_StreamClient::SetConnection( VS_ConnectionSock *conn, const unsigned long milliseconds )
{	return !imp ? false : imp->SetConnection( conn, milliseconds );		}
// end VS_StreamClient::SetConnection

///
/// \brief Закрыть соединение.
///
void VS_StreamClient::CloseConnection( void )
{	if (imp)	imp->CloseConnection();		}
// end VS_StreamClient::CloseConnection

///
/// \brief Ускоряет обмен пакетами для локальной сети.
///
void VS_StreamClient::SetFastStream(bool is_nagle_disable)
{	if (imp)	imp->SetFastStream( is_nagle_disable );		}
// end VS_StreamClient::SetFastStream

///
/// \brief Устанавливает биты приоритета трафика в IP-заголовке.
///
void VS_StreamClient::SetQOSStream(bool qos, _QualityOfService *qos_params)
{	if (imp)	imp->SetQOSStream( qos, qos_params );		}
// end VS_StreamClient::SetQoSSteram

///
/// \brief Устанавливает биты приоритета в IP-заголовке. Может изменять биты на работающем сокете.
///
void VS_StreamClient::SetQOSStreamEx(bool qos, _QualityOfService *qos_params)
{	if (imp)	imp->SetQOSStreamEx( qos, qos_params );		}
// end VS_StreamClient::SetQoSSteramEx

///
/// \brief При истечении данного connectTimeout на отсылку или прием фреймов клиент будет
///  автоматически закрывать соединение (по умолчанию VS_CONNECT_TIMEOUT = 20000).
///
bool VS_StreamClient::SetConnectTimeout( const unsigned long connectTimeout )
{	return !imp ? false : imp->SetConnectTimeout( connectTimeout );		}
// end VS_StreamClient::SetConnectTimeout

///
/// \brief Текущий Connect Timeout
///
unsigned long VS_StreamClient::ConnectTimeout( void ) const
{	return !imp ? 0 : imp->ConnectTimeout();	}
// end VS_StreamClient::ConnectTimeout

///
/// \brief Тип клиента: sender or receiver.
///
stream::ClientType VS_StreamClient::ClientType() const
{	return !imp ? stream::ClientType::uninstalled : imp->type;	}
// end VS_StreamClient::ClientType

///
/// \brief Тип соединения: accept, connect, uninstalled(в т.ч. SetConnection(...)).
///
VS_StreamClient_ConnectType VS_StreamClient::ConnectType( void ) const
{	return !imp ? vs_stream_client_connect_type_unconnected : imp->connectType;		}
// end VS_StreamClient::ConnectType

/*bool VS_StreamClient::Reconnect(  const unsigned long ip,
								const unsigned short port )
{	return !imp ? false : imp->Reconnect( ip , port, VS_RECONNECT_DEFAULT_TIMEOUT );
}*/
//end VS_StreamClient::Reconnect

/*bool VS_StreamClient::ConnectNHPServer(const char* host,const unsigned short port,const long uid, unsigned long timeout, bool isSimpleUdpStream,
									   unsigned long* ip_out,unsigned short* port_out)
{
	return !imp ? vs_stream_client_connect_type_unconnected : imp->ConnectNHPServer(host,port,
											uid,timeout,isSimpleUdpStream,ip_out,port_out);
}*/
bool VS_StreamClient::ConnectNHP(const char* Client_Endpoint,const char *Serv_Endpoint,const char *ConferenceName,
							void * connectEvent, const unsigned long timeout, const char *source_ip)
{
	return !imp ? vs_stream_client_connect_type_unconnected : imp->ConnectNHP(Client_Endpoint,Serv_Endpoint,
															ConferenceName, connectEvent,timeout, source_ip);
}
bool VS_StreamClient::InitUdpListener()
{
	return VS_StreamClient_Implementation::InitUdpListener();
}

void VS_StreamClient::FreeUdpListener()
{
	return VS_StreamClient_Implementation::FreeUdpListener();
}
bool VS_StreamClient::GetBindedUdpAddressV4(char *host, unsigned long host_sz, unsigned short &port)
{
	return VS_StreamClient_Implementation::GetBindedUdpAddressV4(host,host_sz,port);
}
bool VS_StreamClient::GetBindedUdpAddressV6(char *host, unsigned long host_sz, unsigned short &port)
{
	return VS_StreamClient_Implementation::GetBindedUdpAddressV6(host,host_sz,port);
}
bool VS_StreamClient::UdpIsBinded()
{
	return VS_StreamClient_Implementation::UdpIsBinded();
}

//bool VS_StreamClient::ConnectNHPClient(const VS_StreamClient_ConnectType connectType,
//					const char *conferenceName, const char *participantName,
//					const char *connectedParticipantName, const char *connectedEndpointName,
//					const unsigned char *tracks, const unsigned n_tracks,
//					const unsigned long connectTimeout,
//					void *connectEvent, const bool waitForConnection)
//{
//	return !imp ? vs_stream_client_connect_type_unconnected : imp->ConnectNHPClient(connectType,
//					conferenceName,participantName,connectedParticipantName,connectedEndpointName,
//					tracks, n_tracks,
//					connectTimeout,
//					connectEvent,waitForConnection);
//}

#endif
