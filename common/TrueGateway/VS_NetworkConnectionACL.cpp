#include <algorithm>

#include "VS_NetworkConnectionACL.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/netutils.h"
#include "std-generic/cpplib/hton.h"

using namespace std;
using namespace int128_utils;
using namespace netutils;

// ACL data read buffer size
static const size_t BUFSZ = 64 * 1024; // 64 Kb

// ACL connection class
VS_NetworkConnectionACL::VS_NetworkConnectionACL()
: m_mode(ACL_NONE), m_finalized(false)
{}

VS_NetworkConnectionACL::~VS_NetworkConnectionACL()
{}


bool VS_NetworkConnectionACL::SetMode(const ACLMode acl_mode)
{
	if (m_finalized)
		return false;

	m_mode = acl_mode;

	return true;
}

VS_NetworkConnectionACL::ACLMode VS_NetworkConnectionACL::GetMode(void) const
{
	return m_mode;
}

bool VS_NetworkConnectionACL::AddEntry(const uint32_t subnet, const uint32_t mask)
{
	ACLEntry entry;

	if (m_finalized)
		return false;

	entry.type = IP_V4;
	entry.data.ipv4.subnet = subnet;
	entry.data.ipv4.mask = mask;

	if (!CheckEntry(entry))
		m_acl.push_back(entry);

	return true;
}

bool VS_NetworkConnectionACL::AddEntry(const __m128i subnet, const __m128i mask)
{
	ACLEntry entry;

	if (m_finalized)
		return false;

	entry.type = IP_V6;
	entry.data.ipv6.subnet = subnet;
	entry.data.ipv6.mask = mask;

	if (!CheckEntry(entry))
		m_acl.push_back(entry);

	return true;
}

bool VS_NetworkConnectionACL::AddEntry(const uint32_t subnet[], const uint32_t mask[])
{
	__m128i subnet_128, mask_128;

	subnet_128 = int128_load_array(subnet);
	mask_128 = int128_load_array(mask);

	return AddEntry(subnet_128, mask_128);
}

bool VS_NetworkConnectionACL::AddEntry(const ACLEntry &e)
{
	if (e.type == IP_V4)
	{
		return AddEntry(e.data.ipv4.subnet, e.data.ipv4.mask);
	}

	if (e.type == IP_V6)
	{
		return AddEntry(e.data.ipv6.subnet, e.data.ipv6.mask);
	}
	return false;
}

bool VS_NetworkConnectionACL::AddEntry(const char *subnet_cidr)
{
	ACLEntry e;

	if (ParseEntry(e, subnet_cidr))
	{
		return AddEntry(e);
	}

	return false;
}

bool VS_NetworkConnectionACL::Clear()
{
	if (m_finalized)
		return false;

	m_acl.clear();

	return true;
}

void VS_NetworkConnectionACL::Finalize()
{
	if (m_finalized)
		return;

	m_finalized = true;
}

bool VS_NetworkConnectionACL::CheckEntry(const ACLEntry &e) const
{
	auto res = find_if(m_acl.begin(), m_acl.end(), [this, &e](const ACLEntry &v) -> bool {
		return EntriesAreEqual(e, v);
	});

	if (res == m_acl.end())
		return false;

	return true;
}

bool VS_NetworkConnectionACL::HandleFoundResult(const bool found) const
{
	if (GetMode() == ACL_BLACKLIST)
	{
		if (found)
			return false;

		return true;
	}
	else if (GetMode() == ACL_WHITELIST)
	{
		if (found)
			return true;

		return false;
	}


	return false;
}

bool VS_NetworkConnectionACL::IsAllowed(const uint32_t ipv4addr_hostorder) const
{
	bool found;
	if (!m_finalized)
		return false;

	if (GetMode() == ACL_NONE)
		return true;

	auto res = find_if(m_acl.begin(), m_acl.end(), [this, &ipv4addr_hostorder](const ACLEntry &v) -> bool {
		return (v.type == IP_V4 && IsAddressInRange_IPv4(ipv4addr_hostorder, v.data.ipv4.subnet, v.data.ipv4.mask));
	});

	found = res != m_acl.end();
	return HandleFoundResult(found);
}

bool VS_NetworkConnectionACL::IsAllowed(const __m128i ipv6addr_hostorder) const
{
	if (!m_finalized)
		return false;

	if (GetMode() == ACL_NONE)
		return true;

	auto res = find_if(m_acl.begin(), m_acl.end(), [this, &ipv6addr_hostorder](const ACLEntry &v) -> bool {
		return (v.type == IP_V6 && IsAddressInRange_IPv6(ipv6addr_hostorder, v.data.ipv6.subnet, v.data.ipv6.mask));
	});

	bool found = res != m_acl.end();

	return HandleFoundResult(found);
}

bool VS_NetworkConnectionACL::IsAllowed(const uint32_t ipv6addr_hostorder[]) const
{
	return IsAllowed(int128_load_array(ipv6addr_hostorder));
}

