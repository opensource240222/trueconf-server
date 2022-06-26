#pragma once

#include <vector>

#include "parser_types.h"
#include "Header.h"
#include "Query.h"
#include "RRecord.h"

namespace mdns
{

namespace FILTER
{
//	QU
	const uint32_t QUERY =      0x00000001;
	const uint32_t RESPONSE =   0x00000002;
//	TYPE
	const uint32_t PTR =        0x00000004;
	const uint32_t SRV =        0x00000008;
	const uint32_t TXT =        0x00000010;
	const uint32_t A =          0x00000020;
	const uint32_t AAAA =       0x00000040;
	// ANY is just a type, not a directive! (meaning adding it won't add every other type to filter list)
	const uint32_t ANY =        0x00000080;
//	UNICAST FLAG
	const uint32_t QU =         0x00000100;
	const uint32_t CF =         0x00000100;// Flags are intentionally same since they are represented by
	// the same bit
}

//	This structure is used to parse entire DNS packets. After successful parsing,
//	puts all gathered data into corresponding sub-structures (also uses their interfaces for parsing)
//	Has filtering implemented: header, query/record types, unicast/cacheflush flags. Can additionaly
//	search for user-specified domain names in queries/record names or PTR/SRV rData. No matter how
//	many names are specified, parsing will be successful if at least one of them is found (and all other
//	requirements are met)
struct Parser
{
public:
	static const unsigned int DEFAULT_MESSAGE_SIZE;

//	Message header
	mdns::Header header;
//	Queries
	std::vector<mdns::Query> queries;
//	Responses
	std::vector<mdns::RRecord> responses;
//	Authority records
	std::vector<mdns::RRecord> authRecs;
//	Additional records
	std::vector<mdns::RRecord> additRecs;

	Parser();

//	Takes a binary sum of flags defined in namespace mdns::FILTER
//	Flags are divided to groups, and adding multiple flags A and B inside one group means A OR B
//	while adding flags A and B from different groups means A AND B
//	Example:
//  parser.filter(mdns::FILTER::QUERY |
//                mdns::FILTER::PTR |
//                mdns::FILTER::SRV |
//                mdns::FILTER::TXT |
//                mdns::FILTER::QU)
//	will filter for (QUERY && (PTR || SRV || TXT) && QU)
	void filter(uint32_t flags);

//	Note that name should look like it would in a raw package, which is:
//	"\012subsubdomain\009subdomain\006domain\0"
//	Applies logical AND to all other filters specified using filter() function
	void filterName(const std::vector<char>& name);
	void filterName(const void* name, size_t size);

//	Returns true if package was correct and fit user's requirements (stated in
//	filter() and checkForName() functions)
	bool parse(const void* buf, size_t count);

//	Checks if name of index i has been found after parsing. In case there is no name of
//	index i, returns false
	bool nameFound(unsigned int i) const;
//	*Found() checks whether requested (the ones that were chosen to filter) properties
//	of the package have been found
	bool ptrFound() const {return foundPtr;}
	bool srvFound() const {return foundSrv;}
	bool txtFound() const {return foundTxt;}
	bool aFound() const {return foundA;}
	bool aaaaFound() const {return foundAaaa;}
	bool anyFound() const {return foundAny;}
	bool quFound() const {return foundQu;}

//	Deletes all the information stored in the object. After this object can be
//	used as if it was just created.
	void clear();
//	Deletes all the info, but keeps filters intact
	void clearMessage();
private:
	bool filterEnabled;

	bool typeFilterEnabled;
	bool filterQuery;
	bool filterResponse;
	bool filterPtr;
	bool filterSrv;
	bool filterTxt;
	bool filterA;
	bool filterAaaa;
	bool filterAny;
	bool filterQu;

	bool findName;
	std::vector<std::vector<char>> namesToFind;

	std::vector<bool> foundName;
	bool foundPtr;
	bool foundSrv;
	bool foundTxt;
	bool foundA;
	bool foundAaaa;
	bool foundAny;
	bool foundQu;

	bool checkHeaderFilters() const;
	bool checkQueryFilters(const mdns::Query&);
	bool checkRecordFilters(const mdns::RRecord&);
	bool checkFindings() const;

	bool checkName(const std::vector<char>& candidate);
};

}
