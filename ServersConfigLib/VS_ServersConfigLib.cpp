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
//////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#include "VS_ServersConfigLib.h"
#include "../common/tools/Restarter/Restarter.h"
#include "../common/tools/Server/VS_Server.h"
#include "../common/std/cpplib/VS_RegistryKey.h"
#include "../common/std/cpplib/VS_RegistryConst.h"
#include "../common/std/cpplib/VS_Endpoint.h"
#include "../common/std/VS_RegServer.h"
//#include "../../Servers/DirectoryServer/Services/VS_Registry.h"
#include "../common/net/EndpointRegistry.h"
#include "../common/acs/Lib/VS_AcsLib.h"

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

//static inline bool GetServiceEndpoint( char *, const unsigned );
static inline bool SetServiceEndpoint( void );
static inline bool StartService( void );
static inline bool StopService( void );
static inline void RemoveService( void );
static inline unsigned GetServiceState( void );
static inline void SetWWWPInDependence( const bool );
static inline void SetWinMgmtInDependence( void );
static inline void ResetWinMgmtInDependence( void );

/////////////////////////////////////////////////////////////////////////////////////////

static char   *endpointName = 0, *exePath = 0;

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

bool VS_GetExecutablePath( char *name, const unsigned nameSize )
{
	if (exePath) {		free( (void *)exePath );	exePath = 0;	}
	VS_RegistryKey key((const char*) VS_Server::RegistryKey(), false, CONFIGURATION_KEY);
	if (!key.IsValid())		return false;
	std::string folder;
	key.GetString(folder, EXECUTABLE_PATH_TAG_NAME);
	if (!folder.empty())
		exePath = _strdup(folder.c_str());
	if (!exePath)	return false;
	if (name && nameSize) {		strncpy( name, exePath, nameSize - 1 );
								name[nameSize - 1] = 0;		}
	return true;
}
// end VS_GetExecutablePath

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_SetLocalEName( const char *name )
{
	if (!name || !*name)		return false;

	VS_RegistryKey cfg(false,"",false,true);
	if(!cfg.IsValid())
		return false;

	if(!cfg.SetString(name, VS_SERVER_ENDPOINT_ARG_PREF))
		return false;

	if (endpointName)	free( (void *)endpointName );
	endpointName = _strdup( name );
	return endpointName != 0;
}
// end VS_SetLocalEName

bool VS_GetLocalEName( char *name, const unsigned nameSize )
{
	if (!name || nameSize < 6)		return false;
	if (!endpointName){
		VS_RegistryKey cfg(false,"",false,true);
		std::string ep;
		if (!cfg.GetString(ep, VS_SERVER_ENDPOINT_ARG_PREF))
			return false;
		endpointName=_strdup(ep.c_str());
	}

	if (endpointName){
		strncpy( name, endpointName, (size_t)( nameSize - 1 ));
		return true;
	}
	return false;
}
// end VS_GetLocalEName

bool VS_SetLocalENameInService( void )
{
	return SetServiceEndpoint();
}
// end VS_SetLocalENameInService
void VS_SetWinMgmtInDependence( void )
{
	SetWinMgmtInDependence();
}
/*
bool VS_GetServiceEName( char *name, const unsigned nameSize )
{
	if (!name || nameSize < 2)		return false;
	return GetServiceEndpoint( name, nameSize );
}
*/

bool VS_StartService( void ) {
	// copy info to HKCU
	if (endpointName)
	{
		auto data = net::endpoint::Serialize(true, endpointName, false);
		if (data.empty())
			return false;
		net::endpoint::Remove(endpointName);
		if (!net::endpoint::Deserialize(true, data.data(), data.size(), endpointName))
			return false;
	}
	return StartService();
}

bool VS_StopService( void ) {
	return StopService();
}

void VS_RemoveService( void ) {
	RemoveService();
}

int VS_ServiceWatchdog(int val) {
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return true;
	int servw = 1;
	if (val==-1) {
		key.GetValue(&servw, 4, VS_REG_INTEGER_VT, SERVICE_WATCHDOG_KEY_NAME);
	}
	else {
		servw = val==0?0:1;
		key.SetValue(&servw, 4, VS_REG_INTEGER_VT, SERVICE_WATCHDOG_KEY_NAME);
	}
	return servw;
}


unsigned VS_GetServiceState( void ) {	return GetServiceState();	}
// end VS_GetServiceState

bool VS_SetWorkingDir( const char *name )
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return false;
	key.SetString(name, WORKING_DIRECTORY_TAG_NAME);
	return true;
}
// end VS_SetWorkingDir

bool VS_GetWorkingDir( wchar_t *name, const unsigned nameSize )
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return false;
	return key.GetValue(name, (nameSize - 1)*sizeof(wchar_t), VS_REG_WSTRING_VT, WORKING_DIRECTORY_TAG_NAME) > 0;
}

