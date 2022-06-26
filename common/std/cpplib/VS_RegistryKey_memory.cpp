#include "VS_RegistryKey_memory.h"
#include "VS_RegistryKey_internal.h"
#include "../debuglog/VS_Debug.h"

#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>

#include "std-generic/compat/memory.h"
#include <cassert>
#include <cctype>
#include <cinttypes>
#include <cwchar>
#include <iostream>
#include <limits>

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

namespace regkey {

struct icase_less
{
	bool operator()(const std::string& l, const std::string& r) const
	{
		const auto l_size = l.size();
		const auto r_size = r.size();
		if (l_size == r_size)
		{
			for (size_t i = 0; i < l_size; ++i)
			{
				const auto l_char = std::toupper(l[i]);
				const auto r_char = std::toupper(r[i]);
				if (l_char == r_char)
					continue;
				return l_char < r_char;
			}
			return false; // strings are equal
		}
		else
			return l_size < r_size;
	}
};

class Node
{
public:
	using binary_value = std::vector<uint8_t>;
	struct no_value {};
	using value_type = boost::variant<no_value, int32_t, int64_t, std::string, std::wstring, binary_value>;

	node_ptr Clone() const;
	void CopyFrom(const Node& other);

	node_ptr GetNode(const std::string& name, bool create = false);
	node_ptr GetNode(size_t index, std::string& name);
	node_ptr ExtractNode(const std::string& name);
	void PutNode(std::string&& name, node_ptr&& node);
	unsigned NodeCount() const;

	value_type GetValue(const std::string& name);
	value_type GetValue(size_t index, std::string& name);
	void SetValue(std::string&& name, value_type&& value);
	bool HasValue(const std::string& name);
	bool RemoveValue(const std::string& name);
	unsigned ValueCount() const;

	void DumpData(std::ostream& os, unsigned indent_step = 2, unsigned indent = 0) const;

private:
	mutable std::mutex m_mutex;
	boost::container::flat_map<std::string, value_type, icase_less> m_values;
	boost::container::flat_map<std::string, node_ptr, icase_less> m_nodes;
};

node_ptr Node::Clone() const
{
	auto result = std::make_shared<Node>();
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	result->m_values = m_values;
	result->m_nodes.reserve(m_nodes.size());
	for (const auto& kv : m_nodes)
		result->m_nodes.emplace(kv.first, kv.second->Clone());
	return result;
}

void Node::CopyFrom(const Node& other)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	std::lock_guard<decltype(m_mutex)> lock_other(other.m_mutex);
	for (const auto& kv : other.m_values)
		m_values[kv.first] = kv.second;
	for (const auto& kv : other.m_nodes)
	{
		auto it = m_nodes.find(kv.first);
		if (it != m_nodes.end())
			it->second->CopyFrom(*kv.second);
		else
			m_nodes[kv.first] = kv.second->Clone();
	}
}

node_ptr Node::GetNode(const std::string& name, bool create)
{
	node_ptr result;
	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		auto it = m_nodes.find(name);
		if (it != m_nodes.end())
			result = it->second;
		else if (create)
		{
			result = std::make_shared<Node>();
			m_nodes.emplace(name, result);
		}
	}
	return result;
}

node_ptr Node::GetNode(size_t index, std::string& name)
{
	node_ptr result;
	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		if (index < m_nodes.size())
#if BOOST_VERSION < 105800
			std::tie(name, result) = *(m_nodes.begin() + index);
#else
			std::tie(name, result) = *m_nodes.nth(index);
#endif
	}
	return result;
}

node_ptr Node::ExtractNode(const std::string& name)
{
	node_ptr result;
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	auto it = m_nodes.find(name);
	if (it != m_nodes.end())
	{
		result = std::move(it->second);
		m_nodes.erase(it);
	}
	return result;
}

void Node::PutNode(std::string&& name, node_ptr&& node)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	m_nodes[std::move(name)] = std::move(node);
}

unsigned Node::NodeCount() const
{
	return m_nodes.size();
}

Node::value_type Node::GetValue(const std::string& name)
{
	value_type result;
	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		auto it = m_values.find(name);
		if (it != m_values.end())
			result = it->second;
	}
	return result;
}

