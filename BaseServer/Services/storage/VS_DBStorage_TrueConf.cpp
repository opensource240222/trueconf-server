#ifdef _WIN32 // not ported
#include "VS_DBStorage_TrueConf.h"
#include "../../../common/std/VS_ProfileTools.h"
#include "../../../common/std/cpplib/VS_Replace.h"
#include "std/cpplib/StrFromVariantT.h"

#define DEBUG_CURRENT_MODULE VS_DM_DBSTOR

VS_DBStorage_TrueConf::VS_DBStorage_TrueConf()
{

}

VS_DBStorage_TrueConf::~VS_DBStorage_TrueConf()
{

}

bool VS_DBStorage_TrueConf::IsConferendoBS() const
{
	return false;
}

void VS_DBStorage_TrueConf::GetSipProviderByCallId(const char* call_id, std::vector<VS_ExternalAccount>& external_accounts, VS_DBObjects* dbo)
{
AUTO_PROF

	dprint3("$GetSipProviderByCallId(%s)\n", call_id);
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo2 = 0;
	VS_DBObjects* dbo_ptr = dbo;
	if (!dbo)
	{
		dbo_ptr = dbo2 = GetDBO(dboitem);
	}

	try
	{
		int num_providers(0);
		// extract external external accounts
		std::string cmd = std::string() +
			"select * from vs_sip_get_provider_by_call_id( '" + call_id + "' )";
		dbo_ptr->user_get_external_accounts->CommandText = cmd.c_str();
		ADODB::_RecordsetPtr rs=dbo_ptr->user_get_external_accounts->Execute(0,0,ADODB::adCmdText);
		if(rs!=0 && rs->State!=ADODB::adStateClosed )
		{
			for(;!rs->ADOEOF;rs->MoveNext())
			{
				VS_ExternalAccount a;
				for (int i = 0; i < rs->Fields->Count; i++)
				{
					ADODB::FieldPtr fld=  rs->Fields->Item[ (short)i ];
					_bstr_t name = fld->Name;
					const char * cname = (const char*)name;
					ADODB::DataTypeEnum ftype = fld->Type;
					_variant_t val(fld->Value);

					if (_stricmp(cname, "login") == 0)	a.m_login		= (char*)(_bstr_t)fld->Value; else
					if (_stricmp(cname, "passwd") == 0)	a.m_password	= (char*)(_bstr_t)fld->Value; else
					if (_stricmp(cname, "ip") == 0)		a.m_host		= (char*)(_bstr_t)fld->Value; else
					if (_stricmp(cname, "port") == 0)		a.m_port			= (unsigned short)val; else
					if (_stricmp(cname, "reg_mode") == 0)
						a.m_registrationBehavior = (VS_ExternalAccount::RegistrationBehavior)(long)val; else
					if (_stricmp(cname, "isVoIPServer") == 0) a.m_isVoIPServer = (bool)val; else
					if (_stricmp(cname, "TelephonePrefixReplace") == 0) a.m_TelephonePrefixReplace = (char*)(_bstr_t)fld->Value;
				}
				a.m_protocol = "#tel";
				num_providers++;
				dprint4("%d) %s\n", num_providers, a.Serealize().c_str());
				external_accounts.push_back( a );
			}
			if (!num_providers)
				dprint4("\tno sip providers for %s\n", call_id);
		}

	}
	catch(_com_error err)
	{
	}

	if (dbo2)
		ReleaseDBO(dboitem);
}

int VS_DBStorage_TrueConf::FindUsers(VS_Container& cnt, int& entries, VS_AddressBook ab, const vs_user_id& owner, const std::string& query, long client_hash, VS_Container& in_cnt)
{
	if (ab == AB_GROUPS)
		return FindUsersGroups(owner, in_cnt, entries, cnt);
	else if (ab == AB_PHONES)
		return FindUsersPhones(owner, in_cnt, entries, cnt);
	return VS_DBStorage::FindUsers(cnt,entries,ab,owner,query,client_hash,in_cnt);
}

int VS_DBStorage_TrueConf::AddToAddressBook(VS_AddressBook ab,const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt, VS_SimpleStr& add_call_id, std::string& add_display_name, VS_TransportRouterServiceHelper* srv)
{
	if (ab==AB_PHONES)
		return AddToAddressBook_Phones(ab,user_id1,cnt,hash,rCnt);
	return VS_DBStorage::AddToAddressBook(ab,user_id1,cnt,hash,rCnt,add_call_id,add_display_name,srv);
}

