#include "VS_BFCPMessage.h"
#include "VS_BFCPAttribute.h"
#include "std-generic/cpplib/hton.h"

#include <cassert>
#include <numeric>

namespace bfcp {

inline PrimitiveType AdjustPrimitiveTypeByVersion(PrimitiveType type, uint8_t version)
{
	if (version == 1) switch (type)
	{
	case PrimitiveType::FloorStatusAck: return PrimitiveType::FloorStatusAck_v1;
	case PrimitiveType::Goodbye:        return PrimitiveType::Goodbye_v1;
	case PrimitiveType::GoodbyeAck:     return PrimitiveType::GoodbyeAck_v1;
	default: return type;
	}
	return type;
}

Message::~Message()
{
}

std::unique_ptr<Message> Message::DecodeAny(const void* buffer, size_t& size, DecodeStatus& status)
{
	if (size < 4)
	{
		size = 4;
		status.type = DecodeStatus::PartialData;
		return nullptr;
	}

	const uint8_t* p = reinterpret_cast<const uint8_t*>(buffer);
	const uint8_t version = *(p) >> 5;
	if (version < 1 || version > 2)
	{
		size = 0;
		status.type = DecodeStatus::UnknownVersion;
		return nullptr;
	}

	const PrimitiveType type = PrimitiveType(*(p+1));

	std::unique_ptr<Message> m;
	switch (type)
	{
	case PrimitiveType::invalid: break;
	case PrimitiveType::FloorRequest:          m.reset(new Message_FloorRequest); break;
	case PrimitiveType::FloorRelease:          m.reset(new Message_FloorRelease); break;
	case PrimitiveType::FloorRequestQuery:     m.reset(new Message_FloorRequestQuery); break;
	case PrimitiveType::FloorRequestStatus:    m.reset(new Message_FloorRequestStatus); break;
	case PrimitiveType::UserQuery:             m.reset(new Message_UserQuery); break;
	case PrimitiveType::UserStatus:            m.reset(new Message_UserStatus); break;
	case PrimitiveType::FloorQuery:            m.reset(new Message_FloorQuery); break;
	case PrimitiveType::FloorStatus:           m.reset(new Message_FloorStatus); break;
	case PrimitiveType::ChairAction:           m.reset(new Message_ChairAction); break;
	case PrimitiveType::ChairActionAck:        m.reset(new Message_ChairActionAck); break;
	case PrimitiveType::Hello:                 m.reset(new Message_Hello); break;
	case PrimitiveType::HelloAck:              m.reset(new Message_HelloAck); break;
	case PrimitiveType::Error:                 m.reset(new Message_Error); break;
	case PrimitiveType::FloorRequestStatusAck: m.reset(new Message_FloorRequestStatusAck); break;
	// Handled below
	case PrimitiveType::FloorStatusAck:
	case PrimitiveType::Goodbye: // == PrimitiveType::FloorStatusAck_v1
	case PrimitiveType::GoodbyeAck: // == PrimitiveType::Goodbye_v1
	case PrimitiveType::GoodbyeAck_v1:
		break;
	}
	if (version == 1) switch (type)
	{
	case PrimitiveType::FloorStatusAck_v1:     m.reset(new Message_FloorStatusAck); break;
	case PrimitiveType::Goodbye_v1:            m.reset(new Message_Goodbye); break;
	case PrimitiveType::GoodbyeAck_v1:         m.reset(new Message_GoodbyeAck); break;
	default: break;
	}
	else switch (type)
	{
	case PrimitiveType::FloorStatusAck:        m.reset(new Message_FloorStatusAck); break;
	case PrimitiveType::Goodbye:               m.reset(new Message_Goodbye); break;
	case PrimitiveType::GoodbyeAck:            m.reset(new Message_GoodbyeAck); break;
	default: break;
	}

	if (!m)
	{
		const size_t message_size = 12 + 4 * vs_ntohs(*reinterpret_cast<const uint16_t*>(p+2));
		size = message_size;

		status.type = DecodeStatus::UnknownPrimitive;
		status.primitive_type = *(p+1);
		return nullptr;
	}

	m->Decode(buffer, size, status);
	if (status.type != DecodeStatus::Success)
		return nullptr;

	m->UpdateView();
	return m;
}

void Message::Decode(const void* buffer, size_t& size, DecodeStatus& status)
{
	if (size < 12)
	{
		size = 12;
		status.type = DecodeStatus::PartialData;
		return;
	}

	const uint8_t* p = reinterpret_cast<const uint8_t*>(buffer);
	version = *(p) >> 5;
	if (version < 1 || version > 2)
	{
		size = 0;
		status.type = DecodeStatus::UnknownVersion;
		return;
	}

	const size_t message_size = 12 + 4 * vs_ntohs(*reinterpret_cast<const uint16_t*>(p+2));
	if (size < message_size)
	{
		size = message_size;
		status.type = DecodeStatus::PartialData;
		return;
	}

	if (AdjustPrimitiveTypeByVersion(type, version) != PrimitiveType(*(p + 1)))
	{
		size = message_size;
		status.type = DecodeStatus::TypeMismatch;
		return;
	}

	conference_id = vs_ntohl(*reinterpret_cast<const uint32_t*>(p+4));
	transaction_id = vs_ntohs(*reinterpret_cast<const uint16_t*>(p+8));
	user_id = vs_ntohs(*reinterpret_cast<const uint16_t*>(p+10));

	p += 12;
	const uint8_t* const p_end = reinterpret_cast<const uint8_t*>(buffer) + message_size;
	while (p < p_end)
	{
		size_t a_sz(p_end - p);
		std::unique_ptr<Attribute> a(Attribute::DecodeAny(p, a_sz, status));
		if (status.type != DecodeStatus::Success)
		{
			size = message_size;
			return;
		}
		if (a)
			attrs.push_back(std::move(a));
		p += a_sz;
	}
	assert(p == p_end);

	size = message_size;
	status.type = DecodeStatus::Success;
}

bool Message::Encode(void* buffer, size_t& size) const
{
	const size_t attrs_size = std::accumulate(attrs.begin(), attrs.end(), 0u, [](size_t total, const std::unique_ptr<Attribute>& a) {
		size_t a_sz(0);
		a->Encode(nullptr, a_sz);
		return total + a_sz;
	});
	assert(attrs_size % 4 == 0);
	const size_t message_size = 12 + attrs_size;
	if (size < message_size)
	{
		size = message_size;
		return false;
	}

	uint8_t* p = reinterpret_cast<uint8_t*>(buffer);
	*p = uint8_t(version) << 5; p += 1;
	*p = uint8_t(AdjustPrimitiveTypeByVersion(type, version)); p += 1;
	*reinterpret_cast<uint16_t*>(p) = vs_htons(attrs_size/4); p += 2;
	*reinterpret_cast<uint32_t*>(p) = vs_htonl(conference_id); p += 4;
	*reinterpret_cast<uint16_t*>(p) = vs_htons(transaction_id); p += 2;
	*reinterpret_cast<uint16_t*>(p) = vs_htons(user_id); p += 2;

	uint8_t* const p_end = reinterpret_cast<uint8_t*>(buffer) + message_size;
	for (auto& a : attrs)
	{
		size_t a_sz(p_end - p);
		bool success = a->Encode(p, a_sz);
		assert(success);
		p += a_sz;
	}
	assert(p == p_end);

	size = message_size;
	return true;
}

void Message_FloorRequest::UpdateView()
{
	attrs_FLOOR_ID.clear();
	attr_BENEFICIARY_ID = nullptr;
	attr_PARTICIPANT_PROVIDED_INFO = nullptr;
	attr_PRIORITY = nullptr;

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::FLOOR_ID:
			attrs_FLOOR_ID.push_back(a->CastTo<Attribute_FLOOR_ID>());
			break;
		case AttributeType::BENEFICIARY_ID:
			attr_BENEFICIARY_ID = a->CastTo<Attribute_BENEFICIARY_ID>();
			break;
		case AttributeType::PARTICIPANT_PROVIDED_INFO:
			attr_PARTICIPANT_PROVIDED_INFO = a->CastTo<Attribute_PARTICIPANT_PROVIDED_INFO>();
			break;
		case AttributeType::PRIORITY:
			attr_PRIORITY = a->CastTo<Attribute_PRIORITY>();
			break;
		default: break; // No other attributes should be placed in this message.
		}
	}
}

