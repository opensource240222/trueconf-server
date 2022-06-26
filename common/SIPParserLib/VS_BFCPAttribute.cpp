#include "VS_BFCPAttribute.h"
#include "std-generic/cpplib/hton.h"

#include <cassert>
#include <cstring>
#include <numeric>

namespace bfcp {

Attribute::~Attribute()
{
}

void Attribute::UpdateView()
{
}

std::unique_ptr<Attribute> Attribute::DecodeAny(const void* buffer, size_t& size, DecodeStatus& status)
{
	if (size < 2)
	{
		size = 2;
		status.type = DecodeStatus::PartialData;
		return nullptr;
	}

	const uint8_t* p = reinterpret_cast<const uint8_t*>(buffer);
	const AttributeType type = AttributeType(*(p) >> 1);

	std::unique_ptr<Attribute> a;
	switch (type)
	{
	case AttributeType::BENEFICIARY_ID:            a.reset(new Attribute_BENEFICIARY_ID); break;
	case AttributeType::FLOOR_ID:                  a.reset(new Attribute_FLOOR_ID); break;
	case AttributeType::FLOOR_REQUEST_ID:          a.reset(new Attribute_FLOOR_REQUEST_ID); break;
	case AttributeType::PRIORITY:                  a.reset(new Attribute_PRIORITY); break;
	case AttributeType::REQUEST_STATUS:            a.reset(new Attribute_REQUEST_STATUS); break;
	case AttributeType::ERROR_CODE:                a.reset(new Attribute_ERROR_CODE); break;
	case AttributeType::ERROR_INFO:                a.reset(new Attribute_ERROR_INFO); break;
	case AttributeType::PARTICIPANT_PROVIDED_INFO: a.reset(new Attribute_PARTICIPANT_PROVIDED_INFO); break;
	case AttributeType::STATUS_INFO:               a.reset(new Attribute_STATUS_INFO); break;
	case AttributeType::SUPPORTED_ATTRIBUTES:      a.reset(new Attribute_SUPPORTED_ATTRIBUTES); break;
	case AttributeType::SUPPORTED_PRIMITIVES:      a.reset(new Attribute_SUPPORTED_PRIMITIVES); break;
	case AttributeType::USER_DISPLAY_NAME:         a.reset(new Attribute_USER_DISPLAY_NAME); break;
	case AttributeType::USER_URI:                  a.reset(new Attribute_USER_URI); break;
	case AttributeType::BENEFICIARY_INFORMATION:   a.reset(new Attribute_BENEFICIARY_INFORMATION); break;
	case AttributeType::FLOOR_REQUEST_INFORMATION: a.reset(new Attribute_FLOOR_REQUEST_INFORMATION); break;
	case AttributeType::REQUESTED_BY_INFORMATION:  a.reset(new Attribute_REQUESTED_BY_INFORMATION); break;
	case AttributeType::FLOOR_REQUEST_STATUS:      a.reset(new Attribute_FLOOR_REQUEST_STATUS); break;
	case AttributeType::OVERALL_REQUEST_STATUS:    a.reset(new Attribute_OVERALL_REQUEST_STATUS); break;
	default: break;
	}

	if (!a)
	{
		const size_t attribute_size = *(p+1);
		if (attribute_size < 4)
		{
			size = 0;
			status.type = DecodeStatus::MalformedData;
			return nullptr;
		}
		const size_t padding_bytes = 3 - (attribute_size + 3) % 4;
		if (size < attribute_size + padding_bytes)
		{
			size = attribute_size + padding_bytes;
			status.type = DecodeStatus::PartialData;
			return nullptr;
		}
		size = attribute_size + padding_bytes;

		const bool mandatory = (*(p) & 0x01) != 0;
		if (mandatory)
		{
			status.type = DecodeStatus::UnknownMandatoryAttribute;
			status.attribute_type = *(p) >> 1;
			return nullptr;
		}

		// We don't know about this attribute, but we shouldn't treat this as
		// error because standard explicitly allows for extension attributes
		// and this attribute it is not mandatory.
		status.type = DecodeStatus::Success;
		return nullptr;
	}

	a->Decode(buffer, size, status);
	if (status.type != DecodeStatus::Success)
		return nullptr;

	a->UpdateView();
	return a;
}

void Attribute::Decode_Unsigned16(const void* buffer, size_t& size, DecodeStatus& status, uint16_t& value)
{
	if (size < 2)
	{
		size = 2;
		status.type = DecodeStatus::PartialData;
		return;
	}

	const uint8_t* p = reinterpret_cast<const uint8_t*>(buffer);
	const size_t attribute_size = *(p+1);
	if (attribute_size != 4)
	{
		size = 0;
		status.type = DecodeStatus::MalformedData;
		return;
	}
	if (size < attribute_size)
	{
		size = attribute_size;
		status.type = DecodeStatus::PartialData;
		return;
	}

	if (type != AttributeType(*(p) >> 1))
	{
		size = attribute_size;
		status.type = DecodeStatus::TypeMismatch;
		return;
	}

	mandatory = (*(p) & 0x01) != 0;
	value = vs_ntohs(*reinterpret_cast<const uint16_t*>(p+2));

	size = attribute_size;
	status.type = DecodeStatus::Success;
}

void Attribute::Decode_OctetString16(const void* buffer, size_t& size, DecodeStatus& status, uint8_t& b1, uint8_t& b2)
{
	if (size < 2)
	{
		size = 2;
		status.type = DecodeStatus::PartialData;
		return;
	}

	const uint8_t* p = reinterpret_cast<const uint8_t*>(buffer);
	const size_t attribute_size = *(p+1);
	if (attribute_size != 4)
	{
		size = 0;
		status.type = DecodeStatus::MalformedData;
		return;
	}
	if (size < attribute_size)
	{
		size = attribute_size;
		status.type = DecodeStatus::PartialData;
		return;
	}

	if (type != AttributeType(*(p) >> 1))
	{
		size = attribute_size;
		status.type = DecodeStatus::TypeMismatch;
		return;
	}

	mandatory = (*(p) & 0x01) != 0;
	b1 = *(p+2);
	b2 = *(p+3);

	size = attribute_size;
	status.type = DecodeStatus::Success;
}

void Attribute::Decode_OctetString(const void* buffer, size_t& size, DecodeStatus& status, const void*& data, size_t& length)
{
	if (size < 2)
	{
		size = 2;
		status.type = DecodeStatus::PartialData;
		return;
	}

	const uint8_t* p = reinterpret_cast<const uint8_t*>(buffer);
	const size_t attribute_size = *(p+1);
	if (attribute_size < 2)
	{
		size = 0;
		status.type = DecodeStatus::MalformedData;
		return;
	}
	const size_t padding_bytes = 3 - (attribute_size + 3) % 4;
	if (size < attribute_size + padding_bytes)
	{
		size = attribute_size + padding_bytes;
		status.type = DecodeStatus::PartialData;
		return;
	}

	if (type != AttributeType(*(p) >> 1))
	{
		size = attribute_size + padding_bytes;
		status.type = DecodeStatus::TypeMismatch;
		return;
	}

	mandatory = (*(p) & 0x01) != 0;
	data = p+2;
	length = attribute_size - 2;

	size = attribute_size + padding_bytes;
	status.type = DecodeStatus::Success;
}

bool Attribute::Encode_Unsigned16(void* buffer, size_t& size, uint16_t value) const
{
	const size_t attribute_size = 4;
	if (size < attribute_size)
	{
		size = attribute_size;
		return false;
	}

	uint8_t* p = reinterpret_cast<uint8_t*>(buffer);
	*p = (uint8_t(type) << 1) | (mandatory ? 0x01 : 0x00); p += 1;
	*p = attribute_size; p += 1;
	*reinterpret_cast<uint16_t*>(p) = vs_htons(value); p += 2;

	size = attribute_size;
	return true;
}

bool Attribute::Encode_OctetString16(void* buffer, size_t& size, uint8_t b1, uint8_t b2) const
{
	const size_t attribute_size = 4;
	if (size < attribute_size)
	{
		size = attribute_size;
		return false;
	}

	uint8_t* p = reinterpret_cast<uint8_t*>(buffer);
	*p = (uint8_t(type) << 1) | (mandatory ? 0x01 : 0x00); p += 1;
	*p = attribute_size; p += 1;
	*p = b1; p += 1;
	*p = b2; p += 1;

	size = attribute_size;
	return true;
}

bool Attribute::Encode_OctetString(void* buffer, size_t& size, const void* data, size_t length) const
{
	const size_t attribute_size = 2 + length;
	const size_t padding_bytes = 3 - (attribute_size + 3) % 4;
	if (size < attribute_size + padding_bytes)
	{
		size = attribute_size + padding_bytes;
		return false;
	}

	uint8_t* p = reinterpret_cast<uint8_t*>(buffer);
	*p = (uint8_t(type) << 1) | (mandatory ? 0x01 : 0x00); p += 1;
	*p = attribute_size; p += 1;
	std::memcpy(p, data, length); p += length;
	std::memset(p, 0, padding_bytes);

	size = attribute_size + padding_bytes;
	return true;
}

void AttributeGrouped::Decode_Grouped(const void* buffer, size_t& size, DecodeStatus& status, uint16_t& value)
{
	if (size < 2)
	{
		size = 2;
		status.type = DecodeStatus::PartialData;
		return;
	}

	const uint8_t* p = reinterpret_cast<const uint8_t*>(buffer);
	const size_t attribute_size = *(p+1);
	if (attribute_size < 4)
	{
		// Technically a valid grouped attribute can be just 2 bytes long (plus
		// 2 padding bytes). But all grouped attributes that we support must
		// have an additional 16-bit value field, so we require at least 4 bytes.
		size = 0;
		status.type = DecodeStatus::MalformedData;
		return;
	}
	const size_t padding_bytes = 3 - (attribute_size + 3) % 4;
	if (size < attribute_size + padding_bytes)
	{
		size = attribute_size + padding_bytes;
		status.type = DecodeStatus::PartialData;
		return;
	}

	if (type != AttributeType(*(p) >> 1))
	{
		size = attribute_size + padding_bytes;
		status.type = DecodeStatus::TypeMismatch;
		return;
	}

	mandatory = (*(p) & 0x01) != 0;
	value = vs_ntohs(*reinterpret_cast<const uint16_t*>(p+2));

	p += 4;
	const uint8_t* const p_end = reinterpret_cast<const uint8_t*>(buffer) + attribute_size;
	while (p < p_end)
	{
		size_t a_sz(p_end - p);
		std::unique_ptr<Attribute> a(Attribute::DecodeAny(p, a_sz, status));
		if (status.type != DecodeStatus::Success)
		{
			size = attribute_size + padding_bytes;
			status.type = DecodeStatus::MalformedData;
			return;
		}
		if (a)
			attrs.push_back(std::move(a));
		p += a_sz;
	}
	assert(p == p_end);

	size = attribute_size + padding_bytes;
}

bool AttributeGrouped::Encode_Grouped(void* buffer, size_t& size, uint16_t value) const
{
	const size_t attribute_size = 4 + std::accumulate(attrs.begin(), attrs.end(), 0u, [](size_t total, const std::unique_ptr<Attribute>& a) {
		size_t a_sz(0);
		a->Encode(nullptr, a_sz);
		return total + a_sz;
	});
	const size_t padding_bytes = 3 - (attribute_size + 3) % 4;
	if (size < attribute_size + padding_bytes)
	{
		size = attribute_size + padding_bytes;
		return false;
	}

	uint8_t* p = reinterpret_cast<uint8_t*>(buffer);
	*p = (uint8_t(type) << 1) | (mandatory ? 0x01 : 0x00); p += 1;
	*p = attribute_size; p += 1;
	*reinterpret_cast<uint16_t*>(p) = vs_htons(value); p += 2;

	uint8_t* const p_end = reinterpret_cast<uint8_t*>(buffer) + attribute_size;
	for (auto& a : attrs)
	{
		size_t a_sz(p_end - p);
		bool success = a->Encode(p, a_sz);
		assert(success);
		p += a_sz;
	}
	assert(p == p_end);
	std::memset(p, 0, padding_bytes);

	size = attribute_size + padding_bytes;
	return true;
}

void Attribute_BENEFICIARY_ID::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	Decode_Unsigned16(buffer, size, status, value);
}