int VS_DBStorage_TrueConf::UpdateAddressBook(VS_AddressBook ab,const vs_user_id& user_id1,const char* call_id2, VS_Container& cnt, long& hash, VS_Container& rCnt)
{
	if (ab==AB_PHONES)
		return UpdateAddressBook_Phones(ab,user_id1,cnt,hash,rCnt);
	return VS_DBStorage::UpdateAddressBook(ab,user_id1,call_id2,cnt,hash,rCnt);
}

int VS_DBStorage_TrueConf::RemoveFromAddressBook(VS_AddressBook ab, const vs_user_id& user_id1,const vs_user_id& user_id2, VS_Container& cnt, long& hash, VS_Container& rCnt)
{
	if (ab==AB_PHONES)
		return RemoveFromAddressBook_Phones(ab,user_id1,cnt,hash,rCnt);
	return VS_DBStorage::RemoveFromAddressBook(ab,user_id1,user_id2,cnt,hash,rCnt);
}

int VS_DBStorage_TrueConf::FindUsersGroups(const vs_user_id& owner, VS_Container& cnt, int& entries, VS_Container& rCnt)
{
AUTO_PROF
	struct TTmp
	{
		std::wstring gname;
		long type;
		std::vector<std::string> users;

		TTmp(): type(0)
		{	}
	};
	typedef std::map<long, TTmp> TYPE_UsersGroups;
	TYPE_UsersGroups m;
	entries = 0;

	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dprint2("$FindUsersGroups for %s\n", owner.m_str);

		std::string cmd(dbo->ab_group_get_list_users_template);
		VS_ReplaceAll(cmd, "%call_id%", (const char*)owner);
		dbo->ab_group_get_list_users->CommandText = cmd.c_str();

		ADODB::_RecordsetPtr rs = dbo->ab_group_get_list_users->Execute(0,0,ADODB::adCmdText);

		int32_t server_hash(0);
		if (!rs->ADOEOF)
		{
			int32_t client_hash(0);			cnt.GetValue(HASH_PARAM, client_hash);
			server_hash = rs->Fields->Item[HASH_PARAM]->Value;
			rCnt.AddValueI32(HASH_PARAM, server_hash);
			dprint4("compare hash: client<%d> server<%d>\n", client_hash, server_hash);
			if (VS_CompareHash(server_hash, client_hash))
				throw SEARCH_NOT_MODIFIED;
		}

		if (server_hash==0 && dbo->IsPostgreSQL) {		// Set HASH = 2000 year
			rCnt.AddValueI32(HASH_PARAM, 1);
			entries=0;
			throw SEARCH_DONE;
		}

		while (!rs->ADOEOF)
		{
			long ab_gr_id = rs->Fields->Item["ab_gr_id"]->Value;
			VS_WideStr gname = (wchar_t*)(_bstr_t) rs->Fields->Item["name"]->Value;
			VS_SimpleStr str_type = (char*)(_bstr_t) rs->Fields->Item[TYPE_PARAM]->Value;
			long type = 0;
			if (str_type == "USR")
				type = 1;
			else if ((str_type == "SYS") || (str_type == "SYSCG"))
				type = 0;

			VS_SimpleStr user;
			_variant_t var = rs->Fields->Item["call_id2"]->Value;
			if(var.vt!=VT_EMPTY && var.vt!=VT_NULL)
			{
				user = vs::StrFromVariantT(rs->Fields->Item["call_id2"]->Value);
			}
			//VS_SimpleStr dn = (wchar_t*)(_bstr_t) rs->Fields->Item["display_name"]->Value;

			TYPE_UsersGroups::iterator it = m.find(ab_gr_id);
			if (it==m.end()) {
				TTmp tmp;
				tmp.gname = gname;
				tmp.type = type;
				if (!!user)
					tmp.users.emplace_back(user.m_str);
				m.emplace(ab_gr_id, std::move(tmp));
			}else{
				if (!!user)
					it->second.users.emplace_back(user.m_str);
			}

			rs->MoveNext();
		}
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->ab_group_get_list_users->GetCommandText());
		dbo->ProcessCOMError(e,dbo->ab_group_get_list_users->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return SEARCH_FAILED;
	}
	catch (VS_Search_Result r)
	{
		ReleaseDBO(dboitem);
		return r;
	}

	for (TYPE_UsersGroups::iterator it=m.begin(); it!=m.end(); it++)
	{
		TTmp* p = &(it->second);
		dstream3 << "group(" << (long)it->first << ", " << p->gname.c_str() << ")\n";

		// send GID as string
		char buff[256] = {0};
		_ltoa((long)it->first, buff, 10);

		rCnt.AddValue(GID_PARAM, (const char*)buff);
		rCnt.AddValue(GNAME_PARAM, p->gname.c_str());
		rCnt.AddValueI32(GTYPE_PARAM, p->type); // [USR, SYS, SYSCG]

		for (std::vector<std::string>::iterator it2=p->users.begin(); it2!=p->users.end(); it2++)
		{
			dprint3("users(%s)\n", it2->c_str());
			rCnt.AddValue(CALLID_PARAM, it2->c_str());
		}
	}

	entries = m.size();

	ReleaseDBO(dboitem);
	return SEARCH_DONE;
}