bool VS_NetworkConnectionACL::IsAllowed(const char *addr) const
{
	IPAddress res;

	if (StringToIPAddress(addr, res))
	{
		if (res.type == IP_ADDR_V4)
		{
			return IsAllowed(res.addr.ipv4);
		}

		if (res.type == IP_ADDR_V6)
		{
			return IsAllowed(res.addr.ipv6);
		}
	}

	return false;
}

bool VS_NetworkConnectionACL::IsAllowed(const net::address &addr) const
{
	if (!addr.is_unspecified())
	{
		if (addr.is_v6())
		{
			uint32_t a[4];
			auto &&ipv6 = addr.to_v6();

			// convert ipv6 address to host order
			auto &&bytes = ipv6.to_bytes();
			uint32_t *p = reinterpret_cast<uint32_t *>(bytes.data());
			a[3] = vs_ntohl(p[0]);
			a[2] = vs_ntohl(p[1]);
			a[1] = vs_ntohl(p[2]);
			a[0] = vs_ntohl(p[3]);

			return IsAllowed(a);
		}
		return IsAllowed(static_cast<std::uint32_t>(addr.to_v4().to_ulong()));
	}
	return false;
}

bool VS_NetworkConnectionACL::ParseEntry(ACLEntry &e, const char *entry)
{
	char *mask = NULL, subnet = NULL;
	int mask_bits = 0;
	uint32_t mask_value_v4 = 0, subnet_value_v4 = 0;
	IPAddress sub;

	if (entry == NULL)
		return false;

	mask = (char *)strrchr(entry, '/');

	if (mask == NULL)
		return false;

	// parse mask
	mask_bits = atoi(mask + 1);
	{
		string mask_str(entry, mask - entry);

		if (StringToIPAddress(mask_str.c_str(), sub))
		{
			if (sub.type == IP_ADDR_V4)
			{
				e.type = IP_V4;
				e.data.ipv4.mask = GetMaskValue_IPv4(mask_bits);
				e.data.ipv4.subnet = sub.addr.ipv4;

				return true;

			}
			else if (sub.type == IP_ADDR_V6)
			{
				e.type = IP_V6;
				GetMaskValue_IPv6(e.data.ipv6.mask, mask_bits);
				e.data.ipv6.subnet = int128_load_array(sub.addr.ipv6);

				return true;
			}
		}
	}
	return false;
}

void VS_NetworkConnectionACL::ParseACLData(const std::vector<uint8_t> &data)
{

	// iterate over ACL data entries
	for (char *p = (char *)&data[0]; *p != '\0'; p += (strlen(p) + 1))
	{
		AddEntry(p);
	}
}

void VS_NetworkConnectionACL::LoadACL(const ConnectionType type)
{
	const char *peers_dir;

	if (m_finalized)
		return;

	Clear();

	switch (type)
	{
	case CONN_H323:
		peers_dir = H323_PEERS_KEY;
		break;
	case CONN_SIP:
		peers_dir = SIP_PEERS_KEY;
		break;
	}

	{
		VS_RegistryKey key(false, peers_dir);
		const char *acl_data;
		vector<uint8_t> data(BUFSZ);
		uint32_t mode;
		int size;

		if (!key.IsValid())
		{
			SetMode(ACL_NONE);
			Finalize();
			return;
		}
		// Get ACL type
		if (!key.GetValue((void *)&mode, sizeof(mode), VS_REG_INTEGER_VT, CONN_ACL_MODE))
			mode = 0;

		switch (mode)
		{
		case 1:
			acl_data = CONN_ACL_BLACK_LIST;
			SetMode(ACL_BLACKLIST);
			break;
		case 2:
			acl_data = CONN_ACL_WHITE_LIST;
			SetMode(ACL_WHITELIST);
			break;
		default:
			SetMode(ACL_NONE);
			break;
		}

		if (GetMode() == ACL_NONE)
		{
			Finalize();
			return;
		}

		// Read ACL data
		while ((size = key.GetValue((void *)&data[0], data.size() * sizeof(data[0]), VS_REG_STRING_VT, acl_data)) < 0)
		{
			data.resize(data.size() * 2);
		}

		if (size == 0)
		{
			Finalize();
			return;
		}

		// Parse ACL data
		ParseACLData(data);
		Finalize();
	}
}

bool VS_NetworkConnectionACL::EntriesAreEqual(const ACLEntry &e1, const ACLEntry &e2)
{
	if (e1.type != e2.type)
		return false;

	switch (e1.type)
	{
	case IP_V4:
	{
		return (e1.data.ipv4.subnet == e2.data.ipv4.subnet &&
			e1.data.ipv4.mask == e2.data.ipv4.mask);
	}
		break;
	case IP_V6:
	{
		return (int128_cmpeq(e1.data.ipv6.subnet, e2.data.ipv6.subnet) &&
			int128_cmpeq(e1.data.ipv6.mask, e2.data.ipv6.mask));
	}
		break;
	}

	return false;
}
