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
/// \file MediaBrokerConfigDll.cpp
/// \brief
/// \note
///
/////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <Windows.h>

#include "VS_ServerConfig.h"
#include "VS_ServersConfigDll.h"
#include "../ServersConfigLib/VS_ServersConfigLib.h"
#include "../ServersConfigLib/VS_ConfigClient.h"
#include "../common/net/EndpointRegistry.h"
#include "../common/std/cpplib/VS_RegistryKey.h"

VS_ConfigClient client;
/////////////////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{	return TRUE;	}
// end DllMain

/////////////////////////////////////////////////////////////////////////////////////////
void VSSetApplication(int num)
{
	VS_SetApplication(num);
}


bool VSGetSerializeInfo( void** serialize_buffer, unsigned long *size_buffer, const char *endpoint_name)
{
	if (!serialize_buffer || !size_buffer)
		return false;
	auto data = net::endpoint::Serialize(true, endpoint_name, false);
	if (data.empty())
		return false;
	*size_buffer = data.size();
	*serialize_buffer = malloc(data.size());
	memcpy(*serialize_buffer, data.data(), *size_buffer);
	return true;
}

void VSReleaseSerializeInfo( void* serialize_buffer)
{	free(serialize_buffer);	}

bool VSGetExecutablePath( char *name, const unsigned nameSize )
{	return VS_GetExecutablePath( name, nameSize );	}

bool VSSetLocalEName( const char *name )
{	return VS_SetLocalEName( name );	}

bool VSGetLocalEName( char *name, const unsigned nameSize )
{	return VS_GetLocalEName( name, nameSize );	}

bool VSSetLocalENameInService( void )
{	return VS_SetLocalENameInService();		}

void VSSetWinMgmtInDependence( void )
{	VS_SetWinMgmtInDependence();	}

bool VSGetServiceEName( char *name, const unsigned nameSize )
{	return false;	}

bool VSStartService( void )
{	return VS_StartService();	}

bool VSStopService( void )
{	return VS_StopService();	}

void VSRemoveService( void )
{	VS_RemoveService();		}

int VSServiceWatchdog(int val) {
	return VS_ServiceWatchdog(val);
}

unsigned VSGetServiceState( void )
{	return VS_GetServiceState();	}

bool VSSetWorkingDir( const char *name )
{	return VS_SetWorkingDir( name );	}

bool VSGetWorkingDir( char *name, const unsigned nameSize )
{	return VS_GetWorkingDir( name, nameSize );	}

void VSSetWWWPInDependence( const bool isSet )
{	VS_SetWWWPInDependence( isSet );	}

void VSGetWWWPInDependence( bool *isSet )
{	VS_GetWWWPInDependence( isSet );	}

bool VSGetDefaultHost( char *host_name, const unsigned long host_name_size )
{	return VS_GetDefaultHost( host_name, host_name_size );	}

unsigned VSGetHosts( const char *host_name, char **ip, const unsigned long ip_number, const unsigned long ip_size )
{	return VS_GetHosts( host_name, ip, ip_number, ip_size );	}

bool VSAcsLibInit( void )
{	return VS_AcsLibInit();		}

void VSResetAcceptTCP( void )
{	VS_ResetAcceptTCP();	}

unsigned VSSetAcceptTCP( const char *host, const unsigned port )
{	return VS_SetAcceptTCP( host, port );	}

unsigned VSGetAcceptTCP( const unsigned nPosition, char *host, const unsigned hostSize, unsigned *port )
{	return VS_GetAcceptTCP( nPosition, host, hostSize, port );	}

unsigned VSGetCountAcceptTCP( void )
{	return VS_GetCountAcceptTCP();	}

bool VSSetAddressTranslation( const bool isUse )
{	return VS_SetAddressTranslation( isUse );	}

bool VSGetAddressTranslation( bool *isUse )
{	return VS_GetAddressTranslation( isUse );	}

void VSResetConnectTCP( void )
{	VS_ResetConnectTCP();	}

unsigned VSSetConnectTCP( const char *host, const unsigned port )
{	return VS_SetConnectTCP( host, port );	}

unsigned VSGetConnectTCP( const unsigned nPosition, char *host, const unsigned hostSize, unsigned *port )
{	return VS_GetConnectTCP( nPosition, host, hostSize, port );		}

unsigned VSGetCountConnectTCP( void )
{	return VS_GetCountConnectTCP();		}

bool VSCheckTCPPortAvailability(const char *ip, unsigned short port)
{	return	VS_CheckTCPPortAvailability(ip, port);	}

bool VSSetPoolThreadsParams( const unsigned workingThreads, const unsigned jobSize, const unsigned maxPriority, const unsigned maxLifetime )
{	return VS_SetPoolThreadsParams( workingThreads, jobSize, maxPriority, maxLifetime );	}