bool Attribute_BENEFICIARY_ID::Encode(void* buffer, size_t& size) const
{
	return Encode_Unsigned16(buffer, size, value);
}

void Attribute_FLOOR_ID::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	Decode_Unsigned16(buffer, size, status, value);
}

bool Attribute_FLOOR_ID::Encode(void* buffer, size_t& size) const
{
	return Encode_Unsigned16(buffer, size, value);
}

void Attribute_FLOOR_REQUEST_ID::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	Decode_Unsigned16(buffer, size, status, value);
}

bool Attribute_FLOOR_REQUEST_ID::Encode(void* buffer, size_t& size) const
{
	return Encode_Unsigned16(buffer, size, value);
}

void Attribute_PRIORITY::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	uint8_t b1;
	uint8_t b2;

	Decode_OctetString16(buffer, size, status, b1, b2);
	if (status.type == DecodeStatus::Success)
	{
		b1 &= 0x0f;
		if (b1 <= 4)
			value = Priority(b1);
		else
			value = Priority::Highest;
	}
}

bool Attribute_PRIORITY::Encode(void* buffer, size_t& size) const
{
	return Encode_OctetString16(buffer, size, uint8_t(value)&0x0f, 0);
}

void Attribute_REQUEST_STATUS::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	uint8_t b1;

	Decode_OctetString16(buffer, size, status, b1, queue_position);
	if (status.type == DecodeStatus::Success)
	{
		if (b1 < 1 || b1 > 7)
		{
			status.type = DecodeStatus::MalformedData;
			return;
		}
		request_status = RequestStatus(b1);
	}
}