bool VS_GetWorkingDir( char *name, const unsigned nameSize )
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return false;
	return key.GetValue(name, (nameSize - 1)*sizeof(char), VS_REG_STRING_VT, WORKING_DIRECTORY_TAG_NAME) > 0;
}

// end VS_GetWorkingDir

bool VS_SetCluster( bool isSet, unsigned hops, int maxOffline, int period )
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return false;
	/*
	const char   *val = isSet ? CLUSTER_USED : CLUSTER_UNUSED;
	key.SetString(val, CLUSTER_USED_KEY_NAME);
	key.SetValue(&hops, sizeof(hops), VS_REG_INTEGER_VT, REQUESTED_HOP_COUNT_KEY_NAME);
	key.SetValue( &maxOffline, sizeof(maxOffline), VS_REG_INTEGER_VT, CLUSTER_OFFLINE_TIME_TAG_NAME);
	key.SetValue( &period, sizeof(period), VS_REG_INTEGER_VT, CLUSTER_TOUCH_PERIOD_TAG_NAME );
	*/
	return true;
}
// end VS_SetCluster

bool VS_GetCluster( bool* isSet, unsigned* hops, int* maxOffline, int* period )
{
	if (!isSet)		return false;
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return false;
	*isSet = false;		char   val[32] = { 0 };
	/*
	if (key.GetValue(val, sizeof(val), VS_REG_STRING_VT, CLUSTER_USED_KEY_NAME) > 0)
		*isSet = !strcmp( val, CLUSTER_USED );
	if (hops)	key.GetValue(hops, sizeof(hops), VS_REG_INTEGER_VT, REQUESTED_HOP_COUNT_KEY_NAME);

	if (maxOffline)
		key.GetValue(maxOffline, sizeof(maxOffline), VS_REG_INTEGER_VT,CLUSTER_OFFLINE_TIME_TAG_NAME );

	if (period)
		key.GetValue(period, sizeof(period), VS_REG_INTEGER_VT,CLUSTER_TOUCH_PERIOD_TAG_NAME);
	*/

	return true;
}
// end VS_GetCluster

bool VS_GetDefaultHost( char *host_name, const unsigned long host_name_size )
{
  return VS_GetDefaultHostName( host_name, host_name_size );
}
// end VS_GetDefaultHost

unsigned VS_GetHosts( const char *host_name, char **ip,
					   const unsigned long ip_number,
					   const unsigned long ip_size)
{
  return VS_GetHostsByName( host_name, ip, ip_number, ip_size);
}
// end VS_GetHosts

bool VS_AcsLibInit( void )
{
  return VS_AcsLibInitial( );
}
// end VS_AcsLibInit


/////////////////////////////////////////////////////////////////////////////////////////

void VS_SetWWWPInDependence( const bool isSet ) {	SetWWWPInDependence( isSet );	}
// end VS_SetWWWPInDependence

void VS_GetWWWPInDependence( bool *isSet )
{
	if (!isSet)		return;
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return;
	std::string val;
	*isSet = (key.GetString(val, WWWP_IN_DEPENDENCE_KEY_NAME) && val == WWWP_IN_DEPENDENCE_SET);
}
// VS_GetWWWPInDependence

void VS_ResetAcceptTCP( void )
{
	if (!endpointName)		return;
	net::endpoint::ClearAllAcceptTCP(endpointName, false);
}
// end VS_ResetAcceptTCP

unsigned VS_SetAcceptTCP( const char *host, const unsigned port )
{
	if (!endpointName)		return 0;
	return net::endpoint::AddAcceptTCP({ host, net::port(port), net::endpoint::protocol_tcp }, endpointName, false);
}
// end VS_SetAcceptTCP

unsigned VS_GetAcceptTCP( const unsigned nPosition, char *host, const unsigned hostSize,
								unsigned *port )
{
	if (!endpointName || !nPosition)	return 0;
	auto at = net::endpoint::ReadAcceptTCP(nPosition, endpointName, false);
	if (!at)
		return 0;
	if (at->host.empty() || at->port == 0)
		return 0;
	if (host && hostSize > 1)
		strncpy(host, at->host.c_str(), hostSize - 1);
	if (port)
		*port = at->port;
	return nPosition;
}
// end VS_GetAcceptTCP

unsigned VS_GetCountAcceptTCP( void )
{
	if (!endpointName)	return 0;
	return net::endpoint::GetCountAcceptTCP(endpointName, false);
}
// end VS_GetCountAcceptTCP

bool VS_SetAddressTranslation( const bool isUse )
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return false;
	key.SetString(isUse ? ADDRESS_TRANSLATION_USE : ADDRESS_TRANSLATION_UNUSE, ADDRESS_TRANSLATION_KEY_NAME);
	return true;
}
// end VS_SetAddressTranslation

