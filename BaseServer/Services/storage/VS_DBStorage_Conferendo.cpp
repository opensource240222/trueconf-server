#ifdef _WIN32 // not ported
#include "VS_DBStorage_Conferendo.h"
#include "../../../common/std/cpplib/VS_RegistryKey.h"
#include "../../../common/std/cpplib/VS_RegistryConst.h"
#include "../../../common/std/VS_ProfileTools.h"
#include "../../storage/VS_Sphinx.h"

#define DEBUG_CURRENT_MODULE VS_DM_DBSTOR

VS_DBStorage_Conferendo::VS_DBStorage_Conferendo() : m_sphinx_dbo_pool(0)
{

}

VS_DBStorage_Conferendo::~VS_DBStorage_Conferendo()
{
	delete m_sphinx_dbo_pool;
}

bool VS_DBStorage_Conferendo::IsConferendoBS() const
{
	return true;
}

bool VS_DBStorage_Conferendo::ManageGroups_CreateGroup(const vs_user_id& owner, const VS_WideStr& gname, long& gid, long& hash)
{
	return false;
}

bool VS_DBStorage_Conferendo::ManageGroups_DeleteGroup(const vs_user_id& owner, const long gid, long& hash)
{
	return false;
}

bool VS_DBStorage_Conferendo::ManageGroups_RenameGroup(const vs_user_id& owner, const long gid, const VS_WideStr& gname, long& hash)
{
	return false;
}

bool VS_DBStorage_Conferendo::ManageGroups_AddUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash)
{
	return false;
}

bool VS_DBStorage_Conferendo::ManageGroups_DeleteUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash)
{
	return false;
}

bool VS_DBStorage_Conferendo::Init(const VS_SimpleStr& broker_name)
{
	if (VS_DBStorage::Init(broker_name))
	{
		if (IsSphinxParamsPresent())
		{
			unsigned int connections = 0;
			VS_RegistryKey cfg(false, CONFIGURATION_KEY, false, true);
			if (!cfg.IsValid())
				connections = DB_CONNECTIONS_INIT;
			else
			{
				if (cfg.GetValue(&connections, sizeof(connections), VS_REG_INTEGER_VT,DB_CONNECTIONS_TAG ) != 0)
				{
					if (connections > DB_CONNECTIONS_MAX)
						connections = DB_CONNECTIONS_MAX;
					else if (connections < DB_CONNECTIONS_MIN)
						connections = DB_CONNECTIONS_MIN;
				}
				else
					connections = DB_CONNECTIONS_INIT;
			}

			m_sphinx_dbo_pool = new VS_SharedPool(std::make_unique<VS_SphinxDBOFactory>(), connections, false);
		}

		return true;
	}

	return false;
}

bool VS_DBStorage_Conferendo::IsSphinxParamsPresent() const
{
	VS_RegistryKey cfg_root(false, CONFIGURATION_KEY, false, true);

	if (cfg_root.IsValid())
	{
		char buf[256] = {0};
		unsigned long port = 0;

		if (cfg_root.GetValue(buf, sizeof(buf), VS_REG_STRING_VT, DB_SPHINX_HOST_TAG ) > 0)
		{
			cfg_root.GetValue(&port, sizeof(port), VS_REG_INTEGER_VT, DB_SPHINX_PORT_TAG);
			if (port && port <= USHRT_MAX)
				return true;
		}
	}

	return false;
}

void VS_DBStorage_Conferendo::SetUserStatus(const VS_SimpleStr& call_id,int status,const VS_ExtendedStatusStorage &extStatus, bool set_server,const VS_SimpleStr& server)
{

/**
	TODO: store extended statuses
*/
	VS_DBStorage::SetUserStatus(call_id, status,extStatus,set_server, server);

AUTO_PROF
	if (m_sphinx_dbo_pool && !m_sphinx_dbo_pool->Empty())
	{
		const VS_Pool::Item* dboitem;
		VS_SphinxDBObjects* dbo=GetSphinxDBO(dboitem);

		VS_RealUserLogin login(SimpleStrToStringView(call_id));
		dbo->Execute(login.GetUser().c_str(), status);

		ReleaseSphinxDBO(dboitem);
	}
}

VS_SphinxDBObjects* VS_DBStorage_Conferendo::GetSphinxDBO(const VS_Pool::Item* &item)
{
	int n = 0;
	while (state == STATE_RECONNECT)
	{
		dprint1("POOL: Reconnect state, sleeping\n");
		Sleep(10000);
		n++;
		if (n >= 3*6)		// 3 minutes (3*6*10000)
		{
			state = STATE_FAILED;
		}
	}

	n = 0;
	while ((item = m_sphinx_dbo_pool->Get()) == NULL)
	{
		dprint1("POOL: no free db objects, sleeping\n");
		Sleep(500);
		n++;
		if (n >= 3*6*20)	// 3 minutes (3*6*20*500)
		{
			state = STATE_FAILED;
		}
	}

	return (VS_SphinxDBObjects*)item->m_data;
}
#endif