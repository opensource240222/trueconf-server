#pragma once
#if defined(_WIN32)

#include "VS_RegistryKey.h"

namespace regkey {

class RegistryKey final : public Key
{
public:
	RegistryKey(string_view root, bool user, string_view name, bool read_only, bool create);
	~RegistryKey();

	RegistryKey(const RegistryKey&);
	RegistryKey& operator=(const RegistryKey&) = delete;

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
	RegistryKey(std::u16string&& keyNameW, bool user, bool read_only);
	void OpenKey(bool create);

	std::string m_keyName;
	std::u16string m_keyNameW;
	void* m_key;
	// Enumerator
	unsigned long m_ValuesIdx;
	unsigned long m_keyIdx;
	bool m_user;
	bool m_keyReadOnly;
	bool m_keyReset;
};

class RegistryBackend final : public Backend
{
public:
	RegistryBackend();

	virtual bool Init(string_view configuration) override;
	virtual key_ptr Create(string_view root, bool user, string_view name, bool read_only, bool create) override;

private:
	bool m_force_lm;
};

}

#endif