Node::value_type Node::GetValue(size_t index, std::string& name)
{
	value_type result;
	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		if (index < m_values.size())
#if BOOST_VERSION < 105800
			std::tie(name, result) = *(m_values.begin() + index);
#else
			std::tie(name, result) = *m_values.nth(index);
#endif
	}
	return result;
}

void Node::SetValue(std::string&& name, value_type&& value)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	auto it = m_values.emplace(std::move(name), value_type{}).first;
	assert(it != m_values.end());
	it->second = std::move(value);
}

bool Node::HasValue(const std::string& name)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	return m_values.count(name) > 0;
}

bool Node::RemoveValue(const std::string& name)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	return m_values.erase(name) > 0;
}

unsigned Node::ValueCount() const
{
	return m_values.size();
}

namespace {

struct print_value : public boost::static_visitor<void>
{
	explicit print_value(std::ostream& os_) : os(os_) {}

	void operator()(int32_t value) const
	{
		os << value << "i32";
	}
	void operator()(int64_t value) const
	{
		os << value << "i64";
	}
	void operator()(const std::string& value) const
	{
		os << '"' << value << '"';
	}
#if defined(_WIN32) // Not ported yet
	void operator()(const std::wstring& value) const
	{
		auto value_utf8 = vs::UTF16toUTF8Convert(value);
		if (value[0] && value_utf8.empty())
		{
			os << "L?";
			return;
		}
		os << "L\"" << value_utf8 << '"';
	}
#endif
	void operator()(const Node::binary_value& value) const
	{
		os << "BIN:" << std::hex;
		for (uint8_t x: value)
			os << ' ' << x;
		os << std::dec;
	}
	template <class T>
	void operator()(const T&) const
	{
		os << "EMPTY";
	}

private:
	std::ostream& os;
};

}

void Node::DumpData(std::ostream& os, unsigned indent_step, unsigned indent) const
{
	for (const auto& x: m_values)
	{
		for (unsigned i = 0; i < indent; ++i)
			os.put(' ');
		os << x.first << ": ";
		boost::apply_visitor(print_value(os), x.second);
		os << '\n';
	}
	for (const auto& x: m_nodes)
	{
		for (unsigned i = 0; i < indent; ++i)
			os.put(' ');
		os << x.first << "\\\n";
		x.second->DumpData(os, indent_step, indent + indent_step);
	}
}

namespace {

struct get_best_type : public boost::static_visitor<bool>
{
	explicit get_best_type(RegistryVT& type_) : type(type_) {}

	bool operator()(int32_t) const
	{
		type = VS_REG_INTEGER_VT;
		return true;
	}
	bool operator()(int64_t) const
	{
		type = VS_REG_INT64_VT;
		return true;
	}
	bool operator()(const std::string&) const
	{
		type = VS_REG_STRING_VT;
		return true;
	}
#if defined(_WIN32) // Not ported yet
	bool operator()(const std::wstring&) const
	{
		type = VS_REG_WSTRING_VT;
		return true;
	}
#endif
	bool operator()(const Node::binary_value&) const
	{
		type = VS_REG_BINARY_VT;
		return true;
	}
	template <class T>
	bool operator()(const T&) const
	{
		return false;
	}

private:
	RegistryVT& type;
};

bool GetBestType_Memory(const Node::value_type& value, RegistryVT& type)
{
	return boost::apply_visitor(get_best_type(type), value);
}

struct parse_int32 : public boost::static_visitor<int32_t>
{
	parse_int32(void*& buffer_, size_t size_) : buffer(buffer_), size(size_) {}

	int32_t operator()(int32_t value) const
	{
		const size_t result_size = sizeof(int32_t);
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		*static_cast<int32_t*>(buffer) = value;
		return result_size;
	}
	int32_t operator()(const std::string& value) const
	{
		const size_t result_size = sizeof(int32_t);
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		int32_t value_int;
		if (!ParseToInteger(value.c_str(), value_int))
			return 0;
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		*static_cast<int32_t*>(buffer) = value_int;
		return result_size;
	}
#if defined(_WIN32) // Not ported yet
	int32_t operator()(const std::wstring& value) const
	{
		const size_t result_size = sizeof(int32_t);
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		int32_t value_int;
		if (!ParseToInteger(value.c_str(), value_int))
			return 0;
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		*static_cast<int32_t*>(buffer) = value_int;
		return result_size;
	}
#endif
	template <class T>
	int32_t operator()(const T&) const
	{
		return 0;
	}

private:
	//cppcheck-suppress unsafeClassCanLeak
	void*& buffer;
	size_t size;
};

struct parse_int64 : public boost::static_visitor<int32_t>
{
	parse_int64(void*& buffer_, size_t size_) : buffer(buffer_), size(size_) {}

