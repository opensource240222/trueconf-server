/**
 ****************************************************************************
 * (c) 2002-2003 Visicron Inc.  http://www.visicron.net/
 *
 * Project: Visicron Registry Server
 *
 * \file VS_RegDBStorage.cpp
 * This is the abstract class, base of directory server
 * storage class
 *
 * $Revision: 25 $
 * $History: VS_RegDBStorage.cpp $
 *
 * *****************  Version 25  *****************
 * User: Mushakov     Date: 16.03.12   Time: 15:54
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - Add OS and CPU info in Regiatration Server
 *
 * *****************  Version 24  *****************
 * User: Mushakov     Date: 5.10.11    Time: 22:19
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - debug log added
 *
 * *****************  Version 23  *****************
 * User: Mushakov     Date: 5.10.11    Time: 21:36
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - ssl refactoring (SetCert interfaces)
 *
 * *****************  Version 22  *****************
 * User: Ktrushnikov  Date: 8.08.11    Time: 14:23
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - FromVariant_NoTZ() for valid_until and valid_after
 * - include path fixed
 *
 * *****************  Version 21  *****************
 * User: Mushakov     Date: 28.03.11   Time: 15:35
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - db_null added
 *
 * *****************  Version 20  *****************
 * User: Mushakov     Date: 25.03.11   Time: 15:30
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - check conn required lic in offline registration
 *
 * *****************  Version 19  *****************
 * User: Mushakov     Date: 23.03.11   Time: 17:49
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - Server Name Verification added at registration
 *
 * *****************  Version 18  *****************
 * User: Mushakov     Date: 10.03.11   Time: 20:20
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - add USER_INVALID status
 * - revoke/assign ROAMING handled
 *
 * *****************  Version 17  *****************
 * User: Mushakov     Date: 23.02.11   Time: 4:05
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - SecureHandshake ver. 2 added
 *
 * *****************  Version 16  *****************
 * User: Mushakov     Date: 1.11.10    Time: 21:00
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - cert update added
 * - registration from server added
 * - authorization servcer added
 *
 * *****************  Version 15  *****************
 * User: Ktrushnikov  Date: 28.07.10   Time: 16:27
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - VS_FileTime::operator variant_t() doesn't add TimeZone now
 * - Use another func to return variant with TimeZone
 *
 * *****************  Version 14  *****************
 * User: Mushakov     Date: 25.03.10   Time: 19:40
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - log added
 * - reg connect required
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 26.02.10   Time: 17:38
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - log added
 *
 * *****************  Version 12  *****************
 * User: Dront78      Date: 26.02.10   Time: 12:54
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *
 * *****************  Version 11  *****************
 * User: Dront78      Date: 18.02.10   Time: 10:09
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * Armadillo Security Passport added
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 12.02.10   Time: 15:56
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - ARM Keys added
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 8.02.10    Time: 18:04
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - some commemts removed
 * - readingtrial minutes added
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 18.12.09   Time: 18:04
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - Removed VCS_BUILD somewhere
 * - Add new field to license
 * - Chat service for bsServer renamed
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 19.11.09   Time: 17:39
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - recordset Closed
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 19.11.09   Time: 15:12
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - VS_ServCertInfoInterface::GetPublicKey modified
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 28.10.09   Time: 17:58
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - vs_ep_id removed
 * - registration corrected
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 26.10.09   Time: 20:32
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - sign verification added
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 23.10.09   Time: 15:05
 * Updated in $/VSNA/Servers/RegistryServer/Services
 *  - VCS 3
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 7.08.09    Time: 15:14
 * Updated in $/VSNA/Servers/RegistryServer/Services
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 31.03.09   Time: 17:35
 * Created in $/VSNA/Servers/RegistryServer/Services
 * RegistryServer added
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 22.03.07   Time: 16:18
 * Updated in $/VS2005/Servers/DirectoryServer/Services
 * добавлена верификация ProductType при регистрации брокера
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:51
 * Created in $/VS2005/Servers/DirectoryServer/Services
 *
 * *****************  Version 18  *****************
 * User: Stass        Date: 6.12.06    Time: 12:31
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added signed  license encode for server protocol version 3
 *
 * *****************  Version 17  *****************
 * User: Stass        Date: 29.08.06   Time: 13:30
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added start_time and gateways number from DB
 *
 * *****************  Version 16  *****************
 * User: Stass        Date: 2.05.06    Time: 19:36
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 15  *****************
 * User: Mushakov     Date: 2.05.06    Time: 17:56
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 14  *****************
 * User: Stass        Date: 2.05.06    Time: 14:04
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 13  *****************
 * User: Mushakov     Date: 26.04.06   Time: 16:33
 * Updated in $/VS/Servers/DirectoryServer/Services
 *
 * *****************  Version 12  *****************
 * User: Mushakov     Date: 25.04.06   Time: 15:54
 * Updated in $/VS/Servers/DirectoryServer/Services
 * Added certificate issue
 *
 * *****************  Version 11  *****************
 * User: Stass        Date: 5.04.04    Time: 14:29
 * Updated in $/VS/Servers/DirectoryServer/Services
 * fixed DB reconnect, added shutdown when DB failed
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 4.03.04    Time: 21:44
 * Updated in $/VS/Servers/DirectoryServer/Services
 * collected registry for servers
 *
 * *****************  Version 9  *****************
 * User: Stass        Date: 30.01.04   Time: 18:39
 * Updated in $/VS/Servers/DirectoryServer/Services
 * добавил правильное поведение в случае поломки базы
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 28.01.04   Time: 20:09
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added license update logic
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 26.01.04   Time: 16:21
 * Updated in $/VS/Servers/DirectoryServer/Services
 * IP logging changed
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 20.01.04   Time: 19:37
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added debug printout
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 9.01.04    Time: 13:34
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added state save to DB
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 6.01.04    Time: 20:33
 * Updated in $/VS/Servers/DirectoryServer/Services
 * module based debug print
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 30.12.03   Time: 18:14
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added getkey func
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 29.12.03   Time: 18:27
 * Updated in $/VS/Servers/DirectoryServer/Services
 * added RegisterBroker method
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 25.12.03   Time: 21:22
 * Created in $/VS/Servers/DirectoryServer/Services
 * added registration service and storage
 ****************************************************************************/

