#pragma once

#include <vector>
#include <string>

#include "std-generic/attributes.h"
#include "std-generic/cpplib/string_view.h"
#include "parser_types.h"


namespace mdns

{

// Transforms 8-bit integer to operation code
// For more informations about the types, visit parser_types.h
mdns::OPCODE toOpcode(uint8_t value);
// Transforms 8-bit integer to response code
mdns::RCODE toRcode(uint8_t value);

// Transforms 16-bit integer to query/record type
mdns::TYPE toType(uint16_t value);
// Transforms 16-bit integer to query/record class (only INTERNET is supported)
mdns::CLASS toClass(uint16_t value);

// Appends domain name to *name*
bool handleName(const char* buffer, const char*& pointer, size_t size, std::vector<char>& name);

// Transforms domain name in it's dotted form to it's raw representation.
// Example: "seliverstov._trueconf._tcp.local" -> "\013seliverstov\011_trueconf\004_tcp\005local'\0'"
template<class ContinuousCharContainer>
const ContinuousCharContainer& fromDotted(ContinuousCharContainer& domainName)
{
	string_view finder(domainName.data(), domainName.size());
	size_t pointer = finder.find('.', 0);
	if (pointer == string_view::npos)
		pointer = finder.size();
	domainName.insert(domainName.begin(), static_cast<char>(pointer));
	finder = string_view(domainName.data(), domainName.size());
	++pointer;
	while (pointer != finder.size())
	{
		size_t oldPointer = pointer;
		pointer = finder.find('.', pointer + 1);
		if (pointer == string_view::npos)
			pointer = finder.size();
		domainName[oldPointer] = static_cast<char>(pointer - oldPointer - 1);
	}
	if (domainName.back() != '\0')
		domainName.push_back('\0');
	return domainName;
}
// Transforms raw domain name to it's dotted form, if possible. Otherwise, returns
// an empty container.
// Example: "\013seliverstov\011_trueconf\004_tcp\005local\0" -> "seliverstov._trueconf._tcp.local"
template<class ContinuousCharContainer>
const ContinuousCharContainer& toDotted(ContinuousCharContainer& domainName)
{
	if (domainName.empty())
		return domainName;
	size_t pointer = *(domainName.begin());
	domainName.erase(domainName.begin());
	for (auto iter = domainName.begin() + pointer; iter != domainName.end() && *iter != '\0'; ++iter)
	{
		auto oldIter = iter;
		iter += *iter;
		*oldIter = '.';
	}
	return domainName;
}

// Returns a domain name with it's first %index% labels cut off
std::vector<char> cutFrontLabels(const std::vector<char>& domainName, unsigned int index);
// Returns first %index% labels of a domain name
std::vector<char> getFrontLabels(const std::vector<char>& domainName, unsigned int index);

// Returns label offset from the start of container
size_t getLabelOffset(const char* domainName, size_t size, unsigned int index);
size_t getLabelOffset(const std::vector<char>& domainName, unsigned int index);

// Returns a domain name label count
unsigned int getLabelCount(const char* domainName, size_t reservedSize);
unsigned int getLabelCount(const std::vector<char>& domainName);

// Returns domain name size, in bytes
size_t getSize(const char* domainName, size_t reservedSize);

uint16_t checkForPointer(uint16_t value);

}