	int32_t operator()(int32_t value) const
	{
		const size_t result_size = sizeof(int64_t);
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		*static_cast<int64_t*>(buffer) = value;
		return result_size;
	}
	int32_t operator()(int64_t value) const
	{
		const size_t result_size = sizeof(int64_t);
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		*static_cast<int64_t*>(buffer) = value;
		return result_size;
	}
	int32_t operator()(const std::string& value) const
	{
		const size_t result_size = sizeof(int64_t);
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		int64_t value_int;
		if (!ParseToInteger(value.c_str(), value_int))
			return 0;
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		*static_cast<int64_t*>(buffer) = value_int;
		return result_size;
	}
#if defined(_WIN32) // Not ported yet
	int32_t operator()(const std::wstring& value) const
	{
		const size_t result_size = sizeof(int64_t);
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		int64_t value_int;
		if (!ParseToInteger(value.c_str(), value_int))
			return 0;
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		*static_cast<int64_t*>(buffer) = value_int;
		return result_size;
	}
#endif
	template <class T>
	int32_t operator()(const T&) const
	{
		return 0;
	}

private:
	//cppcheck-suppress unsafeClassCanLeak
	void*& buffer;
	size_t size;
};

struct parse_string : public boost::static_visitor<int32_t>
{
	parse_string(void*& buffer_, size_t size_) : buffer(buffer_), size(size_) {}

	int32_t operator()(int32_t value) const
	{
		char value_str[1/*sign*/ + (std::numeric_limits<int32_t>::digits10 + 1)/*digits*/ + 1/*'\0'*/] = {};
		sprintf(value_str, "%" PRIi32, value);
		const size_t result_size = std::strlen(value_str) + 1;
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		std::memcpy(buffer, value_str, result_size);
		return result_size;
	}
	int32_t operator()(int64_t value) const
	{
		char value_str[1/*sign*/ + (std::numeric_limits<int64_t>::digits10 + 1)/*digits*/ + 1/*'\0'*/] = {};
		sprintf(value_str, "%" PRIi64, value);
		const size_t result_size = std::strlen(value_str) + 1;
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		std::memcpy(buffer, value_str, result_size);
		return result_size;
	}
	int32_t operator()(const std::string& value) const
	{
		const size_t result_size = value.size() + 1;
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		std::memcpy(buffer, value.data(), result_size);
		return result_size;
	}
#if defined(_WIN32) // Not ported yet
	int32_t operator()(const std::wstring& value) const
	{
		auto value_utf8 = vs::UTF16toUTF8Convert(value);
		if (value[0] && value_utf8.empty())
		{
			dstream4 << "ReqistryKeyMemory: UTF16->UTF8 failed";
			return 0;
		}

		const size_t result_size = value_utf8.size() + 1;
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		std::memcpy(buffer, value_utf8.data(), result_size);
		return result_size;
	}
#endif
	template <class T>
	int32_t operator()(const T&) const
	{
		return 0;
	}

private:
	//cppcheck-suppress unsafeClassCanLeak
	void*& buffer;
	size_t size;
};

struct parse_string_obj : public boost::static_visitor<bool>
{
	explicit parse_string_obj(std::string& result_) : result(result_) {}