bool Attribute_REQUEST_STATUS::Encode(void* buffer, size_t& size) const
{
	return Encode_OctetString16(buffer, size, uint8_t(request_status), queue_position);
}

void Attribute_ERROR_CODE::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	const void* data(nullptr);
	size_t length(0);

	Decode_OctetString(buffer, size, status, data, length);
	if (status.type == DecodeStatus::Success)
	{
		const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
		if (length < 1)
		{
			status.type = DecodeStatus::MalformedData;
			return;
		}
		code = *(p);
		details.assign(p+1, p+length);
	}
}

bool Attribute_ERROR_CODE::Encode(void* buffer, size_t& size) const
{
	const size_t attribute_size = 2 + 1 + details.size();
	const size_t padding_bytes = 3 - (attribute_size + 3) % 4;
	if (size < attribute_size + padding_bytes)
	{
		size = attribute_size + padding_bytes;
		return false;
	}

	uint8_t* p = reinterpret_cast<uint8_t*>(buffer);
	*p = (uint8_t(type) << 1) | (mandatory ? 0x01 : 0x00); p += 1;
	*p = attribute_size; p += 1;
	*p = code; p += 1;
	std::memcpy(p, details.data(), details.size()); p += details.size();
	std::memset(p, 0, padding_bytes);

	size = attribute_size + padding_bytes;
	return true;
}

