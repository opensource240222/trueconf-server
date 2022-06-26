#pragma once

#define   VS_MEDIA_BROKER_WS_SERVICE_NAME		"VSMediaBroker"
#define   VS_MEDIA_BROKER_WS_DISPLAY_NAME		"Visicron Media Broker"
#define   VS_MEDIA_BROKER_WS_ROOT_KEY_NAME		"Visicron\\MediaBroker"

#define   VS_SBS_WS_SERVICE_NAME				"VSSBServer"
#define   VS_SBS_WS_DISPLAY_NAME				"Visicron Small Business Server"
#define   VS_SBS_WS_ROOT_KEY_NAME				"Visicron\\SBServer"

#define   VS_SBSPLUS_WS_SERVICE_NAME			"VSSBSPlus"
#define   VS_SBSPLUS_WS_DISPLAY_NAME			"Visicron Small Business Server Plus"
#define   VS_SBSPLUS_WS_ROOT_KEY_NAME			"Visicron\\SBSPlus"

#define   VS_ENTERPRISE_WS_SERVICE_NAME			"VSEnterprise"
#define   VS_ENTERPRISE_WS_DISPLAY_NAME			"Visicron Enterprise Server"
#define   VS_ENTERPRISE_WS_ROOT_KEY_NAME		"Visicron\\Enterprise"

#define   VS_DIRECTORY_SERVER_WS_SERVICE_NAME	"VSDirectoryServer"
#define   VS_DIRECTORY_SERVER_WS_DISPLAY_NAME	"Visicron Directory Server"
#define   VS_DIRECTORY_SERVER_WS_ROOT_KEY_NAME	"Visicron\\DirectoryServer"

#define   VS_REGISTRY_SERVER_WS_SERVICE_NAME	"VSRegistryServer"
#define   VS_REGISTRY_SERVER_WS_DISPLAY_NAME	"TrueConf Registry Server"
#define   VS_REGISTRY_SERVER_WS_ROOT_KEY_NAME	"TrueConf\\RegistryServer"

#define   VS_H323_GATEWAY_WS_SERVICE_NAME		"VSH323Gateway"
#define   VS_H323_GATEWAY_WS_DISPLAY_NAME		"Visicron H323 Gateway"
#define   VS_H323_GATEWAY_WS_ROOT_KEY_NAME		"Visicron\\H323Gateway"

#define   VS_GATEWAY_WS_SERVICE_NAME			"VSGateway"
#define	  VS_GATEWAY_WS_DISPLAY_NAME			"TrueConf Gateway"
#define	  VS_GATEWAY_WS_ROOT_KEY_NAME			"TrueConf\\Gateway"

#ifdef _SVKS_M_BUILD_
#define _CUSTOM_SERVER_PREFIX_					"SVKS-M"
#define VS_COMPANY_NAME							""
#define VS_LEGAL_COPYRIGHT						""
#define VS_PRODUCT_NAME_VISICRON_DLL			"Client Library"
#define VS_PRODUCT_NAME_CODECS_DLL				"Codecs Dynamic Link Library"
#define VS_PRODUCT_NAME_TRANSCEIVER				"Transceiver"
#define VS_PRODUCT_NAME_PHP_TRUECONF_DLL		"PHP Extension"
#else
#define _CUSTOM_SERVER_PREFIX_					"TrueConf"
#define VS_PRODUCT_NAME_VISICRON_DLL			"TrueConf Client Library"
#define VS_PRODUCT_NAME_CODECS_DLL				"TrueConf Codecs Library"
#define VS_PRODUCT_NAME_TRANSCEIVER				"TrueConf Transceiver"
#define VS_PRODUCT_NAME_PHP_TRUECONF_DLL		"TrueConf PHP Extension"
#define VS_COMPANY_NAME							"TrueConf LLC"
#define VS_LEGAL_COPYRIGHT						"© 2010-2018 TrueConf LLC. All rights reserved."
#endif

#define VS_PRODUCT_NAME_RESTARTER				"Restart Application"

//new servers

#define   VS_APP_SERVER_WS_SERVICE_NAME		"VSAppServer"
#define   VS_APP_SERVER_WS_DISPLAY_NAME		"Visicron Application Server"
#define   VS_APP_SERVER_WS_ROOT_KEY_NAME	"TrueConf\\AppServer"

#define	  VS_ROUTING_SERVER_WS_SERVICE_NAME	"VSRoutingServer"
#define	  VS_ROUTING_SERVER_WS_DISPLAY_NAME	"Visicron Routing Server"
#define	  VS_ROUTING_SERVER_WS_ROOT_KEY_NAME	"TrueConf\\RoutingServer"

#define	  VS_BASE_SERVER_WS_SERVICE_NAME	"VSBaseServer"
#define	  VS_BASE_SERVER_WS_DISPLAY_NAME	"Visicron Base Server"
#define	  VS_BASE_SERVER_WS_ROOT_KEY_NAME	"TrueConf\\BaseServer"

#define	  VS_SERVER_MANAGER_WS_SERVICE_NAME		"VSServerManager"
#define	  VS_SERVER_MANAGER_WS_DISPLAY_NAME		"Visicron Server Manager"
#define	  VS_SERVER_MANAGER_WS_ROOT_KEY_NAME	"TrueConf\\ServerManager"

#define   VS_TRUECONF_WS_SERVICE_NAME				_CUSTOM_SERVER_PREFIX_
#define   VS_TRUECONF_WS_DISPLAY_NAME				_CUSTOM_SERVER_PREFIX_ " Server"
#define   VS_TRUECONF_WS_ROOT_KEY_NAME				_CUSTOM_SERVER_PREFIX_ "\\Server"