	bool operator()(int32_t value) const
	{
		char value_str[1/*sign*/ + (std::numeric_limits<int32_t>::digits10 + 1)/*digits*/ + 1/*'\0'*/] = {};
		sprintf(value_str, "%" PRIi32, value);
		result = value_str;
		return true;
	}
	bool operator()(int64_t value) const
	{
		char value_str[1/*sign*/ + (std::numeric_limits<int64_t>::digits10 + 1)/*digits*/ + 1/*'\0'*/] = {};
		sprintf(value_str, "%" PRIi64, value);
		result = value_str;
		return true;
	}
	bool operator()(std::string& value) const
	{
		result = std::move(value);
		return true;
	}
#if defined(_WIN32) // Not ported yet
	bool operator()(const std::wstring& value) const
	{
		auto result = vs::UTF16toUTF8Convert(value);
		if (value[0] && result.empty())
		{
			dstream4 << "ReqistryKeyMemory: UTF16->UTF8 failed";
			return false;
		}
		return true;
	}
#endif
	template <class T>
	bool operator()(const T&) const
	{
		return false;
	}

private:
	std::string& result;
};

#if defined(_WIN32) // Not ported yet
struct parse_wstring : public boost::static_visitor<int32_t>
{
	parse_wstring(void*& buffer_, size_t size_) : buffer(buffer_), size(size_) {}

#define VS_WIDEN_(str) L ## str
#define VS_WIDEN(str) VS_WIDEN_(str)
	int32_t operator()(int32_t value) const
	{
		wchar_t value_str[1/*sign*/ + (std::numeric_limits<int32_t>::digits10 + 1)/*digits*/ + 1/*'\0'*/] = {};
		swprintf(value_str, L"%" VS_WIDEN(PRIi32), value);
		const size_t result_size = sizeof(wchar_t) * (std::wcslen(value_str) + 1);
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		std::memcpy(buffer, value_str, result_size);
		return result_size;
	}
	int32_t operator()(int64_t value) const
	{
		wchar_t value_str[1/*sign*/ + (std::numeric_limits<int64_t>::digits10 + 1)/*digits*/ + 1/*'\0'*/] = {};
		swprintf(value_str, L"%" VS_WIDEN(PRIi64), value);
		const size_t result_size = sizeof(wchar_t) * (std::wcslen(value_str) + 1);
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		std::memcpy(buffer, value_str, result_size);
		return result_size;
	}
#undef VS_WIDEN_
#undef VS_WIDEN
	int32_t operator()(const std::string& value) const
	{
		auto value_utf16 = vs::UTF8toUTF16Convert(value);
		if (value[0] && value_utf16.empty())
		{
			dstream4 << "ReqistryKeyMemory: UTF8->UTF16 failed";
			return 0;
		}

		const size_t result_size = sizeof(char16_t) * (value_utf16.size() + 1);
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		std::memcpy(buffer, value_utf16.data(), result_size);
		return result_size;
	}
	int32_t operator()(const std::wstring& value) const
	{
		const size_t result_size = sizeof(char16_t) * (value.size() + 1);
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		std::memcpy(buffer, value.data(), result_size);
		return result_size;
	}
	template <class T>
	int32_t operator()(const T&) const
	{
		return 0;
	}

private:
	void*& buffer;
	size_t size;
};
#endif

struct parse_binary : public boost::static_visitor<int32_t>
{
	parse_binary(void*& buffer_, size_t size_) : buffer(buffer_), size(size_) {}

