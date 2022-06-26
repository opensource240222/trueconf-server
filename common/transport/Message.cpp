#include "Message.h"
#include "std-generic/cpplib/VS_Container.h"

#include "std-generic/compat/memory.h"
#include <atomic>
#include <cassert>
#include <cstring>

namespace transport {

static const size_t c_max_message_field_size = 255;
static const size_t c_max_message_body_size = 16777216;

Message::Message(
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
)
{
	m_index.fill(0);

	if (src_cid.size() > c_max_message_field_size)
		return;
	if (src_service.size() > c_max_message_field_size)
		return;
	if (add_string.size() > c_max_message_field_size)
		return;
	if (dst_cid.size() > c_max_message_field_size)
		return;
	if (dst_service.size() > c_max_message_field_size)
		return;
	if (src_user.size() > c_max_message_field_size)
		return;
	if (dst_user.size() > c_max_message_field_size)
		return;
	if (src_server.size() > c_max_message_field_size)
		return;
	if (dst_server.size() > c_max_message_field_size)
		return;
	if (!body || body_size == 0 || body_size > c_max_message_body_size)
		return;

	const size_t header_size = sizeof(MessageFixedPart)
		+ 1 + src_cid.size() + 1
		+ 1 + src_service.size() + 1
		+ 1 + add_string.size() + 1
		+ 1 + dst_cid.size() + 1
		+ 1 + dst_service.size() + 1
		+ 1 + src_user.size() + 1
		+ 1 + dst_user.size() + 1
		+ 1 + src_server.size() + 1
		+ 1 + dst_server.size() + 1
		;
	assert(header_size <= 0xffff);

	m_msg.resize(header_size + body_size);

	auto& fp = *reinterpret_cast<MessageFixedPart*>(m_msg.data());
	fp.version = 1;
	fp.notify = 0;
	fp.request = is_request ? 1 : 0;
	fp.mark1 = 1;
	fp.head_length = static_cast<uint16_t>(header_size);
	fp.body_length = static_cast<uint32_t>(body_size) - 1;
	fp.seq_number = seq_number;
	fp.ms_life_count = time_limit;

	auto p = m_msg.data() + sizeof(MessageFixedPart);
	const auto p_end = m_msg.data() + fp.head_length;

	auto put_field = [&](unsigned n, string_view value) {
		*p++ = static_cast<uint8_t>(value.size());
		m_index[n] = p - m_msg.data();
		std::memcpy(p, value.data(), value.size());
		p += value.size();
		*p++ = 0;
	};
	put_field(0, src_cid);
	put_field(1, src_service);
	put_field(2, add_string);
	put_field(3, dst_cid);
	put_field(4, dst_service);
	put_field(5, src_user);
	put_field(6, dst_user);
	put_field(7, src_server);
	put_field(8, dst_server);
	assert(p == p_end);

	std::memcpy(p, body, body_size);

	UpdateMessageChecksums(fp);
}

Message::Message(const void* data, size_t size)
{
	if (size < sizeof(MessageFixedPart))
		return;

	auto& fp = *reinterpret_cast<const MessageFixedPart*>(data);
	const size_t msg_size = fp.head_length + fp.body_length + 1;
	if (fp.version < 1
	 || fp.mark1 != 1
	 || fp.head_length < sizeof(MessageFixedPart)
	 || size < msg_size
	 || fp.head_cksum != GetMessageHeaderChecksum(fp)
	 || fp.body_cksum != GetMessageBodyChecksum(fp)
	)
		return;
	m_msg.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + msg_size);
	if (!MakeIndex())
	{
		m_msg.clear();
		m_index.fill(0);
	}
}

Message::Message(std::vector<uint8_t>&& msg)
{
	if (msg.size() < sizeof(MessageFixedPart))
		return;

	auto& fp = *reinterpret_cast<const MessageFixedPart*>(msg.data());
	const size_t msg_size = fp.head_length + fp.body_length + 1;
	if (fp.version < 1
	 || fp.mark1 != 1
	 || fp.head_length < sizeof(MessageFixedPart)
	 || msg.size() < msg_size
	 || fp.head_cksum != GetMessageHeaderChecksum(fp)
	 || fp.body_cksum != GetMessageBodyChecksum(fp)
	)
		return;
	m_msg = std::move(msg);
	if (!MakeIndex())
	{
		m_msg.clear();
		m_index.fill(0);
	}
}

uint8_t Message::Version() const
{
	return !m_msg.empty() ? reinterpret_cast<const MessageFixedPart*>(m_msg.data())->version : 0;
}

bool Message::IsRequest() const
{
	return !m_msg.empty() ? reinterpret_cast<const MessageFixedPart*>(m_msg.data())->request != 0 : false;
}

bool Message::IsReply() const
{
	return !m_msg.empty() ? reinterpret_cast<const MessageFixedPart*>(m_msg.data())->request == 0 : false;
}

bool Message::IsNotify() const
{
	return !m_msg.empty() ? reinterpret_cast<const MessageFixedPart*>(m_msg.data())->notify != 0 : false;
}

MessageType Message::Type() const
{
	if (m_msg.empty())
		return MessageType::Invalid;
	auto& fp = *reinterpret_cast<const MessageFixedPart*>(m_msg.data());
	if (fp.notify)
		return MessageType::Notify;
	else if (fp.request)
		return MessageType::Request;
	else
		return MessageType::Reply;
}