bool VS_GetAddressTranslation( bool *isUse )
{
	if (!isUse)		return false;
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return false;
	std::string val;
	*isUse = (key.GetString(val, ADDRESS_TRANSLATION_KEY_NAME) && val == ADDRESS_TRANSLATION_USE);
	return true;
}
// end VS_GetAddressTranslation

void VS_ResetConnectTCP( const bool is_current_user )
{
	if (!endpointName)		return;
	net::endpoint::ClearAllConnectTCP(endpointName, is_current_user);
}
// end VS_ResetConnectTCP

unsigned VS_SetConnectTCP( const char *host, const unsigned port, const bool is_current_user  )
{
	if (!endpointName)		return 0;
	return net::endpoint::AddConnectTCP({ host, net::port(port), net::endpoint::protocol_tcp }, endpointName, is_current_user);
}
// end VS_SetConnectTCP

unsigned VS_GetConnectTCP( const unsigned nPosition, char *host, const unsigned hostSize,
								unsigned *port )
{
	if (!endpointName || !nPosition)	return 0;
	auto ct = net::endpoint::ReadConnectTCP(nPosition, endpointName, false);
	if (!ct)
		return 0;
	if (ct->host.empty() || ct->port == 0)
		return 0;
	if (host && hostSize > 1)
		strncpy(host, ct->host.c_str(), hostSize - 1);
	if (port)
		*port = ct->port;
	return nPosition;
}
// end VS_GetConnectTCP

unsigned VS_GetCountConnectTCP( void )
{
	if (!endpointName)	return 0;
	return net::endpoint::GetCountConnectTCP(endpointName, false);
}
// end VS_GetCountConnectTCP

bool VS_CheckTCPPortAvailability(const char *ip, unsigned short port)
{
	return VS_CheckTCPPortOnIp(ip, port);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_SetPoolThreadsParams( const unsigned workingThreads, const unsigned jobSize, const unsigned maxPriority, const unsigned maxLifetime )
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return false;
	return key.SetValue(&workingThreads, sizeof(unsigned), VS_REG_INTEGER_VT, PTH_MAX_THREADS_TAG_NAME)
	    && key.SetValue(&jobSize, sizeof(unsigned), VS_REG_INTEGER_VT, PTH_MAX_QUEUE_TASKS_TAG_NAME)
	    && key.SetValue(&maxPriority, sizeof(unsigned), VS_REG_INTEGER_VT, PTH_N_PRIORITIES_TAG_NAME)
	    && key.SetValue(&maxLifetime, sizeof(unsigned), VS_REG_INTEGER_VT, PTH_DEF_LIFETIME_TAG_NAME);
}
// end VS_SetPoolThreadsParams

bool VS_GetPoolThreadsParams( unsigned *workingThreads, unsigned *jobSize, unsigned *maxPriority, unsigned *maxLifetime )
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return false;
	if (workingThreads)		key.GetValue(workingThreads, sizeof(unsigned), VS_REG_INTEGER_VT, PTH_MAX_THREADS_TAG_NAME);
	if (jobSize)			key.GetValue(jobSize, sizeof(unsigned), VS_REG_INTEGER_VT, PTH_MAX_QUEUE_TASKS_TAG_NAME);
	if (maxPriority)		key.GetValue(maxPriority, sizeof(unsigned), VS_REG_INTEGER_VT, PTH_N_PRIORITIES_TAG_NAME);
	if (maxLifetime)		key.GetValue(maxLifetime, sizeof(unsigned), VS_REG_INTEGER_VT, PTH_DEF_LIFETIME_TAG_NAME);
	return true;
}
// end VS_GetPoolThreadsParams

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_SetDataBaseParams( const bool isDB, const char *user, const char *password, const char *string, unsigned conn )
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return false;
	key.SetString(isDB ? STORAGE_TYPE_DB : STORAGE_TYPE_REGISTRY, STORAGE_TYPE_KEY_NAME);
	if (user)
		key.SetString(user, DB_USER_TAG_NAME);
	if (password)
		key.SetString(password, DB_PASSWORD_TAG_NAME);
	if (string)
		key.SetString(string, DB_CONNECTIONSTRING_TAG_NAME);
	key.SetValue(&conn, sizeof(unsigned), VS_REG_INTEGER_VT, DB_CONNECTIONS_TAG_NAME);
	return true;
}
// end VS_SetDataBaseParams