	int32_t operator()(int32_t value) const
	{
		const size_t result_size = sizeof(value);
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		std::memcpy(buffer, &value, result_size);
		return result_size;
	}
	int32_t operator()(int64_t value) const
	{
		const size_t result_size = sizeof(value);
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		std::memcpy(buffer, &value, result_size);
		return result_size;
	}
	int32_t operator()(const std::string& value) const
	{
		const size_t result_size = value.size() + 1;
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		std::memcpy(buffer, value.data(), result_size);
		return result_size;
	}
#if defined(_WIN32) // Not ported yet
	int32_t operator()(const std::wstring& value) const
	{
		const size_t result_size = sizeof(wchar_t) * (value.size() + 1);
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		std::memcpy(buffer, value.data(), result_size);
		return result_size;
	}
#endif
	int32_t operator()(const Node::binary_value& value) const
	{
		const size_t result_size = value.size();
		if (size < result_size)
			return -static_cast<int32_t>(result_size);
		if (buffer == nullptr)
			buffer = ::malloc(result_size);
		std::memcpy(buffer, value.data(), result_size);
		return result_size;
	}
	template <class T>
	int32_t operator()(const T&) const
	{
		return 0;
	}

private:
	//cppcheck-suppress unsafeClassCanLeak
	void*& buffer;
	size_t size;
};

// Parses 'value' to type 'type' and stores it into 'buffer',
// 'size' specifies maximum size of the result that can be stored into 'buffer'.
// If size of the result is greater that 'size' returns -(size of the result).
// If buffer isn't provided (buffer == null) it will be allocated via malloc.
// If parsing fails returns 0, othewise returns size of the result.
int32_t ParseValue_Memory(Node::value_type&& value, RegistryVT type, void*& buffer, size_t size)
{
	switch (type)
	{
	case VS_REG_INTEGER_VT:
		return boost::apply_visitor(parse_int32(buffer, size), value);
	case VS_REG_INT64_VT:
		return boost::apply_visitor(parse_int64(buffer, size), value);
	case VS_REG_STRING_VT:
		return boost::apply_visitor(parse_string(buffer, size), value);
	case VS_REG_WSTRING_VT:
#if defined(_WIN32) // Not ported yet
		return boost::apply_visitor(parse_wstring(buffer, size), value);
#else
		return 0;
#endif
	case VS_REG_BINARY_VT:
		return boost::apply_visitor(parse_binary(buffer, size), value);
	}
	return 0;
}

bool ParseValue_Memory(Node::value_type&& value, std::string& result)
{
	// No std::move(value) because Boost.Variant can't handle rvalue references.
	return boost::apply_visitor(parse_string_obj(result), value);
}

// Formats value of type 'type' stored in (buffer, size) into (Node::value_type value).
// If formating is successful calls 'cb' with (value) as arguments
// and returns its return value, othewise return false.
template <class Callback>
bool FormatValue_Memory(RegistryVT type, const void* buffer, size_t size, Callback&& cb)
{
	switch (type)
	{
	case VS_REG_INTEGER_VT:
		if (size < sizeof(int32_t))
			return false;
		return cb(*static_cast<const int32_t*>(buffer));
	case VS_REG_INT64_VT:
		if (size < sizeof(int64_t))
			return false;
		return cb(*static_cast<const int64_t*>(buffer));
	case VS_REG_STRING_VT:
		return cb(static_cast<const char*>(buffer));
	case VS_REG_WSTRING_VT:
		return cb(static_cast<const wchar_t*>(buffer));
	case VS_REG_BINARY_VT:
	{
		const auto begin = static_cast<const uint8_t*>(buffer);
		const auto end = begin + size;
		return cb(Node::binary_value(begin, end));
	}
	}
	return false;
}

}

MemoryKey::MemoryKey(node_ptr&& node, std::string&& name, bool read_only)
	: m_node(std::move(node))
	, m_name(std::move(name))
	, m_read_only(read_only)
	, m_value_index(static_cast<size_t>(-1))
	, m_subkey_index(static_cast<size_t>(-1))
{
}

MemoryKey::~MemoryKey()
{
}

MemoryKey::MemoryKey(const MemoryKey& x)
	: m_node(x.m_node)
	, m_name(x.m_name)
	, m_read_only(x.m_read_only)
	, m_value_index(static_cast<size_t>(-1))
	, m_subkey_index(static_cast<size_t>(-1))
{
	// m_value_index and m_subkey_index are reset to match behaviour of DB version.
}

key_ptr MemoryKey::Clone() const
{
	return vs::make_unique<MemoryKey>(*this);
}

bool MemoryKey::IsValid() const
{
	return static_cast<bool>(m_node);
}

int32_t MemoryKey::GetValue(void* buffer, size_t size, RegistryVT type, const char* name) const
{
	assert(!(buffer == nullptr && size != 0)); // This combination doesn't make sense, also we use it internally in allocating GetValue.

	if (!m_node)
		return 0;

	return ParseValue_Memory(m_node->GetValue(name), type, buffer, size);
}

int32_t MemoryKey::GetValue(std::unique_ptr<void, free_deleter>& buffer, RegistryVT type, const char* name) const
{
	if (!m_node)
		return 0;

	void* p_data = nullptr;
	const auto res = ParseValue_Memory(m_node->GetValue(name), type, p_data, std::numeric_limits<int32_t>::max());
	if (p_data)
		buffer.reset(p_data);
	return res;
}

