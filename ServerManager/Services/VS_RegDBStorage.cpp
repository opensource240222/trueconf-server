#ifdef _WIN32	// not ported
#include <algorithm>

#include "VS_RegDBStorage.h"

#include "../../common/std/cpplib/VS_RegistryKey.h"
#include "../../common/std/cpplib/VS_RegistryConst.h"
#include "../../common/std/cpplib/VS_SimpleStr.h"
#include "std/cpplib/StrFromVariantT.h"
#include "../../common/std/debuglog/VS_Debug.h"
#include "../../BaseServer/Services/storage/VS_DBStorage.h"
#include "../../common/std/cpplib/VS_FileTime.h"
#include "tlb_import/msado26.tli"

#define DEBUG_CURRENT_MODULE VS_DM_REGDBST

VS_RegDBStorage::VS_RegDBStorage(): m_IsInit(false), IsPostgreSQL(false)
{

}

VS_RegDBStorage::~VS_RegDBStorage()
{

}

bool VS_RegDBStorage::Init()
{
	if (!!db)
		db.Release();
	if (!!log_stats)
		log_stats.Release();
	if (!!app_prop_get_all_props)
		app_prop_get_all_props.Release();

	char buff[512];		memset(buff, 0, 512);

	VS_RegistryKey cfg_root(false, CONFIGURATION_KEY, false, true);
	if ( !cfg_root.IsValid() )
		return false;

	VS_SimpleStr conn;
	VS_SimpleStr username;
	VS_SimpleStr password;
	VS_SimpleStr schema;

	std::string schema_prefix;

	if ( cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, DB_CONNECTIONSTRING_TAG ) > 0 )
	{
		conn = buff;
		memset(buff, 0, 512);
	}
	if ( cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, DB_PASSWORD_TAG) > 0 )
	{
		password = buff;
		memset(buff, 0, 512);
	}
	if ( cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, DB_USER_TAG) > 0 )
	{
		username = buff;
		memset(buff, 0, 512);
	}
	if (cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, DB_SCHEMA_TAG) > 0)
	{
		schema = buff;
	}

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

	fflush(stdout);

	// set default "DB Schema" value for PostgreSQL
	if (IsPostgreSQL && schema.Length() == 0)
	{
		schema = "public";
	}

	if (schema.Length() > 0)
	{
		schema_prefix = schema;
		schema_prefix.append(".");
	}

	try
	{
		db.CreateInstance("ADODB.Connection");
	}
	catch(_com_error e)
	{
		ProcessCOMError(e);
		return false;
	}

	bool connected = false;
	for(int i=0;i<reconnect_max;i++)
	{
		try
		{
			db->Open((char *)conn,(char *)username,(char *)password,ADODB::adConnectUnspecified);
			connected=true;
			break;
		}
		catch(_com_error e)
		{
			dprint0("\t connect to DB failed. Reconnect %d\n", i+1);
			ProcessCOMError(e);
			fflush(stdout);
			SleepEx(reconnect_timeout,false);
		}
	};
	if ( !connected )
	{
		dprint0("\t connect to DB failed. No average-statistics will be available.\n");
		return false;
	}

	try
	{
		log_stats.CreateInstance("ADODB.Command");
		log_stats->ActiveConnection=db;
		log_stats->CommandText=(schema_prefix + "log_stats").c_str();
		log_stats->CommandType=ADODB::adCmdStoredProc;
		log_stats->Parameters->Refresh();

		app_prop_get_all_props.CreateInstance("ADODB.Command");
		app_prop_get_all_props->ActiveConnection=db;
		app_prop_get_all_props->CommandType=ADODB::adCmdStoredProc;
		if (!IsPostgreSQL)
		{
			app_prop_get_all_props->CommandText = (schema_prefix + "sp_get_apps").c_str();
			app_prop_get_all_props->Parameters->Refresh();
		}
		else
		{
			std::string cmd = "select * from ";
			cmd += (schema_prefix + "apps_get()");
			app_prop_get_all_props->CommandText = cmd.c_str();
			app_prop_get_all_props->CommandType = ADODB::adCmdText;
		}
	}
	catch(_com_error e)
	{
		ProcessCOMError(e);
		return false;
	}

	m_IsInit = true;
	return true;
}

