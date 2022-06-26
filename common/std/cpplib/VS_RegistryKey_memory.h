#pragma once

#include "VS_RegistryKey.h"

#include <boost/container/flat_map.hpp>
#include <boost/variant/variant.hpp>

#include <mutex>
#include <vector>

namespace regkey {

class Node;
using node_ptr = std::shared_ptr<Node>;

class MemoryKey final : public Key
{
public:
	MemoryKey(node_ptr&& node, std::string&& name, bool read_only);
	~MemoryKey();

	MemoryKey(const MemoryKey&);
	MemoryKey& operator=(const MemoryKey&) = delete;

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
	node_ptr m_node;
	std::string m_name;
	bool m_read_only;
	size_t m_value_index;
	size_t m_subkey_index;
};

class MemoryBackend final : public Backend
{
public:
	virtual bool Init(string_view configuration) override;
	virtual key_ptr Create(string_view root, bool user, string_view name, bool read_only, bool create) override;

	virtual void TEST_DumpData() override;

private:
	node_ptr m_root;
};

}
