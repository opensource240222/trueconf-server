#pragma once
#include "VS_LocatorStorageImpl.h"

#include <map>
class  VS_LocatorStorageImplMemory : public VS_LocatorStorageImpl
{
public:
	VS_LocatorStorageImplMemory(const char *default_locator = nullptr);
	void SetServerForLogin(const char* login, const char *server);
	void SetServerForCallID(const char* call_id, const char *server);

private:
	bool GetServerByLogin(const std::string& login, const string_view passwd, std::string& server) override;
	bool GetServerByCallID(const std::string& user, std::string & server) override;
	bool IsValid() const override;

	std::map<std::string, std::string>	servers_by_login_;
	std::map<std::string, std::string>	servers_by_call_id_;
	std::string							default_locator_;
};