void Attribute_ERROR_INFO::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	const void* data(nullptr);
	size_t length(0);

	Decode_OctetString(buffer, size, status, data, length);
	if (status.type == DecodeStatus::Success)
	{
		const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
		text.assign(p, p+length);
	}
}

bool Attribute_ERROR_INFO::Encode(void* buffer, size_t& size) const
{
	return Encode_OctetString(buffer, size, text.data(), text.size());
}

void Attribute_PARTICIPANT_PROVIDED_INFO::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	const void* data(nullptr);
	size_t length(0);

	Decode_OctetString(buffer, size, status, data, length);
	if (status.type == DecodeStatus::Success)
	{
		const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
		text.assign(p, p+length);
	}
}

bool Attribute_PARTICIPANT_PROVIDED_INFO::Encode(void* buffer, size_t& size) const
{
	return Encode_OctetString(buffer, size, text.data(), text.size());
}

void Attribute_STATUS_INFO::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	const void* data(nullptr);
	size_t length(0);

	Decode_OctetString(buffer, size, status, data, length);
	if (status.type == DecodeStatus::Success)
	{
		const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
		text.assign(p, p+length);
	}
}

bool Attribute_STATUS_INFO::Encode(void* buffer, size_t& size) const
{
	return Encode_OctetString(buffer, size, text.data(), text.size());
}

void Attribute_SUPPORTED_ATTRIBUTES::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	const void* data(nullptr);
	size_t length(0);

	Decode_OctetString(buffer, size, status, data, length);
	if (status.type == DecodeStatus::Success)
	{
		const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
		const uint8_t* const p_end = reinterpret_cast<const uint8_t*>(data) + length;
		while (p < p_end)
		{
			values.push_back(AttributeType(*(p) >> 1)); p += 1;
		}
	}
}