int VS_DBStorage_TrueConf::FindUsersPhones(const vs_user_id& owner, VS_Container& cnt, int& entries, VS_Container& rCnt)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	VS_Search_Result error(SEARCH_DONE);
	try
	{
		dprint2("$FindUsersPhones for %s\n", owner.m_str);

		std::string cmd(dbo->ab_phones_find_template);
		VS_ReplaceAll(cmd, "%call_id%", (const char*)owner);
		dbo->ab_phones_find->CommandText = cmd.c_str();

		ADODB::_RecordsetPtr rs = dbo->ab_phones_find->Execute(0,0,ADODB::adCmdText);

		int32_t server_hash(0);
		if (!rs->ADOEOF)
		{
			int32_t client_hash(0);			cnt.GetValue(HASH_PARAM, client_hash);
			server_hash = rs->Fields->Item[HASH_PARAM]->Value;
			rCnt.AddValueI32(HASH_PARAM, server_hash);
			dprint4("compare hash: client<%d> server<%d>\n", client_hash, server_hash);
			if (VS_CompareHash(server_hash, client_hash))
				throw true;
		}

		if (server_hash==0 && dbo->IsPostgreSQL) {		// Set HASH = 2000 year
			rCnt.AddValueI32(HASH_PARAM, 1);
			entries=0;
			throw true;
		}

		while (!rs->ADOEOF)
		{
			long dn_id = rs->Fields->Item["dn_id"]->Value;
			char buff[256] = {0};
			_ltoa_s(dn_id, buff, sizeof(buff), 10);
			VS_SimpleStr call_id = (char*)(_bstr_t) rs->Fields->Item["call_id1"]->Value;		// to whom
			VS_SimpleStr call_id2;																// who set this phone (if nobody, then it is phone from profile)
			_variant_t val(rs->Fields->Item["call_id2"]->Value);
			if(val.vt!=VT_NULL && val.vt!=VT_EMPTY)
				call_id2 = (char*)(_bstr_t) val;
			VS_WideStr phone = (wchar_t*)(_bstr_t) rs->Fields->Item[USERPHONE_PARAM]->Value;
			VS_SimpleStr str_type = (char*)(_bstr_t) rs->Fields->Item[TYPE_PARAM]->Value;
			VS_UserPhoneType type = VS_UserPhoneType_ToEnum(str_type);

			entries++;
			rCnt.AddValue(ID_PARAM, buff);
			rCnt.AddValue(CALLID_PARAM, call_id);
			rCnt.AddValue(USERPHONE_PARAM, phone);
			rCnt.AddValueI32(TYPE_PARAM, type);
			rCnt.AddValue(EDITABLE_PARAM, (!!call_id2)?true:false);

			rs->MoveNext();
		}
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->ab_phones_find->GetCommandText());
		dbo->ProcessCOMError(e,dbo->ab_phones_find->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return false;
	}
	catch (bool /*hash_equal*/)
	{ error = SEARCH_NOT_MODIFIED; }

	ReleaseDBO(dboitem);
	return error;
}


