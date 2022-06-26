#pragma once
#if defined(HAVE_CPPDB)

#include "VS_RegistryKey.h"

#include "std-generic/cpplib/VS_CppDBIncludes.h"

namespace regkey {

class DBBackend;

class DBKey final : public Key
{
public:
	DBKey(std::shared_ptr<DBBackend> backend, string_view root, string_view name, bool read_only, bool create);
	~DBKey();

	DBKey(const DBKey&);
	DBKey& operator=(const DBKey&) = delete;

	virtual key_ptr Clone() const override;
	virtual bool IsValid() const override;
	virtual int32_t GetValue(void*, size_t, RegistryVT, const char*) const override;
	virtual int32_t GetValue(std::unique_ptr<void, free_deleter>&, RegistryVT, const char*) const override;
	virtual int32_t GetValueAndType(void*, size_t, RegistryVT&, const char*) const override;
	virtual int32_t GetValueAndType(std::unique_ptr<void, free_deleter>&, RegistryVT&, const char*) const override;
	virtual bool GetString(std::string&, const char*) const override;
	virtual bool SetValue(const void*, size_t, RegistryVT, const char*) override;
	virtual bool HasValue(string_view) const override;
	virtual bool RemoveValue(string_view) override;
	virtual bool RemoveKey(string_view) override;
	virtual bool RenameKey(string_view, string_view) override;
	virtual bool CopyKey(string_view, string_view) override;
	virtual unsigned GetValueCount() const override;
	virtual unsigned GetKeyCount() const override;
	virtual void ResetValues() override;
	virtual int32_t NextValue(std::unique_ptr<void, free_deleter>&, RegistryVT, std::string&) override;
	virtual int32_t NextValueAndType(std::unique_ptr<void, free_deleter>&, RegistryVT&, std::string&) override;
	virtual bool NextString(std::string&, std::string&) override;
	virtual void ResetKey() override;
	virtual key_ptr NextKey(bool) override;
	virtual const char* GetName() const override;
	virtual std::string GetLastWriteTime() const override;

private:
	DBKey(std::shared_ptr<DBBackend> backend, std::string&& key_name, bool read_only);

	std::shared_ptr<DBBackend> m_backend;
	mutable cppdb::session m_session;
	std::string m_key_name;
	bool m_read_only;
	// Used for iterating over values
	cppdb::result m_values;
	// Used for iterating over subkeys
	cppdb::result m_subkeys;
};

class DBBackend final : public Backend, public std::enable_shared_from_this<DBBackend>
{
public:
	cppdb::session OpenSession();

	virtual bool Init(string_view configuration) override;
	virtual key_ptr Create(string_view root, bool user, string_view name, bool read_only, bool create) override;

private:
	cppdb::session OpenSession_internal();

	cppdb::pool::pointer m_pool;
};

}

#endif
