#pragma once

#include <cstddef>

#include "parser_types.h"

namespace mdns
{
//This struct is used to parse DNS header and form MDNS header (where many fields are zero by default)
struct Header
{
public:
//	For types descriptions, visit parser_types.h
	mdns::ID id;
	mdns::QR qr;
	mdns::OPCODE opcode;
	mdns::AA aa;
	mdns::TC tc;
	mdns::RD rd;
	mdns::RA ra;
	mdns::ZERO zero;
	mdns::RCODE rcode;
	mdns::QD qd;
	mdns::AN an;
	mdns::NS ns;
	mdns::AR ar;

//	Construct an object and call parse()
	Header(const void* buffer, size_t size);
//	Fills structure fields, making it ready to form an MDNS header
	Header(
		mdns::QD qd_,
		mdns::AN an_,
		mdns::NS ns_,
		mdns::AR ar_,
		mdns::QR qr_ = mdns::QR::RESPONSE
	) {fillMdns(qd_, an_, ns_, ar_, qr_);}
//	Creates empty struct; still can be put to use using parse() or fillMdns()
	Header();
	void fillMdns(
                  mdns::QD qd_ = 0,
				  mdns::AN an_ = 0,
				  mdns::NS ns_ = 0,
				  mdns::AR ar_ = 0,
				  mdns::QR qr_ = mdns::QR::RESPONSE
	);
//	Takes a byte buffer; if size >= 12, parses it.
//	Returns true on success, false on error.
	bool parse(const void* buffer, size_t size);
//	If package is correctly parsed (or filled), returns size of DNS header, in bytes (always 12)
//	Otherwise, returns zero
	size_t size() const;
//	True if header was correctly filled/parsed
	bool correct() const;
//	Clears all contents
	void clear();

//	Returns message size on success, 0 on error
	size_t form(void* buffer, size_t size) const;

	static const size_t HEADER_SIZE = 12;
private:
	bool correct_;
	size_t size_;
};

}
