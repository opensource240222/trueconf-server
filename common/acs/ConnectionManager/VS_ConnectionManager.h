/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project: Менеджер соединений клиента
//
//  Created: 12.11.02     by  A.Slavetsky
//
/**
 **************************************************************************
 * \file VS_ConnectionManager.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Static pie of TCP-UDP-HTTP/IP communication funtions of clients.
 *
 * Also there is work with regestry that connected with connections.
 *
 * \b Project ManagerOfConnections
 * \author SlavetskyA
 * \date 12.11.02
 *
 * $Revision: 3 $
 *
 * $History: VS_ConnectionManager.h $
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 7.08.12    Time: 21:42
 * Updated in $/VSNA/acs/ConnectionManager
 * - static var isDisableNagle  removed
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:01
 * Updated in $/VSNA/acs/connectionmanager
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 10.10.07   Time: 19:27
 * Updated in $/VS2005/acs/connectionmanager
 * Checking server throu proxy added through proxy
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 20.03.07   Time: 19:44
 * Updated in $/VS2005/acs/connectionmanager
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:50
 * Created in $/VS2005/acs/connectionmanager
 *
 * *****************  Version 36  *****************
 * User: Mushakov     Date: 19.05.06   Time: 12:42
 * Updated in $/VS/acs/ConnectionManager
 *
 * *****************  Version 35  *****************
 * User: Mushakov     Date: 23.03.06   Time: 20:00
 * Updated in $/VS/acs/ConnectionManager
 *
 * *****************  Version 34  *****************
 * User: Avlaskin     Date: 17.12.04   Time: 16:59
 * Updated in $/VS/acs/ConnectionManager
 *
 ****************************************************************************/


#ifndef VS_CONNECTION_MANAGER_H
#define VS_CONNECTION_MANAGER_H

#include "../../net/EndpointRegistry_fwd.h"
#include "../../net/Handshake_fwd.h"

#include <vector>
#include <memory>

class VS_ConnectionSock;
class VS_ConnectionTCP;
class VS_AcsLog;
struct _QualityOfService;

typedef   void (*VS_AcceptHandlerCallback)( const void *cb_arg,
												VS_ConnectionTCP *conn,
												net::HandshakeHeader *hs);
///
/// \brief Инициализация менеджера соединений
/// \param our_endpoint - имя текущего приложения в endpoints-системе адресации
///
bool VS_InstallConnectionManager( const char *our_endpoint );

///
/// \brief Деинициализация менеджера соединений
///
void VS_UninstallConnectionManager( void );

///
/// \brief Вернуть our_endpoint системы менеджера соединений,если она инициализирована
///
char *VS_EndpointName( char *our_endpoint,
						const unsigned size_endpoint, unsigned *necessary_size = 0 );
///
/// \brief Устанавливает новое имя текущего приложения в endpoints-системе адресации,
///  старое имя останется в виде алиаса, т.е. приходящие на старое имя пакеты будут по
///  прежнему доставляться нашим сервисам
/// \param our_endpoint - добавляемое новое,основное имя текущего приложения в
///  endpoints-системе адресации
///
bool VS_SetEndpointName( const char *our_endpoint );

///
/// \brief Проверяет является ли данное имя endpoint именем endpoint нашего приложения,
///  причем проверяется на равенство и алиас (см.VS_SetEndpointName(...))
/// \param endpoint - проверяемое,как наше,имя endpoint
///
bool VS_IsOurEndpointName( const char *endpoint );

///
/// \brief Устанавливает интерфейс для идентификаций USER + PASSWD соединений
///
bool VS_SetAuthenticationInterface( class VS_Authentication *auth );
///
/// \brief Должна возвращать: 0 - не устанавливать соединение (аналогично Esc)
///                           1 - вернула значения, но не запоминать их да-же в RUNTIME
///                           2 - вернула значения, запомнить их в RUNTIME
///                           3 - вернула значения, запомнить их в CRYPT-REGISTRY
class VS_Authentication
{	public:		virtual unsigned Request( const char *protocol, const char *proxy,
											const unsigned short port,
											char *user, const unsigned user_size,
											char *passwd, const unsigned passwd_size ) = 0;
};	// end VS_Authentication class

///
/// \brief Сбрасывает ранее полученные и используемые USER + PASSWD
///
void VS_ResetAuthenticationInformation( void );

///
/// \brief Удаляет интерфейс для идентификаций USER + PASSWD соединений
/// (но не сбрасывает ранее полученные и используемые USER + PASSWD)
///
void VS_ResetAuthenticationInterface( void );

///
/// \brief Инициализировать listen порты,описанные в registry для данного приложения
///
int VS_EstablishAcceptPorts( void );

///
/// \brief Дать информацию о инициализированных listen портах в виде массива указателей на
///  net::endpoint::ConnectTCP. Если connectTCP равно 0 функция вернет количество listen портов.
///
void VS_GetEstablishedAcceptPorts(std::vector<std::unique_ptr<net::endpoint::ConnectTCP>>& connectTCP);

///
/// \brief Деинициализировать listen порты,описанные в registry для данного приложения
///
int VS_RemoveAcceptPorts( void );

