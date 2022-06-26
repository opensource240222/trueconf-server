/////////////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2002 Visicron Inc.  http://www.visicron.net/
//
//  Project:
//
//  Created: 09.12.03     by  A.Slavetsky
//
/////////////////////////////////////////////////////////////////////////////////////////
///
/// \file MediaBrokerConfigDll.h
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////////
void __declspec( dllexport ) VSSetApplication(int num);
bool __declspec( dllexport ) VSGetExecutablePath( char *name, const unsigned nameSize );

/////////////////////////////////////////////////////////////////////////////////////////

bool __declspec( dllexport ) VSSetLocalEName( const char *name );
bool __declspec( dllexport ) VSGetLocalEName( char *name, const unsigned nameSize );

bool __declspec( dllexport ) VSSetLocalENameInService( void );
void __declspec( dllexport ) VSSetWinMgmtInDependence( void );
bool __declspec( dllexport ) VSGetServiceEName( char *name, const unsigned nameSize );
bool __declspec( dllexport ) VSStartService( void );
bool __declspec( dllexport ) VSStopService( void );
void __declspec( dllexport ) VSRemoveService( void );
int __declspec( dllexport ) VSServiceWatchdog(int val);

// Возвращаемые значения.
//	0	Нет нас в списке сервисов или отказано в доступе.  // Fuck !!!
//	1	"The service continue is pending."
//	2	"The service pause is pending."
//	3	"The service is paused."
//	4	"The service is running."
//	5	"The service is starting."
//	6	"The service is stopping."
//	7	"The service is not running."
unsigned __declspec( dllexport ) VSGetServiceState( void );

bool __declspec( dllexport ) VSSetWorkingDir( const char *name );
bool __declspec( dllexport ) VSGetWorkingDir( char *name, const unsigned nameSize );

void __declspec( dllexport ) VSSetWWWPInDependence( const bool isSet );
void __declspec( dllexport ) VSGetWWWPInDependence( bool *isSet );

bool __declspec( dllexport ) VSGetDefaultHost( char *host_name,
												    const unsigned long host_name_size );
unsigned __declspec( dllexport ) VSGetHosts( const char *host_name, char **ip = 0,
									   const unsigned long ip_number = 0,
									   const unsigned long ip_size = 0 );
bool __declspec( dllexport ) VSAcsLibInit( void );

/////////////////////////////////////////////////////////////////////////////////////////

void __declspec( dllexport ) VSResetAcceptTCP( void );
unsigned __declspec( dllexport ) VSSetAcceptTCP( const char *host,
													const unsigned port );
unsigned __declspec( dllexport ) VSGetAcceptTCP( const unsigned nPosition, char *host,
													const unsigned hostSize, unsigned *port );
unsigned __declspec( dllexport ) VSGetCountAcceptTCP( void );

bool __declspec( dllexport ) VSSetAddressTranslation( const bool isUse );
bool __declspec( dllexport ) VSGetAddressTranslation( bool *isUse );

void __declspec( dllexport ) VSResetConnectTCP( void );
unsigned __declspec( dllexport ) VSSetConnectTCP( const char *host,
													const unsigned port );
unsigned __declspec( dllexport ) VSGetConnectTCP( const unsigned nPosition, char *host,
													const unsigned hostSize, unsigned *port );
unsigned __declspec( dllexport ) VSGetCountConnectTCP( void );

bool __declspec(dllexport) VSCheckTCPPortAvailability(const char *ip, unsigned short port);

/////////////////////////////////////////////////////////////////////////////////////////

unsigned __declspec( dllexport ) VSSetRasHost( const char *host, const unsigned port = 1719 );
unsigned __declspec( dllexport ) VSGetRasHost( const unsigned nPosition,
												char *host, const unsigned hostSize,
												unsigned *port );

unsigned __declspec( dllexport ) VSGetH323HomeBrokers(char** Names, unsigned namesNum, const unsigned namesSize, unsigned *currNum);
unsigned __declspec( dllexport ) VSGetH323Parameters(char* bName, unsigned bNameSize,
							  char* gName, unsigned gNameSize,
							  unsigned *maxQuantityOfH323Terminals,
							  unsigned *maxQuantityOfSimCalls,
							  unsigned *maxQuantityOfBridgeConnections,
							  unsigned *maxQuantityOfGatewayConnections,
							  unsigned *isDirectlyConnectionOk);
unsigned __declspec( dllexport ) VSSetH323Parameters(const char *homeBroker,
							  const char *bName,
							  const char *gName,
							  const unsigned maxQuantityOfH323Terminals,
							  const unsigned maxQuantityOfSimCalls,
							  const unsigned maxQuantityOfBridgeConnections,
							  const unsigned maxQuantityOfGatewayConnections,
							  const unsigned isDirectlyConnectionOk);

