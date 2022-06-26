#include "Former.h"

#include "std-generic/cpplib/hton.h"

#include "tools.h"

namespace mdns
{

namespace
{
	uint16_t POINTER_BITS = 0xc000;
}

Former::Former()
{
	clear();
}

void Former::clear()
{
	packet_.clear();
	packet_.reserve(PACKET_SIZE_DEFAULT);
	header_.clear();
	queries_.clear();
	records_.clear();
	correct_ = false;
	formed_ = false;
}

bool Former::setHeader(mdns::QR qr, mdns::QD qd, mdns::AN an, mdns::NS ns, mdns::AR ar)
{
	header_  = mdns::Header(qd, an, ns, ar, qr);
	return correct_ = (header_.correct());
}

bool Former::setHeader(const mdns::Header& header)
{
	header_ = header;
	return correct_ = (header_.correct());
}

bool Former::addQuery(const std::vector<char>& name, mdns::TYPE qtype, mdns::QU qu)
{
	queries_.emplace_back(name, qtype, qu);
	return correct_ = correct_ && (queries_.back().correct());
}

bool Former::addQuery(const mdns::Query& query)
{
	queries_.push_back(query);
	return correct_ = correct_ && (query.correct());
}

bool Former::addRRecord(const std::vector<char>& name, mdns::TYPE rtype, mdns::TTL ttl,
	const std::vector<char>& rData, mdns::CACHEFLUSH cflush)
{
	records_.emplace_back(name, rtype, ttl, rData, cflush);
	return correct_ = correct_ && (records_.back().correct());
}

bool Former::addRRecord(const mdns::RRecord& record)
{
	records_.push_back(record);
	return correct_ = correct_ && (record.correct());
}

char* Former::form()
{
	names_.clear();
	formed_ = false;
	if (!correct_)
		return nullptr;
	size_t size = header_.size();
	for (const auto& query: queries_)
		size += query.size();
	for (const auto& record: records_)
		size += record.size();
	packet_.reserve(size);
	if (!pushHeader(header_))
		return nullptr;
	for (const auto& query: queries_)
		if (!pushQuery(query))
			return nullptr;
	for (const auto& record: records_)
		if (!pushRRecord(record))
			return nullptr;
	formed_ = true;
	return packet_.data();
}

bool Former::pushHeader(const mdns::Header& header)
{
	if (packet_.size() > 12)
		return correct_ = (header.form(packet_.data(), packet_.size()) > 0);
	else
	{
		packet_.resize(mdns::Header::HEADER_SIZE, '\0');
		return correct_ = (header.form(packet_.data(), packet_.size()) > 0);
	}
}

bool Former::pushQuery(const mdns::Query& query)
{
	size_t packetEnd = packet_.size();
	std::vector<char> compressedName;
	unsigned int labelsLeft = compressName(query.name, compressedName);
	insertNamePointers(query.name, labelsLeft, packetEnd);

	if (labelsLeft != getLabelCount(query.name))
	{
		mdns::Query queryCopy = query;
		queryCopy.name = compressedName;
		packet_.resize(packet_.size() + queryCopy.name.size() + 4);
		return correct_ = (queryCopy.form(packet_.data() + packetEnd, packet_.size() - packetEnd) > 0);
	}
	packet_.resize(packet_.size() + query.name.size() + 4);
	return correct_ = (query.form(packet_.data() + packetEnd, packet_.size() - packetEnd) > 0);
}

void Former::insertNamePointers(const std::vector<char>& name, unsigned int labelCount, size_t futureNamePosition)
{
	if (labelCount == 0)
		return;
	for (unsigned int i = 0; i < labelCount; ++i)
	{
		uint16_t pointerPosition = static_cast<uint16_t>(futureNamePosition) +
			static_cast<uint16_t>(getLabelOffset(name, i));
		if (pointerPosition >= POINTER_BITS)
			break;
		names_.emplace(cutFrontLabels(name, i), pointerPosition | POINTER_BITS);
	}
}

bool Former::pushRRecord(const mdns::RRecord& record)
{
	size_t packetEnd = packet_.size();
	std::vector<char> compressedName;
	unsigned int nameLabelsLeft = compressName(record.name, compressedName);

	insertNamePointers(record.name, nameLabelsLeft, packetEnd);

	std::vector<char> compressedDataName;
	std::vector<char> rDataName;
	unsigned int dataLabelsLeft;

	if (record.rtype == mdns::TYPE::PTR || record.rtype == mdns::TYPE::SRV)
	{
		int offset = 0;
		if (record.rtype == mdns::TYPE::SRV)
			offset = 6;
		rDataName.clear();
		rDataName.insert(rDataName.end(), record.rData.begin() + offset, record.rData.end());
		dataLabelsLeft = compressName(rDataName, compressedDataName);
	}

	if (!(rDataName.empty()))
	{
		int offset = 0;
		if (record.rtype == mdns::TYPE::SRV)
			offset = 6;
		if (!(compressedName.empty()))
			insertNamePointers(rDataName, dataLabelsLeft, packetEnd + compressedName.size() + 10 + offset);
		else
			insertNamePointers(rDataName, dataLabelsLeft, packetEnd + record.name.size() + 10 + offset);
	}

	if (!(compressedName.empty()) || !(compressedDataName.empty()))
	{
		mdns::RRecord recordCopy = record;
		if (!(compressedName.empty()))
			recordCopy.name = compressedName;
		if (!(compressedDataName.empty()))
		{
			if (record.rtype == mdns::TYPE::SRV)
			{
				recordCopy.rData.resize(6);
				recordCopy.rData.insert(recordCopy.rData.end(), compressedDataName.begin(), compressedDataName.end());
			} else
				recordCopy.rData = compressedDataName;
		}
		recordCopy.rdLength = static_cast<mdns::RDLENGTH>(recordCopy.rData.size());
		packet_.resize(packet_.size() + recordCopy.name.size() + 10 + recordCopy.rData.size());
		return correct_ = (recordCopy.form(packet_.data() + packetEnd, packet_.size() - packetEnd) > 0);
	}
	packet_.resize(packet_.size() + record.name.size() + 10 + record.rData.size());
	return correct_ = (record.form(packet_.data() + packetEnd, packet_.size() - packetEnd) > 0);
}

unsigned int Former::compressName(const std::vector<char>& name, std::vector<char>& result)
{
	unsigned int i = 0;
	unsigned int labelCount = getLabelCount(name);
	if (names_.empty())
		return labelCount;
	result.clear();
	for (i = 0; i < labelCount; ++i)
	{
		auto iter = names_.end();
		if (names_.end() != (iter = names_.find(cutFrontLabels(name, i))))
		{
			std::vector<char> nameStart = getFrontLabels(name, i);
			result.insert(result.end(), nameStart.begin(), nameStart.end());
			char pointerChar[2];
			*(reinterpret_cast<uint16_t*>(pointerChar)) = vs_htons(iter->second);
			result.push_back(pointerChar[0]);
			result.push_back(pointerChar[1]);
			return i;
		}
	}
	return labelCount;
}

}