bool Attribute_SUPPORTED_ATTRIBUTES::Encode(void* buffer, size_t& size) const
{
	const size_t attribute_size = 2 + values.size();
	const size_t padding_bytes = 3 - (attribute_size + 3) % 4;
	if (size < attribute_size + padding_bytes)
	{
		size = attribute_size + padding_bytes;
		return false;
	}

	uint8_t* p = reinterpret_cast<uint8_t*>(buffer);
	*p = (uint8_t(type) << 1) | (mandatory ? 0x01 : 0x00); p += 1;
	*p = attribute_size; p += 1;
	for (AttributeType v: values)
	{
		*p = uint8_t(v) << 1; p += 1;
	}
	std::memset(p, 0, padding_bytes);

	size = attribute_size + padding_bytes;
	return true;
}

void Attribute_SUPPORTED_PRIMITIVES::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	const void* data(nullptr);
	size_t length(0);

	Decode_OctetString(buffer, size, status, data, length);
	if (status.type == DecodeStatus::Success)
	{
		const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
		const uint8_t* const p_end = reinterpret_cast<const uint8_t*>(data) + length;
		while (p < p_end)
		{
			values.push_back(PrimitiveType(*(p))); p += 1;
		}
	}
}

bool Attribute_SUPPORTED_PRIMITIVES::Encode(void* buffer, size_t& size) const
{
	const size_t attribute_size = 2 + values.size();
	const size_t padding_bytes = 3 - (attribute_size + 3) % 4;
	if (size < attribute_size + padding_bytes)
	{
		size = attribute_size + padding_bytes;
		return false;
	}

	uint8_t* p = reinterpret_cast<uint8_t*>(buffer);
	*p = (uint8_t(type) << 1) | (mandatory ? 0x01 : 0x00); p += 1;
	*p = attribute_size; p += 1;
	for (PrimitiveType v: values)
	{
		*p = uint8_t(v); p += 1;
	}
	std::memset(p, 0, padding_bytes);

	size = attribute_size + padding_bytes;
	return true;
}

void Attribute_USER_DISPLAY_NAME::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	const void* data(nullptr);
	size_t length(0);

	Decode_OctetString(buffer, size, status, data, length);
	if (status.type == DecodeStatus::Success)
	{
		const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
		text.assign(p, p+length);
	}
}

bool Attribute_USER_DISPLAY_NAME::Encode(void* buffer, size_t& size) const
{
	return Encode_OctetString(buffer, size, text.data(), text.size());
}

void Attribute_USER_URI::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	const void* data(nullptr);
	size_t length(0);

	Decode_OctetString(buffer, size, status, data, length);
	if (status.type == DecodeStatus::Success)
	{
		const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
		text.assign(p, p+length);
	}
}

bool Attribute_USER_URI::Encode(void* buffer, size_t& size) const
{
	return Encode_OctetString(buffer, size, text.data(), text.size());
}

void Attribute_BENEFICIARY_INFORMATION::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	Decode_Grouped(buffer, size, status, beneficiary_id);
}

bool Attribute_BENEFICIARY_INFORMATION::Encode(void* buffer, size_t& size) const
{
	return Encode_Grouped(buffer, size, beneficiary_id);
}

void Attribute_FLOOR_REQUEST_INFORMATION::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	Decode_Grouped(buffer, size, status, floor_request_id);
}

bool Attribute_FLOOR_REQUEST_INFORMATION::Encode(void* buffer, size_t& size) const
{
	return Encode_Grouped(buffer, size, floor_request_id);
}

void Attribute_REQUESTED_BY_INFORMATION::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	Decode_Grouped(buffer, size, status, requested_by_id);
}

bool Attribute_REQUESTED_BY_INFORMATION::Encode(void* buffer, size_t& size) const
{
	return Encode_Grouped(buffer, size, requested_by_id);
}

void Attribute_FLOOR_REQUEST_STATUS::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	Decode_Grouped(buffer, size, status, floor_id);
}

bool Attribute_FLOOR_REQUEST_STATUS::Encode(void* buffer, size_t& size) const
{
	return Encode_Grouped(buffer, size, floor_id);
}

void Attribute_OVERALL_REQUEST_STATUS::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	Decode_Grouped(buffer, size, status, floor_request_id);
}

bool Attribute_OVERALL_REQUEST_STATUS::Encode(void* buffer, size_t& size) const
{
	return Encode_Grouped(buffer, size, floor_request_id);
}

