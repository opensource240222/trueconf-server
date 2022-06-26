#include "VS_RegistryConst.h"

//policy
 const char CONFIGURATION_KEY[] = "Configuration";
 const char POLICY_USE_IP[] = "PolicyUseIp";
 const char POLICY_USE_LOGIN[] = "PolicyUseLogin";
 const char POLICY_MAX_FAIL_BEFORE_BAN[] = "PolicyMaxFailBeforeBan";
 const char POLICY_DELAY_TIME[] = "PolicyDelayTime";
 const char POLICY_BAN_TIME[] = "PolicyBanTime";
 const char POLICY_SEND_ACCESS_DENY[] = "PolicySendAccessDeny";
 const char POLICY_SILENT_ON_BAN[] = "PolicySilentOnBan";
 const char POLICY_DELAY_TTL[] = "PolicyDelayTTL";
 const char POLICY_DELAY_INTERVAL[] = "PolicyDelayInterval";
 const char POLICY_MAX_FAIL_BEFORE_DELAY[] = "PolicyMaxFailBeforeDelay";
 const char POLICY_OBSERVE_INTERVAL[] = "PolicyObserveInterval";


const char SIP_PEERS_KEY[] = "SIP Peers";
const char SIP_FROM_HOST[] = "SIP From Host";
const char SIP_RETRY_AFTER[] = "SIP RetryAfter";
const char H323_PEERS_KEY[] = "H323 Peers";
const char TRANSCODERS_KEY[] = "Transcoders";
const char TRANSCEIVER_LOCATION[] = "Transceiver Location";
const char TRANSCEIVERS_KEY[] = "Transceivers";
const char TRANSCEIVER_SHARED_KEY_TAG[] = "Shared Key Transceiver";
const char MIN_FREE_TRANSCEIVERS_TAG[] = "Min free transceivers";
const char MAX_CONFS_BY_TRANSCEIVER_TAG[] = "Max confs by transceiver";
const char MAX_TRANSCEIVER_FREE_TIME_MINUTES[] = "Max transceiver free time in minutes";
const char MAX_TRANSCEIVERS[] = "Max transceivers";
const char RTP_INTERFACE_KEY_TAG[] = "RTP Interface";
const char GROUPS_KEY[] = "Groups";
const char MULTI_CONFERENCES_KEY[] = "Multi Conferences";
const char USERS_KEY[] = "Users";
const char APPLICATION_SETTINGS_KEY[] = "ApplicationSettings";

const char TIME_WAIT_RELOAD[] = "Time Wait Reload";

const char REGISTRATION_MAX_EXPIRES[] = "Registration Max Expires";

const char REGISTRY_STRATEGY[] = "RegisterStrategy";
const char LAST_AUTHORIZATION_RESULT[] = "Last Authorization Result";
const char LAST_CHECK_TIME[] = "Last Check Time";

const char EXECUTABLE_PATH_TAG[]	= EXECUTABLE_PATH_TAG_NAME;
const char WORKING_DIRECTORY_TAG[]	= WORKING_DIRECTORY_TAG_NAME;

const char HTTP_HANDLER_DIRECTORY_TAG[] = HTTP_HANDLER_DIRECTORY_TAG_NAME;
const char HTTP_HANDLER_INDEX_FILE_TAG[] = HTTP_HANDLER_INDEX_FILE_TAG_NAME;
const char HTTP_HANDLER_VERSION_MARKER_TAG[] = HTTP_HANDLER_VERSION_MARKER_TAG_NAME;
const char HTTP_HANDLER_USERS_PASSWORDS_TAG[] = HTTP_HANDLER_USERS_PASSWORDS_TAG_NAME;

// Pool Threads Configuration
const char PTH_MAX_THREADS_TAG[]		= PTH_MAX_THREADS_TAG_NAME;
const char PTH_MAX_QUEUE_TASKS_TAG[]	= PTH_MAX_QUEUE_TASKS_TAG_NAME;
const char PTH_N_PRIORITIES_TAG[]		= PTH_N_PRIORITIES_TAG_NAME;
const char PTH_DEF_LIFETIME_TAG[]		= PTH_DEF_LIFETIME_TAG_NAME;

// SMTP configuration
const char SMTP_SERVER_TAG[]			= SMTP_SERVER_TAG_NAME;
const char SMTP_PORT_TAG[]				= SMTP_PORT_TAG_NAME;
const char SMTP_LOGIN_TAG[]				= SMTP_LOGIN_TAG_NAME;
const char SMTP_PASSWORD_TAG[]			= SMTP_PASSWORD_TAG_NAME;
const char SMTP_FROM_TAG[]				= SMTP_FROM_TAG_NAME;
const char SMTP_USE_SSL_TAG[]			= SMTP_USE_SSL_TAG_NAME;
const char SMTP_AUTH_TYPE_TAG[]			= SMTP_AUTH_TYPE_TAG_NAME;
const char SMTP_MAX_POOL_THREADS_TAG[]	= SMTP_MAX_POOL_THREADS_TAG_NAME;
const char SMTP_DIRECTORY_TAG[]			= SMTP_DIRECTORY_TAG_NAME;
const char SMTP_CHECK_PERIOD_TAG[]		= SMTP_CHECK_PERIOD_TAG_NAME;
const char SMTP_EXTERNAL_CHECK_PERIOD_TAG[]= SMTP_EXTERNAL_CHECK_PERIOD_TAG_NAME;
const char SMTP_ATTEMPTS_PERIOD_TAG[]	= SMTP_ATTEMPTS_PERIOD_TAG_NAME;
const char SMTP_LIFETIME_TAG[]			= SMTP_LIFETIME_TAG_NAME;
const char SMTP_ADMIN_EMAIL_TAG[]		= SMTP_ADMIN_EMAIL_TAG_NAME;

