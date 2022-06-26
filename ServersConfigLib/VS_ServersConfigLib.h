/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 05.09.03     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file VS_ServerConfigLib.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_GetExecutablePath( char *name = 0, const unsigned nameSize = 0 );

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_SetLocalEName( const char *name );
bool VS_GetLocalEName( char *name, const unsigned nameSize );

bool VS_SetLocalENameInService( void );
void VS_SetWinMgmtInDependence( void );
bool VS_GetServiceEName( char *name, const unsigned nameSize );
bool VS_StartService( void );
bool VS_StopService( void );
void VS_RemoveService( void );
// val = -1 - return current set, val = 0 - clear, other set
int VS_ServiceWatchdog(int val);

// Возвращаемые значения.
//	0	Нет нас в списке сервисов или отказано в доступе.  // Fuck !!!
//	1	"The service continue is pending."
//	2	"The service pause is pending."
//	3	"The service is paused."
//	4	"The service is running."
//	5	"The service is starting."
//	6	"The service is stopping."
//	7	"The service is not running."
unsigned VS_GetServiceState( void );

bool VS_SetWorkingDir( const char *name );
bool VS_GetWorkingDir( wchar_t *name, const unsigned nameSize );
bool VS_GetWorkingDir( char *name, const unsigned nameSize );

void VS_SetWWWPInDependence( const bool isSet );
void VS_GetWWWPInDependence( bool *isSet );

bool VS_GetDefaultHost( char *host_name,
						    const unsigned long host_name_size );
unsigned VS_GetHosts( const char *host_name, char **ip = 0,
							const unsigned long ip_number = 0,
							const unsigned long ip_size = 0 );
bool VS_AcsLibInit( void );

/////////////////////////////////////////////////////////////////////////////////////////

void VS_ResetAcceptTCP( void );
unsigned VS_SetAcceptTCP( const char *host,
								const unsigned port );
unsigned VS_GetAcceptTCP( const unsigned nPosition, char *host,
								const unsigned hostSize, unsigned *port );
unsigned VS_GetCountAcceptTCP( void );

bool VS_SetAddressTranslation( const bool isUse );
bool VS_GetAddressTranslation( bool *isUse );

void VS_ResetConnectTCP( const bool is_current_user = false );
unsigned VS_SetConnectTCP( const char *host,
								const unsigned port, const bool is_current_user = false  );
unsigned VS_GetConnectTCP( const unsigned nPosition, char *host,
								const unsigned hostSize, unsigned *port );
unsigned VS_GetCountConnectTCP( void );
bool VS_CheckTCPPortAvailability(const char *ip, unsigned short port);

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_SetPoolThreadsParams( const unsigned workingThreads,
								const unsigned jobSize,
								const unsigned maxPriority,
								const unsigned maxLifetime );

bool VS_GetPoolThreadsParams( unsigned *workingThreads,
								unsigned *jobSize,
								unsigned *maxPriority,
								unsigned *maxLifetime );

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_SetDataBaseParams( const bool isDB,
								const char *user,
								const char *password,
								const char *string,
								unsigned connections );

bool VS_GetDataBaseParams( bool *isDB,
								char *user, const unsigned userSize,
								char *password, const unsigned passwordSize,
								char *string, const unsigned stringSize,
								unsigned* connections);

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_SetSmtpMailerParams( const char *server,
								const unsigned port,
								const unsigned maxPoolThreads,
								const char *directory,
								const unsigned checkPeriod,
								const unsigned externalcheckPeriod,
								const unsigned attemptsPeriod,
								const unsigned lifetime,
								const char *admin_email,
								const char *login,
								const char *password,
								int authType,
								bool useSSL);

bool VS_GetSmtpMailerParams( char *server, const unsigned serverSize,
								unsigned *port,
								unsigned *maxPoolThreads,
								char *directory, const unsigned directorySize,
								unsigned *checkPeriod,
								unsigned *externalcheckPeriod,
								unsigned *attemptsPeriod,
								unsigned *lifetime,
								char *adminEmail, const unsigned adminEmailSize,
								char *login, unsigned int loginSize,
								char *password, unsigned int passwordSize,
								int *authType,
								bool *useSSL);

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_SetConfPropParams( const unsigned moneyWarn,
								const unsigned moneyWarnPeriod,
								const unsigned decLimitPeriod );

bool VS_GetConfPropParams( unsigned *moneyWarn,
								unsigned *moneyWarnPeriod,
								unsigned *decLimitPeriod );

/////////////////////////////////////////////////////////////////////////////////////////
unsigned VS_SetRasHost( const char *host, const unsigned port );

unsigned VS_GetRasHost( const unsigned nPosition, char *host, const unsigned hostSize,
								unsigned *port );

int VS_GetH323HomeBrokers(char** Names, unsigned namesNum, const unsigned namesSize, unsigned *currNum);

unsigned VS_GetH323Parameters(char* bName, unsigned bNameSize,
							  char* gName, unsigned gNameSize,
							  unsigned *maxQuantityOfH323Terminals,
							  unsigned *maxQuantityOfSimCalls,
							  unsigned *maxQuantityOfBridgeConnections,
							  unsigned *maxQuantityOfGatewayConnections,
							  unsigned *isDirectlyConnectionOk);

unsigned VS_SetH323Parameters(const char *homeBroker,
							  const char *bName,
							  const char *gName,
							  const unsigned maxQuantityOfH323Terminals,
							  const unsigned maxQuantityOfSimCalls,
							  const unsigned maxQuantityOfBridgeConnections,
							  const unsigned maxQuantityOfGatewayConnections,
							  const unsigned isDirectlyConnectionOk);

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_SetCluster( const bool isSet, const unsigned hops, int maxOffline, int period );

bool VS_GetCluster( bool *isSet, unsigned *hops, int *maxOffline, int *period );

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_KillService();

/////////////////////////////////////////////////////////////////////////////////////////