///
/// \brief Добавить callback с первичной строкой ожидаемого протокола (в нашем случае это streams
/// и transport),который будет вызываться при получении соединения и чтении первого пакета
/// handshake с аналогичной первичной строкой
/// \param startHandshakeString - первичная строка handshake
/// \param cb - функция callback
/// \param cb_arg - аргумент, который будет передаваться в эту функцию
/// \param is_disable_nagle - если true, то для локалной сети убытряется обмен
/// \param qos - если true, в заголовке IP-пакета выставляются биты приоритета
/// \param qos_params - может передавать структуру данных _QualityOfService, отличную от используемой по умолчанию
///
int VS_AddAcceptHandler( const char *startHandshakeString,
										const VS_AcceptHandlerCallback cb,
										const void *cb_arg = 0 ,
										const bool is_disable_nagle = true,
										const bool qos = false,
										_QualityOfService *qos_params = nullptr );

///
/// \brief Удалить callback,вызываемый при получении соединения
/// \param startHandshakeString - первичная строка handshake
///
int VS_DeleteAcceptHandler( const char *startHandshakeString );

///
/// \brief Удалить все callbacks, вызываемые при получении соединений
///
int VS_DeleteAllAcceptHandlers( void );

///
/// \brief Проверить доступность endpoint's host непосредственно (только для TCP-based протоколов)
/// \brief В success_rc будут положены указатели из rc успешно прошедшие тест
/// \brief Возвращает количество успешных
///
void VS_TestAccessibility(
	const std::vector<std::unique_ptr<net::endpoint::ConnectTCP>>& rc,
	std::vector<net::endpoint::ConnectTCP*>& success_rc,
	const char* tested_endpoint,
	unsigned long& milliseconds
);
///
/// \brief Создать соединение для endpoint,если он описан в Registry, на базе UDP протокола
/// \param endpoint - имя endpoint
///
VS_ConnectionSock *VS_CreateNotGuaranteedConnection( const char *endpoint );
///
/// \brief Создать соединение для endpoint,если он описан в Registry
/// \param endpoint - имя endpoint
/// \param milliseconds - если протокол требует своего,внутреннего handshake,то ограничить этот
///                процесс milliseconds
/// \param canBeNotGuaranteed - если значение равно true,то попытаться сначала установить UDP,
///                      при неудаче TCP подобный протокол
///
/// \param qos - если true, в заголовке IP-пакета выставляются биты приоритета
/// \param qos_params - может передавать структуру данных _QualityOfService, отличную от используемой по умолчанию
VS_ConnectionSock *VS_CreateConnection( const char *endpoint,
												unsigned long &milliseconds,
												unsigned *nTCP = 0,
												const bool canBeNotGuaranteed = false,
												class VS_CreationAttempts *attempts = 0 ,
												const bool is_disable_nagle = true,
												const bool qos = false,
												_QualityOfService *qos_params = nullptr );


VS_ConnectionSock* VS_CreateConnection(net::endpoint::ConnectTCP& rc, unsigned long& milliseconds, const bool is_disable_nagle);


class VS_CreationAttempts
{	public:		virtual bool Attempt( unsigned nTCP, const net::endpoint::ConnectTCP* rc ) = 0;
};	// end VS_CreationAttempts class

#define   VS_CM_MAX_TCP_LIKE_ACCEPT_PORTS      20
#define   VS_CM_MAX_TCP_LIKE_ACCEPT_HANDLERS   20
#define	  VS_CM_RANGE_FOR_ACCEPT_PORTS			8
#define   VS_CM_MIN_TCP_LIKE_HANDSHAKE_TIMEOUT   6000
#define   VS_CM_MAX_TCP_LIKE_HANDSHAKE_TIMEOUT   30000
#define   VS_CM_MIN_TEST_ACCESSIBILITY_TIMEOUT   1000
#define   VS_CM_MAX_TEST_ACCESSIBILITY_TIMEOUT   60000
#define   VS_CM_STR_ALL_CURRENT		""
#define   VS_CM_STR_ALL_CURRENT1	"ALL CURRENT"
#define   VS_CM_STR_ALL_CURRENT2	"ALL_CURRENT"
#define   VS_CM_STR_ALL_CURRENT3	"AllCurrent"
extern char   vs_str_all_current[],		// VS_CM_STR_ALL_CURRENT
				vs_str_all_current1[],	// VS_CM_STR_ALL_CURRENT1
				vs_str_all_current2[],	// VS_CM_STR_ALL_CURRENT2
				vs_str_all_current3[];	// VS_CM_STR_ALL_CURRENT3

extern VS_AcsLog   *tAcsLog;


inline VS_ConnectionTCP* VS_Connect_TCP(const net::endpoint::ConnectTCP& rc, const bool isDisableNagle, unsigned long& mills, bool& wasChange);
inline VS_ConnectionTCP* VS_Pass_Socks(net::endpoint::ConnectTCP& rc, const bool isDisableNagle, unsigned long& mills, bool& wasChange);
inline VS_ConnectionTCP* VS_Pass_HttpTnl(net::endpoint::ConnectTCP& rc, const bool isDisableNagle, unsigned long& mills, bool& wasChange);
inline VS_ConnectionTCP* VS_Pass_HttpTnlMsq(net::endpoint::ConnectTCP& rc, const bool isDisableNagle, unsigned long& mills, bool& wasChange);
inline VS_ConnectionTCP* VS_Pass_InternetOptions(net::endpoint::ConnectTCP& rc, const bool isDisableNagle, unsigned long& mills, bool& wasChange);

#endif // VS_CONNECTION_MANAGER_H
