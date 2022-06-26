/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: Непосредственная реализация поддержки управляющего протокола на клиенте
//
//  Created: 12.11.02     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_TransportClient.h
/// \brief Непосредственная реализация поддержки управляющего протокола на клиенте
/// \note
///

#ifndef VS_TRANSPORT_CLIENT_H
#define VS_TRANSPORT_CLIENT_H

#include <Windows.h>

class VS_SimpleStr;
class VS_WideStr;
class VS_EndpointCallback;

///
/// \brief Инициализация транспортной системы клиента
///
bool VS_InstallTransportClient( VS_EndpointCallback *epcb = 0 );

///
/// \brief Деинициализация транспортной системы клиента
///
void VS_UninstallTransportClient( void );

///
/// \brief Вернуть TransportCID, если он был установлен ранее для транспортной системы клиента
///
char *VS_TransportClientCID( char *cid, const unsigned size_cid,
													unsigned *necessary_size = 0);

char *VS_TransportClientCID( const char *server_id, char *cid, const unsigned size_cid,
													unsigned *necessary_size = 0);
///
/// \brief Установить Default маршрут для неизвестных адрессатов
///
bool VS_SetTransportClientDefaultGate( const char *gate_endpoint );

///
/// \brief Закрыть соединение по контрольному транспорту с дрегим endpoint,если оно существовало
/// \param endpoint - имя endpoint
///
void VS_DisconnectEndpoint( const char *endpoint );

void VS_FullDisconnectEndpoint(const char *otherEndpoint );

///
/// \brief Регистрация сервиса приложения в системе адрессации endpoint+services
/// \param service_name - имя устанавливаемого сервиса
/// \param threadId - Thread's ID которому будут отсылаться через WinMSG Requests и Replies
///            контрольного транспорта
/// \param msgType - integer value of the message value to be retrieved
///
bool VS_RegisterService( const char *service_name, const DWORD threadId,
											const UINT msgType );
///
/// \brief Регистрация сервиса приложения в системе адрессации endpoint+services
/// \param service_name - имя устанавливаемого сервиса
/// \param hWnd - Handle to Window которому будут отсылаться через WinMSG Requests и Replies
///            контрольного транспорта
/// \param msgType - integer value of the message value to be retrieved
///
bool VS_RegisterService( const char *service_name, const HWND hWnd,
											const UINT msgType );
///
/// \brief Проверить,существует ли уже service с таким именем и если существует,то вернуть
/// его регистрационные параметры
///
bool VS_IsServiceExists( const char *service_name, DWORD *threadId,
											HWND *hWnd, UINT *msgType );
///
/// \brief Удалить service с таким именем,если существует
///
void VS_UnregisterService( const char *service_name );


/**
\brief Получить данные сертификата сервера, с которым в данный момент установлено соединение.

@param[out] serverEP		- EndpointName (поле commonName)
@param[out] country			- страна
@param[out] organization	- название организации
@param[out] contact_person	- контактное лицо (поле surname)
@param[out] email			- мыло
@param[out] notBefore		- дата и время начала действия сертификата
@param[out] notAfter		- дата и время окончания действия сертификата

@return	<b>true</b>, если все нормально, <b>false</b> если установленное соединение не
					секьюрное или если оно не установлено.

*/
bool VS_GetCertificateData(	VS_WideStr &serverEP,
							VS_WideStr &country,
						    VS_WideStr &organization,
							VS_WideStr &contact_person,
							VS_WideStr &email,
							VS_SimpleStr &notBefore,
							VS_SimpleStr &notAfter);

///
/// \brief Возвращает последний ip адрес endpointa, соединенного с сервером server_endpoint
/// \param server_endpoint - имя endpoint сервера
void VS_GetEndpointSourceIP( const char *endpoint, char *dest, signed long &length);

///
/// \brief Описание Requests и Replies,которыми оперирует наш транспорт
/// !!! ВНИМАНИЕ - прочие базовые методы смотри в базовом классе VS_TransportMessage,
/// который описан в ../Message/VS_TransportMessage.h. (Comrades! They will not pass!(joke))
///

bool VS_GetTransportClientDefaultGate(char *gate_endpoint, unsigned long len);

#include "../Message/VS_TransportMessage.h"
class VS_ClientMessage : public VS_TransportMessage
{
public:
	VS_ClientMessage( void );
	VS_ClientMessage( MSG *msg );
	VS_ClientMessage( const char *our_service, const char *add_string,
						const char *endpoint, const char *service,
						const unsigned long ms_timelimit,
						const void *body, const unsigned long size,
						const char * to_user=0, const char * from_user=0,
						const char * to_server=0, const char *from_server=0);

	virtual ~VS_ClientMessage( void );
	/// These methods for work with received requests and replies
	bool			Set( MSG *msg );
	/// Return Message Sequence > 0 for recognition Request or Reply, 0 - error
	unsigned long	Send( bool dropGate = false );


	/// Return Message Sequence > 0, 0 - error
	unsigned long	Reply( const unsigned long ms_timelimit,
							const void *body, const unsigned long size );
	/// Return Message Sequence > 0, 0 - timeout
	/// leaveOther - to read other types and to ignore or leave in queue
	unsigned long	Recv( const DWORD milliseconds, const UINT msgType );
	unsigned long	Recv( const DWORD milliseconds, const UINT msgTypeMin, const UINT msgTypeMax );
	unsigned long   isOurMessage;
	bool			dropGate;
private:
	friend struct   VS_TransportClient_Service;
	friend struct   VS_TransportClient_Endpoint;
	friend inline void ProcessingManaging(class VS_ClientMessage&);
	unsigned long	SendAct( bool dropGate = false );
};
// end VS_ClientMessage class

extern const MSG   zeroMsg;

//d78 callback connection interface
enum VS_ConnectionErrors {
	err_disconnect	 =  1, // disconnect if connected
	err_ok			 =  0, // connect ok
	err_inaccessible = -1, // inaccessible
	err_antikclient	 = -2, // old client
	err_antikserver	 = -3, // old server
	err_alienserver	 = -4, // alien (foreign) server
	err_noresources	 = -5, // no resources (memory or something else)
	err_serverbusy	 = -6,

	err_ssl_could_not_established	= -7,
	err_cert_expired				= -8,
	err_cert_not_yet_valid			= -9,
	err_cert_is_invalid				= -10,
	err_srv_name_is_invalid			= -11,
	err_version_do_not_match		= -12,


	err_pending		 = -0xFFA, // init value = wainting for request result
	err_invalid		 = -0xFFB, // invalid state due to uninstalltransport or other reason
	err_fin			 = -0xFFC  // callback already called for this request
};

class VS_EndpointCallback
{
public:
	virtual void OnConnect(const char *to_name, const long error = 0) = 0;
	virtual void OnSidChange(const char *old_name, const char *new_name) = 0;
};

///
/// \brief Соединить ендпоинт с другим по имени.
/// Результат возвращается в callback класс после таймаута.
bool VS_CreateConnect( const char *to_name, int timeout = 20000);


#endif  // VS_TRANSPORT_CLIENT_H
