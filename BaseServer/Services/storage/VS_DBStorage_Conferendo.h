#pragma once
#include "VS_DBStorage.h"

class VS_SphinxDBObjects;

class VS_DBStorage_Conferendo: public VS_DBStorage
{
public:
	VS_DBStorage_Conferendo();
	virtual ~VS_DBStorage_Conferendo();

	virtual bool IsConferendoBS() const;

	virtual bool ManageGroups_CreateGroup(const vs_user_id& owner, const VS_WideStr& gname, long& gid, long& hash);
	virtual bool ManageGroups_DeleteGroup(const vs_user_id& owner, const long gid, long& hash);
	virtual bool ManageGroups_RenameGroup(const vs_user_id& owner, const long gid, const VS_WideStr& gname, long& hash);
	virtual bool ManageGroups_AddUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash);
	virtual bool ManageGroups_DeleteUser(const vs_user_id& owner, const long gid, const VS_SimpleStr& call_id, long& hash);

	virtual bool Init(const VS_SimpleStr& broker_name);
	virtual void SetUserStatus(const VS_SimpleStr& call_id,int status, const VS_ExtendedStatusStorage &extStatus, bool set_server,const VS_SimpleStr& server);

private:

	VS_SharedPool *m_sphinx_dbo_pool;

	bool IsSphinxParamsPresent() const;
	VS_SphinxDBObjects* GetSphinxDBO(const VS_Pool::Item* &item);
	void ReleaseSphinxDBO(const VS_Pool::Item* item) { m_sphinx_dbo_pool->Release(item); }
};