int32_t MemoryKey::GetValueAndType(void* buffer, size_t size, RegistryVT& type, const char* name) const
{
	assert(!(buffer == nullptr && size != 0)); // This combination doesn't make sense, also we use it internally in allocating GetValue.

	if (!m_node)
		return 0;

	auto value = m_node->GetValue(name);
	if (!GetBestType_Memory(value, type))
		return 0;
	return ParseValue_Memory(std::move(value), type, buffer, size);
}

int32_t MemoryKey::GetValueAndType(std::unique_ptr<void, free_deleter>& buffer, RegistryVT& type, const char* name) const
{
	if (!m_node)
		return 0;

	auto value = m_node->GetValue(name);
	if (!GetBestType_Memory(value, type))
		return 0;
	void* p_data = nullptr;
	const auto res = ParseValue_Memory(std::move(value), type, p_data, std::numeric_limits<int32_t>::max());
	if (p_data)
		buffer.reset(p_data);
	return res;
}

bool MemoryKey::GetString(std::string& data, const char* name) const
{
	if (!m_node)
		return false;

	return ParseValue_Memory(m_node->GetValue(name), data);
}

bool MemoryKey::SetValue(const void* buffer, size_t size, RegistryVT type, const char* name)
{
	if (!m_node)
		return false;
	if (m_read_only)
		return false;

	return FormatValue_Memory(type, buffer, size, [&](Node::value_type&& value)
	{
		m_node->SetValue(name, std::move(value));
		return true;
	});
}

bool MemoryKey::HasValue(string_view name) const
{
	if (!m_node)
		return false;

	return m_node->HasValue(std::string(name));
}

bool MemoryKey::RemoveValue(string_view name)
{
	if (!m_node)
		return false;
	if (m_read_only)
		return false;

	return m_node->RemoveValue(std::string(name));
}

bool MemoryKey::RemoveKey(string_view name)
{
	if (!m_node)
		return false;
	if (m_read_only)
		return false;

	m_node->ExtractNode(std::string(name));
	return true;
}

bool MemoryKey::RenameKey(string_view from, string_view to)
{
	if (!m_node)
		return false;
	if (m_read_only)
		return false;
	switch (IsSubKey(from, to))
	{
	case 1: case 2:
		return false; // Moving to a or from a subkey, registry backend doesn't support this so we shouldn't also.
	case 3:
		return true; // Moving to itself, nothing to do.
	}

	std::string node_name;
	node_name.reserve(std::max(from.size(), to.size()));

	node_ptr src = m_node;
	while (true)
	{
		auto sep_pos = from.find('\\');
		if (sep_pos == from.npos)
			break;
		node_name.assign(from.data(), sep_pos);
		src = src->GetNode(node_name);
		if (!src)
			return false;
		from.remove_prefix(sep_pos + 1);
	}
	// It is important to extract the node before finding the destination node.
	// Othrewise we could create a loop if the destination is inside the extracted node.
	node_name.assign(from.begin(), from.end());
	auto node = src->ExtractNode(node_name);
	if (!node)
		return false;

	node_ptr dst = m_node;
	while (true)
	{
		auto sep_pos = to.find('\\');
		if (sep_pos == to.npos)
			break;
		node_name.assign(to.data(), sep_pos);
		dst = dst->GetNode(node_name, true);
		to.remove_prefix(sep_pos + 1);
	}
	node_name.assign(to.begin(), to.end());
	dst->PutNode(std::move(node_name), std::move(node));
	return true;
}

bool MemoryKey::CopyKey(string_view from, string_view to)
{
	if (!m_node)
		return false;
	if (m_read_only)
		return false;
	switch (IsSubKey(from, to))
	{
	case 1: case 2:
		return false; // Copying to a or from a subkey, registry backend doesn't support this so we shouldn't also.
	case 3:
		return true; // Copying to itself, nothing to do.
	}

	std::string node_name;
	node_name.reserve(std::max(from.size(), to.size()));

	node_ptr src = m_node;
	while (true)
	{
		auto sep_pos = from.find('\\');
		if (sep_pos == from.npos)
			break;
		node_name.assign(from.data(), sep_pos);
		src = src->GetNode(node_name);
		if (!src)
			return false;
		from.remove_prefix(sep_pos + 1);
	}
	node_name.assign(from.begin(), from.end());
	auto node = src->GetNode(node_name);
	if (!node)
		return false;

	node_ptr dst = m_node;
	while (true)
	{
		auto sep_pos = to.find('\\');
		if (sep_pos == to.npos)
			break;
		node_name.assign(to.data(), sep_pos);
		dst = dst->GetNode(node_name, true);
		to.remove_prefix(sep_pos + 1);
	}
	node_name.assign(to.begin(), to.end());
	dst->GetNode(node_name, true)->CopyFrom(*node);
	return true;
}