static const char DB_TYPE_PARAM[]		=	"@type";
static const char DB_F1_PARAM[]			=	"@f1";
static const char DB_F2_PARAM[]			=	"@f2";
static const char DB_F3_PARAM[]			=	"@f3";
static const char DB_STATE_PARAM[]	=	"@state";
static const char DB_IP_PARAM[]			=	"@ip";

static const char DB_BROKER_ID_PARAM[]=	"@broker_id";
static const char DB_SERVER_NAME_PARAM[] = "@server_name";

static const char DB_HWKEY_PARAM[]			=	"@hwkey";
static const char DB_KEY_PARAM[]			=	"@key";
static const char DB_SERIAL_PARAM[]		=	"@serial";

#include <algorithm>

#include "VS_RegDBStorage.h"
#include "../../LicenseLib/VS_License.h"
#include "../../common/std/cpplib/VS_IntConv.h"
#include "std/cpplib/md5.h"
#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "../../common/std/debuglog/VS_Debug.h"
#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "../../common/std/cpplib/VS_FileTime.h"
#include "std/cpplib/StrFromVariantT.h"
#include "std/RegistrationStatus.h"
#include "tlb_import/msado26.tli"

#define DEBUG_CURRENT_MODULE VS_DM_REGDBST


VS_RegDBStorage::VS_RegDBStorage():db_null(), IsPostgreSQL(false)
{

	m_brokers.SetPredicate  (VS_SimpleStr::Predicate);
	m_brokers.SetKeyFactory (VS_SimpleStr::Factory, VS_SimpleStr::Destructor);
	///m_brokers.SetDataFactory(VS_Int32::Factory,VS_Int32::Destructor);
	db_null.ChangeType(VT_NULL);

};

void VS_RegDBStorage::ProcessCOMError(_com_error e)
	{
			_bstr_t bstrSource(e.Source());
			_bstr_t bstrDescription(e.Description());

			error_code=VSS_DB_ADO_ERROR;


			VS_FileTime t;
			t.Now();
			char buf[256];

			// Print COM errors.
			int count=db?db->Errors->Count:0;
			dprint1("#at %s\n",t.ToNStr(buf));
			if(count==0)
			{
				dprint1("#COM Error [%08lx] \n",e.Error());
				dprint1(" Msg         = %s \n", e.ErrorMessage());
				dprint1(" Source      = %s \n", (LPCSTR) bstrSource);
				dprint1(" Description = %s \n", (LPCSTR) bstrDescription);
				error_code=VSS_DB_COM_ERROR;
				dprint1(" DB State: %02lx \n", db?db->State:-1);
			}
			else
			{
				ADODB::ErrorPtr ae= db->Errors->GetItem((short)0);
				auto error_number = ae->Number;
				auto error_native = ae->NativeError;
				if (error_number == 0x80004005 && (
					(IsPostgreSQL && (error_native == 0x65 || error_native == 0x23)) || // try to reconnect to Postgres
					(!IsPostgreSQL && (error_native == 0x0 || error_native == 0x11)) // try to reconnect MS-SQL
					))
				{
					if(state==STATE_RUNNING)
					{
						dprint0("#DB connection error, trying to reconnect\n");
						state=STATE_RECONNECT;
						if(!Init())
						{
							state=STATE_FAILED;
							dprint0("#reconnect failed, setting storage state to FAIL\n");
						};
					}
					else
					{
						dprint1("db connection error\n");
					};


					//error_code=VSS_DB_CONN_ERROR
				}
				else
				{
					dprint1("#ADO Error [%08lx] native(%08lx) (total:%d)\n",ae->Number,ae->NativeError,count);
					dprint1(" Source = %s \n", (char*)(_bstr_t)ae->Source);
					dprint1(" Description = %s \n", (char*)(_bstr_t)ae->Description);
					error_code=VSS_DB_ADO_ERROR;
					dprint1(" DB State: %02lx \n", db?db->State:-1);
				};
			};

			fflush(stdout);
};


