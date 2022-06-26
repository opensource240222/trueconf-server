#pragma once

#include <vector>
#include <queue>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>

#include "SecureLib/OpenSSLCompat/tc_ssl.h"
#include "net/tls/TlsCtx.h"
#include "net/Connect.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/event.h"
#include "Utils.h"
#include "Handshaker.h"
#include "Encryptor.h"

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_OTHER

namespace detail { class SocketImpl; }
struct tls {
typedef boost::asio::ip::tcp::endpoint endpoint;

class socket {
	std::shared_ptr<detail::SocketImpl> m_impl;
public:
	typedef boost::asio::ip::tcp::socket::native_handle_type native_handle_type;

	explicit socket(boost::asio::io_service& service);
	socket(boost::asio::io_service& service, bool initSecData);
	socket(socket&& other) noexcept;
	socket(const socket& other) = delete;
	socket& operator=(const socket& other) = delete;
	socket& operator=(socket&& other) noexcept;

	boost::asio::io_service& get_io_service();
	boost::asio::ip::tcp::socket::native_handle_type native_handle();
	void close(boost::system::error_code& ec);
	bool is_open() const;
	tls::endpoint local_endpoint(boost::system::error_code& ec) const;
	tls::endpoint remote_endpoint(boost::system::error_code& ec) const;
	template<typename ConnectHandler>
	void async_connect(const tls::endpoint& endpoint, ConnectHandler&& handler);
	template<typename ConstBufferSequence, typename WriteHandler>
	void async_send(const ConstBufferSequence& buffers, WriteHandler&& handler);
	template<typename MutableBufferSequence, typename ReadHandler>
	void async_receive(const MutableBufferSequence& buffers, ReadHandler&& handler);

	bool reset_secure_data(const void *privKey, unsigned int privKeySize, const char *privKeyPass,
		const void *endCert, unsigned int endCertSize, const void *caCert, unsigned int caCertSize);
	template<typename Func>
	void reset_verify_cb(int flag, Func&& cb);
	bool derive_key(std::vector<uint8_t>&key, size_t wantedLen, const char *label, size_t label_len, const uint8_t *context, size_t context_len);
};
};

namespace detail {
	class SocketImpl : public vs::enable_shared_from_this<SocketImpl> {
		const static size_t c_read_size;

		boost::asio::io_service::strand m_strand;
		boost::asio::ip::tcp::socket	m_socket;
		std::unique_ptr<SSL_CTX, ssl::utils::SSL_CTX_deleter>	m_ctx;
		std::unique_ptr <SSL, ssl::utils::SSL_deleter>		m_ssl;
		BIO		*m_bio_in;
		BIO		*m_bio_out;
		bool	m_secureDataLoaded;
		VS_TlsContext	 m_tlsContext;
		ssl::HandshakeStatus	m_handshakeStatus;
		bool serversFinishedMsgReceived;
		std::unique_ptr<ssl::Handshaker> m_hshaker;
		std::function<void(boost::system::error_code ec)> m_onHandshakeDone;
		std::queue<acs::Handler::stream_buffer> m_rQueue;
		std::queue<acs::Handler::stream_buffer> m_wQueue;
		std::unique_ptr<ssl::Encryptor> m_encryptor;
		std::unique_ptr<ssl::Decryptor> m_decryptor;

		void Destroy();
		void MakeHandshake(const void* data, size_t size);
		void HandleHandshakeWrite(boost::system::error_code ec, size_t transfered);
		void StartHandshakeRead();
		void HandleHandshakeRead(acs::Handler::stream_buffer && buffer);
	protected:
		SocketImpl(boost::asio::io_service &ios, bool initSecData = true);
	public:
		~SocketImpl();
		bool IsValid() const;
		bool DeriveKey(std::vector<uint8_t>&key, size_t wantedLen, const char *label, size_t label_len, const uint8_t *context, size_t context_len);
		template<class Handler>
		void Connect(const tls::endpoint & dstEp, Handler&& onConnect);
		template<typename ConstBufferSequence, typename WriteHandler>
		void AsyncSend(const ConstBufferSequence& buffers, WriteHandler&& handler);
		template<typename MutableBufferSequence, typename ReadHandler>
		void AsyncReceive(const MutableBufferSequence& buffers, ReadHandler&& handler);

	private:
		bool ResetPrivateKey(const void *key, unsigned int size, const char *pass = "");
		bool ResetEndCert(const void *cert, unsigned int size);
		bool AddCert(const void *cert, unsigned int size);
	public:
		bool ResetSecureData(const void *privKey, unsigned int privKeySize, const char *privKeyPass,
			const void *endCert, unsigned int endCertSize, const void *caCert, unsigned int caCertSize);
		template<typename Func>
		void ResetVerifyCb(int flag, Func && cb);