void Attribute_ERROR_CODE::AddUnknownAttribute(uint8_t x)
{
	details.push_back(x << 1);
}

void Attribute_BENEFICIARY_INFORMATION::UpdateView()
{
	attr_USER_DISPLAY_NAME = nullptr;
	attr_USER_URI = nullptr;

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::USER_DISPLAY_NAME:
			attr_USER_DISPLAY_NAME = a->CastTo<Attribute_USER_DISPLAY_NAME>();
			break;
		case AttributeType::USER_URI:
			attr_USER_URI = a->CastTo<Attribute_USER_URI>();
			break;
		default: break; // No other attributes should be placed in this one.
		}
	}
}

void Attribute_FLOOR_REQUEST_INFORMATION::UpdateView()
{
	attr_OVERALL_REQUEST_STATUS = nullptr;
	attrs_FLOOR_REQUEST_STATUS.clear();
	attr_BENEFICIARY_INFORMATION = nullptr;
	attr_REQUESTED_BY_INFORMATION = nullptr;
	attr_PRIORITY = nullptr;
	attr_PARTICIPANT_PROVIDED_INFO = nullptr;

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::OVERALL_REQUEST_STATUS:
			attr_OVERALL_REQUEST_STATUS = a->CastTo<Attribute_OVERALL_REQUEST_STATUS>();
			break;
		case AttributeType::FLOOR_REQUEST_STATUS:
			attrs_FLOOR_REQUEST_STATUS.push_back(a->CastTo<Attribute_FLOOR_REQUEST_STATUS>());
			break;
		case AttributeType::BENEFICIARY_INFORMATION:
			attr_BENEFICIARY_INFORMATION = a->CastTo<Attribute_BENEFICIARY_INFORMATION>();
			break;
		case AttributeType::REQUESTED_BY_INFORMATION:
			attr_REQUESTED_BY_INFORMATION = a->CastTo<Attribute_REQUESTED_BY_INFORMATION>();
			break;
		case AttributeType::PRIORITY:
			attr_PRIORITY = a->CastTo<Attribute_PRIORITY>();
			break;
		case AttributeType::PARTICIPANT_PROVIDED_INFO:
			attr_PARTICIPANT_PROVIDED_INFO = a->CastTo<Attribute_PARTICIPANT_PROVIDED_INFO>();
			break;
		default: break; // No other attributes should be placed in this one.
		}
	}
}

void Attribute_REQUESTED_BY_INFORMATION::UpdateView()
{
	attr_USER_DISPLAY_NAME = nullptr;
	attr_USER_URI = nullptr;

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::USER_DISPLAY_NAME:
			attr_USER_DISPLAY_NAME = a->CastTo<Attribute_USER_DISPLAY_NAME>();
			break;
		case AttributeType::USER_URI:
			attr_USER_URI = a->CastTo<Attribute_USER_URI>();
			break;
		default: break; // No other attributes should be placed in this one.
		}
	}
}

void Attribute_FLOOR_REQUEST_STATUS::UpdateView()
{
	attr_REQUEST_STATUS = nullptr;
	attr_STATUS_INFO = nullptr;

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::REQUEST_STATUS:
			attr_REQUEST_STATUS = a->CastTo<Attribute_REQUEST_STATUS>();
			break;
		case AttributeType::STATUS_INFO:
			attr_STATUS_INFO = a->CastTo<Attribute_STATUS_INFO>();
			break;
		default: break; // No other attributes should be placed in this one.
		}
	}
}

void Attribute_OVERALL_REQUEST_STATUS::UpdateView()
{
	attr_REQUEST_STATUS = nullptr;
	attr_STATUS_INFO = nullptr;

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::REQUEST_STATUS:
			attr_REQUEST_STATUS = a->CastTo<Attribute_REQUEST_STATUS>();
			break;
		case AttributeType::STATUS_INFO:
			attr_STATUS_INFO = a->CastTo<Attribute_STATUS_INFO>();
			break;
		case AttributeType::FLOOR_REQUEST_STATUS:
			// Polycom puts FLOOR-REQUEST-STATUS inside OVERALL-REQUEST-STATUS instead of FLOOR-REQUEST-INFORMATION
			attr_FLOOR_REQUEST_STATUS = a->CastTo<Attribute_FLOOR_REQUEST_STATUS>();
			break;
		default: break; // No other attributes should be placed in this one.
		}
	}
}

}