bool VSGetPoolThreadsParams( unsigned *workingThreads, unsigned *jobSize, unsigned *maxPriority, unsigned *maxLifetime )
{	return VS_GetPoolThreadsParams( workingThreads, jobSize, maxPriority, maxLifetime );	}

bool VSSetDataBaseParams( const bool isDB, const char *user, const char *password, const char *string, unsigned conn )
{	return VS_SetDataBaseParams( isDB, user, password, string, conn );	}

bool VSGetDataBaseParams( bool *isDB, char *user, const unsigned userSize, char *password, const unsigned passwordSize, char *string, const unsigned stringSize,unsigned* conn )
{	return VS_GetDataBaseParams( isDB, user, userSize, password, passwordSize, string, stringSize, conn );	}

bool VSSetSmtpMailerParams( const char *server, const unsigned port, const unsigned maxPoolThreads, const char *directory, const unsigned checkPeriod,const unsigned externalcheckPeriod, const unsigned attemptsPeriod, const unsigned lifetime, const char *admin_email,
						    const char *login, const char *password, int authType, bool useSSL)
{	return VS_SetSmtpMailerParams( server, port, maxPoolThreads, directory, checkPeriod, externalcheckPeriod, attemptsPeriod, lifetime, admin_email, login, password, authType, useSSL );	}

bool VSGetSmtpMailerParams( char *server, const unsigned serverSize, unsigned *port, unsigned *maxPoolThreads, char *directory, const unsigned directorySize, unsigned *checkPeriod, unsigned *externalcheckPeriod, unsigned *attemptsPeriod, unsigned *lifetime, char *adminEmail, const unsigned adminEmailSize,
						    char *login, unsigned int loginSize, char *password, unsigned int passwordSize, int *authType, bool *useSSL)
{	return VS_GetSmtpMailerParams( server, serverSize, port, maxPoolThreads, directory, directorySize, checkPeriod, externalcheckPeriod, attemptsPeriod, lifetime, adminEmail, adminEmailSize, login, loginSize, password, passwordSize, authType, useSSL );	}

bool VSSetConfPropParams( const unsigned moneyWarn, const unsigned moneyWarnPeriod, const unsigned decLimitPeriod )
{	return VS_SetConfPropParams( moneyWarn, moneyWarnPeriod, decLimitPeriod );	}

bool VSGetConfPropParams( unsigned *moneyWarn, unsigned *moneyWarnPeriod, unsigned *decLimitPeriod )
{	return VS_GetConfPropParams( moneyWarn, moneyWarnPeriod, decLimitPeriod );	}

bool VSSetCluster( const bool isSet, const unsigned hops, int maxOffline, int period )
{	return VS_SetCluster( isSet, hops, maxOffline, period );	}

bool VSGetCluster( bool *isSet, unsigned *hops, int *maxOffline, int *period )
{	return VS_GetCluster( isSet, hops, maxOffline, period );	}

bool VSKillService(HWND hwnd, HINSTANCE hinst, LPTSTR lpCmdLine, int nCmdShow)
{
	int app = atoi(lpCmdLine);
	VS_SetApplication(app);
	return VS_KillService();
}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
bool VSInitTransport(HWND hwnd){
	return client.Init(hwnd);
}
void VSReleaseTransport(){
	client.Release();
}
void VSUpdateUser(const char* broker, const char* user, int type){
	client.ReqUpdateUser(broker, user, type);
}
void VSUpdateProp(const char* broker){
	client.ReqUpdateProp(broker);
}
void VSUpdateAddressBook(const char* broker, const char* user, const char * query, int ab){
	client.ReqUpdateAddressBook(broker, user, query, ab);
}

/////////////////////////////////////////////////////////////////////////////////////////
bool VSRegSerial(const char* server_id, const char *server_name, const char* serial, const char *organization_name,
				 const char *country, const char *contact_person, const char *contact_email)
{
	return client.ReqLicence(server_id,server_name, serial, organization_name,
							country, contact_person, contact_email);
}
unsigned VSRegFromFile(const char *from_file)
{
	return client.RegisterFromFile(from_file);
}
bool VSRegSerialToFile(const char* server_id, const char *server_name, const char* serial, const char *organization_name,
						const char *country, const char *contact_person, const char *contact_email,
						const char* to_file)
{
	return client.ReqLicence(server_id, server_name, serial, organization_name,
							country, contact_person, contact_email, to_file);
}
bool VSRegSerialFromFile(const char* from_file, const char* xml_file)
{
	return client.ReqLicence(from_file, xml_file);
}

void VSSendInvites(char *conf, char *pass, char** users, int num)
{
	client.SendInvites(conf, pass, users, num);
}