		boost::asio::io_service& GetIOS() { return m_socket.get_io_service(); }
		tls::socket::native_handle_type NativeHandle() { return m_socket.native_handle(); }
		void CloseConnection(boost::system::error_code& ec) { m_socket.close(ec); }
		bool IsOpen() const { return m_socket.is_open() && m_handshakeStatus == ssl::HandshakeStatus::hs_success; }
		tls::endpoint LocalEp(boost::system::error_code& ec) const { return m_socket.local_endpoint(ec); }
		tls::endpoint RemoteEp(boost::system::error_code& ec) const { return m_socket.remote_endpoint(ec); }
	};

	template<class Handler>
	void detail::SocketImpl::Connect(const tls::endpoint & dstEp, Handler && onConnect)
	{
		assert(m_secureDataLoaded);

		using Protocol = boost::asio::ip::tcp;
		net::Connect<Protocol>(m_strand, dstEp,
			[this, w_this = this->weak_from_this(), onConnect = std::forward<Handler>(onConnect), dstEp]
		(const boost::system::error_code& ec, typename Protocol::socket&& socket) mutable
		{
			auto self = w_this.lock();
			if (!self)
				return;
			if (ec) {
				dstream2 << "tls::socket(" << dstEp << "): connect failed: " << ec.message();
				onConnect(ec);
				return;
			}
			m_socket = std::move(socket);
			m_tlsContext.connectionType = VS_TlsContext::ConnectionType::ct_am_client;
			m_hshaker = std::make_unique<ssl::Handshaker>(m_bio_in, m_bio_out, m_ssl.get(), false);
			m_onHandshakeDone = std::move(onConnect);
			MakeHandshake(nullptr, 0u);
			StartHandshakeRead();
		});
	}

	template<typename ConstBufferSequence, typename WriteHandler>
	void detail::SocketImpl::AsyncSend(const ConstBufferSequence & buffers, WriteHandler && handler)
	{
		vs::event done(true);
		m_strand.dispatch([&]() {
			VS_SCOPE_EXIT{ done.set(); };
			assert(m_secureDataLoaded);
			assert(m_handshakeStatus == ssl::HandshakeStatus::hs_success);
			assert(m_encryptor != nullptr);

			if (m_handshakeStatus != ssl::HandshakeStatus::hs_success) {
				boost::system::error_code err;
				dstream4 << "tls::socket " << m_socket.local_endpoint(err) << " <-> " << m_socket.remote_endpoint(err) << ": async_send failed: No handshake was done!";
				m_socket.get_io_service().post([handler = std::forward<WriteHandler>(handler)]() mutable{handler(boost::asio::error::operation_aborted, 0); });
				return;
			}

			std::vector<boost::asio::const_buffer> toSend;
			for (const auto& buff : buffers) {
				std::vector<uint8_t> encrypted;
				if (!m_encryptor->Encrypt(boost::asio::buffer_cast<const uint8_t*>(buff), boost::asio::buffer_size(buff), encrypted))
					continue;

				m_wQueue.push(std::move(encrypted));
				toSend.emplace_back(boost::asio::buffer(m_wQueue.back().data(), m_wQueue.back().size()));
			}

			if (boost::asio::buffer_size(toSend) == 0u) {
				boost::system::error_code err;
				dstream4 << "tls::socket " << m_socket.local_endpoint(err) << " <-> " << m_socket.remote_endpoint(err) << ": async_send failed: noting to send!";
				m_socket.get_io_service().post([handler = std::forward<WriteHandler>(handler)]() mutable{handler(boost::asio::error::operation_aborted, 0); });
				return;
			}

			size_t actualSize = boost::asio::buffer_size(buffers);
			auto self = this->shared_from_this();
			boost::asio::async_write(m_socket, toSend, m_strand.wrap(
				[this, self, handler = std::forward<WriteHandler>(handler), toPop = toSend.size(), actualSize](const boost::system::error_code& ec, size_t bytes_transferred) mutable{
				while (toPop--) {
					assert(!m_wQueue.empty());
					m_wQueue.pop();
				}

				if (ec) {
					boost::system::error_code err;
					dstream4 << "tls::socket " << m_socket.local_endpoint(err) << " <-> " << m_socket.remote_endpoint(err) << ": async_send failed: " << ec.message();
					m_socket.get_io_service().post([handler = std::forward<WriteHandler>(handler), ec, bytes_transferred]() mutable{handler(ec, bytes_transferred); });
					return;
				}

				m_socket.get_io_service().post([handler = std::forward<WriteHandler>(handler), bytes_transferred, actualSize]() mutable{handler(boost::system::error_code(), actualSize); });
			}));
		});
		done.wait();
	}

