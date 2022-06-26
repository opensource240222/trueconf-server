#pragma once

#include <cstdint>
#include <ostream>

// Portably prints a pointer padded to native size: 0x01234567 or 0x0123456789abcdef.
struct pointer_value
{
	const void* value;
	explicit pointer_value(const void* value_) : value(value_) {}
};
template <class CharT, class Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& s, pointer_value x)
{
	const auto flags(s.flags());
	const auto fill(s.fill());
	s << "0x";
	s.setf(std::ios::hex | std::ios::right, std::ios::basefield | std::ios::adjustfield);
	s.unsetf(std::ios::showbase | std::ios::uppercase);
	s.width(sizeof(void*) * 2);
	s.fill('0');
	s << reinterpret_cast<uintptr_t>(x.value);
	s.fill(fill);
	s.flags(flags);
	return s;
}
