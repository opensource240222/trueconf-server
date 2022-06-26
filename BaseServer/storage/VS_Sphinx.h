#pragma once

#include <vector>
#include "../../common/std/cpplib/VS_Pool.h"

class VS_SphinxDBObjects
{
public:
	VS_SphinxDBObjects();
	~VS_SphinxDBObjects();
	bool Init();
	bool Execute(const char *login, int status);
private:
	static size_t uses_;
	bool inited_;
	void* conn_;
	std::vector<std::string> queries_;
	size_t maxQueryLength_;
};
class VS_SphinxDBOFactory : public VS_Pool::Factory
{
public:
	VS_SphinxDBOFactory() {}
	bool New(VS_Pool::Data &data) override
	{
		VS_SphinxDBObjects *dbo = new VS_SphinxDBObjects;
		if (!dbo->Init())
		{
			delete dbo;
			data = 0;
			return false;
		}

		data = dbo;
		return true;
	}
	void Delete(VS_Pool::Data data) override
	{
		delete static_cast<VS_SphinxDBObjects*>(data);
	}
};