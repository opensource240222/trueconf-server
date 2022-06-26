#include "VS_LocatorStorage.h"
#include "VS_LocatorStorageImplAdoDB.h"
VS_LocatorStorage& VS_LocatorStorage::Instance(std::unique_ptr<VS_LocatorStorageImpl>&& init_impl)
{
	static VS_LocatorStorage instance(!!init_impl ? std::move(init_impl) : VS_LocatorStorageImplAdoDB::Create());
	return instance;
}

VS_LocatorStorage::~VS_LocatorStorage()
{}
VS_LocatorStorage::VS_LocatorStorage(std::unique_ptr<VS_LocatorStorageImpl> && impl) : impl_(std::move(impl))
{}

void VS_LocatorStorage::ReInit(std::unique_ptr<VS_LocatorStorageImpl>&&impl)
{
	impl_ = std::move(impl);
}


bool VS_LocatorStorage::GetServerByLogin(const std::string& login, const string_view passwd, std::string& server)
{
	return (impl_ && impl_->IsValid()) ? impl_->GetServerByLogin(login, passwd, server) : false;
}
bool VS_LocatorStorage::GetServerByCallID(const std::string& user, std::string& server)
{
	return (impl_ && impl_->IsValid()) ? impl_->GetServerByCallID(user, server) : false;
}
bool VS_LocatorStorage::IsValid() const
{
	return !!impl_ ? impl_->IsValid() : false;
}