	template<typename MutableBufferSequence, typename ReadHandler>
	void detail::SocketImpl::AsyncReceive(const MutableBufferSequence & buffers, ReadHandler && handler)
	{
		m_strand.dispatch([this, w_this = weak_from_this(), handler = std::forward<ReadHandler>(handler), buffers]() mutable{
			auto self = w_this.lock();
			if (!self) {
				m_socket.get_io_service().post([handler = std::forward<ReadHandler>(handler)]() mutable{handler(boost::asio::error::operation_aborted, 0); });
				return;
			}

			assert(m_secureDataLoaded);
			assert(m_handshakeStatus == ssl::HandshakeStatus::hs_success);
			if (m_handshakeStatus != ssl::HandshakeStatus::hs_success) {
				boost::system::error_code err;
				dstream4 << "tls::socket " << m_socket.local_endpoint(err) << " <-> " << m_socket.remote_endpoint(err) << ": async_receive failed: No handshake was done!";
				m_socket.get_io_service().post([handler = std::forward<ReadHandler>(handler)]() mutable{handler(boost::asio::error::operation_aborted, 0); });
				return;
			}

			m_rQueue.emplace();
			auto &rBuff = m_rQueue.back();
			rBuff.resize(c_read_size);
			m_socket.async_receive(boost::asio::buffer(rBuff.data(), rBuff.size()), m_strand.wrap(
				[this, self, handler = std::forward<ReadHandler>(handler), buffers](const boost::system::error_code& ec, size_t bytes_transferred) mutable
			{
				acs::Handler::stream_buffer recvData(std::move(m_rQueue.front()));
				recvData.resize(bytes_transferred);
				assert(!m_rQueue.empty());
				m_rQueue.pop();

				if (ec) {
					boost::system::error_code err;
					dstream4 << "tls::socket " << m_socket.local_endpoint(err) << " <-> " << m_socket.remote_endpoint(err) << ": async_receive failed: " << ec.message();
					m_socket.get_io_service().post([handler = std::forward<ReadHandler>(handler), ec, bytes_transferred]() mutable{handler(ec, bytes_transferred); });
					return;
				}
				if (!m_socket.is_open()) {
					boost::system::error_code err;
					dstream4 << "tls::socket " << m_socket.local_endpoint(err) << " <-> " << m_socket.remote_endpoint(err) << ": async_receive failed: socket was closed before dispatch was finished\n";
					return;
				}

				assert(m_decryptor != nullptr);
				int pending(0);
				auto pData = recvData.data();
				size_t dataSize = recvData.size();
				std::vector<uint8_t> decrypted;
				do{
					std::vector<uint8_t> decryptedPart;
					int retCode = m_decryptor->Decrypt(pData, dataSize, decryptedPart);
					if (retCode == -2) {	// Need more data
						dstream4 << "tls::socket note: Decryptor requested more data.\n";
						AsyncReceive(buffers, std::forward<ReadHandler>(handler));
						return;
					}
					else if (retCode == -1) {
						boost::system::error_code err;
						dstream4 << "tls::socket " << m_socket.local_endpoint(err) << " <-> " << m_socket.remote_endpoint(err) << ": decryption failed\n";
						m_socket.get_io_service().post([handler = std::forward<ReadHandler>(handler)]() mutable{handler(boost::asio::error::operation_aborted, 0); });
						return;
					}

					std::move(decryptedPart.begin(), decryptedPart.end(), std::back_inserter(decrypted));
					pending = m_decryptor->Pending();

					// continue loop without new data
					pData = nullptr;
					dataSize = 0;
				}while (pending > 0);

				boost::asio::buffer_copy(buffers, boost::asio::buffer(decrypted.data(), decrypted.size()));
				m_socket.get_io_service().post([handler = std::forward<ReadHandler>(handler), bytesReceived = decrypted.size()]() mutable{handler(boost::system::error_code(), bytesReceived); });
			}));
		});
	}
	template<typename Func>
	inline void detail::SocketImpl::ResetVerifyCb(int flag, Func && cb)
	{
		vs::event done(true);
		m_strand.dispatch([&]() {
			VS_SCOPE_EXIT{ done.set(); };
			SSL_CTX_set_verify(m_ctx.get(), flag, cb);
			SSL_set_verify(m_ssl.get(), flag, cb);
		});
		done.wait();
	}
}

template<typename ConnectHandler>
void tls::socket::async_connect(const tls::endpoint& endpoint, ConnectHandler&& handler)
{
	m_impl->Connect(endpoint, std::forward<ConnectHandler>(handler));
}

template<typename ConstBufferSequence, typename WriteHandler>
void tls::socket::async_send(const ConstBufferSequence& buffers, WriteHandler&& handler)
{
	m_impl->AsyncSend(buffers, std::forward< WriteHandler>(handler));
}

template<typename MutableBufferSequence, typename ReadHandler>
void tls::socket::async_receive(const MutableBufferSequence& buffers, ReadHandler&& handler)
{
	m_impl->AsyncReceive(buffers, std::forward<ReadHandler>(handler));
}

template<typename Func>
inline void tls::socket::reset_verify_cb(int flag, Func && cb)
{
	m_impl->ResetVerifyCb(flag, std::forward<Func>(cb));
}

#undef DEBUG_CURRENT_MODULE