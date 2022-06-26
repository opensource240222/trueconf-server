#pragma once

#include <vector>
#include <cstddef>

#include "parser_types.h"

namespace mdns
{
// This struct is used to parse or/and form DNS query;
struct Query
{
// This struct uses "raw" domain name format, where each domain
// label is preceeded by it's size in bytes.
// For example: \013seliverstov\011_trueconf\004_tcp\005local\0 stands
// for domain name seliverstov._trueconf._tcp._local
public:
	std::vector<char> name;
//	For types descriptions, visit parser_types.h
	mdns::TYPE qtype;
	mdns::QU qu;
	mdns::CLASS qclass;

//	Creates an empty struct; still can be put to use using parse() or fillMdns()
	Query();
//	Construct an object and call parse()
	Query(const void* buffer, const void* query, size_t size);
//	Fills structure fields, making it ready to form an MDNS query
	Query(const std::vector<char>& name_,
		mdns::TYPE qtype_,
		mdns::QU qu_ = mdns::QU::NO,
		mdns::CLASS qclass_ = mdns::CLASS::INTERNET)
		{fill(name_, qtype_, qu_, qclass_);}
//	buffer_ - pointer to DNS header start;
//	query_ - pointer to query start (must be in the same byte array as buffer!);
//	size - entire buffer size;
//	Returns true on success, false on error.
//	As of right now, this function attempts to decompress all compressed names;
//	failure results in parsing error.
	bool parse(const void* buffer_, const void* query_, size_t size);
	void fill(const std::vector<char>& name_,
		mdns::TYPE qtype_,
		mdns::QU qu_ = mdns::QU::NO,
		mdns::CLASS qclass_ = mdns::CLASS::INTERNET);
//	True if query was correctly filled/parsed
	bool correct() const;
//	After parsing, size() is equal to query size in original package;
//	After filling, equals to name size + 4 (estimate query size without compression);
//	Otherwise is zero
	size_t size() const;
//	Clears all contents
	void clear();
//	Returns estimated query size on success, 0 on error (if structure was filled incorrectly)
	size_t form(void* buffer, size_t size) const;
private:
	bool correct_;
	uint32_t size_;
};

}
