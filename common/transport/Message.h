#pragma once

#include "std-generic/cpplib/string_view.h"
#include "std-generic/attributes.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

class VS_Container;

namespace transport {

enum class MessageType
{
	Invalid = 0,
	Request,
	Reply,
	Notify,
};

#pragma pack(push, 1)
struct MessageFixedPart // Version 1
{
	uint32_t version:5;      // Version of header and (may be) the message
	uint32_t notify:1;       // Should be set for notify messages
	uint32_t request:1;      // Should be set for request message and unset for reply message
	uint32_t mark1:1;        // Should be always set
	uint32_t head_cksum:8;   // Checksum of the header
	uint32_t head_length:16; // Length of the header
	uint32_t body_cksum:8;   // Checksum of the body
	uint32_t body_length:24; // Length of the body minus 1
	uint32_t seq_number;     // Sequence number of the message
	uint32_t ms_life_count;  // Message file time in milliseconds
};
static_assert(sizeof(MessageFixedPart) == 16, "!");
#pragma pack(pop)

// Calculate checksum of the header
VS_NODISCARD inline uint8_t GetMessageHeaderChecksum(const MessageFixedPart& msg)
{
	uint_fast8_t cksum = 0xac;
	const uint8_t* p = reinterpret_cast<const uint8_t*>(&msg);
	// Unrolled loop for MessageFixedPart
	cksum += p[0] & 0xdf; // notify is assumed to be 0 in checksum calculation
	// head_cksum is excluded from checksum calculation
	cksum += p[2] + 2;
	cksum += p[3] + 3;
	cksum += p[4] + 4;
	cksum += p[5] + 5;
	cksum += p[6] + 6;
	cksum += p[7] + 7;
	cksum += p[8] + 8;
	cksum += p[9] + 9;
	cksum += p[10] + 10;
	cksum += p[11] + 11;
	cksum += 12 + 13 + 14 + 15; // ms_life_count is assumed to be 0 in ckecksum calculation
	for (size_t i = sizeof(MessageFixedPart); i < msg.head_length; ++i)
		cksum += p[i] + static_cast<uint8_t>(i);
	return cksum & 0xff;
}

// Calculate checksum of the body, assume that body is located immediately after the header
VS_NODISCARD inline uint8_t GetMessageBodyChecksum(const MessageFixedPart& msg)
{
	uint_fast8_t cksum = 0xee;
	const uint8_t* p = reinterpret_cast<const uint8_t*>(&msg);
	// Note: Last msg.head_length bytes of body are ignored in checksum
	// calculation, this is due to mistake in the original implementation.
	for (size_t i = msg.head_length; i < static_cast<size_t>(msg.body_length + 1); i += 0x10)
		cksum += p[i] + static_cast<uint8_t>(i);
	return cksum & 0xff;
}

// Calculate both ckecksums and store them in the header
inline void UpdateMessageChecksums(MessageFixedPart& msg)
{
	msg.body_cksum = GetMessageBodyChecksum(msg);
	msg.head_cksum = GetMessageHeaderChecksum(msg);
}

// Dynamic part which contains null-terminated strings for:
//   1. source connection ID (CID)
//   2. source service name
//   3. extra info (add_string)
//   4. destination connection ID (CID)
//   5. destination service name
//   6. source user
//   7. destination user
//   8. source server
//   9. destination server

class Message
{
	class make_helper;

	static const unsigned n_fields = 9;

public:
	Message() {}

	// Create message by explicitly specifying all fields
	Message(
		bool is_request,
		uint32_t seq_number,
		uint32_t time_limit,
		string_view src_cid,
		string_view src_service,
		string_view add_string,
		string_view dst_cid,
		string_view dst_service,
		string_view src_user,
		string_view dst_user,
		string_view src_server,
		string_view dst_server,
		const void* body,
		size_t body_size
	);
	Message(const Message&) = default;
	Message(Message&& x)
		: m_msg(std::move(x.m_msg))
		, m_index(x.m_index)
	{
		x.m_index.fill(0);
	}
	Message& operator=(const Message&) = default;
	Message& operator=(Message&& x)
	{
		if (this == &x)
			return *this;
		m_msg = std::move(x.m_msg);
		m_index = x.m_index;
		x.m_index.fill(0);
		return *this;
	}

	// Create message by parsing network data
	Message(const void* data, size_t size);
	explicit Message(std::vector<uint8_t>&& msg);

	// Create message using helper interface
	static make_helper Make();

	bool IsValid() const
	{
		return !m_msg.empty();
	}

	uint8_t Version() const;
	bool IsRequest() const;
	bool IsReply() const;
	bool IsNotify() const;
	MessageType Type() const;
	uint32_t SeqNum() const;
	uint32_t TimeLimit() const;
	bool IsFromServer() const;

	bool SetNotity(bool value);
	bool SetTimeLimit(uint32_t value);

	const char* SrcCID() const     { return GetField(0); }
	const char* SrcService() const { return GetField(1); }
	const char* AddString() const  { return GetField(2); }
	const char* DstCID() const     { return GetField(3); }
	const char* DstService() const { return GetField(4); }
	const char* SrcUser() const    { return GetField(5); }
	const char* DstUser() const    { return GetField(6); }
	const char* SrcServer() const  { return GetField(7); }
	const char* DstServer() const  { return GetField(8); }