int VS_DBStorage_TrueConf::AddToAddressBook_Phones(VS_AddressBook ab,const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt)
{
	if (ab!=AB_PHONES)
		return VSS_DB_ERROR;

AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	int error=-1;
	int	error_code_out=-1;

	hash=0;
	ADODB::_CommandPtr cmd;

	dprint2("$AddToAddressBook_Phones %s\n", (const char*)user_id1);

	const char* call_id2=cnt.GetStrValueRef(CALLID_PARAM);
	if(!call_id2||!*call_id2)
		return VSS_USER_NOT_VALID;

	const char* user_phone =cnt.GetStrValueRef(USERPHONE_PARAM);
	if(!user_phone||!*user_phone)
		return VSS_USER_NOT_FOUND;

	int32_t type;
	if (!cnt.GetValue(TYPE_PARAM, type))
		return VSS_USER_NOT_FOUND;
	const char* db_type(VS_UserPhoneType_ToStr((VS_UserPhoneType)(type)));

	cmd=dbo->ab_phones_add;

	try
	{
		ADODB::ParametersPtr p=cmd->Parameters;
		p->Item["@c_call_id1"]->Value				= call_id2;
		p->Item["@c_call_id2"]->Value				= user_id1.m_str;
		p->Item["@c_phone"]->Value					= user_phone;
		p->Item["@c_type"]->Value					= db_type;

		ADODB::_RecordsetPtr rs=cmd->Execute(NULL,NULL,ADODB::adCmdUnspecified);

		if(rs!=0 && rs->State!=ADODB::adStateClosed && !rs->ADOEOF)
		{
			_variant_t val(rs->Fields->Item[HASH_PARAM]->Value);
			if(val.vt!=VT_NULL && val.vt!=VT_EMPTY)
				hash=val;

			long dn_id = rs->Fields->Item["dn_id"]->Value;
			char buff[256] = {0};
			_ltoa_s(dn_id, buff, sizeof(buff), 10);
			rCnt.AddValue(ID_PARAM, buff);
			rCnt.AddValue(TYPE_PARAM, type);
			rCnt.AddValue(USERPHONE_PARAM, user_phone);
			rCnt.AddValue(CALLID_PARAM, call_id2);
			rCnt.AddValue(EDITABLE_PARAM, true);

			error = (hash!=0)? 0: VSS_USER_NOT_FOUND;
			dprint3("$AddToAddressBook_Phones\ncall_id1=%s\ncall_id2=%s\nphone=%s\ntype=%s\nhash=%ld\n", (const char*)user_id1, call_id2, user_phone, db_type, hash);
		}
	}
	catch(_com_error e)
	{
		ADODB::_ConnectionPtr cur_db = cmd->ActiveConnection;
		if(cur_db==0)
		{
			_bstr_t	add_str(cmd->GetCommandText());
			dbo->ProcessCOMError(e,cur_db,(VS_DBStorage*)this,cmd->CommandText);
		}
		if(cur_db->Errors->Count==0)
		{
			error=VSS_USER_NOT_VALID;
			dprint2("$AddToAddressBook_Phones:  call_id not valid %s\n",(PCSTR)(_bstr_t)e.Description());
		}
		else
		{
			ADODB::ErrorPtr ae= cur_db->Errors->GetItem((short)0);
			if(ae->Number==0x80040e2f && ae->NativeError==0x00000a43 )
			{
				dprint3(" user already exists in address book(%s)\n",(PCSTR)(_bstr_t)e.Description());
				error=VSS_USER_EXISTS;
			}
			else if(ae->Number==0x80040e2f && ae->NativeError==0x00000203 )
			{
				dprint3(" user not found in address book(%s)\n",(PCSTR)(_bstr_t)e.Description());
				error=VSS_USER_NOT_FOUND;
			}
			else
			{
				error=VSS_DB_ERROR;
				dbo->ProcessCOMError(e,cur_db,(VS_DBStorage*)this,cmd->CommandText);
			}
		};
	}

	ReleaseDBO(dboitem);
	return error;
}

