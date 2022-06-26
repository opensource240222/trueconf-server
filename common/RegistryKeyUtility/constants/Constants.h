#pragma once

//////////////////////////////////////////////////////////////////////////

static const char EMPTY_STR[] = "";

//////////////////////////////////////////////////////////////////////////

static const char TYPE_I32[] = "i32";
static const char TYPE_I64[] = "i64";
static const char TYPE_STR[] = "str";
static const char TYPE_BIN[] = "bin";
static const char TYPE_B64[] = "b64";

//////////////////////////////////////////////////////////////////////////

static const char * const TYPES[] = { TYPE_I32, TYPE_I64, TYPE_STR, TYPE_BIN, TYPE_B64 };

//////////////////////////////////////////////////////////////////////////

static const char HELP[] = "help";
static const char HELP_H[] = "help,h";
static const char HELP_MESSAGE[] = "Show help";

static const char CONFIG_FILE[] = "ConfigFile";
static const char CONFIG_FILE_C[] = "ConfigFile,c";
static const char CONFIG_FILE_MESSAGE[] = "Path to config file (default file tc_server.cfg)";
static const char DEFAULT_CONFIG_FILE[] = "tc_server.cfg";

static const char REGISTRY_BACKEND[] = "RegistryBackend";
static const char REGISTRY_BACKEND_DEFAUL_VALUE[] = "registry:force_lm=true";
static const char REGISTRY_BACKEND_MESSAGE[] = "Registry backend specification in the form NAME:[OPTIONS]";

static const char BACKEND[] = "Backend";
static const char BACKEND_B[] = "Backend,b";
static const char BACKEND_MESSAGE[] = "--Backend INITSTR, -b INITSTR";

static const char ROOT[] = "root";
static const char ROOT_R[] = "root,r";
static const char DEFAULT_ROOT[] = "TrueConf\\Server";
static const char ROOT_MESSAGE[] = "--Root NAME, -r NAME";

static const char COMMANDS[] = "COMMANDS {do not specify}";
static const char SET[] = "set";
static const char SET_MESSAGE[] = "set KNAME LNAME TYPE VALUE";
static const int  LEN_ARGS_SET = 4;

static const char GET[] = "get";
static const char GET_MESSAGE[] = "get KNAME VNAME";
static const int  LEN_ARGS_GET = 2;

static const char RENAME[] = "rename";
static const char RENAME_MESSAGE[] = "rename KNAME KNEW_NAME";
static const int  LEN_ARGS_RENAME = 2;

static const char REMOVE[] = "remove";
static const char REMOVE_MESSAGE[] = "remove KNAME";
static const int  LEN_ARGS_REMOVE = 1;

static const char STORE[] = "store";
static const char STORE_MESSAGE[] = "store KNAME FILE";
static const int  LEN_ARGS_STORE = 2;

static const char LOAD[] = "load";
static const char LOAD_MESSAGE[] = "load KNAME FILE";
static const int  LEN_ARGS_LOAD = 2;
//////////////////////////////////////////////////////////////////////////

static const char EXAMPLES[] = "Examples:\ntc_regkey -r TrueConf\\Server store \\ test.txt - store root(TrueConf\\Server) to file test.txt\ntc_regkey -r TrueConf\\Server get key_name value_name - show value in key: 'key_name', key_value: 'value_name'\ntc_regkey set key_name value_name i32 10 - set value '10' to key: 'key_name', key_name: 'value_name'\ntc_regkey load \\ test.txt - load data to root\\test\n";

//////////////////////////////////////////////////////////////////////////

static const char * const NAME_COMMAND[] = { SET, GET, RENAME, REMOVE, STORE, LOAD };

//////////////////////////////////////////////////////////////////////////

static const char VNAME[] = "VNAME";;
static const char KNAME[] = "KNAME";
static const char NEW_KNAME[] = "NEW_KNAME";
static const char TYPE[] = "TYPE";
static const char VALUE[] = "VALUE";
static const char FILE_STR[] = "FILE";

//////////////////////////////////////////////////////////////////////////

static const char INVALID_INPUT_LIST_MESSAGE[] = "Invalid input list: ";

//////////////////////////////////////////////////////////////////////////

static const char NOT_EMPTY_ERROR_MESSAGE[] = "Must not be empty";
static const char FILE_NOT_FOUNT_OR_BAD_ERROR_MESSAGE[] = "File not found or bad file";
static const char TYPE_NOT_SUPPORTED_ERROR_MESSAGE[] = "Type not supported";
static const char NOT_BASE64_ERROR_MESSAGE[] = "Value not base64";
static const char NUMBER_ERROR_MESSAGE[] = "Not a number";
static const char OPEN_STORAGE_ERROR_MESSAGE[] = "Error open storage";
static const char INVALID_DATA_STORAGE_ERROR_MESSAGE[] = "Invalid data storage";
static const char INPUT_NOT_SUPPORTED_NAME_ERROR_MESSAGE[] = "Input format not supported (\\)";
static const char INPUT_NOT_SUPPORTED_EMPTY_STR_ERROR_MESSAGE[] = "The input string does not need to be empty";
static const char RENAME_KEY_ERROR_MESSAGE[] = "Error rename key";
static const char OPEN_KEY_ERROR_MESSAGE[] = "Error open key";
static const char EXECUTE_LAOD_COMMAND_ERROR_MESSAGE[] = "Error Execute LoadCommand";
static const char SET_VALUE_TO_ERROR_MESSAGE[] = "Error Set value to";
static const char MESSAGE_MSG[] = "message";

//////////////////////////////////////////////////////////////////////////

static const char DELIMITER = '\\';

//////////////////////////////////////////////////////////////////////////

static const char VALUE_NAME_STR[] = "string";
static const char VALUE_NAME_FILE[] = "filename";

//////////////////////////////////////////////////////////////////////////

static const char ERROR_VALID_COMMAND_MSG[] = "Erron Valid command";

//////////////////////////////////////////////////////////////////////////