	cstring_view SrcCID_sv() const     { return GetField_sv(0); }
	cstring_view SrcService_sv() const { return GetField_sv(1); }
	cstring_view AddString_sv() const  { return GetField_sv(2); }
	cstring_view DstCID_sv() const     { return GetField_sv(3); }
	cstring_view DstService_sv() const { return GetField_sv(4); }
	cstring_view SrcUser_sv() const    { return GetField_sv(5); }
	cstring_view DstUser_sv() const    { return GetField_sv(6); }
	cstring_view SrcServer_sv() const  { return GetField_sv(7); }
	cstring_view DstServer_sv() const  { return GetField_sv(8); }

	// These functions recreate the whole message, it is better to use Make() to construct a new message.
	VS_DEPRECATED bool SetSrcCID(string_view value)     { return SetField(0, value); }
	VS_DEPRECATED bool SetSrcService(string_view value) { return SetField(1, value); }
	VS_DEPRECATED bool SetAddString(string_view value)  { return SetField(2, value); }
	VS_DEPRECATED bool SetDstCID(string_view value)     { return SetField(3, value); }
	VS_DEPRECATED bool SetDstService(string_view value) { return SetField(4, value); }
	VS_DEPRECATED bool SetSrcUser(string_view value)    { return SetField(5, value); }
	VS_DEPRECATED bool SetDstUser(string_view value)    { return SetField(6, value); }
	VS_DEPRECATED bool SetSrcServer(string_view value)  { return SetField(7, value); }
	VS_DEPRECATED bool SetDstServer(string_view value)  { return SetField(8, value); }

	const uint8_t* Body() const;
	size_t BodySize() const;
	bool SetBody(const void* data, size_t size);

	size_t Size() const { return m_msg.size(); }
	const uint8_t* Data() const { return m_msg.data(); }

	static uint32_t GetNewSeqNumber();

private:
	bool MakeIndex();
	const char* GetField(unsigned n) const
	{
		if (m_index[n] == 0)
			return nullptr;
		return reinterpret_cast<const char*>(m_msg.data() + m_index[n]);
	}
	cstring_view GetField_sv(unsigned n) const
	{
		if (m_index[n] == 0)
			return {};
		return { null_terminated, reinterpret_cast<const char*>(m_msg.data() + m_index[n]), *(m_msg.data() + m_index[n] - 1) };
	}
	bool SetField(unsigned n, string_view value);

	std::vector<uint8_t> m_msg;
	std::array<uint16_t, n_fields> m_index; // Offsets of fields in in m_msg
};
static_assert(std::is_copy_constructible<Message>::value, "!");
static_assert(std::is_copy_assignable<Message>::value, "!");
static_assert(std::is_move_constructible<Message>::value, "!");
static_assert(std::is_move_assignable<Message>::value, "!");

class Message::make_helper
{
public:
	make_helper& Copy(const Message& msg);
	make_helper& ReplyTo(const Message& msg);
	make_helper& Reply()                             { is_request = false;            return *this; }
	make_helper& Request()                           { is_request = true;             return *this; }
	make_helper& SeqNumber(uint32_t value)           { seq_number = value;            return *this; }
	make_helper& TimeLimit(uint32_t value)           { time_limit = value;            return *this; }
	make_helper& SrcCID(string_view value)           { src_cid = value;               return *this; }
	make_helper& SrcService(string_view value)       { src_service = value;           return *this; }
	make_helper& SrcServer(string_view value)        { src_server = value;            return *this; }
	make_helper& SrcUser(string_view value)          { src_user = value;              return *this; }
	make_helper& DstCID(string_view value)           { dst_cid = value;               return *this; }
	make_helper& DstService(string_view value)       { dst_service = value;           return *this; }
	make_helper& DstServer(string_view value)        { dst_server = value;            return *this; }
	make_helper& DstUser(string_view value)          { dst_user = value;              return *this; }
	make_helper& AddString(string_view value)        { add_string = value;            return *this; }
	make_helper& Body(const void* data, size_t size) { body = data; body_size = size; return *this; }
	make_helper& Body(const VS_Container& cnt);

	operator Message() const
	{
		return Message(is_request, seq_number ? seq_number : Message::GetNewSeqNumber(), time_limit, src_cid, src_service, add_string, dst_cid, dst_service, src_user, dst_user, src_server, dst_server, body, body_size);
	}

private:
	bool is_request = true;
	uint32_t seq_number = 0;
	uint32_t time_limit = 0xffffffff;
	string_view src_cid;
	string_view src_service;
	string_view add_string;
	string_view dst_cid;
	string_view dst_service;
	string_view src_user;
	string_view dst_user;
	string_view src_server;
	string_view dst_server;
	const void* body = nullptr;
	size_t body_size = 0;

	std::unique_ptr<char[]> body_holder;
};
inline Message::make_helper Message::Make() { return {}; }

}