/////////////////////////////////////////////////////////////////////////////////////////

bool __declspec( dllexport ) VSSetPoolThreadsParams( const unsigned workingThreads,
														const unsigned jobSize,
														const unsigned maxPriority,
														const unsigned maxLifetime );

bool __declspec( dllexport ) VSGetPoolThreadsParams( unsigned *workingThreads,
														unsigned *jobSize,
														unsigned *maxPriority,
														unsigned *maxLifetime );

/////////////////////////////////////////////////////////////////////////////////////////

bool __declspec( dllexport ) VSSetDataBaseParams( const bool isDB,
													const char *user,
													const char *password,
													const char *string,
													unsigned connections );

bool __declspec( dllexport ) VSGetDataBaseParams( bool *isDB,
										char *user, const unsigned userSize,
										char *password, const unsigned passwordSize,
										char *string, const unsigned stringSize,
										unsigned* connections);

/////////////////////////////////////////////////////////////////////////////////////////

bool __declspec( dllexport ) VSSetSmtpMailerParams( const char *server,
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

bool __declspec( dllexport ) VSGetSmtpMailerParams( char *server, const unsigned serverSize,
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

bool __declspec( dllexport ) VSSetConfPropParams( const unsigned moneyWarn,
													const unsigned moneyWarnPeriod,
													const unsigned decLimitPeriod );

bool __declspec( dllexport ) VSGetConfPropParams( unsigned *moneyWarn,
													unsigned *moneyWarnPeriod,
													unsigned *decLimitPeriod );

/////////////////////////////////////////////////////////////////////////////////////////

bool __declspec( dllexport ) VSSetCluster( const bool isSet, const unsigned hops, int maxOffline, int period );

bool __declspec( dllexport ) VSGetCluster( bool *isSet, unsigned *hops, int *maxOffline, int *period );

/////////////////////////////////////////////////////////////////////////////////////////

bool __declspec( dllexport ) VSKillService(HWND hwnd, HINSTANCE hinst, LPTSTR lpCmdLine, int nCmdShow);

/////////////////////////////////////////////////////////////////////////////////////////
bool __declspec( dllexport ) VSInitTransport(HWND hwnd);
void __declspec( dllexport ) VSReleaseTransport();
// update func. type = 0 - add, 2-delete, 1-update
void __declspec( dllexport ) VSUpdateUser(const char* broker, const char* user, int type);
void __declspec( dllexport ) VSUpdateProp(const char* broker);
void __declspec( dllexport ) VSUpdateAddressBook(const char* broker, const char* user, const char * query, int ab);
/////////////////////////////////////////////////////////////////////////////////////////
bool __declspec( dllexport ) VSRegSerial(const char* server_id,
										 const char* server_name,
										 const char* serial,
										 const char *organization_name,
										 const char *country,
										 const char *contact_person,
										 const char *contact_email);

bool __declspec( dllexport ) VSRegSerialToFile(const char* server_id,
											   const char* server_name,
											   const char* serial,
											   const char *organization_name,
											   const char *country,
											   const char *contact_person,
											   const char *contact_email,
											   const char* to_file);

unsigned __declspec( dllexport ) VSRegFromFile(const char *from_file);

bool __declspec( dllexport ) VSRegSerialFromFile(const char* from_file,
												 const char* xml_file);

void __declspec( dllexport ) VSSendInvites(char *conf,
										   char *pass,
										   char** users,
										   int num);
void __declspec( dllexport ) VSDeleteConference(char *conf);

bool __declspec( dllexport )VSGetCertificateData(wchar_t *servEp, const unsigned servEpSz,
												wchar_t *country, const unsigned countrySz,
												wchar_t *organization, const unsigned organizationSz,
												wchar_t *contact_person, const unsigned contact_personSz,
												wchar_t *email, const unsigned emailSz,
												char *notBefore, const unsigned notBeforeSz,
												char *notAfter, const unsigned notAfterSz);

/////////////////////////////////////////////////////////////////////////////////////////
bool __declspec( dllexport ) VSGetSerializeInfo( void **serialize_buffer, unsigned long *size_buffer, const char *endpoint_name);
void __declspec( dllexport ) VSReleaseSerializeInfo( void* serialize_buffer);
/////////////////////////////////////////////////////////////////////////////////////////

bool __declspec( dllexport) VSGetServerIdentifyString(char *buf,unsigned *buf_sz);

void __declspec( dllexport ) VSSetLayout( char * conf, char * caster);
int __declspec(dllexport)  VSCopyRegKeys(char *s, char* d);
__declspec( dllexport ) char * GetUserLoginBySID(char *sSID);

#ifdef __cplusplus
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////
