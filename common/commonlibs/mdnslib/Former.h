#pragma once

#include <vector>

#include "Header.h"
#include "Query.h"
#include "RRecord.h"

#include "std-generic/compat/map.h"

namespace mdns
{

// This class is used to form a correct MDNS package. Actually does it if has at least a correct
// header.
// Usage: fill the structure by calling addQuery and addRRecord functions for each query and resource
// record you want to add. Call setHeader once you're sertain about query/rrecord count; header can be
// set or re-set at any time. After successfully filling the structure (you may want to check the
// return value of set* and add* functions just to be sure) you may call form() to attempt to create
// a correct DNS packet, which, if it is successfully done, can be transported via network.
// Note: implements "smart" names compression (RFC 6762:18.14), for query/record names and PTR/SRV
// rData, so don't get shocked when you see 0xc0 0xXX instead of your beloved domain name or it's
// ending. Also it can't be turned off without changing the code, so unless you are using Parser
// from MDNS_Responder, you'll have to implement name decompression for yours (if you wish to receive
// those packets on other end).
class Former
{
public:
//	Sets message header. Most values (except for those being taken as arguments) are set to default
//	values stated in MDNS standard (RFC 6762:18)
	bool setHeader(mdns::QR qr, mdns::QD qd, mdns::AN an, mdns::NS ns, mdns::AR ar);
	bool setHeader(const mdns::Header& header);
//	Adds query to the message.
//	Note that name should look like it would in a raw package, which is:
//	"\012subsubdomain\009subdomain\006domain\0"
	bool addQuery(const std::vector<char>& name, mdns::TYPE qtype, mdns::QU qu = mdns::QU::NO);
	bool addQuery(const mdns::Query& query);
//	Adds resource record to the message.
//	Note that name should look like it would in a raw package, which is:
//	"\012subsubdomain\009subdomain\006domain\0"
	bool addRRecord(const std::vector<char>& name, mdns::TYPE rtype, mdns::TTL ttl,
		const std::vector<char>& rData, mdns::CACHEFLUSH cflush = mdns::CACHEFLUSH::YES);
	bool addRRecord(const mdns::RRecord& record);
//	Grants read access to header/queries/records set by user
	const mdns::Header& header() const {return header_;}
	const std::vector<mdns::Query>& queries() const {return queries_;}
	const std::vector<mdns::RRecord>& records() const {return records_;}

//	Attempt to form the packet. Returns a pointer to packet start on success, nullptr on
//	failure. Compresses names for the packet, leaving internal data intact.
	char* form();

	Former();
//	Returns packet size, if it was successfuly formed
	size_t size() const {if (formed_) return packet_.size(); else return 0;}
//	Returns a pointer to packet data, if it was successfully formed
	char* data() {if (formed_) return packet_.data(); else return nullptr;}
//	Clears all contents
	void clear();
//	True if all headers, queries and resource records passed previously were correct so far
	bool correct() const {return correct_;}

	std::vector<char>& container() {return packet_;}

	static const size_t PACKET_SIZE_DEFAULT = 512;

private:
	bool correct_;
	bool formed_;
	std::vector<char> packet_;
	vs::map<std::vector<char>, uint16_t> names_;

//	Message header
	mdns::Header header_;
//	Queries
	std::vector<mdns::Query> queries_;
//	Records
	std::vector<mdns::RRecord> records_;

	bool pushHeader(const mdns::Header&);
	bool pushQuery(const mdns::Query&);
	bool pushRRecord(const mdns::RRecord&);
//	Return value: number of labels preceeding pointer in the result;
//	On failure, full label count is returned
	unsigned int compressName(const std::vector<char>& name, std::vector<char>& result);
	void insertNamePointers(const std::vector<char>& name, unsigned int labelCount, size_t futureNamePosition);
};

}