bool VS_RegDBStorage::LogStats(const char* sid, const VS_MediaBrokerStats* stats)
{
	if( !m_IsInit || !stats )
		return false;

	try
	{
		dprint3("$LogStats for %s\n", sid);

		ADODB::ParametersPtr p = log_stats->Parameters;

		p->Item["@sid"]->Value				= sid;
		p->Item["@broker_time"]->Value		= VS_FileTime(stats->m_currTime).ToVariant_WithTZ();
		p->Item["@period"]->Value			= stats->m_periodOfAveraging;

		p->Item["@avg_confs"]->Value		= stats->m_AvConfs;
		p->Item["@avg_eps"]->Value			= stats->m_AvEndpts;
		p->Item["@avg_users"]->Value		= stats->m_AvUsers;
		p->Item["@avg_parts"]->Value		= stats->m_AvParts;

		p->Item["@avg_load"]->Value			= stats->m_AvPrLoad;
		p->Item["@avg_net_load"]->Value		= stats->m_AvNetLoad;

		p->Item["@total_confs"]->Value		= stats->m_TotalConfs;
		p->Item["@total_eps"]->Value		= stats->m_TotalEndpoints;
		p->Item["@total_users"]->Value		= stats->m_TotalUsers;

		log_stats->Execute(NULL,NULL,ADODB::adExecuteNoRecords);
	}
	catch(_com_error e)
	{
		ProcessCOMError(e);
		return false;
	};
	return true;
}

void VS_RegDBStorage::ProcessCOMError(_com_error e)
{
	_bstr_t bstrSource(e.Source());
	_bstr_t bstrDescription(e.Description());

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
		dprint1(" DB State: %02lx \n", db?db->State:-1);
	}
	else
	{
		ADODB::ErrorPtr ae= db->Errors->GetItem((short)0);
		if ((!!ae->SQLState && _strnicmp(ae->SQLState,"08",2)==0) ||		// SQLState error: Class 08 - Connection Exception (http://www.postgresql.org/docs/current/static/errcodes-appendix.html)
			(ae->Number==0x80004005 && (ae->NativeError==0x0 || ae->NativeError==0x11 || (IsPostgreSQL && (ae->NativeError==0x1a)))) )
		{
			dprint0("#DB connection error, trying to reconnect\n");
			dstream4 << "SQLState=" << ae->SQLState << ", Number=" << ae->Number << ", NativeError=" << ae->NativeError;
			Init();
		}
		else
		{
			dprint1("#ADO Error [%08lx] native(%08lx) (total:%d)\n",ae->Number,ae->NativeError,count);
			dprint1(" Source = %s \n", (char*)(_bstr_t)ae->Source);
			dprint1(" Description = %s \n", (char*)(_bstr_t)ae->Description);
			dprint1(" DB State: %02lx \n", db?db->State:-1);
		};
	};

	fflush(stdout);
}

int VS_RegDBStorage::GetAllAppProperties(VS_AppPropertiesMap &prop_map)
{
	int count(0);
	try
	{
		dprint4("$GetAllAppProperties\n");

		ADODB::_RecordsetPtr rs = app_prop_get_all_props->Execute(NULL,NULL,ADODB::adCmdUnspecified);
		if (rs == NULL)
			return 0;
		for(count =0;!rs->ADOEOF;rs->MoveNext() , count++)
		{
			VS_Container	*prop = new VS_Container;
			ADODB::FieldsPtr f= rs->Fields;
			ADODB::FieldPtr p;
			int n = f->Count;

			_bstr_t		name;
			_variant_t	val;

			VS_SimpleStr key_name = vs::StrFromVariantT(f->Item[APP_NAME]->Value);
			for(short i = 0;i<n;i++)
			{
				p = f->GetItem(i);
				name = p->Name;
				val = p->Value;
				if(val.vt!=VT_EMPTY && val.vt !=VT_NULL)
				{
					dstream4 << "\tadding property " << (char*)name << "='" << (wchar_t*)(_bstr_t)val << "'\n";
					prop->AddValue((char*)name,(wchar_t*)(_bstr_t)val);
				}

			}
			prop_map[key_name] = prop;
			prop = 0;
		}

		rs->Close();
	}
	catch(_com_error e)
	{
		ProcessCOMError(e);
		count=0;
	}
	return count;
}
#endif