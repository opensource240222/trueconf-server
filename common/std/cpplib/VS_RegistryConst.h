#pragma once
#include "SecureLib/VS_SSLConfigKeys.h"

extern const char CONFIGURATION_KEY[]; // = "Configuration"
extern const char POLICY_USE_IP[]; // = "PolicyUseIp";
extern const char POLICY_USE_LOGIN[]; // = "PolicyUseLogin";
extern const char POLICY_MAX_FAIL_BEFORE_BAN[]; // = "PolicyMaxFailBeforeBan";
extern const char POLICY_DELAY_TIME[]; // = "PolicyDelayTime";
extern const char POLICY_BAN_TIME[]; // = "PolicyBanTime";
extern const char POLICY_SEND_ACCESS_DENY[]; // = "PolicySendAccessDeny";
extern const char POLICY_SILENT_ON_BAN[]; // = "PolicySilentOnBan";
extern const char POLICY_DELAY_TTL[]; // = "PolicyDelayTTL";
extern const char POLICY_DELAY_INTERVAL[]; // = "PolicyDelayInterval";
extern const char POLICY_MAX_FAIL_BEFORE_DELAY[]; // = "PolicyMaxFailBeforeDelay";
extern const char POLICY_OBSERVE_INTERVAL[]; //= "LoginPolicyObserveInterval";

extern const char SIP_PEERS_KEY[]; // = "SIP Peers"
extern const char SIP_FROM_HOST[];
extern const char SIP_RETRY_AFTER[];
extern const char H323_PEERS_KEY[]; // = "H323 Peers"
extern const char TRANSCODERS_KEY[]; // = "Transcoders"
extern const char TRANSCEIVER_LOCATION[];
extern const char TRANSCEIVERS_KEY[];
extern const char TRANSCEIVER_SHARED_KEY_TAG[];
extern const char MIN_FREE_TRANSCEIVERS_TAG[];
extern const char MAX_CONFS_BY_TRANSCEIVER_TAG[];
extern const char MAX_TRANSCEIVER_FREE_TIME_MINUTES[];
extern const char MAX_TRANSCEIVERS[];
extern const char RTP_INTERFACE_KEY_TAG[];
extern const char GROUPS_KEY[]; // = "Groups";
extern const char MULTI_CONFERENCES_KEY[]; // = "Multi Conferences";
extern const char USERS_KEY[]; // = "Users"
extern const char APPLICATION_SETTINGS_KEY[]; // = "ApplicationSettings"

extern const char TIME_WAIT_RELOAD[]; // = "Time Wait Reload"

extern const char REGISTRATION_MAX_EXPIRES[]; //="Registration Max Expires"

extern const char REGISTRY_STRATEGY[]; //=RegisterStrategy
extern const char LAST_AUTHORIZATION_RESULT[]; // = "Last Authorization Result";
extern const char LAST_CHECK_TIME[]; // = "Last Check Time";


#define	EXECUTABLE_PATH_TAG_NAME				"Executable Path"
extern const char EXECUTABLE_PATH_TAG[];		// = EXECUTABLE_TAG_NAME

#define	WORKING_DIRECTORY_TAG_NAME				"Working Directory"
extern const char WORKING_DIRECTORY_TAG[];		// = WORKING_DIRECTORY_TAG_NAME

#define TRANSPORT_MESSAGES_LOGS_FILE_TAG_NAME			"Transport Messages Logs File Name"
extern const char TRANSPORT_MESSAGES_LOGS_FILE_TAG[];	// = TRANSPORT_MESSAGES_LOGS_FILE_TAG_NAME

#define TRANSPORT_FULL_TRACE_ENABLED	"transport trace enabled"
extern const char TRANSPORT_FULL_TRACE_ENABLED_TAG[];

#define REQUESTED_HOP_COUNT_KEY_NAME			"Requested Hop Count"
extern const char REQUESTED_HOP_COUNT_KEY[];	// = REQUESTED_HOP_COUNT_KEY_NAME