bool VS_RegDBStorage::Init()
{
	char buff[512];

	  ////////////////////////////////////////////////////////////
	  // database init
	  VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);
	  if (!cfg_root.IsValid()) {
		  error_code=VSS_REGISTRY_ERROR;
		  return false;
	  };

	  VS_SimpleStr conn, username, password, schema;
	  std::string schema_prefix;

	  if (cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT,DB_CONNECTIONSTRING_TAG ) > 0)
		  conn = buff;
	  if (cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT,DB_PASSWORD_TAG) > 0)
		  password = buff;
	  if (cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT,DB_USER_TAG ) > 0)
		  username = buff;
	  if (cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, DB_SCHEMA_TAG) > 0)
		  schema = buff;

	  // check for PostgreSQL
	  {
		  std::string s(conn);
		  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
		  if (strstr(s.c_str(), "postgresql"))
		  {
			  IsPostgreSQL = true;
			  dprint1("Trying to connect to a PostgreSQL database.");
		  }
	  }

	  // set default "DB Schema" value for PostgreSQL
	  if (IsPostgreSQL && schema.Length() == 0)
	  {
		  schema = "server";
	  }

	  if (schema.Length() > 0)
	  {
		  schema_prefix = schema;
		  schema_prefix.append(".");
	  }

	  fflush(stdout);

	  try
	  {db.CreateInstance("ADODB.Connection");}
	  catch(_com_error e)
	  {
		  ProcessCOMError(e);
		  return false;
	  }

		bool init=false;
	  for(int i=0;i<reconnect_max;i++)
	  {
		  try
		  {
			  db->Open((char *)conn,(char *)username,(char *)password,ADODB::adConnectUnspecified);
			  init=true;
				break;
		  }
		  catch(_com_error e)
		  {
			  dprint1(".%d.\n",reconnect_max-i);
			  ProcessCOMError(e);
			  fflush(stdout);
			  SleepEx(reconnect_timeout,false);

		  }
	  };
	  if(!init)
			return false;

	  printf(".");
	  try
		{
			reg_cleanup.CreateInstance("ADODB.Command");
			reg_cleanup->ActiveConnection=db;
			if (!IsPostgreSQL)
			{
				reg_cleanup->CommandText = (schema_prefix + "reg_cleanup").c_str();
				reg_cleanup->CommandType = ADODB::adCmdStoredProc;
				reg_cleanup->Parameters->Refresh();
			}
			else
			{
				std::string cmd = "select ";
				cmd += (schema_prefix + "server_cleanup_state()");
				reg_cleanup->CommandText = cmd.c_str();
				reg_cleanup->CommandType = ADODB::adCmdText;
			}

			log_event.CreateInstance("ADODB.Command");
			log_event->ActiveConnection=db;
			if (IsPostgreSQL)
			{
				log_event->CommandText = (schema_prefix + "event_add_event").c_str();
			}
			else
			{
				log_event->CommandText = (schema_prefix + "log_event").c_str();
			}
			log_event->CommandType=ADODB::adCmdStoredProc;
			log_event->Parameters->Refresh();

			log_stats.CreateInstance("ADODB.Command");
			log_stats->ActiveConnection=db;
			if (IsPostgreSQL)
			{
				log_stats->CommandText = (schema_prefix + "stat_add_stat").c_str();
			}
			else
			{
				log_stats->CommandText = (schema_prefix + "log_stats").c_str();
			}
			log_stats->CommandType=ADODB::adCmdStoredProc;
			log_stats->Parameters->Refresh();

			log_ip.CreateInstance("ADODB.Command");
			log_ip->ActiveConnection=db;
			if (IsPostgreSQL)
			{
				log_ip->CommandText = (schema_prefix + "server_change_ip_address").c_str();
			}
			else
			{
				log_ip->CommandText = (schema_prefix + "log_ip").c_str();
			}
			log_ip->CommandType=ADODB::adCmdStoredProc;
			log_ip->Parameters->Refresh();

			register_broker.CreateInstance("ADODB.Command");
			register_broker->ActiveConnection=db;
			if (IsPostgreSQL)
			{
				register_broker->CommandText = (schema_prefix + "server_register_server").c_str();
			}
			else
			{
				register_broker->CommandText = (schema_prefix + "register_broker").c_str();
			}
			register_broker->CommandType=ADODB::adCmdStoredProc;
			if (!IsPostgreSQL)
			{
				register_broker->Parameters->Refresh();
			}
			else
			{
				auto param_ptr = register_broker->CreateParameter("_server_id", ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				register_broker->Parameters->Append(param_ptr);
				param_ptr = register_broker->CreateParameter("_serial", ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				register_broker->Parameters->Append(param_ptr);
				param_ptr = register_broker->CreateParameter("_hwkey", ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				register_broker->Parameters->Append(param_ptr);
				param_ptr = register_broker->CreateParameter("_key", ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				register_broker->Parameters->Append(param_ptr);
				param_ptr = register_broker->CreateParameter("_public_key", ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				register_broker->Parameters->Append(param_ptr);
				param_ptr = register_broker->CreateParameter("_server_name", ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				register_broker->Parameters->Append(param_ptr);
			}

			broker_get_licenses.CreateInstance("ADODB.Command");
			broker_get_licenses->ActiveConnection=db;
			if (IsPostgreSQL)
			{
				broker_get_licenses->CommandText = (schema_prefix + "server_get_licenses").c_str();
			}
			else
			{
				broker_get_licenses->CommandText = (schema_prefix + "broker_get_licenses").c_str();
			}
			broker_get_licenses->CommandType=ADODB::adCmdStoredProc;
			if (!IsPostgreSQL)
			{
				broker_get_licenses->Parameters->Refresh();
			}
			else
			{
				auto param_ptr = broker_get_licenses->CreateParameter("_server_id", ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				broker_get_licenses->Parameters->Append(param_ptr);
				param_ptr = broker_get_licenses->CreateParameter("_server_name", ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				broker_get_licenses->Parameters->Append(param_ptr);
				param_ptr = broker_get_licenses->CreateParameter("_kind", ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				broker_get_licenses->Parameters->Append(param_ptr);
			}

			broker_set_state.CreateInstance("ADODB.Command");
			broker_set_state->ActiveConnection=db;
			if (IsPostgreSQL)
			{
				broker_set_state->CommandText = (schema_prefix + "server_set_state").c_str();
			}
			else
			{
				broker_set_state->CommandText = (schema_prefix + "broker_set_state").c_str();
			}
			broker_set_state->CommandType=ADODB::adCmdStoredProc;
			broker_set_state->Parameters->Refresh();

			broker_get_public_key_by_server_name.CreateInstance("ADODB.Command");
			broker_get_public_key_by_server_name->ActiveConnection=db;
			if (IsPostgreSQL)
			{
				broker_get_public_key_by_server_name->CommandText = (schema_prefix + "server_get_public_key").c_str();
			}
			else
			{
				broker_get_public_key_by_server_name->CommandText = (schema_prefix + "broker_get_public_key_by_server_name").c_str();
			}
			broker_get_public_key_by_server_name->CommandType=ADODB::adCmdStoredProc;
			if (!IsPostgreSQL)
			{
				broker_get_public_key_by_server_name->Parameters->Refresh();
			}
			else
			{
				auto param_ptr = broker_get_public_key_by_server_name->CreateParameter("_server_name", ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				broker_get_public_key_by_server_name->Parameters->Append(param_ptr);
			}

			broker_set_arm_hw_key.CreateInstance("ADODB.Command");
			broker_set_arm_hw_key->ActiveConnection=db;
			if (IsPostgreSQL)
			{
				broker_set_arm_hw_key->CommandText = (schema_prefix + "server_set_arm_hw_key").c_str();
			}
			else
			{
				broker_set_arm_hw_key->CommandText = (schema_prefix + "broker_set_arm_hw_key").c_str();
			}
			broker_set_arm_hw_key->CommandType=ADODB::adCmdStoredProc;
			broker_set_arm_hw_key->Parameters->Refresh();

			broker_get_arm_hw_key.CreateInstance("ADODB.Command");
			broker_get_arm_hw_key->ActiveConnection=db;
			if (IsPostgreSQL)
			{
				broker_get_arm_hw_key->CommandText = (schema_prefix + "server_get_arm_hw_key").c_str();
			}
			else
			{
				broker_get_arm_hw_key->CommandText = (schema_prefix + "broker_get_arm_hw_key").c_str();
			}
			broker_get_arm_hw_key->CommandType=ADODB::adCmdStoredProc;
			if (!IsPostgreSQL)
			{
				broker_get_arm_hw_key->Parameters->Refresh();
			}
			else
			{
				auto param_ptr = broker_get_arm_hw_key->CreateParameter("_server_name", ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				broker_get_arm_hw_key->Parameters->Append(param_ptr);
			}

			broker_get_max_license.CreateInstance("ADODB.Command");
			broker_get_max_license->ActiveConnection=db;
			if (IsPostgreSQL)
			{
				broker_get_max_license->CommandText = (schema_prefix + "server_get_max_license").c_str();
			}
			else
			{
				broker_get_max_license->CommandText = (schema_prefix + "broker_get_max_license").c_str();
			}
			broker_get_max_license->CommandType=ADODB::adCmdStoredProc;
			if (!IsPostgreSQL)
			{
				broker_get_max_license->Parameters->Refresh();
			}
			else
			{
				auto param_ptr = broker_get_max_license->CreateParameter("_server_name", ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				broker_get_max_license->Parameters->Append(param_ptr);
			}

			broker_set_certificate.CreateInstance("ADODB.Command");
			broker_set_certificate->ActiveConnection=db;
			if (IsPostgreSQL)
			{
				broker_set_certificate->CommandText = (schema_prefix + "server_set_certificate").c_str();
			}
			else
			{
				broker_set_certificate->CommandText = (schema_prefix + "broker_set_certificate").c_str();
			}
			broker_set_certificate->CommandType=ADODB::adCmdStoredProc;
			broker_set_certificate->Parameters->Refresh();


			broker_get_certificate.CreateInstance("ADODB.Command");
			broker_get_certificate->ActiveConnection=db;
			if (IsPostgreSQL)
			{
				broker_get_certificate->CommandText = (schema_prefix + "server_get_certificate").c_str();
			}
			else
			{
				broker_get_certificate->CommandText = (schema_prefix + "broker_get_certificate").c_str();
			}
			broker_get_certificate->CommandType = ADODB::adCmdStoredProc;
			if (!IsPostgreSQL)
			{
				broker_get_certificate->Parameters->Refresh();
			}
			else
			{
				auto param_ptr = broker_get_certificate->CreateParameter("_server_name", ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				broker_get_certificate->Parameters->Append(param_ptr);
			}

			broker_get_new_certificateId.CreateInstance("ADODB.Command");
			broker_get_new_certificateId->ActiveConnection=db;
			if (IsPostgreSQL)
			{
				broker_get_new_certificateId->CommandText = (schema_prefix + "certificate_add_certificate").c_str();
			}
			else
			{
				broker_get_new_certificateId->CommandText = (schema_prefix + "broker_get_new_certificateId").c_str();
			}
			broker_get_new_certificateId->CommandType=ADODB::adCmdStoredProc;
			if (!IsPostgreSQL)
			{
				broker_get_new_certificateId->Parameters->Refresh();
			}
			else
			{
				auto param_ptr = broker_get_new_certificateId->CreateParameter("_server_name", ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				broker_get_new_certificateId->Parameters->Append(param_ptr);
				param_ptr = broker_get_new_certificateId->CreateParameter("_expire_at", ADODB::DataTypeEnum::adDBTimeStamp, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				broker_get_new_certificateId->Parameters->Append(param_ptr);
			}

			get_call_cfg_corrector.CreateInstance("ADODB.Command");
			get_call_cfg_corrector->ActiveConnection = db;
			if (!IsPostgreSQL)
			{
				get_call_cfg_corrector->CommandText = (schema_prefix + "get_call_cfg_corrector").c_str();
				get_call_cfg_corrector->CommandType = ADODB::adCmdStoredProc;
				get_call_cfg_corrector->Parameters->Refresh();
			}
			else
			{
				std::string cmd = "select * from ";
				cmd += (schema_prefix + "script_get_script()");
				get_call_cfg_corrector->CommandText = cmd.c_str();
				get_call_cfg_corrector->CommandType = ADODB::adCmdText;
			}
			printf(". ");
		}
		catch(_com_error e)
		{
			ProcessCOMError(e);
			return false;
		}

		error_code=0;
		state=STATE_RUNNING;
		return true;

};

bool VS_RegDBStorage::CleanUp()
{
		try
		{
			dprint3("$CleanUp\n");
			reg_cleanup->Execute(NULL,NULL, ADODB::adCmdUnspecified | ADODB::adExecuteNoRecords);
		}
		catch(_com_error e)
		{
			ProcessCOMError(e);
			return false;
		};
	return true;
};


bool VS_RegDBStorage::LogEvent(VS_BrokerEvents type,const VS_SimpleStr& server,const char* f1,const char* f2,const char* f3,VS_RegStorage::BrokerStates state,bool by_server_name)
{
	VS_AutoLock	lock(this);
	try
	{
		dprint3("$LogEvent: type=%d,server=%s, (%s,%s,%s)\n",type,(const char*)server,f1,f2,f3);

		ADODB::ParametersPtr p=log_event->Parameters;

		if (IsPostgreSQL)
		{
			p->Item["_type"]->Value = type;

			p->Item[by_server_name ? "_server_name" : "_server_id"]->Value = server.m_str;
			p->Item[by_server_name ? "_server_id" : "_server_name"]->Value = db_null;

			p->Item["_f1"]->Value = f1;
			p->Item["_f2"]->Value = f2;
			p->Item["_f3"]->Value = f3;

			p->Item["_server_state"]->Value = state;
		}
		else
		{
			p->Item[DB_TYPE_PARAM]->Value = type;

			p->Item[by_server_name ? DB_SERVER_NAME_PARAM : DB_BROKER_ID_PARAM]->Value = server.m_str;
			p->Item[by_server_name ? DB_BROKER_ID_PARAM : DB_SERVER_NAME_PARAM]->Value = db_null;

			/////p->Item[DB_SERVER_NAME_PARAM]->Value	=server;

			p->Item[DB_F1_PARAM]->Value = f1;
			p->Item[DB_F2_PARAM]->Value = f2;
			p->Item[DB_F3_PARAM]->Value = f3;

			p->Item[DB_STATE_PARAM]->Value = state;
		}

		log_event->Execute(NULL,NULL,ADODB::adExecuteNoRecords);
	}
	catch(_com_error e)
	{
		ProcessCOMError(e);
		return false;
	};
	return true;
};

bool VS_RegDBStorage::LogIP(const VS_SimpleStr& server_name,const char* ip)
{
		try
		{
			dprint3("$LogIP: server=%s, ip=%s\n",(const char*)server_name,ip);

			ADODB::ParametersPtr p=log_ip->Parameters;

			if (IsPostgreSQL)
			{
				p->Item["_server_name"]->Value = server_name.m_str;
				p->Item["_new_ip"]->Value = ip;
			}
			else
			{
				p->Item[DB_SERVER_NAME_PARAM]->Value = server_name.m_str;
				p->Item[DB_IP_PARAM]->Value = ip;
			}

			log_ip->Execute(NULL,NULL,ADODB::adExecuteNoRecords);
		}
		catch(_com_error e)
		{
			ProcessCOMError(e);
			return false;
		};
	return true;
};


bool VS_RegDBStorage::LogStats(const VS_SimpleStr& server_name,const VS_MediaBrokerStats* stats)
{
	if(!stats)
		return false;

		try
		{
			dprint3("$LogStats for %s\n",(const char*)server_name);

			ADODB::ParametersPtr p=log_stats->Parameters;

			VS_FileTime curr_time(stats->m_currTime);
			if (IsPostgreSQL)
			{
				p->Item["_server_name"]->Value = server_name.m_str;
				p->Item["_server_timestamp"]->Value = curr_time.ToVariant_WithTZ();
				p->Item["_period"]->Value = stats->m_periodOfAveraging;

				p->Item["_avg_confs"]->Value = stats->m_AvConfs;
				p->Item["_avg_eps"]->Value = stats->m_AvEndpts;
				p->Item["_avg_users"]->Value = stats->m_AvUsers;
				p->Item["_avg_parts"]->Value = stats->m_AvParts;

				p->Item["_avg_load"]->Value = stats->m_AvPrLoad;
				p->Item["_avg_net_load"]->Value = stats->m_AvNetLoad;

				p->Item["_total_confs"]->Value = stats->m_TotalConfs;
				p->Item["_total_eps"]->Value = stats->m_TotalEndpoints;
				p->Item["_total_users"]->Value = stats->m_TotalUsers;
			}
			else
			{
				p->Item[DB_SERVER_NAME_PARAM]->Value = server_name.m_str;
				p->Item["@broker_time"]->Value			= VS_FileTime(stats->m_currTime).ToVariant_WithTZ();
				p->Item["@period"]->Value = stats->m_periodOfAveraging;

				p->Item["@avg_confs"]->Value = stats->m_AvConfs;
				p->Item["@avg_eps"]->Value = stats->m_AvEndpts;
				p->Item["@avg_users"]->Value = stats->m_AvUsers;
				p->Item["@avg_parts"]->Value = stats->m_AvParts;

				p->Item["@avg_load"]->Value = stats->m_AvPrLoad;
				p->Item["@avg_net_load"]->Value = stats->m_AvNetLoad;

				p->Item["@total_confs"]->Value = stats->m_TotalConfs;
				p->Item["@total_eps"]->Value = stats->m_TotalEndpoints;
				p->Item["@total_users"]->Value = stats->m_TotalUsers;
			}

			log_stats->Execute(NULL,NULL,ADODB::adExecuteNoRecords);
		}
		catch(_com_error e)
		{
			ProcessCOMError(e);
			return false;
		};
	return true;
};

bool VS_RegDBStorage::RegisterServer(const VS_SimpleStr& server_id,const VS_SimpleStr &server_name, const VS_SimpleStr& hwkey,
									const VS_SimpleStr& genkey,const VS_SimpleStr& serial,
									const VS_SimpleStr& pub_key,
									VS_WideStr &organization_name, VS_WideStr &country,
									VS_WideStr &contact_person, VS_WideStr &contact_email,
									unsigned long &cert_serial_number,
									VS_FileTime &notBefore, VS_FileTime &notAfter, VS_SimpleStr &hw_md5,
									int &regRes)
{
	VS_AutoLock lock(this);
	try
	{
		RegStatus state = RegStatus::failed;
		dprint3("$RegisterServer %s,key='%s',serial='%s'\n", server_id.m_str, hwkey.m_str, serial.m_str);

		ADODB::ParametersPtr p=register_broker->Parameters;

		if (IsPostgreSQL)
		{
			p->Item["_server_id"]->Value = server_id.m_str;
			p->Item["_hwkey"]->Value = hwkey.m_str;
			p->Item["_key"]->Value = genkey.m_str;
			p->Item["_serial"]->Value = serial.m_str;
			p->Item["_public_key"]->Value = pub_key.m_str;
			p->Item["_server_name"]->Value = server_name.m_str;
		}
		else
		{
			p->Item[DB_BROKER_ID_PARAM]->Value = server_id.m_str;
			p->Item[DB_HWKEY_PARAM]->Value = hwkey.m_str;
			p->Item[DB_KEY_PARAM]->Value = genkey.m_str;
			p->Item[DB_SERIAL_PARAM]->Value = serial.m_str;

			p->Item["@public_key"]->Value = pub_key.m_str;
			p->Item[DB_SERVER_NAME_PARAM]->Value = server_name.m_str;
		}

		ADODB::_RecordsetPtr rs=register_broker->Execute(NULL,NULL,0);
		if (rs == NULL)
			return false;

		if (IsPostgreSQL)
		{
			state = static_cast<RegStatus>(static_cast<int32_t>(rs->Fields->Item["allow"]->Value));
			VS_FileTime exp_date;
			if (state == RegStatus::succeeded)
			{
				cert_serial_number = rs->Fields->Item["certificate_id"]->Value;
				notAfter = rs->Fields->Item["expire_at"]->Value;
				notBefore = rs->Fields->Item["start_at"]->Value;

				{
					VS_SimpleStr key = vs::StrFromVariantT(rs->Fields->Item[L"key"]->Value);
					char hash[64] = { 0 };
					if (!!key)
					{
						VS_ConvertToMD5(SimpleStrToStringView(key), hash);
						hw_md5 = hash;
					}
				}

				organization_name = vs::WStrFromVariantT(rs->Fields->Item["company_name"]->Value);
				country = vs::WStrFromVariantT(rs->Fields->Item["company_country"]->Value);
				contact_person = vs::WStrFromVariantT(rs->Fields->Item["contact_name"]->Value);
				contact_email = vs::WStrFromVariantT(rs->Fields->Item["contact_email"]->Value);
			}
			// Log registration status description.
			{
				const char *msg;
				switch (state)
				{
				case RegStatus::failed:
					msg = "Server is not found";
					break;
				case RegStatus::succeeded:
					msg = "OK";
					break;
				case RegStatus::changingHardwareIsNotAllowed:
					msg = "HW-key is not correct";
					break;
				case RegStatus::serverNameIsInUse:
					msg = "Server name is empty or not unique";
					break;
				case RegStatus::validLicenseIsNotAvailable:
					msg = "There is no valid license found";
					break;
				default:
					msg = "Unknown registration status code";
					break;
				}

				dprint3("Registration status %d: %s.", state, msg);
			}
		}
		else
		{
			state = static_cast<RegStatus>(static_cast<int32_t>(rs->Fields->Item["allow"]->Value));
			VS_FileTime exp_date;
			if (state == RegStatus::succeeded)
			{
				organization_name = vs::WStrFromVariantT(rs->Fields->Item["company_name"]->Value);
				country = vs::WStrFromVariantT(rs->Fields->Item["company_country"]->Value);
				contact_person = vs::WStrFromVariantT(rs->Fields->Item["contact_name"]->Value);
				contact_email = vs::WStrFromVariantT(rs->Fields->Item["contact_email"]->Value);

				notAfter = rs->Fields->Item["exp_date"]->Value;
				notBefore = rs->Fields->Item["start_date"]->Value;
				cert_serial_number = rs->Fields->Item["cert_id"]->Value;
				VS_SimpleStr key = vs::StrFromVariantT(rs->Fields->Item[L"key"]->Value);
				char hash[64] = { 0 };
				if (!!key)
				{
					VS_ConvertToMD5(SimpleStrToStringView(key), hash);
					hw_md5 = hash;
				}
			};
			dprint3("\t state=%d\n", state);
		}
		rs->Close();
		regRes = static_cast<int32_t>(state);
		return state == RegStatus::succeeded;
	}
	catch(_com_error e)
	{
		regRes = -1;
		ProcessCOMError(e);
		return false;
	};
};

bool VS_RegDBStorage::GetNewCertificateId(const VS_SimpleStr &server_name, const VS_FileTime &exp_date, unsigned long &cert_serial_num)
{
	try
	{
		dprint3("$GetNewCertificateId server_name = %s\n", server_name.m_str);

		ADODB::ParametersPtr p = broker_get_new_certificateId->Parameters;

		if (IsPostgreSQL)
		{
			p->Item["_server_name"]->Value = server_name.m_str;
			p->Item["_expire_at"]->Value = exp_date;
		}
		else
		{
			p->Item["@c_server_name"]->Value = server_name.m_str;
			p->Item["@d_exp_date"]->Value = exp_date;
		}
		ADODB::_RecordsetPtr rs=broker_get_new_certificateId->Execute(NULL,NULL,0);
		if (rs != 0 && rs->State != ADODB::adStateClosed && !rs->ADOEOF)
		{
			if (IsPostgreSQL)
			{
				int error = rs->Fields->Item["error"]->Value;
				if (error == 0)
				{
					cert_serial_num = rs->Fields->Item["certificate_id"]->Value;
				}
				else
				{
					cert_serial_num = 0;
				}
			}
			else
			{
				cert_serial_num = rs->Fields->Item["cert_id"]->Value;
			}
		}
		if (rs != NULL)
		{
			rs->Close();
		}
		return true;
	}
	catch(_com_error e)
	{
		ProcessCOMError(e);
		return false;
	};
}

VS_ServCertInfoInterface::get_info_res
VS_RegDBStorage::GetPublicKey(
	const VS_SimpleStr &server_name,
	VS_SimpleStr &pub_key, uint32_t &vcs_ver)
{
	try
	{
		/**
			получить сертификат, если его нет, то получить Public_key
		*/
		VS_SimpleStr	cert_pem;
		if(get_info_res::ok==GetServerCertificate(server_name,cert_pem))
		{
			VS_Certificate cert;
			VS_PKey        pkey;
			unsigned long cert_len = !cert_pem?0:cert_pem.Length()+1;
			if(cert.SetCert(cert_pem,cert_len,store_PEM_BUF))
			{
				if (0 != cert.CheckExpirationTime())
				{
					dstream4 << "Certificate for "
						<< server_name.m_str << " expired\n";
					return get_info_res::auto_deny; // deny connect server with expired certificate
				}
				std::string srv_ver_buf;
				if(!cert.GetExtension(SERVER_VERSION_EXTENSIONS,srv_ver_buf))
					vcs_ver = 0;
				else
					vcs_ver = atou_s(srv_ver_buf.c_str());
				if(cert.GetCertPublicKey(&pkey))
				{
					char *buf(0);
					uint32_t sz(0);
					pkey.GetPublicKey(store_PEM_BUF,buf,&sz);
					if(sz>0)
					{
						buf = new char[sz];
						if(pkey.GetPublicKey(store_PEM_BUF,buf,&sz))
						{
							pub_key = buf;
							delete [] buf;
							return get_info_res::ok;
						}
						delete [] buf;
					}
				}
			}
		}
		ADODB::ParametersPtr p=broker_get_public_key_by_server_name->Parameters;
		if (IsPostgreSQL)
		{
			p->Item["_server_name"]->Value = server_name.m_str;
		}
		else
		{
			p->Item[DB_SERVER_NAME_PARAM]->Value = server_name.m_str;
		}

		ADODB::_RecordsetPtr rs=broker_get_public_key_by_server_name->Execute(NULL,NULL,0);
		if (rs != 0 && rs->State != ADODB::adStateClosed && !rs->ADOEOF)
		{
			pub_key = vs::StrFromVariantT(rs->Fields->Item["public_key"]->Value);
		}

		if (pub_key.Length() == 0)
		{
			dprint3("Public key not found for \"%s\".", server_name.m_str);
		}
		if (rs != NULL)
		{
			rs->Close();
		}
		return pub_key.Length()>0
			? get_info_res::ok
			: get_info_res::key_is_absent;
	}
	catch(_com_error e)
	{
		ProcessCOMError(e);
		return get_info_res::db_error;
	}
}
VS_ServCertInfoInterface::get_info_res
VS_RegDBStorage::GetServerCertificate(
	const VS_SimpleStr &server_name,
	VS_SimpleStr &cert)
{
	try
	{
		dprint3("$GetServerCertificate server = %s\n", server_name.m_str);
		ADODB::ParametersPtr p = broker_get_certificate->Parameters;

		if (IsPostgreSQL)
		{
			p->Item["_server_name"]->Value = server_name.m_str;
		}
		else
		{
			p->Item["@c_server_name"]->Value = server_name.m_str;
		}

		ADODB::_RecordsetPtr rs = broker_get_certificate->Execute(NULL,NULL, ADODB::adCmdStoredProc);
		if(rs!=0 && rs->State!=ADODB::adStateClosed && !rs->ADOEOF)
			cert = vs::StrFromVariantT(rs->Fields->Item["certificate"]->Value);
		else
			dprint3("cert not found\n");
		if (rs != NULL)
		{
			rs->Close();
		}
		return cert.Length()>0
			? get_info_res::ok
			: get_info_res::key_is_absent;
	}
	catch(_com_error e)
	{
		ProcessCOMError(e);
		return get_info_res::db_error;
	}
}

bool VS_RegDBStorage::SetServerCertificate(const VS_SimpleStr &server_name, const VS_SimpleStr &cert)
{
	VS_AutoLock lock(this);
	try
	{
		dprint3("$SetServerCertificate server = %s\n", server_name.m_str);
		ADODB::ParametersPtr p = broker_set_certificate->Parameters;
		if (IsPostgreSQL)
		{
			p->Item["_server_name"]->Value = server_name.m_str;
			p->Item["_certificate"]->Value = cert.m_str;
		}
		else
		{
			p->Item["@c_server_name"]->Value = server_name.m_str;
			p->Item["@c_certificate"]->Value = cert.m_str;
		}
		broker_set_certificate->Execute(NULL,NULL,ADODB::adExecuteNoRecords);
		return true;
	}
	catch(_com_error e)
	{
		ProcessCOMError(e);
		return false;
	};
}

bool VS_RegDBStorage::GetMaxLicTime(const VS_SimpleStr &server_name, VS_FileTime &notAfter)
{
	try
	{
		dprint3("$GetMaxLicTime server_name = %s\n", server_name.m_str);
		ADODB::ParametersPtr p = broker_get_max_license->Parameters;
		if (IsPostgreSQL)
		{
			p->Item["_server_name"]->Value = server_name.m_str;
		}
		else
		{
			p->Item["@c_server_name"]->Value = server_name.m_str;
		}
		ADODB::_RecordsetPtr rs = broker_get_max_license->Execute(NULL,NULL,0);
		if (rs != 0 && rs->State != ADODB::adStateClosed && !rs->ADOEOF)
		{
			if (IsPostgreSQL)
			{
				notAfter = rs->Fields->Item["valid_until"]->Value;
			}
			else
			{
				notAfter = rs->Fields->Item["dt"]->Value;
			}
		}
		if (rs != NULL)
		{
			rs->Close();
		}
		bool res = !!notAfter;
		char buf[0xff] = {0};
		notAfter.ToStr(buf,0xffff,false);

		dprint3("$GetMaxLicTime return %d. notAfter = %s server_name = %s\n", res, buf, server_name.m_str);
		return !!notAfter;
	}
	catch(_com_error e)
	{
		ProcessCOMError(e);
		return false;
	};
}

bool VS_RegDBStorage::SetState(const VS_SimpleStr& server_name,BrokerStates state)
{

	dprint3("$SetState broker=%s,state=%d\n", server_name.m_str, state);

	VS_Map::Iterator i=m_brokers.Find(server_name);
	if(state==BS_DISCONNECTED)
	{
		if(i!=m_brokers.End())  // found broker->Delete
		{
			m_brokers.Erase(i);
		}
	}
	else
	{
		if(i!=m_brokers.End())  // found broker->Update
		{
			(*i).data=(void*)state; //*(int*)(*i).data=state;
		}
		else //broker not found -> Insert
		{
			m_brokers.Insert(server_name,(void*)state);//m_brokers.Insert(server_name,(VS_Int32)state);
		};
	};

		try
		{
			ADODB::ParametersPtr p=broker_set_state->Parameters;

			if (IsPostgreSQL)
			{
				p->Item["_server_name"]->Value = server_name.m_str;
				p->Item["_state"]->Value = state;
			}
			else
			{
				p->Item[DB_SERVER_NAME_PARAM]->Value = server_name.m_str;
				p->Item[DB_STATE_PARAM]->Value = state;
			}

			broker_set_state->Execute(NULL,NULL,ADODB::adExecuteNoRecords);
		}
		catch(_com_error e)
		{
			ProcessCOMError(e);
			return false;
		};


	return true;
};

VS_RegStorage::BrokerStates VS_RegDBStorage::GetState(const VS_SimpleStr& server_name)
{
	BrokerStates state=BS_DISCONNECTED;
	VS_Map::ConstIterator i=m_brokers.Find(server_name);
	if(i!=m_brokers.End()) //call_id info not found - return zero info
		state=(BrokerStates)(int)(*i).data;///state=(BrokerStates)*(int*)(*i).data;

	dprint3("$GetBrokerState server='%s', state=%d\n", server_name.m_str, state);
	return state;
};

bool VS_RegDBStorage::GetLicenses(const VS_SimpleStr& server, std::map<uint64_t, VS_License>& licenses, bool by_server_name)
{
	BrokerStates state=GetState(server);
	dprint3("$GetLicenses of %s, state:%d\n", server.m_str, state);

	VS_SimpleStr	md5_hw;
	char hash[64] = {0};

	int count=0;

	try
	{
		if (IsPostgreSQL)
		{
			if (by_server_name)
			{
				broker_get_licenses->Parameters->Item["_server_name"]->Value = server.m_str;
				broker_get_licenses->Parameters->Item["_server_id"]->Value = db_null;
			}
			else
			{
				broker_get_licenses->Parameters->Item["_server_name"]->Value = db_null;
				broker_get_licenses->Parameters->Item["_server_id"]->Value = server.m_str;
			}
			broker_get_licenses->Parameters->Item["_kind"]->Value = "VLD";
		}
		else
		{
			broker_get_licenses->Parameters->Item[by_server_name ? DB_SERVER_NAME_PARAM : DB_BROKER_ID_PARAM]->Value = server.m_str;
			broker_get_licenses->Parameters->Item[by_server_name ? DB_BROKER_ID_PARAM : DB_SERVER_NAME_PARAM]->Value = db_null;
			broker_get_licenses->Parameters->Item["@kind"]->Value = "VLD";
		}

		ADODB::_RecordsetPtr rs = broker_get_licenses->Execute(NULL, NULL, 0);

		if (rs != 0 && rs->State != ADODB::adStateClosed)
		{
			ADODB::FieldsPtr f = rs->Fields;
			VS_License lic;

			while (!rs->ADOEOF)
			{
				// It seems that the result parameter names has not changed in PostgreSQL.
				lic.m_error = 0;
				lic.m_serverName = server;

				lic.m_id = (int)f->Item[L"license_id"]->Value;
				lic.m_id |= 0x01000000000000;
				lic.m_onlineusers = f->Item[L"online_users"]->Value;
				lic.m_conferences = f->Item[L"conferences"]->Value;
				VS_FileTime ft_validuntil; ft_validuntil.FromVariant_NoTZ(f->Item[L"valid_until"]->Value);
				lic.m_validuntil = ft_validuntil.chrono_system_clock_time_point();
				lic.m_restrict = f->Item[L"restrict"]->Value;
				VS_FileTime ft_validafter;  ft_validafter.FromVariant_NoTZ(f->Item[L"valid_after"]->Value);
				lic.m_validafter = ft_validafter.chrono_system_clock_time_point();
				lic.m_gateways = f->Item[L"gateways"]->Value;
				lic.m_terminal_pro_users		=	f->Item[L"mobile_users"]->Value;
				lic.m_symmetric_participants = f->Item[L"symmetric_participants"]->Value;
				lic.m_max_guests = f->Item[L"max_guests"]->Value;
				lic.m_trial_conf_minutes = f->Item[L"trial_conf_minutes"]->Value;
				VS_SimpleStr key = vs::StrFromVariantT(f->Item[L"key"]->Value);
				if (!!key)
					VS_ConvertToMD5(SimpleStrToStringView(key), hash);
				lic.m_hw_md5 = hash;

				licenses.emplace(lic.m_id, std::move(lic));

				rs->MoveNext();
				count++;
			}
		};

		if (rs != NULL)
		{
			rs->Close();
		}

		if (count == 0)
		{
			dprint3("THERE ARE NO LICENSES FOUND FOR %s!\n", (const char *)server);
		}

	}
	catch(_com_error e)
	{
		ProcessCOMError(e);
		return false;
	};
  dprint3(" found:%d\n",count);


	return true;
}

bool VS_RegDBStorage::GetArmHwKey(const VS_SimpleStr &server_name, unsigned long &arm_hw_key, VS_SimpleStr &arm_key, unsigned long &hw_lock)
{
	ADODB::_RecordsetPtr rs(0);
	dprint3("$GetArmHwKey sn = %s\n", server_name.m_str);
	try
	{
		ADODB::ParametersPtr p=broker_get_arm_hw_key->Parameters;
		if (IsPostgreSQL)
		{
			p->Item["_server_name"]->Value = server_name.m_str;
		}
		else
		{
			p->Item["@c_server_name"]->Value = server_name.m_str;
		}
		rs=broker_get_arm_hw_key->Execute(NULL,NULL,0);
		if (rs != 0 && rs->State != ADODB::adStateClosed && !rs->ADOEOF)
		{
			const _variant_t value = rs->Fields->Item["arm_hw_key"]->Value;
			arm_hw_key = value.vt == VT_NULL ? 0 : static_cast<long>(value);
			arm_key = vs::StrFromVariantT(rs->Fields->Item["arm_key"]->Value);
			hw_lock = rs->Fields->Item["hw_locked"]->Value;
			dprint3("return arm_hw_key =%ld, arm_key=%s\n", arm_hw_key, arm_key.m_str);
		}
		else
		{
			dprint3("Failed to obtain arm_key for \"%s\"", server_name.m_str);
		}
		if (rs != NULL)
		{
			rs->Close();
		}
		return arm_key!=0;
	}
	catch(_com_error e)
	{
		if (rs != NULL)
		{
			rs->Close();
		}
		ProcessCOMError(e);
		return false;
	}
}

bool VS_RegDBStorage::SetArmHwKey(const VS_SimpleStr &server_name, const unsigned long arm_hw_key, const VS_SimpleStr &arm_key)
{
	dprint3("$SetArmHwKey sn = %s\n", server_name.m_str);
	try
	{
		ADODB::ParametersPtr p=broker_set_arm_hw_key->Parameters;

		if (IsPostgreSQL)
		{
			p->Item["_server_name"]->Value = server_name.m_str;
			p->Item["_arm_hw_key"]->Value = arm_hw_key;
			p->Item["_arm_key"]->Value = arm_key.m_str;
		}
		else
		{
			p->Item["@c_server_name"]->Value = server_name.m_str;
			p->Item["@i_arm_hw_key"]->Value = arm_hw_key;
			p->Item["@c_arm_key"]->Value = arm_key.m_str;
		}

		broker_set_arm_hw_key->Execute(NULL,NULL,ADODB::adExecuteNoRecords);
	}
	catch(_com_error e)
	{
		ProcessCOMError(e);
		return false;
	};
	return true;
}

bool VS_RegDBStorage::GetConfigCorrectorData(const VS_SimpleStr& server_name, VS_SimpleStr &result)
{
	dprint3("$GetConfigCorrectorData sn = %s\n", server_name.m_str);
	try {
		ADODB::_RecordsetPtr rs(0);

		rs = get_call_cfg_corrector->Execute(NULL, NULL, 0);
		if (rs != 0 && rs->State != ADODB::adStateClosed && !rs->ADOEOF)
		{
			if (IsPostgreSQL)
			{
				result = vs::StrFromVariantT(rs->Fields->Item["body"]->Value);
			}
			else
			{
				result = vs::StrFromVariantT(rs->Fields->Item["value"]->Value);
			}
			dprint3("Got data for CallConfigCorrector.\n");
		}
		if (rs != NULL)
		{
			rs->Close();
		}
	}
	catch (_com_error e)
	{
		ProcessCOMError(e);
		return false;
	}

	return true;
}