bool VS_GetDataBaseParams( bool *isDB, char *user, const unsigned userSize, char *password, const unsigned passwordSize, char *string, const unsigned stringSize,unsigned* conn )
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return false;
	if (isDB)
	{
		std::string val;
		*isDB = !(key.GetString(val, STORAGE_TYPE_KEY_NAME) && val == STORAGE_TYPE_REGISTRY);
	}
	if (user)		key.GetValue(user, userSize, VS_REG_STRING_VT, DB_USER_TAG_NAME);
	if (password)	key.GetValue(password, passwordSize, VS_REG_STRING_VT, DB_PASSWORD_TAG_NAME);
	if (string)		key.GetValue(string, stringSize, VS_REG_STRING_VT, DB_CONNECTIONSTRING_TAG_NAME);
	if (conn)
	{
		*conn=DB_CONNECTIONS_INIT;
		key.GetValue(conn, sizeof(unsigned), VS_REG_INTEGER_VT, DB_CONNECTIONS_TAG_NAME);
	}
	return true;
}
// end VS_GetDataBaseParams

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_SetSmtpMailerParams( const char *server, const unsigned port, const unsigned maxPoolThreads, const char *directory, const unsigned checkPeriod, const unsigned externalcheckPeriod,const unsigned attemptsPeriod, const unsigned lifetime, const char *admin_email,
							 const char *login, const char *password, int authType, bool useSSL)
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return false;
	if (server)
		key.SetString(server, SMTP_SERVER_TAG_NAME);
	key.SetValue(&port, sizeof(unsigned), VS_REG_INTEGER_VT, SMTP_PORT_TAG_NAME);
	key.SetValue(&maxPoolThreads, sizeof(unsigned), VS_REG_INTEGER_VT, SMTP_MAX_POOL_THREADS_TAG_NAME);
	if (directory)
		key.SetString(directory, SMTP_DIRECTORY_TAG_NAME);
	key.SetValue(&checkPeriod, sizeof(unsigned), VS_REG_INTEGER_VT, SMTP_CHECK_PERIOD_TAG_NAME);
	key.SetValue(&externalcheckPeriod, sizeof(unsigned), VS_REG_INTEGER_VT, SMTP_EXTERNAL_CHECK_PERIOD_TAG_NAME);
	key.SetValue(&attemptsPeriod, sizeof(unsigned), VS_REG_INTEGER_VT, SMTP_ATTEMPTS_PERIOD_TAG_NAME);
	key.SetValue(&lifetime, sizeof(unsigned), VS_REG_INTEGER_VT, SMTP_LIFETIME_TAG_NAME);
	if (admin_email)
		key.SetString(admin_email, SMTP_ADMIN_EMAIL_TAG_NAME);
	if (login)
		key.SetString(login, SMTP_LOGIN_TAG_NAME);
	if (password)
		key.SetString(password, SMTP_PASSWORD_TAG_NAME);
	key.SetValue(&authType, sizeof(int), VS_REG_INTEGER_VT, SMTP_AUTH_TYPE_TAG_NAME);
	int ssl = useSSL ? 1 : 0;
	key.SetValue(&ssl, sizeof(int), VS_REG_INTEGER_VT, SMTP_USE_SSL_TAG_NAME);
	return true;
}
// end VS_SetSmtpMailerParams

bool VS_GetSmtpMailerParams( char *server, const unsigned serverSize, unsigned *port, unsigned *maxPoolThreads, char *directory, const unsigned directorySize, unsigned *checkPeriod, unsigned *externalcheckPeriod, unsigned *attemptsPeriod, unsigned *lifetime, char *adminEmail, const unsigned adminEmailSize,
							 char *login, unsigned int loginSize, char *password, unsigned int passwordSize, int *authType, bool *useSSL)
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return false;
	if (server)				key.GetValue(server, serverSize, VS_REG_STRING_VT, SMTP_SERVER_TAG_NAME);
	if (port)				key.GetValue(port, sizeof(unsigned), VS_REG_INTEGER_VT, SMTP_PORT_TAG_NAME);
	if (maxPoolThreads)		key.GetValue(maxPoolThreads, sizeof(unsigned), VS_REG_INTEGER_VT, SMTP_MAX_POOL_THREADS_TAG_NAME);
	if (directory)			key.GetValue(directory, directorySize, VS_REG_STRING_VT, SMTP_DIRECTORY_TAG_NAME);
	if (checkPeriod)		key.GetValue(checkPeriod, sizeof(unsigned), VS_REG_INTEGER_VT, SMTP_CHECK_PERIOD_TAG_NAME);
	if (externalcheckPeriod)key.GetValue(externalcheckPeriod, sizeof(unsigned), VS_REG_INTEGER_VT, SMTP_EXTERNAL_CHECK_PERIOD_TAG_NAME);
	if (attemptsPeriod)		key.GetValue(attemptsPeriod, sizeof(unsigned), VS_REG_INTEGER_VT, SMTP_ATTEMPTS_PERIOD_TAG_NAME);
	if (lifetime)			key.GetValue(lifetime, sizeof(unsigned), VS_REG_INTEGER_VT, SMTP_LIFETIME_TAG_NAME);
	if (adminEmail)			key.GetValue(adminEmail, adminEmailSize, VS_REG_STRING_VT, SMTP_ADMIN_EMAIL_TAG_NAME);
	if (login)				key.GetValue(login, loginSize, VS_REG_STRING_VT, SMTP_LOGIN_TAG_NAME);
	if (password)			key.GetValue(password, passwordSize, VS_REG_STRING_VT, SMTP_PASSWORD_TAG_NAME);
	if (authType)			key.GetValue(authType, sizeof(int), VS_REG_INTEGER_VT, SMTP_AUTH_TYPE_TAG_NAME);
	int ssl = 0;
	if (useSSL)				{key.GetValue(&ssl, sizeof(int), VS_REG_INTEGER_VT, SMTP_USE_SSL_TAG_NAME);
							 *useSSL = ssl != 0;}
	return true;
}
// end VS_GetSmtpMailerParams

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_SetConfPropParams( const unsigned moneyWarn, const unsigned moneyWarnPeriod, const unsigned decLimitPeriod )
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return false;
	key.SetValue(&moneyWarn, sizeof(unsigned), VS_REG_INTEGER_VT, CONF_MONEY_WARN_TAG_NAME);
	key.SetValue(&moneyWarnPeriod, sizeof(unsigned), VS_REG_INTEGER_VT, CONF_MONEY_WARN_PERIOD_TAG_NAME);
	key.SetValue(&decLimitPeriod, sizeof(unsigned), VS_REG_INTEGER_VT, CONF_DECLIMITPERIOD_TAG_NAME);
	return true;
}
// end VS_SetConfPropParams

