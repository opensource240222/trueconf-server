#include "VS_ServerConfig.h"
#include "version.h"
#include "../common/tools/Server/VS_Server.h"
#include "../common/std/cpplib/VS_RegistryKey.h"
#include "../ServersConfigLib/VS_ConfigClient.h"
static int ApplicationNum = 0;

// 1 - medib, 2 - dir, 3 - sbs, 4 - gw, 5 - ldapbroker, 6 - enterprise, 8 - singleH323Gw;
// 9 - registry server
// 10 - AS (Application Server)
// 11 - RS (Routing Server)
// 12 - BS (Billing Server)
// 13 - SM (Server Manager)
// 14 - VCS 3
void VS_SetApplication(int num)
{
	ApplicationNum = num;
	VS_RegistryKey::SetDefaultRoot(VS_Server::RegistryKey());
	VS_ConfigClient::SetApplication(ApplicationNum);
}
const char* VS_Server::ShortName()
{
	return ServiceName();
}

const char* VS_Server::LongName()
{
	switch(ApplicationNum)
	{
	case 2:
		return VS_DIRECTORY_SERVER_WS_DISPLAY_NAME;
	case 3:
		return VS_SBS_WS_DISPLAY_NAME;
	case 4:
		return VS_H323_GATEWAY_WS_DISPLAY_NAME;
	case 5:
		return VS_SBSPLUS_WS_DISPLAY_NAME;
	case 6:
		return VS_ENTERPRISE_WS_DISPLAY_NAME;
	case 8:
		return VS_GATEWAY_WS_DISPLAY_NAME;
	case 9:
		return VS_REGISTRY_SERVER_WS_DISPLAY_NAME;
	case 10:
		return VS_APP_SERVER_WS_DISPLAY_NAME;
	case 11:
		return VS_ROUTING_SERVER_WS_DISPLAY_NAME;
	case 12:
		return VS_BASE_SERVER_WS_DISPLAY_NAME;
	case 13:
		return VS_SERVER_MANAGER_WS_DISPLAY_NAME;
	case 14:
		return VS_TRUECONF_WS_DISPLAY_NAME;
	default:
		return VS_TRUECONF_WS_DISPLAY_NAME;
	}
}

const char* VS_Server::RegistryKey()
{
	switch(ApplicationNum)
	{
	case 2:
		return VS_DIRECTORY_SERVER_WS_ROOT_KEY_NAME;
	case 3:
		return VS_SBS_WS_ROOT_KEY_NAME;
	case 4:
		return VS_H323_GATEWAY_WS_ROOT_KEY_NAME;
	case 5:
		return VS_SBSPLUS_WS_ROOT_KEY_NAME;
	case 6:
		return VS_ENTERPRISE_WS_ROOT_KEY_NAME;
	case 8:
		return VS_GATEWAY_WS_ROOT_KEY_NAME;
	case 9:
		return VS_REGISTRY_SERVER_WS_ROOT_KEY_NAME;
	case 10:
		return VS_APP_SERVER_WS_ROOT_KEY_NAME;
	case 11:
		return VS_ROUTING_SERVER_WS_ROOT_KEY_NAME;
	case 12:
		return VS_BASE_SERVER_WS_ROOT_KEY_NAME;
	case 13:
		return VS_SERVER_MANAGER_WS_ROOT_KEY_NAME;
	case 14:
		return VS_TRUECONF_WS_ROOT_KEY_NAME;
	default:
		return VS_TRUECONF_WS_ROOT_KEY_NAME;
	}
}

const char* VS_Server::ServiceName()
{
	switch(ApplicationNum)
	{
	case 2:
		return VS_DIRECTORY_SERVER_WS_SERVICE_NAME;
	case 3:
		return VS_SBS_WS_SERVICE_NAME;
	case 4:
		return VS_H323_GATEWAY_WS_SERVICE_NAME;
	case 5:
		return VS_SBSPLUS_WS_SERVICE_NAME;
	case 6:
		return VS_ENTERPRISE_WS_SERVICE_NAME;
	case 8:
		return VS_GATEWAY_WS_SERVICE_NAME;
	case 9:
		return VS_REGISTRY_SERVER_WS_SERVICE_NAME;
	case 10:
		return VS_APP_SERVER_WS_SERVICE_NAME;
	case 11:
		return VS_ROUTING_SERVER_WS_SERVICE_NAME;
	case 12:
		return VS_BASE_SERVER_WS_SERVICE_NAME;
	case 13:
		return VS_SERVER_MANAGER_WS_SERVICE_NAME;
	case 14:
		return VS_TRUECONF_WS_SERVICE_NAME;
	default:
		return VS_TRUECONF_WS_SERVICE_NAME;
	}
}

const char* VS_Server::ProductVersion()
{
	return STRPRODUCTVER;
}
/////////////////////////////////////////////////////////////////////////////////////////
