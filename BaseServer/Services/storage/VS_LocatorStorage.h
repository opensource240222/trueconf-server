#pragma once
#include <memory>
#include <string>

#include "VS_LocatorStorageImpl.h"
class VS_LocatorStorage
{
	std::unique_ptr<VS_LocatorStorageImpl> impl_;

	VS_LocatorStorage(std::unique_ptr<VS_LocatorStorageImpl> && impl);

public:
	~VS_LocatorStorage();
	static VS_LocatorStorage& Instance(std::unique_ptr<VS_LocatorStorageImpl>&& init_impl = nullptr);

	/* passwd can be used to find language i.e $5~#en or $5~#ru it's helpfull for database*/
	bool GetServerByLogin(const std::string& login, const string_view passwd, std::string& server);
	bool GetServerByCallID(const std::string& user, std::string& server);
	bool IsValid() const;

	void ReInit(std::unique_ptr<VS_LocatorStorageImpl>&& impl);

};