uint32_t Message::SeqNum() const
{
	return !m_msg.empty() ? reinterpret_cast<const MessageFixedPart*>(m_msg.data())->seq_number : 0;
}

uint32_t Message::TimeLimit() const
{
	return !m_msg.empty() ? reinterpret_cast<const MessageFixedPart*>(m_msg.data())->ms_life_count : 0;
}

bool Message::IsFromServer() const
{
	return SrcCID_sv().empty() && SrcUser_sv().empty();
}

bool Message::SetNotity(bool value)
{
	if (m_msg.empty())
		return false;
	reinterpret_cast<MessageFixedPart*>(m_msg.data())->notify = value ? 1 : 0;
	return true;
}

bool Message::SetTimeLimit(uint32_t value)
{
	if (m_msg.empty())
		return false;
	reinterpret_cast<MessageFixedPart*>(m_msg.data())->ms_life_count = value;
	return true;
}

const uint8_t* Message::Body() const
{
	if (m_msg.empty())
		return nullptr;
	auto& fp = *reinterpret_cast<const MessageFixedPart*>(m_msg.data());
	return m_msg.data() + fp.head_length;
}

size_t Message::BodySize() const
{
	if (m_msg.empty())
		return 0;
	auto& fp = *reinterpret_cast<const MessageFixedPart*>(m_msg.data());
	return fp.body_length + 1;
}

bool Message::SetBody(const void* data, size_t size)
{
	if (m_msg.empty())
		return false;
	if (!data || size == 0 || size > c_max_message_body_size)
		return false;

	const size_t header_size = reinterpret_cast<const MessageFixedPart*>(m_msg.data())->head_length;
	m_msg.resize(header_size + size);

	auto& fp = *reinterpret_cast<MessageFixedPart*>(m_msg.data());
	std::memcpy(m_msg.data() + header_size, data, size);
	fp.body_length = static_cast<uint32_t>(size) - 1;
	UpdateMessageChecksums(fp);
	return true;
}

bool Message::MakeIndex()
{
	if (m_msg.empty())
		return false;

	auto& fp = *reinterpret_cast<const MessageFixedPart*>(m_msg.data());
	auto p = m_msg.data() + sizeof(MessageFixedPart);
	const auto p_end = m_msg.data() + fp.head_length;

	for (unsigned i = 0; i < n_fields; ++i)
	{
		if (p >= p_end)
			return false;
		const auto field_size = *p++;
		m_index[i] = p - m_msg.data();
		p += field_size;
		if (p >= p_end)
			return false;
		if (*p++ != 0)
			return false;
	}
	return true;
}

bool Message::SetField(unsigned n, string_view value)
{
	if (m_msg.empty())
		return false;

	if (value.size() > c_max_message_field_size)
		return false;

	auto& fp = *reinterpret_cast<const MessageFixedPart*>(m_msg.data());

	string_view fields[n_fields];
	for (unsigned i = 0; i < n_fields; ++i)
	{
		assert(m_index[i] != 0);
		fields[i] = { reinterpret_cast<const char*>(m_msg.data() + m_index[i]), *(m_msg.data() + m_index[i] - 1) };
	}
	assert(n < n_fields);
	fields[n] = value;

	*this = Message(fp.request != 0, fp.seq_number, fp.ms_life_count,
		fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6], fields[7], fields[8],
		m_msg.data() + fp.head_length, fp.body_length + 1
	);
	return true;
}

Message::make_helper& Message::make_helper::Copy(const Message& msg)
{
	is_request = msg.IsRequest();
	seq_number = msg.SeqNum();
	time_limit = msg.TimeLimit();
	src_cid = msg.SrcCID_sv();
	src_service = msg.SrcService_sv();
	src_server = msg.SrcServer_sv();
	src_user = msg.SrcUser_sv();
	dst_cid = msg.DstCID_sv();
	dst_service = msg.DstService_sv();
	dst_server = msg.DstServer_sv();
	dst_user = msg.DstUser_sv();
	add_string = msg.AddString_sv();
	body = msg.Body();
	body_size = msg.BodySize();
	return *this;
}

Message::make_helper& Message::make_helper::ReplyTo(const Message& msg)
{
	is_request = false;
	seq_number = msg.SeqNum();
	src_cid = msg.DstCID_sv();
	src_service = msg.DstService_sv();
	src_server = {};
	src_user = msg.DstUser_sv();
	dst_cid = msg.SrcCID_sv();
	dst_service = msg.SrcService_sv();
	dst_server = msg.SrcServer_sv();
	dst_user = msg.SrcUser_sv();
	return *this;
}

Message::make_helper& Message::make_helper::Body(const VS_Container& cnt)
{
	size_t sz;
	if (!cnt.Serialize(nullptr, sz))
		return *this;
	auto buffer = vs::make_unique_default_init<char[]>(sz);
	if (!cnt.Serialize(buffer.get(), sz))
		return *this;
	body_holder = std::move(buffer);
	body = body_holder.get();
	body_size = sz;
	return *this;
}

uint32_t Message::GetNewSeqNumber()
{
	static std::atomic<unsigned> seq_num(0);
	uint32_t  new_seq_num = 0;
	while (!(new_seq_num = seq_num++))
		;
	return new_seq_num;
}

}