bool VS_GetConfPropParams( unsigned *moneyWarn, unsigned *moneyWarnPeriod, unsigned *decLimitPeriod )
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return false;
	if (moneyWarn)			key.GetValue(moneyWarn, sizeof(unsigned), VS_REG_INTEGER_VT, CONF_MONEY_WARN_TAG_NAME);
	if (moneyWarnPeriod)	key.GetValue(moneyWarnPeriod, sizeof(unsigned), VS_REG_INTEGER_VT, CONF_MONEY_WARN_PERIOD_TAG_NAME);
	if (decLimitPeriod)		key.GetValue(decLimitPeriod, sizeof(unsigned), VS_REG_INTEGER_VT, CONF_DECLIMITPERIOD_TAG_NAME);
	return true;
}
// end VS_GetConfPropParams

/////////////////////////////////////////////////////////////////////////////////////////

bool VS_KillService()
{
	VS_StopService();	VS_RemoveService();		return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
unsigned VS_SetRasHost( const char *host, const unsigned port )
{
	if (!endpointName)		return 0;
	net::endpoint::ClearAllAcceptUDP(endpointName, false);
	net::endpoint::ClearAllAcceptTCP(endpointName, false);
	net::endpoint::AddAcceptTCP({ host, net::port(port), net::endpoint::protocol_tcp }, endpointName, false);
	return net::endpoint::AddAcceptUDP({ host, net::port(port), net::endpoint::protocol_ras }, endpointName, false);
}
// end VS_SetAcceptTCP

unsigned VS_GetRasHost( const unsigned nPosition, char *host, const unsigned hostSize,
								unsigned *port )
{
	if (!endpointName || !nPosition)	return 0;
	auto au = net::endpoint::ReadAcceptUDP(nPosition, endpointName, false);
	if (!au)
		return 0;
	if (au->host.empty() || au->port == 0)
		return 0;
	if (host && hostSize > 1)
		strncpy(host, au->host.c_str(), hostSize - 1);
	if (port)
		*port = au->port;
	return nPosition;
}
// end VS_GetAcceptTCP
int VS_GetH323HomeBrokers(char** Names, unsigned namesNum, const unsigned namesSize, unsigned *currNum)
{
	*currNum = -1;
	VS_RegistryKey key(false, CONFIGURATION_KEY);
	std::string currBrk;
	key.GetString(currBrk, H323_HOME_BROKER);

	unsigned count = 0;

	VS_RegistryKey   keyEp(false, "Endpoints");
	if (!keyEp.IsValid()) return 0;
	keyEp.ResetKey();
	const char *broker = 0;
	VS_RegistryKey   keybrk;
	while(keyEp.NextKey(keybrk) && count < namesNum) {
		broker = keybrk.GetName();
		if (/*VS_IsBroker(VS_ConvertEndpoint(broker))
			&&*/ strcmp(endpointName, broker)!=0
			&& strcmp(RegServerName, broker)!=0)
		{
			strncpy(Names[count], broker, namesSize);
			if (currBrk == broker)
				*currNum = count;
			count++;
		}
	}
	return count;
}

unsigned VS_GetH323Parameters(char* bName, unsigned bNameSize,
							  char* gName, unsigned gNameSize,
							  unsigned *maxQuantityOfH323Terminals,
							  unsigned *maxQuantityOfSimCalls,
							  unsigned *maxQuantityOfBridgeConnections,
							  unsigned *maxQuantityOfGatewayConnections,
							  unsigned *isDirectlyConnectionOk)
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid())		return 0;
	if (bName) key.GetValue(bName, bNameSize, VS_REG_STRING_VT, H323_BCAST_NAME);
	if (gName) key.GetValue(gName, gNameSize, VS_REG_STRING_VT, H323_GCONF_NAME);
	key.GetValue(maxQuantityOfH323Terminals, sizeof(unsigned), VS_REG_INTEGER_VT, H323_MAX_TERMINALS);
	key.GetValue(maxQuantityOfSimCalls, sizeof(unsigned), VS_REG_INTEGER_VT, H323_MAX_SCALLS);
	key.GetValue(maxQuantityOfBridgeConnections, sizeof(unsigned), VS_REG_INTEGER_VT, H323_MAX_BRIDGE_CONNECT);
	key.GetValue(maxQuantityOfGatewayConnections, sizeof(unsigned), VS_REG_INTEGER_VT, H323_MAX_GATEWAY_CONNECT);
	key.GetValue(isDirectlyConnectionOk, sizeof(unsigned), VS_REG_INTEGER_VT, H323_DIRECTLY_FLAG);
	return 1;
}

