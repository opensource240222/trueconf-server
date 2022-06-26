#include "VS_LocatorStorageImplMemory.h"
VS_LocatorStorageImplMemory::VS_LocatorStorageImplMemory(const char *default_locator) : default_locator_(nullptr == default_locator?"":default_locator)
{
}
void VS_LocatorStorageImplMemory::SetServerForCallID(const char*call_id, const char*server)
{
	servers_by_call_id_[call_id] = server;
}

void VS_LocatorStorageImplMemory::SetServerForLogin(const char* login, const char *server)
{
	servers_by_login_[login] = server;
}

bool VS_LocatorStorageImplMemory::GetServerByCallID(const std::string& login, std::string& server)
{
	auto it = servers_by_call_id_.find(login);
	if (it == servers_by_call_id_.end())
	{
		if (!default_locator_.empty())
		{
			server = default_locator_;
			return true;
		}
		return false;
	}
	server = it->second;
	return true;
}

bool VS_LocatorStorageImplMemory::GetServerByLogin(const std::string& login, const string_view passwd, std::string& server)
{
	auto it = servers_by_login_.find(login);
	if (it == servers_by_login_.end())
	{
		if (!default_locator_.empty())
		{
			server = default_locator_;
			return true;
		}
		return false;
	}
	server = it->second;
	return true;
}

bool VS_LocatorStorageImplMemory::IsValid() const
{
	return true;
}