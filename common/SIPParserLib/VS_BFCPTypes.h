#pragma once

#include <cstdint>

namespace bfcp {

enum class PrimitiveType
{
	invalid = 0,
	FloorRequest = 1,
	FloorRelease = 2,
	FloorRequestQuery = 3,
	FloorRequestStatus = 4,
	UserQuery = 5,
	UserStatus = 6,
	FloorQuery = 7,
	FloorStatus = 8,
	ChairAction = 9,
	ChairActionAck = 10,
	Hello = 11,
	HelloAck = 12,
	Error = 13,
	FloorRequestStatusAck = 14,
	FloorStatusAck = 15,
	Goodbye = 16,
	GoodbyeAck = 17,

	// used internally
	FloorStatusAck_v1 = 16,
	Goodbye_v1 = 17,
	GoodbyeAck_v1 = 18,
};

enum class AttributeType {
	invalid = 0,
	BENEFICIARY_ID = 1,
	FLOOR_ID = 2,
	FLOOR_REQUEST_ID = 3,
	PRIORITY = 4,
	REQUEST_STATUS = 5,
	ERROR_CODE = 6,
	ERROR_INFO = 7,
	PARTICIPANT_PROVIDED_INFO = 8,
	STATUS_INFO = 9,
	SUPPORTED_ATTRIBUTES = 10,
	SUPPORTED_PRIMITIVES = 11,
	USER_DISPLAY_NAME = 12,
	USER_URI = 13,
	BENEFICIARY_INFORMATION = 14,
	FLOOR_REQUEST_INFORMATION = 15,
	REQUESTED_BY_INFORMATION = 16,
	FLOOR_REQUEST_STATUS = 17,
	OVERALL_REQUEST_STATUS = 18,
};

typedef uint32_t ConferenceID;
typedef uint16_t TransactionID;
typedef uint16_t UserID;
typedef uint16_t FloorID;
typedef uint16_t FloorRequestID;

enum class Priority : uint8_t
{
	invalid = 8,
	Lowest = 0,
	Low = 1,
	Normal = 2,
	High = 3,
	Highest = 4,
};

enum class RequestStatus : uint8_t
{
	invalid = 0,
	Pending = 1,
	Accepted = 2,
	Granted = 3,
	Denied = 4,
	Cancelled = 5,
	Released = 6,
	Revoked = 7,
};

struct DecodeStatus
{
	enum Type
	{
		Success = 0,
		PartialData,
		MalformedData,
		TypeMismatch,
		UnknownVersion,
		UnknownMandatoryAttribute,
		UnknownPrimitive,
	};

	Type type;
	uint8_t attribute_type;
	uint8_t primitive_type;

	DecodeStatus()
		: type(Success)
		, attribute_type(0)
		, primitive_type(0)
	{
	}
};

}
