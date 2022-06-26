#include "Parser.h"

namespace mdns
{

//Lowest possible MTU size?
const unsigned int Parser::DEFAULT_MESSAGE_SIZE = 1500;

Parser::Parser()
{
	clear();
}

void Parser::filter(uint32_t flags)
{
	filterEnabled = true;
	if (flags & mdns::FILTER::QUERY)
		filterQuery = true;
	if (flags & mdns::FILTER::RESPONSE)
		filterResponse = true;
	if (flags & mdns::FILTER::PTR)
	{
		typeFilterEnabled = true;
		filterPtr = true;
	}
	if (flags & mdns::FILTER::SRV)
	{
		typeFilterEnabled = true;
		filterSrv = true;
	}
	if (flags & mdns::FILTER::TXT)
	{
		typeFilterEnabled = true;
		filterTxt = true;
	}
	if (flags & mdns::FILTER::A)
	{
		typeFilterEnabled = true;
		filterA = true;
	}
	if (flags & mdns::FILTER::AAAA)
	{
		typeFilterEnabled = true;
		filterAaaa = true;
	}
	if (flags & mdns::FILTER::ANY)
	{
		typeFilterEnabled = true;
		filterAny = true;
	}
	if (flags & mdns::FILTER::QU)
	{
		typeFilterEnabled = true;
		filterQu = true;
	}
}

void Parser::filterName(const std::vector<char>& name)
{
	filterEnabled = findName = true;
	namesToFind.push_back(name);
}

void Parser::filterName(const void* name, size_t size)
{
	const char* name_ = static_cast<const char*>(name);
	std::vector<char> nameToFind;
	nameToFind.insert(nameToFind.end(), name_, name_ + size);
	namesToFind.push_back(std::move(nameToFind));
	filterEnabled = findName = true;
}

bool Parser::parse(const void* buf, size_t count)
{
	const char* buffer = static_cast<const char*>(buf);
	foundName.resize(namesToFind.size(), false);
	size_t generalOffset = 0;
	clearMessage();
	if (!header.parse(buffer, count) || !checkHeaderFilters())
		return false;
	generalOffset += header.size();
	for (unsigned int i = 0; i < header.qd; ++i)
	{
		queries.emplace_back();
		mdns::Query& query = queries.back();
		if (!query.parse(buffer, buffer + generalOffset, count))
			return false;
		generalOffset += query.size();
		checkQueryFilters(query);
	}
	for (unsigned int i = 0; i < header.an; ++i)
	{
		responses.emplace_back();
		mdns::RRecord& record = responses.back();
		if (!record.parse(buffer, buffer + generalOffset, count))
			return false;
		generalOffset += record.size();
		checkRecordFilters(record);
	}
	for (unsigned int i = 0; i < header.ns; ++i)
	{
		authRecs.emplace_back();
		mdns::RRecord& record = authRecs.back();
		if (!record.parse(buffer, buffer + generalOffset, count))
			return false;
		generalOffset += record.size();
		checkRecordFilters(record);
	}
	for (unsigned int i = 0; i < header.ar; ++i)
	{
		additRecs.emplace_back();
		mdns::RRecord& record = additRecs.back();
		if (!record.parse(buffer, buffer + generalOffset, count))
			return false;
		generalOffset += record.size();
		checkRecordFilters(record);
	}
	return checkFindings();
}

bool Parser::nameFound(unsigned int i) const
{
	if (i < foundName.size())
		return foundName[i];
	else
		return false;
}

bool Parser::checkName(const std::vector<char>& candidate)
{
	for (unsigned int i = 0; i < namesToFind.size(); ++i)
	{
		if (namesToFind[i] == candidate)
		{
			if (foundName.size() < i)
				foundName.resize(namesToFind.size(), false);
			foundName[i] = true;
			return true;
		}
	}
	return false;
}

bool Parser::checkHeaderFilters() const
{
	if (!filterEnabled)
		return true;
	if (header.qr == mdns::QR::QUERY && filterResponse && !filterQuery)
		return false;
	if (header.qr == mdns::QR::RESPONSE && filterQuery && !filterResponse)
		return false;
	return true;
}

bool Parser::checkQueryFilters(const mdns::Query& query)
{
	if (!filterEnabled)
		return true;
	bool result = false;
	if (filterQu)
	{
		if (query.qu != mdns::QU::YES)
			return false;
		foundQu = true;
	}
	if (findName)
	{
		if (!checkName(query.name))
			return false;
	}
	if (!typeFilterEnabled)
		return true;

	if (filterPtr && query.qtype == mdns::TYPE::PTR)
		foundPtr = result = true;
	else if (filterSrv && query.qtype == mdns::TYPE::SRV)
		foundSrv = result = true;
	else if (filterTxt && query.qtype == mdns::TYPE::TXT)
		foundTxt = result = true;
	else if (filterA && query.qtype == mdns::TYPE::A)
		foundA = result = true;
	else if (filterAaaa && query.qtype == mdns::TYPE::AAAA)
		foundAaaa = result = true;
	else if (filterAny && query.qtype == mdns::TYPE::ANY)
		foundAny = result = true;
	return result;
}

bool Parser::checkRecordFilters(const mdns::RRecord& record)
{
	if (!filterEnabled)
		return true;
	bool result = false;
//	QU has the same position in the packet as CF
	if (filterQu)
	{
		if (record.cflush != mdns::CACHEFLUSH::YES)
			return false;
		foundQu = true;
	}
	if (findName)
	{
		if (!checkName(record.name))
			return false;
	}

	if (!typeFilterEnabled)
		return true;

	if (filterPtr && record.rtype == mdns::TYPE::PTR)
		foundPtr = result = true;
	else if (filterSrv && record.rtype == mdns::TYPE::SRV)
		foundSrv = result = true;
	else if (filterTxt && record.rtype == mdns::TYPE::TXT)
		foundTxt = result = true;
	else if (filterA && record.rtype == mdns::TYPE::A)
		foundA = result = true;
	else if (filterAaaa && record.rtype == mdns::TYPE::AAAA)
		foundAaaa = result = true;
	else if (filterAny && record.rtype == mdns::TYPE::ANY)
		foundAny = result = true;
	return result;
}

bool Parser::checkFindings() const
{
	if (!filterEnabled)
		return true;
	if (findName)
	{
		bool foundNameFlag = false;
		for (const auto& i: foundName)
			if (i)
				foundNameFlag = true;
		if (!foundNameFlag)
			return false;
	}
	if (filterQu && !foundQu)
		return false;
	if ((filterPtr && foundPtr) ||
		(filterSrv && foundSrv) ||
		(filterTxt && foundTxt) ||
		(filterA && foundA) ||
		(filterAaaa && foundAaaa) ||
		(filterAny && foundAny) ||
		filterQuery || filterResponse ||
		!typeFilterEnabled)
		return true;
	return false;
}

void Parser::clear()
{
	clearMessage();

	namesToFind.clear();

	foundName.clear();

	foundPtr =
	foundSrv =
	foundTxt =
	foundA =
	foundAaaa =
	foundAny =
	foundQu =
	false;

	filterEnabled =
	typeFilterEnabled =
	findName =
	filterQuery =
	filterResponse =
	filterPtr =
	filterSrv =
	filterTxt =
	filterA =
	filterAaaa =
	filterAny =
	filterQu =
	false;
}

void Parser::clearMessage()
{
	header.clear();
	queries.clear();
	responses.clear();
	authRecs.clear();
	additRecs.clear();
}

}