unsigned VS_SetH323Parameters(const char *homeBroker,
							  const char *bName,
							  const char *gName,
							  const unsigned maxQuantityOfH323Terminals,
							  const unsigned maxQuantityOfSimCalls,
							  const unsigned maxQuantityOfBridgeConnections,
							  const unsigned maxQuantityOfGatewayConnections,
							  const unsigned isDirectlyConnectionOk)
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (!key.IsValid() || (NULL==homeBroker))	return 0;
	if (homeBroker)
		key.SetString(homeBroker, H323_HOME_BROKER);
	if (bName)
		key.SetString(bName, H323_BCAST_NAME);
	if (gName)
		key.SetString(gName, H323_GCONF_NAME);
	key.SetValue(&maxQuantityOfH323Terminals, sizeof(unsigned), VS_REG_INTEGER_VT, H323_MAX_TERMINALS);
	key.SetValue(&maxQuantityOfSimCalls, sizeof(unsigned), VS_REG_INTEGER_VT, H323_MAX_SCALLS);
	key.SetValue(&maxQuantityOfBridgeConnections, sizeof(unsigned), VS_REG_INTEGER_VT, H323_MAX_BRIDGE_CONNECT);
	key.SetValue(&maxQuantityOfGatewayConnections, sizeof(unsigned), VS_REG_INTEGER_VT, H323_MAX_GATEWAY_CONNECT);
	key.SetValue(&isDirectlyConnectionOk, sizeof(unsigned), VS_REG_INTEGER_VT, H323_DIRECTLY_FLAG);
	return 1;
}
/////////////////////////////////////////////////////////////////////////////////////////

static inline bool OpenServiceManager( void );
static inline bool OpenOurService( void );
static inline bool CreateOurService( void );
static inline bool ChangeOurService( void );
static inline void CloseOurService( void );
static inline void RemoveOurService( void );
static inline QUERY_SERVICE_CONFIG *AllocQSC( void );
static inline void FreeQSC( QUERY_SERVICE_CONFIG * );
static inline SERVICE_STATUS *AllocQSS( void );
static inline void FreeQSS( SERVICE_STATUS * );
static inline void ModifyWinMgmtInDependence( const bool );
static inline void ModifyWWWPInDependence( const bool );
static inline void ModifyInDependence( const char [], const char [], const bool );

/////////////////////////////////////////////////////////////////////////////////////////

static SC_HANDLE   hsm, hsr;

/////////////////////////////////////////////////////////////////////////////////////////

/*
static inline bool GetServiceEndpoint( char *name, const unsigned nameSize )
{
	bool   ret = false;
	if (OpenOurService())
	{	QUERY_SERVICE_CONFIG   *pConf = AllocQSC();
		CmdlineParam   *params = AllocParams(), *param;
		if (pConf && params)
		{	const char   *args = pConf->lpBinaryPathName;
			if (args)
			{	ParseCmdLine( args, params );
				param = FindParam( params, VS_SERVER_ENDPOINT_ARG_PREF );
				if (param->m_value)
				{	strncpy( name, param->m_value, (size_t)( nameSize - 1 ));
					ret = true;
		}	}	}
		FreeParams( params );	FreeQSC( pConf );
	}
	CloseOurService();
	return ret;
}
// end GetServiceEndpoint
*/
static inline bool SetServiceEndpoint( void )
{
	bool   ret = false;
	if (OpenServiceManager())
	{	if (!OpenOurService())	ret = CreateOurService();
		else					ret = ChangeOurService();
		SetWinMgmtInDependence();
		if (ret)
		{
			VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
			std::string val;
			if (key.GetString(val, WWWP_IN_DEPENDENCE_KEY_NAME))
				ModifyWWWPInDependence(val == WWWP_IN_DEPENDENCE_SET);
	}	}
	CloseOurService();
	return ret;
}
// end SetServiceEndpoint

