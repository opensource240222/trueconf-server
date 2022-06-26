#pragma once

#include "Connection.h"
#include "SecureProvider.h"
#include "../Error.h"
#include "../../transport/Message.h"
#include "SecureLib/VS_SymmetricCrypt.h"
#include "../../std/cpplib/iostream_utils.h"
#include "net/QoSSettings.h"
#include "../../std/debuglog/VS_Debug.h"

#include "std-generic/compat/memory.h"
#include "std-generic/cpplib/ignore.h"
#include <cassert>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

#define TRANSPORT_VERBOSE_LOGS 0

namespace transport {

static size_t GetEncryptedMessageHeaderSize(const VS_SymmetricCrypt* crypt)
{
	static MessageFixedPart test_message;
	uint32_t size = 0;
	// We don't care what data will be encrypted, we want only the size of the result for data of a given size.
	crypt->Encrypt(reinterpret_cast<const unsigned char*>(&test_message), sizeof(test_message), nullptr, &size);
	if (size != sizeof(MessageFixedPart)) // cipher in block mode
		--size;
	return size;
}

template <class Socket>
struct Connection<Socket>::LogPrefix
{
	explicit LogPrefix(const Connection* obj_) : obj(obj_) {}
	const Connection* obj;

	template <class CharT, class Traits>
	friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& s, LogPrefix x)
	{
		s << "transport::Connection(";
		const auto id = x.GetLogID();
		if (id.empty())
			s << pointer_value(x.obj);
		else
			s << id;
		s << "): ";
		return s;
	}
private:
	string_view GetLogID() const { return obj->LogID(); }
};

template <class Socket>
Connection<Socket>::Connection(boost::asio::io_service& ios)
	: base_t(ios)
	, m_sec(nullptr)
	, m_handshake_completed(false)
	, m_minimal_read_size(0)
	, m_encrypted_header_size(0)
{
}

template <class Socket>
Connection<Socket>::~Connection() = default;

template <class Socket>
void Connection<Socket>::Start(socket_type&& socket, const net::HandshakeHeader* handshake)
{
	assert(handshake);
	assert(this->GetState() == net::BufferedConnectionState::empty);

	// Reset state
	m_handshake_completed = false;
	m_sec = nullptr; // Derived class is required to set m_sec in OnHandshakeReply callback.
	m_minimal_read_size = 0;
	m_encrypted_header_size = 0;

	this->SetSocket(std::move(socket), net::QoSSettings::GetInstance().GetTCTransportQoSFlow(socket.local_endpoint(vs::ignore<boost::system::error_code>()).address().is_v6()));
	base_t::Send(handshake, sizeof(net::HandshakeHeader) + handshake->body_length + 1);
}

template <class Socket>
void Connection<Socket>::Accept(socket_type&& socket, const net::HandshakeHeader* handshake_reply)
{
	assert(handshake_reply);
	assert(this->GetState() == net::BufferedConnectionState::empty);

	// Reset state
	m_handshake_completed = true;
	// m_sec can be already set be the caller.
	m_minimal_read_size = 0;
	m_encrypted_header_size = 0;

	this->SetSocket(std::move(socket), net::QoSSettings::GetInstance().GetTCTransportQoSFlow(socket.remote_endpoint(vs::ignore<boost::system::error_code>()).address().is_v6()));
	base_t::Send(handshake_reply, sizeof(net::HandshakeHeader) + handshake_reply->body_length + 1);
	if (m_sec)
		DoSecureHandshakeSends();
}

template <class Socket>
bool Connection<Socket>::HandshakeCompleted() const
{
	return m_handshake_completed;
}

template <class Socket>
bool Connection<Socket>::SecureHandshakeCompleted() const
{
	return !m_sec || m_sec->GetState() == SecureProvider::State::handshake_completed;
}