int VS_DBStorage_TrueConf::UpdateAddressBook_Phones(VS_AddressBook ab,const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt)
{
	if (ab!=AB_PHONES)
		return VSS_DB_ERROR;

AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	int error=-1;
	int	error_code_out=-1;

	hash=0;
	ADODB::_CommandPtr cmd;

	dprint2("$UpdateAddressBook_Phones %s\n", (const char*)user_id1);

	const char* id=cnt.GetStrValueRef(ID_PARAM);
	if(!id||!*id)
		return VSS_USER_NOT_VALID;

	const char* call_id2=cnt.GetStrValueRef(CALLID_PARAM);
	if(!call_id2||!*call_id2)
		return VSS_USER_NOT_VALID;

	const char* user_phone =cnt.GetStrValueRef(USERPHONE_PARAM);
	if(!user_phone||!*user_phone)
		return VSS_USER_NOT_FOUND;

	int32_t type;
	if (!cnt.GetValue(TYPE_PARAM, type))
		return VSS_USER_NOT_FOUND;
	const char* db_type(VS_UserPhoneType_ToStr((VS_UserPhoneType)(type)));

	cmd=dbo->ab_phones_update;
	try
	{
		ADODB::ParametersPtr p=cmd->Parameters;
		p->Item["@c_from_call_id"]->Value			= user_id1.m_str;
		p->Item["@i_dn_id"]->Value					= atol(id);
		p->Item["@c_phone"]->Value					= user_phone;
		p->Item["@c_type"]->Value					= db_type;

		ADODB::_RecordsetPtr rs=cmd->Execute(NULL,NULL,ADODB::adCmdUnspecified);

		if(rs!=0 && rs->State!=ADODB::adStateClosed && !rs->ADOEOF)
		{
			_variant_t val(rs->Fields->Item[HASH_PARAM]->Value);
			if(val.vt!=VT_NULL && val.vt!=VT_EMPTY)
				hash=val;

			rCnt.AddValue(ID_PARAM, id);
			rCnt.AddValue(TYPE_PARAM, type);
			rCnt.AddValue(USERPHONE_PARAM, user_phone);
			rCnt.AddValue(CALLID_PARAM, call_id2);
			rCnt.AddValue(EDITABLE_PARAM, true);

			error = (hash!=0)? 0: VSS_USER_NOT_FOUND;
			dprint3("$UpdateAddressBook_Phones\ncall_id1=%s\ncall_id2=%s\nphone=%s\ntype=%s\nhash=%ld\nid=%s\n", (const char*)user_id1, call_id2, user_phone, db_type, hash, id);
		}
	}
	catch(_com_error e)
	{
		ADODB::_ConnectionPtr cur_db = cmd->ActiveConnection;
		if(cur_db==0)
		{
			_bstr_t	add_str(cmd->GetCommandText());
			dbo->ProcessCOMError(e,cur_db,(VS_DBStorage*)this,cmd->CommandText);
		}
		if(cur_db->Errors->Count==0)
		{
			error=VSS_USER_NOT_VALID;
			dprint2("$UpdateAddressBook_Phones:  call_id not valid %s\n",(PCSTR)(_bstr_t)e.Description());
		}
		else
		{
			ADODB::ErrorPtr ae= cur_db->Errors->GetItem((short)0);
			if(ae->Number==0x80040e2f && ae->NativeError==0x00000a43 )
			{
				dprint3(" user already exists in address book(%s)\n",(PCSTR)(_bstr_t)e.Description());
				error=VSS_USER_EXISTS;
			}
			else if(ae->Number==0x80040e2f && ae->NativeError==0x00000203 )
			{
				dprint3(" user not found in address book(%s)\n",(PCSTR)(_bstr_t)e.Description());
				error=VSS_USER_NOT_FOUND;
			}
			else
			{
				error=VSS_DB_ERROR;
				dbo->ProcessCOMError(e,cur_db,(VS_DBStorage*)this,cmd->CommandText);
			}
		};
	}

	ReleaseDBO(dboitem);
	return error;
}