static inline bool StartService( void )
{
	bool   ret = false;
	if (OpenOurService())
	{
		if (StartService( hsr, 0, 0 )
				|| GetLastError() == ERROR_SERVICE_ALREADY_RUNNING)
			ret = true;
	}
	CloseOurService();
	return ret;
}
// end StartService

static inline bool StopService( void )
{
	bool    ret = false;
	bool is_dep = false;
	VS_GetWWWPInDependence(&is_dep);
	if (OpenOurService())
	{
		if(is_dep)
			ModifyWWWPInDependence(false);

		SERVICE_STATUS   sStat;		memset( (void *)&sStat, 0, sizeof(sStat) );
		if (ControlService( hsr, SERVICE_CONTROL_STOP, &sStat )
				|| GetLastError() == ERROR_SERVICE_NOT_ACTIVE)
			ret = true;

		if(is_dep)
			ModifyWWWPInDependence(true);
	}
	CloseOurService();
	return ret;
}
// end StopService

static inline void RemoveService( void )
{
	ResetWinMgmtInDependence();
	if (OpenOurService())	RemoveOurService();
	CloseOurService();
}
// end RemoveService

static inline unsigned GetServiceState( void )
{
	unsigned   ret = 0;
	if (OpenOurService())
	{	SERVICE_STATUS   *pStat = AllocQSS();
		if (pStat)
		{	switch (pStat->dwCurrentState)
			{
			case SERVICE_CONTINUE_PENDING :		ret = 1;	break;
			case SERVICE_PAUSE_PENDING :		ret = 2;	break;
			case SERVICE_PAUSED :				ret = 3;	break;
			case SERVICE_RUNNING :				ret = 4;	break;
			case SERVICE_START_PENDING :		ret = 5;	break;
			case SERVICE_STOP_PENDING :			ret = 6;	break;
			case SERVICE_STOPPED :				ret = 7;	break;
		}	}
		FreeQSS( pStat );
	}
	CloseOurService();
	return ret;
}
// end GetServiceState

static inline bool OpenServiceManager( void )
{
	hsm = OpenSCManager( 0, 0, SC_MANAGER_ALL_ACCESS );
	if (!hsm)	return false;
	return true;
}
// end OpenServiceManager

static inline bool OpenOurService( void )
{
	if (!hsm && !OpenServiceManager())		return false;
	hsr = OpenService( hsm, VS_Server::ServiceName(), SERVICE_ALL_ACCESS );
	if (!hsr)	return false;
	return true;
}
// end OpenOurService

static inline bool CreateOurService( void )
{
	char   *path = (char *)malloc( 2048 );
	int restart = VS_ServiceWatchdog(-1);
	sprintf( path, "%s /Service %s", exePath, restart?"/r":"");
	hsr = CreateService( hsm, VS_Server::ServiceName(), VS_Server::LongName(),
							SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
							SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
							path, 0, 0, 0, 0, 0 );
	free( (void *)path );
	return hsr != 0;
}
// end CreateOurService

static inline bool ChangeOurService( void )
{
	char   *path = (char *)malloc( 2048 );
	int restart = VS_ServiceWatchdog(-1);
	sprintf( path, "%s /Service %s", exePath, restart?"/r":"");
	BOOL   res = ChangeServiceConfig( hsr, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE,
										path, 0, 0, 0, 0, 0, 0 );
	free( (void *)path );
	return res != 0;
}
// end ChangeOurService

static inline void RemoveOurService( void ) {	DeleteService( hsr );	}
// end RemoveOurService

static inline void CloseOurService( void )
{
	if (hsr) {		CloseServiceHandle( hsr );	hsr = 0;	}
	if (hsm) {		CloseServiceHandle( hsm );	hsm = 0;	}
}
// end CloseOurService

static inline QUERY_SERVICE_CONFIG *AllocQSC( void )
{
	unsigned char   unused = 0;		DWORD   bytesNeeded = 0, bufSize = 0;
	QUERY_SERVICE_CONFIG   *pConf = 0;
	if (QueryServiceConfig( hsr, (QUERY_SERVICE_CONFIG *)&unused, 1, &bytesNeeded )
		|| GetLastError() != ERROR_INSUFFICIENT_BUFFER || !bytesNeeded)		return 0;
	bufSize = bytesNeeded + 128;
	pConf = (QUERY_SERVICE_CONFIG *)malloc( (size_t)bufSize );
	memset( (void *)pConf, 0, (size_t)bufSize );	bytesNeeded = 0;
	if (QueryServiceConfig( hsr, pConf, bufSize, &bytesNeeded ))	return pConf;
	free( (void *)pConf );		return 0;
}
// end AllocQSC