template <class Socket>
bool Connection<Socket>::Send(const Message& message)
{
	if (!HandshakeCompleted() || !SecureHandshakeCompleted())
	{
		this->m_strand.dispatch([this, self = this->shared_from_this(), message]() mutable {		// todo(kt): save weak_ptr
			if (!HandshakeCompleted() || !SecureHandshakeCompleted())
				m_pending_msg_queue.emplace_back(std::move(message));
			else
				this->Send(message);
		});
		return false;
	}

	if (m_sec)
	{
		assert(m_sec->WriteCrypt());
		uint32_t encrypted_size = 0;
		m_sec->WriteCrypt()->Encrypt(message.Data(), message.Size(), nullptr, &encrypted_size);
		auto encrypted_data = vs::make_unique<unsigned char[]>(encrypted_size);
		if (!m_sec->WriteCrypt()->Encrypt(message.Data(), message.Size(), encrypted_data.get(), &encrypted_size))
		{
			dstream1 << LogPrefix(this) << "message encryption failed: " << message.Size() << " -> " << encrypted_size;
			this->OnError(errc::message_encryption_error);
			return true;
		}
#if TRANSPORT_VERBOSE_LOGS
		dstream4 << LogPrefix(this) << "message encrypted: " << message.Size() << " -> " << encrypted_size;
#endif
		base_t::Send(encrypted_data.get(), encrypted_size);
	}
	else
		base_t::Send(message.Data(), message.Size());
	return true;
}

template <class Socket>
size_t Connection<Socket>::OnReceive(const void* data, size_t size)
{
	if (!m_handshake_completed)
		return ReadHandshakeReply(data, size);
	if (m_sec)
	{
		switch (m_sec->GetState())
		{
		case SecureProvider::State::handshake_completed:
			return ReadEncryptedMessage(data, size);
		case SecureProvider::State::handshake_in_progress:
		{
			const auto consumed = m_sec->HandlePacket(data, size);
			DoSecureHandshakeSends();
			if (m_sec->GetState() == SecureProvider::State::error)
				this->OnError(errc::secure_handshake_error);
			else if (m_sec->GetState() == SecureProvider::State::handshake_completed)
				SendPendingMessages();
			return consumed;
		}
		case SecureProvider::State::error:
		default:
			return 0;
		}
	}
	else
		return ReadMessage(data, size);
}

template <class Socket>
void Connection<Socket>::OnClose()
{
	m_handshake_completed = false;
	m_sec = nullptr;
}

template <class Socket>
void Connection<Socket>::DoSecureHandshakeSends()
{
	assert(m_sec);
	// m_sec could be zeroed in OnClose during the unsuccessful Send(...)
	while (m_sec && m_sec->GetState() == SecureProvider::State::handshake_in_progress)
	{
		auto packet = m_sec->GetPacket();
		if (packet.empty())
			break;
		auto buffer = std::make_shared<std::vector<uint8_t>>(std::move(packet));
		base_t::Send({buffer->data(), buffer->size(), buffer});
	}
}

template <class Socket>
size_t Connection<Socket>::ReadHandshakeReply(const void* data, size_t size)
{
	if (size < sizeof(net::HandshakeHeader))
		return 0;

	auto hs = static_cast<const net::HandshakeHeader*>(data);
	if (hs->head_cksum != net::GetHandshakeHeaderChecksum(*hs)
		|| hs->version < 1
		|| strncmp(hs->primary_field, PrimaryField, sizeof(hs->primary_field)) != 0)
	{
		OnHandshakeReply(HandshakeResult::verification_failed, 0, 0, 0, nullptr, nullptr, false);
		return 0;
	}

	const size_t hs_size = sizeof(net::HandshakeHeader) + hs->body_length + 1;
	if (size < hs_size)
		return 0;
	if (hs->body_cksum != net::GetHandshakeBodyChecksum(*hs))
	{
		OnHandshakeReply(HandshakeResult::verification_failed, 0, 0, 0, nullptr, nullptr, false);
		return 0;
	}

	HandshakeResult result;
	uint16_t max_conn_silence_ms;
	uint8_t fatal_silence_coef;
	uint8_t hops;
	const char* server_id;
	const char* client_id;
	bool tcp_keep_alive_support;
	if (!ParseHandshakeReply(static_cast<const net::HandshakeHeader*>(data), server_id, client_id, result, max_conn_silence_ms, fatal_silence_coef, hops, tcp_keep_alive_support))
	{
		OnHandshakeReply(HandshakeResult::verification_failed, 0, 0, 0, nullptr, nullptr, false);
		return 0;
	}

#if TRANSPORT_VERBOSE_LOGS
		dstream4 << LogPrefix(this) << "handshake reply received: result=" << static_cast<unsigned>(result) << ", hops=" << static_cast<unsigned>(hops) << ", sid=\"" << server_id << "\", cid=\"" << client_id << "\", keep_alive=" << std::boolalpha << tcp_keep_alive_support;
#endif
	m_handshake_completed = true;
	OnHandshakeReply(result, max_conn_silence_ms, fatal_silence_coef, hops, server_id, client_id, tcp_keep_alive_support);
	if (m_sec)
		DoSecureHandshakeSends();
	return hs_size;
}

