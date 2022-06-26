#include "EndpointRegistry.h"
#include "SecureLib/VS_SymmetricCrypt.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "std-generic/cpplib/VS_Container.h"
#include "../std/cpplib/VS_Utils.h"
#include "../std/VS_RegServer.h"

#include <boost/algorithm/string/predicate.hpp>

#include "std-generic/compat/memory.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <limits>

namespace net { namespace endpoint {

const char protocol_tcp[] = "Tcp";
const char protocol_socks[] = "Socks";
const char protocol_http_tnl[] = "HttpTnl";
const char protocol_http_tnl_msq[] = "HttpTnlMsq";
const char protocol_ssl[] = "Ssl";
const char protocol_socks_ssl[] = "SocksSsl";
const char protocol_https[] = "Https";
const char protocol_http[] = "Http";
const char protocol_internet_options[] = "InternetOptions";
const char protocol_nhp[] = "Nhp";
const char protocol_direct_udp[] = "DirectUdp";
const char protocol_ras[] = "Raw";
const char protocol_t120[] = "T120";

static const char c_permanent_marker[] = "This Endpoint's Key Is Permanent";
static const string_view c_key_endpoints = "Endpoints";
static const string_view c_key_connect_tcp = "ConnectTCP";
static const string_view c_key_accept_tcp = "AcceptTCP";
static const string_view c_key_connect_udp = "ConnectUDP";
static const string_view c_key_accept_udp = "AcceptUDP";
static const char c_value_permanent[] = "Permanent";
static const char c_value_host[] = "Host";
static const char c_value_port[] = "Port";
static const char c_value_protocol[] = "Protocol";
static const char c_value_socks_proxy_host[] = "Socks";
static const char c_value_socks_proxy_port[] = "Socks Port";
static const char c_value_socks_proxy_user[] = "Socks User";
static const char c_value_socks_proxy_password[] = "Socks Password";
static const char c_value_socks_proxy_version[] = "Socks Version";
static const char c_value_http_proxy_host[] = "HTTP Proxy";
static const char c_value_http_proxy_port[] = "HTTP Proxy Port";
static const char c_value_http_proxy_user[] = "HTTP Proxy User";
static const char c_value_http_proxy_password[] = "HTTP Proxy Password";
static const char c_value_proxy_info[] = "Proxy Info";

static const auto crypt_key = []() {
	auto v = VS_GetPersistentKey();
	for (auto& x : v)
		x = ~x;
	return v;
}();

ConnectTCPFromIDResult GetConnectTCPFromID(string_view id)
{
	if (id.empty())
		return ConnectTCPFromIDResult(false, ConnectTCP());
	const auto type_sep_pos = id.find('#');
	const auto type = id.substr(type_sep_pos != id.npos ? type_sep_pos + 1 : id.size());
	auto server_str = id.substr(0, type_sep_pos);
	net::port port = 0;
	std::string tmp_id;
	if (type.empty())
	{
		port = 4307;
		tmp_id += id;
		tmp_id += "#as";
		id = tmp_id;
	}
	else if (boost::iequals(type, "sm"))
		port = 4310;
	else if (boost::iequals(type, "bs"))
		port = 4308;
	else if (boost::iequals(type, "as") || boost::iequals(type, "vcs"))
		port = 4307;
	else if (boost::iequals(type, "rs"))
		port = 4309;
	else if (boost::iequals(type, "regs"))
	{
		server_str = RegServerHost;
		port = RegServerPort;
	}
	return {true, ConnectTCP(std::string(server_str), port, net::endpoint::protocol_tcp) };
}

bool CreateFromID(string_view id, bool user)
{
	if (id.empty())
		return false;

	auto res = GetConnectTCPFromID(id);
	if (!res.res)
		return false;

	ClearAllConnectTCP(id, user);
	AddConnectTCP(res.connect_tcp, id, user);
	if (user && res.connect_tcp.port == 4307)
	{
		AddConnectTCP(ConnectTCP{ res.connect_tcp.host, 443, net::endpoint::protocol_tcp }, id, user);
		AddConnectTCP(ConnectTCP{ res.connect_tcp.host, 80, net::endpoint::protocol_tcp }, id, user);
	}
	return true;
}

inline size_t GetSerializedSize(const std::string& x)
{
	return 2 + std::min<size_t>(x.size(), 255);
}

inline void DoSerialize(const std::string& x, uint8_t*& p)
{
	*p++ = static_cast<uint8_t>(x.size());
	std::memcpy(p, x.data(), x.size());
	p += x.size();
	*p++ = 0;
}

inline bool DoDeserialize(std::string& x, const uint8_t*& p, const uint8_t* p_end)
{
	if (p + 1 > p_end)
		return false;

	const size_t str_size = *p;
	if (p + 2 + str_size > p_end)
		return false;

	x.assign(reinterpret_cast<const char*>(p+1), str_size);
	p += 2 + str_size;
	return true;
}

inline void DoSerialize(uint16_t x, uint8_t*& p)
{
	*reinterpret_cast<uint16_t*>(p) = x;
	p += 2;
}

inline bool DoDeserialize(uint16_t& x, const uint8_t*& p, const uint8_t* p_end)
{
	if (p + 2 > p_end)
		return false;
	x = *reinterpret_cast<const uint16_t*>(p);
	p += 2;
	return true;
}

inline void DoSerialize(uint8_t x, uint8_t*& p)
{
	*reinterpret_cast<uint8_t*>(p) = x;
	p += 1;
}

inline bool DoDeserialize(uint8_t& x, const uint8_t*& p, const uint8_t* p_end)
{
	if (p + 1 > p_end)
		return false;
	x = *reinterpret_cast<const uint8_t*>(p);
	p += 1;
	return true;
}

bool ConnectTCP::Serialize(void* buffer, size_t& size) const
{
	const size_t required_size = 4
		+ GetSerializedSize(host) + 2 + GetSerializedSize(protocol_name)
		+ GetSerializedSize(socks_host) + 2 + GetSerializedSize(socks_user) + GetSerializedSize(socks_password) + 1
		+ GetSerializedSize(http_host) + 2 + GetSerializedSize(http_user) + GetSerializedSize(http_password)
	;

	if (!buffer || size < required_size)
	{
		size = required_size;
		return false;
	}

	auto p = reinterpret_cast<uint8_t*>(buffer);
	p += 4;
	DoSerialize(host, p);
	DoSerialize(port, p);
	DoSerialize(protocol_name, p);
	DoSerialize(socks_host, p);
	DoSerialize(socks_port, p);
	DoSerialize(socks_user, p);
	DoSerialize(socks_password, p);
	DoSerialize(socks_version, p);
	DoSerialize(http_host, p);
	DoSerialize(http_port, p);
	DoSerialize(http_user, p);
	DoSerialize(http_password, p);

	p = reinterpret_cast<uint8_t*>(buffer);
	*reinterpret_cast<uint32_t*>(p) = VS_SimpleChkSum(p + 4, required_size - 4);

	size = required_size;
	return true;
}

std::vector<uint8_t> ConnectTCP::Serialize() const
{
	std::vector<uint8_t> result;

	size_t size;
	Serialize(nullptr, size);
	result.resize(size);
	if (!Serialize(result.data(), size))
		result.clear();
	return result;
}

bool ConnectTCP::Deserialize(const void* buffer, size_t size)
{
	if (!buffer || size < 4 + (2 + 2 + 2)) // minimal possible size
		return false;

	auto p = reinterpret_cast<const uint8_t*>(buffer);
	const auto p_end = p + size;

	if (*reinterpret_cast<const uint32_t*>(p) != VS_SimpleChkSum(p + 4, size - 4))
		return false;
	p += 4;

	if (!DoDeserialize(host, p, p_end)) return false;
	if (!DoDeserialize(port, p, p_end)) return false;
	if (!DoDeserialize(protocol_name, p, p_end)) return false;

	if (p == p_end)
		return true;
	if (!DoDeserialize(socks_host, p, p_end)) return false;
	if (!DoDeserialize(socks_port, p, p_end)) return false;
	if (!DoDeserialize(socks_user, p, p_end)) return false;
	if (!DoDeserialize(socks_password, p, p_end)) return false;
	if (!DoDeserialize(socks_version, p, p_end)) return false;
	if (!DoDeserialize(http_host, p, p_end)) return false;
	if (!DoDeserialize(http_port, p, p_end)) return false;
	if (!DoDeserialize(http_user, p, p_end)) return false;
	if (!DoDeserialize(http_password, p, p_end)) return false;

	return true;
}

bool AcceptTCP::Serialize(void* buffer, size_t& size) const
{
	const size_t required_size = 4
		+ GetSerializedSize(host) + 2 + GetSerializedSize(protocol_name)
	;

	if (!buffer || size < required_size)
	{
		size = required_size;
		return false;
	}

	auto p = reinterpret_cast<uint8_t*>(buffer);
	p += 4;
	DoSerialize(host, p);
	DoSerialize(port, p);
	DoSerialize(protocol_name, p);

	p = reinterpret_cast<uint8_t*>(buffer);
	*reinterpret_cast<uint32_t*>(p) = VS_SimpleChkSum(p + 4, required_size - 4);

	size = required_size;
	return true;
}

std::vector<uint8_t> AcceptTCP::Serialize() const
{
	std::vector<uint8_t> result;

	size_t size;
	Serialize(nullptr, size);
	result.resize(size);
	if (!Serialize(result.data(), size))
		result.clear();
	return result;
}

bool AcceptTCP::Deserialize(const void* buffer, size_t size)
{
	if (!buffer || size < 4 + (2 + 2 + 2)) // minimal possible size
		return false;

	auto p = reinterpret_cast<const uint8_t*>(buffer);
	const auto p_end = p + size;

	if (*reinterpret_cast<const uint32_t*>(p) != VS_SimpleChkSum(p + 4, size - 4))
		return false;
	p += 4;

	if (!DoDeserialize(host, p, p_end)) return false;
	if (!DoDeserialize(port, p, p_end)) return false;
	if (!DoDeserialize(protocol_name, p, p_end)) return false;

	return true;
}

bool AcceptUDP::Serialize(void* buffer, size_t& size) const
{
	const size_t required_size = 4
		+ GetSerializedSize(host) + 2 + GetSerializedSize(protocol_name)
	;

	if (!buffer || size < required_size)
	{
		size = required_size;
		return false;
	}

	auto p = reinterpret_cast<uint8_t*>(buffer);
	p += 4;
	DoSerialize(host, p);
	DoSerialize(port, p);
	DoSerialize(protocol_name, p);

	p = reinterpret_cast<uint8_t*>(buffer);
	*reinterpret_cast<uint32_t*>(p) = VS_SimpleChkSum(p + 4, required_size - 4);

	size = required_size;
	return true;
}

std::vector<uint8_t> AcceptUDP::Serialize() const
{
	std::vector<uint8_t> result;

	size_t size;
	Serialize(nullptr, size);
	result.resize(size);
	if (!Serialize(result.data(), size))
		result.clear();
	return result;
}

bool AcceptUDP::Deserialize(const void* buffer, size_t size)
{
	if (!buffer || size < 4 + (2 + 2 + 2)) // minimal possible size
		return false;

	auto p = reinterpret_cast<const uint8_t*>(buffer);
	const auto p_end = p + size;

	if (*reinterpret_cast<const uint32_t*>(p) != VS_SimpleChkSum(p + 4, size - 4))
		return false;
	p += 4;

	if (!DoDeserialize(host, p, p_end)) return false;
	if (!DoDeserialize(port, p, p_end)) return false;
	if (!DoDeserialize(protocol_name, p, p_end)) return false;

	return true;
}

std::vector<uint8_t> Serialize(bool is_connect_keys, string_view endpoint_name, bool is_current_user)
{
	std::vector<uint8_t> result;

	if (is_connect_keys)
	{
		const auto n_entries = GetCountConnectTCP(endpoint_name, is_current_user);
		if (!n_entries)
			return result;

		std::vector<std::unique_ptr<ConnectTCP>> entries;
		entries.reserve(n_entries);
		for (unsigned i = 1; i <= n_entries; ++i)
			entries.emplace_back(ReadConnectTCP(i, endpoint_name, is_current_user));

		size_t required_size = 4 + 4;
		for (const auto& x : entries)
		{
			if (!x)
				continue;

			size_t entry_size;
			x->Serialize(nullptr, entry_size);
			required_size += 4 + entry_size;
		}

		result.resize(required_size);
		auto p = reinterpret_cast<uint8_t*>(result.data());
		const auto p_end = p + required_size;

		p += 4;
		*reinterpret_cast<uint32_t*>(p) = n_entries;
		p += 4;
		for (const auto& x : entries)
		{
			if (!x)
				continue;

			size_t entry_size = p_end - (p + 4);
			if (!x->Serialize(p + 4, entry_size))
			{
				result.clear();
				return result;
			}
			*reinterpret_cast<uint32_t*>(p) = entry_size;
			p += 4 + entry_size;
		}
		assert(p == p_end);

		p = reinterpret_cast<uint8_t*>(result.data());
		*reinterpret_cast<uint32_t*>(p) = VS_SimpleChkSum(p + 4, required_size - 4);
	}
	else
	{
		const auto n_entries = GetCountAcceptTCP(endpoint_name, is_current_user);
		if (!n_entries)
			return result;

		std::vector<std::unique_ptr<AcceptTCP>> entries;
		entries.reserve(n_entries);
		for (unsigned i = 1; i <= n_entries; ++i)
			entries.emplace_back(ReadAcceptTCP(i, endpoint_name, is_current_user));

		size_t required_size = 4 + 4;
		for (const auto& x : entries)
		{
			if (!x)
				continue;

			size_t entry_size;
			x->Serialize(nullptr, entry_size);
			required_size += 4 + entry_size;
		}

		result.resize(required_size);
		auto p = reinterpret_cast<uint8_t*>(result.data());
		const auto p_end = p + required_size;

		p += 4;
		*reinterpret_cast<uint32_t*>(p) = n_entries;
		p += 4;
		for (const auto& x : entries)
		{
			if (!x)
				continue;

			size_t entry_size = p_end - (p + 4);
			if (!x->Serialize(p + 4, entry_size))
			{
				result.clear();
				return result;
			}
			*reinterpret_cast<uint32_t*>(p) = entry_size;
			p += 4 + entry_size;
		}
		assert(p == p_end);

		p = reinterpret_cast<uint8_t*>(result.data());
		*reinterpret_cast<uint32_t*>(p) = VS_SimpleChkSum(p + 4, required_size - 4);
	}

	return result;
}

template <class F>
inline bool IterateBuffer(const void* buffer, size_t size, F&& f)
{
	if (!buffer || size < 4 + 4)
		return false;

	auto p = reinterpret_cast<const uint8_t*>(buffer);
	const auto p_end = p + size;

	if (*reinterpret_cast<const uint32_t*>(p) != VS_SimpleChkSum(p + 4, size - 4))
		return false;
	p += 4;

	const unsigned n_entries = *reinterpret_cast<const uint32_t*>(p);
	p += 4;

	for (unsigned i = 0; i < n_entries; ++i)
	{
		if (p + 4 > p_end)
			break;

		const size_t entry_size = *reinterpret_cast<const uint32_t*>(p);
		p += 4;

		if (p + entry_size > p_end)
			break;

		if (!f(i, p, entry_size))
			break;

		p += entry_size;
	}
	return true;
}

bool Deserialize(bool is_connect_keys, const void* buffer, size_t size, string_view endpoint_name, bool is_current_user)
{
	unsigned n_loaded = 0;
	bool is_valid;
	if (is_connect_keys)
	{
		is_valid = IterateBuffer(buffer, size, [&] (unsigned, const void* entry_buffer, size_t entry_size)
		{
			ConnectTCP x;
			if (!x.Deserialize(entry_buffer, entry_size))
				return true;
			AddConnectTCP(x, endpoint_name, is_current_user);
			++n_loaded;
			return true;
		});
	}
	else
	{
		is_valid = IterateBuffer(buffer, size, [&] (unsigned, const void* entry_buffer, size_t entry_size)
		{
			AcceptTCP x;
			if (!x.Deserialize(entry_buffer, entry_size))
				return true;
			AddAcceptTCP(x, endpoint_name, is_current_user);
			++n_loaded;
			return true;
		});
	}

	return is_valid && n_loaded > 0;
}

bool GetFromBuffer(unsigned index, ConnectTCP& c_tcp, const void* buffer, size_t size)
{
	bool loaded = false;
	IterateBuffer(buffer, size, [&] (unsigned i, const void* entry_buffer, size_t entry_size)
	{
		if (i < index)
			return true;

		loaded = c_tcp.Deserialize(entry_buffer, entry_size);
		return false;
	});
	return loaded;
}

bool GetFromBuffer(unsigned index, AcceptTCP& a_tcp, const void* buffer, size_t size)
{
	bool loaded = false;
	IterateBuffer(buffer, size, [&] (unsigned i, const void* entry_buffer, size_t entry_size)
	{
		if (i < index)
			return true;

		loaded = a_tcp.Deserialize(entry_buffer, entry_size);
		return false;
	});
	return loaded;
}

bool GetFromBuffer(unsigned index, AcceptUDP& a_udp, const void* buffer, size_t size)
{
	bool loaded = false;
	IterateBuffer(buffer, size, [&] (unsigned i, const void* entry_buffer, size_t entry_size)
	{
		if (i < index)
			return true;

		loaded = a_udp.Deserialize(entry_buffer, entry_size);
		return false;
	});
	return loaded;
}

unsigned GetCountConnectTCPFromBuffer(const void* buffer, size_t size)
{
	unsigned n_loaded = 0;
	const bool is_valid = IterateBuffer(buffer, size, [&] (unsigned, const void* entry_buffer, size_t entry_size)
	{
		ConnectTCP x;
		if (x.Deserialize(entry_buffer, entry_size))
			++n_loaded;
		return true;
	});
	return is_valid ? n_loaded : 0;
}

unsigned GetCountAcceptTCPFromBuffer(const void* buffer, size_t size)
{
	unsigned n_loaded = 0;
	const bool is_valid = IterateBuffer(buffer, size, [&] (unsigned, const void* entry_buffer, size_t entry_size)
	{
		AcceptTCP x;
		if (x.Deserialize(entry_buffer, entry_size))
			++n_loaded;
		return true;
	});
	return is_valid ? n_loaded : 0;
}

unsigned GetCountAcceptUDPFromBuffer(const void* buffer, size_t size)
{
	unsigned n_loaded = 0;
	const bool is_valid = IterateBuffer(buffer, size, [&] (unsigned, const void* entry_buffer, size_t entry_size)
	{
		AcceptUDP x;
		if (x.Deserialize(entry_buffer, entry_size))
			++n_loaded;
		return true;
	});
	return is_valid ? n_loaded : 0;
}

void Remove(string_view endpoint_name, bool is_current_user)
{
	if (endpoint_name.empty())
		return;

	VS_RegistryKey key(is_current_user, c_key_endpoints, false);
	if (!key.IsValid())
		return;

	key.RemoveKey(endpoint_name);
}

inline std::string EndpointKeyName(string_view endpoint_name)
{
	std::string name;
	name.reserve(c_key_endpoints.size() + 1 + endpoint_name.size());
	name += c_key_endpoints;
	name += '\\';
	name += endpoint_name;
	return name;
}

inline std::string EndpointEntryKeyName(string_view endpoint_name, string_view type_prefix, unsigned index)
{
	const size_t max_index_size = std::numeric_limits<unsigned>::digits10 + 1;

	std::string name;
	name.reserve(c_key_endpoints.size() + 1 + endpoint_name.size() + 1 + type_prefix.size() + max_index_size);
	name += c_key_endpoints;
	name += '\\';
	name += endpoint_name;
	name += '\\';
	name += type_prefix;
	char index_str[max_index_size + 1];
	std::sprintf(index_str, "%u", index);
	name += index_str;
	return name;
}

bool IsPermanent(string_view endpoint_name, bool is_current_user)
{
	if (endpoint_name.empty())
		return false;

	VS_RegistryKey key(is_current_user, EndpointKeyName(endpoint_name));
	if (!key.IsValid())
		return false;

	std::string value;
	key.GetString(value, c_value_permanent);
	return value == c_permanent_marker;
}

void SetAsPermanent(string_view endpoint_name, bool is_current_user)
{
	if (endpoint_name.empty())
		return;

	VS_RegistryKey key(is_current_user, EndpointKeyName(endpoint_name), false);
	if (!key.IsValid())
		return;

	key.SetString(c_permanent_marker, c_value_permanent);
}

void ResetAsPermanent(string_view endpoint_name, bool is_current_user)
{
	if (endpoint_name.empty())
		return;

	VS_RegistryKey key(is_current_user, EndpointKeyName(endpoint_name), false);
	if (!key.IsValid())
		return;

	key.RemoveValue(c_value_permanent);
}

inline bool CanCryptProxyInfo()
{
	return !crypt_key.empty();
}

inline bool ReadCryptedProxyInfo(const VS_RegistryKey& key, VS_Container& cnt)
{
	if (crypt_key.empty() || !key.IsValid())
		return false;

	std::unique_ptr<unsigned char, free_deleter> encr_data;
	const auto ret = key.GetValue(encr_data, VS_REG_BINARY_VT, c_value_proxy_info);
	if (ret <= 0)
		return false;

	const auto encr_size = static_cast<size_t>(ret);

	VS_SymmetricCrypt crypt;
	if (!crypt.Init(alg_sym_AES256, mode_CBC) || !crypt.SetKey(crypt_key.size(), crypt_key.data()))
		return false;

	uint32_t decr_size = 0;
	crypt.Decrypt(encr_data.get(), encr_size, nullptr, &decr_size);
	if (decr_size == 0)
		return false;

	auto decr_data = vs::make_unique<unsigned char[]>(decr_size);
	if (!crypt.Decrypt(encr_data.get(), encr_size, decr_data.get(), &decr_size))
		return false;
	return cnt.Deserialize(decr_data.get(), decr_size);
}

inline bool WriteCryptedProxyInfo(VS_RegistryKey& key, const VS_Container& cnt)
{
	VS_SymmetricCrypt crypt;
	if (!crypt.Init(alg_sym_AES256, mode_CBC) || !crypt.SetKey(crypt_key.size(), crypt_key.data()))
		return false;

	size_t decr_size = 0;
	cnt.Serialize(nullptr, decr_size);
	auto decr_data = vs::make_unique<unsigned char[]>(decr_size);
	if (!cnt.Serialize(decr_data.get(), decr_size))
		return false;

	uint32_t encr_size = 0;
	crypt.Encrypt(decr_data.get(), decr_size, nullptr, &encr_size);
	if (encr_size == 0)
		return false;

	auto encr_data = vs::make_unique<unsigned char[]>(encr_size);
	if (!crypt.Encrypt(decr_data.get(), decr_size, encr_data.get(), &encr_size))
		return false;

	return key.SetValue(encr_data.get(), encr_size, VS_REG_BINARY_VT, c_value_proxy_info);
}

inline unsigned GetCount(string_view endpoint_name, bool is_current_user, string_view type_prefix)
{
	if (endpoint_name.empty())
		return 0;

	const size_t max_index_size = std::numeric_limits<unsigned>::digits10 + 1;
	std::string key_name;
	key_name.reserve(c_key_endpoints.size() + 1 + endpoint_name.size() + 1 + type_prefix.size() + max_index_size);
	key_name += c_key_endpoints;
	key_name += '\\';
	key_name += endpoint_name;
	key_name += '\\';
	key_name += type_prefix;
	const auto index_start_pos = key_name.size();
	char index_str[max_index_size + 1];

	unsigned n_entries = 0;
	while (true)
	{
		sprintf(index_str, "%u", n_entries + 1);
		key_name.replace(index_start_pos, key_name.npos, index_str);
		VS_RegistryKey key(is_current_user, key_name);
		if (!key.IsValid())
			break;
		++n_entries;
	}
	return n_entries;
}

inline void MakeFirst(unsigned index, string_view endpoint_name, bool is_current_user, string_view type_prefix)
{
	if (endpoint_name.empty())
		return;

	if (index < 2)
		return;

	const unsigned n_entries = GetCount(endpoint_name, is_current_user, type_prefix);
	if (index > n_entries)
		return;

	VS_RegistryKey key(is_current_user, EndpointKeyName(endpoint_name), false);
	if (!key.IsValid())
		return;

	const size_t max_index_size = std::numeric_limits<unsigned>::digits10 + 1;
	std::string src_key_name;
	src_key_name.reserve(type_prefix.size() + max_index_size);
	src_key_name += type_prefix;
	const auto src_index_start_pos = src_key_name.size();
	std::string dst_key_name;
	dst_key_name.reserve(type_prefix.size() + max_index_size);
	dst_key_name += type_prefix;
	const auto dst_index_start_pos = dst_key_name.size();
	char index_str[max_index_size + 1];

	// Rename index key to temporary name
	sprintf(index_str, "%u", index);
	src_key_name.replace(src_index_start_pos, src_key_name.npos, index_str);
	dst_key_name.replace(dst_index_start_pos, dst_key_name.npos, "_TMP");
	key.RenameKey(src_key_name, dst_key_name);

	// Shift [1, index) keys by +1
	for (unsigned i = index - 1; i >= 1; --i)
	{
		sprintf(index_str, "%u", i);
		src_key_name.replace(src_index_start_pos, src_key_name.npos, index_str);
		sprintf(index_str, "%u", i + 1);
		dst_key_name.replace(dst_index_start_pos, dst_key_name.npos, index_str);
		key.RenameKey(src_key_name, dst_key_name);
	}

	// Rename from temporary name to first key
	src_key_name.replace(src_index_start_pos, src_key_name.npos, "_TMP");
	dst_key_name.replace(dst_index_start_pos, dst_key_name.npos, "1");
	key.RenameKey(src_key_name, dst_key_name);
}

inline void Delete(unsigned index, string_view endpoint_name, bool is_current_user, string_view type_prefix)
{
	if (endpoint_name.empty())
		return;

	if (index < 1)
		return;

	const unsigned n_entries = GetCount(endpoint_name, is_current_user, type_prefix);
	if (index > n_entries)
		return;

	VS_RegistryKey key(is_current_user, EndpointKeyName(endpoint_name), false);
	if (!key.IsValid())
		return;

	const size_t max_index_size = std::numeric_limits<unsigned>::digits10 + 1;
	std::string src_key_name;
	src_key_name.reserve(type_prefix.size() + max_index_size);
	src_key_name += type_prefix;
	const auto src_index_start_pos = src_key_name.size();
	std::string dst_key_name;
	dst_key_name.reserve(type_prefix.size() + max_index_size);
	dst_key_name += type_prefix;
	const auto dst_index_start_pos = dst_key_name.size();
	char index_str[max_index_size + 1];

	// Remove index key
	sprintf(index_str, "%u", index);
	src_key_name.replace(src_index_start_pos, src_key_name.npos, index_str);
	key.RemoveKey(src_key_name);

	// Shift [index + 1, n_entries] keys by -1
	for (unsigned i = index + 1; i <= n_entries; ++i)
	{
		sprintf(index_str, "%u", i);
		src_key_name.replace(src_index_start_pos, src_key_name.npos, index_str);
		sprintf(index_str, "%u", i - 1);
		dst_key_name.replace(dst_index_start_pos, dst_key_name.npos, index_str);
		key.RenameKey(src_key_name, dst_key_name);
	}
}

inline void ClearAll(string_view endpoint_name, bool is_current_user, string_view type_prefix)
{
	if (endpoint_name.empty())
		return;

	VS_RegistryKey key(is_current_user, EndpointKeyName(endpoint_name), false);
	if (!key.IsValid())
		return;

	const size_t max_index_size = std::numeric_limits<unsigned>::digits10 + 1;
	std::string key_name;
	key_name.reserve(type_prefix.size() + max_index_size);
	key_name += type_prefix;
	const auto index_start_pos = key_name.size();
	char index_str[max_index_size + 1];

	const unsigned n_entries = GetCount(endpoint_name, is_current_user, type_prefix);

	for (unsigned i = 1; i <= n_entries; ++i)
	{
		sprintf(index_str, "%u", i);
		key_name.replace(index_start_pos, key_name.npos, index_str);
		sprintf(index_str, "%u", i - 1);
		key.RemoveKey(key_name);
	}
}

inline bool StoreInRegistry(const std::string& value, const char* name, VS_RegistryKey& key)
{
	if (!value.empty())
		return key.SetString(value.c_str(), name);
	else
	{
		key.RemoveValue(name);
		return true;
	}
}

inline bool StoreInRegistry(int32_t value, const char* name, VS_RegistryKey& key)
{
	if (value != 0)
		return key.SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, name);
	else
	{
		key.RemoveValue(name);
		return true;
	}
}

inline bool StoreInContainer(const std::string& value, const char* name, VS_Container& cnt)
{
	if (!value.empty())
		return cnt.AddValue(name, value);
	return true;
}

inline bool StoreInContainer(int32_t value, const char* name, VS_Container& cnt)
{
	if (value != 0)
		return cnt.AddValue(name, value);
	return true;
}

unsigned GetCountConnectTCP(string_view endpoint_name, bool is_current_user)
{
	return GetCount(endpoint_name, is_current_user, c_key_connect_tcp);
}

std::unique_ptr<ConnectTCP> ReadConnectTCP(unsigned index, string_view endpoint_name, bool is_current_user)
{
	if (endpoint_name.empty())
		return nullptr;

	VS_RegistryKey key(is_current_user, EndpointEntryKeyName(endpoint_name, c_key_connect_tcp, index));
	if (!key.IsValid())
		return nullptr;

	auto result = vs::make_unique<ConnectTCP>();
	int32_t number;

	key.GetString(result->host, c_value_host);
	if (key.GetValue(&number, sizeof(number), VS_REG_INTEGER_VT, c_value_port) > 0)
		result->port = number;
	key.GetString(result->protocol_name, c_value_protocol);
	key.GetString(result->socks_host, c_value_socks_proxy_host);
	if (key.GetValue(&number, sizeof(number), VS_REG_INTEGER_VT, c_value_socks_proxy_port) > 0)
		result->socks_port = number;
	if (key.GetValue(&number, sizeof(number), VS_REG_INTEGER_VT, c_value_socks_proxy_version) > 0)
		result->socks_version = number;
	key.GetString(result->http_host, c_value_http_proxy_host);
	if (key.GetValue(&number, sizeof(number), VS_REG_INTEGER_VT, c_value_http_proxy_port) > 0)
		result->http_port = number;

	VS_Container cnt;
	if (ReadCryptedProxyInfo(key, cnt) && !cnt.IsEmpty())
	{
		const char* value;
		value = cnt.GetStrValueRef(c_value_socks_proxy_user);
		if (value)
			result->socks_user = value;
		value = cnt.GetStrValueRef(c_value_socks_proxy_password);
		if (value)
			result->socks_password = value;
		value = cnt.GetStrValueRef(c_value_http_proxy_user);
		if (value)
			result->http_user = value;
		value = cnt.GetStrValueRef(c_value_http_proxy_password);
		if (value)
			result->http_password = value;
	}
	else
	{
		key.GetString(result->socks_user, c_value_socks_proxy_user);
		key.GetString(result->socks_password, c_value_socks_proxy_password);
		key.GetString(result->http_user, c_value_http_proxy_user);
		key.GetString(result->http_password, c_value_http_proxy_password);
	}
	return result;
}

void MakeFirstConnectTCP(unsigned index, string_view endpoint_name, bool is_current_user)
{
	MakeFirst(index, endpoint_name, is_current_user, c_key_connect_tcp);
}

unsigned AddConnectTCP(const ConnectTCP& c_tcp, string_view endpoint_name, bool is_current_user)
{
	unsigned n_entries = GetCountConnectTCP(endpoint_name, is_current_user);
	if (UpdateConnectTCP(n_entries + 1, c_tcp, endpoint_name, is_current_user))
		++n_entries;
	return n_entries;
}

void AddFirstConnectTCP(const ConnectTCP& c_tcp, string_view endpoint_name, bool is_current_user)
{
	MakeFirstConnectTCP(AddConnectTCP(c_tcp, endpoint_name, is_current_user), endpoint_name, is_current_user);
}

void DeleteConnectTCP(unsigned index, string_view endpoint_name, bool is_current_user)
{
	Delete(index, endpoint_name, is_current_user, c_key_connect_tcp);
}

bool UpdateConnectTCP(unsigned index, const ConnectTCP& c_tcp, string_view endpoint_name, bool is_current_user)
{
	if (endpoint_name.empty())
		return false;

	VS_RegistryKey key(is_current_user, EndpointEntryKeyName(endpoint_name, c_key_connect_tcp, index), false, true);
	if (!key.IsValid())
		return false;

	bool result = true;
	result = StoreInRegistry(c_tcp.host, c_value_host, key) && result;
	result = StoreInRegistry(c_tcp.port, c_value_port, key) && result;
	result = StoreInRegistry(c_tcp.protocol_name, c_value_protocol, key) && result;
	result = StoreInRegistry(c_tcp.socks_host, c_value_socks_proxy_host, key) && result;
	result = StoreInRegistry(c_tcp.socks_port, c_value_socks_proxy_port, key) && result;
	result = StoreInRegistry(c_tcp.socks_version, c_value_socks_proxy_version, key) && result;
	result = StoreInRegistry(c_tcp.http_host, c_value_http_proxy_host, key) && result;
	result = StoreInRegistry(c_tcp.http_port, c_value_http_proxy_port, key) && result;
	if (CanCryptProxyInfo())
	{
		VS_Container cnt;
		result = StoreInContainer(c_tcp.socks_user, c_value_socks_proxy_user, cnt) && result;
		result = StoreInContainer(c_tcp.socks_password, c_value_socks_proxy_password, cnt) && result;
		result = StoreInContainer(c_tcp.http_user, c_value_http_proxy_user, cnt) && result;
		result = StoreInContainer(c_tcp.http_password, c_value_http_proxy_password, cnt) && result;
		result = WriteCryptedProxyInfo(key, cnt) && result;
	}
	else
	{
		result = StoreInRegistry(c_tcp.socks_user, c_value_socks_proxy_user, key) && result;
		result = StoreInRegistry(c_tcp.socks_password, c_value_socks_proxy_password, key) && result;
		result = StoreInRegistry(c_tcp.http_user, c_value_http_proxy_user, key) && result;
		result = StoreInRegistry(c_tcp.http_password, c_value_http_proxy_password, key) && result;
	}
	return result;
}

void ClearAllConnectTCP(string_view endpoint_name, bool is_current_user)
{
	ClearAll(endpoint_name, is_current_user, c_key_connect_tcp);
}

unsigned GetCountAcceptTCP(string_view endpoint_name, bool is_current_user)
{
	return GetCount(endpoint_name, is_current_user, c_key_accept_tcp);
}

std::unique_ptr<AcceptTCP> ReadAcceptTCP(unsigned index, string_view endpoint_name, bool is_current_user)
{
	if (endpoint_name.empty())
		return nullptr;

	VS_RegistryKey key(is_current_user, EndpointEntryKeyName(endpoint_name, c_key_accept_tcp, index));
	if (!key.IsValid())
		return nullptr;

	auto result = vs::make_unique<AcceptTCP>();

	int32_t number;

	key.GetString(result->host, c_value_host);
	if (key.GetValue(&number, sizeof(number), VS_REG_INTEGER_VT, c_value_port) > 0)
		result->port = number;
	key.GetString(result->protocol_name, c_value_protocol);
	return result;
}

void MakeFirstAcceptTCP(unsigned index, string_view endpoint_name, bool is_current_user)
{
	MakeFirst(index, endpoint_name, is_current_user, c_key_accept_tcp);
}

unsigned AddAcceptTCP(const AcceptTCP& a_tcp, string_view endpoint_name, bool is_current_user)
{
	unsigned n_entries = GetCountAcceptTCP(endpoint_name, is_current_user);
	if (UpdateAcceptTCP(n_entries + 1, a_tcp, endpoint_name, is_current_user))
		++n_entries;
	return n_entries;
}

void AddFirstAcceptTCP(const AcceptTCP& a_tcp, string_view endpoint_name, bool is_current_user)
{
	MakeFirstAcceptTCP(AddAcceptTCP(a_tcp, endpoint_name, is_current_user), endpoint_name, is_current_user);
}

void DeleteAcceptTCP(unsigned index, string_view endpoint_name, bool is_current_user)
{
	Delete(index, endpoint_name, is_current_user, c_key_accept_tcp);
}

bool UpdateAcceptTCP(unsigned index, const AcceptTCP& a_tcp, string_view endpoint_name, bool is_current_user)
{
	if (endpoint_name.empty())
		return false;

	VS_RegistryKey key(is_current_user, EndpointEntryKeyName(endpoint_name, c_key_accept_tcp, index), false, true);
	if (!key.IsValid())
		return false;

	bool result = true;
	result = StoreInRegistry(a_tcp.host, c_value_host, key) && result;
	result = StoreInRegistry(a_tcp.port, c_value_port, key) && result;
	result = StoreInRegistry(a_tcp.protocol_name, c_value_protocol, key) && result;
	return result;
}

void ClearAllAcceptTCP(string_view endpoint_name, bool is_current_user)
{
	ClearAll(endpoint_name, is_current_user, c_key_accept_tcp);
}

unsigned GetCountAcceptUDP(string_view endpoint_name, bool is_current_user)
{
	return GetCount(endpoint_name, is_current_user, c_key_accept_udp);
}

std::unique_ptr<AcceptUDP> ReadAcceptUDP(unsigned index, string_view endpoint_name, bool is_current_user)
{
	if (endpoint_name.empty())
		return nullptr;

	VS_RegistryKey key(is_current_user, EndpointEntryKeyName(endpoint_name, c_key_accept_udp, index));
	if (!key.IsValid())
		return nullptr;

	auto result = vs::make_unique<AcceptUDP>();

	int32_t number;

	key.GetString(result->host, c_value_host);
	if (key.GetValue(&number, sizeof(number), VS_REG_INTEGER_VT, c_value_port) > 0)
		result->port = number;
	key.GetString(result->protocol_name, c_value_protocol);
	return result;
}

void MakeFirstAcceptUDP(unsigned index, string_view endpoint_name, bool is_current_user)
{
	MakeFirst(index, endpoint_name, is_current_user, c_key_accept_udp);
}

unsigned AddAcceptUDP(const AcceptUDP& a_udp, string_view endpoint_name, bool is_current_user)
{
	unsigned n_entries = GetCountAcceptUDP(endpoint_name, is_current_user);
	if (UpdateAcceptUDP(n_entries + 1, a_udp, endpoint_name, is_current_user))
		++n_entries;
	return n_entries;
}

void AddFirstAcceptUDP(const AcceptUDP& a_udp, string_view endpoint_name, bool is_current_user)
{
	MakeFirstAcceptUDP(AddAcceptUDP(a_udp, endpoint_name, is_current_user), endpoint_name, is_current_user);
}

void DeleteAcceptUDP(unsigned index, string_view endpoint_name, bool is_current_user)
{
	Delete(index, endpoint_name, is_current_user, c_key_accept_udp);
}

bool UpdateAcceptUDP(unsigned index, const AcceptUDP& a_udp, string_view endpoint_name, bool is_current_user)
{
	if (endpoint_name.empty())
		return false;

	VS_RegistryKey key(is_current_user, EndpointEntryKeyName(endpoint_name, c_key_accept_udp, index), false, true);
	if (!key.IsValid())
		return false;

	bool result = true;
	result = StoreInRegistry(a_udp.host, c_value_host, key) && result;
	result = StoreInRegistry(a_udp.port, c_value_port, key) && result;
	result = StoreInRegistry(a_udp.protocol_name, c_value_protocol, key) && result;
	return result;
}

void ClearAllAcceptUDP(string_view endpoint_name, bool is_current_user)
{
	ClearAll(endpoint_name, is_current_user, c_key_accept_udp);
}

}}