const char TRANSPORT_MESSAGES_LOGS_FILE_TAG[] = TRANSPORT_MESSAGES_LOGS_FILE_TAG_NAME;
const char TRANSPORT_FULL_TRACE_ENABLED_TAG[] = TRANSPORT_FULL_TRACE_ENABLED;
const char SESSIONID_TAG[]				= SESSIONID_TAG_NAME;

//Licensing
const char LICENSE_KEY[]			=	LICENSE_KEY_NAME;
const char LICENSE_DATA_TAG[]	= LICENSE_DATA_TAG_NAME;


const char WWWP_IN_DEPENDENCE_KEY[]		= WWWP_IN_DEPENDENCE_KEY_NAME;
const char ADDRESS_TRANSLATION_KEY[]	= ADDRESS_TRANSLATION_KEY_NAME;

// database configuration settings
const char DB_CONNECTIONSTRING_TAG[]	= DB_CONNECTIONSTRING_TAG_NAME;
const char DB_CONNECTIONSTRING_TAG_1[]	= DB_CONNECTIONSTRING_TAG_1_NAME;
const char DB_USER_TAG[]							= DB_USER_TAG_NAME;
const char DB_USER_TAG_1[]							= DB_USER_TAG_1_NAME;
const char DB_PASSWORD_TAG[]					= DB_PASSWORD_TAG_NAME;
const char DB_PASSWORD_TAG_1[]					= DB_PASSWORD_TAG_1_NAME;
const char DB_CONNECTIONS_TAG[]				= DB_CONNECTIONS_TAG_NAME;
const char DB_SPHINX_QUERIES_TAG[]		= DB_SPHINX_QUERIES_TAG_NAME;
const char DB_SPHINX_HOST_TAG[]			= DB_SPHINX_HOST_TAG_NAME;
const char DB_SPHINX_PORT_TAG[]			= DB_SPHINX_PORT_TAG_NAME;
const char DB_SPHINX_USER_TAG[]			= DB_SPHINX_USER_TAG_NAME;
const char DB_SPHINX_PASSWORD_TAG[]		= DB_SPHINX_PASSWORD_TAG_NAME;
const char DB_SPHINX_DATABASE_TAG[]		= DB_SPHINX_DATABASE_TAG_NAME;
const char DB_SCHEMA_TAG[]				= DB_SCHEMA_NAME;
// servise configuration
const char CONF_MONEY_WARN_TAG[]		= CONF_MONEY_WARN_TAG_NAME;
const char CONF_MONEY_WARN_PERIOD_TAG[]	= CONF_MONEY_WARN_PERIOD_TAG_NAME;
const char CONF_DECLIMITPERIOD_TAG[]	= CONF_DECLIMITPERIOD_TAG_NAME;

//manager
const char SERVER_MANAGER_TAG[]			= "Server Manager";
const char KEY_TAG[]					= "Key";

const char CURRENT_CONNECT_TAG[]			= "Current Connect";
const char CURRENT_GATEWAY_CONNECT_TAG[]	= "Current Gateway Connect";

const char SAVE_CONF_STAT_TAG[]			= "Save Conf Stat";

// Amazon DDNS

const char DDNS_NOTIFY_REG_ABOUT_IP[] = DDNS_NOTIFY_REG_ABOUT_IP_NAME;
const char DDNS_NOTIFY_REG_WITH_ACCEPT_TCP[] = DDNS_NOTIFY_REG_WITH_ACCEPT_TCP_NAME;

// Call Config Corrector
const char CFG_CORRECTOR_DATA[] = CFG_CORRECTOR_DATA_KEY_NAME;
const char CFG_CORRECTOR_FILENAME[] = CFG_CORRECTOR_FILENAME_KEY_NAME;
const char CFG_CORRECTOR_TIMESTAMP[] = CFG_CORRECTOR_TIMESTAMP_KEY_NAME;

// H323/SIP Peers
const char BASE_PEERS_CONFIGURATION_TAG[] = "#base";
const char VOIP_GATEWAY_TAG[] = "VoIP Gateway";

const char TELEPHONE_PREFIX_TAG[] = "Telephone Prefix";

// SIP/H323 connections ACL
const char CONN_ACL_MODE[] = CONN_ACL_MODE_NAME;
const char CONN_ACL_WHITE_LIST[] = CONN_ACL_WHITE_LIST_NAME;
const char CONN_ACL_BLACK_LIST[] = CONN_ACL_BLACK_LIST_NAME;