template <class Socket>
size_t Connection<Socket>::ReadMessage(const void* data, size_t size)
{
	if (m_minimal_read_size != 0 && size < m_minimal_read_size)
		return 0;

	if (size < sizeof(MessageFixedPart))
	{
		m_minimal_read_size = sizeof(MessageFixedPart);
		return 0;
	}
	auto head = reinterpret_cast<const MessageFixedPart*>(data);
	if (!CheckMessageFixedHeader(head))
		return 0;
	if (size >= head->head_length && !CheckMessageHeader(head))
		return 0;

	const size_t message_size = head->head_length + head->body_length + 1;
	if (size < message_size)
	{
		m_minimal_read_size = message_size;
		return 0;
	}

	if (!CheckMessageBody(head))
		return 0;
#if TRANSPORT_VERBOSE_LOGS
		dstream4 << LogPrefix(this) << "message received: size=" << message_size;
#endif

	OnMessage(Message(data, message_size));

	m_minimal_read_size = sizeof(MessageFixedPart);
	return message_size;
}

template <class Socket>
size_t Connection<Socket>::ReadEncryptedMessage(const void* data, size_t size)
{
	assert(m_sec);
	assert(m_sec->ReadCrypt());

	if (m_minimal_read_size != 0 && size < m_minimal_read_size)
		return 0;

	if (m_encrypted_header_size == 0)
		m_encrypted_header_size = GetEncryptedMessageHeaderSize(m_sec->ReadCrypt());

	if (m_decrypted_message.size() < sizeof(MessageFixedPart))
	{
		// Try to decrypt message header, we need at least m_encrypted_header_size bytes to do that.
		if (size < m_encrypted_header_size)
		{
			m_minimal_read_size = m_encrypted_header_size;
			return 0;
		}
		uint32_t decrypted_size = m_encrypted_header_size + m_sec->ReadCrypt()->GetBlockSize();
		m_decrypted_message.resize(decrypted_size);
		if (!m_sec->ReadCrypt()->Decrypt(static_cast<const unsigned char*>(data), m_encrypted_header_size, m_decrypted_message.data(), &decrypted_size) || decrypted_size < sizeof(MessageFixedPart))
		{
			dstream1 << LogPrefix(this) << "message decryption failed: decrypted " << decrypted_size << " bytes from " << m_encrypted_header_size << " bytes";
			m_decrypted_message.clear();
			this->OnError(errc::message_encryption_error);
			return 0;
		}
		m_decrypted_message.resize(decrypted_size);
	}

	auto head = reinterpret_cast<const MessageFixedPart*>(m_decrypted_message.data());
	if (!CheckMessageFixedHeader(head))
	{
		m_decrypted_message.clear();
		return 0;
	}

	const size_t message_size = head->head_length + head->body_length + 1;

	// We need to determine total size of the encrypted message.
	// To do that we encrypt (arbitrary) data of size equal to the size of the
	// decrypted message.
	if (size < message_size)
	{
		// We don't have enough data (encrypted body is larger that unencrypted).
		m_minimal_read_size = message_size;
		return 0;
	}
	uint32_t encrypted_message_size = 0;
	m_sec->ReadCrypt()->Encrypt(static_cast<const unsigned char*>(data), message_size, nullptr, &encrypted_message_size);

	// Try to decrypt remaining part of message, we need at least encrypted_message_size bytes to do that.
	if (size < encrypted_message_size)
	{
		m_minimal_read_size = encrypted_message_size;
		return 0;
	}
	const size_t prev_decrypted_size = m_decrypted_message.size();
	const uint32_t encrypted_size = encrypted_message_size - m_encrypted_header_size;
	uint32_t decrypted_size = encrypted_size + m_sec->ReadCrypt()->GetBlockSize();
	m_decrypted_message.resize(prev_decrypted_size + decrypted_size);
	if (!m_sec->ReadCrypt()->Decrypt(static_cast<const unsigned char*>(data) + m_encrypted_header_size, encrypted_size, m_decrypted_message.data() + prev_decrypted_size, &decrypted_size) || decrypted_size < message_size - prev_decrypted_size)
	{
		dstream1 << LogPrefix(this) << "message decryption failed: decrypted " << decrypted_size << " bytes from " << encrypted_size << " bytes";
		m_decrypted_message.clear();
		this->OnError(errc::message_encryption_error);
		return 0;
	}
	// Update header pointer after reallocation.
	head = reinterpret_cast<const MessageFixedPart*>(m_decrypted_message.data());
	assert(static_cast<size_t>(head->head_length + head->body_length + 1) == message_size);
	assert(prev_decrypted_size + decrypted_size >= message_size);
	m_decrypted_message.resize(message_size);

	if (!CheckMessageHeader(head))
	{
		m_decrypted_message.clear();
		return 0;
	}
	if (!CheckMessageBody(head))
	{
		m_decrypted_message.clear();
		return 0;
	}

#if TRANSPORT_VERBOSE_LOGS
		dstream4 << LogPrefix(this) << "encrypted message received: size=" << m_decrypted_message.size();
#endif
	OnMessage(Message(std::move(m_decrypted_message)));

	m_minimal_read_size = m_encrypted_header_size;
	return encrypted_message_size;
}