void VSDeleteConference(char *conf)
{
	client.DeleteConference(conf);
}
/////////////////////////////////////////////////////////////////////////////////////////
unsigned VSSetRasHost( const char *host, const unsigned port )
{
	return VS_SetRasHost( host , port  );
}
unsigned VSGetRasHost( const unsigned nPosition, char *host, const unsigned hostSize, unsigned *port )
{
	return VS_GetRasHost( nPosition, host, hostSize, port );
}

unsigned VSGetH323HomeBrokers(char** Names, unsigned namesNum, const unsigned namesSize, unsigned *currNum)
{
	return VS_GetH323HomeBrokers(Names, namesNum, namesSize, currNum);
}

unsigned  VSGetH323Parameters(char* bName, unsigned bNameSize,
							  char* gName, unsigned gNameSize,
							  unsigned *maxQuantityOfH323Terminals,
							  unsigned *maxQuantityOfSimCalls,
							  unsigned *maxQuantityOfBridgeConnections,
							  unsigned *maxQuantityOfGatewayConnections,
							  unsigned *isDirectlyConnectionOk)
{
	return VS_GetH323Parameters(bName, bNameSize, gName, gNameSize,
							  maxQuantityOfH323Terminals, maxQuantityOfSimCalls,
							  maxQuantityOfBridgeConnections, maxQuantityOfGatewayConnections,
							  isDirectlyConnectionOk);
}

unsigned  VSSetH323Parameters(const char *homeBroker,
							  const char *bName,
							  const char *gName,
							  const unsigned maxQuantityOfH323Terminals,
							  const unsigned maxQuantityOfSimCalls,
							  const unsigned maxQuantityOfBridgeConnections,
							  const unsigned maxQuantityOfGatewayConnections,
							  const unsigned isDirectlyConnectionOk)
{
	return VS_SetH323Parameters(homeBroker, bName, gName,
		maxQuantityOfH323Terminals,
		maxQuantityOfSimCalls,
		maxQuantityOfBridgeConnections,
		maxQuantityOfGatewayConnections,
		isDirectlyConnectionOk);
}

bool VSGetCertificateData(wchar_t *servEp, const unsigned servEpSz,
						wchar_t *country, const unsigned countrySz,
						wchar_t *organization, const unsigned organizationSz,
						wchar_t *contact_person, const unsigned contact_personSz,
						wchar_t *email, const unsigned emailSz,
						char *notBefore, const unsigned notBeforeSz,
						char *notAfter, const unsigned notAfterSz)
{
	bool res(false);
	VS_SimpleStr strNotBefore, strNotAfter;
	VS_WideStr strServEp, strCountry, strOrganization, strPerson, strEmail;
	res = client.GetCertificateData(strServEp,strCountry,strOrganization,strPerson,strEmail,strNotBefore,strNotAfter);
	if(!res || ((strServEp.Length()+1)*sizeof(wchar_t)>servEpSz) ||
		((strCountry.Length() + 1)*sizeof(wchar_t)>countrySz)	||
		((strOrganization.Length() + 1)*sizeof(wchar_t)>organizationSz) ||
		((strPerson.Length() + 1)*sizeof(wchar_t) > contact_personSz) ||
		((strEmail.Length() + 1)*sizeof(wchar_t)>emailSz) ||
		(strNotBefore.Length() + 1>notBeforeSz) ||
		(strNotAfter.Length() + 1> notAfterSz))

		return false;


	wcscpy(servEp, !!strServEp ? strServEp.m_str : L"");
	wcscpy(country, !!strCountry ? strCountry.m_str : L"");
	wcscpy(organization, !!strOrganization ? strOrganization.m_str : L"");
	wcscpy(contact_person, !!strPerson ? strPerson.m_str : L"");
	wcscpy(email, !!strEmail ? strEmail.m_str : L"");
	strcpy(notBefore, !!strNotBefore ? strNotBefore.m_str : "");
	strcpy(notAfter, !!strNotAfter ? strNotAfter.m_str : "");
	return true;
}

bool __declspec( dllexport) VSGetServerIdentifyString(char *buf,unsigned *buf_sz)
{
	VS_SimpleStr	str;
	if(client.GetIdentifyStr(str))
	{
		if(str.ByteLength()> *buf_sz)
		{
			*buf_sz = str.ByteLength();
			return false;
		}
		else
		{
			*buf_sz = str.ByteLength();
			memcpy(buf,str,*buf_sz);
			return true;
		}

	}
	else
		return false;
}
/////////////////////////////////////////////////////////////////////////////////////////

void VSSetLayout(char *conf, char *caster)
{
	client.SetLayout(conf,caster);
}

int VSCopyRegKeys(char *s, char* d)
{
	if (!s || !*s || !d || !*d)
		return -1;
	return VS_RegistryKey::CopyKey(false, s, false, d);
}
