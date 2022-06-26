#pragma once

#include "newtransport/Handshake.h"
#include "net/BufferedConnection.h"

namespace transport {

struct MessageFixedPart;
class Message;
class SecureProvider;

template <class Socket = boost::asio::ip::tcp::socket>
class Connection : public net::BufferedConnection<Socket>
{
	using base_t = net::BufferedConnection<Socket>;
protected:
	using socket_type = typename base_t::socket_type;

	Connection(boost::asio::io_service& ios);
	~Connection();

	void Start(socket_type&& socket, const net::HandshakeHeader* handshake);
	void Accept(socket_type&& socket, const net::HandshakeHeader* handshake_reply);

	bool HandshakeCompleted() const;
	bool SecureHandshakeCompleted() const;

	// Sends a message to remote peer (encrypting it is SecureProvider is present).
	// Can send messages only if all handshakes are completed.
	// Return true is message was scheduled for send.
	bool Send(const Message& message);

	// Called when handshake reply is received.
	virtual void OnHandshakeReply(HandshakeResult result, uint16_t max_conn_silence_ms, uint8_t fatal_silence_coef, uint8_t hops, const char* server_id, const char* client_id, bool tcp_keep_alive_support) = 0;

	// Called when transport message is received.
	virtual void OnMessage(Message&& message) = 0;

	size_t OnReceive(const void* data, size_t size) override;
	void OnClose() override;

private:
	void DoSecureHandshakeSends();
	size_t ReadHandshakeReply(const void* data, size_t size);
	size_t ReadMessage(const void* data, size_t size);
	size_t ReadEncryptedMessage(const void* data, size_t size);
	bool CheckMessageFixedHeader(const MessageFixedPart* message);
	bool CheckMessageHeader(const MessageFixedPart* message);
	bool CheckMessageBody(const MessageFixedPart* message);
	void SendPendingMessages();

protected:
	SecureProvider* m_sec;
private:
	bool m_handshake_completed;
	size_t m_minimal_read_size;
	std::vector<uint8_t> m_decrypted_message;
	size_t m_encrypted_header_size;
	std::vector<Message> m_pending_msg_queue;

	struct LogPrefix;
};
extern template class Connection<>;

}
