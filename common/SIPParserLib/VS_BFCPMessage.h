#pragma once

#include "VS_BFCPTypes.h"
#include "VS_BFCPAttribute_fwd.h"
#include "std/cpplib/container_helpers.h"

#include <cassert>
#include <memory>
#include <vector>

namespace bfcp {

class Message
{
public:
	uint8_t version = 0;
	const PrimitiveType type;
	ConferenceID conference_id = 0;
	TransactionID transaction_id = 0;
	UserID user_id = 0;

	std::vector<std::unique_ptr<Attribute>> attrs;

	virtual ~Message();
	virtual void UpdateView() = 0;

	// Tries to decode message from byte stream.
	// On success sets status to Success and size to amount of consumed bytes.
	// If buffer doesn't contain enough data to decode a message sets status to PartialData
	// and size to minimal amount of bytes required for next attempt.
	// If error occurs during decoding sets appropriate status and size to
	// amount of byte that might be skipped to recover from error.
	virtual void Decode(const void* buffer, size_t& size, DecodeStatus& status);

	// Encodes message to byte stream.
	// On success returns true and sets size to amount of produced bytes.
	// On failure returns false and sets size to minimal buffer size required to successful
	// encoding.
	virtual bool Encode(void* buffer, size_t& size) const;

	// Casts this object to a concrete message class, checking the 'type' field.
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

	static std::unique_ptr<Message> DecodeAny(const void* buffer, size_t& size, DecodeStatus& status);

protected:
	explicit Message(PrimitiveType type_)
		: type(type_)
	{
	}

	template <class... A>
	Message(uint8_t version_, PrimitiveType type_, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: version(version_)
		, type(type_)
		, conference_id(conference_id_)
		, transaction_id(transaction_id_)
		, user_id(user_id_)
	{
		push_back_into(attrs, std::forward<A>(a)...);
	}
};

class Message_FloorRequest;
class Message_FloorRelease;
class Message_FloorRequestQuery;
class Message_FloorRequestStatus;
class Message_UserQuery;
class Message_UserStatus;
class Message_FloorQuery;
class Message_FloorStatus;
class Message_ChairAction;
class Message_ChairActionAck;
class Message_Hello;
class Message_HelloAck;
class Message_Error;

class Message_FloorRequest : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::FloorRequest;
	std::vector<Attribute_FLOOR_ID*> attrs_FLOOR_ID;
	Attribute_BENEFICIARY_ID* attr_BENEFICIARY_ID = nullptr;
	Attribute_PARTICIPANT_PROVIDED_INFO* attr_PARTICIPANT_PROVIDED_INFO = nullptr;
	Attribute_PRIORITY* attr_PRIORITY = nullptr;

	Message_FloorRequest() : Message(s_type) {}
	template <class... A>
	Message_FloorRequest(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_FloorRelease : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::FloorRelease;
	Attribute_FLOOR_REQUEST_ID* attr_FLOOR_REQUEST_ID = nullptr;

	Message_FloorRelease() : Message(s_type) {}
	template <class... A>
	Message_FloorRelease(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_FloorRequestQuery : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::FloorRequestQuery;
	Attribute_FLOOR_REQUEST_ID* attr_FLOOR_REQUEST_ID = nullptr;

	Message_FloorRequestQuery() : Message(s_type) {}
	template <class... A>
	Message_FloorRequestQuery(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_FloorRequestStatus : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::FloorRequestStatus;
	Attribute_FLOOR_REQUEST_INFORMATION* attr_FLOOR_REQUEST_INFORMATION = nullptr;

	Message_FloorRequestStatus() : Message(s_type) {}
	template <class... A>
	Message_FloorRequestStatus(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_UserQuery : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::UserQuery;
	Attribute_BENEFICIARY_ID* attr_BENEFICIARY_ID = nullptr;

	Message_UserQuery() : Message(s_type) {}
	template <class... A>
	Message_UserQuery(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_UserStatus : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::UserStatus;
	Attribute_BENEFICIARY_INFORMATION* attr_BENEFICIARY_INFORMATION = nullptr;
	std::vector<Attribute_FLOOR_REQUEST_INFORMATION*> attrs_FLOOR_REQUEST_INFORMATION;

	Message_UserStatus() : Message(s_type) {}
	template <class... A>
	Message_UserStatus(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_FloorQuery : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::FloorQuery;
	std::vector<Attribute_FLOOR_ID*> attrs_FLOOR_ID;

	Message_FloorQuery() : Message(s_type) {}
	template <class... A>
	Message_FloorQuery(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_FloorStatus : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::FloorStatus;
	Attribute_FLOOR_ID* attr_FLOOR_ID = nullptr;
	std::vector<Attribute_FLOOR_REQUEST_INFORMATION*> attrs_FLOOR_REQUEST_INFORMATION;

	Message_FloorStatus() : Message(s_type) {}
	template <class... A>
	Message_FloorStatus(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_ChairAction : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::ChairAction;
	Attribute_FLOOR_REQUEST_INFORMATION* attr_FLOOR_REQUEST_INFORMATION = nullptr;

	Message_ChairAction() : Message(s_type) {}
	template <class... A>
	Message_ChairAction(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_ChairActionAck : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::ChairActionAck;

	Message_ChairActionAck() : Message(s_type) {}
	template <class... A>
	Message_ChairActionAck(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_Hello : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::Hello;

	Message_Hello() : Message(s_type) {}
	template <class... A>
	Message_Hello(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_HelloAck : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::HelloAck;
	Attribute_SUPPORTED_PRIMITIVES* attr_SUPPORTED_PRIMITIVES = nullptr;
	Attribute_SUPPORTED_ATTRIBUTES* attr_SUPPORTED_ATTRIBUTES = nullptr;

	Message_HelloAck() : Message(s_type) {}
	template <class... A>
	Message_HelloAck(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_Error : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::Error;
	Attribute_ERROR_CODE* attr_ERROR_CODE = nullptr;
	Attribute_ERROR_INFO* attr_ERROR_INFO = nullptr;

	Message_Error() : Message(s_type) {}
	template <class... A>
	Message_Error(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_FloorRequestStatusAck : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::FloorRequestStatusAck;

	Message_FloorRequestStatusAck() : Message(s_type) {}
	template <class... A>
	Message_FloorRequestStatusAck(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_FloorStatusAck : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::FloorStatusAck;

	Message_FloorStatusAck() : Message(s_type) {}
	template <class... A>
	Message_FloorStatusAck(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_Goodbye : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::Goodbye;

	Message_Goodbye() : Message(s_type) {}
	template <class... A>
	Message_Goodbye(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

class Message_GoodbyeAck : public Message
{
public:
	static const PrimitiveType s_type = PrimitiveType::GoodbyeAck;

	Message_GoodbyeAck() : Message(s_type) {}
	template <class... A>
	Message_GoodbyeAck(uint8_t version, ConferenceID conference_id_, TransactionID transaction_id_, UserID user_id_, A&&... a)
		: Message(version, s_type, conference_id_, transaction_id_, user_id_, std::forward<A>(a)...) {}
	void UpdateView() override;
};

}
