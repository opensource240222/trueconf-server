#pragma once
#include "VS_LocatorStorageImpl.h"
#include "../../../common/std/cpplib/VS_Pool.h"
#include <memory>
class VS_DBObjects;
class  VS_LocatorStorageImplAdoDB : public VS_LocatorStorageImpl
{
	bool GetServerByLogin(const std::string& login, const string_view passwd, std::string& server) override;
	bool GetServerByCallID(const std::string& user, std::string& server) override;
	bool IsValid() const override;

public:
	VS_LocatorStorageImplAdoDB(const std::shared_ptr<VS_Pool> &dbo_pool);
	static std::unique_ptr<VS_LocatorStorageImplAdoDB> Create();
	static void SetDBOPool(const std::shared_ptr<VS_Pool> &);

private:
	std::shared_ptr<VS_Pool> m_dbo_pool;
	VS_DBObjects *GetDBO(const VS_Pool::Item* &item);
};
