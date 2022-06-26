#pragma once
#include "std-generic/cpplib/ThreadUtils.h"
#ifdef _WIN32	// not ported
#include "VS_LocatorStorageImplAdoDB.h"
#include "VS_DBStorage.h"
#include "../../../common/std/cpplib/VS_RegistryConst.h"
#include "../../../common/std/cpplib/VS_SimpleStr.h"
#include "std/cpplib/StrFromVariantT.h"
#include "../../../common/std/debuglog/VS_Debug.h"
#include "../../../common/std/VS_ProfileTools.h"
#include "std-generic/cpplib/scope_exit.h"
#include <thread>
#include <boost/algorithm/string/predicate.hpp>
#include "../../../common/std/cpplib/VS_Replace.h"

#define DEBUG_CURRENT_MODULE VS_DM_RESOLVE

namespace
{
	std::weak_ptr<VS_Pool> c_dbo_pool_ref;
}
void VS_LocatorStorageImplAdoDB::SetDBOPool(const std::shared_ptr<VS_Pool> & pool)
{
	c_dbo_pool_ref = pool;
}
std::unique_ptr<VS_LocatorStorageImplAdoDB> VS_LocatorStorageImplAdoDB::Create()
{
	auto pool = c_dbo_pool_ref.lock();
	if(!pool)
	{
		dprint1("VS_LocatorStorageImplAdoDB: dbo pool is not set. Call VS_LocatorStorageImplAdoDB::SetDBOPool(...) before\n");
		return nullptr;
	}
	return std::make_unique<VS_LocatorStorageImplAdoDB>(pool);
}
VS_LocatorStorageImplAdoDB::VS_LocatorStorageImplAdoDB(const std::shared_ptr<VS_Pool> &dbo_pool):m_dbo_pool(dbo_pool)
{
}
VS_DBObjects *VS_LocatorStorageImplAdoDB::GetDBO(const VS_Pool::Item* &item)
{
	int n = 0;
	while ((item = m_dbo_pool->Get()) == NULL) {
		dprint1("POOL: no free db objects, sleeping\n");
		vs::SleepFor(std::chrono::milliseconds(500));
		++n;
		if (n >= 3 * 6 * 2) {
			return nullptr;
		}
	};

	return static_cast<VS_DBObjects*>(item->m_data);
}
bool VS_LocatorStorageImplAdoDB::IsValid() const
{
	return !!m_dbo_pool;
}

bool VS_LocatorStorageImplAdoDB::GetServerByCallID(const std::string& call_id, std::string& server)
{
AUTO_PROF
	if (!IsValid())
	{
		dprint1("LocSRV: VS_LocatorStorageImplAdoDB::IsValid() == false\n");
		return false;
	}
	const VS_Pool::Item *it(nullptr);
	auto dbo = GetDBO(it);
	if (!dbo)
		return false;
	dstream3 << "LocSRV: GetServerByCallID call_id = " << call_id;
	VS_SCOPE_EXIT{ m_dbo_pool->Release(it); };
	try
	{
		auto p = dbo->get_server_by_call_id->Parameters;
		p->Item["@c_call_id"]->Value = call_id.c_str();
		auto rs = dbo->get_server_by_call_id->Execute(nullptr,nullptr,0);
		if (rs != 0)
			if (rs->State != ADODB::adStateClosed)
			{
				if (!rs->ADOEOF)
				{
					auto str_val = vs::StrFromVariantT(rs->Fields->Item[static_cast<short>(0)]->Value);
					if (!str_val.IsEmpty())
					{
						server = str_val.m_str;
						server += "#bs";
					}
				}
				rs->Close();
			}
		dstream3 << "LocSRV: server = " << server;
		return true;
	}
	catch (_com_error e)
	{
		_bstr_t	add_str(dbo->get_server_by_call_id->GetCommandText());
		dbo->ProcessCOMError(e, dbo->get_server_by_call_id->ActiveConnection, nullptr, add_str);
		return false;
	}
}
bool VS_LocatorStorageImplAdoDB::GetServerByLogin(const std::string& login, const string_view passwd, std::string& server)
{
	AUTO_PROF

	if (!IsValid())
	{
		dprint1("LocSRV: VS_LocatorStorageImplAdoDB::IsValid() == false\n");
		return false;
	}
	const VS_Pool::Item *it(nullptr);
	auto dbo = GetDBO(it);
	if (!dbo)
		return false;
	VS_SCOPE_EXIT{ m_dbo_pool->Release(it); };
	try
	{
		dstream3 << "LocSRV: GetServerByLogin login = " << login;
		std::string cmd_txt = R"s(select * from vs_get_server_by_login('%login%','%passwd%'))s";
		VS_ReplaceAll(cmd_txt, "%login%", login);
		if (boost::starts_with(passwd, "$5") || boost::starts_with(passwd, "$7")){		// https://projects.trueconf.com/bin/view/Projects/ServiceEmailLogin
			dstream3 << "Use password='" << passwd << "' to find server also!\n";
			VS_ReplaceAll(cmd_txt, "%passwd%", passwd);
		}
		else
			VS_ReplaceAll(cmd_txt, R"('%passwd%')", "NULL");

		dbo->user_find->CommandText = cmd_txt.c_str();
		ADODB::_RecordsetPtr rs = dbo->user_find->Execute(0, 0, ADODB::adCmdText);
		if (rs != 0)
			if (rs->State != ADODB::adStateClosed)
			{
				VS_SCOPE_EXIT{ rs->Close(); };
				if (!rs->ADOEOF)
				{
					VS_SimpleStr res_server = vs::StrFromVariantT(rs->Fields->Item["bs_server"]->Value);
					if (res_server){
						server = res_server.m_str;
						server += "#bs";
					}
				}
			}
		dstream3 << "LocSRV: server = " << server;
		return true;
	}
	catch (_com_error e)
	{
		_bstr_t	add_str(dbo->get_server_by_login->GetCommandText());
		dbo->ProcessCOMError(e, dbo->get_server_by_login->ActiveConnection, nullptr, add_str);
		return false;
	}
}
#endif