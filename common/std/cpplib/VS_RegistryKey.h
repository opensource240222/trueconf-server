#pragma once

#include "std-generic/attributes.h"
#include "std-generic/cpplib/deleters.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/TimeUtils.h"

#include <cstdint>
#include <ctime>
#include <memory>
#include <string>
#include <type_traits>

#ifdef _SVKS_M_BUILD_
#define VS_DEFAULT_ROOT "SVKS-M"
#else
#define VS_DEFAULT_ROOT "TrueConf"
#endif

enum RegistryVT : int
{
	VS_REG_INTEGER_VT = 0, // int32_t
	VS_REG_INT64_VT,       // int64_t
	VS_REG_STRING_VT,      // char[], zero terminated
	VS_REG_WSTRING_VT,     // wchar_t[], zero terminated
	VS_REG_BINARY_VT,      // uint8_t[]
};

namespace regkey {

class Key;
typedef std::unique_ptr<Key> key_ptr;

class Key
{
public:
	virtual ~Key() {};
	VS_NODISCARD virtual key_ptr Clone() const = 0;
	VS_NODISCARD virtual bool IsValid() const = 0;
	virtual int32_t GetValue(void* buffer, size_t size, RegistryVT type, const char* name) const = 0;
	virtual int32_t GetValue(std::unique_ptr<void, free_deleter>& buffer, RegistryVT type, const char* name) const = 0;
	template <class T>
	int32_t GetValue(std::unique_ptr<T, free_deleter>& buffer, RegistryVT type, const char* name) const
	{
		static_assert(std::is_trivially_destructible<T>::value, "free() can't destroy non trivial objects");
		std::unique_ptr<void, free_deleter> b;
		const auto res = GetValue(b, type, name);
		buffer.reset(static_cast<T*>(b.release()));
		return res;
	}
	virtual int32_t GetValueAndType(void* buffer, size_t size, RegistryVT& type, const char* name) const = 0;
	virtual int32_t GetValueAndType(std::unique_ptr<void, free_deleter>& buffer, RegistryVT& type, const char* name) const = 0;
	template <class T>
	int32_t GetValueAndType(std::unique_ptr<T, free_deleter>& buffer, RegistryVT& type, const char* name) const
	{
		static_assert(std::is_trivially_destructible<T>::value, "free() can't destroy non trivial objects");
		std::unique_ptr<void, free_deleter> b;
		const auto res = GetValueAndType(b, type, name);
		buffer.reset(static_cast<T*>(b.release()));
		return res;
	}
	virtual bool GetString(std::string& value, const char* name) const = 0;
	virtual bool SetValue(const void* buffer, size_t size, RegistryVT type, const char* name) = 0;
	template <class T>
	bool SetValueI32(T val, const char* name)
	{
		auto val32 = static_cast<int32_t>(val);
		return SetValue(&val32, sizeof(val32), VS_REG_INTEGER_VT, name);
	}
	template <class T>
	bool SetValueI64(T val, const char* name)
	{
		auto val64 = static_cast<int64_t>(val);
		return SetValue(&val64, sizeof(val64), VS_REG_INT64_VT, name);
	}
	bool SetString(const char* value, const char* name)
	{
		return SetValue(value, 0, VS_REG_STRING_VT, name);
	}
	VS_NODISCARD virtual bool HasValue(string_view name) const = 0;
	virtual bool RemoveValue(string_view name) = 0;
	virtual bool RemoveKey(string_view name) = 0;
	virtual bool RenameKey(string_view from, string_view to) = 0;
	virtual bool CopyKey(string_view from, string_view to) = 0;
	virtual unsigned GetValueCount() const = 0;
	virtual unsigned GetKeyCount() const = 0;
	virtual void ResetValues() = 0;
	virtual int32_t NextValue(std::unique_ptr<void, free_deleter>& buffer, RegistryVT type, std::string& name) = 0;
	template <class T>
	int32_t NextValue(std::unique_ptr<T, free_deleter>& buffer, RegistryVT type, std::string& name)
	{
		static_assert(std::is_trivially_destructible<T>::value, "free() can't destroy non trivial objects");
		std::unique_ptr<void, free_deleter> b;
		const auto res = NextValue(b, type, name);
		buffer.reset(static_cast<T*>(b.release()));
		return res;
	}
	virtual int32_t NextValueAndType(std::unique_ptr<void, free_deleter>& buffer, RegistryVT& type, std::string& name) = 0;
	template <class T>
	int32_t NextValueAndType(std::unique_ptr<T, free_deleter>& buffer, RegistryVT& type, std::string& name)
	{
		static_assert(std::is_trivially_destructible<T>::value, "free() can't destroy non trivial objects");
		std::unique_ptr<void, free_deleter> b;
		const auto res = NextValueAndType(b, type, name);
		buffer.reset(static_cast<T*>(b.release()));
		return res;
	}
	virtual bool NextString(std::string& value, std::string& name) = 0;
	virtual void ResetKey() = 0;
	virtual key_ptr NextKey(bool read_only = true) = 0;
	virtual const char* GetName() const = 0;
	virtual std::string GetLastWriteTime() const = 0;
};

class Backend
{
public:
	VS_NODISCARD static std::shared_ptr<Backend> Create(string_view configuration);