template <class Socket>
bool Connection<Socket>::CheckMessageFixedHeader(const MessageFixedPart* message)
{
	if (message->version < 1 || message->mark1 != 1 || message->head_length < sizeof(MessageFixedPart))
	{
		dstream1 << LogPrefix(this) << "invalid message: version=" << message->version << ", mark1=" << message->mark1 << ", head_length=" << message->head_length;
		this->OnError(errc::invalid_message_header);
		return false;
	}
	return true;
}

template <class Socket>
bool Connection<Socket>::CheckMessageHeader(const MessageFixedPart* message)
{
	const unsigned head_cksum = GetMessageHeaderChecksum(*message);
	if (message->head_cksum != head_cksum)
	{
		dstream1 << LogPrefix(this) << "invalid message: head_cksum=" << head_cksum << ", stored head_cksum=" << static_cast<unsigned>(message->head_cksum);
		this->OnError(errc::invalid_message_head_cksum);
		return false;
	}
	return true;
}

template <class Socket>
bool Connection<Socket>::CheckMessageBody(const MessageFixedPart* message)
{
	const unsigned body_cksum = GetMessageBodyChecksum(*message);
	if (message->body_cksum != body_cksum)
	{
		dstream1 << LogPrefix(this) << "invalid message: body_cksum=" << body_cksum << ", stored body_cksum=" << static_cast<unsigned>(message->body_cksum);
		this->OnError(errc::invalid_message_body_cksum);
		return false;
	}
	return true;
}

template <class Socket>
void Connection<Socket>::SendPendingMessages()
{
	for (auto const& m : m_pending_msg_queue)
		this->Send(m);
	m_pending_msg_queue.clear();
}

}

#undef DEBUG_CURRENT_MODULE
