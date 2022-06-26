#pragma once

#include "VS_RouterMessage.h"

#include <ostream>

template <class charT, class traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& s, const VS_RouterMessage& x)
{
	auto flags(s.flags());
	s.width(0);
	s << std::dec;

	if (x.SrcCID() && *x.SrcCID())
		s << "cid:" << x.SrcCID();
	else
		s << x.SrcUser();
	s << "@" << x.SrcServer() << ":" << x.SrcService() << " > ";

	if (x.DstCID() && *x.DstCID())
		s << "cid:" << x.DstCID();
	else
		s << x.DstUser();
	s << "@" << x.DstServer() << ":" << x.DstService();

	s.flags(flags);
	return s;
}