#pragma once

#include <string>
#include <vector>

#include "parser_types.h"

namespace mdns
{

// This struct is used to parse or/and form DNS record;
struct RRecord
{
// This struct uses "raw" domain name format, where each domain
// label is preceeded by it's size in bytes.
// For example: \013seliverstov\011_trueconf\004_tcp\005local\0 stands
// for domain name seliverstov._trueconf._tcp._local
public:
	std::vector<char> name;
//	For types descriptions, visit parser_types.h
	mdns::TYPE rtype;
	mdns::CACHEFLUSH cflush;
	mdns::CLASS rclass;
	mdns::TTL ttl;
	mdns::RDLENGTH rdLength;
	std::vector<char> rData;

//	Creates an empty struct; still can be put to use using parse() or fillMdns()
	RRecord();
//	Construct an object and call parse()
	RRecord(const void* buffer, const void* record, size_t size);
//	Fills structure fields, making it ready to form an Mmdns record
	RRecord(const std::string& name_, mdns::TYPE rtype_, mdns::TTL ttl_,
		const std::vector<char>& rData_,
		mdns::CACHEFLUSH cflush_ = mdns::CACHEFLUSH::NO, mdns::CLASS rclass_ = mdns::CLASS::INTERNET)
		{fill({name_.begin(), name_.end()}, rtype_, ttl_, rData_, cflush_, rclass_);}
	RRecord(const std::vector<char>& name_, mdns::TYPE rtype_, mdns::TTL ttl_,
		const std::vector<char>& rData_,
		mdns::CACHEFLUSH cflush_ = mdns::CACHEFLUSH::NO, mdns::CLASS rclass_ = mdns::CLASS::INTERNET)
		{fill(name_, rtype_, ttl_, rData_, cflush_, rclass_);}
//	buffer_ - pointer to DNS header start;
//	record_ - pointer to record start (must be in the same byte array as buffer!);
//	size - entire buffer size;
//	Returns true on success, false on error.
//	As of right now, this function attempts to decompress all compressed names and
//	names hidden in rData (if type is PTR or SRV);
//	failure results in parsing error.
	bool parse(const void* buffer_, const void* record_, size_t size);
	void fill(const std::vector<char>& name_, mdns::TYPE rtype_, mdns::TTL ttl_,
		const std::vector<char>& rData_,
		mdns::CACHEFLUSH cflush_ = mdns::CACHEFLUSH::NO, mdns::CLASS rclass_ = mdns::CLASS::INTERNET);
//	True if record was correctly filled/parsed
	bool correct() const;
//	After parsing, size() is equal to record size in original package;
//	After filling, equals to name size + 10 + rdata size (estimate record size without compression);
//	Otherwise is zero
	uint32_t size() const;
//	Clears all contents
	void clear();
//	Returns estimated record size on success, 0 on error (if structure was filled incorrectly)
	size_t form(void* buffer, size_t size) const;
private:
	bool correct_;
	uint32_t size_;

};

bool operator<(const RRecord& one, const RRecord& two);
bool operator>(const RRecord& one, const RRecord& two);
bool operator==(const RRecord& one, const RRecord& two);
bool operator!=(const RRecord& one, const RRecord& two);

// Returns 1 if first > second, -1 if first < second, 0 if first == second
int clashCompetingRecords(const std::vector<RRecord>& first, const std::vector<RRecord>& second);

}