#define HTTP_HANDLER_DIRECTORY_INIT						"http_root"
#define HTTP_HANDLER_DIRECTORY_TAG_NAME					"HttpHandlerDirectory"
extern const char HTTP_HANDLER_DIRECTORY_TAG[];			// = HTTP_HANDLER_DIRECTORY_TAG_NAME
#define HTTP_HANDLER_INDEX_FILE_INIT					"bad_request.htm"
#define HTTP_HANDLER_INDEX_FILE_TAG_NAME				"HttpHandlerIndexFile"
extern const char HTTP_HANDLER_INDEX_FILE_TAG[];		// = HTTP_HANDLER_INDEX_FILE_TAG_NAME
#define HTTP_HANDLER_VERSION_MARKER_INIT				"Product Version: "
#define HTTP_HANDLER_VERSION_MARKER_TAG_NAME			"HttpHandlerVersionMarker"
extern const char HTTP_HANDLER_VERSION_MARKER_TAG[];	// = HTTP_HANDLER_VERSION_MARKER_TAG_NAME
#define HTTP_HANDLER_USERS_PASSWORDS_TAG_NAME			"HttpHandlerUsersPasswords"
extern const char HTTP_HANDLER_USERS_PASSWORDS_TAG[];	// = HTTP_HANDLER_USERS_PASSWORDS_TAG_NAME

// Pool Threads Configuration
#define	PTH_MAX_THREADS_TAG_NAME				"PoolThreadsMax"
extern const char PTH_MAX_THREADS_TAG[];		// = PTH_MAX_THREADS_TAG_NAME
#define	PTH_MAX_QUEUE_TASKS_TAG_NAME			"PoolThreadsQueueTasksMax"
extern const char PTH_MAX_QUEUE_TASKS_TAG[];	// = PTH_MAX_QUEUE_TASKS_TAG_NAME
#define	PTH_N_PRIORITIES_TAG_NAME				"PoolThreadsPriorities"
extern const char PTH_N_PRIORITIES_TAG[];		// = PTH_N_PRIORITIES_TAG_NAME
#define	PTH_DEF_LIFETIME_TAG_NAME				"PoolThreadsLifetime"
extern const char PTH_DEF_LIFETIME_TAG[];		// = PTH_DEF_LIFETIME_TAG_NAME

//SMTP configuration
#define	SMTP_SERVER_TAG_NAME					"SMTP Server"
extern const char SMTP_SERVER_TAG[];			// = SMTP_SERVER_TAG_NAME
#define	SMTP_PORT_TAG_NAME						"SMTP Port"
extern const char SMTP_PORT_TAG[];				// = SMTP_PORT_TAG_NAME
#define SMTP_LOGIN_TAG_NAME						"SMTP Login"
extern const char SMTP_LOGIN_TAG[];
#define SMTP_PASSWORD_TAG_NAME					"SMTP Password"
extern const char SMTP_PASSWORD_TAG[];
#define SMTP_FROM_TAG_NAME						"SMTP From"
extern const char SMTP_FROM_TAG[];
#define SMTP_USE_SSL_TAG_NAME					"SMTP Use SSL"
extern const char SMTP_USE_SSL_TAG[];
#define SMTP_AUTH_TYPE_TAG_NAME					"SMTP Authentication Type"
extern const char SMTP_AUTH_TYPE_TAG[];
#define	SMTP_MAX_POOL_THREADS_TAG_NAME			"SMTP Pool Threads"
extern const char SMTP_MAX_POOL_THREADS_TAG[];	// = SMTP_MAX_POOL_THREADS_TAG_NAME
#define	SMTP_DIRECTORY_TAG_NAME					"SMTP Directory"
extern const char SMTP_DIRECTORY_TAG[];			// = SMTP_DIRECTORY_TAG_NAME
///
#define	SMTP_EXTERNAL_CHECK_PERIOD_TAG_NAME				"SMTP External Check Period"
extern const char SMTP_EXTERNAL_CHECK_PERIOD_TAG[];		// = SMTP_EXTERNAL_CHECK_PERIOD_TAG_NAME
///
#define	SMTP_CHECK_PERIOD_TAG_NAME				"SMTP Check Period"
extern const char SMTP_CHECK_PERIOD_TAG[];		// = SMTP_CHECK_PERIOD_TAG_NAME
#define	SMTP_ATTEMPTS_PERIOD_TAG_NAME			"SMTP Attempts Period"
extern const char SMTP_ATTEMPTS_PERIOD_TAG[];	// = SMTP_ATTEMPTS_PERIOD_TAG_NAME
#define	SMTP_LIFETIME_TAG_NAME					"SMTP Lifetime"
extern const char SMTP_LIFETIME_TAG[];			// = SMTP_LIFETIME_TAG_NAME
#define	SMTP_ADMIN_EMAIL_TAG_NAME				"SMTP Admin Email"
extern const char SMTP_ADMIN_EMAIL_TAG[];		// = SMTP_ADMIN_EMAIL_TAG_NAME