static inline void FreeQSC( QUERY_SERVICE_CONFIG *pConf )
{	if (pConf)	free( (void *)pConf );	}
// end FreeQSC

static inline SERVICE_STATUS *AllocQSS( void )
{
	SERVICE_STATUS   *pStat = (SERVICE_STATUS *)malloc( sizeof(SERVICE_STATUS) );
	memset( (void *)pStat, 0, sizeof(SERVICE_STATUS) );
	if (QueryServiceStatus( hsr, pStat ))	return pStat;
	free( (void *)pStat );		return 0;
}
// end AllocQSS

static inline void FreeQSS( SERVICE_STATUS *pStat )
{	if (pStat)	free( (void *)pStat );	}
// end FreeQSS

static inline void SetWinMgmtInDependence( void )
{	ModifyWinMgmtInDependence( true );		}
// end SetWinMgmtInDependence

static inline void ResetWinMgmtInDependence( void )
{	ModifyWinMgmtInDependence( false );		}
// end ResetWinMgmtInDependence

static inline void SetWWWPInDependence( const bool isSet )
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	key.SetString(isSet ? WWWP_IN_DEPENDENCE_SET : WWWP_IN_DEPENDENCE_UNSET, WWWP_IN_DEPENDENCE_KEY_NAME);
	ModifyWWWPInDependence( isSet );
}
// end SetWWWPInDependence

static inline void ModifyWinMgmtInDependence( const bool isSet )
{	ModifyInDependence( VS_SERVER_CONFIG_WINMGMT_NAME, VS_Server::ServiceName(), isSet );	}
// end ModifyWinMgmtInDependence

static inline void ModifyWWWPInDependence( const bool isSet )
{	ModifyInDependence( VS_Server::ServiceName(), VS_SERVER_CONFIG_W3SRV_NAME, isSet );		}
// end ModifyWWWPInDependence

static inline void ModifyInDependence( const char service[], const char inService[], const bool isSet )
{
	if (!OpenServiceManager())		return;
	SC_HANDLE   w3srv = OpenService( hsm, inService, SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG );
	if (!w3srv)		return;
	QUERY_SERVICE_CONFIG   *pConf = 0;
	unsigned char   unused = 0;		DWORD   bytesNeeded = 0;
	if (!QueryServiceConfig( w3srv, (QUERY_SERVICE_CONFIG *)&unused, 1, &bytesNeeded )
			&& GetLastError() == ERROR_INSUFFICIENT_BUFFER && bytesNeeded)
	{	DWORD   bufSize = bytesNeeded + 128;
		pConf = (QUERY_SERVICE_CONFIG *)malloc( (size_t)bufSize );
		memset( (void *)pConf, 0, (size_t)bufSize );	bytesNeeded = 0;
		if (QueryServiceConfig( w3srv, pConf, bufSize, &bytesNeeded ))
		{	size_t   szDep = 0;		char   *newDep = 0;		bool   alreadyFlag = false;
			if (pConf->lpDependencies)
			{	for (char *c = pConf->lpDependencies; *c;)
				{	if (!strcmp( service, c ))	alreadyFlag = true;
					const size_t   sz = strlen( c ) + 1;	szDep += sz;	c += sz;	}
				++szDep;
			}
			if (!isSet)
			{	if (alreadyFlag)
				{	newDep = (char *)malloc( szDep );	memset( (void *)newDep, 0, szDep );
					for (char *c1 = pConf->lpDependencies, *c2 = newDep; *c1;)
					{	const size_t   sz = strlen( c1 ) + 1;
					if (memcmp( (const void *)c1, (const void *)service, sz ))
						{	memcpy( (void *)c2, (const void *)c1, sz );		c2 += sz;	}
						c1 += sz;
			}	}	}
			else
			{	if (!alreadyFlag)
				{	size_t   newSz = szDep + strlen( service ) + 2;
					newDep = (char *)malloc( newSz );	memset( (void *)newDep, 0, newSz );
					if (!pConf->lpDependencies)		strcpy( newDep, service );
					else
					{	memcpy( (void *)newDep, (const void *)pConf->lpDependencies, szDep );
						strcpy( newDep + (szDep - 1), service );
			}	}	}
			if (newDep)
			{	ChangeServiceConfig( w3srv, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE,
											SERVICE_NO_CHANGE, 0, 0, 0, newDep, 0, 0, 0 );
				free( (void *)newDep );
		}	}
		free( (void *)pConf );
	}
	CloseServiceHandle( w3srv );
}
// end ModifyInDependence

/////////////////////////////////////////////////////////////////////////////////////////
