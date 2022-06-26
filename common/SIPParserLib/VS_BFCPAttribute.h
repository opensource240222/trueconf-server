#pragma once

#include "VS_BFCPAttribute_fwd.h"
#include "VS_BFCPTypes.h"
#include "std/cpplib/container_helpers.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace bfcp {

class Attribute
{
public:
	const AttributeType type;
	bool mandatory;

	virtual ~Attribute();
	virtual void UpdateView();

	// Tries to decode attribute from byte stream.
	// On success sets status to Success and size to amount of consumed bytes.
	// If buffer doesn't contain enough data to decode an attribute sets status to PartialData
	// and size to minimal amount of bytes required for next attempt.
	// If error occurs during decoding sets appropriate status and size to
	// amount of byte that might be skipped to recover from error.
	virtual void Decode(const void* buffer, size_t& size, DecodeStatus& status) = 0;

	// Encodes attribute to byte stream.
	// On success returns true and sets size to amount of produced bytes.
	// On failure returns false and sets size to minimal buffer size required to successful
	// encoding.
	virtual bool Encode(void* buffer, size_t& size) const = 0;

	static std::unique_ptr<Attribute> DecodeAny(const void* buffer, size_t& size, DecodeStatus& status);

	// Casts this object to a concrete attribute class, checking the 'type' field.
	template <class T>
	T* CastTo()
	{
		assert(this->type == T::s_type);
		return static_cast<T*>(this);
	}
	template <class T>
	const T* CastTo() const
	{
		assert(this->type == T::s_type);
		return static_cast<const T*>(this);
	}

protected:
	explicit Attribute(AttributeType type_)
		: type(type_)
		, mandatory(true)
	{
	}

	// Helper functions that implement decoding/encoding for common attribute layouts
	void Decode_Unsigned16(const void* buffer, size_t& size, DecodeStatus& status, uint16_t& value);
	void Decode_OctetString16(const void* buffer, size_t& size, DecodeStatus& status, uint8_t& b1, uint8_t& b2);
	void Decode_OctetString(const void* buffer, size_t& size, DecodeStatus& status, const void*& data, size_t& length);
	bool Encode_Unsigned16(void* buffer, size_t& size, uint16_t value) const;
	bool Encode_OctetString16(void* buffer, size_t& size, uint8_t b1, uint8_t b2) const;
	bool Encode_OctetString(void* buffer, size_t& size, const void* data, size_t length) const;
};

class AttributeGrouped : public Attribute
{
public:
	std::vector<std::unique_ptr<Attribute>> attrs;

protected:
	template <class... A>
	AttributeGrouped(AttributeType type_, A&&... a)
		: Attribute(type_)
	{
		push_back_into(attrs, std::forward<A>(a)...);
	}

	// Helper functions that implement decoding/encoding for common attribute layouts
	void Decode_Grouped(const void* buffer, size_t& size, DecodeStatus& status, uint16_t& value);
	bool Encode_Grouped(void* buffer, size_t& size, uint16_t value) const;
};

class Attribute_BENEFICIARY_ID : public Attribute
{
public:
	static const AttributeType s_type = AttributeType::BENEFICIARY_ID;
	UserID value;

