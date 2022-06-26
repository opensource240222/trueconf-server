#include "parser_types.h"

#include "tools.h"

#include "std-generic/cpplib/hton.h"
#include "std-generic/attributes.h"

namespace mdns
{

namespace
{
const uint16_t NAMESIZE_DEFAULT = 255;
const uint16_t NAMESIZE_MAX     = 16384;
}

mdns::OPCODE toOpcode(uint8_t value)
{
	if (value > 5)
		return mdns::OPCODE::NONE;
	return static_cast<mdns::OPCODE>(value);
}

mdns::RCODE toRcode(uint8_t value)
{
	if (value > 10)
		return mdns::RCODE::NONE;
	return static_cast<mdns::RCODE>(value);
}

mdns::TYPE toType(uint16_t value)
{
	switch(value)
	{
	case 1: VS_FALLTHROUGH;
	case 2: VS_FALLTHROUGH;
	case 5: VS_FALLTHROUGH;
	case 6: VS_FALLTHROUGH;
	case 12: VS_FALLTHROUGH;
	case 15: VS_FALLTHROUGH;
	case 16: VS_FALLTHROUGH;
	case 17: VS_FALLTHROUGH;
	case 18: VS_FALLTHROUGH;
	case 24: VS_FALLTHROUGH;
	case 25: VS_FALLTHROUGH;
	case 28: VS_FALLTHROUGH;
	case 29: VS_FALLTHROUGH;
	case 33: VS_FALLTHROUGH;
	case 35: VS_FALLTHROUGH;
	case 36: VS_FALLTHROUGH;
	case 37: VS_FALLTHROUGH;
	case 39: VS_FALLTHROUGH;
	case 41: VS_FALLTHROUGH;
	case 42: VS_FALLTHROUGH;
	case 43: VS_FALLTHROUGH;
	case 44: VS_FALLTHROUGH;
	case 45: VS_FALLTHROUGH;
	case 46: VS_FALLTHROUGH;
	case 47: VS_FALLTHROUGH;
	case 48: VS_FALLTHROUGH;
	case 49: VS_FALLTHROUGH;
	case 50: VS_FALLTHROUGH;
	case 51: VS_FALLTHROUGH;
	case 52: VS_FALLTHROUGH;
	case 55: VS_FALLTHROUGH;
	case 59: VS_FALLTHROUGH;
	case 60: VS_FALLTHROUGH;
	case 61: VS_FALLTHROUGH;
	case 249: VS_FALLTHROUGH;
	case 250: VS_FALLTHROUGH;
	case 251: VS_FALLTHROUGH;
	case 252: VS_FALLTHROUGH;
	case 255: VS_FALLTHROUGH;
	case 256: VS_FALLTHROUGH;
	case 257: return static_cast<mdns::TYPE>(value); break;
	default: return mdns::TYPE::NONE;
	}
}

mdns::CLASS toClass(uint16_t value)
{
	if (value == 1)
		return mdns::CLASS::INTERNET;
	return mdns::CLASS::NONE;
}

bool handleName(const char* buffer, const char*& pointer, size_t size, std::vector<char>& name)
{
	const char* packetEnd = buffer + size - 1;
	const char* returnPoint = 0;
	if (pointer > packetEnd)
		return false;
	bool compressedAlready = false;
	name.reserve(NAMESIZE_DEFAULT);
	while (*pointer != '\0')
	{
		uint16_t offset = 0;
		unsigned char number = *pointer;
		if (pointer + 1 > packetEnd)
			return false;
		if (0 != (offset = checkForPointer(vs_ntohs(*(reinterpret_cast<const uint16_t*>(pointer))))))
		{
			if (buffer + offset >= pointer)
				return false;
			if (!compressedAlready)
			{
				compressedAlready = true;
				returnPoint = pointer;
			}
			pointer = buffer + offset;
			number = *pointer;
		}
		if (pointer + 1 + number > packetEnd)
			return false;
		name.insert(name.end(), pointer, pointer + 1 + number);
		if (name.size() > NAMESIZE_MAX)
			return false;
		pointer += number + 1;
	}
	name.push_back('\0');
	if (compressedAlready)
		pointer = returnPoint + 2;
	else
		++pointer;
	return true;
}

std::vector<char> cutFrontLabels(const std::vector<char>& domainName, unsigned int index)
{
	if (domainName.size() == 0)
		return {};
	unsigned int pointer = 0;
	for (unsigned int i = 0; i < index; i++)
	{
		if (!(pointer < domainName.size()))
			return {};
		if (domainName[pointer] == '\0')
			return {domainName.begin() + pointer, domainName.begin() + pointer + 1};
		pointer += static_cast<unsigned int>(domainName[pointer]) + 1;
	}
	return {domainName.begin() + pointer, domainName.end()};
}

std::vector<char> getFrontLabels(const std::vector<char>& domainName, unsigned int index)
{
	if (domainName.size() == 0)
		return {};
	unsigned int pointer = 0;
	for (unsigned int i = 0; i < index; i++)
	{
		if (!(pointer < domainName.size()) || domainName[pointer] == '\0')
			return {domainName.begin(), domainName.end()};
		pointer += static_cast<unsigned int>(domainName[pointer]) + 1;
	}
	return {domainName.begin(), domainName.begin() + pointer};
}

size_t getLabelOffset(const std::vector<char>& domainName, unsigned int index)
{
	if (domainName.size() == 0)
		return domainName.size();
	unsigned int pointer = 0;
	for (unsigned int i = 0; i < index; i++)
	{
		if (!(pointer < domainName.size()) || domainName[pointer] == '\0')
			return domainName.size();
		pointer += static_cast<unsigned int>(domainName[pointer]) + 1;
	}
	return pointer;
}

size_t getLabelOffset(const char* domainName, size_t size, unsigned int index)
{
	if (size == 0)
		return size;
	unsigned int pointer = 0;
	for (unsigned int i = 0; i < index; i++)
	{
		if (!(pointer < size) || domainName[pointer] == '\0')
			return size;
		pointer += static_cast<unsigned int>(domainName[pointer]) + 1;
	}
	return pointer;
}

unsigned int getLabelCount(const char* domainName, size_t reservedSize)
{
	if (reservedSize == 0)
		return 0;
	unsigned int pointer = 0;
	unsigned int index = 0;
	while (pointer < reservedSize && domainName[pointer] != '\0')
	{
		pointer += static_cast<unsigned int>(domainName[pointer]) + 1;
		++index;
	}
	if (pointer < reservedSize)
		return index;
	else
		return 0;
}

unsigned int getLabelCount(const std::vector<char>& domainName)
{
	if (domainName.size() == 0)
		return 0;
	unsigned int pointer = 0;
	unsigned int index = 0;
	while (pointer < domainName.size() && domainName[pointer] != '\0')
	{
		pointer += static_cast<unsigned int>(domainName[pointer]) + 1;
		++index;
	}
	if (pointer < domainName.size())
		return index;
	else
		return 0;
}

size_t getSize(const char* domainName, size_t reservedSize)
{
	if (reservedSize == 0)
		return 0;
	unsigned int pointer = 0;
	while (pointer < reservedSize && domainName[pointer] != '\0')
		pointer += static_cast<unsigned int>(domainName[pointer]) + 1;
	if (pointer < reservedSize)
		return pointer;
	else
		return 0;
}

namespace
{
const uint16_t POINTER_BITMASK = 0xc000;
const uint16_t OFFSET_BITMASK = 0x3fff;
}

uint16_t checkForPointer(uint16_t value)
{
	if ((value & POINTER_BITMASK) == POINTER_BITMASK)
		return value & OFFSET_BITMASK;
	return 0;
}

}
