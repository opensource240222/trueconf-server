#include "Header.h"

#include "std-generic/cpplib/hton.h"

#include "tools.h"

namespace mdns
{

namespace
{
const uint16_t QR_BITMASK = 0x8000;
const uint16_t QR_DIV = 0x8000;
const uint16_t OPCODE_BITMASK = 0x7800;
const uint16_t OPCODE_DIV = 0x0800;
const uint16_t AA_BITMASK = 0x0400;
const uint16_t AA_DIV = 0x0400;
const uint16_t TC_BITMASK = 0x0200;
const uint16_t TC_DIV = 0x0200;
const uint16_t RD_BITMASK = 0x0100;
const uint16_t RD_DIV = 0x0100;
const uint16_t RA_BITMASK = 0x0080;
const uint16_t RA_DIV = 0x0080;
const uint16_t ZERO_BITMASK = 0x0070;
const uint16_t ZERO_DIV = 0x0010;
const uint16_t RCODE_BITMASK = 0x000f;
//const size_t Header::HEADER_SIZE = 12;
}

Header::Header()
{
//	Set all fields to 0 just in case
	clear();
}

Header::Header(const void* buffer, size_t size)
{
	parse(buffer, size);
}

void Header::fillMdns(mdns::QD qd_, mdns::AN an_, mdns::NS ns_, mdns::AR ar_, mdns::QR qr_)
{
	id = 0;
	qr = qr_;
	opcode = mdns::OPCODE::QUERY;
	if (qr_ == mdns::QR::QUERY)
		aa = mdns::AA::NO;
	else
		aa = mdns::AA::YES;
	tc = mdns::TC::NO;
	rd = mdns::RD::NO;
	ra = mdns::RA::NO;
	zero = 0;
	rcode = mdns::RCODE::ERROR_NONE;
	qd = qd_;
	an = an_;
	ns = ns_;
	ar = ar_;
	size_ = 12;
	correct_ = true;
}

bool Header::parse(const void* buffer, size_t size)
{
	correct_ = false;
	if (size < HEADER_SIZE)
		return false;
	id = static_cast<mdns::ID>(vs_ntohs(*(static_cast<const uint16_t*>(buffer))));
	uint16_t flags = vs_ntohs((static_cast<const uint16_t*>(buffer))[1]);
	qr = static_cast<mdns::QR>((flags & QR_BITMASK) / QR_DIV);
	if (mdns::OPCODE::NONE == (opcode = toOpcode((flags & OPCODE_BITMASK) / OPCODE_DIV)))
		return false;
	aa = static_cast<mdns::AA>((flags & AA_BITMASK) / AA_DIV);
	tc = static_cast<mdns::TC>((flags & TC_BITMASK) / TC_DIV);
	rd = static_cast<mdns::RD>((flags & RD_BITMASK) / RD_DIV);
	ra = static_cast<mdns::RA>((flags & RA_BITMASK) / RA_DIV);
	zero = static_cast<mdns::ZERO>((flags & ZERO_BITMASK) / ZERO_DIV);
	if (zero != 0)
		return false;
	if (mdns::RCODE::NONE == (rcode = toRcode((flags & RCODE_BITMASK))))
		return false;
	qd = static_cast<mdns::QD>(vs_ntohs((static_cast<const uint16_t*>(buffer))[2]));
	an = static_cast<mdns::AN>(vs_ntohs((static_cast<const uint16_t*>(buffer))[3]));
	ns = static_cast<mdns::NS>(vs_ntohs((static_cast<const uint16_t*>(buffer))[4]));
	ar = static_cast<mdns::AR>(vs_ntohs((static_cast<const uint16_t*>(buffer))[5]));
	correct_ = true;
	size_ = HEADER_SIZE;
	return true;
}

size_t Header::size() const
{
	if (correct_)
		return size_;
	return 0;
}

bool Header::correct() const
	{return correct_;}

void Header::clear()
{
	correct_ = false;
	size_ = 0;

	id = 0;
	qr = mdns::QR::NONE;
	opcode = mdns::OPCODE::NONE;
	aa = mdns::AA::NONE;
	tc = mdns::TC::NONE;
	rd = mdns::RD::NONE;
	ra = mdns::RA::NONE;
	zero = 0;
	rcode = mdns::RCODE::NONE;
	qd = an = ns = ar = 0;
}

size_t Header::form(void* buffer, size_t size) const
{
	if (size < HEADER_SIZE)
		return 0;
	static_cast<uint16_t*>(buffer)[0] = vs_htons(id);
	if (qr == mdns::QR::NONE ||
		opcode == mdns::OPCODE::NONE ||
		aa == mdns::AA::NONE ||
		tc == mdns::TC::NONE ||
		rd == mdns::RD::NONE ||
		ra == mdns::RA::NONE ||
		rcode == mdns::RCODE::NONE)
		return 0;
	uint16_t flags = 0;
	flags |= ((static_cast<uint16_t>(qr) * QR_DIV) & QR_BITMASK) |
	         ((static_cast<uint16_t>(opcode) * OPCODE_DIV) & OPCODE_BITMASK) |
             ((static_cast<uint16_t>(aa) * AA_DIV) & AA_BITMASK) |
             ((static_cast<uint16_t>(tc) * TC_DIV) & TC_BITMASK) |
             ((static_cast<uint16_t>(rd) * RD_DIV) & RD_BITMASK) |
             ((static_cast<uint16_t>(ra) * RA_DIV) & RA_BITMASK) |
             ((static_cast<uint16_t>(zero) * ZERO_DIV) & ZERO_BITMASK) |
             (static_cast<uint16_t>(rcode) & RCODE_BITMASK);
	static_cast<uint16_t*>(buffer)[1] = vs_htons(flags);
	static_cast<uint16_t*>(buffer)[2] = vs_htons(qd);
	static_cast<uint16_t*>(buffer)[3] = vs_htons(an);
	static_cast<uint16_t*>(buffer)[4] = vs_htons(ns);
	static_cast<uint16_t*>(buffer)[5] = vs_htons(ar);
	return HEADER_SIZE;
}

}