// Router Monitor Debug
#define DEBUG_ROUTER_TAG_NAME                    "Debug Router"
#define DEBUG_STATISTIC_TAG_NAME                 "Debug Statistic"
#define SUPPRESS_ACS_WATCHDOG_TAG_NAME           "Suppress ACS Watchdog"
#define STATUS_KEY_TAG_NAME						 "Status Security"
// session id
#define SESSIONID_TAG_NAME						"Session Id"
extern const char SESSIONID_TAG[];

//licensing
#define LICENSE_KEY_NAME							"Licenses"
extern const char LICENSE_KEY[];
#define LICENSE_DATA_TAG_NAME					"Data"
extern const char LICENSE_DATA_TAG[];

#define	ADDRESS_TRANSLATION_KEY_NAME			"Use Address Translation"
extern const char ADDRESS_TRANSLATION_KEY[];	// = ADDRESS_TRANSLATION_KEY_NAME
#define	ADDRESS_TRANSLATION_USE					"Use"
#define	ADDRESS_TRANSLATION_UNUSE				"Unuse"


// For all MediaBrokers start key

// conference money settings
#define CONF_MONEY_WARN_INIT					"300"
#define CONF_MONEY_WARN_MIN						"60"
#define CONF_MONEY_WARN_MAX						"600"
#define CONF_MONEY_WARN_TAG_NAME				"Conference Money Warning Threshold"
extern const char CONF_MONEY_WARN_TAG[];		// = CONF_MONEY_WARN_TAG_NAME
#define CONF_MONEY_WARN_PERIOD_INIT				"60"
#define CONF_MONEY_WARN_PERIOD_MIN				"30"
#define CONF_MONEY_WARN_PERIOD_MAX				"300"
#define CONF_MONEY_WARN_PERIOD_TAG_NAME			"Conference Money Warning Period"
extern const char CONF_MONEY_WARN_PERIOD_TAG[];	// = CONF_MONEY_WARN_PERIOD_TAG_NAME
#define CONF_DECLIMITPERIOD_INIT				"12"
#define CONF_DECLIMITPERIOD_MIN					"3"
#define CONF_DECLIMITPERIOD_MAX					"120"
#define CONF_DECLIMITPERIOD_TAG_NAME			"Conference DecLimit Period"
extern const char CONF_DECLIMITPERIOD_TAG[];	// = CONF_DECLIMITPERIOD_TAG_NAME


//database configuration settings
#define	STORAGE_TYPE_KEY_NAME					"Storage Type"
extern const char STORAGE_TYPE_KEY[];			// = STORAGE_TYPE_KEY_NAME
#define	STORAGE_TYPE_REGISTRY					"Registry"
#define	STORAGE_TYPE_DB								"DB"
#define	STORAGE_TYPE_LDAP							"LDAP"
#define DB_CONNECTIONSTRING_TAG_NAME			"DB Connection String"
extern const char DB_CONNECTIONSTRING_TAG[];	// = DB_CONNECTIONSTRING_TAG_NAME
#define DB_CONNECTIONSTRING_TAG_1_NAME			"DB Connection String 1"
extern const char DB_CONNECTIONSTRING_TAG_1[];

#define DB_USER_TAG_NAME						"DB User"
extern const char DB_USER_TAG[];				// = DB_USER_TAG_NAME
#define DB_USER_TAG_1_NAME						"DB User 1"
extern const char DB_USER_TAG_1[];

#define DB_PASSWORD_TAG_NAME					"DB Password"
extern const char DB_PASSWORD_TAG[];			// = DB_PASSWORD_TAG_NAME

#define DB_PASSWORD_TAG_1_NAME					"DB Password 1"
extern const char DB_PASSWORD_TAG_1[];			// = DB_PASSWORD_TAG_NAME

#define DB_SCHEMA_NAME							"DB Schema"
extern const char DB_SCHEMA_TAG[];


#define DB_SPHINX_QUERIES_TAG_NAME				"DB Sphinx Queries"
#define DB_SPHINX_HOST_TAG_NAME					"DB Sphinx Host"
#define DB_SPHINX_PORT_TAG_NAME					"DB Sphinx Port"
#define DB_SPHINX_USER_TAG_NAME					"DB Sphinx User"
#define DB_SPHINX_PASSWORD_TAG_NAME				"DB Sphinx Password"
#define DB_SPHINX_DATABASE_TAG_NAME				"DB Sphinx Database"
extern const char DB_SPHINX_QUERIES_TAG[];
extern const char DB_SPHINX_HOST_TAG[];
extern const char DB_SPHINX_PORT_TAG[];
extern const char DB_SPHINX_USER_TAG[];
extern const char DB_SPHINX_PASSWORD_TAG[];
extern const char DB_SPHINX_DATABASE_TAG[];