	Attribute_BENEFICIARY_ID() : Attribute(s_type), value(0){}
	explicit Attribute_BENEFICIARY_ID(UserID value_) : Attribute(s_type), value(value_) {}
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_FLOOR_ID : public Attribute
{
public:
	static const AttributeType s_type = AttributeType::FLOOR_ID;
	FloorID value;

	Attribute_FLOOR_ID() : Attribute(s_type), value(0){}
	explicit Attribute_FLOOR_ID(FloorID value_) : Attribute(s_type), value(value_) {}
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_FLOOR_REQUEST_ID : public Attribute
{
public:
	static const AttributeType s_type = AttributeType::FLOOR_REQUEST_ID;
	FloorRequestID value;

	Attribute_FLOOR_REQUEST_ID() : Attribute(s_type), value(0) {}
	explicit Attribute_FLOOR_REQUEST_ID(FloorRequestID value_) : Attribute(s_type), value(value_) {}
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_PRIORITY : public Attribute
{
public:
	static const AttributeType s_type = AttributeType::PRIORITY;
	Priority value;

	Attribute_PRIORITY() : Attribute(s_type), value(Priority::invalid) {}
	explicit Attribute_PRIORITY(Priority value_) : Attribute(s_type), value(value_) {}
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_REQUEST_STATUS : public Attribute
{
public:
	static const AttributeType s_type = AttributeType::REQUEST_STATUS;
	RequestStatus request_status;
	uint8_t queue_position = 0;

	Attribute_REQUEST_STATUS() : Attribute(s_type), request_status(RequestStatus::invalid) {}
	Attribute_REQUEST_STATUS(RequestStatus request_status_, uint8_t queue_position_)
		: Attribute(s_type)
		, request_status(request_status_)
		, queue_position(queue_position_)
	{}
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_ERROR_CODE : public Attribute
{
public:
	static const AttributeType s_type = AttributeType::ERROR_CODE;
	uint8_t code = 0;
	std::vector<uint8_t> details;

	Attribute_ERROR_CODE() : Attribute(s_type) {}
	explicit Attribute_ERROR_CODE(uint8_t code_) : Attribute(s_type) , code(code_) {}
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;

	void AddUnknownAttribute(uint8_t x);
};

class Attribute_ERROR_INFO : public Attribute
{
public:
	static const AttributeType s_type = AttributeType::ERROR_INFO;
	std::string text;

	Attribute_ERROR_INFO() : Attribute(s_type) {}
	explicit Attribute_ERROR_INFO(std::string&& text_) : Attribute(s_type), text(std::move(text_)) {}
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_PARTICIPANT_PROVIDED_INFO : public Attribute
{
public:
	static const AttributeType s_type = AttributeType::PARTICIPANT_PROVIDED_INFO;
	std::string text;

	Attribute_PARTICIPANT_PROVIDED_INFO() : Attribute(s_type) {}
	explicit Attribute_PARTICIPANT_PROVIDED_INFO(std::string&& text_) : Attribute(s_type), text(std::move(text_)) {}
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_STATUS_INFO : public Attribute
{
public:
	static const AttributeType s_type = AttributeType::STATUS_INFO;
	std::string text;

	Attribute_STATUS_INFO() : Attribute(s_type) {}
	explicit Attribute_STATUS_INFO(std::string&& text_) : Attribute(s_type), text(std::move(text_)) {}
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_SUPPORTED_ATTRIBUTES : public Attribute
{
public:
	static const AttributeType s_type = AttributeType::SUPPORTED_ATTRIBUTES;
	std::vector<AttributeType> values;

	template <class... A>
	explicit Attribute_SUPPORTED_ATTRIBUTES(A&&... a)
		: Attribute(s_type)
	{
		push_back_into(values, std::forward<A>(a)...);
	}
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_SUPPORTED_PRIMITIVES : public Attribute
{
public:
	static const AttributeType s_type = AttributeType::SUPPORTED_PRIMITIVES;
	std::vector<PrimitiveType> values;

	template <class... A>
	explicit Attribute_SUPPORTED_PRIMITIVES(A&&... a)
		: Attribute(s_type)
	{
		push_back_into(values, std::forward<A>(a)...);
	}
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_USER_DISPLAY_NAME: public Attribute
{
public:
	static const AttributeType s_type = AttributeType::USER_DISPLAY_NAME;
	std::string text;

	Attribute_USER_DISPLAY_NAME() : Attribute(s_type) {}
	explicit Attribute_USER_DISPLAY_NAME(std::string&& text_) : Attribute(s_type), text(std::move(text_)) {}
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_USER_URI: public Attribute
{
public:
	static const AttributeType s_type = AttributeType::USER_URI;
	std::string text;

	Attribute_USER_URI() : Attribute(s_type) {}
	explicit Attribute_USER_URI(std::string&& text_) : Attribute(s_type), text(std::move(text_)) {}
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_BENEFICIARY_INFORMATION : public AttributeGrouped
{
public:
	static const AttributeType s_type = AttributeType::BENEFICIARY_INFORMATION;
	UserID beneficiary_id = 0;
	Attribute_USER_DISPLAY_NAME* attr_USER_DISPLAY_NAME = nullptr;
	Attribute_USER_URI* attr_USER_URI = nullptr;

	Attribute_BENEFICIARY_INFORMATION() : AttributeGrouped(s_type) {}
	template <class... A>
	Attribute_BENEFICIARY_INFORMATION(UserID beneficiary_id_, A&&... a)
		: AttributeGrouped(s_type, std::forward<A>(a)...)
		, beneficiary_id(beneficiary_id_)
	{}
	void UpdateView() override;
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_FLOOR_REQUEST_INFORMATION : public AttributeGrouped
{
public:
	static const AttributeType s_type = AttributeType::FLOOR_REQUEST_INFORMATION;
	FloorRequestID floor_request_id = 0;
	Attribute_OVERALL_REQUEST_STATUS* attr_OVERALL_REQUEST_STATUS = nullptr;
	std::vector<Attribute_FLOOR_REQUEST_STATUS*> attrs_FLOOR_REQUEST_STATUS;
	Attribute_BENEFICIARY_INFORMATION* attr_BENEFICIARY_INFORMATION = nullptr;
	Attribute_REQUESTED_BY_INFORMATION* attr_REQUESTED_BY_INFORMATION = nullptr;
	Attribute_PRIORITY* attr_PRIORITY = nullptr;
	Attribute_PARTICIPANT_PROVIDED_INFO* attr_PARTICIPANT_PROVIDED_INFO = nullptr;

	Attribute_FLOOR_REQUEST_INFORMATION() : AttributeGrouped(s_type) {}
	template <class... A>
	Attribute_FLOOR_REQUEST_INFORMATION(FloorRequestID floor_request_id_, A&&... a)
		: AttributeGrouped(s_type, std::forward<A>(a)...)
		, floor_request_id(floor_request_id_)
	{}
	void UpdateView() override;
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_REQUESTED_BY_INFORMATION : public AttributeGrouped
{
public:
	static const AttributeType s_type = AttributeType::REQUESTED_BY_INFORMATION;
	UserID requested_by_id = 0;
	Attribute_USER_DISPLAY_NAME* attr_USER_DISPLAY_NAME = nullptr;
	Attribute_USER_URI* attr_USER_URI = nullptr;

	Attribute_REQUESTED_BY_INFORMATION() : AttributeGrouped(s_type) {}
	template <class... A>
	Attribute_REQUESTED_BY_INFORMATION(UserID requested_by_id_, A&&... a)
		: AttributeGrouped(s_type, std::forward<A>(a)...)
		, requested_by_id(requested_by_id_)
	{}
	void UpdateView() override;
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_FLOOR_REQUEST_STATUS : public AttributeGrouped
{
public:
	static const AttributeType s_type = AttributeType::FLOOR_REQUEST_STATUS;
	FloorID floor_id = 0;
	Attribute_REQUEST_STATUS* attr_REQUEST_STATUS = nullptr;
	Attribute_STATUS_INFO* attr_STATUS_INFO = nullptr;

	Attribute_FLOOR_REQUEST_STATUS() : AttributeGrouped(s_type) {}
	template <class... A>
	Attribute_FLOOR_REQUEST_STATUS(FloorID floor_id_, A&&... a)
		: AttributeGrouped(s_type, std::forward<A>(a)...)
		, floor_id(floor_id_)
	{}
	void UpdateView() override;
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

class Attribute_OVERALL_REQUEST_STATUS : public AttributeGrouped
{
public:
	static const AttributeType s_type = AttributeType::OVERALL_REQUEST_STATUS;
	FloorRequestID floor_request_id = 0;
	Attribute_REQUEST_STATUS* attr_REQUEST_STATUS = nullptr;
	Attribute_STATUS_INFO* attr_STATUS_INFO = nullptr;
	Attribute_FLOOR_REQUEST_STATUS* attr_FLOOR_REQUEST_STATUS = nullptr; // Workaround for Polycom

	Attribute_OVERALL_REQUEST_STATUS() : AttributeGrouped(s_type) {}
	template <class... A>
	Attribute_OVERALL_REQUEST_STATUS(FloorRequestID floor_request_id_, A&&... a)
		: AttributeGrouped(s_type, std::forward<A>(a)...)
		, floor_request_id(floor_request_id_)
	{}
	void UpdateView() override;
	void Decode(const void* buffer, size_t& size, DecodeStatus& status) override;
	bool Encode(void* buffer, size_t& size) const override;
};

}