void Message_FloorRelease::UpdateView()
{
	attr_FLOOR_REQUEST_ID = nullptr;

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::FLOOR_REQUEST_ID:
			attr_FLOOR_REQUEST_ID = a->CastTo<Attribute_FLOOR_REQUEST_ID>();
			break;
		default: break; // No other attributes should be placed in this message.
		}
	}
}

void Message_FloorRequestQuery::UpdateView()
{
	attr_FLOOR_REQUEST_ID = nullptr;

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::FLOOR_REQUEST_ID:
			attr_FLOOR_REQUEST_ID = a->CastTo<Attribute_FLOOR_REQUEST_ID>();
			break;
		default: break; // No other attributes should be placed in this message.
		}
	}
}

void Message_FloorRequestStatus::UpdateView()
{
	attr_FLOOR_REQUEST_INFORMATION = nullptr;

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::FLOOR_REQUEST_INFORMATION:
			attr_FLOOR_REQUEST_INFORMATION = a->CastTo<Attribute_FLOOR_REQUEST_INFORMATION>();
			break;
		default: break; // No other attributes should be placed in this message.
		}
	}
}

void Message_UserQuery::UpdateView()
{
	attr_BENEFICIARY_ID = nullptr;

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::BENEFICIARY_ID:
			attr_BENEFICIARY_ID = a->CastTo<Attribute_BENEFICIARY_ID>();
			break;
		default: break; // No other attributes should be placed in this message.
		}
	}
}