#define	DB_CONNECTIONS_TAG_NAME				"DB Connections"
extern const char DB_CONNECTIONS_TAG[];	// Number of concurrent database connections
//description: this value depends on number concurrent conferences on a broker
const int	DB_CONNECTIONS_INIT		= 4;		//<default db connections
const int	DB_CONNECTIONS_MIN		= 2;
const int	DB_CONNECTIONS_MAX		= 24;


#define	WWWP_IN_DEPENDENCE_KEY_NAME				"WWWP In Dependence"
extern const char WWWP_IN_DEPENDENCE_KEY[];		// = WWWP_IN_DEPENDENCE_KEY_NAME
#define	WWWP_IN_DEPENDENCE_SET					"Set"
#define	WWWP_IN_DEPENDENCE_UNSET				"Unset"
#define	SERVICE_WATCHDOG_KEY_NAME				"Serwice Watchdog"


/// H323 Gateway configuration
#define H323_HOME_BROKER						"H323 Home Broker"
#define H323_MAX_TERMINALS						"H323 Max Connected Terminals"
#define H323_MAX_SCALLS							"H323 Max Simaltinous Calls"
#define H323_MAX_BRIDGE_CONNECT					"H323 Max Bridge Connections"
#define H323_MAX_GATEWAY_CONNECT				"H323 Max Gateway Connections"
#define H323_LOG_DIRECTORY				        "H323 Log Directory"
#define H323_DIRECTLY_FLAG				        "H323 Try Dirrect Connect"
#define H323_BCAST_NAME							"H323 Broadcast Name"
#define H323_GCONF_NAME							"H323 GroupConf Name"

#define TR_FILE_STORAGE_KEY						"File Storage Path"
#define TR_HTTP_DOWNLOAD_PREFIX_KEY				"Files Download prefix"
#define TR_TRACKERS_KEY                         "trackers"
#define TR_FILES_LIFETIME						"FilesLifeTime"
#define TR_FILES_RETRYTIME						"FilesRetryTime"
#define TR_CHECK_USELESS_TIMEOUT				"CheckUselessTimeout"
// LibTorrent extensions
#define TR_DHT_KEY								"TR Use DHT"
#define TR_LSD_KEY								"TR Use LSD"
#define TR_NAT_PMP_KEY							"TR Use NAT_PMP"
#define TR_UPNP_KEY								"TR Use UPNP"
#define TR_LOCK_FILES_KEY						"TR Lock Files"
#define TR_FILE_POLL_SIZE						"TR File Pool Size"

extern const char SERVER_MANAGER_TAG[];
extern const char KEY_TAG[];
extern const char CURRENT_CONNECT_TAG[];
extern const char CURRENT_GATEWAY_CONNECT_TAG[];

extern const char SAVE_CONF_STAT_TAG[];

// Amazon DDNS
#define DDNS_NOTIFY_REG_ABOUT_IP_NAME           "NotifyRegAboutIP"
#define DDNS_NOTIFY_REG_WITH_ACCEPT_TCP_NAME    "NotifyRegWithAcceptTCP"
extern const char DDNS_NOTIFY_REG_ABOUT_IP[];
extern const char DDNS_NOTIFY_REG_WITH_ACCEPT_TCP[];

// Call Config Corrector
#define CFG_CORRECTOR_DATA_KEY_NAME                 "CallCfgCorrectorLua"
extern const char CFG_CORRECTOR_DATA[];
#define CFG_CORRECTOR_FILENAME_KEY_NAME             "CallCfgCorrectorLuaFile"
extern const char CFG_CORRECTOR_FILENAME[];

#define CFG_CORRECTOR_TIMESTAMP_KEY_NAME             "CallCfgCorrectorLuaTimestamp"
extern const char CFG_CORRECTOR_TIMESTAMP[];

// H323/SIP Peers
extern const char BASE_PEERS_CONFIGURATION_TAG[]; // = "#base";
extern const char VOIP_GATEWAY_TAG[]; // = "VoIP Gateway";

extern const char TELEPHONE_PREFIX_TAG[]; // "Telephone Prefix"

// SIP/H323 connections ACL
#define CONN_ACL_MODE_NAME "Filter Terminals Mode"
extern const char CONN_ACL_MODE[];

#define CONN_ACL_WHITE_LIST_NAME "WhiteList"
extern const char CONN_ACL_WHITE_LIST[];

#define CONN_ACL_BLACK_LIST_NAME "BlackList"
extern const char CONN_ACL_BLACK_LIST[];