unsigned MemoryKey::GetValueCount() const
{
	if (!m_node)
		return 0;

	return m_node->ValueCount();
}

unsigned MemoryKey::GetKeyCount() const
{
	if (!m_node)
		return 0;

	return m_node->NodeCount();
}

void MemoryKey::ResetValues()
{
	m_value_index = 0;
}

int32_t MemoryKey::NextValue(std::unique_ptr<void, free_deleter>& buffer, RegistryVT type, std::string& name)
{
	if (!m_node)
		return 0;

	auto value = m_node->GetValue(m_value_index++, name);
	void* p_data = nullptr;
	const auto res = ParseValue_Memory(std::move(value), type, p_data, std::numeric_limits<int32_t>::max());
	if (p_data)
		buffer.reset(p_data);
	return res;
}

int32_t MemoryKey::NextValueAndType(std::unique_ptr<void, free_deleter>& buffer, RegistryVT& type, std::string& name)
{
	if (!m_node)
		return 0;

	auto value = m_node->GetValue(m_value_index++, name);
	if (!GetBestType_Memory(value, type))
		return 0;
	void* p_data = nullptr;
	const auto res = ParseValue_Memory(std::move(value), type, p_data, std::numeric_limits<int32_t>::max());
	if (p_data)
		buffer.reset(p_data);
	return res;
}

bool MemoryKey::NextString(std::string& data, std::string& name)
{
	if (!m_node)
		return 0;

	auto value = m_node->GetValue(m_value_index++, name);

	return ParseValue_Memory(std::move(value), data);
}

void MemoryKey::ResetKey()
{
	m_subkey_index = 0;
}

key_ptr MemoryKey::NextKey(bool read_only)
{
	if (!m_node)
		return nullptr;

	std::string node_name;
	auto node = m_node->GetNode(m_subkey_index++, node_name);
	if (!node)
		return nullptr;
	return vs::make_unique<MemoryKey>(std::move(node), std::move(node_name), read_only);
}

const char* MemoryKey::GetName() const
{
	return m_name.c_str();
}

std::string MemoryKey::GetLastWriteTime() const
{
	return {};
}

bool MemoryBackend::Init(string_view)
{
	m_root = std::make_shared<Node>();
	return true;
}

key_ptr MemoryBackend::Create(string_view root, bool, string_view name, bool read_only, bool create)
{
	if (!IsKeyNameValid(root) || !IsKeyNameValid(name))
		return vs::make_unique<KeyStub>();

	create = create && !read_only;

	std::string node_name;
	node_name.reserve(std::max(root.size(), name.size()));
	node_ptr node = m_root;

	typedef boost::algorithm::find_iterator<string_view::const_iterator> find_iterator;
	for (find_iterator name_it(root, boost::algorithm::token_finder([](char x) { return x != '\\'; }, boost::algorithm::token_compress_on)); !name_it.eof(); ++name_it)
	{
		node_name.assign(name_it->begin(), name_it->end());
		node = node->GetNode(node_name, create);
		if (!node)
			return vs::make_unique<KeyStub>();
	}
	for (find_iterator name_it(name, boost::algorithm::token_finder([](char x) { return x != '\\'; }, boost::algorithm::token_compress_on)); !name_it.eof(); ++name_it)
	{
		node_name.assign(name_it->begin(), name_it->end());
		node = node->GetNode(node_name, create);
		if (!node)
			return vs::make_unique<KeyStub>();
	}
	// At this point node_name contains name of the last key, which is exactly what we need to pass to MemoryKey ctor.

	return vs::make_unique<MemoryKey>(std::move(node), std::move(node_name), read_only);
}

void MemoryBackend::TEST_DumpData()
{
	m_root->DumpData(std::cerr, 2, 0);
}

}