void Message_UserStatus::UpdateView()
{
	attr_BENEFICIARY_INFORMATION = nullptr;
	attrs_FLOOR_REQUEST_INFORMATION.clear();

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::BENEFICIARY_INFORMATION:
			attr_BENEFICIARY_INFORMATION = a->CastTo<Attribute_BENEFICIARY_INFORMATION>();
			break;
		case AttributeType::FLOOR_REQUEST_INFORMATION:
			attrs_FLOOR_REQUEST_INFORMATION.push_back(a->CastTo<Attribute_FLOOR_REQUEST_INFORMATION>());
			break;
		default: break; // No other attributes should be placed in this message.
		}
	}
}

void Message_FloorQuery::UpdateView()
{
	attrs_FLOOR_ID.clear();

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::FLOOR_ID:
			attrs_FLOOR_ID.push_back(a->CastTo<Attribute_FLOOR_ID>());
			break;
		default: break; // No other attributes should be placed in this message.
		}
	}
}

void Message_FloorStatus::UpdateView()
{
	attr_FLOOR_ID = nullptr;
	attrs_FLOOR_REQUEST_INFORMATION.clear();

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::FLOOR_ID:
			attr_FLOOR_ID = a->CastTo<Attribute_FLOOR_ID>();
			break;
		case AttributeType::FLOOR_REQUEST_INFORMATION:
			attrs_FLOOR_REQUEST_INFORMATION.push_back(a->CastTo<Attribute_FLOOR_REQUEST_INFORMATION>());
			break;
		default: break; // No other attributes should be placed in this message.
		}
	}
}

void Message_ChairAction::UpdateView()
{
	attr_FLOOR_REQUEST_INFORMATION = nullptr;

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::FLOOR_REQUEST_INFORMATION:
			attr_FLOOR_REQUEST_INFORMATION = a->CastTo<Attribute_FLOOR_REQUEST_INFORMATION>();
			break;
		default: break; // No other attributes should be placed in this message.
		}
	}
}

void Message_ChairActionAck::UpdateView()
{
}

void Message_Hello::UpdateView()
{
}

void Message_HelloAck::UpdateView()
{
	attr_SUPPORTED_PRIMITIVES = nullptr;
	attr_SUPPORTED_ATTRIBUTES = nullptr;

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::SUPPORTED_PRIMITIVES:
			attr_SUPPORTED_PRIMITIVES = a->CastTo<Attribute_SUPPORTED_PRIMITIVES>();
			break;
		case AttributeType::SUPPORTED_ATTRIBUTES:
			attr_SUPPORTED_ATTRIBUTES = a->CastTo<Attribute_SUPPORTED_ATTRIBUTES>();
			break;
		default: break; // No other attributes should be placed in this message.
		}
	}
}

void Message_Error::UpdateView()
{
	attr_ERROR_CODE = nullptr;
	attr_ERROR_INFO = nullptr;

	for (auto& a: attrs)
	{
		switch (a->type)
		{
		case AttributeType::ERROR_CODE:
			attr_ERROR_CODE = a->CastTo<Attribute_ERROR_CODE>();
			break;
		case AttributeType::ERROR_INFO:
			attr_ERROR_INFO = a->CastTo<Attribute_ERROR_INFO>();
			break;
		default: break; // No other attributes should be placed in this message.
		}
	}
}

void Message_FloorRequestStatusAck::UpdateView()
{
}

void Message_FloorStatusAck::UpdateView()
{
}

void Message_Goodbye::UpdateView()
{
}

void Message_GoodbyeAck::UpdateView()
{
}

}