	virtual ~Backend() {};
	virtual key_ptr Create(string_view root, bool user, string_view name, bool read_only = true, bool create = false) = 0;

	// Dumps all data stored in the backend to stderr.
	// This function is supposed to be called from the debugger and not in real code.
	virtual void TEST_DumpData();

private:
	virtual bool Init(string_view configuration) = 0;
};

}

class VS_RegistryKey
{
public:
	VS_DEPRECATED static int CopyKey(bool s_user, const char* src, bool d_user, const char* dst);

	static void SetDefaultRoot(string_view root)
	{
		s_default_root.assign(root.begin(), root.end());
	}
	static const std::string& GetDefaultRoot()
	{
		return s_default_root;
	}

	VS_NODISCARD static bool InitDefaultBackend(string_view configuration);
	static const std::string& GetDefaultBackendConfiguration()
	{
		return s_backend_configuration;
	}

	VS_RegistryKey() noexcept
		: m_key(s_no_key)
	{}
	VS_RegistryKey(bool user, string_view name, bool readOnly = true, bool create = false)
		: VS_RegistryKey(s_default_root, user, name, readOnly, create)
	{}
	VS_RegistryKey(string_view root, bool user, string_view name, bool readOnly = true, bool create = false)
		: m_key(s_backend->Create(root, user, name, readOnly, create).release())
	{}

#if defined(_MSC_VER) || (!defined(__clang__) && defined(__GNUC__) && __GNUC__ < 6)
	// Workaround for insane false -> pointer implicit conversion in MSVC and GCC (prior to 6.0)
	// which makes calls like VS_RegistryKey("foo", false, "bar") ambiguous
	// ("foo" -> true, false -> (const char*)0, "bar" -> true).
	VS_RegistryKey(bool user, const char* name, bool readOnly = true, bool create = false)
		: VS_RegistryKey(user, string_view(name), readOnly, create) {}
	VS_RegistryKey(const char* root, bool user, const char* name, bool readOnly = true, bool create = false)
		: VS_RegistryKey(string_view(root), user, string_view(name), readOnly, create) {}
	VS_RegistryKey(string_view root, bool user, const char* name, bool readOnly = true, bool create = false)
		: VS_RegistryKey(root, user, string_view(name), readOnly, create) {}
	VS_RegistryKey(const char* root, bool user, string_view name, bool readOnly = true, bool create = false)
		: VS_RegistryKey(string_view(root), user, name, readOnly, create) {}
#endif

	VS_RegistryKey(const VS_RegistryKey& rk)
		: m_key(rk.m_key->Clone().release())
	{
	}
	VS_RegistryKey(VS_RegistryKey&& rk) noexcept
		: m_key(std::move(rk.m_key))
	{
		rk.m_key.reset(s_no_key);
	}
	VS_RegistryKey& operator=(const VS_RegistryKey& rk)
	{
		if (this == &rk)
			return *this;
		m_key.reset(rk.m_key->Clone().release());
		return *this;
	}
	VS_RegistryKey& operator=(VS_RegistryKey&& rk) noexcept
	{
		if (this == &rk)
			return *this;
		m_key = std::move(rk.m_key);
		rk.m_key.reset(s_no_key);
		return *this;
	}

	/// Check if key is open
	VS_NODISCARD bool IsValid() const
	{
		return m_key->IsValid();
	}

	/// Read value from Registry
	/// Returns: 0 - if not found
	///        > 0 - size of value stored in buffer
	///        < 0 - required size
	int32_t GetValue(void* buffer, size_t size, RegistryVT type, const char* name) const
	{
		return m_key->GetValue(buffer, size, type, name);
	}
	int32_t GetValue(std::unique_ptr<void, free_deleter>& buffer, RegistryVT type, const char* name) const
	{
		return m_key->GetValue(buffer, type, name);
	}
	template <class T>
	int32_t GetValue(std::unique_ptr<T, free_deleter>& buffer, RegistryVT type, const char* name) const
	{
		return m_key->GetValue(buffer, type, name);
	}
	int32_t GetValueAndType(std::unique_ptr<void, free_deleter>& buffer, RegistryVT& type, const char* name) const
	{
		return m_key->GetValueAndType(buffer, type, name);
	}
	template <class T>
	int32_t GetValueAndType(std::unique_ptr<T, free_deleter>& buffer, RegistryVT& type, const char* name) const
	{
		return m_key->GetValueAndType(buffer, type, name);
	}

	// Read string from Registry into std::string
	// Returns true if read is successful
	bool GetString(std::string& value, const char* name) const
	{
		return m_key->GetString(value, name);
	}

