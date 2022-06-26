#include "Query.h"

#include <cstring>

#include "tools.h"

#include "std-generic/cpplib/hton.h"

namespace mdns
{

namespace
{
const uint16_t QU_BITMASK = 0x8000;
const uint16_t QU_DIV = 0x8000;
const uint16_t QCLASS_BITMASK = 0x7fff;
}

bool Query::parse(const void* buffer_, const void* query_, size_t size)
{
	const char* buffer = static_cast<const char*>(buffer_);
	const char* query = static_cast<const char*>(query_);
	correct_ = false;
	const char* packetEnd = buffer + size - 1;
	const char* i = query;
	if (i > packetEnd)
		return false;
	name.clear();
	if (!handleName(buffer, i, size, name))
		return false;
	if (i + 1 > packetEnd)
		return false;
	if (mdns::TYPE::NONE == (qtype = toType(vs_ntohs(*(reinterpret_cast<const uint16_t*>(i))))))
		return false;
	if (i + 3 > packetEnd)
		return false;
	uint16_t quClass = vs_ntohs(*(reinterpret_cast<const uint16_t*>(i + 2)));
	qu = static_cast<mdns::QU>((quClass & QU_BITMASK) / QU_DIV);
	if (mdns::CLASS::NONE == (qclass = toClass(quClass & QCLASS_BITMASK)))
		return false;
	correct_ = true;
	size_ = i + 2 * sizeof(uint16_t) - query;
	return true;
}

void Query::fill(const std::vector<char>& name_, mdns::TYPE qtype_, mdns::QU qu_, mdns::CLASS qclass_)
{
	name = name_;
	qtype = qtype_;
	qu = qu_;
	qclass = qclass_;
	correct_ = true;
	size_ = name_.size() + 4;
}

Query::Query(const void* buffer, const void* query, size_t size)
{
	parse(static_cast<const char*>(buffer), static_cast<const char*>(query), size);
}

Query::Query()
{
	clear();
}

bool Query::correct() const
	{return correct_;}

size_t Query::size() const
{
	if (correct_)
		return size_;
	return 0;
}

void Query::clear()
{
	correct_ = false;
	size_ = 0;

	name.clear();
	qtype = mdns::TYPE::NONE;
	qu = mdns::QU::NONE;
	qclass = mdns::CLASS::NONE;
}

size_t Query::form(void* buffer, size_t size) const
{
	char* charBuffer = static_cast<char*>(buffer);
	if (name.size() == 0 ||
        qtype == mdns::TYPE::NONE ||
        qu == mdns::QU::NONE ||
        qclass == mdns::CLASS::NONE)
		return 0;
	if (size < name.size() + 4)
		return 0;
	memcpy(buffer, name.data(), name.size());
	reinterpret_cast<uint16_t*>(charBuffer + name.size())[0] =
		vs_htons(static_cast<uint16_t>(qtype));
	reinterpret_cast<uint16_t*>(charBuffer + name.size())[1] =
		vs_htons(static_cast<uint16_t>(qclass) | ((static_cast<uint16_t>(qu) * QU_DIV) & QU_BITMASK));
	return name.size() + 4;
}

}