int VS_DBStorage_TrueConf::RemoveFromAddressBook_Phones(VS_AddressBook ab,const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt)
{
	if (ab!=AB_PHONES)
		return VSS_DB_ERROR;

AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	int error=-1;
	int	error_code_out=-1;

	hash=0;
	ADODB::_CommandPtr cmd;

	dprint2("$RemoveFromAddressBook_Phones %s\n", (const char*)user_id1);

	const char* id=cnt.GetStrValueRef(ID_PARAM);
	if(!id||!*id)
		return VSS_USER_NOT_VALID;

	cmd=dbo->ab_phones_delete;

	try
	{
		ADODB::ParametersPtr p=cmd->Parameters;
		p->Item["@c_from_call_id"]->Value			= user_id1.m_str;
		p->Item["@i_dn_id"]->Value					= atol(id);

		ADODB::_RecordsetPtr rs=cmd->Execute(NULL,NULL,ADODB::adCmdUnspecified);

		if(rs!=0 && rs->State!=ADODB::adStateClosed && !rs->ADOEOF)
		{
			_variant_t val(rs->Fields->Item[HASH_PARAM]->Value);
			if(val.vt!=VT_NULL && val.vt!=VT_EMPTY)
				hash=val;

			rCnt.AddValue(ID_PARAM, id);

			error = (hash!=0)? 0: VSS_USER_NOT_FOUND;
			dprint3("$RemoveFromAddressBook_Phones\ncall_id1=%s\nhash=%ld\nid=%s\n", (const char*)user_id1, hash, id);
		}
	}
	catch(_com_error e)
	{
		ADODB::_ConnectionPtr cur_db = cmd->ActiveConnection;
		if(cur_db==0)
		{
			_bstr_t	add_str(cmd->GetCommandText());
			dbo->ProcessCOMError(e,cur_db,(VS_DBStorage*)this,cmd->CommandText);
		}
		if(cur_db->Errors->Count==0)
		{
			error=VSS_USER_NOT_VALID;
			dprint2("$RemoveFromAddressBook_Phones:  call_id not valid %s\n",(PCSTR)(_bstr_t)e.Description());
		}
		else
		{
			ADODB::ErrorPtr ae= cur_db->Errors->GetItem((short)0);
			if(ae->Number==0x80040e2f && ae->NativeError==0x00000a43 )
			{
				dprint3(" user already exists in address book(%s)\n",(PCSTR)(_bstr_t)e.Description());
				error=VSS_USER_EXISTS;
			}
			else if(ae->Number==0x80040e2f && ae->NativeError==0x00000203 )
			{
				dprint3(" user not found in address book(%s)\n",(PCSTR)(_bstr_t)e.Description());
				error=VSS_USER_NOT_FOUND;
			}
			else
			{
				error=VSS_DB_ERROR;
				dbo->ProcessCOMError(e,cur_db,(VS_DBStorage*)this,cmd->CommandText);
			}
		};
	}

	ReleaseDBO(dboitem);
	return error;
}

VS_UserPhoneType VS_DBStorage_TrueConf::VS_UserPhoneType_ToEnum(const char* type)
{
	if (_stricmp("MOB",type)==0)			return USERPHONETYPE_MOBILE;
	else if (_stricmp("WORK",type)==0)		return USERPHONETYPE_WORK;
	else if (_stricmp("HOME",type)==0)		return USERPHONETYPE_HOME;
	else									return USERPHONETYPE_OTHER;
}

const char* VS_DBStorage_TrueConf::VS_UserPhoneType_ToStr(VS_UserPhoneType type)
{
	if (type == USERPHONETYPE_MOBILE)		return "MOB";
	else if (type == USERPHONETYPE_WORK)	return "WORK";
	else if (type == USERPHONETYPE_HOME)	return "HOME";
	else									return "OTHER";
}

bool VS_DBStorage_TrueConf::ManageGroups_CreateGroup(const vs_user_id& owner, const VS_WideStr& gname, long& gid, long& hash)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dstream2 << "$ManageGroups_CreateGroup for " << owner.m_str << " group " << gname.m_str << "\n";

		ADODB::ParametersPtr p=dbo->ab_group_create->Parameters;
		p->Item["@c_call_id1"]->Value = owner.m_str;
		p->Item["@c_name"]->Value = gname.m_str;
		p->Item["@c_type"]->Value = "USR";		// 'USR', 'SYS', 'SYSCG'

		/*
		std::wstring cmd(dbo->ab_group_create_template);
		VS_ReplaceAll(cmd, "%owner%", owner.m_str);
		VS_ReplaceAll(cmd, L"%gname%", gname.m_str);
		dbo->ab_group_create->CommandText = cmd.c_str();
		*/

		ADODB::_RecordsetPtr rs = dbo->ab_group_create->Execute(0,0,ADODB::adCmdUnspecified);

		if (!rs->ADOEOF)
		{
			gid = rs->Fields->Item["ab_gr_id"]->Value;
			hash = rs->Fields->Item[HASH_PARAM]->Value;
		}
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->ab_group_create->GetCommandText());
		dbo->ProcessCOMError(e,dbo->ab_group_create->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return false;
	}
	ReleaseDBO(dboitem);
	return true;
}