	/// Store value into Registry
	/// When type is VS_REG_STRING_VT or VS_REG_WSTRING_VT, size is ignored and
	/// buffer must contain a null-terminated string of appropriate type.
	bool SetValue(const void* buffer, size_t size, RegistryVT type, const char* name)
	{
		return m_key->SetValue(buffer, size, type, name);
	}

	template <class T>
	bool SetValueI32(T val, const char* name)
	{
		return m_key->SetValueI32(val, name);
	}

	template <class T>
	bool SetValueI64(T val, const char* name)
	{
		return m_key->SetValueI64(val, name);
	}

	bool SetString(const char* value, const char* name)
	{
		return m_key->SetString(value, name);
	}

	// GetValue/SetValue helpers that work with specific types.
	int32_t GetValue(std::chrono::system_clock::time_point& tp, const char* name)
	{
		int64_t winticks;
		int32_t size = m_key->GetValue(&winticks, sizeof(winticks), VS_REG_BINARY_VT, name);
		if (size == sizeof(int64_t))
			tp = tu::WindowsTickToUnixSeconds(winticks);
		return size;
	}
	bool SetValue(const std::chrono::system_clock::time_point tp, const char* name)
	{
		int64_t winticks = tu::UnixSecondsToWindowsTicks(tp);
		return m_key->SetValue(&winticks, sizeof(winticks), VS_REG_BINARY_VT, name);
	}

	/// Check if value exists
	VS_NODISCARD bool HasValue(string_view name) const
	{
		return m_key->HasValue(name);
	}

	/// Remove value from Registry
	bool RemoveValue(string_view name)
	{
		return m_key->RemoveValue(name);
	}

	bool RemoveKey(string_view name)
	{
		return m_key->RemoveKey(name);
	}

	// Moves content of the subkey pointed to by path 'from' into subkey pointed to by path 'to'.
	// Creates the destination subkey if necessary.
	// If the destination subkey already exists its current content is removed.
	bool RenameKey(string_view from, string_view to)
	{
		return m_key->RenameKey(from, to);
	}

	// Copies content of the subkey pointed to by path 'from' into subkey pointed to by path 'to'.
	// Creates the destination subkey if necessary.
	// If the destination subkey already exists its current content is removed. //TODO: ???
	bool CopyKey(string_view from, string_view to)
	{
		return m_key->CopyKey(from, to);
	}

	// Returns number of values stored directly in this key.
	// NOTE: Results of this function are not cached (this is by design), so calls to function are not cheap.
	unsigned GetValueCount() const
	{
		return m_key->GetValueCount();
	}

	// Returns number of keys stored directly in this key.
	// NOTE: Results of this function are not cached (this is by design), so calls to function are not cheap.
	unsigned GetKeyCount() const
	{
		return m_key->GetKeyCount();
	}

	/// Enumerate values of current key
	void ResetValues()
	{
		return m_key->ResetValues();
	}
	int32_t NextValue(std::unique_ptr<void, free_deleter>& buffer, RegistryVT type, std::string& name)
	{
		return m_key->NextValue(buffer, type, name);
	}
	template <class T>
	int32_t NextValue(std::unique_ptr<T, free_deleter>& buffer, RegistryVT type, std::string& name)
	{
		return m_key->NextValue(buffer, type, name);
	}
	int32_t NextValueAndType(std::unique_ptr<void, free_deleter>& buffer, RegistryVT& type, std::string& name)
	{
		return m_key->NextValueAndType(buffer, type, name);
	}
	template <class T>
	int32_t NextValueAndType(std::unique_ptr<T, free_deleter>& buffer, RegistryVT& type, std::string& name)
	{
		return m_key->NextValueAndType(buffer, type, name);
	}
	bool NextString(std::string& value, std::string& name)
	{
		return m_key->NextString(value, name);
	}

	/// Enumerate Keys
	void ResetKey()
	{
		return m_key->ResetKey();
	}
	bool NextKey(VS_RegistryKey& key, bool readOnly = true)
	{
		auto next_key = m_key->NextKey(readOnly);
		if (next_key)
		{
			key.reset(std::move(next_key));
			return true;
		}
		else
			return false;
	}

	/// KeyName
	const char* GetName() const
	{
		return m_key->GetName();
	}

	/// Get Key last write time
	std::string GetLastWriteTime() const
	{
		return m_key->GetLastWriteTime();
	}

	static void TEST_DumpData();

private:
	void reset(regkey::key_ptr key)
	{
		m_key.reset(key.release());
	}

private:
	static std::string s_default_root;
	static std::shared_ptr<regkey::Backend> s_backend;
	static std::string s_backend_configuration;

	static regkey::Key* s_no_key;
	struct key_deleter { void operator()(const regkey::Key* p) const {
		if (p != s_no_key)
			delete p;
	}};
	std::unique_ptr<regkey::Key, key_deleter> m_key;
};
