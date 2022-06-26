#include "RRecord.h"

#include <algorithm>
#include <cstring>

#include "std-generic/attributes.h"
#include "std-generic/cpplib/hton.h"

#include "tools.h"

namespace mdns
{

namespace
{
const uint16_t CF_BITMASK = 0x8000;
const uint16_t CF_DIV = 0x8000;
const uint16_t RCLASS_BITMASK = 0x7fff;
}

bool RRecord::parse(const void* buffer_, const void* record_, size_t size)
{
	const char* buffer = static_cast<const char*>(buffer_);
	const char* record = static_cast<const char*>(record_);
	correct_ = false;
	const char* packetEnd = buffer + size - 1;
	const char* i = record;
	name.clear();
	if (!handleName(buffer, i, size, name))
		return false;
	if (i + 1 > packetEnd)
		return false;
	if (mdns::TYPE::NONE == (rtype = toType(vs_ntohs(*(reinterpret_cast<const uint16_t*>(i))))))
		return false;
	if (i + 3 > packetEnd)
		return false;
	uint16_t cfClass = vs_ntohs(*(reinterpret_cast<const uint16_t*>(i + 2)));
	cflush = static_cast<mdns::CACHEFLUSH>((cfClass & CF_BITMASK) / CF_DIV);
//	toClass invalidates OPT RRecords, so i have to cast it manually
	if (mdns::CLASS::NONE == (rclass = static_cast<mdns::CLASS>(cfClass & RCLASS_BITMASK)))
		return false;
	i += 4;
	if (i + 3 > packetEnd)
		return false;
	ttl = vs_ntohl(*(reinterpret_cast<const uint32_t*>(i)));
	i += 4;
	if (i + 1 > packetEnd)
		return false;
	rdLength = vs_ntohs(*(reinterpret_cast<const uint16_t*>(i)));
	i += 2;
	if (i + rdLength - 1 > packetEnd)
		return false;
	if (rtype == mdns::TYPE::SRV || rtype == mdns::TYPE::PTR)
	{
		rData.clear();
		if (rtype == mdns::TYPE::SRV)
		{
			if (i + 5 > packetEnd)
				return false;
			rData.insert(rData.end(), i, i + 6);
			i += 6;
		}
		if (!handleName(buffer, i, size, rData))
			return false;
	}
	else
	{
		rData.clear();
		rData.reserve(rdLength);
		rData.insert(rData.end(), i, i + rdLength);
		i += rdLength;
	}
	correct_ = true;
	size_ = i - record;
	return true;
}

void RRecord::fill(const std::vector<char>& name_, mdns::TYPE rtype_, mdns::TTL ttl_,
	const std::vector<char>& rData_, mdns::CACHEFLUSH cflush_,
	mdns::CLASS rclass_)
{
	name = name_;
	rtype = rtype_;
	ttl = ttl_;
	rdLength = static_cast<mdns::RDLENGTH>(rData_.size());
	rData = rData_;
	cflush = cflush_;
	rclass = rclass_;
	correct_ = true;
	size_ = name_.size() + 10 + rData.size();
}

RRecord::RRecord()
{
	clear();
}

RRecord::RRecord(const void* buffer, const void* record, size_t size)
{
	parse(static_cast<const char*>(buffer), static_cast<const char*>(record), size);
}

bool RRecord::correct() const
	{return correct_;}

uint32_t RRecord::size() const
{
	if (correct_)
		return size_;
	return 0;
}

void RRecord::clear()
{
	correct_ = false;
	size_ = 0;

	name.clear();
	rtype = mdns::TYPE::NONE;
	cflush = mdns::CACHEFLUSH::NONE;
	rclass = mdns::CLASS::NONE;
	ttl = 0;
	rdLength = 0;
	rData.clear();
}

size_t RRecord::form(void* buffer, size_t size) const
{
	char* charBuffer = static_cast<char*>(buffer);
	if (name.size() == 0 ||
		rtype == mdns::TYPE::NONE ||
		cflush == mdns::CACHEFLUSH::NONE ||
		rclass == mdns::CLASS::NONE ||
		rdLength != rData.size())
		return 0;
	if (name.size() + 10 + rData.size() > size)
		return 0;
	memcpy(buffer, name.data(), name.size());
	reinterpret_cast<uint16_t*>(charBuffer + name.size())[0] =
		vs_htons(static_cast<uint16_t>(rtype));
	reinterpret_cast<uint16_t*>(charBuffer + name.size())[1] =
		vs_htons(static_cast<uint16_t>(rclass) | ((static_cast<uint16_t>(cflush) * CF_DIV) & CF_BITMASK));
	reinterpret_cast<uint32_t*>(charBuffer + name.size())[1] =
		vs_htonl(ttl);
	reinterpret_cast<uint16_t*>(charBuffer + name.size())[4] =
		vs_htons(rdLength);
	memcpy(static_cast<char*>(buffer) + name.size() + 10, rData.data(), rData.size());
	return true;
}

bool operator<(const RRecord& one, const RRecord& two)
{
	if (one.name != two.name)
	{
		if (one.name < two.name)
			return true;
		return false;
	}
	if (one.cflush != two.cflush)
	{
		if (one.cflush < two.cflush)
			return true;
		return false;
	}
	if (one.rclass != two.rclass)
	{
		if (one.rclass < two.rclass)
			return true;
		return false;
	}
	if (one.rtype != two.rtype)
	{
		if (one.rtype < two.rtype)
			return true;
		return false;
	}
	if (one.rData != two.rData)
	{
		if (one.rData < two.rData)
			return true;
		return false;
	}
	return false;
}

bool operator>(const RRecord& one, const RRecord& two)
{
	if (one.name != two.name)
	{
		if (one.name > two.name)
			return true;
		return false;
	}
	if (one.cflush != two.cflush)
	{
		if (one.cflush > two.cflush)
			return true;
		return false;
	}
	if (one.rclass != two.rclass)
	{
		if (one.rclass > two.rclass)
			return true;
		return false;
	}
	if (one.rtype != two.rtype)
	{
		if (one.rtype > two.rtype)
			return true;
		return false;
	}
	if (one.rData != two.rData)
	{
		if (one.rData > two.rData)
			return true;
		return false;
	}
	return false;
}

bool operator==(const RRecord& one, const RRecord& two)
{
	if (one.name != two.name)
		return false;
	if (one.cflush != two.cflush)
		return false;
	if (one.rclass != two.rclass)
		return false;
	if (one.rtype != two.rtype)
		return false;
	if (one.rData != two.rData)
		return false;
	return true;
}

bool operator!=(const RRecord& one, const RRecord& two)
{
	if (one.name != two.name)
		return true;
	if (one.cflush != two.cflush)
		return true;
	if (one.rclass != two.rclass)
		return true;
	if (one.rtype != two.rtype)
		return true;
	if (one.rData != two.rData)
		return true;
	return false;
}

int clashCompetingRecords(const std::vector<RRecord>& first, const std::vector<RRecord>& second)
{
	std::vector<RRecord> one, two;
	one.resize(first.size());
	two.resize(second.size());
	std::partial_sort_copy(first.begin(), first.end(), one.begin(), one.end());
	std::partial_sort_copy(second.begin(), second.end(), two.begin(), two.end());
	auto result = std::mismatch(one.begin(), one.end(), two.begin(), two.end());
	if (result.first == one.end() && result.second == two.end())
		return 0;
	if (result.first == one.end() && result.second != two.end())
		return -1;
	if (result.second == two.end() && result.first != first.end())
		return 1;
	if (*(result.first) > *(result.second))
		return 1;
	return -1;
}

}