bool VS_DBStorage_TrueConf::ManageGroups_DeleteGroup(const vs_user_id& owner, const long gid, long& hash)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dprint2("$ManageGroups_DeleteGroup gid %ld\n", gid);

		std::string cmd(dbo->ab_group_delete_template);
		char buff[256] = {0};	_itoa(gid, buff, 10);
		VS_ReplaceAll(cmd, "%gid%", (const char*)buff);
		dbo->ab_group_delete->CommandText = cmd.c_str();

		ADODB::_RecordsetPtr rs = dbo->ab_group_delete->Execute(0,0,ADODB::adCmdText);

		if (!rs->ADOEOF)
		{
			hash = rs->Fields->Item[HASH_PARAM]->Value;
		}
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->ab_group_delete->GetCommandText());
		dbo->ProcessCOMError(e,dbo->ab_group_delete->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return false;
	}
	ReleaseDBO(dboitem);
	return true;
}

bool VS_DBStorage_TrueConf::ManageGroups_RenameGroup(const vs_user_id& owner, const long gid, const VS_WideStr& gname, long& hash)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dstream2 << "$ManageGroups_RenameGroup gid " << gid << " name " << gname.m_str << "\n";

		std::wstring cmd(dbo->ab_group_edit_template);
		wchar_t buff[256] = {0};	_itow(gid, buff, 10);
		VS_ReplaceAll(cmd, L"%gid%", buff);
		VS_ReplaceAll(cmd, L"%gname%", gname.m_str);
		dbo->ab_group_edit->CommandText = cmd.c_str();

		ADODB::_RecordsetPtr rs = dbo->ab_group_edit->Execute(0,0,ADODB::adCmdText);

		if (!rs->ADOEOF)
		{
			hash = rs->Fields->Item[HASH_PARAM]->Value;
		}
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->ab_group_edit->GetCommandText());
		dbo->ProcessCOMError(e,dbo->ab_group_edit->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return false;
	}
	ReleaseDBO(dboitem);
	return true;
}

bool VS_DBStorage_TrueConf::ManageGroups_AddUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dprint2("$ManageGroups_AddUser gid %ld user %s\n", gid, call_id.m_str);

		ADODB::ParametersPtr p=dbo->ab_group_add_user->Parameters;
		p->Item["@i_ab_gr_id"]->Value = gid;
		p->Item["@c_call_id2"]->Value = call_id.m_str;

		ADODB::_RecordsetPtr rs = dbo->ab_group_add_user->Execute(0,0,ADODB::adCmdUnspecified);

		if (!rs->ADOEOF)
		{
			hash = rs->Fields->Item[HASH_PARAM]->Value;
		}
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->ab_group_add_user->GetCommandText());
		dbo->ProcessCOMError(e,dbo->ab_group_add_user->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return false;
	}
	ReleaseDBO(dboitem);
	return true;
}

bool VS_DBStorage_TrueConf::ManageGroups_DeleteUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dstream2 << "$ManageGroups_DeleteUser gid " << gid << " user " << call_id.m_str << "\n";

		ADODB::ParametersPtr p=dbo->ab_group_delete_user->Parameters;
		p->Item["@i_ab_gr_id"]->Value = gid;
		p->Item["@c_call_id2"]->Value = call_id.m_str;

		ADODB::_RecordsetPtr rs = dbo->ab_group_delete_user->Execute(0,0,ADODB::adCmdUnspecified);

		if (!rs->ADOEOF)
		{
			hash = rs->Fields->Item[HASH_PARAM]->Value;
		}
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->ab_group_delete_user->GetCommandText());
		dbo->ProcessCOMError(e,dbo->ab_group_delete_user->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return false;
	}
	ReleaseDBO(dboitem);
	return true;
}
#endif